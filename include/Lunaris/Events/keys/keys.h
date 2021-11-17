#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/generic_event_handler.h>

#include <allegro5/allegro.h>
#include <functional>
#include <mutex>
#include <thread>

namespace Lunaris {

	class keys : protected generic_event_handler {
	public:
		struct key_event {
			int key_id = 0;
			bool down = false;
		};
	private:
		bool keychain[ALLEGRO_KEY_MAX]{ false };

		std::function<void(const key_event&)> event_handler;

		void handle_events(const ALLEGRO_EVENT&);
	public:
		keys();

		void hook_event(const std::function<void(const key_event&)>);
		void unhook_event();

		bool is_key_pressed(const int); // by keycode

		using generic_event_handler::hook_exception_handler;
		using generic_event_handler::unhook_exception_handler;
	};
}