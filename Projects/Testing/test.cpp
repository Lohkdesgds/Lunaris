#define LUNARIS_HEADER_ONLY

#include <Lunaris/utility.h>
#include <Lunaris/graphics.h>
#include <Lunaris/events.h>

using namespace Lunaris;

//#define TEST_PARALLEL
//#define TEST_FULLSCREEN
//#define LOCK_FPS_TO 144.0
#define SET_TPS_TO 20	// X / sec
#define SET_MOVE_DELAY 3 // X * sec
#define SET_MOVE_AMOUNT 0.007

const std::string random_img_url =  "https://picsum.photos/1024"; // "https://www.dropbox.com/s/nnl1tbypldv1un6/Photo_fur_2018.jpg?dl=1"; 


int main()
{
	display mywindow;
	//std::vector<hybrid_memory<texture>> mytextures;

	display_config mywindow_settings;
	mywindow_settings.extra_flags |= ALLEGRO_RESIZABLE;
	mywindow_settings.window_name = "My beautiful app";
#ifndef TEST_FULLSCREEN
	mywindow_settings.fullscreen = false;
	mywindow_settings.mode.width = 1600;
	mywindow_settings.mode.height = 900;
#else
	mywindow_settings.fullscreen = true;
#endif

	cout << console::color::GREEN << "Downloading random image from " << random_img_url;

	file fp;
	if (!fp.open_temp("lunaris_XXXXX.tmp", "wb+")) {
		cout << console::color::RED << "Failed to open temp file.";
		return 0;
	}

	downloader down;
	if (!down.get_store(random_img_url, [&](const char* buf, size_t len) { if (!fp.write(buf, len)) { cout << console::color::RED << "FATAL ERROR WRITING TO FILE! ABORT!"; std::terminate(); } })) {
		cout << console::color::RED << "Failed to download random image.";
		return 0;
	}

	cout << "File at " << fp.get_current_path();
	fp.flush();


	//fp.delete_and_close();

	auto random_texture = make_hybrid<texture>();
	if (!random_texture->load(fp.get_current_path())) {
		cout << console::color::YELLOW << "Failed to load random image.";
		//return 0;
	}

	

	if (!mywindow.create(mywindow_settings)) {
		cout << console::color::RED << "Failed to create window";
		return 0;
	}

	/*for (size_t p = 0; p < 5; p++) {
		auto textur = make_hybrid<texture>();
		if (!textur->load("bitmaps/" + std::to_string(p) + ".jpg")) {
			cout << console::color::RED << "Failed to load texture";
		}
		else mytextures.push_back(textur);
	}*/

	cout << console::color::GREEN << "Screen size: " << mywindow.get_width() << "x" << mywindow.get_height();

	auto font_u = make_hybrid<font>();
	if (!font_u->create_builtin_font()) {
		cout << console::color::RED << "Failed to create builtin font";
		return 0;
	}


	block blk;
	//for(auto& i : mytextures) blk.texture_insert(i);
	if (!random_texture->empty()) blk.texture_insert(random_texture);
	//blk.set<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX, true);
	//blk.set<bool>(enum_sprite_boolean_e::DRAW_THINK_BOX, true);
	//blk.set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, color(25,25,0,16));
	//blk.set<color>(enum_sprite_color_e::DRAW_THINK_BOX, color(25,0,0,18));

	blk.set<double>(enum_sprite_double_e::SCALE_G, 1.8);
	blk.set<double>(enum_block_double_e::DRAW_FRAMES_PER_SECOND, 2.0);
	//blk.set<double>(enum_sprite_double_e::OUT_OF_SIGHT_POS, 0.95);

	text txt;
	txt.font_set(font_u);
	txt.set<std::string>(enum_text_string_e::STRING, "Screen: ----x----\nFPS: ----\nTPS: --");
	txt.set<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE, true); // deform pos
	txt.set<double>(enum_sprite_double_e::SCALE_G, 0.06);
	txt.set<double>(enum_sprite_double_e::SCALE_X, 0.6);
	txt.set<double>(enum_sprite_double_e::POS_X, -0.992);
	txt.set<double>(enum_sprite_double_e::POS_Y, -0.992);
	for (int __c = 1; 255 - 25 * __c > 0; __c++) {
		int ctee = (255 - 25 * __c);
		txt.shadow_insert(text_shadow(0.001 * __c, 0.01 * __c, color(ctee/10, ctee/10, ctee/10, ctee)));
	}

	const bool& is_blk_in_sight = blk.get<bool>(enum_sprite_boolean_e::RO_DRAW_DRAWN_LAST_DRAW);
	const float zoom_now = 0.99f;


	transform transf;

	transf.build_classic_fixed_proportion(mywindow.get_width(), mywindow.get_height(), 1.0f, zoom_now);
	//transf.build_classic_fixed_proportion(mywindow.get_width(), mywindow.get_height());
	transf.apply();

	bool __sync = true;
	bool reapply_transf = false;

	ALLEGRO_EVENT_QUEUE* ev_qu = al_create_event_queue();
	al_register_event_source(ev_qu, mywindow.get_event_source());
	

	keyboard mykb;

	mykb.hook_each_key_event([&](keyboard& thus, const int key) {
		cout << console::color::GRAY << "Key pressed: #" << key;
	});
	mykb.hook_each_key_phrase_event([&](keyboard& thus, const std::string& currstr) {
		cout << console::color::GRAY << "Combined string so far: " << currstr;
	});
	mykb.hook_enter_line_phrase_event([&](keyboard& thus, const std::string& currstr) {
		cout << console::color::GRAY << "FINAL STRING: " << currstr;
		cout << console::color::GRAY << "Cleaning itself...";
		thus.clear();
	});


	keys keymngr;
	keymngr.hook_event([&](const keys::key_event& key) {
		cout << console::color::DARK_GRAY << (key.down ? "KEY_DOWN" : "KEY_UP") << ": " << key.key_id;
	});

	mouse mousectrl(mywindow.get_current_transform_function());
	mousectrl.hook_event([&](const int, const mouse::mouse_event& mous) {
		cout << console::color::DARK_GRAY << "MOUSE EVENT: { FIX: " << mous.real_posx << "x" << mous.real_posy << " REL: " << mous.relative_posx << "x" << mous.relative_posy << " } & "
			<< (mous.is_button_pressed(0) ? '1' : '0')
			<< (mous.is_button_pressed(1) ? '1' : '0')
			<< (mous.is_button_pressed(2) ? '1' : '0')
			<< (mous.is_button_pressed(3) ? '1' : '0')
			<< (mous.is_button_pressed(4) ? '1' : '0')
			<< (mous.is_button_pressed(5) ? '1' : '0')
			<< (mous.is_button_pressed(6) ? '1' : '0')
			<< (mous.is_button_pressed(7) ? '1' : '0')
			<< (mous.is_button_pressed(8) ? '1' : '0')
			<< " W=" << mous.scroll_event_id(0) << " Z=" << mous.scroll_event_id(1);
	});

	/*generic_event_handler generic_event;
	//generic_event.install_joystick();
	generic_event.install_keyboard();
	generic_event.install_mouse();
	//generic_event.install_touch();
	generic_event.install_other(mywindow.get_event_source());

	generic_event.hook_event_handler([&](const ALLEGRO_EVENT& ev) {
		cout << console::color::DARK_PURPLE << "RAW_GENERIC_EVENT#" << ev.type << " DETECTED!";
	});*/

	size_t fps_counting = 0;
	size_t tps_counting = 0;

	mywindow.hook_event_handler([&](const ALLEGRO_EVENT& ev) {

		cout << console::color::AQUA << "DISPLAY EVENT: " << console::color::BLUE << "Event #" << ev.type << " triggered.";

		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			cout << console::color::GREEN << "Display CLOSE called.";
			__sync = false;
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			cout << console::color::GREEN << "Screen size is now: " << ev.display.width << "x" << ev.display.height;
			transf.build_classic_fixed_proportion(ev.display.width, ev.display.height, 1.0f, zoom_now);
			reapply_transf = true;
		}
	});

	/*std::thread disp_upt_thr([&] {
		cout << "Event listening!";
		while (__sync) {
			ALLEGRO_EVENT ev;
			al_wait_for_event(ev_qu, &ev);

			
		}
	});*/

	std::thread test_accel([&] {
		cout << "Accelerating!";

		bool positive = true;
		bool first = true;

		while (__sync) {
			auto point_in_time = std::chrono::steady_clock::now() + std::chrono::microseconds(static_cast<long long>(1000000 * SET_MOVE_DELAY * (first ? 0.5 : 1.0)));

			first = false;

			blk.set<double>(enum_sprite_double_e::ACCEL_X, SET_MOVE_AMOUNT * (positive ? 1.0 : -1.0));
			//blk.set<double>(enum_sprite_double_e::ACCEL_Y, SET_MOVE_AMOUNT * (positive ? 1.0 : -1.0));

			positive = !positive;

			std::this_thread::sleep_until(point_in_time);
		}

	});

	std::thread thinker([&] {
		cout << "Ticking!";
		while (__sync) {
			auto point_in_time = std::chrono::steady_clock::now() + std::chrono::microseconds(1000000 / SET_TPS_TO);

			tps_counting++;
			
			blk.think();
			txt.think();

			//blk.set<double>(enum_sprite_double_e::POS_X, 0.6 * cos(0.576 * al_get_time()));
			//blk.set<double>(enum_sprite_double_e::POS_Y, 0.6 * sin(0.224 * al_get_time()));

			//txt.set<double>(enum_sprite_double_e::DRAW_RELATIVE_CENTER_X, cos(al_get_time()));
			//txt.set<double>(enum_sprite_double_e::DRAW_RELATIVE_CENTER_Y, cos(al_get_time()));

			std::this_thread::sleep_until(point_in_time);
			//cout << "Tick!";
		}
	});

	std::thread counterr_to_scr([&] {
		cout << "Debug text working!";
		while (__sync) {
			auto point_in_time = std::chrono::steady_clock::now() + std::chrono::seconds(1);

			size_t __tmp_fps = fps_counting;
			fps_counting = 0;
			size_t __tmp_tps = tps_counting;
			tps_counting = 0;

			txt.set<std::string>(enum_text_string_e::STRING,
				(	"Screen: " + std::to_string(mywindow.get_width()) + "x" + std::to_string(mywindow.get_height()) + "@" + std::to_string(mywindow.get_frequency()) + "\n"
					"FPS: " + std::to_string(__tmp_fps) + "\n"
					"TPS: " + std::to_string(__tmp_tps) + 
					(is_blk_in_sight ? "" : "\nDETECTED: OUT_OF_SIGHT_BLK") +
					(mywindow.get_is_economy_mode_activated() ? "\nECONOMY MODE" : "")
					)
			);

			std::this_thread::sleep_until(point_in_time);
		}
	});



	while (__sync) {
#ifdef LOCK_FPS_TO
		auto point_in_time = std::chrono::steady_clock::now() + std::chrono::microseconds(static_cast<unsigned>(1000000.0 / (LOCK_FPS_TO * 1.088)));
#endif

		fps_counting++;

		//transform cpy = transf;
		//cpy.translate(mywindow.get_width() * cos(al_get_time() * 0.517f) * 0.5f, mywindow.get_height() * sin(al_get_time() * 0.673f) * 0.5f);
		//cpy.invert();
		//cpy.translate_inverse(cos(al_get_time() * 0.517f), sin(al_get_time() * 0.673f));
		//cpy.rotate_inverse(pow((1.0 + cos(al_get_time() * 0.41)) * 73.442, 0.4));
		//cpy.invert();
		//cpy.apply();


		if (reapply_transf) {
			transf.apply();
			reapply_transf = false;
		}

		al_clear_to_color(color(
			static_cast<unsigned char>(cos(1.5 * al_get_time()) * 90 + 128),
			static_cast<unsigned char>(cos(0.42 * al_get_time()) * 90 + 128),
			static_cast<unsigned char>(cos(2.3 * al_get_time()) * 90 + 128) // 90 was 127
		));

		
		blk.draw();
		txt.draw();

		// This draws borders (1:1 and fullscreen)
		/*{
			transform tf;
			tf.build_classic_fixed_proportion(mywindow.get_width(), mywindow.get_height(), 1.0f, zoom_now);
			tf.apply();
			al_draw_rectangle(-0.99, -0.99, 0.99, 0.99, al_map_rgba(127, 127, 0, 100), 0.03);

			tf.build_classic_fixed_proportion(mywindow.get_width(), mywindow.get_height(), (1.0f * mywindow.get_width() / mywindow.get_height()), zoom_now);
			tf.apply();
			al_draw_rectangle(-0.99, -0.99, 0.99, 0.99, al_map_rgba(127, 0, 0, 100), 0.03);
		}*/

		mywindow.flip();
#ifdef LOCK_FPS_TO
		std::this_thread::sleep_until(point_in_time);
#endif
	}

	__sync = false;

	thinker.join();
	counterr_to_scr.join();
	//disp_upt_thr.join();
	test_accel.join();

	return 0;
}

/*
 
int main()
{	
	display mywindow;

	display_config mywindow_settings;
	mywindow_settings.extra_flags |= ALLEGRO_RESIZABLE;
	mywindow_settings.fullscreen = false;
	mywindow_settings.window_name = "My beautiful app";
	mywindow_settings.mode.width = 720;
	mywindow_settings.mode.height = 480;

	if (!mywindow.create(mywindow_settings)) {
		cout << console::color::RED << "Failed to create window";
		return 0;
	}

	bool textures_ready = false;
	bool failed_to_load = false;
	size_t bitmaps_loaded = 0;
	float bitmaps_loaded_fluid = 0.0f;
	texture images[5];

	//system("dir");

#ifdef TEST_PARALLEL
	std::thread parallel([&] {
		cout << console::color::YELLOW << "Loading bitmaps in parallel...";
#endif
		for (size_t nam = 0; nam < 5; nam++)
		{
			texture_config conf;
			conf.path = "./bitmaps/" + std::to_string(nam) + ".jpg";

			if (!images[nam].load(conf)) {
				cout << console::color::RED << "Failed to load " + conf.path;
				failed_to_load = true;
#ifdef TEST_PARALLEL
				return;
#else
				return 0;
#endif
			}
			else cout << console::color::GREEN << "Loaded " + conf.path;
			bitmaps_loaded = nam + 1;
		}
#ifdef TEST_PARALLEL
		cout << console::color::GREEN << "Done loading bitmaps in parallel!";
		textures_ready = true;
		return;
	});
#endif

	size_t frames_count = 0;
	const size_t interval = 60;


#ifdef TEST_PARALLEL
	{
		double prop = al_get_time();
		while (!textures_ready || bitmaps_loaded_fluid < 4.3f) {
			if (failed_to_load) {
				cout << console::color::RED << "Failed.";
				parallel.join();
				return 0;
			}

			ALLEGRO_COLOR color = al_map_rgb(
				cos(1.5 * al_get_time()) * 47 + 108 + ((bitmaps_loaded_fluid / 5.0f) * 100.0f),
				cos(0.42 * al_get_time()) * 27 + 48 + ((bitmaps_loaded_fluid / 5.0f) * 180.0f),
				cos(2.3 * al_get_time()) * 27 + 28 + ((bitmaps_loaded_fluid / 5.0f) * 200.0f)
			);

			float diff = static_cast<float>(0.5 / (al_get_time() - prop));
			prop = al_get_time();

			bitmaps_loaded_fluid = ((diff - 1.0f) * bitmaps_loaded_fluid + bitmaps_loaded) / diff;

			al_clear_to_color(color);
			mywindow.flip();
		}
	}

	parallel.join();
#endif

	transform transf;

	unsigned last_sec_i = 0;
	float exp_x = 1.0f, exp_y = 1.0f;

	for (auto now = std::chrono::system_clock::now(); std::chrono::system_clock::now() - now < std::chrono::seconds(interval);)
	{
		if (static_cast<unsigned>(al_get_time()) > last_sec_i + 1) {
			last_sec_i = static_cast<unsigned>(al_get_time());
			mywindow.acknowledge_resize();
			transf.build_classic_fixed_proportion(mywindow.get_width(), mywindow.get_height(), 1.0f);
			transf.apply();

			transform cpy = transf;
			cpy.invert();

			exp_x = exp_y = -1.0f;

			cpy.transform_coords(exp_x, exp_y);
			exp_x = fabs(exp_x);
			exp_y = fabs(exp_y);

			//resiz_calc(transf, mywindow);
			cout << console::color::DARK_GRAY << "Updated scale once [expected limits: " << exp_x << ";" << exp_y << "]";
		}

		ALLEGRO_COLOR color = al_map_rgb(
			cos(1.5 * al_get_time()) * 90 + 128,
			cos(0.42 * al_get_time()) * 90 + 128,
			cos(2.3 * al_get_time()) * 90 + 128 // 90 was 127
		);

		al_clear_to_color(color);

		al_draw_rectangle(-exp_x, -exp_y, exp_x, exp_y, Lunaris::color(1.0f, 1.0f, 0.0f), 0.08f);
		al_draw_rectangle(-0.98f, -0.98f, 0.98f, 0.98f, Lunaris::color(1.0f, 0.0f, 0.0f), 0.06f);

		size_t offset = static_cast<size_t>(al_get_time());

		for (unsigned off = 0; off < 5; off++) {
			const size_t index = (offset + off) % 5;

			const int biggest = images[index].get_height() > images[index].get_width() ? images[index].get_height() : images[index].get_width();

			images[index].draw_scaled_rotated_at(
				0.0f, 0.0f,
				0.0f, 0.0f, //mywindow.get_width() / 2.0f, mywindow.get_height() / 2.0f,
				1.7f / biggest, 1.7f / biggest,
				//0.7f / images[frame_count].get_width(), 0.7f / images[frame_count].get_height(),
				al_get_time() * 0.1 + (off * 0.015f));
		}

		frames_count++;
		mywindow.flip();
	}


	cout << "The end! FPS: " << console::color::YELLOW << (1.0 * frames_count / interval);

	mywindow.destroy();

	std::this_thread::sleep_for(std::chrono::seconds(20));

	return 0;
}*/