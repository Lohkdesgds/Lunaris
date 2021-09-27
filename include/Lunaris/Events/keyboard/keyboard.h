#pragma once

#include <allegro5/allegro.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>

#include "../shared_start.h"

namespace Lunaris {

	class keyboard : public __common_event {
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
	};
}