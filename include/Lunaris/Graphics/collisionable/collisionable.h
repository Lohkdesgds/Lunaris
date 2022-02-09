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

/*
NOTES:
- Factors. Add ENUM specifications
- Rename Collisionable things to a smaller name lmao
*/

namespace Lunaris {

	constexpr float default_collision_oversize = 1e-3;
	constexpr float default_collision_oversize_prop = 1.005f;

	struct supported_fast_point_2d {
		float px, py;
	};

	// all, not used yet
	enum class enum_fast_collisionable_float_e {
		CORRECTION_FACTOR, // [0.0, inf) (default: 1.000001f) Proportion fix for movement. 1.0 means value calculated is applied 1:1. Less means less move fix.
		MINIMUM_FIX_DELTA, // [0.0, inf) (default: 0.0003f) Minimum movement value.
		FIX_DELTA_CENTER_PROP, // [0.0, 1.0] (default: 0.0000001f) Center delta multiplier
		ALT_DIRECTION_PROP, // [0.0, 1.0] (default: 0.1f) Percentage applied to the other axis, like, if code thinks thing should go up/down, 20% of the value will go like left/right
		PROPORTION_MOVE_ON_COLLISION, // [0.0, inf) (default: 0.49f) Amount sliced to objects on collision. 0.49f is near 0.5f that means 1/2 for each side.

		SIZE
	};

	// sprite only, not used yet
	enum class enum_fast_collisionable_sprite_float_e {
		REFLECTIVENESS, // [0.0, 1.0) 244 & 255 lines @ fast_collisionable_sprite::revert_once() (SPEED MULTIPLIER)

		_SIZE
	};


	/// <summary>
	/// <para>Classes derived from this will work with the fast collision algorithm.</para>
	/// <para>- The collision test is based on https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon </para>
	/// <para>- The revert algorithm was created by me. If collision happens, you should revert_once(), so based on movement it knows where it should go.</para>
	/// </summary>
	class supported_fast_collisionable {
	protected:
		static bool line_on_segment(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
		static int line_orientation(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
		static bool line_do_intersect(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
		static bool polygon_is_point_inside(const supported_fast_point_2d&, const std::vector<supported_fast_point_2d>&);

		std::vector<supported_fast_point_2d> generated_on_think; // commonly based on screen position (for easier "real" collision)
		float last_px = 0.0f; // related to center of object, generally [-1, 1] kind of coordinate.
		float last_py = 0.0f; // related to center of object, generally [-1, 1] kind of coordinate.
		float direction_x_revert = 0.0f; // old pos - actual pos, related to [-1, 1] kind of coordinate.
		float direction_y_revert = 0.0f; // old pos - actual pos, related to [-1, 1] kind of coordinate.
		float direction_x_final = 0.0f;
		float direction_y_final = 0.0f;
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
		/// <para>Just check for collision with that. No smart stuff. Won't save in memory.</para>
		/// </summary>
		/// <param name="{supported_fast_collisionable&amp;}">Reference to another fast collisionable one.</param>
		/// <returns>{bool} True if collided.</returns>
		bool collide_test(const supported_fast_collisionable&) const;

		/// <summary>
		/// <para>Just check for collision with a point. Won't save in memory.</para>
		/// </summary>
		/// <param name="{float}">Point X.</param>
		/// <param name="{float}">Point Y.</param>
		/// <returns>{bool} True if collide.</returns>
		bool collide_test(const float&, const float&) const;

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

		std::vector<std::unique_ptr<supported_fast_collisionable>>& get();

		/// <summary>
		/// <para>Think, test and apply all collisions once.</para>
		/// </summary>
		void think_all();
	};
}