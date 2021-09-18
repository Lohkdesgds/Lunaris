#include "socket.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			SocketCore::_core SocketCore::__start;

			void* SocketInfo::_aux_get_in_addr()
			{
				const sockaddr* sa = (struct sockaddr*)(&addr);

				if (sa->sa_family == AF_INET) // IPv4 address
					return &(((struct sockaddr_in*)sa)->sin_addr);
				// else IPv6 address
				return &(((struct sockaddr_in6*)sa)->sin6_addr);
			}

			u_short SocketInfo::_aux_get_in_port()
			{
				const sockaddr* sa = (struct sockaddr*)(&addr);

				if (sa->sa_family == AF_INET) // IPv4 address
					return (((struct sockaddr_in*)sa)->sin_port);
				// else IPv6 address
				return (((struct sockaddr_in6*)sa)->sin6_port);
			}

			bool SocketInfo::gen_data()
			{
				socklen_t sockaddrlen = sizeof(addr);

				const sockaddr* sockaddr = (struct sockaddr*)(&addr);

				// converting network address to presentation address
				if (inet_ntop(sockaddr->sa_family, _aux_get_in_addr(), name, INET6_ADDRSTRLEN * sizeof(char)) == NULL)
					return false;

				// converting network port to host port
				port = ntohs(_aux_get_in_port());

				return true;
			}

			bool SocketInfo::set_socket_buffering_block(const bool block) const
			{
				u_long iMode = block ? 0 : 1; // 0 blocks
				return ioctlsocket(socket, FIONBIO, &iMode) == 0;
			}

			SocketInfo::SocketInfo(const SOCKADDR_STORAGE& ad, const SOCKET skt)
			{
				std::copy((char*)&ad, (char*)&ad + sizeof(SOCKADDR_STORAGE), (char*)&addr);
				socket = skt;
				gen_data();
			}

			SocketInfo::SocketInfo(SOCKET skt)
			{
				if (skt == INVALID_SOCKET) throw std::exception("INVALID SOCKET!");
				socket = skt;

				socklen_t sockaddrlen = sizeof(SOCKADDR_STORAGE);
				if (getpeername(socket, (struct sockaddr*)(&addr), &sockaddrlen) < 0) throw std::exception("INVALID SOCKET! It wasn't bound");

				gen_data();				
			}

			SocketInfo::SocketInfo(SocketInfo&& ad)
			{
				std::copy((char*)&ad.addr, (char*)&ad.addr + sizeof(SOCKADDR_STORAGE), (char*)&addr);
				std::copy((char*)ad.name, (char*)ad.name + sizeof(ad.name), (char*)name);
				port = ad.port;
				socket = ad.socket;
				ad.socket = INVALID_SOCKET;
			}

			void SocketInfo::operator=(SocketInfo&& ad)
			{
				std::copy((char*)&ad.addr, (char*)&ad.addr + sizeof(SOCKADDR_STORAGE), (char*)&addr);
				std::copy((char*)ad.name, (char*)ad.name + sizeof(ad.name), (char*)name);
				port = ad.port;
				socket = ad.socket;
				ad.socket = INVALID_SOCKET;
			}

			SocketInfo::~SocketInfo()
			{
				close();
			}

			bool SocketInfo::operator==(const SOCKADDR_STORAGE& oth)
			{
				return memcmp(&addr, &oth, sizeof(SOCKADDR_STORAGE)) == 0;
			}

			bool SocketInfo::operator!=(const SOCKADDR_STORAGE& oth)
			{
				return memcmp(&addr, &oth, sizeof(SOCKADDR_STORAGE)) != 0;
			}

			const char* SocketInfo::get_url() const
			{
				return name;
			}

			const u_short& SocketInfo::get_port() const
			{
				return port;
			}

			void SocketInfo::close()
			{
				if (socket != INVALID_SOCKET) {
					closesocket(socket);
					socket = INVALID_SOCKET;
				}
			}

			SOCKADDR_STORAGE& SocketInfo::get_addr()
			{
				return addr;
			}

			const SOCKET& SocketInfo::get_socket() const
			{
				return socket;
			}

			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#    SocketInfo   #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//
			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//
			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#    SocketCore   #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//

			SocketCore::_core::_core()
			{
				if (!started) {
					if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
						throw std::exception("FAILED TO START THE CORE!");
					}
					else started = true;
				}
			}

			socket::result_type SocketCore::setup_client(SocketInfo& client, const socket::protocol& protocol, const socket::family& family, const u_short port_u, const std::string& url)
			{
				const char* Address = url.length() ? url.c_str() : nullptr;
				char Port[8]{};
				int RetVal{};
				ADDRINFO Hints;
				ADDRINFO* AddrInfo = nullptr, * AI = nullptr;

				sprintf_s(Port, "%hu", port_u);
				ZeroMemory(&Hints, sizeof(Hints));

				Hints.ai_family = static_cast<int>(family); // Family
				Hints.ai_socktype = static_cast<int>(protocol); // SocketType

				if ((RetVal = getaddrinfo(Address, Port, &Hints, &AddrInfo)) != 0) {
					//core->printlog(socket::log_event_type::ERROR, ("Cannot resolve address '") + url + "', port " + Port + ", error " + std::to_string(RetVal) + ": " + std::string((char*)gai_strerror(RetVal)));
					return socket::result_type::CANT_RESOLVE_HOSTNAME;
				}

				AI = AddrInfo;
				for (int i = 0; AI != nullptr; AI = AI->ai_next) {
					SOCKET sock = INVALID_SOCKET;
					if ((sock = ::socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol)) == INVALID_SOCKET) {
						//core->printlog(socket::log_event_type::WARN, "socket() failed with error " + std::to_string(WSAGetLastError()));
						continue;
					}

					//core->printlog(socket::log_event_type::INFO, "Attempting to connect to " + std::string(Address ? Address : "localhost"));

					if (::connect(sock, AI->ai_addr, (int)AI->ai_addrlen) == SOCKET_ERROR) {
						/*std::string AddrName;
						AddrName.resize(NI_MAXHOST);

						int err = WSAGetLastError();
						if (getnameinfo(AI->ai_addr, (int)AI->ai_addrlen, AddrName.data(), static_cast<DWORD>(AddrName.size()), NULL, 0, NI_NUMERICHOST) != 0)
							AddrName = "<unknown>";*/

						//core->printlog(socket::log_event_type::WARN, "connect() to " + AddrName + " failed with error " + std::to_string(err));
						::closesocket(sock);
					}
					else {
						if (AI == NULL) {
							//WSACleanup();
							//core->printlog(socket::log_event_type::ERROR, "Unable to connect to the server");
							freeaddrinfo(AddrInfo);
							return socket::result_type::CANT_CONNECT;
						}

						//core->client.protocol = protocol;
						//core->client.family = (AI->ai_family == PF_INET) ? socket::family::IPV4 : socket::family::IPV6;

						client = SocketInfo{ sock }; // copy & build info

						/*if (!core->client.gen_host()) // get port, address, udp route (if)
						{
							core->printlog(socket::log_event_type::ERROR, "Cannot generate host address, error " + std::to_string(WSAGetLastError()));
							freeaddrinfo(AddrInfo);
							return socket::result_type::FAILED_ROUTE_ADDRESSING; // because how else will it sendto (if udp)? (TCP should know its path too)
						}
						if (!core->client.gen_local()) // get port, address, udp route (if)
						{
							core->printlog(socket::log_event_type::WARN, "Cannot generate local address, error " + std::to_string(WSAGetLastError()));
							// self no need for exception
						}*/

						break;
					}
				}

				freeaddrinfo(AddrInfo);

				//if (server && core->server.size() == 0) return socket::result_type::SERVER_COULD_NOT_LISTEN;
				return socket::result_type::NO_ERROR;
			}

			socket::result_type SocketCore::setup_server(std::vector<SOCKET>& vec, const socket::protocol& protocol, const socket::family& family, const u_short port_u)
			{
				char Port[8]{};
				int RetVal{};
				ADDRINFO Hints;
				ADDRINFO* AddrInfo = nullptr, * AI = nullptr;

				sprintf_s(Port, "%hu", port_u);
				ZeroMemory(&Hints, sizeof(Hints));

				Hints.ai_family = static_cast<int>(family); // Family
				Hints.ai_socktype = static_cast<int>(protocol); // SocketType
				Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

				if ((RetVal = getaddrinfo(nullptr, Port, &Hints, &AddrInfo)) != 0) {
					//core->printlog(socket::log_event_type::ERROR, ("Cannot resolve address '") + url + "', port " + Port + ", error " + std::to_string(RetVal) + ": " + std::string((char*)gai_strerror(RetVal)));
					return socket::result_type::CANT_RESOLVE_HOSTNAME;
				}

				AI = AddrInfo;
				for (int i = 0; AI != nullptr; AI = AI->ai_next) {

					// Highly unlikely, but check anyway.
					if (i == FD_SETSIZE) {
						//core->printlog(socket::log_event_type::WARN, "Number of addrinfo reached FD_SETSIZE. This is so rare, keep an eye for errors.");
						break;
					}

					// Only IPV4 and IPV6
					if ((AI->ai_family != PF_INET) && (AI->ai_family != PF_INET6))
						continue;

					//_data::_conn_info svconfig;
					SOCKET newcon = INVALID_SOCKET;

					// Open a socket with the correct address family for this address.
					if ((newcon = ::socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol)) == INVALID_SOCKET) {
						//core->printlog(socket::log_event_type::WARN, "socket() failed with error " + std::to_string(WSAGetLastError()));
						continue;
					}

					if ((AI->ai_family == PF_INET6) && IN6_IS_ADDR_LINKLOCAL((IN6_ADDR*)INETADDR_ADDRESS(AI->ai_addr)) && (((SOCKADDR_IN6*)(AI->ai_addr))->sin6_scope_id == 0)) {
						//core->printlog(socket::log_event_type::WARN, "IPv6 link local addresses should specify a scope ID!");
					}

					// REUSE***
					{
						int enable = 1;
						if (::setsockopt(newcon, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int)) < 0) {
							//core->printlog(socket::log_event_type::WARN, "setsockopt(SO_REUSEADDR) failed with error " + std::to_string(WSAGetLastError()) + ", but this might not change too much, so no closesocket.");
						}
						// this is not available or doesn't matter (?)
						/*if (setsockopt(svconfig.connection, SOL_SOCKET, SO_REUSEPORT, (char*)&enable, sizeof(int)) < 0) {
							log_error(std::string("setsockopt(SO_REUSEPORT ) failed with error ") + std::to_string(WSAGetLastError()) + ", but this might not change too much, so no closesocket.");
						}*/
					}


					if (::bind(newcon, AI->ai_addr, (int)AI->ai_addrlen) == SOCKET_ERROR) {
						//core->printlog(socket::log_event_type::ERROR, "bind() failed with error " + std::to_string(WSAGetLastError()));
						closesocket(newcon);
						continue;
					}

					// Specific for TCP
					if (protocol == socket::protocol::TCP) {
						if (::listen(newcon, 5) == SOCKET_ERROR) {
							//core->printlog(socket::log_event_type::ERROR, "listen() failed with error " + std::to_string(WSAGetLastError()));
							closesocket(newcon);
							continue;
						}
					}

					//svconfig.protocol = protocol;
					//svconfig.family = (AI->ai_family == PF_INET) ? socket::family::IPV4 : socket::family::IPV6;

					//svconfig.gen_local();

					/*core->printlog(socket::log_event_type::INFO,
						"'Listening' on address " + svconfig.local.address + ", "
						"port " + std::to_string(svconfig.local.port) + ", "
						"protocol " + std::string((svconfig.protocol == socket::protocol::TCP) ? "TCP" : "UDP") + ", "
						"protocol family " + std::string((svconfig.family == socket::family::IPV4) ? "PF_INET" : "PF_INET6"));*/

					//core->server.push_back(svconfig);
					vec.push_back(newcon);

					i++;
				}

				freeaddrinfo(AddrInfo);

				if (vec.size() == 0) return socket::result_type::SERVER_COULD_NOT_LISTEN;
				return socket::result_type::NO_ERROR;
			}


			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#    SocketCore   #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//
			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//
			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#      Client     #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//

			const socket::protocol& SocketClient::get_protocol() const
			{
				return self->protocol;
			}

			const socket::family& SocketClient::get_family() const
			{
				return self->family;
			}

			SocketClient::SocketClient(SocketClient&& oth)
			{
				std::unique_lock<std::shared_mutex> l1(oth.self->secure, std::defer_lock);
				std::unique_lock<std::shared_mutex> l2(self->secure, std::defer_lock);
				std::lock(l1, l2);

				self = std::move(oth.self);
				oth.self = std::make_unique<_clientdata>();
			}

			void SocketClient::operator=(SocketClient&& oth)
			{
				std::unique_lock<std::shared_mutex> l1(oth.self->secure, std::defer_lock);
				std::unique_lock<std::shared_mutex> l2(self->secure, std::defer_lock);
				std::lock(l1, l2);

				self = std::move(oth.self);
				oth.self = std::make_unique<_clientdata>();
			}

			bool SocketClient::connect(const std::string& url, const u_short port, const socket::protocol& protocol, const socket::family& family)
			{
				std::unique_lock<std::shared_mutex> l(self->secure);

				self->family = family;
				self->protocol = protocol;
				auto res = setup_client(self->host, protocol, family, port, url);

				if (res != socket::result_type::NO_ERROR) return false;
				if (protocol == socket::protocol::TCP) return true; // UDP needs extra steps.

				SOCKADDR_STORAGE addr{};
				socklen_t sockaddrlen = sizeof(SOCKADDR_STORAGE);

				char buf[2] = "1";

				if (::send(self->host.get_socket(), buf, 1, 0) != 1) return false; // can't send, something is not good																				#1

				u_short newport = 0;

				if (::recvfrom(self->host.get_socket(), (char*)&newport, sizeof(newport), 0, (sockaddr*)&addr, &sockaddrlen) != sizeof(newport)) return false; // can't read, something is not good	#2

				if (newport <= 0 || newport >= 1 << 16) return false; // invalid port

				res = setup_client(self->host, protocol, family, newport, url);
				if (res != socket::result_type::NO_ERROR) return false; // bad stuff

				if (::send(self->host.get_socket(), buf, 1, 0) != 1) return false; // can't send, something is not good																				#3

				if (::recvfrom(self->host.get_socket(), buf, 1, 0, (sockaddr*)&self->host.addr, &sockaddrlen) != 1) return false; // can't read, something is not good											#4

				//self->host = SocketInfo{ addr, self->host.get_socket() };

				return true;
			}

			bool SocketClient::send(const Buffer& vec)
			{
				if (vec.size() > socket::default_max_package) return false; // too big

				std::shared_lock<std::shared_mutex> l(self->secure);

				for (size_t pos = 0; pos < vec.size();)
				{
					int sizelim = static_cast<int>(vec.size() - pos);
					if (sizelim > socket::default_max_package) sizelim = socket::default_max_package;

					int _p = -1;
					if (self->protocol == socket::protocol::TCP)	_p = ::send(self->host.get_socket(), vec.data() + pos, sizelim, 0);
					else											_p = ::sendto(self->host.get_socket(), vec.data() + pos, sizelim, 0, (sockaddr*)&self->host.get_addr(), sizeof(SOCKADDR_STORAGE));

					if (_p < 0) {
						int err = WSAGetLastError();
						switch (err) {
						case WSAEWOULDBLOCK:
							return false;
						case WSAECONNRESET:
							return false;
						}
					}
					else pos += _p;
				}
				return true;
			}

			bool SocketClient::recv(Buffer& vec, const size_t lim)
			{
				const int curr_limit = static_cast<int>(lim < socket::default_max_package ? lim : socket::default_max_package);
				size_t curr = vec.size();
				vec.resize(curr_limit + curr);

				std::shared_lock<std::shared_mutex> l(self->secure);

				while(1)
				{
					int _p = -1;
					SOCKADDR_STORAGE _temp{};
					int _temp_len = sizeof(SOCKADDR_STORAGE);

					self->host.set_socket_buffering_block(false);
					int _preres = 0;
					if (self->protocol == socket::protocol::TCP)	_preres = ::recv(self->host.get_socket(), vec.data() + curr, curr_limit, MSG_PEEK);
					else											_preres = ::recvfrom(self->host.get_socket(), vec.data() + curr, curr_limit, MSG_PEEK, (sockaddr*)&_temp, &_temp_len);
					int err = WSAGetLastError();
					self->host.set_socket_buffering_block(true);

					if (_preres < 0) {
						switch (err) {
						case WSAEWOULDBLOCK:
							vec.resize(curr);
							vec.shrink_to_fit();
							return false;
						case WSAECONNRESET:
							self->host.close();
							vec.resize(curr);
							vec.shrink_to_fit();
							return false;
						default:
							vec.resize(curr);
							vec.shrink_to_fit();
							return false;
						}
					}

					if (self->protocol == socket::protocol::TCP)	_p = ::recv(self->host.get_socket(), vec.data() + curr, curr_limit, 0);
					else											_p = ::recvfrom(self->host.get_socket(), vec.data() + curr, curr_limit, 0, (sockaddr*)&_temp, &_temp_len);

					if (_p < 0) {
						err = WSAGetLastError();
						switch (err) {
						case WSAEWOULDBLOCK:
							vec.resize(curr);
							vec.shrink_to_fit();
							return false;
						case WSAECONNRESET:
							self->host.close();
							vec.resize(curr);
							vec.shrink_to_fit();
							return false;
						default:
							vec.resize(curr);
							vec.shrink_to_fit();
							return false;
						}
					}

					if (self->protocol == socket::protocol::UDP && self->host != _temp) { // try again. discard.
						vec.resize(curr);
						vec.shrink_to_fit();
						return false;
					}
					if (_p == 0) {
						self->host.close();
						vec.resize(curr);
						vec.shrink_to_fit();
						return false;
					}
					else { // good
						vec.resize(_p + curr);
						vec.shrink_to_fit();
						break;
					}
				}
				return true;
			}

			int SocketClient::peek(Buffer& vec, const size_t lim) const
			{
				const int curr_limit = static_cast<int>(lim < socket::default_max_package ? lim : socket::default_max_package);
				size_t curr = vec.size();
				vec.resize(curr_limit + curr);

				std::shared_lock<std::shared_mutex> l(self->secure);

				int _p = -1;
				SOCKADDR_STORAGE _temp{};
				int _temp_len = sizeof(SOCKADDR_STORAGE);


				self->host.set_socket_buffering_block(false);
				int _preres = 0;
				if (self->protocol == socket::protocol::TCP)	_preres = ::recv(self->host.get_socket(), vec.data() + curr, curr_limit, MSG_PEEK);
				else											_preres = ::recvfrom(self->host.get_socket(), vec.data() + curr, curr_limit, MSG_PEEK, (sockaddr*)&_temp, &_temp_len);
				int err = WSAGetLastError();
				self->host.set_socket_buffering_block(true);

				if (_preres < 0) return -err;
				return _preres;
			}

			bool SocketClient::valid() const
			{
				return self->host.get_socket() != INVALID_SOCKET;
			}

			void SocketClient::close()
			{
				self = std::make_unique<_clientdata>();
			}

			const char* SocketClient::get_url() const
			{
				return self->host.get_url();
			}

			const u_short SocketClient::get_port() const
			{
				return self->host.get_port();
			}

			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#      Client     #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//
			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//
			//#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#      Server     #=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#//

			SOCKET SocketServer::common_select(std::vector<SOCKET>& servers)
			{
				int i = 0;
				fd_set SockSet{};
				int NumSocks = static_cast<int>(servers.size());
				if (!NumSocks) {
					//core->printlog(socket::log_event_type::ERROR, "No server listeners available!");
					return INVALID_SOCKET;
				}

				FD_ZERO(&SockSet);

				for (i = 0; i < NumSocks; i++) {
					if (FD_ISSET(servers[i], &SockSet))
						break;
				}
				if (i == NumSocks) {
					for (i = 0; i < NumSocks; i++)
						FD_SET(servers[i], &SockSet);
					if (select(NumSocks, &SockSet, 0, 0, 0) == SOCKET_ERROR) {
						const auto err = WSAGetLastError();
						//core->printlog(socket::log_event_type::ERROR, "select() failed with error " + std::to_string(err));
						return INVALID_SOCKET;
					}
				}
				for (i = 0; i < NumSocks; i++) {
					if (FD_ISSET(servers[i], &SockSet)) {
						FD_CLR(servers[i], &SockSet);
						break;
					}
				}

				return servers[i];
			}

			SocketServer::SocketServer(SocketServer&& oth)
			{
				std::unique_lock<std::shared_mutex> l1(oth.self->secure, std::defer_lock);
				std::unique_lock<std::shared_mutex> l2(self->secure, std::defer_lock);
				std::lock(l1, l2);

				self = std::move(oth.self);
				oth.self = std::make_unique<_serverdata>();
			}

			void SocketServer::operator=(SocketServer&& oth)
			{
				std::unique_lock<std::shared_mutex> l1(oth.self->secure, std::defer_lock);
				std::unique_lock<std::shared_mutex> l2(self->secure, std::defer_lock);
				std::lock(l1, l2);

				self = std::move(oth.self);
				oth.self = std::make_unique<_serverdata>();

			}

			bool SocketServer::start(const u_short port, const socket::protocol& protocol, const socket::family& family)
			{
				std::unique_lock<std::shared_mutex> l(self->secure);
				self->closing = false;

				self->family = family;
				self->protocol = protocol;
				auto res = setup_server(self->sockets, protocol, family, port);
				return res == socket::result_type::NO_ERROR;
			}

			SocketClient SocketServer::listen()
			{
				std::shared_lock<std::shared_mutex> l(self->secure);

				while (!self->closing && self->sockets.size()) {
					switch (self->protocol) {
					case socket::protocol::TCP:
					{
						SOCKADDR_STORAGE From{};
						socklen_t FromLen = sizeof(SOCKADDR_STORAGE);

						SOCKET selected = common_select(self->sockets);
						if (selected == INVALID_SOCKET) {
							std::this_thread::yield();
							continue;
						}

						SOCKET sock = INVALID_SOCKET;

						if (sock = accept(selected, (LPSOCKADDR)&From, &FromLen); sock == INVALID_SOCKET) continue;

						SocketClient client;
						client.self->host = SocketInfo{ From, sock }; // bind socket here
						client.self->family = self->family;
						client.self->protocol = self->protocol;
						return std::move(client);
					}
						break;
					case socket::protocol::UDP:
					{
						SOCKADDR_STORAGE From{};
						socklen_t FromLen = sizeof(SOCKADDR_STORAGE);

						SOCKET selected = common_select(self->sockets);
						if (selected == INVALID_SOCKET) {
							std::this_thread::yield();
							continue;
						}

						char buf[2] = "1";
						if (::recvfrom(selected, buf, 1, 0, (sockaddr*)&From, &FromLen) != 1) continue; // can't read, something is not good									#1

						u_short newport = 0;
						std::vector<SOCKET> newsockets;

						auto res = setup_server(newsockets, self->protocol, self->family, 0);
						if (res != socket::result_type::NO_ERROR || newsockets.size() == 0) continue; // bad
						
						SOCKADDR_STORAGE addr{};
						socklen_t sockaddrlen = sizeof(SOCKADDR_STORAGE);
						if (getsockname(newsockets[0], (struct sockaddr*)(&addr), &sockaddrlen) < 0) throw std::exception("INVALID SOCKET! It wasn't bound");

						SocketClient client;
						client.self->host = SocketInfo{ addr, newsockets[0] }; // temporary local addr. Get port.

						newport = client.self->host.get_port();

						if (::sendto(selected, (char*)&newport, sizeof(newport), 0, (sockaddr*)&From, FromLen) != sizeof(newport)) continue; // can't send, something is not good				#2

						SOCKET newselect = common_select(newsockets);

						for (auto& i : newsockets) {
							if (i != newselect) closesocket(i);
						}
						newsockets.clear();

						//client.self->host = SocketInfo{ addr, newselect }; // temporary local addr. Get port.

						if (::recvfrom(newselect, buf, 1, 0, (sockaddr*)&From, &FromLen) != 1) continue; // can't read, something is not good?!									#3

						if (::sendto(newselect, buf, 1, 0, (sockaddr*)&From, FromLen) != 1) continue; // can't send, something is not good										#4

						client.self->host = SocketInfo{ From, newselect };
						client.self->family = self->family;
						client.self->protocol = self->protocol;
						return std::move(client);
					}
						break;
					}
				}

				throw std::exception("Something wrong happened while listen()ing.");
				return {};
			}

			void SocketServer::close()
			{
				self->closing = true;
				for (auto& i : self->sockets) closesocket(i); // quantity stays the same
				std::unique_lock<std::shared_mutex> l(self->secure); // lock so amount in list can be cleaned (if locked first, listen() may block)
				self->sockets.clear();
			}

			size_t SocketServer::has_servers() const
			{
				return self->sockets.size();
			}

		}
	}
}