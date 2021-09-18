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
#include "../../Tools/Buffer/buffer.h"
#include "../../Tools/SuperMutex/supermutex.h"
#include "../../Tools/SuperThread/superthread.h"
#include "../../Tools/Common/common.h"
#include "../../Tools/Socket/socket.h"
#include "../Logger/logger.h"
#include "../Events/events.h"
#include "../EventTimer/eventtimer.h"
#include "../SmartFile/smartfile.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace connection {
				constexpr int default_port = 42069;
				constexpr unsigned package_size_udp = 400; // maximum safe ipv4 is 508, but 300-400 is better. // src= https://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet#:~:text=The%20maximum%20safe%20UDP%20payload%20is%20508%20bytes.&text=Except%20on%20an%20IPv6-only,bytes%20may%20be%20preferred%20instead.
				constexpr unsigned maximum_package_size_tcp = Tools::socket::default_max_package; // 32 kbytes // CAN'T BE HIGHER THAN 64kB!
				constexpr size_t limit_packages_stuck_send = 2; // Packages
				constexpr unsigned long long trigger_sync_send_thread = 10; // packages, bigger can cause higher ping. Consider LONG LONG limits!
				constexpr unsigned long long trigger_sync_multiplier_slowdown = 3; // sleep_for (this * (pack count - trigger_sync_send_thread) );
				constexpr unsigned long long trigger_max_time_allowed = 400;

				const double pinging_time = 20.0; // seconds
				//const double timeout_send = 3.0; // seconds

				//constexpr int automatic_self_point = 3000; // DON'T SET THIS TO less or eq 0.
				const auto default_performance_connection = Tools::superthread::performance_mode::PERFORMANCE;
			}

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
				unsigned long long packet_loss_count = 0;

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
				/// <para>Sad times, one more packet loss to add?</para>
				/// </summary>
				void packet_loss_add();

				/// <summary>
				/// <para>Get packet loss total count.</para>
				/// </summary>
				/// <returns>{unsigned long long} Total packet loss so far.</returns>
				unsigned long long packet_loss_total() const;

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
				// lower, higher priority (0 = highest priority)
				using priority_level = unsigned char;

				SmartFile file;
				Tools::Buffer data;
				priority_level priority = 0;

				Package(const Package&) = delete;
				void operator=(const Package&) = delete;

				Package() = default;

				Package(Package&&);
				void operator=(Package&&);

				Package(const char*, const priority_level = 0);
				Package(const std::string&, const priority_level = 0);
				Package(const Tools::Buffer&, const priority_level = 0);
				Package(const char*, SmartFile&&, const priority_level = 0);
				Package(const std::string&, SmartFile&&, const priority_level = 0);
				Package(const Tools::Buffer&, SmartFile&&, const priority_level = 0);

				void set(SmartFile&&);
				void set(const std::string&);
				void set(const Tools::Buffer&);
				void set(const priority_level);

				const SmartFile& get_file() const;
				SmartFile& get_file();
				SmartFile&& cut_file();
				const Tools::Buffer& get_data() const;
				Tools::Buffer& get_data();
				std::string get_string() const;
				std::string get_string();

				bool empty() const;
			};

			/// <summary>
			/// <para>A connection itself (one to one).</para>
			/// </summary>
			class Connection : Tools::SocketClient {
				enum class __package_type { DATA = 1, FILE, SYNC, PING, PONG }; // DATA, then FILE is recommended

				// - - - - ESSENTIAL - - - - //
				NetworkMonitor network_analysis;

				// - - - - WORKING - - - - //
				struct __any_package_info {
					unsigned long long internal_counter = 0; // set exactly before send()
					unsigned short size = 0;
					int checksum = 0;
					__package_type type = __package_type::DATA;
					Package::priority_level priority = 0;
					bool finale = false; // if true, there are more packages to combine
				};
				struct _conn_package_udp { // UDP has maximum "payload"
					__any_package_info info;
					char data[connection::package_size_udp - sizeof(__any_package_info)]{};
				};
				struct _conn_packageinfo_tcp { // this + any data size, because TCP is smort
					__any_package_info info;
				};
				// - - - - This creates an interface for any protocol - - - - //
				class handle_stuff {
					const Tools::socket::protocol& mode_copy;

					_conn_packageinfo_tcp tcp;
					Tools::Buffer tcp_buffer;

					_conn_package_udp udp;

					bool has_set = false;
				public:
					handle_stuff(const Tools::socket::protocol&);

					// get some bytes from Package. Returns if it was possible to "absorb" anything from Package. Package is automatically "erased". If package is empty, returns false.
					bool absorb(Package&);
					// get some bytes from Buffer. Returns true if size was less than the limit (it depends on protocol, see connection namespace)
					bool absorb(const __package_type&, const Tools::Buffer&);

					// absorbed data is transformed in raw data to send. unsigned long long is the internal package counter that you should ++ every send.
					Tools::Buffer transform(const unsigned long long);
				};

				Tools::SuperThread<> thr_send { Tools::superthread::performance_mode::PERFORMANCE };
				Tools::SuperThread<> thr_recv { Tools::superthread::performance_mode::PERFORMANCE };

				std::vector<Package> received;
				std::vector<Package> sending;

				Tools::Waiter recv_event;
				Tools::Waiter send_queue_update;

				Tools::SuperMutex send_mtx;
				Tools::SuperMutex recv_mtx;

				// counting_send = this side's count;
				// counting_send_other_side = other's side return of what it has received (this counting_send's received in other side); 
				// counting_recv = received counting (used to send so other side save as ..._other_side;
				// recvs_since_last_sync = this sum every recv, -= trigger_sync_send_thread when bigger that that
				std::atomic<unsigned long long> counting_send = 0, counting_send_other_side = 0, counting_recv = 0, recvs_since_last_sync = 0;
				std::atomic<unsigned long long> ping_calc = 0; // used on ping-pong. On pong, this should have the time when ping'ed. If == 0, can send new ping
				std::atomic<bool> pong_back_please = false; // on ping, this is set to true. Send thread sends pong and set this to false back.
				unsigned long long counting_recv_last_sync = 0; // when recvs_since_last_sync counts to 10, this is used to copy counting_recv

				Tools::SuperMutex recv_auto_mtx;
				std::function<void(Connection&, Package&)> recv_auto;

				bool dont_kill_conn = false;

				Package _send_package_buffer, _recv_package_buffer;


				// get how many packages ahead this is (package count)
				unsigned long long how_far_ahead_this_is() const;

				// if there's a function override, this will return true. If not handled by any function, false.
				bool recv_auto_is_function(Package&);

				// automatic send via SocketClient and network analysis add (if good)
				bool send_count_auto(const Tools::Buffer&);
				// automatic recv via SocketClient and network analysis add (if good)
				bool recv_count_auto(Tools::Buffer&, const size_t);
				

				void handle_send(Tools::boolThreadF);
				void handle_recv(Tools::boolThreadF);

				// starts handle's
				void init();

				friend class Hosting; // Hosting has Connection's there

				void force_move_from(Tools::SocketClient&&);
			public:
				Connection(const Connection&) = delete;
				void operator=(const Connection&) = delete;
				void operator=(Connection&&) = delete;
				Connection(Connection&&) = delete;
				Connection() = default;

				~Connection();

				/// <summary>
				/// <para>Connects to a URL/IP.</para>
				/// </summary>
				/// <param name="{std::string}">The URL/IP.</param>
				/// <param name="{socket::protocol}">UDP (as fast as it can get, but can fail sometimes) or TCP (reliable, but slower)?</param>
				/// <param name="{unsigned short}">Port number.</param>
				/// <returns>{bool} True if connected successfully.</returns>
				bool connect(const std::string& = "localhost", const Tools::socket::protocol& = Tools::socket::protocol::TCP, const u_short = connection::default_port);

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
				/// <para>Get information about ping, bytes sent, recv, average and so on.</para>
				/// </summary>
				/// <returns>{NetworkMonitor} Const reference to actual network info.</returns>
				const NetworkMonitor& get_network_info() const;

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
				/// <para>Send a package.</para>
				/// </summary>
				/// <param name="{Package}">A package to send</param>
				/// <param name="{bool}">Wait if limit has reached?</param>
				/// <returns>{unsigned long long} How many packages are in queue (if no queue, returns 1, because this is one in queue).</returns>
				unsigned long long send_package(Package&&, const bool = true);

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
				/// <para>Get number of packages that came broken (this doesn't count lost or missing packages)</para>
				/// </summary>
				/// <param name="{bool}">Just read or read and reset internal counter?</param>
				/// <returns>{unsigned long long} Amount of packages that failed on checksum.</returns>
				unsigned long long get_packet_loss_count(const bool = false);
			};

			/// <summary>
			/// <para>This handles host. It will connect and create Connection smart pointers that you can handle later.</para>
			/// </summary>
			class Hosting {
				struct _hostdata {
					Tools::SocketServer server;

					std::vector<std::shared_ptr<Connection>> conns;
					Tools::SuperMutex conns_mu;

					Tools::SuperThread<> conns_listen_auto;
				};

				std::unique_ptr<_hostdata> self = std::make_unique<_hostdata>();

				void _listen_auto(Tools::boolThreadF);
			public:
				Hosting(const Hosting&) = delete;
				Hosting(Hosting&&) = delete;
				void operator=(const Hosting&) = delete;
				void operator=(Hosting&&) = delete;

				Hosting() = default;

				/// <summary>
				/// <para>Initialize a Host.</para>
				/// </summary>
				/// <param name="{protocol}">Protocol used.</param>
				/// <param name="{u_short}">Port.</param>
				Hosting(const Tools::socket::protocol&, const u_short = connection::default_port);

				~Hosting();

				//using Tools::Socket::recv;
				//using Tools::Socket::send;

				/// <summary>
				/// <para>Initialize a Host.</para>
				/// </summary>
				/// <param name="{protocol}">Protocol used.</param>
				/// <param name="{u_short}">Port.</param>
				/// <returns>{bool} True if successful.</returns>
				bool start(const Tools::socket::protocol& = Tools::socket::protocol::TCP, const u_short = connection::default_port);

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
				bool is_running() const;

				/// <summary>
				/// <para>Sets a limit of connections (won't disconnect if amount connected right now is bigger than this value).</para>
				/// <para>New connections will connect and disconnect instantly.</para>
				/// </summary>
				/// <param name="{size_t}">Maximum amount of connections allowed.</param>
				//void set_connections_limit(const size_t);

				/// <summary>
				/// <para>Gets a specific connected client connection [0..size()-1].</para>
				/// <para>PS: The Connection object is created when this function is called. The actual socket information that is shared.</para>
				/// </summary>
				/// <param name="{size_t}">The connection position in vector.</param>
				/// <returns>{std::shared_ptr} The connection smart pointer.</returns>
				std::shared_ptr<Connection> get_connection(const size_t);

				/// <summary>
				/// <para>Gets latest connected client connection.</para>
				/// </summary>
				/// <returns>{std::shared_ptr} The connection smart pointer.</returns>
				std::shared_ptr<Connection> get_latest_connection();
			};

			int checksum_fast(const char*, const size_t, const unsigned short = 100);

		}
	}
}