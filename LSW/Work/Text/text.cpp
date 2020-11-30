#include "text.h"

namespace LSW {
	namespace v5 {
		namespace Work {

			void Text::_think_lines()
			{
				if (!fontt) return;

				const auto delta_t = get_direct<std::chrono::milliseconds>(text::e_chronomillis_readonly::LAST_UPDATE_STRING);
				const auto ups_val = get_direct<double>(text::e_double::UPDATES_PER_SECOND);
				const auto max_length_line_size = get_direct<double>(text::e_double::MAX_TEXT_LENGTH_SIZE) * fontt.get_line_height() / (get_direct<double>(sprite::e_double::SCALE_G) * get_direct<double>(sprite::e_double::SCALE_X));
				const auto max_length_total = get_direct<int>(text::e_integer::TOTAL_TEXT_MAX_LENGTH);
				const auto max_length_line = get_direct<int>(text::e_integer::LINE_MAX_LENGTH);
				const auto max_lines = get_direct<int>(text::e_integer::MAX_LINES_AMOUNT);
				const auto scroll_on_line_limit = get_direct<bool>(text::e_boolean::SCROLL_INSTEAD_OF_MAX_LEN_SIZE_BLOCK);
				const auto scroll_movement = get_direct<double>(text::e_double::SCROLL_SPEED);
				const auto scroll_offset = get_direct<int>(text::e_integer::SCROLL_TICKS_PAUSED_SIDES);

				//TOTAL_TEXT_MAX_LENGTH, LINE_MAX_LENGTH, MAX_LINES_AMOUNT

				bool instantaneous = ups_val <= 0.0;

				// STRING
				if (instantaneous || std::chrono::system_clock::now().time_since_epoch() > delta_t) // do update string
				{
					//if (get_direct<std::string>(sprite::e_string::ID) == "HEYO MAYO!") std::cout << "UPDATE\n";
					set(text::e_chronomillis_readonly::LAST_UPDATE_STRING, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + std::chrono::milliseconds(instantaneous ? 0 : (unsigned long long)(1000.0 / ups_val)));

					bool got_limited = false;

					auto p_str = get_direct<Tools::Cstring>(text::e_cstring::STRING);
					if (max_length_total && p_str.size() > static_cast<size_t>(max_length_total)) {
						p_str = p_str.substr(0, max_length_total);
						if (!get_direct<Tools::Cstring&>(text::e_cstring::STRING)->is_function()) set(text::e_cstring::STRING, p_str);
						got_limited = true;
					}

					size_t arrp = 0;

					size_t pos = 0;
					while (((pos = p_str.find('\n')) != std::string::npos) || (max_length_line && p_str.size() > static_cast<size_t>(max_length_line)) || (max_length_line_size > 0.0 && fontt.get_width(p_str.s_str().c_str()) > max_length_line_size)) {

						auto line = p_str.substr(0, pos);

						if (max_length_line && line.size_utf8() > static_cast<size_t>(max_length_line)) {
							if (scroll_movement == 0.0) {
								while (line.size_utf8() > static_cast<size_t>(max_length_line)) line.pop_utf8();
							}
							else {
								size_t cut = (static_cast<size_t>((al_get_time()) * 3.0)) % (line.size() + static_cast<size_t>(scroll_offset) * 2) - static_cast<size_t>(scroll_offset);

								if (cut <= static_cast<size_t>(scroll_offset)) cut = 0;
								else cut -= static_cast<size_t>(scroll_offset);
								// from now, [0, len + scroll_offset)

								if (cut >= line.size()) cut = line.size() - 1;
								// done

								//const double targ_siz = max_length_line_size; title.get_direct<double>(text::e_double::MAX_TEXT_LENGTH_SIZE);
								//double curr_sx = title.get_direct<double>(sprite::e_double::SCALE_G) * title.get_direct<double>(sprite::e_double::SCALE_X);
								//while (extra_font_ref.get_width(cpy.c_str() + cut) / (extra_font_ref.get_line_height() / curr_sx) > targ_siz) cpy.pop_back();

								for (size_t k = 0; k < cut && line.size_utf8() > static_cast<size_t>(max_length_line); k++) line.pop_front_utf8();
								while (line.size_utf8() > static_cast<size_t>(max_length_line)) line.pop_utf8();
							}
							pos = line.size();
							if (pos) pos--; // last char is erased if not because of + 1 later there
						}
						if (max_length_line_size > 0.0 && !scroll_on_line_limit && fontt.get_width(line.s_str().c_str()) > max_length_line_size) {
							if (scroll_movement == 0.0) {
								while (fontt.get_width(line.s_str().c_str()) > max_length_line_size) line.pop_utf8();
							}
							else {
								size_t cut = (static_cast<size_t>((al_get_time()) * 3.0)) % (line.size() + static_cast<size_t>(scroll_offset) * 2) - static_cast<size_t>(scroll_offset);

								if (cut <= static_cast<size_t>(scroll_offset)) cut = 0;
								else cut -= static_cast<size_t>(scroll_offset);
								// from now, [0, len + scroll_offset)

								if (cut >= line.size()) cut = line.size() - 1;
								// done

								//const double targ_siz = max_length_line_size; title.get_direct<double>(text::e_double::MAX_TEXT_LENGTH_SIZE);
								//double curr_sx = title.get_direct<double>(sprite::e_double::SCALE_G) * title.get_direct<double>(sprite::e_double::SCALE_X);
								//while (extra_font_ref.get_width(cpy.c_str() + cut) / (extra_font_ref.get_line_height() / curr_sx) > targ_siz) cpy.pop_back();

								for (size_t k = 0; k < cut && fontt.get_width(line.s_str().c_str()) > max_length_line_size; k++) line.pop_front_utf8();
								while (fontt.get_width(line.s_str().c_str()) > max_length_line_size) line.pop_utf8();
							}
							pos = line.size();
							if (pos) pos--; // last char is erased if not because of + 1 later there
						}

						if (arrp < _buf_lines.size()) {
							_buf_lines[arrp] = std::move(line);
						}
						else if (!max_lines || arrp < static_cast<size_t>(max_lines)) _buf_lines.push_back(std::move(line));
						else {
							got_limited = true;
							break;
						}
						arrp++;

						if (pos >= p_str.size()) {
							p_str.clear();
							break;
						}
						p_str = std::move(p_str.substr(pos + 1));
					}

					if (p_str.size() > 0){
						if (arrp < static_cast<size_t>(max_lines) || !max_lines) {
							if (arrp < _buf_lines.size()) {
								_buf_lines[arrp] = std::move(p_str);
							}
							else _buf_lines.push_back(std::move(p_str));
							arrp++;
						}
						else {
							got_limited = true;
						}
					}
					else if (arrp == static_cast<size_t>(max_lines)) {
						got_limited = true;
					}

					_buf_lines.resize(arrp);

					if (max_length_line_size > 0.0 && scroll_on_line_limit) {
						for (auto& i : _buf_lines) {
							//while (fontt.get_width(i.s_str().c_str()) > max_length_line_size) i.pop_front_utf8();

							if (max_length_line && i.size_utf8() > static_cast<size_t>(max_length_line)) {
								if (scroll_movement == 0.0) {
									while (i.size_utf8() > static_cast<size_t>(max_length_line)) i.pop_utf8();
								}
								else {
									size_t cut = (static_cast<size_t>((al_get_time()) * 3.0)) % (i.size() + static_cast<size_t>(scroll_offset) * 2) - static_cast<size_t>(scroll_offset);

									if (cut <= static_cast<size_t>(scroll_offset)) cut = 0;
									else cut -= static_cast<size_t>(scroll_offset);
									// from now, [0, len + scroll_offset)

									if (cut >= i.size()) cut = i.size() - 1;
									// done

									//const double targ_siz = max_length_line_size; title.get_direct<double>(text::e_double::MAX_TEXT_LENGTH_SIZE);
									//double curr_sx = title.get_direct<double>(sprite::e_double::SCALE_G) * title.get_direct<double>(sprite::e_double::SCALE_X);
									//while (extra_font_ref.get_width(cpy.c_str() + cut) / (extra_font_ref.get_line_height() / curr_sx) > targ_siz) cpy.pop_back();

									for (size_t k = 0; k < cut && i.size_utf8() > static_cast<size_t>(max_length_line); k++) i.pop_front_utf8();
									while (i.size_utf8() > static_cast<size_t>(max_length_line)) i.pop_utf8();
								}
							}
							if (max_length_line_size > 0.0 && fontt.get_width(i.s_str().c_str()) > max_length_line_size) {
								if (scroll_movement == 0.0) {
									while (fontt.get_width(i.s_str().c_str()) > max_length_line_size) i.pop_utf8();
								}
								else {
									size_t cut = (static_cast<size_t>((al_get_time()) * 3.0)) % (i.size() + static_cast<size_t>(scroll_offset) * 2) - static_cast<size_t>(scroll_offset);

									if (cut <= static_cast<size_t>(scroll_offset)) cut = 0;
									else cut -= static_cast<size_t>(scroll_offset);
									// from now, [0, len + scroll_offset)

									if (cut >= i.size()) cut = i.size() - 1;
									// done

									//const double targ_siz = max_length_line_size; title.get_direct<double>(text::e_double::MAX_TEXT_LENGTH_SIZE);
									//double curr_sx = title.get_direct<double>(sprite::e_double::SCALE_G) * title.get_direct<double>(sprite::e_double::SCALE_X);
									//while (extra_font_ref.get_width(cpy.c_str() + cut) / (extra_font_ref.get_line_height() / curr_sx) > targ_siz) cpy.pop_back();

									for (size_t k = 0; k < cut && fontt.get_width(i.s_str().c_str()) > max_length_line_size; k++) i.pop_front_utf8();
									while (fontt.get_width(i.s_str().c_str()) > max_length_line_size) i.pop_utf8();
								}
							}

						}
					}

					set<uintptr_t>(text::e_uintptrt_readonly::LAST_CALCULATED_LINE_AMOUNT, _buf_lines.size());
					set(text::e_boolean_readonly::REACHED_LIMIT, got_limited);
				}
			}
			
			void Text::_draw_text(Interface::Camera& ruler)
			{
				if (!_buf_lines.size() || !fontt) return; // nothing to draw

				double off_x = 0.0;
				double off_y = 0.0;

				// auto looks smaller, maybe good?
				const auto s_dist_x = get_direct<double>(text::e_double::SHADOW_DISTANCE_X);
				const auto s_dist_y = get_direct<double>(text::e_double::SHADOW_DISTANCE_Y);
				const auto s_col = get_direct<Interface::Color>(text::e_color::SHADOW_COLOR);
				const auto n_col = get_direct<Interface::Color>(sprite::e_color::COLOR);
				const auto mode = get_direct<int>(text::e_integer::STRING_MODE);
				const auto mode_y = get_direct<int>(text::e_integer::STRING_Y_MODE);
				const auto scale_g = get_direct<double>(sprite::e_double::SCALE_G);
				const auto scale_x = get_direct<double>(sprite::e_double::SCALE_X);
				const auto scale_y = get_direct<double>(sprite::e_double::SCALE_Y);
				const auto posx = get_direct<double>(sprite::e_double_readonly::POSX);
				const auto posy = get_direct<double>(sprite::e_double_readonly::POSY);
				const auto font_siz = fontt.get_line_height();
				const auto lineadj = get_direct<double>(text::e_double::LINE_ADJUST);
				const auto force_color = get_direct<bool>(text::e_boolean::USE_COLOR_INSTEAD_OF_AUTO);

				const double t_rotation_rad = get_direct<double>(sprite::e_double_readonly::ROTATION) * ALLEGRO_PI / 180.0;
				double p_rotation_rad = 0.0;

				if (font_siz <= 0) throw Handling::Abort(__FUNCSIG__, "Invalid font size! Please fix text::e_integer::FONT_SIZE.", Handling::abort::abort_level::GIVEUP);

				{
					auto fl = get_direct<Sprite_Base>(text::e_sprite_ref::FOLLOWING); // safer
					if (get_direct<bool>(sprite::e_boolean::AFFECTED_BY_CAM) && !fl.is_eq_s<bool>(sprite::e_boolean::AFFECTED_BY_CAM, *this)) {
						fl.get(sprite::e_double_readonly::REALISTIC_RESULT_POSX, off_x);
						fl.get(sprite::e_double_readonly::REALISTIC_RESULT_POSY, off_y);
					}
					else {
						fl.get(sprite::e_double_readonly::POSX, off_x);
						fl.get(sprite::e_double_readonly::POSY, off_y);
					}
					fl.get(sprite::e_double_readonly::ROTATION, p_rotation_rad);
					//set(sprite::e_boolean::AFFECTED_BY_CAM, fl.get_direct<bool>(sprite::e_boolean::AFFECTED_BY_CAM));
					p_rotation_rad *= ALLEGRO_PI / 180.0;
				}

				const double rotation_rad = t_rotation_rad + p_rotation_rad;

				const bool should_care_about_shadow = (s_dist_x != 0.0 || s_dist_y != 0.0);

				double pos_now[2];

				pos_now[0] = (((posx)*cos(p_rotation_rad)) - ((posy)*sin(p_rotation_rad)) + off_x); // transformed to sprite's coords
				pos_now[1] = (((posy)*cos(p_rotation_rad)) + ((posx)*sin(p_rotation_rad)) + off_y); // transformed to sprite's coords



				auto& cls = ruler.get_classic();
				double csx, csy;

				csx = scale_g * scale_x / font_siz;
				csy = scale_g * scale_y / font_siz;
				//csx = (((csx)*cos(p_rotation_rad)) - ((csy)*sin(p_rotation_rad)));
				//csy = (((csy)*cos(p_rotation_rad)) + ((csx)*sin(p_rotation_rad)));


				Interface::Camera shadow_cam;// = ruler;

				if (should_care_about_shadow) {
					shadow_cam = ruler;
					shadow_cam.classic_transform((cls.x - pos_now[0] - s_dist_x) / csx, (cls.y - pos_now[1] - s_dist_y) / csy, cls.sx * csx, cls.sy * csy, cls.rot + rotation_rad);
				}
				ruler.classic_transform((cls.x - pos_now[0]) / csx, (cls.y - pos_now[1]) / csy, cls.sx * csx, cls.sy * csy, cls.rot + rotation_rad);

				ruler.apply();


				const double compensate = 0.5 * fontt.get_line_height();
				const double height = lineadj * fontt.get_line_height();
				const double y_offset = mode_y == 0 ? 0 : (mode_y == static_cast<int>(text::e_text_y_modes::CENTER) ? (0.5 * height * (_buf_lines.size() - 1)) : (height * (_buf_lines.size() - 1)));

				for (size_t o = 0; o < _buf_lines.size(); o++) {
					const auto i = _buf_lines[o];

					if (should_care_about_shadow) {
						shadow_cam.apply();
						fontt.draw(s_col, 0.0, -compensate + height * o - y_offset, mode, i.s_str().c_str());
						ruler.apply();
					}
					if (force_color) {
						fontt.draw(n_col, 0.0, -compensate + height * o - y_offset, mode, i.s_str().c_str());
					}
					else {
						fontt.draw(0.0, -compensate + height * o - y_offset, mode, i);
					}
				}


				if (is_eq(sprite::e_boolean::DRAW_DOT, true)) {
					al_draw_filled_circle(
						/* X1: */ 0.0,
						/* Y1: */ 0.0,
						/* SCL */ 2.0f * font_siz,
						al_map_rgba(0, 45, 90, 90));
				}
			}
			
			void Text::draw_task(Interface::Camera& ruler)
			{
				if (!fontt) throw Handling::Abort(__FUNCSIG__, "No Font texture!", Handling::abort::abort_level::GIVEUP);

				const auto delta_t = get_direct<std::chrono::milliseconds>(text::e_chronomillis_readonly::LAST_UPDATE_BITMAP);
				const auto use_buffer = get_direct<bool>(text::e_boolean::USE_BITMAP_BUFFER);
				const auto scale_buff = get_direct<double>(text::e_double::BUFFER_SCALE_RESOLUTION);
				const auto ups_val = get_direct<double>(text::e_double::UPDATES_PER_SECOND);
				const auto last_update = get_direct<double>(sprite::e_double_readonly::LAST_UPDATE); // al_get_time

				//if (ups_val <= 0.0)	throw Handling::Abort(__FUNCSIG__, "Invalid UPS value! (UPDATES_PER_SECOND must be >= 0.0)", Handling::abort::abort_level::GIVEUP);

				bool instantaneous = ups_val <= 0.0;


				if (instantaneous || std::chrono::system_clock::now().time_since_epoch() > delta_t) // do update string
				{
					Interface::Target targ(get_direct<uintptr_t>(sprite::e_uintptrt::INDEX_TARGET_IN_USE));
					set(text::e_chronomillis_readonly::LAST_UPDATE_BITMAP, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + std::chrono::milliseconds(instantaneous ? 0 : (unsigned long long)(1000.0 / ups_val)));

					if (use_buffer) {
						if (scale_buff <= 0.0 || !buff.copy_attributes(targ.get(), true, scale_buff)) { // ensure loaded
							throw Handling::Abort(__FUNCSIG__, "Failed to copy reference's attributes and generate bitmap.", Handling::abort::abort_level::GIVEUP);
						}
					}


					if (al_get_time() - last_update >= sprite::maximum_time_between_collisions) {
						_think_lines();
					}


					if (use_buffer) {
						if (buff.empty()) return;

						buff.set_as_target();
						al_clear_to_color(Interface::Color(0, 0, 0, 0));
						auto cpy = ruler;
						cpy.classic_update(buff);
						_draw_text(cpy);

						targ.apply();
					}
					else buff.reset();
				}

				if (!use_buffer) {
					_draw_text(ruler);
				}
				else if (!buff.empty()) { // same as Block
					ruler.classic_transform(0.0, 0.0, 1.0, 1.0, 0.0);
					ruler.apply();

					buff.draw(0, 0, buff.get_width(), buff.get_height(), -1.0f, -1.0f, 2.0f, 2.0f);
				}

			}
						
			void Text::think_task(const int u)
			{
				_think_lines(); // this way so if drawing thread can also do this if needed
			}
						
			Text::Text() : Sprite_Base()
			{
				set<std::chrono::milliseconds>(text::e_chronomillis_defaults);
				set<Tools::Cstring>(text::e_string_defaults);
				set<Sprite_Base>(text::e_sprite_ref_defaults);
				set<double>(text::e_double_defaults);
				set<Interface::Color>(text::e_color_defaults);
				set<int>(text::e_integer_defaults);
				set<bool>(text::e_boolean_defaults);
				set<uintptr_t>(text::e_uintptrt_defaults);

				set(Work::text::e_color::SHADOW_COLOR, Interface::Color(0, 0, 0));
				set(text::e_chronomillis_readonly::LAST_UPDATE_BITMAP, MILLI_NOW);
				set(Work::sprite::e_integer::COLLISION_MODE, static_cast<int>(Work::sprite::e_collision_mode_cast::COLLISION_STATIC));
			}
						
			Text::Text(const Text& other)
			{
				*this = other;
			}
						
			Text::Text(Text&& other)
			{
				*this = std::move(other);
			}
						
			void Text::operator=(const Text& oth)
			{
				this->Sprite_Base::operator=(oth);

				fontt = oth.fontt;

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(oth.get<std::chrono::milliseconds>());
				set<Tools::Cstring>(oth.get<Tools::Cstring>());
				set<Sprite_Base>(oth.get<Sprite_Base>());
			}
			
			void Text::operator=(Text&& oth)
			{
				this->Sprite_Base::operator=(std::move(oth));

				fontt = std::move(oth.fontt);

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(std::move(oth.get<std::chrono::milliseconds>()));
				set<Tools::Cstring>(std::move(oth.get<Tools::Cstring>()));
				set<Sprite_Base>(std::move(oth.get<Sprite_Base>()));
			}
			
			void Text::clone(const Text& oth)
			{
				this->Sprite_Base::clone(oth);

				fontt = oth.fontt;

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(*oth.get<std::chrono::milliseconds>());
				set<Tools::Cstring>(*oth.get<Tools::Cstring>());
				set<Sprite_Base>(*oth.get<Sprite_Base>());
			}
						
			void Text::set(const Interface::Font& f)
			{
				fontt = f;
			}

			bool Text::check_fit(Tools::Cstring p_str) const
			{
				if (!fontt) return false;

				const auto max_length_line_size = get_direct<double>(text::e_double::MAX_TEXT_LENGTH_SIZE) * fontt.get_line_height() / (get_direct<double>(sprite::e_double::SCALE_G) * get_direct<double>(sprite::e_double::SCALE_X));
				const auto max_length_total = get_direct<int>(text::e_integer::TOTAL_TEXT_MAX_LENGTH);
				const auto max_length_line = get_direct<int>(text::e_integer::LINE_MAX_LENGTH);
				const auto max_lines = get_direct<int>(text::e_integer::MAX_LINES_AMOUNT);
				const auto scroll_on_line_limit = get_direct<bool>(text::e_boolean::SCROLL_INSTEAD_OF_MAX_LEN_SIZE_BLOCK);

				if (max_length_total && p_str.size() > static_cast<size_t>(max_length_total)) return false;
				if (max_lines) {
					size_t num_lines = 1;
					for (size_t _p = 0; _p < p_str.size();) {
						if ((_p = p_str.substr(_p).find('\n')) != std::string::npos) {
							if (++num_lines > static_cast<size_t>(max_lines)) return false;
							_p++;
						}
					}
				}
				bool got_limited = false;

				size_t arrp = 0;

				size_t pos = 0;
				while (((pos = p_str.find('\n')) != std::string::npos) || (max_length_line && p_str.size() > static_cast<size_t>(max_length_line)) || (max_length_line_size > 0.0 && fontt.get_width(p_str.s_str().c_str()) > max_length_line_size)) {

					auto line = p_str.substr(0, pos);

					if (max_length_line && line.size_utf8() > static_cast<size_t>(max_length_line)) {
						while (line.size_utf8() > static_cast<size_t>(max_length_line)) line.pop_utf8();
						pos = line.size();
						if (pos) pos--; // last char is erased if not because of + 1 later there
					}
					if (max_length_line_size > 0.0 && !scroll_on_line_limit && fontt.get_width(line.s_str().c_str()) > max_length_line_size) {
						while (fontt.get_width(line.s_str().c_str()) > max_length_line_size) line.pop_utf8();
						pos = line.size();
						if (pos) pos--; // last char is erased if not because of + 1 later there
					}

					if (max_lines && arrp >= static_cast<size_t>(max_lines)) {
						return false;
					}
					arrp++;

					if (pos >= p_str.size()) {
						p_str.clear();
						break;
					}
					p_str = std::move(p_str.substr(pos + 1));
				}

				if (p_str.size() > 0) {
					if (max_lines && arrp >= static_cast<size_t>(max_lines)) {
						return false;
					}
				}

				return true;
			}		

			// implementation

			template Tools::FastFunction<std::chrono::milliseconds>;
			template Tools::FastFunction<Tools::Cstring>;
			template Tools::FastFunction<Sprite_Base>;
			template Tools::FastFunction<std::string>;
			template Tools::FastFunction<double>;
			template Tools::FastFunction<bool>;
			template Tools::FastFunction<int>;
			template Tools::FastFunction<Interface::Color>;
			template Tools::FastFunction<uintptr_t>;
			template Tools::FastFunction<sprite::e_tie_functional>;
			template Tools::FastFunction<sprite::functional>;

			template Tools::Resource<Text>;

			template Tools::SuperFunctionalMap<std::chrono::milliseconds>;
			template Tools::SuperFunctionalMap<Tools::Cstring>;
			template Tools::SuperFunctionalMap<Sprite_Base>;
			template Tools::SuperFunctionalMap<std::string>;
			template Tools::SuperFunctionalMap<double>;
			template Tools::SuperFunctionalMap<bool>;
			template Tools::SuperFunctionalMap<int>;
			template Tools::SuperFunctionalMap<Interface::Color>;
			template Tools::SuperFunctionalMap<uintptr_t>;
			template Tools::SuperFunctionalMap<sprite::e_tie_functional>;
			template Tools::SuperFunctionalMap<sprite::functional>;

			template Tools::SuperMap<std::function<std::chrono::milliseconds(void)>>;
			template Tools::SuperMap<std::function<Tools::Cstring(void)>>;
			template Tools::SuperMap<std::function<Sprite_Base(void)>>;
			template Tools::SuperMap<std::function<std::string(void)>>;
			template Tools::SuperMap<std::function<double(void)>>;
			template Tools::SuperMap<std::function<bool(void)>>;
			template Tools::SuperMap<std::function<int(void)>>;
			template Tools::SuperMap<std::function<Interface::Color(void)>>;
			template Tools::SuperMap<std::function<uintptr_t(void)>>;
			template Tools::SuperMap<std::function<sprite::e_tie_functional(void)>>;
			template Tools::SuperMap<std::function<sprite::functional(void)>>;

			template Tools::SuperPair<std::chrono::milliseconds>;
			template Tools::SuperPair<Tools::Cstring>;
			template Tools::SuperPair<Sprite_Base>;
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