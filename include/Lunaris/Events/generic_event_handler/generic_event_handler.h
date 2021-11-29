#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Events/shared_start.h>

#include <allegro5/allegro.h>
#include <functional>

namespace Lunaris {

	enum class events {
		KEYBOARD	= 1 << 0,
		MOUSE		= 1 << 1,
		JOYSTICK	= 1 << 2,
		CONTROL		= JOYSTICK,
		TOUCH		= 1 << 3
	};
	enum class __internal_events {
		THREAD_QUIT = 2048
	};

	constexpr int operator+(__internal_events a) { return static_cast<int>(a); }
	
	constexpr events operator|(events a, events b) { return static_cast<events>(static_cast<int>(a) | static_cast<int>(b)); }
	constexpr int operator&(events a, events b) { return (static_cast<int>(a) & static_cast<int>(b)); }
	constexpr events operator^(events a, events b) { return static_cast<events>(static_cast<int>(a) & static_cast<int>(b)); }

	/// <summary>
	/// <para>generic_event_handler can handle any kind of event from Allegro (or based on ALLEGRO_EVENT_SOURCE).</para>
	/// <para>This is the basic generic async (it creates its own thread) class you've ever wanted.</para>
	/// <para>The only downside compared to other event classes is that this is raw events. If you're into ALLEGRO_EVENT, have fun!</para>
	/// </summary>
	class generic_event_handler : public NonCopyable {
		struct core : public NonCopyable, public NonMovable {
			std::thread m_thr;
			std::recursive_mutex m_safe;
			bool m_run_confirm = false; // m_thr return
			std::function<void(const std::exception&)> m_err; // on error
			std::function<void(ALLEGRO_EVENT&)> m_evhlr; // on event
			queue_unique m_queue;
			user_unique m_custom;

			void _async(); // m_thr run

			core();
			~core();

			void signal_stop(); // deleting calls this
			void set_event_handler(std::function<void(ALLEGRO_EVENT&)>);
			void set_exception_handler(std::function<void(const std::exception&)>);
			void add_event_source(ALLEGRO_EVENT_SOURCE*);
			void remove_event_source(ALLEGRO_EVENT_SOURCE*);
		};
		std::unique_ptr<core> m_movable;

		void build_if_none();
	protected:
		core& get_core();
		core* get_core_ptr() const;
	public:
		generic_event_handler() = default;

		/// <summary>
		/// <para>Install one or many common event sources.</para>
		/// </summary>
		/// <param name="{events}">Combinable enum (via OR operator)</param>
		void install(const events);

		/// <summary>
		/// <para>Uninstall one or many common event sources.</para>
		/// </summary>
		/// <param name="{events}">Combinable enum (via OR operator)</param>
		void uninstall(const events);
		
		/// <summary>
		/// <para>Install a pre-defined unique user event source.</para>
		/// </summary>
		/// <param name="{user_unique}">User unique type event source.</param>
		void install(const user_unique&);

		/// <summary>
		/// <para>Install a pre-defined shared user event source.</para>
		/// </summary>
		/// <param name="{user_shared}">User shared type event source.</param>
		void install(const user_shared&);
		
		/// <summary>
		/// <para>Install a pre-defined unique timer event source.</para>
		/// </summary>
		/// <param name="{timer_unique}">Timer unique type event source.</param>
		void install(const timer_unique&);
		
		/// <summary>
		/// <para>Install a pre-defined shared timer event source.</para>
		/// </summary>
		/// <param name="{timer_shared}">Timer shared type event source.</param>
		void install(const timer_shared&);
		
		/// <summary>
		/// <para>Uninstall a pre-defined unique user event source.</para>
		/// </summary>
		/// <param name="{user_unique}">User unique type event source.</param>
		void uninstall(const user_unique&);

		/// <summary>
		/// <para>Uninstall a pre-defined shared user event source.</para>
		/// </summary>
		/// <param name="{user_shared}">User shared type event source.</param>
		void uninstall(const user_shared&);
		
		/// <summary>
		/// <para>Uninstall a pre-defined unique timer event source.</para>
		/// </summary>
		/// <param name="{timer_unique}">Timer unique type event source.</param>
		void uninstall(const timer_unique&);
		
		/// <summary>
		/// <para>Uninstall a pre-defined shared timer event source.</para>
		/// </summary>
		/// <param name="{timer_shared}">Timer shared type event source.</param>
		void uninstall(const timer_shared&);

		/// <summary>
		/// <para>Install a custom ALLEGRO_EVENT_SOURCE event source.</para>
		/// </summary>
		/// <param name="{ALLEGRO_EVENT_SOURCE*}">An event source.</param>
		void install_other(ALLEGRO_EVENT_SOURCE*);

		/// <summary>
		/// <para>Install a combo of custom ALLEGRO_EVENT_SOURCE event sources.</para>
		/// </summary>
		/// <param name="{vector&lt;ALLEGRO_EVENT_SOURCE*&gt;}">An event source combo.</param>
		void install_other(std::vector<ALLEGRO_EVENT_SOURCE*>);

		/// <summary>
		/// <para>Uninstall a custom ALLEGRO_EVENT_SOURCE event source.</para>
		/// </summary>
		/// <param name="{ALLEGRO_EVENT_SOURCE*}">An event source.</param>
		void uninstall_other(ALLEGRO_EVENT_SOURCE*);

		/// <summary>
		/// <para>Uninstall a group of custom ALLEGRO_EVENT_SOURCE event source at once.</para>
		/// </summary>
		/// <param name="{vector&lt;ALLEGRO_EVENT_SOURCE*&gt;}">An event source combo.</param>
		void uninstall_other(std::vector<ALLEGRO_EVENT_SOURCE*>);

		/// <summary>
		/// <para>Hook a function to handle events from any registered event source.</para>
		/// </summary>
		/// <param name="{function}">A function that handles ALLEGRO_EVENT data.</param>
		void hook_event_handler(std::function<void(const ALLEGRO_EVENT&)>);

		/// <summary>
		/// <para>Unhook any previously hooked event handler.</para>
		/// </summary>
		void unhook_event_handler();

		/// <summary>
		/// <para>Hook a function to handle possible exceptions!</para>
		/// </summary>
		/// <param name="{function}">A function to do something with exceptions.</param>
		void hook_exception_handler(std::function<void(const std::exception&)>);

		/// <summary>
		/// <para>Unhook any previously hooked exception handler.</para>
		/// </summary>
		void unhook_exception_handler();
	};

}