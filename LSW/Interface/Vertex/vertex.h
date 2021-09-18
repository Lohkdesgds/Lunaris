#pragma once

#include "../../Handling/Initialize/initialize.h"
#include "../Color/color.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>A generic vertex with Color and texture offset information.</para>
			/// </summary>
			class Vertex : public ALLEGRO_VERTEX {
			public:
				Vertex();

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{Vertex}">Vertex to copy from.</param>
				Vertex(const Vertex&);

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{RAW VERTEX}">Raw Vertex to copy from.</param>
				Vertex(ALLEGRO_VERTEX);

				/// <summary>
				/// <para>Copy operator.</para>
				/// </summary>
				/// <param name="{Vertex}">Vertex to copy from.</param>
				/// <returns>{Vertex} itself.</returns>
				Vertex& operator=(const Vertex&);

				/// <summary>
				/// <para>Intialize with 2D position and color.</para>
				/// </summary>
				/// <param name="{float}">X.</param>
				/// <param name="{float}">Y.</param>
				/// <param name="{Color}">Color.</param>
				Vertex(const float, const float, const Color&);

				/// <summary>
				/// <para>Intialize with 3D position and color.</para>
				/// </summary>
				/// <param name="{float}">X.</param>
				/// <param name="{float}">Y.</param>
				/// <param name="{float}">Z.</param>
				/// <param name="{Color}">Color.</param>
				Vertex(const float, const float, const float, const Color&);

				/// <summary>
				/// <para>Intialize with 2D position, color and texture position.</para>
				/// </summary>
				/// <param name="{float}">X.</param>
				/// <param name="{float}">Y.</param>
				/// <param name="{Color}">Color.</param>
				/// <param name="{float}">U (in pixels).</param>
				/// <param name="{float}">V (in pixels).</param>
				Vertex(const float, const float, const Color&, const float, const float);

				/// <summary>
				/// <para>Intialize with 3D position, color and texture position.</para>
				/// </summary>
				/// <param name="{float}">X.</param>
				/// <param name="{float}">Y.</param>
				/// <param name="{float}">Z.</param>
				/// <param name="{Color}">Color.</param>
				/// <param name="{float}">U (in pixels).</param>
				/// <param name="{float}">V (in pixels).</param>
				Vertex(const float, const float, const float, const Color&, const float, const float);

				/// <summary>
				/// <para>Get the X position information.</para>
				/// </summary>
				/// <returns>{float} X data.</returns>
				float get_x() const;

				/// <summary>
				/// <para>Get the Y position information.</para>
				/// </summary>
				/// <returns>{float} Y data.</returns>
				float get_y() const;

				/// <summary>
				/// <para>Get the Z position information.</para>
				/// </summary>
				/// <returns>{float} Z data.</returns>
				float get_z() const;

				/// <summary>
				/// <para>Get the U texture position information.</para>
				/// </summary>
				/// <returns>{float} U data.</returns>
				float get_u() const;

				/// <summary>
				/// <para>Get the V texture position information.</para>
				/// </summary>
				/// <returns>{float} V data.</returns>
				float get_v() const;

				/// <summary>
				/// <para>Get the Color.</para>
				/// </summary>
				/// <returns>{Color} Color data.</returns>
				Color get_color() const;

			};

		}
	}
}
