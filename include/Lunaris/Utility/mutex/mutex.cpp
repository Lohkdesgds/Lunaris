#include "mutex.h"

namespace Lunaris {

	LUNARIS_DECL void shared_recursive_mutex::lock()
	{
		const auto this_id = std::this_thread::get_id();
		if (_owner == this_id) {
			++_count;
		}
		else {
			this->std::shared_mutex::lock();
			_owner = this_id;
			_count = 1;
		}
	}

	LUNARIS_DECL bool shared_recursive_mutex::try_lock()
	{
		const auto this_id = std::this_thread::get_id();
		const bool got = this->std::shared_mutex::try_lock();
		if (got) {
			_owner = this_id;
			_count = 1;
		}
		return got;
	}

	LUNARIS_DECL void shared_recursive_mutex::unlock()
	{
		const auto this_id = std::this_thread::get_id();
		if (_count > 0 && _owner == this_id) {
			if (--_count == 0) {
				_owner = std::thread::id();
				this->std::shared_mutex::unlock();
			}
		}
		else { // count 0 or not owner
			throw std::runtime_error("shared_recursive_mutex unlock on non-owned or already unlocked mutex!");
		}
	}

	LUNARIS_DECL bool fast_one_way_mutex::run()
	{
		return slave_ack = !request_stop;
	}

	LUNARIS_DECL void fast_one_way_mutex::lock()
	{
		request_stop = true;
		while (slave_ack) std::this_thread::sleep_for(std::chrono::milliseconds(10)); // fastest way is power hungry
	}

	LUNARIS_DECL void fast_one_way_mutex::unlock()
	{
		request_stop = false;
	}

	LUNARIS_DECL fast_lock_guard::fast_lock_guard(fast_one_way_mutex& r)
		: ref(r)
	{
		ref.lock();
	}

	LUNARIS_DECL fast_lock_guard::~fast_lock_guard()
	{
		ref.unlock();
	}

}