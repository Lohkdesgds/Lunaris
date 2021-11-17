#include "thread.h"
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
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Successfully launched thread block %p", this);
#endif

		while (_run_ctrl()) {
			try {
				func();
			}
			catch (const std::exception& e) {
#ifdef LUNARIS_VERBOSE_BUILD
				PRINT_DEBUG("Exception at thread block %p: %s", this, e.what());
#endif
				if (exp_hdlr) exp_hdlr(e);
				else {
					_exception = std::current_exception();
				}
			}
			catch (...) {
#ifdef LUNARIS_VERBOSE_BUILD
				PRINT_DEBUG("Exception at thread block %p: UNCAUGHT", this);
#endif
				if (exp_hdlr) exp_hdlr(std::runtime_error("UNCAUGHT"));
				else {
					_exception = std::current_exception();
				}
			}
		}

#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Successfully closed thread block %p", this);
#endif
		_ended_gracefully = true;
	}

	inline thread::thread(std::function<void(void)> fun, const speed mode, const double interv, std::function<void(const std::exception&)> excpt)
	{
		task_async(fun, mode, interv, excpt);
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
	inline void thread::task_async(std::function<void(void)> fun, const speed mode, const double interv, std::function<void(const std::exception&)> excpt)
	{
		if (!fun) throw std::runtime_error("Invalid function for thread!");
		if (interv > 86400) throw std::runtime_error("Invalid time interval! (More than 1 day? Seriously?)");
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Syncing thread block %p", data.get());
#endif
		join();
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Building thread block %p", data.get());
#endif
		data->should_quit = false;
		data->func = fun;
		data->mode = mode;
		data->interval_seconds = interv;
		data->exp_hdlr = excpt;
		data->thr = std::thread([piece = this->data]{ piece->_thr_work(); });
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Built thread block %p", data.get());
#endif
	}

	inline void thread::set_speed(const speed mode, const double interv)
	{
		data->mode = mode;
		data->interval_seconds = interv;
	}

	inline void thread::signal_stop()
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Signal stop thread block %p", data.get());
#endif
		data->should_quit = true;
	}

	inline void thread::join(const bool skip_any_exception)
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Joining thread block %p", data.get());
#endif
		data->should_quit = true;
		while (!data->_ended_gracefully) std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (data->thr.joinable()) data->thr.join();
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Joined thread block %p", data.get());
#endif
		if (!skip_any_exception && data->_exception) std::rethrow_exception(data->_exception);
	}

	inline void thread::force_kill(const bool skip_any_exception)
	{
#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Forcing kill thread block %p", data.get());
#endif
		if (data->thr.joinable()) {
			data->should_quit = true;

			for (size_t p = 0; p < 10 && !data->_ended_gracefully; p++) std::this_thread::sleep_for(std::chrono::milliseconds(25));

			if (data->_ended_gracefully) {
#ifdef LUNARIS_VERBOSE_BUILD
				PRINT_DEBUG("Force kill not needed on thread block %p, joining", data.get());
#endif
				data->thr.join();
#ifdef LUNARIS_VERBOSE_BUILD
				PRINT_DEBUG("Joined thread block %p", data.get());
#endif
			}
			else { // it has to die :/
				auto handl = data->thr.native_handle();
				data->thr.detach();
				::TerminateThread(handl, 0); // goodbye my friend.
#ifdef LUNARIS_VERBOSE_BUILD
				PRINT_DEBUG("Killed thread block %p", data.get());
#endif
			}
		}
		if (!skip_any_exception && data->_exception) std::rethrow_exception(data->_exception);
	}

	inline bool async_thread_info::has_ended() const
	{
		return ended.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}

	inline void async_thread_info::force_destroy()
	{
		if (__destroyed) return;
		__destroyed = true;
		::TerminateThread(id, 0); // goodbye my friend.
	}

	inline bool async_thread_info::exists() const
	{
		return id != std::thread::native_handle_type{};
	}

	inline async_thread_info throw_thread(const std::function<void(void)> f)
	{
		if (!f) return {};
		std::promise<bool> _prom;
		async_thread_info _mak;
		_mak.ended = _prom.get_future();
		std::thread _launch = std::thread(
			[pm = std::move(_prom), f]() mutable {
				try {
#ifdef LUNARIS_VERBOSE_BUILD
					PRINT_DEBUG("This thread is throw type");
#endif
					f();
#ifdef LUNARIS_VERBOSE_BUILD
					PRINT_DEBUG("This throw thread type ended tasks"); 
#endif
				} 
#ifdef LUNARIS_VERBOSE_BUILD
				catch (const std::exception& e) {
					PRINT_DEBUG("This throw thread type got exception: %s", e.what()); 
					pm.set_value(false);
					return;
				}
#endif
				catch (...) {
#ifdef LUNARIS_VERBOSE_BUILD
					PRINT_DEBUG("This throw thread type got exception"); 
#endif
					pm.set_value(false);
					return;
				}
				pm.set_value(true); 
			}
		);
		_mak.id = _launch.native_handle();
		_launch.detach();
		return _mak;
	}

}