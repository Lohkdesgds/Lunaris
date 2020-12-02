#include "block.h"


namespace LSW {
	namespace v5 {
		namespace Work {
						
			void Block::draw_task(Interface::Camera& c)
			{
				if (bitmaps.empty()) return;
				/*{
					const auto delta_t = get_direct<std::chrono::milliseconds>(block::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION);

					if (const double _dd = get_direct<double>(block::e_double::TIE_SIZE_TO_DISPLAY_PROPORTION); _dd > 0.0 && (std::chrono::system_clock::now().time_since_epoch() > delta_t)) {
						set(block::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() + block::default_delta_t_frame_delay));

						Interface::Target targ(get_direct<uintptr_t>(sprite::e_uintptrt::INDEX_TARGET_IN_USE));
						for (auto& i : bitmaps) i.copy_attributes(targ.get(), true, _dd);
					}
				}*/

				size_t frame = static_cast<size_t>(get_direct<uintptr_t>(block::e_uintptr_t::FRAME));

				if (!get_direct<bool>(block::e_boolean::SET_FRAME_VALUE_READONLY)) {

					const double delta = get_direct<double>(block::e_double::FRAMES_PER_SECOND); // delta t, 1/t = sec
					std::chrono::milliseconds last_time = get_direct<std::chrono::milliseconds>(block::e_chronomillis_readonly::LAST_FRAME);

					if (delta > 0.0) { // if delta <= 0 or frame < 0, static
						std::chrono::milliseconds delta_tr = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(1.0 / delta));

						if (delta_tr.count() > 0) {

							if (std::chrono::system_clock::now().time_since_epoch() > last_time * block::max_frames_behind) {
								last_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
							}
							else {
								while (std::chrono::system_clock::now().time_since_epoch() > last_time) {
									last_time += delta_tr;
									if (++frame >= bitmaps.size()) frame = 0;
								}
							}
						}
					}

					set<std::chrono::milliseconds>(block::e_chronomillis_readonly::LAST_FRAME, last_time);

					set<uintptr_t>(block::e_uintptr_t::FRAME, frame);
				}
				if (frame >= bitmaps.size()) frame = static_cast<size_t>(bitmaps.size() - 1);

				Interface::Bitmap& rnn = bitmaps[frame];
				if (!rnn) throw Handling::Abort(__FUNCSIG__, "Unexpected NULL on draw!");

				// P.S.: Text copied this

				float cx, cy, px, py, dsx, dsy, rot_rad;
				int bmpx, bmpy;
				bmpx = rnn.get_width();
				bmpy = rnn.get_height();
				if (bmpx <= 0 || bmpy <= 0) {
					throw Handling::Abort(__FUNCSIG__, "Somehow the texture have < 0 width / height!");
				}

				cx = 1.0f * bmpx * ((get_direct<double>(sprite::e_double::CENTER_X) + 1.0) * 0.5);
				cy = 1.0f * bmpy * ((get_direct<double>(sprite::e_double::CENTER_Y) + 1.0) * 0.5);
				rot_rad = 1.0f * get_direct<double>(sprite::e_double_readonly::ROTATION) * ALLEGRO_PI / 180.0;
				px = get_direct<double>(sprite::e_double_readonly::POSX);
				py = get_direct<double>(sprite::e_double_readonly::POSY);
				dsx = 1.0f * (get_direct<double>(sprite::e_double::SCALE_X)) * (get_direct<double>(sprite::e_double::SCALE_G)) * (1.0 / bmpx);
				dsy = 1.0f * (get_direct<double>(sprite::e_double::SCALE_Y)) * (get_direct<double>(sprite::e_double::SCALE_G)) * (1.0 / bmpy);


				if (get_direct<bool>(sprite::e_boolean::USE_COLOR)) {
					rnn.draw(
						get_direct<Interface::Color>(sprite::e_color::COLOR),
						cx, cy,
						px, py,
						dsx, dsy,
						rot_rad);
				}
				else {
					rnn.draw(
						cx, cy,
						px, py,
						dsx, dsy,
						rot_rad);
				}

			}
						
			Block::Block() : Sprite_Base()
			{
				//reference.be_reference_to_target(true);

				set<double>(block::e_double_defaults);
				set<bool>(block::e_boolean_defaults);
				set<uintptr_t>(block::e_uintptr_t_defaults); // same as uintptr_t
				set<std::chrono::milliseconds>(block::e_chronomillis_defaults);

				set(block::e_chronomillis_readonly::LAST_FRAME, MILLI_NOW);
				//set(block::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}
						
			Block::Block(const Block& other)
			{
				*this = other;
			}
			
			Block::Block(Block&& other) noexcept
			{
				*this = std::move(other);
			}
			
			void Block::operator=(const Block& oth)
			{
				this->Sprite_Base::operator=(oth);

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(oth.get<std::chrono::milliseconds>());

				bitmaps = oth.bitmaps;

				set(block::e_chronomillis_readonly::LAST_FRAME, MILLI_NOW);
				//set(block::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}
			
			void Block::operator=(Block&& other) noexcept
			{
				this->Sprite_Base::operator=(std::move(other));

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(std::move(other.get<std::chrono::milliseconds>()));

				bitmaps = std::move(other.bitmaps);

				set(block::e_chronomillis_readonly::LAST_FRAME, MILLI_NOW);
				//set(block::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}
			
			void Block::clone(const Block& other)
			{
				this->Sprite_Base::clone(other);

				// difference from Sprite_Base
				set<std::chrono::milliseconds>(*other.get<std::chrono::milliseconds>());

				bitmaps = other.bitmaps;

				set(block::e_chronomillis_readonly::LAST_FRAME, MILLI_NOW);
				//set(block::e_chronomillis_readonly::LAST_TIE_FRAME_VERIFICATION, MILLI_NOW);
			}

			void Block::clone_bitmaps(const Block& other)
			{
				bitmaps = other.bitmaps;
			}
			
			void Block::insert(const Interface::Bitmap& bmp)
			{
				bitmaps.push_back(bmp);
			}
			
			size_t Block::remove(const std::function<bool(const Interface::Bitmap&)> remf)
			{
				size_t match = 0;
				for (size_t p = 0; p < bitmaps.size(); p++) {
					auto& i = bitmaps[p];
					if (remf(i)) {
						match++;
						bitmaps.erase(bitmaps.begin() + p);
					}
				}
				return match;
			}

			// implementation

			template Tools::FastFunction<std::chrono::milliseconds>;
			template Tools::FastFunction<std::string>;
			template Tools::FastFunction<double>;
			template Tools::FastFunction<bool>;
			template Tools::FastFunction<int>;
			template Tools::FastFunction<Interface::Color>;
			template Tools::FastFunction<uintptr_t>;
			template Tools::FastFunction<sprite::e_tie_functional>;
			template Tools::FastFunction<sprite::functional>;

			template Tools::Resource<Block>;

			template Tools::SuperFunctionalMap<std::chrono::milliseconds>;
			template Tools::SuperFunctionalMap<std::string>;
			template Tools::SuperFunctionalMap<double>;
			template Tools::SuperFunctionalMap<bool>;
			template Tools::SuperFunctionalMap<int>;
			template Tools::SuperFunctionalMap<Interface::Color>;
			template Tools::SuperFunctionalMap<uintptr_t>;
			template Tools::SuperFunctionalMap<sprite::e_tie_functional>;
			template Tools::SuperFunctionalMap<sprite::functional>;

			template Tools::SuperMap<std::function<std::chrono::milliseconds(void)>>;
			template Tools::SuperMap<std::function<std::string(void)>>;
			template Tools::SuperMap<std::function<double(void)>>;
			template Tools::SuperMap<std::function<bool(void)>>;
			template Tools::SuperMap<std::function<int(void)>>;
			template Tools::SuperMap<std::function<Interface::Color(void)>>;
			template Tools::SuperMap<std::function<uintptr_t(void)>>;
			template Tools::SuperMap<std::function<sprite::e_tie_functional(void)>>;
			template Tools::SuperMap<std::function<sprite::functional(void)>>;

			template Tools::SuperPair<std::chrono::milliseconds>;
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