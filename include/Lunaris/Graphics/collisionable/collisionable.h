#pragma once

#include <functional>

#include "../sprite/sprite.h"

namespace Lunaris {

	constexpr float default_collision_oversize = 1e-3;
	constexpr float default_collision_oversize_prop = 1.01f;

	// simple fixed (non rotation) reliable collision calculator.
	class collisionable {
	public:
		enum class direction_op { DIR_NONE = 0, DIR_NORTH = 1 << 0, DIR_SOUTH = 1 << 1, DIR_EAST = 1 << 2, DIR_WEST = 1 << 3 };
	private:
		enum class direction_internal { NORTH, SOUTH, EAST, WEST };

		sprite& wrap;
		std::function<void(int, sprite&)> workar;
		
		const float &posx, &posy;
		float dx_max = 0.0;
		float dy_max = 0.0;
		unsigned directions_cases[4] = { 0,0,0,0 };
		bool was_col = false;

		float get_size_x() const;
		float get_size_y() const;
	public:
		collisionable(sprite&);

		// calculate overlap with X
		bool overlap(const collisionable&);
		// get resulting combo of overlap (direction_op combo)
		int result() const;
		// reset for new overlapping calculation
		void reset();
		// automatic work (if you want to set a default response, do set_work(...)
		void work();
		// set a work function (useful for work() call only on collision thread). Receive result and sprite linked to this
		void set_work(const std::function<void(int, sprite&)>);

		unsigned read_cases(const direction_op) const;
	};

	class collisionable_v2 {
		enum class direction_internal { NORTH, SOUTH, EAST, WEST };
		enum class direction_corners_internal {NW, NE, SW, SE};

		sprite& wrap;
		std::function<void(int, sprite&)> workar;

		unsigned directions_cases[4] = { 0,0,0,0 };
		bool was_col = false;

		float last_self_area = 0.0f;
		const float &nwx, &nwy, &nex, &ney, &swx, &swy, &sex, &sey;
		//const float &propx, &propy, &propg; // scales

		void each_pt_col(const float&, const float&, const direction_corners_internal&);
	public:
		collisionable_v2(sprite&);

		bool overlap(const collisionable_v2&);
		int result() const;
		void reset();
		void work();
		void set_work(const std::function<void(int, sprite&)>);
	};
}