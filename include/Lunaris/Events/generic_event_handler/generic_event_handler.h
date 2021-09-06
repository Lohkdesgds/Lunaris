#pragma once

#include <allegro5/allegro.h>
#include <functional>

#include "../shared_start.h"

namespace Lunaris {

	// generic all_in_one handler (to be implemented)
	
	class generic_event_handler : public __common_event {

		std::function<void(const ALLEGRO_EVENT&)> generic_event;

		void handle_events(const ALLEGRO_EVENT&);

	public:
		generic_event_handler();

		void install_keyboard();
		void install_mouse();
		void install_joystick();
		void install_touch();

		void install_other(ALLEGRO_EVENT_SOURCE*);

		void uninstall_keyboard();
		void uninstall_mouse();
		void uninstall_joystick();
		void uninstall_touch();

		void uninstall_other(ALLEGRO_EVENT_SOURCE*);

		void hook_event_handler(const std::function<void(const ALLEGRO_EVENT&)>);
		void unhook_event_handler();
	};
}