#pragma once

#include <Lunaris/__macro/macros.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdexcept>

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
		/// <para>Build a camera that keeps fixed proportion for the drawing.</para>
		/// <para>This means proportions different than the float there will "extend" the borders. There will be always a safe area with that proportion in the center.</para>
		/// <para>The safe area goes from -1.0 to 1.0.</para>
		/// <para>The auto part means it tries to get current target display. Returns false if failed.</para>
		/// </summary>
		/// <param name="{float}">Proportion in the middle (defaults to 16:9).</param>
		/// <param name="{float}">Zoom (defaults to 1:1).</param>
		/// <returns>{bool} True if got things right.</returns>
		bool build_classic_fixed_proportion_auto(const float = 16.0f / 9.0f, const float = 1.0f);

		/// <summary>
		/// <para>Build a camera that works as [-1,1] in both directions.</para>
		/// <para>This is like building normally but with proportion being the screen itself.</para>
		/// </summary>
		/// <param name="{int}">Canvas width.</param>
		/// <param name="{int}">Canvas height.</param>
		/// <param name="{float}">Zoom (defaults to 1:1).</param>
		void build_classic_fixed_proportion_stretched(const int, const int, const float = 1.0f);

		/// <summary>
		/// <para>Build a camera that works as [-1,1] in both directions. It gets display automatically.</para>
		/// <para>This is like building normally but with proportion being the screen itself.</para>
		/// </summary>
		/// <param name="{float}">Zoom (defaults to 1:1).</param>
		/// <returns>{bool} True if got things right.</returns>
		bool build_classic_fixed_proportion_stretched_auto(const float = 1.0f);

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
		void transform_coords(float&, float&) const;

		/// <summary>
		/// <para>Transform some coordinates using internal matrix, but the inverse of it.</para>
		/// </summary>
		/// <param name="{float}">Coordinate X.</param>
		/// <param name="{float}">Coordinate Y.</param>
		void transform_inverse_coords(float&, float&) const;

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

		/// <summary>
		/// <para>Compare THIS scaling to THAT scale.</para>
		/// <para>If result is > 1.0f, like 2.0f, this scaled 2x compared to that (this camera is 2x zoomed in compared to that).</para>
		/// </summary>
		/// <param name="{transform}">Some other transform</param>
		/// <param name="{float}">Resulting X</param>
		/// <param name="{float}">Resulting Y</param>
		void compare_scale_of(const transform&, float&, float&);

		/// <summary>
		/// <para>Tries to get what is the X/Y relation</para>
		/// <para>The test is done from 0 to 1000 relative coords.</para>
		/// </summary>
		/// <returns>{float} Possible proportion.</returns>
		float get_x_by_y() const;

		/// <summary>
		/// <para>Check if those values are in range.</para>
		/// <para>Being in range means if the translated position on screen is on screen.</para>
		/// <para>NOTE: needs a screen to work correctly.</para>
		/// </summary>
		/// <param name="{float}">Position X</param>
		/// <param name="{float}">Position Y</param>
		/// <param name="{float}">Tolerance (1.0f == 100%, 2.0f = 2x area).</param>
		/// <returns>{bool} Is is in range?</returns>
		bool in_range(const float, const float, const float = 1.0f);

		/// <summary>
		/// <para>Check if those values are in range.</para>
		/// <para>Being in range means if the translated position on screen is on screen.</para>
		/// <para>Value stored is measured on current transform.</para>
		/// <para>NOTE: values are only set IF OUT OF RANGE!</para>
		/// <para>NOTE2: needs a screen to work correctly.</para>
		/// <para>NOTE3: Positive usually means down right. If X is 0.5f, it means you're 0.5f to the right OFF of the range.</para>
		/// </summary>
		/// <param name="{float}">Position X</param>
		/// <param name="{float}">Position Y</param>
		/// <param name="{float}">Off in X (0 if in range, positive = right)</param>
		/// <param name="{float}">Off in Y (0 if in range, positive = down)</param>
		/// <param name="{float}">Tolerance (1.0f == 100%, 2.0f = 2x area).</param>
		/// <returns>{bool} Is is in range?</returns>
		bool in_range_store(const float, const float, float&, float&, const float = 1.0f);
	};

}