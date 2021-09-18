#pragma once

#include "../../Handling/Initialize/initialize.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>Color is a RGBA float struct.</para>
			/// </summary>
			class Color : public ALLEGRO_COLOR {
			public:
				Color();

				/// <summary>
				/// <para>Intialize with the RGBA float values.</para>
				/// <para>RANGE: [0.0..1.0]</para>
				/// </summary>
				/// <param name="{float}">Red.</param>
				/// <param name="{float}">Green.</param>
				/// <param name="{float}">Blue.</param>
				/// <param name="{float}">Alpha.</param>
				Color(const float, const float, const float, const float = 1.0);

				/// <summary>
				/// <para>Intialize with the RGBA unsigned char values.</para>
				/// <para>RANGE: [0..255]</para>
				/// </summary>
				/// <param name="{char}">Red.</param>
				/// <param name="{char}">Green.</param>
				/// <param name="{char}">Blue.</param>
				/// <param name="{char}">Alpha.</param>
				Color(const unsigned char, const unsigned char, const unsigned char, const unsigned char = 255);

				/// <summary>
				/// <para>Intialize with the RGBA int values.</para>
				/// <para>RANGE: [0..255]</para>
				/// </summary>
				/// <param name="{int}">Red.</param>
				/// <param name="{int}">Green.</param>
				/// <param name="{int}">Blue.</param>
				/// <param name="{int}">Alpha.</param>
				Color(const int, const int, const int, const int = 255);

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{Color}">Color to copy from.</param>
				Color(const Color&);

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{RAW COLOR}">RAW COLOR to copy from.</param>
				Color(ALLEGRO_COLOR);

				/// <summary>
				/// <para>Get the RED information.</para>
				/// </summary>
				/// <returns>{float} RED data.</returns>
				float get_r() const;

				/// <summary>
				/// <para>Get the GREEN information.</para>
				/// </summary>
				/// <returns>{float} GREEN data.</returns>
				float get_g() const;

				/// <summary>
				/// <para>Get the BLUE information.</para>
				/// </summary>
				/// <returns>{float} BLUE data.</returns>
				float get_b() const;

				/// <summary>
				/// <para>Get the ALPHA information.</para>
				/// </summary>
				/// <returns>{float} ALPHA data.</returns>
				float get_a() const;

				/// <summary>
				/// <para>The target will be filled with this color.</para>
				/// </summary>
				void clear_to_this() const;

				/// <summary>
				/// <para>Copy operator.</para>
				/// </summary>
				/// <param name="{Color}">Color to copy from.</param>
				/// <returns>{Color} itself.</returns>
				Color& operator=(const Color& c);
			};

		}
	}
}