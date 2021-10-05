#include "block.h"

namespace Lunaris {

	LUNARIS_DECL std::shared_lock<std::shared_mutex> block::mu_shared_read_control() const
	{
		return std::shared_lock<std::shared_mutex>(textures_mtx);
	}

	LUNARIS_DECL std::unique_lock<std::shared_mutex> block::mu_shared_write_control() const
	{
		return std::unique_lock<std::shared_mutex>(textures_mtx);
	}

	LUNARIS_DECL void block::generic_draw_no_lock(const texture& bmp)
	{
		const int bmpx = bmp.get_width();
		const int bmpy = bmp.get_height();

		if (bmpx <= 0 || bmpy <= 0) {
			throw std::runtime_error("Texture had invalid size!");
		}

		const float dsx = 1.0f * static_cast<float>(get<float>(enum_sprite_float_e::SCALE_X)) * static_cast<float>(get<float>(enum_sprite_float_e::SCALE_G)) * (1.0f / bmpx);
		const float dsy = 1.0f * static_cast<float>(get<float>(enum_sprite_float_e::SCALE_Y)) * static_cast<float>(get<float>(enum_sprite_float_e::SCALE_G)) * (1.0f / bmpy);

		const double& is_tinted = get<bool>(enum_sprite_boolean_e::DRAW_USE_COLOR);

		if (is_tinted) {
			bmp.draw_tinted_scaled_rotated_at(
				get<color>(enum_sprite_color_e::DRAW_TINT),
				get<float>(enum_sprite_float_e::DRAW_RELATIVE_CENTER_X), get<float>(enum_sprite_float_e::DRAW_RELATIVE_CENTER_Y),
				0.0f, 0.0f,
				dsx, dsy,
				0.0f);
		}
		else {
			bmp.draw_scaled_rotated_at(
				get<float>(enum_sprite_float_e::DRAW_RELATIVE_CENTER_X), get<float>(enum_sprite_float_e::DRAW_RELATIVE_CENTER_Y),
				0.0f, 0.0f,
				dsx, dsy,
				0.0f);
		}
	}

	LUNARIS_DECL void block::draw_task(transform transf, transform transf2, const float& limit_x, const float& limit_y)
	{
		// this is for range check

		// assume pos0 for now:

		if (textures.empty()) return;

		auto lock = mu_shared_read_control();

		size_t& frame = get<size_t>(enum_block_sizet_e::RO_DRAW_FRAME);

		if (!get<bool>(enum_block_bool_e::DRAW_SET_FRAME_VALUE_READONLY)) {

			const double& delta = get<double>(enum_block_double_e::DRAW_FRAMES_PER_SECOND); // delta t, 1/t = sec
			std::chrono::milliseconds& last_time = get<std::chrono::milliseconds>(enum_block_cmilliseconds_e::RO_DRAW_LAST_FRAME);

			if (delta > 0.0) { // if delta <= 0 or frame < 0, static
				std::chrono::milliseconds delta_tr = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(1.0 / delta));

				if (delta_tr.count() > 0) {

					if (std::chrono::system_clock::now().time_since_epoch() > last_time * block_max_frames_behind) {
						last_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
					}
					else {
						while (std::chrono::system_clock::now().time_since_epoch() > last_time) {
							last_time += delta_tr;
							if (++frame >= textures.size()) frame = 0;
						}
					}
				}
			}
		}

		if (frame >= textures.size()) frame = static_cast<size_t>(textures.size() - 1);

		generic_draw_no_lock(*textures[frame]);
	}

	LUNARIS_DECL block::block() :
		sprite(),
		fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE), size_t, enum_block_sizet_e>(default_block_sizet_il),
		fixed_multi_map_work<static_cast<size_t>(enum_block_cmilliseconds_e::_SIZE), std::chrono::milliseconds, enum_block_cmilliseconds_e>(default_block_cmilliseconds_il),
		fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE), bool, enum_block_bool_e>(default_block_bool_il),
		fixed_multi_map_work<static_cast<size_t>(enum_block_double_e::_SIZE), double, enum_block_double_e>(default_block_double_il)
	{
	}

	LUNARIS_DECL void block::texture_insert(const hybrid_memory<texture>& oth)
	{
		auto lock = mu_shared_write_control();
		textures.push_back(oth);
	}

	LUNARIS_DECL const hybrid_memory<texture>& block::texture_index(const size_t index) const
	{
		auto lock = mu_shared_read_control();
		if (index >= textures.size()) throw std::out_of_range("index out of range");
		return textures[index];
	}

	LUNARIS_DECL size_t block::texture_size() const
	{
		return textures.size();
	}

	LUNARIS_DECL void block::texture_remove(const size_t index)
	{
		auto lock = mu_shared_write_control();
		if (index >= textures.size()) throw std::out_of_range("index out of range");
		textures.erase(textures.begin() + index);
	}

	LUNARIS_DECL void block::texture_remove_all()
	{
		auto lock = mu_shared_write_control();
		textures.clear();
	}

}