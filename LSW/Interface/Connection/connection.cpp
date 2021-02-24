#include "connection.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool ConnectionCore::initialize(const std::string& ip_str, const int port, const connection::connection_type contype)
			{
				if (init) return true;

				if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
				{
					failure = true;
					return false;
				}

				struct addrinfo hints{};

				SecureZeroMemory(&hints, sizeof(hints));
				if (contype != connection::connection_type::CLIENT) hints.ai_family = contype == connection::connection_type::HOST_IPV6 ? AF_INET6 : AF_INET;
				else hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				if (contype != connection::connection_type::CLIENT) hints.ai_flags = AI_PASSIVE;

				// Resolve the server address and port
				if (getaddrinfo(ip_str.empty() ? nullptr : ip_str.c_str(), Tools::sprintf_a("%d", port).c_str(), &hints, &result) != 0) return [&] {failure = true; return false; }();

				init = true;
				return true;
			}

			bool ConnectionCore::as_client(SOCKET& ConnectSocket)
			{
				if (failure || !init) return false;

				struct addrinfo* ptr = NULL;

				// Attempt to connect to an address until one succeeds
				for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

					// Create a SOCKET for connecting to server
					ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
						ptr->ai_protocol);
					if (ConnectSocket == INVALID_SOCKET) return false;

					// Connect to server.
					if (connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
						closesocket(ConnectSocket);
						ConnectSocket = INVALID_SOCKET;
						continue;
					}
					break;
				}

				freeaddrinfo(result);

				if (ConnectSocket == INVALID_SOCKET) return false;
				// connected
				return true;
			}

			bool ConnectionCore::as_host(SOCKET& ListenSocket)
			{
				if (failure || !init) return false;

				ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
				if (ListenSocket == INVALID_SOCKET) return false;

				// Setup the TCP listening socket
				if (bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
					freeaddrinfo(result);
					closesocket(ListenSocket);
					return false;
				}

				freeaddrinfo(result);
				// ready to listen
				return true;
			}

			void NetworkMonitor::__avg_task(nm_transf& n)
			{
				// this function is called once a sec, so current_added is how many bytes were added in the last sec
				n.current_bytes_per_sec = n._bytes_coming.exchange(0);

				n.bytes += n.current_bytes_per_sec;
				if (n.peak_bytes_per_second < n.current_bytes_per_sec) n.peak_bytes_per_second = n.current_bytes_per_sec;
				if (n.first_update_ms == 0) n.first_update_ms = Tools::now();
				n.last_update_ms = Tools::now();
			}

			void NetworkMonitor::_average_thr(const Interface::RawEvent& ev)
			{
				if (ev.type() == ALLEGRO_EVENT_TIMER && ev.timer_event().source == timer_second) {
					__avg_task(sending); // same formula
					__avg_task(recving); // same formula
				}
			}

			void NetworkMonitor::any_add(nm_transf& n, const unsigned long long a)
			{
				n._bytes_coming += a;
			}
			
			unsigned long long NetworkMonitor::any_get_total(const nm_transf& n) const
			{
				return n.bytes;
			}
			
			unsigned long long NetworkMonitor::any_get_peak(const nm_transf& n) const
			{
				return n.peak_bytes_per_second;
			}
			
			unsigned long long NetworkMonitor::any_get_current_bytes_per_second(const nm_transf& n) const
			{
				return n.current_bytes_per_sec;
			}
			
			unsigned long long NetworkMonitor::any_get_average_total(const nm_transf& n) const
			{
				const auto diff_t = (n.last_update_ms - n.first_update_ms);
				return n.bytes / (diff_t ? diff_t : 1); // if never updated, give current bytes
			}

			NetworkMonitor::NetworkMonitor()
			{
				timer_second.set_delta(1.0);
				per_sec_calc.add(timer_second);
				per_sec_calc.set_run_autostart([&](const Interface::RawEvent& ev) { _average_thr(ev); });
				timer_second.start();
			}
			
			NetworkMonitor::~NetworkMonitor()
			{
				timer_second.stop();
				per_sec_calc.stop();
			}

			void NetworkMonitor::clear()
			{
				ping = nm_ping{};
				sending.first_update_ms = sending.last_update_ms = sending.peak_bytes_per_second = sending.bytes = sending.current_bytes_per_sec = 0;
				sending._bytes_coming = 0;
				recving.first_update_ms = recving.last_update_ms = recving.peak_bytes_per_second = recving.bytes = recving.current_bytes_per_sec = 0;
				recving._bytes_coming = 0;
			}

			// ping stuff
			void NetworkMonitor::ping_new(const unsigned a)
			{
				ping.current = a;
				if (ping.adaptative_avg == 0.0) ping.adaptative_avg = a; // jump to number if zero lol
				ping.adaptative_avg = ((1.0 * ping.adaptative_avg * ping.adaptativeness) + ping.current) / (ping.adaptativeness + 1.0);
				if (a > ping.peak) ping.peak = a;
			}
			
			void NetworkMonitor::ping_set_adaptativeness(const double a)
			{
				if (a > 0.0) ping.adaptativeness = a;
			}
			
			unsigned NetworkMonitor::ping_now() const
			{
				return ping.current;
			}

			unsigned NetworkMonitor::ping_peak() const
			{
				return ping.peak;
			}
			
			double NetworkMonitor::ping_average_now() const
			{
				return ping.adaptative_avg;
			}

			// transf send
			void NetworkMonitor::send_add(const unsigned long long a)
			{
				any_add(sending, a);
			}
			
			unsigned long long NetworkMonitor::send_get_total() const
			{
				return any_get_total(sending);
			}
			
			unsigned long long NetworkMonitor::send_get_peak() const
			{
				return any_get_peak(sending);
			}
			
			unsigned long long NetworkMonitor::send_get_current_bytes_per_second() const
			{
				return any_get_current_bytes_per_second(sending);
			}
			
			unsigned long long NetworkMonitor::send_get_average_total() const
			{
				return any_get_average_total(sending);
			}

			// transf recv
			void NetworkMonitor::recv_add(const unsigned long long a)
			{
				any_add(recving, a);
			}
			
			unsigned long long NetworkMonitor::recv_get_total() const
			{
				return any_get_total(recving);
			}
			
			unsigned long long NetworkMonitor::recv_get_peak() const
			{
				return any_get_peak(recving);
			}
			
			unsigned long long NetworkMonitor::recv_get_current_bytes_per_second() const
			{
				return any_get_current_bytes_per_second(recving);
			}
			
			unsigned long long NetworkMonitor::recv_get_average_total() const
			{
				return any_get_average_total(recving);
			}

			Package::Package(Package&& pkg)
			{
				*this = std::move(pkg);
			}

			bool Package::has_data() const
			{
				return small_data.size() || big_file.is_open();
			}

			void Package::operator=(Package&& pkg)
			{
				big_file = std::move(pkg.big_file);
				small_data = std::move(pkg.small_data);
				type = pkg.type;
			}

			Connection::__package::__package(const std::string& str, const __package_type t, const bool en)
			{
				package_type = static_cast<int>(t);
				wait_for_more = en;
				buffer_len = static_cast<int>(str.size() < connection::package_size ? str.size() : connection::package_size);
				std::copy(str.begin(), str.begin() + buffer_len, buffer);
			}


			bool Connection::has_priority_waiting() const
			{
				return send_pkg_small_priority.size();
			}

			Package Connection::get_next_priority_auto()
			{
				Package pkg;

				Tools::AutoLock l(send_pkg_mtx);

				if (send_pkg_small_priority.size()) {
					pkg.small_data = std::move(send_pkg_small_priority.front());
					send_pkg_small_priority.erase(send_pkg_small_priority.begin());
					pkg.type = static_cast<int>(__package_type::PRIORITY_PACKAGE_SMALL);
				}

				return pkg;
			}

			Package Connection::get_next_package_auto()
			{
				Package pkg;

				Tools::AutoLock l(send_pkg_mtx);

				/*if (send_pkg_small_priority.size()) { // handled by get_next_priority_auto()
					pkg.small_data = std::move(send_pkg_small_priority.front());
					send_pkg_small_priority.erase(send_pkg_small_priority.begin());
					pkg.type = static_cast<int>(__package_type::PRIORITY_PACKAGE_SMALL);
				}
				else */
				if (send_pkg_small_normal.size()) {
					pkg.small_data = std::move(send_pkg_small_normal.front());
					send_pkg_small_normal.erase(send_pkg_small_normal.begin());
					pkg.type = static_cast<int>(__package_type::PACKAGE_SMALL);
				}
				else if (send_pkg_file.size()) {
					pkg.big_file = std::move(send_pkg_file.front());
					send_pkg_file.erase(send_pkg_file.begin());
					pkg.type = static_cast<int>(__package_type::PACKAGE_FILE);
				}

				return pkg;
			}

			void Connection::submit_next_package_recv(Package&& pkg)
			{
				if (read_over) {
					Tools::AutoLock lr(read_over_mtx);
					if (read_over) {
						read_over(*this, pkg); // this hadles now
						pkg = Package{}; // reset
						return;
					}
				}

				Tools::AutoLock l(recv_pkg_mtx);
				recv_pkg_normal.push_back(std::move(pkg));
				recv_pkg_signal.signal_one();
			}

			bool Connection::slice_auto(Package& pkg, __package& sml)
			{
				if (!pkg.has_data()) return false;

				if (!pkg.small_data.empty()) {
					auto& easy = pkg.small_data;

					sml.package_type = pkg.type;
					sml.wait_for_more = easy.size() > connection::package_size;
					sml.buffer_len = static_cast<int>(easy.size() > connection::package_size ? connection::package_size : easy.size());
					std::copy(easy.begin(), easy.begin() + sml.buffer_len, sml.buffer);
					easy.erase(easy.begin(), easy.begin() + sml.buffer_len);
				}
				else if (!pkg.big_file.eof()) {
					std::string buf;

					sml.package_type = pkg.type;
					sml.buffer_len = static_cast<int>(pkg.big_file.read(buf, connection::package_size));
					sml.wait_for_more = !pkg.big_file.eof();
					std::copy(buf.begin(), buf.begin() + sml.buffer_len, sml.buffer);
				}

				return true;
			}

			void Connection::format_ping(__package& pkg)
			{
				pkg.wait_for_more = false;
				pkg.package_type = static_cast<int>(__package_type::PING);

				std::string buf = std::to_string(Tools::now());
				std::copy(buf.begin(), buf.end(), pkg.buffer); // sure this is not bigger than package size, no doubt
				pkg.buffer_len = static_cast<int>(buf.size());
			}

			bool Connection::interpret_ping_recv(const __package& pkg)
			{
				if (pkg.package_type != static_cast<int>(__package_type::PONG)) return false;

				unsigned long long _now = Tools::now();

				unsigned long long here{};
				if (sscanf_s(pkg.buffer, "%llu", &here) != 1) return false;

				network_analysis.ping_new(_now - here);
				return true;
			}

			Connection::__package_type Connection::safer_cast_type(const int i)
			{
				if (i < 1 || i > static_cast<int>(__package_type::_FAILED)) return __package_type::_FAILED;
				return static_cast<__package_type>(i);
			}

			bool Connection::manage_status_good(const connection::_connection_status& st)
			{
				switch (st) {
				case connection::_connection_status::DISCONNECTED:
					keep_connection = false;
					closesocket(connected);
					thr_send.stop();
					thr_recv.stop();
					return false;
				case connection::_connection_status::FAILED:
					return false;
				}
				return true;
			}

			void Connection::err_f(const std::string& str)
			{
				debug(str);
				if (!err_debug) return;
				Tools::AutoLock l(err_debug_mtx);
				if (err_debug) err_debug(str);
			}

			void Connection::handle_send(Tools::boolThreadF run)
			{
				const std::string socket_identification = "&1" + Tools::sprintf_a("%05" PRIu64, Tools::get_thread_id()) + "&9SOCKET&8#&6" + Tools::sprintf_a("%05" PRIx64, connected) + " &d - SEND -> &b";
				debug(socket_identification + "&aHANDLE CONNECTION STARTED SEND");

				bool should_ping = true;
				EventTimer timm(connection::pinging_time);
				EventHandler evhdr{ Tools::superthread::performance_mode::EXTREMELY_LOW_POWER };

				evhdr.add(timm);
				evhdr.set_run_autostart([&](const Interface::RawEvent& re) {
					if (re.timer_event().source == timm) {
						should_ping = true;
					}
				});
				timm.start();

				packages_since_sync = 0;

				if (!setup_socket_buffer_send()) {
					debug(socket_identification + "Can't set buffer size");
				}

				std::vector<Package> next_task;

				while (run() && keep_connection) {					
					try {

						if (packages_since_sync >= static_cast<long long>(connection::trigger_sync_send_thread)) // trigger slowdown
						{
							auto diff = connection::trigger_sync_multiplier_slowdown * packages_since_sync - connection::trigger_sync_send_thread;
							if (diff > connection::trigger_max_time_allowed) diff = connection::trigger_max_time_allowed;
							Tools::sleep_for(std::chrono::milliseconds(diff));
						}

						__package go;
						bool good_to_go = false;

						if (packages_received_trigger_sync >= connection::trigger_sync_send_thread) // somewhat important. RECEIVED SOME, SEND UPDATE!
						{
							packages_received_trigger_sync -= connection::trigger_sync_send_thread;

							go.wait_for_more = false;
							go.package_type = static_cast<int>(__package_type::SIGNAL);

							good_to_go = true;
						}
						else if (should_ping) // somewhat important
						{
							should_ping = false;
							format_ping(go);
							good_to_go = true;
						}
						else if (has_priority_waiting() && next_task.size() < connection::limit_packages_stuck_send) // can't be too big!
						{
							next_task.insert(next_task.begin() + (next_task.size() ? (next_task.size() - 1) : 0), std::move(get_next_priority_auto())); // automatic insert
							debug(socket_identification + "PRIORITY GOT AHEAD OF CURRENT TASK. TASKING PRIORITY NOW.");
							send_pkg_signal.signal_one();
						}
						else if (next_task.size() && next_task.front().has_data()) // still has data
						{
							good_to_go = slice_auto(next_task.front(), go);

							if (!next_task.front().has_data()) debug(socket_identification + "ABOUT TO COMPLETE ONE MORE SEND TYPE #" + std::to_string(next_task.front().type));
							//else debug(socket_identification + "SENDING FILE SLICE #" + std::to_string(next_pkg_ready.type));
						}
						else // get next important task
						{
							if (next_task.size()) next_task.erase(next_task.begin());

							if (next_task.empty()) {
								next_task.push_back(std::move(get_next_package_auto()));
								send_pkg_signal.signal_one();
							}
							continue;
						}

						while (good_to_go && !manage_status_good(auto_send(go))) {
							err_f(socket_identification + "&cSEND FAILED!");
							if (!run() || !keep_connection) break;
							Tools::sleep_for(std::chrono::milliseconds(200)); // try again.
						}

						packages_since_sync++;
						
					} // try

					catch (const Handling::Abort& e) {
						//err_broadcast(std::string("SEND Exception: ") + e.what() + "{vals=[count=" + std::to_string(_sending_packages_count_raw) + ";lastsync=" + std::to_string(_sending_packages_confirmed_last) + "]}");
						err_f(socket_identification + "&cException: " + e.what());
					}

					catch (const std::exception& e) {
						//err_broadcast(std::string("SEND Exception: ") + e.what() + "{vals=[count=" + std::to_string(_sending_packages_count_raw) + ";lastsync=" + std::to_string(_sending_packages_confirmed_last) + "]}");
						err_f(socket_identification + "&cException: " + e.what());
					}

					catch (...) {
						//err_broadcast(std::string("SEND Unknown exception: ") + "{vals=[count=" + std::to_string(_sending_packages_count_raw) + ";lastsync=" + std::to_string(_sending_packages_confirmed_last) + "]}");
						err_f(socket_identification + "&cUnknown exception!");
					}
				}

			}

			void Connection::handle_recv(Tools::boolThreadF run)
			{
				const std::string socket_identification = "&1" + Tools::sprintf_a("%05" PRIu64, Tools::get_thread_id()) + "&9SOCKET&8#&6" + Tools::sprintf_a("%05" PRIx64, connected) + " &d<- RECV -  &3";
				debug(socket_identification + "&aHANDLE CONNECTION STARTED RECV");

				Package priority, small, file;

				if (!setup_socket_buffer_recv()) {
					debug(socket_identification + "Can't set buffer size");
				}

				while (run() && keep_connection) {
					try{
						__package go;
												
						if (!manage_status_good(auto_recv(go))) continue;

						packages_received_trigger_sync++;

						auto transl = safer_cast_type(go.package_type);

						switch (transl) {
						case __package_type::PING:
							go.package_type = static_cast<int>(__package_type::PONG);
							if (!manage_status_good(auto_send(go))) err_f(socket_identification + "&cFAILED TO SEND PONG!");
							break;

						case __package_type::PONG:
							if (!interpret_ping_recv(go)) err_f(socket_identification + "&cFAILED TO TASK PING CALCULATION!");
							else debug(socket_identification + "PING GOOD! VALUE=" + std::to_string(network_analysis.ping_now()));
							break;

						case __package_type::ASK_SIGNAL:

							break;

						case __package_type::SIGNAL:
							packages_since_sync -= connection::trigger_sync_send_thread;
							break;

						/*case __package_type::SYNC_SIGNAL:
							if (!manage_status_good(auto_send(__package{ "", __package_type::SYNC_SIGNAL_BACK, true }))) err_f(socket_identification + "&cFAILED TO SEND SYNC_SIGNAL_BACK!"); // SENDER request, RECVER recast to SENDER, SENDER knows it's good there
							break;

						case __package_type::SYNC_SIGNAL_BACK:
							debug(socket_identification + "SYNCED");
							in_sync = true;
							break;*/

						case __package_type::PRIORITY_PACKAGE_SMALL:
							priority.small_data.append(go.buffer, go.buffer_len);
							if (!go.wait_for_more) {
								debug(socket_identification + "COMPLETELY GOT PRIORITY PACKAGE");
								submit_next_package_recv(std::move(priority));
							}
							break;

						case __package_type::PACKAGE_SMALL:
							small.small_data.append(go.buffer, go.buffer_len);
							if (!go.wait_for_more) {
								debug(socket_identification + "COMPLETELY GOT SMALL PACKAGE");
								submit_next_package_recv(std::move(small));
							}
							break;

						case __package_type::PACKAGE_FILE:
							if (!file.big_file.is_open()) {
								if (!file.big_file.open_temp(smartfile::file_modes::WRITE)) {
									err_f(socket_identification + "&cCANNOT CREATE TEPORARY FILE FOR FILE TRANSFER!");
								}
							}
							if (file.big_file.is_writable()) { // everything fine
								std::string temp;
								temp.append(go.buffer, go.buffer_len);
								file.big_file.write(temp);
								//debug(socket_identification + "WRITE " + std::to_string(temp.length()) + " BYTE(S) CALL");
								if (!go.wait_for_more) {
									debug(socket_identification + "COMPLETELY GOT FILE PACKAGE");
									submit_next_package_recv(std::move(file));
								}
							}
							else {
								err_f(socket_identification + "&cWEIRD BROKEN FILE CREATED INTERNALLY. WHAT?!");
								file.big_file.close(); // reset lol wtf
							}
							break;
						}

					} // try

					catch (const Handling::Abort& e) {
						//err_broadcast(std::string("RECV Exception: ") + e.what() + "{vals=[count=" + std::to_string(_sending_packages_count_raw) + ";lastsync=" + std::to_string(_sending_packages_confirmed_last) + "]}");
						err_f(socket_identification + "&cException: " + e.what());
					}

					catch (const std::exception& e) {
						//err_broadcast(std::string("RECV Exception: ") + e.what() + "{vals=[count=" + std::to_string(_sending_packages_count_raw) + ";lastsync=" + std::to_string(_sending_packages_confirmed_last) + "]}");
						err_f(socket_identification + "&cException: " + e.what());
					}

					catch (...) {
						//err_broadcast(std::string("RECV Unknown exception: ") + "{vals=[count=" + std::to_string(_sending_packages_count_raw) + ";lastsync=" + std::to_string(_sending_packages_confirmed_last) + "]}");
						err_f(socket_identification + "&cUnknown exception!");
					}

				}

			}

			bool Connection::setup_socket_buffer_send()
			{
				socklen_t optlen = sizeof(__package_size_buffer);
				auto res1 = setsockopt(connected, SOL_SOCKET, SO_SNDBUF, (char*)&__package_size_buffer, optlen);
				return res1 != -1;
			}

			bool Connection::setup_socket_buffer_recv()
			{
				socklen_t optlen = sizeof(__package_size_buffer);
				auto res2 = setsockopt(connected, SOL_SOCKET, SO_RCVBUF, (char*)&__package_size_buffer, optlen);
				return res2 != -1;
			}

			connection::_connection_status Connection::auto_send(const __package& pkg)
			{
				return ensure_send((char*)&pkg, sizeof(pkg));
			}

			connection::_connection_status Connection::auto_recv(__package& pkg)
			{
				return ensure_recv((char*)&pkg, sizeof(pkg));
			}

			connection::_connection_status Connection::ensure_send(const char* ptr, const int len)
			{
				for (int p = 0; p < len;) {
					int _n = ::send(connected, ptr + p, len - p, 0);
					if (_n == 0) return connection::_connection_status::DISCONNECTED;
					else if (_n < 0) return connection::_connection_status::FAILED;
					p += _n;
				}
				network_analysis.send_add(len);
				return connection::_connection_status::GOOD;
			}

			connection::_connection_status Connection::ensure_recv(char* ptr, const int len)
			{
				for (int p = 0; p < len;) {
					int _n = ::recv(connected, ptr + p, len - p, 0);
					if (_n <= 0) return connection::_connection_status::DISCONNECTED;
					else if (_n < 0) return connection::_connection_status::FAILED;
					p += _n;
				}
				network_analysis.recv_add(len);
				return connection::_connection_status::GOOD;
			}

			void Connection::init()
			{
				keep_connection = true;
				thr_send.set([&](Tools::boolThreadF f) { handle_send(f); });
				thr_recv.set([&](Tools::boolThreadF f) { handle_recv(f); });
				thr_send.start();
				thr_recv.start();
			}

			Connection::Connection(SOCKET socket)
			{
				if (socket != INVALID_SOCKET) {
					connected = socket;
					init();
				}
			}

			Connection::~Connection()
			{
				close();
			}

			bool Connection::connect(const std::string& a, const int b)
			{
				if (!core.initialize(a, b, connection::connection_type::CLIENT)) return false;
				if (!core.as_client(connected)) return false;
				init();
				return true;
			}

			void Connection::close()
			{
				keep_connection = false;
				if (connected != INVALID_SOCKET) {
					::closesocket(connected);
					connected = INVALID_SOCKET;

					thr_send.stop();
					thr_recv.stop();

					thr_send.join();
					thr_recv.join();

					{
						Tools::AutoLock l(send_pkg_mtx);
						send_pkg_small_priority.clear();
						send_pkg_small_normal.clear();
						send_pkg_file.clear();
					}

					{
						Tools::AutoLock l(recv_pkg_mtx);
						recv_pkg_normal.clear();
					}

				}
			}

			bool Connection::is_connected() const
			{
				return connected != INVALID_SOCKET && keep_connection;
			}

			size_t Connection::packages_received() const
			{
				return recv_pkg_normal.size();
			}

			size_t Connection::packages_sending() const
			{
				return send_pkg_file.size() + send_pkg_small_normal.size() + send_pkg_small_priority.size();
			}

			double Connection::buffer_sending_load() const
			{
				return packages_since_sync * 1.0 / connection::trigger_sync_send_thread;
			}

			const NetworkMonitor& Connection::get_network_info() const
			{
				return network_analysis;
			}

			bool Connection::has_package() const
			{
				return recv_pkg_normal.size();
			}

			bool Connection::wait_for_package(const std::chrono::milliseconds t)
			{
				if (has_package()) return true;
				if (read_over) return false; // read_over override this

				if (t.count() == 0) { // UNLIMITED TIME
					while (!has_package()) recv_pkg_signal.wait_signal(200);
				}
				else { // ONE TRY
					if (!has_package()) recv_pkg_signal.wait_signal(t.count());
				}
				return has_package();
			}

			Package Connection::get_next(const bool wait)
			{
				if (wait) wait_for_package();
				else if (!has_package()) return Package{};

				Tools::AutoLock l(recv_pkg_mtx);

				Package got = std::move(recv_pkg_normal.front());
				recv_pkg_normal.erase(recv_pkg_normal.begin());
				return std::move(got);
			}

			size_t Connection::send_priority_package(const std::string& str)
			{
				Tools::AutoLock l(send_pkg_mtx);
				send_pkg_small_priority.push_back(str);
				return send_pkg_small_priority.size();
			}

			size_t Connection::send_package(const std::string& str)
			{
				while (packages_sending() >= connection::limit_packages_stuck_send) send_pkg_signal.wait_signal(100);
				Tools::AutoLock l(send_pkg_mtx);
				send_pkg_small_normal.push_back(str);
				return send_pkg_small_normal.size();
			}

			size_t Connection::send_package(std::string&& str)
			{
				while (packages_sending() >= connection::limit_packages_stuck_send) send_pkg_signal.wait_signal(100);
				Tools::AutoLock l(send_pkg_mtx);
				send_pkg_small_normal.push_back(std::move(str));
				return send_pkg_small_normal.size();
			}

			size_t Connection::send_package(SmartFile&& data)
			{
				while (packages_sending() >= connection::limit_packages_stuck_send) send_pkg_signal.wait_signal(100);
				Tools::AutoLock l(send_pkg_mtx);
				data.seek(0, smartfile::file_seek::BEGIN);
				send_pkg_file.push_back(std::move(data));
				return send_pkg_file.size();
			}

			void Connection::overwrite_reads_to(std::function<void(Connection&, Package&)> f)
			{
				Tools::AutoLock l(read_over_mtx);
				read_over = f;
			}

			void Connection::reset_overwrite_reads()
			{
				Tools::AutoLock l(read_over_mtx);
				read_over = std::function<void(Connection&, Package&)>();
			}

			void Connection::debug_error_function(std::function<void(const std::string&)> f)
			{
				Tools::AutoLock l(err_debug_mtx);
				err_debug = f;
			}

			void Connection::set_mode(const Tools::superthread::performance_mode m)
			{
				if (m == Tools::superthread::performance_mode::_COUNT) return;
				thr_send.set_performance_mode(m);
				thr_recv.set_performance_mode(m);
			}

			void Connection::reset_mode_default()
			{
				thr_send.set_performance_mode(connection::default_performance_connection);
				thr_recv.set_performance_mode(connection::default_performance_connection);
			}

			void Hosting::handle_disconnects(Tools::boolThreadF run)
			{
				while (run() && keep_connection) {
					Tools::AutoLock safe(connections_m);

					for (size_t p = 0; p < connections.size(); p++) {
						if (!connections[p]->is_connected()) {
							//printf_s("\nSomeone has disconnected!");

							if (disconnected_f) disconnected_f((uintptr_t)connections[p].get());
							connections.erase(connections.begin() + p--);
						}
					}
				}
			}

			void Hosting::handle_queue(Tools::boolThreadF run)
			{
				while (run() && keep_connection) {
					if (listen(Listening, SOMAXCONN) == SOCKET_ERROR) continue;

					// Accept a client socket
					SOCKET ClientSocket = ::accept(Listening, NULL, NULL);
					if (ClientSocket == INVALID_SOCKET) continue;

					std::shared_ptr<Connection> dis = std::make_shared<Connection>(ClientSocket);

					if (max_connections_allowed > 0 && connections.size() >= max_connections_allowed) {
						dis->close();
						continue;
					}

					//dis->start_internally_as_host(); // cause sended last time, so should receive so there's no error
					Tools::AutoLock safe(connections_m);

					if (new_connection_f) new_connection_f(dis);

					connections.emplace_back(std::move(dis));

					//printf_s("\nSomeone has connected!");

					connection_event.signal_one();
				}
				closesocket(Listening);
				Listening = INVALID_SOCKET;
			}

			void Hosting::init()
			{
				keep_connection = true;
				handle_thread.set([&](Tools::boolThreadF f) { handle_queue(f); });
				handle_thread.start();
				handle_disc_thread.set([&](Tools::boolThreadF f) { handle_disconnects(f); });
				handle_disc_thread.start();
			}

			Hosting::Hosting(const int port, const bool ipv6)
			{
				core.initialize("", port, ipv6 ? connection::connection_type::HOST_IPV6 : connection::connection_type::HOST_IPV4);
				core.as_host(Listening);
				init();
			}

			Hosting::Hosting(const bool ipv6)
			{
				core.initialize("", connection::default_port, ipv6 ? connection::connection_type::HOST_IPV6 : connection::connection_type::HOST_IPV4);
				core.as_host(Listening);
				init();
			}

			Hosting::~Hosting()
			{
				close();
			}

			size_t Hosting::size() const
			{
				Tools::AutoLock safe(connections_m);
				return connections.size();
			}

			void Hosting::close()
			{
				keep_connection = false;
				if (Listening != INVALID_SOCKET) {
					::closesocket(Listening);
					Listening = INVALID_SOCKET;
					handle_thread.join();
					handle_disc_thread.join();
					Tools::AutoLock luck(connections_m);

					for (size_t p = 0; p < connections.size(); p++) {
						if (disconnected_f) disconnected_f((uintptr_t)connections[p].get());
					}

					connections.clear();
				}
			}

			bool Hosting::is_connected() const
			{
				return keep_connection && Listening != INVALID_SOCKET;
			}

			void Hosting::set_connections_limit(const size_t nl)
			{
				max_connections_allowed = nl;
			}

			std::shared_ptr<Connection> Hosting::get_connection(const size_t p)
			{
				Tools::AutoLock luck(connections_m);
				if (connections.size() <= p) return std::shared_ptr<Connection>();
				return connections[p];
			}

			std::shared_ptr<Connection> Hosting::get_latest_connection()
			{
				Tools::AutoLock luck(connections_m);
				while (connections.size() == 0) {
					luck.unlock();
					connection_event.wait_signal(1000);
					luck.lock();
				}
				return connections.back();
			}

			void Hosting::on_new_connection(std::function<void(std::shared_ptr<Connection>)> f)
			{
				Tools::AutoLock safe(connections_m);
				new_connection_f = f;
			}

			void Hosting::reset_on_new_connection()
			{
				Tools::AutoLock safe(connections_m);
				new_connection_f = std::function<void(std::shared_ptr<Connection>)>();
			}

			void Hosting::on_connection_close(std::function<void(const uintptr_t)> f)
			{
				Tools::AutoLock safe(connections_m);
				disconnected_f = f;
			}

			void Hosting::reset_on_connection_close()
			{
				Tools::AutoLock safe(connections_m);
				disconnected_f = std::function<void(const uintptr_t)>();
			}

			std::string transform_any_to_package(void* data, const size_t size)
			{
				if (!data || !size) return "";
				std::string buf;
				buf.resize(size);
				std::copy((char*)data, (char*)data + size, buf.begin());
				return std::move(buf);
			}

			bool transform_any_package_back(void* data, const size_t size, const std::string& src)
			{
				if (src.size() != size) return false;
				std::copy(src.begin(), src.begin() + size, (char*)data);
				return true;
			}

		}
	}
}