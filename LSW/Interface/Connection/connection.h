#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
// C++
#include <vector>
#include <thread>
#include <string>
#include <mutex>
#include <functional>
#include <memory>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "../../Handling/Abort/abort.h"
#include "../../Tools/SuperMutex/supermutex.h"
#include "../../Tools/SuperThread/superthread.h"
#include "../Logger/logger.h"
#include "../Events/events.h"
#include "../EventTimer/eventtimer.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace connection {
				constexpr int default_port = 42069;
				constexpr unsigned package_size = 1 << 8;
				constexpr size_t max_buffering_packages_default = 30;
				//const std::chrono::milliseconds min_delay_no_tasks = std::chrono::milliseconds(5);

				const double pinging_time = 2.0; // seconds
				const double syncing_time = 0.5; // seconds

				enum class _internal_tasks {NOT_IT,
					PING,PONG, // both sides will task this, no reason to send back result
					RECV_OVERLOAD_WAIT, // [0] recv buffer is overloaded, other's [1] recv receive and wait for RECV_OVERLOAD_CONTINUE
					RECV_OVERLOAD_CONTINUE, // if [0] once overloaded, send this
					SYNC,SYNC_BACK // Send sends this and wait for bool to toggle meaning RECV received.
				};
			}

			struct _pack_1 {
				unsigned sys = 0; // if sys, _internal_task it is!
				unsigned long long timestamp; // millisec, automatic set
				bool has_pack_2 = true;
				_pack_1();
			};
			struct _pack_2 {
				char data[connection::package_size] = { 0 };
				unsigned data_len = 0;
				size_t sum_with_n_more = 0;
				//unsigned sys = 0; // if sys, _internal_task it is!
			};
			
			struct _unprocessed_pack {
				std::string data;
				unsigned sys = 0;

				_unprocessed_pack(const std::string&);
				_unprocessed_pack(std::string&&, unsigned);
				_unprocessed_pack(_unprocessed_pack&&);
				_unprocessed_pack(const _unprocessed_pack&);

				void operator=(_unprocessed_pack&&);
				void operator=(const _unprocessed_pack&);
			};

			/// <summary>
			/// <para>This is the WinSock base.</para>
			/// <para>You won't use this directly.</para>
			/// </summary>
			class ConnectionCore {
				WSADATA wsaData = WSADATA();
				addrinfo* result = nullptr;
				bool failure = false, init = false;
			public:
				// ip (can be null for server), port, ipv6? -1 for client
				bool initialize(const char*, const int = connection::default_port, const int = 0);
				bool as_client(SOCKET&);
				bool as_host(SOCKET&);
			};

			/// <summary>
			/// <para>A connection itself (one to one).</para>
			/// </summary>
			class Connection {
				ConnectionCore core;
				Logger logg;

				mutable Tools::SuperMutex packs_received_m;
				std::vector<std::string> packs_received;
				Tools::SuperMutex packs_sending_m;
				std::vector<_unprocessed_pack> packs_sending;

				size_t buf_max = connection::max_buffering_packages_default;

				SOCKET connected = INVALID_SOCKET;
				bool keep_connection = false;

				Tools::SuperThread<> send_thread{ Tools::superthread::performance_mode::PERFORMANCE };
				Tools::SuperThread<> recv_thread{ Tools::superthread::performance_mode::PERFORMANCE };

				// debugging
				size_t packages_sent = 0, packages_sent_bytes = 0;
				size_t packages_recv = 0, packages_recv_bytes = 0;
				size_t last_ping = 100;

				bool recv_this_buffer_overload = true; // self overload
				bool friend_there_recv_buffer_overload = false; // received other side is overloaded
				bool in_sync_signal_received = true;

				Tools::Waiter package_come_wait;

				std::function<void(const uintptr_t, const std::string&)> alt_receive_autodiscard; // your handle, no saving. It calls this instead.
				std::function<std::string(void)>						 alt_generate_auto;		 // it will read only from this if set.

				// returns false if disconnected (aka -1) | size must be > 0
				bool ensure_send(char*, const int);
				// returns false if disconnected (aka -1) | size must be > 0
				bool ensure_recv(char*, const int);

				void handle_send(Tools::boolThreadF);
				void handle_recv(Tools::boolThreadF);

				// starts handle's
				void init();
			public:
				Connection(const Connection&) = delete;
				Connection(Connection&&) = delete;

				/// <summary>
				/// <para>FOR DIRECT HOST SET ONLY. This is not meant to be used if SOCKET is not valid.</para>
				/// <para>Use at your own risk.</para>
				/// </summary>
				/// <param name="{SOCKET}">The RAW SOCKET (winsock) connection.</param>
				Connection(SOCKET = INVALID_SOCKET);
				~Connection();

				/// <summary>
				/// <para>Connects to a URL/IP.</para>
				/// </summary>
				/// <param name="{char*}">The URL/IP.</param>
				/// <param name="{int}">Port number.</param>
				/// <returns>{bool} True if connected successfully.</returns>
				bool connect(const char* = "127.0.0.1", const int = connection::default_port);

				/// <summary>
				/// <para>Close communication and aux threads.</para>
				/// </summary>
				void close();

				/// <summary>
				/// <para>Is this connected to something?</para>
				/// </summary>
				/// <returns>{bool} True if connected.</returns>
				bool is_connected() const;

				/// <summary>
				/// <para>Is there something for me?</para>
				/// </summary>
				/// <returns>{bool} True if has something to get.</returns>
				bool has_package() const;

				/// <summary>
				/// <para>Whether it's in sync, aka up to date.</para>
				/// </summary>
				/// <returns>{bool} True if right now it is in sync. False just means it is syncing.</returns>
				bool in_sync() const;

				/// <summary>
				/// <para>If the other side can't receive, send becomes overloaded</para>
				/// <para>Trying to send more packages will hold thread.</para>
				/// </summary>
				/// <returns>{bool} Is send "locked"?</returns>
				bool is_receiving_buffer_full() const;

				/// <summary>
				/// <para>Is receive buffer full?</para>
				/// <para>This will ask server to stop sending, but it can't refuse new data.</para>
				/// <para>This might be true after not being 100% full, but it will be false after some get_next() or something.</para>
				/// </summary>
				/// <returns>{bool} Is recv "full" or at least once was and still almost full?</returns>
				bool is_other_receiving_buffer_full() const;

				/// <summary>
				/// <para>Wait some time for a package.</para>
				/// </summary>
				/// <param name="{std::chrono::milliseconds}">Timeout. 0 = Infinite.</param>
				/// <returns>{bool} True if has something to get.</returns>
				bool wait_for_package(const std::chrono::milliseconds = std::chrono::milliseconds(0));

				/// <summary>
				/// <para>Get next package.</para>
				/// </summary>
				/// <returns>{std::string} The package received.</returns>
				std::string get_next();

				/// <summary>
				/// <para>Send a package of bytes.</para>
				/// </summary>
				/// <param name="{std::string}">The bytes you want to send.</param>
				/// <param name="{bool}">Force even if buffer is full.</param>
				/// <param name="{bool}">If full, lock? (P.S.: IT MIGHT STILL LOCK if send thread locked the mutex for some milliseconds)</param>
				/// <returns>{bool} True if added to send queue. False if FULL and no lock (third arg).</returns>
				bool send_package(std::string, bool = false, bool = true);

				/// <summary>
				/// <para>Total small packages sent (not package itself, the small ones).</para>
				/// </summary>
				/// <returns>{size_t} Small packages sent.</returns>
				size_t get_packages_sent() const;

				/// <summary>
				/// <para>Total all data sent in bytes.</para>
				/// </summary>
				/// <returns>{size_t} Sent, in bytes.</returns>
				size_t get_packages_sent_bytes() const;

				/// <summary>
				/// <para>Total small packages received (not package itself, the small ones).</para>
				/// </summary>
				/// <returns>{size_t} Small packages received.</returns>
				size_t get_packages_recv() const;

				/// <summary>
				/// <para>Total all data received in bytes.</para>
				/// </summary>
				/// <returns>{size_t} Received, in bytes.</returns>
				size_t get_packages_recv_bytes() const;

				/// <summary>
				/// <para>Get last ping information.</para>
				/// </summary>
				/// <returns>{size_t} Ping in milliseconds.</returns>
				size_t get_ping();

				/// <summary>
				/// <para>Set a max buffering of packages in memory (both send and recv).</para>
				/// <para>By package: each complete package of yours.</para>
				/// <para>Example: 1 mean one FULL package of yours (does not translate to direct memory size).</para>
				/// <para>0 means infinite (no limit).</para>
				/// </summary>
				/// <param name="{size_t}">Amount of packages maximum in buffer.</param>
				void set_max_buffering(const size_t);

				/// <summary>
				/// <para>Set a function to handle RECV RAW data.</para>
				/// <para>WARN: All data from RECV will be handled exclusively by your custom function if you set one!</para>
				/// <para>PS: DO NOT SET A FUNCTION THAT CAN POTENTIALLY LOCK!</para>
				/// </summary>
				/// <param name="{std::function}">The function to handle a complete package reading.</param>
				void overwrite_reads_to(std::function<void(const uintptr_t, const std::string&)>);

				/// <summary>
				/// <para>Set a function to handle SEND RAW data.</para>
				/// <para>WARN: All data has to be SENT by this. This has to be the one generating strings. Send_package won't work.</para>
				/// <para>PS: DO NOT SET A FUNCTION THAT CAN POTENTIALLY LOCK!</para>
				/// </summary>
				/// <param name="{std::function}">The function to generate strings somehow. This should return empty string if no package yet, so it won't lock stuff.</param>
				void overwrite_sends_to(std::function<std::string(void)>);

				/// <summary>
				/// <para>Resets to default way of handling packages.</para>
				/// </summary>
				void reset_overwrite_reads();

				/// <summary>
				/// <para>Resets to default way of handling packages.</para>
				/// </summary>
				void reset_overwrite_sends();

				/// <summary>
				/// <para>Set performance mode in connection threads.</para>
				/// <para>This may affect maximum bandwidth.</para>
				/// </summary>
				/// <param name="{performance_mode}">Performance mode.</param>
				void set_mode(const Tools::superthread::performance_mode);
			};

			/// <summary>
			/// <para>This handles host. It will connect and create Connection smart pointers that you can handle later.</para>
			/// </summary>
			class Hosting {
				ConnectionCore core;
				Logger logg;

				SOCKET Listening = INVALID_SOCKET;
				bool keep_connection = false;

				size_t max_connections_allowed = 1;

				Tools::SuperThread<> handle_thread { Tools::superthread::performance_mode::VERY_LOW_POWER };
				Tools::SuperThread<> handle_disc_thread{ Tools::superthread::performance_mode::EXTREMELY_LOW_POWER };

				Tools::Waiter connection_event;
				std::vector<std::shared_ptr<Connection>> connections;
				mutable Tools::SuperMutex connections_m;

				std::function<void(std::shared_ptr<Connection>)> new_connection_f; // handle new connection automatically
				std::function<void(const uintptr_t)> disconnected_f; // handle disconnected event automatically

				void handle_disconnects(Tools::boolThreadF);
				void handle_queue(Tools::boolThreadF);

				void init();
			public:
				Hosting(const Hosting&) = delete;
				Hosting(Hosting&&) = delete;

				/// <summary>
				/// <para>Initialize a Host.</para>
				/// </summary>
				/// <param name="{int}">Port.</param>
				/// <param name="{bool}">IPV6?</param>
				Hosting(const int, const bool = false);

				/// <summary>
				/// <para>Initialize a Host using default port.</para>
				/// </summary>
				/// <param name="{bool}">IPV6?</param>
				Hosting(const bool = false);
				~Hosting();

				/// <summary>
				/// <para>How many clients are connected?</para>
				/// </summary>
				/// <returns>{size_t} The amount of connected clients.</returns>
				size_t size() const;

				/// <summary>
				/// <para>Close all connections.</para>
				/// </summary>
				void close();

				/// <summary>
				/// <para>Is host online?</para>
				/// </summary>
				/// <returns>{bool} True if listening and ready.</returns>
				bool is_connected() const;

				/// <summary>
				/// <para>Sets a limit of connections (won't disconnect if amount connected right now is bigger than this value).</para>
				/// <para>New connections will connect and disconnect instantly.</para>
				/// </summary>
				/// <param name="{size_t}">Maximum amount of connections allowed.</param>
				void set_connections_limit(const size_t);

				/// <summary>
				/// <para>Gets a specific connected client connection [0..size()-1].</para>
				/// </summary>
				/// <param name="{size_t}">The connection position in vector.</param>
				/// <returns>{std::shared_ptr} The connection smart pointer.</returns>
				std::shared_ptr<Connection> get_connection(const size_t);

				/// <summary>
				/// <para>Gets latest connected client connection.</para>
				/// </summary>
				/// <returns>{std::shared_ptr} The connection smart pointer.</returns>
				std::shared_ptr<Connection> get_latest_connection();

				/// <summary>
				/// <para>Set a function to handle new connections directly.</para>
				/// <para>PS: This received the shared pointer exactly before adding into the vector!</para>
				/// <para>PS: DO NOT SET A FUNCTION THAT CAN POTENTIALLY LOCK!</para>
				/// </summary>
				/// <param name="{std::function}">The function to handle new connections.</param>
				void on_new_connection(std::function<void(std::shared_ptr<Connection>)>);

				/// <summary>
				/// <para>Erase current new connection custom function.</para>
				/// </summary>
				void reset_on_new_connection();

				/// <summary>
				/// <para>Set a function to handle disconnections directly.</para>
				/// <para>PS: This received the uintptr_t exactly before deleting from vector!</para>
				/// <para>PS: DO NOT SET A FUNCTION THAT CAN POTENTIALLY LOCK!</para>
				/// </summary>
				/// <param name="{std::function}">The function to handle disconnections.</param>
				void on_connection_close(std::function<void(const uintptr_t)>);

				/// <summary>
				/// <para>Erase current connection close custom function.</para>
				/// </summary>
				void reset_on_connection_close();
			};

			/// <summary>
			/// <para>Transforms anything to std::string directly for you (using memcpy_s).</para>
			/// </summary>
			/// <param name="{void*}">Some data.</param>
			/// <param name="{size_t}">Data's size.</param>
			/// <returns>{std::string} This data 'converted' to a string.</returns>
			std::string transform_any_to_package(void*, const size_t);

			/// <summary>
			/// <para>Transforms anything to std::string directly for you (using memcpy_s).</para>
			/// </summary>
			/// <param name="{void*}">Some data.</param>
			/// <param name="{size_t}">Data's size.</param>
			/// <param name="{errno_t}">If result is empty, this holds error (if memcpy_s failed).</param>
			/// <returns>{std::string} This data 'converted' to a string.</returns>
			std::string transform_any_to_package(void*, const size_t, errno_t&);

			/// <summary>
			/// <para>Transforms back a package 'converted' via transform_any_to_package.</para>
			/// </summary>
			/// <param name="{void*}">Data to be written.</param>
			/// <param name="{size_t}">Data size.</param>
			/// <param name="{std::string}">The string data to read from.</param>
			/// <returns>{bool} True on success.</returns>
			bool transform_any_package_back(void*, const size_t, const std::string&);
		}
	}
}