#pragma once

#include <allegro5/allegro.h>
#include <functional>
#include <mutex>
#include <thread>

#include "../shared_start.h"

namespace Lunaris {

	class keys : public __common_event {
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
		~keys();

		void hook_event(const std::function<void(const key_event&)>);
		void unhook_event();

		bool is_key_pressed(const int); // by keycode
	};
}