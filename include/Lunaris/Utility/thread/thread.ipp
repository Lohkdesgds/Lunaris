#pragma once

namespace Lunaris {

	inline bool thread::_data::_run_ctrl()
	{
		if (should_quit) return false; // quit fast

		switch (mode) {
		case speed::UNLEASHED:			// no control
			break;
		case speed::HIGH_PERFORMANCE:	// sync threads
			std::this_thread::yield();
			break;
		case speed::INTERVAL:			// custom delta
		{
			std::this_thread::sleep_until(_wait_until);
			_wait_until = std::chrono::high_resolution_clock::now() + std::chrono::duration<double, std::ratio<1>>(interval_seconds);
		}
			break;
		case speed::ONCE:
			if (!should_quit) {
				should_quit = true;
				return true;
			}
			break;
		}

		return !should_quit;
	}

	inline void thread::_data::_thr_work()
	{
		_ended_gracefully = false;

		while (_run_ctrl()) {
			try {
				func();
			}
			catch (...) {
				_exception = std::current_exception();
				break;
			}
		}

		_ended_gracefully = true;
	}

	inline thread::thread(std::function<void(void)> fun, const speed mode, const double interv)
	{
		task_async(fun, mode, interv);
	}

	inline thread::~thread()
	{
		join(true);
	}

	inline thread::thread(thread&& oth) noexcept
		: data(std::move(oth.data))
	{
	}

	inline void thread::operator=(thread&& oth) noexcept
	{
		join();
		data = std::move(oth.data);
	}

	// keep running undefinitely
	inline void thread::task_async(std::function<void(void)> fun, const speed mode, const double interv)
	{
		if (!fun) throw std::runtime_error("Invalid function for thread!");
		if (interv > 86400) throw std::runtime_error("Invalid time interval! (More than 1 day? Seriously?)");
		join();
		data->should_quit = false;
		data->func = fun;
		data->mode = mode;
		data->interval_seconds = interv;
		data->thr = std::thread([piece = this->data]{ piece->_thr_work(); });
	}

	inline void thread::signal_stop()
	{
		data->should_quit = true;
	}

	inline void thread::join(const bool skip_any_exception)
	{
		data->should_quit = true;
		if (data->thr.joinable()) {
			data->thr.join();
			if (!skip_any_exception && data->_exception) std::rethrow_exception(data->_exception);
		}
	}

	inline void thread::force_kill()
	{
		if (data->thr.joinable()) {
			data->should_quit = true;

			for (size_t p = 0; p < 10 && !data->_ended_gracefully; p++) std::this_thread::sleep_for(std::chrono::milliseconds(25));

			if (data->_ended_gracefully) {
				data->thr.join();
			}
			else { // it has to die :/
				auto handl = data->thr.native_handle();
				data->thr.detach();
				::TerminateThread(handl, 0); // goodbye my friend.
			}

			if (data->_exception) std::rethrow_exception(data->_exception);
		}
	}

}