#include "generic_event_handler.h"

namespace Lunaris {

	LUNARIS_DECL void generic_event_handler::core::_async()
	{
		m_run_confirm = true;
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Event handler thread spawned %p", this);
#endif

		for (bool keep = true; keep;) {
			ALLEGRO_EVENT ev;
			try {
				al_wait_for_event(m_queue.get(), &ev);

				switch (ev.type) {
				case +__internal_events::THREAD_QUIT:
					keep = false;
					continue;
				}

				std::lock_guard<std::recursive_mutex> luck(m_safe);
				if (m_evhlr) m_evhlr(ev);
			}
			catch (const std::exception& e) {
#ifdef LUNARIS_VERBOSE_BUILD
				PRINT_DEBUG("Event queue exception %p: %s", this, e.what());
#endif
				if (m_err) m_err(e);
			}
			catch (...) {
#ifdef LUNARIS_VERBOSE_BUILD
				PRINT_DEBUG("Event queue exception %p: UNCAUGHT", this);
#endif
				if (m_err) m_err(std::runtime_error("UNCAUGHT"));
			}
		}
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Event handler thread ended %p", this);
#endif
		m_run_confirm = false;
	}

	LUNARIS_DECL generic_event_handler::core::core()
		: m_queue(make_unique_queue()), m_custom(make_unique_user_event_source())
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Created event queue %p", this);
#endif
		al_register_event_source(m_queue.get(), m_custom.get());
		m_thr = std::thread([this] { _async(); });
	}

	LUNARIS_DECL generic_event_handler::core::~core()
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Destroyed event queue %p", this);
#endif
		if (m_run_confirm) {
#ifdef LUNARIS_VERBOSE_BUILD
			PRINT_DEBUG("Event handler waiting async event thread to stop.");
#endif
			signal_stop();
			if (m_thr.joinable()) m_thr.join();
#ifdef LUNARIS_VERBOSE_BUILD
			PRINT_DEBUG("Event handler has ended all tasks.");
#endif
		}
		m_queue.reset(); // first
	}

	LUNARIS_DECL void generic_event_handler::core::signal_stop()
	{
		if (!m_run_confirm) return; // not running
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Signal stop event queue %p", this);
#endif
		ALLEGRO_EVENT ev;
		ev.user.type = +__internal_events::THREAD_QUIT;
		ev.user.source = m_custom.get();
		if (!al_emit_user_event(m_custom.get(), &ev, nullptr)) {
#ifdef LUNARIS_VERBOSE_BUILD
			PRINT_DEBUG("Skipped emit user event kill thread @ common event. Probably already ended?!");
#endif
		}
	}

	LUNARIS_DECL void generic_event_handler::core::set_event_handler(std::function<void(ALLEGRO_EVENT&)> f)
	{
		std::lock_guard<std::recursive_mutex> luck(m_safe);
		m_evhlr = f;
	}

	LUNARIS_DECL void generic_event_handler::core::set_exception_handler(std::function<void(const std::exception&)> f)
	{
		std::lock_guard<std::recursive_mutex> luck(m_safe);
		m_err = f;
	}

	LUNARIS_DECL void generic_event_handler::core::add_event_source(ALLEGRO_EVENT_SOURCE* src)
	{
		if (!src || !m_queue) return;
		al_register_event_source(m_queue.get(), src);
	}

	LUNARIS_DECL void generic_event_handler::core::remove_event_source(ALLEGRO_EVENT_SOURCE* src)
	{
		if (!src || !m_queue) return;
		al_unregister_event_source(m_queue.get(), src);
	}

	LUNARIS_DECL void generic_event_handler::build_if_none()
	{
		if (!m_movable) m_movable = std::make_unique<core>();
	}

	LUNARIS_DECL generic_event_handler::core& generic_event_handler::get_core()
	{
		build_if_none();
		return *m_movable.get();
	}

	LUNARIS_DECL generic_event_handler::core* generic_event_handler::get_core_ptr() const
	{
		return m_movable.get();
	}

	LUNARIS_DECL void generic_event_handler::install(const events evs)
	{
		if (evs & events::JOYSTICK) { __joystick_allegro_start();	install_other(al_get_joystick_event_source()); }
		if (evs & events::KEYBOARD) { __keyboard_allegro_start();	install_other(al_get_keyboard_event_source()); }
		if (evs & events::MOUSE)	{ __mouse_allegro_start();		install_other(al_get_mouse_event_source()); }
		if (evs & events::TOUCH)	{ __touch_allegro_start();		install_other(al_get_touch_input_event_source()); }
	}

	LUNARIS_DECL void generic_event_handler::uninstall(const events evs)
	{
		if (evs & events::JOYSTICK) { __joystick_allegro_start();	uninstall_other(al_get_joystick_event_source()); }
		if (evs & events::KEYBOARD) { __keyboard_allegro_start();	uninstall_other(al_get_keyboard_event_source()); }
		if (evs & events::MOUSE)	{ __mouse_allegro_start();		uninstall_other(al_get_mouse_event_source()); }
		if (evs & events::TOUCH)	{ __touch_allegro_start();		uninstall_other(al_get_touch_input_event_source()); }
	}

	LUNARIS_DECL void generic_event_handler::install(const user_unique& v)
	{
		install_other(v.get());
	}

	LUNARIS_DECL void generic_event_handler::install(const user_shared& v)
	{
		install_other(v.get());
	}

	LUNARIS_DECL void generic_event_handler::install(const timer_unique& v)
	{
		if (v) install_other(al_get_timer_event_source(v.get()));
	}

	LUNARIS_DECL void generic_event_handler::install(const timer_shared& v)
	{
		if (v) install_other(al_get_timer_event_source(v.get()));
	}
	 
	LUNARIS_DECL void generic_event_handler::uninstall(const user_unique& v)
	{
		uninstall_other(v.get());
	}

	LUNARIS_DECL void generic_event_handler::uninstall(const user_shared& v)
	{
		uninstall_other(v.get());
	}

	LUNARIS_DECL void generic_event_handler::uninstall(const timer_unique& v)
	{
		if (v) uninstall_other(al_get_timer_event_source(v.get()));
	}

	LUNARIS_DECL void generic_event_handler::uninstall(const timer_shared& v)
	{
		if (v) uninstall_other(al_get_timer_event_source(v.get()));
	}

	LUNARIS_DECL void generic_event_handler::install_other(ALLEGRO_EVENT_SOURCE* src)
	{
		if (!src) return;
		build_if_none();
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Installing event source %p <- %p", m_movable.get(), src);
#endif
		m_movable->add_event_source(src);
	}

	LUNARIS_DECL void generic_event_handler::install_other(std::vector<ALLEGRO_EVENT_SOURCE*> srcs)
	{
		for (const auto& e : srcs) install_other(e);
	}

	LUNARIS_DECL void generic_event_handler::uninstall_other(ALLEGRO_EVENT_SOURCE* src)
	{
		if (!src) return;
		build_if_none();
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Removing event source %p </- %p", m_movable.get(), src);
#endif
		m_movable->remove_event_source(src);
	}

	LUNARIS_DECL void generic_event_handler::uninstall_other(std::vector<ALLEGRO_EVENT_SOURCE*> srcs)
	{
		for (const auto& e : srcs) uninstall_other(e);
	}

	LUNARIS_DECL void generic_event_handler::hook_event_handler(std::function<void(const ALLEGRO_EVENT&)> f)
	{
		if (!f) return;
		build_if_none();
		m_movable->set_event_handler(f);
	}

	LUNARIS_DECL void generic_event_handler::unhook_event_handler()
	{
		build_if_none();
		m_movable->set_event_handler([](auto&) {});
	}
	
	LUNARIS_DECL void generic_event_handler::hook_exception_handler(std::function<void(const std::exception&)> f)
	{
		if (!f) return;
		build_if_none();
		m_movable->set_exception_handler(f);
	}

	LUNARIS_DECL void generic_event_handler::unhook_exception_handler()
	{
		build_if_none();
		m_movable->set_exception_handler([](auto&) {});
	}

}