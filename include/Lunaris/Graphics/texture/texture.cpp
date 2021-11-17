#include "texture.h"

namespace Lunaris {

	LUNARIS_DECL void __bitmap_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_primitives_addon_initialized() && !al_init_primitives_addon()) throw std::runtime_error("Can't start Primitives!");
		if (!al_is_image_addon_initialized() && !al_init_image_addon()) throw std::runtime_error("Can't start Image!");
	}

	LUNARIS_DECL texture_config& texture_config::set_format(const int var)
	{
		format = var;
		return *this;
	}

	LUNARIS_DECL texture_config& texture_config::set_flags(const int var)
	{
		flags = var;
		return *this;
	}

	LUNARIS_DECL texture_config& texture_config::set_width(const int var)
	{
		width = var;
		return *this;
	}

	LUNARIS_DECL texture_config& texture_config::set_height(const int var)
	{
		height = var;
		return *this;
	}

	LUNARIS_DECL texture_config& texture_config::set_path(const std::string& str)
	{
		path = str;
		fileref.reset_this();
		return *this;
	}

	LUNARIS_DECL texture_config& texture_config::set_file(const hybrid_memory<file>& fp)
	{
		fileref = fp;
		path.clear();
		return *this;
	}

	LUNARIS_DECL bool texture::check_ready() const
	{
		if (!bitmap) return false;
		//al_set_new_bitmap_flags(((al_get_bitmap_flags(bitmap) & ~ALLEGRO_MEMORY_BITMAP) & ~ALLEGRO_CONVERT_BITMAP) | ALLEGRO_VIDEO_BITMAP);
		if ((al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP) && al_get_current_display() != nullptr) {
#ifdef LUNARIS_VERBOSE_BUILD
			PRINT_DEBUG("Converting bitmap %p", bitmap);
#endif
			al_convert_bitmap(bitmap);
		}
		return bitmap != nullptr;
	}

	LUNARIS_DECL texture::texture(const texture_config& conf)
	{
		if (!create(conf)) throw std::runtime_error("Can't create bitmap!");
	}

	LUNARIS_DECL texture::~texture()
	{
		destroy();
	}

	LUNARIS_DECL texture::texture(texture&& oth) noexcept
		: bitmap(oth.bitmap), fileref(std::move(oth.fileref))
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Moved bitmap (new <-  %p)", oth.bitmap);
#endif
		oth.bitmap = nullptr;
	}

	LUNARIS_DECL void texture::operator=(texture&& oth) noexcept
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Moved bitmap (%p <- %p)", bitmap, oth.bitmap);
#endif
		destroy();
		bitmap = oth.bitmap;
		fileref = std::move(oth.fileref);
		oth.bitmap = nullptr;
	}

	LUNARIS_DECL bool texture::create(const texture_config& conf)
	{
		__bitmap_allegro_start();

#ifdef LUNARIS_VERBOSE_BUILD
		if (bitmap) PRINT_DEBUG("Recreating bitmap for %p (texture %p)", bitmap, this);
		else PRINT_DEBUG("Creating bitmap (texture %p)", this);
#endif
		destroy();

		if (conf.format > 0) 
			al_set_new_bitmap_format(conf.format);
		if (conf.flags > 0) 
			al_set_new_bitmap_flags(conf.flags);

		if (!conf.path.empty()) {
			bitmap = al_load_bitmap(conf.path.c_str());
		}
		else if (!conf.fileref.empty() && conf.fileref->size() > 0) {
			fileref = conf.fileref;
			fileref->seek(0, file::seek_mode_e::BEGIN);
			bitmap = al_load_bitmap_f(fileref->get_fp(), nullptr); // al_identify_bitmap_f(fileref->get_fp())
			if (!bitmap) bitmap = al_load_bitmap(fileref->get_path().c_str()); // the path will still work while the file is hooked here lol
		}
		else if (conf.width > 0 && conf.height > 0) {
			bitmap = al_create_bitmap(conf.width, conf.height);
		}
		else throw std::runtime_error("Invalid bitmap creation configuration!");

#ifdef LUNARIS_VERBOSE_BUILD
		if (bitmap) PRINT_DEBUG("Good bitmap %p (texture %p)", bitmap, this);
		else PRINT_DEBUG("Bad bitmap creation (texture %p)", this);
#endif

		return bitmap != nullptr;
	}

	LUNARIS_DECL bool texture::create(const int width, const int height)
	{
		texture_config conf;
		conf.width = width;
		conf.height = height;
		return create(conf);
	}

	LUNARIS_DECL bool texture::create(const std::string& path)
	{
		texture_config conf;
		conf.path = path;
		return create(conf);
	}

	LUNARIS_DECL bool texture::load(const texture_config& conf)
	{
		return create(conf);
	}

	LUNARIS_DECL bool texture::load(const std::string& path)
	{
		texture_config conf;
		conf.path = path;
		return create(conf);
	}

	LUNARIS_DECL bool texture::load(hybrid_memory<file> ref)
	{
		texture_config conf;
		conf.fileref = ref;
		return create(conf);
	}

	LUNARIS_DECL texture texture::duplicate()
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Duplicating bitmap %p", bitmap);
#endif
		ALLEGRO_BITMAP* bmp = get_raw_bitmap();
		if (!bmp) throw std::runtime_error("Invalid texture!");
		texture temp;
		if (!(temp.bitmap = al_clone_bitmap(bmp))) throw std::runtime_error("Can't duplicate texture!");
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Duplicated bitmap (%p -> %p)", bitmap, temp.bitmap);
#endif
		return temp;
	}

	LUNARIS_DECL texture texture::create_sub(const int px, const int py, const int dx, const int dy)
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Sub creating bitmap %p", bitmap);
#endif
		ALLEGRO_BITMAP* bmp = get_raw_bitmap();
		if (!bmp) throw std::runtime_error("Invalid texture!");
		if (px + dx > al_get_bitmap_width(bmp) || py + dy > al_get_bitmap_height(bmp) || px < 0 || py < 0 || dx <= 0 || dy <= 0) throw std::runtime_error("Invalid size or position to create a sub texture!");
		texture temp;
		if (!(temp.bitmap = al_create_sub_bitmap(bmp, px, py, dx, dy))) throw std::runtime_error("Can't create sub bitmap!");
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Sub created bitmap (%p -> %p)", bitmap, temp.bitmap);
#endif
		return temp;
	}

	LUNARIS_DECL int texture::get_width() const
	{
		if (auto bmp = get_raw_bitmap(); bmp) return al_get_bitmap_width(bmp);
		return 0;
	}

	LUNARIS_DECL int texture::get_height() const
	{
		if (auto bmp = get_raw_bitmap(); bmp) return al_get_bitmap_height(bmp);
		return 0;
	}

	LUNARIS_DECL int texture::get_format() const
	{
		if (auto bmp = get_raw_bitmap(); bmp) return al_get_bitmap_format(bmp);
		return 0;
	}

	LUNARIS_DECL int texture::get_flags() const
	{
		if (auto bmp = get_raw_bitmap(); bmp) return al_get_bitmap_flags(bmp);
		return 0;
	}

	LUNARIS_DECL ALLEGRO_BITMAP* texture::get_raw_bitmap() const
	{
		return bitmap;
	}

	LUNARIS_DECL texture::operator ALLEGRO_BITMAP* () const
	{
		return get_raw_bitmap();
	}

	LUNARIS_DECL bool texture::empty() const
	{
		return bitmap == nullptr;
	}

	LUNARIS_DECL void texture::destroy()
	{
		if (bitmap) {
#ifdef LUNARIS_VERBOSE_BUILD
			PRINT_DEBUG("Del bitmap %p", bitmap);
#endif
			al_destroy_bitmap(bitmap);
			bitmap = nullptr;
		}
	}

	LUNARIS_DECL void texture::draw_at(const float x, const float y, const int flags) const
	{
		if (check_ready()) al_draw_bitmap(get_raw_bitmap(), x, y, flags);
	}

	LUNARIS_DECL void texture::draw_tinted_at(const color& color, const float x, const float y, const int flags) const
	{
		if (check_ready()) al_draw_tinted_bitmap(get_raw_bitmap(), color, x, y, flags);
	}

	LUNARIS_DECL void texture::draw_region_at(const float sx, const float sy, const float dx, const float dy, const float px, const float py, const int flags) const
	{
		if (check_ready()) al_draw_bitmap_region(get_raw_bitmap(), sx, sy, dx, dy, px, py, flags);
	}

	LUNARIS_DECL void texture::draw_tinted_region_at(const color& color, const float sx, const float sy, const float dx, const float dy, const float px, const float py, const int flags) const
	{
		if (check_ready()) al_draw_tinted_bitmap_region(get_raw_bitmap(), color, sx, sy, dx, dy, px, py, flags);
	}

	LUNARIS_DECL void texture::draw_rotated_at(const float cx, const float cy, const float dx, const float dy, const float angle, const int flags) const
	{
		if (check_ready()) al_draw_rotated_bitmap(get_raw_bitmap(), get_width() * ((cx + 1.0f) * 0.5f), get_height() * ((cy + 1.0f) * 0.5f), dx, dy, angle, flags);
	}

	LUNARIS_DECL void texture::draw_tinted_rotated_at(const color& color, const float cx, const float cy, const float dx, const float dy, const float angle, const int flags) const
	{
		if (check_ready()) al_draw_tinted_rotated_bitmap(get_raw_bitmap(), color, get_width() * ((cx + 1.0f) * 0.5f), get_height() * ((cy + 1.0f) * 0.5f), dx, dy, angle, flags);
	}

	LUNARIS_DECL void texture::draw_scaled_rotated_at(const float cx, const float cy, const float dx, const float dy, const float sx, const float sy, const float angle, const int flags) const
	{
		if (check_ready()) al_draw_scaled_rotated_bitmap(get_raw_bitmap(), get_width() * ((cx + 1.0f) * 0.5f), get_height() * ((cy + 1.0f) * 0.5f), dx, dy, sx, sy, angle, flags);
	}

	LUNARIS_DECL void texture::draw_tinted_scaled_rotated_at(const color& color, const float cx, const float cy, const float dx, const float dy, const float sx, const float sy, const float angle, const int flags) const
	{
		if (check_ready()) al_draw_tinted_scaled_rotated_bitmap(get_raw_bitmap(), color, get_width() * ((cx + 1.0f) * 0.5f), get_height() * ((cy + 1.0f) * 0.5f), dx, dy, sx, sy, angle, flags);
	}

	LUNARIS_DECL void texture::draw_scaled_at(const float dx, const float dy, const float dw, const float dh, const int flags) const
	{
		if (check_ready()) al_draw_scaled_bitmap(get_raw_bitmap(), 0, 0, get_width(), get_height(), dx, dy, dw, dh, flags);
	}

	LUNARIS_DECL void texture::draw_scaled_region_at(const float sx, const float sy, const float sw, const float sh, const float dx, const float dy, const float dw, const float dh, const int flags) const
	{
		if (check_ready()) al_draw_scaled_bitmap(get_raw_bitmap(), sx, sy, sw, sh, dx, dy, dw, dh, flags);
	}

	LUNARIS_DECL void texture::draw_tinted_scaled_at(const color& color, const float dx, const float dy, const float dw, const float dh, const int flags) const
	{
		if (check_ready()) al_draw_tinted_scaled_bitmap(get_raw_bitmap(), color, 0, 0, get_width(), get_height(), dx, dy, dw, dh, flags);
	}

	LUNARIS_DECL void texture::draw_tinted_scaled_region_at(const color& color, const float sx, const float sy, const float sw, const float sh, const float dx, const float dy, const float dw, const float dh, const int flags) const
	{
		if (check_ready()) al_draw_tinted_scaled_bitmap(get_raw_bitmap(), color, sx, sy, sw, sh, dx, dy, dw, dh, flags);
	}

	LUNARIS_DECL void texture::set_as_target() const
	{
		if (bitmap) al_set_target_bitmap(bitmap);
	}

	LUNARIS_DECL bool texture_gif::check_ready() const
	{
		if (!animation) return false;
		if (animation->frames_count == 0) return false;

		if ((al_get_bitmap_flags(animation->frames[0].rendered) & ALLEGRO_MEMORY_BITMAP) && al_get_current_display() != nullptr) {
			for (int fram = 0; fram < animation->frames_count; fram++) {
				ALLEGRO_BITMAP* it = animation->frames[fram].rendered;
				al_convert_bitmap(it);
			}
		}
		return animation != nullptr;
	}

	LUNARIS_DECL texture_gif::~texture_gif()
	{
		destroy();
	}

	LUNARIS_DECL texture_gif::texture_gif(texture_gif&& oth) noexcept
		: animation(oth.animation)
	{
		bitmap = oth.bitmap;
		fileref = std::move(oth.fileref);
		oth.animation = nullptr;
		oth.bitmap = nullptr;
	}

	LUNARIS_DECL void texture_gif::operator=(texture_gif&& oth) noexcept
	{
		destroy();

		animation = oth.animation;
		bitmap = oth.bitmap;
		fileref = std::move(oth.fileref);
		oth.animation = nullptr;
		oth.bitmap = nullptr;
	}

	LUNARIS_DECL bool texture_gif::load(const std::string& path)
	{
		__bitmap_allegro_start();
		destroy();

		if (path.empty()) return false;
		if (!(animation = algif_load_animation(path.c_str()))) return false;

		start_time = al_get_time();

		bitmap = get_raw_bitmap();

		return bitmap != nullptr;
	}

	LUNARIS_DECL bool texture_gif::load(const hybrid_memory<file>& fp)
	{
		__bitmap_allegro_start();
		destroy();

		if (fp.empty() || fp->size() == 0) return false;
		fileref = fp;

		if (!(animation = algif_load_animation_f(fileref->get_fp()))) {
			if (!(animation = algif_load_animation(fileref->get_path().c_str()))) // direct path may work lol, the file will exist until the end (I think)
				return false;
		}
		//fileref->modify_no_destroy(true); // unhappy

		start_time = al_get_time();

		bitmap = get_raw_bitmap();

		return bitmap != nullptr;
		return false;
	}

	LUNARIS_DECL int texture_gif::get_width() const
	{
		return animation ? animation->width : 0;
	}

	LUNARIS_DECL int texture_gif::get_height() const
	{
		return animation ? animation->height : 0;
	}

	LUNARIS_DECL ALLEGRO_BITMAP* texture_gif::get_raw_bitmap() const
	{
		return animation ? algif_get_bitmap(animation, al_get_time() - start_time) : nullptr;
	}

	LUNARIS_DECL bool texture_gif::empty()
	{
		return animation != nullptr && bitmap != nullptr;
	}

	LUNARIS_DECL void texture_gif::destroy()
	{
		if (animation) {
			algif_destroy_animation(animation);
			bitmap = nullptr;
			animation = nullptr;
		}
	}

	LUNARIS_DECL double texture_gif::get_interval_average() const
	{
		if (!animation) return 0.0;
		if (animation->frames_count == 0) return 0.0;

		double total = 0.0;

		for (int p = 0; p < animation->frames_count; p++) {
			total += algif_get_frame_duration(animation, p);
		}

		total /= 1.0 *animation->frames_count;
		return total;
	}

	LUNARIS_DECL double texture_gif::get_interval_longest() const
	{
		if (!animation) return 0.0;
		if (animation->frames_count == 0) return 0.0;

		double total = 0.0;

		for (int p = 0; p < animation->frames_count; p++) {
			const double rn = algif_get_frame_duration(animation, p);
			if (rn > total) total = rn;
		}

		return total;
	}

	LUNARIS_DECL double texture_gif::get_interval_shortest() const
	{
		if (!animation) return 0.0;
		if (animation->frames_count == 0) return 0.0;

		double total = algif_get_frame_duration(animation, 0);

		for (int p = 0; p < animation->frames_count; p++) {
			const double rn = algif_get_frame_duration(animation, p);
			if (rn < total) total = rn;
		}

		return total;
	}

}
