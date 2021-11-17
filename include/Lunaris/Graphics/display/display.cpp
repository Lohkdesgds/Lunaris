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

	LUNARIS_DECL display_config& display_config::set_auto_economy_mode(const bool var)
	{
		return set_economy_framerate_limit(var ? 30 : 0);
	}

	LUNARIS_DECL display_config& display_config::set_economy_framerate_limit(const double var)
	{
		min_frames = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_single_buffer(const bool var)
	{
		single_buffer = var;
		return *this;
	}

	LUNARIS_DECL display_config& display_config::set_framerate_limit(const double var)
	{
		max_frames = var > 0.0 ? var : 0.0;
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

	LUNARIS_DECL display_config& display_config::set_wait_for_display_draw(const bool var)
	{
		wait_for_display_draw = var;
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

	LUNARIS_DECL display::_clipboard::_clipboard(ALLEGRO_DISPLAY* d)
		: src(d)
	{
	}

	LUNARIS_DECL bool display::_clipboard::has_text() const
	{
		return src ? al_clipboard_has_text(src) : false;
	}

	LUNARIS_DECL std::string display::_clipboard::get_text() const
	{
		if (!src) return {};
		std::string cpy;
		char* buf = al_get_clipboard_text(src);
		if (!buf) return {};
		cpy = buf;
		al_free(buf);
		return cpy;
	}

	LUNARIS_DECL bool display::_clipboard::clear_text()
	{
		return set_text("");
	}

	LUNARIS_DECL bool display::_clipboard::set_text(const std::string& var)
	{
		return src ? al_set_clipboard_text(src, var.c_str()) : false;
	}

	LUNARIS_DECL void display::fix_timers()
	{
		if (!ev_qu) throw std::runtime_error("Expected queue, no queue found, error!");

		if (!update_tasks) {
			update_tasks = al_create_timer(0.5);
			al_register_event_source(ev_qu, al_get_timer_event_source(update_tasks));
			al_start_timer(update_tasks);
		}

		if (economy_mode) {
			double max_fps = economy_fps > 0.0 ? economy_fps : 0.0;
			if (default_fps > 0.0 && (default_fps < max_fps || max_fps <= 0.0)) max_fps = default_fps;
	
			if (max_fps > 0.0) {
				if (!timed_draw) {
					timed_draw = al_create_timer(1.0 / max_fps);
					al_register_event_source(ev_qu, al_get_timer_event_source(timed_draw));
					al_start_timer(timed_draw);
				}
				else {
					al_set_timer_speed(timed_draw, 1.0 / max_fps);
				}
			}
			else {
				if (timed_draw) {
					al_unregister_event_source(ev_qu, al_get_timer_event_source(timed_draw));
					al_stop_timer(timed_draw);
					al_destroy_timer(timed_draw);
					timed_draw = nullptr;
				}
			}
		}
		else {
			if (default_fps > 0.0) {
				if (!timed_draw) {
					timed_draw = al_create_timer(1.0 / default_fps);
					al_register_event_source(ev_qu, al_get_timer_event_source(timed_draw));
					al_start_timer(timed_draw);
				}
				else {
					al_set_timer_speed(timed_draw, 1.0 / default_fps);
				}
			}
			else {
				if (timed_draw) {
					al_unregister_event_source(ev_qu, al_get_timer_event_source(timed_draw));
					al_stop_timer(timed_draw);
					al_destroy_timer(timed_draw);
					timed_draw = nullptr;
				}
			}
		}
	}

	LUNARIS_DECL bool display::auto_get_next_event(ALLEGRO_EVENT& rev)
	{
		return ev_qu ? ((wait_for_display_flip_before_drop && timed_draw) ? (al_wait_for_event_timed(ev_qu, &rev, max_time_wait_for_event)) : (al_get_next_event(ev_qu, &rev))) : false;
	}

	LUNARIS_DECL display::display(const display_config& conf)
	{
		if (!create(conf)) throw std::runtime_error("Can't create display!");
	}

	LUNARIS_DECL display::~display()
	{
		destroy();
	}

	LUNARIS_DECL display::_clipboard display::clipboard() const
	{
		return { window };
	}

	LUNARIS_DECL bool display::create(const display_config& conf)
	{
		PRINT_DEBUG("Display %p is being (re)created", this);
		destroy();
		__display_allegro_start();
		
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


		if (!(ev_qu = al_create_event_queue())) {
			return false;
		}
		al_init_user_event_source(&evsrc);
		al_register_event_source(ev_qu, &evsrc);

		if (!(window = al_create_display(conf.mode.width > 0 ? conf.mode.width : 0, conf.mode.height > 0 ? conf.mode.height : 0)))
		{
			al_destroy_user_event_source(&evsrc);
			if (ev_qu) al_destroy_event_queue(ev_qu);
			ev_qu = nullptr;
			return false;
		}			

		if (conf.use_basic_internal_event_system) {
			al_register_event_source(ev_qu, al_get_display_event_source(window));
		}
		al_set_target_backbuffer(window);

		al_identity_transform(&latest_transform);

		economy_fps = conf.min_frames < 0.0 ? 0.0 : conf.min_frames;
		default_fps = conf.max_frames < 0.0 ? 0.0 : conf.max_frames;
		wait_for_display_flip_before_drop = conf.wait_for_display_draw;

		fix_timers();

		if (auto* c = al_get_current_transform(); c) latest_transform = *c;

#ifdef LUNARIS_VERBOSE_BUILD
		if (window != nullptr) PRINT_DEBUG("Display %p successfully launched", this);
		else PRINT_DEBUG("Display %p failed to launch", this);
#endif

		return window != nullptr;
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

	LUNARIS_DECL future<bool> display::post_task(std::function<bool(void)> f)
	{
		promise<bool> prom;
		if (!f) {
			auto fn = prom.get_future();
			prom.set_value(false);
			return fn;
		}
		auto fn2 = prom.get_future().then([f](auto) -> bool { try { return f(); } catch (...) { return false; } });
		promises.push_back(std::move(prom));
		return fn2;
	}

	LUNARIS_DECL future<bool> display::add_run_once_in_drawing_thread(std::function<bool(void)> f)
	{
		return post_task(f);
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

	LUNARIS_DECL future<bool> display::toggle_flag(const int flg)
	{
		return post_task([this, flg] {
			if (window) {
				if (!al_set_display_flag(window, flg, !(al_get_display_flags(window) & flg))) return false;
				if (flg & ALLEGRO_FULLSCREEN_WINDOW) al_acknowledge_resize(window);

				ALLEGRO_EVENT bev;
				bev.user.type = static_cast<int>(custom_events::DISPLAY_FLAG_TOGGLE);
				bev.user.data1 = static_cast<intptr_t>(al_get_display_width(window));
				bev.user.data2 = static_cast<intptr_t>(al_get_display_height(window));
				bev.user.data3 = (intptr_t)window;

				return al_emit_user_event(&evsrc, &bev, nullptr);
			}
			return false;
		});
	}

	LUNARIS_DECL bool display::set_icon(ALLEGRO_BITMAP* bmp)
	{
		if (window && bmp) {
			al_set_display_icon(window, bmp);
			return true;
		}
		return false;
	}
#ifdef _WIN32
	LUNARIS_DECL future<bool> display::set_icon_from_icon_resource(const int id)
	{
		return post_task([this, id] {
			if (!window) return false;
			HICON icon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(id), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
			if (icon) {
				HWND winhandle = al_get_win_window_handle(window);
				SetClassLongPtr(winhandle, GCLP_HICON, (LONG_PTR)icon);
				SetClassLongPtr(winhandle, GCLP_HICONSM, (LONG_PTR)icon);
				if (last_icon_handle) DestroyIcon(last_icon_handle);
				last_icon_handle = icon;
				return true;
			}
			return false;
		});
	}
#endif
	LUNARIS_DECL bool display::get_is_economy_mode_activated() const
	{
		return economy_mode;
	}

	LUNARIS_DECL double display::get_economy_fps() const
	{
		return economy_fps;
	}

	LUNARIS_DECL double display::get_fps_limit() const
	{
		return default_fps;
	}

	LUNARIS_DECL void display::set_is_auto_economy_set(const bool var)
	{
		set_economy_fps(var ? 30 : 0);
	}

	LUNARIS_DECL void display::set_economy_fps(const double var)
	{
		economy_fps = var < 0.0 ? 0.0 : var;
		fix_timers();
	}

	LUNARIS_DECL void display::set_fps_limit(const double var)
	{
		default_fps = var < 0.0 ? 0.0 : var;
		fix_timers();
	}

	LUNARIS_DECL void display::hold_draw(const bool var)
	{
		totally_hold_draw = var;
	}

	LUNARIS_DECL bool display::get_wait_for_flip() const
	{
		return wait_for_display_flip_before_drop;
	}

	LUNARIS_DECL void display::set_wait_for_flip(const bool var)
	{
		wait_for_display_flip_before_drop = var;
	}

	LUNARIS_DECL bool display::empty() const
	{
		return window == nullptr;
	}

	LUNARIS_DECL void display::destroy()
	{
		if (timed_draw) {
			al_stop_timer(timed_draw);
			if (ev_qu) al_unregister_event_source(ev_qu, al_get_timer_event_source(timed_draw));
			al_destroy_timer(timed_draw);
			timed_draw = nullptr;
		}
		if (update_tasks) {
			al_stop_timer(update_tasks);
			if (ev_qu) al_unregister_event_source(ev_qu, al_get_timer_event_source(update_tasks));
			al_destroy_timer(update_tasks);
			update_tasks = nullptr;
		}
		if (window) {
			al_destroy_display(window);
			window = nullptr;
		}
		if (ev_qu) {
			al_destroy_user_event_source(&evsrc);
			al_destroy_event_queue(ev_qu);
			ev_qu = nullptr;
		}
#ifdef _WIN32
		if (last_icon_handle) {
			DestroyIcon(last_icon_handle);
			last_icon_handle = nullptr;
		}
#endif
	}

	LUNARIS_DECL ALLEGRO_DISPLAY* display::get_raw_display() const
	{
		return window;
	}

	LUNARIS_DECL display::operator std::vector<ALLEGRO_EVENT_SOURCE*>() const
	{
		return get_event_sources();
	}

	LUNARIS_DECL std::vector<ALLEGRO_EVENT_SOURCE*> display::get_event_sources() const
	{
		std::vector<ALLEGRO_EVENT_SOURCE*> _tmp;
		if (window) {
			_tmp.push_back(al_get_display_event_source(window));
			_tmp.push_back((ALLEGRO_EVENT_SOURCE*)&evsrc);
		}
		return _tmp;
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
		if (window) al_set_target_backbuffer(window);
	}

	LUNARIS_DECL void display::flip()
	{
		try {
			ALLEGRO_EVENT ev;
			while (!flag_draw_timed && auto_get_next_event(ev)) // drop if draw, always
			{
				switch (ev.type) {

				case ALLEGRO_EVENT_DISPLAY_EXPOSE:
				case ALLEGRO_EVENT_DISPLAY_FOUND:
				case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:

					economy_mode = false;
					fix_timers();
					if (auto* c = al_get_current_transform(); c) latest_transform = *c;

					break;

				case ALLEGRO_EVENT_DISPLAY_LOST:
				case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:

					economy_mode = true;
					fix_timers();
					if (auto* c = al_get_current_transform(); c) latest_transform = *c;

					break;

				case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:

					hold_draw(true);
					al_acknowledge_drawing_halt(window);

					break;

				case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:

					al_acknowledge_drawing_resume(window);
					hold_draw(false);
					if (auto* c = al_get_current_transform(); c) latest_transform = *c;

					break;

				case ALLEGRO_EVENT_DISPLAY_RESIZE:

					acknowledge_resize();
					if (auto* c = al_get_current_transform(); c) latest_transform = *c;

					break;

				case static_cast<int>(custom_events::DISPLAY_FLAG_TOGGLE):

					if (auto* c = al_get_current_transform(); c) latest_transform = *c;
					break;

				case ALLEGRO_EVENT_TIMER: // DRAW EVENT!

					if (ev.timer.source == update_tasks) {
						if (auto* c = al_get_current_transform(); c) latest_transform = *c;

						if (promises.size()) {
							promises.safe([](std::vector<promise<bool>>& vec) { for (auto& i : vec) { i.set_value(true); } vec.clear(); });
						}
					}
					else {
						flag_draw_timed = true;
					}

					break;
				}
			}

			const bool can_draw_now = (flag_draw_timed || !timed_draw) && !totally_hold_draw && window;
			flag_draw_timed = false;

			if (can_draw_now) al_flip_display();

			if (promises.size()) { // maybe there's something to do before that's available
				promises.safe([](std::vector<promise<bool>>& vec) { for (auto& i : vec) { i.set_value(true); } vec.clear(); });
			}
		}
		catch (const std::exception& e) {
			PRINT_DEBUG("Display exception %p: %s", this, e.what());
			m_err.csafe([&e](const std::function<void(const std::exception&)>& f) {if (f) f(e); });
		}
		catch (...) {
			PRINT_DEBUG("Display exception %p: UNCAUGHT", this);
			m_err.csafe([](const std::function<void(const std::exception&)>& f) {if (f) f(std::runtime_error("UNCAUGHT")); });
		}
	}

	LUNARIS_DECL future<bool> display::acknowledge_resize()
	{
		return post_task([this] {
			if (window) return al_acknowledge_resize(window);
			return false;
		});
	}

	LUNARIS_DECL void display::hook_exception_handler(std::function<void(const std::exception&)> f)
	{
		m_err = f;
	}

	LUNARIS_DECL void display::unhook_exception_handler()
	{
		m_err = std::function<void(const std::exception&)>{};
	}

	LUNARIS_DECL bool display_async::safe_run::can_run()
	{
		return !(is_paused = is_lock);
	}

	LUNARIS_DECL void display_async::safe_run::lock(const bool skip_wait)
	{
		is_lock = true;
		if (skip_wait) return;
		while (!is_paused) std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	LUNARIS_DECL void display_async::safe_run::unlock() 
	{
		is_lock = false;
	}

	LUNARIS_DECL void display_async::async_run()
	{
		if (!safer.can_run()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			return;
		}

		flip();

		if (hooked_draw) hooked_draw(*this);
	}

	LUNARIS_DECL display_async::display_async(const display_config& conf)
	{
		if (!create(conf)) throw std::runtime_error("Cannot create display.");
	}

	LUNARIS_DECL display_async::~display_async()
	{
		destroy();
	}

	LUNARIS_DECL bool display_async::create(const display_config& conf)
	{
		promise<bool> prom;
		future<bool> fut1 = prom.get_future();
		future<bool> fut2 = fut1.then([&conf, this](auto) {
			bool gud = this->display::create(conf); 
			PRINT_DEBUG("Display %p is async", this);
			return gud;
		});

		promises.push_back(std::move(prom));
		thr.task_async([this] { async_run(); }, thread::speed::UNLEASHED, 0.0, [this](const std::exception& e){m_err.csafe([&e](const std::function<void(const std::exception&)>& f) { PRINT_DEBUG("Display async exception (display %p): %s", al_get_current_display(), e.what()); if (f) f(e); }); });

		fut2.wait();

		return fut2.get();
	}

	LUNARIS_DECL bool display_async::create(const int width, const int height, const int freq)
	{
		display_config conf;
		conf.mode.width = width;
		conf.mode.height = height;
		conf.mode.freq = freq;
		conf.fullscreen = false;
		return create(conf);
	}

	LUNARIS_DECL bool display_async::create(const std::string& wname, const int width, const int height, const int freq)
	{
		display_config conf;
		conf.mode.width = width;
		conf.mode.height = height;
		conf.mode.freq = freq;
		conf.fullscreen = false;
		conf.window_title = wname;
		return create(conf);
	}

	LUNARIS_DECL void display_async::hook_draw_function(std::function<void(const display_async&)> f)
	{
		safer.lock();
		hooked_draw = f;
		safer.unlock();
	}

	LUNARIS_DECL void display_async::unhook_draw_function()
	{
		safer.lock();
		hooked_draw = {};
		safer.unlock();
	}

	LUNARIS_DECL void display_async::destroy(const bool skip_except)
	{
		thr.join(skip_except);
		this->display::destroy();
	}

	LUNARIS_DECL display_event::display_event(display& rf, const ALLEGRO_EVENT& ev)
		: _ev(ev), _ref(rf)
	{
		source = ev.display.source;

		if (_ev.type == static_cast<int>(display::custom_events::DISPLAY_FLAG_TOGGLE)) {
			_transl.width = _ev.user.data1;
			_transl.height = _ev.user.data2;
			_transl.orientation = al_get_display_orientation(_ref.get_raw_display());
			_transl.source = (ALLEGRO_DISPLAY*)_ev.user.data3;
			_transl.type = ALLEGRO_EVENT_DISPLAY_RESIZE;
			al_get_window_position(_ref.get_raw_display(), &_transl.x, &_transl.y);
		}
	}

	LUNARIS_DECL display* display_event::operator->()
	{
		return &_ref;
	}

	LUNARIS_DECL display* display_event::operator->() const
	{
		return &_ref;
	}

	LUNARIS_DECL bool display_event::valid() const
	{
		return source == _ref.get_raw_display();
	}

	LUNARIS_DECL bool display_event::is_close() const
	{
		return _ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE;
	}

	LUNARIS_DECL bool display_event::is_switch_off() const
	{
		return _ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT || _ev.type == ALLEGRO_EVENT_DISPLAY_LOST;
	}

	LUNARIS_DECL bool display_event::is_switch_on() const
	{
		return _ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN || _ev.type == ALLEGRO_EVENT_DISPLAY_FOUND || _ev.type == ALLEGRO_EVENT_DISPLAY_EXPOSE;
	}

	LUNARIS_DECL bool display_event::is_resize() const
	{
		return _ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE || _ev.type == static_cast<int>(display::custom_events::DISPLAY_FLAG_TOGGLE);
	}

	LUNARIS_DECL bool display_event::is_flag_change() const
	{
		return _ev.type == static_cast<int>(display::custom_events::DISPLAY_FLAG_TOGGLE);
	}

	LUNARIS_DECL bool display_event::is_emergency_stop() const
	{
		return _ev.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING;
	}

	LUNARIS_DECL bool display_event::is_emergency_stop_gone() const
	{
		return _ev.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING;
	}

	LUNARIS_DECL int display_event::get_type() const
	{
		return _ev.type;
	}

	LUNARIS_DECL future<bool> display_event::post_task(std::function<bool(void)> f)
	{
		return _ref.post_task(f);
	}

	LUNARIS_DECL const ALLEGRO_DISPLAY_EVENT& display_event::as_display() const
	{
		if (_ev.type == static_cast<int>(display::custom_events::DISPLAY_FLAG_TOGGLE))
			return _transl;
		else return _ev.display;
	}

	LUNARIS_DECL const ALLEGRO_TIMER_EVENT& display_event::as_timer() const
	{
		return _ev.timer;
	}

	LUNARIS_DECL const ALLEGRO_EVENT& display_event::get_event() const
	{
		return _ev;
	}

	LUNARIS_DECL display& display_event::get_display()
	{
		return _ref;
	}

}