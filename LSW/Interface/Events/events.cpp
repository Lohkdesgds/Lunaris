#include "events.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			RawEvent::RawEvent(ALLEGRO_EVENT&& evx)
			{
				ev = std::move(evx);
			}

			int RawEvent::type() const
			{
				return ev.type;
			}

			const ALLEGRO_DISPLAY_EVENT&  RawEvent::display_event() const
			{
				return ev.display;
			}
			
			const ALLEGRO_JOYSTICK_EVENT& RawEvent::joystick_event() const 
			{
				return ev.joystick;
			}
			
			const ALLEGRO_KEYBOARD_EVENT& RawEvent::keyboard_event() const 
			{
				return ev.keyboard;
			}
			
			const ALLEGRO_MOUSE_EVENT&    RawEvent::mouse_event() const 
			{
				return ev.mouse;
			}
			
			const ALLEGRO_TIMER_EVENT&    RawEvent::timer_event() const 
			{
				return ev.timer;
			}
			
			const ALLEGRO_TOUCH_EVENT&    RawEvent::touch_event() const 
			{
				return ev.touch;
			}
			
			const ALLEGRO_USER_EVENT&     RawEvent::user_event() const 
			{
				return ev.user;
			}

			Event::Event()
			{
				Handling::init_basic();
			}

			Event::Event(const Event& ev)
			{
				Handling::init_basic();
				*this = ev;
			}

			Event::Event(Event&& ev) noexcept
			{
				Handling::init_basic();
				*this = std::move(ev);
			}

			void Event::operator=(const Event& ev)
			{
				core = ev.core;
			}

			void Event::operator=(Event&& ev) noexcept
			{
				core = std::move(ev.core);
			}

			Event::Event(ALLEGRO_EVENT_SOURCE* ev)
			{
				if (!ev) throw Handling::Abort(__FUNCSIG__, "Invalid RAW event source.");
				Handling::init_basic();
				core = std::shared_ptr<ALLEGRO_EVENT_SOURCE>(ev, [](ALLEGRO_EVENT_SOURCE* e) {});
			}

			bool Event::operator==(const Event& e) const
			{
				return core.get() == e.core.get();
			}

			bool Event::operator!=(const Event& e) const
			{
				return !(*this == e);
			}

			const Event get_keyboard_event()
			{
				Handling::init_basic();
				Handling::init_keyboard();
				return Event(al_get_keyboard_event_source());
			}
			const Event get_mouse_event()
			{
				Handling::init_basic();
				Handling::init_mouse();
				return Event(al_get_mouse_event_source());
			}

			const Event get_joystick_event()
			{
				Handling::init_basic();
				Handling::init_joypad();
				return Event(al_get_joystick_event_source());
			}

			const Event get_touchscreen_event()
			{
				Handling::init_basic();
				Handling::init_touch();
				return Event(al_get_touch_input_event_source());
			}

			void EventHandler::__init()
			{
				own_queue = std::shared_ptr<ALLEGRO_EVENT_QUEUE>(al_create_event_queue(),
					[](ALLEGRO_EVENT_QUEUE* ev) { al_destroy_event_queue(ev); ev = nullptr; });
			}
			
			EventHandler::EventHandler()
			{
				Handling::init_basic();
				__init();
			}
			
			EventHandler::EventHandler(const Tools::superthread::performance_mode& m)
			{
				thr.set_performance_mode(m);
				Handling::init_basic();
				__init();
			}

			EventHandler::~EventHandler()
			{
				stop();
			}
			
			void EventHandler::set_mode(const eventhandler::handling_mode& mod)
			{
				mode = mod;
			}

			void EventHandler::add(const Event& ev)
			{
				Tools::AutoLock wrk(thr_m);
				if (!ev.core) return;
				if (!own_queue) __init(); // delete queue and recreate clean
				if (!al_is_event_source_registered(own_queue.get(), ev.core.get())) {
					al_register_event_source(own_queue.get(), ev.core.get());
				}
			}
			
			bool EventHandler::has(const Event& ev) const
			{
				if (!ev.core) return false;
				if (!own_queue) return false;
				return al_is_event_source_registered(own_queue.get(), ev.core.get());
			}

			void EventHandler::remove(const Event& ev)
			{
				Tools::AutoLock wrk(thr_m);
				if (!ev.core) return;
				if (!own_queue) __init(); // delete queue and recreate clean
				if (al_is_event_source_registered(own_queue.get(), ev.core.get())) {
					al_unregister_event_source(own_queue.get(), ev.core.get());
				}
			}

			void EventHandler::set_stop()
			{
				thr.stop();
			}
			
			void EventHandler::set_performance_mode(const Tools::superthread::performance_mode& m)
			{
				thr.set_performance_mode(m);
			}

			void EventHandler::stop()
			{
				thr.stop();
				thr.join(); // sync and clear (may take up to 1 sec)
				Tools::AutoLock wrk(thr_m);
				own_queue.reset();
				trigger_func = std::function<void(const RawEvent&)>();
			}

			void EventHandler::set_run_autostart(const std::function<void(const RawEvent&)> f)
			{
				if (f) {
					if (thr.running()) thr.join();
					if (!own_queue) __init(); // delete queue and recreate clean
					
					trigger_func = f;

					thr.set([&](Tools::boolThreadF keep) {
						ALLEGRO_EVENT ev{};
						while (keep()) {

							try {
								if (!own_queue) {
									//std::this_thread::sleep_for(std::chrono::milliseconds(20));
									continue;
								}
								switch (mode) {
								case eventhandler::handling_mode::BUFFERING_AUTO:
									if (!al_wait_for_event_timed(own_queue.get(), &ev, 1.0)) {
										continue;
									}
									break;
								case eventhandler::handling_mode::NO_BUFFER_SKIP:
									if (!al_get_next_event(own_queue.get(), &ev)) {
										continue;
									}
									else if (!al_is_event_queue_empty(own_queue.get())) al_flush_event_queue(own_queue.get()); // no buffer!
									break;
								}

								if (ev.type) {
									RawEvent re(std::move(ev));
									trigger_func(re);
								}
							}
							catch (...) {
#ifdef _DEBUG
								std::cout << "__INTERNAL__ __SKIP__ UNKNOWN ERROR AT EVENT_HANDLER" << std::endl;
#endif
							}
						}
						return;
					});
					thr.start();
				}
			}

			bool EventHandler::running() const
			{
				return thr.running();
			}

			const RawEvent EventHandler::wait_event_manually(const double ww)
			{
				Tools::AutoLock wrk(thr_m);
				ALLEGRO_EVENT ev;
				if (!own_queue) return ev;
				al_wait_for_event_timed(own_queue.get(), &ev, ww);
				return ev;				
			}

			bool operator==(ALLEGRO_EVENT_SOURCE* f, const Event& e)
			{
				return e.operator==(f);
			}

		}
	}
}