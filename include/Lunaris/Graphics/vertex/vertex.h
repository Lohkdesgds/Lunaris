#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/color.h>
#include <Lunaris/Graphics/texture.h>
#include <Lunaris/Utility/mutex.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdexcept>
#include <vector>

namespace Lunaris {

	void __vertex_allegro_start();

	/// <summary>
	/// <para>A vertex point with X, Y and Z (hardware accelerated only), texture offset and color information.</para>
	/// </summary>
	struct vertex_point : public ALLEGRO_VERTEX {
		vertex_point() = default;

		/// <summary>
		/// <para>Create a vertex_point.</para>
		/// </summary>
		/// <param name="{float}">The X value.</param>
		/// <param name="{float}">The Y value.</param>
		/// <param name="{color}">Color of this point.</param>
		vertex_point(const float, const float, const Lunaris::color&);

		/// <summary>
		/// <para>Create a vertex_point.</para>
		/// </summary>
		/// <param name="{float}">The X value.</param>
		/// <param name="{float}">The Y value.</param>
		/// <param name="{float}">The Z value.</param>
		/// <param name="{color}">Color of this point.</param>
		vertex_point(const float, const float, const float, const Lunaris::color&);

		/// <summary>
		/// <para>Create a vertex_point.</para>
		/// </summary>
		/// <param name="{float}">The X value.</param>
		/// <param name="{float}">The Y value.</param>
		/// <param name="{float}">The Z value.</param>
		/// <param name="{float}">The U texture coordinates in pixels.</param>
		/// <param name="{float}">The V texture coordinates in pixels.</param>
		/// <param name="{color}">Color of this point.</param>
		vertex_point(const float, const float, const float, const float, const float, const Lunaris::color&);
	};

	/// <summary>
	/// <para>vertexes is a easy combo of points that are translated to lines or triangles easily.</para>
	/// </summary>
	class vertexes {
	public:
		enum class types {
			POINT_LIST,			// Each vertex is a point, literally.
			LINE_LIST,			// Pairs of points defining lines. Expects pair of vertex points. Each pair is a line.
			LINE_STRIP,			// Combo of points drawing a complex line. Every two points create a line.
			LINE_LOOP,			// Combo of points like LINE_STRIP but the last point also connects to the first one
			TRIANGLE_LIST,		// Triangle list (each three vertex points make a triangle). Expects multiple of 3.
			TRIANGLE_STRIP,		// Triangle strip is a "keep going" triangle list. 3 points make one triangle one by one ([0,1,2], [1,2,3], [2,3,4], ...)
			TRIANGLE_FAN		// A "center" point and triangles around. All triangles share the first vertex point.
		};
	private:
		std::vector<vertex_point> points;
		hybrid_memory<texture> textur;
		types type = types::TRIANGLE_LIST;
		mutable std::shared_mutex safe_mtx;
	public:
		/// <summary>
		/// <para>Add a vertex point to the existing list of vertex points.</para>
		/// </summary>
		/// <param name="{vertex_point}">A vertex point to add.</param>
		void push_back(const vertex_point&);

		/// <summary>
		/// <para>Add a vertex point to the existing list of vertex points.</para>
		/// </summary>
		/// <param name="{vertex_point}">A vertex point to add.</param>
		void push_back(vertex_point&&);

		/// <summary>
		/// <para>Set a texture to the triangles/lines</para>
		/// </summary>
		/// <param name="{texture}">A hybrid memory to a texture.</param>
		void set_texture(const hybrid_memory<texture>&);

		/// <summary>
		/// <para>Remove texture, leaving only raw colors.</para>
		/// </summary>
		void remove_texture();

		/// <summary>
		/// <para>Get how many vertex points are stored.</para>
		/// </summary>
		/// <returns>{size_t} Amount of vertex points saved.</returns>
		size_t size() const;

		/// <summary>
		/// <para>Whether there's a texture set or not</para>
		/// </summary>
		/// <returns>{bool} True if has texture set.</returns>
		bool has_texture() const;

		/// <summary>
		/// <para>Draws itself on screen.</para>
		/// </summary>
		void draw();
	};

}