#include "shared_start.h"

namespace Lunaris {

	LUNARIS_DECL void __keyboard_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_keyboard_installed() && !al_install_keyboard()) throw std::runtime_error("Can't start Keyboard!");
	}

	LUNARIS_DECL void __mouse_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_mouse_installed() && !al_install_mouse()) throw std::runtime_error("Can't start Mouse!");
	}

	LUNARIS_DECL void __joystick_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_joystick_installed() && !al_install_joystick()) throw std::runtime_error("Can't start Joystick!");
	}

	LUNARIS_DECL void __touch_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_touch_input_installed() && !al_install_touch_input()) throw std::runtime_error("Can't start Touch!");
	}

	LUNARIS_DECL void __generic_events_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
	}

	namespace specific {

		LUNARIS_DECL ALLEGRO_EVENT_SOURCE* __new_user_event()
		{
			ALLEGRO_EVENT_SOURCE* ev = new ALLEGRO_EVENT_SOURCE();
			al_init_user_event_source(ev);
			return ev;
		}

		LUNARIS_DECL void __del_user_event(ALLEGRO_EVENT_SOURCE* ev)
		{
			al_destroy_user_event_source(ev);
			delete ev;
		}
	}

	LUNARIS_DECL user_unique make_unique_user_event_source()
	{
		__generic_events_start();
		return user_unique(specific::__new_user_event(), specific::__del_user_event);
	}

	LUNARIS_DECL user_shared make_shared_user_event_source()
	{
		__generic_events_start();
		return user_shared(specific::__new_user_event(), specific::__del_user_event);
	}

	LUNARIS_DECL queue_unique make_unique_queue()
	{
		__generic_events_start();
		return queue_unique(al_create_event_queue(), al_destroy_event_queue);
	}

	LUNARIS_DECL queue_shared make_shared_queue()
	{
		__generic_events_start();
		return queue_shared(al_create_event_queue(), al_destroy_event_queue);
	}

	LUNARIS_DECL timer_unique make_unique_timer(const double dt, const bool st)
	{
		__generic_events_start();
		auto _r = timer_unique(al_create_timer(dt), al_destroy_timer);
		if (st) al_start_timer(_r.get());
		return _r;
	}

	LUNARIS_DECL timer_shared make_shared_timer(const double dt, const bool st)
	{
		__generic_events_start();
		auto _r = timer_shared(al_create_timer(dt), al_destroy_timer);
		if (st) al_start_timer(_r.get());
		return _r;
	}

}