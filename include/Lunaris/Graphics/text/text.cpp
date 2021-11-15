#include "text.h"

namespace Lunaris {

	LUNARIS_DECL text_shadow::text_shadow(const float a, const float b, const color c)
		: offset_x(a), offset_y(b), clr(c)
	{
	}

	LUNARIS_DECL text_shadow& text_shadow::set_off_x(const float var)
	{
		offset_x = var;
		return *this;
	}

	LUNARIS_DECL text_shadow& text_shadow::set_off_y(const float var)
	{
		offset_y = var;
		return *this;
	}

	LUNARIS_DECL text_shadow& text_shadow::set_color(const color& var)
	{
		clr = var;
		return *this;
	}

	LUNARIS_DECL std::shared_lock<std::shared_mutex> text::mu_shared_read_control() const
	{
		return std::shared_lock<std::shared_mutex>(font_mtx);
	}

	LUNARIS_DECL std::unique_lock<std::shared_mutex> text::mu_shared_write_control() const
	{
		return std::unique_lock<std::shared_mutex>(font_mtx);
	}

	LUNARIS_DECL void text::draw_task(transform transf, transform drawntransf, const float& limit_x, const float& limit_y)
	{
		const float& draws_per_sec_textured = get<float>(enum_text_float_e::DRAW_UPDATES_PER_SEC);
		const float& resolution_prop = get<float>(enum_text_float_e::DRAW_RESOLUTION);
		float real_prop = resolution_prop < 0.01f ? 0.01 : resolution_prop;

		auto* old_targ = al_get_target_bitmap();
		bool need_draw = draws_per_sec_textured <= 0.0f;

		if (draws_per_sec_textured > 0.0f) {
			if (!if_texture) if_texture = std::make_unique<_texture_mode>();			

			const double rn = al_get_time();

			if (rn - if_texture->last_draw < 1.0f / draws_per_sec_textured || if_texture->last_draw == 0.0)
			{
				if (if_texture->last_draw == 0.0) if_texture->last_draw = rn;


				transform transf2;
				transf2.identity();
				transf2.apply();

				ALLEGRO_BITMAP* ref = al_get_target_bitmap();
				if (!ref) throw std::runtime_error("No display?!");

				const int cxx = (al_get_bitmap_width(ref) * resolution_prop) < 32 ? 32 : (al_get_bitmap_width(ref) * resolution_prop);
				const int cyy = (al_get_bitmap_height(ref) * resolution_prop) < 32 ? 32 : (al_get_bitmap_height(ref) * resolution_prop);

				if (if_texture->mapped.empty() || (if_texture->mapped.get_width() != cxx) || (if_texture->mapped.get_height() != cyy))
				{
					if_texture->mapped.create(cxx, cyy);
					need_draw = true;
				}
				else {
					if_texture->mapped.draw_scaled_at(0.0f, 0.0f, al_get_bitmap_width(ref), al_get_bitmap_height(ref));
					transf.apply();
					need_draw = false;
				}
			}
			else {
				need_draw = true;
				if_texture->last_draw = rn;
			}
		}
		else {
			if (if_texture) if_texture.reset();
			real_prop = 1.0f;
		}

		if (!need_draw) return;
		else if (draws_per_sec_textured > 0.0f && if_texture && !if_texture->mapped.empty()) {
			if_texture->mapped.set_as_target();
			color(0.0f, 0.0f, 0.0f, 0.0f).clear_to_this();
		}

		auto lock = mu_shared_read_control();
		if (font_used.empty()) return;

		bomb disable_halt_auto([transf, old_targ] {al_hold_bitmap_drawing(false); al_set_target_bitmap(old_targ); transf.apply(); });
		al_hold_bitmap_drawing(true); // may improve big text drawing

		const float& scale_g = get<float>(enum_sprite_float_e::SCALE_G);
		const float& scale_x = get<float>(enum_sprite_float_e::SCALE_X);
		const float& scale_y = get<float>(enum_sprite_float_e::SCALE_Y);
		const float& draw_line_height = get<float>(enum_text_float_e::DRAW_LINE_PROP);
		const float& line_offset = get<float>(enum_text_float_e::DRAW_ALIGNMENT_PROP_Y);
		const float& draw_pos_x = get<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_X);
		const float& draw_pos_y = get<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_Y);
		const float& center_x = get<float>(enum_sprite_float_e::DRAW_RELATIVE_CENTER_X);
		const float& center_y = get<float>(enum_sprite_float_e::DRAW_RELATIVE_CENTER_Y);
		const std::string to_str = get<safe_string>(enum_text_safe_string_e::STRING).read();
		const int& text_alignment = get<int>(enum_text_integer_e::DRAW_ALIGNMENT);
		const color& text_clr = get<color>(enum_sprite_color_e::DRAW_TINT);

		int height = font_used->get_line_height();
		if (height <= 0) throw std::runtime_error("Font said invalid height size!");

		transform current;

		//transf_back = transf; // later just apply back
		current = transf; // working here

		const float csx = scale_g * scale_x / height;
		const float csy = scale_g * scale_y / height;

		drawntransf.scale_inverse(1.0f / (csx * real_prop), 1.0f / (csy * real_prop));
		drawntransf.apply();

		const auto text_len = font_used->get_width(to_str);

		std::stringstream ss(to_str);

		{
			std::string _temp;
			size_t linecount_off = 0;

			while (std::getline(ss, _temp, '\n')) {

				for (const auto& i : shadows)
				{
					font_used->draw(
						i.clr,
						static_cast<float>(center_x + i.offset_x) * text_len, (static_cast<float>(center_y + i.offset_y) + static_cast<float>(linecount_off) * draw_line_height + line_offset) * height,
						text_alignment,
						_temp);
				}

				font_used->draw(
					text_clr,
					static_cast<float>(center_x) * text_len, (static_cast<float>(center_y) + static_cast<float>(linecount_off) * draw_line_height + line_offset) * height,
					text_alignment,
					_temp);

				linecount_off++;
			}
		}
	}

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