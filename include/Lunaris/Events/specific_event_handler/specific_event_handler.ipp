#include "specific_event_handler.h"
#pragma once

namespace Lunaris {

	template<class EventHandlerType, class SourceClass>
	inline void specific_event_handler<EventHandlerType, SourceClass>::handle_events(const ALLEGRO_EVENT& ev)
	{
		if (ev.type == ALLEGRO_EVENT_TIMER && ev.timer.source == timer.get()) {
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
	
		for (auto& e : adding) if (e) get_core().add_event_source(e);
		for (auto& e : removing) if (e) get_core().remove_event_source(e);
	
		last_event_source = nevs;
	}
	
	template<class EventHandlerType, class SourceClass>
	inline specific_event_handler<EventHandlerType, SourceClass>::specific_event_handler(SourceClass& rf)
		: generic_event_handler(), __ref(rf), timer(make_unique_timer(0.5, true))
	{
		install(timer);
		get_core().set_event_handler([this](const ALLEGRO_EVENT& ev) { handle_events(ev); });
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("%p is SPECIFIC object (class: %s)", get_core_ptr(), typeid(SourceClass).name());
#endif
		check_time();
	}
	
	template<class EventHandlerType, class SourceClass>
	inline void specific_event_handler<EventHandlerType, SourceClass>::hook_event_handler(const std::function<void(EventHandlerType&)> f)
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		generic_event = f;
	}
	
	template<class EventHandlerType, class SourceClass>
	inline void specific_event_handler<EventHandlerType, SourceClass>::unhook_event_handler()
	{
		std::lock_guard<std::recursive_mutex> luck(get_core().m_safe);
		generic_event = {};
	}

}