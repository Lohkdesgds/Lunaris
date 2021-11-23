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

	/// <summary>
	/// <para>Create a ALLEGRO_EVENT_SOURCE with default settings as unique_ptr (initialized for user events).</para>
	/// </summary>
	/// <returns>{user_unique} a std::unique_ptr of ALLEGRO_EVENT_SOURCE (as user event).</returns>
	user_unique make_unique_user_event_source();

	/// <summary>
	/// <para>Create a ALLEGRO_EVENT_SOURCE with default settings as shared_ptr (initialized for user events).</para>
	/// </summary>
	/// <returns>{user_shared} a std::shared_ptr of ALLEGRO_EVENT_SOURCE (as user event).</returns>
	user_shared make_shared_user_event_source();

	/// <summary>
	/// <para>Create a ALLEGRO_EVENT_QUEUE with easy destructor.</para>
	/// </summary>
	/// <returns>{queue_unique} A std::unique_ptr of ALLEGRO_EVENT_QUEUE.</returns>
	queue_unique make_unique_queue();

	/// <summary>
	/// <para>Create a ALLEGRO_EVENT_QUEUE with easy destructor.</para>
	/// </summary>
	/// <returns>{queue_shared} A std::shared_ptr of ALLEGRO_EVENT_QUEUE.</returns>
	queue_shared make_shared_queue();

	/// <summary>
	/// <para>Create a timer quick and easy.</para>
	/// </summary>
	/// <param name="{double}">Delta time in seconds for each event.</param>
	/// <param name="{bool}">Whether start time already or not.</param>
	/// <returns>{timer_unique} A std::unique_ptr to a ALLEGRO_TIMER ready to go.</returns>
	timer_unique make_unique_timer(const double, const bool);

	/// <summary>
	/// <para>Create a timer quick and easy.</para>
	/// </summary>
	/// <param name="{double}">Delta time in seconds for each event.</param>
	/// <param name="{bool}">Whether start time already or not.</param>
	/// <returns>{timer_shared} A std::shared_ptr to a ALLEGRO_TIMER ready to go.</returns>
	timer_shared make_shared_timer(const double, const bool);

}