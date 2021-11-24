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
		return size_t();
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

}