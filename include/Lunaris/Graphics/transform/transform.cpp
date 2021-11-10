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

	LUNARIS_DECL void transform::build_classic_fixed_proportion_stretched(const int ww, const int wh, const float zoom)
	{
		if (ww <= 0 || wh <= 0) throw std::runtime_error("width or height zero or negative on transform");
		build_classic_fixed_proportion(ww, wh, ww * 1.0f / wh, zoom);
	}

	LUNARIS_DECL bool transform::build_classic_fixed_proportion_stretched_auto(const float zoom)
	{
		auto* ptr = al_get_current_display();
		if (!ptr) return false;
		build_classic_fixed_proportion(al_get_display_width(ptr), al_get_display_height(ptr), al_get_display_width(ptr) * 1.0f / al_get_display_height(ptr), zoom);
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

	LUNARIS_DECL void transform::transform_coords(float& x, float& y) const
	{
		al_transform_coordinates(&t, &x, &y);
	}

	LUNARIS_DECL void transform::transform_inverse_coords(float& x, float& y) const
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

	LUNARIS_DECL void transform::compare_scale_of(const transform& oth, float& xx, float& yy)
	{
		float scalh[4] = { 0.0f,0.0f,1000.0f,1000.0f }; // here
		float scalt[4] = { 0.0f,0.0f,1000.0f,1000.0f }; // there

		transform_coords(scalh[0], scalh[1]);
		transform_coords(scalh[2], scalh[3]);
		oth.transform_coords(scalt[0], scalt[1]);
		oth.transform_coords(scalt[2], scalt[3]);

		xx = (scalh[2] - scalh[0]) / (scalt[2] - scalt[0]);
		yy = (scalh[3] - scalh[1]) / (scalt[3] - scalt[1]);
	}

	LUNARIS_DECL bool transform::in_range(const float xx, const float yy, const float tol)
	{
		if (tol <= 0.0f) return false;

		float xo = 0.0f, yo = 0.0f;
		transform_inverse_coords(xo, yo);
		transform assist;
		assist.build_classic_fixed_proportion_stretched_auto(1.0f);
		float px = 0.0f, py = 0.0f;
		assist.compare_scale_of(*this, px, py);

		xo += px;
		yo += py;

		return (xx) <= (xo + px * tol) && (xx) >= (xo - px * tol) &&
			   (yy) <= (yo + py * tol) && (yy) >= (yo - py * tol);
	}

	LUNARIS_DECL bool transform::in_range_store(const float xx, const float yy, float& sx, float& sy, const float tol)
	{
		if (tol <= 0.0f) return false;

		float xo = 0.0f, yo = 0.0f;
		transform_inverse_coords(xo, yo);
		transform assist;
		assist.build_classic_fixed_proportion_stretched_auto(1.0f);
		float px = 0.0f, py = 0.0f;
		assist.compare_scale_of(*this, px, py);

		xo += px;
		yo += py;

		if ((xx) <= (xo + px * tol) && (xx) >= (xo - px * tol) && (yy) <= (yo + py * tol) && (yy) >= (yo - py * tol)) return true; // in range = don't care

		if (!((xx) <= (xo + px * tol) && (xx) >= (xo - px * tol))) sx = (xx)-(xo + px * tol); // + right
		else sx = 0.0f;
		if (!((yy) <= (yo + py * tol) && (yy) >= (yo - py * tol))) sy = (yy)-(yo + py * tol); // + down
		else sy = 0.0f;

		return false;
	}

}