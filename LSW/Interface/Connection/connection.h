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
#include "../../Tools/Common/common.h"
#include "../Logger/logger.h"
#include "../Events/events.h"
#include "../EventTimer/eventtimer.h"
#include "../SmartFile/smartfile.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace connection {
				constexpr int default_port = 42069;
				constexpr unsigned package_size = 1 << 10; // 1024 bytes
				constexpr unsigned default_limit = 600;

				const double pinging_time = 5.0; // seconds

				enum class _connection_status {
					DISCONNECTED = -1, // error, disconnected
					EMPTY = 0,  // nothing has been read/sent
					GOOD = 1   // has read a full package
				};

				enum class connection_type {
					CLIENT = -1,
					HOST_IPV4,
					HOST_IPV6
				};

				constexpr int automatic_self_point = 3000; // DON'T SET THIS TO less or eq 0.
				const auto default_performance_connection = Tools::superthread::performance_mode::PERFORMANCE;
			}

			/// <summary>
			/// <para>__package is a small package used by Connection to send information back and forth</para>
			/// <para>You probably don't need to touch this.</para>
			/// </summary>
			struct __package {
				enum class package_type { DATA = 1, SYNC, REQUEST, PING, PONG };

				// data and info
				struct _data {
					char buffer[connection::package_size]; // raw buffer
					unsigned data_len; // buffer used len
					unsigned long long remaining; // how many are still coming
					bool full; // consider this full?
				};
				// syncronization and delay control
				struct _sync {
					unsigned long long sent_count; // this is its counter right now
					unsigned long long received_count; // who sends thinks you're at this right now
					unsigned long long buffer_sending_size; // literally buffer's size
					unsigned long long buffer_receive_size; // literally buffer's size
					long long add_availability; // delta how many are available "again" (aka get_next() cleaned 10, so +10) // positive
					bool no_limit; // no limit, as it says, so add_availability is "useless"
				};
				// request
				struct _rqst {
					package_type request; // request host to do a task. Currently available: SYNC, PING
				};
				// ping
				struct _ping {
					unsigned long long self_time_ms; // sender's time since epoch, ms
				};
				// pong
				struct _pong : public _ping {}; // copy

				union {
					_data data;
					_sync sync;
					_rqst rqst;
					_ping ping;
					_pong pong;
				} pack;

				package_type type{};

				/// <summary>
				/// <para>Prepare package for a type. This will initialize some stuff beforehand.</para>
				/// </summary>
				/// <param name="{package_type}">What package type you're planning?</param>
				__package(const package_type);

				/// <summary>
				/// <para>No preparation, just ZeroMemory.</para>
				/// </summary>
				__package();
				
				/// <summary>
				/// <para>Prepare package for a type. This will initialize some stuff beforehand.</para>
				/// </summary>
				/// <param name="{package_type}">What package type you're planning?</param>
				void prepare_to(const package_type);
			};

			/// <summary>
			/// <para>Packages sometimes are bigger than the small packages used for communication</para>
			/// <para>This is a big "combined data" used to combine or slice a bigger package from or to small ones.</para>
			/// </summary>
			struct combined_data {
				std::string buffer;
				bool is_full = true;
				size_t __represents_n_packages = 0;

				combined_data& operator+(const combined_data&);
				combined_data& operator+=(const combined_data&);
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
				/// <summary>
				/// <para>Initialize basic WinSock stuff.</para>
				/// </summary>
				/// <param name="{std::string}">URL to connect to. Set to empty string if you're hosting.</param>
				/// <param name="{int}">What port?</param>
				/// <param name="{connection_type}">What type of communication is this?</param>
				bool initialize(const std::string&, const int = connection::default_port, const connection::connection_type = connection::connection_type::HOST_IPV4);

				/// <summary>
				/// <para>Complete initialization as client.</para>
				/// <para>PS: Call initialize FIRST.</para>
				/// </summary>
				/// <param name="{SOCKET}">SOCKET to connect.</param>
				bool as_client(SOCKET&);

				/// <summary>
				/// <para>Complete initialization as host. This will listen for connections.</para>
				/// <para>PS: Call initialize FIRST.</para>
				/// </summary>
				/// <param name="{SOCKET}">SOCKET to use as listener.</param>
				bool as_host(SOCKET&);
			};

			/// <summary>
			/// <para>NetworkMonitor is used for Connection's performance monitoring.</para>
			/// <para>You won't use this directly.</para>
			/// </summary>
			class NetworkMonitor {
			public:
				struct nm_ping{
					unsigned current = 0;
					double adaptative_avg = 0;
					double adaptativeness = 10; // (10 * current + new) / (10 + 1)
				};
				struct nm_transf {
					std::atomic<unsigned long long> _bytes_coming;
					unsigned long long bytes = 0;
					unsigned long long peak_bytes_per_second = 0;
					unsigned long long last_update_ms = 0;
					unsigned long long first_update_ms = 0;
					unsigned long long current_bytes_per_sec = 0; // copy of _bytes_coming secured
				};
			private:
				nm_ping ping{};
				nm_transf sending{};
				nm_transf recving{};

				Interface::EventTimer timer_second;
				Interface::EventHandler per_sec_calc{ Tools::superthread::performance_mode::EXTREMELY_LOW_POWER };

				void __avg_task(nm_transf&);
				// responsible for send/recv data
				void _average_thr(const Interface::RawEvent&);

				// generic any nm_transf
				void any_add(nm_transf&, const unsigned long long);
				unsigned long long any_get_total(const nm_transf&) const;
				unsigned long long any_get_peak(const nm_transf&) const;
				unsigned long long any_get_current_bytes_per_second(const nm_transf&) const;
				unsigned long long any_get_average_total(const nm_transf&) const;
			public:
				NetworkMonitor();
				~NetworkMonitor();

				/// <summary>
				/// <para>Clears all data. Internal thread will continue to run.</para>
				/// </summary>
				void clear();

				/// <summary>
				/// <para>Adds a new ping information.</para>
				/// </summary>
				/// <param name="{unsigned}">Ping information</param>
				void ping_new(const unsigned);

				/// <summary>
				/// <para>Set how slow the adaptative average algorithm should hit.</para>
				/// <para>Low values are more responsive to changes, higher values slow down changes.</para>
				/// </summary>
				/// <param name="{double}">Adaptativeness coefficient. Has to be more than 0.</param>
				void ping_set_adaptativeness(const double);

				/// <summary>
				/// <para>Get latest ping information.</para>
				/// </summary>
				/// <returns>{unsigned} Last ping.</returns>
				unsigned ping_now() const;

				/// <summary>
				/// <para>Get the adaptative average ping.</para>
				/// </summary>
				/// <returns>{double} Ping, average.</returns>
				double ping_average_now() const;

				/// <summary>
				/// <para>Adds bytes as SEND.</para>
				/// </summary>
				/// <param name="{unsigned long long}">How many have you sent this time?</param>
				void send_add(const unsigned long long);

				/// <summary>
				/// <para>Get the sum of all send bytes.</para>
				/// </summary>
				/// <returns>{unsigned long long} Amount, in bytes.</returns>
				unsigned long long send_get_total() const;

				/// <summary>
				/// <para>Get the highest send in one second so far.</para>
				/// </summary>
				/// <returns>{unsigned long long} Peak send amount in one second.</returns>
				unsigned long long send_get_peak() const;

				/// <summary>
				/// <para>Get byte throughput right now.</para>
				/// </summary>
				/// <returns>{unsigned long long} Bytes per second last second.</returns>
				unsigned long long send_get_current_bytes_per_second() const;

				/// <summary>
				/// <para>Get average bytes per second since the beginning.</para>
				/// </summary>
				/// <returns>{unsigned long long} Bytes per second.</returns>
				unsigned long long send_get_average_total() const;

				/// <summary>
				/// <para>Adds bytes as RECV.</para>
				/// </summary>
				/// <param name="{unsigned long long}">How many have you recv this time?</param>
				void recv_add(const unsigned long long);

				/// <summary>
				/// <para>Get the sum of all recv bytes.</para>
				/// </summary>
				/// <returns>{unsigned long long} Amount, in bytes.</returns>
				unsigned long long recv_get_total() const;

				/// <summary>
				/// <para>Get the highest recv in one second so far.</para>
				/// </summary>
				/// <returns>{unsigned long long} Peak recv amount in one second.</returns>
				unsigned long long recv_get_peak() const;

				/// <summary>
				/// <para>Get byte throughput right now.</para>
				/// </summary>
				/// <returns>{unsigned long long} Bytes per second last second.</returns>
				unsigned long long recv_get_current_bytes_per_second() const;

				/// <summary>
				/// <para>Get average bytes per second since the beginning.</para>
				/// </summary>
				/// <returns>{unsigned long long} Bytes per second.</returns>
				unsigned long long recv_get_average_total() const;
			};

			/// <summary>
			/// <para>A connection itself (one to one).</para>
			/// </summary>
			class Connection {
				// - - - - ESSENTIAL - - - - //
				ConnectionCore core;
				Logger logg;

				SOCKET connected = INVALID_SOCKET;
				bool keep_connection = false;

				// - - - - WORKING - - - - //

				// shared
				__package::_sync myself{ 0,0,0,0,0,false }; // MEE
				__package::_sync yourself{ 0,0,0,0,0,false }; // COPY/READ ONLY..

				Tools::SuperMutex buffer_available_send_mtx;
				long long buffer_available_send = 0; // OTHER SIDE SAY THIS ONE, DO NOT TOUCH THIS

				Tools::SuperMutex buffer_available_calculated_mtx;
				long long buffer_available_calculated = 0; // available to recv, other side can send, so, positive.

				long long my_buffer_limit = 0;

				unsigned long long last_recv_remaining = 0;

				std::atomic<bool> should_ping = true;
				std::atomic<bool> should_sync = false;
				std::atomic<bool> should_request_sync = false;
				std::atomic<size_t> freed_packages = 0;

				std::vector<__package> between;
				Tools::SuperMutex between_mtx;

				// work
				Tools::SuperThread<> connection_handle_send{ connection::default_performance_connection };
				Tools::SuperThread<> connection_handle_recv{ connection::default_performance_connection };

				// info
				NetworkMonitor network_analysis;

				// triggers
				Tools::Waiter received_package;
				Tools::Waiter sent_once_package;

				// buffers
				Tools::SuperMutex buffer_sending_mtx;
				std::vector<combined_data> buffer_sending;
				Tools::SuperMutex buffer_receive_mtx;
				std::vector<combined_data> buffer_receive;

				// overwrites
				Tools::SuperMutex send_overwrite_mtx;
				std::function<std::string(void)> send_overwrite;
				Tools::SuperMutex recv_overwrite_mtx;
				std::function<void(const uintptr_t, const std::string&)> recv_overwrite;


				// - - - - FUNCTIONS - - - - //

				// current limit THIS SENDING to other. return new limit
				long long add_current_limit(const int);
				void pop_current_limit();

				// current THIS RECEIVING DATA limit and stuff.
				void calculate_add_available(const int);
				void calculate_modify_buffer(const long long);
				long long calculate_cut_value();

				// update package "myself" so SYNC can send it
				void update_myself_package();

				// recv() holds if nothing to read?
				void set_recv_hold(const bool);

				// guaranteed all or nothing send
				connection::_connection_status ensure_send(char*, const int);
				// guaranteed all or nothing recv
				connection::_connection_status ensure_recv(char*, const int);

				void handle_connection_send(Tools::boolThreadF, Tools::SuperThread<>&);
				void handle_connection_recv(Tools::boolThreadF, Tools::SuperThread<>&);

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
				/// <param name="{std::string}">The URL/IP.</param>
				/// <param name="{int}">Port number.</param>
				/// <returns>{bool} True if connected successfully.</returns>
				bool connect(const std::string& = "127.0.0.1", const int = connection::default_port);

				/// <summary>
				/// <para>Close communication and aux threads.</para>
				/// <para>WARN: On close, any remaining data to send or data to read is also CLEARED!</para>
				/// </summary>
				void close();

				/// <summary>
				/// <para>Is this connected to something?</para>
				/// </summary>
				/// <returns>{bool} True if connected.</returns>
				bool is_connected() const;

				/// <summary>
				/// <para>Get information about ping, bytes sent, recv, average and so on.</para>
				/// </summary>
				/// <returns>{NetworkMonitor} Const reference to actual network info.</returns>
				const NetworkMonitor& get_network_info() const;

				/// <summary>
				/// <para>Is there something for me?</para>
				/// </summary>
				/// <returns>{bool} True if has something to get.</returns>
				bool has_package() const;

				/// <summary>
				/// <para>Limit recv buffer to this amount of small packages at once.</para>
				/// <para>WARN: if host try to send huge file, this may BREAK the file data.</para>
				/// <para>Value == 0 means infinite.</para>
				/// </summary>
				/// <param name="{long long}">Maximum buffer on sending part. 0 means infinite.</param>
				void set_max_buffering(const long long = connection::default_limit);

				/// <summary>
				/// <para>How many does it believe it can send before overload?</para>
				/// </summary>
				/// <returns>{long long} How many? Less than 0 means infinite.</returns>
				long long in_memory_can_send() const;

				/// <summary>
				/// <para>How many does it believe it can receive before overload?</para>
				/// </summary>
				/// <returns>{long long} How many? Less than 0 means infinite.</returns>
				long long in_memory_can_read() const;

				/// <summary>
				/// <para>Estimated packages that are on their way to the other side.</para>
				/// <para>The real number can be equal or greater than this.</para>
				/// <para>PS: This value sometimes might never get to zero!</para>
				/// </summary>
				/// <returns>{unsigned} Estimated packages that are in traffic to get there.</returns>
				unsigned small_packages_on_the_way() const;

				/// <summary>
				/// <para>Estimated packages that are still coming.</para>
				/// <para>The real number can be equal or greater than this.</para>
				/// <para>PS: This value sometimes might never get to zero!</para>
				/// </summary>
				/// <returns>{unsigned} Estimated packages that are still coming.</returns>
				unsigned small_packages_on_my_way() const;

				/// <summary>
				/// <para>How many packages have come versus how many were reported 'sent' in last sync.</para>
				/// <para>This should be as low as possible. Big numbers mean bad syncronization.</para>
				/// </summary>
				/// <returns>{unsigned} Difference between reported sent by other side versus real received.</returns>
				unsigned small_packages_received_since_last_sync() const;

				/// <summary>
				/// <para>Wait some time for a package.</para>
				/// </summary>
				/// <param name="{std::chrono::milliseconds}">Timeout. 0 = Infinite.</param>
				/// <returns>{bool} True if has something to get.</returns>
				bool wait_for_package(const std::chrono::milliseconds = std::chrono::milliseconds(0));

				/// <summary>
				/// <para>Get next package.</para>
				/// <para>Can be an incomplete package (not full) if buffer is full or sender has done it in steps.</para>
				/// </summary>
				/// <param name="{bool}">Force wait for full package (a full tagged pack). Can still return non-full, but only if there's no package to get.</param>
				/// <returns>{combined_data} The package received (or part of, depending on limit configuration).</returns>
				combined_data get_next(const bool = true);

				/// <summary>
				/// <para>Send a package of bytes.</para>
				/// <para>PS: If you're planning on sending huge data (big files), you probably want to "flush".</para>
				/// </summary>
				/// <param name="{std::string}">The bytes you want to send.</param>
				/// <param name="{bool}">Wait for the package to get sent? (if there are some, wait flush?)</param>
				void send_package(const std::string&, const bool = false);

				/// <summary>
				/// <para>Send a package of bytes (part of or complete).</para>
				/// <para>PS: If you want to send multiple files, set .is_full to false.</para>
				/// <para>All packages with .is_full will be combined in the end when a .is_full true is sent.</para>
				/// </summary>
				/// <param name="{combined_data}">The bytes you want to send.</param>
				/// <param name="{bool}">Wait for the package to get sent? (if there are some, wait flush?)</param>
				void send_package(const combined_data&, const bool = false);

				/// <summary>
				/// <para>Total small packages sent (not package itself, the small ones).</para>
				/// </summary>
				/// <returns>{unsigned long long} Small packages sent.</returns>
				unsigned long long get_packages_sent() const;

				/// <summary>
				/// <para>Total all data sent in bytes.</para>
				/// </summary>
				/// <returns>{unsigned long long} Sent, in bytes.</returns>
				unsigned long long get_packages_sent_bytes() const;

				/// <summary>
				/// <para>Total small packages received (not package itself, the small ones).</para>
				/// </summary>
				/// <returns>{unsigned long long} Small packages received.</returns>
				unsigned long long get_packages_recv() const;

				/// <summary>
				/// <para>Total all data received in bytes.</para>
				/// </summary>
				/// <returns>{unsigned long long} Received, in bytes.</returns>
				unsigned long long get_packages_recv_bytes() const;

				/// <summary>
				/// <para>Get last ping information.</para>
				/// </summary>
				/// <returns>{size_t} Ping in milliseconds.</returns>
				size_t get_ping();

				/// <summary>
				/// <para>Tells if send thread is waiting for the other side to free up some space.</para>
				/// </summary>
				/// <returns>{bool} True if limit has reached.</returns>
				bool is_overloaded() const;

				/// <summary>
				/// <para>Set a function to handle RECV RAW data.</para>
				/// <para>WARN: All data from RECV will be handled exclusively by your custom function if you set one!</para>
				/// <para>OBS: This won't work if overwrite_reads_small_chunk_to is set.</para>
				/// <para>PS: DO NOT SET A FUNCTION THAT CAN POTENTIALLY LOCK!</para>
				/// </summary>
				/// <param name="{std::function}">The function to handle a complete package reading.</param>
				void overwrite_reads_to(std::function<void(const uintptr_t, const std::string&)>);

				/// <summary>
				/// <para>Set a function to handle SEND RAW data.</para>
				/// <para>WARN: All data has to be SENT by this. This has to be the one generating strings. Send_package won't work.</para>
				/// <para>OBS: This won't work if overwrite_reads_small_chunk_to is set.</para>
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
				/// <para>Force constant performance mode in connection threads.</para>
				/// <para>This may affect maximum bandwidth.</para>
				/// </summary>
				/// <param name="{performance_mode}">Performance mode.</param>
				void set_mode(const Tools::superthread::performance_mode);

				/// <summary>
				/// <para>Reset to default performance mode</para>
				/// </summary>
				void reset_mode_default();
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

				Tools::SuperThread<> handle_thread{ Tools::superthread::performance_mode::VERY_LOW_POWER };
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

			/// <summary>
			/// <para>Reads from file and sends automatically.</para>
			/// </summary>
			/// <param name="{SmartFile}">A opened ready to send SmartFile.</param>
			/// <param name="{Connection}">Connection that will send this file.</param>
			/// <param name="{size_t}">Read/send chunk size. 0 is automatic. Less than connection::package_size defaults to connection::package_size.</param>
			bool transform_and_send_file_auto(SmartFile&, Connection&, size_t = 0);
		}
	}
}