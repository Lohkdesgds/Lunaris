#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include <stdexcept>
#include <string>

// deps
#include "../color.h"

namespace Lunaris {

	void __bitmap_allegro_start();

	// width and height OR path. Path is chosen if both situations.
	struct texture_config {
		int format = ALLEGRO_PIXEL_FORMAT_ANY;
		int flags = ALLEGRO_CONVERT_BITMAP | ALLEGRO_MIN_LINEAR;
		int width = 0;
		int height = 0;
		std::string path;
	};

	class texture {
		ALLEGRO_BITMAP* bitmap = nullptr;

		bool check_ready() const;
	public:
		texture() = default;
		texture(const texture_config&);
		~texture();

		texture(texture&&) noexcept;
		void operator=(texture&&) noexcept;

		texture(const texture&) = delete;
		void operator=(const texture&) = delete;

		bool create(const texture_config&);
		bool create(const int, const int);
		bool create(const std::string&);

		bool load(const texture_config&);
		bool load(const std::string&);

		texture duplicate();
		texture create_sub(const int, const int, const int, const int);

		int get_width() const;
		int get_height() const;
		int get_format() const;
		int get_flags() const;

		bool empty() const;

		void destroy();

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{float}">Pos X.</param>
		/// <param name="{float}">Pos Y.</param>
		/// <param name="{int}">Flags.</param>
		void draw_at(const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{color}">Tint.</param>
		/// <param name="{float}">Pos X.</param>
		/// <param name="{float}">Pos Y.</param>
		/// <param name="{int}">Flags.</param>
		void draw_tinted_at(const color&, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap region.
		/// </summary>
		/// <param name="{float}">Source Top left X.</param>
		/// <param name="{float}">Source Top left Y.</param>
		/// <param name="{float}">Source Width (pixels).</param>
		/// <param name="{float}">Source Height (pixels).</param>
		/// <param name="{float}">Pos X.</param>
		/// <param name="{float}">Pos Y.</param>
		/// <param name="{int}">Flags.</param>
		void draw_region_at(const float, const float, const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap region.
		/// </summary>
		/// <param name="{color}">Tint.</param>
		/// <param name="{float}">Source Top left X.</param>
		/// <param name="{float}">Source Top left Y.</param>
		/// <param name="{float}">Source Width (pixels).</param>
		/// <param name="{float}">Source Height (pixels).</param>
		/// <param name="{float}">Pos X.</param>
		/// <param name="{float}">Pos Y.</param>
		/// <param name="{int}">Flags.</param>
		void draw_tinted_region_at(const color&, const float, const float, const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{float}">Center X (relative) [-1.0, 1.0].</param>
		/// <param name="{float}">Center Y (relative) [-1.0, 1.0].</param>
		/// <param name="{float}">Pos X.</param>
		/// <param name="{float}">Pos Y.</param>
		/// <param name="{int}">Flags.</param>
		void draw_rotated_at(const float, const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{color}">Tint.</param>
		/// <param name="{float}">Center X (relative) [-1.0, 1.0].</param>
		/// <param name="{float}">Center Y (relative) [-1.0, 1.0].</param>
		/// <param name="{float}">Pos X.</param>
		/// <param name="{float}">Pos Y.</param>
		/// <param name="{int}">Flags.</param>
		void draw_tinted_rotated_at(const color&, const float, const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{float}">Center X (relative) [-1.0, 1.0].</param>
		/// <param name="{float}">Center Y (relative) [-1.0, 1.0].</param>
		/// <param name="{float}">Pos X.</param>
		/// <param name="{float}">Pos Y.</param>
		/// <param name="{float}">Scale X.</param>
		/// <param name="{float}">Scale Y.</param>
		/// <param name="{int}">Flags.</param>
		void draw_scaled_rotated_at(const float, const float, const float, const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{color}">Tint.</param>
		/// <param name="{float}">Center X (relative) [-1.0, 1.0].</param>
		/// <param name="{float}">Center Y (relative) [-1.0, 1.0].</param>
		/// <param name="{float}">Pos X.</param>
		/// <param name="{float}">Pos Y.</param>
		/// <param name="{float}">Scale X.</param>
		/// <param name="{float}">Scale Y.</param>
		/// <param name="{int}">Flags.</param>
		void draw_tinted_scaled_rotated_at(const color&, const float, const float, const float, const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{float}">Dest. Top left X.</param>
		/// <param name="{float}">Dest. Top left Y.</param>
		/// <param name="{float}">Dest. Width (pixels).</param>
		/// <param name="{float}">Dest. Height (pixels).</param>
		/// <param name="{int}">Flags.</param>
		void draw_scaled_at(const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{float}">Source Top left X.</param>
		/// <param name="{float}">Source Top left Y.</param>
		/// <param name="{float}">Source Width (pixels).</param>
		/// <param name="{float}">Source Height (pixels).</param>
		/// <param name="{float}">Dest. Top left X.</param>
		/// <param name="{float}">Dest. Top left Y.</param>
		/// <param name="{float}">Dest. Width (pixels).</param>
		/// <param name="{float}">Dest. Height (pixels).</param>
		/// <param name="{int}">Flags.</param>
		void draw_scaled_region_at(const float, const float, const float, const float, const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{color}">Tint.</param>
		/// <param name="{float}">Dest. Top left X.</param>
		/// <param name="{float}">Dest. Top left Y.</param>
		/// <param name="{float}">Dest. Width (pixels).</param>
		/// <param name="{float}">Dest. Height (pixels).</param>
		/// <param name="{int}">Flags.</param>
		void draw_tinted_scaled_at(const color&, const float, const float, const float, const float, const int = 0) const;

		/// <summary>
		/// Draw bitmap.
		/// </summary>
		/// <param name="{color}">Tint.</param>
		/// <param name="{float}">Source Top left X.</param>
		/// <param name="{float}">Source Top left Y.</param>
		/// <param name="{float}">Source Width (pixels).</param>
		/// <param name="{float}">Source Height (pixels).</param>
		/// <param name="{float}">Dest. Top left X.</param>
		/// <param name="{float}">Dest. Top left Y.</param>
		/// <param name="{float}">Dest. Width (pixels).</param>
		/// <param name="{float}">Dest. Height (pixels).</param>
		/// <param name="{int}">Flags.</param>
		void draw_tinted_scaled_region_at(const color&, const float, const float, const float, const float, const float, const float, const float, const float, const int = 0) const;

		void set_as_target() const;
	};
}
