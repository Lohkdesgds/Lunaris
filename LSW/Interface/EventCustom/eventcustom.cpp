#include "eventcustom.h"


namespace LSW {
	namespace v5 {
		namespace Interface {

			void EventCustom::start_as_custom()
			{
				core = std::shared_ptr<ALLEGRO_EVENT_SOURCE>(
					[] { auto i = new ALLEGRO_EVENT_SOURCE(); al_init_user_event_source(i); return i; }(),
					[](ALLEGRO_EVENT_SOURCE* ev) { al_destroy_user_event_source(ev); });
			}

			EventCustom::EventCustom() : Event()
			{
				Handling::init_basic();
			}

			EventCustom::EventCustom(const EventCustom& e) : Event(e)
			{
				Handling::init_basic();
				start_as_custom();
			}

			EventCustom::EventCustom(EventCustom&& e) : Event(std::move(e))
			{
				Handling::init_basic();
				start_as_custom();
			}

			void EventCustom::send_custom_event(const int at, const intptr_t a, const intptr_t b, const intptr_t c, const intptr_t d)
			{
				if (!core) return;
				ALLEGRO_EVENT ev;
				ev.type = at;
				ev.user.data1 = a;
				ev.user.data2 = b;
				ev.user.data3 = c;
				ev.user.data4 = d;
				ev.user.source = core.get();
				ev.user.timestamp = al_get_time();
				al_emit_user_event(core.get(), &ev, NULL);
			}

			void EventCustom::send_custom_event_alt(const int at, const custom_alt& data)
			{
				if (!core) return;
				ALLEGRO_EVENT ev;
				ev.type = at;
				ev.user.data1 = data.data1;
				ev.user.data2 = data.data2;
				ev.user.data3 = data.data3;
				ev.user.data4 = data.data4;
				ev.user.source = core.get();
				ev.user.timestamp = al_get_time();
				al_emit_user_event(core.get(), &ev, NULL);
			}

		}
	}
}