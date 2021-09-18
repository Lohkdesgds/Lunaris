#pragma once

#include <memory>
#include <functional>

#include "../../Handling/Abort/abort.h"
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/Resource/resource.h"
#include "../Bitmap/bitmap.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			struct classic_2d {
				float x = 0,
					y = 0,
					sx = 1,
					sy = 1,
					rot = 0;
			};

			/// <summary>
			/// <para>Camera is a setting to transform overall coordinates to another one.</para>
			/// </summary>
			class Camera {
				ALLEGRO_TRANSFORM t{};
				classic_2d cl_2d;
				Bitmap last_targ;
			public:
				Camera() = default;

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{Camera}">Camera to copy from.</param>
				Camera(const Camera&);

				/// <summary>
				/// <para>Copy operator.</para>
				/// </summary>
				/// <param name="{Camera}">Camera to copy from.</param>
				void operator=(const Camera&);

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
				/// <para>Builds a camera based on common properties.</para>
				/// </summary>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{float}">Scale X.</param>
				/// <param name="{float}">Scale Y.</param>
				/// <param name="{float}">Angle (rad).</param>
				void build_transform(const float, const float, const float, const float, const float);

				/// <summary>
				/// <para>Builds a camera based on common properties, but 3D.</para>
				/// </summary>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{float}">Position Z.</param>
				/// <param name="{float}">Looking towards X.</param>
				/// <param name="{float}">Looking towards Y.</param>
				/// <param name="{float}">Looking towards Z.</param>
				/// <param name="{float}">Up vector X.</param>
				/// <param name="{float}">Up vector Y.</param>
				/// <param name="{float}">Up vector Z.</param>
				void build_3d_transform(const float, const float, const float, const float, const float, const float, const float = 0.0f, const float = 1.0f, const float = 0.0f);

				/// <summary>
				/// <para>Classic LSW way to build a camera.</para>
				/// </summary>
				/// <param name="{Bitmap}">Reference Bitmap (to transform coordinates to).</param>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{float}">Scale X.</param>
				/// <param name="{float}">Scale Y.</param>
				/// <param name="{float}">Angle (rad).</param>
				/// <returns>{bool} True if successfully created.</returns>
				bool classic_transform(const Bitmap&, const float, const float, const float, const float, const float);
				
				/// <summary>
				/// <para>Classic LSW way to build a camera (assuming you've set a Bitmap earlier).</para>
				/// </summary>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{float}">Scale X.</param>
				/// <param name="{float}">Scale Y.</param>
				/// <param name="{float}">Angle (rad).</param>
				/// <returns>{bool} True if successfully created.</returns>
				bool classic_transform(const float, const float, const float, const float, const float);

				/// <summary>
				/// <para>Refresh coordinates (useful if reference Bitmap has changed in size).</para>
				/// </summary>
				/// <returns>{bool} True if successful.</returns>
				bool classic_refresh();

				/// <summary>
				/// <para>Updates the internal Bitmap reference (for classic transformations).</para>
				/// </summary>
				/// <param name="{Bitmap}">A Bitmap.</param>
				/// <returns>{bool} True if successfully set.</returns>
				bool classic_update(const Bitmap&);

				/// <summary>
				/// <para>Gets classic settings information (only classic stuff).</para>
				/// </summary>
				/// <returns>{classic_2d} The classic data.</returns>
				const classic_2d& get_classic() const;

				/// <summary>
				/// <para>Gets classic settings information (only classic stuff).</para>
				/// </summary>
				/// <returns>{classic_2d} The classic data.</returns>
				classic_2d& get_classic();

				/// <summary>
				/// <para>Gets internal target used to transform the classic way.</para>
				/// </summary>
				/// <returns>{Bitmap} The reference Bitmap for transformations.</returns>
				Bitmap get_target() const;

				/// <summary>
				/// <para>Rotates the matrix.</para>
				/// </summary>
				/// <param name="{float}">Angle (rad).</param>
				void rotate(const float);

				/// <summary>
				/// <para>Scales the matrix.</para>
				/// </summary>
				/// <param name="{float}">Scale.</param>
				void scale(const float, const float);

				/// <summary>
				/// <para>Transform some coordinates using internal matrix.</para>
				/// </summary>
				/// <param name="{float}">Coordinate X.</param>
				/// <param name="{float}">Coordinate Y.</param>
				void transform(float&, float&);

				/// <summary>
				/// <para>Transform some coordinates in 3D using internal matrix.</para>
				/// </summary>
				/// <param name="{float}">Coordinate X.</param>
				/// <param name="{float}">Coordinate Y.</param>
				/// <param name="{float}">Coordinate Z.</param>
				void transform_3d(float&, float&, float&);

				/// <summary>
				/// <para>Compose (combine) two Cameras by matrix multiplication.</para>
				/// </summary>
				/// <param name="{Camera}">Camera to compose with.</param>
				void compose(const Camera&);

				/// <summary>
				/// <para>Translate the internal matrix.</para>
				/// </summary>
				/// <param name="{float}">Translation X.</param>
				/// <param name="{float}">Translation Y.</param>
				void translate(const float, const float);

				/// <summary>
				/// <para>Translate the internal matrix in 3D.</para>
				/// </summary>
				/// <param name="{float}">Translation X.</param>
				/// <param name="{float}">Translation Y.</param>
				/// <param name="{float}">Translation Z.</param>
				void translate_3d(const float, const float, const float);

				/// <summary>
				/// <para>Applies internal matrix transformation to current thread display.</para>
				/// </summary>
				void apply() const;
			};

		}
	}
}