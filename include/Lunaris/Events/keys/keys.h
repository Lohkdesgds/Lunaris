#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/generic_event_handler.h>

#include <allegro5/allegro.h>
#include <functional>
#include <mutex>
#include <thread>

namespace Lunaris {

	/// <summary>
	/// <para>keys is a very useful class for gaming.</para>
	/// <para>Wanna know if a key is pressed? Bang. Wanna know when it was pressed or released? Piece of cake.</para>
	/// <para>This is meant to be the easy fast key state buffer/event ready to go.</para>
	/// </summary>
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

		/// <summary>
		/// <para>Hook a function to get every key event.</para>
		/// </summary>
		/// <param name="{function}">A function to handle key_event translated events.</param>
		void hook_event(const std::function<void(const key_event&)>);

		/// <summary>
		/// <para>Unhook any function previously set.</para>
		/// </summary>
		void unhook_event();

		/// <summary>
		/// <para>Get direct information of the key.</para>
		/// <para>This keeps the keyboard information in memory since this object was created. If the user launched the app with a key pressed, this may be wrong then.</para>
		/// </summary>
		/// <param name="{int}">ALLEGRO_KEY_** key value to check.</param>
		/// <returns>{bool} True means it is pressed.</returns>
		bool is_key_pressed(const int); // by keycode

		using generic_event_handler::hook_exception_handler;
		using generic_event_handler::unhook_exception_handler;
	};
}