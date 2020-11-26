#include "color.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			Color::Color()
			{
				Handling::init_basic();
				Handling::init_graphics();
			}

			Color::Color(const float r, const float g, const float b, const float a)
			{
				Handling::init_basic();
				Handling::init_graphics();

				this->r = r > 1.0 ? 1.0 : r;
				this->g = g > 1.0 ? 1.0 : g;
				this->b = b > 1.0 ? 1.0 : b;
				this->a = a > 1.0 ? 1.0 : a;
			}
			
			Color::Color(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a)
			{
				Handling::init_basic();
				Handling::init_graphics();

				this->r = r * 1.0 / 0xFF;
				this->g = g * 1.0 / 0xFF;
				this->b = b * 1.0 / 0xFF;
				this->a = a * 1.0 / 0xFF;
			}
			
			Color::Color(const int r, const int g, const int b, const int a)
			{
				Handling::init_basic();
				Handling::init_graphics();

				this->r = r < 0 ? 0 : (r > 255 ? 1.0 : r * 1.0 / 0xFF);
				this->g = g < 0 ? 0 : (g > 255 ? 1.0 : g * 1.0 / 0xFF);
				this->b = b < 0 ? 0 : (b > 255 ? 1.0 : b * 1.0 / 0xFF);
				this->a = a < 0 ? 0 : (a > 255 ? 1.0 : a * 1.0 / 0xFF);
			}
			
			Color::Color(const Color& old)
			{
				Handling::init_basic();
				Handling::init_graphics();

				this->r = old.r;
				this->g = old.g;
				this->b = old.b;
				this->a = old.a;
			}
			
			Color::Color(ALLEGRO_COLOR old)
			{
				Handling::init_basic();
				Handling::init_graphics();

				this->r = old.r;
				this->g = old.g;
				this->b = old.b;
				this->a = old.a;
			}
			
			float Color::get_r() const
			{
				return r;
			}
			
			float Color::get_g() const
			{
				return g;
			}
			
			float Color::get_b() const
			{
				return b;
			}
			
			float Color::get_a() const
			{
				return a;
			}
			
			void Color::clear_to_this() const
			{
				al_clear_to_color(*this);
			}

			Color& Color::operator=(const Color& old)
			{
				this->r = old.r;
				this->g = old.g;
				this->b = old.b;
				this->a = old.a;
				return *this;
			}
		}
	}
}