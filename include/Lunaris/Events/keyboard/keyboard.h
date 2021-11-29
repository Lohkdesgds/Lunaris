#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/generic_event_handler.h>

#include <allegro5/allegro.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>

namespace Lunaris {

	/// <summary>
	/// <para>keyboard is meant to handle keyboard input as string.</para>
	/// <para>Each key press can be combined to a modifier and generate a string. This class does this for you.</para>
	/// <para>Keyboard events on a display can be easily translated to pure UTF8 string on the fly.</para>
	/// </summary>
	class keyboard : protected generic_event_handler {
		std::vector<int> data;
		std::function<void(keyboard&, const int)> each_key; // all keyboard keys
		std::function<void(keyboard&, const std::string&)> each_combined, enter_combined; // only valid input

		std::string gen_output() const;
		void handle_events(const ALLEGRO_EVENT&);

		void tool_conv(std::string&, const size_t) const; // to, pos in vector
	public:
		keyboard();

		/// <summary>
		/// <para>Hook a function to read each individual key press.</para>
		/// </summary>
		/// <param name="{function}">A function that reads integer key values (raw).</param>
		void hook_each_key_event(const std::function<void(keyboard&, const int)>);

		/// <summary>
		/// <para>Hook a function that will get the latest combined string (so far).</para>
		/// <para>Useful when you want to show the user their input in real time on screen.</para>
		/// </summary>
		/// <param name="{function}">A function that gets the current string translated input in UTF8.</param>
		void hook_each_key_phrase_event(const std::function<void(keyboard&, const std::string&)>);

		/// <summary>
		/// <para>Hook a function to get the final (pos-ENTER) user input!</para>
		/// </summary>
		/// <param name="{function}">A function to get the final string in UTF8.</param>
		void hook_enter_line_phrase_event(const std::function<void(keyboard&, const std::string&)>);

		/// <summary>
		/// <para>Unhook the each-key hooked function.</para>
		/// </summary>
		void unhook_each_key_event();

		/// <summary>
		/// <para>Unhook the latest string hooked function.</para>
		/// </summary>
		void unhook_each_key_phrase_event();

		/// <summary>
		/// <para>Unhook the final string function.</para>
		/// </summary>
		void unhook_enter_line_phrase_event();

		/// <summary>
		/// <para>Force clear the input. This will clear any combo buffer.</para>
		/// </summary>
		void clear();

		using generic_event_handler::hook_exception_handler;
		using generic_event_handler::unhook_exception_handler;
	};
}