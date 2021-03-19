#include "superthread.h"
#pragma once

namespace LSW {
	namespace v5 {
		namespace Tools {

			template<typename T>
			template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
			inline void SuperThread<T>::_set_promise_forced()
			{
				data->promise.set_value(T{});
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void SuperThread<T>::_set_promise_forced()
			{
				data->promise.set_value();
			}

			template<typename T>
			inline void SuperThread<T>::_perf()
			{
				switch (data->perform) {
				case superthread::performance_mode::_COUNT: // NOOOPE
					data->perform = superthread::performance_mode::PERFORMANCE;
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
				if (!arg)
					throw Handling::Abort(__FUNCSIG__, "Invalid thread argument internally!");

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
			inline SuperThread<T>::SuperThread(SuperThread&& b)
			{
				data = std::move(b.data);
				b.data = std::make_unique<_static_as_run>();
			}

			template<typename T>
			inline void SuperThread<T>::operator=(SuperThread&& b)
			{
				data = std::move(b.data);
				b.data = std::make_unique<_static_as_run>();
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
				data->promise = std::move(Promise<T>([&, dat = data.get(), f] {
					try {
						dat->_thread_done_flag = false;
						dat->_die_already = false;
						T cpy = f([&] { _perf(); return !dat->_die_already && !al_get_thread_should_stop(dat->thr); });
						dat->_thread_done_flag = true;
						return std::move(cpy);
					}
					catch (const Handling::Abort& e) { // for now. later: save and get elsewhere
						dat->had_abort = true;
						dat->latest_abort = e;
						std::cout << "Exception at SuperThread #" << Tools::get_thread_id() << ": " << e.what() << std::endl;
					}
					catch (const std::exception& e) { // for now. later: save and get elsewhere
						dat->had_abort = true;
						dat->latest_abort = Handling::Abort(__FUNCSIG__, e.what(), Handling::abort::abort_level::GIVEUP);
						std::cout << "Exception at SuperThread #" << Tools::get_thread_id() << ": " << e.what() << std::endl;
					}
					catch (...) { // for now. later: save and get elsewhere
						dat->had_abort = true;
						dat->latest_abort = Handling::Abort(__FUNCSIG__, "Unknown exception at SuperThread", Handling::abort::abort_level::GIVEUP);
						std::cout << "Unknown exception at SuperThread #" << Tools::get_thread_id() << "." << std::endl;
					}
					dat->_thread_done_flag = true;
					return T{};
				}));
			}

			template<typename T>
			template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
			inline void SuperThread<T>::set(const std::function<T(boolThreadF)> f)
			{
				join();
				data->promise = std::move(Promise<T>([&, dat = data.get(), f] {
					try {
						dat->_thread_done_flag = false;
						dat->_die_already = false;
						f([&] { _perf(); return !dat->_die_already && !al_get_thread_should_stop(dat->thr); });
						dat->_thread_done_flag = true;
					}
					catch (const Handling::Abort& e) { // for now. later: save and get elsewhere
						dat->had_abort = true;
						dat->latest_abort = e;
						std::cout << "Exception at SuperThread #" << Tools::get_thread_id() << ": " << e.what() << std::endl;
					}
					catch (const std::exception& e) { // for now. later: save and get elsewhere
						dat->had_abort = true;
						dat->latest_abort = Handling::Abort(__FUNCSIG__, e.what(), Handling::abort::abort_level::GIVEUP);
						std::cout << "Exception at SuperThread #" << Tools::get_thread_id() << ": " << e.what() << std::endl;
					}
					catch (...) { // for now. later: save and get elsewhere
						dat->had_abort = true;
						dat->latest_abort = Handling::Abort(__FUNCSIG__, "Unknown exception at SuperThread", Handling::abort::abort_level::GIVEUP);
						std::cout << "Unknown exception at SuperThread #" << Tools::get_thread_id() << "." << std::endl;
					}
					dat->_thread_done_flag = true;
				}));
			}

			template<typename T>
			inline Future<T> SuperThread<T>::start()
			{
				join();
				Future<T> fut = data->promise.get_future();
				data->thr = al_create_thread(__run_i_al, &data->promise);
				al_start_thread(data->thr);

				return fut;
			}

			template<typename T>
			inline void SuperThread<T>::set_performance_mode(const superthread::performance_mode& mode)
			{
				data->perform = mode;
			}

			template<typename T>
			inline void SuperThread<T>::stop()
			{
				data->_die_already = true;
				if (data->thr) al_set_thread_should_stop(data->thr);
			}

			template<typename T>
			inline void SuperThread<T>::join()
			{
				if (data->thr) {
					stop();
					al_join_thread(data->thr, nullptr);
					while (!data->_thread_done_flag) std::this_thread::sleep_for(std::chrono::milliseconds(20));
					al_destroy_thread(data->thr);
					if (!data->promise.has_set()) _set_promise_forced();
					data->thr = nullptr;
				}
			}

			template<typename T>
			inline void SuperThread<T>::kill()
			{
				if (data->thr) {
					stop();
					if (data->_thread_done_flag) {
						if (!data->promise.has_set()) _set_promise_forced();
						al_join_thread(data->thr, nullptr);
						al_destroy_thread(data->thr);
						data->thr = nullptr;
					}
					else { // might be internally blocked or the user is crazy and wants to kill it anyways.
						al_destroy_thread(data->thr);
						data->thr = nullptr;
						if (!data->promise.has_set()) _set_promise_forced();
						data->_thread_done_flag = true;
					}
				}
			}

			template<typename T>
			inline bool SuperThread<T>::running() const
			{
				return data->thr && !data->_die_already && !data->_thread_done_flag;
			}

			template<typename T>
			inline bool SuperThread<T>::had_abort() const
			{
				return data->had_abort;
			}

			template<typename T>
			inline Handling::Abort SuperThread<T>::get_abort() const
			{
				return data->latest_abort;
			}

		}
	}
}