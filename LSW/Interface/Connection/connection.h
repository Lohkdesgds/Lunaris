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
				constexpr unsigned package_size = 1 << 16; // 64 kB max package size (probably up to 400 Mbit internet, ~50 MByte/s on i9 9900k 4.8 GHz) (can go up to probably 512 kB)
				constexpr size_t limit_packages_stuck_send = 2; // MemoryFiles
				constexpr unsigned long long trigger_sync_send_thread = 10; // packages, bigger can cause higher ping. Consider LONG LONG limits!
				constexpr unsigned long long trigger_sync_multiplier_slowdown = 2; // sleep_for (this * (pack count - trigger_sync_send_thread) );
				constexpr unsigned long long trigger_max_time_allowed = 500;

				const double pinging_time = 10.0; // seconds
				//const double timeout_send = 3.0; // seconds

				enum class _connection_status {
					DISCONNECTED = -1, // error, disconnected
					FAILED = 0,  // nothing has been read/sent
					GOOD = 1   // has read a full package
				};

				enum class connection_type {
					CLIENT = -1,
					HOST_IPV4,
					HOST_IPV6
				};

				//constexpr int automatic_self_point = 3000; // DON'T SET THIS TO less or eq 0.
				const auto default_performance_connection = Tools::superthread::performance_mode::PERFORMANCE;
			}

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
				struct nm_ping {
					unsigned current = 0;
					unsigned peak = 0;
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
				/// <para>Get highest ping information.</para>
				/// </summary>
				/// <returns>{unsigned} Highest ping.</returns>
				unsigned ping_peak() const;

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

			struct Package {
				SmartFile big_file;
				std::string small_data;
				int type = 0;

				Package() = default;
				Package(const Package&) = delete;
				Package(Package&&);

				bool has_data() const;
				void operator=(const Package&) = delete;
				void operator=(Package&&);
			};

			/// <summary>
			/// <para>A connection itself (one to one).</para>
			/// </summary>
			class Connection {
				enum class __package_type { PING = 1, PONG, SIGNAL, PRIORITY_PACKAGE_SMALL, PACKAGE_SMALL, PACKAGE_FILE, _FAILED };

				struct __package {

					struct {
						int type = 0;
						int len = 0;
						bool has_more = false;
					} info;
					struct {
						char buffer[connection::package_size]{};
					} data;

					__package() = default;

					/// <summary>
					/// <para>Directly generate a package.</para>
					/// </summary>
					/// <param name="{std::string}">Buffer, size equal or less than connection::package_size, or it will cut the data.</param>
					/// <param name="{__package_type}">The package type.</param>
					/// <param name="{bool}">Is this complete?</param>
					__package(const std::string&, const __package_type, const bool);
				};

				const int __package_size_buffer = sizeof(__package);

				// - - - - ESSENTIAL - - - - //
				ConnectionCore core;
				NetworkMonitor network_analysis;
				Logger logg;

				SOCKET connected = INVALID_SOCKET;
				bool keep_connection = false;

				// - - - - WORKING - - - - //

				Tools::SuperThread<> thr_send { Tools::superthread::performance_mode::PERFORMANCE };
				Tools::SuperThread<> thr_recv { Tools::superthread::performance_mode::PERFORMANCE };

				std::atomic<long long> packages_since_sync = 0;
				std::atomic<unsigned long long> packages_received_trigger_sync = 0;

				// get_next_package_auto() handles these:
				Tools::SuperMutex send_pkg_mtx;
				std::vector<std::string> send_pkg_small_priority;
				std::vector<std::string> send_pkg_small_normal;
				std::vector<SmartFile> send_pkg_file;
				Tools::Waiter send_pkg_signal;

				// submit_next_package_recv() handles these:
				Tools::SuperMutex recv_pkg_mtx;
				std::vector<Package> recv_pkg_normal;
				Tools::Waiter recv_pkg_signal;

				Tools::SuperMutex read_over_mtx;
				std::function<void(Connection&, Package&)> read_over;

				Tools::SuperMutex err_debug_mtx;
				std::function<void(const std::string&)> err_debug;


				Tools::SuperMutex send_mtx;
				Tools::SuperMutex recv_mtx;


				bool has_priority_waiting() const;

				Package get_next_priority_auto();
				Package get_next_package_auto();

				void submit_next_package_recv(Package&&);

				bool slice_auto(Package&, __package&);

				void format_ping(__package&);
				bool interpret_ping_recv(const __package&);

				__package_type safer_cast_type(const int);
				bool manage_status_good(const std::string&, const connection::_connection_status&);

				void check_error_disconnect();

				// also debug()
				void err_f(const std::string&);

				void handle_send(Tools::boolThreadF);
				void handle_recv(Tools::boolThreadF);

				bool setup_socket_buffer_send();
				bool setup_socket_buffer_recv();

				connection::_connection_status auto_send(const __package&);
				connection::_connection_status auto_recv(__package&);

				connection::_connection_status ensure_send(const char*, const int);
				connection::_connection_status ensure_recv(char*, const int);

				connection::_connection_status combine(const connection::_connection_status, const connection::_connection_status);

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
				bool connect(const std::string & = "127.0.0.1", const int = connection::default_port);

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
				/// <para>How many Packages in memory received.</para>
				/// </summary>
				/// <returns>{size_t} Buffer size.</returns>
				size_t packages_received() const;

				/// <summary>
				/// <para>How many Packages in memory to send.</para>
				/// </summary>
				/// <returns>{size_t} Buffer size.</returns>
				size_t packages_sending() const;

				/// <summary>
				/// <para>Buffer load. It can be higher than 1.0 (100%) if too fast.</para>
				/// </summary>
				/// <returns>{double} Buffer usage [0.0,inf?]</returns>
				double buffer_sending_load() const;

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
				/// <para>Wait some time for a package.</para>
				/// </summary>
				/// <param name="{std::chrono::milliseconds}">Timeout. 0 = Infinite.</param>
				/// <returns>{bool} True if has something to get.</returns>
				bool wait_for_package(const std::chrono::milliseconds = std::chrono::milliseconds(0));

				/// <summary>
				/// <para>Get next package.</para>
				/// </summary>
				/// <param name="{bool}">Wait for package, if there's none yet?</param>
				/// <returns>{Package} Full data.</returns>
				Package get_next(const bool = true);

				/// <summary>
				/// <para>Send a package of bytes ahead of everyone. This will come as normal package on the other side, but as fast as it can.</para>
				/// </summary>
				/// <param name="{std::string}">The bytes you want to send.</param>
				/// <returns>{size_t} Added successfully?.</returns>
				size_t send_priority_package(const std::string&);

				/// <summary>
				/// <para>Send a package of bytes.</para>
				/// <para>PS: If you're planning on sending huge data (big files), you should use SmartFile.</para>
				/// </summary>
				/// <param name="{std::string}">The bytes you want to send.</param>
				/// <returns>{size_t} Current MemoryFile pending size.</returns>
				size_t send_package(const std::string&);

				/// <summary>
				/// <para>Send a package of bytes moving so no issues with medium sized data.</para>
				/// <para>PS: If you're planning on sending huge data (big files), you should use SmartFile.</para>
				/// </summary>
				/// <param name="{std::string}">The bytes you want to send.</param>
				/// <returns>{size_t} Current MemoryFile pending size.</returns>
				size_t send_package(std::string&&);

				/// <summary>
				/// <para>Send a package of bytes. May lock if maximum sending buffer has reached.</para>
				/// </summary>
				/// <param name="{SmartFile}">The data being sent.</param>
				/// <returns>{size_t} Current SmartFile vector size.</returns>
				size_t send_package(SmartFile&&);

				/// <summary>
				/// <para>Set a function to handle RECV RAW data.</para>
				/// <para>WARN: All data from RECV will be handled exclusively by your custom function if you set one!</para>
				/// <para>OBS: This won't work if overwrite_reads_small_chunk_to is set.</para>
				/// <para>PS: DO NOT SET A FUNCTION THAT CAN POTENTIALLY LOCK!</para>
				/// </summary>
				/// <param name="{std::function}">The function to handle a complete package reading.</param>
				void overwrite_reads_to(std::function<void(Connection&, Package&)>);

				/// <summary>
				/// <para>Resets to default way of handling packages.</para>
				/// </summary>
				void reset_overwrite_reads();

				/// <summary>
				/// <para>Set a function that receives errors.</para>
				/// <para>These errors can help you with bugs!</para>
				/// </summary>
				/// <param name="{std::function}">The function to show error somewhere.</param>
				void debug_error_function(std::function<void(const std::string&)> = std::function<void(const std::string&)>());

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