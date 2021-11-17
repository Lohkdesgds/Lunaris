#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/generic_event_handler.h>

#include <allegro5/allegro.h>
#include <functional>

namespace Lunaris {

	/// <summary>
	/// <para>Build a generic event handler that handles events with this class.</para>
	/// <para>IMPORTANT NOTE: The class MUST EXIST while THIS THING HERE exists. MAKE SURE IT EXISTS!</para>
	/// </summary>
	/// <typeparam name="{EventHandlerType}"></typeparam>
	template<class EventHandlerType, class SourceClass>
	class specific_event_handler : protected generic_event_handler, public NonMovable {

		std::function<void(EventHandlerType&)> generic_event;
		std::vector<ALLEGRO_EVENT_SOURCE*> last_event_source;
		timer_unique timer;

		void handle_events(const ALLEGRO_EVENT&);
		void check_time();

		SourceClass& __ref;
	public:
		specific_event_handler(SourceClass&);

		void hook_event_handler(const std::function<void(EventHandlerType&)>);
		void unhook_event_handler();

		using generic_event_handler::hook_exception_handler;
		using generic_event_handler::unhook_exception_handler;
	};
}

#include "specific_event_handler.ipp"