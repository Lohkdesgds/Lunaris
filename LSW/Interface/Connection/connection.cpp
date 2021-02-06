#include "connection.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			_pack_1::_pack_1()
			{
				timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}

			_unprocessed_pack::_unprocessed_pack(const std::string& s)
			{
				data = s;
			}

			_unprocessed_pack::_unprocessed_pack(std::string&& s, unsigned u)
			{
				data = std::move(s);
				sys = u;
			}

			_unprocessed_pack::_unprocessed_pack(_unprocessed_pack&& p)
			{
				*this = std::move(p);
			}

			_unprocessed_pack::_unprocessed_pack(const _unprocessed_pack& p)
			{
				*this = p;
			}

			void _unprocessed_pack::operator=(_unprocessed_pack&& p)
			{
				data = std::move(p.data); // here
				sys = p.sys;
			}

			void _unprocessed_pack::operator=(const _unprocessed_pack& p)
			{
				data = p.data;
				sys = p.sys;
			}

			bool ConnectionCore::initialize(const char* ip_str, const int port, const int isthis_ipv6)
			{
				if (init) return true;

				if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
				{
					failure = true;
					return false;
				}

				struct addrinfo hints;

				char port_str[16];
				sprintf_s(port_str, "%d", port);

				SecureZeroMemory(&hints, sizeof(hints));
				if (isthis_ipv6 >= 0) hints.ai_family = isthis_ipv6 ? AF_INET6 : AF_INET;
				else hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				if (isthis_ipv6 >= 0) hints.ai_flags = AI_PASSIVE;

				// Resolve the server address and port
				if (getaddrinfo(ip_str, port_str, &hints, &result) != 0) return [&] {failure = true; return false; }();

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

			bool Connection::ensure_send(char* data, const int len)
			{
				if (len <= 0) return true;
				int fi = 0;
				while (fi != len)
				{
					auto _now = ::send(connected, data + fi, len - fi, 0);
					if (_now < 0) return false;
					fi += _now;
				}
				packages_sent_bytes += len;
				return true;
			}

			bool Connection::ensure_recv(char* data, const int len)
			{
				if (len <= 0) return true;
				int fi = 0;
				while (fi != len)
				{
					auto _now = ::recv(connected, data + fi, len - fi, 0);
					if (_now < 0) return false;
					fi += _now;
				}
				packages_recv_bytes += len;
				return true;
			}

			void Connection::handle_send(Tools::boolThreadF run)
			{
				std::stringstream ss;
				ss << std::this_thread::get_id();
				const std::string common = std::string("[HANDLE_SEND] T#") + ss.str() + std::string(": ");

				unsigned do_sys = 0;

				EventTimer timm(connection::pinging_time);
				EventTimer timsync(connection::syncing_time);

				EventHandler evhdr;
				evhdr.add(timm);
				evhdr.add(timsync);
				evhdr.set_run_autostart([&](const Interface::RawEvent& re) {
					// add to vector so the thread here just send sync.
					if (re.timer_event().source == timm) {
						do_sys = static_cast<unsigned>(connection::_internal_tasks::PING);
					}
					else if (re.timer_event().source == timsync) {
						do_sys = static_cast<unsigned>(connection::_internal_tasks::SYNC);
					}
				});
				timm.start();
				timsync.start();
				

				debug(common + "Started.");

				while (run() && keep_connection) {

					if (!in_sync_signal_received) {
						std::this_thread::yield();
						continue;
					}

					std::string pack;
					_pack_1 sys_t;
					
					if (do_sys) {
						sys_t.sys = do_sys;

						do_sys = 0;

						switch (sys_t.sys) {
						case static_cast<unsigned>(connection::_internal_tasks::PING): // first
						{
							sys_t.has_pack_2 = false;

							Tools::AutoLock luck(packs_sending_m);

							if (!ensure_send((char*)&sys_t, sizeof(_pack_1))) {
								keep_connection = false;
								package_come_wait.signal_all();
								continue;
							}
						}
							break;
						case static_cast<unsigned>(connection::_internal_tasks::SYNC): // sync stuff
						{
							sys_t.has_pack_2 = false;

							Tools::AutoLock luck(packs_sending_m);

							in_sync_signal_received = false;
							debug(common + "QUESTIONING SYNC");

							if (!ensure_send((char*)&sys_t, sizeof(_pack_1))) {
								keep_connection = false;
								package_come_wait.signal_all();
								continue;
							}
						}
							break;
						}
						continue; // go back to while
					}

					// * * * * * * * * * * * * * * * * * * * * * * CERTAINLY NOT do_sys * * * * * * * * * * * * * * * * * * * * * * //

					// let ping go lol
					if (friend_there_recv_buffer_overload) {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						std::this_thread::yield();
						continue;
					}


					if (alt_generate_auto) {
						pack = alt_generate_auto();
						if (pack.empty()) {
							//std::this_thread::sleep_for(connection::min_delay_no_tasks); // SuperThread handles this now
							continue;
						}
						//debug(common + "(auto) tasked.");
					}
					else {
						Tools::AutoLock luck(packs_sending_m);
						if (packs_sending.size() == 0) {
							//std::this_thread::sleep_for(connection::min_delay_no_tasks); // SuperThread handles this now
							continue;
						}
						else {
							auto& _temp = packs_sending.front();
							pack = std::move(_temp.data);
							sys_t.sys = _temp.sys;
							packs_sending.erase(packs_sending.begin());
							//debug(common + "(default) send tasked.");
						}
					}

					if (!pack.length()) continue; // just to be 100% sure.

					const unsigned len = (static_cast<unsigned>(pack.length()) - 1) / connection::package_size; // if len == package_size, it is only one pack, so (len-1)...

					if (!ensure_send((char*)&sys_t, sizeof(_pack_1))) {
						keep_connection = false;
						break;
					}

					if (!sys_t.has_pack_2) continue; // no pack to send, just info

					for (unsigned count = 0; count <= len; count++) {
						_pack_2 one;
						one.sum_with_n_more = len - count;
						std::string res = pack.substr(0, connection::package_size);
						one.data_len = static_cast<unsigned>(res.length());
						size_t _count = 0;
						for (auto& i : res) one.data[_count++] = i;
						if (pack.length() > connection::package_size) pack = pack.substr(connection::package_size);
						else pack.clear();

						if (!ensure_send((char*)&one, sizeof(_pack_2))) {
							keep_connection = false;
							break;
						}

						packages_sent++;

						debug(common + "SEND once.");
					}
				}
				evhdr.stop(); // make sure this happens before timar death
			}

			void Connection::handle_recv(Tools::boolThreadF run)
			{
				std::stringstream ss;
				ss << std::this_thread::get_id();
				const std::string common = std::string("[HANDLE_RECV] T#") + ss.str() + std::string(": ");

				debug(common + "Started.");

				while (run() && keep_connection) {

					if (has_package()) package_come_wait.signal_all();

					// buffer is too big to continue recv. Send stop on other side.
					if (buf_max) {
						if (packs_received.size() >= buf_max) {
							if (!recv_this_buffer_overload) {
								recv_this_buffer_overload = true;

								Tools::AutoLock luck(packs_sending_m);

								_pack_1 smol;
								smol.has_pack_2 = false;
								smol.sys = static_cast<unsigned>(connection::_internal_tasks::RECV_OVERLOAD_WAIT);
								debug(common + "RECV OVERLOAD WAIT SENT");

								if (!ensure_send((char*)&smol, sizeof(_pack_1))) {
									keep_connection = false;
									package_come_wait.signal_all();
									continue;
								}
							}
						}
						else if (recv_this_buffer_overload) { // not overloaded anymore lol
							recv_this_buffer_overload = false;

							Tools::AutoLock luck(packs_sending_m);

							_pack_1 smol;
							smol.has_pack_2 = false;
							smol.sys = static_cast<unsigned>(connection::_internal_tasks::RECV_OVERLOAD_CONTINUE);
							debug(common + "RECV OVERLOAD CONTINUE SENT");

							if (!ensure_send((char*)&smol, sizeof(_pack_1))) {
								keep_connection = false;
								package_come_wait.signal_all();
								continue;
							}
						}
					}

					// new code:

					_pack_1 identify; // has_pack_2
					_pack_2 pack;

					if (!ensure_recv((char*)&identify, sizeof(_pack_1))) {
						keep_connection = false;
						package_come_wait.signal_all();
						continue;
					}

					// no _pack_2
					if (!identify.has_pack_2) {
						switch (identify.sys) {
						case static_cast<unsigned>(connection::_internal_tasks::PING): // return pong with data
						{
							Tools::AutoLock luck(packs_sending_m);

							identify.sys = static_cast<unsigned>(connection::_internal_tasks::PONG);
							identify.has_pack_2 = false;

							if (!ensure_send((char*)&identify, sizeof(_pack_1))) {
								keep_connection = false;
								package_come_wait.signal_all();
								continue;
							}
						}
							break;
						case static_cast<unsigned>(connection::_internal_tasks::PONG): // PING RETURNED! (ping pong is good because reference in time can be different, idk)
						{
							const auto _time_now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
							last_ping = (_time_now - identify.timestamp) / 2;
						}
							break;
						case static_cast<unsigned>(connection::_internal_tasks::SYNC): // return SYNC_BACK
						{
							Tools::AutoLock luck(packs_sending_m);

							identify.sys = static_cast<unsigned>(connection::_internal_tasks::SYNC_BACK);
							identify.has_pack_2 = false;

							if (!ensure_send((char*)&identify, sizeof(_pack_1))) {
								keep_connection = false;
								package_come_wait.signal_all();
								continue;
							}
						}
							break;
						case static_cast<unsigned>(connection::_internal_tasks::SYNC_BACK): // return SYNC_BACK
						{
							in_sync_signal_received = true; // in sync
							debug(common + "SYNC INDEED");
						}
							break;
						case static_cast<unsigned>(connection::_internal_tasks::RECV_OVERLOAD_WAIT): // other side overload
						{
							friend_there_recv_buffer_overload = true; // no mutex thanks god
							debug(common + "RECV OVERLOAD WAIT RECV");
						}
							break;
						case static_cast<unsigned>(connection::_internal_tasks::RECV_OVERLOAD_CONTINUE): // other side can continue
						{
							friend_there_recv_buffer_overload = false; // no mutex thanks god
							debug(common + "RECV OVERLOAD CONTINUE RECV");
						}
							break;
						}
						continue;
					}

					// has pack 2

					if (!ensure_recv((char*)&pack, sizeof(_pack_2))) {
						keep_connection = false;
						package_come_wait.signal_all();
						continue;
					}

					packages_recv++;

					debug(common + "RECV once.");


					std::string data;
					Tools::AutoLock luck(packs_received_m);
					for (unsigned p = 0; p < pack.data_len; p++) data += pack.data[p];

					while (pack.sum_with_n_more > 0) {

						if (!ensure_recv((char*)&pack, sizeof(_pack_2))) {
							keep_connection = false;
							package_come_wait.signal_all();
							continue;
						}

						packages_recv++;

						debug(common + "RECV once.");

						for (unsigned p = 0; p < pack.data_len; p++) data += pack.data[p];
						//printf_s("\nReceived one package.");
					}

					if (identify.sys == 0) {
						if (alt_receive_autodiscard) {
							alt_receive_autodiscard((uintptr_t)this, data);
							package_come_wait.signal_all();
							debug(common + "(auto) tasked.");
						}
						else {
							packs_received.emplace_back(std::move(data));
							package_come_wait.signal_all();
							debug(common + "(default) tasked.");
						}
					}
					/*else {
						// * * * * * * * * * * * * * * * * * * * * IF IT NEEDS TO READ, GO HERE * * * * * * * * * * * * * * * * * * * * //
						switch (identify.sys) {
						case static_cast<unsigned>(connection::_internal_tasks::PING): // just a echo // second
						{
							Tools::AutoLock luck(packs_sending_m);

							_pack_1 smol;
							smol.has_pack_2 = false;
							smol.sys = static_cast<unsigned>(connection::_internal_tasks::PONG);

							if (!ensure_send((char*)&smol, sizeof(_pack_1))) {
								keep_connection = false;
								package_come_wait.signal_all();
								continue;
							}

							packs_sending.push_back(_unprocessed_pack{ std::move(data), static_cast<unsigned>(connection::_internal_tasks::PONG) });
						}
							break;
						case static_cast<unsigned>(connection::_internal_tasks::PONG): // response // third
						{
							long long noww = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
							long long* before = (long long*)data.data(); // direct raw data

							last_ping = (noww - *before) / 2;
						}
							break;
						}
					}*/
				}
			}

			void Connection::init()
			{
				keep_connection = true;
				send_thread.set([&](Tools::boolThreadF f) { handle_send(f); });
				recv_thread.set([&](Tools::boolThreadF f) { handle_recv(f); });
				send_thread.start();
				recv_thread.start();
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

			bool Connection::connect(const char* a, const int b)
			{
				if (!core.initialize(a, b, -1)) return false;
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
					send_thread.join();
					recv_thread.join();
					Tools::AutoLock lol1(packs_sending_m);
					Tools::AutoLock lol2(packs_received_m);
					packs_sending.clear();
					packs_received.clear();
				}
			}

			bool Connection::is_connected() const
			{
				return connected != INVALID_SOCKET && keep_connection;
			}

			bool Connection::has_package() const
			{
				Tools::AutoLock luck(packs_received_m);
				return packs_received.size();
			}

			bool Connection::in_sync() const
			{
				return in_sync_signal_received;
			}

			bool Connection::is_receiving_buffer_full() const
			{
				return recv_this_buffer_overload;
			}

			bool Connection::is_other_receiving_buffer_full() const
			{
				return friend_there_recv_buffer_overload;
			}

			bool Connection::wait_for_package(const std::chrono::milliseconds t)
			{
				if (has_package()) return true;
				package_come_wait.wait_signal(t.count());
				return has_package();
			}

			std::string Connection::get_next()
			{
				if (alt_receive_autodiscard) throw Handling::Abort(__FUNCSIG__, "Recvs are overrwriten by a function! You cannot recv package when function is set!");
				if (packs_received.size() == 0) return "";
				Tools::AutoLock luck(packs_received_m);
				std::string cpy = std::move(packs_received.front());
				packs_received.erase(packs_received.begin());
				return std::move(cpy);
			}

			bool Connection::send_package(std::string pack, bool break_law, bool lock_on_full)
			{
				if (alt_generate_auto) throw Handling::Abort(__FUNCSIG__, "Sends are overrwriten by a function! You cannot send package when function is set!");

				Tools::AutoLock luck(packs_sending_m);

				while (!break_law && buf_max && packs_sending.size() >= buf_max) {
					if (!lock_on_full) return false;
					luck.unlock();
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					std::this_thread::yield();
					luck.lock();
				}

				packs_sending.push_back(std::move(pack));
				return true;
			}

			size_t Connection::get_packages_sent() const
			{
				return packages_sent;
			}

			size_t Connection::get_packages_sent_bytes() const
			{
				return packages_sent_bytes;
			}

			size_t Connection::get_packages_recv() const
			{
				return packages_recv;
			}

			size_t Connection::get_packages_recv_bytes() const
			{
				return packages_recv_bytes;
			}

			size_t Connection::get_ping()
			{
				return last_ping;
			}

			void Connection::set_max_buffering(const size_t mx)
			{
				buf_max = mx;
			}

			void Connection::overwrite_reads_to(std::function<void(const uintptr_t, const std::string&)> ow)
			{
				alt_receive_autodiscard = ow;
			}

			void Connection::overwrite_sends_to(std::function<std::string(void)> ow)
			{
				alt_generate_auto = ow;
			}

			void Connection::reset_overwrite_reads()
			{
				alt_receive_autodiscard = std::function<void(const uintptr_t, const std::string&)>();
			}

			void Connection::reset_overwrite_sends()
			{
				alt_generate_auto = std::function<std::string(void)>();
			}

			void Connection::set_mode(const Tools::superthread::performance_mode m)
			{
				send_thread.set_performance_mode(m);
				recv_thread.set_performance_mode(m);
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
				core.initialize(nullptr, port, ipv6);
				core.as_host(Listening);
				init();
			}

			Hosting::Hosting(const bool ipv6)
			{
				core.initialize(nullptr, connection::default_port, ipv6);
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
				errno_t ignor;
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


		}
	}
}