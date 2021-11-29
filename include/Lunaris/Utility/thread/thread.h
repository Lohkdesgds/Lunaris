#pragma once

#include <Lunaris/__macro/macros.h>

#include <Windows.h>
#include <thread>
#include <functional>
#include <memory>
#include <future>

namespace Lunaris {

	/// <summary>
	/// <para>thread is a std::thread with loop, timer and kill support. It's not perfect, but it works.</para>
	/// <para>You can create one with a timed interval, performance mode, unleashed or run only once.</para>
	/// <para>All others but once do call your function while valid. It's done automatically.</para>
	/// </summary>
	class thread : public NonCopyable {
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
			bool _ended_gracefully = true;
			std::exception_ptr _exception; // rethrows on join() if any
			std::function<void(const std::exception&)> exp_hdlr;

			bool _run_ctrl(); // called by _thr_work()'s loop
			void _thr_work();
		};

		std::shared_ptr<_data> data = std::make_shared<_data>();
	public:
		thread() = default;

		/// <summary>
		/// <para>Create a thread directly from the constructor.</para>
		/// </summary>
		/// <param name="{function}">The looping function (or one run if speed is once).</param>
		/// <param name="{speed}">Thread speed mode.</param>
		/// <param name="{double}">Delta, in seconds, if the speed is the timed one (interval).</param>
		/// <param name="{function}">Exception handler (automatically handle exceptions).</param>
		thread(std::function<void(void)>, const speed = speed::HIGH_PERFORMANCE, const double = 1.0 / 60, std::function<void(const std::exception&)> = {});
		
		~thread();

		/// <summary>
		/// <para>Move the thread information around.</para>
		/// </summary>
		/// <param name="{thread}">Moving thread.</param>
		thread(thread&&) noexcept;

		/// <summary>
		/// <para>Move the thread information around.</para>
		/// </summary>
		/// <param name="{thread}">Moving thread.</param>
		void operator=(thread&&) noexcept;

		/// <summary>
		/// <para>Create a thread with these settings.</para>
		/// </summary>
		/// <param name="{function}">The looping function (or one run if speed is once).</param>
		/// <param name="{speed}">Thread speed mode.</param>
		/// <param name="{double}">Delta, in seconds, if the speed is the timed one (interval).</param>
		/// <param name="{function}">Exception handler (automatically handle exceptions).</param>
		void task_async(std::function<void(void)>, const speed = speed::HIGH_PERFORMANCE, const double = 1.0 / 60, std::function<void(const std::exception&)> = {});

		/// <summary>
		/// <para>Change speed mode and time on the fly.</para>
		/// </summary>
		/// <param name="{speed}">New thread speed mode.</param>
		/// <param name="{double}">New delta, if speed is the timed one (interval).</param>
		void set_speed(const speed, const double = 1.0 / 60);

		/// <summary>
		/// <para>Set to next cycle tell thread to stop.</para>
		/// </summary>
		void signal_stop();

		/// <summary>
		/// <para>Tells thread to stop and wait for join.</para>
		/// </summary>
		/// <param name="{bool}">Skip saved exceptions not handled? (if you had an exception handler, this shoudn't throw any exception anyway).</param>
		void join(const bool = false);

		/// <summary>
		/// <para>Force kill the thread. It'll end abruptly.</para>
		/// <para>WARNING: This may cause memory leak! Some say it does, VS does not like what I did here, so use this only if you really need to!</para>
		/// </summary>
		/// <param name="{bool}">Skip saved exceptions not handled? (if you had an exception handler, this shoudn't throw any exception anyway).</param>
		void force_kill(const bool = false);

		/// <summary>
		/// <para>If this is still a valid running thread.</para>
		/// </summary>
		/// <returns>{bool} True if there's something running.</returns>
		bool valid() const;

		/// <summary>
		/// <para>If this has no things running.</para>
		/// </summary>
		/// <returns>{bool} True if there's no thread running (there shouldn't be).</returns>
		bool empty() const;
	};

	struct async_thread_info {
	private:
		bool __destroyed = false;
	public:
		std::thread::native_handle_type id = {};
		std::future<bool> ended;

		/// <summary>
		/// <para>Get if the thread has ended its job (from std::future status)</para>
		/// </summary>
		/// <returns></returns>
		bool has_ended() const;

		/// <summary>
		/// <para>Force thread to stop abruptly. This may cause memory leak, so this is last resort!</para>
		/// </summary>
		void force_destroy();

		/// <summary>
		/// <para>If there's any thread there.</para>
		/// </summary>
		/// <returns>{bool} True if this is still related to a valid thread.</returns>
		bool exists() const;

		/// <summary>
		/// <para>If there's any thread there.</para>
		/// </summary>
		/// <returns>{bool} True if this is still related to a valid thread.</returns>
		bool valid() const;

		/// <summary>
		/// <para>Is this still something?</para>
		/// </summary>
		/// <returns>{bool} True if there's nothing to do anymore, this is empty or something.</returns>
		bool empty() const;
	};

	/// <summary>
	/// <para>Launch pure async thread! If you don't save the return value, you'll have a zombie thread.</para>
	/// </summary>
	/// <param name="{function}">Function to launch.</param>
	/// <returns>{async_thread_info} The information needed.</returns>
	async_thread_info throw_thread(const std::function<void(void)>);


}

#include "thread.ipp"