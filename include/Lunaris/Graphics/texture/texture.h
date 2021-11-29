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

	/// <summary>
	/// <para>Texture configuration.</para>
	/// <para>You can't have size and path set at the same time. Only one of them is used.</para>
	/// </summary>
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

	/// <summary>
	/// <para>texture is the bitmap handler you'll use everywhere.</para>
	/// <para>This easily create, destroy, duplicate and draw the bitmap in a very easy way.</para>
	/// </summary>
	class texture : public NonCopyable {
	protected:
		ALLEGRO_BITMAP* bitmap = nullptr;
		hybrid_memory<file> fileref;

		virtual bool check_ready() const;
	public:
		texture() = default;
		texture(const texture_config&);
		virtual ~texture();

		/// <summary>
		/// <para>Move a texture to this.</para>
		/// </summary>
		/// <param name="{texture}">Texture being moved.</param>
		texture(texture&&) noexcept;

		/// <summary>
		/// <para>Move a texture to this.</para>
		/// </summary>
		/// <param name="{texture}">Texture being moved.</param>
		void operator=(texture&&) noexcept;

		/// <summary>
		/// <para>Create a texture using this configuration.</para>
		/// </summary>
		/// <param name="{texture_config}">A configuration.</param>
		/// <returns>{bool} True if success.</returns>
		bool create(const texture_config&);

		/// <summary>
		/// <para>Directly create a texture of given size.</para>
		/// </summary>
		/// <param name="{int}">Width.</param>
		/// <param name="{int}">Height.</param>
		/// <returns>{bool} True if success.</returns>
		bool create(const int, const int);

		/// <summary>
		/// <para>Load a texture from a file path.</para>
		/// </summary>
		/// <param name="{std::string}">Path.</param>
		/// <returns>{bool} True if success.</returns>
		bool create(const std::string&);

		/// <summary>
		/// <para>Load or create a texture with this configuration.</para>
		/// </summary>
		/// <param name="{texture_config}">A configuration.</param>
		/// <returns>{bool} True if success.</returns>
		bool load(const texture_config&);

		/// <summary>
		/// <para>Load a texture from a file path.</para>
		/// </summary>
		/// <param name="{std::string}">Path.</param>
		/// <returns>{bool} True if success.</returns>
		virtual bool load(const std::string&);

		/// <summary>
		/// <para>Load and hook this texture to an already loaded file.</para>
		/// <para>The hybrid_memory must not be deleted from outside. If you do nothing, this reference will keep it available while in use.</para>
		/// </summary>
		/// <param name="{hybrid_memory&lt;file&gt;}">A file.</param>
		/// <returns>{bool} True if success.</returns>
		virtual bool load(hybrid_memory<file>);

		/// <summary>
		/// <para>Duplicate internal texture.</para>
		/// <para>If the texture was from a file, now it is completely duplicated in memory.</para>
		/// </summary>
		/// <returns>{texture} A duplicated texture of this.</returns>
		texture duplicate();

		/// <summary>
		/// <para>Create a sub part of current texture (still targeting this one).</para>
		/// <para>This original piece must still exist. The memory is shared.</para>
		/// </summary>
		/// <param name="{int}">Offset X.</param>
		/// <param name="{int}">Offset Y.</param>
		/// <param name="{int}">Width.</param>
		/// <param name="{int}">Height.</param>
		/// <returns>A sub texture of this.</returns>
		texture create_sub(const int, const int, const int, const int);

		/// <summary>
		/// <para>Get the width of the texture bitmap.</para>
		/// </summary>
		/// <returns>{int} Width.</returns>
		virtual int get_width() const;

		/// <summary>
		/// <para>Get the height of the texture bitmap.</para>
		/// </summary>
		/// <returns>{int} Height.</returns>
		virtual int get_height() const;

		/// <summary>
		/// <para>Get the pixel format of the texture bitmap.</para>
		/// </summary>
		/// <returns>{int} Pixel format.</returns>
		int get_format() const;

		/// <summary>
		/// <para>Get texture bitmap flags used to create this.</para>
		/// </summary>
		/// <returns>{int} Flags.</returns>
		int get_flags() const;

		/// <summary>
		/// <para>Get the raw bitmap pointer.</para>
		/// </summary>
		/// <returns>{ALLEGRO_BITMAP*} Internal bitmap pointer.</returns>
		virtual ALLEGRO_BITMAP* get_raw_bitmap() const;

		/// <summary>
		/// <para>Get the raw bitmap pointer.</para>
		/// </summary>
		/// <returns>{ALLEGRO_BITMAP*} Internal bitmap pointer.</returns>
		virtual operator ALLEGRO_BITMAP* () const;

		/// <summary>
		/// <para>It is considered valid if the bitmap is not null.</para>
		/// </summary>
		/// <returns>{bool} Is there a bitmap?</returns>
		virtual bool valid() const;

		/// <summary>
		/// <para>It's true if there's no bitmap loaded.</para>
		/// </summary>
		/// <returns>{bool} No bitmap around?</returns>
		virtual bool empty() const;

		/// <summary>
		/// <para>Unload and unreference texture bitmap.</para>
		/// </summary>
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

		/// <summary>
		/// <para>Set this bitmap as target for this thread.</para>
		/// </summary>
		void set_as_target() const;
	};

	/// <summary>
	/// <para>This is still a texture, but the catch is that it runs a function every time it's drawn or got.</para>
	/// </summary>
	class texture_functional : public texture {
		std::function<void(texture&)> func; // tied to check_ready()
		mutable fast_one_way_mutex fastmu;

		bool check_ready() const;
	public:
		texture_functional() = default;
		~texture_functional();

		/// <summary>
		/// <para>Move a functional texture to this.</para>
		/// </summary>
		/// <param name="{texture_functional}">A texture_functional.</param>
		texture_functional(texture_functional&&) noexcept;

		/// <summary>
		/// <para>Move a functional texture to this.</para>
		/// </summary>
		/// <param name="{texture_functional}">A texture_functional.</param>
		void operator=(texture_functional&&) noexcept;

		/// <summary>
		/// <para>Hook a function to this functional texture.</para>
		/// </summary>
		/// <param name="{function}">A function that does something with the texture.</param>
		void hook_function(std::function<void(texture&)>);

		/// <summary>
		/// <para>Reset hooked function to none.</para>
		/// </summary>
		void unhook_function();

		/// <summary>
		/// <para>Get the raw bitmap pointer.</para>
		/// </summary>
		/// <returns>{ALLEGRO_BITMAP*} Internal bitmap pointer.</returns>
		ALLEGRO_BITMAP* get_raw_bitmap() const;

		/// <summary>
		/// <para>Get the raw bitmap pointer.</para>
		/// </summary>
		/// <returns>{ALLEGRO_BITMAP*} Internal bitmap pointer.</returns>
		operator ALLEGRO_BITMAP* () const;

		using texture::create;
		using texture::load;
		using texture::duplicate;
		using texture::create_sub;
		using texture::get_width;
		using texture::get_height;
		using texture::get_format;
		using texture::get_flags;
		using texture::valid;
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

	/// <summary>
	/// <para>texture_gif is a texture that loads a GIF format and behaves like a simple texture.</para>
	/// <para>This is like that because many places use texture already, so this is the way to "import" GIF support.</para>
	/// <para>You can still get each frame or frame times easily.</para>
	/// </summary>
	class texture_gif : public texture {
		ALGIF_ANIMATION* animation = nullptr;
		double start_time = 0.0;

		bool check_ready() const;
	public:
		texture_gif() = default;
		~texture_gif();

		/// <summary>
		/// <para>Move a texture_gif to this.</para>
		/// </summary>
		/// <param name="{texture_functional}">A texture_functional.</param>
		texture_gif(texture_gif&&) noexcept;

		/// <summary>
		/// <para>Move a gif texture to this.</para>
		/// </summary>
		/// <param name="{texture_functional}">A texture_functional.</param>
		void operator=(texture_gif&&) noexcept;

		/// <summary>
		/// <para>Load a gif from a file path.</para>
		/// </summary>
		/// <param name="{std::string}">Path.</param>
		/// <returns>{bool} True if success.</returns>
		bool load(const std::string&);

		/// <summary>
		/// <para>Load and hook this gif texture to an already loaded file.</para>
		/// <para>The hybrid_memory must not be deleted from outside. If you do nothing, this reference will keep it available while in use.</para>
		/// </summary>
		/// <param name="{hybrid_memory&lt;file&gt;}">A file.</param>
		/// <returns>{bool} True if success.</returns>
		bool load(const hybrid_memory<file>&);

		/// <summary>
		/// <para>Get the width of the gif texture bitmap.</para>
		/// </summary>
		/// <returns>{int} Width.</returns>
		int get_width() const;

		/// <summary>
		/// <para>Get the height of the gif texture bitmap.</para>
		/// </summary>
		/// <returns>{int} Height.</returns>
		int get_height() const;

		/// <summary>
		/// <para>Get the raw bitmap pointer of this point in time.</para>
		/// </summary>
		/// <returns>{ALLEGRO_BITMAP*} Internal bitmap pointer.</returns>
		ALLEGRO_BITMAP* get_raw_bitmap() const;

		/// <summary>
		/// <para>Get the raw bitmap pointer of this point in time.</para>
		/// </summary>
		/// <returns>{ALLEGRO_BITMAP*} Internal bitmap pointer.</returns>
		operator ALLEGRO_BITMAP* () const;

		/// <summary>
		/// <para>It is considered valid if the bitmap and the gif buffer is not null.</para>
		/// </summary>
		/// <returns>{bool} Is there a gif?</returns>
		bool valid() const;

		/// <summary>
		/// <para>It's true if there's no bitmap and gif buffer loaded.</para>
		/// </summary>
		/// <returns>{bool} No gif around?</returns>
		bool empty() const;

		/// <summary>
		/// <para>Unload all frames and gif.</para>
		/// </summary>
		void destroy();

		/// <summary>
		/// <para>Get the average time of this GIF animation.</para>
		/// </summary>
		/// <returns>{double} Time, in seconds, or -1 if empty.</returns>
		double get_interval_average() const;

		/// <summary>
		/// <para>Get the longest interval between two frames of this GIF animation.</para>
		/// </summary>
		/// <returns>{double} Time, in seconds, or -1 if empty.</returns>
		double get_interval_longest() const;

		/// <summary>
		/// <para>Get the shortest interval between two frames of this GIF animation.</para>
		/// </summary>
		/// <returns>{double} Time, in seconds, or -1 if empty.</returns>
		double get_interval_shortest() const;

		/// <summary>
		/// <para>Get how many frames are in the animation.</para>
		/// <para>This is not called size() because it could mean texture size or something.</para>
		/// </summary>
		/// <returns>{size_t} </returns>
		size_t get_amount_of_frames() const;

		/// <summary>
		/// <para>Get a specific frame by index.</para>
		/// </summary>
		/// <param name="{size_t}">Frame number.</param>
		/// <returns>{ALLEGRO_BITMAP*} Raw bitmap (null if out of range or empty).</returns>
		ALLEGRO_BITMAP* index(const size_t) const;

		/// <summary>
		/// <para>Get the screen time of this specific frame.</para>
		/// </summary>
		/// <returns>{double} Time, in seconds, or -1 if empty.</returns>
		double get_interval_of_index(const size_t) const;

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
