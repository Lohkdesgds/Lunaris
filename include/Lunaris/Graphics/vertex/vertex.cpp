#include "vertex.h"


namespace Lunaris {

	LUNARIS_DECL void __vertex_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_primitives_addon_initialized() && !al_init_primitives_addon()) throw std::runtime_error("Can't start Primitives!");
		if (!al_is_image_addon_initialized() && !al_init_image_addon()) throw std::runtime_error("Can't start Image!");
	}

	LUNARIS_DECL vertex_point::vertex_point(const float _x, const float _y, const Lunaris::color& _c)
	{
		this->x = _x;
		this->y = _y;
		this->z = 0;
		this->u = 0;
		this->v = 0;
		this->color = _c;
	}

	LUNARIS_DECL vertex_point::vertex_point(const float _x, const float _y, const float _z, const Lunaris::color& _c)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->u = 0;
		this->v = 0;
		this->color = _c;
	}

	LUNARIS_DECL vertex_point::vertex_point(const float _x, const float _y, const float _z, const float _u, const float _v, const Lunaris::color& _c)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->u = _u;
		this->v = _v;
		this->color = _c;
	}

	LUNARIS_DECL vertexes::vertexes()
	{
		__vertex_allegro_start();
	}

	LUNARIS_DECL void vertexes::push_back(const vertex_point& v)
	{
		std::unique_lock<std::shared_mutex>(safe_mtx);
		points.push_back(v);
	}

	LUNARIS_DECL void vertexes::push_back(vertex_point&& v)
	{
		std::unique_lock<std::shared_mutex>(safe_mtx);
		points.push_back(std::move(v));
	}

	LUNARIS_DECL void vertexes::push_back(std::initializer_list<vertex_point> l)
	{
		std::unique_lock<std::shared_mutex>(safe_mtx);
		points.insert(points.end(), l.begin(), l.end());
	}

	LUNARIS_DECL void vertexes::set_texture(const hybrid_memory<texture>& t)
	{
		std::unique_lock<std::shared_mutex>(safe_mtx);
		textur = t;
	}

	LUNARIS_DECL void vertexes::remove_texture()
	{
		std::unique_lock<std::shared_mutex>(safe_mtx);
		textur.reset_this();
	}

	LUNARIS_DECL size_t vertexes::size() const
	{
		std::shared_lock<std::shared_mutex>(safe_mtx);
		return points.size();
	}

	LUNARIS_DECL vertex_point vertexes::index(const size_t i) const
	{
		std::shared_lock<std::shared_mutex>(safe_mtx);
		if (i >= points.size()) throw std::out_of_range("index was too high on vertexes.");
		return points[i];
	}

	LUNARIS_DECL void vertexes::safe(std::function<void(std::vector<vertex_point>&)> f)
	{
		if (!f) return;
		std::unique_lock<std::shared_mutex>(safe_mtx);
		f(points);
	}

	LUNARIS_DECL void vertexes::csafe(std::function<void(const std::vector<vertex_point>&)> f) const
	{
		if (!f) return;
		std::shared_lock<std::shared_mutex>(safe_mtx);
		f(points);
	}

	LUNARIS_DECL bool vertexes::has_texture() const
	{
		std::shared_lock<std::shared_mutex>(safe_mtx);
		return textur.valid() && !textur->empty();
	}

	LUNARIS_DECL void vertexes::set_mode(types t)
	{
		type = t;
	}

	LUNARIS_DECL vertexes::types vertexes::get_mode() const
	{
		return type;
	}

	LUNARIS_DECL void vertexes::draw()
	{
		std::shared_lock<std::shared_mutex>(safe_mtx);
		if (!points.size()) return;
		al_draw_prim(points.data(), nullptr, textur.valid() ? textur->get_raw_bitmap() : nullptr, 0, static_cast<int>(points.size()), static_cast<int>(type));
	}

	LUNARIS_DECL bool vertexes::valid() const
	{
		switch (type) {
		case types::POINT_LIST:
			return points.size() > 0;
		case types::LINE_LIST:
			return (points.size() > 1) && ((points.size() % 2) == 0);
		case types::LINE_STRIP:
			return points.size() > 1;
		case types::LINE_LOOP:
			return points.size() > 1;
		case types::TRIANGLE_LIST:
			return (points.size() > 2) && ((points.size() % 3) == 0);
		case types::TRIANGLE_STRIP:
			return points.size() > 2;
		case types::TRIANGLE_FAN:
			return points.size() > 2;
		}
		return false;
	}

	LUNARIS_DECL bool vertexes::empty() const
	{
		return points.size() == 0;
	}

}