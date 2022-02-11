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
#include <shared_mutex>

#undef max
#undef min

/*
NOTES:
- Factors. Add ENUM specifications
- Rename Collisionable things to a smaller name lmao
*/

namespace Lunaris {

	struct supported_fast_point_2d {
		float px, py;
	};

	// all, not used yet
	enum class enum_collisionable_float_e {
		// READONLY DATA
		RO_LAST_PX,							// Last position in X saved in memory for further collision testing
		RO_LAST_PY,							// Last position in Y saved in memory for further collision testing
		RO_DIRECTION_X_REVERT,				// Last calculated revert movement based on last position
		RO_DIRECTION_Y_REVERT,				// Last calculated revert movement based on last position
		RO_DIRECTION_X_FINAL,				// Final value calculated to apply on itself when there's collision
		RO_DIRECTION_Y_FINAL,				// Final value calculated to apply on itself when there's collision

		// REFERENCE DATA (set by user)
		CORRECTION_FACTOR,					// [0.0, inf) (default: 1.000001f) Proportion fix for movement. 1.0 means value calculated is applied 1:1. Less means less move fix.
		MINIMUM_FIX_DELTA,					// [0.0, inf) (default: 0.0003f) Minimum movement value.
		FIX_DELTA_CENTER_PROP,				// [0.0, 1.0] (default: 0.000005f) Center delta multiplier
		ALT_DIRECTION_PROP,					// [0.0, 1.0] (default: 0.02f) Percentage applied to the other axis, like, if code thinks thing should go up/down, 20% of the value will go like left/right
		PROPORTION_MOVE_ON_COLLISION,		// [0.0, inf) (default: 0.49f) Amount sliced to objects on collision. 0.49f is near 0.5f that means 1/2 for each side.

		_SIZE
	};

	enum class enum_collisionable_boolean_e {
		// READONLY DATA
		RO_LAST_WAS_COLLISION,				// True if last check was a collision.

		// REFERENCE DATA (set by user)
		LOCKED,								// (default: false) Lock any changes by collision in position?

		_SIZE
	};

	// sprite only, not used yet
	enum class enum_collisionable_sprite_float_e {
		// REFERENCE DATA (set by user)
		REFLECTIVENESS,						// [0.0, 1.0) (default: 0.5f) How much of the speed is converted back to the diretion it should go? Multiple collision tests result in lower result

		_SIZE
	};


	const std::initializer_list<multi_pair<float, enum_collisionable_float_e>>			default_collisionable_float_il = {
		// READONLY DATA
		{0.0f,			enum_collisionable_float_e::RO_LAST_PX},
		{0.0f,			enum_collisionable_float_e::RO_LAST_PY},
		{0.0f,			enum_collisionable_float_e::RO_DIRECTION_X_REVERT},
		{0.0f,			enum_collisionable_float_e::RO_DIRECTION_Y_REVERT},
		{0.0f,			enum_collisionable_float_e::RO_DIRECTION_X_FINAL},
		{0.0f,			enum_collisionable_float_e::RO_DIRECTION_Y_FINAL},

		// REFERENCE DATA (set by user)
		{1.000001f,		enum_collisionable_float_e::CORRECTION_FACTOR},
		{0.0003f,		enum_collisionable_float_e::MINIMUM_FIX_DELTA},
		{0.0001f,		enum_collisionable_float_e::FIX_DELTA_CENTER_PROP},
		{0.02f,			enum_collisionable_float_e::ALT_DIRECTION_PROP},
		{0.49f,			enum_collisionable_float_e::PROPORTION_MOVE_ON_COLLISION}
	};

	const std::initializer_list<multi_pair<bool, enum_collisionable_boolean_e>>		default_collisionable_boolean_il = {
		// REFERENCE DATA (set by user)
		{false,			enum_collisionable_boolean_e::RO_LAST_WAS_COLLISION},

		// REFERENCE DATA (set by user)
		{false,			enum_collisionable_boolean_e::LOCKED}
	};

	const std::initializer_list<multi_pair<float, enum_collisionable_sprite_float_e>>	default_collisionable_sprite_float_il = {
		// REFERENCE DATA (set by user)
		{0.5f,			enum_collisionable_sprite_float_e::REFLECTIVENESS}
	};



	/// <summary>
	/// <para>Classes derived from this will work with the fast collision algorithm.</para>
	/// <para>- The collision test is based on https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon </para>
	/// <para>- The revert algorithm was created by me. If collision happens, you should revert_once(), so based on movement it knows where it should go.</para>
	/// <para>Note: it may not be thread-safe. This is aimed to be configured and applied to a manager or directly from it.</para>
	/// </summary>
	class collisionable_base :
		public fixed_multi_map_work<static_cast<size_t>(enum_collisionable_float_e::_SIZE), float, enum_collisionable_float_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_collisionable_boolean_e::_SIZE), bool, enum_collisionable_boolean_e>
	{
	protected:
		static bool line_on_segment(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
		static int line_orientation(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
		static bool line_do_intersect(const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&, const supported_fast_point_2d&);
		static bool polygon_is_point_inside(const supported_fast_point_2d&, const std::vector<supported_fast_point_2d>&);

		std::vector<supported_fast_point_2d> generated_on_think; // commonly based on screen position (for easier "real" collision)
		std::function<void(collisionable_base*)> on_collision_do;

		// Easy get index of vector (always valid, but vec_fit() first is recommended)
		supported_fast_point_2d& vec_get_at(const size_t);
		// Prepare generated_on_think to this size (internal resize if needed)
		void vec_fit(const size_t);
	public:
		collisionable_base();

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
		/// <para>Automatically check collision, and if there's one, calculate expected behaviour and apply.</para>
		/// </summary>
		/// <param name="{collisionable_base&amp;}">Reference to another fast collisionable one.</param>
		void collide_auto(collisionable_base&);

		/// <summary>
		/// <para>Just check for collision with that. No smart stuff. Won't save in memory.</para>
		/// </summary>
		/// <param name="{collisionable_base&amp;}">Reference to another fast collisionable one.</param>
		/// <returns>{bool} True if collided.</returns>
		bool collide_test(const collisionable_base&) const;

		/// <summary>
		/// <para>Just check for collision with a point. Won't save in memory.</para>
		/// </summary>
		/// <param name="{float}">Point X.</param>
		/// <param name="{float}">Point Y.</param>
		/// <returns>{bool} True if collide.</returns>
		bool collide_test(const float&, const float&) const;

		/// <summary>
		/// <para>If on apply() there was a collision, this is called.</para>
		/// </summary>
		void set_run_on_collision(std::function<void(collisionable_base*)>);

		/// <summary>
		/// <para>Reset any function set to run on collision.</para>
		/// </summary>
		void unset_run_on_collision();

		/// <summary>
		/// <para>Apply data acquired during collide_auto (if not locked).</para>
		/// </summary>
		void apply();

		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_float_e::_SIZE), float, enum_collisionable_float_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_float_e::_SIZE), float, enum_collisionable_float_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_float_e::_SIZE), float, enum_collisionable_float_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_float_e::_SIZE), float, enum_collisionable_float_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_boolean_e::_SIZE), bool, enum_collisionable_boolean_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_boolean_e::_SIZE), bool, enum_collisionable_boolean_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_boolean_e::_SIZE), bool, enum_collisionable_boolean_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_boolean_e::_SIZE), bool, enum_collisionable_boolean_e>::size;
	};

	/// <summary>
	/// <para>Implementation of collisionable_base for sprites.</para>
	/// <para>This references a sprite and handle its collision.</para>
	/// </summary>
	class collisionable_sprite : 
		public collisionable_base,
		public fixed_multi_map_work<static_cast<size_t>(enum_collisionable_sprite_float_e::_SIZE), float, enum_collisionable_sprite_float_e>
	{
		sprite& ref;
	public:
		collisionable_sprite(sprite&);

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

		using collisionable_base::set;
		using collisionable_base::get;
		using collisionable_base::index;
		using collisionable_base::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_sprite_float_e::_SIZE), float, enum_collisionable_sprite_float_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_sprite_float_e::_SIZE), float, enum_collisionable_sprite_float_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_sprite_float_e::_SIZE), float, enum_collisionable_sprite_float_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_collisionable_sprite_float_e::_SIZE), float, enum_collisionable_sprite_float_e>::size;
	};

	/// <summary>
	/// <para>Implementation of collisionable_base for vertexes.</para>
	/// <para>This references a vertexes and handle its collision.</para>
	/// </summary>
	class collisionable_vertexes : public collisionable_base {
		vertexes& ref;

		float get_center_x() const;
		float get_center_y() const;
	public:
		collisionable_vertexes(vertexes&);

		/// <summary>
		/// <para>This tries to go back a step based on last movement.</para>
		/// </summary>
		void revert_once();

		/// <summary>
		/// <para>This does think() (or equivalent) and check movement caused by that.</para>
		/// <para>As vertexes, this only calculates possible movement of object compared to last time you did think_once().</para>
		/// </summary>
		void think_once(); // no collision, continue its work (think())

		using collisionable_base::set;
		using collisionable_base::get;
		using collisionable_base::index;
		using collisionable_base::size;
	};

	/// <summary>
	/// <para>This is an easy-to-use class that integrates implemented variations of collisionable_base.</para>
	/// <para>The objects must follow the same rules to work properly.</para>
	/// </summary>
	class collisionable_manager : public NonCopyable {
	protected:
		std::vector<std::unique_ptr<collisionable_base>> objs;
		mutable std::shared_mutex objs_safe;
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
		/// <para>Add a reference to a sprite (automatically builds collision object inside referencing this).</para>
		/// </summary>
		/// <param name="{sprite&amp;}">Sprite to test collision.</param>
		/// <param name="{std::function}">Do you want to setup something before the real push? Use this.</param>
		/// <param name="{bool}">Lock this sprite movement (from this)?</param>
		void push_back(sprite&, std::function<void(collisionable_sprite*)>, const bool = false);

		/// <summary>
		/// <para>Add a reference to a vertexes (automatically builds collision object inside referencing this).</para>
		/// </summary>
		/// <param name="{vertexes&amp;}">Vertexes to test collision.</param>
		/// <param name="{std::function}">Do you want to setup something before the real push? Use this.</param>
		/// <param name="{bool}">Lock this sprite movement (from this)?</param>
		void push_back(vertexes&, std::function<void(collisionable_vertexes*)>, const bool = false);

		/// <summary>
		/// <para>The amount of collisionables in here.</para>
		/// </summary>
		/// <returns>{size_t} Vector size.</returns>
		size_t size() const;

		/// <summary>
		/// <para>It is considered valid if there's at least 2 collisionables set.</para>
		/// </summary>
		/// <returns>{bool} True if 2 or more set.</returns>
		bool valid() const;

		/// <summary>
		/// <para>It is considered empty if there's no collisionable set.</para>
		/// </summary>
		/// <returns>{bool} True if no collisionables pushed yet.</returns>
		bool empty() const;

		/// <summary>
		/// <para>Access internal vector safely.</para>
		/// </summary>
		/// <param name="{function}">A function that handles a vector of unique ptr of collisionables.</param>
		void safe(std::function<void(std::vector<std::unique_ptr<collisionable_base>>&)>);

		/// <summary>
		/// <para>Access internal vector safely.</para>
		/// </summary>
		/// <param name="{function}">A function that handles a vector of unique ptr of collisionables.</param>
		void csafe(std::function<void(const std::vector<std::unique_ptr<collisionable_base>>&)>) const;

		/// <summary>
		/// <para>Think, test and apply all collisions once.</para>
		/// </summary>
		void think_all();
	};


}