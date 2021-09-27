#include "transform.h"

namespace Lunaris {

	void transform::get_current_transform()
	{
		t = *al_get_current_transform();
	}

	void transform::invert()
	{
		al_invert_transform(&t);
	}

	bool transform::check_inverse(const float tol)
	{
		return al_check_inverse(&t, tol);
	}

	void transform::identity()
	{
		al_identity_transform(&t);
	}

	void transform::build_classic_fixed_proportion(const int ww, const int wh, const float scal_def, const float zoom)
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

	void transform::build_transform(const float x, const float y, const float sx, const float sy, const float th)
	{
		al_build_transform(&t, x, y, sx, sy, th);
	}

	void transform::rotate(const float r)
	{
		al_rotate_transform(&t, r);
	}

	void transform::rotate_inverse(const float r)
	{
		invert();
		rotate(r);
		invert();
	}

	void transform::scale(const float sx, const float sy)
	{
		al_scale_transform(&t, sx, sy);
	}

	void transform::scale_inverse(const float sx, const float sy)
	{
		invert();
		scale(sx, sy);
		invert();
	}

	void transform::transform_coords(float& x, float& y)
	{
		al_transform_coordinates(&t, &x, &y);
	}

	void transform::transform_inverse_coords(float& x, float& y)
	{
		transform _temp = *this;
		_temp.invert();
		_temp.transform_coords(x, y);
	}

	void transform::compose(const transform& c)
	{
		al_compose_transform(&t, &c.t);
	}

	void transform::translate(const float x, const float y)
	{
		al_translate_transform(&t, x, y);
	}

	void transform::translate_inverse(const float x, const float y)
	{
		invert();
		translate(x, y);
		invert();
	}

	void transform::apply() const
	{
		al_use_transform(&t);
	}

}