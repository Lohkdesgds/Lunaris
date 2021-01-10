#pragma once

#include "../../Handling/Abort/abort.h"
#include "../../Handling/Initialize/initialize.h"
#include "../../Tools/SuperThread/superthread.h"
#include "../../Tools/Future/future.h"
#include "../../Tools/SuperMutex/supermutex.h"
#include "../../Tools/Common/common.h"
#include "../../Tools/Any/any.h"
#include "../Events/events.h"
#include "../EventTimer/eventtimer.h"
#include "../EventCustom/eventcustom.h"
#include "../Bitmap/bitmap.h"
#include "../Target/target.h"
#include "../Font/font.h"
#include "../Camera/camera.h"
#include "../Logger/logger.h"
#include "../PathManager/pathmanager.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace display {
				const int minimum_display_size[2] = { 320, 240 };
				constexpr int default_new_display_flags = ALLEGRO_RESIZABLE | ALLEGRO_OPENGL /* | ALLEGRO_FULLSCREEN_WINDOW*/;
				constexpr size_t max_fps_cap = 300;
			}

			// Safe smart pointer of RAW DISPLAY.
			using DisplayStrongPtr = std::shared_ptr<ALLEGRO_DISPLAY>;
			// Any type.
			using DisplayAnySPtr = Tools::Any;
			// Function that returns any.
			using DisplayAnyFSPtr = std::function<DisplayAnySPtr(void)>;

			using DisplayTask = std::function<void(const Camera&)>;

			/// <summary>
			/// <para>Display is the window used to draw stuff.</para>
			/// </summary>
			class Display {
				int new_display_flags_apply = display::default_new_display_flags;
				int new_display_refresh_rate = 0; // not set
				int new_resolution[2] = { display::minimum_display_size[0], display::minimum_display_size[1] };
				bool should_vsync = false;
				bool force_vsync_refresh = false;
				bool is_fullscreen = false;
				bool hide_mouse_new = true;
				double double_buffering = 0.0; // <= 0.0 = no
				size_t fps_cap = 0;
				size_t frames_per_second = 0;
				size_t fails_out_of_range = 0; // out of range ignored because this is faster
				size_t fails_unexpected = 0; // other error?
				std::string window_name;

				// target index
				Interface::Target targ;

				static Tools::SuperMutex sync_threads;
				DisplayStrongPtr disp;
				Bitmap dbuffer, icon;
				EventHandler display_events{ Tools::superthread::performance_mode::LOW_POWER };

				//PathManager pathing;
				Tools::SuperThread<bool> thr{ Tools::superthread::performance_mode::HIGH_PERFORMANCE };

				size_t draw_tasks_count = 0;
				std::vector<std::pair<size_t, DisplayTask>> draw_tasks;
				Tools::SuperMutex draw_tasks_m;

				std::vector<Tools::Promise<DisplayAnySPtr>> once_tasks; // change later to a class that manages ALLEGRO_BITMAP*
				Tools::SuperMutex once_tasks_m;

				std::mutex camfu_m;
				//std::shared_ptr<Camera> camera;
				std::function<Camera(void)> camera_fu;
				Camera cam_latest;
				bool refresh_camera = false;

				bool thread_run(Tools::boolThreadF);

				void _reset_display_and_path();
				void thread_init();
				void thread_deinit();

			public:
				/// <summary>
				/// <para>Custom index for Target. If you set this different than one, you'll have to set each Sprite_Base based class to target this number. Defaults to 0.</para>
				/// </summary>
				/// <param name="{size_t}">Index for Target stuff.</param>
				Display(const size_t = 0);

				~Display();

				/// <summary>
				/// <para>Start drawing thread.</para>
				/// </summary>
				/// <returns>{Future} A Future that you can .wait() or .then() when display is closed.</returns>
				Tools::Future<bool> init();

				//path is global now. change later to local so this makes sense
				//void set_path(const PathManager&);

				/// <summary>
				/// <para>See if an event was from this Display using .source.</para>
				/// </summary>
				/// <param name="{RAW DISPLAY}">A RAW DISPLAY pointer.</param>
				/// <returns>{bool} True if this is the source.</returns>
				bool operator==(ALLEGRO_DISPLAY*) const;

				/// <summary>
				/// <para>Stops the thread and join.</para>
				/// </summary>
				void deinit();

				/// <summary>
				/// <para>Try to warp mouse to a point in screen. Range: [-1.0, 1.0], being -1,-1 the top left corner.</para>
				/// <para>Causes event ALLEGRO_EVENT_MOUSE_WARPED.</para>
				/// </summary>
				/// <param name="{double}">X coordinate [-1.0, 1.0].</param>
				/// <param name="{double}">Y coordinate [-1.0, 1.0].</param>
				/// <returns>{bool} True if worked.</returns>
				bool move_mouse_to(const double, const double);

				/// <summary>
				/// <para>Set the drawing thread to stop (no lock).</para>
				/// </summary>
				void set_stop();

				/// <summary>
				/// <para>If greater than 0.0, it will create and use a second buffer as main display with the proportion (based on screen).</para>
				/// <para>FOR SECURITY REASONS, you can't set more than 8.0 from here.</para>
				/// </summary>
				/// <param name="{double}">Scale of buffer, or no buffer if &lt;= 0.0.</param>
				void set_double_buffering_scale(const double);

				/// <summary>
				/// <para>The double buffering attribute. 0 means disabled. [0.0,8.0]</para>
				/// </summary>
				/// <returns>{double} Buffer scale.</returns>
				double get_current_buffering_scale() const;

				/// <summary>
				/// <para>Set a Camera to be applied to drawing thread.</para>
				/// </summary>
				/// <param name="{Camera}">The Camera smart pointer.</param>
				void set_camera(const Camera&);

				/// <summary>
				/// <para>Set a function that gives a valid Camera to be applied to drawing thread.</para>
				/// </summary>
				/// <param name="{std::function}">The function that returns a Camera.</param>
				void set_camera(std::function<Camera(void)>);

				/// <summary>
				/// <para>Limits the framerate to up to this value.</para>
				/// </summary>
				/// <param name="{size_t}">FPS cap (0 = unlimited).</param>
				void set_fps_cap(const size_t);

				/// <summary>
				/// <para>Gets the latest camera set in this Display (via set_camera).</para>
				/// </summary>
				/// <returns>{Camera&} The Camera.</returns>
				Camera& get_current_camera();

				/// <summary>
				/// <para>Gets if drawing thread exists and it's running (the display itself might not exist, try display_ready()).</para>
				/// </summary>
				/// <returns>{bool} True if running.</returns>
				bool running() const;

				/// <summary>
				/// <para>What flags should the new display have?</para>
				/// </summary>
				/// <param name="{int}">Binary combination of flags (ALLEGRO flags, like ALLEGRO_OPENGL|ALLEGRO_WINDOWED).</param>
				void set_new_flags(const int = display::default_new_display_flags);

				/// <summary>
				/// <para>What flags does this display have set?</para>
				/// </summary>
				/// <returns>{int} Flags.</returns>
				int get_flags() const;

				/// <summary>
				/// <para>Get Display width.</para>
				/// </summary>
				/// <returns>{int} Display width.</returns>
				const int get_width() const;

				/// <summary>
				/// <para>Get Display height.</para>
				/// </summary>
				/// <returns>{int} Display height.</returns>
				const int get_height() const;

				/// <summary>
				/// <para>Set resolution for the display.</para>
				/// </summary>
				/// <param name="{int}">Width.</param>
				void set_width(const int);

				/// <summary>
				/// <para>Set resolution for the display.</para>
				/// </summary>
				/// <param name="{int}">Height.</param>
				void set_height(const int);

				/// <summary>
				/// <para>Set display refresh rate.</para>
				/// </summary>
				/// <param name="{int}">Refresh rate. 0 goes with default settings.</param>
				void set_new_refresh_rate(const int);

				/// <summary>
				/// <para>Get current display refresh rate setting.</para>
				/// </summary>
				/// <returns>{int} The refresh rate.</returns>
				int get_refresh_rate() const;

				/// <summary>
				/// <para>Should it use VSync?</para>
				/// <para>IF SECOND PARAMETER IS SET TO TRUE, SOME TEXTURES MIGHT GLITCH OR THE PERFORMANCE MAY GET VERY LOW.</para>
				/// <para>It's recommended to restart the app.</para>
				/// </summary>
				/// <param name="{bool}">VSync on/off?</param>
				/// <param name="{bool}">Force refresh right now?</param>
				void set_vsync(const bool, const bool = false);

				/// <summary>
				/// <para>Is it with VSync enabled?</para>
				/// </summary>
				/// <returns>{bool} True if setting says so.</returns>
				bool get_vsync() const;

				/// <summary>
				/// <para>Switch between fullscreen and windowed mode.</para>
				/// </summary>
				/// <returns>{bool} Is it fullscreen now?</returns>
				bool toggle_fullscreen();

				/// <summary>
				/// <para>Sets is it should be fullscreen or not.</para>
				/// </summary>
				/// <param name="{bool}">Fullscreen?</param>
				void set_fullscreen(const bool);

				/// <summary>
				/// <para>Sets future or actual window name.</para>
				/// </summary>
				/// <param name="{std::string}">Title name.</param>
				void set_window_name(const std::string&);

				/// <summary>
				/// <para>Sets future or actual window icon.</para>
				/// </summary>
				/// <param name="{Bitmap}">Window icon.</param>
				void set_window_icon(const Bitmap&);

				/// <summary>
				/// <para>Hide the mouse in screen.</para>
				/// </summary>
				/// <param name="{bool}">Hide?</param>
				void hide_mouse(const bool);

				/// <summary>
				/// <para>Set the display to refresh the camera.</para>
				/// </summary>
				void set_refresh_camera();

				/// <summary>
				/// <para>Is the display up? (the thread can be running, but not with a screen yet).</para>
				/// </summary>
				/// <returns>{bool} Display existance.</returns>
				bool display_ready() const;

				/// <summary>
				/// <para>Get the fps cap.</para>
				/// </summary>
				/// <returns>{size_t} Current FPS cap.</returns>
				size_t get_fps_cap() const;

				/// <summary>
				/// <para>Adds a task to be done once only. Future gives you the result.</para>
				/// <para>The Future doesn't hold the exact type here, so you'll have to remember what your task returns.</para>
				/// </summary>
				/// <param name="{DisplayAnyFSPtr}">std::function that returns any type.</param>
				/// <returns>{Future} A Future that will hold the result of your function once done by drawing thread.</returns>
				Tools::Future<DisplayAnySPtr> add_once_task(DisplayAnyFSPtr);

				/// <summary>
				/// <para>Adds a task to be done every draw (exactly before flipping the screen).</para>
				/// <para>Useful for drawing your stuff indefinitely.</para>
				/// </summary>
				/// <param name="{std::function}">A function.</param>
				/// <returns>{size_t} The function ID in functions internal vector.</returns>
				size_t add_draw_task(DisplayTask);

				/// <summary>
				/// <para>Removes a task by ID (added via add_draw_task).</para>
				/// </summary>
				/// <param name="{size_t}">The task ID.</param>
				/// <returns>{std::function} The task function removed.</returns>
				DisplayTask remove_draw_task(const size_t);

				/// <summary>
				/// <para>Gets the display Event source.</para>
				/// </summary>
				/// <returns>{Event} The Event source.</returns>
				Event get_event_source() const;

				/// <summary>
				/// <para>Gets the frames per second (updates every second).</para>
				/// </summary>
				/// <returns>{size_t} Frames per second.</returns>
				size_t get_frames_per_second() const;

				/// <summary>
				/// <para>Shows how many errors of "out of range" happened (normally skipped, just debugging).</para>
				/// </summary>
				/// <returns>{size_t} Number of errors since init.</returns>
				size_t debug_errors_out_of_range_skip() const;

				/// <summary>
				/// <para>Shows how many errors of other sources happened (normally skipped).</para>
				/// </summary>
				/// <returns>{size_t} Number of errors since init.</returns>
				size_t debug_errors_unexpected() const;
			};

			
			bool operator==(ALLEGRO_DISPLAY*, const Display&);
			bool operator!=(ALLEGRO_DISPLAY*, const Display&);

		}
	}
}