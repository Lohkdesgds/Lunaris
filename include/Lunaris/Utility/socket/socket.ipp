#pragma once

namespace Lunaris {

	socket_core::_data socket_core::data;

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

	inline SOCKET socket_core::gen_client(const char* addr, const u_short port, const int protocol, const int family)
	{
		char Port[8]{};
		ADDRINFO Hints;
		ADDRINFO* AddrInfo = nullptr;

		sprintf_s(Port, "%hu", port);
		ZeroMemory(&Hints, sizeof(Hints));

		Hints.ai_family = static_cast<int>(family);
		Hints.ai_socktype = protocol;

		if (getaddrinfo(addr ? addr : "localhost", Port, &Hints, &AddrInfo) != 0) return INVALID_SOCKET;

		for (ADDRINFO* AI = AddrInfo; AI != nullptr; AI = AI->ai_next)
		{
			SOCKET sock = INVALID_SOCKET;

			if ((sock = ::socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol)) == INVALID_SOCKET) continue;

			if (::connect(sock, AI->ai_addr, (int)AI->ai_addrlen) == SOCKET_ERROR) {
				::closesocket(sock);
				continue;
			}
		
			freeaddrinfo(AddrInfo);

			if (AI == NULL) return INVALID_SOCKET;

			return sock;
		}

		freeaddrinfo(AddrInfo);
		return INVALID_SOCKET;
	}

	inline std::vector<SOCKET> socket_core::gen_host(const u_short port, const int protocol, const int family)
	{
		std::vector<SOCKET> sockets;

		char Port[8]{};
		ADDRINFO Hints;
		ADDRINFO* AddrInfo = nullptr;

		sprintf_s(Port, "%hu", port);
		ZeroMemory(&Hints, sizeof(Hints));

		Hints.ai_family = static_cast<int>(family);
		Hints.ai_socktype = static_cast<int>(protocol);
		Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

		if (getaddrinfo(nullptr, Port, &Hints, &AddrInfo) != 0) return {};

		ADDRINFO* AI = AddrInfo;
		for (int i = 0; AI != nullptr && i != FD_SETSIZE; AI = AI->ai_next)
		{
			if ((AI->ai_family != PF_INET) && (AI->ai_family != PF_INET6)) continue;

			SOCKET newcon = INVALID_SOCKET;

			if ((newcon = ::socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol)) == INVALID_SOCKET) continue;
		
			if (::bind(newcon, AI->ai_addr, (int)AI->ai_addrlen) == SOCKET_ERROR) {
				closesocket(newcon);
				continue;
			}

			// Specific for TCP
			if (protocol == SOCK_STREAM) {
				if (::listen(newcon, 5) == SOCKET_ERROR) {
					closesocket(newcon);
					continue;
				}
			}

			sockets.push_back(newcon);
			i++;
		}

		freeaddrinfo(AddrInfo);

		return sockets;
	}

	inline SOCKET socket_core::common_select(std::vector<SOCKET>& servers, const long to)
	{
		size_t i = 0;
		fd_set SockSet{};
		size_t NumSocks = servers.size();

		timeval timeout_time;
		timeout_time.tv_sec = to;
		timeout_time.tv_usec = 0;

		if (!NumSocks) {
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
			if (select(static_cast<int>(NumSocks), &SockSet, nullptr, nullptr, ((to > 0) ? &timeout_time : nullptr)) == SOCKET_ERROR) {
				return INVALID_SOCKET;
			}
		}
		for (i = 0; i < NumSocks; i++) {
			if (FD_ISSET(servers[i], &SockSet)) {
				FD_CLR(servers[i], &SockSet);
				break;
			}
		}

		if (i >= servers.size()) return INVALID_SOCKET;

		return servers[i];
	}


	template<int protocol, bool host>
	inline bool socket<protocol, host>::setup(const socket_config& config)
	{
		if (has_socket()) close_socket();

		if (host) {
			std::vector<SOCKET> res = gen_host(config.port, protocol, static_cast<int>(config.family));
			if (!res.empty()) {
				for(auto& i : res) add_socket(i);
				return true;
			}
		}
		else {
			SOCKET res = gen_client(config.ip_address.empty() ? nullptr : config.ip_address.c_str(), config.port, protocol, static_cast<int>(config.family));
			if (res != INVALID_SOCKET) {
				add_socket(res);
				return true;
			}
		}
		return false;
	}

	template<int protocol, bool host>
	inline bool socket<protocol, host>::convert_from(socket_config& conf, const SOCKADDR_STORAGE& addr)
	{
		const auto fun_get_in_addr = [&]() -> void* {
			const sockaddr* sa = (struct sockaddr*)(&addr);
			if (sa->sa_family == AF_INET) // IPv4 address
				return &(((struct sockaddr_in*)sa)->sin_addr);
			// else IPv6 address
			return &(((struct sockaddr_in6*)sa)->sin6_addr);
		};
		const auto fun_get_in_port = [&]() -> USHORT {
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
		if (data->connection != INVALID_SOCKET) ::closesocket(data->connection);
		data->connection = INVALID_SOCKET;
	}

	template<int protocol, bool host>
	inline void socket_client<protocol, host>::add_socket(SOCKET conn)
	{
		close_socket();
		data->connection = conn;
	}

	template<int protocol, bool host>
	inline bool socket_client<protocol, host>::has_socket()
	{
		return data->connection != INVALID_SOCKET;
	}

	template<int protocol, bool host>
	inline socket_client<protocol, host>::socket_client(SOCKET socket, const SOCKADDR_STORAGE& addr)
	{
		if (socket == INVALID_SOCKET) throw std::runtime_error("Invalid socket!");
		data->connection = socket;
		data->info_host = addr;
	}


	template<int protocol, bool host>
	inline void socket_host<protocol, host>::close_socket()
	{
		for (auto& i : data->listeners) { if (i != INVALID_SOCKET) ::closesocket(i); }
		data->listeners.clear();
	}

	template<int protocol, bool host>
	inline void socket_host<protocol, host>::add_socket(SOCKET conn)
	{
		if (conn != INVALID_SOCKET) data->listeners.push_back(conn);
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

	inline std::vector<char> TCP_client::recv(const size_t amount, const bool wait)
	{
		if (!has_socket()) return {};
		std::vector<char> raw;

		u_long iMode = (wait && amount != static_cast<size_t>(-1)) ? 0 : 1; // 0 blocks
		if (ioctlsocket(data->connection, FIONBIO, &iMode) != 0) throw std::runtime_error("Can't set socket property properly");

		size_t blocks_tries = 0;

		for (size_t remaining = 0; remaining < amount;)
		{
			char buf[socket_default_tcp_buffer_size]{};
			const size_t get_rn = amount - remaining;

			int res = ::recv(data->connection, buf, static_cast<int>((get_rn > socket_default_tcp_buffer_size) ? socket_default_tcp_buffer_size : (get_rn)), 0);

			if (res < 0) {
				int err = WSAGetLastError();
				switch (err) {
				case WSAEWOULDBLOCK: // no data to read
				{
					if (raw.size() && ++blocks_tries > 3) return raw;
					std::this_thread::yield();
					continue;
				}
				case WSAENETRESET: // failed in the middle of something
				case WSAECONNRESET: // still offline or became offline right now
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


	inline TCP_client TCP_host::listen(const long to)
	{
		SOCKET selected = INVALID_SOCKET;
		while (1) {
			selected = common_select(data->listeners, to);
			if (selected == INVALID_SOCKET) {
				std::this_thread::yield();
				continue;
			}

			SOCKADDR_STORAGE From{};
			socklen_t FromLen = sizeof(SOCKADDR_STORAGE);

			SOCKET sock = INVALID_SOCKET;

			if (sock = accept(selected, (LPSOCKADDR)&From, &FromLen); sock == INVALID_SOCKET) continue;

		
			return TCP_client{ sock, From };
		}
		return {};
	}


	inline bool UDP_client::send(const std::vector<char>& raw)
	{
		if (!has_socket() || raw.size() > socket_maximum_udp_buffer_size) return false;

		int res = ::send(data->connection, raw.data(), static_cast<int>(raw.size()), 0);
		if (res < 0) close_socket();

		return res == raw.size();
	}

	inline std::vector<char> UDP_client::recv(const size_t amount, const bool wait)
	{
		if (!has_socket() || (amount > socket_maximum_udp_buffer_size && amount != static_cast<size_t>(-1))) return {};
		std::vector<char> raw;

		u_long iMode = (wait && amount != static_cast<size_t>(-1)) ? 0 : 1; // 0 blocks
		if (ioctlsocket(data->connection, FIONBIO, &iMode) != 0) throw std::runtime_error("Can't set socket property properly");

		const int expected = static_cast<int>(amount > socket_maximum_udp_buffer_size ? socket_maximum_udp_buffer_size : amount);

		raw.resize(expected);

		for (size_t blocks_tries = 0; blocks_tries < 3; blocks_tries++) {

			SOCKADDR_STORAGE _temp{};
			int _temp_len = sizeof(SOCKADDR_STORAGE);

			int res = ::recvfrom(data->connection, raw.data(), expected, 0, (sockaddr*)&_temp, &_temp_len);

			if (res < 0) {
				int err = WSAGetLastError();
				switch (err) {
				case WSAEWOULDBLOCK: // no data to read
				{
					std::this_thread::yield();
					continue;
				}
				case WSAENETRESET: // failed in the middle of something
				case WSAECONNRESET: // still offline or became offline right now
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

	inline const socket_config& UDP_client::last_recv_info() const
	{
		return conf;
	}

	inline UDP_host::UDP_host_handler UDP_host::recv(const size_t amount, const long to)
	{
		std::vector<char> raw;
		const int expected = static_cast<int>(amount > socket_maximum_udp_buffer_size ? socket_maximum_udp_buffer_size : amount);
		raw.resize(expected);

		SOCKADDR_STORAGE From{};
		socklen_t FromLen = sizeof(SOCKADDR_STORAGE);

		SOCKET selected = common_select(data->listeners, to);

		if (selected == INVALID_SOCKET) return UDP_host_handler();

		SOCKADDR_STORAGE _temp{};
		int _temp_len = sizeof(SOCKADDR_STORAGE);

		int res = ::recvfrom(selected, raw.data(), expected, 0, (sockaddr*)&_temp, &_temp_len);
		if (res != expected) return UDP_host_handler();

		return UDP_host_handler(selected, _temp, std::move(raw));
	}

	inline UDP_host::UDP_host_handler::UDP_host_handler(SOCKET sock, const SOCKADDR_STORAGE& ad, std::vector<char>&& raw)
		: socket(sock), addr(ad), data(std::move(raw))
	{
		if (!socket::convert_from(conf, addr)) throw std::runtime_error("Could not get info properly! Data may be corrupted!");
	}

	inline bool UDP_host::UDP_host_handler::valid() const
	{
		return socket != INVALID_SOCKET;
	}

	inline const std::vector<char>& UDP_host::UDP_host_handler::get() const // not recv
	{
		return data;
	}

	inline bool UDP_host::UDP_host_handler::send(const std::vector<char>& raw)
	{
		if (raw.size() > socket_maximum_udp_buffer_size) return false;

		int res = ::sendto(socket, raw.data(), static_cast<int>(raw.size()), 0, (sockaddr*)(&addr), sizeof(SOCKADDR_STORAGE));

		return res == raw.size();
	}

	inline SOCKADDR_STORAGE UDP_host::UDP_host_handler::address() const
	{
		return addr;
	}

	inline const socket_config& UDP_host::UDP_host_handler::info() const
	{
		return conf;
	}

}