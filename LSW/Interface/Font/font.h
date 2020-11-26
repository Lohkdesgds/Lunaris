#pragma once

// C++
#include <string>
#include <functional>
// Others
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/CString/cstring.h"
#include "../../Tools/AdvancedShared/advancedshared.h"
#include "../../Tools/Resource/resource.h"
#include "../Color/color.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>Font holds a font to draw text.</para>
			/// </summary>
			class Font {
				Tools::AdvancedShared<ALLEGRO_FONT> font;

				bool set(ALLEGRO_FONT*);
				ALLEGRO_FONT* quick() const;
				ALLEGRO_FONT* quick();
				Color hex(const int) const;

			public:
				Font();

				/// <summary>
				/// <para>Clones (the reference) of the internal Font.</para>
				/// </summary>
				/// <param name="{Font}">A Font to copy the reference.</param>
				Font(const Font&);

				/// <summary>
				/// <para>Moves the internal Font's data to this.</para>
				/// </summary>
				/// <param name="{Font}">A Font to move data from.</param>
				Font(Font&&);

				/// <summary>
				/// <para>Copy operator.</para>
				/// </summary>
				/// <param name="{Font}">A Font to copy the reference.</param>
				void operator=(const Font&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{Font}">A Font to move from.</param>
				void operator=(Font&&);

				/// <summary>
				/// <para>Load a font with specific size (you may want multiple Fonts for multiple sizes).</para>
				/// </summary>
				/// <param name="{char*}">Path to font.</param>
				/// <param name="{int}">Size (1:1 in Camera, in units per EM; use negative for pixels in height).</param>
				/// <param name="{int}">Flags.</param>
				/// <returns>{bool} True if success.</returns>
				bool load(const char*, const int = 75, const int = 0);

				/// <summary>
				/// <para>Operator bool.</para>
				/// </summary>
				/// <returns>{bool} True if loaded.</returns>
				operator bool() const;

				/// <summary>
				/// <para>Operator bool.</para>
				/// </summary>
				/// <returns>{bool} True if not loaded.</returns>
				const bool operator!() const;

				/// <summary>
				/// <para>Compare operator.</para>
				/// </summary>
				/// <param name="{Font}">Another Font.</param>
				/// <returns>{bool} True if pointers are the same (Same Font with different sizes are not equal!).</returns>
				const bool operator==(const Font&) const;

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
				/// <param name="{char*}">String.</param>
				/// <returns>{int} The length, in pixels.</returns>
				int get_width(const char*) const;

				/// <summary>
				/// <para>Gets the width of a text with current Font.</para>
				/// </summary>
				/// <param name="{CString}">String.</param>
				/// <returns>{int} The length, in pixels.</returns>
				int get_width(Tools::Cstring) const;

				/// <summary>
				/// <para>Direct one color draw of text (faster).</para>
				/// </summary>
				/// <param name="{Color}">The color to draw font.</param>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{int}">Flags.</param>
				/// <param name="{char*}">String to draw.</param>
				void draw(Color, const float, const float, const int, const char*) const;

				/// <summary>
				/// <para>Draw a CString.</para>
				/// </summary>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{int}">Flags.</param>
				/// <param name="{CString*}">Colored String.</param>
				void draw(const float, const float, const int, Tools::Cstring) const;
			};

		}
	}
}