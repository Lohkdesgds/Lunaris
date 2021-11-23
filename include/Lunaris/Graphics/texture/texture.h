#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/color.h>
#include <Lunaris/Imported/algif5.h>
#include <Lunaris/Utility/file.h>
#include <Lunaris/Utility/memory.h>
#include <Lunaris/Utility/mutex.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdexcept>
#include <string>


namespace Lunaris {

	void __bitmap_allegro_start();

	// width and height OR path. Path is chosen if both situations.
	struct texture_config {
		int format = ALLEGRO_PIXEL_FORMAT_ANY;
		int flags = ALLEGRO_CONVERT_BITMAP | ALLEGRO_MIN_LINEAR;
		int width = 0;
		int height = 0;
		std::string path;
		hybrid_memory<file> fileref;

		texture_config& set_format(const int);
		texture_config& set_flags(const int);
		texture_config& set_width(const int);
		texture_config& set_height(const int);
		texture_config& set_path(const std::string&);
		texture_config& set_file(const hybrid_memory<file>&);
	};

	class texture : public NonCopyable {
	protected:
		ALLEGRO_BITMAP* bitmap = nullptr;
		hybrid_memory<file> fileref;

		virtual bool check_ready() const;
	public:
		texture() = default;
		texture(const texture_config&);
		virtual ~texture();

		texture(texture&&) noexcept;
		void operator=(texture&&) noexcept;

		bool create(const texture_config&);
		bool create(const int, const int);
		bool create(const std::string&);

		bool load(const texture_config&);
		virtual bool load(const std::string&);
		virtual bool load(hybrid_memory<file>);

		texture duplicate();
		texture create_sub(const int, const int, const int, const int);

		virtual int get_width() const;
		virtual int get_height() const;
		int get_format() const;
		int get_flags() const;

		virtual ALLEGRO_BITMAP* get_raw_bitmap() const;
		virtual operator ALLEGRO_BITMAP* () const;

		virtual bool empty() const;

		virtual void destroy();

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

	class functional_texture : public texture {
		std::function<void(ALLEGRO_BITMAP*)> func; // tied to check_ready()
		mutable fast_one_way_mutex fastmu;

		bool check_ready() const;
	public:
		functional_texture() = default;
		~functional_texture();

		functional_texture(functional_texture&&) noexcept;
		void operator=(functional_texture&&) noexcept;

		void hook_function(std::function<void(ALLEGRO_BITMAP*)>);
		void unhook_function();

		using texture::create;
		using texture::load;
		using texture::duplicate;
		using texture::create_sub;
		using texture::get_width;
		using texture::get_height;
		using texture::get_format;
		using texture::get_flags;
		using texture::get_raw_bitmap;
		using texture::operator ALLEGRO_BITMAP*;
		using texture::empty;
		using texture::destroy;
		using texture::draw_at;
		using texture::draw_tinted_at;
		using texture::draw_region_at;
		using texture::draw_tinted_region_at;
		using texture::draw_rotated_at;
		using texture::draw_tinted_rotated_at;
		using texture::draw_scaled_rotated_at;
		using texture::draw_tinted_scaled_rotated_at;
		using texture::draw_scaled_at;
		using texture::draw_scaled_region_at;
		using texture::draw_tinted_scaled_at;
		using texture::draw_tinted_scaled_region_at;
	};

	class texture_gif : public texture {
		ALGIF_ANIMATION* animation = nullptr;
		double start_time = 0.0;

		bool check_ready() const;
	public:
		texture_gif() = default;
		~texture_gif();

		texture_gif(texture_gif&&) noexcept;
		void operator=(texture_gif&&) noexcept;

		bool load(const std::string&);
		bool load(const hybrid_memory<file>&);

		int get_width() const;
		int get_height() const;

		ALLEGRO_BITMAP* get_raw_bitmap() const;
		operator ALLEGRO_BITMAP* () const;
		bool empty();
		void destroy();

		double get_interval_average() const;
		double get_interval_longest() const;
		double get_interval_shortest() const;

		using texture::duplicate;
		using texture::draw_at;
		using texture::draw_tinted_at;
		using texture::draw_region_at;
		using texture::draw_tinted_region_at;
		using texture::draw_rotated_at;
		using texture::draw_tinted_rotated_at;
		using texture::draw_scaled_rotated_at;
		using texture::draw_tinted_scaled_rotated_at;
		using texture::draw_scaled_at;
		using texture::draw_scaled_region_at;
		using texture::draw_tinted_scaled_at;
		using texture::draw_tinted_scaled_region_at;
	};
}
