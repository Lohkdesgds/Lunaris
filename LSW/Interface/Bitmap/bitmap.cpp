#include "bitmap.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool Bitmap::first_time = true;

			bool Bitmap::set(ALLEGRO_BITMAP* nptr)
			{
				if (!nptr) return false;
				Tools::AdvancedShared<ALLEGRO_BITMAP> s(nptr, [](ALLEGRO_BITMAP* b) { if (al_is_system_installed() && b) { al_destroy_bitmap(b); b = nullptr; } });
				//bitmap = [s] {return s.get(); };
				if (!bitmap_orig) bitmap_orig = std::move(s);
				else {
					bitmap_orig.swap_destroy(s);
				}
				return !bitmap_orig.null();
			}

			bool Bitmap::set_nodel(ALLEGRO_BITMAP* nptr)
			{
				if (!nptr) return false;
				Tools::AdvancedShared<ALLEGRO_BITMAP> s(nptr, [](ALLEGRO_BITMAP* b) { });
				//bitmap = [s] {return s.get(); };
				if (!bitmap_orig) bitmap_orig = std::move(s);
				else {
					bitmap_orig.swap(s);
				}
				return !bitmap_orig.null();
			}

			ALLEGRO_BITMAP* Bitmap::quick() const
			{
				//if (use_target_as_it) return get_global_reference() ? get_global_reference() : nullptr;
				if (!bitmap_orig.null()) return bitmap_orig.get();
				return nullptr;
			}

			ALLEGRO_BITMAP* Bitmap::quick()
			{
				//if (use_target_as_it) return get_global_reference() ? get_global_reference() : nullptr;
				if (!bitmap_orig.null()) return bitmap_orig.get();
				return nullptr;
			}

			int Bitmap::interpret_drawing_mode(const bitmap::drawing_mode e) const
			{
				switch (e) {
				case bitmap::drawing_mode::DEFAULT:
					return 0;
				case bitmap::drawing_mode::FLIP_HORIZONTAL:
					return ALLEGRO_FLIP_HORIZONTAL;
				case bitmap::drawing_mode::FLIP_VERTICAL:
					return ALLEGRO_FLIP_VERTICAL;
				case bitmap::drawing_mode::FLIP_HORIZONTAL_VERTICAL:
					return ALLEGRO_FLIP_HORIZONTAL | ALLEGRO_FLIP_VERTICAL;
				}
				return 0;
			}

			Bitmap::Bitmap()
			{
				Handling::init_basic();
				Handling::init_graphics();

				if (first_time) {
					first_time = false;
					set_new_bitmap_flags(); // first setup
				}
			}

			Bitmap::Bitmap(const Bitmap& oth)
			{
				*this = oth;
			}

			Bitmap::Bitmap(Bitmap&& oth)
			{
				*this = std::move(oth);
			}

			void Bitmap::operator=(const Bitmap& oth)
			{
				bitmap_orig = oth.bitmap_orig;
				//use_target_as_it = oth.use_target_as_it;
			}

			void Bitmap::operator=(Bitmap&& oth)
			{
				bitmap_orig = std::move(oth.bitmap_orig);
				//use_target_as_it = oth.use_target_as_it;
			}

			int Bitmap::copy_attributes(const Bitmap& rf, const bool cpy, const double prop)
			{
				if (is_sub()) return 0;
				int sx, sy;
				sx = rf.get_width() * prop;
				sy = rf.get_height() * prop;

				if (sx <= 0 || sy <= 0) return false;
				if (sx != get_width() || sy != get_height()) {

					Bitmap secure;
					secure.set(al_create_bitmap(sx, sy));

					if (cpy && quick()) {
						ALLEGRO_BITMAP* shh = al_get_target_bitmap();
						secure.set_as_target();
						draw(0, 0, get_width(), get_height(), 0, 0, sx, sy);
						if (shh) al_set_target_bitmap(shh);
					}
					*this = std::move(secure);

					return 2;
				}
				return 1;
			}

			void Bitmap::check_bitmaps_memory()
			{
				al_convert_memory_bitmaps();
			}

			bool Bitmap::create(const int w, const int h)
			{
				if (!bitmap_orig) {
					return set(al_create_bitmap(w, h));
				}
				return false;
			}

			bool Bitmap::create_sub(const Bitmap& o, const int x, const int y, const int w, const int h)
			{
				if (!bitmap_orig) {
					return set(al_create_sub_bitmap(o.quick(), x, y, w, h));
				}
				return false;
			}

			void Bitmap::reset()
			{
				if (quick()) *this = Bitmap{};
			}

			Bitmap::operator bool() const
			{
				return !(!bitmap_orig); // ikr but visual studio doesnt
			}

			const bool Bitmap::operator!() const
			{
				return !bitmap_orig;
			}

			const bool Bitmap::operator==(Bitmap& o) const
			{
				return quick() == o.quick();
			}

			Bitmap::operator ALLEGRO_BITMAP*() const
			{
				return quick();
			}

			const bool Bitmap::empty() const
			{
				return !bitmap_orig;
			}

			void Bitmap::set_new_bitmap_flags(const int flags)
			{
				al_set_new_bitmap_flags(flags);
				check_bitmaps_memory();
			}

			bool Bitmap::load(const char* p)
			{
				if (!bitmap_orig) {
					return set(al_load_bitmap(p));
				}
				return false;
			}

			bool Bitmap::save(const char* p)
			{
				if (!bitmap_orig.null()) {
					return al_save_bitmap(p, quick());
				}
				return false;
			}

			void Bitmap::force(ALLEGRO_BITMAP* b)
			{
				if (b) set_nodel(b);
			}

			bool Bitmap::clone(const Bitmap& o)
			{
				if (!bitmap_orig) {
					return set(al_clone_bitmap(o.quick()));
				}
				return false;
			}

			const int Bitmap::get_flags() const
			{
				if (auto q = quick(); q) return al_get_bitmap_flags(q);
				return -1;
			}

			const int Bitmap::get_format() const
			{
				if (auto q = quick(); q) return al_get_bitmap_format(q);
				return -1;
			}

			const int Bitmap::get_width() const
			{
				if (auto q = quick(); q) return al_get_bitmap_width(q);
				return -1;
			}

			const int Bitmap::get_height() const
			{
				if (auto q = quick(); q) return al_get_bitmap_height(q);
				return -1;
			}

			const Color Bitmap::get_pixel(const int x, const int y) const
			{
				if (auto q = quick(); q) return al_get_pixel(q, x, y);
				return Color();
			}

			ALLEGRO_LOCKED_REGION* Bitmap::lock(const int format, const int flags)
			{
				return quick() ? al_lock_bitmap(quick(), format, flags) : nullptr;
			}

			ALLEGRO_LOCKED_REGION* Bitmap::lock(const int x, const int y, const int w, const int h, const int format, const int flags)
			{
				return quick() ? al_lock_bitmap_region(quick(), x, y, w, h, format, flags) : nullptr;
			}

			void Bitmap::unlock()
			{
				if (quick() && is_locked()) al_unlock_bitmap(quick());
			}

			const bool Bitmap::is_locked() const
			{
				if (auto q = quick(); q) return al_is_bitmap_locked(q);
				return false;
			}

			const bool Bitmap::is_compatible_with_current_display() const
			{
				if (auto q = quick(); q) return al_is_compatible_bitmap(q);
				return false;
			}

			const bool Bitmap::is_sub() const
			{
				if (auto q = quick(); q) return al_is_sub_bitmap(q);
				return false;
			}

			void Bitmap::clear_to_color(Color c)
			{
				if (auto q = quick(); q) {
					ALLEGRO_BITMAP* cpy = al_get_target_bitmap();
					al_set_target_bitmap(q);
					al_clear_to_color(c);
					if (cpy) al_set_target_bitmap(cpy);
				}
			}

			void Bitmap::draw(const float x, const float y, const bitmap::drawing_mode f) const
			{
				if (auto q = quick(); q) al_draw_bitmap(q, x, y, interpret_drawing_mode(f));
			}

			void Bitmap::draw(const float x, const float y, const float w, const float h, const bitmap::drawing_mode f) const
			{
				if (auto q = quick(); q) al_draw_scaled_bitmap(q, 0, 0, get_width(), get_height(), x, y, w, h, interpret_drawing_mode(f));
			}

			void Bitmap::draw(const float x1, const float y1, const float w1, const float h1, const float x2, const float y2, const float w2, const float h2, const bitmap::drawing_mode f) const
			{
				if (auto q = quick(); q) al_draw_scaled_bitmap(q, x1, y1, w1, h1, x2, y2, w2, h2, interpret_drawing_mode(f));
			}

			void Bitmap::draw(Color c, const float x, const float y, const float w, const float h, const bitmap::drawing_mode f) const
			{
				if (auto q = quick(); q) al_draw_tinted_scaled_bitmap(q, c, 0, 0, get_width(), get_height(), x, y, w, h, interpret_drawing_mode(f));
			}

			void Bitmap::draw(Color c, const float x1, const float y1, const float w1, const float h1, const float x2, const float y2, const float w2, const float h2, const bitmap::drawing_mode f) const
			{
				if (auto q = quick(); q) al_draw_tinted_scaled_bitmap(q, c, x1, y1, w1, h1, x2, y2, w2, h2, interpret_drawing_mode(f));
			}

			void Bitmap::draw(const float cx, const float cy, const float px, const float py, const float sx, const float sy, const float a, const bitmap::drawing_mode f) const
			{
				if (auto q = quick(); q) al_draw_scaled_rotated_bitmap(q, cx, cy, px, py, sx, sy, a, interpret_drawing_mode(f));
			}

			void Bitmap::draw(Color c, const float cx, const float cy, const float px, const float py, const float sx, const float sy, const float a, const bitmap::drawing_mode f) const
			{
				if (auto q = quick(); q) al_draw_tinted_scaled_rotated_bitmap(q, c, cx, cy, px, py, sx, sy, a, interpret_drawing_mode(f));
			}

			void Bitmap::mask_off(Color c)
			{
				if (auto q = quick(); q) al_convert_mask_to_alpha(q, c);
			}

			void Bitmap::set_as_target() const
			{
				if (auto q = quick(); q) {
					al_set_target_bitmap(q);
				}
			}

			// implementation

			template Tools::AdvancedShared<ALLEGRO_BITMAP>;
			template Tools::Resource<Bitmap>;

		}
	}
}