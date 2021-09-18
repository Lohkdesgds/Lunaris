#pragma once

#include "../Events/events.h"


namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace eventtimer {
				const double default_timer_delta = 1.0; // sec, used to start event
			}

			/// <summary>
			/// <para>A timer Event. This has to exist while EventHandler is tied to this.</para>
			/// </summary>
			class EventTimer : public Event {
				std::shared_ptr<ALLEGRO_TIMER> tim;
				void init_timer();
			public:
				EventTimer();

				/// <summary>
				/// <para>Start EventTimer with the time set already.</para>
				/// </summary>
				/// <param name=""></param>
				EventTimer(const double);

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{EventTimer}">EventTimer to copy reference.</param>
				EventTimer(const EventTimer&);

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{EventTimer}">EventTimer to move.</param>
				EventTimer(EventTimer&&);

				EventTimer(ALLEGRO_EVENT_SOURCE*) = delete;


				/// <summary>
				/// <para>Compare if event source is the same.</para>
				/// </summary>
				/// <param name="{EventTimer}">Another EventTimer.</param>
				/// <returns>{bool} True if they are.</returns>
				bool operator==(const EventTimer&) const;

				/// <summary>
				/// <para>Compare if event source is the same.</para>
				/// </summary>
				/// <param name="{RAW EVENT}">Another RAW TIMER EVENT.</param>
				/// <returns>{bool} True if they are.</returns>
				bool operator==(ALLEGRO_TIMER*) const;

				/// <summary>
				/// <para>Set timer delta in seconds.</para>
				/// </summary>
				/// <param name="{double}">Delta in seconds.</param>
				void set_delta(const double);

				/// <summary>
				/// <para>Get timer delta in seconds.</para>
				/// </summary>
				/// <returns>{double} Delta set in seconds.</returns>
				double get_delta() const;

				/// <summary>
				/// <para>Have you started the timer yet?</para>
				/// </summary>
				/// <returns>{bool} True if timer started already.</returns>
				bool running() const;

				/// <summary>
				/// <para>Start timer event.</para>
				/// </summary>
				void start();

				/// <summary>
				/// <para>Stop timer event.</para>
				/// </summary>
				void stop();

			};



			/// <summary>
			/// <para>Compare if events are the same.</para>
			/// </summary>
			/// <param name="{RAW EVENT}">A RAW EVENT.</param>
			/// <param name="{EventTimer}">Another Event.</param>
			/// <returns>{bool} True if they are.</returns>
			bool operator==(ALLEGRO_TIMER*, const EventTimer&);
		}
	}
}