#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/transform.h>
#include <Lunaris/Graphics/sprite.h>
#include <Lunaris/Graphics/texture.h>

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

	/// <summary>
	/// <para>block is a sprite with texture and extra animation features.</para>
	/// </summary>
	class block :
		public sprite,
		public fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE),					size_t,							enum_block_sizet_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_block_cmilliseconds_e::_SIZE),			std::chrono::milliseconds,		enum_block_cmilliseconds_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_block_bool_e::_SIZE),					bool,							enum_block_bool_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_block_double_e::_SIZE),				double,							enum_block_double_e>
	{
		// variables
		mutable std::shared_mutex textures_mtx;
		std::vector<hybrid_memory<texture>> textures;
		//_block_drawing_references_fast _draw_fast;

		// funcs
		std::shared_lock<std::shared_mutex> mu_shared_read_control() const; // easier
		std::unique_lock<std::shared_mutex> mu_shared_write_control() const; // easier

		void generic_draw_no_lock(const texture&); // uses internal properties

	protected:
		void draw_task(transform, transform, const float&, const float&);

	public:
		block();

		/// <summary>
		/// <para>Add a new texture to the list (you can animate or select via set).</para>
		/// </summary>
		/// <param name="{hybrid_memory&lt;texture&gt;}">A texture.</param>
		void texture_insert(const hybrid_memory<texture>&);

		/// <summary>
		/// <para>Get one of the internal textures </para>
		/// </summary>
		/// <param name="{size_t}">Index value.</param>
		/// <returns>{hybrid_memory&lt;texture&gt;} The texture, or empty if out of range.</returns>
		hybrid_memory<texture> texture_index(const size_t) const;

		/// <summary>
		/// <para>How many textures are in this object?</para>
		/// </summary>
		/// <returns>{size_t} The amount of textures added in this object.</returns>
		size_t texture_size() const;

		/// <summary>
		/// <para>Remove one of the textures from this object.</para>
		/// </summary>
		/// <param name="{size_t}">Index value.</param>
		void texture_remove(const size_t);

		/// <summary>
		/// <para>Remove all textures from this object.</para>
		/// </summary>
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