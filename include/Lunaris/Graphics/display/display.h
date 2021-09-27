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
	const std::chrono::milliseconds economy_mode_delay = std::chrono::milliseconds(16); // when "not focused" or alt-tabbed, display will slow down (~60 fps)
	const std::function<void(void)> default_quiet_safe_function_thread = [] { std::this_thread::sleep_for(std::chrono::milliseconds(6)); };

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
		bool self_draw = false; // draws by itself (you should not call flip() or anything like that)
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
		display_config& set_self_draw(const bool);
	};

	std::vector<display_options> get_current_modes(const int = 0);

	class display {
		class self_draw_block {
			struct safe_run {
				volatile bool is_lock = false; // user changing things
				bool is_paused = false; // tell user thread ack

				bool can_run(); // true if good, false if wait (continue;)
				void lock(const bool = false); // tell thread to stop, wait for ack (if false)
				void unlock(); // tell thread it's good, no wait
			} safer;

			std::thread thr; // if self draw.
			std::exception_ptr _exception_stored;
			bool had_exception = false;
			std::function<void(void)> draw_func = default_quiet_safe_function_thread; // it should always be something
			std::vector<std::function<void(void)>> once_funcs;
			std::mutex once_funcs_safer; // as this is rarely used, it makes sense
			
			bool is_drawing = false; // self thread tell
			bool keep_drawing = false; // set to true before thread, false to kill thread

			void thr_drawing();
		public:
			self_draw_block();
			~self_draw_block();

			void add_run_once(const std::function<void(void)>);
			void set_function(const std::function<void(void)>);
			void rethrow_any_exception(); // if any exception stored, rethrow, else no throw
		};

		ALLEGRO_TRANSFORM latest_transform{}; // useful elsewhere, trust me (see mouse)
		ALLEGRO_DISPLAY* window = nullptr;
		ALLEGRO_EVENT_QUEUE* ev_qu = nullptr;
		double last_event_check = 0.0;
		std::unique_ptr<self_draw_block> draw_self; // opt
		std::function<void(const ALLEGRO_EVENT&)> hooked_events;
		bool economy_mode = false; // auto detects if display is hidden (then goes to 30 fps). Available with conf.use_basic_internal_event_system
		std::recursive_mutex sensitive;

		void _flip_nocheck();
		std::function<void(void)> combine_func_to_essential(const std::function<void(void)> = {});
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

		// only if self_draw
		void hook_draw_function(std::function<void(void)>);
		// only if self_draw. It will wait if there's a function set already.
		void add_run_once_in_drawing_thread(std::function<void(void)>);

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
		operator std::function<ALLEGRO_TRANSFORM(void)>() const; // same as ^^

		void set_as_target() const;
		void flip();

		// not needed if use_basic_internal_event_system was on (defaults to on)
		void acknowledge_resize();
	};
}