#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Utility/future.h>
#include <Lunaris/Utility/thread.h>
#include <Lunaris/Utility/safe_data.h>
#include <Lunaris/Events/specific_event_handler.h>
#include <Lunaris/Graphics/texture.h>

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

	const double max_time_wait_for_event = 1.0;

	void __display_allegro_start();

	/// <summary>
	/// <para>display_options is each possible display configuration or your custom configuration.</para>
	/// <para>This is used for display creation or display mode listing.</para>
	/// </summary>
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

	/// <summary>
	/// <para>display_config is used when creating a display.</para>
	/// <para>A display has some things automatically done (like the economy mode). You can setup things with this.</para>
	/// </summary>
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
		bool wait_for_display_draw = true; // if there's a timer for display aka max_frames is defined, when flip() should it wait for an DISPLAY FLIP event before returning the function or just drop if there's nothing to do?

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
		display_config& set_wait_for_display_draw(const bool);
	};

	/// <summary>
	/// <para>Get the possible display modes of the display. 0 should be the main display.</para>
	/// </summary>
	/// <param name=""></param>
	/// <returns>{vector} Vector of display modes of this display.</returns>
	std::vector<display_options> get_current_modes(const int = 0);

	/// <summary>
	/// <para>A display with all the tools you may use.</para>
	/// <para>This class can have its own display event handler internally, fps limiter and economy mode (when not selected).</para>
	/// </summary>
	class display : public NonCopyable, public NonMovable {
	public:
		class _clipboard : public NonCopyable{
			ALLEGRO_DISPLAY* src;
		public:
			_clipboard(ALLEGRO_DISPLAY*);

			bool has_text() const;
			std::string get_text() const;
			bool clear_text();
			bool set_text(const std::string&);
		};
		enum class custom_events {DISPLAY_FLAG_TOGGLE = 1024};
	private:
		ALLEGRO_DISPLAY* window = nullptr;
		ALLEGRO_EVENT_QUEUE* ev_qu = nullptr;
		ALLEGRO_TIMER* timed_draw = nullptr;
		ALLEGRO_TIMER* update_tasks = nullptr;
		ALLEGRO_EVENT_SOURCE evsrc; // on toggle, because it's broken somehow lol
		std::string latest_window_title;

		ALLEGRO_TRANSFORM latest_transform{}; // useful elsewhere, trust me (see mouse)

		bool economy_mode = false;
		bool totally_hold_draw = false;
		bool flag_draw_timed = false; // when timer, this is used
		bool wait_for_display_flip_before_drop = true;
		bool _can_draw_now = false;

		double economy_fps = 0.0; // 0 == no delay
		double default_fps = 0.0; // 0 == no delay

		hybrid_memory<texture> current_icon;
#ifdef _WIN32
		HICON last_icon_handle = nullptr;
#endif
		void fix_timers();

		// automatically handle wait_for_display_flip_before_drop property and wait or get directly (or max_time_wait_for_event)
		bool auto_get_next_event(ALLEGRO_EVENT&);
	protected:
		safe_vector<promise<bool>> promises; // when events, they can list things to do here, or maybe another thread somewhere else, idk
		std::vector<promise<void>> promises_on_destroy; // run on destroy
		safe_data<std::function<void(const std::exception&)>> m_err; // on error
	public:
		display() = default;

		/// <summary>
		/// <para>Create a display directly with this configuration.</para>
		/// </summary>
		/// <param name="{display_config}">A display configuration.</param>
		display(const display_config&);

		// Close and destroy.
		~display();

		/// <summary>
		/// <para>The clipboard is not really per display, but related to one, so that's why this is here.</para>
		/// </summary>
		/// <returns>{_clipboard} A temporary clipboard object.</returns>
		_clipboard clipboard() const;

		/// <summary>
		/// <para>Create a display with default configuration (fullscreen) or with your custom configuration.</para>
		/// </summary>
		/// <param name="{display_config}">Optional display configuration.</param>
		/// <returns>{bool} True on success.</returns>
		bool create(const display_config& = {});

		/// <summary>
		/// <para>Create display with default config, but specific resolution and screen refresh rate (also works as fps limiter).</para>
		/// </summary>
		/// <param name="{int}">Width.</param>
		/// <param name="{int}">Height.</param>
		/// <param name="{int}">Frequency and fps limiter (at the same time).</param>
		/// <returns>{bool} True on success.</returns>
		bool create(const int, const int, const int = 0);

		/// <summary>
		/// <para>Create display with default config, but specific resolution, title and screen refresh rate (also works as fps limiter).</para>
		/// </summary>
		/// <param name="{std::string}">Window title.</param>
		/// <param name="{int}">Width.</param>
		/// <param name="{int}">Height.</param>
		/// <param name="{int}">Frequency and fps limiter (at the same time).</param>
		/// <returns>{bool} True on success.</returns>
		bool create(const std::string&, const int, const int, const int = 0);

		/// <summary>
		/// <para>Change window title.</para>
		/// </summary>
		/// <param name="{std::string}">New window title.</param>
		void set_window_title(const std::string&);

		/// <summary>
		/// <para>Get latest set by you window title (there's no way to get the real title, so this is the latest one you've set using this class).</para>
		/// </summary>
		/// <returns>{std::string} Window title, if there's one cached, else empty string.</returns>
		const std::string& get_window_title() const;

		/// <summary>
		/// <para>Add a task to do as the display thread (the one you're calling flip()). It is useful in multithreaded applications.</para>
		/// </summary>
		/// <param name="{function}">A function to run on the display thread.</param>
		/// <returns>{future} False if failed or if your function returns false, else true.</returns>
		future<bool> post_task(std::function<bool(void)>);

		/// <summary>
		/// <para>Add a task to do right before destroy close the display.</para>
		/// <para>Useful after any texture creation that should only exist while display exists.</para>
		/// <para>OPENGL is known to be very single-threaded in the drawing stuff, so this may help with that.</para>
		/// </summary>
		/// <param name="{function}">A function to run on the display thread right before display is destroyed.</param>
		/// <returns>{future} You can check later if you'd like to.</returns>
		future<void> post_task_on_destroy(std::function<void(void)>);

		/// <summary>
		/// <para>[DEPRECATED] Add a task to do as the display thread (the one you're calling flip()). It is useful in multithreaded applications.</para>
		/// </summary>
		/// <param name="{function}">A function to run on the display thread.</param>
		/// <returns>{future} False if failed or if your function returns false, else true.</returns>
		future<bool> add_run_once_in_drawing_thread(std::function<bool(void)>);

		/// <summary>
		/// <para>Get display width.</para>
		/// </summary>
		/// <returns>{int} Display width.</returns>
		int get_width() const;

		/// <summary>
		/// <para>Get display height.</para>
		/// </summary>
		/// <returns>{int} Display height.</returns>
		int get_height() const;

		/// <summary>
		/// <para>Get display frequency.</para>
		/// </summary>
		/// <returns>{int} Display frequency.</returns>
		int get_frequency() const;

		/// <summary>
		/// <para>Get display flags.</para>
		/// </summary>
		/// <returns>{int} Display flags.</returns>
		int get_flags() const;

		/// <summary>
		/// <para>Toggle specific display flag (from drawing thread, "async").</para>
		/// </summary>
		/// <param name="{int}">Display flag to toggle.</param>
		/// <returns>{future} True in the future if good.</returns>
		future<bool> toggle_flag(const int);

		/// <summary>
		/// <para>Set a texture as icon.</para>
		/// </summary>
		/// <param name="{hybrid_memory&lt;texture&gt;}">A texture.</param>
		/// <returns>{bool} True on success.</returns>
		bool set_icon(const hybrid_memory<texture>&);

#ifdef _WIN32
		/// <summary>
		/// <para>Load an icon directly from a resource ID (VS).</para>
		/// </summary>
		/// <param name="{int}">Resource ID.</param>
		/// <returns>{future} True on success.</returns>
		future<bool> set_icon_from_icon_resource(const int);
#endif
		/// <summary>
		/// <para>Does this display have its own event handler in it?</para>
		/// <para>This can only be set when creating a display.</para>
		/// </summary>
		/// <returns>{bool} True if there is one.</returns>
		bool has_event_queue() const;

		/// <summary>
		/// <para>Is economy mode set?</para>
		/// <para>When economy mode is set, this means that the screen is out of focus.</para>
		/// <para>This only works if there's an event queue (has_event_queue() will tell you, if you don't know somehow).</para>
		/// </summary>
		/// <returns>{bool} True if there is one.</returns>
		bool get_is_economy_mode_activated() const;

		/// <summary>
		/// <para>Get current fps setting for economy mode.</para>
		/// <para>This is the framerate you'll experience when the screen is not in focus.</para>
		/// <para>This only works if there's an event queue (has_event_queue() will tell you, if you don't know somehow).</para>
		/// </summary>
		/// <returns>{double} Current economy mode FPS limiter. Zero means infinite or no event queue to control it.</returns>
		double get_economy_fps() const;

		/// <summary>
		/// <para>Get the maximum fps allowed in here (by timer).</para>
		/// <para>This is the framerate you'll experience when the screen is in focus.</para>
		/// <para>This only works if there's an event queue (has_event_queue() will tell you, if you don't know somehow).</para>
		/// </summary>
		/// <returns>{double} Maximum fps allowed. Zero means infinite or no event queue to control it.</returns>
		double get_fps_limit() const;

		/// <summary>
		/// <para>Set the economy mode maximum FPS.</para>
		/// <para>This only works if there's an event queue (has_event_queue() will tell you, if you don't know somehow).</para>
		/// </summary>
		/// <param name="{double}"> Frames per second limiter. Zero means infinite.</param>
		void set_economy_fps(const double);

		/// <summary>
		/// <para>Set the maximum FPS allowed.</para>
		/// <para>This only works if there's an event queue (has_event_queue() will tell you, if you don't know somehow).</para>
		/// </summary>
		/// <param name="{double}"> Frames per second limiter. Zero means infinite.</param>
		void set_fps_limit(const double);

		/// <summary>
		/// <para>Stop the drawing stuff for real. This is used on HALT events, but you can force the display to stop too.</para>
		/// <para>It's not recommended to use this because of the event thing, but on desktop maybe that's ok.</para>
		/// </summary>
		/// <param name="{bool}">Stop or not?</param>
		void hold_draw(const bool);

		/// <summary>
		/// <para>The drawing thread can return when not allowed to draw (fps limiter thing).</para>
		/// <para>If you set wait_for_flip, it'll wait for the next frame to return on flip(), else returns every time there's nothing to do or after doing any task.</para>
		/// <para>Fun fact: the draw is done after the return event, so the information on screen is always as fresh as it can be.</para>
		/// <para>Waiting for flip is useful when you do things outside the drawing thread (multitasking), but if you're single thread maybe you'd want to task things even when display is not ready to draw.</para>
		/// <para>Vsync may break "no wait".</para>
		/// </summary>
		/// <returns>{bool} Is this waiting for a draw thread to then return flip()?</returns>
		bool get_wait_for_flip() const;

		/// <summary>
		/// <para>The drawing thread can return when not allowed to draw (fps limiter thing).</para>
		/// <para>If you set wait_for_flip, it'll wait for the next frame to return on flip(), else returns every time there's nothing to do or after doing any task.</para>
		/// <para>Fun fact: the draw is done after the return event, so the information on screen is always as fresh as it can be.</para>
		/// <para>Waiting for flip is useful when you do things outside the drawing thread (multitasking), but if you're single thread maybe you'd want to task things even when display is not ready to draw.</para>
		/// <para>Vsync may break "no wait".</para>
		/// </summary>
		/// <param name="{bool}">Wait for draw time?</param>
		void set_wait_for_flip(const bool);

		/// <summary>
		/// <para>Is there a display? If not, it's empty!</para>
		/// </summary>
		/// <returns>{bool} No display in here.</returns>
		bool empty() const;

		/// <summary>
		/// <para>Is this a valid display to draw? Is there a display to draw? That's the question.</para>
		/// </summary>
		/// <returns>{bool} True if there's a display.</returns>
		bool valid() const;

		/// <summary>
		/// <para>This is a funny function.</para>
		/// <para>If you're NOT on the drawing thread (by display target check), this will add a task to destroy the display on the display thread.</para>
		/// <para>If this is THE DRAWING THREAD, it will destroy the display immediately.</para>
		/// </summary>
		/// <returns>{future} Destroyed successfully?</returns>
		future<bool> destroy();

		/// <summary>
		/// <para>Get the internal raw display, because why not?</para>
		/// </summary>
		/// <returns>{ALLEGRO_DISPLAY*} The raw display.</returns>
		ALLEGRO_DISPLAY* get_raw_display() const;

		/// <summary>
		/// <para>Get the internal raw display, because why not?</para>
		/// </summary>
		operator ALLEGRO_DISPLAY*() const;

		/// <summary>
		/// <para>As the function name suggest, get the possible event sources of this display.</para>
		/// </summary>
		/// <returns>{vector} A combo of event sources (there may be more than one).</returns>
		std::vector<ALLEGRO_EVENT_SOURCE*> get_event_sources() const;

		/// <summary>
		/// <para>Used on event stuff, get the event source(s) of this display.</para>
		/// </summary>
		operator std::vector<ALLEGRO_EVENT_SOURCE*>() const;

		/// <summary>
		/// <para>Get a function that copy the latest display transformation.</para>
		/// <para>This is used on things that depends on display transformation to work right, like mouse related stuff.</para>
		/// </summary>
		/// <returns>{function} A function that gets the latest display transformation matrix.</returns>
		std::function<ALLEGRO_TRANSFORM(void)> get_current_transform_function();

		/// <summary>
		/// <para>Get a function that copy the latest display transformation.</para>
		/// <para>This is used on things that depends on display transformation to work right, like mouse related stuff.</para>
		/// </summary>
		operator std::function<ALLEGRO_TRANSFORM(void)>() const;

		/// <summary>
		/// <para>Force set this as the target for this thread.</para>
		/// <para>WARNING: you should do this only on the thread the display was created, or things may break.</para>
		/// </summary>
		void set_as_target() const;

		/// <summary>
		/// <para>Flips the display, if it can.</para>
		/// <para>This also checks for events and do tasks related to the display (post_task).</para>
		/// <para>Depending on the configuration, this may return without really flipping the display (fps limiter or something + no wait).</para>
		/// </summary>
		void flip();

		/// <summary>
		/// <para>This forces the display to acknowledge itself about a resize. This is not fast.</para>
		/// <para>By the way, this is added as a task to do on flip(), so that's why the return is a future.</para>
		/// <para>If you're using internal event system, this is useless, but still available. Who am I to judge your decisions, right?</para>
		/// </summary>
		/// <returns>{future} True if good.</returns>
		future<bool> acknowledge_resize();

		/// <summary>
		/// <para>What if an exception happens? You can directly handle things if you set an exception handler.</para>
		/// </summary>
		/// <param name="{function}">A function to handle std::exception.</param>
		void hook_exception_handler(std::function<void(const std::exception&)>);

		/// <summary>
		/// <para>So you don't like to handle exceptions, hm? This resets the function.</para>
		/// </summary>
		void unhook_exception_handler();
	};

	/// <summary>
	/// <para>display_async is an easy display, but async. Easy, right?</para>
	/// <para>This flips by itself.</para>
	/// </summary>
	class display_async : public display {
		fast_one_way_mutex safer; // gotta go fast

		thread thr;
		std::function<void(const display_async&)> hooked_draw;
		bool m_destroy_silent = false;
		bool destroy_self_on_next = false;
		promise<bool> on_self_destroy_set; // valid if destroy_self_on_next was true once.

		void async_run();
	public:
		display_async() = default;
		display_async(const display_config&);
		~display_async();


		/// <summary>
		/// <para>Create an async display with default configuration (fullscreen) or with your custom configuration.</para>
		/// </summary>
		/// <param name="{display_config}">Optional display configuration.</param>
		/// <returns>{bool} True on success.</returns>
		bool create(const display_config& = {});

		/// <summary>
		/// <para>Create an async display with default config, but specific resolution and screen refresh rate (also works as fps limiter).</para>
		/// </summary>
		/// <param name="{int}">Width.</param>
		/// <param name="{int}">Height.</param>
		/// <param name="{int}">Frequency and fps limiter (at the same time).</param>
		/// <returns>{bool} True on success.</returns>
		bool create(const int, const int, const int = 0);

		/// <summary>
		/// <para>Create an async display with default config, but specific resolution, title and screen refresh rate (also works as fps limiter).</para>
		/// </summary>
		/// <param name="{std::string}">Window title.</param>
		/// <param name="{int}">Width.</param>
		/// <param name="{int}">Height.</param>
		/// <param name="{int}">Frequency and fps limiter (at the same time).</param>
		/// <returns>{bool} True on success.</returns>
		bool create(const std::string&, const int, const int, const int = 0);

		/// <summary>
		/// <para>Set a function to loop alongside flip().</para>
		/// <para>How else would you tell this what to draw? Big brain!</para>
		/// </summary>
		/// <param name="{function}">A function that gets the display object and does funky drawing stuff.</param>
		void hook_draw_function(std::function<void(const display_async&)>);

		/// <summary>
		/// <para>Set a function to loop alongside flip().</para>
		/// <para>How else would you tell this what to draw? Big brain!</para>
		/// </summary>
		/// <param name="{function}">A function that does funky drawing stuff.</param>
		void hook_draw_function(std::function<void(void)>);

		/// <summary>
		/// <para>Disable the drawing function (why?).</para>
		/// <para>This will leave the drawing thread just refreshing itself, by the way.</para>
		/// </summary>
		void unhook_draw_function();

		/// <summary>
		/// <para>Set next frame to destroy (stop) the thread and destroy display.</para>
		/// </summary>
		/// <param name="{bool}">On object destruction, throw any unhandled exception?</param>
		/// <returns>{future} Was it ok?</returns>
		future<bool> destroy(const bool = false);
	};

	/// <summary>
	/// <para>display_event is the translated raw event to a more pleasant interface.</para>
	/// </summary>
	class display_event : public NonCopyable, public NonMovable {
		// cached stuff:
		ALLEGRO_DISPLAY* source = nullptr;
		const ALLEGRO_EVENT& _ev;
		display& _ref;

		ALLEGRO_DISPLAY_EVENT _transl{}; // non standard window event translated
	public:
		display_event(display&, const ALLEGRO_EVENT&);

		/// <summary>
		/// <para>Directly interface with the display that triggered this.</para>
		/// </summary>
		/// <returns>{display*} The display from where the event came from.</returns>
		display* operator->();

		/// <summary>
		/// <para>Directly interface with the display that triggered this.</para>
		/// </summary>
		/// <returns>{display*} The display from where the event came from.</returns>
		display* operator->() const;

		/// <summary>
		/// <para>Is this event valid to work with?</para>
		/// </summary>
		/// <returns>{bool} True if valid.</returns>
		bool valid() const;

		/// <summary>
		/// <para>Is this event from another display somehow or empty?</para>
		/// </summary>
		/// <returns>{bool} Empty invalid event?</returns>
		bool empty() const;

		/// <summary>
		/// <para>Easy call to check if this is a close display event.</para>
		/// </summary>
		/// <returns>{bool} True if it is.</returns>
		bool is_close() const;

		/// <summary>
		/// <para>Easy call to check if this is a switch off display event.</para>
		/// </summary>
		/// <returns>{bool} True if it is.</returns>
		bool is_switch_off() const;

		/// <summary>
		/// <para>Easy call to check if this is a switch on display event.</para>
		/// </summary>
		/// <returns>{bool} True if it is.</returns>
		bool is_switch_on() const;

		/// <summary>
		/// <para>Easy call to check if this is a resize display event.</para>
		/// </summary>
		/// <returns>{bool} True if it is.</returns>
		bool is_resize() const;

		/// <summary>
		/// <para>Easy call to check if this is a flag change display event.</para>
		/// </summary>
		/// <returns>{bool} True if it is.</returns>
		bool is_flag_change() const;

		/// <summary>
		/// <para>Easy call to check if this is a emergency stop (HALT) display event.</para>
		/// </summary>
		/// <returns>{bool} True if it is.</returns>
		bool is_emergency_stop() const;

		/// <summary>
		/// <para>Easy call to check if this is a emergency stop gone (RESUME) display event.</para>
		/// </summary>
		/// <returns>{bool} True if it is.</returns>
		bool is_emergency_stop_gone() const;

		/// <summary>
		/// <para>Get the raw event type.</para>
		/// </summary>
		/// <returns>{int} Raw event ID.</returns>
		int get_type() const;

		/// <summary>
		/// <para>Directly post a task to the display.</para>
		/// </summary>
		/// <param name="{function}">A function to run on the display draw thread.</param>
		/// <returns>{future} True if your function run good.</returns>
		future<bool> post_task(std::function<bool(void)>);

		/// <summary>
		/// <para>If you're sure this is a display event, read the display part.</para>
		/// </summary>
		/// <returns>{ALLEGRO_DISPLAY_EVENT} The raw display event.</returns>
		const ALLEGRO_DISPLAY_EVENT& as_display() const;

		/// <summary>
		/// <para>If you're sure this is a timer event, read the timer part.</para>
		/// </summary>
		/// <returns>{ALLEGRO_TIMER_EVENT} The raw timer event.</returns>
		const ALLEGRO_TIMER_EVENT& as_timer() const;

		/// <summary>
		/// <para>Get the raw event itself.</para>
		/// </summary>
		/// <returns>{ALLEGRO_EVENT} The Allegro event itself.</returns>
		const ALLEGRO_EVENT& get_event() const;

		/// <summary>
		/// <para>Get the display that triggered this event.</para>
		/// </summary>
		/// <returns>{display&amp;} The display.</returns>
		display& get_display();
	};

	using display_event_handler = specific_event_handler<display_event, display>;

}