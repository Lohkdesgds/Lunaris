#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/shared_start.h>

#include <allegro5/allegro.h>
#include <functional>

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
		void install_other(std::vector<ALLEGRO_EVENT_SOURCE*>);

		void uninstall_keyboard();
		void uninstall_mouse();
		void uninstall_joystick();
		void uninstall_touch();

		void uninstall_other(ALLEGRO_EVENT_SOURCE*);
		void uninstall_other(std::vector<ALLEGRO_EVENT_SOURCE*>);

		void hook_event_handler(const std::function<void(const ALLEGRO_EVENT&)>);
		void unhook_event_handler();
	};

	/// <summary>
	/// <para>Build a generic event handler that handles events with this class.</para>
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<class EventHandlerType, class SourceClass>
	class specific_event_handler : public __common_event {

		std::function<void(EventHandlerType&)> generic_event;

		ALLEGRO_TIMER* timer_check = nullptr;
		std::vector<ALLEGRO_EVENT_SOURCE*> last_event_source;

		void handle_events(const ALLEGRO_EVENT&);
		void check_time();

		SourceClass& __ref;
	public:
		specific_event_handler(SourceClass&);
		~specific_event_handler();

		void hook_event_handler(const std::function<void(EventHandlerType&)>);
		void unhook_event_handler();
	};
}

#include "generic_event_handler.ipp"