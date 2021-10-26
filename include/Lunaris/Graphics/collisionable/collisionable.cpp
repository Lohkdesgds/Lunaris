#include "collisionable.h"

namespace Lunaris {

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

	LUNARIS_DECL bool collisionable::result::is_dir(const direction_combo& op)
	{
		return (dir_to & (static_cast<int>(op))) != 0;
	}

	LUNARIS_DECL collisionable::each_result collisionable::each_pt_col(const float& px, const float& py, const collisionable& oth) const
	{
		each_result _tmp;

		float area[4];

		// north
		{
			const float& ax = nwx;
			const float& ay = nwy;
			const float& bx = nex;
			const float& by = ney;
			area[static_cast<int>(direction_index::NORTH)] = 0.5f * (px * ay + py * bx + ax * by - ay * bx - px * by - py * ax);
		}
		// south
		{
			const float& ax = swx;
			const float& ay = swy;
			const float& bx = sex;
			const float& by = sey;
			area[static_cast<int>(direction_index::SOUTH)] = 0.5f * (px * ay + py * bx + ax * by - ay * bx - px * by - py * ax);
		}
		// east
		{
			const float& ax = nex;
			const float& ay = ney;
			const float& bx = sex;
			const float& by = sey;
			area[static_cast<int>(direction_index::EAST)] = 0.5f * (px * ay + py * bx + ax * by - ay * bx - px * by - py * ax);
		}
		// west
		{
			const float& ax = nwx;
			const float& ay = nwy;
			const float& bx = swx;
			const float& by = swy;
			area[static_cast<int>(direction_index::WEST)] = 0.5f * (px * ay + py * bx + ax * by - ay * bx - px * by - py * ax);
		}

		float sum_areas = 0.0f;
		for (const auto& i : area) sum_areas += fabsf(i);

		if (sum_areas <= (default_collision_oversize_prop * last_self_area)) {

			float _quick_small = sum_areas;
			for (const auto& i : area) if (_quick_small > fabsf(i)) _quick_small = fabsf(i);

			unsigned index = 0;
			for (; index < 4; index++) if (_quick_small == fabsf(area[index])) break;
			if (index == 4) return _tmp;

			_tmp.one_direction = static_cast<direction_index>(index);

			const float speed_dx = speedx - oth.speedx;
			const float speed_dy = speedy - oth.speedy;

			if (fabsf(speed_dx) > fabsf(speed_dy)) {
				_tmp.moment_dir = oth.cy - cy;
			}
			else {
				_tmp.moment_dir = oth.cx - cx;
			}
		}

		return _tmp;
	}

	LUNARIS_DECL collisionable::result collisionable::combine_to(const collisionable& othr)
	{
		each_result arr[4];
		result res;

		arr[0] = each_pt_col(othr.nwx, othr.nwy, othr);
		arr[1] = each_pt_col(othr.nex, othr.ney, othr);
		arr[2] = each_pt_col(othr.swx, othr.swy, othr);
		arr[3] = each_pt_col(othr.sex, othr.sey, othr);

		//arr[4] = othr.each_pt_col(nwx, nwy, *this);
		//arr[5] = othr.each_pt_col(nex, ney, *this);
		//arr[6] = othr.each_pt_col(swx, swy, *this);
		//arr[7] = othr.each_pt_col(sex, sey, *this);

		//for (size_t p = 4; p < 8; p++) {
		//	arr[p].one_direction = fix_index_inverse(arr[p].one_direction);
		//}

		unsigned opts[4] = { 0u,0u,0u,0u };

		for (const auto& i : arr) { if (i.one_direction != direction_index::NONE) { ++opts[static_cast<int>(i.one_direction)]; res.moment_dir += i.moment_dir; } }

		unsigned maxval = 0;
		for (const auto& i : opts) if (maxval < i) maxval = i; // find max val
		if (maxval == 0) return res;

		for (size_t p = 0; p < 4; p++) {
			auto& i = opts[p];
			if (i == maxval) {
				switch (p) {
				case static_cast<int>(direction_index::NORTH):
					res.dir_to |= static_cast<int>(direction_combo::DIR_NORTH);
					break;
				case static_cast<int>(direction_index::SOUTH):
					res.dir_to |= static_cast<int>(direction_combo::DIR_SOUTH);
					break;
				case static_cast<int>(direction_index::EAST):
					res.dir_to |= static_cast<int>(direction_combo::DIR_EAST);
					break;
				case static_cast<int>(direction_index::WEST):
					res.dir_to |= static_cast<int>(direction_combo::DIR_WEST);
					break;
				}
			}
		}

		return res;
	}

	LUNARIS_DECL collisionable::direction_index collisionable::fix_index_rot(const direction_index fx) const
	{
		if (fx == direction_index::_MAX || fx == direction_index::NONE) return fx;
		int rw = static_cast<int>(fx);

		long long fixx = (static_cast<long long>(rot * 180.0f / static_cast<float>(ALLEGRO_PI)) % 360); // degrees
		if (fixx < 0) fixx += 360;
		fixx /= 45;
		switch (fixx) {
		case 7:
		case 0:
			return static_cast<direction_index>((rw) % 4);
		case 1:
		case 2:
			return static_cast<direction_index>((rw + 1) % 4);
		case 3:
		case 4:
			return static_cast<direction_index>((rw + 2) % 4);
		case 5:
		case 6:
			return static_cast<direction_index>((rw + 3) % 4);
		default:
			return static_cast<direction_index>((rw) % 4);
		}
	}

	LUNARIS_DECL collisionable::direction_index collisionable::fix_index_inverse(const direction_index fx) const
	{
		switch (fx) {
		case direction_index::NORTH:
			return direction_index::SOUTH;
		case direction_index::SOUTH:
			return direction_index::NORTH;
		case direction_index::WEST:
			return direction_index::EAST;
		case direction_index::EAST:
			return direction_index::WEST;
		default:
			return direction_index::NONE;
		}
	}

	LUNARIS_DECL int collisionable::fix_op_rot_each(const direction_combo a)
	{
		switch (a) {
		case direction_combo::DIR_NORTH:
			switch (fix_index_rot(direction_index::NORTH)) {
			case direction_index::NORTH:
				return static_cast<int>(direction_combo::DIR_NORTH);
			case direction_index::SOUTH:
				return static_cast<int>(direction_combo::DIR_SOUTH);
			case direction_index::EAST:
				return static_cast<int>(direction_combo::DIR_EAST);
			case direction_index::WEST:
				return static_cast<int>(direction_combo::DIR_WEST);
			default:
				return 0;
			}
		case direction_combo::DIR_SOUTH:
			switch (fix_index_rot(direction_index::SOUTH)) {
			case direction_index::NORTH:
				return static_cast<int>(direction_combo::DIR_NORTH);
			case direction_index::SOUTH:
				return static_cast<int>(direction_combo::DIR_SOUTH);
			case direction_index::EAST:
				return static_cast<int>(direction_combo::DIR_EAST);
			case direction_index::WEST:
				return static_cast<int>(direction_combo::DIR_WEST);
			default:
				return 0;
			}
		case direction_combo::DIR_EAST:
			switch (fix_index_rot(direction_index::EAST)) {
			case direction_index::NORTH:
				return static_cast<int>(direction_combo::DIR_NORTH);
			case direction_index::SOUTH:
				return static_cast<int>(direction_combo::DIR_SOUTH);
			case direction_index::EAST:
				return static_cast<int>(direction_combo::DIR_EAST);
			case direction_index::WEST:
				return static_cast<int>(direction_combo::DIR_WEST);
			default:
				return 0;
			}
		case direction_combo::DIR_WEST:
			switch (fix_index_rot(direction_index::WEST)) {
			case direction_index::NORTH:
				return static_cast<int>(direction_combo::DIR_NORTH);
			case direction_index::SOUTH:
				return static_cast<int>(direction_combo::DIR_SOUTH);
			case direction_index::EAST:
				return static_cast<int>(direction_combo::DIR_EAST);
			case direction_index::WEST:
				return static_cast<int>(direction_combo::DIR_WEST);
			default:
				return 0;
			}
		default:
			return 0;
		}
	}

	LUNARIS_DECL int collisionable::fix_op_rot(const int o)
	{
		int res = 0;
		if ((o & static_cast<int>(direction_combo::DIR_NORTH)) != 0) res |= fix_op_rot_each(direction_combo::DIR_NORTH);
		if ((o & static_cast<int>(direction_combo::DIR_SOUTH)) != 0) res |= fix_op_rot_each(direction_combo::DIR_SOUTH);
		if ((o & static_cast<int>(direction_combo::DIR_EAST)) != 0)  res |= fix_op_rot_each(direction_combo::DIR_EAST);
		if ((o & static_cast<int>(direction_combo::DIR_WEST)) != 0)  res |= fix_op_rot_each(direction_combo::DIR_WEST);
		return res;
	}

	LUNARIS_DECL int collisionable::fix_op_invert(const int o)
	{
		int res = 0;
		if ((o & static_cast<int>(direction_combo::DIR_NORTH)) != 0) res |= static_cast<int>(direction_combo::DIR_SOUTH);
		if ((o & static_cast<int>(direction_combo::DIR_SOUTH)) != 0) res |= static_cast<int>(direction_combo::DIR_NORTH);
		if ((o & static_cast<int>(direction_combo::DIR_EAST)) != 0)  res |= static_cast<int>(direction_combo::DIR_WEST);
		if ((o & static_cast<int>(direction_combo::DIR_WEST)) != 0)  res |= static_cast<int>(direction_combo::DIR_EAST);
		return res;
	}

	LUNARIS_DECL collisionable::collisionable(sprite& ref)
		: wrap(ref),
		nwx(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X)),
		nwy(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y)),
		nex(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X)),
		ney(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y)),
		swx(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X)),
		swy(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y)),
		sex(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X)),
		sey(ref.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y)),
		cx(ref.get<float>(enum_sprite_float_e::POS_X)),
		cy(ref.get<float>(enum_sprite_float_e::POS_Y)),
		speedx(ref.get<float>(enum_sprite_float_e::RO_THINK_SPEED_X)),
		speedy(ref.get<float>(enum_sprite_float_e::RO_THINK_SPEED_Y)),
		rot(ref.get<float>(enum_sprite_float_e::ROTATION))
	{
	}

	LUNARIS_DECL void collisionable::overlap(collisionable& oth)
	{
		auto a = combine_to(oth);
		auto b = oth.combine_to(*this);		

		if (a.dir_to != 0 || b.dir_to != 0) {
			result res;
			res.dir_to = fix_op_rot(a.dir_to) | fix_op_invert(oth.fix_op_rot(b.dir_to));
			res.moment_dir = a.moment_dir;
			//a.dir_to = fix_op_rot(a.dir_to);
			cases.push_back(res);
		}
	}

	LUNARIS_DECL void collisionable::reset()
	{
		cases.clear();
		last_self_area = (nwx * ney + nwy * swx + nex * swy - ney * swx - nwx * swy - nwy * nex);
	}

	LUNARIS_DECL void collisionable::work()
	{
		if (!workar) return;
		switch (cases.size()) {
		case 0:
			workar({}, wrap);
			return;
		case 1:
			workar(cases[0], wrap);
			return;
		default:
		{
			if (work_all) {
				for(auto& eac : cases) workar(eac, wrap);
			}
			else {
				size_t tmp = random() % cases.size();
				workar(cases[tmp], wrap);
			}
		}
			return;
		}
	}

	LUNARIS_DECL void collisionable::set_work(const std::function<void(result, sprite&)> f)
	{
		workar = f;
	}

	LUNARIS_DECL collisionable::each_result collisionable::quick_one_point_overlap(const float px, const float py)
	{
		return each_pt_col(px, py, *this);
	}

	LUNARIS_DECL collisionable::result collisionable::quick_one_sprite_overlap(const collisionable& oth)
	{
		return combine_to(oth);
	}

	LUNARIS_DECL void collisionable::set_work_works_all_cases(const bool var)
	{
		work_all = var;
	}


	LUNARIS_DECL void work_all_auto(collisionable* be, const collisionable* en)
	{
		if (be >= en) return;

		for (collisionable* ptr_p = be; ptr_p != en; ++ptr_p) ptr_p->reset();
		
		for (collisionable* ptr_a = be; ptr_a != en; ++ptr_a)
		{
			for (collisionable* ptr_b = be; ptr_b != en; ++ptr_b)
			{
				if (ptr_a != ptr_b) ptr_a->overlap(*ptr_b);
			}
		}

		for (collisionable* ptr_p = be; ptr_p != en; ++ptr_p) ptr_p->work();
	}

}