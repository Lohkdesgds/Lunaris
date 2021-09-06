#pragma once

#include "../transform.h"
#include "../sprite.h"
#include "../texture.h"

#include <vector>
#include <shared_mutex>
#include <chrono>

namespace Lunaris {

	
	constexpr size_t block_max_frames_behind = 30;


	enum class enum_block_cmilliseconds_e {
		RO_DRAW_LAST_FRAME,				// last frame change (related to double FRAMES_PER_SECOND)

		_SIZE
	};

	enum class enum_block_sizet_e {
		RO_DRAW_FRAME,					// the frame selected right now

		_SIZE
	};

	enum class enum_block_bool_e {
		DRAW_SET_FRAME_VALUE_READONLY,	// lock frame

		_SIZE
	};

	enum class enum_block_double_e {
		// REFERENCE DATA (set by user)

		// * Specific for some "tasks"
		DRAW_FRAMES_PER_SECOND,			// smooth POS_X (real in-screen value)

		_SIZE
	};

	const std::initializer_list<multi_pair<size_t, enum_block_sizet_e>>										default_block_sizet_il = {
		// READONLY DATA
		{0,			enum_block_sizet_e::RO_DRAW_FRAME }
	};

	const std::initializer_list<multi_pair<std::chrono::milliseconds, enum_block_cmilliseconds_e>>			default_block_cmilliseconds_il = {
		// READONLY DATA
		{{},		enum_block_cmilliseconds_e::RO_DRAW_LAST_FRAME }
	};

	const std::initializer_list<multi_pair<bool, enum_block_bool_e>>										default_block_bool_il = {
		// READONLY DATA
		{false,		enum_block_bool_e::DRAW_SET_FRAME_VALUE_READONLY }
	};

	const std::initializer_list<multi_pair<double, enum_block_double_e>>									default_block_double_il = {
		// READONLY DATA
		{0.0,		enum_block_double_e::DRAW_FRAMES_PER_SECOND }
	};


	class block :
		public sprite,
		public fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE),					size_t,							enum_block_sizet_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_block_cmilliseconds_e::_SIZE),			std::chrono::milliseconds,		enum_block_cmilliseconds_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_block_bool_e::_SIZE),					bool,							enum_block_bool_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_block_double_e::_SIZE),				double,							enum_block_double_e>
	{
		// assist structs decl
		/*struct _block_drawing_references_fast {
			const double& RO_DRAW_PROJ_POS_X; // smooth POS_X (real in-screen value)
			const double& RO_DRAW_PROJ_POS_Y; // smooth POS_Y (real in-screen value)
			const double& RO_DRAW_PROJ_ROTATION; // smooth ROTATION (real in-screen value)
			const double& RO_DRAW_LAST_DRAW; // last draw time (double, in seconds)

			const double& DRAW_RELATIVE_CENTER_X; // generally [-1.0, 1.0] = When rotating, where should the anchor be? (relative to sprite size)
			const double& DRAW_RELATIVE_CENTER_Y; // generally [-1.0, 1.0] = When rotating, where should the anchor be? (relative to sprite size)
			const double& DRAW_ELASTIC_POSITION_PROP; // (0.0, 1.0] = How smooth position update should be. Near 0 means smoother (also means very out of date because of that)

			const double& POS_X; // Target position to be (may change if collision)
			const double& POS_Y; // Target position to be (may chance if collision)
			const double& ROTATION;	// Target rotation to be (may change if collision)
			const double& SCALE_G; // Scale (general)
			const double& SCALE_X; // Scale (horizontal)
			const double& SCALE_Y; // Scale (vertical)
			const double& OUT_OF_SIGHT_POS; // [0.0, inf) = What coords (any direction) should be considered "too far" so it should not attempt to draw itself?

			_block_drawing_references_fast() = default;
			_block_drawing_references_fast(const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&, const double&);
		};*/

		// variables
		mutable std::shared_mutex textures_mtx;
		std::vector<hybrid_memory<texture>> textures;
		//_block_drawing_references_fast _draw_fast;

		// funcs
		std::shared_lock<std::shared_mutex> mu_shared_read_control() const; // easier
		std::unique_lock<std::shared_mutex> mu_shared_write_control() const; // easier

		void generic_draw_no_lock(const texture&); // uses internal properties

	protected:
		void draw_task(const transform&, const float&, const float&);

	public:
		block();

		void texture_insert(const hybrid_memory<texture>&);
		const hybrid_memory<texture>& texture_index(const size_t) const;
		size_t texture_size() const;
		void texture_remove(const size_t);
		void texture_remove_all();

		// inheritance
		using sprite::set;
		using sprite::get;
		using sprite::index;
		using sprite::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE), size_t, enum_block_sizet_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE), size_t, enum_block_sizet_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE), size_t, enum_block_sizet_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE), size_t, enum_block_sizet_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_cmilliseconds_e::_SIZE), std::chrono::milliseconds, enum_block_cmilliseconds_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_cmilliseconds_e::_SIZE), std::chrono::milliseconds, enum_block_cmilliseconds_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_cmilliseconds_e::_SIZE), std::chrono::milliseconds, enum_block_cmilliseconds_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_cmilliseconds_e::_SIZE), std::chrono::milliseconds, enum_block_cmilliseconds_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_bool_e::_SIZE), bool, enum_block_bool_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_bool_e::_SIZE), bool, enum_block_bool_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_bool_e::_SIZE), bool, enum_block_bool_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_bool_e::_SIZE), bool, enum_block_bool_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_double_e::_SIZE), double, enum_block_double_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_double_e::_SIZE), double, enum_block_double_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_double_e::_SIZE), double, enum_block_double_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_block_double_e::_SIZE), double, enum_block_double_e>::size;
	};

}