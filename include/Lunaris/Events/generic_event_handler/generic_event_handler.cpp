#include "generic_event_handler.h"

namespace Lunaris {

	void generic_event_handler::handle_events(const ALLEGRO_EVENT& ev)
	{
		if (generic_event) generic_event(ev);
	}

	generic_event_handler::generic_event_handler() : __common_event()
	{
	}

	void generic_event_handler::install_keyboard()
	{
		__keyboard_allegro_start();
		install_other(al_get_keyboard_event_source());
	}

	void generic_event_handler::install_mouse()
	{
		__mouse_allegro_start();
		install_other(al_get_mouse_event_source());
	}

	void generic_event_handler::install_joystick()
	{
		__joystick_allegro_start();
		install_other(al_get_joystick_event_source());
	}

	void generic_event_handler::install_touch()
	{
		__touch_allegro_start();
		install_other(al_get_touch_input_event_source());
	}

	void generic_event_handler::install_other(ALLEGRO_EVENT_SOURCE* even)
	{
		auto lucky = get_lock();
		if (even && !al_is_event_source_registered(get_event_queue(), even)) 
			al_register_event_source(get_event_queue(), even);
	}

	void generic_event_handler::uninstall_keyboard()
	{
		uninstall_other(al_get_keyboard_event_source());
	}

	void generic_event_handler::uninstall_mouse()
	{
		uninstall_other(al_get_mouse_event_source());
	}

	void generic_event_handler::uninstall_joystick()
	{
		uninstall_other(al_get_joystick_event_source());
	}

	void generic_event_handler::uninstall_touch()
	{
		uninstall_other(al_get_touch_input_event_source());
	}

	void generic_event_handler::uninstall_other(ALLEGRO_EVENT_SOURCE* even)
	{
		auto lucky = get_lock();
		if (even && al_is_event_source_registered(get_event_queue(), even))
			al_unregister_event_source(get_event_queue(), even);
	}

	void generic_event_handler::hook_event_handler(const std::function<void(const ALLEGRO_EVENT&)> f)
	{
		auto lucky = get_lock();
		generic_event = f;
	}

	void generic_event_handler::unhook_event_handler()
	{
		auto lucky = get_lock();
		generic_event = {};
	}

}