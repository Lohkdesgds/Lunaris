#include "collisionable.h"

namespace Lunaris {

	LUNARIS_DECL bool line_on_segment(const supported_fast_point_2d& p, const supported_fast_point_2d& q, const supported_fast_point_2d& r)
	{
		return ((q.px <= std::max(p.px, r.px)) && (q.px >= std::min(p.px, r.px)) &&
			(q.py <= std::max(p.py, r.py)) && (q.py >= std::min(p.py, r.py)));
	}

	LUNARIS_DECL int line_orientation(const supported_fast_point_2d& p, const supported_fast_point_2d& q, const supported_fast_point_2d& r)
	{
		const int val = (q.py - p.py) * (r.px - q.px) - (q.px - p.px) * (r.py - q.py);
		return (val == 0) ? 0 : (val > 0 ? 1 : 2);
	}

	LUNARIS_DECL bool line_do_intersect(const supported_fast_point_2d& p1, const supported_fast_point_2d& q1, const supported_fast_point_2d& p2, const supported_fast_point_2d& q2)
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

	LUNARIS_DECL bool polygon_is_point_inside(const supported_fast_point_2d& p, const std::vector<supported_fast_point_2d>& polygon)
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


	LUNARIS_DECL supported_fast_point_2d& supported_fast_collisionable::vec_get_at(const size_t p)
	{
		if (p >= generated_on_think.size()) generated_on_think.resize(p); // guaranteed size resize. Do vec_fit though, more efficient
		return generated_on_think[p];
	}

	LUNARIS_DECL void supported_fast_collisionable::vec_fit(const size_t s)
	{
		if (s != generated_on_think.size()) generated_on_think.resize(s);
	}

	LUNARIS_DECL const std::vector<supported_fast_point_2d>& supported_fast_collisionable::read_points() const
	{
		return generated_on_think;
	}

	LUNARIS_DECL float supported_fast_collisionable::get_dx() const
	{
		return direction_x_revert;
	}

	LUNARIS_DECL float supported_fast_collisionable::get_dy() const
	{
		return direction_y_revert;
	}

	LUNARIS_DECL void supported_fast_collisionable::collide_auto(supported_fast_collisionable& oth)
	{
		const auto me_pts = read_points();
		const auto oth_pts = oth.read_points();
		bool col_once = false;

		for (const auto& it : me_pts)
		{
			if (col_once = polygon_is_point_inside(it, oth_pts))
				break;
		}

		if (!col_once) return;

		float widn = 0.0f; // sum X
		float hein = 0.0f; // sum Y

		for (const auto& it : me_pts) {
			widn += it.px * 1.0f / me_pts.size();
			hein += it.py * 1.0f / me_pts.size();
		}
		for (const auto& it : oth_pts) {
			widn -= it.px * 1.0f / oth_pts.size();
			hein -= it.py * 1.0f / oth_pts.size();
		}

		// if widn > 0, OTH - - - - THIS
		// if hein > 0, THIS is below OTH

		const float coefx = fabsf(get_dx() - oth.get_dx());
		const float coefy = fabsf(get_dy() - oth.get_dy());
		const float divf = (coefx + coefy == 0.0f) ? 1.0f : (coefx + coefy);

		if (!locked) {
			direction_x_revert += 0.12f * (coefx) * (widn > 0.0f ? 1.0f : -1.0f) * (coefx * 1.0f / divf) + (widn > 0.0f ? 0.000001f : -0.000001f);
			direction_y_revert += 0.12f * (coefy) * (hein > 0.0f ? 1.0f : -1.0f) * (coefy * 1.0f / divf) + (hein > 0.0f ? 0.000001f : -0.000001f);
		}
		if (!oth.locked) {
			oth.direction_x_revert += -0.12f * (coefx) * (widn > 0.0f ? 1.0f : -1.0f) * (coefx * 1.0f / divf) - (widn > 0.0f ? 0.000001f : -0.000001f);
			oth.direction_y_revert += -0.12f * (coefy) * (hein > 0.0f ? 1.0f : -1.0f) * (coefy * 1.0f / divf) - (hein > 0.0f ? 0.000001f : -0.000001f);
		}

		last_was_collision = true;
		oth.last_was_collision = true;

		revert_once();
		oth.revert_once();
	}

	LUNARIS_DECL bool supported_fast_collisionable::collide(const supported_fast_collisionable& oth) const
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

	LUNARIS_DECL void supported_fast_collisionable::lock(const bool b)
	{
		locked = b;
	}

	LUNARIS_DECL void supported_fast_collisionable::apply()
	{
		if (last_was_collision)
			revert_once();
	}

	LUNARIS_DECL fast_collisionable_sprite::fast_collisionable_sprite(sprite& s)
		: ref(s)
	{
		vec_fit(4); // sprite is fixed!
	}

	LUNARIS_DECL void fast_collisionable_sprite::revert_once()
	{
		if (fabsf(direction_x_revert) > 2.0f) direction_x_revert /= 0.5f * fabsf(direction_x_revert);
		if (fabsf(direction_y_revert) > 2.0f) direction_y_revert /= 0.5f * fabsf(direction_y_revert);
		//const float& speed_deg_ref = ref.get<float>(enum_sprite_float_e::THINK_ELASTIC_SPEED_PROP);

		ref.set<float>(enum_sprite_float_e::POS_X, ref.get<float>(enum_sprite_float_e::POS_X) + direction_x_revert);
		ref.set<float>(enum_sprite_float_e::POS_Y, ref.get<float>(enum_sprite_float_e::POS_Y) + direction_y_revert);
		ref.set<float>(enum_sprite_float_e::RO_THINK_SPEED_X, ref.get<float>(enum_sprite_float_e::RO_THINK_SPEED_X) * 0.25f);
		ref.set<float>(enum_sprite_float_e::RO_THINK_SPEED_Y, ref.get<float>(enum_sprite_float_e::RO_THINK_SPEED_Y) * 0.25f);
	}

	LUNARIS_DECL void fast_collisionable_sprite::think_once()
	{
		ref.think();
		const float entcenter[2] = { ref.get<float>(enum_sprite_float_e::POS_X), ref.get<float>(enum_sprite_float_e::POS_Y) };

		vec_get_at(0).px = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X);
		vec_get_at(0).py = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y);
		vec_get_at(1).px = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X);
		vec_get_at(1).py = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y);
		vec_get_at(2).px = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X);
		vec_get_at(2).py = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y);
		vec_get_at(3).px = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X);
		vec_get_at(3).py = ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y);

		if (last_was_collision) {
			last_was_collision = false;
			direction_x_revert += 0.1f * (last_px - entcenter[0]);
			last_px = entcenter[0];
			direction_y_revert += 0.1f * (last_py - entcenter[1]);
			last_py = entcenter[1];
		}
		else {
			direction_x_revert = last_px - entcenter[0];
			last_px = entcenter[0];
			direction_y_revert = last_py - entcenter[1];
			last_py = entcenter[1];
		}
	}

	LUNARIS_DECL float fast_collisionable_vertexes::get_center_x() const
	{
		float sum = 0.0f;
		const size_t len = ref.size();
		if (len == 0) return 0.0f;
		ref.csafe([&](const std::vector<vertex_point>& vec) {for (const auto& i : vec) sum += i.x; });
		sum /= len;
		return sum;
	}

	LUNARIS_DECL float fast_collisionable_vertexes::get_center_y() const
	{
		float sum = 0.0f;
		const size_t len = ref.size();
		if (len == 0) return 0.0f;
		ref.csafe([&](const std::vector<vertex_point>& vec) {for (const auto& i : vec) sum += i.y; });
		sum /= len;
		return sum;
	}

	LUNARIS_DECL fast_collisionable_vertexes::fast_collisionable_vertexes(vertexes& s)
		: ref(s)
	{
	}

	LUNARIS_DECL void fast_collisionable_vertexes::revert_once()
	{
		if (fabsf(direction_x_revert) > 0.5f) direction_x_revert /= 2.0f * (fabsf(direction_x_revert) + 0.0001f);
		if (fabsf(direction_y_revert) > 0.5f) direction_y_revert /= 2.0f * (fabsf(direction_y_revert) + 0.0001f);
		//const float& speed_deg_ref = ref.get<float>(enum_sprite_float_e::THINK_ELASTIC_SPEED_PROP);

		ref.translate(direction_x_revert, direction_y_revert);
		//ref.safe([&](std::vector<vertex_point>& vec) {
		//	for (auto& it : vec) {
		//		it.x += direction_x_revert;
		//		it.y += direction_y_revert;
		//	}
		//});
	}

	LUNARIS_DECL void fast_collisionable_vertexes::think_once()
	{
		const float entcenter[2] = { get_center_x(), get_center_y() };

		ref.generate_transformed();
		vec_fit(ref.size());
		ref.csafe_transformed([&](const std::vector<vertex_point>& vec) { for (size_t p = 0; p < vec.size(); ++p) { vec_get_at(p).px = vec[p].x; vec_get_at(p).py = vec[p].y; } });

		if (last_was_collision) {
			last_was_collision = false;
			direction_x_revert += 0.1f * (last_px - entcenter[0]);
			last_px = entcenter[0];
			direction_y_revert += 0.1f * (last_py - entcenter[1]);
			last_py = entcenter[1];
		}
		else {
			direction_x_revert = last_px - entcenter[0];
			last_px = entcenter[0];
			direction_y_revert = last_py - entcenter[1];
			last_py = entcenter[1];
		}
	}

	LUNARIS_DECL void fast_collisionable_manager::push_back(sprite& s, const bool lckd)
	{
		auto ptr = std::unique_ptr<supported_fast_collisionable>(new fast_collisionable_sprite(s));
		ptr->lock(lckd);
		objs.push_back(std::move(ptr));
	}

	LUNARIS_DECL void fast_collisionable_manager::push_back(vertexes& s, const bool lckd)
	{
		auto ptr = std::unique_ptr<supported_fast_collisionable>(new fast_collisionable_vertexes(s));
		ptr->lock(lckd);
		objs.push_back(std::move(ptr));
	}

	LUNARIS_DECL void fast_collisionable_manager::think_all()
	{
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


//	LUNARIS_DECL bool collisionable::final_result::is_sprite() const
//	{
//		return spr.has_value();
//	}
//
//	LUNARIS_DECL bool collisionable::final_result::is_vertexes() const
//	{
//		return vex.has_value();
//	}
//
//	LUNARIS_DECL sprite& collisionable::final_result::get_sprite()
//	{
//		if (spr.has_value()) return spr->get();
//		throw std::runtime_error("Invalid option! This is not a sprite!");
//		return spr->get(); // never here
//	}
//
//	LUNARIS_DECL vertexes& collisionable::final_result::get_vertexes()
//	{
//		if (vex.has_value()) return vex->get();
//		throw std::runtime_error("Invalid option! This is not a vertexes!");
//		return vex->get(); // never here
//	}
//
//	/* * * * * * * * * * * * * * * * * * * * * * * * * LINE COL SYSTEM * * * * * * * * * * * * * * * * * * * * * * * * */
//
//	LUNARIS_DECL bool collisionable::line_on_segment(const point_2d_ref& p, const point_2d_ref& q, const point_2d_ref& r) const
//	{
//		return ((q.px <= std::max(p.px, r.px)) && (q.px >= std::min(p.px, r.px)) &&
//			(q.py <= std::max(p.py, r.py)) && (q.py >= std::min(p.py, r.py)));
//	}
//
//	LUNARIS_DECL int collisionable::line_orientation(const point_2d_ref &p, const point_2d_ref& q, const point_2d_ref& r) const
//	{
//		const int val = (q.py - p.py) * (r.px - q.px) - (q.px - p.px) * (r.py - q.py);
//		return (val == 0) ? 0 : (val > 0 ? 1 : 2);
//	}
//
//	LUNARIS_DECL bool collisionable::line_do_intersect(const point_2d_ref& p1, const point_2d_ref& q1, const point_2d_ref& p2, const point_2d_ref& q2) const
//	{
//		const int o1 = line_orientation(p1, q1, p2);
//		const int o2 = line_orientation(p1, q1, q2);
//		const int o3 = line_orientation(p2, q2, p1);
//		const int o4 = line_orientation(p2, q2, q1);
//
//		// General case
//		if (o1 != o2 && o3 != o4)
//			return true;
//
//		// Special Cases
//		// p1, q1 and p2 are collinear and p2 lies on segment p1q1
//		if (o1 == 0 && line_on_segment(p1, p2, q1)) return true;
//
//		// p1, q1 and q2 are collinear and q2 lies on segment p1q1
//		if (o2 == 0 && line_on_segment(p1, q2, q1)) return true;
//
//		// p2, q2 and p1 are collinear and p1 lies on segment p2q2
//		if (o3 == 0 && line_on_segment(p2, p1, q2)) return true;
//
//		// p2, q2 and q1 are collinear and q1 lies on segment p2q2
//		if (o4 == 0 && line_on_segment(p2, q1, q2)) return true;
//
//		return false; // Doesn't fall in any of the above cases
//	}
//
//	/* * * * * * * * * * * * * * * * * * * * * * * * * POINT COL SYSTEM * * * * * * * * * * * * * * * * * * * * * * * * */
//
//	LUNARIS_DECL float collisionable::polygon_area(const std::vector<point_2d_ref>& lst) const {
//		float area = 0.0f;
//		size_t j = lst.size() - 1;
//		for (size_t i = 0; i < lst.size(); i++) {
//			area += (lst[j].px + lst[i].px) * (lst[j].py - lst[i].py);
//			j = i;
//		}
//		return fabsf(area * 0.5f);
//	}
//
//	LUNARIS_DECL bool collisionable::polygon_collision_dot(const point_2d_ref &dot) const
//	{
//		if (points.size() <= 2) return false;
//
//		const float maxarea = polygon_area(points);
//		const auto* begg = points.data();
//
//		float dotarea = 0.0f;
//		size_t j = points.size() - 1;
//
//		for (size_t p = 0; p < points.size(); ++p)
//		{
//			dotarea += polygon_area({
//				point_2d_ref{dot.px, dot.py},
//				point_2d_ref{begg[p].px,begg[p].py},
//				point_2d_ref{begg[j].px,begg[j].py}
//			});
//
//			j = p;
//		}
//
//		return maxarea * default_collision_oversize_prop >= dotarea;
//	}
//
//	LUNARIS_DECL collisionable::point_2d collisionable::polygon_get_center() const
//	{
//		if (points.size() == 0) return {};
//
//		point_2d pt;
//
//		for (const auto& it : points)
//		{
//			pt.px += it.px * (1.0f / points.size());
//			pt.py += it.py * (1.0f / points.size());
//		}
//
//		return pt;
//	}
//
//	LUNARIS_DECL void collisionable::polygon_get_min_max(point_2d& ma, point_2d& mi) const
//	{
//		if (points.size() == 0) return;
//		ma.px = mi.px = points[0].px;
//		ma.py = mi.py = points[0].py;
//		for (size_t p = 1; p < points.size(); ++p) {
//			const auto& it = points[p];
//			if (it.px > ma.px) ma.px = it.px;
//			if (it.px < mi.px) mi.px = it.px;
//			if (it.py > ma.py) ma.py = it.py;
//			if (it.py < mi.py) mi.py = it.py;
//		}
//	}
//
//	LUNARIS_DECL bool collisionable::polygon_get_triangle_nearest_of(const point_2d& dot, point_2d& near1, point_2d& near2) const
//	{
//		if (points.size() < 2) return false;
//
//		struct sort_it {
//			size_t off;
//			float dist = 0.0f;
//		};
//		std::vector<sort_it> vec;
//
//		for (size_t p = 0; p < points.size(); ++p)
//		{
//			vec.push_back({
//				p,
//				powf((dot.px - points[p].px) * (dot.px - points[p].px), 2.0f) + powf((dot.py - points[p].py) * (dot.py - points[p].py), 2.0f)
//				});
//		}
//
//		// sort by lowest
//		std::sort(vec.begin(), vec.end(), [](const sort_it& a, const sort_it& b) { return a.dist < b.dist; });
//
//		// copy 2 lower vals. Guaranteed 2 because of check before.
//		near1.px = points[vec[0].off].px;
//		near1.py = points[vec[0].off].py;
//		near2.px = points[vec[1].off].px;
//		near2.py = points[vec[1].off].py;
//
//		return true;
//	}
//
//	/* * * * * * * * * * * * * * * * * * * * * * * * * FULL COL SYSTEM * * * * * * * * * * * * * * * * * * * * * * * * */
//
//
//	LUNARIS_DECL collisionable::collisionable(sprite& s)
//	{
//		replace(s);
//	}
//
//	LUNARIS_DECL collisionable::collisionable(vertexes& v)
//	{
//		replace(v);
//	}
//
//	LUNARIS_DECL void collisionable::replace(sprite& s)
//	{
//		std::unique_lock<shared_recursive_mutex> luck(safe_mtx);
//
//		points.clear();
//		vertexes_opt.reset();
//		sprite_opt = s;
//		compensation = s.copy_transform_in_use();
//
//		update_points();
//	}
//
//	LUNARIS_DECL void collisionable::replace(vertexes& v)
//	{
//		std::unique_lock<shared_recursive_mutex> luck(safe_mtx);
//
//		points.clear();
//		sprite_opt.reset();
//		vertexes_opt = v;
//		compensation = v.copy_transform_in_use();
//
//		update_points();
//	}
//
//	LUNARIS_DECL void collisionable::reset()
//	{
//		std::unique_lock<shared_recursive_mutex> luck(safe_mtx);
//
//		if (empty()) throw std::runtime_error("Can't do reset() with no parameter if sprite is not set.");
//		results.clear();
//		compensation = is_sprite() ? sprite_opt->get().copy_transform_in_use() : vertexes_opt->get().copy_transform_in_use();
//		if (is_vertexes()) update_points();
//	}
//
//	LUNARIS_DECL bool collisionable::collide(collisionable& oth)
//	{
//		std::unique_lock<shared_recursive_mutex> luck1(safe_mtx);
//		std::unique_lock<shared_recursive_mutex> luck2(oth.safe_mtx);
//
//		point_2d th_limits[2]{};
//		point_2d oh_limits[2]{};
//		
//		polygon_get_min_max(th_limits[1], th_limits[0]);
//		oth.polygon_get_min_max(oh_limits[1], oh_limits[0]);
//
//		// if one's max is less than min of other or vice versa then no col.
//		if (th_limits[1].px < oh_limits[0].px || th_limits[0].px > oh_limits[1].px ||
//			th_limits[1].py < oh_limits[0].py || th_limits[0].py > oh_limits[1].py)
//			return false;
//
//
//		bool did_col_once = false;
//		auto objc = polygon_get_center();
//		
//		for (const auto& ea : points)
//		{
//			if (oth.polygon_collision_dot(ea))
//			{
//				did_col_once = true;
//				point_2d nears[2];
//
//				oth.polygon_get_triangle_nearest_of(objc, nears[0], nears[1]);
//
//				const float m = (nears[1].py - nears[0].py) / (nears[1].px - (nears[0].px == nears[1].px ? (nears[0].px + 1) : nears[0].px));
//				const float b = nears[0].py - (m * nears[0].px);
//
//				const float x = (m * ea.py + ea.px - m * b) / (m * m + 1);
//				const float y = (m * m * ea.py + m * ea.px + b) / (m * m + 1);
//
//				each_result res, oth_res;
//
//				res.has_collision = true;
//				res.precise[0] = (x - ea.px) * 0.5f;
//				res.precise[1] = (y - ea.py) * 0.5f;
//
//				oth_res.has_collision = true;
//				oth_res.precise[0] = (x - ea.px) * (-0.5f);
//				oth_res.precise[1] = (y - ea.py) * (-0.5f);
//
//				results.push_back(res);
//				oth.results.push_back(oth_res);
//			}
//		}
//		
//		if (!did_col_once)
//		{
//			for (size_t p = 0; !did_col_once && p < (points.size() - 2); ++p)
//			{
//				for (size_t j = 0; !did_col_once && j < (oth.points.size() - 2); ++j) {
//					did_col_once |= line_do_intersect(points[p], points[(p + 2) % points.size()], oth.points[j], oth.points[(j + 2) % oth.points.size()]);
//				}
//			}
//		
//			if (did_col_once)
//			{
//				auto objc_oth = oth.polygon_get_center();
//		
//				const float dx = (objc.px - objc_oth.px); // positive = objc is to the right, object to left
//				const float dy = (objc.py - objc_oth.py); // positive = objc is down, object is up
//		
//				each_result res, oth_res;
//				res.has_collision = true;
//				oth_res.has_collision = true;
//		
//				if (fabsf(dx) < fabsf(dy)) // y
//				{
//					const float dymax = th_limits[1].py - th_limits[0].py;
//
//					res.extended[1] = (dymax > dy ? (dymax - dy) : dy) * 0.25f;
//					oth_res.extended[1] = (dymax > dy ? (dymax - dy) : dy) * (-0.25f);
//				}
//				else // x
//				{
//					const float dxmax = th_limits[1].px - th_limits[0].px;
//
//					res.extended[0] = (dxmax > dx ? (dxmax - dx) : dx) * 0.25f;
//					oth_res.extended[0] = (dxmax > dx ? (dxmax - dx) : dx) * (-0.25f);
//				}
//		
//				//compensation.transform_coords(res.precise[0], res.precise[1]);
//		
//				results.push_back(res);
//				oth.results.push_back(oth_res);
//			}
//		}
//
//		return did_col_once;
//	}
//
//	LUNARIS_DECL void collisionable::set_work(std::function<void(final_result&)> f)
//	{
//		if (!f) throw std::invalid_argument("Work function may not be empty!");
//		std::unique_lock<shared_recursive_mutex> luck(safe_mtx);
//		workar = f;
//	}
//
//	LUNARIS_DECL void collisionable::work()
//	{
//		if (results.empty() || !workar || empty()) return;
//
//		std::unique_lock<shared_recursive_mutex> luck(safe_mtx);
//
//		final_result fr;
//
//#ifdef LUNARIS_VERBOSE_BUILD
//		PRINT_DEBUG("(%p) Has collision [%zu]", (void*)this, results.size());
//#endif
//
//		for (const auto& it : results)
//		{
//			fr.fx += it.precise[0] * 1.0f / results.size();
//			fr.fy += it.precise[1] * 1.0f / results.size();
//		}
//
//		if (fr.fx == 0.0f && fr.fy == 0.0f)
//		{
//#ifdef LUNARIS_VERBOSE_BUILD
//			PRINT_DEBUG("(%p) Collision is not dot type. Calculating by center instead.", (void*)this);
//#endif
//
//			for (const auto& it : results)
//			{
//				fr.fx += it.extended[0] * 1.0f / results.size();
//				fr.fy += it.extended[1] * 1.0f / results.size();
//			}
//		}
//
//		if (fr.fx == 0.0f && fr.fy == 0.0f) { // hmmm do nothing
//			return;
//		}
//
//		float comp[2] = { 1.0f, 1.0f };
//
//		compensation.transform_coords(comp[0], comp[1]);
//
//		fr.fx /= (fabsf(comp[0]) + 0.001f);
//		fr.fy /= (fabsf(comp[1]) + 0.001f);
//
//		if (is_vertexes()) {
//			fr.vex = vertexes_opt;
//		}
//		else {
//			fr.spr = sprite_opt;
//		}
//
//		workar(fr);
//	}
//
//	LUNARIS_DECL bool collisionable::valid() const
//	{
//		return sprite_opt.has_value() || vertexes_opt.has_value();
//	}
//
//	LUNARIS_DECL bool collisionable::empty() const
//	{
//		return !sprite_opt.has_value() && !vertexes_opt.has_value();
//	}
//
//	LUNARIS_DECL bool collisionable::is_vertexes() const
//	{
//		return vertexes_opt.has_value();
//	}
//
//	LUNARIS_DECL bool collisionable::is_sprite() const
//	{
//		return sprite_opt.has_value();
//	}
//
//	LUNARIS_DECL void collisionable::update_points()
//	{
//		if (empty()) return;
//
//		std::unique_lock<shared_recursive_mutex> luck(safe_mtx);
//
//		if (is_sprite())
//		{
//			points.push_back({ sprite_opt->get().get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X), sprite_opt->get().get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y) });
//			points.push_back({ sprite_opt->get().get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X), sprite_opt->get().get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y) });
//			points.push_back({ sprite_opt->get().get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X), sprite_opt->get().get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y) });
//			points.push_back({ sprite_opt->get().get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X), sprite_opt->get().get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y) });		
//		}
//		else
//		{
//			vertexes_opt->get().generate_transformed();
//			vertexes_opt->get().csafe_transformed([this](const std::vector<vertex_point>& vec) {
//				for (auto& it : vec) {
//					points.push_back({ it.x, it.y });
//				}
//			});
//		}
//	}
//
//	LUNARIS_DECL void collisionable::csafe(std::function<void(const std::vector<point_2d_ref>&)> f) const
//	{
//		if (!f) return;
//		std::shared_lock<shared_recursive_mutex> luck(safe_mtx);
//		f(points);
//	}
//
//	LUNARIS_DECL float collisionable_legacy::get_size_x() const
//	{
//		return default_collision_oversize + (wrap.get<float>(enum_sprite_float_e::SCALE_G) * wrap.get<float>(enum_sprite_float_e::SCALE_X));
//	}
//
//	LUNARIS_DECL float collisionable_legacy::get_size_y() const
//	{
//		return default_collision_oversize + (wrap.get<float>(enum_sprite_float_e::SCALE_G) * wrap.get<float>(enum_sprite_float_e::SCALE_Y));
//	}
//
//	LUNARIS_DECL collisionable_legacy::collisionable_legacy(sprite& wrp)
//		: wrap(wrp), posx(wrp.get<float>(enum_sprite_float_e::POS_X)), posy(wrp.get<float>(enum_sprite_float_e::POS_Y))
//	{
//	}
//
//	LUNARIS_DECL bool collisionable_legacy::overlap(const collisionable_legacy& ol)
//	{
//		const float sizx = get_size_x();
//		const float sizy = get_size_y();
//		const float ol_sizx = ol.get_size_x();
//		const float ol_sizy = ol.get_size_y();
//
//		const float dist_x = (posx - ol.posx);
//		const float diff_x = fabs(dist_x) - (sizx + ol_sizx) * 1.0f / 2.0f; // if < 0, col
//		const bool col_x = diff_x < 0.0;
//		const float dist_y = (posy - ol.posy);
//		const float diff_y = fabs(dist_y) - (sizy + ol_sizy) * 1.0f / 2.0f; // if < 0, col
//		const bool col_y = diff_y < 0.0;
//		const bool is_col = col_y && col_x;
//
//		if (is_col) {
//			if (fabs(diff_x) < fabs(diff_y)) {
//				if (fabs(diff_x) > fabs(dx_max)) dx_max = diff_x;
//
//				if (dist_x > 0.0) {
//					directions_cases[static_cast<int>(direction_internal::WEST)]++; // VEM DO WEST
//				}
//				else {
//					directions_cases[static_cast<int>(direction_internal::EAST)]++; // VEM DO EAST
//				}
//			}
//			else {
//				if (fabs(diff_y) > fabs(dy_max)) dy_max = diff_y;
//				if (dist_y > 0.0) {
//					directions_cases[static_cast<int>(direction_internal::NORTH)]++; // VEM DO NORTH
//				}
//				else {
//					directions_cases[static_cast<int>(direction_internal::SOUTH)]++; // VEM DO SOUTH
//				}
//			}
//		}
//
//		/*
//		Y:
//		P1 - P2 > 0 == col vem do NORTH
//		P1 - P2 < 0 == col vem do SOUTH
//
//		X:
//		P1 - P2 > 0 == col vem do WEST
//		P1 - P2 < 0 == col vem do EAST
//		*/
//
//		was_col |= is_col;
//		return is_col;
//	}
//
//	LUNARIS_DECL int collisionable_legacy::result() const
//	{
//		const bool n = directions_cases[static_cast<int>(direction_internal::NORTH)] > 0;
//		const bool s = directions_cases[static_cast<int>(direction_internal::SOUTH)] > 0;
//		const bool w = directions_cases[static_cast<int>(direction_internal::WEST)] > 0;
//		const bool e = directions_cases[static_cast<int>(direction_internal::EAST)] > 0;
//
//		const int east = static_cast<int>(direction_op::DIR_EAST);
//		const int west = static_cast<int>(direction_op::DIR_WEST);
//		const int north = static_cast<int>(direction_op::DIR_NORTH);
//		const int south = static_cast<int>(direction_op::DIR_SOUTH);
//
//
//		if (n) { // north
//			if (w && !e) {
//				return south | east; // GOTO EAST
//			}
//			if (e && !w) {
//				return south | west; // GOTO WEST
//			}
//			if (s) {
//				if (directions_cases[static_cast<int>(direction_internal::NORTH)] > directions_cases[static_cast<int>(direction_internal::SOUTH)]) return south;
//				if (directions_cases[static_cast<int>(direction_internal::SOUTH)] > directions_cases[static_cast<int>(direction_internal::NORTH)]) return north;
//				return 0; // NO GOTO
//			}
//			return south; // GOTO SOUTH
//		}
//		else if (s) { //south
//			if (w && !e) {
//				return north | east; // GOTO EAST
//			}
//			if (e && !w) {
//				return north | west; // GOTO WEST
//			}
//			return north; // GOTO NORTH
//		}
//		else if (w) { // west
//			if (e) {
//				if (directions_cases[static_cast<int>(direction_internal::EAST)] > directions_cases[static_cast<int>(direction_internal::WEST)]) return west;
//				if (directions_cases[static_cast<int>(direction_internal::WEST)] > directions_cases[static_cast<int>(direction_internal::EAST)]) return east;
//				return 0;
//			}
//			return east;
//		}
//		else if (e) { // east
//			return west;
//		}
//		return 0;
//	}
//
//	LUNARIS_DECL void collisionable_legacy::reset()
//	{
//		was_col = false;
//		for (auto& i : directions_cases) i = 0;
//		dx_max = dy_max = 0.0;
//	}
//
//	LUNARIS_DECL void collisionable_legacy::work()
//	{
//		if (workar) {
//			int res = this->result();
//			workar(res, wrap);
//		}
//	}
//
//	LUNARIS_DECL void collisionable_legacy::set_work(const std::function<void(int, sprite&)> f)
//	{
//		workar = f;
//	}
//
//	LUNARIS_DECL unsigned collisionable_legacy::read_cases(const direction_op transl) const
//	{
//		switch (transl) {
//		case direction_op::DIR_NORTH:
//			return directions_cases[static_cast<int>(direction_internal::NORTH)];
//		case direction_op::DIR_SOUTH:
//			return directions_cases[static_cast<int>(direction_internal::SOUTH)];
//		case direction_op::DIR_EAST:
//			return directions_cases[static_cast<int>(direction_internal::EAST)];
//		case direction_op::DIR_WEST:
//			return directions_cases[static_cast<int>(direction_internal::WEST)];
//		default:
//			return 0;
//		}
//	}
//	
//	LUNARIS_DECL void work_all_auto(collisionable* be, const collisionable* en)
//	{
//		if (be >= en) return;
//	
//		for (collisionable* ptr_p = be; ptr_p != en; ++ptr_p) ptr_p->reset();
//		
//		for (collisionable* ptr_a = be; ptr_a != en; ++ptr_a)
//		{
//			for (collisionable* ptr_b = be; ptr_b != en; ++ptr_b)
//			{
//				if (ptr_a != ptr_b) ptr_a->collide(*ptr_b);
//			}
//		}
//	
//		for (collisionable* ptr_p = be; ptr_p != en; ++ptr_p) ptr_p->work();
//	}

}