#pragma once
#include "socket.h"

namespace Lunaris {

#ifdef _WIN32
	inline socket_core::_data socket_core::data;
#endif

	inline std::string socket_config::format() const
	{
		switch (family) {
		case socket_config::e_family::IPV4:
			return ip_address + ":" + std::to_string(port);
		default:
			return "[" + ip_address + "]:" + std::to_string(port);
		}
	}

	inline socket_config& socket_config::set_family(const e_family& var)
	{
		family = var;
		return *this;
	}

	inline socket_config& socket_config::set_port(const u_short& var)
	{
		port = var;
		return *this;
	}

	inline socket_config& socket_config::set_ip_address(const std::string& var)
	{
		ip_address = var;
		return *this;
	}
#ifdef _WIN32
	inline socket_core::_data::_data()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // wake up
			throw std::exception("Socket core can't start!");
		}
	}

	inline socket_core::_data::~_data()
	{
		WSACleanup(); // end
	}
#endif
	inline SocketType socket_core::gen_client(const char* addr, const u_short port, const int protocol, const int family)
	{
		char Port[8]{};
		SocketAddrInfo Hints;
		SocketAddrInfo* AddrInfo = nullptr;
#ifdef _WIN32
		sprintf_s(Port, "%hu", port);
		ZeroMemory(&Hints, sizeof(Hints));
#else
		sprintf(Port, "%hu", port);
		memset(&Hints, 0, sizeof(Hints));
#endif

		Hints.ai_family = static_cast<int>(family);
		Hints.ai_socktype = protocol;

		if (getaddrinfo(addr ? addr : "localhost", Port, &Hints, &AddrInfo) != 0) return SocketInvalid;

		for (SocketAddrInfo* AI = AddrInfo; AI != nullptr; AI = AI->ai_next)
		{
			SocketType sock = SocketInvalid;

			if ((sock = ::socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol)) == SocketInvalid) continue;

			if (::connect(sock, AI->ai_addr, (int)AI->ai_addrlen) == SocketError) {
				closeSocket(sock);
				continue;
			}

			freeaddrinfo(AddrInfo);

			if (AI == NULL) return SocketInvalid;

			return sock;
		}

		freeaddrinfo(AddrInfo);
		return SocketInvalid;
	}

	inline std::vector<SocketType> socket_core::gen_host(const u_short port, const int protocol, const int family)
	{
		std::vector<SocketType> sockets;

		char Port[8]{};
		SocketAddrInfo Hints;
		SocketAddrInfo* AddrInfo = nullptr;

#ifdef _WIN32
		sprintf_s(Port, "%hu", port);
		ZeroMemory(&Hints, sizeof(Hints));
#else
		sprintf(Port, "%hu", port);
		memset(&Hints, 0, sizeof(Hints));
#endif

		Hints.ai_family = static_cast<int>(family);
		Hints.ai_socktype = static_cast<int>(protocol);
		Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

		if (getaddrinfo(nullptr, Port, &Hints, &AddrInfo) != 0) return {};

		SocketAddrInfo* AI = AddrInfo;
		for (int i = 0; AI != nullptr && i != FD_SETSIZE; AI = AI->ai_next)
		{
			if ((AI->ai_family != static_cast<int>(socket_config::e_family::IPV4)) && (AI->ai_family != static_cast<int>(socket_config::e_family::IPV6))) continue;

			SocketType newcon = SocketInvalid;

			if ((newcon = ::socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol)) == SocketInvalid) continue;

			{
				int on = 1;
				setsockopt(newcon, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
			}

			if (::bind(newcon, AI->ai_addr, (int)AI->ai_addrlen) == SocketError) {
				closeSocket(newcon);
				continue;
			}

			// Specific for TCP
			if (protocol == SOCK_STREAM) {
				if (::listen(newcon, 5) == SocketError) {
					closeSocket(newcon);
					continue;
				}
			}

			sockets.push_back(newcon);
			i++;
		}

		freeaddrinfo(AddrInfo);

		return sockets;
	}

	inline SocketType socket_core::common_select(std::vector<SocketType>& servers, const long to)
	{
		// Before POLL, only works on WINDOWS:
		// 
		//size_t i = 0;
		//fd_set SockSet{};
		//size_t NumSocks = servers.size();
		//
		//timeval timeout_time;
		//timeout_time.tv_sec = to;
		//timeout_time.tv_usec = 0;
		//
		//if (!NumSocks) {
		//	return SocketInvalid;
		//}
		//
		//FD_ZERO(&SockSet);
		//
		//for (i = 0; i < NumSocks; i++) {
		//	if (FD_ISSET(servers[i], &SockSet))
		//		break;
		//}
		//if (i == NumSocks) {
		//	for (i = 0; i < NumSocks; i++)
		//		FD_SET(servers[i], &SockSet);
		//	int thussock = ::select(static_cast<int>(NumSocks), &SockSet, nullptr, nullptr, ((to > 0) ? &timeout_time : nullptr));
		//	if (thussock == SocketError || thussock == SocketTimeout) {
		//		return SocketInvalid;
		//	}
		//}
		//for (i = 0; i < NumSocks; i++) {
		//	if (FD_ISSET(servers[i], &SockSet)) {
		//		FD_CLR(servers[i], &SockSet);
		//		break;
		//	}
		//}
		//
		//if (i >= servers.size()) return SocketInvalid;
		//
		//return servers[i];

		if (servers.size() == 0) return SocketInvalid;

		const unsigned long nfds = static_cast<unsigned long>(servers.size());

		std::unique_ptr<SocketPollFD[]> pul(new SocketPollFD[nfds]);
		memset(pul.get(), 0, sizeof(SocketPollFD) * nfds);

		for (size_t p = 0; p < nfds; p++) {
			pul[p].events = SocketPOLLIN; // SocketPOLLIN = check data to read
			pul[p].fd = servers[p];
		}

		int res = pollSocket(pul.get(), nfds, to);

		if (res < 0) {
			return SocketInvalid;
		}

		for (size_t pp = 0; pp < nfds; pp++) {
			auto& it = pul[pp];
			if (it.revents != SocketPOLLIN) continue; // not "read data available"
			if (std::find(servers.begin(), servers.end(), it.fd) != servers.end()) return it.fd; // this is the one.
		}

		return SocketInvalid;
	}


	template<int protocol, bool host>
	inline bool socket<protocol, host>::setup(const socket_config& config)
	{
		if (has_socket()) close_socket();

		if (host) {
			std::vector<SocketType> res = gen_host(config.port, protocol, static_cast<int>(config.family));
			if (!res.empty()) {
				for (auto& i : res) add_socket(i);
				return true;
			}
		}
		else {
			SocketType res = gen_client(config.ip_address.empty() ? nullptr : config.ip_address.c_str(), config.port, protocol, static_cast<int>(config.family));
			if (res != SocketInvalid) {
				add_socket(res);
				return true;
			}
		}
		return false;
	}

	template<int protocol, bool host>
	inline bool socket<protocol, host>::convert_from(socket_config& conf, const SocketStorage& addr)
	{
		const auto fun_get_in_addr = [&]() -> void* {
			const sockaddr* sa = (struct sockaddr*)(&addr);
			if (sa->sa_family == AF_INET) // IPv4 address
				return &(((struct sockaddr_in*)sa)->sin_addr);
			// else IPv6 address
			return &(((struct sockaddr_in6*)sa)->sin6_addr);
		};
		const auto fun_get_in_port = [&]() -> unsigned short {
			const sockaddr* sa = (struct sockaddr*)(&addr);
			if (sa->sa_family == AF_INET)
				return (((struct sockaddr_in*)sa)->sin_port); // IPv4 address
			return (((struct sockaddr_in6*)sa)->sin6_port);// else IPv6 address		
		};

		char name[INET6_ADDRSTRLEN + 1]{};
		socklen_t sockaddrlen = sizeof(addr);
		const sockaddr* sockaddr = (struct sockaddr*)(&addr);

		if (inet_ntop(sockaddr->sa_family, fun_get_in_addr(), name, INET6_ADDRSTRLEN * sizeof(char)) == NULL) return false;

		conf.ip_address = name;
		conf.family = sockaddr->sa_family == AF_INET ? socket_config::e_family::IPV4 : socket_config::e_family::IPV6;
		conf.port = ntohs(fun_get_in_port());

		return true;
	}

	template<int protocol, bool host>
	inline void socket_client<protocol, host>::close_socket()
	{
		if (data->connection != SocketInvalid) closeSocket(data->connection);
		data->connection = SocketInvalid;
	}

	template<int protocol, bool host>
	inline void socket_client<protocol, host>::add_socket(SocketType conn)
	{
		close_socket();
		data->connection = conn;
	}

	template<int protocol, bool host>
	inline bool socket_client<protocol, host>::has_socket()
	{
		return data->connection != SocketInvalid;
	}

	template<int protocol, bool host>
	inline socket_client<protocol, host>::socket_client(SocketType socket, const SocketStorage& addr)
	{
		if (socket == SocketInvalid) throw std::runtime_error("Invalid socket!");
		data->connection = socket;
		data->info_host = addr;
	}


	template<int protocol, bool host>
	inline void socket_host<protocol, host>::close_socket()
	{
		for (auto& i : data->listeners) { if (i != SocketInvalid) closeSocket(i); }
		data->listeners.clear();
	}

	template<int protocol, bool host>
	inline void socket_host<protocol, host>::add_socket(SocketType conn)
	{
		if (conn != SocketInvalid) data->listeners.push_back(conn);
	}

	template<int protocol, bool host>
	inline bool socket_host<protocol, host>::has_socket()
	{
		return data->listeners.size() != 0;
	}


	inline bool TCP_client::send(const std::vector<char>& raw)
	{
		if (!has_socket()) return false;

		for (size_t remaining = 0; remaining < raw.size();)
		{
			const size_t sending_size = raw.size() - remaining;
			int res = ::send(data->connection, raw.data() + remaining, static_cast<int>((sending_size > socket_default_tcp_buffer_size) ? socket_default_tcp_buffer_size : sending_size), 0);
			if (res <= 0) return false;
			remaining += res;
		}

		return true;
	}

	inline bool TCP_client::send(const char* raw, const size_t len)
	{
		if (!has_socket()) return false;

		for (size_t remaining = 0; remaining < len;)
		{
			const size_t sending_size = len - remaining;
			int res = ::send(data->connection, raw + remaining, static_cast<int>((sending_size > socket_default_tcp_buffer_size) ? socket_default_tcp_buffer_size : sending_size), 0);
			if (res <= 0) return false;
			remaining += res;
		}

		return true;
	}

	inline std::vector<char> TCP_client::recv(const size_t amount, const bool wait)
	{
		if (!has_socket()) return {};
		std::vector<char> raw;

		u_long iMode = (wait && amount != static_cast<size_t>(-1)) ? 0 : 1; // 0 blocks
		if (ioctlSocket(data->connection, FIONBIO, &iMode) != 0) throw std::runtime_error("Can't set socket property properly");

		size_t blocks_tries = 0;

		for (size_t remaining = 0; remaining < amount;)
		{
			char buf[socket_default_tcp_buffer_size]{};
			const size_t get_rn = amount - remaining;

			int res = ::recv(data->connection, buf, static_cast<int>((get_rn > socket_default_tcp_buffer_size) ? socket_default_tcp_buffer_size : (get_rn)), 0);

			if (res < 0) {
				int err = theSocketError;
				switch (err) {
				case SocketWOULDBLOCK: // no data to read
				{
					if (raw.size() && ++blocks_tries > 3) return raw;
					std::this_thread::yield();
					continue;
				}
				case SocketNETRESET: // failed in the middle of something
				case SocketCONNRESET: // still offline or became offline right now
					this->close_socket();
					return raw;
				}
			}
			else if (res == 0) { // disconnect
				this->close_socket();
				return raw;
			}
			else blocks_tries = 0;

			raw.insert(raw.end(), std::begin(buf), std::begin(buf) + res);
			remaining += res;
		}
		return raw;
	}

	template<typename T, std::enable_if_t<std::is_pod_v<T>, int>>
	inline bool TCP_client::recv(T& var, const bool wait)
	{
		auto vec = this->recv(sizeof(T), wait);
		if (vec.size() != sizeof(T)) return false;
#ifdef _WIN32
		return memcpy_s(&var, sizeof(var), vec.data(), vec.size()) == 0;
#else
		return memcpy(&var, vec.data(), vec.size()) != nullptr;
#endif
	}


	inline TCP_client TCP_host::listen(const long to)
	{
		SocketType selected = SocketInvalid;
		do {
			selected = common_select(data->listeners, to);
			//selected = data->listeners.size() ? data->listeners[0] : SocketInvalid;
			if (selected == SocketInvalid || selected == SocketError) {
				std::this_thread::yield();
				continue;
			}

			SocketStorage From{};
			socklen_t FromLen = sizeof(SocketStorage);

			SocketType sock = SocketInvalid;

			if (sock = accept(selected, (SocketSockAddrPtr)&From, &FromLen); sock == SocketInvalid) continue;

			return TCP_client{ sock, From };
		} while (to == 0);
		return {};
	}


	inline bool UDP_client::send(const std::vector<char>& raw)
	{
		if (!has_socket() || raw.size() > socket_maximum_udp_buffer_size) return false;

		int res = ::send(data->connection, raw.data(), static_cast<int>(raw.size()), 0);
		if (res < 0) close_socket();

		return res == raw.size();
	}

	inline bool UDP_client::send(const char* raw, const size_t len)
	{
		if (!has_socket() || len > socket_maximum_udp_buffer_size) return false;

		int res = ::send(data->connection, raw, static_cast<int>(len), 0);
		if (res < 0) close_socket();

		return res == len;
	}

	inline std::vector<char> UDP_client::recv(const size_t amount, const bool wait)
	{
		if (!has_socket() || (amount > socket_maximum_udp_buffer_size && amount != static_cast<size_t>(-1))) return {};
		std::vector<char> raw;

		u_long iMode = (wait && amount != static_cast<size_t>(-1)) ? 0 : 1; // 0 blocks
		if (ioctlSocket(data->connection, FIONBIO, &iMode) != 0) throw std::runtime_error("Can't set socket property properly");

		const int expected = static_cast<int>(amount > socket_maximum_udp_buffer_size ? socket_maximum_udp_buffer_size : amount);

		raw.resize(expected);

		for (size_t blocks_tries = 0; blocks_tries < 3; blocks_tries++) {

			SocketStorage _temp{};
			socklen_t _temp_len = sizeof(SocketStorage);

			int res = ::recvfrom(data->connection, raw.data(), expected, 0, (sockaddr*)&_temp, &_temp_len);

			if (res < 0) {
				int err = theSocketError;
				switch (err) {
				case SocketWOULDBLOCK: // no data to read
				{
					std::this_thread::yield();
					continue;
				}
				case SocketNETRESET: // failed in the middle of something
				case SocketCONNRESET: // still offline or became offline right now
					this->close_socket();
					return raw;
				}
			}
			else if (res == 0) { // disconnect
				this->close_socket();
				raw.clear();
				return raw;
			}
			/*else if (std::memcmp(&_temp, &data->info_host, _temp_len) != 0) {
				throw std::runtime_error("Unexpected elsewhere sending data to this!");
			}*/
			else if (res != expected) {
				throw std::runtime_error("Unexpected recv size!");
			}

			socket::convert_from(conf, _temp);
			break;
		}
		return raw;
	}

	template<typename T, std::enable_if_t<std::is_pod_v<T>, int>>
	inline bool UDP_client::recv(T& var, const bool wait)
	{
		auto vec = this->recv(sizeof(T), wait);
		if (vec.size() != sizeof(T)) return false;
#ifdef _WIN32
		return memcpy_s(&var, sizeof(var), vec.data(), vec.size()) == 0;
#else
		return memcpy(&var, vec.data(), vec.size()) != nullptr;
#endif
	}

	inline const socket_config& UDP_client::last_recv_info() const
	{
		return conf;
	}

	inline UDP_host::UDP_host_handler UDP_host::recv(const size_t amount, const long to)
	{
		std::vector<char> raw;
		const int expected = static_cast<int>(amount > socket_maximum_udp_buffer_size ? socket_maximum_udp_buffer_size : amount);
		raw.resize(expected);

		SocketStorage From{};
		socklen_t FromLen = sizeof(SocketStorage);

		SocketType selected = common_select(data->listeners, to);

		if (selected == SocketInvalid) return UDP_host_handler();

		SocketStorage _temp{};
		socklen_t _temp_len = sizeof(SocketStorage);

		int res = ::recvfrom(selected, raw.data(), expected, 0, (sockaddr*)&_temp, &_temp_len);
		if (res != expected) return UDP_host_handler();

		return UDP_host_handler(selected, _temp, std::move(raw));
	}

	inline UDP_host::UDP_host_handler::UDP_host_handler(SocketType sock, const SocketStorage& ad, std::vector<char>&& raw)
		: socket(sock), addr(ad), data(std::move(raw))
	{
		if (!socket::convert_from(conf, addr)) throw std::runtime_error("Could not get info properly! Data may be corrupted!");
	}

	inline bool UDP_host::UDP_host_handler::valid() const
	{
		return socket != SocketInvalid;
	}

	inline const std::vector<char>& UDP_host::UDP_host_handler::get() const // not recv
	{
		return data;
	}

	template<typename T, std::enable_if_t<std::is_pod_v<T>, int>>
	inline bool Lunaris::UDP_host::UDP_host_handler::get_as(T& var)
	{
		if (data.size() != sizeof(T)) return false;
#ifdef _WIN32
		return memcpy_s(&var, sizeof(var), data.data(), data.size()) == 0;
#else
		return memcpy(&var, data.data(), data.size()) != nullptr;
#endif
	}

	inline bool UDP_host::UDP_host_handler::send(const std::vector<char>& raw)
	{
		if (raw.size() > socket_maximum_udp_buffer_size) return false;

		int res = ::sendto(socket, raw.data(), static_cast<int>(raw.size()), 0, (sockaddr*)(&addr), sizeof(SocketStorage));

		return res == raw.size();
	}

	inline bool UDP_host::UDP_host_handler::send(const char* raw, const size_t len)
	{
		if (len > socket_maximum_udp_buffer_size) return false;

		int res = ::sendto(socket, raw, static_cast<int>(len), 0, (sockaddr*)(&addr), sizeof(SocketStorage));

		return res == len;
	}

	inline SocketStorage UDP_host::UDP_host_handler::address() const
	{
		return addr;
	}

	inline const socket_config& UDP_host::UDP_host_handler::info() const
	{
		return conf;
	}

}