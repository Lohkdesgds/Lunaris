#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline void SuperThread<T>::_set_promise_forced()
			{
				promise.set_value(T{});
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void SuperThread<T>::_set_promise_forced()
			{
				promise.set_value();
			}

			template<typename T>
			inline void SuperThread<T>::_perf()
			{
				switch (perform) {
				case superthread::performance_mode::_COUNT: // NOOOPE
					perform = superthread::performance_mode::PERFORMANCE;
					std::this_thread::yield();
					break;

				case superthread::performance_mode::NO_CONTROL: // UNLEASHED!
					break;

				case superthread::performance_mode::HIGH_PERFORMANCE: // yield!
					std::this_thread::yield(); // sync
					break;

				case superthread::performance_mode::PERFORMANCE:
					//for(short u = 0; u < 4; u++) std::this_thread::yield(); // slow down
					sleep_for(std::chrono::microseconds(250));
					break;

				case superthread::performance_mode::BALANCED:
					//std::this_thread::yield();
					sleep_for(std::chrono::microseconds(500)); // top 1000 loops
					break;

				case superthread::performance_mode::LOW_POWER:
					//std::this_thread::yield();
					sleep_for(std::chrono::milliseconds(3)); // top 200 loops
					break;

				case superthread::performance_mode::VERY_LOW_POWER:
					//std::this_thread::yield();
					sleep_for(std::chrono::milliseconds(8)); // top 67 loops
					break;

				case superthread::performance_mode::EXTREMELY_LOW_POWER:
					//std::this_thread::yield();
					sleep_for(std::chrono::milliseconds(25)); // top 20 loops
					break;
				}
			}

			template<typename T>
			inline void* SuperThread<T>::__run_i_al(ALLEGRO_THREAD* thr, void* arg)
			{
				if (!arg) throw Handling::Abort(__FUNCSIG__, "Invalid thread argument internally!");
				Promise<T>* f = (Promise<T>*)arg;
				f->work(); // has !al_get_thread_should_stop(thr) internally
				return thr;
			}

			template<typename T>
			inline SuperThread<T>::SuperThread()
			{
				Handling::init_basic();
			}

			template<typename T>
			inline SuperThread<T>::SuperThread(const superthread::performance_mode& m)
			{
				Handling::init_basic();
				set_performance_mode(m);
			}

			template<typename T>
			inline SuperThread<T>::SuperThread(const std::function<T(boolThreadF)> f)
			{
				Handling::init_basic();
				set(f);
			}

			template<typename T>
			inline SuperThread<T>::~SuperThread()
			{
				kill();
			}

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline void SuperThread<T>::set(const std::function<T(boolThreadF)> f)
			{
				join();
				promise = std::move(Promise<T>([&, f] {
					try {
						_thread_done_flag = false;
						_die_already = false;
						T cpy = f([&] { _perf(); return !_die_already && !al_get_thread_should_stop(thr); });
						_thread_done_flag = true;
						return std::move(cpy);
					}
					catch (const Handling::Abort& e) { // for now. later: save and get elsewhere
						std::cout << "Exception at SuperThread #" << Tools::get_thread_id() << ": " << e.what() << std::endl;
					}
					catch (...) { // for now. later: save and get elsewhere
						std::cout << "Unknown exception at SuperThread #" << Tools::get_thread_id() << "." << std::endl;
					}
					_thread_done_flag = true;
					return T{};
					}));
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void SuperThread<T>::set(const std::function<T(boolThreadF)> f)
			{
				join();
				promise = std::move(Promise<T>([&, f] {
					try {
						_thread_done_flag = false;
						_die_already = false;
						f([&] { _perf(); return !_die_already && !al_get_thread_should_stop(thr); });
						_thread_done_flag = true;
					}
					catch (const Handling::Abort& e) { // for now. later: save and get elsewhere
						std::cout << "Exception at SuperThread #" << Tools::get_thread_id() << ": " << e.what() << std::endl;
					}
					catch (...) { // for now. later: save and get elsewhere
						std::cout << "Unknown exception at SuperThread #" << Tools::get_thread_id() << "." << std::endl;
					}
					_thread_done_flag = true;
					}));
			}

			template<typename T>
			inline Future<T> SuperThread<T>::start()
			{
				join();
				Future<T> fut = promise.get_future();
				thr = al_create_thread(__run_i_al, &promise);
				al_start_thread(thr);

				return fut;
			}

			template<typename T>
			inline void SuperThread<T>::set_performance_mode(const superthread::performance_mode& mode)
			{
				perform = mode;
			}

			template<typename T>
			inline void SuperThread<T>::stop()
			{
				_die_already = true;
				if (thr) al_set_thread_should_stop(thr);
			}

			template<typename T>
			inline void SuperThread<T>::join()
			{
				if (thr) {
					stop();
					al_join_thread(thr, nullptr);
					while (!_thread_done_flag) std::this_thread::sleep_for(std::chrono::milliseconds(20));
					al_destroy_thread(thr);
					if (!promise.has_set()) _set_promise_forced();
					thr = nullptr;
				}
			}

			template<typename T>
			inline void SuperThread<T>::kill()
			{
				if (thr) {
					stop();
					if (_thread_done_flag) {
						if (!promise.has_set()) _set_promise_forced();
						al_join_thread(thr, nullptr);
						al_destroy_thread(thr);
						thr = nullptr;
					}
					else { // might be internally blocked or the user is crazy and wants to kill it anyways.
						al_destroy_thread(thr);
						thr = nullptr;
						if (!promise.has_set()) _set_promise_forced();
						_thread_done_flag = true;
					}
				}
			}

			template<typename T>
			inline bool SuperThread<T>::running() const
			{
				return thr && !_die_already && !_thread_done_flag;
			}

		}
	}
}