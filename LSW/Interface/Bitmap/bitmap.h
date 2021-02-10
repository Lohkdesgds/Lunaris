#pragma once

// C++
#include <string>
#include <functional>
// Others
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/AdvancedShared/advancedshared.h"
#include "../../Tools/Resource/resource.h"
#include "../Color/color.h"


namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace bitmap {
				constexpr int default_new_bitmap_flags = ALLEGRO_MIN_LINEAR;// | ALLEGRO_MAG_LINEAR;

				enum class drawing_mode {DEFAULT, FLIP_HORIZONTAL, FLIP_VERTICAL, FLIP_HORIZONTAL_VERTICAL};
			}

			class Bitmap {
				static bool first_time;

				Tools::AdvancedShared<ALLEGRO_BITMAP> bitmap_orig;

				// set new at bitmap (set(al_load_bitmap...) or something like that)
				bool set(ALLEGRO_BITMAP*);
				// set bitmap with no del
				bool set_nodel(ALLEGRO_BITMAP*);

				// need it quick?
				ALLEGRO_BITMAP* quick() const;
				// need it quick?
				ALLEGRO_BITMAP* quick();

				int interpret_drawing_mode(const bitmap::drawing_mode) const;
			public:
				Bitmap();

				/// <summary>
				/// <para>Clones (the reference) of the internal Bitmap.</para>
				/// </summary>
				/// <param name="{Bitmap}">A Bitmap to copy the reference.</param>
				Bitmap(const Bitmap&);

				/// <summary>
				/// <para>Moves the internal Bitmap's data to this.</para>
				/// </summary>
				/// <param name="{Bitmap}">A Bitmap to move data from.</param>
				Bitmap(Bitmap&&) noexcept;

				/// <summary>
				/// <para>Copy operator (+1 reference).</para>
				/// </summary>
				/// <param name="{Bitmap}">A Bitmap to copy the reference.</param>
				void operator=(const Bitmap&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{Bitmap}">A Bitmap to move from.</param>
				void operator=(Bitmap&&) noexcept;

				/// <summary>
				/// <para>Creates a Bitmap with the exact size of this Bitmap if needed. Sub bitmaps will fail.</para>
				/// </summary>
				/// <param name="{Bitmap}">The reference size.</param>
				/// <param name="{bool}">Redraw this into new this if new? (sets reference as target after copy).</param>
				/// <param name="{float}">Proportion (scalable based on reference, 1.0 means 100%).</param>
				/// <returns>{int} 0 fail, 1 good, 2 had to resize.</returns>
				int copy_attributes(const Bitmap&, const bool = true, const double = 1.0);

				/// <summary>
				/// <para>If you change default bitmap flags or recreated display, call this so everyone is converted to new standards.</para>
				/// <para>ALLEGRO_VIDEO_BITMAP is forced by default.</para>
				/// </summary>
				static void check_bitmaps_memory();

				/// <summary>
				/// <para>Create a blank empty transparent Bitmap.</para>
				/// </summary>
				/// <param name="{int}">Its width.</param>
				/// <param name="{int}">Its height.</param>
				/// <returns>{bool} True if success.</returns>
				bool create(const int, const int);

				/// <summary>
				/// <para>Creates a sub-bitmap (a trimmed reference of a Bitmap).</para>
				/// <para>THIS DOES NOT COPY! If the referenced Bitmap dies, this will become invalid.</para>
				/// </summary>
				/// <param name="{Bitmap}">The Bitmap reference.</param>
				/// <param name="{int}">Position X (top left is 0).</param>
				/// <param name="{int}">Position Y (top left is 0).</param>
				/// <param name="{int}">Width.</param>
				/// <param name="{int}">Height.</param>
				/// <returns>{bool} True if success.</returns>
				bool create_sub(const Bitmap&, const int, const int, const int, const int);

				/// <summary>
				/// <para>Unreference, reset to clean Bitmap.</para>
				/// </summary>
				void reset();

				/// <summary>
				/// <para>If there's something set, true.</para>
				/// </summary>
				/// <returns>{bool} True if exists.</returns>
				operator bool() const;

				/// <summary>
				/// <para>Checks valid bitmap.</para>
				/// </summary>
				/// <returns>{bool} True if doesn't exist (!exist).</returns>
				const bool operator!() const;

				/// <summary>
				/// <para>Compares if bitmap is the same (same reference).</para>
				/// </summary>
				/// <param name="{Bitmap}">Another Bitmap.</param>
				/// <returns>{bool} True if they reference the same memory.</returns>
				const bool operator==(Bitmap&) const;

				/// <summary>
				/// <para>In rare cases you might want to cast this to a RAW BITMAP.</para>
				/// </summary>
				operator ALLEGRO_BITMAP*() const;

				/// <summary>
				/// <para>Checks if it's set.</para>
				/// </summary>
				/// <returns>{bool} True if empty.</returns>
				const bool empty() const;

				/// <summary>
				/// <para>Sets new Bitmaps flags.</para>
				/// <para>This is a global setting.</para>
				/// </summary>
				/// <param name="{int}">Properties.</param>
				static void set_new_bitmap_flags(const int = bitmap::default_new_bitmap_flags);

				/// <summary>
				/// <para>Load Bitmap from file.</para>
				/// <para>This is the RAW path, %tags% won't work. Use Tools::interpret_path.</para>
				/// </summary>
				/// <param name="{std::string}">Path.</param>
				/// <returns>{bool} True if success.</returns>
				bool load(const std::string&);

				/// <summary>
				/// <para>Save Bitmap to file.</para>
				/// <para>This is the RAW path, %tags% won't work. Use Tools::interpret_path.</para>
				/// </summary>
				/// <param name="{std::string}">Path.</param>
				/// <returns>{bool} True if success.</returns>
				bool save(const std::string&);

				/// <summary>
				/// <para>Forces a RAW BITMAP to be its value (it won't delete it afterwards).</para>
				/// </summary>
				/// <param name="{RAW BITMAP}">The RAW BITMAP.</param>
				void force(ALLEGRO_BITMAP*);

				/// <summary>
				/// <para>Clones a Bitmap (not reference).</para>
				/// </summary>
				/// <param name="{Bitmap}">Bitmap to clone.</param>
				/// <returns>{bool} True if success.</returns>
				bool clone(const Bitmap&);

				/// <summary>
				/// <para>Get Bitmap flags.</para>
				/// </summary>
				/// <returns>{int} Bitmap flags.</returns>
				const int get_flags() const;

				/// <summary>
				/// <para>Get Bitmap format.</para>
				/// </summary>
				/// <returns>{int} Bitmap format.</returns>
				const int get_format() const;

				/// <summary>
				/// <para>Get Bitmap width.</para>
				/// </summary>
				/// <returns>{int} Bitmap width.</returns>
				const int get_width() const;

				/// <summary>
				/// <para>Get Bitmap height.</para>
				/// </summary>
				/// <returns>{int} Bitmap height.</returns>
				const int get_height() const;

				/// <summary>
				/// <para>Gets a specific pixel information.</para>
				/// </summary>
				/// <param name="{int}">Position X.</param>
				/// <param name="{int}">Position Y.</param>
				/// <returns>{Color} The RGB color there.</returns>
				const Color get_pixel(const int, const int) const;

				/// <summary>
				/// <para>Deep data locking mechanism, if you want to read raw data.</para>
				/// <para>This will lock the whole internal Bitmap and its references. Anything done while locked is undefined behaviour.</para>
				/// </summary>
				/// <param name="{int}">Format (you can use itself format, get_format()).</param>
				/// <param name="{int}">Flag (ALLEGRO_LOCK_READONLY, ALLEGRO_LOCK_WRITEONLY, ALLEGRO_LOCK_READWRITE).</param>
				/// <returns>{RAW LOCKED REGION} RAW LOCKED REGION pointer for RAW stuff (or nullptr if FAILED).</returns>
				ALLEGRO_LOCKED_REGION* lock(const int, const int);

				/// <summary>
				/// <para>Deep data locking mechanism, if you want to read raw data.</para>
				/// <para>This will lock the internal Bitmap and its references. Anything done while locked is undefined behaviour.</para>
				/// </summary>
				/// <param name="{int}">Position X.</param>
				/// <param name="{int}">Position Y.</param>
				/// <param name="{int}">Width.</param>
				/// <param name="{int}">Height.</param>
				/// <param name="{int}">Format (you can use itself format, get_format()).</param>
				/// <param name="{int}">Flag (ALLEGRO_LOCK_READONLY, ALLEGRO_LOCK_WRITEONLY, ALLEGRO_LOCK_READWRITE).</param>
				/// <returns>{RAW LOCKED REGION} RAW LOCKED REGION pointer for RAW stuff (or nullptr if FAILED).</returns>
				ALLEGRO_LOCKED_REGION* lock(const int, const int, const int, const int, const int, const int);

				/// <summary>
				/// <para>Unlocks (if locked somehow).</para>
				/// </summary>
				void unlock();

				/// <summary>
				/// <para>Is the bitmap locked?</para>
				/// </summary>
				/// <returns>{bool} True if locked.</returns>
				const bool is_locked() const;

				/// <summary>
				/// <para>Gets if this bitmap is compatible with the current target Display (has to be in same thread).</para>
				/// <para>If it's not compatible, drawing will be probably very slow.</para>
				/// </summary>
				/// <returns>{bool} True if compatible.</returns>
				const bool is_compatible_with_current_display() const;

				/// <summary>
				/// <para>Is this a fully created or loaded bitmap or is it a sub Bitmap?</para>
				/// </summary>
				/// <returns>{bool} True if it's a sub Bitmap</returns>
				const bool is_sub() const;

				/// <summary>
				/// <para>Clear the bitmap to this Color.</para>
				/// </summary>
				/// <param name="{Color}">A Color to fill the Bitmap with.</param>
				void clear_to_color(Color);

				/// <summary>
				/// <para>Draws the Bitmap.</para>
				/// </summary>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{bitmap::drawing_mode}">How it should draw.</param>
				void draw(const float, const float, const bitmap::drawing_mode = bitmap::drawing_mode::DEFAULT) const;

				/// <summary>
				/// <para>Draws the Bitmap.</para>
				/// </summary>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{float}">Width.</param>
				/// <param name="{float}">Height.</param>
				/// <param name="{bitmap::drawing_mode}">How it should draw.</param>
				void draw(const float, const float, const float, const float, const bitmap::drawing_mode = bitmap::drawing_mode::DEFAULT) const;

				/// <summary>
				/// <para>Draws the Bitmap.</para>
				/// </summary>
				/// <param name="{float}">Bitmap's cut X</param>
				/// <param name="{float}">Bitmap's cut Y</param>
				/// <param name="{float}">Bitmap's cut Width.</param>
				/// <param name="{float}">Bitmap's cut Height.</param>
				/// <param name="{float}">Target's X</param>
				/// <param name="{float}">Target's Y</param>
				/// <param name="{float}">Target's Width.</param>
				/// <param name="{float}">Target's Height.</param>
				/// <param name="{bitmap::drawing_mode}">How it should draw.</param>
				void draw(const float, const float, const float, const float, const float, const float, const float, const float, const bitmap::drawing_mode = bitmap::drawing_mode::DEFAULT) const;

				/// <summary>
				/// <para>Draws the Bitmap.</para>
				/// </summary>
				/// <param name="{Color}">Color Filter.</param>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{float}">Width.</param>
				/// <param name="{float}">Height.</param>
				/// <param name="{bitmap::drawing_mode}">How it should draw.</param>
				void draw(Color, const float, const float, const float, const float, const bitmap::drawing_mode = bitmap::drawing_mode::DEFAULT) const;

				/// <summary>
				/// <para>Draws the Bitmap.</para>
				/// </summary>
				/// <param name="{Color}">Color Filter.</param>
				/// <param name="{float}">Bitmap's cut X</param>
				/// <param name="{float}">Bitmap's cut Y</param>
				/// <param name="{float}">Bitmap's cut Width.</param>
				/// <param name="{float}">Bitmap's cut Height.</param>
				/// <param name="{float}">Target's X</param>
				/// <param name="{float}">Target's Y</param>
				/// <param name="{float}">Target's Width.</param>
				/// <param name="{float}">Target's Height.</param>
				/// <param name="{bitmap::drawing_mode}">How it should draw.</param>
				void draw(Color, const float, const float, const float, const float, const float, const float, const float, const float, const bitmap::drawing_mode = bitmap::drawing_mode::DEFAULT) const;

				/// <summary>
				/// <para>Draws the Bitmap.</para>
				/// </summary>
				/// <param name="{float}">Center point X in Bitmap.</param>
				/// <param name="{float}">Center point Y in Bitmap.</param>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{float}">Scale X.</param>
				/// <param name="{float}">Scale Y.</param>
				/// <param name="{float}">Angle (rad).</param>
				/// <param name="{bitmap::drawing_mode}">How it should draw.</param>
				void draw(const float, const float, const float, const float, const float, const float, const float = 0.0, const bitmap::drawing_mode = bitmap::drawing_mode::DEFAULT) const;

				/// <summary>
				/// <para>Draws the Bitmap.</para>
				/// </summary>
				/// <param name="{Color}">Color Filter.</param>
				/// <param name="{float}">Center point X in Bitmap.</param>
				/// <param name="{float}">Center point Y in Bitmap.</param>
				/// <param name="{float}">Position X.</param>
				/// <param name="{float}">Position Y.</param>
				/// <param name="{float}">Scale X.</param>
				/// <param name="{float}">Scale Y.</param>
				/// <param name="{float}">Angle (rad).</param>
				/// <param name="{bitmap::drawing_mode}">How it should draw.</param>
				void draw(Color, const float, const float, const float, const float, const float, const float, const float = 0.0, const bitmap::drawing_mode = bitmap::drawing_mode::DEFAULT) const;

				/// <summary>
				/// <para>Chroma key, specific color, like those PINK as "transparent" assets.</para>
				/// </summary>
				/// <param name="{Color}">The color to remove and transform to transparent.</param>
				void mask_off(Color);

				/// <summary>
				/// <para>Set this as the target (so you can draw others into this).</para>
				/// </summary>
				void set_as_target() const;
			};

		}
	}
}