#pragma once


#include <Windows.h>
#include <functional>
#include <thread>
#include <string>

#include "../SuperThread/superthread.h"


namespace LSW {
	namespace v5 {
		namespace Tools {

			namespace launcher {
				constexpr size_t buf_size = 1 << 3; // small because if big it will wait too much to flush
			}

			/// <summary>
			/// <para>This can launch an app and get its output for as long as it runs.</para>
			/// </summary>
			class Launcher {
				HANDLE g_hChildStd_OUT_Rd = nullptr;
				HANDLE g_hChildStd_OUT_Wr = nullptr;
				SECURITY_ATTRIBUTES saAttr{};

				PROCESS_INFORMATION piProcInfo{}; // handle = piProcInfo.hProcess
#ifdef UNICODE
				STARTUPINFOA siStartInfo{};
#else
				STARTUPINFO siStartInfo{};
#endif

				char inter_buf[launcher::buf_size] = { 0 };

				std::function<void(const std::string&)> prunt = std::function<void(const std::string&)>();
				SuperThread<> autosav;
				//bool keep = false;
				//bool still_running = false;

				void keep_reading(boolThreadF);
			public:
				~Launcher();

				/// <summary>
				/// <para>Sets a function to handle task output.</para>
				/// </summary>
				/// <param name="{std::function}">The function to handle task output.</param>
				void hook_output(const std::function<void(const std::string&)>);

				/// <summary>
				/// <para>Launchs a executable.</para>
				/// <para># WARNING: IT DOESN'T AUTOMATICALLY HANDLE %TAGS%!</para>
				/// </summary>
				/// <param name="{std::string}">The path or command to run.</param>
				/// <returns>{bool} True if successfully launched.</returns>
				bool launch(const std::string&);

				/// <summary>
				/// <para>Gets if the task is still up and running.</para>
				/// </summary>
				/// <returns>{bool} True if still running.</returns>
				bool running();

				/// <summary>
				/// <para>Forces the task to be killed.</para>
				/// <para>It will set to stop and wait it terminate.</para>
				/// </summary>
				void stop();
			};

		}
	}
}