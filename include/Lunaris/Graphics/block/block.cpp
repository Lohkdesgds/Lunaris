#include "block.h"

namespace Lunaris {

	/*block::_block_drawing_references_fast::_block_drawing_references_fast(const double& a, const double& b, const double& c, const double& d, const double& e, const double& f, const double& g, const double& h, const double& i, const double& j, const double& k, const double& l, const double& m, const double& n) :
		RO_DRAW_PROJ_POS_X(a),
		RO_DRAW_PROJ_POS_Y(b),
		RO_DRAW_PROJ_ROTATION(c),
		RO_DRAW_LAST_DRAW(d),
		DRAW_RELATIVE_CENTER_X(e),
		DRAW_RELATIVE_CENTER_Y(f),
		DRAW_ELASTIC_POSITION_PROP(g),
		POS_X(h),
		POS_Y(i),
		ROTATION(j),
		SCALE_G(k),
		SCALE_X(l),
		SCALE_Y(m),
		OUT_OF_SIGHT_POS(n)
	{
	}*/

	std::shared_lock<std::shared_mutex> block::mu_shared_read_control() const
	{
		return std::shared_lock<std::shared_mutex>(textures_mtx);
	}

	std::unique_lock<std::shared_mutex> block::mu_shared_write_control() const
	{
		return std::unique_lock<std::shared_mutex>(textures_mtx);
	}

	void block::generic_draw_no_lock(const texture& bmp)
	{
		const int bmpx = bmp.get_width();
		const int bmpy = bmp.get_height();

		if (bmpx <= 0 || bmpy <= 0) {
			throw std::runtime_error("Texture had invalid size!");
		}

		const float rot_rad = static_cast<float>(get<double>(enum_sprite_double_e::RO_DRAW_PROJ_ROTATION)) * ALLEGRO_PI / 180.0f;
		const float dsx = 1.0f * static_cast<float>(get<double>(enum_sprite_double_e::SCALE_X)) * static_cast<float>(get<double>(enum_sprite_double_e::SCALE_G)) * (1.0f / bmpx);
		const float dsy = 1.0f * static_cast<float>(get<double>(enum_sprite_double_e::SCALE_Y)) * static_cast<float>(get<double>(enum_sprite_double_e::SCALE_G)) * (1.0f / bmpy);

		bmp.draw_scaled_rotated_at(
			static_cast<float>(get<double>(enum_sprite_double_e::DRAW_RELATIVE_CENTER_X)), static_cast<float>(get<double>(enum_sprite_double_e::DRAW_RELATIVE_CENTER_Y)),
			static_cast<float>(get<double>(enum_sprite_double_e::RO_DRAW_PROJ_POS_X)), static_cast<float>(get<double>(enum_sprite_double_e::RO_DRAW_PROJ_POS_Y)),
			dsx, dsy,
			rot_rad);

		/*const float rot_rad = static_cast<float>(_draw_fast.RO_DRAW_PROJ_ROTATION) * ALLEGRO_PI / 180.0f;
		const float dsx = 1.0f * static_cast<float>(_draw_fast.SCALE_X) * static_cast<float>(_draw_fast.SCALE_G) * (1.0f / bmpx);
		const float dsy = 1.0f * static_cast<float>(_draw_fast.SCALE_Y) * static_cast<float>(_draw_fast.SCALE_G) * (1.0f / bmpy);

		bmp.draw_scaled_rotated_at(
			static_cast<float>(_draw_fast.DRAW_RELATIVE_CENTER_X), static_cast<float>(_draw_fast.DRAW_RELATIVE_CENTER_Y),
			static_cast<float>(_draw_fast.RO_DRAW_PROJ_POS_X), static_cast<float>(_draw_fast.RO_DRAW_PROJ_POS_Y),
			dsx, dsy,
			rot_rad);*/

		// later: color tinted option

		/*if (get_direct<bool>(sprite::e_boolean::USE_COLOR)) {
			bmp.draw(
				get_direct<Interface::Color>(sprite::e_color::COLOR),
				cx, cy,
				static_cast<float>(_draw_fast.RO_DRAW_PROJ_POS_X), static_cast<float>(_draw_fast.RO_DRAW_PROJ_POS_Y),
				dsx, dsy,
				rot_rad);
		}
		else {
			bmp.draw(
				cx, cy,
				static_cast<float>(_draw_fast.RO_DRAW_PROJ_POS_X), static_cast<float>(_draw_fast.RO_DRAW_PROJ_POS_Y),
				dsx, dsy,
				rot_rad);
		}*/
	}

	void block::draw_task(const transform& transf, const float& limit_x, const float& limit_y)
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

			//set<std::chrono::milliseconds>(enum_block_cmilliseconds_e::RO_DRAW_LAST_FRAME, last_time); // should update as it goes now, because it's a ref
			//set<uintptr_t>(enum_block_sizet_e::RO_DRAW_FRAME, frame); // should update as it goes now, because it's a ref
		}

		if (frame >= textures.size()) frame = static_cast<size_t>(textures.size() - 1);

		generic_draw_no_lock(*textures[frame]);
	}

	block::block() : 
		sprite(),
		fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE), size_t, enum_block_sizet_e>(default_block_sizet_il),
		fixed_multi_map_work<static_cast<size_t>(enum_block_cmilliseconds_e::_SIZE), std::chrono::milliseconds, enum_block_cmilliseconds_e>(default_block_cmilliseconds_il),
		fixed_multi_map_work<static_cast<size_t>(enum_block_sizet_e::_SIZE), bool, enum_block_bool_e>(default_block_bool_il),
		fixed_multi_map_work<static_cast<size_t>(enum_block_double_e::_SIZE), double, enum_block_double_e>(default_block_double_il)
	{
	}

	void block::texture_insert(const hybrid_memory<texture>& oth)
	{
		auto lock = mu_shared_write_control();
		textures.push_back(oth);
	}

	const hybrid_memory<texture>& block::texture_index(const size_t index) const
	{
		auto lock = mu_shared_read_control();
		if (index >= textures.size()) throw std::out_of_range("index out of range");
		return textures[index];
	}

	size_t block::texture_size() const
	{
		return textures.size();
	}

	void block::texture_remove(const size_t index)
	{
		auto lock = mu_shared_write_control();
		if (index >= textures.size()) throw std::out_of_range("index out of range");
		textures.erase(textures.begin() + index);
	}

	void block::texture_remove_all()
	{
		auto lock = mu_shared_write_control();
		textures.clear();
	}

}