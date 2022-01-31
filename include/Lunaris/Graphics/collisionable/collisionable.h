#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/sprite.h>
#include <Lunaris/Graphics/vertex.h>
#include <Lunaris/Utility/random.h>

#include <functional>
#include <algorithm>
#include <optional>

#undef max
#undef min

namespace Lunaris {

	constexpr float default_collision_oversize = 1e-3;
	constexpr float default_collision_oversize_prop = 1.005f;


	class collisionable : public NonCopyable {
	public:
		struct final_result {
			float fx = 0.0f;
			float fy = 0.0f;
		};

		using optional_sprite_ref = std::optional<std::reference_wrapper<sprite>>;
	private:
		struct each_result {
			float precise[2] = { 0.0f,0.0f };
			float extended[2] = { 0.0f,0.0f };
			bool has_collision = false;
		};
		struct point_2d_ref {
			const float& px;
			const float& py;
		};
		struct point_2d {
			float px = 0.0f;
			float py = 0.0f;
		};

		// points is set on construction or replace()
		std::vector<point_2d_ref> points;
		std::vector<each_result> results;
		transform compensation;
		std::function<void(final_result&, optional_sprite_ref&)> workar;
		optional_sprite_ref sprite_opt;

		bool line_on_segment(const point_2d_ref&, const point_2d_ref&, const point_2d_ref&) const;
		int line_orientation(const point_2d_ref&, const point_2d_ref&, const point_2d_ref&) const;
		bool line_do_intersect(const point_2d_ref&, const point_2d_ref&, const point_2d_ref&, const point_2d_ref&) const;

		float polygon_area(const std::vector<point_2d_ref>&) const;

		bool polygon_collision_dot(const point_2d_ref&) const;
		point_2d polygon_get_center() const;
		void polygon_get_min_max(point_2d&, point_2d&) const;
		bool polygon_get_triangle_nearest_of(const point_2d&, point_2d&, point_2d&) const;
	public:
		collisionable(sprite&);
		// implement:
		collisionable(vertexes&);

		void replace(sprite&);
		void replace(vertexes&);

		void reset(transform);

		// if you used a sprite, this is fine.
		void reset();

		bool collide(collisionable&);

		void set_work(std::function<void(final_result&, optional_sprite_ref&)>);

		void work();
	};


	/// <summary>
	/// <para>collisionable_legacy is the old collision system that only works with position and scale.</para>
	/// <para>This is still here because on a scenario with no rotation this works perfectly.</para>
	/// <para>It's legacy because I'm working on a way better collision system that works with rotation and any other transformation.</para>
	/// </summary>
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

		/// <summary>
		/// <para>Check and combine collision information with another object.</para>
		/// </summary>
		/// <param name="{collisionable_legacy}">Another collision_legacy object.</param>
		/// <returns></returns>
		bool overlap(const collisionable_legacy&);

		/// <summary>
		/// <para>Combine overlap information and give a direction_op result combo.</para>
		/// </summary>
		/// <returns>{int} direction_op combo of flags.</returns>
		int result() const;
		
		/// <summary>
		/// <para>Reset for new collision overlap combo.</para>
		/// </summary>
		void reset();

		/// <summary>
		/// <para>Automatically work as set on set_work.</para>
		/// </summary>
		void work();

		/// <summary>
		/// <para>Set a function to easily work() with the information in the end.</para>
		/// <para>The function arguments are the referenced sprite and result().</para>
		/// </summary>
		/// <param name="{function}">A function to handle collision information after the overlap() easily.</param>
		void set_work(const std::function<void(int, sprite&)>);

		/// <summary>
		/// <para>After all overlap(), this can tell you how many cases of each direction happened.</para>
		/// </summary>
		/// <param name="{direction_op}">The direction to check.</param>
		/// <returns>{unsigned} Number of cases.</returns>
		unsigned read_cases(const direction_op) const;
	};

	/// <summary>
	/// <para>collisionable is the way to check collision between sprites.</para>
	/// <para>This is not perfect yet, but the detection is very sharp.</para>
	/// </summary>
	//class collisionable {
	//public:
	//	enum class direction_index { NONE = -1, NORTH, SOUTH, EAST, WEST, _MAX };
	//	enum class direction_combo { DIR_NONE = 0, DIR_NORTH = 1 << 0, DIR_SOUTH = 1 << 1, DIR_EAST = 1 << 2, DIR_WEST = 1 << 3 };
	//	struct result {
	//		float moment_dir = 0.0f;
	//		int dir_to = 0;
	//
	//		/// <summary>
	//		/// <para>Test if dir_to has a direction_combo set.</para>
	//		/// </summary>
	//		/// <param name="{direction_combo}">A direction.</param>
	//		/// <returns>{bool} True if flag is set.</returns>
	//		bool is_dir(const direction_combo&);
	//	};
	//	struct each_result {
	//		float moment_dir = 0.0f;
	//		direction_index one_direction = direction_index::NONE;
	//	};
	//private:
	//	std::vector<result> cases;
	//
	//	sprite& wrap;
	//	std::function<void(result, sprite&)> workar;
	//
	//	float last_self_area = 0.0f;
	//	bool work_all = true;
	//	const float &nwx, &nwy, &nex, &ney, &swx, &swy, &sex, &sey, &cx, &cy, &speedx, &speedy, &rot;
	//
	//	each_result each_pt_col(const float&, const float&, const collisionable&) const;
	//	result combine_to(const collisionable&);
	//
	//	direction_index fix_index_rot(const direction_index) const; // expects direction_index
	//	direction_index fix_index_inverse(const direction_index) const;
	//	int fix_op_rot_each(const direction_combo); // expects direction_combo unique
	//	int fix_op_rot(const int); // expects direction_combo combo
	//	int fix_op_invert(const int); // just north <-> south etc
	//public:
	//	collisionable(sprite&);
	//
	//	/// <summary>
	//	/// <para>Test collision with another collision and push case into the list for further work().</para>
	//	/// </summary>
	//	/// <param name="{collisionable}">Another collisionable.</param>
	//	void overlap(collisionable&);
	//
	//	/// <summary>
	//	/// <para>Reset list of collided objects.</para>
	//	/// </summary>
	//	void reset();
	//
	//	/// <summary>
	//	/// <para>Run the work function with latest information.</para>
	//	/// </summary>
	//	void work();
	//
	//	/// <summary>
	//	/// <para>Set the working function to handle the collision information.</para>
	//	/// </summary>
	//	/// <param name="{function}">The function that does things with collision information.</param>
	//	void set_work(const std::function<void(result, sprite&)>);
	//
	//	/// <summary>
	//	/// <para>This can be used for a quick point collision check.</para>
	//	/// </summary>
	//	/// <param name="{float}">Position X.</param>
	//	/// <param name="{float}">Position Y.</param>
	//	/// <returns></returns>
	//	each_result quick_one_point_overlap(const float, const float);
	//	
	//	/// <summary>
	//	/// <para>This can be used for a quick collision check.</para>
	//	/// </summary>
	//	/// <param name="{collisionable}">Another collisionable object.</param>
	//	/// <returns>{result} The result of this collision.</returns>
	//	result quick_one_sprite_overlap(const collisionable&);
	//
	//	/// <summary>
	//	/// <para>Should it work all cases or only one max (if more than one collision case, get random one).</para>
	//	/// </summary>
	//	/// <param name="{bool}">Process every collision?</param>
	//	void set_work_works_all_cases(const bool);
	//};

	/// <summary>
	/// <para>Work on many collisionable at once.</para>
	/// </summary>
	/// <param name="{collisionable*}">Begin.</param>
	/// <param name="{collisionable*}">End.</param>
	void work_all_auto(collisionable*, const collisionable*);

}