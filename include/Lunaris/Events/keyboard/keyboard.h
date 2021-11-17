#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/generic_event_handler.h>

#include <allegro5/allegro.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>

namespace Lunaris {

	class keyboard : protected generic_event_handler {
		std::vector<int> data;
		std::function<void(keyboard&, const int)> each_key; // all keyboard keys
		std::function<void(keyboard&, const std::string&)> each_combined, enter_combined; // only valid input

		std::string gen_output() const;
		void handle_events(const ALLEGRO_EVENT&);

		void tool_conv(std::string&, const size_t) const; // to, pos in vector
	public:
		keyboard();

		void hook_each_key_event(const std::function<void(keyboard&, const int)>);
		void hook_each_key_phrase_event(const std::function<void(keyboard&, const std::string&)>);
		void hook_enter_line_phrase_event(const std::function<void(keyboard&, const std::string&)>);

		void unhook_each_key_event();
		void unhook_each_key_phrase_event();
		void unhook_enter_line_phrase_event();

		void clear();

		using generic_event_handler::hook_exception_handler;
		using generic_event_handler::unhook_exception_handler;
	};
}