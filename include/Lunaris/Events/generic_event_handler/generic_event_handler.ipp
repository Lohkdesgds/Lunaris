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
		std::vector<ALLEGRO_EVENT_SOURCE*> nevs = __ref;

		std::vector<ALLEGRO_EVENT_SOURCE*> removing;
		std::vector<ALLEGRO_EVENT_SOURCE*> adding;

		for (const auto& i : nevs)
		{
			if (i == nullptr) continue;
			if (std::find(last_event_source.begin(), last_event_source.end(), i) == last_event_source.end()) 
			{ // new one
				adding.push_back(i);
			}
		}

		for (const auto& i : last_event_source)
		{
			if (std::find(nevs.begin(), nevs.end(), i) == nevs.end())
			{ // remove one
				if (i == nullptr) continue;
				removing.push_back(i);
			}
		}

		if (removing.empty() && adding.empty()) return;

		printf_s("changed event source\n");

		for(auto& e : adding) if (e) al_register_event_source(get_event_queue(), e);
		for(auto& e : removing) if (e) al_unregister_event_source(get_event_queue(), e);

		last_event_source = nevs;
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
		for (auto& e : last_event_source) if (e) al_unregister_event_source(get_event_queue(), e);
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