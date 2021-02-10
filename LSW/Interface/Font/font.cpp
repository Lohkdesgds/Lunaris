#include "font.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool Font::set(ALLEGRO_FONT* f)
			{
				if (!f) return false;

				if (!font) {
					font.create(f, [](ALLEGRO_FONT* b) { if (al_is_system_installed() && b) { al_destroy_font(b); b = nullptr; } });
				}
				else {
					font.swap_destroy(f);
				}

				return !(!font);
			}

			ALLEGRO_FONT* Font::quick() const
			{
				if (font) return font.get();
				return nullptr;
			}

			ALLEGRO_FONT* Font::quick()
			{
				if (font) return font.get();
				return nullptr;
			}

			Color Font::hex(const int hx) const
			{
				switch (hx) {
				case 0x0:
					return Color(0, 0, 0);
				case 0x1:
					return Color(0, 0, 170);
				case 0x2:
					return Color(0, 170, 0);
				case 0x3:
					return Color(0, 170, 170);
				case 0x4:
					return Color(170, 0, 0);
				case 0x5:
					return Color(170, 0, 170);
				case 0x6:
					return Color(170, 170, 0);
				case 0x7:
					return Color(170, 170, 170);
				case 0x8:
					return Color(84, 84, 84);
				case 0x9:
					return Color(84, 84, 255);
				case 0xA:
					return Color(84, 255, 84);
				case 0xB:
					return Color(84, 255, 255);
				case 0xC:
					return Color(255, 84, 84);
				case 0xD:
					return Color(255, 84, 255);
				case 0xE:
					return Color(255, 255, 84);
				default:
					return Color(255, 255, 255);
				}
			}

			Font::Font()
			{
				Handling::init_basic();
				Handling::init_font();
			}

			Font::Font(const Font& f)
			{
				*this = f;
			}

			Font::Font(Font&& f) noexcept
			{
				*this = std::move(f);
			}

			void Font::operator=(const Font& f)
			{
				font = f.font;
			}

			void Font::operator=(Font&& f) noexcept
			{
				font = std::move(f.font);
			}

			bool Font::load(const std::string& p, const int s, const int f)
			{
				return set(al_load_font(p.c_str(), s, f));
			}

			Font::operator bool() const
			{
				return !(!font);
			}

			const bool Font::operator!() const
			{
				return !font;
			}

			const bool Font::operator==(const Font& o) const
			{
				return quick() == o.quick();
			}

			int Font::get_line_ascent() const
			{
				if (auto q = quick(); q) return al_get_font_ascent(q);
				return -1;
			}

			int Font::get_line_descent() const
			{
				if (auto q = quick(); q) return al_get_font_descent(q);
				return -1;
			}

			int Font::get_line_height() const
			{
				if (auto q = quick(); q) return al_get_font_line_height(q);
				return -1;
			}

			int Font::get_width(const std::string& str) const
			{
				if (auto q = quick(); q) return al_get_text_width(q, str.c_str());
				return -1;
			}

			int Font::get_width(Tools::Cstring str) const
			{
				if (auto q = quick(); q) return al_get_text_width(q, str.s_str().c_str());
				return -1;
			}

			void Font::draw(Color c, const float x, const float y, const int f, const std::string& s) const
			{
				if (auto q = quick(); q) al_draw_text(q, c, x, y, f, s.c_str());
			}

			void Font::draw(const float x, const float y, const int f, Tools::Cstring s) const
			{
				if (auto q = quick(); q) {
					double compensate = 0.0;
					if (f != 0) {
						switch (f) {
						case ALLEGRO_ALIGN_CENTER:
							compensate = 0.5;
							break;
						case ALLEGRO_ALIGN_RIGHT:
							compensate = 1.0;
							break;
						}
					}


					std::string thebuff;
					int offset_x_f = - compensate * al_get_text_width(q, s.s_str().c_str());
					Tools::char_c* data_ = s.data();

					for (size_t p = 0; p < s.size();/* p++*/) { // p++ there down vvv

						auto clr_now = hex(static_cast<int>(data_->cr));

						for (auto _ref = data_->cr; _ref == data_->cr && p < s.size();) {
							thebuff += data_->ch;
							data_++;
							p++;
						}

						al_draw_text(q, clr_now, offset_x_f + x, y, 0, thebuff.c_str());

						offset_x_f += al_get_text_width(q, thebuff.c_str());
						thebuff.clear();
					}
				}
			}

			// implementation

			template Tools::AdvancedShared<ALLEGRO_FONT>;
			template Tools::Resource<Font>;

		}
	}
}