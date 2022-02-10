#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Graphics/color.h>
#include <Lunaris/Graphics/texture.h>
#include <Lunaris/Graphics/transform.h>
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
			POINT_LIST		= ALLEGRO_PRIM_POINT_LIST,			// Each vertex is a point, literally.
			LINE_LIST		= ALLEGRO_PRIM_LINE_LIST,			// Pairs of points defining lines. Expects pair of vertex points. Each pair is a line.
			LINE_STRIP		= ALLEGRO_PRIM_LINE_STRIP,			// Combo of points drawing a complex line. Every two points create a line.
			LINE_LOOP		= ALLEGRO_PRIM_LINE_LOOP,			// Combo of points like LINE_STRIP but the last point also connects to the first one
			TRIANGLE_LIST	= ALLEGRO_PRIM_TRIANGLE_LIST,		// Triangle list (each three vertex points make a triangle). Expects multiple of 3.
			TRIANGLE_STRIP	= ALLEGRO_PRIM_TRIANGLE_STRIP,		// Triangle strip is a "keep going" triangle list. 3 points make one triangle one by one ([0,1,2], [1,2,3], [2,3,4], ...)
			TRIANGLE_FAN	= ALLEGRO_PRIM_TRIANGLE_FAN			// A "center" point and triangles around. All triangles share the first vertex point.
			// if changed later, update valid()
		};
	private:
		std::vector<vertex_point> points;
		std::vector<vertex_point> npts; // transformed. Valid  
		hybrid_memory<texture> textur;
		types type = types::TRIANGLE_LIST;
		mutable std::shared_mutex safe_mtx;
		transform latest_transform;
	public:
		vertexes();

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
		/// <para>Add multiple vertex points at once (to the end of the list).</para>
		/// </summary>
		/// <param name="{initializer_list}">A list of vertex points.</param>
		void push_back(std::initializer_list<vertex_point>);

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
		/// <para>Get a copy of a specific vertex</para>
		/// </summary>
		/// <param name="{size_t}">The index value.</param>
		/// <returns>{vertex_point} The vertex there, or exception if out of range.</returns>
		vertex_point index(const size_t) const;

		/// <summary>
		/// <para>Read the vector points like a safe_vector.</para>
		/// </summary>
		/// <param name="{function}">A function that does something with a vector.</param>
		void safe(std::function<void(std::vector<vertex_point>&)>);

		/// <summary>
		/// <para>Read the vector points like a safe_vector.</para>
		/// </summary>
		/// <param name="{function}">A function that reads a vector of vertex_point.</param>
		void csafe(std::function<void(const std::vector<vertex_point>&)>) const;

		/// <summary>
		/// <para>Generate the vector points like a safe_vector, but translated using latest transform.</para>
		/// <para>This creates new buffer inside (doubles it) and does refresh when called. Clear buffer with free_transformed() (only if not in use anymore!).</para>
		/// </summary>
		void generate_transformed();

		/// <summary>
		/// <para>Read the vector points like a safe_vector, but translated using latest transform.</para>
		/// <para>WARN: Be sure you had generate_transformed() once to generate the transformed data BEFORE READING IT (or it'll be empty or old data).</para>
		/// </summary>
		/// <param name="{function}">A function that reads a vector of vertex_point.</param>
		void csafe_transformed(std::function<void(const std::vector<vertex_point>&)>) const;

		/// <summary>
		/// <para>Did you generate a transformed array via safe_transformed and you want to free that up? This is it.</para>
		/// </summary>
		void free_transformed();

		/// <summary>
		/// <para>Whether there's a texture set or not</para>
		/// </summary>
		/// <returns>{bool} True if has texture set.</returns>
		bool has_texture() const;

		/// <summary>
		/// <para>Set points interpretation mode.</para>
		/// </summary>
		/// <param name="{types}">Which type?</param>
		void set_mode(types);

		/// <summary>
		/// <para>Get the mode set.</para>
		/// </summary>
		/// <returns>{types} The mode being used.</returns>
		types get_mode() const;

		/// <summary>
		/// <para>Draws itself on screen.</para>
		/// </summary>
		void draw();

		/// <summary>
		/// <para>Is this a valid well built object? Some mode types require special vertex sizes, so this checks that as well.</para>
		/// </summary>
		/// <returns>{bool} True if good.</returns>
		bool valid() const;

		/// <summary>
		/// <para>If there's a vertex set, it's NOT empty.</para>
		/// </summary>
		/// <returns>{bool} True if no vertex is set.</returns>
		bool empty() const;

		/// <summary>
		/// <para>Get the latest transform used in draw()</para>
		/// </summary>
		/// <returns>{transform} The transform.</returns>
		transform copy_transform_in_use() const;

		/// <summary>
		/// <para>Translates all points using this. This is like += everyone.</para>
		/// </summary>
		/// <param name="{float}">Move in X.</param>
		/// <param name="{float}">Move in Y.</param>
		void translate(const float, const float);
	};

}