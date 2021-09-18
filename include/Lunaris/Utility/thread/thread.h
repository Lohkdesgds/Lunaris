#pragma once

#include <thread>
#include <functional>
#include <memory>

namespace Lunaris {

	class thread {
	public:
		enum class speed {
			UNLEASHED,				// no control
			HIGH_PERFORMANCE,		// yield() each loop for sync (may affect performance a little bit, but very responsive)
			INTERVAL,				// follow a fixed time interval
			ONCE					// only once
		};
	private:
		struct _data {
			std::thread thr;
			std::function<void(void)> func;
			std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> _wait_until; // used so the up to is actually near the expected value
			double interval_seconds = 1.0 / 60;
			speed mode = speed::HIGH_PERFORMANCE;
			bool should_quit = false;
			bool _ended_gracefully = false;
			std::exception_ptr _exception; // rethrows on join() if any

			bool _run_ctrl(); // called by _thr_work()'s loop
			void _thr_work();
		};

		std::shared_ptr<_data> data = std::make_shared<_data>();
	public:
		thread() = default;

		// same as task_async
		thread(std::function<void(void)>, const speed = speed::HIGH_PERFORMANCE, const double = 1.0 / 60);
		~thread();

		thread(const thread&) = delete;
		void operator=(const thread&) = delete;

		thread(thread&&) noexcept;
		void operator=(thread&&) noexcept;

		// keep running undefinitely (double is only used if speed is set to interval)
		void task_async(std::function<void(void)>, const speed = speed::HIGH_PERFORMANCE, const double = 1.0 / 60);

		void signal_stop();

		// stop and join threads (bool: skip exception rethrow if any?)
		void join(const bool = false);

		// caution: may cause memory leak! Use at your own risk!
		void force_kill();
	};

}

#include "thread.ipp"