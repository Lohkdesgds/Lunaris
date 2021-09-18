#include "progressbar.h"

namespace LSW {
	namespace v5 {
		namespace Work {

			double ProgressBar::max(const double val, const double l, const double u)
			{
				return val < l ? l : (val > u ? u : val);
			}

			double ProgressBar::beyond(const double val, const double by)
			{
				return fabs(val) > by ? (val < 0.0 ? val + by : val - by) : 0.0;
			}
			
			void ProgressBar::draw_task(Interface::Camera& c) // 0.1 looks like \ .
			{
				const auto posx_v = get_direct<double>(sprite::e_double_readonly::POSX);
				const auto posy_v = get_direct<double>(sprite::e_double_readonly::POSY);
				const auto scale_x = get_direct<double>(sprite::e_double::SCALE_X) * get_direct<double>(sprite::e_double::SCALE_G);
				const auto scale_y = get_direct<double>(sprite::e_double::SCALE_Y) * get_direct<double>(sprite::e_double::SCALE_G);
				const auto background_nomove = get_direct<bool>(progressbar::e_boolean::FIXED_BACKGROUND);

				const auto border = get_direct<double>(progressbar::e_double::BORDER_THICKNESS);
				const auto progress_smooth = get_direct<double>(progressbar::e_double_readonly::PROGRESS_SMOOTH);
				const auto progress_raw = get_direct<double>(progressbar::e_double::PROGRESS);
				const auto forced_smooth = max(get_direct<double>(progressbar::e_double::SMOOTHNESS), 0.0, 1e9);

				const auto perc_run = get_direct<double>(sprite::e_double_readonly::PERC_CALC_SMOOTH);

				auto progress = (((1.0 - perc_run) * progress_raw + perc_run * progress_smooth) + (forced_smooth * progress_smooth)) / (1.0 + forced_smooth);
				set(progressbar::e_double_readonly::PROGRESS_SMOOTH, progress);

				// convert range [0.0,1.0] to [-1.0,1.0]
				progress = -1.0 + 2.0 * progress;
				

				Interface::Vertex verts_bg[] = {
					{ 
						static_cast<float>(posx_v - scale_x * 0.5 * (background_nomove ? 1.0 : max(-progress))),
						static_cast<float>(posy_v - scale_y * 0.5),
						get_direct<Interface::Color>(progressbar::e_color::BACKGROUND_TOP_LEFT)
					}, //top left
					{ 
						static_cast<float>(posx_v + scale_x * 0.5),
						static_cast<float>(posy_v - scale_y * 0.5),
						get_direct<Interface::Color>(progressbar::e_color::BACKGROUND_TOP_RIGHT)
					}, //top right
					{ 
						static_cast<float>(posx_v - scale_x * 0.5 * (background_nomove ? 1.0 : max(-progress))),
						static_cast<float>(posy_v + scale_y * 0.5),
						get_direct<Interface::Color>(progressbar::e_color::BACKGROUND_BOTTOM_LEFT)
					}, //bottom left
					{ 
						static_cast<float>(posx_v + scale_x * 0.5),
						static_cast<float>(posy_v + scale_y * 0.5),
						get_direct<Interface::Color>(progressbar::e_color::BACKGROUND_BOTTOM_RIGHT)
					}  //bottom right
				};

				Interface::Vertex verts_fg[] = {
					{ 
						static_cast<float>(posx_v - scale_x * 0.5),
						static_cast<float>(posy_v - scale_y * 0.5),
						get_direct<Interface::Color>(progressbar::e_color::FOREGROUND_TOP_LEFT)
					}, //top left
					{
						static_cast<float>(posx_v + scale_x * 0.5 * max(progress)),
						static_cast<float>(posy_v - scale_y * 0.5),
						get_direct<Interface::Color>(progressbar::e_color::FOREGROUND_TOP_RIGHT)
					}, //top right
					{
						static_cast<float>(posx_v - scale_x * 0.5), 
						static_cast<float>(posy_v + scale_y * 0.5),
						get_direct<Interface::Color>(progressbar::e_color::FOREGROUND_BOTTOM_LEFT)
					}, //bottom left
					{
						static_cast<float>(posx_v + scale_x * 0.5 * max(progress)),
						static_cast<float>(posy_v + scale_y * 0.5), 
						get_direct<Interface::Color>(progressbar::e_color::FOREGROUND_BOTTOM_RIGHT)
					}  //bottom right
				};

				if (border > 0.0) {
					const auto roundness = get_direct<double>(progressbar::e_double::BORDER_ROUNDNESS);
					const auto prop_x = get_direct<double>(progressbar::e_double::BORDER_PROPORTION_X);
					const auto prop_y = get_direct<double>(progressbar::e_double::BORDER_PROPORTION_Y);

					al_draw_rounded_rectangle(
						static_cast<float>((posx_v - scale_x * 0.5 * prop_x)), static_cast<float>((posy_v - scale_y * 0.5 * prop_y)),
						static_cast<float>((posx_v + scale_x * 0.5 * prop_x)), static_cast<float>((posy_v + scale_y * 0.5 * prop_y)),
						roundness, roundness,
						get_direct<Interface::Color>(progressbar::e_color::BORDER),
						border);
				}

				al_draw_prim(verts_bg, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP);
				al_draw_prim(verts_fg, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP);

				
				// draw progress bar lol
			}

			ProgressBar::ProgressBar() : Sprite_Base()
			{
				set<Interface::Color>(progressbar::e_vertex_defaults);
				set<double>(progressbar::e_double_defaults);
				set<bool>(progressbar::e_boolean_defaults);
			}

			ProgressBar::ProgressBar(const ProgressBar& other)
			{
				*this = other;
			}

			ProgressBar::ProgressBar(ProgressBar&& other)
			{
				*this = std::move(other);
			}

			void ProgressBar::operator=(const ProgressBar& other)
			{
				this->Sprite_Base::operator=(other);
			}

			void ProgressBar::operator=(ProgressBar&& other)
			{
				this->Sprite_Base::operator=(std::move(other));
			}

			void ProgressBar::clone(const ProgressBar& other)
			{
				this->Sprite_Base::clone(other);
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

			template Tools::Resource<ProgressBar>;

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