#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/sprite.h>
#include <Lunaris/Graphics/transform.h>
#include <Lunaris/Graphics/font.h>
#include <Lunaris/Utility/safe_data.h>

#include <vector>
#include <shared_mutex>
#include <chrono>
#include <string>
#include <sstream>

#include <Lunaris/Utility/bomb.h>
#include <Lunaris/Graphics/texture.h>

namespace Lunaris {

	enum class enum_text_float_e {
		DRAW_LINE_PROP,				// [0.0,inf) 1.0 means font height for multiple lines
		DRAW_ALIGNMENT_PROP_Y,		// (-inf, inf) Usually between -1.0 and 1.0, offset based on font height. 0.5 centers the font
		DRAW_UPDATES_PER_SEC,		// [0.0, inf) 0.0 or lower disables the feature. Bigger than 0.0 enables texture drawing (less times drawing in a texture). Some features are not completely ported to this high performance mode!
		DRAW_RESOLUTION,			// (0.01, inf) Enabled if DRAW_UPDATES_PER_SEC is > 0.0. This value is the text buffer resolution compared to target. If 0.5, if target is 600x600, the text resolution will be 300x300

		_SIZE
	};

	enum class enum_text_safe_string_e {
		STRING,						// The actual string drawn

		_SIZE
	};

	enum class enum_text_integer_e {
		DRAW_ALIGNMENT,				// CENTER, LEFT or RIGHT?

		_SIZE
	};


	const std::initializer_list<multi_pair<float, enum_text_float_e>>									default_text_float_il = {
		// READONLY DATA
		{1.0,		enum_text_float_e::DRAW_LINE_PROP },
		{0.0,		enum_text_float_e::DRAW_ALIGNMENT_PROP_Y },
		{30.0,		enum_text_float_e::DRAW_UPDATES_PER_SEC },
		{1.0,		enum_text_float_e::DRAW_RESOLUTION }
	};

	const std::initializer_list<multi_pair<safe_data<std::string>, enum_text_safe_string_e>>			default_text_string_il = {
		// READONLY DATA
		{std::string(""),		enum_text_safe_string_e::STRING}
	};

	const std::initializer_list<multi_pair<int, enum_text_integer_e>>									default_text_integer_il = {
		// READONLY DATA
		{0,		enum_text_integer_e::DRAW_ALIGNMENT}
	};

	struct text_shadow {
		float offset_x = 0.0;
		float offset_y = 0.0;
		color clr;

		text_shadow() = default;
		text_shadow(const float, const float, const color);

		text_shadow& set_off_x(const float);
		text_shadow& set_off_y(const float);
		text_shadow& set_color(const color&);
	};


	class text :
		public sprite,
		public fixed_multi_map_work<static_cast<size_t>(enum_text_float_e::_SIZE), float, enum_text_float_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_text_safe_string_e::_SIZE), safe_data<std::string>, enum_text_safe_string_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>
	{
		struct _texture_mode {
			texture mapped;
			double last_draw = 0.0;
		};

		// variables
		mutable std::shared_mutex font_mtx; // used for font_used and shadows
		hybrid_memory<font> font_used;
		std::vector<text_shadow> shadows;
		std::unique_ptr<_texture_mode> if_texture;

		// funcs
		std::shared_lock<std::shared_mutex> mu_shared_read_control() const; // easier
		std::unique_lock<std::shared_mutex> mu_shared_write_control() const; // easier

	protected:
		void draw_task(transform, transform, const float&, const float&);

	public:
		using safe_string = safe_data<std::string>;

		text();

		void font_set(const hybrid_memory<font>&);

		void shadow_insert(text_shadow);
		const text_shadow& shadow_index(const size_t) const;
		size_t shadow_size() const;
		void shadow_remove(const size_t);
		void shadow_remove_all();

		// inheritance
		using sprite::set;
		using sprite::get;
		using sprite::index;
		using sprite::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_float_e::_SIZE), float, enum_text_float_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_float_e::_SIZE), float, enum_text_float_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_float_e::_SIZE), float, enum_text_float_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_float_e::_SIZE), float, enum_text_float_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_safe_string_e::_SIZE), text::safe_string, enum_text_safe_string_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_safe_string_e::_SIZE), text::safe_string, enum_text_safe_string_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_safe_string_e::_SIZE), text::safe_string, enum_text_safe_string_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_safe_string_e::_SIZE), text::safe_string, enum_text_safe_string_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>::size;
	};


}