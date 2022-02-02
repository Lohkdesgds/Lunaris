#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/sprite.h>
#include <Lunaris/Graphics/vertex.h>
#include <Lunaris/Utility/random.h>
#include <Lunaris/Utility/mutex.h>

#include <functional>
#include <algorithm>
#include <optional>
#include <memory>

#undef max
#undef min

namespace Lunaris {

	constexpr float default_collision_oversize = 1e-3;
	constexpr float default_collision_oversize_prop = 1.005f;

	struct supported_fast_point_2d {
		float px, py;
	};

	bool line_on_segment(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
	int line_orientation(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
	bool line_do_intersect(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
	bool polygon_is_point_inside(const supported_fast_point_2d&, const std::vector<supported_fast_point_2d>&);

	/// <summary>
	/// <para>Classes derived from this will work with the fast collision algorithm.</para>
	/// <para>- The collision test is based on https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon </para>
	/// <para>- The revert algorithm was created by me. If collision happens, you should revert_once(), so based on movement it knows where it should go.</para>
	/// </summary>
	class supported_fast_collisionable {
	protected:
		std::vector<supported_fast_point_2d> generated_on_think; // commonly based on screen position (for easier "real" collision)
		float last_px = 0.0f; // related to center of object, generally [-1, 1] kind of coordinate.
		float last_py = 0.0f; // related to center of object, generally [-1, 1] kind of coordinate.
		float direction_x_revert = 0.0f; // old pos - actual pos, related to [-1, 1] kind of coordinate.
		float direction_y_revert = 0.0f; // old pos - actual pos, related to [-1, 1] kind of coordinate.
		bool last_was_collision = false;
		bool locked = false; // no movement?

		// Easy get index of vector (always valid, but vec_fit() first is recommended)
		supported_fast_point_2d& vec_get_at(const size_t);
		// Prepare generated_on_think to this size (internal resize if needed)
		void vec_fit(const size_t);
	public:
		/// <summary>
		/// <para>This tries to go back a step based on last movement.</para>
		/// </summary>
		virtual void revert_once() = 0;

		/// <summary>
		/// <para>This does think() (or equivalent) and check movement caused by that.</para>
		/// </summary>
		virtual void think_once() = 0;

		/// <summary>
		/// <para>Quick way to get all points stored inside. THIS IS NOT THREAD SAFE!</para>
		/// <para>This may change size and data while thinking/reverting/colliding</para>
		/// </summary>
		/// <returns>{vector} Reference to internal vector of current points (NOT THREAD SAFE).</returns>
		const std::vector<supported_fast_point_2d>& read_points() const;

		/// <summary>
		/// <para>Get current direction to go if collision happens. This is used alongside other algorithms for better behaviour.</para>
		/// </summary>
		/// <returns>{float} Movement in X.</returns>
		float get_dx() const;

		/// <summary>
		/// <para>Get current direction to go if collision happens. This is used alongside other algorithms for better behaviour.</para>
		/// </summary>
		/// <returns>{float} Movement in Y.</returns>
		float get_dy() const;

		/// <summary>
		/// <para>Automatically check collision, and if there's one, calculate expected behaviour and apply.</para>
		/// </summary>
		/// <param name="{supported_fast_collisionable&amp;}">Reference to another fast collisionable one.</param>
		void collide_auto(supported_fast_collisionable&);

		/// <summary>
		/// <para>Just check for collision with that. No smart stuff.</para>
		/// </summary>
		/// <param name="{supported_fast_collisionable&amp;}">Reference to another fast collisionable one.</param>
		/// <returns>{bool} True if collided.</returns>
		bool collide(const supported_fast_collisionable&) const;

		/// <summary>
		/// <para>Lock movement. If true, this is a static "wall".</para>
		/// </summary>
		/// <param name="{bool}">Lock position?</param>
		void lock(const bool);

		/// <summary>
		/// <para>Apply data acquired during collide_auto (if not locked).</para>
		/// </summary>
		void apply();
	};

	/// <summary>
	/// <para>Implementation of supported_fast_collisionable for sprites.</para>
	/// <para>This references a sprite and handle its collision.</para>
	/// </summary>
	class fast_collisionable_sprite : public supported_fast_collisionable {
		sprite& ref;
	public:
		fast_collisionable_sprite(sprite&);

		/// <summary>
		/// <para>This tries to go back a step based on last movement.</para>
		/// <para>As sprite, this also divides speed by 4 (for less bumpyness).</para>
		/// </summary>
		void revert_once();

		/// <summary>
		/// <para>This does think() (or equivalent) and check movement caused by that.</para>
		/// <para>As sprite, it think() and then calculate movement of the object.</para>
		/// </summary>
		void think_once(); // no collision, continue its work (think())
	};

	/// <summary>
	/// <para>Implementation of supported_fast_collisionable for vertexes.</para>
	/// <para>This references a vertexes and handle its collision.</para>
	/// </summary>
	class fast_collisionable_vertexes : public supported_fast_collisionable {
		vertexes& ref;

		float get_center_x() const;
		float get_center_y() const;
	public:
		fast_collisionable_vertexes(vertexes&);

		/// <summary>
		/// <para>This tries to go back a step based on last movement.</para>
		/// </summary>
		void revert_once();

		/// <summary>
		/// <para>This does think() (or equivalent) and check movement caused by that.</para>
		/// <para>As vertexes, this only calculates possible movement of object compared to last time you did think_once().</para>
		/// </summary>
		void think_once(); // no collision, continue its work (think())
	};

	/// <summary>
	/// <para>This is an easy-to-use class that integrates implemented variations of supported_fast_collisionable.</para>
	/// <para>The objects must follow the same rules to work properly.</para>
	/// </summary>
	class fast_collisionable_manager : public NonCopyable {
	protected:
		std::vector<std::unique_ptr<supported_fast_collisionable>> objs;
	public:
		/// <summary>
		/// <para>Add a reference to a sprite (automatically builds collision object inside referencing this).</para>
		/// </summary>
		/// <param name="{sprite&amp;}">Sprite to test collision.</param>
		/// <param name="{bool}">Lock this sprite movement (from this)?</param>
		void push_back(sprite&, const bool = false);

		/// <summary>
		/// <para>Add a reference to a vertexes (automatically builds collision object inside referencing this).</para>
		/// </summary>
		/// <param name="{vertexes&amp;}">Vertexes to test collision.</param>
		/// <param name="{bool}">Lock this sprite movement (from this)?</param>
		void push_back(vertexes&, const bool = false);

		/// <summary>
		/// <para>Think, test and apply all collisions once.</para>
		/// </summary>
		void think_all();
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
	/// <para>The reference MUST BE CONVEX.</para>
	/// </summary>
	//class collisionable : public NonCopyable {
	//public:
	//	using optional_sprite_ref = std::optional<std::reference_wrapper<sprite>>;
	//	using optional_vertexes_ref = std::optional<std::reference_wrapper<vertexes>>;
	//
	//	struct final_result {
	//		float fx = 0.0f;
	//		float fy = 0.0f;
	//		optional_sprite_ref spr;
	//		optional_vertexes_ref vex;
	//
	//		bool is_sprite() const;
	//		bool is_vertexes() const;
	//
	//		sprite& get_sprite();
	//		vertexes& get_vertexes();
	//	};
	//
	//	struct point_2d_ref {
	//		const float& px;
	//		const float& py;
	//	};
	//private:
	//	struct each_result {
	//		float precise[2] = { 0.0f,0.0f };
	//		float extended[2] = { 0.0f,0.0f };
	//		bool has_collision = false;
	//	};
	//	struct point_2d {
	//		float px = 0.0f;
	//		float py = 0.0f;
	//	};
	//
	//	// points is set on construction or replace()
	//	std::vector<point_2d_ref> points;
	//	std::vector<each_result> results;
	//	transform compensation;
	//	std::function<void(final_result&)> workar;
	//	mutable shared_recursive_mutex safe_mtx;
	//
	//	optional_sprite_ref sprite_opt;
	//	optional_vertexes_ref vertexes_opt;
	//
	//	// three commands below based on https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
	//
	//	bool line_on_segment(const point_2d_ref&, const point_2d_ref&, const point_2d_ref&) const;
	//	// 1 == clockwise, 2 == counterclockwise, 0 == points are collinear (same line)
	//	int line_orientation(const point_2d_ref&, const point_2d_ref&, const point_2d_ref&) const;
	//	// two lines, four points. Do they cross?
	//	bool line_do_intersect(const point_2d_ref&, const point_2d_ref&, const point_2d_ref&, const point_2d_ref&) const;
	//
	//	// calculates area of polygon, based on https://www.geeksforgeeks.org/area-of-a-polygon-with-given-n-ordered-vertices/
	//	float polygon_area(const std::vector<point_2d_ref>&) const;
	//
	//	// direct test of dot inside this "polygon"
	//	bool polygon_collision_dot(const point_2d_ref&) const;
	//	// sum of points, average X and Y, center, of this
	//	point_2d polygon_get_center() const;
	//	// get minimum and maximum X and Y values for smart collision detection
	//	void polygon_get_min_max(point_2d&, point_2d&) const;
	//	// get triangle of points near a point (from this "polygon")
	//	bool polygon_get_triangle_nearest_of(const point_2d&, point_2d&, point_2d&) const;
	//public:
	//	/// <summary>
	//	/// <para>Link a sprite to the collisionable thing.</para>
	//	/// </summary>
	//	/// <param name="{sprite&amp;}">Sprite reference.</param>
	//	collisionable(sprite&);
	//	
	//	/// <summary>
	//	/// <para>Link a vertexes to the collisionable thing.</para>
	//	/// </summary>
	//	/// <param name="{vertexes&amp;}">Vertexes reference.</param>
	//	collisionable(vertexes&);
	//
	//	/// <summary>
	//	/// <para>Re-setup this object to a new sprite reference.</para>
	//	/// </summary>
	//	/// <param name="{sprite&amp;}">Sprite reference.</param>
	//	void replace(sprite&);
	//	
	//	/// <summary>
	//	/// <para>Re-setup this object to a new vertexes reference.</para>
	//	/// </summary>
	//	/// <param name="{vertexes&amp;}">Vertexes reference.</param>
	//	void replace(vertexes&);
	//
	//	/// <summary>
	//	/// <para>Reset and copy current drawing matrix to a buffer (for further transformations with collisions).</para>
	//	/// </summary>
	//	void reset();
	//
	//	/// <summary>
	//	/// <para>Collide and store collision information (on both)</para>
	//	/// </summary>
	//	/// <param name="{collisionable&amp;}">Another collisionable.</param>
	//	/// <returns></returns>
	//	bool collide(collisionable&);
	//
	//	/// <summary>
	//	/// <para>Function to work() directly with the data after collision.</para>
	//	/// </summary>
	//	/// <param name="{function}">Function to handle information and do something.</param>
	//	void set_work(std::function<void(final_result&)>);
	//
	//	/// <summary>
	//	/// <para>Get all data and do something (with pre-defined work function).</para>
	//	/// </summary>
	//	void work();
	//
	//	/// <summary>
	//	/// <para>Is there a valid vertexes or sprite here?.</para>
	//	/// </summary>
	//	/// <returns>{bool} True if good.</returns>
	//	bool valid() const;
	//
	//	/// <summary>
	//	/// <para>Is there a valid vertexes or sprite here?.</para>
	//	/// </summary>
	//	/// <returns>{bool} True if not set.</returns>
	//	bool empty() const;
	//
	//	/// <summary>
	//	/// <para>Is this set with a vertexes?</para>
	//	/// </summary>
	//	/// <returns>{bool} True if this has a vertexes reference.</returns>
	//	bool is_vertexes() const;
	//
	//	/// <summary>
	//	/// <para>Is this set with a sprite?</para>
	//	/// </summary>
	//	/// <returns>{bool} True if this has a sprite reference.</returns>
	//	bool is_sprite() const;
	//
	//	/// <summary>
	//	/// <para>If correctly set at least once, this updates the points reference.</para>
	//	/// <para>If this is a sprite, you don't need this. Sprites are "fixed" in point amount, so once set, it's good.</para>
	//	/// <para>If this is a vertexes, you can use this to update the points. This is useful if the vertexes has more or less points right now.</para>
	//	/// </summary>
	//	void update_points();
	//
	//	/// <summary>
	//	/// <para>Read the vector points like a safe_vector.</para>
	//	/// </summary>
	//	/// <param name="{function}">A function that reads a vector of point_2d_ref.</param>
	//	void csafe(std::function<void(const std::vector<point_2d_ref>&)>) const;
	//};
	//
	///// <summary>
	///// <para>Work on many collisionable at once.</para>
	///// <para>This reset, collide and work all of them.</para>
	///// </summary>
	///// <param name="{collisionable*}">Begin.</param>
	///// <param name="{collisionable*}">End.</param>
	//void work_all_auto(collisionable*, const collisionable*);

}