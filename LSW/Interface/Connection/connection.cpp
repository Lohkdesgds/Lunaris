#include "connection.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			__package::__package(const package_type t)
			{
				prepare_to(t);
			}
			__package::__package()
			{
				ZeroMemory(&pack, sizeof(pack));
			}

			void __package::prepare_to(const package_type t)
			{
				ZeroMemory(&pack, sizeof(pack));
				type = t;

				switch (t) { // the "different than 0" ones
				case package_type::DATA:
					pack.data.full = true;
					break;
				case package_type::REQUEST:
					pack.rqst.request = package_type::SYNC; // most of the time it is like this
					break;
				case package_type::SYNC:
					pack.sync.add_availability = 0;
					pack.sync.no_limit = false;
					break;
				case package_type::PING:
					pack.ping.self_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
					break;
				}
			}

			combined_data& combined_data::operator+(const combined_data& comb)
			{
				buffer += comb.buffer;
				is_full |= comb.is_full;
				return *this;
			}

			combined_data& combined_data::operator+=(const combined_data& comb)
			{
				return (*this = *this + comb);
			}

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
			}
			
			void NetworkMonitor::ping_set_adaptativeness(const double a)
			{
				if (a > 0.0) ping.adaptativeness = a;
			}
			
			unsigned NetworkMonitor::ping_now() const
			{
				return ping.current;
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

			long long Connection::add_current_limit(const int add)
			{
				//if (add < 0) return 0; // some error?
				Tools::AutoLock l(buffer_available_send_mtx);
				return (buffer_available_send += add);
			}

			void Connection::pop_current_limit()
			{
				Tools::AutoLock l(buffer_available_send_mtx);
				if (buffer_available_send > 0) buffer_available_send--;
			}

			void Connection::calculate_add_available(const int add)
			{
				if (my_buffer_limit == 0) return;

				Tools::AutoLock l(buffer_available_calculated_mtx);

				buffer_available_calculated += add;
			}

			void Connection::calculate_modify_buffer(const long long to_what_val_buffer_max)
			{
				Tools::AutoLock l(buffer_available_calculated_mtx);

				long long new_val = (to_what_val_buffer_max <= 0 ? 0 : to_what_val_buffer_max);

				if (new_val == 0) {
					my_buffer_limit = 0; // infinite
					buffer_available_calculated = 0;
				}
				else {
					buffer_available_calculated += (new_val - my_buffer_limit);
					my_buffer_limit = new_val;
				}
			}

			long long Connection::calculate_cut_value()
			{
				if (my_buffer_limit == 0) return 0;

				Tools::AutoLock l(buffer_available_calculated_mtx);

				long long cpy = buffer_available_calculated;
				buffer_available_calculated = 0;
				return cpy < 0 ? 0 : cpy;
			}
			
			void Connection::update_myself_package()
			{
				myself.buffer_receive_size = buffer_receive.size();
				myself.buffer_sending_size = buffer_sending.size();

				if (my_buffer_limit != 0) {
					myself.add_availability = calculate_cut_value();
					myself.no_limit = false;
				}
				else {
					myself.add_availability = 0;
					myself.no_limit = true;
				}

				//myself.you_can_send_up_to = myself.you_can_send_up_to > 0 ? (static_cast<long long>(buffer_receive.size()) > myself.you_can_send_up_to ? 0 : myself.you_can_send_up_to - static_cast<long long>(buffer_receive.size())) : -1;
			}
			
			void Connection::set_recv_hold(const bool will)
			{
				u_long iMode = will ? 0 : 1;
				ioctlsocket(connected, FIONBIO, &iMode);
			}

			connection::_connection_status Connection::ensure_send(char* data, const int len)
			{
				if (len <= 0) return connection::_connection_status::EMPTY;
				int fi = 0;
				while (fi != len)
				{
					auto _now = ::send(connected, data + fi, len - fi, 0);
					if (_now < 0) return connection::_connection_status::DISCONNECTED;
					fi += _now;
				}
				network_analysis.send_add(len);

				return connection::_connection_status::GOOD;
			}

			connection::_connection_status Connection::ensure_recv(char* data, const int len)
			{
				if (len <= 0) return connection::_connection_status::EMPTY;
				int fi = 0;
				while (fi != len)
				{
					auto _now = ::recv(connected, data + fi, len - fi, 0);
					if (_now < 0) return connection::_connection_status::EMPTY;
					/*if (_now < 0) {
						auto err = WSAGetLastError();
						if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR) return connection::_connection_status::EMPTY;
						else return connection::_connection_status::DISCONNECTED;
					}*/
					fi += _now;
					if (fi == 0) return connection::_connection_status::EMPTY;
				}
				network_analysis.recv_add(len);

				return connection::_connection_status::GOOD;
			}

			void Connection::handle_connection_send(Tools::boolThreadF run, Tools::SuperThread<>& selfthr)
			{
				// * * * * * * * STARTUP * * * * * * * //
				const std::string socket_identification = "&7SOCKET&8#&1" + Tools::sprintf_a("%08" PRIx64, Tools::get_thread_id()) + " &d - SEND -> &1";
				debug(socket_identification + "&aHANDLE CONNECTION STARTED SEND");

				// - - - - - > VARIABLES < - - - - - //
				EventTimer timm(connection::pinging_time);
				EventHandler evhdr{ Tools::superthread::performance_mode::EXTREMELY_LOW_POWER };
				bool cant_send_first_time = true;
				// control
				bool tasked_once = false;

				// job on going
				combined_data data_working_on;

				// - - - - - > PREPARE/START < - - - - - //
				evhdr.add(timm);
				evhdr.set_run_autostart([&](const Interface::RawEvent& re) {
					if (re.timer_event().source == timm) {
						should_ping = true;
					}
				});
				timm.start();

				while ((tasked_once && keep_connection) || run()) {

					// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
					// * * * * * * * * * * * * * GENERATE DATA * * * * * * * * * * * * * //
					// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

					__package sending;
					tasked_once = true;

					// - - - - - - > SYNC EVENT, PRIORITY < - - - - - - //
					if (should_request_sync)
					{
						debug(socket_identification + "&eREQUEST SYNC");

						should_request_sync = false;

						sending.prepare_to(__package::package_type::REQUEST);
						sending.pack.rqst.request = __package::package_type::SYNC;
					}
					// - - - - - - > SYNC EVENT, PRIORITY < - - - - - - //
					else if (should_sync)
					{
						debug(socket_identification + "&eSYNC");

						should_sync = false;

						sending.prepare_to(__package::package_type::SYNC);
						update_myself_package();
						sending.pack.sync = myself;
					}
					// - - - - - - > PING EVENT, ONCE IN A WHILE < - - - - - - //
					else if (should_ping)
					{
						if (!data_working_on.buffer.empty() || buffer_sending.size()) should_request_sync = is_overloaded(); // task to do, sync

						debug(socket_identification + "&6PING");

						should_ping = false;

						sending.prepare_to(__package::package_type::PING);
					}
					// - - - - - - > OTHER THREAD ASKED FOR A TASK < - - - - - - //
					else if (between.size())
					{
						debug(socket_identification + "&3SHARED");
						{
							Tools::AutoLock l(between_mtx);
							sending = between[0];
							between.erase(between.begin());
						}
					}
					// - - - - - - > HAS JOB, COMPLETE IT FAST < - - - - - - //
					else if (data_working_on.buffer.size()) // still has job to do
					{
						if (is_overloaded()) { // LIMIT REACHED!
							if (cant_send_first_time) {
								should_request_sync = true;
								debug(socket_identification + "&4CAN'T SEND, OVERLOAD?!");
							}
							cant_send_first_time = false;
							tasked_once = false; // STOP SENDING DATA
						}
						else { // LET'S GOO
							cant_send_first_time = true; 

							sending.prepare_to(__package::package_type::DATA);

							pop_current_limit(); // take one from history if there's a limit. Updates CAN_SEND for futher use!
							if (buffer_available_send > 0) debug(socket_identification + " &dhas " + std::to_string(buffer_available_send) + " slot(s)");

							sending.pack.data.remaining = data_working_on.is_full ? (is_overloaded() ? ((data_working_on.buffer.size() - 1) / connection::package_size) : false) : 1; // false if can't send or not full from source
							sending.pack.data.data_len = static_cast<unsigned>(data_working_on.buffer.size() > connection::package_size ? connection::package_size : data_working_on.buffer.size());
							std::copy(data_working_on.buffer.begin(), data_working_on.buffer.begin() + sending.pack.data.data_len, sending.pack.data.buffer);
							data_working_on.buffer.erase(data_working_on.buffer.begin(), data_working_on.buffer.begin() + sending.pack.data.data_len);
							sending.pack.data.full = data_working_on.is_full && data_working_on.buffer.empty();

							//should_request_sync |= sending.pack.data.remaining == 0;
						}
					}
					// - - - - - - > NO JOB PENDING, GENERATE NEW JOB < - - - - - - //
					else if (data_working_on.buffer.empty() && (buffer_sending.size() || send_overwrite)) // no job, prepare next job (if there is one)
					{
						//should_sync = true;

						Tools::AutoLock l2(send_overwrite_mtx); // send_overwrite secure

						debug(socket_identification + "&8DATA");

						// = = = = = If has send_overwrite, try = = = = = //
						if (send_overwrite) {
							data_working_on.buffer = send_overwrite();
						}

						// = = = = = If no send_overwrite or no result, check default = = = = = //
						if (data_working_on.buffer.empty() && buffer_sending.size()) {
							Tools::AutoLock l(buffer_sending_mtx);

							data_working_on = std::move(buffer_sending.front());
							buffer_sending.erase(buffer_sending.begin());
							sent_once_package.signal_all();
						}

						// = = = = = Check any result = = = = = //
						tasked_once = !data_working_on.buffer.empty();
					}
					else { // no task
						tasked_once = false;
						std::this_thread::sleep_for(std::chrono::milliseconds(5)); // relax
					}

					// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
					// * * * * * * * * * * * * * SEND DATA TO HOST * * * * * * * * * * * * * //
					// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

					if (tasked_once) {
						connection::_connection_status res = ensure_send((char*)&sending, sizeof(sending));

						switch (res) {
						case connection::_connection_status::DISCONNECTED:
							keep_connection = false;
							closesocket(connected);
							break;
						case connection::_connection_status::GOOD:
							// one more send!
							myself.sent_count++;
							break;
						}
					}

				}
			}

			void Connection::handle_connection_recv(Tools::boolThreadF run, Tools::SuperThread<>& selfthr)
			{
				// * * * * * * * STARTUP * * * * * * * //
				const std::string socket_identification = "&7SOCKET&8#&1" + Tools::sprintf_a("%08" PRIx64, Tools::get_thread_id()) + " &d<- RECV -  &1";
				debug(socket_identification + "&aHANDLE CONNECTION STARTED RECV");

				// - - - - - > VARIABLES < - - - - - //
				// control
				bool tasked_once = false;

				// job on going
				combined_data data_working_on;

				// - - - - - > PREPARE/START < - - - - - //
				set_recv_hold(false);


				// * * * * * * * WORK * * * * * * * //
				while ((tasked_once && keep_connection) || run()) {
					// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
					// * * * * * * * * * * * * * RETRIEVE DATA * * * * * * * * * * * * * //
					// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


					tasked_once = false;

					__package received;
					connection::_connection_status res = ensure_recv((char*)&received, sizeof(received));

					// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
					// * * * * * * * * * * * * *  WORK ON DATA * * * * * * * * * * * * * //
					// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

					if (has_package()) received_package.signal_all();

					// tabbing a little bit makes things more readable ;P
					switch(res) {
						case connection::_connection_status::DISCONNECTED:
						{
							keep_connection = false;
							closesocket(connected);
						}
						break;

						case connection::_connection_status::EMPTY: // relax
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(5));
						}
						break;

						case connection::_connection_status::GOOD:
						{
							// one more recv!
							myself.received_count++;

							switch (received.type) {

								// - - - - - - > REQUESTING < - - - - - - //
								case __package::package_type::REQUEST:
								{
									// should_sync should not be set to true here, it would loop.
									debug(socket_identification + "&eREQUEST GOT");

									// available now
									switch (received.pack.rqst.request) {
									case __package::package_type::PING:
										should_ping = true;
										break;
									case __package::package_type::SYNC:
										should_sync = true;
										break;
									}
								}
								break;

								// - - - - - - > SYNC TASK, SYNC INFO < - - - - - - //
								case __package::package_type::SYNC:
								{
									// should_sync should not be set to true here, it would loop.
									debug(socket_identification + "&eSYNCED");

									yourself = received.pack.sync;

									Tools::AutoLock l(buffer_available_calculated_mtx, false);

									if (yourself.no_limit) {
										l.lock();
										if (buffer_available_send >= 0) {
											debug(socket_identification + " &dadded unlimited slots.");
										}
										buffer_available_send = -1;
									}
									else {
										l.lock();
										if (buffer_available_send < 0) buffer_available_send = 0;
										l.unlock();
										auto ref = add_current_limit(yourself.add_availability);
										debug(socket_identification + " &dadded slots, " + std::to_string(ref) + " now");
									}
								}
								break;

								// - - - - - - > GOT PING, RETURN PONG! < - - - - - - //
								case __package::package_type::PING:
								{
									//should_sync = true; // ping will refresh sides any time
									debug(socket_identification + "&6PONG");

									__package _sending;
									_sending.prepare_to(__package::package_type::PONG);
									_sending.pack.pong.self_time_ms = received.pack.ping.self_time_ms;

									Tools::AutoLock l(between_mtx);
									between.push_back(std::move(_sending));
								}
								break;

								// - - - - - - > GOT PONG, LET'S GO! < - - - - - - //
								case __package::package_type::PONG:
								{
									debug(socket_identification + "&6GOT PONG!");

									network_analysis.ping_new(Tools::now() - received.pack.pong.self_time_ms);
									//ping = Tools::now() - received.pack.pong.self_time_ms;
								}
								break;

								// - - - - - - > NO JOB PENDING, GENERATE NEW JOB < - - - - - - //
								case __package::package_type::DATA:
								{
									tasked_once = true;

									data_working_on.is_full = received.pack.data.full; // last one rules
									data_working_on.buffer.resize(data_working_on.buffer.size() + received.pack.data.data_len);
									std::copy(received.pack.data.buffer, received.pack.data.buffer + received.pack.data.data_len, data_working_on.buffer.end() - received.pack.data.data_len);
									data_working_on.__represents_n_packages++;

									bool buffer_overflow_help = (my_buffer_limit > 0 && data_working_on.__represents_n_packages == my_buffer_limit);

									if (received.pack.data.full || buffer_overflow_help) {

										//data_working_on.is_full = data_working_on.is_full && !buffer_overflow_help; // if overflow, not full(?)

										// = = = = = Try and, if success, move/erase = = = = = //
										if (recv_overwrite) {
											Tools::AutoLock l(recv_overwrite_mtx);
											if (recv_overwrite) {
												recv_overwrite((uintptr_t)this, std::move(data_working_on.buffer));
												calculate_add_available(static_cast<int>(data_working_on.__represents_n_packages)); // instant regen because of automatic "free up"
												data_working_on.__represents_n_packages = 0;
											}

										}

										// = = = = = If not empty, it didn't move, so let's go = = = = = //
										if (!data_working_on.buffer.empty()) {
											Tools::AutoLock l(buffer_receive_mtx);
											buffer_receive.push_back(std::move(data_working_on)); // no automatic regen "free up". get_next() has to be called so new space can be "available" to read
											data_working_on.__represents_n_packages = 0; // just to be completely sure.

											// other side already remove one. No need for calculate_add_available(-1);
											received_package.signal_all();
										}

									}
									last_recv_remaining = received.pack.data.remaining;
								}
								break;
							}

						}
						break;
					}

				}
			}

			void Connection::init()
			{
				keep_connection = true;
				connection_handle_send.set([&](Tools::boolThreadF f) { handle_connection_send(f, connection_handle_send); });
				connection_handle_recv.set([&](Tools::boolThreadF f) { handle_connection_recv(f, connection_handle_recv); });
				connection_handle_send.start();
				connection_handle_recv.start();
			}

			Connection::Connection(SOCKET socket)
			{
				if (socket != INVALID_SOCKET) {
					connected = socket;
					init();
				}
				//else throw Handling::Abort(__FUNCSIG__, "Invalid SOCKET at Connection constructor", Handling::abort::abort_level::GIVEUP);
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
				//printf_s("\nConnected");
				return true;
			}

			void Connection::close()
			{
				keep_connection = false;
				if (connected != INVALID_SOCKET) {
					::closesocket(connected);
					connected = INVALID_SOCKET;

					connection_handle_send.join();
					connection_handle_recv.join();
					
					{
						Tools::AutoLock lol1(between_mtx);
						between.clear();
					}

					Tools::AutoLock lol2(buffer_sending_mtx);
					Tools::AutoLock lol3(buffer_receive_mtx);

					buffer_sending.clear();
					buffer_receive.clear();
					calculate_cut_value(); // reset
				}
			}

			/*int Connection::performance_status_send() const
			{
				return balance_sending;
			}

			int Connection::performance_status_recv() const
			{
				return balance_receive;
			}*/

			bool Connection::is_connected() const
			{
				return connected != INVALID_SOCKET && keep_connection;
			}

			const NetworkMonitor& Connection::get_network_info() const
			{
				return network_analysis;
			}

			bool Connection::has_package() const
			{
				return buffer_receive.size();
			}

			void Connection::set_max_buffering(const long long u)
			{
				calculate_modify_buffer(u);
				should_sync = true;
			}

			long long Connection::in_memory_can_send() const
			{
				return buffer_available_send;
			}

			long long Connection::in_memory_can_read() const
			{
				return buffer_available_calculated;
			}

			// me sending versus what has come already
			unsigned Connection::small_packages_on_the_way() const
			{

				const auto res = (myself.sent_count - yourself.received_count); // since last update, how many hasn't been there yet?
				return (res ? res : (myself.buffer_sending_size ? 1 : 0));
			}

			// information of _data.remaining
			unsigned Connection::small_packages_on_my_way() const
			{
				return last_recv_remaining ? last_recv_remaining : (unsigned)((bool)yourself.buffer_sending_size); // estimated because there can be 2 or more "full packages" coming. If yourself.buffer_sending_size, each can be 1 or more packages.
			}

			// them vs what I have received so far
			unsigned Connection::small_packages_received_since_last_sync() const
			{
				return (myself.received_count - yourself.sent_count);
			}

			bool Connection::wait_for_package(const std::chrono::milliseconds t)
			{
				if (has_package()) return true;
				while (!has_package()) received_package.wait_signal(t.count());
				return true;
			}

			combined_data Connection::get_next(const bool force_wait_for_full)
			{
				if (!has_package()) return {"", false};
				combined_data result{"", false};
				Tools::AutoLock l(buffer_receive_mtx);
				do {
					if (buffer_receive.size() == 0) {
						should_sync = true;

						if (force_wait_for_full) {
							l.unlock();
							while (buffer_receive.size() == 0) { std::this_thread::yield(); std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // really wait
							l.lock();
						}
						else return std::move(result);
					}
					combined_data& yo = buffer_receive.front();

					result.buffer += yo.buffer;
					result.is_full |= yo.is_full;

					calculate_add_available(static_cast<int>(yo.__represents_n_packages)); // if it gets something, free up so new stuff can keep coming!
					should_sync = true; // new "free", update sending side!

					buffer_receive.erase(buffer_receive.begin());

				} while (force_wait_for_full && !result.is_full);

				should_sync = true;

				return std::move(result);
			}

			void Connection::send_package(const std::string& str, const bool waitt)
			{
				send_package(combined_data{ str, true }, waitt);
			}

			void Connection::send_package(const combined_data& data, const bool waitt)
			{
				{
					Tools::AutoLock l(buffer_sending_mtx);
					buffer_sending.push_back(data);
				}
				while (buffer_sending.size()) sent_once_package.wait_signal(100);
			}

			unsigned long long Connection::get_packages_sent() const
			{
				return myself.sent_count;
			}

			unsigned long long Connection::get_packages_sent_bytes() const
			{
				return network_analysis.send_get_total();
			}

			unsigned long long Connection::get_packages_recv() const
			{
				return myself.received_count;
			}

			unsigned long long Connection::get_packages_recv_bytes() const
			{
				return network_analysis.recv_get_total();
			}

			size_t Connection::get_ping()
			{
				return network_analysis.ping_now();
			}

			bool Connection::is_overloaded() const
			{
				return in_memory_can_send() == 0;
			}

			void Connection::overwrite_reads_to(std::function<void(const uintptr_t, const std::string&)> ow)
			{
				Tools::AutoLock l(recv_overwrite_mtx);
				recv_overwrite = ow;
			}

			void Connection::overwrite_sends_to(std::function<std::string(void)> ow)
			{
				Tools::AutoLock l(send_overwrite_mtx);
				send_overwrite = ow;
			}

			/*void Connection::set_performance_watchdog(std::function<void(const connection::_who, const connection::_performance_adapt)> ow)
			{
				Tools::AutoLock l(perf_monitor_mtx);
				perf_monitor = ow;
			}*/

			void Connection::reset_overwrite_reads()
			{
				Tools::AutoLock l(recv_overwrite_mtx);
				recv_overwrite = std::function<void(const uintptr_t, const std::string&)>();
			}

			void Connection::reset_overwrite_sends()
			{
				Tools::AutoLock l(send_overwrite_mtx);
				send_overwrite = std::function<std::string(void)>();
			}

			/*void Connection::reset_performance_watchdog()
			{
				Tools::AutoLock l(perf_monitor_mtx);
				perf_monitor = std::function<void(const connection::_who, const connection::_performance_adapt)>();
			}*/

			void Connection::set_mode(const Tools::superthread::performance_mode m)
			{
				if (m == Tools::superthread::performance_mode::_COUNT) return;
				connection_handle_send.set_performance_mode(m);
				connection_handle_recv.set_performance_mode(m);
			}

			void Connection::reset_mode_default()
			{
				connection_handle_send.set_performance_mode(connection::default_performance_connection);
				connection_handle_recv.set_performance_mode(connection::default_performance_connection);
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
				errno_t ignor{};
				return transform_any_to_package(data, size, ignor);
			}

			std::string transform_any_to_package(void* data, const size_t size, errno_t& err)
			{
				if (!data || !size) return "";
				std::string buf;
				buf.resize(size);
				if (err = memcpy_s(buf.data(), size, data, size)) return "";
				return std::move(buf);
			}

			bool transform_any_package_back(void* data, const size_t size, const std::string& src)
			{
				if (src.size() != size) return false;
				return memcpy_s(data, size, src.data(), src.size()) == 0;
			}

			bool transform_and_send_file_auto(SmartFile& fp, Connection& cn, size_t read_buf_size)
			{
				if (!fp.is_open()) return false;
				if (!fp.is_readable()) return false;
				if (!cn.is_connected()) return false;

				if (read_buf_size == 0) read_buf_size = connection::package_size * 20;
				else if (read_buf_size < connection::package_size) read_buf_size = connection::package_size;

				while (!fp.eof())
				{
					combined_data data;
					fp.read(data.buffer, read_buf_size);
					data.is_full = fp.eof();
					cn.send_package(data);
				}

				return true;
			}
		}
	}
}