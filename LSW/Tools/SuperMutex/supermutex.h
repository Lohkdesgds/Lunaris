#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace LSW {
	namespace v5 {
		namespace Tools {

			/// <summary>
			/// <para>SuperMutex is a smart std::mutex that uses conditional to work a little better and in sequence, as a semaphore.</para>
			/// <para>If multiple threads try to lock at the same time, it shall let the first one lock first, than the second, then the third. It is not 100% perfect because conditional may not trigger if a thread tries exactly at the same time the variable is unlocked, so there is a timeout so then it retries over and over.</para>
			/// </summary>
			class SuperMutex {
				class weird_mtx {
					std::mutex m;
					bool locked = false;
				public:
					bool try_lock();
					void unlock();
					bool is_locked() const;
				} mu;
				std::condition_variable cond;
			public:

				/// <summary>
				/// <para>Tries to lock and will wait for it to get unlocked so it can lock.</para>
				/// </summary>
				void lock();

				/// <summary>
				/// <para>Unlocks and (if there's someone) notifies next that they can lock.</para>
				/// </summary>
				void unlock();

				/// <summary>
				/// <para>Is this locked?</para>
				/// </summary>
				/// <returns>{bool} True if someone already locked.</returns>
				bool is_locked() const;
			};

			/// <summary>
			/// <para>AutoLock is like a std::unique_lock, but to be used with SuperMutex.</para>
			/// <para>You can lock and unlock as you please.</para>
			/// </summary>
			class AutoLock {
				SuperMutex& you;
				bool hasunlocked = false;
			public:
				/// <summary>
				/// <para>Automatically links a SuperMutex and locks (or not, based on bool).</para>
				/// </summary>
				/// <param name="{SuperMutex}">The SuperMutex to link.</param>
				/// <param name="{bool}">Lock directly?</param>
				AutoLock(SuperMutex&, const bool = true);
				~AutoLock();

				/// <summary>
				/// <para>Unlocks the linked SuperMutex.</para>
				/// <para>If already unlocked (by you), it skips.</para>
				/// </summary>
				void unlock();

				/// <summary>
				/// <para>Locks the linked SuperMutex.</para>
				/// <para>If already locked (by you), it skips (PS: one unlock() is needed no matter what)</para>
				/// </summary>
				void lock();
			};

			/// <summary>
			/// <para>This is a conditional implementation of something like signals.</para>
			/// <para>You can wait_signal() in multiple threads and start each one with signal_one() or start them with signal_all() all at the same time.</para>
			/// </summary>
			class Waiter {
				std::condition_variable cond;
			public:

				/// <summary>
				/// <para>Waits for a signal.</para>
				/// </summary>
				/// <param name="{size_t}">Timeout in milliseconds, 0 to infinite.</param>
				/// <returns></returns>
				void wait_signal(const size_t = 0);

				/// <summary>
				/// <para>Signal to one waiting thread that it can continue.</para>
				/// </summary>
				void signal_one();

				/// <summary>
				/// <para>Signal to all threads waiting that they can continue.</para>
				/// </summary>
				void signal_all();
			};

		}
	}
}