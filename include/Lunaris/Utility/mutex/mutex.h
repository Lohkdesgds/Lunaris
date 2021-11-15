#pragma once

#include <Lunaris/__macro/macros.h>

#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <thread>
#include <stdexcept>

namespace Lunaris {

	class shared_recursive_mutex : public std::shared_mutex {
		std::atomic<std::thread::id> _owner;
		std::atomic<unsigned> _count = 0;
	public:
		void lock();
		bool try_lock();
		void unlock();

		using std::shared_mutex::lock_shared;
		using std::shared_mutex::try_lock_shared;
		using std::shared_mutex::unlock_shared;
	};

}
