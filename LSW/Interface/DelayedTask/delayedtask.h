#pragma once

#include "../../Handling/Abort/abort.h"
#include "../../Tools/Future/future.h"
#include "../../Tools/SuperThread/superthread.h"
#include "../../Tools/SuperMutex/supermutex.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace delayedtask {

				enum class delayed_type {SPAWN_SOLO, QUEUE};
				constexpr size_t maximum_aborts_saved = 100;

			}

			/// <summary>
			/// <para>DelayedTask is a delayed tasking manager. It can delay tasks in order or by themselves (spawning threads).</para>
			/// </summary>
			class DelayedTask {
				struct _task {
					std::chrono::system_clock::time_point when;
					Tools::Promise<void> what;

					_task(const std::function<void(void)>&);
				};

				Tools::SuperSemaphore mu;
				std::vector<Tools::SuperThread<>> solo_delayed; // multithread queue
				Tools::SuperThread<> single_queue{ Tools::superthread::performance_mode::VERY_LOW_POWER };
				std::vector<_task> single_queue_queue;

				std::vector<Handling::Abort> aborts;
				Tools::SuperMutex abort_mu;

				void add_abort(const Handling::Abort&);

				void run_solo_autoabort(const std::function<void(void)>&);

				void queue_task(Tools::boolThreadF);
			public:
				DelayedTask();
				~DelayedTask();

				/// <summary>
				/// <para>Push back a new task to be done in the future.</para>
				/// </summary>
				/// <param name=""></param>
				/// <param name=""></param>
				/// <param name=""></param>
				/// <returns>{bool} True if had a failure.</returns>
				Tools::Future<void> push_back(const std::function<void(void)>, const std::chrono::milliseconds, const delayedtask::delayed_type = delayedtask::delayed_type::QUEUE);

				/// <summary>
				/// <para>Did a task failed?</para>
				/// </summary>
				/// <returns>{bool} True if had a failure.</returns>
				bool had_abort() const;

				/// <summary>
				/// <para>Get last Abort (if any). If none, returns empty error.</para>
				/// </summary>
				/// <returns>{Handling::Abort} Abort information.</returns>
				Handling::Abort pop_last_abort();

				/// <summary>
				/// <para>Sum of all tasks to do (solo and queued).</para>
				/// </summary>
				/// <returns>{size_t} Sum of all tasks.</returns>
				size_t size() const;

				/// <summary>
				/// <para>Number of threads (solo tasks) in queue.</para>
				/// </summary>
				/// <returns>{size_t} Number of queued thread tasks.</returns>
				size_t threads_queued() const;
			};
		}
	}
}