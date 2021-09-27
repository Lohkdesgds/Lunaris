#include "texture.h"

namespace Lunaris {

	void __bitmap_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_primitives_addon_initialized() && !al_init_primitives_addon()) throw std::runtime_error("Can't start Primitives!");
		if (!al_is_image_addon_initialized() && !al_init_image_addon()) throw std::runtime_error("Can't start Image!");
	}

	bool texture::check_ready() const
	{
		if (!bitmap) return false;
		//al_set_new_bitmap_flags(((al_get_bitmap_flags(bitmap) & ~ALLEGRO_MEMORY_BITMAP) & ~ALLEGRO_CONVERT_BITMAP) | ALLEGRO_VIDEO_BITMAP);
		if ((al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP) && al_get_current_display() != nullptr) {
			//al_set_new_display_flags(ALLEGRO_VIDEO_BITMAP);
			//printf_s("Would convert #%p\n", (void*)bitmap);
			al_convert_bitmap(bitmap);
		}
		return bitmap != nullptr;
	}

	texture::texture(const texture_config& conf)
	{
		if (!create(conf)) throw std::runtime_error("Can't create bitmap!");
	}

	texture::~texture()
	{
		destroy();
	}

	texture::texture(texture&& oth) noexcept
		: bitmap(oth.bitmap)
	{
		oth.bitmap = nullptr;
	}

	void texture::operator=(texture&& oth) noexcept
	{
		destroy();
		bitmap = oth.bitmap;
		oth.bitmap = nullptr;
	}

	bool texture::create(const texture_config& conf)
	{
		__bitmap_allegro_start();

		if (conf.format > 0) 
			al_set_new_bitmap_format(conf.format);
		if (conf.flags > 0) 
			al_set_new_bitmap_flags(conf.flags);

		if (!conf.path.empty()) {
			bitmap = al_load_bitmap(conf.path.c_str());
		}
		else if (conf.width > 0 && conf.height > 0) {
			bitmap = al_create_bitmap(conf.width, conf.height);
		}
		else throw std::runtime_error("Invalid bitmap creation configuration!");

		return bitmap != nullptr;
	}

	bool texture::create(const int width, const int height)
	{
		texture_config conf;
		conf.width = width;
		conf.height = height;
		return create(conf);
	}

	bool texture::create(const std::string& path)
	{
		texture_config conf;
		conf.path = path;
		return create(conf);
	}

	bool texture::load(const texture_config& conf)
	{
		return create(conf);
	}

	bool texture::load(const std::string& path)
	{
		texture_config conf;
		conf.path = path;
		return create(conf);
	}

	texture texture::duplicate()
	{
		if (!bitmap) throw std::runtime_error("Invalid texture!");
		texture temp;
		if (!(temp.bitmap = al_clone_bitmap(bitmap))) throw std::runtime_error("Can't duplicate texture!");
		return temp;
	}

	texture texture::create_sub(const int px, const int py, const int dx, const int dy)
	{
		if (!bitmap) throw std::runtime_error("Invalid texture!");
		if (px + dx > al_get_bitmap_width(bitmap) || py + dy > al_get_bitmap_height(bitmap) || px < 0 || py < 0 || dx <= 0 || dy <= 0) throw std::runtime_error("Invalid size or position to create a sub texture!");
		texture temp;
		if (!(temp.bitmap = al_create_sub_bitmap(bitmap, px, py, dx, dy))) throw std::runtime_error("Can't create sub bitmap!");
		return temp;
	}

	int texture::get_width() const
	{
		if (bitmap) return al_get_bitmap_width(bitmap);
		return 0;
	}

	int texture::get_height() const
	{
		if (bitmap) return al_get_bitmap_height(bitmap);
		return 0;
	}

	int texture::get_format() const
	{
		if (bitmap) return al_get_bitmap_format(bitmap);
		return 0;
	}

	int texture::get_flags() const
	{
		if (bitmap) return al_get_bitmap_flags(bitmap);
		return 0;
	}

	bool texture::empty() const
	{
		return bitmap == nullptr;
	}

	void texture::destroy()
	{
		if (bitmap) {
			al_destroy_bitmap(bitmap);
			bitmap = nullptr;
		}
	}

	void texture::draw_at(const float x, const float y, const int flags) const
	{
		if (check_ready()) al_draw_bitmap(bitmap, x, y, flags);
	}

	void texture::draw_tinted_at(const color& color, const float x, const float y, const int flags) const
	{
		if (check_ready()) al_draw_tinted_bitmap(bitmap, color, x, y, flags);
	}

	void texture::draw_region_at(const float sx, const float sy, const float dx, const float dy, const float px, const float py, const int flags) const
	{
		if (check_ready()) al_draw_bitmap_region(bitmap, sx, sy, dx, dy, px, py, flags);
	}

	void texture::draw_tinted_region_at(const color& color, const float sx, const float sy, const float dx, const float dy, const float px, const float py, const int flags) const
	{
		if (check_ready()) al_draw_tinted_bitmap_region(bitmap, color, sx, sy, dx, dy, px, py, flags);
	}

	void texture::draw_rotated_at(const float cx, const float cy, const float dx, const float dy, const float angle, const int flags) const
	{
		if (check_ready()) al_draw_rotated_bitmap(bitmap, get_width() * ((cx + 1.0f) * 0.5f), get_height() * ((cy + 1.0f) * 0.5f), dx, dy, angle, flags);
	}

	void texture::draw_tinted_rotated_at(const color& color, const float cx, const float cy, const float dx, const float dy, const float angle, const int flags) const
	{
		if (check_ready()) al_draw_tinted_rotated_bitmap(bitmap, color, get_width() * ((cx + 1.0f) * 0.5f), get_height() * ((cy + 1.0f) * 0.5f), dx, dy, angle, flags);
	}

	void texture::draw_scaled_rotated_at(const float cx, const float cy, const float dx, const float dy, const float sx, const float sy, const float angle, const int flags) const
	{
		if (check_ready()) al_draw_scaled_rotated_bitmap(bitmap, get_width() * ((cx + 1.0f) * 0.5f), get_height() * ((cy + 1.0f) * 0.5f), dx, dy, sx, sy, angle, flags);
	}

	void texture::draw_tinted_scaled_rotated_at(const color& color, const float cx, const float cy, const float dx, const float dy, const float sx, const float sy, const float angle, const int flags) const
	{
		if (check_ready()) al_draw_tinted_scaled_rotated_bitmap(bitmap, color, get_width() * ((cx + 1.0f) * 0.5f), get_height() * ((cy + 1.0f) * 0.5f), dx, dy, sx, sy, angle, flags);
	}

	void texture::draw_scaled_at(const float dx, const float dy, const float dw, const float dh, const int flags) const
	{
		if (check_ready()) al_draw_scaled_bitmap(bitmap, 0, 0, get_width(), get_height(), dx, dy, dw, dh, flags);
	}

	void texture::draw_scaled_region_at(const float sx, const float sy, const float sw, const float sh, const float dx, const float dy, const float dw, const float dh, const int flags) const
	{
		if (check_ready()) al_draw_scaled_bitmap(bitmap, sx, sy, sw, sh, dx, dy, dw, dh, flags);
	}

	void texture::draw_tinted_scaled_at(const color& color, const float dx, const float dy, const float dw, const float dh, const int flags) const
	{
		if (check_ready()) al_draw_tinted_scaled_bitmap(bitmap, color, 0, 0, get_width(), get_height(), dx, dy, dw, dh, flags);
	}

	void texture::draw_tinted_scaled_region_at(const color& color, const float sx, const float sy, const float sw, const float sh, const float dx, const float dy, const float dw, const float dh, const int flags) const
	{
		if (check_ready()) al_draw_tinted_scaled_bitmap(bitmap, color, sx, sy, sw, sh, dx, dy, dw, dh, flags);
	}

	void texture::set_as_target() const
	{
		if (bitmap) al_set_target_bitmap(bitmap);
	}

}
