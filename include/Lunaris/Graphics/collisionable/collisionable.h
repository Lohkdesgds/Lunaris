#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/sprite.h>
#include <Lunaris/Utility/random.h>

#include <functional>

namespace Lunaris {

	constexpr float default_collision_oversize = 1e-3;
	constexpr float default_collision_oversize_prop = 1.005f;

	// simple fixed (non rotation) reliable collision calculator.
	class collisionable_legacy {
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
		collisionable_legacy(sprite&);

		// calculate overlap with X
		bool overlap(const collisionable_legacy&);
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

	class collisionable {
	public:
		enum class direction_index { NONE = -1, NORTH, SOUTH, EAST, WEST, _MAX };
		enum class direction_combo { DIR_NONE = 0, DIR_NORTH = 1 << 0, DIR_SOUTH = 1 << 1, DIR_EAST = 1 << 2, DIR_WEST = 1 << 3 };
		struct result {
			float moment_dir = 0.0f;
			int dir_to = 0;

			bool is_dir(const direction_combo&);
		};
		struct each_result {
			float moment_dir = 0.0f;
			direction_index one_direction = direction_index::NONE;
		};
	private:
		std::vector<result> cases;

		sprite& wrap;
		std::function<void(result, sprite&)> workar;

		float last_self_area = 0.0f;
		bool work_all = true;
		const float &nwx, &nwy, &nex, &ney, &swx, &swy, &sex, &sey, &cx, &cy, &speedx, &speedy, &rot;

		each_result each_pt_col(const float&, const float&, const collisionable&) const;
		result combine_to(const collisionable&);

		direction_index fix_index_rot(const direction_index) const; // expects direction_index
		direction_index fix_index_inverse(const direction_index) const;
		int fix_op_rot_each(const direction_combo); // expects direction_combo unique
		int fix_op_rot(const int); // expects direction_combo combo
		int fix_op_invert(const int); // just north <-> south etc
	public:
		collisionable(sprite&);

		void overlap(collisionable&);
		void reset();
		void work();
		void set_work(const std::function<void(result, sprite&)>);

		// simple true/false point collision check
		each_result quick_one_point_overlap(const float, const float);
		// simple true/false sprite collision check
		result quick_one_sprite_overlap(const collisionable&);

		// if true, if there are more than 1 collision cases, work() will be called multiple times, else only once with a random option
		void set_work_works_all_cases(const bool);
	};

	// begin, end
	void work_all_auto(collisionable*, const collisionable*);


}