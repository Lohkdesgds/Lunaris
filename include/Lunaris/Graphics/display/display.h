#pragma once

#include <allegro5/allegro.h>
//#include <allegro5/allegro_opengl.h>
//#include <allegro5/allegro_direct3d.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include <vector>
#include <stdexcept>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

namespace Lunaris {

	const double default_display_self_check_time = 0.5; // check events twice a second (related to last_event_check)
	const std::chrono::milliseconds economy_mode_delay = std::chrono::milliseconds(15); // when "not focused" or alt-tabbed, display will slow down (~60 fps)

	void __display_allegro_start();

	struct display_options {
		int width = 0;
		int height = 0;
		int freq = 0;
		int display_index = 0;

		display_options& set_width(const int);
		display_options& set_height(const int);
		display_options& set_frequency(const int);
		display_options& set_display_index(const int);
	};

	struct display_config {
		display_options mode;
		std::string window_title = "Lunaris App";
		int extra_flags = ALLEGRO_DIRECT3D_INTERNAL; // flags_combine()
		int samples = 4; // ALLEGRO_SAMPLE_BUFFERS = samples > 0, ALLEGRO_SAMPLES = samples.
		bool fullscreen = true; // flags_combine()
		bool vsync = false;
		bool single_buffer = false;
		bool use_basic_internal_event_system = true; // automatically "acknowledge" events like resizing. If you want close events, you should register the display in a Event handler.

		int flags_combine() const;

		display_config& set_display_mode(const display_options&);
		display_config& set_extra_flags(const int);
		display_config& set_samples(const int);
		display_config& set_fullscreen(const bool);
		display_config& set_vsync(const bool);
		display_config& set_single_buffer(const bool);
		display_config& set_use_basic_internal_event_system(const bool);
		display_config& set_window_title(const std::string&);
	};

	std::vector<display_options> get_current_modes(const int = 0);

	class display {
		ALLEGRO_TRANSFORM latest_transform; // useful elsewhere, trust me (see mouse)
		ALLEGRO_DISPLAY* window = nullptr;
		ALLEGRO_EVENT_QUEUE* ev_qu = nullptr;
		double last_event_check = 0.0;
		std::function<void(const ALLEGRO_EVENT&)> hooked_events;
		bool economy_mode = false; // auto detects if display is hidden (then goes to 30 fps). Available with conf.use_basic_internal_event_system
		std::mutex sensitive;
	public:
		display() = default;
		display(const display_config&);
		~display();

		display(display&&) noexcept;
		void operator=(display&&) noexcept;

		display(const display&) = delete;
		void operator=(const display&) = delete;

		bool create(const display_config& = {});
		bool create(const int, const int, const int = 0);
		bool create(const std::string&, const int, const int, const int = 0);

		void set_window_title(const std::string&);

		// hook reset on re-create or destroy
		void hook_event_handler(std::function<void(const ALLEGRO_EVENT&)>);
		void unhook_event_handler();

		int get_width() const;
		int get_height() const;
		int get_frequency() const;
		int get_flags() const;

		// only if use_basic_internal_event_system is enabled
		bool get_is_economy_mode_activated() const;

		bool empty() const;

		void destroy();

		ALLEGRO_EVENT_SOURCE* get_event_source();
		std::function<ALLEGRO_TRANSFORM(void)> get_current_transform_function(); // keep this valid while using it!

		void set_as_target() const;
		void flip();

		// not needed if use_basic_internal_event_system was on (defaults to on)
		void acknowledge_resize();
	};

}