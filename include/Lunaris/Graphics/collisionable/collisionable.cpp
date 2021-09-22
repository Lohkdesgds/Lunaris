#include "collisionable.h"

namespace Lunaris {

	float collisionable::get_size_x() const
	{
		return default_collision_oversize + (wrap.get<float>(enum_sprite_float_e::SCALE_G) * wrap.get<float>(enum_sprite_float_e::SCALE_X));
	}

	float collisionable::get_size_y() const
	{
		return default_collision_oversize + (wrap.get<float>(enum_sprite_float_e::SCALE_G) * wrap.get<float>(enum_sprite_float_e::SCALE_Y));
	}

	collisionable::collisionable(sprite& wrp)
		: wrap(wrp), posx(wrp.get<float>(enum_sprite_float_e::POS_X)), posy(wrp.get<float>(enum_sprite_float_e::POS_Y))
	{
	}

	bool collisionable::overlap(const collisionable& ol)
	{
		const float sizx = get_size_x();
		const float sizy = get_size_y();
		const float ol_sizx = ol.get_size_x();
		const float ol_sizy = ol.get_size_y();

		const float dist_x = (posx - ol.posx);
		const float diff_x = fabs(dist_x) - (sizx + ol_sizx) * 1.0 / 2.0; // if < 0, col
		const bool col_x = diff_x < 0.0;
		const float dist_y = (posy - ol.posy);
		const float diff_y = fabs(dist_y) - (sizy + ol_sizy) * 1.0 / 2.0; // if < 0, col
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

	int collisionable::result() const
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

	void collisionable::reset()
	{
		was_col = false;
		for (auto& i : directions_cases) i = 0;
		dx_max = dy_max = 0.0;
	}

	void collisionable::work()
	{
		if (workar) {
			int res = this->result();
			workar(res, wrap);
		}
	}

	void collisionable::set_work(const std::function<void(int, sprite&)> f)
	{
		workar = f;
	}

	unsigned collisionable::read_cases(const direction_op transl) const
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

}