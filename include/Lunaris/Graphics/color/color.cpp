#include "color.h"

namespace Lunaris {

	color::color(const float r, const float g, const float b, const float a)
	{
		this->r = r > 1.0 ? 1.0 : r;
		this->g = g > 1.0 ? 1.0 : g;
		this->b = b > 1.0 ? 1.0 : b;
		this->a = a > 1.0 ? 1.0 : a;
	}

	color::color(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a)
	{
		this->r = r * 1.0 / 0xFF;
		this->g = g * 1.0 / 0xFF;
		this->b = b * 1.0 / 0xFF;
		this->a = a * 1.0 / 0xFF;
	}

	color::color(const int r, const int g, const int b, const int a)
	{
		this->r = r < 0 ? 0 : (r > 255 ? 1.0 : r * 1.0 / 0xFF);
		this->g = g < 0 ? 0 : (g > 255 ? 1.0 : g * 1.0 / 0xFF);
		this->b = b < 0 ? 0 : (b > 255 ? 1.0 : b * 1.0 / 0xFF);
		this->a = a < 0 ? 0 : (a > 255 ? 1.0 : a * 1.0 / 0xFF);
	}

	color::color(const color& old)
	{
		this->r = old.r;
		this->g = old.g;
		this->b = old.b;
		this->a = old.a;
	}

	color::color(ALLEGRO_COLOR old)
	{
		this->r = old.r;
		this->g = old.g;
		this->b = old.b;
		this->a = old.a;
	}

	float color::get_r() const
	{
		return r;
	}

	float color::get_g() const
	{
		return g;
	}

	float color::get_b() const
	{
		return b;
	}

	float color::get_a() const
	{
		return a;
	}

	void color::clear_to_this() const
	{
		al_clear_to_color(*this);
	}

	color& color::operator=(const color& old)
	{
		this->r = old.r;
		this->g = old.g;
		this->b = old.b;
		this->a = old.a;
		return *this;
	}
}