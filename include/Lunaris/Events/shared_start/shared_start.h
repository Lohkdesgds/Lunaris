#pragma once

#include <Lunaris/__macro/macros.h>

#include <allegro5/allegro.h>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <functional>
#include <memory>

namespace Lunaris {

	void __keyboard_allegro_start();
	void __mouse_allegro_start();
	void __joystick_allegro_start();
	void __touch_allegro_start();
	void __generic_events_start();

	namespace specific {
		// raw new, use make_**_user_event_source instead!!
		ALLEGRO_EVENT_SOURCE* __new_user_event();
		// raw delete, use make_**_user_event_source instead!!
		void __del_user_event(ALLEGRO_EVENT_SOURCE*);
	}

	using user_unique = std::unique_ptr<ALLEGRO_EVENT_SOURCE, void(*)(ALLEGRO_EVENT_SOURCE*)>;
	using user_shared = std::shared_ptr<ALLEGRO_EVENT_SOURCE>;
	using queue_unique = std::unique_ptr<ALLEGRO_EVENT_QUEUE, void(*)(ALLEGRO_EVENT_QUEUE*)>;
	using queue_shared = std::shared_ptr<ALLEGRO_EVENT_QUEUE>;
	using timer_unique = std::unique_ptr<ALLEGRO_TIMER, void(*)(ALLEGRO_TIMER*)>;
	using timer_shared = std::shared_ptr<ALLEGRO_TIMER>;

	user_unique make_unique_user_event_source();
	user_shared make_shared_user_event_source();

	queue_unique make_unique_queue();
	queue_shared make_shared_queue();

	// create timer. time? start already?
	timer_unique make_unique_timer(const double, const bool);
	// create timer. time? start already?
	timer_shared make_shared_timer(const double, const bool);

}