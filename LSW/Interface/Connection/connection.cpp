#include "connection.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			void NetworkMonitor::__avg_task(nm_transf& n)
			{
				// this function is called once a sec, so current_added is how many bytes were added in the last sec
				n.current_bytes_per_sec = n._bytes_coming.exchange(0);

				n.bytes += n.current_bytes_per_sec;
				if (n.peak_bytes_per_second < n.current_bytes_per_sec) n.peak_bytes_per_second = n.current_bytes_per_sec;
				if (n.first_update_ms == 0) n.first_update_ms = Tools::now();
				n.last_update_ms = Tools::now();
			}

			void NetworkMonitor::_average_thr(const Interface::RawEvent& ev)
			{
				if (ev.type() == ALLEGRO_EVENT_TIMER && ev.timer_event().source == timer_second) {
					__avg_task(sending); // same formula
					__avg_task(recving); // same formula
				}
			}

			void NetworkMonitor::any_add(nm_transf& n, const unsigned long long a)
			{
				n._bytes_coming += a;
			}

			unsigned long long NetworkMonitor::any_get_total(const nm_transf& n) const
			{
				return n.bytes;
			}

			unsigned long long NetworkMonitor::any_get_peak(const nm_transf& n) const
			{
				return n.peak_bytes_per_second;
			}

			unsigned long long NetworkMonitor::any_get_current_bytes_per_second(const nm_transf& n) const
			{
				return n.current_bytes_per_sec;
			}

			unsigned long long NetworkMonitor::any_get_average_total(const nm_transf& n) const
			{
				const auto diff_t = (n.last_update_ms - n.first_update_ms);
				return n.bytes / (diff_t ? diff_t : 1); // if never updated, give current bytes
			}

			NetworkMonitor::NetworkMonitor()
			{
				timer_second.set_delta(1.0);
				per_sec_calc.add(timer_second);
				per_sec_calc.set_run_autostart([&](const Interface::RawEvent& ev) { _average_thr(ev); });
				timer_second.start();
			}

			NetworkMonitor::~NetworkMonitor()
			{
				timer_second.stop();
				per_sec_calc.stop();
			}

			void NetworkMonitor::clear()
			{
				ping = nm_ping{};
				sending.first_update_ms = sending.last_update_ms = sending.peak_bytes_per_second = sending.bytes = sending.current_bytes_per_sec = 0;
				sending._bytes_coming = 0;
				recving.first_update_ms = recving.last_update_ms = recving.peak_bytes_per_second = recving.bytes = recving.current_bytes_per_sec = 0;
				recving._bytes_coming = 0;
				packet_loss_count = 0;
			}

			void NetworkMonitor::packet_loss_add()
			{
				packet_loss_count++;
			}

			unsigned long long NetworkMonitor::packet_loss_total() const
			{
				return packet_loss_count;
			}

			// ping stuff
			void NetworkMonitor::ping_new(const unsigned a)
			{
				ping.current = a;
				if (ping.adaptative_avg == 0.0) ping.adaptative_avg = a; // jump to number if zero lol
				ping.adaptative_avg = ((1.0 * ping.adaptative_avg * ping.adaptativeness) + ping.current) / (ping.adaptativeness + 1.0);
				if (a > ping.peak) ping.peak = a;
			}

			void NetworkMonitor::ping_set_adaptativeness(const double a)
			{
				if (a > 0.0) ping.adaptativeness = a;
			}

			unsigned NetworkMonitor::ping_now() const
			{
				return ping.current;
			}

			unsigned NetworkMonitor::ping_peak() const
			{
				return ping.peak;
			}

			double NetworkMonitor::ping_average_now() const
			{
				return ping.adaptative_avg;
			}

			// transf send
			void NetworkMonitor::send_add(const unsigned long long a)
			{
				any_add(sending, a);
			}

			unsigned long long NetworkMonitor::send_get_total() const
			{
				return any_get_total(sending);
			}

			unsigned long long NetworkMonitor::send_get_peak() const
			{
				return any_get_peak(sending);
			}

			unsigned long long NetworkMonitor::send_get_current_bytes_per_second() const
			{
				return any_get_current_bytes_per_second(sending);
			}

			unsigned long long NetworkMonitor::send_get_average_total() const
			{
				return any_get_average_total(sending);
			}

			// transf recv
			void NetworkMonitor::recv_add(const unsigned long long a)
			{
				any_add(recving, a);
			}

			unsigned long long NetworkMonitor::recv_get_total() const
			{
				return any_get_total(recving);
			}

			unsigned long long NetworkMonitor::recv_get_peak() const
			{
				return any_get_peak(recving);
			}

			unsigned long long NetworkMonitor::recv_get_current_bytes_per_second() const
			{
				return any_get_current_bytes_per_second(recving);
			}

			unsigned long long NetworkMonitor::recv_get_average_total() const
			{
				return any_get_average_total(recving);
			}


			Package::Package(Package&& oth)
			{
				(*this) = std::move(oth);
			}

			void Package::operator=(Package&& oth)
			{
				file = std::move(oth.file);
				data = std::move(oth.data);
				priority = oth.priority;
			}

			Package::Package(const char* str, const priority_level pri)
			{
				data = str;
				priority = pri;
			}

			Package::Package(const std::string& str, const priority_level pri)
			{
				priority = pri;
				data = str;
			}

			Package::Package(const Tools::Buffer& vec, const priority_level pri)
			{
				priority = pri;
				data = vec;
			}

			Package::Package(const char* str, SmartFile&& fp, const priority_level pri)
			{
				data = str;
				priority = pri;
				file = std::move(fp);
			}

			Package::Package(const std::string& str, SmartFile&& fp, const priority_level pri)
			{
				data = str;
				priority = pri;
				file = std::move(fp);
			}

			Package::Package(const Tools::Buffer& vec, SmartFile&& fp, const priority_level pri)
			{
				priority = pri;
				data = vec;
				file = std::move(fp);
			}

			void Package::set(SmartFile&& fp)
			{
				file = std::move(fp);
			}

			void Package::set(const std::string& str)
			{
				std::copy(str.begin(), str.end(), data.begin());
			}

			void Package::set(const Tools::Buffer& vec)
			{
				data = vec;
			}

			void Package::set(const priority_level pri)
			{
				priority = pri;
			}

			const SmartFile& Package::get_file() const
			{
				return file;
			}

			SmartFile& Package::get_file()
			{
				return file;
			}
			
			SmartFile&& Package::cut_file()
			{
				return std::move(file);
			}
			
			const Tools::Buffer& Package::get_data() const
			{
				return data;
			}
			
			Tools::Buffer& Package::get_data()
			{
				return data;
			}
			
			std::string Package::get_string() const
			{
				std::string str;
				str.resize(data.size());
				std::copy(data.begin(), data.end(), str.begin());
				return str;
			}

			std::string Package::get_string()
			{
				std::string str;
				str.resize(data.size());
				std::copy(data.begin(), data.end(), str.begin());
				return str;
			}

			bool Package::empty() const
			{
				return data.empty() && !file.is_open();
			}


			void Connection::handle_send(Tools::boolThreadF run)
			{
				Logger logg;
				_send_package_buffer = {};

				bool should_ping = true;
				EventTimer timm(connection::pinging_time);
				EventHandler evhdr{ Tools::superthread::performance_mode::EXTREMELY_LOW_POWER };

				evhdr.add(timm);
				evhdr.set_run_autostart([&](const Interface::RawEvent& re) {
					if (re.timer_event().source == timm) {
						should_ping = true;
					}
				});
				timm.start();

				handle_stuff handling{ this->get_protocol() };

				while (run())
				{
					try {
						const auto far_ahead = how_far_ahead_this_is();

						// slow down if not in sync
						if (far_ahead > connection::trigger_sync_send_thread) {
							//std::cout << "slowdown" << std::endl;
							auto diff = connection::trigger_sync_multiplier_slowdown * far_ahead - connection::trigger_sync_send_thread;
							if (diff > connection::trigger_max_time_allowed) diff = connection::trigger_max_time_allowed;
							Tools::sleep_for(std::chrono::milliseconds(diff));
						}

						__any_package_info info;
						bool good_to_send = false;

						if (recvs_since_last_sync >= connection::trigger_sync_send_thread && counting_recv_last_sync != counting_recv)
						{
							recvs_since_last_sync -= connection::trigger_sync_send_thread;
							counting_recv_last_sync = counting_recv; // update that on other side, "I'm at XXX"

							good_to_send = handling.absorb(__package_type::SYNC, { (const char*)&counting_recv_last_sync, sizeof(counting_recv_last_sync) });
						}
						else if (pong_back_please)
						{
							pong_back_please = false;

							good_to_send = handling.absorb(__package_type::PONG, {});
						}
						else if (should_ping && ping_calc == 0) // 0 == did pong
						{
							should_ping = false;
							ping_calc = Tools::now();

							good_to_send = handling.absorb(__package_type::PING, {});
						}
						else if (_send_package_buffer.empty()) // if no package, get one or continue
						{
							if (sending.size() == 0) {
								std::this_thread::yield();
								send_queue_update.wait_signal(50);
								continue; // nothing to do
							}

							Tools::AutoLock l(send_mtx);

							_send_package_buffer = std::move(sending.front());
							sending.erase(sending.begin());
							continue;
						}
						else if (!_send_package_buffer.empty())
						{
							good_to_send = handling.absorb(_send_package_buffer);
						}



						/*_conn_package_udp small;


						if (recvs_since_last_sync >= connection::trigger_sync_send_thread)
						{
							recvs_since_last_sync -= connection::trigger_sync_send_thread;
							unsigned long long res = counting_recv; // update that on other side, "I'm at XXX"
							//std::cout << "sync #" << res << std::endl;

							std::copy((char*)&res, (char*)&res + sizeof(res), (char*)small.data);
							small.info.finale = true;
							small.info.size = sizeof(res);
							small.info.type = __package_type::SYNC;
						}
						else if (pong_back_please) 
						{
							pong_back_please = false;

							small.info.finale = true;
							small.info.size = 0;
							small.info.type = __package_type::PONG;
						}
						else if (should_ping && ping_calc == 0) // 0 == did pong
						{
							should_ping = false;
							ping_calc = Tools::now();

							small.info.finale = true;
							small.info.size = 0;
							small.info.type = __package_type::PING;
						}
						else if (_send_package_buffer.empty()) // if no package, get one or continue
						{
							if (sending.size() == 0) {
								std::this_thread::yield();
								continue; // nothing to do
							}

							Tools::AutoLock l(send_mtx);

							_send_package_buffer = std::move(sending.front());
							sending.erase(sending.begin());
							continue;
						}
						else // has package to send.
						{


							small.info.priority = _send_package_buffer.priority; // priority

							if (_send_package_buffer.data.size()) {
								small.info.type = __package_type::DATA; // type

								size_t len_copying = _send_package_buffer.data.size();
								if (len_copying > sizeof(small.data)) len_copying = sizeof(small.data);
								small.info.size = static_cast<unsigned short>(len_copying); // size

								std::copy(_send_package_buffer.data.begin(), _send_package_buffer.data.begin() + len_copying, (char*)small.data); // data
								_send_package_buffer.data.erase(_send_package_buffer.data.begin(), _send_package_buffer.data.begin() + len_copying);

								small.info.finale = _send_package_buffer.data.size() == 0 && _send_package_buffer.file.eof(); // has more? // BIG OBSERVATION: FILE EOF BECAUSE IF THERE'S FILE, THIS IS NOT THE END!!!!!!!!! <<<<<<<<< LOOK, DON'T FORGET!!!!
							}
							else if (!_send_package_buffer.file.eof()) {
								small.info.type = __package_type::FILE; // type

								Tools::Buffer _temp;
								small.info.size = static_cast<unsigned short>(_send_package_buffer.file.read(_temp, sizeof(small.data))); // size

								std::copy(_temp.begin(), _temp.end(), (char*)small.data); // data

								small.info.finale = _send_package_buffer.file.eof(); // has more?
							}
							else {
								_send_package_buffer = {};
								logg << L::SLF << fsr(E::ERRR) << "Exception: invalid internal package state." << L::ELF;
								continue; // what?
							}
						}*/

						// HERE THERE BE PACKAGES TO SEND! (take a look at continue;s!)

						// ordering package
						//small.info.internal_counter = ++counting_send;
						//small.info.checksum = checksum_fast(small.data, sizeof(small.data));
						// transform package
						//Tools::Buffer _transf;
						//_transf.resize(sizeof(small));
						//std::copy((char*)&small, (char*)&small + sizeof(small), _transf.begin());

						if (!good_to_send) continue;

						auto response = handling.transform(++counting_send);
						if (response.empty()) {
							--counting_send; // failed count
							logg << L::SLF << fsr(E::WARN) << "Exception: Unexpected empty data in final send code." << L::ELF;
							continue;
						}

						if (!send_count_auto(response))
						{
							logg << L::SLF << fsr(E::ERRR) << "Exception: can't send data." << L::ELF;
						}
					}
					catch (const Handling::Abort& ab)
					{
						logg << L::SLF << fsr(E::ERRR) << "Something aborted in Connection's send thread: " << +ab << L::ELF;
					}
					catch (const std::exception& e)
					{
						logg << L::SLF << fsr(E::ERRR) << "Something aborted in Connection's send thread: " << e.what() << L::ELF;
					}
					catch (...) 
					{
						logg << L::SLF << fsr(E::ERRR) << "Something aborted in Connection's send thread! Uncaught!" << L::ELF;
					}
				}
			}

			void Connection::handle_recv(Tools::boolThreadF run)
			{
				Logger logg;
				_recv_package_buffer = {};

				while (run())
				{
					try {

						Tools::Buffer tempbuf;

						auto good = this->recv_count_auto(tempbuf, get_protocol() == Tools::socket::protocol::TCP ? sizeof(_conn_packageinfo_tcp) : sizeof(_conn_package_udp));
						if (!good) {
							std::this_thread::yield();
							continue;
						}
						if (tempbuf.size() < sizeof(__any_package_info)) {
							logg << L::SLF << fsr(E::ERRR) << "Exception: invalid package size! Skipped." << L::ELF;
						}

						network_analysis.recv_add(tempbuf.size());

						__any_package_info& info = *(__any_package_info*)tempbuf.data();
						Tools::Buffer buffer;
						if (tempbuf.size() > sizeof(__any_package_info)) {
							buffer = Tools::Buffer{ tempbuf.data() + sizeof(__any_package_info), tempbuf.size() - sizeof(__any_package_info) };
						}
						else if (info.size > 0 && get_protocol() == Tools::socket::protocol::TCP) {
							while (!this->recv_count_auto(buffer, info.size)) {
								logg << L::SLF << fsr(E::ERRR) << "Exception: can't download file data! Can't continue if can't download data!" << L::ELF;
								Tools::sleep_for(std::chrono::milliseconds(30));
							}
						}

						if (!_recv_package_buffer.empty()) {
							if (info.checksum != checksum_fast(buffer.data(), buffer.size()))
							{
								logg << L::SLF << fsr(E::ERRR) << "Exception: checksum failed. Saving what came right." << L::ELF;
								network_analysis.packet_loss_add();

								Tools::AutoLock l(recv_mtx);
								received.push_back(std::move(_recv_package_buffer)); // clean up
								recv_event.signal_one();
							}
							else if (info.internal_counter != (counting_recv + 1))
							{
								if (info.internal_counter == counting_recv)
								{
									logg << L::SLF << fsr(E::WARN) << "Exception: duplicated package received. Skipping duplicated." << L::ELF;
									continue;
								}
								else
								{
									if (info.internal_counter >= counting_recv)
										network_analysis.packet_loss_add();

									logg << L::SLF << fsr(E::ERRR) << "Exception: ordering messed up." << L::ELF;

									Tools::AutoLock l(recv_mtx);
									received.push_back(std::move(_recv_package_buffer)); // clean up
									recv_event.signal_one();
								}
							}
							else if (info.priority != _recv_package_buffer.priority)
							{
								logg << L::SLF << fsr(E::ERRR) << "Exception: priority don't match before end." << L::ELF;

								Tools::AutoLock l(recv_mtx);
								received.push_back(std::move(_recv_package_buffer)); // clean up
								recv_event.signal_one();
							}
						}

						_recv_package_buffer.priority = info.priority; // priority
						if (info.internal_counter > counting_recv) counting_recv = info.internal_counter; // only further.

						switch (info.type)
						{
						case __package_type::SYNC:
						{
							// other side's recv count, save on counting_send_other_side
							unsigned long long temp = 0;

							if (buffer.size() < sizeof(temp)) {
								logg << L::SLF << fsr(E::ERRR) << "Exception: sync data size is smaller than expected." << L::ELF;
								continue;
							}

							std::copy(buffer.begin(), buffer.begin() + sizeof(temp), (char*)&temp);

							/*if (temp <= counting_send_other_side) {
								logg << L::SLF << fsr(E::ERRR) << "Exception: sync counting other side's recv is behind what they said they were before." << L::ELF;
							}
							else counting_send_other_side = temp;*/
							if (temp > counting_send_other_side) counting_send_other_side = temp;
						}
						continue; // no data to save
						case __package_type::PING:
						{
							pong_back_please = true;
						}
						continue; // no data to save
						case __package_type::PONG:
						{
							const auto diff = (Tools::now() - ping_calc) / 2;
							ping_calc = 0; // can call new ping
							network_analysis.ping_new(static_cast<unsigned int>(diff));
						}
						continue; // no data to save
						case __package_type::DATA:
						{
							size_t siz = _recv_package_buffer.data.size();
							_recv_package_buffer.data.resize(siz + info.size); // size
							std::copy(buffer.begin(), buffer.begin() + info.size, _recv_package_buffer.data.begin() + siz);
						}
						break;
						case __package_type::FILE:
						{
							if (!_recv_package_buffer.file.is_open()) {
								if (!_recv_package_buffer.file.open_temp()) {
									logg << L::SLF << fsr(E::ERRR) << "Exception: can't open temporary file. Data was lost!" << L::ELF;
									continue;
								}
							}

							/*std::string _temp;
							_temp.resize(info.size); // size
							std::copy((char*)pkg.data, (char*)pkg.data + pkg.info.size, _temp.begin()); // be sure*/

							_recv_package_buffer.file.write(buffer, info.size);
						}
						break;
						}

						if (info.finale) {
							if (!recv_auto_is_function(_recv_package_buffer)) {
								Tools::AutoLock l(recv_mtx);
								received.push_back(std::move(_recv_package_buffer));
								recv_event.signal_one();
							}
							_recv_package_buffer = {};
						}

						/*_conn_package_udp pkg;

						Tools::Buffer _transf;

						auto good = this->recv_count_auto(_transf, sizeof(_conn_package_udp));

						if (!good) {
							std::this_thread::yield();
							continue;
						}

						network_analysis.recv_add(_transf.size());

						_transf.copy_to((void*)&pkg, sizeof(pkg));
						//std::copy(_transf.begin(), _transf.end(), (char*)&pkg);

						if (!_recv_package_buffer.empty()) {
							if (pkg.info.checksum != checksum_fast(pkg.data, sizeof(pkg.data)))
							{
								logg << L::SLF << fsr(E::ERRR) << "Exception: checksum failed. Saving what came right." << L::ELF;
								network_analysis.packet_loss_add();

								Tools::AutoLock l(recv_mtx);
								received.push_back(std::move(_recv_package_buffer)); // clean up
								recv_event.signal_one();
							}
							else if (pkg.info.internal_counter != (counting_recv + 1))
							{
								if (pkg.info.internal_counter == counting_recv)
								{
									logg << L::SLF << fsr(E::WARN) << "Exception: duplicated package received. Skipping duplicated." << L::ELF;
									continue;
								}
								else
								{
									logg << L::SLF << fsr(E::ERRR) << "Exception: ordering messed up." << L::ELF;

									Tools::AutoLock l(recv_mtx);
									received.push_back(std::move(_recv_package_buffer)); // clean up
									recv_event.signal_one();
								}
							}
							else if (pkg.info.priority != _recv_package_buffer.priority)
							{
								logg << L::SLF << fsr(E::ERRR) << "Exception: priority don't match before end." << L::ELF;

								Tools::AutoLock l(recv_mtx);
								received.push_back(std::move(_recv_package_buffer)); // clean up
								recv_event.signal_one();
							}
						}

						_recv_package_buffer.priority = pkg.info.priority; // priority
						if (pkg.info.internal_counter > counting_recv) counting_recv = pkg.info.internal_counter; // only further.

						switch (pkg.info.type)
						{
						case __package_type::SYNC:
						{
							// other side's recv count, save on counting_send_other_side
							unsigned long long temp = 0;
							std::copy((char*)pkg.data, (char*)pkg.data + sizeof(temp), (char*)&temp);
							if (temp <= counting_send_other_side) {
								logg << L::SLF << fsr(E::ERRR) << "Exception: sync counting other side's recv is behind what they said they were before." << L::ELF;
							}
							else counting_send_other_side = temp;
						}
						continue; // no data to save
						case __package_type::PING:
						{
							pong_back_please = true;
						}
						continue; // no data to save
						case __package_type::PONG:
						{
							const auto diff = (Tools::now() - ping_calc) / 2;
							ping_calc = 0; // can call new ping
							network_analysis.ping_new(static_cast<unsigned int>(diff));
						}
						continue; // no data to save
						case __package_type::DATA:
						{
							size_t siz = _recv_package_buffer.data.size();
							_recv_package_buffer.data.resize(siz + pkg.info.size); // size
							std::copy((char*)pkg.data, (char*)pkg.data + pkg.info.size, _recv_package_buffer.data.begin() + siz);
						}
						break;
						case __package_type::FILE:
						{
							if (!_recv_package_buffer.file.is_open()) {
								if (!_recv_package_buffer.file.open_temp()) {
									logg << L::SLF << fsr(E::ERRR) << "Exception: can't open temporary file. Data was lost!" << L::ELF;
									continue;
								}
							}

							std::string _temp;
							_temp.resize(pkg.info.size); // size
							std::copy((char*)pkg.data, (char*)pkg.data + pkg.info.size, _temp.begin()); // be sure

							_recv_package_buffer.file.write(_temp, pkg.info.size);
						}
						break;
						}

						if (pkg.info.finale) {
							if (!recv_auto_is_function(_recv_package_buffer)) {
								Tools::AutoLock l(recv_mtx);
								received.push_back(std::move(_recv_package_buffer));
								recv_event.signal_one();
							}
							_recv_package_buffer = {};
						}*/
					}
					catch (const Handling::Abort& ab)
					{
						logg << L::SLF << fsr(E::ERRR) << "Something aborted in Connection's recv thread: " << +ab << L::ELF;
					}
					catch (const std::exception& e)
					{
						logg << L::SLF << fsr(E::ERRR) << "Something aborted in Connection's recv thread: " << e.what() << L::ELF;
					}
					catch (...)
					{
						logg << L::SLF << fsr(E::ERRR) << "Something aborted in Connection's recv thread! Uncaught!" << L::ELF;
					}
				}
			}

			void Connection::init()
			{
				thr_send.stop();
				thr_recv.stop();

				//network_analysis.clear();

				thr_send.set([&](auto b) {handle_send(b); });
				thr_recv.set([&](auto b) {handle_recv(b); });

				thr_send.start();
				thr_recv.start();
			}



			Connection::handle_stuff::handle_stuff(const Tools::socket::protocol& prot)
				: mode_copy(prot)
			{
			}

			bool Connection::handle_stuff::absorb(Package& pkg)
			{
				switch (mode_copy) {
				case Tools::socket::protocol::UDP:
				{
					if (!pkg.empty()) {
						// - - - - - - - - - - - - - PACKAGE DATA/FILE - - - - - - - - - - - - - //
						udp.info.priority = pkg.priority; // priority

						if (pkg.data.size()) {
							udp.info.type = __package_type::DATA; // type

							size_t len_copying = pkg.data.size();
							if (len_copying > sizeof(udp.data)) len_copying = sizeof(udp.data);
							udp.info.size = static_cast<unsigned short>(len_copying); // size

							std::copy(pkg.data.begin(), pkg.data.begin() + len_copying, (char*)udp.data); // data
							pkg.data.erase(pkg.data.begin(), pkg.data.begin() + len_copying);

							udp.info.finale = pkg.data.size() == 0 && pkg.file.eof(); // has more? // BIG OBSERVATION: FILE EOF BECAUSE IF THERE'S FILE, THIS IS NOT THE END!!!!!!!!! <<<<<<<<< LOOK, DON'T FORGET!!!!
						}
						else if (!pkg.file.eof()) {
							udp.info.type = __package_type::FILE; // type

							Tools::Buffer _temp;
							udp.info.size = static_cast<unsigned short>(pkg.file.read(_temp, sizeof(udp.data))); // size

							std::copy(_temp.begin(), _temp.end(), (char*)udp.data); // data

							udp.info.finale = pkg.file.eof(); // has more?
						}
						// - - - - - - - - - - - - - PACKAGE DATA/FILE - - - - - - - - - - - - - //
					}
					else return false;

					udp.info.checksum = checksum_fast(udp.data, udp.info.size);
					has_set = true;
				}
					break;
				case Tools::socket::protocol::TCP:
				{
					tcp_buffer.resize(0); // reset, but it may not shrink by itself. No problem.

					if (!pkg.empty()) {
						// - - - - - - - - - - - - - PACKAGE DATA/FILE - - - - - - - - - - - - - //
						tcp.info.priority = pkg.priority; // priority

						if (pkg.data.size()) {
							tcp.info.type = __package_type::DATA; // type

							size_t len_copying = pkg.data.size();
							if (len_copying > (connection::maximum_package_size_tcp - sizeof(__any_package_info))) len_copying = (connection::maximum_package_size_tcp - sizeof(__any_package_info));
							tcp.info.size = static_cast<unsigned short>(len_copying); // size
							tcp_buffer.resize(len_copying);

							std::copy(pkg.data.begin(), pkg.data.begin() + len_copying, tcp_buffer.begin()); // data
							pkg.data.erase(pkg.data.begin(), pkg.data.begin() + len_copying);

							tcp.info.finale = pkg.data.size() == 0 && pkg.file.eof(); // has more? // BIG OBSERVATION: FILE EOF BECAUSE IF THERE'S FILE, THIS IS NOT THE END!!!!!!!!! <<<<<<<<< LOOK, DON'T FORGET!!!!
						}
						else if (!pkg.file.eof()) {
							tcp.info.type = __package_type::FILE; // type

							Tools::Buffer _temp;
							tcp.info.size = static_cast<unsigned short>(pkg.file.read(_temp, (connection::maximum_package_size_tcp - sizeof(__any_package_info)))); // size
							tcp_buffer.resize(tcp.info.size);

							std::copy(_temp.begin(), _temp.end(), tcp_buffer.begin()); // data

							tcp.info.finale = pkg.file.eof(); // has more?
						}
						// - - - - - - - - - - - - - PACKAGE DATA/FILE - - - - - - - - - - - - - //
					}
					else return false;

					tcp.info.checksum = checksum_fast(tcp_buffer.data(), tcp_buffer.size());
					has_set = true;
				}
					break;
				}
				return true;
			}

			bool Connection::handle_stuff::absorb(const Connection::__package_type& info, const Tools::Buffer& if_no_pkg)
			{
				switch (mode_copy) {
				case Tools::socket::protocol::UDP:
				{
					udp.info.type = info;
										
					if (if_no_pkg.size() > sizeof(udp.data)) return false; // can't

					std::copy(if_no_pkg.begin(), if_no_pkg.end(), udp.data);
					udp.info.size = static_cast<unsigned short>(if_no_pkg.size());
					udp.info.finale = true;

					udp.info.checksum = checksum_fast(udp.data, udp.info.size);
					has_set = true;
				}
					break;
				case Tools::socket::protocol::TCP:
				{
					tcp_buffer.resize(0); // reset, but it may not shrink by itself. No problem.
					tcp.info.type = info;
										
					if (if_no_pkg.size() > (connection::maximum_package_size_tcp - sizeof(__any_package_info))) return false; // can't

					tcp_buffer.resize(if_no_pkg.size());

					std::copy(if_no_pkg.begin(), if_no_pkg.end(), tcp_buffer.begin());
					tcp.info.size = static_cast<unsigned short>(if_no_pkg.size());
					tcp.info.finale = true;

					tcp.info.checksum = checksum_fast(tcp_buffer.data(), tcp_buffer.size());
					has_set = true;
				}
					break;
				}
				return true;
			}

			Tools::Buffer Connection::handle_stuff::transform(const unsigned long long finale_count)
			{
				if (!has_set) return {};

				Tools::Buffer buf;

				switch (mode_copy) {
				case Tools::socket::protocol::UDP:
				{
					udp.info.internal_counter = finale_count;
					buf.resize(sizeof(udp));

					std::copy((char*)&udp, (char*)&udp + sizeof(udp), buf.begin());
				}
					break;
				case Tools::socket::protocol::TCP:
				{
					tcp.info.internal_counter = finale_count;
					buf.resize(sizeof(tcp) + tcp_buffer.size());

					std::copy((char*)&tcp, (char*)&tcp + sizeof(tcp), buf.begin());
					std::copy(tcp_buffer.begin(), tcp_buffer.end(), buf.begin() + sizeof(tcp));
				}
					break;
				}

				return std::move(buf);
			}



			unsigned long long Connection::how_far_ahead_this_is() const
			{
				return counting_send - counting_send_other_side;
			}

			bool Connection::recv_auto_is_function(Package& pkg)
			{
				if (recv_auto) {
					Tools::AutoLock l(recv_auto_mtx);
					if (recv_auto) {
						recv_auto(*this, pkg);
						return true;
					}
				}
				return false;
			}

			bool Connection::send_count_auto(const Tools::Buffer& vec)
			{
				if (this->SocketClient::send(vec)) {
					network_analysis.send_add(vec.size());
					//send_calls++;
					return true;
				}
				return false;
			}

			bool Connection::recv_count_auto(Tools::Buffer& vec, const size_t lim)
			{
				if (this->SocketClient::recv(vec, lim)) {
					network_analysis.recv_add(vec.size());
					recvs_since_last_sync++;
					return true;
				}
				return false;
			}

			// only accessible via friend or internally.
			void Connection::force_move_from(Tools::SocketClient&& oth)
			{
				close();
				/*oth.thr_send.stop();
				oth.thr_recv.stop();
				oth.thr_send.join();
				oth.thr_recv.join();*/
				dont_kill_conn = true; // shared
				(*(SocketClient*)this) = std::move(oth); // what else?
				init(); // init again
			}

			Connection::~Connection()
			{
				close();
			}

			bool Connection::connect(const std::string& url, const Tools::socket::protocol& protocol, const u_short port)
			{
				using namespace LSW::v5::Tools;
				if (!this->SocketClient::connect(url, port, protocol, socket::family::ANY)) return false;
				//if (res != Tools::socket::result_type::NO_ERROR) return false;
				init();
				return true;
			}

			void Connection::close()
			{
				//if (core) core->printlog(Tools::socket::log_event_type::INFO, "Connection::close() has been called.");
				thr_send.stop();
				thr_recv.stop();
				//if (dont_kill_conn && this->core->client.self.get()) this->core->client.self = std::make_shared<Tools::Socket::_data::_conn_info::referenceable>();
				this->Tools::SocketClient::close();
			}

			bool Connection::is_connected() const
			{
				if (get_protocol() == Tools::socket::protocol::TCP) {
					Tools::Buffer buf;
					const auto res = this->Tools::SocketClient::peek(buf); // can get err code here
					return res > 0;
				}
				else { // UDP is connectionless.
					return valid();
				}
				//return this->Tools::Socket::is_connected() != Tools::socket::connection_status::DISCONNECTED && (thr_send.running() && thr_recv.running());
			}

			const NetworkMonitor& Connection::get_network_info() const
			{
				return network_analysis;
			}

			size_t Connection::packages_received() const
			{
				return received.size();
			}

			bool Connection::has_package() const
			{
				return received.size();
			}

			size_t Connection::packages_sending() const
			{
				return sending.size();
			}

			bool Connection::wait_for_package(const std::chrono::milliseconds t)
			{
				if (has_package()) return true;
				if (t.count() == 0) { // UNLIMITED TIME
					while (!has_package()) recv_event.wait_signal(200);
				}
				else { // ONE TRY
					if (!has_package()) recv_event.wait_signal(t.count());
				}
				return has_package();
			}

			Package Connection::get_next(const bool wait)
			{
				while (wait && !wait_for_package(std::chrono::milliseconds(50))) std::this_thread::yield();
				if (!has_package()) return Package{};

				Tools::AutoLock l(recv_mtx);

				Package mov = std::move(received.front());
				received.erase(received.begin());
				return std::move(mov);
			}

			unsigned long long Connection::send_package(Package&& pkg, const bool wait_if_limit)
			{
				if (wait_if_limit) {
					while (sending.size() >= connection::limit_packages_stuck_send) {
						Tools::sleep_for(std::chrono::milliseconds(50));
						std::this_thread::yield();
					}
				}

				Tools::AutoLock l(send_mtx);
				// organize
				for (size_t p = 0; p < sending.size(); p++)
				{
					if (sending[p].priority > pkg.priority)
					{
						sending.insert(sending.begin() + p, std::move(pkg));
						return static_cast<unsigned long long>(sending.size());
					}
				}
				sending.push_back(std::move(pkg));
				send_queue_update.signal_one();
				return static_cast<unsigned long long>(sending.size());
			}

			void Connection::overwrite_reads_to(std::function<void(Connection&, Package&)> f)
			{
				Tools::AutoLock l(recv_auto_mtx);
				recv_auto = f;
				if (!recv_auto) return;
				while (has_package()) {
					auto pkg = get_next();
					recv_auto(*this, pkg);
				}
			}

			void Connection::reset_overwrite_reads()
			{
				Tools::AutoLock l(recv_auto_mtx);
				recv_auto = std::function<void(Connection&, Package&)>();
			}






			void Hosting::_listen_auto(Tools::boolThreadF run)
			{
				auto* const interf = self.get();

				while (run())
				{
					std::shared_ptr<Connection> newclient = std::make_shared<Connection>();
					try {
						newclient->force_move_from(std::move(interf->server.listen()));
					}
					catch (...) { // enhance later
						continue;
					}

					Tools::AutoLock l(interf->conns_mu);

					if (newclient->valid()) {
						interf->conns.push_back(std::move(newclient));
					}
				}
			}

			Hosting::Hosting(const Tools::socket::protocol& protocol, const u_short port)
			{
				if (!start(protocol, port)) throw Handling::Abort(__FUNCSIG__, "Failed to start Hosting.", Handling::abort::abort_level::GIVEUP);
			}

			Hosting::~Hosting()
			{
				close();
			}

			bool Hosting::start(const Tools::socket::protocol& protocol, const u_short port)
			{
				close();
				Tools::AutoLock l(self->conns_mu);
				if (self->server.start(port, protocol, Tools::socket::family::ANY)) {
					self->conns_listen_auto.set([&](auto f) {_listen_auto(f); });
					self->conns_listen_auto.start();
					return true;
				}
				return false;
			}

			size_t Hosting::size() const
			{
				return self->conns.size();
			}

			void Hosting::close()
			{
				self->conns_listen_auto.stop();
				self->server.close();
				self->conns_listen_auto.join();

				Tools::AutoLock l(self->conns_mu);
				for (auto& i : self->conns) i->close();
				self->conns.clear();
			}

			bool Hosting::is_running() const
			{
				return self->server.has_servers() && self->conns_listen_auto.running();
			}

			std::shared_ptr<Connection> Hosting::get_connection(const size_t p)
			{
				Tools::AutoLock l(self->conns_mu);
				if (p >= self->conns.size()) return {};
				return self->conns[p];
				/*Connection conn;
				conn.core->_mode = Tools::socket::mode::CLIENT;
				conn.core->client.self = core->connected[p].self;
				return conn;*/
			}

			std::shared_ptr<Connection> Hosting::get_latest_connection()
			{
				Tools::AutoLock l(self->conns_mu);
				if (self->conns.size() == 0) return {};
				return self->conns.back();
				/*Connection conn;
				conn.core->_mode = Tools::socket::mode::CLIENT;
				conn.core->client.self = core->connected.back().self;
				return std::move(conn);*/
			}


			int checksum_fast(const char* ptr, const size_t len, const unsigned short max)
			{
				int checksum = 0;
				for (size_t p = 0; p < len; p++)
				{
					checksum += static_cast<int>(ptr[p] + std::numeric_limits<char>::max());
					checksum %= max;
				}
				return (checksum + 1) % max; // empty string returns 1 then
			}
		}
	}
}