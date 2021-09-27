#include "font.h"

namespace Lunaris {

	void __font_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_primitives_addon_initialized() && !al_init_primitives_addon()) throw std::runtime_error("Can't start Primitives!");
		if (!al_is_font_addon_initialized() && !al_init_font_addon()) throw std::runtime_error("Can't start Font!");
		if (!al_is_ttf_addon_initialized() && !al_init_ttf_addon()) throw std::runtime_error("Can't start TTF Font!");
	}

	bool font::check_ready() const
	{
		return font_ptr != nullptr;
	}

	font::font(const font_config& conf)
	{
		if (conf.path.empty()) {
			if (!create_builtin_font()) throw std::runtime_error("Can't create builtin font!");
		}
		else if (!load(conf)) throw std::runtime_error("Can't create font!");
	}

	font::~font()
	{
		destroy();
	}

	font::font(font&& oth) noexcept
		: font_ptr(oth.font_ptr)
	{
		oth.font_ptr = nullptr;
	}

	void font::operator=(font&& oth) noexcept
	{
		destroy();
		font_ptr = oth.font_ptr;
		oth.font_ptr = nullptr;
	}

	bool font::create_builtin_font()
	{
		__font_allegro_start();
		destroy();

		return (font_ptr = al_create_builtin_font()) != nullptr;
	}

	bool font::load(const font_config& conf)
	{
		__font_allegro_start();
		destroy();

		if (conf.path.empty() || conf.resolution == 0) 
			return false;

		if (conf.bmp_flags != 0)
			al_set_new_bitmap_flags(conf.bmp_flags);

		if (conf.ttf) {
			font_ptr = al_load_ttf_font(conf.path.c_str(), conf.resolution, conf.font_flags);
		}
		else {
			font_ptr = al_load_font(conf.path.c_str(), conf.resolution, conf.font_flags);
		}

		return font_ptr != nullptr;
	}

	bool font::load(const std::string& path, const bool ttf)
	{
		font_config conf;
		conf.path = path;
		conf.ttf = ttf;
		return load(conf);
	}

	void font::destroy()
	{
		if (font_ptr) {
			al_destroy_font(font_ptr);
			font_ptr = nullptr;
		}
	}

	int font::get_line_ascent() const
	{
		if (check_ready()) return al_get_font_ascent(font_ptr);
		return 0;
	}

	int font::get_line_descent() const
	{
		if (check_ready()) return al_get_font_descent(font_ptr);
		return 0;
	}

	int font::get_line_height() const
	{
		if (check_ready()) return al_get_font_line_height(font_ptr);
		return 0;
	}

	int font::get_width(const std::string& str) const
	{
		if (check_ready()) return al_get_text_width(font_ptr, str.c_str());
		return 0;
	}

	void font::draw(color c, const float x, const float y, const int f, const std::string& s) const
	{
		if (check_ready()) al_draw_text(font_ptr, c, x, y, f, s.c_str());
	}

}