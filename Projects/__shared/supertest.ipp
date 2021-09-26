#pragma once

#include <iterator>

using namespace Lunaris;

//#define RUN_AND_TEST(FUNC, ERRMSG) { \
//if (!FUNC) { \
//  cout << console::color::RED << ERRMSG; return 1; \
//}}

#define TESTLU(X, ERRMSG) if (run_and_test([&]{return X;}, ERRMSG) != 0) return 1;

const std::string fixed_audio_src_url = "https://cdn.discordapp.com/attachments/888270629990707331/888270836677607425/music_01.ogg";
const std::string fixed_image_src_url = "https://media.discordapp.net/attachments/888270629990707331/888272596720844850/3.jpg?width=918&height=612";
const std::string fixed_image_src_url_sha256_precalc = "e02e1baec55f8e37b5a5b3ef29d403fc2a2084267f05dfc75d723536081aff10";
const std::string temp_local_file_path = "lunaris_temp_local.tmp";
const std::string random_img_url =  "https://picsum.photos/1024"; // "https://www.dropbox.com/s/nnl1tbypldv1un6/Photo_fur_2018.jpg?dl=1"; 

constexpr size_t num_of_entities_in_package_test = 1000;


void hold_user();
int run_and_test(std::function<bool(void)>, const std::string&);

int utility_test(const std::string&); // GOOD 100%
int audio_test(); // GOOD 100%
int events_test(); // GOOD (enough)
int graphics_test(); // TODO

int main(int argc, char* argv[]) {
	TESTLU(argc >= 1, "IRREGULAR STARTUP! Can't proceed.");
	const std::string currpath = argv[0];

	if (argc > 1) {
		cout << "Hello someone calling me with custom arguments! I received those:";
		for (int a = 0; a < argc; a++) cout << "Argument #" << a << ": '" << argv[a] << "'";
		cout << "Have a great day! Exiting the app...";
		return 0;
	}

	//if (utility_test(currpath) != 0) return 1;
	//if (audio_test() != 0) return 1;
	//if (events_test() != 0) return 1;
	if (graphics_test() != 0) return 1; // todo
}

void hold_user()
{
	while (1) std::this_thread::sleep_for(std::chrono::seconds(9999));
}

int run_and_test(std::function<bool(void)> f, const std::string& str)
{
	if (!f()) {
		cout << console::color::RED << str;
		//hold_user();
		return 1;
	}
	return 0;
}

int utility_test(const std::string& self_path)
{
	cout << console::color::DARK_BLUE << "======================================";
	cout << console::color::BLUE << "# Starting utility_test...";
	cout << console::color::DARK_BLUE << "======================================";

	cout << console::color::GREEN << "Console is probably working (you can see me, right?)";

	cout << console::color::LIGHT_PURPLE << "Testing 'downloader'...";
	{
		cout << "Downloading a file...";

		downloader down;
		down.get(fixed_image_src_url);

		cout << "Downloaded " << console::color::AQUA << down.bytes_read() << console::color::GRAY << " byte(s)";
		TESTLU((down.bytes_read() > 0), "Zero bytes is a issue.");
		
		cout << console::color::GREEN << "PASSED!";

		cout << console::color::LIGHT_PURPLE << "Testing 'hash'...";

		cout << "Testing hash on this file...";
	
		const auto _res = sha256(down.read());

		cout << "SHA256 of this file: " << console::color::AQUA << _res;
		TESTLU(_res == fixed_image_src_url_sha256_precalc, "Hash don't match expected? Bad thing!");

		cout << console::color::GREEN << "PASSED!";

		cout << console::color::LIGHT_PURPLE << "Testing 'file'...";
		cout << "Creating temporary local file...";

		file fp;
		TESTLU(fp.open(temp_local_file_path, "wb"), "Failed to create a local file.");

		cout << console::color::GREEN << "PASSED!";

		cout << "Writing data to that temporary file...";

		TESTLU(fp.write(down.read().data(), down.read().size()), "Couldn't write all data into temporary file.");
		TESTLU(fp.flush(), "Couldn't flush temporary file after write.");
		fp.close();

		cout << console::color::GREEN << "PASSED!";

		cout << "Reopening, reading and checking...";

		TESTLU(fp.open(temp_local_file_path, "rb"), "Failed to open the local file.");

		{
			std::vector<char> _tmpvec;
			char buftemp[256];
			while (1) {
				size_t ree = fp.read(buftemp, 256);
				if (ree > 256) {
					cout << console::color::RED << "FATAL ERROR! Read more than asked?!";
					return 1;
				}
				else if (ree > 0) _tmpvec.insert(_tmpvec.end(), std::begin(buftemp), std::begin(buftemp) + ree);
				else break;
			}

			cout << console::color::GREEN << "PASSED!";

			cout << "Comparing data read from file...";

			const auto _res2 = sha256(_tmpvec);

			cout << "SHA256 of this file: " << console::color::AQUA << _res2;
			TESTLU(_res2 == _res, "Hash don't match expected? Bad thing!");

			cout << console::color::GREEN << "PASSED!";

			fp.delete_and_close();
		}
	}

	{
		cout << console::color::LIGHT_PURPLE << "Testing 'socket' (TCP)...";

		{
			cout << "Trying to host and connect to itself...";

			TCP_client tcp_client, tcp_host_client;
			TCP_host tcp_host;


			TESTLU(tcp_host.setup(socket_config().set_family(socket_config::e_family::IPV4).set_port(12345)), "Can't setup host!");

			cout << console::color::LIGHT_PURPLE << "Testing 'thread'...";

			cout << "Setting up simple thread to listen() host's port...";

			thread tcp_thread([&] {
				//while(!tcp_host_client.has_socket())
				tcp_host_client = tcp_host.listen();
			}, thread::speed::ONCE);

			cout << "Trying to connect to host...";

			TESTLU(tcp_client.setup(socket_config().set_family(socket_config::e_family::IPV4).set_port(12345)), "Can't connect to itself!");

			cout << "Connected. Syncing threads...";

			while (!tcp_host_client.has_socket()) std::this_thread::sleep_for(std::chrono::milliseconds(50));

			tcp_thread.join(); // should join easy

			TESTLU(tcp_host_client.has_socket(), "Didn't connect properly! (host)");

			cout << console::color::GREEN << "PASSED!";

			cout << console::color::LIGHT_PURPLE << "Testing 'random'...";
			cout << "Testing random numbers...";
			
			{
				std::string transl_list;
				for (unsigned a = 0; a < 10; a++) transl_list += std::to_string(random() % 100) + ", ";
				if (transl_list.size()) transl_list.erase(transl_list.end() - 2, transl_list.end() - 1);

				cout << console::color::YELLOW << "User, does this sequence look random to you?: [ " << console::color::AQUA << transl_list << console::color::YELLOW << "]";
				cout << console::color::YELLOW << "If so, wait 5 seconds. Else please quit and report.";

				std::this_thread::sleep_for(std::chrono::seconds(5));

				cout << console::color::YELLOW << "PASSED! (timedout user, so assuming good)";
			}

			cout << console::color::LIGHT_PURPLE << "Testing 'package'...";
			cout << "Generating random package filled with random numbers...";

			package pack_sending;
			for (size_t p = 0; p < num_of_entities_in_package_test; p++) pack_sending << random();

			cout << "Setting up receiving thread...";

			const size_t expected_size = pack_sending.read_as_array().size();
			package pack_received;

			tcp_thread.task_async([&] {
				pack_received = tcp_client.recv(expected_size);
			}, thread::speed::ONCE);


			cout << "Sending package HOST -> CLIENT...";

			tcp_host_client.send(pack_sending);

			cout << "Sended. Syncing receiver thread...";

			tcp_thread.join();

			cout << "Good. Testing data...";

			pack_sending.reset_internal_iterator();

			for (size_t p = 0; p < num_of_entities_in_package_test; p++) 
			{
				unsigned long long a, b;
				pack_sending >> a;
				pack_received >> b;

				TESTLU(a == b, "Transfer don't match!");
			}

			cout << console::color::GREEN << "PASSED!";
		}

		cout << console::color::LIGHT_PURPLE << "Testing 'socket' (UDP)...";

		{
			UDP_host udp_host;
			UDP_client udp_client;

			TESTLU(udp_host.setup(socket_config().set_family(socket_config::e_family::IPV4).set_port(9898)), "Could not setup UDP host.");
			TESTLU(udp_client.setup(socket_config().set_family(socket_config::e_family::IPV4).set_port(9898)), "Could not setup UDP client.");

			cout << "UDP stuff looks ok. Trying to send/recv one to another...";


			thread echothread([&] {
				auto handler = udp_host.recv(sizeof(unsigned long long), 1);
				if (handler.valid()) 
					handler.send(handler.get());
			}, thread::speed::HIGH_PERFORMANCE);

			for (size_t p = 0; p < num_of_entities_in_package_test; p++)
			{
				package sending_pack;

				sending_pack << random();
				udp_client.send(sending_pack);

				package recvd_pack = udp_client.recv(sizeof(unsigned long long));

				TESTLU(recvd_pack == sending_pack, "Transfer don't match!");
			}

			cout << console::color::GREEN << "PASSED!";

			cout << "Syncing threads...";
		}

		cout << console::color::LIGHT_PURPLE << "Testing 'future'...";

		{
			promise<unsigned long long>* my_promise = new promise<unsigned long long>();

			future<unsigned long long> long_future = my_promise->get_future();

			unsigned long long nxt_here = 0;
			bool all_good = true;

			for (size_t opts = 0; opts < 10; opts++) {
				long_future = long_future.then([opts,&nxt_here,&all_good](const unsigned long long& val) {
					const auto nxt = random();
					cout << "- @ position: " << opts << ", got random value: " << ((nxt_here == val) ? console::color::AQUA : console::color::RED) << val << console::color::GRAY << ".";
					if (nxt_here != val) {
						cout << console::color::RED << "Values didn't match once!";
						all_good = false;
					}
					cout << "                   Next should be: " << nxt;
					nxt_here = nxt;
					return nxt;
				});
			}

			nxt_here = random();
			cout << "Applying '" << nxt_here << "' value in the future stack.";
			my_promise->set_value(nxt_here);
			delete my_promise; // test if value keeps saved if killed.

			const auto val_last = *long_future.get();

			cout << "Last value: " << val_last;
			
			TESTLU(nxt_here == val_last && all_good, "Something wasn't right. One value didn't match!");

			cout << console::color::GREEN << "PASSED!";
		}
	}

	cout << console::color::LIGHT_PURPLE << "Testing 'process'...";

	{
		cout << "Testing a process... (self with FIRST_ARGUMENT..., it should show raw test here in dark green with =)";

		process proc;
		bool had_any_output = false;

		proc.hook_stdout([&](const std::string& out) {
			cout << console::color::DARK_GREEN << "= " << out;
			had_any_output = true;
		});
		
		TESTLU(proc.launch(self_path + " FIRST_ARGUMENT SECOND_ARGUMENT THIRD_ARGUMENT \"Forth argument, but within these \\\"\""), "Could not launch process in a reasonable time!");

		while (proc.running()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

		cout << "It seems that the task has ended.";

		proc.stop();

		TESTLU(had_any_output, "I didn't say a thing? No output registered!");

		cout << console::color::GREEN << "PASSED! (some output and no errors)";
	}

	cout << console::color::LIGHT_PURPLE << "Testing 'memory'...";

	{
		hybrid_memory<int> hybrid_src;
		hybrid_memory<int> hybrid_cpy;

		cout << "Generating a hybrid_memory...";

		hybrid_src = make_hybrid<int>();

		cout << "Copying (shared reference) hybrid memory...";

		hybrid_cpy = hybrid_src; // shared cpy

		cout << "Setting value for first and comparing memories...";

		*hybrid_src = 10;

		TESTLU(hybrid_src == hybrid_cpy, "They are not the same!");
		TESTLU(*hybrid_src == *hybrid_cpy, "They are not the same in value somehow!");

		cout << "Trying to change their both pointers...";

		hybrid_src.replace_shared(std::make_unique<int>(50));

		TESTLU(hybrid_src == hybrid_cpy, "They are not the same!");
		TESTLU(*hybrid_src == *hybrid_cpy, "They are not the same in value somehow!");

		cout << "Trying to change only one...";

		hybrid_src.replace_this(std::make_unique<int>(100));

		TESTLU(hybrid_src != hybrid_cpy, "They are the same still!");
		TESTLU(*hybrid_src != *hybrid_cpy, "They are the same in value somehow!");

		cout << console::color::GREEN << "PASSED!";
	}

	cout << console::color::LIGHT_PURPLE << "Testing 'multi_map'...";

	{
		cout << "First, testing multiple_data<>...";

		multiple_data<int, char, double, float> supadupa;

		supadupa = 3.14f;
		supadupa = 15.54978;
		supadupa = 'a';
		supadupa = 15;

		TESTLU(static_cast<float>(supadupa) == 3.14f, "Failed testing FLOAT");
		TESTLU(static_cast<double>(supadupa) == 15.54978, "Failed testing DOUBLE");
		TESTLU(static_cast<char>(supadupa) == 'a', "Failed testing CHAR");
		TESTLU(static_cast<int>(supadupa) == 15, "Failed testing INT");

		multiple_data<int, char, double, float> othar = supadupa;

		TESTLU(othar == supadupa, "Failed testing match on copy of SINGLE element (==)");
		TESTLU(othar.is_all_equal(supadupa), "Failed testing match on copy of ALL elements (is_all_equal())");

		cout << "Testing multi_map<>...";

		multi_map<int, char, unsigned, int, double> mymap;

		mymap += {10, 'a', 15u, 20, 3.14};
		mymap += {15, 'b', 4532u, 999, 12.54234};

		TESTLU(mymap['b'] == 15, "Failed trying to read key on multi_map");
		TESTLU(mymap[4532u] == 15, "Failed trying to read key on multi_map");
		TESTLU(mymap[999] == 15, "Failed trying to read key on multi_map");
		TESTLU(mymap[12.54234] == 15, "Failed trying to read key on multi_map");
		TESTLU(mymap['a'] == 10, "Failed trying to read key on multi_map");
		TESTLU(mymap[15u] == 10, "Failed trying to read key on multi_map");
		TESTLU(mymap[20] == 10, "Failed trying to read key on multi_map");
		TESTLU(mymap[3.14] == 10, "Failed trying to read key on multi_map");

		cout << "Testing fixed_multi_map<> with std::string (complex)...";

		fixed_multi_map<std::string, 3, int, char, std::string> fixedmap({
			{ std::string("Stringo"), 10, 'a', "yoo1key1" },
			{ std::string("Stringa"), 12, 'b', "yoo2key2" },
			{ std::string("Stringy"), 15, 'c', "yoo3key3" }
		});

		TESTLU(fixedmap[10] == "Stringo", "Failed trying to read key on fixed_multi_map");
		TESTLU(fixedmap['a'] == "Stringo", "Failed trying to read key on fixed_multi_map");
		TESTLU(fixedmap[std::string("yoo1key1")] == "Stringo", "Failed trying to read key on fixed_multi_map");
		TESTLU(fixedmap[12] == "Stringa", "Failed trying to read key on fixed_multi_map");
		TESTLU(fixedmap['b'] == "Stringa", "Failed trying to read key on fixed_multi_map");
		TESTLU(fixedmap[std::string("yoo2key2")] == "Stringa", "Failed trying to read key on fixed_multi_map");
		TESTLU(fixedmap[15] == "Stringy", "Failed trying to read key on fixed_multi_map");
		TESTLU(fixedmap['c'] == "Stringy", "Failed trying to read key on fixed_multi_map");
		TESTLU(fixedmap[std::string("yoo3key3")] == "Stringy", "Failed trying to read key on fixed_multi_map");

		cout << console::color::GREEN << "PASSED!";
	}

	cout << console::color::DARK_BLUE << "======================================";
	cout << console::color::BLUE << "# Ended utility_test!";
	cout << console::color::DARK_BLUE << "======================================";
	return 0;
}

int audio_test()
{
	cout << console::color::DARK_BLUE << "======================================";
	cout << console::color::BLUE << "# Starting audio_test...";
	cout << console::color::DARK_BLUE << "======================================";

	downloader download_audio;
	file audio_file;

	TESTLU(audio_file.open_temp("lunaris_debug_XXXXX.ogg", "wb+"), "Can't create temporary file for Audio testing!");

	TESTLU(download_audio.get_store(fixed_audio_src_url, [&audio_file](const char* buf, const size_t siz) { if (!audio_file.write(buf, siz)) { cout << console::color::RED << "Failed once writing in temporary file!"; }}), "Can't download temporary file!");

	TESTLU(audio_file.flush(), "Failed to flush the temporary file?!");

	voice my_voice;
	mixer my_mixer;
	mixer my_submixer;
	track my_track;
	sample my_sample;

	cout << "Starting Voice...";

	TESTLU(my_voice.create(), "Can't start Voice!");
	TESTLU(my_voice.exists(), "Voice is reporting inexistent?!");

	cout << console::color::GREEN << "PASSED!";

	cout << "Starting Mixer...";

	TESTLU(my_mixer.create(), "Can't start Mixer!");
	TESTLU(my_mixer.exists(), "Mixer is reporting inexistent?!");

	cout << console::color::GREEN << "PASSED!";

	cout << "Starting SubMixer...";

	TESTLU(my_submixer.create(), "Can't start SubMixer!");
	TESTLU(my_submixer.exists(), "SubMixer is reporting inexistent?!");

	cout << console::color::GREEN << "PASSED!";

	cout << "Loading Sample...";

	TESTLU(my_sample.load(audio_file.get_current_path()), "Could not load temporary audio file named '" + audio_file.get_current_path() + "'!");

	cout << console::color::GREEN << "PASSED!";

	cout << "Loading Track and attaching things together...";

	TESTLU(my_track.load(my_sample), "Can't load sample in track!");
	TESTLU(my_mixer.attach_to(my_voice), "Can't attach main mixer to voice!");
	TESTLU(my_submixer.attach_to(my_mixer), "Can't attach submixer to mixer!");
	TESTLU(my_track.attach_to(my_submixer), "Can't attach track to submixer!");

	cout << console::color::GREEN << "PASSED!";

	my_track.set_gain(0.3f);

	cout << "Loaded fancy music. Detailed info:";
	cout << "Channels: " << console::color::AQUA << my_track.get_channels_num();
	cout << "Bit depth: " << console::color::AQUA << my_track.get_depth_bits();
	cout << "Frequency: " << console::color::AQUA << static_cast<int>(my_track.get_frequency() / 1000) << (my_track.get_frequency() % 1000 != 0 ? ("." + std::to_string((my_track.get_frequency() / 100) % 10)) : "") << " kHz";
	cout << "Length: " << console::color::AQUA << my_track.get_time_s() << " second(s)";
	cout << "Starting track...";

	my_track.play();

	TESTLU(my_track.is_playing(), "Track didn't report it started playing!");

	cout << console::color::GREEN << "PASSED!";
	
	cout << "Testing if some duration match...";

	std::this_thread::sleep_for(std::chrono::seconds(3));

	my_track.pause();

	cout << "Time reported: " << my_track.get_position_ms() << " ms";

	if (fabsf(my_track.get_position_ms() * 0.001f - 3.0f) < 0.2f) {
		cout << console::color::GREEN << "PASSED!";
	}
	else {
		cout << console::color::RED << "Time report is not good. Failed.";
		return 1;
		//hold_user();
	}

	cout << "Seeking file to 15 sec...";

	my_track.set_position_ms(15000);

	cout << "Time reported: " << my_track.get_position_ms() << " ms";

	if (fabsf(my_track.get_position_ms() * 0.001f - 15.0f) < 0.2f) {
		cout << console::color::GREEN << "PASSED!";
	}
	else {
		cout << console::color::RED << "Time report after seek is not good. Failed.";
		return 1;
		//hold_user();
	}

	my_track.set_speed(2.0f);

	cout << "Testing if 2.0x speed cause 2x time...";

	my_track.play();

	std::this_thread::sleep_for(std::chrono::seconds(3));

	my_track.pause();

	cout << "Time reported: " << my_track.get_position_ms() << " ms";

	if (fabsf(my_track.get_position_ms() * 0.001f - 21.0f) < 0.2f) {
		cout << console::color::GREEN << "PASSED!";
	}
	else {
		cout << console::color::RED << "Time report after speed is not good. Failed.";
		return 1;
		//hold_user();
	}

	my_track.stop();
	my_track.set_speed(1.0f);
	my_track.set_position_ms(60000);

	cout << console::color::YELLOW << "The next tests need the user's attention. Please check if the [!] looks right.";

	std::this_thread::sleep_for(std::chrono::seconds(3));

	cout << "[!] Testing pan (balance) for 5 seconds... (you have to tell if it works. It should change between left and right as time goes by)";

	auto time_after = std::chrono::system_clock::now() + std::chrono::seconds(5);

	my_track.play();

	while (std::chrono::system_clock::now() < time_after) {
		const double dt = cos(al_get_time() * 3.0);
		my_track.set_pan(dt);
		std::cout << "PAN: " << dt << "      \r";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	//my_track.stop();
	my_track.set_pan(ALLEGRO_AUDIO_PAN_NONE);

	cout << "[!] Testing mute on sub-mixer (tell me if it works or not)";

	time_after = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while (std::chrono::system_clock::now() < time_after) {
		const bool mt = cos(al_get_time() * 3.0) > 0.0;
		my_submixer.mute(mt);
		std::cout << "MUTE: " << (mt ? "ON" : "OFF") << " \r";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	my_submixer.mute(false);

	cout << "[!] Testing volume on sub-mixer (tell me if it works or not)";

	time_after = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while (std::chrono::system_clock::now() < time_after) {
		const float dt = cos(al_get_time() * 3.0) * 0.5f + 0.5f;
		my_submixer.set_gain(dt);
		std::cout << "VOL: " << static_cast<int>(100.0 * dt) << "%      \r";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	my_submixer.set_gain(1.0f);

	cout << "[!] Testing volume on main mixer (tell me if it works or not)";

	time_after = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while (std::chrono::system_clock::now() < time_after) {
		const float dt = sin(al_get_time() * 3.0) * 0.5f + 0.5f;
		my_mixer.set_gain(dt);
		std::cout << "VOL: " << static_cast<int>(100.0 * dt) << "%      \r";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	my_mixer.set_gain(1.0f);

	cout << "[!] Testing mute on voice (tell me if it works or not)";

	time_after = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while (std::chrono::system_clock::now() < time_after) {
		const bool mt = sin(al_get_time() * 3.0) > 0.0;
		if (!my_voice.mute(mt)) {
			cout << console::color::YELLOW << "[WARN] Sadly this VOICE device doesn't support mute?! (This is possible if stream file doesn't allow it)";
			break;
		}
		else {
			std::cout << "MUTE: " << (mt ? "ON" : "OFF") << " \r";
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	my_voice.mute(false);

	my_track.stop();

	cout << console::color::DARK_BLUE << "======================================";
	cout << console::color::BLUE << "# Ended audio_test!";
	cout << console::color::DARK_BLUE << "======================================";

	return 0;
}

int graphics_test()
{
	cout << console::color::DARK_BLUE << "======================================";
	cout << console::color::BLUE << "# Starting graphics_test...";
	cout << console::color::DARK_BLUE << "======================================";

	std::atomic<bool> keep_running_things = true;
	block blk_fixed, blk_mouse;
	text txt_main;
	display my_display;
	//file fp; // random file
	thread blocks_col;
	mouse mousing(my_display);
	collisionable cols[2] = { {blk_mouse, "mouse"}, {blk_fixed, "block"}};
	const color no_collision = color(127, 255, 127);
	const color has_collision = color(255, 127, 127);
	const color mouse_no_collision = color(127, 255, 255);
	const color mouse_has_collision = color(255, 127, 255);
	//auto random_texture = make_hybrid<texture>();
	auto font_u = make_hybrid<font>();

	//cout << "Opening temporary file for temporary image...'";
	//
	//TESTLU(fp.open_temp("lunaris_XXXXX.tmp", "wb+"), "Failed to open temp file.");
	//
	//cout << "Downloading random image from '" << random_img_url << "'";
	//
	//{
	//	downloader down;
	//	TESTLU(down.get_store(random_img_url, [&](const char* buf, size_t len) { if (!fp.write(buf, len)) { cout << console::color::RED << "FATAL ERROR WRITING TO FILE! ABORT!"; std::terminate(); } }), "Failed to download random image.");
	//}
	//
	//cout << "Temporary image file perfectly saved at '" << fp.get_current_path() << "'";
	//fp.flush();

	cout << "Creating display...";

	TESTLU(my_display.create(display_config().set_fullscreen(false).set_display_mode(display_options().set_width(1280).set_height(720)).set_window_title("GRAPHICS TEST").set_self_draw(true).set_extra_flags(ALLEGRO_DIRECT3D_INTERNAL | ALLEGRO_RESIZABLE)), "Failed to create the display");

	cout << "Setting up some variables...";

	blk_fixed.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, false);
	blk_mouse.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, false);
	txt_main.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, false);

	txt_main.set<text::safe_string>(enum_text_safe_string_e::STRING, std::string("Test string text"));
	txt_main.set<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE, true); // deform pos
	txt_main.set<float>(enum_sprite_float_e::SCALE_G, 0.1f);
	txt_main.set<float>(enum_sprite_float_e::SCALE_X, 0.3f);
	txt_main.set<float>(enum_sprite_float_e::POS_X, -0.992f);
	txt_main.set<float>(enum_sprite_float_e::POS_Y, -0.992f);
	txt_main.set<float>(enum_sprite_float_e::DRAW_MOVEMENT_RESPONSIVENESS, 3.0f);
	for (int __c = 1; 255 - 25 * __c > 0; __c++) {
		int ctee = (255 - 25 * __c);
		txt_main.shadow_insert(text_shadow(0.0001f * __c, 0.007f * __c, color(ctee / 10, ctee / 10, ctee / 10, ctee)));
	}

	blk_mouse.set<float>(enum_sprite_float_e::SCALE_G, 0.25f);
	blk_mouse.set<bool>(enum_sprite_boolean_e::DRAW_THINK_BOX, true);

	blk_fixed.set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, color(255,255,255));
	blk_fixed.set<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX, true);
	blk_fixed.set<float>(enum_sprite_float_e::SCALE_G, 0.5f);

	cout << "Applying default transformation to display...";

	my_display.add_run_once_in_drawing_thread([&my_display] {
		transform transf;
		transf.build_classic_fixed_proportion(my_display.get_width(), my_display.get_height(), 1.0f, 1.0f);
		transf.apply();
	});

	cout << "Loading texture in video memory and default font...";

	{
		bool good = false, dod = false;
		my_display.add_run_once_in_drawing_thread([&] {
			good = /*random_texture->load(fp.get_current_path()) && */font_u->create_builtin_font();
			dod = true;
		});

		while (!dod) std::this_thread::sleep_for(std::chrono::milliseconds(50));
		TESTLU(good, "Couldn't load texture/font for test!");
	}

	cout << "Hooking mouse to a sprite...";

	mousing.hook_event([&blk_mouse](int type, const mouse::mouse_event& ev) {
		if (type == ALLEGRO_EVENT_MOUSE_AXES) {
			blk_mouse.set<float>(enum_sprite_float_e::POS_X, ev.real_posx);
			blk_mouse.set<float>(enum_sprite_float_e::POS_Y, ev.real_posy);
		}
		else if (type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.is_button_pressed(0)) {
			blk_mouse.set<float>(enum_sprite_float_e::ACCEL_ROTATION, 0.06f);
		}
		else if (type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && !ev.is_button_pressed(0)) {
			blk_mouse.set<float>(enum_sprite_float_e::ACCEL_ROTATION, 0.0f);
		}
	});

	cout << "Setting up drawing call...";

	my_display.hook_draw_function([&] {
		al_clear_to_color(al_map_rgb(
			cos(al_get_time() * 0.4111) * 100 + 150,
			sin(al_get_time() * 0.2432) * 100 + 150,
			cos(al_get_time() * 0.5321) * 100 + 150
		));

		blk_fixed.draw();
		blk_mouse.draw();
		txt_main.draw();

		{
			transform savv, raww;
			savv.get_current_transform();

			raww.identity();
			raww.apply();

			al_draw_circle(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X), blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y), 30, al_map_rgb(150, 0, 0), 5);
			al_draw_circle(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X), blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y), 30, al_map_rgb(0, 150, 0), 5);
			al_draw_circle(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X), blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y), 30, al_map_rgb(0, 0, 150), 5);
			al_draw_circle(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X), blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y), 30, al_map_rgb(150, 150, 0), 5);

			al_draw_circle(blk_fixed.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X), blk_fixed.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y), 10, al_map_rgb(150, 0, 0), 3);
			al_draw_circle(blk_fixed.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X), blk_fixed.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y), 10, al_map_rgb(0, 150, 0), 3);
			al_draw_circle(blk_fixed.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X), blk_fixed.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y), 10, al_map_rgb(0, 0, 150), 3);
			al_draw_circle(blk_fixed.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X), blk_fixed.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y), 10, al_map_rgb(150, 150, 0), 3);

			savv.apply();
		}
	});

	cout << "Setting up display events function...";

	my_display.hook_event_handler([&keep_running_things](const ALLEGRO_EVENT& ev) {
		cout << console::color::AQUA << "DISPLAY EVENT: " << console::color::BLUE << "Event #" << ev.type << " triggered.";
		
		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			cout << console::color::GREEN << "Closing app...";
			keep_running_things = false;
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			cout << console::color::GREEN << "Screen size is now: " << ev.display.width << "x" << ev.display.height;
			transform transf;
			transf.build_classic_fixed_proportion(ev.display.width, ev.display.height, 1.0f, 1.0f);
			transf.apply();
		}
	});

	cout << "Setting up collision & extra tasks thread...";
	
	for (auto& i : cols) i.set_work([&](collisionable::result res, sprite& one) {
		one.set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, res.dir_to != 0 ? has_collision : no_collision);
		//if (data != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_ROTATION, was_clockwise ? 0.1f : -0.1f);
		if (res.dir_to != 0) {
			if ((res.dir_to & static_cast<int>(collisionable::direction_op::DIR_NORTH)) != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_Y, 0.005f);
			if ((res.dir_to & static_cast<int>(collisionable::direction_op::DIR_SOUTH)) != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_Y, -0.005f);
			if ((res.dir_to & static_cast<int>(collisionable::direction_op::DIR_EAST))  != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_X, -0.005f);
			if ((res.dir_to & static_cast<int>(collisionable::direction_op::DIR_WEST))  != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_X, 0.005f);
			one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_ROTATION, res.moment_dir);
		}
		if (res.dir_to != 0) {
			if (res.is_dir(collisionable::direction_op::DIR_NORTH)) cout << i.nam() << " COL NORTH #" << (size_t)((void*)&i);
			if (res.is_dir(collisionable::direction_op::DIR_SOUTH)) cout << i.nam() << " COL SOUTH #" << (size_t)((void*)&i);
			if (res.is_dir(collisionable::direction_op::DIR_EAST))  cout << i.nam() << " COL EAST  #" << (size_t)((void*)&i);
			if (res.is_dir(collisionable::direction_op::DIR_WEST))  cout << i.nam() << " COL WEST  #" << (size_t)((void*)&i);
		}
		one.think(); 
	});


	blocks_col.task_async([&] {

		work_all_auto(std::begin(cols), std::end(cols));

		///cols[0].reset();
		///cols[1].reset();
		///cols[1].overlap(cols[0]);
		///cols[0].work();
		///cols[1].work();
		
		//for (auto& i : cols) i.reset();
		//for (size_t p = 0; p < std::size(cols); p++)
		//{
		//	for (size_t q = 0; q < std::size(cols); q++) {
		//		if (q != p) cols[p].overlap(cols[q]);
		//	}
		//}
		//for (auto& i : cols) i.work();

		// extra mine
		//blk_mouse.set<float>(enum_sprite_float_e::ROTATION, al_get_time() * 0.15f);
		//blk_fixed.set<float>(enum_sprite_float_e::ROTATION, al_get_time() * 0.05f);

		txt_main.set<float>(enum_sprite_float_e::ROTATION, cos(al_get_time() * 2.5) * 0.3f);

		{
			std::string dat;

			dat += "POINTS MOUSE RAW:\n";
			dat += "TOPLEFT:   [red]    [" + std::to_string(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_X)) + ";" + std::to_string(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHWEST_Y)) + "]\n";
			dat += "TOPRIGHT:  [green]  [" + std::to_string(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_X)) + ";" + std::to_string(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_NORTHEAST_Y)) + "]\n";
			dat += "DOWNLEFT:  [blue]   [" + std::to_string(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_X)) + ";" + std::to_string(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHWEST_Y)) + "]\n";
			dat += "DOWNRIGHT: [yellow] [" + std::to_string(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_X)) + ";" + std::to_string(blk_mouse.get<float>(enum_sprite_float_e::RO_THINK_POINT_SOUTHEAST_Y)) + "]\n";
			dat += "ROTATION ANGLE (DEGREES): " +
				std::to_string(static_cast<unsigned long long>(blk_mouse.get<float>(enum_sprite_float_e::ROTATION) * 180.0f / ALLEGRO_PI) % 360) + " or " + 
				std::to_string(static_cast<unsigned long long>(blk_mouse.get<float>(enum_sprite_float_e::RO_DRAW_PROJ_ROTATION) * 180.0f / ALLEGRO_PI) % 360) + " (proj/smooth)";

			txt_main.set<text::safe_string>(enum_text_safe_string_e::STRING, dat);
		}
	}, thread::speed::INTERVAL, 1.0/20);

	cout << "Enabling things on screen...";

	txt_main.font_set(font_u);
	//blk_mouse.texture_insert(random_texture);
	blk_mouse.set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, color(75, 75, 255));
	blk_mouse.set<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX, true);

	blk_fixed.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, true);
	blk_mouse.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, true);
	txt_main.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, true);

	cout << console::color::DARK_GRAY << "Done! Close the screen if you want to exit.";
	cout << console::color::DARK_GRAY << "This test is not complete, but this loading stuff and so on is kinda of a test.";

	while (keep_running_things) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	cout << console::color::DARK_GRAY << "Detected end! Closing stuff...";

	blocks_col.join();
	txt_main.set<text::safe_string>(enum_text_safe_string_e::STRING, std::string("Closing..."));
	my_display.destroy();
	// the rest should be fine.

	cout << console::color::DARK_GRAY << "Everything should be good now.";
	

	cout << console::color::DARK_BLUE << "======================================";
	cout << console::color::BLUE << "# Ended graphics_test!";
	cout << console::color::DARK_BLUE << "======================================";
	return 0;
}

int events_test()
{
	cout << console::color::DARK_BLUE << "======================================";
	cout << console::color::BLUE << "# Starting events_test...";
	cout << console::color::DARK_BLUE << "======================================";

	cout << console::color::YELLOW << "Sometimes this test needs your attention. Please follow the steps so we know if things are working as expected.";

	cout << "Creating display... (keep it in focus while the test is running)";

	display disp;
	TESTLU(disp.create(display_config().set_display_mode(display_options().set_width(640).set_height(480)).set_fullscreen(false).set_use_basic_internal_event_system(false).set_window_title("EVENTS_TEST")), "Could not create a simple display for testing!");

	int last_event_id = 0;

	generic_event_handler event_generic;
	event_generic.hook_event_handler([&](const ALLEGRO_EVENT& ev) {
		last_event_id = static_cast<int>(ev.type);
	});

	event_generic.install_other(disp.get_event_source());

	cout << "Checking common events from Display. Please do what I say from now on:";

	auto timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);

	cout << console::color::YELLOW << "Please MINIMIZE or tab-out the window (you have 15 seconds to do it).";
	timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
	while (std::chrono::system_clock::now() < timeoutt && (last_event_id != ALLEGRO_EVENT_DISPLAY_SWITCH_OUT)) std::this_thread::sleep_for(std::chrono::milliseconds(50));
	TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
	cout << console::color::GREEN << "Good!";

	cout << console::color::YELLOW << "Please MAXIMIZE or select the window (you have 15 seconds to do it).";
	timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
	while (std::chrono::system_clock::now() < timeoutt && (last_event_id != ALLEGRO_EVENT_DISPLAY_SWITCH_IN)) std::this_thread::sleep_for(std::chrono::milliseconds(50));
	TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
	cout << console::color::GREEN << "Good!";

	cout << console::color::YELLOW << "Please try to CLOSE (the X) the window (you have 15 seconds to do it).";
	timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
	while (std::chrono::system_clock::now() < timeoutt && (last_event_id != ALLEGRO_EVENT_DISPLAY_CLOSE)) std::this_thread::sleep_for(std::chrono::milliseconds(50));
	TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
	cout << console::color::GREEN << "Good!";

	cout << console::color::GREEN << "PASSED!";

	cout << "Now we're going to test keyboard input (please keep screen selected while doing it):";
	{
		keyboard kb;
		int last_key = 0;
		std::string last_string;
		std::string curr_string;
		bool keep_clean = true;

		kb.hook_each_key_event([&](keyboard& self, int key) {
			last_key = key;
		});
		kb.hook_each_key_phrase_event([&](keyboard& self, const std::string& cpy) {
			if (keep_clean) self.clear();
			else curr_string = cpy;
		});
		kb.hook_enter_line_phrase_event([&](keyboard& self, const std::string& str) {
			last_string = str;
			self.clear();
		});		

		cout << console::color::YELLOW << "Please hit the H key (you have 15 seconds to do it).";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && (std::tolower(last_key) != 'h')) std::this_thread::sleep_for(std::chrono::milliseconds(50));
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!";

		keep_clean = false;

		cout << console::color::YELLOW << "Please type \"LunARiS\" and hit ENTER (yes, with those CAPS there) (you have 60 seconds to do it).";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(60);
		while (std::chrono::system_clock::now() < timeoutt && (last_string != "LunARiS")) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); std::cout << "Currently detected: " << curr_string << "   \r"; }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.        ");
		cout << console::color::GREEN << "Good!                            ";

		cout << console::color::GREEN << "PASSED!";
	}

	cout << "Now some mouse testing (please keep screen selected while doing it):";
	{
		mouse mousse(disp);

		transform transf;
		transf.build_classic_fixed_proportion(disp.get_width(), disp.get_height(), 1.0f * disp.get_width() / disp.get_height());		
		transf.apply();

		int step = 0;

		mousse.hook_event([&disp,&step](int bid, const mouse::mouse_event& ev) {
			disp.set_as_target();
			al_clear_to_color(al_map_rgb(0, 0, 0));
			switch (step) {
			case 0:
				al_draw_filled_circle(ev.real_posx, ev.real_posy, 0.1f, al_map_rgb(127, 127, 127));
				break;
			case 1:
				al_draw_filled_circle(ev.real_posx, ev.real_posy, 0.1f, al_map_rgb(127, 255, 127));
				break;
			case 2:
				al_draw_filled_circle(ev.real_posx, ev.real_posy, 0.1f, al_map_rgb(127, 255, 255));
				break;
			case 3:
				al_draw_filled_circle(ev.real_posx, ev.real_posy, 0.1f, al_map_rgb(127, 127, 255));
				break;
			case 4:
				al_draw_filled_circle(ev.real_posx, ev.real_posy, 0.1f, al_map_rgb(0, 255, 0));
				break;
			default: // 5
				al_draw_filled_circle(ev.real_posx, ev.real_posy, 0.1f, al_map_rgb(0, 127, 255));
				break;
			}
			disp.flip();
		});
		
		cout << console::color::YELLOW << "Please hit the button #0 of your mouse (probably the one you select things).";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && !mousse.current_mouse().is_button_pressed(0)) std::this_thread::sleep_for(std::chrono::milliseconds(50));
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!";
		++step;
		
		cout << console::color::YELLOW << "Please hit the button #1 of your mouse (probably the other one).";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && !mousse.current_mouse().is_button_pressed(1)) std::this_thread::sleep_for(std::chrono::milliseconds(50));
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!";
		++step;

		cout << console::color::YELLOW << "Ok, now move the mouse to the left side of the WINDOW.";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && mousse.current_mouse().relative_posx > -0.9f) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); std::cout << "RELPOSX: " << mousse.current_mouse().relative_posx << "   \r"; }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!                          ";
		++step;

		cout << console::color::YELLOW << "Ok, now to the right side!";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && mousse.current_mouse().relative_posx < 0.9f) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); std::cout << "RELPOSX: " << mousse.current_mouse().relative_posx << "   \r"; }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!                          ";
		++step;

		cout << console::color::YELLOW << "Now to the top!";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && mousse.current_mouse().relative_posy > -0.9f) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); std::cout << "RELPOSY: " << mousse.current_mouse().relative_posy << "   \r"; }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!                          ";
		++step;

		cout << console::color::YELLOW << "Now to the bottom!";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && mousse.current_mouse().relative_posy < 0.9f) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); std::cout << "RELPOSY: " << mousse.current_mouse().relative_posy << "   \r"; }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!                          ";

		cout << console::color::GREEN << "PASSED!";
	}

	disp.destroy();

	cout << "If keyboard and mouse are working, we can assume any other event type is probably working too (they are kinda the same tbh).";

	cout << console::color::DARK_BLUE << "======================================";
	cout << console::color::BLUE << "# Ended events_test!";
	cout << console::color::DARK_BLUE << "======================================";
	return 0;
}