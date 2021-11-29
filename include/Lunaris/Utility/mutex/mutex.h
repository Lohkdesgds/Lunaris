#pragma once

#include <Lunaris/__macro/macros.h>

#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <thread>
#include <stdexcept>

namespace Lunaris {

	/// <summary>
	/// <para>Shared_recursive_mutex is a shared_mutex with the recursive feature when locked non-shared mode.</para>
	/// </summary>
	class shared_recursive_mutex : public std::shared_mutex {
		std::atomic<std::thread::id> _owner;
		std::atomic<unsigned> _count = 0;
	public:
		/// <summary>
		/// <para>Locks for write (Blocks reads and writes from other threads).</para>
		/// </summary>
		void lock();

		/// <summary>
		/// <para>Tries to lock for write (if successful, blocks read and writes from other threads).</para>
		/// </summary>
		/// <returns>{bool} Successfully locked?</returns>
		bool try_lock();

		/// <summary>
		/// <para>Unlocks write (allows other thread to lock or any read (shared) thread to read).</para>
		/// </summary>
		void unlock();

		using std::shared_mutex::lock_shared;
		using std::shared_mutex::try_lock_shared;
		using std::shared_mutex::unlock_shared;
	};

	/// <summary>
	/// <para>The reason this exists is because sometimes you have a speedy thread that rarely needs to stop so another thread can change its data.</para>
	/// <para>Default mutex is obviously slower than a literal copy to a bool and return.</para>
	/// <para>Using two bools one can be the last "fast thread" ack and the other the expected wait/continue.</para>
	/// <para>You are supposed to check run() return. If true, you can go, else hold somehow (do not read or write whatever data you have).</para>
	/// <para>The external thread should call lock() and unlock() like a mutex. The difference is that if run() is not being called, lock() will hold FOREVER.</para>
	/// <para>As generally external threads are not the performance ones, the waiting is just a while() with sleep for 10 ms.</para>
	/// </summary>
	class fast_one_way_mutex : public NonCopyable, public NonMovable {
		bool slave_ack = false;
		bool request_stop = false;
	public:
		/// <summary>
		/// <para>Call this on the performance thread. If returns TRUE, you can do whatever you'd like (safe).</para>
		/// <para>If false, wait somehow (testing the value in a reasonable time).</para>
		/// </summary>
		/// <returns>{bool} TRUE if you're good to go this time.</returns>
		bool run();

		/// <summary>
		/// <para>This is called by an external non-performance thread. Lock the resources and wait for the performance thread to "ack" automatically with this.</para>
		/// <para>There's not much going on. It should feel like std::mutex's lock().</para>
		/// </summary>
		void lock();

		/// <summary>
		/// <para>As for every lock() there's an unlock. This does NOT wait for ack, because it doesn't matter.</para>
		/// <para>You're good to go after this.</para>
		/// </summary>
		void unlock();
	};

	/// <summary>
	/// <para>Very useful lock() and unlock() done automatically for you.</para>
	/// <para>This MUST NOT be called on the performance thread. This is for the external thread changing something in a random time once only!</para>
	/// <para>The main performance thread (loop) should keep an eye on run()</para>
	/// </summary>
	class fast_lock_guard : public NonCopyable, public NonMovable {
		fast_one_way_mutex& ref;
	public:
		fast_lock_guard(fast_one_way_mutex&);
		~fast_lock_guard();
	};

}
