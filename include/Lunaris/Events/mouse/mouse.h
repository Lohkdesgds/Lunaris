#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/generic_event_handler.h>

#include <allegro5/allegro.h>
#include <thread>
#include <mutex>
#include <functional>

namespace Lunaris {

	/// <summary>
	/// <para>mouse is a must have when you're working with transform and mice.</para>
	/// <para>This translates the most common values for you automatically (camera-based coords, -1 to 1 compared to screen, translated buttons and events and even the raw information if you really need it)</para>
	/// <para>This NEEDS A DISPLAY WHEN CREATED because it will use that as reference for the coordinate translations (the function is a operator available directly from display class).</para>
	/// </summary>
	class mouse : protected generic_event_handler {
	public:
		struct mouse_event {
			float real_posx = -1.0f, real_posy = 1.0f; // may change if proportion is not 1:1, but lower goes [-1.0f, 1.0f]
			float relative_posx = -1.0f, relative_posy = -1.0f; // [-1.0f, 1.0f]
			int buttons_pressed = 0;
			int buttons_unpressed = 0;
			uint8_t scroll_event = 0; // updated each event.
			ALLEGRO_MOUSE_EVENT raw_mouse_event;
			mouse& mouse_itself;

			mouse_event(mouse&);

			/// <summary>
			/// <para>On event this has only the event flag, else it's the real mouse buttons pressed information.</para>
			/// <para>"0" is the first mouse button.</para>
			/// </summary>
			/// <param name="{int}">Mouse button to check.</param>
			/// <returns>{bool} True if pressed.</returns>
			bool is_button_pressed(const int) const;

			/// <summary>
			/// <para>On event this has only the event flag, else it's the real mouse buttons not pressed information.</para>
			/// <para>"0" is the first mouse button.</para>
			/// </summary>
			/// <param name="{int}">Mouse button to check.</param>
			/// <returns>{bool} True if unpressed.</returns>
			bool is_button_unpressed(const int) const;

			/// <summary>
			/// <para>Had any scroll event?</para>
			/// </summary>
			/// <returns>{bool} True if yes.</returns>
			bool got_scroll_event() const;

			/// <summary>
			/// <para>Get scroll translated to -1, 0 or 1 per ID. The known ones are W (ID 0) and Z (ID 1).</para>
			/// </summary>
			/// <param name="{int}">Scroll ID.</param>
			/// <returns>{int} 1, 0 or -1 depending on the direction.</returns>
			int scroll_event_id(const int) const; // positive = one way, negative = other way. [W, Z]

			/// <summary>
			/// <para>The mouse source.</para>
			/// </summary>
			/// <returns>{mouse&amp;} The mouse source.</returns>
			mouse& get_source();
		};
	private:
		mouse_event mouse_rn;

		std::function<void(const int, const mouse_event&)> event_handler;
		std::function<ALLEGRO_TRANSFORM(void)> current_transform_getter;

		void set_mouse_axis_plus(const int, const int); // which axis (w, ...), value (-1, 1, 0)

		void handle_events(const ALLEGRO_EVENT&);
	public:
		mouse(std::function<ALLEGRO_TRANSFORM(void)>);

		/// <summary>
		/// <para>Hook a function to handle mouse events.</para>
		/// </summary>
		/// <param name="{function}">A function to do something with the mouse information.</param>
		void hook_event(const std::function<void(const int, const mouse_event&)>);

		/// <summary>
		/// <para>Unhook old function tied to mouse events.</para>
		/// </summary>
		void unhook_event();

		/// <summary>
		/// <para>Read the latest mouse value (the values may change, but the memory is valid while the object exists).</para>
		/// </summary>
		/// <returns>{mouse_event} Latest mouse event reference (may update itself while reading).</returns>
		const mouse_event& current_mouse() const;

		using generic_event_handler::hook_exception_handler;
		using generic_event_handler::unhook_exception_handler;
	};

}