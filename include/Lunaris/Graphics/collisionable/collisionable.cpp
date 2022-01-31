#include "collisionable.h"

namespace Lunaris {

	/* * * * * * * * * * * * * * * * * * * * * * * * * LINE COL SYSTEM * * * * * * * * * * * * * * * * * * * * * * * * */

	LUNARIS_DECL bool collisionable::line_on_segment(const point_2d_ref& p, const point_2d_ref& q, const point_2d_ref& r) const
	{
		return ((q.px <= std::max(p.px, r.px)) && (q.px >= std::min(p.px, r.px)) &&
			(q.py <= std::max(p.py, r.py)) && (q.py >= std::min(p.py, r.py)));
	}

	// To find orientation of ordered triplet (p, q, r).
	// The function returns following values
	// 0 --> p, q and r are collinear
	// 1 --> Clockwise
	// 2 --> Counterclockwise
	LUNARIS_DECL int collisionable::line_orientation(const point_2d_ref &p, const point_2d_ref& q, const point_2d_ref& r) const
	{
		const int val = (q.py - p.py) * (r.px - q.px) - (q.px - p.px) * (r.py - q.py);
		return (val == 0) ? 0 : (val > 0 ? 1 : 2);
	}

	// The main function that returns true if line segment 'p1q1'
	// and 'p2q2' intersect.
	LUNARIS_DECL bool collisionable::line_do_intersect(const point_2d_ref& p1, const point_2d_ref& q1, const point_2d_ref& p2, const point_2d_ref& q2) const
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

	/* * * * * * * * * * * * * * * * * * * * * * * * * POINT COL SYSTEM * * * * * * * * * * * * * * * * * * * * * * * * */

	LUNARIS_DECL float collisionable::polygon_area(const std::vector<point_2d_ref>& lst) const {
		float area = 0.0f;
		size_t j = lst.size() - 1;
		for (size_t i = 0; i < lst.size(); i++) {
			area += (lst[j].px + lst[i].px) * (lst[j].py - lst[i].py);
			j = i;
		}
		return fabsf(area * 0.5f);
	}

	LUNARIS_DECL bool collisionable::polygon_collision_dot(const point_2d_ref &dot) const
	{
		if (points.size() <= 2) return false;

		const float maxarea = polygon_area(points);
		const auto* begg = points.data();

		float dotarea = 0.0f;
		size_t j = points.size() - 1;

		for (size_t p = 0; p < points.size(); ++p)
		{
			dotarea += polygon_area({
				point_2d_ref{dot.px, dot.py},
				point_2d_ref{begg[p].px,begg[p].py},
				point_2d_ref{begg[j].px,begg[j].py}
			});

			j = p;
		}

		return maxarea * default_collision_oversize_prop >= dotarea;
	}

	LUNARIS_DECL collisionable::point_2d collisionable::polygon_get_center() const
	{
		if (points.size() == 0) return {};

		point_2d pt;

		for (const auto& it : points)
		{
			pt.px += it.px * (1.0f / points.size());
			pt.py += it.py * (1.0f / points.size());
		}

		return pt;
	}

	LUNARIS_DECL void collisionable::polygon_get_min_max(point_2d& ma, point_2d& mi) const
	{
		if (points.size() == 0) return;
		ma.px = mi.px = points[0].px;
		ma.py = mi.py = points[0].py;
		for (size_t p = 1; p < points.size(); ++p) {
			const auto& it = points[p];
			if (it.px > ma.px) ma.px = it.px;
			if (it.px < mi.px) mi.px = it.px;
			if (it.py > ma.py) ma.py = it.py;
			if (it.py < mi.py) mi.py = it.py;
		}
	}

	LUNARIS_DECL bool collisionable::polygon_get_triangle_nearest_of(const point_2d& dot, point_2d& near1, point_2d& near2) const
	{
		if (points.size() < 2) return false;

		struct sort_it {
			size_t off;
			float dist = 0.0f;
		};
		std::vector<sort_it> vec;

		for (size_t p = 0; p < points.size(); ++p)
		{
			vec.push_back({
				p,
				powf((dot.px - points[p].px) * (dot.px - points[p].px), 2.0f) + powf((dot.py - points[p].py) * (dot.py - points[p].py), 2.0f)
				});
		}

		// sort by lowest
		std::sort(vec.begin(), vec.end(), [](const sort_it& a, const sort_it& b) { return a.dist < b.dist; });

		// copy 2 lower vals. Guaranteed 2 because of check before.
		near1.px = points[vec[0].off].px;
		near1.py = points[vec[0].off].py;
		near2.px = points[vec[1].off].px;
		near2.py = points[vec[1].off].py;

		return true;
	}

	/* * * * * * * * * * * * * * * * * * * * * * * * * FULL COL SYSTEM * * * * * * * * * * * * * * * * * * * * * * * * */


	LUNARIS_DECL collisionable::collisionable(sprite& s)
	{
		replace(s);
	}

	LUNARIS_DECL collisionable::collisionable(vertexes& v)
	{
		replace(v);
	}

	LUNARIS_DECL void collisionable::replace(sprite& s)
	{
		points.clear();
		points.push_back({ s.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X), s.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y) });
		points.push_back({ s.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X), s.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y) });
		points.push_back({ s.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X), s.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y) });
		points.push_back({ s.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X), s.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y) });
		sprite_opt = s;
		compensation = s.copy_transform_inuse();
	}

	LUNARIS_DECL void collisionable::replace(vertexes& v)
	{
		points.clear();
		sprite_opt.reset();
		v.safe([this](std::vector<vertex_point>& vec) {
			for (auto& it : vec) {
				points.push_back({ it.x, it.y });
			}
		});
	}

	LUNARIS_DECL void collisionable::reset(transform ntrasf)
	{
		results.clear();
		compensation = ntrasf;
	}

	LUNARIS_DECL void collisionable::reset()
	{
		if (!sprite_opt.has_value()) throw std::runtime_error("Can't do reset() with no parameter if sprite is not set.");
		results.clear();
		compensation = sprite_opt->get().copy_transform_inuse();
	}

	LUNARIS_DECL bool collisionable::collide(collisionable& oth)
	{
		point_2d th_limits[2]{};
		point_2d oh_limits[2]{};
		
		polygon_get_min_max(th_limits[1], th_limits[0]);
		oth.polygon_get_min_max(oh_limits[1], oh_limits[0]);

		// if one's max is less than min of other or vice versa then no col.
		if (th_limits[1].px < oh_limits[0].px || th_limits[0].px > oh_limits[1].px ||
			th_limits[1].py < oh_limits[0].py || th_limits[0].py > oh_limits[1].py)
			return false;


		bool did_col_once = false;
		auto objc = polygon_get_center();
		
		for (const auto& ea : points)
		{
			if (oth.polygon_collision_dot(ea))
			{
				did_col_once = true;
				point_2d nears[2];

				oth.polygon_get_triangle_nearest_of(objc, nears[0], nears[1]);

				const float m = (nears[1].py - nears[0].py) / (nears[1].px - (nears[0].px == nears[1].px ? (nears[0].px + 1) : nears[0].px));
				const float b = nears[0].py - (m * nears[0].px);

				const float x = (m * ea.py + ea.px - m * b) / (m * m + 1);
				const float y = (m * m * ea.py + m * ea.px + b) / (m * m + 1);

				each_result res, oth_res;

				res.has_collision = true;
				res.precise[0] = (x - ea.px) * 0.5f;
				res.precise[1] = (y - ea.py) * 0.5f;

				oth_res.has_collision = true;
				oth_res.precise[0] = (x - ea.px) * (-0.5f);
				oth_res.precise[1] = (y - ea.py) * (-0.5f);

				results.push_back(res);
				oth.results.push_back(oth_res);
			}
		}
		
		if (!did_col_once)
		{
			for (size_t p = 0; !did_col_once && p < (points.size() - 2); ++p)
			{
				for (size_t j = 0; !did_col_once && j < (oth.points.size() - 2); ++j) {
					did_col_once |= line_do_intersect(points[p], points[(p + 2) % points.size()], oth.points[j], oth.points[(j + 2) % oth.points.size()]);
				}
			}
		
			if (did_col_once)
			{
				auto objc_oth = oth.polygon_get_center();
		
				const float dx = (objc.px - objc_oth.px); // positive = objc is to the right, object to left
				const float dy = (objc.py - objc_oth.py); // positive = objc is down, object is up
		
				each_result res, oth_res;
				res.has_collision = true;
				oth_res.has_collision = true;
		
				if (fabsf(dx) < fabsf(dy)) // y
				{
					const float dymax = th_limits[1].py - th_limits[0].py;

					res.extended[1] = (dymax > dy ? (dymax - dy) : dy) * 0.25f;
					oth_res.extended[1] = (dymax > dy ? (dymax - dy) : dy) * (-0.25f);
				}
				else // x
				{
					const float dxmax = th_limits[1].px - th_limits[0].px;

					res.extended[0] = (dxmax > dx ? (dxmax - dx) : dx) * 0.25f;
					oth_res.extended[0] = (dxmax > dx ? (dxmax - dx) : dx) * (-0.25f);
				}
		
				//compensation.transform_coords(res.precise[0], res.precise[1]);
		
				results.push_back(res);
				oth.results.push_back(oth_res);
			}
		}

		return did_col_once;
	}

	LUNARIS_DECL void collisionable::set_work(std::function<void(final_result&, optional_sprite_ref&)> f)
	{
		workar = f;
	}

	LUNARIS_DECL void collisionable::work()
	{
		if (results.empty() || !workar) return;

		final_result fr;

		//printf_s("- Has collision [%zu]\n", results.size());

		for (const auto& it : results)
		{
			fr.fx += it.precise[0] * 1.0f / results.size();
			fr.fy += it.precise[1] * 1.0f / results.size();
		}

		if (fr.fx == 0.0f && fr.fy == 0.0f)
		{
			//printf_s("- Collision is not dot type\n");
			for (const auto& it : results)
			{
				fr.fx += it.extended[0] * 1.0f / results.size();
				fr.fy += it.extended[1] * 1.0f / results.size();
			}
		}

		if (fr.fx == 0.0f && fr.fy == 0.0f) {
			//printf_s("- Collision resulted in zero?!\n");
			return;
		}

		float comp[2] = { 1.0f, 1.0f };

		compensation.transform_coords(comp[0], comp[1]);

		fr.fx /= (fabsf(comp[0]) + 0.001f);
		fr.fy /= (fabsf(comp[1]) + 0.001f);

		workar(fr, sprite_opt);
	}


	LUNARIS_DECL float collisionable_legacy::get_size_x() const
	{
		return default_collision_oversize + (wrap.get<float>(enum_sprite_float_e::SCALE_G) * wrap.get<float>(enum_sprite_float_e::SCALE_X));
	}

	LUNARIS_DECL float collisionable_legacy::get_size_y() const
	{
		return default_collision_oversize + (wrap.get<float>(enum_sprite_float_e::SCALE_G) * wrap.get<float>(enum_sprite_float_e::SCALE_Y));
	}

	LUNARIS_DECL collisionable_legacy::collisionable_legacy(sprite& wrp)
		: wrap(wrp), posx(wrp.get<float>(enum_sprite_float_e::POS_X)), posy(wrp.get<float>(enum_sprite_float_e::POS_Y))
	{
	}

	LUNARIS_DECL bool collisionable_legacy::overlap(const collisionable_legacy& ol)
	{
		const float sizx = get_size_x();
		const float sizy = get_size_y();
		const float ol_sizx = ol.get_size_x();
		const float ol_sizy = ol.get_size_y();

		const float dist_x = (posx - ol.posx);
		const float diff_x = fabs(dist_x) - (sizx + ol_sizx) * 1.0f / 2.0f; // if < 0, col
		const bool col_x = diff_x < 0.0;
		const float dist_y = (posy - ol.posy);
		const float diff_y = fabs(dist_y) - (sizy + ol_sizy) * 1.0f / 2.0f; // if < 0, col
		const bool col_y = diff_y < 0.0;
		const bool is_col = col_y && col_x;

		if (is_col) {
			if (fabs(diff_x) < fabs(diff_y)) {
				if (fabs(diff_x) > fabs(dx_max)) dx_max = diff_x;

				if (dist_x > 0.0) {
					directions_cases[static_cast<int>(direction_internal::WEST)]++; // VEM DO WEST
				}
				else {
					directions_cases[static_cast<int>(direction_internal::EAST)]++; // VEM DO EAST
				}
			}
			else {
				if (fabs(diff_y) > fabs(dy_max)) dy_max = diff_y;
				if (dist_y > 0.0) {
					directions_cases[static_cast<int>(direction_internal::NORTH)]++; // VEM DO NORTH
				}
				else {
					directions_cases[static_cast<int>(direction_internal::SOUTH)]++; // VEM DO SOUTH
				}
			}
		}

		/*
		Y:
		P1 - P2 > 0 == col vem do NORTH
		P1 - P2 < 0 == col vem do SOUTH

		X:
		P1 - P2 > 0 == col vem do WEST
		P1 - P2 < 0 == col vem do EAST
		*/

		was_col |= is_col;
		return is_col;
	}

	LUNARIS_DECL int collisionable_legacy::result() const
	{
		const bool n = directions_cases[static_cast<int>(direction_internal::NORTH)] > 0;
		const bool s = directions_cases[static_cast<int>(direction_internal::SOUTH)] > 0;
		const bool w = directions_cases[static_cast<int>(direction_internal::WEST)] > 0;
		const bool e = directions_cases[static_cast<int>(direction_internal::EAST)] > 0;

		const int east = static_cast<int>(direction_op::DIR_EAST);
		const int west = static_cast<int>(direction_op::DIR_WEST);
		const int north = static_cast<int>(direction_op::DIR_NORTH);
		const int south = static_cast<int>(direction_op::DIR_SOUTH);


		if (n) { // north
			if (w && !e) {
				return south | east; // GOTO EAST
			}
			if (e && !w) {
				return south | west; // GOTO WEST
			}
			if (s) {
				if (directions_cases[static_cast<int>(direction_internal::NORTH)] > directions_cases[static_cast<int>(direction_internal::SOUTH)]) return south;
				if (directions_cases[static_cast<int>(direction_internal::SOUTH)] > directions_cases[static_cast<int>(direction_internal::NORTH)]) return north;
				return 0; // NO GOTO
			}
			return south; // GOTO SOUTH
		}
		else if (s) { //south
			if (w && !e) {
				return north | east; // GOTO EAST
			}
			if (e && !w) {
				return north | west; // GOTO WEST
			}
			return north; // GOTO NORTH
		}
		else if (w) { // west
			if (e) {
				if (directions_cases[static_cast<int>(direction_internal::EAST)] > directions_cases[static_cast<int>(direction_internal::WEST)]) return west;
				if (directions_cases[static_cast<int>(direction_internal::WEST)] > directions_cases[static_cast<int>(direction_internal::EAST)]) return east;
				return 0;
			}
			return east;
		}
		else if (e) { // east
			return west;
		}
		return 0;
	}

	LUNARIS_DECL void collisionable_legacy::reset()
	{
		was_col = false;
		for (auto& i : directions_cases) i = 0;
		dx_max = dy_max = 0.0;
	}

	LUNARIS_DECL void collisionable_legacy::work()
	{
		if (workar) {
			int res = this->result();
			workar(res, wrap);
		}
	}

	LUNARIS_DECL void collisionable_legacy::set_work(const std::function<void(int, sprite&)> f)
	{
		workar = f;
	}

	LUNARIS_DECL unsigned collisionable_legacy::read_cases(const direction_op transl) const
	{
		switch (transl) {
		case direction_op::DIR_NORTH:
			return directions_cases[static_cast<int>(direction_internal::NORTH)];
		case direction_op::DIR_SOUTH:
			return directions_cases[static_cast<int>(direction_internal::SOUTH)];
		case direction_op::DIR_EAST:
			return directions_cases[static_cast<int>(direction_internal::EAST)];
		case direction_op::DIR_WEST:
			return directions_cases[static_cast<int>(direction_internal::WEST)];
		default:
			return 0;
		}
	}

	//LUNARIS_DECL bool collisionable::result::is_dir(const direction_combo& op)
	//{
	//	return (dir_to & (static_cast<int>(op))) != 0;
	//}
	//
	//LUNARIS_DECL collisionable::each_result collisionable::each_pt_col(const float& px, const float& py, const collisionable& oth) const
	//{
	//	each_result _tmp;
	//
	//	float area[4];
	//
	//	// north
	//	{
	//		const float& ax = nwx;
	//		const float& ay = nwy;
	//		const float& bx = nex;
	//		const float& by = ney;
	//		area[static_cast<int>(direction_index::NORTH)] = 0.5f * (px * ay + py * bx + ax * by - ay * bx - px * by - py * ax);
	//	}
	//	// south
	//	{
	//		const float& ax = swx;
	//		const float& ay = swy;
	//		const float& bx = sex;
	//		const float& by = sey;
	//		area[static_cast<int>(direction_index::SOUTH)] = 0.5f * (px * ay + py * bx + ax * by - ay * bx - px * by - py * ax);
	//	}
	//	// east
	//	{
	//		const float& ax = nex;
	//		const float& ay = ney;
	//		const float& bx = sex;
	//		const float& by = sey;
	//		area[static_cast<int>(direction_index::EAST)] = 0.5f * (px * ay + py * bx + ax * by - ay * bx - px * by - py * ax);
	//	}
	//	// west
	//	{
	//		const float& ax = nwx;
	//		const float& ay = nwy;
	//		const float& bx = swx;
	//		const float& by = swy;
	//		area[static_cast<int>(direction_index::WEST)] = 0.5f * (px * ay + py * bx + ax * by - ay * bx - px * by - py * ax);
	//	}
	//
	//	float sum_areas = 0.0f;
	//	for (const auto& i : area) sum_areas += fabsf(i);
	//
	//	if (sum_areas <= (default_collision_oversize_prop * last_self_area)) {
	//
	//		float _quick_small = sum_areas;
	//		for (const auto& i : area) if (_quick_small > fabsf(i)) _quick_small = fabsf(i);
	//
	//		unsigned index = 0;
	//		for (; index < 4; index++) if (_quick_small == fabsf(area[index])) break;
	//		if (index == 4) return _tmp;
	//
	//		_tmp.one_direction = static_cast<direction_index>(index);
	//
	//		const float speed_dx = speedx - oth.speedx;
	//		const float speed_dy = speedy - oth.speedy;
	//
	//		if (fabsf(speed_dx) > fabsf(speed_dy)) {
	//			_tmp.moment_dir = oth.cy - cy;
	//		}
	//		else {
	//			_tmp.moment_dir = oth.cx - cx;
	//		}
	//	}
	//
	//	return _tmp;
	//}
	//
	//LUNARIS_DECL collisionable::result collisionable::combine_to(const collisionable& othr)
	//{
	//	each_result arr[4];
	//	result res;
	//
	//	arr[0] = each_pt_col(othr.nwx, othr.nwy, othr);
	//	arr[1] = each_pt_col(othr.nex, othr.ney, othr);
	//	arr[2] = each_pt_col(othr.swx, othr.swy, othr);
	//	arr[3] = each_pt_col(othr.sex, othr.sey, othr);
	//
	//	//arr[4] = othr.each_pt_col(nwx, nwy, *this);
	//	//arr[5] = othr.each_pt_col(nex, ney, *this);
	//	//arr[6] = othr.each_pt_col(swx, swy, *this);
	//	//arr[7] = othr.each_pt_col(sex, sey, *this);
	//
	//	//for (size_t p = 4; p < 8; p++) {
	//	//	arr[p].one_direction = fix_index_inverse(arr[p].one_direction);
	//	//}
	//
	//	unsigned opts[4] = { 0u,0u,0u,0u };
	//
	//	for (const auto& i : arr) { if (i.one_direction != direction_index::NONE) { ++opts[static_cast<int>(i.one_direction)]; res.moment_dir += i.moment_dir; } }
	//
	//	unsigned maxval = 0;
	//	for (const auto& i : opts) if (maxval < i) maxval = i; // find max val
	//	if (maxval == 0) return res;
	//
	//	for (size_t p = 0; p < 4; p++) {
	//		auto& i = opts[p];
	//		if (i == maxval) {
	//			switch (p) {
	//			case static_cast<int>(direction_index::NORTH):
	//				res.dir_to |= static_cast<int>(direction_combo::DIR_NORTH);
	//				break;
	//			case static_cast<int>(direction_index::SOUTH):
	//				res.dir_to |= static_cast<int>(direction_combo::DIR_SOUTH);
	//				break;
	//			case static_cast<int>(direction_index::EAST):
	//				res.dir_to |= static_cast<int>(direction_combo::DIR_EAST);
	//				break;
	//			case static_cast<int>(direction_index::WEST):
	//				res.dir_to |= static_cast<int>(direction_combo::DIR_WEST);
	//				break;
	//			}
	//		}
	//	}
	//
	//	return res;
	//}
	//
	//LUNARIS_DECL collisionable::direction_index collisionable::fix_index_rot(const direction_index fx) const
	//{
	//	if (fx == direction_index::_MAX || fx == direction_index::NONE) return fx;
	//	int rw = static_cast<int>(fx);
	//
	//	long long fixx = (static_cast<long long>(rot * 180.0f / static_cast<float>(ALLEGRO_PI)) % 360); // degrees
	//	if (fixx < 0) fixx += 360;
	//	fixx /= 45;
	//	switch (fixx) {
	//	case 7:
	//	case 0:
	//		return static_cast<direction_index>((rw) % 4);
	//	case 1:
	//	case 2:
	//		return static_cast<direction_index>((rw + 1) % 4);
	//	case 3:
	//	case 4:
	//		return static_cast<direction_index>((rw + 2) % 4);
	//	case 5:
	//	case 6:
	//		return static_cast<direction_index>((rw + 3) % 4);
	//	default:
	//		return static_cast<direction_index>((rw) % 4);
	//	}
	//}
	//
	//LUNARIS_DECL collisionable::direction_index collisionable::fix_index_inverse(const direction_index fx) const
	//{
	//	switch (fx) {
	//	case direction_index::NORTH:
	//		return direction_index::SOUTH;
	//	case direction_index::SOUTH:
	//		return direction_index::NORTH;
	//	case direction_index::WEST:
	//		return direction_index::EAST;
	//	case direction_index::EAST:
	//		return direction_index::WEST;
	//	default:
	//		return direction_index::NONE;
	//	}
	//}
	//
	//LUNARIS_DECL int collisionable::fix_op_rot_each(const direction_combo a)
	//{
	//	switch (a) {
	//	case direction_combo::DIR_NORTH:
	//		switch (fix_index_rot(direction_index::NORTH)) {
	//		case direction_index::NORTH:
	//			return static_cast<int>(direction_combo::DIR_NORTH);
	//		case direction_index::SOUTH:
	//			return static_cast<int>(direction_combo::DIR_SOUTH);
	//		case direction_index::EAST:
	//			return static_cast<int>(direction_combo::DIR_EAST);
	//		case direction_index::WEST:
	//			return static_cast<int>(direction_combo::DIR_WEST);
	//		default:
	//			return 0;
	//		}
	//	case direction_combo::DIR_SOUTH:
	//		switch (fix_index_rot(direction_index::SOUTH)) {
	//		case direction_index::NORTH:
	//			return static_cast<int>(direction_combo::DIR_NORTH);
	//		case direction_index::SOUTH:
	//			return static_cast<int>(direction_combo::DIR_SOUTH);
	//		case direction_index::EAST:
	//			return static_cast<int>(direction_combo::DIR_EAST);
	//		case direction_index::WEST:
	//			return static_cast<int>(direction_combo::DIR_WEST);
	//		default:
	//			return 0;
	//		}
	//	case direction_combo::DIR_EAST:
	//		switch (fix_index_rot(direction_index::EAST)) {
	//		case direction_index::NORTH:
	//			return static_cast<int>(direction_combo::DIR_NORTH);
	//		case direction_index::SOUTH:
	//			return static_cast<int>(direction_combo::DIR_SOUTH);
	//		case direction_index::EAST:
	//			return static_cast<int>(direction_combo::DIR_EAST);
	//		case direction_index::WEST:
	//			return static_cast<int>(direction_combo::DIR_WEST);
	//		default:
	//			return 0;
	//		}
	//	case direction_combo::DIR_WEST:
	//		switch (fix_index_rot(direction_index::WEST)) {
	//		case direction_index::NORTH:
	//			return static_cast<int>(direction_combo::DIR_NORTH);
	//		case direction_index::SOUTH:
	//			return static_cast<int>(direction_combo::DIR_SOUTH);
	//		case direction_index::EAST:
	//			return static_cast<int>(direction_combo::DIR_EAST);
	//		case direction_index::WEST:
	//			return static_cast<int>(direction_combo::DIR_WEST);
	//		default:
	//			return 0;
	//		}
	//	default:
	//		return 0;
	//	}
	//}
	//
	//LUNARIS_DECL int collisionable::fix_op_rot(const int o)
	//{
	//	int res = 0;
	//	if ((o & static_cast<int>(direction_combo::DIR_NORTH)) != 0) res |= fix_op_rot_each(direction_combo::DIR_NORTH);
	//	if ((o & static_cast<int>(direction_combo::DIR_SOUTH)) != 0) res |= fix_op_rot_each(direction_combo::DIR_SOUTH);
	//	if ((o & static_cast<int>(direction_combo::DIR_EAST)) != 0)  res |= fix_op_rot_each(direction_combo::DIR_EAST);
	//	if ((o & static_cast<int>(direction_combo::DIR_WEST)) != 0)  res |= fix_op_rot_each(direction_combo::DIR_WEST);
	//	return res;
	//}
	//
	//LUNARIS_DECL int collisionable::fix_op_invert(const int o)
	//{
	//	int res = 0;
	//	if ((o & static_cast<int>(direction_combo::DIR_NORTH)) != 0) res |= static_cast<int>(direction_combo::DIR_SOUTH);
	//	if ((o & static_cast<int>(direction_combo::DIR_SOUTH)) != 0) res |= static_cast<int>(direction_combo::DIR_NORTH);
	//	if ((o & static_cast<int>(direction_combo::DIR_EAST)) != 0)  res |= static_cast<int>(direction_combo::DIR_WEST);
	//	if ((o & static_cast<int>(direction_combo::DIR_WEST)) != 0)  res |= static_cast<int>(direction_combo::DIR_EAST);
	//	return res;
	//}
	//
	//LUNARIS_DECL collisionable::collisionable(sprite& ref)
	//	: wrap(ref),
	//	cx(ref.get<float>(enum_sprite_float_e::POS_X)),
	//	cy(ref.get<float>(enum_sprite_float_e::POS_Y)),
	//	nwx(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X)),
	//	nwy(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y)),
	//	nex(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X)),
	//	ney(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y)),
	//	swx(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X)),
	//	swy(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y)),
	//	sex(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X)),
	//	sey(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y)),
	//	speedx(ref.get<float>(enum_sprite_float_e::RO_THINK_SPEED_X)),
	//	speedy(ref.get<float>(enum_sprite_float_e::RO_THINK_SPEED_Y)),
	//	rot(ref.get<float>(enum_sprite_float_e::ROTATION))
	//{
	//}
	//
	//LUNARIS_DECL void collisionable::overlap(collisionable& oth)
	//{
	//	auto a = combine_to(oth);
	//	auto b = oth.combine_to(*this);		
	//
	//	if (a.dir_to != 0 || b.dir_to != 0) {
	//		result res;
	//		res.dir_to = fix_op_rot(a.dir_to) | fix_op_invert(oth.fix_op_rot(b.dir_to));
	//		res.moment_dir = a.moment_dir;
	//		//a.dir_to = fix_op_rot(a.dir_to);
	//		cases.push_back(res);
	//	}
	//}
	//
	//LUNARIS_DECL void collisionable::reset()
	//{
	//	cases.clear();
	//	last_self_area = (nwx * ney + nwy * swx + nex * swy - ney * swx - nwx * swy - nwy * nex);
	//}
	//
	//LUNARIS_DECL void collisionable::work()
	//{
	//	if (!workar) return;
	//	switch (cases.size()) {
	//	case 0:
	//		workar({}, wrap);
	//		return;
	//	case 1:
	//		workar(cases[0], wrap);
	//		return;
	//	default:
	//	{
	//		if (work_all) {
	//			for(auto& eac : cases) workar(eac, wrap);
	//		}
	//		else {
	//			size_t tmp = random() % cases.size();
	//			workar(cases[tmp], wrap);
	//		}
	//	}
	//		return;
	//	}
	//}
	//
	//LUNARIS_DECL void collisionable::set_work(const std::function<void(result, sprite&)> f)
	//{
	//	workar = f;
	//}
	//
	//LUNARIS_DECL collisionable::each_result collisionable::quick_one_point_overlap(const float px, const float py)
	//{
	//	return each_pt_col(px, py, *this);
	//}
	//
	//LUNARIS_DECL collisionable::result collisionable::quick_one_sprite_overlap(const collisionable& oth)
	//{
	//	return combine_to(oth);
	//}
	//
	//LUNARIS_DECL void collisionable::set_work_works_all_cases(const bool var)
	//{
	//	work_all = var;
	//}
	//
	
	LUNARIS_DECL void work_all_auto(collisionable* be, const collisionable* en)
	{
		if (be >= en) return;
	
		for (collisionable* ptr_p = be; ptr_p != en; ++ptr_p) ptr_p->reset();
		
		for (collisionable* ptr_a = be; ptr_a != en; ++ptr_a)
		{
			for (collisionable* ptr_b = be; ptr_b != en; ++ptr_b)
			{
				if (ptr_a != ptr_b) ptr_a->collide(*ptr_b);
			}
		}
	
		for (collisionable* ptr_p = be; ptr_p != en; ++ptr_p) ptr_p->work();
	}

}