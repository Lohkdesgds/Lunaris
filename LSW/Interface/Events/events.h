#pragma once

#include <memory>
#include <functional>

#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/SuperThread/superthread.h"
#include "../../Tools/SuperMutex/supermutex.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			/// <summary>
			/// <para>The Event itself (raw).</para>
			/// </summary>
			class RawEvent {
				ALLEGRO_EVENT ev;
			public:
				RawEvent() = delete;
				RawEvent(ALLEGRO_EVENT&&);

				/// <summary>
				/// <para>What event type was it? (ALLEGRO_EVENT_*).</para>
				/// </summary>
				/// <returns>{int} Event type ID.</returns>
				int type() const;

				/// <summary>
				/// <para>If type is this one, get this information.</para>
				/// </summary>
				/// <returns>{ALLEGRO_DISPLAY_EVENT} The event data.</returns>
				const ALLEGRO_DISPLAY_EVENT&  display_event() const;

				/// <summary>
				/// <para>If type is this one, get this information.</para>
				/// </summary>
				/// <returns>{ALLEGRO_JOYSTICK_EVENT} The event data.</returns>
				const ALLEGRO_JOYSTICK_EVENT& joystick_event() const;

				/// <summary>
				/// <para>If type is this one, get this information.</para>
				/// </summary>
				/// <returns>{ALLEGRO_KEYBOARD_EVENT} The event data.</returns>
				const ALLEGRO_KEYBOARD_EVENT& keyboard_event() const;

				/// <summary>
				/// <para>If type is this one, get this information.</para>
				/// </summary>
				/// <returns>{ALLEGRO_MOUSE_EVENT} The event data.</returns>
				const ALLEGRO_MOUSE_EVENT&    mouse_event() const;

				/// <summary>
				/// <para>If type is this one, get this information.</para>
				/// </summary>
				/// <returns>{ALLEGRO_TIMER_EVENT} The event data.</returns>
				const ALLEGRO_TIMER_EVENT&    timer_event() const;

				/// <summary>
				/// <para>If type is this one, get this information.</para>
				/// </summary>
				/// <returns>{ALLEGRO_TOUCH_EVENT} The event data.</returns>
				const ALLEGRO_TOUCH_EVENT&    touch_event() const;

				/// <summary>
				/// <para>If type is this one, get this information.</para>
				/// </summary>
				/// <returns>{ALLEGRO_USER_EVENT} The event data.</returns>
				const ALLEGRO_USER_EVENT&     user_event() const;
			};

			/// <summary>
			/// <para>Event is a event source from one of many known things, like keyboard, screen, timer...</para>
			/// </summary>
			class Event {
			protected:
				std::shared_ptr<ALLEGRO_EVENT_SOURCE> core;
			public:
				Event();

				/// <summary>
				/// <para>Copy constructor.</para>
				/// </summary>
				/// <param name="{Event}">Event to copy.</param>
				Event(const Event&);

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{Event}">Event to move.</param>
				Event(Event&&) noexcept;

				/// <summary>
				/// <para>Copy operator.</para>
				/// </summary>
				/// <param name="{Event}">Event to copy.</param>
				void operator=(const Event&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{Event}">Event to move.</param>
				void operator=(Event&&) noexcept;

				/// <summary>
				/// <para>Direct RAW transformation to Event. This won't deinit the event source by itself!</para>
				/// </summary>
				/// <param name="{RAW EVENT SOURCE}">Event source.</param>
				Event(ALLEGRO_EVENT_SOURCE*);

				/// <summary>
				/// <para>Compare if event source is the same.</para>
				/// </summary>
				/// <param name="{Event}">Another Event.</param>
				/// <returns>{bool} True if they are.</returns>
				bool operator==(const Event&) const;

				/// <summary>
				/// <para>Compare if event source is the same.</para>
				/// </summary>
				/// <param name="{Event}">Another Event.</param>
				/// <returns>{bool} True if they are NOT.</returns>
				bool operator!=(const Event&) const;

				friend class EventHandler;
			};

			/// <summary>
			/// <para>Generates and creates a keyboard Event.</para>
			/// </summary>
			/// <returns>{Event} The Event.</returns>
			const Event get_keyboard_event();

			/// <summary>
			/// <para>Generates and creates a mouse Event.</para>
			/// </summary>
			/// <returns>{Event} The Event.</returns>
			const Event get_mouse_event();

			/// <summary>
			/// <para>Generates and creates a joystick Event.</para>
			/// </summary>
			/// <returns>{Event} The Event.</returns>
			const Event get_joystick_event();

			/// <summary>
			/// <para>Generates and creates a touchscreen Event.</para>
			/// </summary>
			/// <returns>{Event} The Event.</returns>
			const Event get_touchscreen_event();


			namespace eventhandler {
				enum class handling_mode {BUFFERING_AUTO, NO_BUFFER_SKIP};
			}


			/// <summary>
			/// <para>Handle multiple Event sources with this. It will start a thread internally and call the function as events happen.</para>
			/// </summary>
			class EventHandler {
				std::shared_ptr<ALLEGRO_EVENT_QUEUE> own_queue; // trigger itself
				std::function<void(const RawEvent&)> trigger_func;
				Tools::SuperThread<> thr{ Tools::superthread::performance_mode::PERFORMANCE };
				Tools::SuperMutex thr_m;
				eventhandler::handling_mode mode = eventhandler::handling_mode::BUFFERING_AUTO;

				void __init();
			public:
				EventHandler();

				/// <summary>
				/// <para>Set internal thread's performance.</para>
				/// </summary>
				/// <param name="{performance_mode}">Performance mode.</param>
				EventHandler(const Tools::superthread::performance_mode&);

				~EventHandler();

				/// <summary>
				/// <para>What mode should it do?</para>
				/// <para>- BUFFERING_AUTO: Buffers events if necessary, it will go through all events sometime. This is more CPU friendly.</para>
				/// <para>- NO_BUFFER_SKIP: If it can't keep up, skip to latest and only work with latest up-to-date events. It may use more CPU.</para>
				/// </summary>
				/// <param name="{eventhandler::handling_mode}">Mode to handle events.</param>
				void set_mode(const eventhandler::handling_mode&);

				/// <summary>
				/// <para>Adds a Event to be handled by this (non-exclusive).</para>
				/// </summary>
				/// <param name="{Event}">A Event source.</param>
				void add(const Event&);

				/// <summary>
				/// <para>Is the event registered?</para>
				/// </summary>
				/// <param name="{Event}">A Event source.</param>
				/// <returns>{bool} True if it is.</returns>
				bool has(const Event&) const;

				/// <summary>
				/// <para>Remove a Event (if handled by this).</para>
				/// </summary>
				/// <param name="{Event}">A Event source.</param>
				void remove(const Event&);

				/// <summary>
				/// <para>Set to stop internal thread (no lock, stop properly with stop()).</para>
				/// </summary>
				void set_stop();

				/// <summary>
				/// <para>Set internal thread's performance.</para>
				/// </summary>
				/// <param name="{performance_mode}">Performance mode.</param>
				void set_performance_mode(const Tools::superthread::performance_mode&);

				/// <summary>
				/// <para>Stops the internal thread and cleanup (you'll have to reset to start again).</para>
				/// </summary>
				void stop();

				/// <summary>
				/// <para>Starts thread to handle events and call function as they happen.</para>
				/// </summary>
				/// <param name="{std::function}">A function to handle the Events.</param>
				void set_run_autostart(const std::function<void(const RawEvent&)>);

				/// <summary>
				/// <para>Is it running a thread internally right now?</para>
				/// </summary>
				/// <returns>{bool} Thread running.</returns>
				bool running() const;

				/// <summary>
				/// <para>Wait for an event manually (ONLY VALID IF YOU DON'T CALL SET_RUN_AUTOSTART).</para>
				/// </summary>
				/// <returns>{RawEvent} The event, if any. if type == 0 it's empty.</returns>
				const RawEvent wait_event_manually(const double = -1.0);
			};


			/// <summary>
			/// <para>Compare if events are the same.</para>
			/// </summary>
			/// <param name="{RAW EVENT}">A RAW EVENT.</param>
			/// <param name="{Event}">Another Event.</param>
			/// <returns>{bool} True if they are.</returns>
			bool operator==(ALLEGRO_EVENT_SOURCE*, const Event&);
		}
	}
}