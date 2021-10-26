#include "sprite.h"

namespace Lunaris {

	LUNARIS_DECL float sprite::get_real_pos_x(const bool refresh)
	{
		if (refresh) m_assist_transform.get_current_transform();

		float limit_x = -1.f, limit_y = -1.f;
		m_assist_transform.transform_inverse_coords(limit_x, limit_y);

		limit_x = fabsf(limit_x); // maximum X position that it can draw with current transformation

		return static_cast<float>(get<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE) ? (get<float>(enum_sprite_float_e::POS_X) * static_cast<float>(limit_x)) : get<float>(enum_sprite_float_e::POS_X));
	}

	LUNARIS_DECL float sprite::get_real_pos_y(const bool refresh)
	{
		if (refresh) m_assist_transform.get_current_transform();

		float limit_x = -1.f, limit_y = -1.f;
		m_assist_transform.transform_inverse_coords(limit_x, limit_y);

		limit_y = fabsf(limit_y); // maximum Y position that it can draw with current transformation

		return static_cast<float>(get<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE) ? (get<float>(enum_sprite_float_e::POS_Y) * static_cast<float>(limit_y)) : get<float>(enum_sprite_float_e::POS_Y));
	}

	LUNARIS_DECL sprite::sprite() :
		fixed_multi_map_work<static_cast<size_t>(enum_sprite_float_e::_SIZE), float, enum_sprite_float_e>(default_sprite_float_il),
		fixed_multi_map_work<static_cast<size_t>(enum_sprite_double_e::_SIZE), double, enum_sprite_double_e>(default_sprite_double_il),
		fixed_multi_map_work<static_cast<size_t>(enum_sprite_boolean_e::_SIZE), bool, enum_sprite_boolean_e>(default_sprite_boolean_il),
		fixed_multi_map_work<static_cast<size_t>(enum_sprite_color_e::_SIZE), color, enum_sprite_color_e>(default_sprite_color_il)
	{
	}

	LUNARIS_DECL void sprite::draw()
	{
		// - - - - - - - - Get current camera transform - - - - - - - - //
		m_assist_transform.get_current_transform();

		float limit_x = -1.f, limit_y = -1.f;
		m_assist_transform.transform_inverse_coords(limit_x, limit_y);
		limit_x = fabsf(limit_x); // maximum X position that it can draw with current transformation
		limit_y = fabsf(limit_y); // maximum Y position that it can draw with current transformation

		const float real_posx = get_real_pos_x();
		const float real_posy = get_real_pos_y();


		// - - - - - - - - Get references to work with - - - - - - - - //
		const bool& draw_should_draw = get<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW);
		const bool& draw_draw_box = get<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX); // colored rectangle
		const bool& draw_think_box = get<bool>(enum_sprite_boolean_e::DRAW_THINK_BOX); // debug colored rectangle
		const double& think_delta = get<double>(enum_sprite_double_e::RO_THINK_DELTA_THINKS);
		const float& out_of_sight_prop = get<float>(enum_sprite_float_e::OUT_OF_SIGHT_POS);
		const float& responsiveness = get<float>(enum_sprite_float_e::DRAW_MOVEMENT_RESPONSIVENESS);
		const float& scale_g = get<float>(enum_sprite_float_e::SCALE_G);
		const float& scale_x = get<float>(enum_sprite_float_e::SCALE_X);
		const float& scale_y = get<float>(enum_sprite_float_e::SCALE_Y);
		double& last_draw_v = get<double>(enum_sprite_double_e::RO_DRAW_LAST_DRAW);

		// refs to smoothing algorithms
		const float& target_pos_x = get<float>(enum_sprite_float_e::POS_X); // target posx
		const float& target_pos_y = get<float>(enum_sprite_float_e::POS_Y); // target posy
		const float& target_rot = get<float>(enum_sprite_float_e::ROTATION); // target posy

		// this actual vals
		float& draw_pos_x = get<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_X);
		float& draw_pos_y = get<float>(enum_sprite_float_e::RO_DRAW_PROJ_POS_Y);
		float& draw_rot = get<float>(enum_sprite_float_e::RO_DRAW_PROJ_ROTATION);


		// - - - - - - - - Working - - - - - - - - //
		if ( draw_should_draw && // basic rule
			(out_of_sight_prop <= 0.0f || (
				(fabsf(target_pos_x) < fabsf(out_of_sight_prop * (limit_x + 0.5f * (scale_g * scale_x)))) && // within borders (version 1.0)
				(fabsf(target_pos_y) < fabsf(out_of_sight_prop * (limit_y + 0.5f * (scale_g * scale_y))))
			)
			))	// within borders (version 1.0)
		{
			double timee = al_get_time();
			double dt = timee - last_draw_v;
			last_draw_v = timee;

			double perc_run = (think_delta > 0.0 ? (static_cast<double>(responsiveness) / think_delta) : 0.1) * dt; // pow(dt, 0.90);		// ex: 5 per sec * 0.2 (1/5 sec) = 1, so posx = actual posx...
			if (perc_run > 1.0) perc_run = 1.0;					// 1.0 is "set value"
			if (perc_run < 1.0 / 5000) perc_run = 1.0 / 5000; // can't be infinitely smooth right? come on

			draw_pos_x = ((1.0 - perc_run) * draw_pos_x + perc_run * real_posx);
			draw_pos_y = ((1.0 - perc_run) * draw_pos_y + perc_run * real_posy);
			draw_rot   = (1.0 - perc_run) * draw_rot   + perc_run * target_rot;

			transform _temp; // transform by TRANSFORMATION. Assume POSX and POSY 0 from now on

			if (draw_think_box) { // if think box, calculate think box raw pos
				_temp.identity();
				_temp.rotate(draw_rot);
				_temp.translate(real_posx, real_posy);
				_temp.compose(m_assist_transform);
				_temp.apply();

				al_draw_filled_rectangle(
					/* X1: */ - (scale_g * scale_x) * 0.5,
					/* Y1: */ - (scale_g * scale_y) * 0.5,
					/* X2: */   (scale_g * scale_x) * 0.5,
					/* Y2: */   (scale_g * scale_y) * 0.5,
					get<color>(enum_sprite_color_e::DRAW_THINK_BOX)
				);
			}

			// after think box calculate real pos (real I mean the drawing position pos)
			_temp.identity();
			_temp.rotate(draw_rot);
			_temp.translate(draw_pos_x, draw_pos_y);
			_temp.compose(m_assist_transform);
			_temp.apply();

			if (draw_draw_box) {
				al_draw_filled_rectangle(
					/* X1: */ - (scale_g * scale_x) * 0.5,
					/* Y1: */ - (scale_g * scale_y) * 0.5,
					/* X2: */   (scale_g * scale_x) * 0.5,
					/* Y2: */   (scale_g * scale_y) * 0.5,
					get<color>(enum_sprite_color_e::DRAW_DRAW_BOX)
				);
			}

			draw_task(m_assist_transform, _temp, limit_x, limit_y); // for now
			set<bool>(enum_sprite_boolean_e::RO_DRAW_DRAWN_LAST_DRAW, true);

			m_assist_transform.apply(); // reset transformation
		}
		else {
			set<bool>(enum_sprite_boolean_e::RO_DRAW_DRAWN_LAST_DRAW, false);
		}
	}

	LUNARIS_DECL void sprite::think()
	{
		// not used
		//float limit_x = -1.f, limit_y = -1.f;
		//m_assist_transform.transform_inverse_coords(limit_x, limit_y);
		//limit_x = fabsf(limit_x); // maximum X position that it can draw with current transformation
		//limit_y = fabsf(limit_y); // maximum Y position that it can draw with current transformation

		const float& elasticity = get<float>(enum_sprite_float_e::THINK_ELASTIC_SPEED_PROP);
		double& last_think_v = get<double>(enum_sprite_double_e::RO_THINK_LAST_THINK);
		double& think_delta_v = get<double>(enum_sprite_double_e::RO_THINK_DELTA_THINKS);

		float& curr_speed_x = get<float>(enum_sprite_float_e::RO_THINK_SPEED_X);
		float& curr_speed_y = get<float>(enum_sprite_float_e::RO_THINK_SPEED_Y);
		float& curr_speed_rot = get<float>(enum_sprite_float_e::RO_THINK_SPEED_ROTATION);

		float& curr_pos_x = get<float>(enum_sprite_float_e::POS_X);
		float& curr_pos_y = get<float>(enum_sprite_float_e::POS_Y);
		float& curr_pos_rot = get<float>(enum_sprite_float_e::ROTATION);

		float& curr_accel_x = get<float>(enum_sprite_float_e::ACCEL_X);
		float& curr_accel_y = get<float>(enum_sprite_float_e::ACCEL_Y);
		float& curr_accel_rot = get<float>(enum_sprite_float_e::ACCEL_ROTATION);

		double timee = al_get_time();
		think_delta_v = timee - last_think_v;
		last_think_v = timee;

		// speed per tick think

		curr_pos_x += curr_speed_x;
		curr_pos_y += curr_speed_y;
		curr_pos_rot += curr_speed_rot;

		curr_speed_x += curr_accel_x;
		curr_speed_y += curr_accel_y;
		curr_speed_rot += curr_accel_rot;

		if (elasticity > 1.0) throw std::runtime_error("Elasticity is higher than 1.0, this means speed would go to infinite!");

		curr_speed_x *= elasticity;
		curr_speed_y *= elasticity;
		curr_speed_rot *= elasticity;

		think_task();

		const float& scale_g = get<float>(enum_sprite_float_e::SCALE_G);
		const float& scale_x = get<float>(enum_sprite_float_e::SCALE_X);
		const float& scale_y = get<float>(enum_sprite_float_e::SCALE_Y);

		float& nwx = get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X);
		float& nex = get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X);
		float& swx = get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X);
		float& sex = get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X);
		float& nwy = get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y);
		float& ney = get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y);
		float& swy = get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y);
		float& sey = get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y);

		float __rot = get<float>(enum_sprite_float_e::ROTATION);
		//while (__rot > ALLEGRO_PI * 2.0f) __rot -= ALLEGRO_PI * 2.0f;

		transform _temp;
		_temp.identity();
		_temp.rotate(__rot);
		_temp.translate(curr_pos_x, curr_pos_y);
		_temp.compose(m_assist_transform);

		// top left
		nwx = - (scale_g * scale_x) * 0.5;
		nwy = - (scale_g * scale_y) * 0.5;
		// top right
		nex =   (scale_g * scale_x) * 0.5;
		ney = - (scale_g * scale_y) * 0.5;
		// bottom left
		swx = - (scale_g * scale_x) * 0.5;
		swy =   (scale_g * scale_y) * 0.5;
		// bottom right
		sex =   (scale_g * scale_x) * 0.5;
		sey =   (scale_g * scale_y) * 0.5;
		

		_temp.transform_coords(nwx, nwy);
		_temp.transform_coords(nex, ney);
		_temp.transform_coords(swx, swy);
		_temp.transform_coords(sex, sey);
	}

}