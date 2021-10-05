#include "transform.h"

namespace Lunaris {

	LUNARIS_DECL void transform::get_current_transform()
	{
		t = *al_get_current_transform();
	}

	LUNARIS_DECL void transform::invert()
	{
		al_invert_transform(&t);
	}

	LUNARIS_DECL bool transform::check_inverse(const float tol)
	{
		return al_check_inverse(&t, tol);
	}

	LUNARIS_DECL void transform::identity()
	{
		al_identity_transform(&t);
	}

	LUNARIS_DECL void transform::build_classic_fixed_proportion(const int ww, const int wh, const float scal_def, const float zoom)
	{
		if (ww * 1.0f / wh > scal_def) { // ww >>>> wh, use vertical
			const float f_x = wh * 0.5f * scal_def;
			const float f_y = wh * 0.5f;

			build_transform(ww * 0.5f, wh * 0.5f, f_x * zoom, f_y * zoom, 0.0f);
		}
		else { // ww <<<< wh, use horizontal
			const float f_x = ww * 0.5f;
			const float f_y = ww * 0.5f / scal_def;

			build_transform(ww * 0.5f, wh * 0.5f, f_x * zoom, f_y * zoom, 0.0f);
		}
	}

	LUNARIS_DECL bool transform::build_classic_fixed_proportion_auto(const float scal_def, const float zoom)
	{
		auto* ptr = al_get_current_display();
		if (!ptr) return false;
		build_classic_fixed_proportion(al_get_display_width(ptr), al_get_display_height(ptr), scal_def, zoom);
		return true;
	}

	LUNARIS_DECL void transform::build_transform(const float x, const float y, const float sx, const float sy, const float th)
	{
		al_build_transform(&t, x, y, sx, sy, th);
	}

	LUNARIS_DECL void transform::rotate(const float r)
	{
		al_rotate_transform(&t, r);
	}

	LUNARIS_DECL void transform::rotate_inverse(const float r)
	{
		invert();
		rotate(r);
		invert();
	}

	LUNARIS_DECL void transform::scale(const float sx, const float sy)
	{
		al_scale_transform(&t, sx, sy);
	}

	LUNARIS_DECL void transform::scale_inverse(const float sx, const float sy)
	{
		invert();
		scale(sx, sy);
		invert();
	}

	LUNARIS_DECL void transform::transform_coords(float& x, float& y)
	{
		al_transform_coordinates(&t, &x, &y);
	}

	LUNARIS_DECL void transform::transform_inverse_coords(float& x, float& y)
	{
		transform _temp = *this;
		_temp.invert();
		_temp.transform_coords(x, y);
	}

	LUNARIS_DECL void transform::compose(const transform& c)
	{
		al_compose_transform(&t, &c.t);
	}

	LUNARIS_DECL void transform::translate(const float x, const float y)
	{
		al_translate_transform(&t, x, y);
	}

	LUNARIS_DECL void transform::translate_inverse(const float x, const float y)
	{
		invert();
		translate(x, y);
		invert();
	}

	LUNARIS_DECL void transform::apply() const
	{
		al_use_transform(&t);
	}

}