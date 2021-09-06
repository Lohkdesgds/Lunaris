#include "text.h"

namespace Lunaris {

	text_shadow::text_shadow(const double a, const double b, const color c)
		: offset_x(a), offset_y(b), clr(c)
	{
	}

	std::shared_lock<std::shared_mutex> text::mu_shared_read_control() const
	{
		return std::shared_lock<std::shared_mutex>(font_mtx);
	}

	std::unique_lock<std::shared_mutex> text::mu_shared_write_control() const
	{
		return std::unique_lock<std::shared_mutex>(font_mtx);
	}

	void text::draw_task(const transform& transf, const float& limit_x, const float& limit_y) // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! // NOT DONE! 
	{
		auto lock = mu_shared_read_control();

		if (font_used.empty()) return;

		const double& scale_g = get<double>(enum_sprite_double_e::SCALE_G);
		const double& scale_x = get<double>(enum_sprite_double_e::SCALE_X);
		const double& scale_y = get<double>(enum_sprite_double_e::SCALE_Y);
		const double& draw_line_height = get<double>(enum_text_double_e::DRAW_LINE_PROP);
		const double& draw_pos_x = get<double>(enum_sprite_double_e::RO_DRAW_PROJ_POS_X);
		const double& draw_pos_y = get<double>(enum_sprite_double_e::RO_DRAW_PROJ_POS_Y);
		const double& center_x = get<double>(enum_sprite_double_e::DRAW_RELATIVE_CENTER_X);
		const double& center_y = get<double>(enum_sprite_double_e::DRAW_RELATIVE_CENTER_Y);
		const std::string& to_str = get<std::string>(enum_text_string_e::STRING);
		const int& text_alignment = get<int>(enum_text_integer_e::DRAW_ALIGNMENT);
		const color& text_clr = get<color>(enum_sprite_color_e::DRAW_TINT);

		int height = font_used->get_line_height();
		if (height <= 0) throw std::runtime_error("Font said invalid height size!");

		transform current, transf_back;

		transf_back = transf; // later just apply back
		current = transf; // working here

		const double csx = scale_g * scale_x / height;
		const double csy = scale_g * scale_y / height;

		current.scale_inverse(1.0 / csx, 1.0 / csy);
		current.translate_inverse(-draw_pos_x / csx, -draw_pos_y / csy);


		/*const double const_scale_x = (scale_g * scale_x);
		const double const_scale_y = (scale_g * scale_y);

		current.translate_inverse(- draw_pos_x * const_scale_x * height, - draw_pos_y * const_scale_y * height);
		current.scale_inverse(static_cast<float>(1.0 * height / const_scale_x), static_cast<float>(1.0 * height / const_scale_y));*/
		current.apply();

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

	text::text() :
		sprite(),
		fixed_multi_map_work<static_cast<size_t>(enum_text_double_e::_SIZE), double, enum_text_double_e>(default_text_double_il),
		fixed_multi_map_work<static_cast<size_t>(enum_text_string_e::_SIZE), std::string, enum_text_string_e>(default_text_string_il),
		fixed_multi_map_work<static_cast<size_t>(enum_text_integer_e::_SIZE), int, enum_text_integer_e>(default_text_integer_il)
	{
	}

	void text::font_set(const hybrid_memory<font>& newfont)
	{
		auto lock = mu_shared_write_control();
		font_used = newfont;
	}

	void text::shadow_insert(text_shadow oth)
	{
		auto lock = mu_shared_write_control();
		shadows.push_back(oth);
	}

	const text_shadow& text::shadow_index(const size_t index) const
	{
		auto lock = mu_shared_read_control();
		if (index >= shadows.size()) throw std::out_of_range("index out of range");
		return shadows[index];
	}

	size_t text::shadow_size() const
	{
		return shadows.size();
	}

	void text::shadow_remove(const size_t index)
	{
		auto lock = mu_shared_write_control();
		if (index >= shadows.size()) throw std::out_of_range("index out of range");
		shadows.erase(shadows.begin() + index);
	}

	void text::shadow_remove_all()
	{
		auto lock = mu_shared_write_control();
		shadows.clear();
	}

}