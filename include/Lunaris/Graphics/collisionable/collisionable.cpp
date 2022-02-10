#include "collisionable.h"

namespace Lunaris {

	LUNARIS_DECL bool collisionable_base::line_on_segment(const supported_fast_point_2d& p, const supported_fast_point_2d& q, const supported_fast_point_2d& r)
	{
		return ((q.px <= std::max(p.px, r.px)) && (q.px >= std::min(p.px, r.px)) &&
			(q.py <= std::max(p.py, r.py)) && (q.py >= std::min(p.py, r.py)));
	}

	LUNARIS_DECL int collisionable_base::line_orientation(const supported_fast_point_2d& p, const supported_fast_point_2d& q, const supported_fast_point_2d& r)
	{
		const int val = (q.py - p.py) * (r.px - q.px) - (q.px - p.px) * (r.py - q.py);
		return (val == 0) ? 0 : (val > 0 ? 1 : 2);
	}

	LUNARIS_DECL bool collisionable_base::line_do_intersect(const supported_fast_point_2d& p1, const supported_fast_point_2d& q1, const supported_fast_point_2d& p2, const supported_fast_point_2d& q2)
	{
		const int o1 = line_orientation(p1, q1, p2);
		const int o2 = line_orientation(p1, q1, q2);
		const int o3 = line_orientation(p2, q2, p1);
		const int o4 = line_orientation(p2, q2, q1);

		// General case
		if (o1 != o2 && o3 != o4)
			return true;

		// Special Cases
		// p1, q1 and p2 are collinear and p2 lies on segment p1q1
		if (o1 == 0 && line_on_segment(p1, p2, q1)) return true;

		// p1, q1 and q2 are collinear and q2 lies on segment p1q1
		if (o2 == 0 && line_on_segment(p1, q2, q1)) return true;

		// p2, q2 and p1 are collinear and p1 lies on segment p2q2
		if (o3 == 0 && line_on_segment(p2, p1, q2)) return true;

		// p2, q2 and q1 are collinear and q1 lies on segment p2q2
		if (o4 == 0 && line_on_segment(p2, q1, q2)) return true;

		return false; // Doesn't fall in any of the above cases
	}

	LUNARIS_DECL bool collisionable_base::polygon_is_point_inside(const supported_fast_point_2d& p, const std::vector<supported_fast_point_2d>& polygon)
	{
		const size_t n = polygon.size();

		// There must be at least 3 vertices in polygon[]
		if (n < 3) return false;

		float maxx = 0.0f;
		for (const auto& it : polygon) {
			if (it.px > maxx) maxx = it.px + 1.0f;
		}
		if (p.px > maxx) maxx = p.px + 1.0f;

		// Create a point for line segment from p to infinite
		supported_fast_point_2d extreme = { maxx, p.py };

		// Count intersections of the above line with sides of polygon
		size_t count = 0, i = 0;
		do
		{
			size_t next = (i + 1) % n;

			// Check if the line segment from 'p' to 'extreme' intersects
			// with the line segment from 'polygon[i]' to 'polygon[next]'
			if (line_do_intersect(polygon[i], polygon[next], p, extreme))
			{
				// If the point 'p' is collinear with line segment 'i-next',
				// then check if it lies on segment. If it lies, return true,
				// otherwise false
				if (line_orientation(polygon[i], p, polygon[next]) == 0)
					return line_on_segment(polygon[i], p, polygon[next]);

				count++;
			}

			i = next;
		} while (i != 0);

		// Return true if count is odd, false otherwise
		return count & 1; // Same as (count%2 == 1)
	}


	LUNARIS_DECL supported_fast_point_2d& collisionable_base::vec_get_at(const size_t p)
	{
		if (p >= generated_on_think.size()) generated_on_think.resize(p); // guaranteed size resize. Do vec_fit though, more efficient
		return generated_on_think[p];
	}

	LUNARIS_DECL void collisionable_base::vec_fit(const size_t s)
	{
		if (s != generated_on_think.size()) generated_on_think.resize(s);
	}

	LUNARIS_DECL collisionable_base::collisionable_base() :
		fixed_multi_map_work<static_cast<size_t>(enum_fast_collisionable_float_e::_SIZE), float, enum_fast_collisionable_float_e>(default_fast_collisionable_float_il),
		fixed_multi_map_work<static_cast<size_t>(enum_fast_collisionable_boolean_e::_SIZE), bool, enum_fast_collisionable_boolean_e>(default_fast_collisionable_boolean_il)
	{
	}

	LUNARIS_DECL const std::vector<supported_fast_point_2d>& collisionable_base::read_points() const
	{
		return generated_on_think;
	}

	//LUNARIS_DECL float collisionable_base::get_dx() const
	//{
	//	return direction_x_revert;
	//}
	//
	//LUNARIS_DECL float collisionable_base::get_dy() const
	//{
	//	return direction_y_revert;
	//}

	LUNARIS_DECL void collisionable_base::collide_auto(collisionable_base& oth)
	{
		bool& last_was_collision = get<bool>(enum_fast_collisionable_boolean_e::RO_LAST_WAS_COLLISION);
		bool& oth_last_was_collision = oth.get<bool>(enum_fast_collisionable_boolean_e::RO_LAST_WAS_COLLISION);

		if (last_was_collision && oth_last_was_collision) return; // already col

		const auto& me_pts = read_points();
		const auto& oth_pts = oth.read_points();

		if (me_pts.size() < 2 || oth_pts.size() < 2) return;

		bool col_once = false;

		for (const auto& it : me_pts)
		{
			if (col_once |= polygon_is_point_inside(it, oth_pts))
				break;
		}

		if (!col_once) return;

		last_was_collision = true;
		oth_last_was_collision = true;

		float center_me[2] = { 0.0f,0.0f };
		float center_me_limit_min[2] = { me_pts[0].px, me_pts[0].py };
		float center_me_limit_max[2] = { me_pts[0].px, me_pts[0].py };
		float center_ot[2] = { 0.0f,0.0f };
		float center_ot_limit_min[2] = { oth_pts[0].px, oth_pts[0].py };
		float center_ot_limit_max[2] = { oth_pts[0].px, oth_pts[0].py };

		for (const auto& it : me_pts) {
			center_me[0] += it.px * 1.0f / me_pts.size();
			center_me[1] += it.py * 1.0f / me_pts.size();

			if (center_me_limit_min[0] > it.px) center_me_limit_min[0] = it.px;
			if (center_me_limit_min[1] > it.py) center_me_limit_min[1] = it.py;

			if (center_me_limit_max[0] < it.px) center_me_limit_max[0] = it.px;
			if (center_me_limit_max[1] < it.py) center_me_limit_max[1] = it.py;
		}
		for (const auto& it : oth_pts) {
			center_ot[0] += it.px * 1.0f / oth_pts.size();
			center_ot[1] += it.py * 1.0f / oth_pts.size();

			if (center_ot_limit_min[0] > it.px) center_ot_limit_min[0] = it.px;
			if (center_ot_limit_min[1] > it.py) center_ot_limit_min[1] = it.py;

			if (center_ot_limit_max[0] < it.px) center_ot_limit_max[0] = it.px;
			if (center_ot_limit_max[1] < it.py) center_ot_limit_max[1] = it.py;
		}

		const float& prop_move_col = get<float>(enum_fast_collisionable_float_e::PROPORTION_MOVE_ON_COLLISION);
		const float& alt_direction_prop = get<float>(enum_fast_collisionable_float_e::ALT_DIRECTION_PROP);
		const float& min_fix_delta = get<float>(enum_fast_collisionable_float_e::MINIMUM_FIX_DELTA);
		const float& fix_delta_center_prop = get<float>(enum_fast_collisionable_float_e::FIX_DELTA_CENTER_PROP);


		const float ddx = center_me[0] - center_ot[0];
		const float ddy = center_me[1] - center_ot[1];

		const float directx = ddx * 1.0f / (0.000000000001f + (fabsf(center_me_limit_max[0] - center_me_limit_min[0]) + fabsf(center_ot_limit_max[0] - center_ot_limit_min[0])));
		const float directy = ddy * 1.0f / (0.000000000001f + (fabsf(center_me_limit_max[1] - center_me_limit_min[1]) + fabsf(center_ot_limit_max[1] - center_ot_limit_min[1])));

		const float somemovex = (fabsf(directx) < fabsf(directy) ? alt_direction_prop : 1.0f) * (fabsf(fix_delta_center_prop * ddx) < min_fix_delta ? ((ddx >= 0.0f ? 1.0f : -1.0f) * min_fix_delta) : (fix_delta_center_prop * ddx));
		const float somemovey = (fabsf(directx) > fabsf(directy) ? alt_direction_prop : 1.0f) * (fabsf(fix_delta_center_prop * ddy) < min_fix_delta ? ((ddy >= 0.0f ? 1.0f : -1.0f) * min_fix_delta) : (fix_delta_center_prop * ddy));		

		get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_FINAL) += (somemovex + prop_move_col * get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_REVERT));
		get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_FINAL) += (somemovey + prop_move_col * get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_REVERT));
		oth.get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_FINAL) -= (somemovex + prop_move_col * get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_REVERT));
		oth.get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_FINAL) -= (somemovey + prop_move_col * get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_REVERT));
	}

	LUNARIS_DECL bool collisionable_base::collide_test(const collisionable_base& oth) const
	{
		const auto me_pts = read_points();
		const auto oth_pts = oth.read_points();
		bool col_once = false;

		for (const auto& it : me_pts)
		{
			if (col_once = polygon_is_point_inside(it, oth_pts))
				break;
		}
		return col_once;
	}

	LUNARIS_DECL bool collisionable_base::collide_test(const float& x, const float& y) const
	{
		const auto me_pts = read_points();
		return polygon_is_point_inside({ x, y }, me_pts);
	}

	LUNARIS_DECL void collisionable_base::apply()
	{
		revert_once();
	}

	LUNARIS_DECL collisionable_sprite::collisionable_sprite(sprite& s) : 
		fixed_multi_map_work<static_cast<size_t>(enum_fast_collisionable_sprite_float_e::_SIZE), float, enum_fast_collisionable_sprite_float_e>(default_fast_collisionable_sprite_float_il),
		collisionable_base(),
		ref(s)
	{
		vec_fit(4); // sprite is fixed!
	}

	LUNARIS_DECL void collisionable_sprite::revert_once()
	{
		const bool& last_was_collision = get<bool>(enum_fast_collisionable_boolean_e::RO_LAST_WAS_COLLISION);

		if (last_was_collision) {
			const float& direction_x_final = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_FINAL);
			const float& direction_y_final = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_FINAL);
			const float& reflectiveness = get<float>(enum_fast_collisionable_sprite_float_e::REFLECTIVENESS);

			ref.set<float>(enum_sprite_float_e::POS_X, ref.get<float>(enum_sprite_float_e::POS_X) + direction_x_final * 1.000001f);
			ref.set<float>(enum_sprite_float_e::POS_Y, ref.get<float>(enum_sprite_float_e::POS_Y) + direction_y_final * 1.000001f);

			ref.set<float>(enum_sprite_float_e::RO_THINK_SPEED_X, (direction_x_final >= 1.0f ? 1.0f : -1.0f) * fabsf(ref.get<float>(enum_sprite_float_e::RO_THINK_SPEED_X) * reflectiveness));
			ref.set<float>(enum_sprite_float_e::RO_THINK_SPEED_Y, (direction_y_final >= 1.0f ? 1.0f : -1.0f) * fabsf(ref.get<float>(enum_sprite_float_e::RO_THINK_SPEED_Y) * reflectiveness));
		}
	}

	LUNARIS_DECL void collisionable_sprite::think_once()
	{
		ref.think();

		const float entcenter[2] = { ref.get<float>(enum_sprite_float_e::POS_X), ref.get<float>(enum_sprite_float_e::POS_Y) };

		bool& last_was_collision = get<bool>(enum_fast_collisionable_boolean_e::RO_LAST_WAS_COLLISION);
		float& direction_x_final = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_FINAL);
		float& direction_y_final = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_FINAL);
		float& direction_x_revert = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_REVERT);
		float& direction_y_revert = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_REVERT);
		float& last_px = get<float>(enum_fast_collisionable_float_e::RO_LAST_PX);
		float& last_py = get<float>(enum_fast_collisionable_float_e::RO_LAST_PY);


		if (!last_was_collision) {
			direction_x_revert = last_px - entcenter[0];
			direction_y_revert = last_py - entcenter[1];
		}
		else last_was_collision = false;

		direction_x_final = direction_y_final = 0.0f;
		last_px = entcenter[0];
		last_py = entcenter[1];
		
		//printf_s("UPD %p [%.4f,%.4f]\n", (void*)this, direction_x_revert, direction_y_revert);

		vec_get_at(0).px = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X);
		vec_get_at(0).py = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y);
		vec_get_at(1).px = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X);
		vec_get_at(1).py = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y);
		vec_get_at(2).px = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X);
		vec_get_at(2).py = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y);
		vec_get_at(3).px = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X);
		vec_get_at(3).py = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y);
	}

	LUNARIS_DECL float collisionable_vertexes::get_center_x() const
	{
		float sum = 0.0f;
		const size_t len = ref.size();
		if (len == 0) return 0.0f;
		ref.csafe([&](const std::vector<vertex_point>& vec) {for (const auto& i : vec) sum += i.x; });
		sum /= len;
		return sum;
	}

	LUNARIS_DECL float collisionable_vertexes::get_center_y() const
	{
		float sum = 0.0f;
		const size_t len = ref.size();
		if (len == 0) return 0.0f;
		ref.csafe([&](const std::vector<vertex_point>& vec) {for (const auto& i : vec) sum += i.y; });
		sum /= len;
		return sum;
	}

	LUNARIS_DECL collisionable_vertexes::collisionable_vertexes(vertexes& s) :
		collisionable_base(),
		ref(s)
	{
	}

	LUNARIS_DECL void collisionable_vertexes::revert_once()
	{
		const bool& last_was_collision = get<bool>(enum_fast_collisionable_boolean_e::RO_LAST_WAS_COLLISION);

		if (last_was_collision) {
			const float& direction_x_final = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_FINAL);
			const float& direction_y_final = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_FINAL);

			ref.translate(direction_x_final * 1.000001f, direction_y_final * 1.000001f);
		}
	}

	LUNARIS_DECL void collisionable_vertexes::think_once()
	{
		const float entcenter[2] = { get_center_x(), get_center_y() };

		bool& last_was_collision = get<bool>(enum_fast_collisionable_boolean_e::RO_LAST_WAS_COLLISION);
		float& direction_x_final = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_FINAL);
		float& direction_y_final = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_FINAL);
		float& direction_x_revert = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_X_REVERT);
		float& direction_y_revert = get<float>(enum_fast_collisionable_float_e::RO_DIRECTION_Y_REVERT);
		float& last_px = get<float>(enum_fast_collisionable_float_e::RO_LAST_PX);
		float& last_py = get<float>(enum_fast_collisionable_float_e::RO_LAST_PY);

		ref.generate_transformed();
		vec_fit(ref.size());
		ref.csafe_transformed([&](const std::vector<vertex_point>& vec) { for (size_t p = 0; p < vec.size(); ++p) { vec_get_at(p).px = vec[p].x; vec_get_at(p).py = vec[p].y; } });

		if (!last_was_collision) {
			direction_x_revert = last_px - entcenter[0];
			direction_y_revert = last_py - entcenter[1];
		}
		else last_was_collision = false;

		direction_x_final = direction_y_final = 0.0f;
		last_px = entcenter[0];
		last_py = entcenter[1];
	}

	LUNARIS_DECL void collisionable_manager::push_back(sprite& s, const bool lckd)
	{
		auto ptr = std::unique_ptr<collisionable_base>(new collisionable_sprite(s));
		ptr->set<bool>(enum_fast_collisionable_boolean_e::LOCKED, lckd);
		objs.push_back(std::move(ptr));
	}

	LUNARIS_DECL void collisionable_manager::push_back(vertexes& s, const bool lckd)
	{
		auto ptr = std::unique_ptr<collisionable_base>(new collisionable_vertexes(s));
		ptr->set<bool>(enum_fast_collisionable_boolean_e::LOCKED, lckd);
		objs.push_back(std::move(ptr));
	}

	LUNARIS_DECL size_t collisionable_manager::size() const
	{
		return objs.size();
	}

	LUNARIS_DECL bool collisionable_manager::valid() const
	{
		return objs.size() >= 2;
	}

	LUNARIS_DECL bool collisionable_manager::empty() const
	{
		return objs.size() == 2;
	}

	LUNARIS_DECL void collisionable_manager::safe(std::function<void(std::vector<std::unique_ptr<collisionable_base>>&)> f)
	{
		if (f) return;
		std::unique_lock<std::shared_mutex> luck(objs_safe);

		f(objs);
	}

	LUNARIS_DECL void collisionable_manager::csafe(std::function<void(const std::vector<std::unique_ptr<collisionable_base>>&)> f) const
	{
		if (f) return;
		std::shared_lock<std::shared_mutex> luck(objs_safe);

		f(objs);
	}

	LUNARIS_DECL void collisionable_manager::think_all()
	{
		std::unique_lock<std::shared_mutex> luck(objs_safe);

		for (auto& it : objs) it->think_once(); // think()

		for (size_t a = 0; a < objs.size(); ++a)
		{
			for (size_t b = 0; b < objs.size(); ++b)
			{
				if (b != a) {
					objs[a]->collide_auto(*objs[b].get());
				}
			}
		}

		for (auto& it : objs) it->apply(); // do move if needed (based on collision)
	}
}