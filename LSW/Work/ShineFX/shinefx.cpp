#include "shinefx.h"

namespace LSW {
	namespace v5 {
		namespace Work {

			void ShineFX::check_internal()
			{
				const double delta = get_direct<double>(shinefx::e_double::FRAMES_PER_SECOND); // delta t, 1/t = sec
				const double scaleg = get_direct<double>(shinefx::e_double::TIED_SIZE_TO_DISPLAY_PROPORTION);
				const std::chrono::milliseconds last_time = get_direct<std::chrono::milliseconds>(shinefx::e_chronomillis_readonly::LAST_FRAME);

				if (delta > 0.0) { // if delta <= 0, inf
					std::chrono::milliseconds delta_tr = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(1.0 / delta));

					if (MILLI_NOW - last_time > delta_tr) {
						set<std::chrono::milliseconds>(shinefx::e_chronomillis_readonly::LAST_FRAME, MILLI_NOW);
					}
					else return; // no task
				}

				Interface::Target targ{ get_direct<uintptr_t>(sprite::e_uintptrt::INDEX_TARGET_IN_USE) };
				auto btarg = targ.get();

				// tied resolution
				{
					const auto delta_t = get_direct<std::chrono::milliseconds>(shinefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION);

					if (scaleg > 0.0 && (std::chrono::system_clock::now().time_since_epoch() > delta_t)) {
						set(shinefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() + shinefx::default_delta_t_frame_delay));

						buffer.copy_attributes(btarg, true, scaleg);
					}
				}


				// update bitmap
				const double delta_rad = get_direct<double>(shinefx::e_double::EACH_SIZE_RAD);
				const double time_now = al_get_time() * get_direct<double>(shinefx::e_double::SPEED_ROTATION_TIME);
				const Interface::Color foreground_color = get_direct<Interface::Color>(shinefx::e_color::FOREGROUND);
				const Interface::Color background_color = get_direct<Interface::Color>(shinefx::e_color::BACKGROUND);
				const double cxx = get_direct<double>(shinefx::e_double::CENTER_X);
				const double cyy = get_direct<double>(shinefx::e_double::CENTER_Y);

				if (buffer) { // just to be sure
					buffer.set_as_target();

					Interface::Camera cpy;
					cpy.classic_transform(buffer, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
					cpy.apply();

					background_color.clear_to_this();

					for (double radd = 0; radd < ALLEGRO_PI * 2; radd += delta_rad) {
						Interface::Vertex v[] = {
							{
								static_cast<float>(cxx),
								static_cast<float>(cyy),
								foreground_color
							}, //center
							{ 
								static_cast<float>(2.0 * cos(radd + time_now * 0.2)),
								static_cast<float>(2.0 * sin(radd + time_now * 0.2)),
								foreground_color
							}, //top left
							{ 
								static_cast<float>(2.0 * cos(radd + (delta_rad * 0.5) + time_now * 0.2)),
								static_cast<float>(2.0 * sin(radd + (delta_rad * 0.5) + time_now * 0.2)),
								foreground_color
							} //top right
						};

						al_draw_prim(v, NULL, NULL, 0, 3, ALLEGRO_PRIM_TRIANGLE_LIST);
					}

					btarg.set_as_target();
				}
				else throw Handling::Abort(__FUNCSIG__, "Cannot draw ShineFX because it couldn't get a valid bitmap somehow!", Handling::abort::abort_level::GIVEUP);
			}

			void ShineFX::draw_task(Interface::Camera& c)
			{
				check_internal();
				c.apply();

				if (!buffer) throw Handling::Abort(__FUNCSIG__, "Buffer null, can't draw!", Handling::abort::abort_level::GIVEUP);

				common_bitmap_draw_task(buffer);
			}

			ShineFX::ShineFX() : Sprite_Base()
			{
				set<double>(shinefx::e_double_defaults);
				set<Interface::Color>(shinefx::e_color_defaults);
				set<std::chrono::milliseconds>(shinefx::e_chronomillis_defaults);

				set(shinefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

			ShineFX::ShineFX(const ShineFX& other)
			{
				*this = other;
			}

			ShineFX::ShineFX(ShineFX&& other) noexcept
			{
				*this = std::move(other);
			}

			void ShineFX::operator=(const ShineFX& oth)
			{
				this->Sprite_Base::operator=(oth);

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(oth.get<std::chrono::milliseconds>());

				buffer = oth.buffer;

				set(shinefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

			void ShineFX::operator=(ShineFX&& other) noexcept
			{
				this->Sprite_Base::operator=(std::move(other));

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(std::move(other.get<std::chrono::milliseconds>()));

				buffer = std::move(other.buffer);

				set(shinefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

			void ShineFX::clone(const ShineFX& other)
			{
				this->Sprite_Base::clone(other);

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(*other.get<std::chrono::milliseconds>());

				buffer = other.buffer;

				set(shinefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

		}
	}
}