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

}