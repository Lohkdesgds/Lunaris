#pragma once

#include <Lunaris/__macro/macros.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#endif

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <stdexcept>

#ifdef _WIN32
using SocketType = SOCKET;
using SocketStorage = SOCKADDR_STORAGE;
using SocketAddrInfo = ADDRINFO;
using SocketSockAddrPtr = LPSOCKADDR;
constexpr SocketType SocketInvalid = INVALID_SOCKET;
constexpr int SocketError = SOCKET_ERROR;
constexpr int SocketTimeout = 0;
#define closeSocket(...) ::closesocket(__VA_ARGS__)
#define ioctlSocket(...) ::ioctlsocket(__VA_ARGS__)
#define theSocketError WSAGetLastError()
#define SocketWOULDBLOCK WSAEWOULDBLOCK
#define SocketNETRESET WSAENETRESET
#define SocketCONNRESET WSAECONNRESET
#else
using SocketType = int;
using SocketStorage = sockaddr_storage;
using SocketAddrInfo = addrinfo;
using SocketSockAddrPtr = sockaddr*;
constexpr SocketType SocketInvalid = -1;
constexpr int SocketError = -1;
constexpr int SocketTimeout = 0;
#define closeSocket(...) ::close(__VA_ARGS__)
#define ioctlSocket(...) ::ioctl(__VA_ARGS__)
#define theSocketError errno
#define SocketWOULDBLOCK EWOULDBLOCK
#define SocketNETRESET ENETRESET
#define SocketCONNRESET ECONNRESET
#endif


namespace Lunaris {

	constexpr size_t socket_default_tcp_buffer_size = 1 << 10;
	constexpr size_t socket_maximum_udp_buffer_size = 508; // https://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet#:~:text=The%20maximum%20safe%20UDP%20payload%20is%20508%20bytes.&text=Any%20UDP%20payload%20this%20size,any%20router%20for%20any%20reason.

	struct socket_config {
		//enum class protocol { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };
		enum class e_family { IPV4 = PF_INET, IPV6 = PF_INET6, ANY = PF_UNSPEC };

		e_family family = e_family::ANY;
		std::string ip_address; // defaults empty
		u_short port = 50420;

		std::string format() const;

		socket_config& set_family(const e_family&);
		socket_config& set_port(const u_short&);
		socket_config& set_ip_address(const std::string&);
	};

	// start WSA / stop WSA
	class socket_core {
#ifdef _WIN32
		struct _data {
			WSADATA wsaData = WSADATA();
			bool init_done = false;
			_data();
			~_data();
		};

		static _data data;
#endif
	public:
		// ip, port, protocol, family
		SocketType gen_client(const char*, const u_short, const int, const int);

		// port, protocol, family
		std::vector<SocketType> gen_host(const u_short, const int, const int);

		// common for server
		SocketType common_select(std::vector<SocketType>&, const long = 0);
	};

	template<int protocol, bool host>
	class socket : protected socket_core {

		virtual void close_socket() = 0;
		virtual void add_socket(SocketType) = 0;
		virtual bool has_socket() = 0;
	public:
		socket() = default;

		bool setup(const socket_config&);

		static bool convert_from(socket_config&, const SocketStorage&);
	};

	template<int protocol, bool host>
	class socket_client : public socket<protocol, host> {
	protected:
		struct _data {
			SocketType connection = SocketInvalid;
			SocketStorage info_host{};
		};

		std::unique_ptr<_data> data = std::make_unique<_data>();

		void close_socket();
		void add_socket(SocketType);
		bool has_socket();
	public:
		using socket<protocol, host>::setup;

		socket_client() = default;
		socket_client(SocketType, const SocketStorage&);
	};

	template<int protocol, bool host>
	class socket_host : public socket<protocol, host> {
	protected:
		struct _data {
			std::vector<SocketType> listeners;
		};

		std::unique_ptr<_data> data = std::make_unique<_data>();

		void close_socket();
		void add_socket(SocketType);
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
		SocketStorage addr{};
		socket_config conf;
		SocketType socket = SocketInvalid;
		std::vector<char> data;

		friend class UDP_host;
		UDP_host_handler(SocketType, const SocketStorage&, std::vector<char>&&);
		UDP_host_handler() = default;
	public:
		bool valid() const;
		const std::vector<char>& get() const; // not recv
		bool send(const std::vector<char>&);
		SocketStorage address() const;

		const socket_config& info() const; // read only, this shouldn't work as a config itself.
	};

}

#include "socket.ipp"