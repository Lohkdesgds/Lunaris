#include "display.h"

namespace Lunaris {

	void __display_allegro_start()
	{
		if (!al_is_system_installed() && !al_init()) throw std::runtime_error("Can't start Allegro!");
		if (!al_is_primitives_addon_initialized() && !al_init_primitives_addon()) throw std::runtime_error("Can't start Primitives!");
	}

	display_options& display_options::set_width(const int var)
	{
		width = var;
		return *this;
	}
	
	display_options& display_options::set_height(const int var)
	{
		height = var;
		return *this;
	}

	display_options& display_options::set_frequency(const int var)
	{
		freq = var;
		return *this;
	}

	display_options& display_options::set_display_index(const int var)
	{
		display_index = var;
		return *this;
	}

	int display_config::flags_combine() const
	{
		int res = extra_flags
			& ~ALLEGRO_FULLSCREEN 
			& ~ALLEGRO_FULLSCREEN_WINDOW
			& ~ALLEGRO_WINDOWED;

		return res | ((fullscreen || (mode.width == 0 && mode.height == 0)) ? ALLEGRO_FULLSCREEN_WINDOW : 0);
	}

	display_config& display_config::set_display_mode(const display_options& var)
	{
		mode = var;
		return *this;
	}

	display_config& display_config::set_extra_flags(const int var)
	{
		extra_flags = var;
		return *this;
	}

	display_config& display_config::set_samples(const int var)
	{
		samples = var;
		return *this;
	}

	display_config& display_config::set_fullscreen(const bool var)
	{
		fullscreen = var;
		return *this;
	}

	display_config& display_config::set_vsync(const bool var)
	{
		vsync = var;
		return *this;
	}

	display_config& display_config::set_single_buffer(const bool var)
	{
		single_buffer = var;
		return *this;
	}

	display_config& display_config::set_use_basic_internal_event_system(const bool var)
	{
		use_basic_internal_event_system = var;
		return *this;
	}

	display_config& display_config::set_window_title(const std::string& var)
	{
		window_title = var;
		return *this;
	}

	std::vector<display_options> get_current_modes(const int index)
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

	display::display(const display_config& conf)
	{
		if (!create(conf)) throw std::runtime_error("Can't create display!");
	}

	display::~display()
	{
		destroy();
	}

	display::display(display&& oth) noexcept
		: window(oth.window), ev_qu(oth.ev_qu), last_event_check(oth.last_event_check), hooked_events(oth.hooked_events)
	{
		oth.window = nullptr;
		oth.ev_qu = nullptr;
		oth.last_event_check = 0.0;
		oth.hooked_events = {};
	}

	void display::operator=(display&& oth) noexcept
	{
		destroy();

		window = oth.window;
		ev_qu = oth.ev_qu;
		last_event_check = oth.last_event_check;
		hooked_events = oth.hooked_events;

		oth.window = nullptr;
		oth.ev_qu = nullptr;
		oth.last_event_check = 0.0;
		oth.hooked_events = {};
	}

	bool display::create(const display_config& conf)
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
			if (!(ev_qu = al_create_event_queue()))
				return false;
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

		return true;
	}

	bool display::create(const int width, const int height, const int freq)
	{
		display_config conf;
		conf.mode.width = width;
		conf.mode.height = height;
		conf.mode.freq = freq;
		conf.fullscreen = false;
		return create(conf);
	}

	bool display::create(const std::string& wname, const int width, const int height, const int freq)
	{
		display_config conf;
		conf.mode.width = width;
		conf.mode.height = height;
		conf.mode.freq = freq;
		conf.fullscreen = false;
		conf.window_title = wname;
		return create(conf);
	}

	void display::set_window_title(const std::string& str)
	{
		if (!empty() && !str.empty()) al_set_window_title(window, str.c_str());
	}

	void display::hook_event_handler(std::function<void(const ALLEGRO_EVENT&)> f)
	{
		std::lock_guard<std::mutex> lucky(sensitive);
		hooked_events = f;
	}

	void display::unhook_event_handler()
	{
		std::lock_guard<std::mutex> lucky(sensitive);
		hooked_events = {};
	}

	int display::get_width() const
	{
		if (window) return al_get_display_width(window);
		return 0;
	}

	int display::get_height() const
	{
		if (window) return al_get_display_height(window);
		return 0;
	}

	int display::get_frequency() const
	{
		if (window) return al_get_display_refresh_rate(window);
		return 0;
	}

	int display::get_flags() const
	{
		if (window) return al_get_display_flags(window);
		return 0;
	}

	bool display::get_is_economy_mode_activated() const
	{
		return economy_mode;
	}

	bool display::empty() const
	{
		return window == nullptr;
	}

	void display::destroy()
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
		hooked_events = {};
	}

	ALLEGRO_EVENT_SOURCE* display::get_event_source()
	{
		return window ? al_get_display_event_source(window) : nullptr;
	}

	std::function<ALLEGRO_TRANSFORM(void)> display::get_current_transform_function()
	{
		return [&] {return latest_transform; };
	}

	void display::set_as_target() const
	{
		if (window) al_set_target_backbuffer(window);
	}

	void display::flip()
	{
		if (window) {
			al_flip_display();

			if (economy_mode) std::this_thread::sleep_for(economy_mode_delay);
			
			if (((al_get_time() - last_event_check) > default_display_self_check_time)) {

				std::lock_guard<std::mutex> lucky(sensitive);

				const ALLEGRO_TRANSFORM* cpy = al_get_current_transform();
				if (cpy) latest_transform = *cpy;

				if (ev_qu) {

					ALLEGRO_EVENT ev;
					while (al_get_next_event(ev_qu, &ev)) {

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

	void display::acknowledge_resize()
	{
		if (window) al_acknowledge_resize(window);
	}

}