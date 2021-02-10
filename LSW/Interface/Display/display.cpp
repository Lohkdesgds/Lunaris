#include "display.h"


namespace LSW {
	namespace v5 {
		namespace Interface {
			
			Tools::SuperMutex Display::sync_threads;
			
			bool Display::thread_run(Tools::boolThreadF keep)
			{
				bool verynice = true;
				bool redraw = false;
				bool regen_disp_pls = false;
				bool check_dbuf = false;

				Camera fixed_default_camera;
				fixed_default_camera.identity();
				Color black(0.0f, 0.0f, 0.0f, 1.0f);


				bool was_vsync = should_vsync;
				size_t frames = 0;

				EventTimer timar(1.0);
				EventTimer fpsc(1.0);
				EventTimer doublebuffer_check(2.0);

				EventHandler handler;
				handler.add(timar);
				handler.add(fpsc);
				handler.add(doublebuffer_check);

				timar.start();
				if (fps_cap) {
					fpsc.set_delta(1.0 / fps_cap);
					fpsc.start();
				}
				doublebuffer_check.start();

				// always timer event lmao, refreshes once per sec
				handler.set_run_autostart([&](const RawEvent& raw) {

					if (raw.timer_event().source == timar) {
						frames_per_second = frames;
						frames = 0;

						// check vsync
						if (was_vsync != should_vsync) {
							was_vsync = should_vsync;
							if (force_vsync_refresh) regen_disp_pls = true;
						}

						// check fps cap
						if (fps_cap) {
							if (fps_cap > display::max_fps_cap) fps_cap = display::max_fps_cap;

							if (fpsc.get_delta() != 1.0 / fps_cap) {
								fpsc.set_delta(1.0 / fps_cap);
							}
							if (!fpsc.running()) {
								fpsc.start();
							}
						}
						else {
							if (fpsc.running()) fpsc.stop();
							redraw = true; // just in case
						}
					}
					else if (raw.timer_event().source == fpsc) {
						redraw = true;
					}
					else if (raw.timer_event().source == doublebuffer_check)
					{
						check_dbuf = true;
					}
				});

				try {
					thread_init();

					if (camera_fu) {
						std::lock_guard<std::mutex> luck(camfu_m);
						cam_latest = camera_fu();
					}

					targ.set([&] { Bitmap b; b.force(disp ? al_get_backbuffer(disp.get()) : nullptr); return b; });

					while (keep()) {

						if (fps_cap) {
							while (!redraw && fps_cap && keep()) { std::this_thread::yield(); std::this_thread::sleep_for(std::chrono::milliseconds(1)); } // up to 500 fps good cap, 1000 limit
							redraw = false;
						}
						if (regen_disp_pls) {
							regen_disp_pls = false;
							_reset_display_and_path();
							refresh_camera = true;
						}
						if (check_dbuf)
						{
							check_dbuf = false;
							if (double_buffering <= 0.0)
							{
								targ.set([&] { Bitmap b; b.force(disp ? al_get_backbuffer(disp.get()) : nullptr); return b; });
								dbuffer.reset();
								refresh_camera = true;
							}
							else {
								Bitmap ref;
								ref.force(al_get_backbuffer(disp.get()));
								refresh_camera |= 2 == dbuffer.copy_attributes(ref, false, double_buffering);
								targ.set(dbuffer);
							}

						}

						frames++;

						if (refresh_camera) {
							std::lock_guard<std::mutex> cam_m_luck(camfu_m);
							if (refresh_camera) {
								if (camera_fu) {
									std::lock_guard<std::mutex> luck(camfu_m);
									cam_latest = camera_fu();
								}
								if (refresh_camera) refresh_camera = !cam_latest.classic_update(targ.get());
								//refresh_camera = false;
								cam_latest.classic_refresh();

								Bitmap::check_bitmaps_memory();
							}
						}

						if (once_tasks.size()) {
							Tools::AutoLock luck(once_tasks_m);
							for (auto& i : once_tasks) i.work(); //i.set_value(10);
							once_tasks.clear();
						}

						targ.apply(); // apply(), get() -> Bitmap, set(const Bitmap&), 

						/*draw_tasks_m.lock();
						for (auto& i : draw_tasks) i.second();
						draw_tasks_m.unlock();*/

						cam_latest.apply();

						black.clear_to_this();

						try {
							for (size_t p = 0; p < draw_tasks.size(); p++) draw_tasks.at(p).second(cam_latest);
						}
#ifdef _DEBUG
						catch (const std::out_of_range& e) { // I know sometimes it WILL be out of range if you change the draw_tasks size, but mutex costs A LOT (from 5000+ fps to ~300 fps), so just ignore out of range...
							std::cout << "__INTERNAL__ __SKIP__ OUT OF RANGE ERROR AT DRAW_TASKS: " << e.what() << std::endl;
#else
						catch (const std::out_of_range&) {
#endif
							fails_out_of_range++;
						}
						catch (Handling::Abort a) {
#ifdef _DEBUG
							std::cout << "__INTERNAL__ __SKIP__ ERROR AT DRAW_TASKS: " << a.what() << std::endl;
#endif
							fails_unexpected++;
						}
						catch (...) { // probably can skip (later: add counter by time)
#ifdef _DEBUG
							std::cout << "__INTERNAL__ __SKIP__ UNKNOWN ERROR AT DRAW_TASKS" << std::endl;
#endif
							fails_unexpected++;
						}

						if (!dbuffer.empty()) {
							al_set_target_backbuffer(disp.get());
							fixed_default_camera.apply();
							dbuffer.draw(0, 0, get_width(), get_height());
							al_flip_display();
							targ.apply();
						}
						else {
							al_set_target_backbuffer(disp.get());
							al_flip_display();
						}

					}
				}
				catch (...) { // enhance later
					verynice = false;
				}

				if (once_tasks_m.is_locked()) once_tasks_m.unlock(); // this is my safer mutex no worries.

				thread_deinit();

				handler.stop();
				timar.stop();

				return verynice;
			}
						
			void Display::_reset_display_and_path()
			{
				Tools::AutoLock sync(sync_threads);

				is_fullscreen |= static_cast<bool>(new_display_flags_apply & ALLEGRO_FULLSCREEN);
				is_fullscreen |= static_cast<bool>(new_display_flags_apply & ALLEGRO_FULLSCREEN_WINDOW);
				new_display_flags_apply &= ALLEGRO_FULLSCREEN;
				new_display_flags_apply |= display::default_new_display_flags;
				if (is_fullscreen) new_display_flags_apply |= ALLEGRO_FULLSCREEN_WINDOW;

				al_set_new_display_flags(new_display_flags_apply);
				if (new_display_refresh_rate > 0) al_set_new_display_refresh_rate(new_display_refresh_rate);
				al_set_new_display_option(ALLEGRO_VSYNC, should_vsync ? 1 : 2, ALLEGRO_SUGGEST);
				// prettier drawings (anti aliasing on primitives)
				al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, ALLEGRO_SUGGEST, 2);
				al_set_new_display_option(ALLEGRO_SAMPLES, ALLEGRO_SUGGEST, 8);
				// endof prettier drawings

				if (display_events.running()) {
					display_events.stop();
				}
				if (disp) {
					disp.reset();
				}

				disp = DisplayStrongPtr(al_create_display(new_resolution[0], new_resolution[1]), [](ALLEGRO_DISPLAY* d) { al_destroy_display(d); d = nullptr; });
				if (!disp.get()) {
					Logger logg;
					logg << L::SLF << fsr(E::ERRR) << "Cannot create display with these settings/environment!" << L::ELF;
					debug("Cannot create display with these settings/environment!");
					logg.flush();
					std::terminate();
				}

				al_convert_bitmaps();

				al_apply_window_constraints(disp.get(), true);
				al_set_window_constraints(disp.get(), display::minimum_display_size[0], display::minimum_display_size[1], 0, 0); // minimum size 640,480, max not defined

				{
					targ.set([&] { Bitmap b; b.force(disp ? al_get_backbuffer(disp.get()) : nullptr); return b; });
				}
				al_inhibit_screensaver(true);

				if (window_name.length()) al_set_window_title(disp.get(), window_name.c_str());
				if (icon) al_set_display_icon(disp.get(), icon);

				if (hide_mouse_new) al_hide_mouse_cursor(disp.get());
				else				al_show_mouse_cursor(disp.get());


				display_events.add(Event(al_get_display_event_source(disp.get())));

				display_events.set_run_autostart([&](const RawEvent& ev) {

					if (ev.type() == ALLEGRO_EVENT_DISPLAY_RESIZE && ev.display_event().source == disp.get()) {
						al_acknowledge_resize(disp.get());
						refresh_camera = true;
					}
				});
			}
			
			void Display::thread_init()
			{
				fails_out_of_range = fails_unexpected = 0;

				_reset_display_and_path();

				refresh_camera = true;
			}
			
			void Display::thread_deinit()
			{
				display_events.stop();
				if (disp) {
					disp.reset();
					disp = nullptr;
				}
			}
						
			Display::Display(const size_t nind) : targ(nind)
			{
				Handling::init_basic();
				Handling::init_graphics();
				Handling::init_font();
				cam_latest.classic_transform(0.0, 0.0, 1.0, 1.0, 0.0);
			}
						
			Display::~Display()
			{
				deinit();
			}
						
			Tools::Future<bool> Display::init()
			{
				thr.join();
				thr.set([&](Tools::boolThreadF f) {return thread_run(f); });
				return std::move(thr.start());
			}
						
			bool Display::operator==(ALLEGRO_DISPLAY* ptr) const 
			{
				return disp ? (disp.get() == ptr) : false;
			}
						
			void Display::deinit()
			{
				thr.stop();
				thr.join();
			}
						
			bool Display::move_mouse_to(const double x, const double y)
			{
				if (disp) {
					return al_set_mouse_xy(disp.get(), get_width() * (0.5 * (x + 1.0)), get_height() * (0.5 * (y + 1.0)));
				}
				return false;
			}

			void Display::set_stop()
			{
				thr.stop();
			}
						
			void Display::set_double_buffering_scale(const double bufs)
			{
				double_buffering = bufs > 8.0 ? 8.0 : (bufs < 0.0 ? 0.0 : bufs);
			}
						
			double Display::get_current_buffering_scale() const
			{
				return double_buffering;
			}
						
			void Display::set_camera(const Camera& cam)
			{
				std::lock_guard<std::mutex> luck(camfu_m);
				camera_fu = std::function<Camera(void)>();
				refresh_camera = true;
			}

			void Display::set_camera(std::function<Camera(void)> f)
			{
				std::lock_guard<std::mutex> luck(camfu_m);
				camera_fu = f;
				refresh_camera = true;
			}
						
			void Display::set_fps_cap(const size_t cap)
			{
				fps_cap = cap;
			}
			
			Camera& Display::get_current_camera()
			{
				if (camera_fu) throw Handling::Abort(__FUNCSIG__, "Camera is set as a FUNCTION. You can't reference this! Not secure!");
				return cam_latest;
			}
						
			bool Display::running() const
			{
				return disp.operator bool();
			}
						
			void Display::set_new_flags(const int flags)
			{
				new_display_flags_apply = flags;
				if (!disp) is_fullscreen = (new_display_flags_apply & ALLEGRO_FULLSCREEN || new_display_flags_apply & ALLEGRO_FULLSCREEN_WINDOW);
			}
						
			int Display::get_flags() const
			{
				return disp.get() ? al_get_display_flags(disp.get()) : new_display_flags_apply;
			}
						
			const int Display::get_width() const
			{
				return disp.get() ? al_get_display_width(disp.get()) : new_resolution[0];
			}
						
			const int Display::get_height() const
			{
				return disp.get() ? al_get_display_height(disp.get()) : new_resolution[1];
			}
						
			void Display::set_width(const int xx)
			{
				if (xx >= display::minimum_display_size[0]) {
					new_resolution[0] = xx;
					if (disp) {
						add_once_task([&] { return al_resize_display(disp.get(), new_resolution[0], get_height()); });
					}
				}
			}
						
			void Display::set_height(const int yy)
			{
				if (yy >= display::minimum_display_size[1]) {
					new_resolution[1] = yy;
					if (disp) {
						add_once_task([&] { return al_resize_display(disp.get(), get_width(), new_resolution[1]); });
					}
				}
			}
						
			void Display::set_new_refresh_rate(const int hz)
			{
				new_display_refresh_rate = hz;
			}
						
			int Display::get_refresh_rate() const
			{
				return disp.get() ? al_get_display_refresh_rate(disp.get()) : new_display_refresh_rate;
			}
						
			void Display::set_vsync(const bool vs, const bool force)
			{
				should_vsync = vs;
				force_vsync_refresh = force;
			}
						
			bool Display::get_vsync() const
			{
				return should_vsync;
			}
						
			bool Display::toggle_fullscreen()
			{
				is_fullscreen = !is_fullscreen;
				if (disp) al_toggle_display_flag(disp.get(), ALLEGRO_FULLSCREEN_WINDOW, is_fullscreen);
				return is_fullscreen;
			}
						
			void Display::set_fullscreen(const bool fullscr)
			{
				is_fullscreen = fullscr;
				if (disp) al_toggle_display_flag(disp.get(), ALLEGRO_FULLSCREEN_WINDOW, is_fullscreen);
			}

			void Display::set_window_name(const std::string& wn)
			{
				window_name = wn;
				if (disp) {
					al_set_window_title(disp.get(), window_name.c_str());
				}
			}

			void Display::set_window_icon(const Bitmap& ic)
			{
				add_once_task([&, ic] {
					icon.clone(ic);
					if (disp && icon) {
						al_set_display_icon(disp.get(), icon);
					}
					return 0;
				});
			}
						
			void Display::hide_mouse(const bool hid)
			{
				if (disp) {
					if (hid) al_hide_mouse_cursor(disp.get());
					else al_show_mouse_cursor(disp.get());
				}
				hide_mouse_new = hid;
			}
						
			void Display::set_refresh_camera()
			{
				refresh_camera = true;
			}
						
			bool Display::display_ready() const
			{
				return disp.get();
			}
						
			size_t Display::get_fps_cap() const
			{
				return fps_cap;
			}
						
			Tools::Future<DisplayAnySPtr> Display::add_once_task(DisplayAnyFSPtr tsk)
			{
				Tools::Promise<DisplayAnySPtr> my_future(tsk);
				Tools::Future<DisplayAnySPtr> future = std::move(my_future.get_future());

				Tools::AutoLock luck(once_tasks_m);

				once_tasks.emplace_back(std::move(my_future));

				return std::move(future);
			}
						
			size_t Display::add_draw_task(DisplayTask tsk)
			{
				Tools::AutoLock luck(draw_tasks_m);

				draw_tasks.push_back({ ++draw_tasks_count, std::move(tsk) });
				size_t this_task = draw_tasks_count;

				return this_task;
			}
						
			DisplayTask Display::remove_draw_task(const size_t tskid)
			{
				DisplayTask task;
				Tools::AutoLock luck(draw_tasks_m);

				for (size_t p = 0; p < draw_tasks.size(); p++) {
					if (draw_tasks[p].first == tskid) {
						task = draw_tasks[p].second;
						draw_tasks.erase(draw_tasks.begin() + p);
						return task;
					}
				}
				return task;
			}
						
			Event Display::get_event_source() const
			{
				if (disp) {
					return Event(al_get_display_event_source(disp.get()));
				}
				return Event();
			}
						
			size_t Display::get_frames_per_second() const
			{
				return frames_per_second;
			}
						
			size_t Display::debug_errors_out_of_range_skip() const
			{
				return fails_out_of_range;
			}
						
			size_t Display::debug_errors_unexpected() const
			{
				return fails_unexpected;
			}
						
			bool operator==(ALLEGRO_DISPLAY* u, const Display& d)
			{
				return d.operator==(u);
			}
			
			bool operator!=(ALLEGRO_DISPLAY* u, const Display& d)
			{
				return !d.operator==(u);
			}

			// implementation

			template Tools::Future<bool>;
			template Tools::Future<DisplayAnySPtr>;

		}
	}
}