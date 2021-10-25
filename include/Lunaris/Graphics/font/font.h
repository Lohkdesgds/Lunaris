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

	struct font_config {
		int bmp_flags = ALLEGRO_CONVERT_BITMAP | ALLEGRO_MIN_LINEAR;
		int font_flags = 0;
		bool ttf = true;
		int resolution = 50;
		std::string path;
#ifdef LUNARIS_ALPHA_TESTING
		hybrid_memory<file> fileref;
#endif

		font_config& set_bitmap_flags(const int);
		font_config& set_font_flags(const int);
		font_config& set_is_ttf(const bool);
		font_config& set_resolution(const int);
		font_config& set_path(const std::string&);
#ifdef LUNARIS_ALPHA_TESTING
		font_config& set_file(const hybrid_memory<file>&);
#endif
	};

	class font {
		ALLEGRO_FONT* font_ptr = nullptr;
#ifdef LUNARIS_ALPHA_TESTING
		hybrid_memory<file> fileref;
#endif

		bool check_ready() const;
	public:
		font() = default;
		font(const font_config&);
		~font();

		font(font&&) noexcept;
		void operator=(font&&) noexcept;

		font(const font&) = delete;
		void operator=(const font&) = delete;

		bool create_builtin_font();

		bool load(const font_config&);
		bool load(const std::string&, const bool);
#ifdef LUNARIS_ALPHA_TESTING
		// assume TTF
		bool load(const hybrid_memory<file>&);
#endif

		ALLEGRO_FONT* get_raw_font() const;

		bool empty() const;

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