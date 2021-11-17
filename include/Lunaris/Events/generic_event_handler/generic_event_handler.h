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
		CONTROL = JOYSTICK,
		TOUCH		= 1 << 3
	};
	enum class __internal_events {
		THREAD_QUIT = 2048
	};
	constexpr int operator+(__internal_events a) { return static_cast<int>(a); }
	
	constexpr events operator|(events a, events b) { return static_cast<events>(static_cast<int>(a) | static_cast<int>(b)); }
	constexpr int operator&(events a, events b) { return (static_cast<int>(a) & static_cast<int>(b)); }
	constexpr events operator^(events a, events b) { return static_cast<events>(static_cast<int>(a) & static_cast<int>(b)); }

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

		void install(const events);
		void uninstall(const events);

		void install(const user_unique&);
		void install(const user_shared&);
		void install(const timer_unique&);
		void install(const timer_shared&);

		void uninstall(const user_unique&);
		void uninstall(const user_shared&);
		void uninstall(const timer_unique&);
		void uninstall(const timer_shared&);

		void install_other(ALLEGRO_EVENT_SOURCE*);
		void install_other(std::vector<ALLEGRO_EVENT_SOURCE*>);

		void uninstall_other(ALLEGRO_EVENT_SOURCE*);
		void uninstall_other(std::vector<ALLEGRO_EVENT_SOURCE*>);

		void hook_event_handler(std::function<void(const ALLEGRO_EVENT&)>);
		void unhook_event_handler();

		void hook_exception_handler(std::function<void(const std::exception&)>);
		void unhook_exception_handler();
	};

}