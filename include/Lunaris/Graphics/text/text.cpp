#include "text.h"

namespace Lunaris {

	LUNARIS_DECL text_shadow::text_shadow(const float a, const float b, const color c)
		: offset_x(a), offset_y(b), clr(c)
	{
	}

	LUNARIS_DECL std::shared_lock<std::shared_mutex> text::mu_shared_read_control() const
	{
		return std::shared_lock<std::shared_mutex>(font_mtx);
	}

	LUNARIS_DECL std::unique_lock<std::shared_mutex> text::mu_shared_write_control() const
	{
		return std::unique_lock<std::shared_mutex>(font_mtx);
	}

	LUNARIS_DECL void text::draw_task(transform transf, transform drawntransf, const float& limit_x, const float& limit_y) // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! 
	{
		auto lock = mu_shared_read_control();

		if (font_used.empty()) return;

		const float& scale_g = get<float>(enum_sprite_float_e::SCALE_G);
		const float& scale_x = get<float>(enum_sprite_float_e::SCALE_X);
		const float& scale_y = get<float>(enum_sprite_float_e::SCALE_Y);
		const float& draw_line_height = get<float>(enum_text_float_e::DRAW_LINE_PROP);
		const float& draw_pos_x = get<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_X);
		const float& draw_pos_y = get<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_Y);
		const float& center_x = get<float>(enum_sprite_float_e::DRAW_RELATIVE_CENTER_X);
		const float& center_y = get<float>(enum_sprite_float_e::DRAW_RELATIVE_CENTER_Y);
		const std::string to_str = get<safe_string>(enum_text_safe_string_e::STRING).read();
		const int& text_alignment = get<int>(enum_text_integer_e::DRAW_ALIGNMENT);
		const color& text_clr = get<color>(enum_sprite_color_e::DRAW_TINT);

		int height = font_used->get_line_height();
		if (height <= 0) throw std::runtime_error("Font said invalid height size!");

		transform current, transf_back;

		transf_back = transf; // later just apply back
		current = transf; // working here

		const float csx = scale_g * scale_x / height;
		const float csy = scale_g * scale_y / height;

		drawntransf.scale_inverse(1.0 / csx, 1.0 / csy);
		drawntransf.apply();

		//current.scale_inverse(1.0 / csx, 1.0 / csy);
		//current.translate_inverse(-draw_pos_x / csx, -draw_pos_y / csy);
		//
		//
		///*const float const_scale_x = (scale_g * scale_x);
		//const float const_scale_y = (scale_g * scale_y);
		//
		//current.translate_inverse(- draw_pos_x * const_scale_x * height, - draw_pos_y * const_scale_y * height);
		//current.scale_inverse(static_cast<float>(1.0 * height / const_scale_x), static_cast<float>(1.0 * height / const_scale_y));*/
		//current.apply();

		const auto text_len = font_used->get_width(to_str);

		std::stringstream ss(to_str);

		al_hold_bitmap_drawing(true); // may improve big text drawing

		{
			std::string _temp;
			size_t linecount_off = 0;

			while (std::getline(ss, _temp, '\n')) {

				for (const auto& i : shadows)
				{
					font_used->draw(
						i.clr,
						static_cast<float>(center_x + i.offset_x) * text_len, (static_cast<float>(center_y + i.offset_y) + static_cast<float>(linecount_off) * draw_line_height) * height,
						text_alignment,
						_temp);
				}

				font_used->draw(
					text_clr,
					static_cast<float>(center_x) * text_len, (static_cast<float>(center_y) + static_cast<float>(linecount_off) * draw_line_height) * height,
					text_alignment,
					_temp);

				linecount_off++;
			}
		}

		al_hold_bitmap_drawing(false);

		transf_back.apply();
	}					// NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! 

	LUNARIS_DECL text::text() :
		sprite(),
		fixed_multi_map_work<static_cast<size_t>(enum_text_float_e::_SIZE), float, enum_text_float_e>(default_text_float_il),
		fixed_multi_map_work<static_cast<size_t>(enum_text_safe_string_e::_SIZE), safe_string, enum_text_safe_string_e>(default_text_string_il),
		fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>(default_text_integer_il)
	{
	}

	LUNARIS_DECL void text::font_set(const hybrid_memory<font>& newfont)
	{
		auto lock = mu_shared_write_control();
		font_used = newfont;
	}

	LUNARIS_DECL void text::shadow_insert(text_shadow oth)
	{
		auto lock = mu_shared_write_control();
		shadows.push_back(oth);
	}

	LUNARIS_DECL const text_shadow& text::shadow_index(const size_t index) const
	{
		auto lock = mu_shared_read_control();
		if (index >= shadows.size()) throw std::out_of_range("index out of range");
		return shadows[index];
	}

	LUNARIS_DECL size_t text::shadow_size() const
	{
		return shadows.size();
	}

	LUNARIS_DECL void text::shadow_remove(const size_t index)
	{
		auto lock = mu_shared_write_control();
		if (index >= shadows.size()) throw std::out_of_range("index out of range");
		shadows.erase(shadows.begin() + index);
	}

	LUNARIS_DECL void text::shadow_remove_all()
	{
		auto lock = mu_shared_write_control();
		shadows.clear();
	}

}