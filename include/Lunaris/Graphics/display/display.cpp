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

	LUNARIS_DECL display::display(const display_config& conf)
	{
		if (!create(conf)) throw std::runtime_error("Can't create display!");
	}

	LUNARIS_DECL display::~display()
	{
		destroy();
	}

	LUNARIS_DECL bool display::check_has_clipboard() const
	{
		return window ? al_clipboard_has_text(window) : false;
	}

	LUNARIS_DECL std::string display::get_clipboard() const
	{
		if (!window) return {};
		std::string cpy;
		char* buf = al_get_clipboard_text(window);
		if (!buf) return {};
		cpy = buf;
		al_free(buf);
		al_set_clipboard_text(window, nullptr);
		return cpy;
	}

	LUNARIS_DECL bool display::set_clipboard(const std::string& var) const
	{
		return window ? al_set_clipboard_text(window, var.c_str()) : false;
	}

	LUNARIS_DECL bool display::create(const display_config& conf)
	{
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


		if (conf.use_basic_internal_event_system) {
			if (!(ev_qu = al_create_event_queue())) {
				return false;
			}					
		}

		if (!(window = al_create_display(conf.mode.width > 0 ? conf.mode.width : 0, conf.mode.height > 0 ? conf.mode.height : 0)))
		{
			if (ev_qu) al_destroy_event_queue(ev_qu);
			ev_qu = nullptr;
			return false;
		}			

		if (conf.use_basic_internal_event_system) {
			al_register_event_source(ev_qu, al_get_display_event_source(window));
		}
		al_set_target_backbuffer(window);

		al_identity_transform(&latest_transform);

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

	LUNARIS_DECL void display::hook_event_handler(std::function<void(const ALLEGRO_EVENT&)> f)
	{
		hooked_events = f;
	}

	LUNARIS_DECL void display::unhook_event_handler()
	{
		hooked_events = std::function<void(const ALLEGRO_EVENT&)>{};
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

	LUNARIS_DECL bool display::set_icon(ALLEGRO_BITMAP* bmp)
	{
		if (window && bmp) {
			al_set_display_icon(window, bmp);
			return true;
		}
		return false;
	}
#ifdef _WIN32
	LUNARIS_DECL bool display::set_icon_from_icon_resource(const int id)
	{
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
	}
#endif
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
		if (ev_qu) {
			al_destroy_event_queue(ev_qu);
			ev_qu = nullptr;
			last_event_check = 0.0;
		}
		if (window) {
			al_destroy_display(window);
			window = nullptr;
		}
#ifdef _WIN32
		if (last_icon_handle) {
			DestroyIcon(last_icon_handle);
			last_icon_handle = nullptr;
		}
#endif
		hooked_events.reset();
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
		if (window) al_set_target_backbuffer(window);
	}

	LUNARIS_DECL void display::flip()
	{
		if (window) {
			al_flip_display();

			if (economy_mode) std::this_thread::sleep_for(economy_mode_delay);

			if (((al_get_time() - last_event_check) > default_display_self_check_time)) {

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
						hooked_events.csafe([&](const std::function<void(const ALLEGRO_EVENT&)>& fev) {
							if (fev) fev(ev);
						});
					}
				}
			}
		}
	}

	LUNARIS_DECL void display::acknowledge_resize()
	{
		if (window) al_acknowledge_resize(window);
	}

	LUNARIS_DECL bool display_async::safe_run::can_run()
	{
		is_paused = is_lock;
		return !is_paused;
	}

	LUNARIS_DECL void display_async::safe_run::lock(const bool skip_wait)
	{
		is_lock = true;
		if (skip_wait) return;
		while (!is_paused) std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	LUNARIS_DECL void display_async::safe_run::unlock() 
	{
		is_lock = false;
	}

	LUNARIS_DECL void display_async::async_run()
	{
		if (!safer.can_run()) return;

		if (promises.size()) {
			promises.safe([](std::vector<promise<bool>>& vec) { for (auto& i : vec) { i.set_value(true); } });
			promises.clear();
		}

		if (hooked_draw) hooked_draw(*this);

		flip();
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
		future<bool> fut2 = fut1.then([&conf, this](auto) { return this->display::create(conf); });

		promises.push_back(std::move(prom));
		thr.task_async([this] { async_run(); }, thread::speed::HIGH_PERFORMANCE);

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

	LUNARIS_DECL future<bool> display_async::add_run_once_in_drawing_thread(std::function<void(void)> f)
	{
		promise<bool> prom;
		if (!f) { auto fn = prom.get_future(); prom.set_value(false); return fn; }
		auto fn2 = prom.get_future().then([f](auto) -> bool { try { f(); return true; } catch (...) { return false; } });
		promises.push_back(std::move(prom));
		return fn2;
	}

	LUNARIS_DECL void display_async::destroy(const bool skip_except)
	{
		thr.join(skip_except);
		this->display::destroy();
	}

}