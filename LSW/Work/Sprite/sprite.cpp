#include "sprite.h"

namespace LSW {
	namespace v5 {
		namespace Work {

			Sprite_Base::easier_collision_handle::easier_collision_handle(const easier_collision_handle& cpy)
			{
				posx = cpy.posx;
				posy = cpy.posy;
				sizx = cpy.sizx;
				sizy = cpy.sizy;
				dx_max = cpy.dx_max;
				dy_max = cpy.dy_max;
				memcpy_s(&directions_cases, sizeof(directions_cases), &cpy.directions_cases, sizeof(cpy.directions_cases));
				was_col = cpy.was_col;
			}

			bool Sprite_Base::easier_collision_handle::overlap(const easier_collision_handle& ol) // me == P1
			{
				double dist_x = (posx - ol.posx);
				double diff_x = fabs(dist_x) - (sizx + ol.sizx) * 1.0 / 2.0; // if < 0, col
				bool col_x = diff_x < 0.0;
				double dist_y = (posy - ol.posy);
				double diff_y = fabs(dist_y) - (sizy + ol.sizy) * 1.0 / 2.0; // if < 0, col
				bool col_y = diff_y < 0.0;
				bool is_col = col_y && col_x;

				if (is_col) {
					if (fabs(diff_x) < fabs(diff_y)) {
						if (fabs(diff_x) > fabs(dx_max)) dx_max = diff_x;
						
						if (dist_x > 0.0) {
							directions_cases[static_cast<int>(sprite::e_direction_array_version::WEST)]++; // VEM DO WEST
						}
						else {
							directions_cases[static_cast<int>(sprite::e_direction_array_version::EAST)]++; // VEM DO EAST
						}
					}
					else {
						if (fabs(diff_y) > fabs(dy_max)) dy_max = diff_y;
						if (dist_y > 0.0) {
							directions_cases[static_cast<int>(sprite::e_direction_array_version::NORTH)]++; // VEM DO NORTH
						}
						else {
							directions_cases[static_cast<int>(sprite::e_direction_array_version::SOUTH)]++; // VEM DO SOUTH
						}
					}
				}

				/*
				Y:
				P1 - P2 > 0 == col vem do NORTH
				P1 - P2 < 0 == col vem do SOUTH

				X:
				P1 - P2 > 0 == col vem do WEST
				P1 - P2 < 0 == col vem do EAST
				*/

				was_col |= is_col;
				return is_col;
			}

			int Sprite_Base::easier_collision_handle::process_result()
			{
				bool n, s, w, e;
				n = directions_cases[static_cast<int>(sprite::e_direction_array_version::NORTH)] > 0;
				s = directions_cases[static_cast<int>(sprite::e_direction_array_version::SOUTH)] > 0;
				w = directions_cases[static_cast<int>(sprite::e_direction_array_version::WEST)] > 0;
				e = directions_cases[static_cast<int>(sprite::e_direction_array_version::EAST)] > 0;

				const int east = static_cast<int>(sprite::e_direction::EAST);
				const int west = static_cast<int>(sprite::e_direction::WEST);
				const int north = static_cast<int>(sprite::e_direction::NORTH);
				const int south = static_cast<int>(sprite::e_direction::SOUTH);


				if (n) { // north
					if (w && !e) {
						return south | east; // GOTO EAST
					}
					if (e && !w) {
						return south | west; // GOTO WEST
					}
					if (s) {
						if (directions_cases[static_cast<int>(sprite::e_direction_array_version::NORTH)] > directions_cases[static_cast<int>(sprite::e_direction_array_version::SOUTH)]) return south;
						if (directions_cases[static_cast<int>(sprite::e_direction_array_version::SOUTH)] > directions_cases[static_cast<int>(sprite::e_direction_array_version::NORTH)]) return north;
						return 0; // NO GOTO
					}
					return south; // GOTO SOUTH
				}
				else if (s) { //south
					if (w && !e) {
						return north | east; // GOTO EAST
					}
					if (e && !w) {
						return north | west; // GOTO WEST
					}
					return north; // GOTO NORTH
				}
				else if (w) { // west
					if (e) {
						if (directions_cases[static_cast<int>(sprite::e_direction_array_version::EAST)] > directions_cases[static_cast<int>(sprite::e_direction_array_version::WEST)]) return west;
						if (directions_cases[static_cast<int>(sprite::e_direction_array_version::WEST)] > directions_cases[static_cast<int>(sprite::e_direction_array_version::EAST)]) return east;
						return 0;
					}
					return east;
				}
				else if (e) { // east
					return west;
				}
				return 0;
			}

			void Sprite_Base::easier_collision_handle::reset_directions()
			{
				for (auto& i : directions_cases) i = 0;
				dx_max = dy_max = 0.0;
			}

			void Sprite_Base::easier_collision_handle::setup(const double px, const double py, const double sx, const double sy)
			{
				posx = px;
				posy = py;
				sizx = sprite::game_collision_oversize + sx;
				sizy = sprite::game_collision_oversize + sy;
				reset_directions();
				was_col = false;
			}

			void Sprite_Base::common_bitmap_draw_task(const Interface::Bitmap& bmp)
			{
				const int bmpx = bmp.get_width();
				const int bmpy = bmp.get_height();
				if (bmpx <= 0 || bmpy <= 0) {
					throw Handling::Abort(__FUNCSIG__, "Somehow the texture have < 0 width | height!");
				}

				const float cx = 1.0f * bmpx * ((get_direct<double>(sprite::e_double::CENTER_X) + 1.0) * 0.5);
				const float cy = 1.0f * bmpy * ((get_direct<double>(sprite::e_double::CENTER_Y) + 1.0) * 0.5);
				const float rot_rad = 1.0f * get_direct<double>(sprite::e_double_readonly::ROTATION) * ALLEGRO_PI / 180.0;
				const float px = get_direct<double>(sprite::e_double_readonly::POSX);
				const float py = get_direct<double>(sprite::e_double_readonly::POSY);
				const float dsx = 1.0f * (get_direct<double>(sprite::e_double::SCALE_X)) * (get_direct<double>(sprite::e_double::SCALE_G)) * (1.0 / bmpx);
				const float dsy = 1.0f * (get_direct<double>(sprite::e_double::SCALE_Y)) * (get_direct<double>(sprite::e_double::SCALE_G)) * (1.0 / bmpy);

				if (get_direct<bool>(sprite::e_boolean::USE_COLOR)) {
					bmp.draw(
						get_direct<Interface::Color>(sprite::e_color::COLOR),
						cx, cy,
						px, py,
						dsx, dsy,
						rot_rad);
				}
				else {
					bmp.draw(
						cx, cy,
						px, py,
						dsx, dsy,
						rot_rad);
				}
			}

			Sprite_Base::Sprite_Base()
			{
				Handling::init_basic();
				Handling::init_graphics();

				set<sprite::functional>(sprite::e_functional_defaults);
				set<double>(sprite::e_double_defaults);
				set<bool>(sprite::e_boolean_defaults);
				set<std::string>(sprite::e_string_defaults);
				set<int>(sprite::e_integer_defaults);
				set<Interface::Color>(sprite::e_color_defaults);
				set(sprite::e_uintptrt::DATA_FROM, (uintptr_t)this);
			}

			Sprite_Base::Sprite_Base(const Sprite_Base& other) // copy
			{
				*this = other;
			}

			Sprite_Base::Sprite_Base(Sprite_Base&& other)
			{
				*this = std::move(other);
			}

			void Sprite_Base::clone(const Sprite_Base& other) // reference
			{
				set<sprite::functional>(*other.get<sprite::functional>());
				set<sprite::e_tie_functional>(*other.get<sprite::e_tie_functional>());
				set<double>(*other.get<double>());
				set<bool>(*other.get<bool>());
				set<std::string>(*other.get<std::string>());
				set<int>(*other.get<int>());
				set<Interface::Color>(*other.get<Interface::Color>());
				set<uintptr_t>(*other.get<uintptr_t>());
				set(sprite::e_uintptrt::DATA_FROM, (uintptr_t)this);
			}

			void Sprite_Base::operator=(const Sprite_Base& oth)
			{
				set<sprite::functional>(oth.get<sprite::functional>());
				set<sprite::e_tie_functional>(oth.get<sprite::e_tie_functional>());
				set<double>(oth.get<double>());
				set<bool>(oth.get<bool>());
				set<std::string>(oth.get<std::string>());
				set<int>(oth.get<int>());
				set<Interface::Color>(oth.get<Interface::Color>());
				set<uintptr_t>(oth.get<uintptr_t>());
			}

			void Sprite_Base::operator=(Sprite_Base&& other)
			{
				set<sprite::functional>(std::move(other.get<sprite::functional>()));
				set<sprite::e_tie_functional>(std::move(other.get<sprite::e_tie_functional>()));
				set<double>(std::move(other.get<double>()));
				set<bool>(std::move(other.get<bool>()));
				set<std::string>(std::move(other.get<std::string>()));
				set<int>(std::move(other.get<int>()));
				set<Interface::Color>(std::move(other.get<Interface::Color>()));
				set<uintptr_t>(std::move(other.get<uintptr_t>()));
			}

			void Sprite_Base::draw(const Interface::Camera& cam, const bool run_anyway)
			{
				if (!run_anyway && get_direct<uintptr_t>(sprite::e_uintptrt::DATA_FROM) != (uintptr_t)this) return;  // only original copy should update parameters
				if (is_eq(sprite::e_boolean::DRAW, false)) {
					set(sprite::e_boolean_readonly::IS_OUTSIDE_SCREEN, true);
					return;
				}

				//if (is_eq(sprite::e_boolean::DRAW_COLOR_BOX, true) || is_eq(sprite::e_boolean::DRAW_DOT, true)) {

				const auto affected_by_cam = get_direct<bool>(sprite::e_boolean::AFFECTED_BY_CAM);
				const auto targ_rotation = get_direct<double>(sprite::e_double::TARG_ROTATION);
				const auto real_targ_posx = get_direct<double>(sprite::e_double::TARG_POSX);
				const auto real_targ_posy = get_direct<double>(sprite::e_double::TARG_POSY);
				const auto targ_posx = get_direct<double>(sprite::e_double_readonly::PROJECTED_POSX);
				const auto targ_posy = get_direct<double>(sprite::e_double_readonly::PROJECTED_POSY);
				const auto scale_g = get_direct<double>(sprite::e_double::SCALE_G);
				const auto scale_x = get_direct<double>(sprite::e_double::SCALE_X);
				const auto scale_y = get_direct<double>(sprite::e_double::SCALE_Y);
				const auto update_delta = get_direct<double>(sprite::e_double_readonly::UPDATE_DELTA);
				const auto last_draw_v = get_direct<double>(sprite::e_double_readonly::LAST_DRAW);
				const auto rotation_v = get_direct<double>(sprite::e_double_readonly::ROTATION);
				const auto posx_v = get_direct<double>(sprite::e_double_readonly::POSX);
				const auto posy_v = get_direct<double>(sprite::e_double_readonly::POSY);
				const auto last_update = get_direct<double>(sprite::e_double_readonly::LAST_UPDATE); // al_get_time

				Interface::Camera _cleancam = cam;

				if (!affected_by_cam) {
					_cleancam.classic_transform(0.0, 0.0, 1.0, 1.0, 0.0);
				}
				_cleancam.apply();

				const double calculated_scale_x = scale_g * scale_x;
				const double calculated_scale_y = scale_g * scale_y;

				// should draw
				if (const double dist_range = get_direct<double>(sprite::e_double::DISTANCE_DRAWING_SCALE); dist_range > 0.0) {

					const double max_dist = (1.0 + (sqrt(calculated_scale_x * calculated_scale_x + calculated_scale_y * calculated_scale_y))); // this should work even with rotation because of the nature of the calculation lmao

					if (fabs(real_targ_posx - _cleancam.get_classic().x) > (max_dist * 1.0 * dist_range / _cleancam.get_classic().sx) || fabs(real_targ_posy - _cleancam.get_classic().y) > (max_dist * 1.0 * dist_range / _cleancam.get_classic().sy)) {
						set(sprite::e_boolean_readonly::IS_OUTSIDE_SCREEN, true);

						return; // should skip yeah
					}
				}

				easy_collision.latest_camera = cam.get_classic();

				// delta T calculation

				double timee = al_get_time();
				double dt = timee - last_draw_v;
				set(sprite::e_double_readonly::LAST_DRAW, timee);

				double perc_run = (update_delta > 0.0 ? (0.5 / update_delta) : 0.1) * pow(dt, 0.86);		// ex: 5 per sec * 0.2 (1/5 sec) = 1, so posx = actual posx...
				if (perc_run > 1.0) perc_run = 1.0;					// 1.0 is "set value"
				if (perc_run < 1.0 / 500) perc_run = 1.0 / 500; // can't be infinitely smooth right? come on

				// new position calculation

				if (al_get_time() - last_update >= sprite::maximum_time_between_collisions || get_direct<bool>(sprite::e_boolean_readonly::IS_OUTSIDE_SCREEN)) {
					set(sprite::e_double_readonly::ROTATION, targ_rotation);
					set(sprite::e_double_readonly::POSX, real_targ_posx);
					set(sprite::e_double_readonly::POSY, real_targ_posy);
					set(sprite::e_double_readonly::PERC_CALC_SMOOTH, 0.0);
					set(sprite::e_double_readonly::REALISTIC_RESULT_POSX, real_targ_posx); // TIMEOUT
					set(sprite::e_double_readonly::REALISTIC_RESULT_POSY, real_targ_posy); // TIMEOUT
				}
				else {
					set(sprite::e_double_readonly::ROTATION, (1.0 - perc_run) * rotation_v + perc_run * targ_rotation);
					set(sprite::e_double_readonly::POSX, (1.0 - perc_run) * posx_v + perc_run * targ_posx);
					set(sprite::e_double_readonly::POSY, (1.0 - perc_run) * posy_v + perc_run * targ_posy);
					set(sprite::e_double_readonly::PERC_CALC_SMOOTH, perc_run);
				}

				set(sprite::e_boolean_readonly::IS_OUTSIDE_SCREEN, false);


				if (is_eq(sprite::e_boolean::DRAW_COLOR_BOX, true)) {

					al_draw_filled_rectangle(
						/* X1: */ real_targ_posx - calculated_scale_x * 0.5,
						/* Y1: */ real_targ_posy - calculated_scale_y * 0.5,
						/* X2: */ real_targ_posx + calculated_scale_x * 0.5,
						/* Y2: */ real_targ_posy + calculated_scale_y * 0.5,
						get_direct<Interface::Color>(sprite::e_color::COLOR) //al_map_rgba(90, easy_collision.was_col ? 45 : 90, easy_collision.was_col ? 45 : 90, 90)
					);
				}
				if (is_eq(sprite::e_boolean::DRAW_DEBUG_BOX, true)) {

					al_draw_filled_rectangle(
						/* X1: */ real_targ_posx - calculated_scale_x * 0.515,
						/* Y1: */ real_targ_posy - calculated_scale_y * 0.515,
						/* X2: */ real_targ_posx + calculated_scale_x * 0.515,
						/* Y2: */ real_targ_posy + calculated_scale_y * 0.515,
						Interface::Color(180, easy_collision.was_col ? 90 : 180, easy_collision.was_col ? 90 : 180, 180)
					);
				}

				if (is_eq(sprite::e_boolean::DRAW_DOT, true)) {
					al_draw_filled_circle(
						/* X1: */ posx_v,
						/* X1: */ posy_v,
						/* SCL */ 0.05f,
						Interface::Color(90, easy_collision.was_col ? 45 : 90, easy_collision.was_col ? 45 : 90, 90)
					);
				}
				//}
					
				draw_task(_cleancam);
			}

			void Sprite_Base::collide(const Sprite_Base& oth, const bool run_anyway) // if colliding, do not accept speed entry (keyboard player moving)
			{
				if (!run_anyway && get_direct<uintptr_t>(sprite::e_uintptrt::DATA_FROM) != (uintptr_t)this) return; // this is a copy following attrbutes from somewhere else, no duplication in collision

				if (oth.is_eq(sprite::e_integer::COLLISION_MODE, static_cast<int>(sprite::e_collision_mode_cast::COLLISION_NONE))) return; // no collision
				if (oth.is_eq(sprite::e_integer::COLLISION_MODE, static_cast<int>(sprite::e_collision_mode_cast::COLLISION_INVERSE))) return; // no collision
				if (is_eq(sprite::e_integer::COLLISION_MODE, static_cast<int>(sprite::e_collision_mode_cast::COLLISION_NONE))) return; // no collision
				if (is_eq(sprite::e_integer::COLLISION_MODE, static_cast<int>(sprite::e_collision_mode_cast::COLLISION_STATIC))) return; // no collision

				easy_collision.overlap(oth.easy_collision);
			}

			void Sprite_Base::update_and_clear(const Interface::Config& conf, const bool run_anyway)// needs to use ACCELERATION_X when collision, ELASTICITY_X to know how much of it goes backwards, TARG_POSX
			{
				if (!run_anyway && get_direct<uintptr_t>(sprite::e_uintptrt::DATA_FROM) != (uintptr_t)this) return; // this is a copy following attrbutes from somewhere else, no duplication in update

				{
					double update_now = al_get_time();
					double last_update = get_direct<double>(sprite::e_double_readonly::LAST_UPDATE);
					if (update_now - last_update > sprite::maximum_time_between_collisions) last_update = update_now - 1.0;
					set(sprite::e_double_readonly::LAST_UPDATE, update_now);
					set(sprite::e_double_readonly::UPDATE_DELTA, (update_now - last_update));
				}

				const auto roughness = get_direct<double>(sprite::e_double::ROUGHNESS);
				const auto affected_cam = get_direct<bool>(sprite::e_boolean::AFFECTED_BY_CAM);
				int nowgo = 0;

				// delayed work to do (auto-clean)
				if (auto f = get_direct<sprite::functional>(sprite::e_tie_functional::DELAYED_WORK_AUTODEL); f) {
					f(Tools::Any{});
					set<sprite::functional>(sprite::e_tie_functional::DELAYED_WORK_AUTODEL, [](auto) {});
				}


				if (!is_eq(sprite::e_integer::COLLISION_MODE, static_cast<int>(sprite::e_collision_mode_cast::COLLISION_STATIC)) &&
					!is_eq(sprite::e_integer::COLLISION_MODE, static_cast<int>(sprite::e_collision_mode_cast::COLLISION_NONE))) {

					nowgo = easy_collision.process_result();

					double sum_dx = 0.0;
					double sum_dy = 0.0;

					if (nowgo & static_cast<int>(sprite::e_direction::SOUTH)) {
						if (get_direct<double>(sprite::e_double_readonly::SPEED_Y) <= 0.0) {
							set(sprite::e_double_readonly::SPEED_Y, 0.0);// sprite::minimum_sprite_accel_collision* get_direct<double>(sprite::e_double::ELASTICITY_Y));
							//set(sprite::e_double::ACCELERATION_Y, sprite::minimum_sprite_accel_collision);
						}
						sum_dy = fabs(easy_collision.dy_max);
					}
					else if (nowgo & static_cast<int>(sprite::e_direction::NORTH)) {
						if (get_direct<double>(sprite::e_double_readonly::SPEED_Y) >= 0.0) {
							set(sprite::e_double_readonly::SPEED_Y, 0.0); //-sprite::minimum_sprite_accel_collision * get_direct<double>(sprite::e_double::ELASTICITY_Y));
							//set(sprite::e_double::ACCELERATION_Y, -sprite::minimum_sprite_accel_collision);
						}
						sum_dy = -fabs(easy_collision.dy_max);
					}

					if (nowgo & static_cast<int>(sprite::e_direction::EAST)) {
						if (get_direct<double>(sprite::e_double_readonly::SPEED_X) <= 0.0) {
							set(sprite::e_double_readonly::SPEED_X, 0.0); // sprite::minimum_sprite_accel_collision * get_direct<double>(sprite::e_double::ELASTICITY_X));
							//set(sprite::e_double::ACCELERATION_X, sprite::minimum_sprite_accel_collision);
						}
						sum_dx = fabs(easy_collision.dx_max);
					}
					else if (nowgo & static_cast<int>(sprite::e_direction::WEST)) {
						if (get_direct<double>(sprite::e_double_readonly::SPEED_X) >= 0.0) {
							set(sprite::e_double_readonly::SPEED_X, 0.0); //-sprite::minimum_sprite_accel_collision * get_direct<double>(sprite::e_double::ELASTICITY_X));
							//set(sprite::e_double::ACCELERATION_X, -sprite::minimum_sprite_accel_collision);
						}
						sum_dx = -fabs(easy_collision.dx_max);
					}

					const auto targ_posx_c = get_direct<double>(sprite::e_double::TARG_POSX);
					const auto targ_posy_c = get_direct<double>(sprite::e_double::TARG_POSY);
					const auto speed_x_c = get_direct<double>(sprite::e_double_readonly::SPEED_X);
					const auto speed_y_c = get_direct<double>(sprite::e_double_readonly::SPEED_Y);
					const auto accel_x = get_direct<double>(sprite::e_double::ACCELERATION_X);
					const auto accel_y = get_direct<double>(sprite::e_double::ACCELERATION_Y);


					set<double>(sprite::e_double::TARG_POSX, targ_posx_c + speed_x_c + sum_dx);
					set<double>(sprite::e_double_readonly::PROJECTED_POSX, targ_posx_c + speed_x_c + 2.0 * sum_dx);

					// has to go west and accel help? good, else nah
					//if ((nowgo & static_cast<int>(sprite::e_direction::WEST) && accel_x <= 0.0) || (nowgo & static_cast<int>(sprite::e_direction::EAST) && accel_x >= 0.0)) {
					set(sprite::e_double_readonly::SPEED_X, (speed_x_c + get_direct<double>(sprite::e_double::ACCELERATION_X)) * roughness * get_direct<double>(sprite::e_double::ELASTICITY_X));
					//}
					
					set<double>(sprite::e_double::TARG_POSY, targ_posy_c + speed_y_c + sum_dy);
					set<double>(sprite::e_double_readonly::PROJECTED_POSY, targ_posy_c + speed_y_c + 2.0 * sum_dy);

					// has to go north and accel help? good, else nah
					//if ((nowgo & static_cast<int>(sprite::e_direction::NORTH) && accel_y <= 0.0) || (nowgo & static_cast<int>(sprite::e_direction::SOUTH) && accel_y >= 0.0)) {
					set(sprite::e_double_readonly::SPEED_Y, (speed_y_c + get_direct<double>(sprite::e_double::ACCELERATION_Y)) * roughness * get_direct<double>(sprite::e_double::ELASTICITY_Y));
					//}



					/*if (auto spr = (get_direct<double>(sprite::e_double::SPEED_ROTATION])(); spr != 0.0) {
						double act = (get_direct<double>(sprite::e_double::TARG_ROTATION])();
						get_direct<double>(sprite::e_double::TARG_ROTATION] = [spr,act] {return act + spr; };
					}*/
				}

				else { // read only
					set<double>(sprite::e_double_readonly::PROJECTED_POSX, get_direct<double>(sprite::e_double::TARG_POSX));
					set<double>(sprite::e_double_readonly::PROJECTED_POSY, get_direct<double>(sprite::e_double::TARG_POSY));
				}


				{
					std::array<double, 4> m = { 0.0 };
					bool is_mouse_pressed = false;
					bool mouse_collide = false;
					bool avoid_trigger = false;
					bool invalid_nomove = !get_direct<bool>(sprite::e_boolean_readonly::COLLISION_COLLIDED);


					double mouse_b4[2];

					mouse_b4[0] = get_direct<double>(sprite::e_double_readonly::MOUSE_CLICK_LAST_X);
					mouse_b4[1] = get_direct<double>(sprite::e_double_readonly::MOUSE_CLICK_LAST_Y);

					if (conf.has("mouse", "press_count")) {

						is_mouse_pressed = conf.get_as<unsigned>("mouse", "press_count") > 0;

						if (affected_cam) {
							m[0] = conf.get_as<double>("mouse", "x");
							m[1] = conf.get_as<double>("mouse", "y");
						}
						else {
							m[0] = conf.get_as<double>("mouse", "rx");
							m[1] = conf.get_as<double>("mouse", "ry");
						}

						m[2] = (m[0] - get_direct<double>(sprite::e_double::TARG_POSX)) / (0.5 * get_direct<double>(sprite::e_double::SCALE_X) * get_direct<double>(sprite::e_double::SCALE_G));
						m[3] = (m[1] - get_direct<double>(sprite::e_double::TARG_POSY)) / (0.5 * get_direct<double>(sprite::e_double::SCALE_Y) * get_direct<double>(sprite::e_double::SCALE_G));

						mouse_collide = (
							(fabs(m[2]) < 1.0) &&
							(fabs(m[3]) < 1.0)
							);
					}
					sprite::e_tie_functional last_state = get_direct<sprite::e_tie_functional>(sprite::e_tief_readonly::LAST_STATE);
					sprite::e_tie_functional new_state{};

					const bool old_was_mouse = (static_cast<int>(last_state) >= static_cast<int>(sprite::e_tie_functional::_MOUSE_BEGIN)) && (static_cast<int>(last_state) <= static_cast<int>(sprite::e_tie_functional::_MOUSE_END));

					if (mouse_collide) {
						if (is_mouse_pressed) {
							new_state = sprite::e_tie_functional::COLLISION_MOUSE_CLICK;
							avoid_trigger = get_direct<bool>(sprite::e_boolean_readonly::COLLISION_MOUSE_CLICK) || !get_direct<bool>(sprite::e_boolean_readonly::COLLISION_COLLIDED);
						}
						else if (last_state == sprite::e_tie_functional::COLLISION_MOUSE_CLICK) {
							new_state = sprite::e_tie_functional::COLLISION_MOUSE_UNCLICK;
						}
						else {
							new_state = sprite::e_tie_functional::COLLISION_MOUSE_ON;
						}
					}
					else if (easy_collision.was_col) {
						new_state = sprite::e_tie_functional::COLLISION_COLLIDED_OTHER;
					}
					else {
						new_state = sprite::e_tie_functional::COLLISION_NONE;
					}

					const bool new_is_mouse = (static_cast<int>(new_state) >= static_cast<int>(sprite::e_tie_functional::_MOUSE_BEGIN)) && (static_cast<int>(new_state) <= static_cast<int>(sprite::e_tie_functional::_MOUSE_END));

					set(sprite::e_boolean_readonly::INVALIDATE_MOUSE_NOMOVE, get_direct<bool>(sprite::e_boolean_readonly::INVALIDATE_MOUSE_NOMOVE) || (fabs(m[0] - mouse_b4[0]) >= sprite::move_accept_move_max_as_none) || (fabs(m[1] - mouse_b4[1]) >= sprite::move_accept_move_max_as_none));

					if (new_state != last_state) {

						Tools::Any ref;
						if (new_is_mouse) ref = m;

						// if was mouse, hm
						if (old_was_mouse && !new_is_mouse) {
							if (auto f = get_direct<sprite::functional>(sprite::e_tie_functional::COLLISION_MOUSE_OFF); f) f(ref);
							this->mouse_event(sprite::e_tie_functional::COLLISION_MOUSE_OFF, ref);
						}

						if (!invalid_nomove) {
							switch (new_state) {
							case sprite::e_tie_functional::COLLISION_MOUSE_CLICK:
							{
								if (!get_direct<bool>(sprite::e_boolean_readonly::INVALIDATE_MOUSE_NOMOVE)) {
									if (auto f = get_direct<sprite::functional>(sprite::e_tie_functional::COLLISION_MOUSE_CLICK_NOMOVE); f) f(ref);
									this->mouse_event(sprite::e_tie_functional::COLLISION_MOUSE_CLICK_NOMOVE, ref);
								}

								set(sprite::e_double_readonly::MOUSE_CLICK_LAST_X, m[0]);
								set(sprite::e_double_readonly::MOUSE_CLICK_LAST_Y, m[1]);
								set(sprite::e_boolean_readonly::INVALIDATE_MOUSE_NOMOVE, false);
							}
							break;
							case sprite::e_tie_functional::COLLISION_MOUSE_UNCLICK:
							{
								if (!get_direct<bool>(sprite::e_boolean_readonly::INVALIDATE_MOUSE_NOMOVE)) {
									if (auto f = get_direct<sprite::functional>(sprite::e_tie_functional::COLLISION_MOUSE_UNCLICK_NOMOVE); f) f(ref);
									this->mouse_event(sprite::e_tie_functional::COLLISION_MOUSE_UNCLICK_NOMOVE, ref);
								}

								set(sprite::e_double_readonly::MOUSE_CLICK_LAST_X, m[0]);
								set(sprite::e_double_readonly::MOUSE_CLICK_LAST_Y, m[1]);
								set(sprite::e_boolean_readonly::INVALIDATE_MOUSE_NOMOVE, false);
							}
							break;
							}
						}

						set<sprite::e_tie_functional>(sprite::e_tief_readonly::LAST_STATE, new_state);

						if (auto f = get_direct<sprite::functional>(new_state); !avoid_trigger && f) f(ref);
						this->mouse_event(new_state, ref);
					}


					set(sprite::e_boolean_readonly::COLLISION_COLLIDED, mouse_collide);
					set(sprite::e_boolean_readonly::COLLISION_MOUSE_PRESSED, is_mouse_pressed);
					if (mouse_collide) {
						set(sprite::e_boolean_readonly::COLLISION_MOUSE_CLICK, is_mouse_pressed);
					}

				}

				double scale_x, scale_y, targx, targy;

				scale_x = get_direct<double>(sprite::e_double::SCALE_G) * get_direct<double>(sprite::e_double::SCALE_X);
				scale_y = get_direct<double>(sprite::e_double::SCALE_G) * get_direct<double>(sprite::e_double::SCALE_Y);
				targx = get_direct<double>(sprite::e_double::TARG_POSX);
				targy = get_direct<double>(sprite::e_double::TARG_POSY);

				if (!affected_cam) { // if not affected, inverse effect
					Interface::classic_2d& cpy = easy_collision.latest_camera;

					targx /= cpy.sx;
					targy /= cpy.sy;
					targx += cpy.x;
					targy += cpy.y;
					scale_x /= cpy.sx;
					scale_y /= cpy.sy;
				}

				set(sprite::e_double_readonly::REALISTIC_RESULT_POSX, targx);
				set(sprite::e_double_readonly::REALISTIC_RESULT_POSY, targy);
				set(sprite::e_double_readonly::REALISTIC_RESULT_SCALE_X, scale_x);
				set(sprite::e_double_readonly::REALISTIC_RESULT_SCALE_Y, scale_y);

				easy_collision.setup(targx, targy, scale_x, scale_y);

				think_task(nowgo);
			}

			// implementation

			template Tools::FastFunction<std::string>;
			template Tools::FastFunction<double>;
			template Tools::FastFunction<bool>;
			template Tools::FastFunction<int>;
			template Tools::FastFunction<Interface::Color>;
			template Tools::FastFunction<uintptr_t>;
			template Tools::FastFunction<sprite::e_tie_functional>;
			template Tools::FastFunction<sprite::functional>;

			template Tools::Resource<Sprite_Base>;

			template Tools::SuperFunctionalMap<std::string>;
			template Tools::SuperFunctionalMap<double>;
			template Tools::SuperFunctionalMap<bool>;
			template Tools::SuperFunctionalMap<int>;
			template Tools::SuperFunctionalMap<Interface::Color>;
			template Tools::SuperFunctionalMap<uintptr_t>;
			template Tools::SuperFunctionalMap<sprite::e_tie_functional>;
			template Tools::SuperFunctionalMap<sprite::functional>;

			template Tools::SuperMap<std::function<std::string(void)>>;
			template Tools::SuperMap<std::function<double(void)>>;
			template Tools::SuperMap<std::function<bool(void)>>;
			template Tools::SuperMap<std::function<int(void)>>;
			template Tools::SuperMap<std::function<Interface::Color(void)>>;
			template Tools::SuperMap<std::function<uintptr_t(void)>>;
			template Tools::SuperMap<std::function<sprite::e_tie_functional(void)>>;
			template Tools::SuperMap<std::function<sprite::functional(void)>>;

			template Tools::SuperPair<std::string>;
			template Tools::SuperPair<double>;
			template Tools::SuperPair<bool>;
			template Tools::SuperPair<int>;
			template Tools::SuperPair<Interface::Color>;
			template Tools::SuperPair<uintptr_t>;
			template Tools::SuperPair<sprite::e_tie_functional>;
			template Tools::SuperPair<sprite::functional>;

		}
	}
}