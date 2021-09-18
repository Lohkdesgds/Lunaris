#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>

#include "../../Handling/Abort/abort.h"

namespace LSW {
	namespace v5 {
		namespace Tools {

			class __anyMutex {
			public:
				virtual void lock() = 0;
				virtual bool try_lock() = 0;
				virtual void unlock() = 0;
				virtual bool is_locked() const = 0;
			};

			/// <summary>
			/// <para>Do you need absurd performance? This might be the fastest mutex there.</para>
			/// <para>This will probably perform better than MOST mutexes out there.</para>
			/// <para>Only downside: it may use some CPU while locked.</para>
			/// </summary>
			class SuperMutex : public __anyMutex {
				std::atomic<char> m2;
				std::thread::id who{};
			public:
				/// <summary>
				/// <para>Tries to lock and will wait for it to get unlocked so it can lock.</para>
				/// </summary>
				void lock();
				/// <summary>
				/// <para>Tries to lock and will wait for it to get unlocked so it can lock.</para>
				/// </summary>
				bool try_lock();

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
			/// <para>SuperSemaphore is a smart std::mutex that uses conditional to work a little better and in sequence, as a semaphore.</para>
			/// <para>If multiple threads try to lock at the same time, it shall let the first one lock first, than the second, then the third. It is not 100% perfect because conditional may not trigger if a thread tries exactly at the same time the variable is unlocked, so there is a timeout so then it retries over and over.</para>
			/// </summary>
			class SuperSemaphore : public __anyMutex {
				SuperMutex mu;
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

				/// <summary>
				/// <para>Tries to lock. True if success.</para>
				/// </summary>
				/// <returns>{bool} True if locked.</returns>
				bool try_lock();
			};

			/// <summary>
			/// <para>AutoLock is like a std::unique_lock, but to be used with mutexes from this library.</para>
			/// <para>You can lock and unlock as you please.</para>
			/// </summary>
			class AutoLock {
				__anyMutex& you;
			public:
				/// <summary>
				/// <para>Automatically links a Mutex and locks (or not, based on bool).</para>
				/// </summary>
				/// <param name="{SuperMutex}">The SuperMutex to link.</param>
				/// <param name="{bool}">Lock directly?</param>
				AutoLock(__anyMutex&, const bool = true);
				~AutoLock();

				/// <summary>
				/// <para>Unlocks the linked Mutex.</para>
				/// <para>If already unlocked (by you), it skips.</para>
				/// </summary>
				void unlock();

				/// <summary>
				/// <para>Locks the linked Mutex.</para>
				/// <para>If already locked (by you), it skips (PS: one unlock() is needed no matter what)</para>
				/// </summary>
				void lock();

				/// <summary>
				/// <para>Tries to lock. True if success.</para>
				/// </summary>
				/// <returns>{bool} True if locked.</returns>
				bool try_lock();
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