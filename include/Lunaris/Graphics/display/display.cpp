#include "display.h"

namespace Lunaris {

	LUNARIS_DECL void __display_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_primitives_addon_initialized() && !al_init_primitives_addon()) throw std::runtime_error("Can't start Primitives!");
	}

	LUNARIS_DECL display_options& display_options::set_width(const int var)
	{
		width = var;
		return *this;
	}
	
	LUNARIS_DECL display_options& display_options::set_height(const int var)
	{
		height = var;
		return *this;
	}

	LUNARIS_DECL display_options& display_options::set_frequency(const int var)
	{
		freq = var;
		return *this;
	}

	LUNARIS_DECL display_options& display_options::set_display_index(const int var)
	{
		display_index = var;
		return *this;
	}

	LUNARIS_DECL int display_config::flags_combine() const
	{
		int res = extra_flags
			& ~ALLEGRO_FULLSCREEN 
			& ~ALLEGRO_FULLSCREEN_WINDOW
			& ~ALLEGRO_WINDOWED;

		return res | ((fullscreen || (mode.width == 0 && mode.height == 0)) ? ALLEGRO_FULLSCREEN_WINDOW : 0);
	}

	LUNARIS_DECL display_config& display_config::set_display_mode(const display_options& var)
	{
		mode = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_extra_flags(const int var)
	{
		extra_flags = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_samples(const int var)
	{
		samples = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_fullscreen(const bool var)
	{
		fullscreen = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_vsync(const bool var)
	{
		vsync = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_single_buffer(const bool var)
	{
		single_buffer = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_use_basic_internal_event_system(const bool var)
	{
		use_basic_internal_event_system = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_window_title(const std::string& var)
	{
		window_title = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_self_draw(const bool var)
	{
		self_draw = var;
		return *this;
	}

	LUNARIS_DECL std::vector<display_options> get_current_modes(const int index)
	{
		__display_allegro_start();

		if (index < 0 || index >= al_get_num_video_adapters()) return {};
		std::vector<display_options> opts;

		al_set_new_display_adapter(index);

		for (int j = 0; j < al_get_num_display_modes(); j++) {
			ALLEGRO_DISPLAY_MODE mode{};
			al_get_display_mode(j, &mode);
			display_options conv{};
			conv.display_index = index;
			conv.width = mode.width;
			conv.height = mode.height;
			conv.freq = mode.refresh_rate;
			opts.push_back(conv);
		}

		return opts;
	}

	LUNARIS_DECL bool display::self_draw_block::safe_run::can_run() // true if good, false if wait (continue;)
	{
		is_paused = is_lock;
		return !is_paused;
	}

	LUNARIS_DECL void display::self_draw_block::safe_run::lock(const bool skip_wait) // tell thread to stop, wait for ack
	{
		is_lock = true;
		if (skip_wait) return;
		while (!is_paused) std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	LUNARIS_DECL void display::self_draw_block::safe_run::unlock() // tell thread it's good, no wait
	{
		is_lock = false;
	}

	LUNARIS_DECL void display::self_draw_block::thr_drawing()
	{
		is_drawing = true;
		while (keep_drawing)
		{
			if (!safer.can_run()) continue;
			try {
				if (once_funcs.size()) {
					std::lock_guard<std::mutex> luck(once_funcs_safer);
					for (const auto& i : once_funcs) i();
					once_funcs.clear();
				}
				draw_func(); // no check, faster.
				std::this_thread::yield(); // no 100% CPU usage, this helps balancing CPU stuff.
			}
			catch (...) {
				_exception_stored = std::current_exception();
				had_exception = true;
			}
		}
		is_drawing = false;
	}

	LUNARIS_DECL display::self_draw_block::self_draw_block()
	{
		keep_drawing = true;
		is_drawing = false;
		thr = std::thread([this] {thr_drawing(); });
		while (!is_drawing) std::this_thread::sleep_for(std::chrono::milliseconds(10)); // wait thread start.
	}

	LUNARIS_DECL display::self_draw_block::~self_draw_block()
	{
		if (!is_drawing) return;
		keep_drawing = false;
		while (is_drawing) std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (thr.joinable()) thr.join();
		rethrow_any_exception(); // if there
	}

	LUNARIS_DECL void display::self_draw_block::add_run_once(const std::function<void(void)> f)
	{
		if (!f) return;
		std::lock_guard<std::mutex> luck(once_funcs_safer);
		once_funcs.push_back(f);
	}

	LUNARIS_DECL void display::self_draw_block::set_function(const std::function<void(void)> f)
	{
		if (!f) return;
		safer.lock(!is_drawing);
		draw_func = f;
		safer.unlock();
	}

	LUNARIS_DECL void display::self_draw_block::rethrow_any_exception()
	{
		if (!had_exception) return;
		safer.lock(!is_drawing);
		std::exception_ptr _mov = _exception_stored;
		had_exception = false;
		safer.unlock();
		std::rethrow_exception(_mov);
	}

	LUNARIS_DECL void display::_flip_nocheck()
	{
		if (window) {
			al_flip_display();

			if (economy_mode) std::this_thread::sleep_for(economy_mode_delay);

			if (((al_get_time() - last_event_check) > default_display_self_check_time)) {

				std::lock_guard<std::recursive_mutex> lucky(sensitive);

				const ALLEGRO_TRANSFORM* cpy = al_get_current_transform();
				if (cpy) latest_transform = *cpy;

				if (ev_qu) {

					ALLEGRO_EVENT ev;
					while (ev_qu && al_get_next_event(ev_qu, &ev)) {

						switch (ev.type) {

						case ALLEGRO_EVENT_DISPLAY_EXPOSE: // shown again
						case ALLEGRO_EVENT_DISPLAY_FOUND:
						case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:

							if (ev.display.source != window)
								throw std::runtime_error("Unexpected external event on display thread! Display is not itself?");

							economy_mode = false;

							break;

						case ALLEGRO_EVENT_DISPLAY_LOST:
						case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:

							if (ev.display.source != window)
								throw std::runtime_error("Unexpected external event on display thread! Display is not itself?");

							economy_mode = true;

							break;

						case ALLEGRO_EVENT_DISPLAY_RESIZE:

							if (ev.display.source != window)
								throw std::runtime_error("Unexpected external event on display thread! Display is not itself?");

							acknowledge_resize();
							break;
						}

						// after handling by itself, call hooked function (if any)
						if (hooked_events) hooked_events(ev);
					}
				}
			}
		}
	}

	LUNARIS_DECL std::function<void(void)> display::combine_func_to_essential(const std::function<void(void)> f)
	{
		if (!f) return [this] {_flip_nocheck(); };
		return [this, f] { f(); _flip_nocheck(); };
	}

	LUNARIS_DECL display::display(const display_config& conf)
	{
		if (!create(conf)) throw std::runtime_error("Can't create display!");
	}

	LUNARIS_DECL display::~display()
	{
		destroy();
	}

	LUNARIS_DECL display::display(display&& oth) noexcept
		: window(oth.window), ev_qu(oth.ev_qu), last_event_check(oth.last_event_check), hooked_events(oth.hooked_events), draw_self(std::move(oth.draw_self))
	{
		oth.window = nullptr;
		oth.ev_qu = nullptr;
		oth.last_event_check = 0.0;
		oth.hooked_events = {};
	}

	LUNARIS_DECL void display::operator=(display&& oth) noexcept
	{
		destroy();

		window = oth.window;
		ev_qu = oth.ev_qu;
		last_event_check = oth.last_event_check;
		hooked_events = oth.hooked_events;
		draw_self = std::move(oth.draw_self);

		oth.window = nullptr;
		oth.ev_qu = nullptr;
		oth.last_event_check = 0.0;
		oth.hooked_events = {};
	}

	LUNARIS_DECL bool display::create(const display_config& conf)
	{
		destroy();
		__display_allegro_start();

		bool ended_task_async = false;
		bool success_on_task = false;

		const auto func_setup = [&,conf] {
			al_set_new_display_flags(conf.flags_combine());
			al_set_new_bitmap_flags(ALLEGRO_CONVERT_BITMAP);
			al_set_new_display_option(ALLEGRO_VSYNC, conf.vsync ? 1 : 2, ALLEGRO_SUGGEST);
			al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, conf.samples ? 1 : 0, ALLEGRO_SUGGEST);
			al_set_new_display_option(ALLEGRO_SAMPLES, conf.samples, ALLEGRO_SUGGEST); // may glitch on create display?!
			al_set_new_display_option(ALLEGRO_SINGLE_BUFFER, conf.single_buffer ? 1 : 0, ALLEGRO_SUGGEST);
			if (conf.mode.freq > 0)
				al_set_new_display_refresh_rate(conf.mode.freq);
			if (!conf.window_title.empty())
				al_set_new_window_title(conf.window_title.c_str());


			if (conf.use_basic_internal_event_system) {
				if (!(ev_qu = al_create_event_queue())) {
					success_on_task = false;
					ended_task_async = true;
					return;
				}					
			}

			if (!(window = al_create_display(conf.mode.width > 0 ? conf.mode.width : 0, conf.mode.height > 0 ? conf.mode.height : 0)))
			{
				if (ev_qu) al_destroy_event_queue(ev_qu);
				ev_qu = nullptr;
				success_on_task = false;
				ended_task_async = true;
				return;
			}			

			if (conf.use_basic_internal_event_system) {
				al_register_event_source(ev_qu, al_get_display_event_source(window));
			}
			al_set_target_backbuffer(window);

			al_identity_transform(&latest_transform);


			success_on_task = window != nullptr;
			ended_task_async = true;
		};

		if (conf.self_draw) {
			draw_self = std::make_unique<self_draw_block>();
			draw_self->add_run_once(func_setup);
			draw_self->set_function(combine_func_to_essential());

			while (!ended_task_async) std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		else {
			func_setup();
		}

		return success_on_task;
	}

	LUNARIS_DECL bool display::create(const int width, const int height, const int freq)
	{
		display_config conf;
		conf.mode.width = width;
		conf.mode.height = height;
		conf.mode.freq = freq;
		conf.fullscreen = false;
		return create(conf);
	}

	LUNARIS_DECL bool display::create(const std::string& wname, const int width, const int height, const int freq)
	{
		display_config conf;
		conf.mode.width = width;
		conf.mode.height = height;
		conf.mode.freq = freq;
		conf.fullscreen = false;
		conf.window_title = wname;
		return create(conf);
	}

	LUNARIS_DECL void display::set_window_title(const std::string& str)
	{
		if (!empty() && !str.empty()) al_set_window_title(window, str.c_str());
	}

	LUNARIS_DECL void display::hook_event_handler(std::function<void(const ALLEGRO_EVENT&)> f)
	{
		std::lock_guard<std::recursive_mutex> lucky(sensitive);
		hooked_events = f;
	}

	LUNARIS_DECL void display::unhook_event_handler()
	{
		std::lock_guard<std::recursive_mutex> lucky(sensitive);
		hooked_events = {};
	}

	LUNARIS_DECL void display::hook_draw_function(std::function<void(void)> f)
	{
		if (draw_self) draw_self->set_function(combine_func_to_essential(f));
		else throw std::runtime_error("Trying to set a draw_function to a non self drawing display!");
	}

	LUNARIS_DECL void display::add_run_once_in_drawing_thread(std::function<void(void)> f)
	{
		if (draw_self) draw_self->add_run_once(f);
		else throw std::runtime_error("Trying to set a once_function to a non self drawing display!");
	}

	LUNARIS_DECL int display::get_width() const
	{
		if (window) return al_get_display_width(window);
		return 0;
	}

	LUNARIS_DECL int display::get_height() const
	{
		if (window) return al_get_display_height(window);
		return 0;
	}

	LUNARIS_DECL int display::get_frequency() const
	{
		if (window) return al_get_display_refresh_rate(window);
		return 0;
	}

	LUNARIS_DECL int display::get_flags() const
	{
		if (window) return al_get_display_flags(window);
		return 0;
	}

	LUNARIS_DECL void display::toggle_flag(const int flg)
	{
		if (window) {
			al_toggle_display_flag(window, flg, !(al_get_display_flags(window) & flg));
			acknowledge_resize();
		}
	}

	LUNARIS_DECL bool display::get_is_economy_mode_activated() const
	{
		return economy_mode;
	}

	LUNARIS_DECL bool display::empty() const
	{
		return window == nullptr;
	}

	LUNARIS_DECL void display::destroy()
	{
		if (draw_self) {
			draw_self.reset();
		}
		if (ev_qu) {
			std::lock_guard<std::recursive_mutex> lucky(sensitive);
			al_destroy_event_queue(ev_qu);
			ev_qu = nullptr;
			last_event_check = 0.0;
		}
		if (window) {
			al_destroy_display(window);
			window = nullptr;
		}
		hooked_events = {};
	}

	LUNARIS_DECL ALLEGRO_DISPLAY* display::get_raw_display() const
	{
		return window;
	}

	LUNARIS_DECL ALLEGRO_EVENT_SOURCE* display::get_event_source()
	{
		return window ? al_get_display_event_source(window) : nullptr;
	}

	LUNARIS_DECL std::function<ALLEGRO_TRANSFORM(void)> display::get_current_transform_function()
	{
		return [&] {return latest_transform; };
	}

	LUNARIS_DECL display::operator std::function<ALLEGRO_TRANSFORM()>() const
	{
		return [&] {return latest_transform; };
	}

	LUNARIS_DECL void display::set_as_target() const
	{
		if (draw_self) throw std::runtime_error("You should not call set_as_target() if self drawing is enabled! It will bug!");
		if (window) al_set_target_backbuffer(window);
	}

	LUNARIS_DECL void display::flip()
	{
		if (draw_self) throw std::runtime_error("You should not call flip() if self drawing is enabled!");
		_flip_nocheck();
	}

	LUNARIS_DECL void display::acknowledge_resize()
	{
		if (window) al_acknowledge_resize(window);
	}

}