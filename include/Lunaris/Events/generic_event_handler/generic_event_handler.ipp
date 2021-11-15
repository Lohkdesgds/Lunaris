#include "generic_event_handler.h"
#pragma once

namespace Lunaris {

	template<class EventHandlerType, class SourceClass>
	inline void specific_event_handler<EventHandlerType, SourceClass>::handle_events(const ALLEGRO_EVENT& ev)
	{
		if (ev.type == ALLEGRO_EVENT_TIMER && ev.timer.source == timer_check) {
			check_time();
			return;
		}
		if (generic_event) {
			EventHandlerType bld{ __ref, ev };
			generic_event(bld);
		}
	}

	template<class EventHandlerType, class SourceClass>
	inline void specific_event_handler<EventHandlerType, SourceClass>::check_time()
	{
		ALLEGRO_EVENT_SOURCE* nev = __ref;
		if (nev == last_event_source) return;

		if (last_event_source) al_unregister_event_source(get_event_queue(), last_event_source);
		last_event_source = nev;
		if (last_event_source) al_register_event_source(get_event_queue(), last_event_source);
	}

	template<class EventHandlerType, class SourceClass>
	inline specific_event_handler<EventHandlerType, SourceClass>::specific_event_handler(SourceClass& rf)
		: __common_event(), __ref(rf)
	{
		timer_check = al_create_timer(0.5);
		al_start_timer(timer_check);
		al_register_event_source(get_event_queue(), al_get_timer_event_source(timer_check));

		check_time();
	}

	template<class EventHandlerType, class SourceClass>
	inline specific_event_handler<EventHandlerType, SourceClass>::~specific_event_handler()
	{
		if (last_event_source) al_unregister_event_source(get_event_queue(), last_event_source);
		al_unregister_event_source(get_event_queue(), al_get_timer_event_source(timer_check));
		this->stop();
		al_destroy_timer(timer_check);
	}

	template<class EventHandlerType, class SourceClass>
	inline void specific_event_handler<EventHandlerType, SourceClass>::hook_event_handler(const std::function<void(EventHandlerType&)> f)
	{
		auto lucky = get_lock();
		generic_event = f;
	}

	template<class EventHandlerType, class SourceClass>
	inline void specific_event_handler<EventHandlerType, SourceClass>::unhook_event_handler()
	{
		auto lucky = get_lock();
		generic_event = {};
	}

}