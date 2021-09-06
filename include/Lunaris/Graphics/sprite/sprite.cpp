#include "sprite.h"

namespace Lunaris {

	double sprite::get_real_pos_x(const bool refresh)
	{
		if (refresh) m_assist_transform.get_current_transform();

		float limit_x = -1.f, limit_y = -1.f;
		m_assist_transform.transform_inverse_coords(limit_x, limit_y);

		limit_x = fabsf(limit_x); // maximum X position that it can draw with current transformation

		return static_cast<double>(get<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE) ? (get<double>(enum_sprite_double_e::POS_X) * static_cast<double>(limit_x)) : get<double>(enum_sprite_double_e::POS_X));
	}

	double sprite::get_real_pos_y(const bool refresh)
	{
		if (refresh) m_assist_transform.get_current_transform();

		float limit_x = -1.f, limit_y = -1.f;
		m_assist_transform.transform_inverse_coords(limit_x, limit_y);

		limit_y = fabsf(limit_y); // maximum Y position that it can draw with current transformation

		return static_cast<double>(get<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE) ? (get<double>(enum_sprite_double_e::POS_Y) * static_cast<double>(limit_y)) : get<double>(enum_sprite_double_e::POS_Y));
	}

	sprite::sprite() :
		fixed_multi_map_work<static_cast<size_t>(enum_sprite_double_e::_SIZE), double, enum_sprite_double_e>(default_sprite_double_il),
		fixed_multi_map_work<static_cast<size_t>(enum_sprite_boolean_e::_SIZE), bool, enum_sprite_boolean_e>(default_sprite_boolean_il),
		fixed_multi_map_work<static_cast<size_t>(enum_sprite_color_e::_SIZE), color, enum_sprite_color_e>(default_sprite_color_il)
	{
	}

	void sprite::draw()
	{
		// - - - - - - - - Get current camera transform - - - - - - - - //
		m_assist_transform.get_current_transform();

		float limit_x = -1.f, limit_y = -1.f;
		m_assist_transform.transform_inverse_coords(limit_x, limit_y);
		limit_x = fabsf(limit_x); // maximum X position that it can draw with current transformation
		limit_y = fabsf(limit_y); // maximum Y position that it can draw with current transformation

		const double real_posx = get_real_pos_x();
		const double real_posy = get_real_pos_y();


		// - - - - - - - - Get references to work with - - - - - - - - //
		const bool& draw_should_draw = get<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW);
		const bool& draw_draw_box = get<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX); // colored rectangle
		const bool& draw_think_box = get<bool>(enum_sprite_boolean_e::DRAW_THINK_BOX); // debug colored rectangle
		const double& think_delta = get<double>(enum_sprite_double_e::RO_THINK_DELTA_THINKS);
		const double& out_of_sight_prop = get<double>(enum_sprite_double_e::OUT_OF_SIGHT_POS);
		const double& responsiveness = get<double>(enum_sprite_double_e::DRAW_MOVEMENT_RESPONSIVENESS);
		const double& scale_g = get<double>(enum_sprite_double_e::SCALE_G);
		const double& scale_x = get<double>(enum_sprite_double_e::SCALE_X);
		const double& scale_y = get<double>(enum_sprite_double_e::SCALE_Y);
		double& last_draw_v = get<double>(enum_sprite_double_e::RO_DRAW_LAST_DRAW);
		const bool& deform_coords = get<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE); // [-1,1] goes like [-limit_x,limit_x]

		// refs to smoothing algorithms
		const double& target_pos_x = get<double>(enum_sprite_double_e::POS_X); // target posx
		const double& target_pos_y = get<double>(enum_sprite_double_e::POS_Y); // target posy
		const double& target_rot = get<double>(enum_sprite_double_e::ROTATION); // target posy

		// this actual vals
		double& draw_pos_x = get<double>(enum_sprite_double_e::RO_DRAW_PROJ_POS_X);
		double& draw_pos_y = get<double>(enum_sprite_double_e::RO_DRAW_PROJ_POS_Y);
		double& draw_rot = get<double>(enum_sprite_double_e::RO_DRAW_PROJ_ROTATION);


		// - - - - - - - - Working - - - - - - - - //
		if ( draw_should_draw && // basic rule
			(out_of_sight_prop <= 0.0 || (
				(fabs(target_pos_x) < fabs(out_of_sight_prop * (limit_x + 0.5 * (scale_g * scale_x)))) && // within borders (version 1.0)
				(fabs(target_pos_y) < fabs(out_of_sight_prop * (limit_y + 0.5 * (scale_g * scale_y))))
			)
			))	// within borders (version 1.0)
		{
			double timee = al_get_time();
			double dt = timee - last_draw_v;
			last_draw_v = timee;

			double perc_run = (think_delta > 0.0 ? (responsiveness / think_delta) : 0.1) * dt; // pow(dt, 0.90);		// ex: 5 per sec * 0.2 (1/5 sec) = 1, so posx = actual posx...
			if (perc_run > 1.0) perc_run = 1.0;					// 1.0 is "set value"
			if (perc_run < 1.0 / 1500) perc_run = 1.0 / 1500; // can't be infinitely smooth right? come on

			//const double vect_x_wrk = (deform_coords ? (target_pos_x * static_cast<double>(limit_x)) : target_pos_x) - draw_pos_x; // where it should go - where it is
			//const double vect_y_wrk = (deform_coords ? (target_pos_y * static_cast<double>(limit_y)) : target_pos_y) - draw_pos_y; // where it should go - where it is

			draw_pos_x = ((1.0 - perc_run) * draw_pos_x + perc_run * real_posx);
			draw_pos_y = ((1.0 - perc_run) * draw_pos_y + perc_run * real_posy);
			draw_rot   = (1.0 - perc_run) * draw_rot   + perc_run * target_rot;

			if (draw_draw_box) {
				al_draw_filled_rectangle(
					/* X1: */ draw_pos_x - (scale_g * scale_x) * 0.5,
					/* Y1: */ draw_pos_y - (scale_g * scale_y) * 0.5,
					/* X2: */ draw_pos_x + (scale_g * scale_x) * 0.5,
					/* Y2: */ draw_pos_y + (scale_g * scale_y) * 0.5,
					get<color>(enum_sprite_color_e::DRAW_DRAW_BOX)
				);
			}
			if (draw_think_box) {
				al_draw_filled_rectangle(
					/* X1: */ real_posx - (scale_g * scale_x) * 0.5,
					/* Y1: */ real_posy - (scale_g * scale_y) * 0.5,
					/* X2: */ real_posx + (scale_g * scale_x) * 0.5,
					/* Y2: */ real_posy + (scale_g * scale_y) * 0.5,
					get<color>(enum_sprite_color_e::DRAW_THINK_BOX)
				);
			}

			draw_task(m_assist_transform, limit_x, limit_y); // for now
			set<bool>(enum_sprite_boolean_e::RO_DRAW_DRAWN_LAST_DRAW, true);
		}
		else {
			set<bool>(enum_sprite_boolean_e::RO_DRAW_DRAWN_LAST_DRAW, false);
		}
	}

	void sprite::think()
	{
		// not used
		//float limit_x = -1.f, limit_y = -1.f;
		//m_assist_transform.transform_inverse_coords(limit_x, limit_y);
		//limit_x = fabsf(limit_x); // maximum X position that it can draw with current transformation
		//limit_y = fabsf(limit_y); // maximum Y position that it can draw with current transformation

		const double& elasticity = get<double>(enum_sprite_double_e::THINK_ELASTIC_SPEED_PROP);
		double& last_think_v = get<double>(enum_sprite_double_e::RO_THINK_LAST_THINK);
		double& think_delta_v = get<double>(enum_sprite_double_e::RO_THINK_DELTA_THINKS);

		double& curr_speed_x = get<double>(enum_sprite_double_e::RO_THINK_SPEED_X);
		double& curr_speed_y = get<double>(enum_sprite_double_e::RO_THINK_SPEED_Y);
		double& curr_pos_x = get<double>(enum_sprite_double_e::POS_X);
		double& curr_pos_y = get<double>(enum_sprite_double_e::POS_Y);
		double& curr_accel_x = get<double>(enum_sprite_double_e::ACCEL_X);
		double& curr_accel_y = get<double>(enum_sprite_double_e::ACCEL_Y);

		//const bool& deform_coords = get<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE); // [-1,1] goes like [-limit_x,limit_x] // (deform_coords ? (target_pos_x * static_cast<double>(limit_x)) : target_pos_x) // (deform_coords ? (target_pos_y * static_cast<double>(limit_y)) : target_pos_y)

		double timee = al_get_time();
		think_delta_v = timee - last_think_v;
		last_think_v = timee;

		// speed per tick think

		curr_pos_x += curr_speed_x;
		curr_pos_y += curr_speed_y;

		curr_speed_x += curr_accel_x;
		curr_speed_y += curr_accel_y;

		if (elasticity > 1.0) throw std::runtime_error("Elasticity is higher than 1.0, this means speed would go to infinite!");

		curr_speed_x *= elasticity;
		curr_speed_y *= elasticity;

		think_task();
	}

}