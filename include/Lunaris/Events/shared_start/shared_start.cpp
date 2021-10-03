#include "shared_start.h"

namespace Lunaris {

	LUNARIS_DECL void __keyboard_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_keyboard_installed() && !al_install_keyboard()) throw std::runtime_error("Can't start Keyboard!");
	}

	LUNARIS_DECL void __mouse_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_mouse_installed() && !al_install_mouse()) throw std::runtime_error("Can't start Mouse!");
	}

	LUNARIS_DECL void __joystick_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_joystick_installed() && !al_install_joystick()) throw std::runtime_error("Can't start Joystick!");
	}

	LUNARIS_DECL void __touch_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_touch_input_installed() && !al_install_touch_input()) throw std::runtime_error("Can't start Touch!");
	}

	LUNARIS_DECL void __common_event::running_thread()
	{
		thread_working = true;
		while (keep_running) {
			ALLEGRO_EVENT ev;
			try {
				auto lucky = get_lock(true);
				if (!lucky.try_lock()) continue; // as fast as it can

				if (ev_qu && al_wait_for_event_timed(ev_qu, &ev, 0.1f)) {
					//lucky.unlock();
					handle_events(ev);
				}
				else {
					lucky.unlock(); // free up for a while
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			}
			catch (const std::exception& e) {
				printf_s("FATAL ERROR: %s\n", e.what());
			}
			catch (...) {
				printf_s("FATAL ERROR: UNCAUGHT!\n");
			}
		}
		thread_working = false;
	}

	LUNARIS_DECL void __common_event::start()
	{
		if (ev_qu) return;// throw std::runtime_error("Tried to start twice?");

		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");

		keep_running = true;

		if (!(ev_qu = al_create_event_queue())) throw std::bad_alloc();

		thr = std::thread([&] {running_thread(); });
	}

	LUNARIS_DECL void __common_event::stop()
	{
		if (ev_qu) {
			keep_running = false;
			while (thread_working) std::this_thread::sleep_for(std::chrono::milliseconds(50));
			auto lucky = get_lock();
			if (thr.joinable()) thr.join();
			al_destroy_event_queue(ev_qu);
			ev_qu = nullptr;
		}
	}

	LUNARIS_DECL std::unique_lock<std::recursive_mutex> __common_event::get_lock(const bool deferred)
	{
		if (deferred) return std::unique_lock<std::recursive_mutex>(safety, std::defer_lock);
		return std::unique_lock<std::recursive_mutex>(safety);
	}

	LUNARIS_DECL ALLEGRO_EVENT_QUEUE* __common_event::get_event_queue() const
	{
		return ev_qu;
	}

	LUNARIS_DECL __common_event::__common_event()
	{
		start();
	}

	LUNARIS_DECL __common_event::~__common_event()
	{
		stop();
	}

}