#pragma once

#include <Lunaris/__macro/macros.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

namespace Lunaris {

	class transform {
		ALLEGRO_TRANSFORM t;
	public:
		/// <summary>
		/// <para>Reads current applied transform.</para>
		/// </summary>
		void get_current_transform();

		/// <summary>
		/// <para>Invert the internal matrix.</para>
		/// </summary>
		void invert();

		/// <summary>
		/// <para>Checks if the transformation has an inverse using the supplied tolerance.</para>
		/// </summary>
		/// <param name="{float}">The tolerance.</param>
		/// <returns>{bool} True if has inverse.</returns>
		bool check_inverse(const float);

		/// <summary>
		/// <para>Reset matrix to the identity one.</para>
		/// </summary>
		void identity();

		/// <summary>
		/// <para>Build a camera that keeps fixed proportion for the drawing.</para>
		/// <para>This means proportions different than the float there will "extend" the borders. There will be always a safe area with that proportion in the center.</para>
		/// <para>The safe area goes from -1.0 to 1.0.</para>
		/// </summary>
		/// <param name="{int}">Canvas width.</param>
		/// <param name="{int}">Canvas height.</param>
		/// <param name="{float}">Proportion in the middle (defaults to 16:9).</param>
		/// <param name="{float}">Zoom (defaults to 1:1).</param>
		void build_classic_fixed_proportion(const int, const int, const float = 16.0f / 9.0f, const float = 1.0f);

		/// <summary>
		/// <para>Builds a transform based on common properties.</para>
		/// </summary>
		/// <param name="{float}">Position X.</param>
		/// <param name="{float}">Position Y.</param>
		/// <param name="{float}">Scale X.</param>
		/// <param name="{float}">Scale Y.</param>
		/// <param name="{float}">Angle (rad).</param>
		void build_transform(const float, const float, const float, const float, const float);

		/// <summary>
		/// <para>Rotates the matrix.</para>
		/// </summary>
		/// <param name="{float}">Angle (rad).</param>
		void rotate(const float);

		/// <summary>
		/// <para>Rotates the matrix inverting twice.</para>
		/// <para>Useful if you're rotating the actual pos, not target (end).</para>
		/// </summary>
		/// <param name="{float}">Angle (rad).</param>
		void rotate_inverse(const float);

		/// <summary>
		/// <para>Scales the matrix.</para>
		/// </summary>
		/// <param name="{float}">Scale.</param>
		void scale(const float, const float);

		/// <summary>
		/// <para>Scales the matrix inverting twice.</para>
		/// <para>Useful if you're scaling the actual pos, not target (end).</para>
		/// </summary>
		/// <param name="{float}">Scale.</param>
		void scale_inverse(const float, const float);

		/// <summary>
		/// <para>Transform some coordinates using internal matrix.</para>
		/// </summary>
		/// <param name="{float}">Coordinate X.</param>
		/// <param name="{float}">Coordinate Y.</param>
		void transform_coords(float&, float&);

		/// <summary>
		/// <para>Transform some coordinates using internal matrix, but the inverse of it.</para>
		/// </summary>
		/// <param name="{float}">Coordinate X.</param>
		/// <param name="{float}">Coordinate Y.</param>
		void transform_inverse_coords(float&, float&);

		/// <summary>
		/// <para>Compose (combine) two transforms by matrix multiplication.</para>
		/// </summary>
		/// <param name="{transform}">transform to compose with.</param>
		void compose(const transform&);

		/// <summary>
		/// <para>Translate the internal matrix.</para>
		/// </summary>
		/// <param name="{float}">Translation X.</param>
		/// <param name="{float}">Translation Y.</param>
		void translate(const float, const float);

		/// <summary>
		/// <para>Translate the internal matrix inverting twice.</para>
		/// <para>Useful if you're translating the actual pos, not target (end).</para>
		/// </summary>
		/// <param name="{float}">Translation X.</param>
		/// <param name="{float}">Translation Y.</param>
		void translate_inverse(const float, const float);

		/// <summary>
		/// <para>Applies internal matrix transformation to current thread display.</para>
		/// </summary>
		void apply() const;
	};

}