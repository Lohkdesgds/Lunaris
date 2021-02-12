#include "bubblefx.h"

namespace LSW {
	namespace v5 {
		namespace Work {

			void BubbleFX::check_internal()
			{
				const double delta = get_direct<double>(bubblefx::e_double::FRAMES_PER_SECOND); // delta t, 1/t = sec
				const double scaleg = get_direct<double>(bubblefx::e_double::TIE_SIZE_TO_DISPLAY_PROPORTION);
				const std::chrono::milliseconds last_time = get_direct<std::chrono::milliseconds>(bubblefx::e_chronomillis_readonly::LAST_FRAME);

				if (delta > 0.0) { // if delta <= 0, inf
					std::chrono::milliseconds delta_tr = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(1.0 / delta));

					if (MILLI_NOW - last_time > delta_tr) {
						set<std::chrono::milliseconds>(bubblefx::e_chronomillis_readonly::LAST_FRAME, MILLI_NOW);
					}
					else return; // no task
				}

				Interface::Target targ{ get_direct<uintptr_t>(sprite::e_uintptrt::INDEX_TARGET_IN_USE) };
				auto btarg = targ.get();

				// tied resolution
				{
					const auto delta_t = get_direct<std::chrono::milliseconds>(bubblefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION);

					if (scaleg > 0.0 && (std::chrono::system_clock::now().time_since_epoch() > delta_t)) {
						set(bubblefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() + bubblefx::default_delta_t_frame_delay));

						buffer.copy_attributes(btarg, true, scaleg);
					}
				}

				const auto dot_size = get_direct<double>(bubblefx::e_double::DOTS_SIZE);
				const auto dot_vari = get_direct<double>(bubblefx::e_double::DOTS_VARIATION_PLUS);

				// particles
				for (auto& i : positions)
				{
					i.lastsize = fabs(dot_size) + fabs(cos(Tools::random()) * dot_vari);
					i.posx = 1.1 - (Tools::random() % 1200) / 500.0;
					i.posy = 1.1 - (Tools::random() % 1200) / 500.0;

					i.lastpositionscalculated[0] = ((i.posx + 1.0) / 2.0);// *btarg.get_width()* scaleg;
					i.lastpositionscalculated[1] = ((i.posy + 1.0) / 2.0);// *btarg.get_height()* scaleg;
				}

				if (buffer) { // just to be sure
					buffer.set_as_target();
					Interface::Camera cpy;
					cpy.classic_transform(buffer, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
					cpy.apply();

					const auto blur_intensity = static_cast<float>(get_direct<double>(bubblefx::e_double::BLUR_INTENSITY));
					const auto p_assist = get_direct<uintptr_t>(bubblefx::e_uintptrt_readonly::VECTOR_POSITION_DRAWING);
					const auto amont_limit = get_direct<uintptr_t>(bubblefx::e_uintptrt::DOTS_PER_FRAME_MAX);

					for (uintptr_t k = 0; k < amont_limit && (k + p_assist) < positions.size(); k++)
					{
						auto& i = positions[k + p_assist];
						al_draw_filled_circle(i.posx, i.posy, i.lastsize, al_map_rgba_f(
							static_cast<float>(Tools::limit_maximize((al_get_time() * 3.0 + 17.49) * 0.3f * ((Tools::random() % 1000) / 1000.0f))) * static_cast<float>(1.0f - (blur_intensity)),
							static_cast<float>(Tools::limit_maximize((al_get_time() * 3.0 +  5.31) * 0.1f * ((Tools::random() % 1000) / 1000.0f))) * static_cast<float>(1.0f - (blur_intensity)),
							static_cast<float>(Tools::limit_maximize((al_get_time() * 3.0 -  8.45) * 0.8f * ((Tools::random() % 1000) / 1000.0f))) * static_cast<float>(1.0f - (blur_intensity)),
							static_cast<float>(1.0f - (blur_intensity))));
					}

					if (p_assist + amont_limit >= positions.size())	set<uintptr_t>(bubblefx::e_uintptrt_readonly::VECTOR_POSITION_DRAWING, 0ULL);
					else											set<uintptr_t>(bubblefx::e_uintptrt_readonly::VECTOR_POSITION_DRAWING, p_assist + amont_limit);

					btarg.set_as_target();
				}
				else throw Handling::Abort(__FUNCSIG__, "Cannot draw BubbleFX because it couldn't get a valid bitmap somehow!", Handling::abort::abort_level::GIVEUP);
			}


			void BubbleFX::draw_task(Interface::Camera& c)
			{
				check_internal();

				if (!buffer) throw Handling::Abort(__FUNCSIG__, "Buffer null, can't draw!", Handling::abort::abort_level::GIVEUP);

				common_bitmap_draw_task(buffer);
			}

			BubbleFX::BubbleFX() : Sprite_Base()
			{
				set<double>(bubblefx::e_double_defaults);
				set<uintptr_t>(bubblefx::e_uintptrt_defaults);
				set<std::chrono::milliseconds>(bubblefx::e_chronomillis_defaults);

				set(bubblefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

			BubbleFX::BubbleFX(const BubbleFX& other)
			{
				*this = other;
			}

			BubbleFX::BubbleFX(BubbleFX&& other)
			{
				*this = std::move(other);
			}

			void BubbleFX::operator=(const BubbleFX& oth)
			{
				this->Sprite_Base::operator=(oth);

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(oth.get<std::chrono::milliseconds>());

				buffer = oth.buffer;
				positions = oth.positions;

				set(bubblefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

			void BubbleFX::operator=(BubbleFX&& other)
			{
				this->Sprite_Base::operator=(std::move(other));

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(std::move(other.get<std::chrono::milliseconds>()));

				buffer = std::move(other.buffer);
				positions = std::move(other.positions);

				set(bubblefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

			void BubbleFX::clone(const BubbleFX& other)
			{
				this->Sprite_Base::clone(other);

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(*other.get<std::chrono::milliseconds>());

				buffer = other.buffer;
				positions = other.positions;

				set(bubblefx::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

			void BubbleFX::reset_positions(const size_t& amount)
			{
				if (!amount) return;

				positions.resize(amount);

				const double scaleg = get_direct<double>(bubblefx::e_double::TIE_SIZE_TO_DISPLAY_PROPORTION);
				const auto dot_size = get_direct<double>(bubblefx::e_double::DOTS_SIZE);
				const auto dot_vari = get_direct<double>(bubblefx::e_double::DOTS_VARIATION_PLUS);

				Interface::Target targ{ get_direct<uintptr_t>(sprite::e_uintptrt::INDEX_TARGET_IN_USE) };
				auto btarg = targ.get();

				// particles
				for (auto& i : positions)
				{
					i.lastsize = fabs(dot_size) + fabs(cos(Tools::random()) * dot_vari);
					i.posx = 1.1 - (Tools::random() % 1200) / 500.0;
					i.posy = 1.1 - (Tools::random() % 1200) / 500.0;

					i.lastpositionscalculated[0] = ((i.posx + 1.0) / 2.0);// *btarg.get_width()* scaleg;
					i.lastpositionscalculated[1] = ((i.posy + 1.0) / 2.0);// *btarg.get_height()* scaleg;

					/*if (!btarg.empty()) i.lastpositionscalculated[0] = ((i.posx + 1.0) / 2.0) * btarg.get_width() * scaleg;
					else				i.lastpositionscalculated[0] = ((i.posx + 1.0) / 2.0) * 1920.0 * scaleg;

					if (!btarg.empty()) i.lastpositionscalculated[1] = ((i.posy + 1.0) / 2.0) * btarg.get_height() * scaleg;
					else				i.lastpositionscalculated[1] = ((i.posy + 1.0) / 2.0) * 1080.0 * scaleg;*/
				}
			}


		}
	}
}