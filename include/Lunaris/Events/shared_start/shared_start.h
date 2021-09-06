#pragma once

#include <allegro5/allegro.h>

#include <thread>
#include <mutex>
#include <stdexcept>

namespace Lunaris {

	void __keyboard_allegro_start();
	void __mouse_allegro_start();
	void __joystick_allegro_start();
	void __touch_allegro_start();

	class __common_event {
		std::thread thr;
		std::recursive_mutex safety;
		bool keep_running = false;
		bool thread_working = false;
		ALLEGRO_EVENT_QUEUE* ev_qu = nullptr;

		void start();
		void stop();
		void running_thread();
	protected:
		virtual void handle_events(const ALLEGRO_EVENT&) = 0;

		std::unique_lock<std::recursive_mutex> get_lock();
		ALLEGRO_EVENT_QUEUE* get_event_queue() const;
	public:
		__common_event();
		~__common_event();

		__common_event(const __common_event&) = delete;
		__common_event(__common_event&&) = delete;
		void operator=(const __common_event&) = delete;
		void operator=(__common_event&&) = delete;
	};

}