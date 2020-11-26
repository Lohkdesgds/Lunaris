#pragma once


// C++
#include <functional>

#include "../../Handling/Abort/abort.h"
#include "../../Handling/Initialize/initialize.h"
#include "../Future/future.h"
#include "../Common/common.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			namespace superthread {

				enum class performance_mode {
					BALANCED, // up to 2000 events / sec
					LOW_POWER, // up to 500 events / sec
					VERY_LOW_POWER, // up to 125 events / sec
					PERFORMANCE, // up to 10000 events / sec
					HIGH_PERFORMANCE // undefined max number of events
				};

			}

			// function that returns TRUE if you should KEEP RUNNING.
			using boolThreadF = std::function<bool(void)>;

			/// <summary>
			/// <para>A thread class with many tools for good performance and control.</para>
			/// </summary>
			template<typename T = void>
			class SuperThread {
				ALLEGRO_THREAD* thr = nullptr;
				bool _thread_done_flag = true;
				bool _die_already = false;
				Promise<T> promise;
				superthread::performance_mode perform{}; // balanced

				void _perf();

				static void* __run_i_al(ALLEGRO_THREAD* thr, void* arg);

				// if you have to set after kill, easy way
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				void _set_promise_forced();

				// if you have to set after kill, easy way
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				void _set_promise_forced();
			public:
				/// <summary>
				/// <para>Initialize necessary stuff.</para>
				/// </summary>
				SuperThread();

				SuperThread(const SuperThread&) = delete;
				SuperThread(SuperThread&&) = delete;

				/// <summary>
				/// <para>Constructor that sets directly the performance mode.</para>
				/// </summary>
				/// <param name="{performance_mode}">How fast should this run?</param>
				SuperThread(const superthread::performance_mode&);

				/// <summary>
				/// <para>Constructor that sets directly the function.</para>
				/// </summary>
				/// <param name="{std::function}">The function loop based on bool function argument.</param>
				SuperThread(const std::function<T(boolThreadF)>);
				~SuperThread();


				/// <summary>
				/// <para>Sets the function that this thread will run (loop or not).</para>
				/// </summary>
				/// <param name="{std::function}">The function loop based on bool function argument.</param>
				template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
				void set(const std::function<T(boolThreadF)>);

				/// <summary>
				/// <para>Sets the function that this thread will run (loop or not).</para>
				/// </summary>
				/// <param name="{std::function}">The function loop based on bool function argument.</param>
				template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
				void set(const std::function<T(boolThreadF)>);

				/// <summary>
				/// <para>Starts the thread and gives you a Future if you need the response later of type T.</para>
				/// </summary>
				/// <returns>{Future} The Future that will hold the value the function you've set returns.</returns>
				Future<T> start();

				/// <summary>
				/// <para>Multitasking sometimes doesn't work without some adjustments. What performance mode you want?</para>
				/// </summary>
				/// <param name="{performance_mode}">How hard should it hit the CPU?</param>
				void set_performance_mode(const superthread::performance_mode&);

				/// <summary>
				/// <para>Sets internal bool to say stop.</para>
				/// </summary>
				void stop();

				/// <summary>
				/// <para>Tries to join the thread (if possible) (can lock).</para>
				/// <para>PS: It will set to stop before actually trying to join.</para>
				/// </summary>
				void join();

				/// <summary>
				/// <para>Kills the thread via terminate.</para>
				/// </summary>
				void kill();

				/// <summary>
				/// <para>Returns true if there's a thread running right now.</para>
				/// </summary>
				/// <returns>{bool} Running?</returns>
				bool running() const;
			};

		}
	}
}

#ifndef LSW_AVOID_NON_LIB
#include "superthread.ipp"
#endif