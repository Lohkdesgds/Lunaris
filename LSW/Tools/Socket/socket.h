#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <string>
#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <functional>
#include <optional>

#include "../Buffer/buffer.h"

#undef min
#undef max
#undef ERROR
#undef NO_ERROR

namespace LSW {
	namespace v5 {
		namespace Tools {

			namespace socket {

				enum class protocol { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };
				enum class family { IPV4 = PF_INET, IPV6 = PF_INET6, ANY = PF_UNSPEC };
				enum class result_type { NO_ERROR, CANT_RESOLVE_HOSTNAME, CANT_CONNECT, FAILED_ROUTE_ADDRESSING, SERVER_COULD_NOT_LISTEN, FAILED_TO_SYNC, CANT_START_LISTENER_AUTO };

				constexpr size_t max_ip_name_len = INET6_ADDRSTRLEN;
				constexpr u_short port_default = 60000;
				constexpr size_t default_max_package = 1 << 15;

			}
			
			/// <summary>
			/// <para>Socket address and management.</para>
			/// </summary>
			class SocketInfo {
				SOCKET socket = INVALID_SOCKET;
				SOCKADDR_STORAGE addr{};
				char name[socket::max_ip_name_len + 1]{};
				u_short port = 0;

				void* _aux_get_in_addr();
				u_short _aux_get_in_port();

				bool gen_data();

				friend class SocketClient; // only client can use empty constructor

				SocketInfo() = default;

				// bool -> blocking?
				bool set_socket_buffering_block(const bool) const;
			public:
				/// <summary>
				/// <para>Set SOCKET and address information directly.</para>
				/// </summary>
				/// <param name="{SOCKADDR_STORAGE}">Sockaddr information.</param>
				/// <param name="{SOCKET}">The SOCKET.</param>
				SocketInfo(const SOCKADDR_STORAGE&, const SOCKET);

				/// <summary>
				/// <para>Set SOCKET directly.</para>
				/// <para>This should be from a binded source (connect or somewhere that getpeername returns something to work 100%).</para>
				/// </summary>
				/// <param name="{SOCKET}">The SOCKET.</param>
				SocketInfo(SOCKET);

				SocketInfo(const SocketInfo&) = delete;
				void operator=(const SocketInfo&) = delete;

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{SocketInfo}">What's been moved.</param>
				SocketInfo(SocketInfo&&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{SocketInfo}">What's been moved.</param>
				void operator=(SocketInfo&&);

				~SocketInfo();

				/// <summary>
				/// <para>Compare this with Sockaddr directly (from recvfrom).</para>
				/// </summary>
				/// <param name="{SOCKADDR_STORAGE}">Socket address information.</param>
				/// <returns>{bool} True if equal.</returns>
				bool operator==(const SOCKADDR_STORAGE&);

				/// <summary>
				/// <para>Compare this with Sockaddr directly (from recvfrom).</para>
				/// </summary>
				/// <param name="{SOCKADDR_STORAGE}">Socket address information.</param>
				/// <returns>{bool} True if different.</returns>
				bool operator!=(const SOCKADDR_STORAGE&);

				/// <summary>
				/// <para>Get URL connection information.</para>
				/// </summary>
				/// <returns>{const char*} IP Address.</returns>
				const char* get_url() const;

				/// <summary>
				/// <para>Get port number.</para>
				/// </summary>
				/// <returns>{unsigned short} Port number.</returns>
				const u_short& get_port() const;

				/// <summary>
				/// <para>Close current socket.</para>
				/// </summary>
				void close();

				/// <summary>
				/// <para>Get current address information.</para>
				/// </summary>
				/// <returns>{SOCKADDR_STORAGE} Reference to internal address.</returns>
				SOCKADDR_STORAGE& get_addr();

				/// <summary>
				/// <para>Get actual SOCKET.</para>
				/// </summary>
				/// <returns>{SOCKET} The SOCKET.</returns>
				const SOCKET& get_socket() const;
			};

			/// <summary>
			/// <para>Any TCP/UDP stuff needs WinSock to start. This does that once.</para>
			/// </summary>
			class SocketCore {
				struct _core {
					WSADATA wsaData = WSADATA();
					bool started = false;
					_core();
				};
				static _core __start;
			protected:
				/// <summary>
				/// <para>Easy setup for clients.</para>
				/// </summary>
				/// <param name="{SocketInfo}">Socket connection information storage.</param>
				/// <param name="{protocol}">The protocol to be used.</param>
				/// <param name="{family}">The family to be used.</param>
				/// <param name="{unsigned short}">The port to be used.</param>
				/// <param name="{std::string}">IP Address.</param>
				/// <returns>{result_type} The result, if had errors or not.</returns>
				socket::result_type setup_client(SocketInfo&, const socket::protocol&, const socket::family&, const u_short, const std::string&);

				/// <summary>
				/// <para>Easy setup for servers.</para>
				/// </summary>
				/// <param name="{std::vector}">Vector to store all possible server connection inputs.</param>
				/// <param name="{protocol}">The protocol to be used.</param>
				/// <param name="{family}">The family to be used.</param>
				/// <param name="{unsigned short}">The port to be used.</param>
				/// <returns>{result_type} The result, if had errors or not.</returns>
				socket::result_type setup_server(std::vector<SOCKET>&, const socket::protocol&, const socket::family&, const u_short);
			};

			/// <summary>
			/// <para>SocketClient is a classic client interface to communicate to another end. This is used directly or from a server host (generated by listen).</para>
			/// <para>All raw connection stuff is done by this. Send and Recv are manual (the bytes you send is exactly what is being sent, no verification or data check).</para>
			/// </summary>
			class SocketClient : SocketCore {
				struct _clientdata {
					std::shared_mutex secure;
					SocketInfo host;
					socket::protocol protocol{};
					socket::family family{};

					_clientdata() = default;
					_clientdata(const _clientdata&) = delete;
					_clientdata(_clientdata&&) = delete;
					void operator=(const _clientdata&) = delete;
					void operator=(_clientdata&&) = delete;
				};

				std::unique_ptr<_clientdata> self = std::make_unique<_clientdata>();

				friend class SocketServer;
			protected:
				const socket::protocol& get_protocol() const;
				const socket::family& get_family() const;
			public:
				SocketClient() = default;

				SocketClient(const SocketClient&) = delete;
				void operator=(const SocketClient&) = delete;

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{SocketClient}">Another SocketClient.</param>
				SocketClient(SocketClient&&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{SocketClient}">Another SocketClient.</param>
				void operator=(SocketClient&&);

				/// <summary>
				/// <para>Connect to a host (it's recommended to close() first if you have called this once).</para>
				/// </summary>
				/// <param name="{std::string}">IP Address.</param>
				/// <param name="{unsigned short}">Port number.</param>
				/// <param name="{protocol}">What protocol to use.</param>
				/// <param name="{family}">Family chosen.</param>
				/// <returns>{bool} True if no issues.</returns>
				bool connect(const std::string&, const u_short = socket::port_default, const socket::protocol& = socket::protocol::TCP, const socket::family& = socket::family::ANY);

				/// <summary>
				/// <para>Send data through the socket (no check if socket is valid).</para>
				/// </summary>
				/// <param name="{Buffer}">A Buffer with data to send.</param>
				/// <returns>{bool} True if internal send function worked as expected.</returns>
				bool send(const Buffer&);

				/// <summary>
				/// <para>Try to receive some data from socket (no check if socket is valid).</para>
				/// </summary>
				/// <param name="{Buffer}">Buffer to store data from the socket.</param>
				/// <param name="{size_t}">Expected buffer size (can result in smaller data afterwards).</param>
				/// <returns>{bool} True if internal recv function worked as expected.</returns>
				bool recv(Buffer&, const size_t = static_cast<size_t>(-1));

				/// <summary>
				/// <para>Like recv, but only peeks. This won't erase the data in socket's buffer.</para>
				/// </summary>
				/// <param name="{Buffer}">Buffer to store data from the socket.</param>
				/// <param name="{size_t}">Expected buffer size (can result in smaller data afterwards).</param>
				/// <returns>{int} Bytes read or error number (if negative; it correspond to WSAGetLastError code, but negative). 0 means disconnected.</returns>
				int peek(Buffer&, const size_t = static_cast<size_t>(-1)) const;

				/// <summary>
				/// <para>Check if socket is valid (literally the value of the socket).</para>
				/// </summary>
				/// <returns>{bool} True if there is a valid socket number id.</returns>
				bool valid() const;

				/// <summary>
				/// <para>Close connection (UDP doesn't update other side in real time).</para>
				/// </summary>
				void close();

				/// <summary>
				/// <para>Get current IP Address path.</para>
				/// </summary>
				/// <returns>{const char*} IP Address used on connection.</returns>
				const char* get_url() const;

				/// <summary>
				/// <para>Get current port in use.</para>
				/// </summary>
				/// <returns>{unsigned short} Port number.</returns>
				const u_short get_port() const;
			};

			/// <summary>
			/// <para>This handles multiple SOCKETs at once. You can listen to all of them and get new connections as they come. You have to start and listen.</para>
			/// </summary>
			class SocketServer : SocketCore {
				struct _serverdata {
					std::shared_mutex secure;
					std::vector<SOCKET> sockets;
					u_short port = 0;
					socket::protocol protocol{};
					socket::family family{};
					bool closing = false;

					_serverdata() = default;
					_serverdata(const _serverdata&) = delete;
					_serverdata(_serverdata&&) = delete;
					void operator=(const _serverdata&) = delete;
					void operator=(_serverdata&&) = delete;
				};

				std::unique_ptr<_serverdata> self = std::make_unique<_serverdata>();

				SOCKET common_select(std::vector<SOCKET>&);
			public:
				SocketServer() = default;

				SocketServer(const SocketServer&) = delete;
				void operator=(const SocketServer&) = delete;

				/// <summary>
				/// <para>Move constructor.</para>
				/// </summary>
				/// <param name="{SocketServer}">Another SocketServer.</param>
				SocketServer(SocketServer&&);

				/// <summary>
				/// <para>Move operator.</para>
				/// </summary>
				/// <param name="{SocketServer}">Another SocketServer.</param>
				void operator=(SocketServer&&);

				/// <summary>
				/// <para>Start the host (if called once, close() before trying new configs).</para>
				/// </summary>
				/// <param name="{unsigned short}">Port to listen.</param>
				/// <param name="{protocol}">Protocol used.</param>
				/// <param name="{family}">Family used.</param>
				/// <returns>{bool} True if ready to listen, else error.</returns>
				bool start(const u_short = socket::port_default, const socket::protocol& = socket::protocol::TCP, const socket::family& = socket::family::ANY);

				/// <summary>
				/// <para>Wait and get new connection, if possible. Check valid() after this to be sure.</para>
				/// </summary>
				/// <returns>{SocketClient} A new SocketClient tied to new connection.</returns>
				SocketClient listen();

				/// <summary>
				/// <para>Close all server listening sockets.</para>
				/// </summary>
				void close();

				/// <summary>
				/// <para>Amount of sockets ready to listen. 0 means none are set.</para>
				/// </summary>
				/// <returns>{size_t} Amount of sockets ready to listen.</returns>
				size_t has_servers() const;
			};
		}
	}
}
