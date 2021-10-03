#pragma once

#include <Lunaris/__macro/macros.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <stdexcept>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

namespace Lunaris {
	
	constexpr size_t socket_default_tcp_buffer_size = 1 << 10;
	constexpr size_t socket_maximum_udp_buffer_size = 508; // https://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet#:~:text=The%20maximum%20safe%20UDP%20payload%20is%20508%20bytes.&text=Any%20UDP%20payload%20this%20size,any%20router%20for%20any%20reason.

	struct socket_config {
		//enum class protocol { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };
		enum class e_family { IPV4 = PF_INET, IPV6 = PF_INET6, ANY = PF_UNSPEC };

		e_family family			= e_family::ANY;
		std::string ip_address; // defaults empty
		u_short port			= 50420;

		std::string format() const;

		socket_config& set_family(const e_family&);
		socket_config& set_port(const u_short&);
		socket_config& set_ip_address(const std::string&);
	};

	// start WSA / stop WSA
	class socket_core {
		struct _data {
			WSADATA wsaData = WSADATA();
			bool init_done = false;
			_data();
			~_data();
		};

		static _data data;
	public:
		// ip, port, protocol, family
		SOCKET gen_client(const char*, const u_short, const int, const int);

		// port, protocol, family
		std::vector<SOCKET> gen_host(const u_short, const int, const int);

		// common for server
		SOCKET common_select(std::vector<SOCKET>&, const long = 0);
	};

	template<int protocol, bool host>
	class socket : protected socket_core {

		virtual void close_socket() = 0;
		virtual void add_socket(SOCKET) = 0;
		virtual bool has_socket() = 0;
	public:
		socket() = default;

		bool setup(const socket_config&);

		static bool convert_from(socket_config&, const SOCKADDR_STORAGE&);
	};

	template<int protocol, bool host>
	class socket_client : public socket<protocol, host> {
	protected:
		struct _data {
			SOCKET connection = INVALID_SOCKET;
			SOCKADDR_STORAGE info_host{};
		};

		std::unique_ptr<_data> data = std::make_unique<_data>();

		void close_socket();
		void add_socket(SOCKET);
		bool has_socket();
	public:
		using socket<protocol, host>::setup;

		socket_client() = default;
		socket_client(SOCKET, const SOCKADDR_STORAGE&);
	};

	template<int protocol, bool host>
	class socket_host : public socket<protocol, host> {
	protected:
		struct _data {
			std::vector<SOCKET> listeners;
		};

		std::unique_ptr<_data> data = std::make_unique<_data>();

		void close_socket();
		void add_socket(SOCKET);
		bool has_socket();
	public:
		using socket<protocol, host>::setup;
	};



	class TCP_client : public socket_client<SOCK_STREAM, false> {
	public:
		using socket_client<SOCK_STREAM, false>::socket_client;
		using socket_client<SOCK_STREAM, false>::has_socket;
		using socket_client<SOCK_STREAM, false>::close_socket;

		bool send(const std::vector<char>&);
		std::vector<char> recv(const size_t = static_cast<size_t>(-1), const bool = true);
	};


	class TCP_host : public socket_host<SOCK_STREAM, true> {
	public:
		TCP_client listen(const long = 0); // timeout? 0 == block forever
	};


	class UDP_client : public socket_client<SOCK_DGRAM, false> {
		socket_config conf;
	public:
		using socket_client<SOCK_DGRAM, false>::socket_client;
		using socket_client<SOCK_DGRAM, false>::has_socket;
		using socket_client<SOCK_DGRAM, false>::close_socket;

		bool send(const std::vector<char>&);
		std::vector<char> recv(const size_t = static_cast<size_t>(-1), const bool = true);

		const socket_config& last_recv_info() const;
	};


	class UDP_host : public socket_host<SOCK_DGRAM, true> {
	public:
		class UDP_host_handler;

		UDP_host_handler recv(const size_t = static_cast<size_t>(-1), const long = 0); // package size, timeout time
	};

	class UDP_host::UDP_host_handler {
		SOCKADDR_STORAGE addr{};
		socket_config conf;
		SOCKET socket = INVALID_SOCKET;
		std::vector<char> data;

		friend class UDP_host;
		UDP_host_handler(SOCKET, const SOCKADDR_STORAGE&, std::vector<char>&&);
		UDP_host_handler() = default;
	public:
		bool valid() const;
		const std::vector<char>& get() const; // not recv
		bool send(const std::vector<char>&);
		SOCKADDR_STORAGE address() const;

		const socket_config& info() const; // read only, this shouldn't work as a config itself.
	};
	
}

#include "socket.ipp"