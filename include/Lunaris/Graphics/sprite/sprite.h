#pragma once

#include "../../Utility/multi_map.h"
#include "../../Utility/memory.h"
#include "../transform.h"
#include "../color.h"

namespace Lunaris {

	/*
	* How is this structure?
	* 1. All "stored" data have their ID (enum) // and if they're "readonly" or not.
	* 2. "readonly" variables also start as RO_##
	* 3. Generally names goes like (RO_)(TASKTYPE_)NAME... (example: RO_DRAW_SOMETHING = Read Only, Draw task, "Something")
	*/

	enum class enum_sprite_float_e {
		// READONLY DATA
		RO_DRAW_PROJ_POS_X,				// smooth POS_X (real in-screen value)									-- IN_USE / READY
		RO_DRAW_PROJ_POS_Y,				// smooth POS_Y (real in-screen value)									-- IN_USE / READY
		RO_DRAW_PROJ_ROTATION,			// smooth ROTATION (real in-screen value) (in rad)						-- IN_USE / READY
		RO_THINK_SPEED_X,				// accel makes speed. Speed is smooth.									-- IN_USE
		RO_THINK_SPEED_Y,				// accel makes speed. Speed is smooth.									-- IN_USE / READY
		RO_THINK_POINT_NORTHWEST_X,		// Automatically calculates pixel-like positions for further collision
		RO_THINK_POINT_NORTHWEST_Y,		// Automatically calculates pixel-like positions for further collision
		RO_THINK_POINT_NORTHEAST_X,		// Automatically calculates pixel-like positions for further collision
		RO_THINK_POINT_NORTHEAST_Y,		// Automatically calculates pixel-like positions for further collision
		RO_THINK_POINT_SOUTHWEST_X,		// Automatically calculates pixel-like positions for further collision
		RO_THINK_POINT_SOUTHWEST_Y,		// Automatically calculates pixel-like positions for further collision
		RO_THINK_POINT_SOUTHEAST_X,		// Automatically calculates pixel-like positions for further collision
		RO_THINK_POINT_SOUTHEAST_Y,		// Automatically calculates pixel-like positions for further collision

		// REFERENCE DATA (set by user)

		// * Specific for some "tasks"
		//DRAW_ELASTIC_POSITION_PROP,			// (0.0, 1.0] = How smooth position update should be. Near 0 means smoother (also means very out of date because of that)
		DRAW_RELATIVE_CENTER_X,				// generally [-1.0, 1.0] = When rotating, where should the anchor be? (relative to sprite size)
		DRAW_RELATIVE_CENTER_Y,				// generally [-1.0, 1.0] = When rotating, where should the anchor be? (relative to sprite size)
		DRAW_MOVEMENT_RESPONSIVENESS,		// (0.0, inf) = how fast should it "update" the screen pos compared to thinking pos? 1.0 is "smooth", ~1 tick frame behind (fairly smooth). Default is 1.25. Bigger than 1 is more accurate, but less smooth. Do not use 0.0 or less.
		THINK_ELASTIC_SPEED_PROP,			// (0.0, 1.0] = Resistance on speed (per tick). With an accel for a while, speed builds up. If accel is 0, speed is *= this per tick. (also known as ELASTICITY)

		// * Used by all "tasks"
		POS_X,								// Target position to be (may change if collision or acceleration)
		POS_Y,								// Target position to be (may chance if collision or acceleration)
		ROTATION,							// Target rotation to be (may change if collision or acceleration)
		ACCEL_X,							// Acceleration in X (affects internal SPEED and affect POS_X)		-- IN_USE
		ACCEL_Y,							// Acceleration in Y (affects internal SPEED and affect POS_Y)		-- IN_USE
		SCALE_G,							// Scale (general)													-- IN_USE
		SCALE_X,							// Scale (horizontal)												-- IN_USE
		SCALE_Y,							// Scale (vertical)													-- IN_USE
		OUT_OF_SIGHT_POS,					// [0.0, inf) = What coords (any direction) should be considered "too far" so it should not attempt to draw itself? Default: 2.0 (distant from screen)
		_SIZE								// For easy sizing
	};

	enum class enum_sprite_double_e {
		// READONLY DATA
		RO_THINK_LAST_THINK,			// last think time (double, in seconds)									-- IN_USE / READY
		RO_THINK_DELTA_THINKS,			// (last think time) - (one before that)								-- IN_USE / READY
		RO_DRAW_LAST_DRAW,				// last draw time (double, in seconds)									-- IN_USE

		_SIZE
	};

	enum class enum_sprite_boolean_e {
		// READONLY DATA
		RO_DRAW_DRAWN_LAST_DRAW,			// if OUT_OF_SIGHT or DRAW_SHOULD_DRAW is false, this is false, else true.

		// REFERENCE DATA (set by user)

		// * Specific for some "tasks"
		DRAW_SHOULD_DRAW,					// if it should draw at all (skips draw())
		DRAW_USE_COLOR,						// use color set to tint currrent drawing?
		DRAW_DRAW_BOX,						// draw colored box where images would be if any (not effective when there are things drawn after)
		DRAW_THINK_BOX,						// draw colored box where thinking position thinks position is right now
		DRAW_TRANSFORM_COORDS_KEEP_SCALE,	// the position is modified so if e.g. POSX goes from -3 to 3, a -1 POSX is transformed to -3. This may break collision visually. This affects RO_DRAW_PROJ_POS_### stuff. TODO: Collision should *= last camera transformation aka -3,3 stuff

		_SIZE
	};

	enum class enum_sprite_color_e {
		DRAW_TINT,
		DRAW_DRAW_BOX,
		DRAW_THINK_BOX,

		_SIZE
	};



	const std::initializer_list<multi_pair<float, enum_sprite_float_e>>			default_sprite_float_il = {
		// READONLY DATA
		{0.0f,		enum_sprite_float_e::RO_DRAW_PROJ_POS_X},
		{0.0f,		enum_sprite_float_e::RO_DRAW_PROJ_POS_Y},
		{0.0f,		enum_sprite_float_e::RO_DRAW_PROJ_ROTATION},
		{0.0f,		enum_sprite_float_e::RO_THINK_SPEED_X},
		{0.0f,		enum_sprite_float_e::RO_THINK_SPEED_Y},
		{0.0f,		enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X},
		{0.0f,		enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y},
		{0.0f,		enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X},
		{0.0f,		enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y},
		{0.0f,		enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X},
		{0.0f,		enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y},
		{0.0f,		enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X},
		{0.0f,		enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y},

		// REFERENCE DATA (set by user)
		{0.0f,		enum_sprite_float_e::DRAW_RELATIVE_CENTER_X},
		{0.0f,		enum_sprite_float_e::DRAW_RELATIVE_CENTER_Y},
		{1.25f,		enum_sprite_float_e::DRAW_MOVEMENT_RESPONSIVENESS},
		{0.9f,		enum_sprite_float_e::THINK_ELASTIC_SPEED_PROP},
		{0.0f,		enum_sprite_float_e::POS_X},
		{0.0f,		enum_sprite_float_e::POS_Y},
		{0.0f,		enum_sprite_float_e::ROTATION},
		{0.0f,		enum_sprite_float_e::ACCEL_X},
		{0.0f,		enum_sprite_float_e::ACCEL_Y},
		{1.0f,		enum_sprite_float_e::SCALE_G},
		{1.0f,		enum_sprite_float_e::SCALE_X},
		{1.0f,		enum_sprite_float_e::SCALE_Y},
		{2.0f,		enum_sprite_float_e::OUT_OF_SIGHT_POS}
	};

	const std::initializer_list<multi_pair<double, enum_sprite_double_e>>			default_sprite_double_il = {
		// READONLY DATA
		{0.0,		enum_sprite_double_e::RO_THINK_LAST_THINK},
		{1.0/20,	enum_sprite_double_e::RO_THINK_DELTA_THINKS},
		{0.0,		enum_sprite_double_e::RO_DRAW_LAST_DRAW}
	};

	const std::initializer_list<multi_pair<bool, enum_sprite_boolean_e>>			default_sprite_boolean_il = {
		// READONLY DATA
		{false,		enum_sprite_boolean_e::RO_DRAW_DRAWN_LAST_DRAW},
		// REFERENCE DATA (set by user)
		{true,		enum_sprite_boolean_e::DRAW_SHOULD_DRAW},
		{false,		enum_sprite_boolean_e::DRAW_USE_COLOR},
		{false,		enum_sprite_boolean_e::DRAW_DRAW_BOX},
		{false,		enum_sprite_boolean_e::DRAW_THINK_BOX},
		{false,		enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE}
	};

	const std::initializer_list<multi_pair<color, enum_sprite_color_e>>				default_sprite_color_il = {
		{color(255,255,255,255),		enum_sprite_color_e::DRAW_TINT},
		{color(190,190, 85,130),		enum_sprite_color_e::DRAW_DRAW_BOX},
		{color( 85, 85,190,130),		enum_sprite_color_e::DRAW_THINK_BOX}
	};


	class sprite :
		public fixed_multi_map_work<static_cast<size_t>(enum_sprite_float_e::_SIZE), float, enum_sprite_float_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_sprite_double_e::_SIZE), double, enum_sprite_double_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_sprite_boolean_e::_SIZE), bool, enum_sprite_boolean_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_sprite_color_e::_SIZE), color, enum_sprite_color_e>
	{
		transform m_assist_transform{}; // it doesn't need to be created every time, and it can be shared between collision and drawing threads

		float get_real_pos_x(const bool = false); // if DRAW_TRANSFORM_COORDS_KEEP_SCALE, it's the resulting pos. Bool: refresh m_assist_transform?
		float get_real_pos_y(const bool = false); // if DRAW_TRANSFORM_COORDS_KEEP_SCALE, it's the resulting pos
	protected:
		// raw transform, adapted transform, limit_x, limit_y
		virtual void draw_task(transform, transform, const float&, const float&) {}
		virtual void think_task() {}
	public:
		sprite();

		void draw();
		void think();

		//int collide(const sprite&) const;

		// inheritance
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_float_e::_SIZE), float, enum_sprite_float_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_float_e::_SIZE), float, enum_sprite_float_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_float_e::_SIZE), float, enum_sprite_float_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_float_e::_SIZE), float, enum_sprite_float_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_double_e::_SIZE), double, enum_sprite_double_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_double_e::_SIZE), double, enum_sprite_double_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_double_e::_SIZE), double, enum_sprite_double_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_double_e::_SIZE), double, enum_sprite_double_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_boolean_e::_SIZE), bool, enum_sprite_boolean_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_boolean_e::_SIZE), bool, enum_sprite_boolean_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_boolean_e::_SIZE), bool, enum_sprite_boolean_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_boolean_e::_SIZE), bool, enum_sprite_boolean_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_color_e::_SIZE), color, enum_sprite_color_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_color_e::_SIZE), color, enum_sprite_color_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_color_e::_SIZE), color, enum_sprite_color_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_sprite_color_e::_SIZE), color, enum_sprite_color_e>::size;
	};



}

/*
inline void test() { // a way to get every key in key
	auto& ref = self<float>();
	auto it = ref.find(block_float_e::SPEED_X);

	it->keys == false; // test bool part lmao
}
*/