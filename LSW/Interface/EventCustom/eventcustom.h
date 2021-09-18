#pragma once

#include "../Events/events.h"


namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>If you want 4 intptr_t to char[32] and vice-versa, you can use this.</para>
			/// </summary>
			union custom_alt {
				char data[sizeof(intptr_t)*4];
				struct {
					intptr_t data1;
					intptr_t data2;
					intptr_t data3;
					intptr_t data4;
				};
			};

			/// <summary>
			/// <para>A custom User Event. This has to exist while EventHandler is tied to this.</para>
			/// </summary>
			class EventCustom : public Event {
				void start_as_custom();
			public:
				EventCustom();

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{EventCustom}">EventCustom to copy reference.</param>
				EventCustom(const EventCustom&);

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{EventCustom}">EventCustom to move.</param>
				EventCustom(EventCustom&&);

				EventCustom(ALLEGRO_EVENT_SOURCE*) = delete;

				/// <summary>
				/// <para>Send some custom data.</para>
				/// <para>You can send up to 32 bytes of data per event (via up to 4 unsigned long long).</para>
				/// </summary>
				/// <param name="{int}">Event ID (DON'T USE 0) (you can use this to do many event types in custom event source(s)).</param>
				/// <param name="{intptr_t}">Data slot.</param>
				/// <param name="{intptr_t}">Data slot.</param>
				/// <param name="{intptr_t}">Data slot.</param>
				/// <param name="{intptr_t}">Data slot.</param>
				void send_custom_event(const int, const intptr_t, const intptr_t, const intptr_t, const intptr_t);

				/// <summary>
				/// <para>Send some custom data.</para>
				/// <para>You can send up to 32 bytes of data per event (via up to 4 unsigned long long).</para>
				/// </summary>
				/// <param name="{int}">In this case normally this is used as "how many left".</param>
				/// <param name="{custom_alt}">A struct with an array that combine the fours slots into 32 bit string (you'll have to know what you're getting in the other side).</param>
				void send_custom_event_alt(const int, const custom_alt&);
			};

		}
	}
}