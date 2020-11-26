#include "connection.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			_unprocessed_pack::_unprocessed_pack(const std::string& s)
			{
				data = std::move(s);
			}

			_unprocessed_pack::_unprocessed_pack(std::string&& s, unsigned u)
			{
				data = std::move(s);
				sys = u;
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

			void Connection::handle_send(Tools::boolThreadF run)
			{
				std::stringstream ss;
				ss << std::this_thread::get_id();
				const std::string common = std::string("[HANDLE_SEND] T#") + ss.str() + std::string(": ");

				unsigned do_sys = 0;

				EventTimer timm(connection::pinging_time);

				EventHandler evhdr;
				evhdr.add(timm);
				evhdr.set_run_autostart([&](const Interface::RawEvent& re) {
					// add to vector so the thread here just send sync.
					do_sys = static_cast<unsigned>(connection::_internal_tasks::PING);
				});
				timm.start();
				

				debug(common + "Started.");

				while (run() && keep_connection) {

					std::string pack;
					unsigned sys_t = 0;

					if (do_sys) {
						switch (do_sys) {
						case static_cast<unsigned>(connection::_internal_tasks::PING): // first
						{
							sys_t = do_sys;
							long long noww = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
							pack.resize(sizeof(noww));
							memcpy_s(pack.data(), pack.size(), (void*)&noww, sizeof(noww));
						}
							break;
						}
						do_sys = 0;
					}
					else if (alt_generate_auto) {
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
							auto _temp = packs_sending.front();
							pack = std::move(_temp.data);
							sys_t = _temp.sys;
							packs_sending.erase(packs_sending.begin());
							//debug(common + "(default) send tasked.");
						}
					}

					if (!pack.length()) continue; // just to be 100% sure.

					const unsigned len = (static_cast<unsigned>(pack.length()) - 1) / connection::package_size; // if len == package_size, it is only one pack, so (len-1)...

					for (unsigned count = 0; count <= len; count++) {
						_pack one;
						one.sum_with_n_more = len - count;
						one.sys = sys_t;
						std::string res = pack.substr(0, connection::package_size);
						one.data_len = static_cast<unsigned>(res.length());
						size_t _count = 0;
						for (auto& i : res) one.data[_count++] = i;
						if (pack.length() > connection::package_size) pack = pack.substr(connection::package_size);
						else pack.clear();

						auto fi = ::send(connected, (char*)&one, sizeof(_pack), 0);
						packages_sent++;

						debug(common + "SEND once.");

						if (fi < 0) {
							keep_connection = false;
							break;
						}
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
					_pack pack;

					auto fi = ::recv(connected, (char*)&pack, sizeof(_pack), 0);
					packages_recv++;

					debug(common + "RECV once.");

					if (fi > 0) { // if one is _pack and there're more packs, they will not have _internal_pack in between

						std::string data;
						Tools::AutoLock luck(packs_received_m);
						for (unsigned p = 0; p < pack.data_len; p++) data += pack.data[p];

						while (pack.sum_with_n_more > 0) {
							auto fi2 = ::recv(connected, (char*)&pack, sizeof(_pack), 0);
							packages_recv++;

							debug(common + "RECV once.");

							if (fi2 < 0) {
								keep_connection = false;
								break;
							}
							else if (fi2 == 0) continue;

							for (unsigned p = 0; p < pack.data_len; p++) data += pack.data[p];
							//printf_s("\nReceived one package.");
						}

						if (pack.sys != 0) { // system package
							switch (pack.sys) {
							case static_cast<unsigned>(connection::_internal_tasks::PING): // just a echo // second
							{
								Tools::AutoLock luck(packs_sending_m);
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
						}
						else if (alt_receive_autodiscard) {
							alt_receive_autodiscard(data);
							debug(common + "(auto) tasked.");
						}
						else {
							packs_received.emplace_back(std::move(data));
							debug(common + "(default) tasked.");
						}
					}
					else if (fi < 0) { // lost connection
						keep_connection = false;
						break;
					}
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

			std::string Connection::get_next()
			{
				if (alt_receive_autodiscard) throw Handling::Abort(__FUNCSIG__, "Recvs are overrwriten by a function! You cannot recv package when function is set!");
				Tools::AutoLock luck(packs_received_m);
				if (packs_received.size() == 0) return "";
				std::string cpy = std::move(packs_received.front());
				packs_received.erase(packs_received.begin());
				return std::move(cpy);
			}

			void Connection::send_package(std::string pack)
			{
				if (alt_generate_auto) throw Handling::Abort(__FUNCSIG__, "Sends are overrwriten by a function! You cannot send package when function is set!");
				Tools::AutoLock luck(packs_sending_m);

				packs_sending.push_back(std::move(pack));
			}

			size_t Connection::get_packages_sent() const
			{
				return packages_sent;
			}

			size_t Connection::get_packages_recv() const
			{
				return packages_recv;
			}

			size_t Connection::get_ping()
			{
				return last_ping;
			}

			void Connection::overwrite_reads_to(std::function<void(const std::string&)> ow)
			{
				alt_receive_autodiscard = ow;
			}

			void Connection::overwrite_sends_to(std::function<std::string(void)> ow)
			{
				alt_generate_auto = ow;
			}

			void Connection::reset_overwrite_reads()
			{
				alt_receive_autodiscard = std::function<void(const std::string&)>();
			}

			void Connection::reset_overwrite_sends()
			{
				alt_generate_auto = std::function<std::string(void)>();
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
					connections.emplace_back(std::move(dis));

					//printf_s("\nSomeone has connected!");

					for (size_t p = 0; p < connections.size(); p++) {
						if (!connections[p]->is_connected()) {
							//printf_s("\nSomeone has disconnected!");
							connections.erase(connections.begin() + p--);
						}
					}

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
					Tools::AutoLock luck(connections_m);
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

		}
	}
}