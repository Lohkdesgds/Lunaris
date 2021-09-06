#pragma once

#include "../sprite.h"
#include "../transform.h"
#include "../font.h"

#include <vector>
#include <shared_mutex>
#include <chrono>
#include <string>
#include <sstream>

namespace Lunaris {

	enum class enum_text_double_e {
		DRAW_LINE_PROP,				// [0.0,inf) 1.0 means font height for multiple lines

		_SIZE
	};

	enum class enum_text_string_e {
		STRING,						// The actual string drawn

		_SIZE
	};

	enum class enum_text_integer_e {
		DRAW_ALIGNMENT,				// CENTER, LEFT or RIGHT?

		_SIZE
	};


	const std::initializer_list<multi_pair<double, enum_text_double_e>>									default_text_double_il = {
		// READONLY DATA
		{1.0,		enum_text_double_e::DRAW_LINE_PROP }
	};

	const std::initializer_list<multi_pair<std::string, enum_text_string_e>>							default_text_string_il = {
		// READONLY DATA
		{"",		enum_text_string_e::STRING}
	};

	const std::initializer_list<multi_pair<int, enum_text_integer_e>>									default_text_integer_il = {
		// READONLY DATA
		{0,		enum_text_integer_e::DRAW_ALIGNMENT}
	};

	struct text_shadow {
		double offset_x = 0.0;
		double offset_y = 0.0;
		color clr;

		text_shadow() = default;
		text_shadow(const double, const double, const color);
	};


	class text :
		public sprite,
		public fixed_multi_map_work<static_cast<size_t>(enum_text_double_e::_SIZE), double, enum_text_double_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_text_string_e::_SIZE), std::string, enum_text_string_e>,
		public fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>
	{
		// variables
		mutable std::shared_mutex font_mtx; // used for font_used and shadows
		hybrid_memory<font> font_used;
		std::vector<text_shadow> shadows;

		// funcs
		std::shared_lock<std::shared_mutex> mu_shared_read_control() const; // easier
		std::unique_lock<std::shared_mutex> mu_shared_write_control() const; // easier

	protected:
		void draw_task(const transform&, const float&, const float&);

	public:
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
		using fixed_multi_map_work<static_cast<size_t>(enum_text_double_e::_SIZE), double, enum_text_double_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_double_e::_SIZE), double, enum_text_double_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_double_e::_SIZE), double, enum_text_double_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_double_e::_SIZE), double, enum_text_double_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_string_e::_SIZE), std::string, enum_text_string_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_string_e::_SIZE), std::string, enum_text_string_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_string_e::_SIZE), std::string, enum_text_string_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_string_e::_SIZE), std::string, enum_text_string_e>::size;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>::set;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>::get;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>::index;
		using fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>::size;
	};


}