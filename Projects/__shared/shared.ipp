//#define LUNARIS_ALPHA_TESTING
//#define LUNARIS_HEADER_ONLY
#include <Lunaris/all.h>

#include <iterator>

//#define ENABLE_ALPHA_TESTING_HARD

#ifdef ENABLE_ALPHA_TESTING_HARD
#include <Psapi.h>
#endif

using namespace Lunaris;

#define TESTLU(X, ERRMSG) if (run_and_test([&]{return X;}, ERRMSG) != 0) return 1;
#define AUTOEXCEPT(X) [&]{try {auto v = X; return v; } catch (const std::exception& e) { cout << console::color::RED << "EXCEPTION: " << e.what(); } catch(...) {cout << console::color::RED << "EXCEPTION: UNCAUGHT!";} return 0; }()

const std::string fixed_audio_src_url = "https://cdn.discordapp.com/attachments/888270629990707331/888270836677607425/music_01.ogg";
const std::string fixed_image_src_url = "https://media.discordapp.net/attachments/888270629990707331/888272596720844850/3.jpg?width=918&height=612";
const std::string fixed_image_src_url_sha256_precalc = "e02e1baec55f8e37b5a5b3ef29d403fc2a2084267f05dfc75d723536081aff10";
const std::string temp_local_file_path = "lunaris_temp_local.tmp";
const std::string random_img_url = "https://picsum.photos/1024"; //"https://www.dropbox.com/s/nnl1tbypldv1un6/Photo_fur_2018.jpg?dl=1"; 
const std::string fixed_my_catto_GIF_url = "https://media.discordapp.net/attachments/888270629990707331/892966440431403029/cat.gif";
#ifdef ENABLE_ALPHA_TESTING_HARD
const std::string fixed_random_font_url = "https://cdn.discordapp.com/attachments/632626072478810128/904055227567722616/Lobster_1.3.ttf";
#endif

constexpr size_t num_of_entities_in_package_test = 1000;

void hold_user();
int run_and_test(std::function<bool(void)>, const std::string&);

int utility_test(const std::string&); // GOOD 100%
int audio_test(); // GOOD 100%
int events_test(); // GOOD (enough)
int graphics_test(); 

#ifdef ENABLE_ALPHA_TESTING_HARD
void hard_test()
{
	display disp;
	font foont;
	downloader down;

	cout << console::color::DARK_PURPLE << "Initializing ALPHA test...";

	bool good = disp.create(display_config().set_display_mode(display_options().set_width(800).set_height(600)).set_extra_flags(ALLEGRO_DIRECT3D_INTERNAL | ALLEGRO_RESIZABLE).set_fullscreen(false).set_use_basic_internal_event_system(true).set_window_title("Memory test"));
	if (!good) {
		cout << console::color::RED << "Bad news disp.";
		std::terminate();
	}
	
	{
		good = down.get(fixed_random_font_url);
		if (!good) {
			cout << console::color::RED << "Bad news downloading.";
			disp.destroy();
			std::terminate();
		}
	}

	const size_t iterations = 100000;
	const size_t to_mega = 1024 * 1024;
	const color blackkk(0, 0, 0);
	const double difftime = 0.125;
	size_t counter = 0, pp = 0;

	const auto print_info_console = [&] {
		PROCESS_MEMORY_COUNTERS_EX rawmemusage;
		rawmemusage.cb = sizeof(rawmemusage);
		auto gud = GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&rawmemusage, sizeof(rawmemusage));
		if (!gud) {
			cout << console::color::RED << "Issue getting current memory information.";
		}

		printf_s("TIME:%08.3lf | IT@%020zu > MemPeak: %08.6lf MB | Now: %08.6lf MB | Charge: %08.6lf MB  %c", counter * difftime, pp, rawmemusage.PeakWorkingSetSize * 1.0 / to_mega, rawmemusage.WorkingSetSize * 1.0 / to_mega, rawmemusage.PagefileUsage * 1.0 / to_mega, ((counter % 20) == 0) ? '\n' : '\r');
		++counter;
	};

	thread monitor(print_info_console, thread::speed::INTERVAL, difftime);

	disp.hook_event_handler([&](const ALLEGRO_EVENT& ev) {
		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			pp = iterations;
		}
	});

	for (pp = 0; pp < iterations && !disp.empty(); pp++) {

		auto tempmemfp = make_hybrid<file>();
		if (!(tempmemfp->open("lunaris_alpha_test_XXXX.ttf", file::open_mode_e::READWRITE_REPLACE))) {
			cout << console::color::RED << "Could not create tempfile. Trying again...";
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			continue;
		}

		if (tempmemfp->write(down.read().data(), down.read().size()) != down.read().size()) {
			cout << console::color::RED << "Could not write tempfile. Trying again...";
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			continue;
		}
		tempmemfp->flush();

		if (!foont.load(tempmemfp)) {
			cout << console::color::RED << "Could not load texture. Trying again...";
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			continue;
		}

		blackkk.clear_to_this();
		foont.draw(color(200, 200, 200), random() % 20, random() % 20, 0, ("Hello there #" + std::to_string(pp)));
		disp.flip();

		foont.destroy();
		tempmemfp.reset_shared();
	}

	monitor.join();
}
#endif

int main(int argc, char* argv[]) {
	TESTLU(argc >= 1, "IRREGULAR STARTUP! Can't proceed.");
	const std::string currpath = argv[0];


	cout << console::color::YELLOW << "Lunaris version short: " << LUNARIS_VERSION_SHORT;
	cout << console::color::YELLOW << "Lunaris version long:  " << LUNARIS_VERSION_LONG;
	cout << console::color::YELLOW << "Lunaris version date:  " << LUNARIS_VERSION_DATE;
			

	if (argc > 1) {
		cout << "Hello someone calling me with custom arguments! I received those:";
		for (int a = 0; a < argc; a++) cout << "Argument #" << a << ": '" << argv[a] << "'";
		cout << "Have a great day! Exiting the app...";
		return 0;
	}

#ifdef ENABLE_ALPHA_TESTING_HARD
	hard_test();
	return 0;
#endif

	if (AUTOEXCEPT(utility_test(currpath)) != 0) return 1;
	if (AUTOEXCEPT(audio_test()) != 0) return 1;
	if (AUTOEXCEPT(events_test()) != 0) return 1;
	if (AUTOEXCEPT(graphics_test()) != 0) return 1;
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

	cout << console::color::LIGHT_PURPLE << "Testing 'bomb'...";
	{
		cout << "Creating a bomb...";

		bool realboom = false;
		{
			bomb mybomb([&] {realboom = true; });
			cout << "The bomb has been planted. Testing it...";
		}
		TESTLU(realboom, "The bomb didn't ignite. Bad news.");

		cout << "Good, but is the anti-bomb squad good? Let's test them. Another bomb, but this time...";
		bool shoulddefuse = false;
		realboom = false;
		{
			bomb mybomb([&] {realboom = true; });
			bomb mybomb2([&] {shoulddefuse = true; });
			cout << "The bombs has been planted. Didn't I tell you there's two now? If we move one to another, the one being changed should explode if not defused before.";

			mybomb2 = std::move(mybomb);
			TESTLU(shoulddefuse, "Oh no bombs are not working as we expected.");
			TESTLU(mybomb.is_defused(), "What? Duplicated bombs?");
			TESTLU(!mybomb2.is_defused(), "BOMB DISAPPEARED WHILE MOVING OH NO!");

			cout << "All good, defusing now.";

			mybomb2.defuse();
			cout << "Defused! Is that enough?";
		}
		TESTLU(!realboom, "The bomb exploded. We're all dead :(");

		cout << "Defuse works!";

		cout << "Creating a timed bomb with time = 2.2 sec...";

		realboom = false;
		{
			timed_bomb mybomb([&] {realboom = true; }, 2.2);
			cout << "The bomb has been planted. Waiting up to 5 seconds and checking status";
			for (int a = 0; a < 5; a++) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (!realboom) cout << "Bomb didn't trigger yet... [" << (a + 1) << "/5]";
				else break;
			}
			TESTLU(realboom, "The bomb didn't ignite. Bad news.");

			cout << "Timed bomb exploded in time!";
		}

		cout << "Now testing if a timed bomb explode early if destroyed. Time = 2 sec.";

		realboom = false;
		{
			timed_bomb mybomb([&] {realboom = true; }, 2.0);
			cout << "The bomb has been planted. Dropping the bomb alone!";
		}
		TESTLU(realboom, "The bomb didn't ignite. Bad news.");

		cout << "Timed bomb exploded on destroy!";
		cout << "Testing moving them around and defusing now.";

		shoulddefuse = false;
		realboom = false;
		{
			timed_bomb mybomb([&] {realboom = true; }, 2.0);
			timed_bomb mybomb2([&] {shoulddefuse = true; }, 2.0);
			cout << "The bombs has been planted.";

			mybomb2 = std::move(mybomb);
			TESTLU(shoulddefuse, "Oh no bombs are not working as we expected.");
			TESTLU(mybomb.is_defused(), "What? Duplicated bombs?");
			TESTLU(!mybomb2.is_defused(), "BOMB DISAPPEARED WHILE MOVING OH NO!");

			cout << "All good, defusing now.";

			mybomb2.defuse();
			cout << "Defused! Is that enough?";
		}
		TESTLU(!realboom, "The bomb exploded. We're all dead :(");
		cout << "Yes it is!";

		cout << console::color::GREEN << "PASSED!";
	}

	cout << console::color::LIGHT_PURPLE << "Testing 'throw_thread'...";
	{
		std::atomic_int counter = 0;
		cout << "Launching an async zombie thread (monitoring it)";

		{
			auto info = throw_thread([&counter] {std::this_thread::sleep_for(std::chrono::seconds(3));  while (++counter < 1000); });

			std::this_thread::sleep_for(std::chrono::seconds(1));
			TESTLU(counter == 0, "The thread didn't respect the time I asked for them.");

			cout << "Waiting zombie thread to count up to 1000!";
			for (int a = 0; a < 10; a++) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (counter < 1000) cout << "Counter at: " << counter << " [" << (a + 1) << "/5]";
				else break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			TESTLU(counter == 1000, "The thread didn't respect the time I asked for them.");
			TESTLU(info.has_ended(), "Future value was not set!");

		}

		counter = 0;
		cout << "Launching totally async this time!";

		{
			throw_thread([&counter] {std::this_thread::sleep_for(std::chrono::seconds(3));  while (++counter < 1000); });
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
		TESTLU(counter == 0, "The thread didn't respect the time I asked for them.");

		cout << "Waiting zombie thread to count up to 1000!";
		for (int a = 0; a < 10; a++) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			if (counter < 1000) cout << "Counter at: " << counter << " [" << (a + 1) << "/5]";
			else break;
		}
		TESTLU(counter == 1000, "The thread didn't respect the time I asked for them.");

		cout << console::color::GREEN << "PASSED!";
	}

	cout << console::color::LIGHT_PURPLE << "Testing 'paths'...";
	{
		cout << "Checking for common paths...";

		cout << "Name of the app: " << get_app_name();
		cout << "Changing app name to 'MY APP'...";
		set_app_name("MY APP");
		cout << "Name of the app now: " << get_app_name();

		cout << "Checking for common paths (expected: MY APP somewhere sometimes)...";

		cout << "Found path: '" << get_standard_path(ALLEGRO_RESOURCES_PATH) << "'";
		cout << "Found path: '" << get_standard_path(ALLEGRO_TEMP_PATH) << "'";
		cout << "Found path: '" << get_standard_path(ALLEGRO_USER_DATA_PATH) << "'";
		cout << "Found path: '" << get_standard_path(ALLEGRO_USER_HOME_PATH) << "'";
		cout << "Found path: '" << get_standard_path(ALLEGRO_USER_SETTINGS_PATH) << "'";
		cout << "Found path: '" << get_standard_path(ALLEGRO_USER_DOCUMENTS_PATH) << "'";
		cout << "Found path: '" << get_standard_path(ALLEGRO_EXENAME_PATH) << "'";

		cout << "Creating random paths...";

		make_path(get_standard_path() + "POGGERSFOLDER/DEEP/");
		make_path(get_standard_path() + "POGGERSFOLDER/OTHERDEEP/");

		cout << "Erasing them...";

		size_t amount = remove_all(get_standard_path());
		cout << "Removed " << amount << " paths.";

		TESTLU(amount == 4, "Wait, that's not right... Wrong number of paths erased!");

		cout << console::color::GREEN << "PASSED!";
	}

	cout << console::color::LIGHT_PURPLE << "Testing 'config'...";
	{
		const std::string temp_path_conf = "temp_conf.conf";
		const unsigned long long val = random();
		const std::string str = "lala" + std::to_string(random()) + "lolo" + std::to_string(random());

		{
			cout << "Opening a temporary config file...";

			config conf;
			conf.set("global", config::config_section_mode::MEMORY_ONLY);
			conf.set("global", "yoyo", true);
			conf.set("global", "yaya", false);

			cout << "Testing boolean...";

			TESTLU(conf.get_as<bool>("global", "yoyo") == true, "Config can't set values properly");
			TESTLU(conf.get_as<bool>("global", "yaya") == false, "Config can't set values properly");

			conf.set("global", "numba", val);

			cout << "Testing number...";

			TESTLU(conf.get_as<unsigned long long>("global", "numba") == val, "Config can't set values properly");

			conf.set("global", "strang", str);

			cout << "Testing string...";

			TESTLU(conf.get("global", "strang") == str, "Config can't set values properly");

			cout << "Testing saving the file...";

			conf.save_path(temp_path_conf);
			conf.set("global", config::config_section_mode::SAVE);

			TESTLU(conf.flush(), "Could not temporarily save file in current folder!");

			cout << "Testing reloading the file...";
		}
		{
			config conf;
			TESTLU(conf.load(temp_path_conf), "Could not reload config properly");

			cout << "Checking all values...";

			TESTLU(conf.get_as<bool>("global", "yoyo") == true, "Config can't reload values properly");
			TESTLU(conf.get_as<bool>("global", "yaya") == false, "Config can't reload values properly");
			TESTLU(conf.get_as<unsigned long long>("global", "numba") == val, "Config can't reload values properly");
			TESTLU(conf.get("global", "strang") == str, "Config can't reload values properly");

			cout << "Removing file...";

			conf.auto_save(false);

			std::remove(temp_path_conf.c_str());
		}
		cout << console::color::GREEN << "PASSED!";
	}

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
		TESTLU(fp.open(temp_local_file_path, file::open_mode_e::READWRITE_REPLACE), "Failed to create a local file.");

		cout << console::color::GREEN << "PASSED!";

		cout << "Writing data to that temporary file...";

		TESTLU(fp.write(down.read().data(), down.read().size()), "Couldn't write all data into temporary file.");
		TESTLU(fp.flush(), "Couldn't flush temporary file after write.");
		fp.close();

		cout << console::color::GREEN << "PASSED!";

		cout << "Reopening, reading and checking...";

		TESTLU(fp.open(temp_local_file_path, file::open_mode_e::READ_TRY), "Failed to open the local file.");

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

			fp.close();
		}


		{
			cout << console::color::LIGHT_PURPLE << "Testing 'memfile'...";
			cout << "Creating memfile...";

			const size_t randomsize = 1024 * 1024;
			memfile memfp;
			std::string memmycpy;
			TESTLU(memfp.open(randomsize), "Failed to create a memfile.");

			cout << console::color::GREEN << "PASSED!";

			cout << "Writing data to memfile...";

			for (size_t p = 0; p < randomsize; p++) {
				char ch = '0' + random() % 10;
				memfp.write(&ch, 1);
				memmycpy += ch;
			}

			TESTLU(memfp.flush(), "Couldn't flush memfile after write.");
			memfp.seek(0, file::seek_mode_e::BEGIN);

			cout << console::color::GREEN << "PASSED!";

			{
				std::vector<char> _tmpvec;
				char buftemp[256];
				while (1) {
					size_t ree = memfp.read(buftemp, 256);
					if (ree > 256) {
						cout << console::color::RED << "FATAL ERROR! Read more than asked?!";
						return 1;
					}
					else if (ree > 0) _tmpvec.insert(_tmpvec.end(), std::begin(buftemp), std::begin(buftemp) + ree);
					else break;
				}

				cout << console::color::GREEN << "PASSED!";

				cout << "Comparing data read from memfile...";

				const auto _res3 = sha256(_tmpvec);
				const auto _res4 = sha256(memmycpy);

				cout << "SHA256 of this random stuff: " << console::color::AQUA << _res3;
				TESTLU(_res3 == _res4, "Hash don't match expected? Bad thing!");

				cout << console::color::GREEN << "PASSED!";

				memfp.close();
			}
		}
	}

	cout << console::color::LIGHT_PURPLE << "Testing 'hash' (comparing SHA256 to supermess)...";
	{
		cout << "Creating random string of characters...";

		std::vector<char> rngg;
		for (size_t p = 0; p < (1 << 16); p++)
		{
			rngg.push_back(static_cast<char>(random() % 0xFF));
		}

		cout << "Checking the time to hash with SHA256 once...";

		auto rn1 = std::chrono::high_resolution_clock::now();
		const auto __s256 = sha256(rngg);
		auto l81 = std::chrono::high_resolution_clock::now();

		cout << "Now a messy string one...";
		auto rn2 = std::chrono::high_resolution_clock::now();
		const auto __smes = encrypt_supermess_auto(rngg);
		auto l82 = std::chrono::high_resolution_clock::now();

		cout << "SHA256 took: " << std::chrono::duration_cast<std::chrono::duration<double>>(l81 - rn1).count() * 1e6 << " us";
		cout << "SPMESS took: " << std::chrono::duration_cast<std::chrono::duration<double>>(l82 - rn2).count() * 1e6 << " us";

		cout << console::color::YELLOW << "PASSED?";
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
				cout << "Host's client got connection info: " << tcp_host_client.info().format();
			}, thread::speed::ONCE);

			cout << "Trying to connect to host...";

			TESTLU(tcp_client.setup(socket_config().set_family(socket_config::e_family::IPV4).set_port(12345)), "Can't connect to itself!");

			cout << "Main client got connection info: " << tcp_client.info().format();
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

				cout << console::color::GREEN << "PASSED! (timedout user, so assuming good)";
			}

			cout << console::color::LIGHT_PURPLE << "Testing 'safe_data'...";
			{
				cout << "Setting up many threads trying to access the same sensitive data...";

				const size_t running_time_max = 5;
				const size_t numthreads = 32;
				safe_data<std::string> safer;

				std::atomic<unsigned long long> num_times[numthreads];
				std::atomic<bool> startline = false;
				thread my_threads[numthreads];

				for (size_t p = 0; p < numthreads * 0.5f; p++) {
					my_threads[p].task_async([p, &num_times, &safer, &startline] {
						while (!startline) std::this_thread::yield();
						safer.set("value = " + std::to_string(random()));
						++num_times[p];
					});
				}
				for (size_t p = numthreads * 0.5; p < numthreads; p++) {
					my_threads[p].task_async([p, &num_times, &safer, &startline] {
						while (!startline) std::this_thread::yield();
						std::string res = safer.read();
						if (!res.empty()) ++num_times[p];
					});
				}
				cout << "Waiting a second before starting...";

				std::this_thread::sleep_for(std::chrono::seconds(1));

				cout << "Running threads for " << running_time_max << " seconds...";

				startline = true;
				std::this_thread::sleep_for(std::chrono::seconds(running_time_max));
				for (auto& i : my_threads) i.signal_stop();

				cout << "Waiting threads to stop";
				for (auto& i : my_threads) i.join();

				cout << "These are the results:";

				for (size_t p = 0; p < numthreads * 0.5f; p++) {
					cout << "Thread #" << p << ": (write) " << console::color::AQUA << (num_times[p] / running_time_max) << console::color::DARK_GRAY << " ips"; // iterations per sec
				}
				for (size_t p = numthreads * 0.5; p < numthreads; p++) {
					cout << "Thread #" << p << ": (read) " << console::color::LIGHT_PURPLE << (num_times[p] / running_time_max) << console::color::DARK_GRAY << " ips"; // iterations per sec
				}

				for (const auto& i : num_times) {
					TESTLU(i != 0, "One of the threads got ZERO. That's not good.");
				}

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

			cout << "Testing random struct comparison...";

			struct __random_struct {
				unsigned long long ra = random(), rb = random();
			};

			__random_struct my, me;

			package pack_t_sen;
			pack_t_sen.import_as_data((char*)&my, sizeof(my));

			tcp_host_client.send(pack_t_sen);

			package pack_t_rec = tcp_client.recv(sizeof(me));
			pack_t_rec.read_as_data((char*)&me, sizeof(me));

			TESTLU((me.ra == my.ra && me.rb == my.rb), "Raw package messed up.");

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

		proc.hook_stdout([&](const std::string& out, const process::message_type& typ) {
			switch (typ) {
			case process::message_type::START:
				cout << console::color::DARK_GREEN << "[BEGIN] " << out;
			break;
			case process::message_type::APP_OUTPUT:
				cout << console::color::DARK_GREEN << "[RUNNG] " << out;
			break;
			case process::message_type::ENDED:
				cout << console::color::DARK_GREEN << "[ENDED] " << out;
			break;
			}
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

		cout << "Trying something complex...";

		// if all of this compile, good!
		class example_complex {
			hybrid_memory<int> lol;
			char lel;
			double lil;
		public:
			example_complex(hybrid_memory<int>&& a, const char b, const double& c)
				: lol(std::move(a)), lel(b), lil(c)
			{
			}
		};

		hybrid_src = make_hybrid<int>();

		auto handlr = make_hybrid<example_complex>(std::move(hybrid_src), 'c', 3.1415);

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
	tempfile audio_file;

	TESTLU(audio_file.open("lunaris_debug_XXXXX.ogg"), "Can't create temporary file for Audio testing!");

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

	TESTLU(my_sample.load(audio_file.get_path()), "Could not load temporary audio file named '" + audio_file.get_path() + "'!");

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
		cout << "PAN: " << dt;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	//my_track.stop();
	my_track.set_pan(ALLEGRO_AUDIO_PAN_NONE);

	cout << "[!] Testing mute on sub-mixer (tell me if it works or not)";

	time_after = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while (std::chrono::system_clock::now() < time_after) {
		const bool mt = cos(al_get_time() * 3.0) > 0.0;
		my_submixer.mute(mt);
		cout << "MUTE: " << (mt ? "ON" : "OFF");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	my_submixer.mute(false);

	cout << "[!] Testing volume on sub-mixer (tell me if it works or not)";

	time_after = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while (std::chrono::system_clock::now() < time_after) {
		const float dt = cos(al_get_time() * 3.0) * 0.5f + 0.5f;
		my_submixer.set_gain(dt);
		cout << "VOL: " << static_cast<int>(100.0 * dt) << "%";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	my_submixer.set_gain(1.0f);

	cout << "[!] Testing volume on main mixer (tell me if it works or not)";

	time_after = std::chrono::system_clock::now() + std::chrono::seconds(5);

	while (std::chrono::system_clock::now() < time_after) {
		const float dt = sin(al_get_time() * 3.0) * 0.5f + 0.5f;
		my_mixer.set_gain(dt);
		cout << "VOL: " << static_cast<int>(100.0 * dt) << "%";
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
			cout << "MUTE: " << (mt ? "ON" : "OFF");
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

	const float fixprop = 1.0f; // global camera proportion

	std::atomic<bool> keep_running_things = true;
	display_async my_display;
	block blk_fixed, blk_mouse, topleft_dc;
	text txt_main;
	thread col_and_tools;
	mouse mousing(my_display);
	keys kb;
	float off_x = 0.0f, off_y = 0.0f, zuum = 1.0f;
	collisionable cols[2] = { {blk_mouse}, {blk_fixed} };
	const color no_collision = color(127, 255, 127);
	const color has_collision = color(255, 127, 127);
	const color mouse_no_collision = color(127, 255, 255);
	const color mouse_has_collision = color(255, 127, 255);
	//auto random_texture = make_hybrid<texture>();
	auto font_u = make_hybrid<font>();
	auto ffbmp = make_hybrid_derived<texture, texture_functional>(); // self changing texture
	auto bmppp = make_hybrid<texture>(); // random img from internet
	auto giffye = make_hybrid_derived<texture, texture_gif>(); // gif
	auto tempfp = make_hybrid_derived<file, tempfile>(); // random file
	tempfile* fp = (tempfile*)tempfp.get();
	auto tempfp2 = make_hybrid_derived<file, tempfile>(); // random file 2
	tempfile* fp2 = (tempfile*)tempfp2.get();
	vertexes polygony;

	cout << "Creating display...";

	my_display.post_task_on_destroy([&] {
			font_u.reset_shared();
			ffbmp.reset_shared();
			bmppp.reset_shared();
			giffye.reset_shared();
		});

	TESTLU(my_display.create(display_config()
		.set_fullscreen(false)
		.set_display_mode(display_options().set_width(1800).set_height(900))
		.set_window_title("GRAPHICS TEST")
		.set_extra_flags(ALLEGRO_OPENGL | ALLEGRO_RESIZABLE)
		.set_framerate_limit(300)
		.set_economy_framerate_limit(20)
		.set_wait_for_display_draw(true)
		//.set_fullscreen(true)
	), "Failed to create the display");

	{
		texture_gif* oop = (texture_gif*)giffye.get();

		cout << "Opening temporary file for temporary GIF...'";

		TESTLU(fp->open("lunaris_XXXXX.gif"), "Failed to open temp file.");
		TESTLU(fp2->open("lunaris_XXXXX.jpg"), "Failed to open temp file.");

		cout << "Downloading random image from '" << fixed_my_catto_GIF_url << "'";

		{
			downloader down;
			TESTLU(down.get_store(fixed_my_catto_GIF_url, [&](const char* buf, size_t len) { if (!fp->write(buf, len)) { cout << console::color::RED << "FATAL ERROR WRITING TO FILE! ABORT!"; std::terminate(); } }), "Failed to download random image.");
		}

		cout << "Downloading random image from '" << random_img_url << "'";

		{
			downloader down;
			TESTLU(down.get_store(random_img_url, [&](const char* buf, size_t len) { if (!fp2->write(buf, len)) { cout << console::color::RED << "FATAL ERROR WRITING TO FILE! ABORT!"; std::terminate(); } }), "Failed to download random image.");
		}

		cout << "Temporary image files perfectly saved at '" << fp->get_path() << "' and '" << fp2->get_path() << "'";
		TESTLU(tempfp->flush(), "Cannot flush GIF file");
		TESTLU(tempfp2->flush(), "Cannot flush JPG file");


		cout << "Loading image like memory...";
		TESTLU(oop->load(tempfp), "Could not load GIF");
		TESTLU(bmppp->load(tempfp2), "Could not load JPG");

		cout << "GIF has avg=" << oop->get_interval_average() * 1000.0 << "ms;max=" << oop->get_interval_longest() * 1000.0 << "ms;min=" << oop->get_interval_shortest() * 1000.0 << "ms interval info.";

		topleft_dc.texture_insert(giffye);
		topleft_dc.texture_insert(bmppp);
		//topleft_dc.set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, color(1.0f, 1.0f, 1.0f, 0.01f));
		topleft_dc.set<float>(enum_sprite_float_e::POS_X, 0.7f);
		topleft_dc.set<float>(enum_sprite_float_e::POS_Y, -0.7f);
		topleft_dc.set<float>(enum_sprite_float_e::SCALE_G, 0.4f);
		topleft_dc.set<float>(enum_sprite_float_e::SCALE_X, 1.2f);
		topleft_dc.set<double>(enum_block_double_e::DRAW_FRAMES_PER_SECOND, 3.0);
		topleft_dc.set<color>(enum_sprite_color_e::DRAW_TINT, color(0.7f, 0.7f, 0.7f, 0.7f));
		topleft_dc.set<bool>(enum_sprite_boolean_e::DRAW_USE_COLOR, true);
		//topleft_dc.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, true);
		//topleft_dc.set<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX, true);
	}

	cout << "Setting up some variables...";

	blk_fixed.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, false);
	blk_mouse.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, false);
	txt_main.set<bool>(enum_sprite_boolean_e::DRAW_SHOULD_DRAW, false);

	txt_main.set<text::safe_string>(enum_text_safe_string_e::STRING, std::string("Test string text"));
	txt_main.set<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE, true); // deform pos fix
	txt_main.set<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_NO_EFFECT_ON_SCALE, true); // deform scale fix
	txt_main.set<float>(enum_sprite_float_e::SCALE_G, 0.1f);
	txt_main.set<float>(enum_sprite_float_e::SCALE_X, 0.3f);
	//txt_main.set<float>(enum_sprite_float_e::POS_X, -0.992f);
	//txt_main.set<float>(enum_sprite_float_e::POS_Y, -0.992f);
	txt_main.set<float>(enum_sprite_float_e::POS_X, -0.992f);
	txt_main.set<float>(enum_sprite_float_e::POS_Y, -0.992f);
	//txt_main.set<float>(enum_text_float_e::DRAW_UPDATES_PER_SEC, 0.0f);
	//txt_main.set<float>(enum_text_float_e::DRAW_RESOLUTION, 0.5f);
	txt_main.set<float>(enum_sprite_float_e::DRAW_MOVEMENT_RESPONSIVENESS, 3.0f);
	for (int __c = 1; 255 - 25 * __c > 0; __c++) {
		int ctee = (255 - 25 * __c);
		txt_main.shadow_insert(text_shadow(0.0001f * __c, 0.007f * __c, color(ctee / 10, ctee / 10, ctee / 10, ctee)));
	}

	blk_mouse.set<float>(enum_sprite_float_e::SCALE_G, 0.25f);
	blk_mouse.set<float>(enum_sprite_float_e::POS_X, -0.3f);
	blk_mouse.set<float>(enum_sprite_float_e::POS_Y, -0.3f);
	blk_mouse.set<float>(enum_sprite_float_e::OUT_OF_SIGHT_POS, 0.8f);
	blk_mouse.set<bool>(enum_sprite_boolean_e::DRAW_THINK_BOX, true);

	blk_fixed.set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, color(255,255,255));
	blk_fixed.set<bool>(enum_sprite_boolean_e::DRAW_DRAW_BOX, true);
	blk_fixed.set<float>(enum_sprite_float_e::SCALE_G, 0.5f);
	blk_fixed.set<bool>(enum_sprite_boolean_e::DRAW_TRANSFORM_COORDS_KEEP_SCALE, true); // deform pos

	{
		texture_functional* ftt = (texture_functional*)ffbmp.get();
		TESTLU(my_display.post_task([&] {return ftt->create(512, 512); }).get(), "Couldn't create texture/font for test!");
		ftt->hook_function([](texture& self) {
			const float dtim = static_cast<float>(al_get_time());
			al_draw_filled_rectangle(0, 0, self.get_width(), self.get_height(), color(0.6f + 0.8f * cosf(dtim * 1.3f), 0.6f + 0.8f * cosf(dtim * 0.57f + 0.8754f), 0.6f + 0.8f * cosf(dtim * 2.25f + 1.8896f)));
		});
	}

	polygony.push_back(vertex_point{ -0.9f, -0.9f, 0.0f, 0.0f, 0.0f, color(255,150,150) });
	polygony.push_back(vertex_point{ -0.7f, -0.9f, 0.0f, 512.0f, 0.0f, color(150,255,150) });
	polygony.push_back(vertex_point{ -0.7f, -0.7f, 0.0f, 512.0f, 512.0f, color(150,150,255) });
	polygony.push_back(vertex_point{ -0.9f, -0.7f, 0.0f, 0.0f, 512.0f, color(255,150,255) });
	polygony.set_mode(vertexes::types::TRIANGLE_STRIP);
	polygony.set_texture(ffbmp);

	cout << "Applying default transformation to display...";

	my_display.add_run_once_in_drawing_thread([&my_display,&fixprop] {
		transform transf;
		transf.build_classic_fixed_proportion(my_display.get_width(), my_display.get_height(), fixprop, 1.0f);
		transf.apply();
		return true;
	});

	cout << "Loading texture in video memory and default font...";

	{
		auto dod = my_display.add_run_once_in_drawing_thread([&] {
			return /*random_texture->load(fp.get_current_path()) && */font_u->create_builtin_font();
		});

		dod.wait();
		TESTLU(dod.get(), "Couldn't load texture/font for test!");
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

	double __time_count = 0.0;
	size_t loops_call = 0;

	my_display.hook_draw_function([&](const auto& _u) {
		al_clear_to_color(al_map_rgb(
			cos(al_get_time() * 0.4111) * 100 + 150,
			sin(al_get_time() * 0.2432) * 100 + 150,
			cos(al_get_time() * 0.5321) * 100 + 150
		));

		blk_fixed.draw();
		blk_mouse.draw();
		txt_main.draw();
		polygony.draw();

		topleft_dc.draw();

		if (al_get_time() - __time_count > 10.0) {
			__time_count = al_get_time();
			cout << "Display loops: " << loops_call * 0.1;
			loops_call = 0;
		}
		++loops_call;

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

	display_event_handler dispevh(my_display);

	dispevh.hook_event_handler([&keep_running_things, &off_x, &off_y, &zuum, &fixprop](display_event& ev) {
		cout << console::color::AQUA << "DISPLAY EVENT: " << console::color::BLUE << "Event #" << ev.get_type() << " triggered.";

		if (ev.is_close()) {
			cout << console::color::GREEN << "Closing app...";
			keep_running_things = false;
		}
		else if (ev.is_resize()) {
			cout << console::color::GREEN << "Screen size is now: " << ev.as_display().width << "x" << ev.as_display().height << ". Posted task.";
			ev.post_task([wd = ev.as_display().width, ht = ev.as_display().height, fixprop, zuum, off_x, off_y] {
				transform transf;
				transf.build_classic_fixed_proportion(wd, ht, fixprop, zuum);
				transf.translate_inverse(off_x, off_y);
				cout << console::color::DARK_GREEN << "Camera at " << off_x << " x " << off_y << " * " << zuum;
				transf.apply();
				return true;
			});
		}
	});

	dispevh.hook_exception_handler([](const std::exception& e) {
		cout << console::color::RED << "DISPLAY EVENT EXCEPTION: " << e.what();
	});


	kb.hook_event([&](const keys::key_event& ev) {
		if (!ev.down) return;

		switch(ev.key_id) {
		case ALLEGRO_KEY_F11:
			my_display.toggle_flag(ALLEGRO_FULLSCREEN_WINDOW);

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			break;
		case ALLEGRO_KEY_R:
			cout << console::color::GREEN << "Randomizing camera...";
			off_x = (random() % 1000) * 0.001f - 0.5f;
			off_y = (random() % 1000) * 0.001f - 0.5f;
			my_display.add_run_once_in_drawing_thread([off_x, off_y, zuum, fixprop] {
				transform transf;
				transf.build_classic_fixed_proportion_auto(fixprop, zuum);
				transf.translate_inverse(off_x, off_y);
				cout << console::color::DARK_GREEN << "Camera at " << off_x << " x " << off_y << " * " << zuum;
				transf.apply();
				return true;
			});
			break;
		case ALLEGRO_KEY_F:
			cout << console::color::GREEN << "Randomizing zoom...";
			zuum = 1.0f + (random() % 1000) * 0.002f - 0.5f;
			my_display.add_run_once_in_drawing_thread([off_x, off_y, zuum, fixprop] {
				transform transf;
				transf.build_classic_fixed_proportion_auto(fixprop, zuum);
				transf.translate_inverse(off_x, off_y);
				cout << console::color::DARK_GREEN << "Camera at " << off_x << " x " << off_y << " * " << zuum;
				transf.apply();
				return true;
			});
			break;
		case ALLEGRO_KEY_0:
			cout << console::color::GREEN << "Zeroing camera...";
			off_x = 0.0f;
			off_y = 0.0f;
			zuum = 1.0f;
			my_display.add_run_once_in_drawing_thread([off_x, off_y,zuum, fixprop] {
				transform transf;
				transf.build_classic_fixed_proportion_auto(fixprop, 1.0f);
				transf.translate_inverse(off_x, off_y);
				cout << console::color::DARK_GREEN << "Camera at " << off_x << " x " << off_y << " * " << zuum;
				float ax = -1.0f, ay = -1.0f;
				transf.transform_inverse_coords(ax, ay);
				cout << console::color::DARK_GREEN << "Converted inverse coords " << fabsf(ax) << " x " << fabsf(ay);
				transf.apply();
				return true;
			});
			break;
		}
	});

	cout << "Setting up collision & extra tasks thread...";
	
	for (auto& i : cols) i.set_work([&](collisionable::result res, sprite& one) {
		one.set<color>(enum_sprite_color_e::DRAW_DRAW_BOX, res.dir_to != 0 ? has_collision : no_collision);
		//if (data != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_ROTATION, was_clockwise ? 0.1f : -0.1f);
		if (res.dir_to != 0) {
			if ((res.dir_to & static_cast<int>(collisionable::direction_combo::DIR_NORTH)) != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_Y, 0.005f);
			if ((res.dir_to & static_cast<int>(collisionable::direction_combo::DIR_SOUTH)) != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_Y, -0.005f);
			if ((res.dir_to & static_cast<int>(collisionable::direction_combo::DIR_EAST))  != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_X, -0.005f);
			if ((res.dir_to & static_cast<int>(collisionable::direction_combo::DIR_WEST))  != 0) one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_X, 0.005f);
			one.set<float>(enum_sprite_float_e::RO_THINK_SPEED_ROTATION, res.moment_dir);
		}
		//if (res.dir_to != 0) {
		//	if (res.is_dir(collisionable::direction_op::DIR_NORTH)) cout << " COL NORTH #" << (size_t)((void*)&i);
		//	if (res.is_dir(collisionable::direction_op::DIR_SOUTH)) cout << " COL SOUTH #" << (size_t)((void*)&i);
		//	if (res.is_dir(collisionable::direction_op::DIR_EAST))  cout << " COL EAST  #" << (size_t)((void*)&i);
		//	if (res.is_dir(collisionable::direction_op::DIR_WEST))  cout << " COL WEST  #" << (size_t)((void*)&i);
		//}
		one.think(); 
	});


	col_and_tools.task_async([&] {

		work_all_auto(std::begin(cols), std::end(cols));

		// just clipboard.
		//if (my_display.check_has_clipboard()) {
		//	std::string cpy = my_display.get_clipboard();
		//
		//	cout << console::color::GREEN << "CLIPBOARD!: " << cpy;
		//}

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
				std::to_string(static_cast<unsigned long long>(blk_mouse.get<float>(enum_sprite_float_e::ROTATION) * 180.0f / static_cast<float>(ALLEGRO_PI)) % 360) + " or " + 
				std::to_string(static_cast<unsigned long long>(blk_mouse.get<float>(enum_sprite_float_e::RO_DRAW_PROJ_ROTATION) * 180.0f / static_cast<float>(ALLEGRO_PI)) % 360) + " (proj/smooth)";

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

	col_and_tools.join();
	txt_main.set<text::safe_string>(enum_text_safe_string_e::STRING, std::string("Closing..."));

	my_display.hook_draw_function([](const auto&) {std::this_thread::sleep_for(std::chrono::milliseconds(100)); }); // change
	my_display.add_run_once_in_drawing_thread([&] {
		font_u->destroy();
		bmppp->destroy();
		giffye->destroy();
		return true;
	}).wait();


	//my_display.destroy();
	// the rest should be fine.

	my_display.destroy().wait();

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
	TESTLU(disp.create(display_config().set_display_mode(display_options().set_width(640).set_height(480)).set_fullscreen(false).set_use_basic_internal_event_system(true).set_auto_economy_mode(false).set_window_title("EVENTS_TEST")), "Could not create a simple display for testing!");

	int last_event_id = 0;

	generic_event_handler event_generic;
	event_generic.hook_event_handler([&](const ALLEGRO_EVENT& ev) {
		last_event_id = static_cast<int>(ev.type);
	});


	event_generic.install_other(disp.get_event_sources());

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

	menu mymenu( disp, {
		menu_each_menu()
			.set_name("This one")
			.set_id(9900)
			.push(menu_each_default()
				.set_name("Click me")
				.set_id(910)
			)
			.push(menu_each_empty()
			)
			.push(menu_each_menu()
				.set_name("Not this one")
				.set_id(9901)
				.push(menu_each_default()
					.set_name("Hey")
					.set_id(301)
					.set_flags(ALLEGRO_MENU_ITEM_CHECKBOX)
				)
				.push(menu_each_default()
					.set_name("Lists")
					.set_id(302)
					.set_flags(ALLEGRO_MENU_ITEM_CHECKBOX)
				)
				.push(menu_each_menu()
					.set_name("Exist?")
					.set_id(9902)
					.push(menu_each_default()
						.set_name("LOL")
						.set_id(300)
					)
				)
			),
		menu_each_menu()
			.set_name("About")
			.set_id(9903)
			.push(menu_each_default()
				.set_name("Copyright? Probably not")
				.set_id(520)
			)
			.push(menu_each_empty()
			)
			.push(menu_each_menu()
				.set_name("Others")
				.set_id(9904)
				.push(menu_each_default()
					.set_name("Lunaris B" + std::to_string(LUNARIS_BUILD_NUMBER))
					.set_id(540)
					.set_flags(ALLEGRO_MENU_ITEM_DISABLED)
				)
				.push(menu_each_default()
					.set_name("Beta testing")
					.set_id(550)
					.set_flags(ALLEGRO_MENU_ITEM_DISABLED)
				)
			),
		menu_each_menu()
			.set_name("Crash app")
			.set_id(112)
			.push(menu_each_menu()
				.set_name("Fatal crash call lol")
				.set_id(9906)
				.push(menu_each_menu()
					.set_name("Do you really want a fatal crash?")
					.set_id(9907)
					.push(menu_each_menu()
						.set_name("Okay, click this and the app will just DIE")
						.set_id(9908)
						.push(menu_each_menu()
							.set_name("Do you really want that? LOL")
							.set_id(9909)
							.push(menu_each_default()
								.set_name("STD::TERMINATE()")
								.set_id(911)
							)
						)
					)
				)
			),
		menu_each_menu()
			.set_name("LUL")
			.set_id(12344)
			.push(menu_each_default()
				.set_name("Keeps toggling lol")
				.set_id(12345)
			),
		menu_each_menu()
			.set_name("This is empty")
			.set_id(3696),
		menu_each_menu()
			.set_name("List of times increasing...")
			.set_id(3699)
		});
	{

		//);


		bool clicked_right = false;

		const auto menu_f = [&](menu_event& mev) {
			clicked_right |= (mev.get_name() == "Click me");
			cout << mev.get_id() << " -> '" << mev.get_name() << "'";
			if (mev.get_id() == 911) std::terminate(); // fatal error thingy
			if (!clicked_right && (mev.get_id() != 3696 && mev.get_id() != 3699)) {
				mev.patch_toggle_flag(menu_flags::DISABLED);
				mev.patch_name("This was not it. Random number voila: " + std::to_string(random()));
			}
			//mymenu.apply(); // test if works
		};

		menu_event_handler menuev(mymenu);
		mymenu.show();
		menuev.hook_event_handler(menu_f);

		cout << console::color::YELLOW << "Please go through the menu and select 'This one' -> 'Click me'.";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(180);
		while (std::chrono::system_clock::now() < timeoutt && !clicked_right) { 
			disp.flip();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			mymenu.find_id(112).set_caption("Crash app (rng: " + std::to_string(random() % 1000) + ")");
			mymenu.find_id(12345).toggle_flags(menu_flags::DISABLED);
		}
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test menus. FAILED.");
		cout << console::color::GREEN << "Good!";

		mymenu.hide();
		disp.flip();

		cout << console::color::YELLOW << "Do it again, please.";
		clicked_right = false;
		
		auto mymenu2 = mymenu.duplicate_as(menu::menu_type::POPUP);

		menu_event_handler menuev2(mymenu2);
		mymenu2.show();
		menuev2.hook_event_handler(menu_f);
		
		cout << console::color::YELLOW << "Please go through the menu and select 'This one' -> 'Click me'.";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(180);
		while (std::chrono::system_clock::now() < timeoutt && !clicked_right) { 

			menu_handler submen = mymenu2["List of times increasing..."];
			if (submen.size() > 5) submen.pop_front();
			submen.push(menu_each_default().set_name("Yolo Time: " + std::to_string(al_get_time())).set_flags(menu_flags::DISABLED));

			disp.flip();
			mymenu2.show();

			//mymenu2.show();
			std::this_thread::sleep_for(std::chrono::milliseconds(1500)); }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test menus. FAILED.");
		cout << console::color::GREEN << "Good!";

		mymenu2.hide();
		disp.flip();
	}


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
		while (std::chrono::system_clock::now() < timeoutt && (last_string != "LunARiS")) { std::this_thread::sleep_for(std::chrono::milliseconds(500)); cout << "Currently detected: " << curr_string; }
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
		while (std::chrono::system_clock::now() < timeoutt && mousse.current_mouse().relative_posx > -0.9f) { std::this_thread::sleep_for(std::chrono::milliseconds(250)); cout << "RELPOSX: " << mousse.current_mouse().relative_posx; }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!                          ";
		++step;

		cout << console::color::YELLOW << "Ok, now to the right side!";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && mousse.current_mouse().relative_posx < 0.9f) { std::this_thread::sleep_for(std::chrono::milliseconds(250)); cout << "RELPOSX: " << mousse.current_mouse().relative_posx; }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!                          ";
		++step;

		cout << console::color::YELLOW << "Now to the top!";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && mousse.current_mouse().relative_posy > -0.9f) { std::this_thread::sleep_for(std::chrono::milliseconds(250)); cout << "RELPOSY: " << mousse.current_mouse().relative_posy; }
		TESTLU(std::chrono::system_clock::now() < timeoutt, "TIMED OUT! Couldn't test last event properly. FAILED.");
		cout << console::color::GREEN << "Good!                          ";
		++step;

		cout << console::color::YELLOW << "Now to the bottom!";
		timeoutt = std::chrono::system_clock::now() + std::chrono::seconds(15);
		while (std::chrono::system_clock::now() < timeoutt && mousse.current_mouse().relative_posy < 0.9f) { std::this_thread::sleep_for(std::chrono::milliseconds(250)); cout << "RELPOSY: " << mousse.current_mouse().relative_posy; }
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