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
#include <sys/poll.h>
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
#include <functional>

#ifdef _WIN32
using SocketType = SOCKET;
using SocketStorage = SOCKADDR_STORAGE;
using SocketAddrInfo = ADDRINFO;
using SocketSockAddrPtr = LPSOCKADDR;
using SocketPollFD = WSAPOLLFD;
constexpr SocketType SocketInvalid = INVALID_SOCKET;
constexpr int SocketError = SOCKET_ERROR;
constexpr int SocketTimeout = 0;
#define closeSocket(...) ::closesocket(__VA_ARGS__)
#define ioctlSocket(...) ::ioctlsocket(__VA_ARGS__)
#define pollSocket(...) ::WSAPoll(__VA_ARGS__)
#define theSocketError WSAGetLastError()
#define SocketWOULDBLOCK WSAEWOULDBLOCK
#define SocketNETRESET WSAENETRESET
#define SocketCONNRESET WSAECONNRESET
#define SocketPOLLIN POLLRDNORM
#else
using SocketType = int;
using SocketStorage = sockaddr_storage;
using SocketAddrInfo = addrinfo;
using SocketSockAddrPtr = sockaddr*;
using SocketPollFD = pollfd;
constexpr SocketType SocketInvalid = -1;
constexpr int SocketError = -1;
constexpr int SocketTimeout = 0;
#define closeSocket(...) ::close(__VA_ARGS__)
#define ioctlSocket(...) ::ioctl(__VA_ARGS__)
#define pollSocket(...) ::poll(__VA_ARGS__)
#define theSocketError errno
#define SocketWOULDBLOCK EWOULDBLOCK
#define SocketNETRESET ENETRESET
#define SocketCONNRESET ECONNRESET
#define SocketPOLLIN POLLIN
#endif


namespace Lunaris {

	constexpr size_t socket_default_tcp_buffer_size = 1 << 10;
	constexpr size_t socket_maximum_udp_buffer_size = 508; // https://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet#:~:text=The%20maximum%20safe%20UDP%20payload%20is%20508%20bytes.&text=Any%20UDP%20payload%20this%20size,any%20router%20for%20any%20reason.

	/// <summary>
	/// <para>This is used to setup a socket.</para>
	/// </summary>
	struct socket_config {
		enum class e_family { 
			IPV4 = PF_INET,		// IPV4 is the good old XXX.XXX.XXX.XXX format IP
			IPV6 = PF_INET6,	// IPV6 is the new FFFF:FFFF:FFFF....FFFF:FFFF stuff (hex)
			ANY = PF_UNSPEC		// This is when you don't care or both.
		};

		e_family family = e_family::ANY;
		std::string ip_address;
		u_short port = 50420;

		/// <summary>
		/// <para>Prints the ip and port for you as a string.</para>
		/// </summary>
		/// <returns>{std::string} Formatted IP and port.</returns>
		std::string format() const;

		/// <summary>
		/// <para>Set if it should use IPV4, IPV6 or any (client tries both, host opens host for both).</para>
		/// </summary>
		/// <param name="{e_family}">The family you want.</param>
		/// <returns>{socket_config&amp;} Itself.</returns>
		socket_config& set_family(const e_family&);

		/// <summary>
		/// <para>What port are you using?</para>
		/// </summary>
		/// <param name="{u_short}">A port (less than 65536).</param>
		/// <returns>{socket_config&amp;} Itself.</returns>
		socket_config& set_port(const u_short&);

		/// <summary>
		/// <para>What's the IP address? (if host, this does nothing. This should work with URL string too (for clients)).</para>
		/// </summary>
		/// <param name="{std::string}">The IP/path.</param>
		/// <returns>{socket_config&amp;} Itself.</returns>
		socket_config& set_ip_address(const std::string&);

		/// <summary>
		/// <para>Get back information from a SocketStorage (translation).</para>
		/// </summary>
		/// <param name="{SocketStorage}">Socket IP information.</param>
		/// <returns>{bool} True if success.</returns>
		bool parse(const SocketStorage&);
	};

	/// <summary>
	/// <para>This is the internal initializer thing.</para>
	/// </summary>
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
		/// <summary>
		/// <para>Generate a client.</para>
		/// </summary>
		/// <param name="{char*}">IP.</param>
		/// <param name="{u_short}">Port.</param>
		/// <param name="{int}">Protocol.</param>
		/// <param name="{int}">Family.</param>
		/// <returns>{SocketType} The resulting socket.</returns>
		SocketType gen_client(const char*, const u_short, const int, const int);

		/// <summary>
		/// <para>Generate host sockets.</para>
		/// </summary>
		/// <param name="{u_short}">Port.</param>
		/// <param name="{int}">Protocol.</param>
		/// <param name="{int}">Family.</param>
		/// <returns>{vector&lt;SocketType&gt;} The resulting combo of sockets.</returns>
		std::vector<SocketType> gen_host(const u_short, const int, const int);

		// common for server

		/// <summary>
		/// <para>Select the socket with something ready.</para>
		/// <para>This is used by HOST.</para>
		/// </summary>
		/// <param name="{vector&lt;SocketType&gt;}">List of sockets.</param>
		/// <param name="{long}">Timeout in seconds. 0 means infinite.</param>
		/// <returns>{SocketType} A valid socket if successful.</returns>
		SocketType common_select(std::vector<SocketType>&, const long = 0);
	};

	/// <summary>
	/// <para>This is an abstract socket class with the format of the things and look.</para>
	/// </summary>
	template<int protocol, bool host>
	class socket : protected socket_core {
	protected:
		virtual void close_socket() = 0;
		virtual void add_socket(SocketType) = 0;
		virtual bool has_socket() const = 0;
		virtual bool valid() const = 0;
		virtual bool empty() const = 0;
	public:
		socket() = default;

		/// <summary>
		/// <para>Create a socket with given information.</para>
		/// </summary>
		/// <param name="{socket_config}">Your configuration.</param>
		/// <returns>{bool} True if success.</returns>
		bool setup(const socket_config&);
	};

	/// <summary>
	/// <para>This is the barebone client class (use TCP_client or UDP_client instead).</para>
	/// </summary>
	template<int protocol, bool host>
	class socket_client : public socket<protocol, host> {
	protected:
		struct _data {
			SocketType connection = SocketInvalid;
			SocketStorage info_host{};
		};

		std::unique_ptr<_data> data = std::make_unique<_data>();

		/// <summary>
		/// <para>Close the connection.</para>
		/// </summary>
		void close_socket();

		/// <summary>
		/// <para>Set the internal socket to this.</para>
		/// </summary>
		/// <param name="{SocketType}">The socket.</param>
		void add_socket(SocketType);

		/// <summary>
		/// <para>Whether there's a valid socket set or not.</para>
		/// </summary>
		/// <returns>{bool} True if has valid socket.</returns>
		bool has_socket() const;

		/// <summary>
		/// <para>If it has a socket, it's considered valid.</para>
		/// </summary>
		/// <returns>{bool} True if has valid socket.</returns>
		bool valid() const;

		/// <summary>
		/// <para>If it has no valid socket, it's considered empty.</para>
		/// </summary>
		/// <returns>{bool} True if has no valid socket.</returns>
		bool empty() const;
	public:
		using socket<protocol, host>::setup;

		socket_client() = default;

		/// <summary>
		/// <para>Make this client this socket with this information (copy).</para>
		/// </summary>
		/// <param name="{SocketType}">The socket.</param>
		/// <param name="{SocketStorate}">Socket information.</param>
		socket_client(SocketType, const SocketStorage&);

		/// <summary>
		/// <para>Get socket information in socket_config format.</para>
		/// </summary>
		/// <returns>{socket_config} Socket information.</returns>
		socket_config info() const;
	};

	/// <summary>
	/// <para>This is the barebone host class (use TCP_host or UDP_host instead).</para>
	/// </summary>
	template<int protocol, bool host>
	class socket_host : public socket<protocol, host> {
	protected:
		struct _data {
			std::vector<SocketType> listeners;
		};

		std::unique_ptr<_data> data = std::make_unique<_data>();

		/// <summary>
		/// <para>Close all connections</para>
		/// </summary>
		void close_socket();

		/// <summary>
		/// <para>Add a socket to internal list of sockets.</para>
		/// </summary>
		/// <param name="{SocketType}">The socket.</param>
		void add_socket(SocketType);

		/// <summary>
		/// <para>Has any socket set?</para>
		/// </summary>
		/// <returns>{bool} True if has more than 0.</returns>
		bool has_socket() const;

		/// <summary>
		/// <para>If it has at least one valid socket, it's considered valid.</para>
		/// </summary>
		/// <returns>{bool} True if has one or more valid sockets.</returns>
		bool valid() const;

		/// <summary>
		/// <para>If it has no valid socket, it's considered empty.</para>
		/// </summary>
		/// <returns>{bool} True if has no valid socket.</returns>
		bool empty() const;
	public:
		using socket<protocol, host>::setup;
	};

	/// <summary>
	/// <para>TCP_client, as the name suggests, is a socket handler as client using TCP protocol.</para>
	/// </summary>
	class TCP_client : public socket_client<SOCK_STREAM, false> {
	public:
		using socket_client<SOCK_STREAM, false>::socket_client;
		using socket_client<SOCK_STREAM, false>::has_socket;
		using socket_client<SOCK_STREAM, false>::valid;
		using socket_client<SOCK_STREAM, false>::empty;
		using socket_client<SOCK_STREAM, false>::close_socket;
		using socket_client<SOCK_STREAM, false>::info;

		/// <summary>
		/// <para>Send some data through the socket.</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">The data to send.</param>
		/// <returns>{bool} True if success.</returns>
		bool send(const std::vector<char>&);

		/// <summary>
		/// <para>Send some raw data through the socket.</para>
		/// </summary>
		/// <param name="{char*}">Buffer.</param>
		/// <param name="{size_t}">Buffer length.</param>
		/// <returns>{bool} True if success.</returns>
		bool send(const char*, const size_t);

		/// <summary>
		/// <para>Tries to recv some data.</para>
		/// </summary>
		/// <param name="{size_t}">Wait for how much data?</param>
		/// <param name="{bool}">Wait for something? (false tries some times without locking and returns what it got).</param>
		/// <returns>{vector&lt;char&gt;} What it got. If it was waiting, probably the size you expected. If disconnected or no wait and no data, maybe less than that or zero.</returns>
		std::vector<char> recv(const size_t = static_cast<size_t>(-1), const bool = true);

		/// <summary>
		/// <para>Tries to recv directly into a variable (using its size as expected size to receive).</para>
		/// <para>If no wait is set and the amount of data read is less than the variable size, you lose the data.</para>
		/// </summary>
		/// <param name="{T&amp;}">Variable being set.</param>
		/// <param name="{bool}">Wait for package? (You should, if you know there will be one there. Data is lost if the package is not the same size as this.)</param>
		/// <param name="{function}">Fallback. If can't translate, you can still get the data you've got through this function. The vector is deleted, so copy or move the data from it.</param>
		/// <returns>{bool} True if completely filled.</returns>
		template<typename T, std::enable_if_t<std::is_pod_v<T>, int> = 0>
		bool recv(T&, const bool = true, const std::function<void(std::vector<char>&)> = {});
	};

	/// <summary>
	/// <para>TCP_host is the hosting socket manager that can listen to new clients and enables you to have a TCP host.</para>
	/// <para>You use a TCP_client for each new connection.</para>
	/// </summary>
	class TCP_host : public socket_host<SOCK_STREAM, true> {
	public:
		using socket_host<SOCK_STREAM, true>::has_socket;
		using socket_host<SOCK_STREAM, true>::valid;
		using socket_host<SOCK_STREAM, true>::empty;
		using socket_host<SOCK_STREAM, true>::close_socket;

		/// <summary>
		/// <para>Listen for a client for a while (or forever).</para>
		/// </summary>
		/// <param name="{long}">For how long? 0 means infinite. Time in seconds.</param>
		/// <returns>{TCP_client} Hopefully a valid client (has_socket() will return true if valid)</returns>
		TCP_client listen(const long = 0);
	};

	/// <summary>
	/// <para>UDP_client, as the name suggests, is a socket handler as client using UDP protocol.</para>
	/// </summary>
	class UDP_client : public socket_client<SOCK_DGRAM, false> {
		socket_config conf;
	public:
		using socket_client<SOCK_DGRAM, false>::socket_client;
		using socket_client<SOCK_DGRAM, false>::has_socket;
		using socket_client<SOCK_DGRAM, false>::valid;
		using socket_client<SOCK_DGRAM, false>::empty;
		using socket_client<SOCK_DGRAM, false>::close_socket;

		/// <summary>
		/// <para>Send some data through the socket.</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">The data to send.</param>
		/// <returns>{bool} True if success.</returns>
		bool send(const std::vector<char>&);

		/// <summary>
		/// <para>Send some raw data through the socket.</para>
		/// </summary>
		/// <param name="{char*}">Buffer.</param>
		/// <param name="{size_t}">Buffer length.</param>
		/// <returns>{bool} True if success.</returns>
		bool send(const char*, const size_t);

		/// <summary>
		/// <para>Tries to recv some data.</para>
		/// </summary>
		/// <param name="{size_t}">Wait for how much data?</param>
		/// <param name="{bool}">Wait for something? (false tries some times without locking and returns what it got).</param>
		/// <returns>{vector&lt;char&gt;} What it got. If it was waiting, probably the size you expected. If disconnected or no wait and no data, maybe less than that or zero.</returns>
		std::vector<char> recv(const size_t = static_cast<size_t>(-1), const bool = true);

		/// <summary>
		/// <para>Tries to recv directly into a variable (using its size as expected size to receive).</para>
		/// <para>If no wait is set and the amount of data read is less than the variable size, you lose the data.</para>
		/// </summary>
		/// <param name="{T&amp;}">Variable being set.</param>
		/// <param name="{bool}">Wait for package? (You should, if you know there will be one there. Data is lost if the package is not the same size as this.)</param>
		/// <returns>{bool} True if completely filled.</returns>
		template<typename T, std::enable_if_t<std::is_pod_v<T>, int> = 0>
		bool recv(T&, const bool = true);

		/// <summary>
		/// <para>Get socket information in socket_config format.</para>
		/// </summary>
		/// <returns>{socket_config} Socket information.</returns>
		const socket_config& info() const;
	};

	/// <summary>
	/// <para>UDP_host works like a server. The sockets in it can receive data from anyone, and you use the "from" information to answer back.</para>
	/// <para>UDP is connectionless, so what this means is that the "clients" from this are not real unique clients, they're just a way to answer back using the socket that got the data in first place.</para>
	/// <para>Because of that, there's no way to "recv" a specific address. You are always listening to everyone and answering as things happen.</para>
	/// </summary>
	class UDP_host : public socket_host<SOCK_DGRAM, true> {
	public:
		class UDP_host_handler;

		using socket_host<SOCK_DGRAM, true>::has_socket;
		using socket_host<SOCK_DGRAM, true>::valid;
		using socket_host<SOCK_DGRAM, true>::empty;
		using socket_host<SOCK_DGRAM, true>::close_socket;

		/// <summary>
		/// <para>Like TCP's listen, but it actually gets the data already. It's more like a client of any package.</para>
		/// <para>Be sure that the package size is correct, because UDP is all about blocks of data.</para>
		/// </summary>
		/// <param name="{size_t}">The package size.</param>
		/// <param name="{long}">Timeout, in seconds. Zero means infinite.</param>
		/// <returns>{UDP_host_handler} A UDP client-like object that you can answer directly to. You can't recv from it because all requests goes to this original host.</returns>
		UDP_host_handler recv(const size_t, const long = 0); // package size, timeout time
	};

	/// <summary>
	/// <para>This is the way you can answer back to whoever sent you something. Sadly, as UDP is "from anyone", you can't recv from this, because it does not make sense.</para>
	/// <para>The socket is the one that can listen to anyone, that's why it makes no sense.</para>
	/// </summary>
	class UDP_host::UDP_host_handler {
		SocketStorage addr{};
		socket_config conf;
		SocketType socket = SocketInvalid;
		std::vector<char> data;

		friend class UDP_host;

		UDP_host_handler(SocketType, const SocketStorage&, std::vector<char>&&);
		UDP_host_handler() = default;
	public:
		/// <summary>
		/// <para>Whether this object has a valid socket or not.</para>
		/// </summary>
		/// <returns>{bool} True if has valid socket in it.</returns>
		bool valid() const;

		/// <summary>
		/// <para>Whether this object has no valid socket in it.</para>
		/// </summary>
		/// <returns>{bool} True if no valid socket.</returns>
		bool empty() const;

		/// <summary>
		/// <para>The data got from recv before.</para>
		/// </summary>
		/// <returns>{vector&lt;char&gt;&amp;} Internal buffer with recv content.</returns>
		const std::vector<char>& get() const;

		/// <summary>
		/// <para>Write the internal buffer to this type directly.</para>
		/// <para>The variable type MUST have the same size as the package.</para>
		/// </summary>
		/// <param name="{T}">The variable to copy to.</param>
		/// <returns>{bool} True if success.</returns>
		template<typename T, std::enable_if_t<std::is_pod_v<T>, int> = 0>
		bool get_as(T&);

		/// <summary>
		/// <para>Size of the package stored from recv.</para>
		/// </summary>
		/// <returns>{size_t} Size of the package.</returns>
		size_t size() const;

		/// <summary>
		/// <para>Send some data through the socket.</para>
		/// </summary>
		/// <param name="{vector&lt;char&gt;}">The data to send.</param>
		/// <returns>{bool} True if success.</returns>
		bool send(const std::vector<char>&);

		/// <summary>
		/// <para>Send some raw data through the socket.</para>
		/// </summary>
		/// <param name="{char*}">Buffer.</param>
		/// <param name="{size_t}">Buffer length.</param>
		/// <returns>{bool} True if success.</returns>
		bool send(const char*, const size_t);

		/// <summary>
		/// <para>Literal address in the SocketStorage format.</para>
		/// </summary>
		/// <returns>{SocketStorage} Its address in SocketStorage format.</returns>
		SocketStorage address() const;

		/// <summary>
		/// <para>Translated socket information as socket_config.</para>
		/// <para>You shouldn't use this as a configuration to a new socket. It'll fail, probably.</para>
		/// </summary>
		/// <returns>{socket_config} Address information in socket_config format.</returns>
		const socket_config& info() const;
	};

}

#include "socket.ipp"