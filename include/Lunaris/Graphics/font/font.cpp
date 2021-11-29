#include "font.h"

namespace Lunaris {

	LUNARIS_DECL void __font_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_primitives_addon_initialized() && !al_init_primitives_addon()) throw std::runtime_error("Can't start Primitives!");
		if (!al_is_font_addon_initialized() && !al_init_font_addon()) throw std::runtime_error("Can't start Font!");
		if (!al_is_ttf_addon_initialized() && !al_init_ttf_addon()) throw std::runtime_error("Can't start TTF Font!");
	}

	LUNARIS_DECL font_config& font_config::set_bitmap_flags(const int var)
	{
		bmp_flags = var;
		return *this;
	}

	LUNARIS_DECL font_config& font_config::set_font_flags(const int var)
	{
		font_flags = var;
		return *this;
	}

	LUNARIS_DECL font_config& font_config::set_is_ttf(const bool var)
	{
		ttf = var;
		return *this;
	}

	LUNARIS_DECL font_config& font_config::set_resolution(const int var)
	{
		resolution = var;
		return *this;
	}

	LUNARIS_DECL font_config& font_config::set_path(const std::string& var)
	{
		path = var;
		fileref.reset_this();
		return *this;
	}

	LUNARIS_DECL font_config& font_config::set_file(const hybrid_memory<file>& var)
	{
		fileref = var;
		path.clear();
		return *this;
	}

	LUNARIS_DECL bool font::check_ready() const
	{
		return font_ptr != nullptr;
	}

	LUNARIS_DECL font::font(const font_config& conf)
	{
		if ((!conf.path.empty()) || (!conf.fileref.empty() && conf.fileref->size() > 0)) {
			if (!load(conf)) throw std::runtime_error("Can't create font!");
		}
		else if (!create_builtin_font()) throw std::runtime_error("Can't create builtin font!");
	}

	LUNARIS_DECL font::~font()
	{
		destroy();
	}

	LUNARIS_DECL font::font(font&& oth) noexcept
		: font_ptr(oth.font_ptr), fileref(std::move(oth.fileref))
	{		
		oth.font_ptr = nullptr;
	}

	LUNARIS_DECL void font::operator=(font&& oth) noexcept
	{
		destroy();
		fileref = std::move(oth.fileref);
		font_ptr = oth.font_ptr;
		oth.font_ptr = nullptr;
	}

	LUNARIS_DECL bool font::create_builtin_font()
	{
		__font_allegro_start();

#ifdef LUNARIS_VERBOSE_BUILD
		if (font_ptr) PRINT_DEBUG("Recreating font for %p (font class %p) [builtin]", font_ptr, this);
		else PRINT_DEBUG("Creating font (font class %p) [builtin]", this);
#endif

		destroy();

		font_ptr = al_create_builtin_font();

#ifdef LUNARIS_VERBOSE_BUILD
		if (font_ptr) PRINT_DEBUG("Good [builtin] font %p (font class %p)", font_ptr, this);
		else PRINT_DEBUG("Bad [builtin] font creation (font class %p)", this);
#endif

		return font_ptr != nullptr;
	}

	LUNARIS_DECL bool font::load(const font_config& conf)
	{
		__font_allegro_start();

#ifdef LUNARIS_VERBOSE_BUILD
		if (font_ptr) PRINT_DEBUG("Recreating font for %p (font class %p)", font_ptr, this);
		else PRINT_DEBUG("Creating font (font class %p)", this);
#endif

		destroy();

		if (conf.bmp_flags != 0)
			al_set_new_bitmap_flags(conf.bmp_flags);

		if (conf.ttf) {
			if (!conf.path.empty()) font_ptr = al_load_ttf_font(conf.path.c_str(), conf.resolution, conf.font_flags);
			else if (!conf.fileref.empty() && conf.fileref->size() > 0) {
				fileref = conf.fileref;
				fileref->modify_no_destroy(true); // very specific: https://www.allegro.cc/manual/5/al_load_ttf_font_f: `The file handle is owned by the returned ALLEGRO_FONT object and must not be freed by the caller, as FreeType expects to be able to read from it at a later time.`
				fileref->seek(0, file::seek_mode_e::BEGIN);
				font_ptr = al_load_ttf_font_f(fileref->get_fp(), fileref->get_path().c_str(), conf.resolution, conf.font_flags);
				if (!font_ptr) {
					fileref->modify_no_destroy(false); // as this is not hooked like file, I think it can just die lol
					font_ptr = al_load_ttf_font(fileref->get_path().c_str(), conf.resolution, conf.font_flags); // hacky way, file will still be there
				}
			}
		}
		else {
			if (!conf.path.empty()) font_ptr = al_load_font(conf.path.c_str(), conf.resolution, conf.font_flags);
			else if (!conf.fileref.empty() && conf.fileref->size() > 0) { // as of a hacky way
				fileref = conf.fileref;
				fileref->seek(0, file::seek_mode_e::BEGIN);
				font_ptr = al_load_font(fileref->get_path().c_str(), conf.resolution, conf.font_flags); // al_load_font_f is not supported
			}
		}

#ifdef LUNARIS_VERBOSE_BUILD
		if (font_ptr) PRINT_DEBUG("Good font %p (font class %p)", font_ptr, this);
		else PRINT_DEBUG("Bad font creation (font class %p)", this);
#endif

		return font_ptr != nullptr;
	}

	LUNARIS_DECL bool font::load(const std::string& path, const bool ttf)
	{
		font_config conf;
		conf.path = path;
		conf.ttf = ttf;
		return load(conf);
	}

	LUNARIS_DECL bool font::load(const hybrid_memory<file>& ref)
	{
		font_config conf;
		conf.fileref = ref;
		conf.ttf = true;
		return load(conf);
	}

	LUNARIS_DECL ALLEGRO_FONT* font::get_raw_font() const
	{
		return font_ptr;
	}

	LUNARIS_DECL bool font::empty() const
	{
		return font_ptr == nullptr;
	}

	LUNARIS_DECL bool font::valid() const
	{
		return font_ptr != nullptr;
	}

	LUNARIS_DECL void font::destroy()
	{
		if (font_ptr) {
#ifdef LUNARIS_VERBOSE_BUILD
			PRINT_DEBUG("Del font %p", font_ptr);
#endif
			al_destroy_font(font_ptr);
			font_ptr = nullptr;
		}
		fileref.reset_this();
	}

	LUNARIS_DECL int font::get_line_ascent() const
	{
		if (check_ready()) return al_get_font_ascent(font_ptr);
		return 0;
	}

	LUNARIS_DECL int font::get_line_descent() const
	{
		if (check_ready()) return al_get_font_descent(font_ptr);
		return 0;
	}

	LUNARIS_DECL int font::get_line_height() const
	{
		if (check_ready()) return al_get_font_line_height(font_ptr);
		return 0;
	}

	LUNARIS_DECL int font::get_width(const std::string& str) const
	{
		if (check_ready()) return al_get_text_width(font_ptr, str.c_str());
		return 0;
	}

	LUNARIS_DECL void font::draw(color c, const float x, const float y, const int f, const std::string& s) const
	{
		if (check_ready()) al_draw_text(font_ptr, c, x, y, f, s.c_str());
	}

}