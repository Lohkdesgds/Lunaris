#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/color.h>
#include <Lunaris/Utility/file.h>
#include <Lunaris/Utility/memory.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdexcept>
#include <string>
#include <optional>

namespace Lunaris {

	void __font_allegro_start();

	/// <summary>
	/// <para>Font creation configuration.</para>
	/// <para>You can load a TTF font or non TTF easily with this.</para>
	/// <para>When using file it NEEDS a real file, I mean, it can't be a memfile. It will fail.</para>
	/// </summary>
	struct font_config {
		int bmp_flags = ALLEGRO_CONVERT_BITMAP | ALLEGRO_MIN_LINEAR;
		int font_flags = 0;
		bool ttf = true;
		int resolution = 50;
		std::string path;
		hybrid_memory<file> fileref;

		font_config& set_bitmap_flags(const int);
		font_config& set_font_flags(const int);
		font_config& set_is_ttf(const bool);
		font_config& set_resolution(const int);
		font_config& set_path(const std::string&);
		font_config& set_file(const hybrid_memory<file>&);
	};

	/// <summary>
	/// <para>font is the handle to work with text on screen.</para>
	/// <para>This easily create, destroy, duplicate and draw the font in a very easy way.</para>
	/// </summary>
	class font : public NonCopyable {
		ALLEGRO_FONT* font_ptr = nullptr;
		hybrid_memory<file> fileref;

		bool check_ready() const;
	public:
		font() = default;
		font(const font_config&);
		~font();

		/// <summary>
		/// <para>Move the font.</para>
		/// </summary>
		/// <param name="{font&amp;&amp;}">Font being moved.</param>
		font(font&&) noexcept;

		/// <summary>
		/// <para>Move the font.</para>
		/// </summary>
		/// <param name="{font&amp;&amp;}">Font being moved.</param>
		void operator=(font&&) noexcept;

		/// <summary>
		/// <para>Create the builtin font from Allegro.</para>
		/// </summary>
		/// <returns></returns>
		bool create_builtin_font();

		/// <summary>
		/// <para>Load a font using this configuration.</para>
		/// </summary>
		/// <param name="{font_config}">A configuration.</param>
		/// <returns>{bool} True if success.</returns>
		bool load(const font_config&);

		/// <summary>
		/// <para>Load a font from the path directly.</para>
		/// </summary>
		/// <param name="{std::string}">The path.</param>
		/// <param name="{bool}">Is it TTF?</param>
		/// <returns>{bool} True if success.</returns>
		bool load(const std::string&, const bool);
		
		/// <summary>
		/// <para>Load a TTF font directly from a file.</para>
		/// </summary>
		/// <param name="{file}">A TTF file.</param>
		/// <returns>{bool} True if success.</returns>
		bool load(const hybrid_memory<file>&);

		/// <summary>
		/// <para>Get the raw ALLEGRO_FONT*.</para>
		/// </summary>
		/// <returns>{ALLEGRO_FONT*} The raw pointer.</returns>
		ALLEGRO_FONT* get_raw_font() const;

		/// <summary>
		/// <para>Is this empty/null?</para>
		/// </summary>
		/// <returns>{bool} True if null.</returns>
		bool empty() const;

		/// <summary>
		/// <para>Do you have something loaded in here?</para>
		/// </summary>
		/// <returns>{bool} True if not null.</returns>
		bool valid() const;

		/// <summary>
		/// <para>Unload the font and unref file.</para>
		/// </summary>
		void destroy();

		/// <summary>
		/// <para>Height of the top part of the font.</para>
		/// </summary>
		/// <returns>{int} Height.</returns>
		int get_line_ascent() const;

		/// <summary>
		/// <para>Height of the bottom part of the font.</para>
		/// </summary>
		/// <returns>{int} Height.</returns>
		int get_line_descent() const;

		/// <summary>
		/// <para>Full height of the font.</para>
		/// </summary>
		/// <returns>{int} Height.</returns>
		int get_line_height() const;

		/// <summary>
		/// <para>Gets the width of a text with current Font.</para>
		/// </summary>
		/// <param name="{std::string}">String.</param>
		/// <returns>{int} The length, in pixels.</returns>
		int get_width(const std::string&) const;

		/// <summary>
		/// <para>Direct one color draw of text (faster).</para>
		/// </summary>
		/// <param name="{Color}">The color to draw font.</param>
		/// <param name="{float}">Position X.</param>
		/// <param name="{float}">Position Y.</param>
		/// <param name="{int}">Flags.</param>
		/// <param name="{std::string}">String to draw.</param>
		void draw(color, const float, const float, const int, const std::string&) const;
	};
}