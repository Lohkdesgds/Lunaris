#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Utility/future.h>
#include <Lunaris/Utility/thread.h>
#include <Lunaris/Utility/safe_data.h>

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

	const double default_display_self_check_time = 0.5; // check events twice a second (related to last_event_check)
	const std::chrono::milliseconds economy_mode_delay = std::chrono::milliseconds(16); // when "not focused" or alt-tabbed, display will slow down (~60 fps)
	const std::function<void(void)> default_quiet_safe_function_thread = [] { std::this_thread::sleep_for(std::chrono::milliseconds(6)); };

	void __display_allegro_start();
	void __display_menu_allegro_start();

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
		bool auto_economy_mode = true;
		bool single_buffer = false;
		bool use_basic_internal_event_system = true; // automatically "acknowledge" events like resizing. If you want close events, you should register the display in a Event handler.
		double max_frames = 1.0 / 500;

		int flags_combine() const;

		display_config& set_display_mode(const display_options&);
		display_config& set_extra_flags(const int);
		display_config& set_samples(const int);
		display_config& set_fullscreen(const bool);
		display_config& set_vsync(const bool);
		display_config& set_auto_economy_mode(const bool);
		display_config& set_single_buffer(const bool);
		// set framerate limit. Only works on display_async.
		display_config& set_framerate_limit(const double);
		display_config& set_use_basic_internal_event_system(const bool);
		display_config& set_window_title(const std::string&);
	};

	struct display_sub_menu {
	private:
		std::string __makename;
	public:
		display_sub_menu();

		std::string name;
		uint16_t id = 0;
		int flags = 0;
		std::vector<display_sub_menu> sub_menus;

		display_sub_menu& make_this_division();
		display_sub_menu& set_name(const std::string&);
		display_sub_menu& set_id(const uint16_t);
		display_sub_menu& set_flags(const int);
		display_sub_menu& push(const display_sub_menu&);

		std::vector<ALLEGRO_MENU_INFO> generate();
	};

	struct display_menu_event {
		enum class flags { DISABLED = ALLEGRO_MENU_ITEM_DISABLED, AS_CHECKBOX = ALLEGRO_MENU_ITEM_CHECKED, CHECKED = ALLEGRO_MENU_ITEM_CHECKBOX };
		ALLEGRO_MENU* source = nullptr;
		std::string name;
		uint16_t id;
		bool checked = false; // if checkbox
		// possible flags: ALLEGRO_MENU_ITEM_DISABLED, ALLEGRO_MENU_ITEM_CHECKED, probably ALLEGRO_MENU_ITEM_CHECKBOX
		void toggle_flag(const flags) const;
	};

	class display_menu {
		std::vector<display_sub_menu> menus;
		std::shared_ptr<ALLEGRO_MENU> __menu;
	public:
		display_menu();

		display_menu& push(const display_sub_menu&);
		ALLEGRO_MENU* generate();
		void destroy();
	};

	std::vector<display_options> get_current_modes(const int = 0);

	class display : public NonCopyable, public NonMovable {
		ALLEGRO_DISPLAY* window = nullptr;
		ALLEGRO_EVENT_QUEUE* ev_qu = nullptr;

		ALLEGRO_TRANSFORM latest_transform{}; // useful elsewhere, trust me (see mouse)
		double last_event_check = 0.0;
		bool economy_mode = false, auto_economy = false;
#ifdef _WIN32
		HICON last_icon_handle = nullptr;
#endif
		safe_data<std::function<void(const ALLEGRO_EVENT&)>> hooked_events;
		safe_data<std::function<void(const display_menu_event&)>> menu_events;

		display_menu menu;
	public:
		display() = default;
		display(const display_config&);
		~display();

		// check if there's any clipboard
		bool check_has_clipboard() const;
		// this will cut your clipboard!
		std::string get_clipboard() const;
		// this will set your clipboard!
		bool set_clipboard(const std::string&) const;

		bool create(const display_config& = {});
		bool create(const int, const int, const int = 0);
		bool create(const std::string&, const int, const int, const int = 0);

		void set_window_title(const std::string&);

		// hook reset on re-create or destroy
		void hook_event_handler(std::function<void(const ALLEGRO_EVENT&)>);
		void unhook_event_handler();
		// hook menu handler
		void hook_menu_event_handler(std::function<void(const display_menu_event&)>);
		void unhook_menu_event_handler();

		int get_width() const;
		int get_height() const;
		int get_frequency() const;
		int get_flags() const;

		void toggle_flag(const int);

		bool set_icon(ALLEGRO_BITMAP*);
#ifdef _WIN32
		bool set_icon_from_icon_resource(const int);
#endif

		void set_menu(const display_menu&);
		void delete_menu();

		// only if use_basic_internal_event_system is enabled
		bool get_is_economy_mode_activated() const;
		void set_is_auto_economy_set(const bool);

		bool empty() const;

		void destroy();

		ALLEGRO_DISPLAY* get_raw_display() const;
		ALLEGRO_EVENT_SOURCE* get_event_source();
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

		// skip exceptions?
		void destroy(const bool = false);

		// set fps limit, directly
		void set_framerate_limit(const double);

		using display::set_window_title;
		using display::hook_event_handler;
		using display::unhook_event_handler;
		using display::hook_menu_event_handler;
		using display::unhook_menu_event_handler;
		using display::get_width;
		using display::get_height;
		using display::get_frequency;
		using display::get_flags;
		using display::toggle_flag;
		using display::set_menu;
		using display::delete_menu;
		using display::get_is_economy_mode_activated;
		using display::set_is_auto_economy_set;
		using display::empty;
		using display::get_raw_display;
		using display::get_event_source;
		using display::get_current_transform_function;
		using display::operator std::function<ALLEGRO_TRANSFORM(void)>;
	};
}