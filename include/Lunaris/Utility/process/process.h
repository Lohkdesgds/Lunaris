#pragma once

#include <Lunaris/__macro/macros.h>

#include <Windows.h>
#include <thread>
#include <mutex>
#include <string>
#include <functional>

namespace Lunaris {

	/// <summary>
	/// <para>process handles a process for you in parallel. You set a function to get STDOUT and you can monitor it!</para>
	/// </summary>
	class process {
	public:
		enum class message_type {START /*Started process signal*/, APP_OUTPUT /*Each message*/, ENDED /*Ended process signal*/ };
	private:
		HANDLE g_hChildStd_OUT_Rd = nullptr;
		HANDLE g_hChildStd_OUT_Wr = nullptr;
		SECURITY_ATTRIBUTES saAttr{};
		PROCESS_INFORMATION piProcInfo{}; // handle = piProcInfo.hProcess

		mutable std::mutex safe_log;
		std::function<void(const std::string&, const message_type&)> log_func;
		std::thread thr_read;
		bool keep_running = false;
		bool is_running = false;
		bool has_run_once = false;

		struct thr_data {
			COMMTIMEOUTS timeouts = { 0, 0, 0, 0, 0 };
			std::string block;
			DWORD dwRead = 0;
			char buffer[512]{};
			size_t p_ar = 0;
		} td;

#ifdef UNICODE
		STARTUPINFOA siStartInfo{};
#else
		STARTUPINFO siStartInfo{};
#endif

		void log(const std::string&, const message_type&) const;
		void thr_read_output();
	public:
		~process();

		/// <summary>
		/// <para>Sets a function to handle task output.</para>
		/// <para>This will be triggered every breakline.</para>
		/// </summary>
		/// <param name="{std::function}">The function to handle task output.</param>
		void hook_stdout(std::function<void(const std::string&, const message_type&)>);

		/// <summary>
		/// <para>Launchs a executable.</para>
		/// </summary>
		/// <param name="{std::string}">The path or command to run.</param>
		/// <returns>{bool} True if successfully launched.</returns>
		bool launch(const std::string&);

		/// <summary>
		/// <para>Gets if the task is still up and running.</para>
		/// </summary>
		/// <returns>{bool} True if still running.</returns>
		bool running() const;

		/// <summary>
		/// <para>Set task to stop and wait.</para>
		/// </summary>
		void stop();

		/// <summary>
		/// <para>It is considered valid if it's running.</para>
		/// </summary>
		/// <returns>{bool} True if running.</returns>
		bool valid() const;

		/// <summary>
		/// <para>It is considered empty if there's nothing running.</para>
		/// </summary>
		/// <returns>{bool} True if stopped.</returns>
		bool empty() const;
	};

}

#include "process.ipp"