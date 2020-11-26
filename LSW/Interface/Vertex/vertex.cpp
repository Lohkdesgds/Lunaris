#include "vertex.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			Vertex::Vertex()
			{
				Handling::init_basic();
				Handling::init_graphics();
				x = y = z = u = v = 0.0f;
			}

			Vertex::Vertex(const Vertex& vertx)
			{
				Handling::init_basic();
				Handling::init_graphics();
				*this = vertx;
			}

			Vertex::Vertex(ALLEGRO_VERTEX vertx)
			{
				x = y = z = u = v = 0.0f;
				x = vertx.x;
				y = vertx.y;
				z = vertx.z;
				color = vertx.color;
				u = vertx.u;
				v = vertx.v;
			}

			Vertex& Vertex::operator=(const Vertex& vertx)
			{
				x = vertx.x;
				y = vertx.y;
				z = vertx.z;
				color = vertx.color;
				u = vertx.u;
				v = vertx.v;
				return *this;
			}

			Vertex::Vertex(const float xx, const float yy, const Color& cc)
			{
				x = xx;
				y = yy;
				z = u = v = 0.0f;
				color = cc;
			}

			Vertex::Vertex(const float xx, const float yy, const float zz, const Color& cc)
			{
				u = v = 0.0f;
				x = xx;
				y = yy;
				z = zz;
				color = cc;
			}

			Vertex::Vertex(const float xx, const float yy, const Color& cc, const float uu, const float vv)
			{
				z = 0.0f;
				x = xx;
				y = yy;
				u = uu;
				v = vv;
				color = cc;
			}

			Vertex::Vertex(const float xx, const float yy, const float zz, const Color& cc, const float uu, const float vv)
			{
				x = xx;
				y = yy;
				z = zz;
				u = uu;
				v = vv;
				color = cc;
			}

			float Vertex::get_x() const
			{
				return x;
			}

			float Vertex::get_y() const
			{
				return y;
			}

			float Vertex::get_z() const
			{
				return z;
			}

			float Vertex::get_u() const
			{
				return u;
			}

			float Vertex::get_v() const
			{
				return v;
			}

			Color Vertex::get_color() const
			{
				return color;
			}

		}
	}
}