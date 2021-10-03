#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/shared_start.h>

#include <allegro5/allegro.h>
#include <thread>
#include <mutex>
#include <functional>

namespace Lunaris {

	const double default_mouse_update_rate = 1.0 / 60;

	class mouse : public __common_event {
	public:
		struct mouse_event {
			float real_posx = -1.0f, real_posy = 1.0f; // may change if proportion is not 1:1, but lower goes [-1.0f, 1.0f]
			float relative_posx = -1.0f, relative_posy = -1.0f; // [-1.0f, 1.0f]
			int buttons_pressed = 0;
			uint8_t scroll_event = 0; // updated each event.

			bool is_button_pressed(const int) const;
			bool got_scroll_event() const;
			int scroll_event_id(const int) const; // positive = one way, negative = other way. [W, Z]
		};
	private:
		mouse_event mouse_rn;
		bool hint_changes_coming = false;

		//bool had_mouse_event = false;
		//float quick_cpy_mouse[2] = { 0.0f,0.0f };

		//ALLEGRO_TIMER* timer = nullptr;
		std::function<void(const int, const mouse_event&)> event_handler;
		std::function<ALLEGRO_TRANSFORM(void)> current_transform_getter;

		void set_mouse_axis_plus(const int, const int); // which axis (w, ...), value (-1, 1, 0)

		void handle_events(const ALLEGRO_EVENT&);
	public:
		mouse(std::function<ALLEGRO_TRANSFORM(void)>);
		~mouse();

		void hook_event(const std::function<void(const int, const mouse_event&)>); // ALLEGRO_EVENT thing, mouse info
		void unhook_event();

		const mouse_event& current_mouse() const;
	};

}