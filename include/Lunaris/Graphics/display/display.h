#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Utility/future.h>
#include <Lunaris/Utility/thread.h>
#include <Lunaris/Utility/safe_data.h>
#include <Lunaris/Events/generic_event_handler.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#ifdef _WIN32
#include <allegro5/allegro_windows.h>
#include <Windows.h>
#endif
#include <vector>
#include <stdexcept>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

namespace Lunaris {

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
		double max_frames = 0; // 0 == no set
		double min_frames = 30; // 0 == no set

		int flags_combine() const;

		display_config& set_display_mode(const display_options&);
		display_config& set_extra_flags(const int);
		display_config& set_samples(const int);
		display_config& set_fullscreen(const bool);
		display_config& set_vsync(const bool);
		// deprecated. Now it works like set_economy_framerate_limit(30) if true or set_economy_framerate_limit(0) if false
		display_config& set_auto_economy_mode(const bool);
		display_config& set_economy_framerate_limit(const double);
		display_config& set_single_buffer(const bool);
		// set framerate limit. Precision gets lower as value goes higher. This is not perfect, but it's near.
		display_config& set_framerate_limit(const double);
		display_config& set_use_basic_internal_event_system(const bool);
		display_config& set_window_title(const std::string&);
	};

	std::vector<display_options> get_current_modes(const int = 0);

	class display : public NonCopyable, public NonMovable {
	public:
		class _clipboard : public NonCopyable{
			ALLEGRO_DISPLAY* src;
		public:
			_clipboard(ALLEGRO_DISPLAY*);

			bool has_text() const;
			std::string get_text() const;
			bool set_text(const std::string&);
		};
	private:
		ALLEGRO_DISPLAY* window = nullptr;
		ALLEGRO_EVENT_QUEUE* ev_qu = nullptr;
		ALLEGRO_TIMER* timed_draw = nullptr;
		ALLEGRO_TIMER* update_transform = nullptr;

		ALLEGRO_TRANSFORM latest_transform{}; // useful elsewhere, trust me (see mouse)

		bool economy_mode = false;
		bool totally_hold_draw = false;
		bool flag_draw_timed = false; // when timer, this is used

		double economy_fps = 0.0; // 0 == no delay
		double default_fps = 0.0; // 0 == no delay
#ifdef _WIN32
		HICON last_icon_handle = nullptr;
#endif

		void fix_timers();
	public:
		display() = default;
		display(const display_config&);
		~display();

		_clipboard clipboard() const;

		bool create(const display_config& = {});
		bool create(const int, const int, const int = 0);
		bool create(const std::string&, const int, const int, const int = 0);

		void set_window_title(const std::string&);

		int get_width() const;
		int get_height() const;
		int get_frequency() const;
		int get_flags() const;

		void toggle_flag(const int);

		bool set_icon(ALLEGRO_BITMAP*);
#ifdef _WIN32
		bool set_icon_from_icon_resource(const int);
#endif

		// only if use_basic_internal_event_system is enabled
		bool get_is_economy_mode_activated() const;

		double get_economy_fps() const;
		double get_fps_limit() const;

		// deprecated. Consider using set_economy_fps_limit with 30 for enabled and 0 to disabled
		void set_is_auto_economy_set(const bool);

		void set_economy_fps(const double);
		void set_fps_limit(const double);

		// stop drawing at all
		void hold_draw(const bool);

		bool empty() const;

		void destroy();

		ALLEGRO_DISPLAY* get_raw_display() const;

		operator ALLEGRO_EVENT_SOURCE*() const;
		ALLEGRO_EVENT_SOURCE* get_event_source() const;

		std::function<ALLEGRO_TRANSFORM(void)> get_current_transform_function(); // keep this valid while using it!
		operator std::function<ALLEGRO_TRANSFORM(void)>() const; // same as ^^

		void set_as_target() const;
		void flip();

		// not needed if use_basic_internal_event_system was on (defaults to on)
		void acknowledge_resize();
	};

	class display_async : public display {
		struct safe_run {
			volatile bool is_lock = false; // user changing things
			bool is_paused = false; // tell user thread ack

			bool can_run(); // true if good, false if wait (continue;)
			void lock(const bool = false); // tell thread to stop, wait for ack (if false)
			void unlock(); // tell thread it's good, no wait
		} safer;

		thread thr;
		safe_vector<promise<bool>> promises;
		std::function<void(const display_async&)> hooked_draw;
		void async_run();
	public:
		display_async() = default;
		display_async(const display_config&);
		~display_async();

		bool create(const display_config & = {});
		bool create(const int, const int, const int = 0);
		bool create(const std::string&, const int, const int, const int = 0);

		void hook_draw_function(std::function<void(const display_async&)>);
		void unhook_draw_function();

		future<bool> add_run_once_in_drawing_thread(std::function<void(void)>);

		// skip exceptions? DO NOT CALL FROM ITSELF
		void destroy(const bool = false);

		using display::clipboard;
		using display::set_window_title;
		using display::get_width;
		using display::get_height;
		using display::get_frequency;
		using display::get_flags;
		using display::toggle_flag;
		using display::set_icon;
#ifdef _WIN32
		using display::set_icon_from_icon_resource;
#endif
		using display::get_is_economy_mode_activated;
		using display::get_economy_fps;
		using display::get_fps_limit;
		using display::set_is_auto_economy_set;
		using display::set_economy_fps;
		using display::set_fps_limit;
		using display::hold_draw;
		using display::empty;
		using display::get_raw_display;
		using display::operator ALLEGRO_EVENT_SOURCE*;
		using display::get_event_source;
		using display::get_current_transform_function;
		using display::operator std::function<ALLEGRO_TRANSFORM(void)>;
	};

	class display_event : public NonCopyable, public NonMovable {
		// cached stuff:
		ALLEGRO_DISPLAY* source = nullptr;
		const ALLEGRO_EVENT& _ev;
		display& _ref;
	public:
		display_event(display&, const ALLEGRO_EVENT&);

		bool valid() const;

		bool is_close() const;
		bool is_switch_off() const;
		bool is_switch_on() const;
		bool is_resize() const;
		bool is_emergency_stop() const;
		bool is_emergency_stop_gone() const;

		const ALLEGRO_EVENT& get_event() const;
		display& get_display();
	};

	using display_event_handler = specific_event_handler<display_event, display>;

}