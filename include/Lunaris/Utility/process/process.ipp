#pragma once

namespace Lunaris {

    void process::log(const std::string& msg) const
    {
        std::lock_guard<std::mutex> l(safe_log);
        if (log_func) log_func(msg);
    }

	void process::thr_read_output()
	{
        log("Process #" + std::to_string((uintptr_t)piProcInfo.hProcess) + " started.");
        is_running = true;

        SetCommTimeouts(g_hChildStd_OUT_Rd, &td.timeouts);

        while (keep_running && (WaitForSingleObject(piProcInfo.hProcess, 25) == WAIT_TIMEOUT)) {

            if (!ReadFile(g_hChildStd_OUT_Rd, td.buffer, sizeof(td.buffer), &td.dwRead, nullptr)) continue;

            for (td.p_ar = 0; td.p_ar < td.dwRead; td.p_ar++) {
                char& ch = td.buffer[td.p_ar];
                switch (ch) {
                case '\n':
                    log(td.block);
                    td.block.clear();
                    break;
                default:
                    td.block += ch;
                    break;
                }
            }
        }

        log("Process #" + std::to_string((uintptr_t)piProcInfo.hProcess) + " closed successfully.");

        CloseHandle(piProcInfo.hProcess);
        is_running = false;
	}

    process::~process()
    {
        stop();
    }

    void process::hook_stdout(std::function<void(const std::string&)> fun)
    {
        std::lock_guard<std::mutex> l(safe_log);
        log_func = fun;
    }

    bool process::launch(const std::string& cmd)
    {
        stop();

        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        // Create a pipe for the child process's STDOUT. 
        if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
            return false;

        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
            return false;

        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
#ifdef UNICODE
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
#else
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
#endif
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = g_hChildStd_OUT_Wr;
        siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        bool good = CreateProcessA(NULL,
            const_cast<char*>(cmd.c_str()),   // command line 
            NULL,          // process security attributes 
            NULL,          // primary thread security attributes 
            TRUE,          // handles are inherited 
            0,             // creation flags 
            NULL,          // use parent's environment 
            NULL,          // use parent's current directory 
            &siStartInfo,  // STARTUPINFO pointer 
            &piProcInfo);  // receives PROCESS_INFORMATION 

        if (!good) return false; // bad

        is_running = true; // assume it's running because it's all good!

        CloseHandle(piProcInfo.hThread);

        CloseHandle(g_hChildStd_OUT_Wr);

        if (thr_read.joinable()) thr_read.join();
        keep_running = true;
        thr_read = std::thread([&] { thr_read_output(); });

        return true;
    }

    bool process::running() const
    {
        return keep_running && is_running;
    }

    void process::stop()
    {
        if (keep_running) {
            keep_running = false;

            for (size_t tries = 0; tries < 10 && is_running; tries++) std::this_thread::sleep_for(std::chrono::milliseconds(50));

            if (is_running) { // has a task running still. Must kill.
                TerminateProcess(piProcInfo.hProcess, 0);
                thr_read.detach();
                TerminateThread(thr_read.native_handle(), 0);
                is_running = false;

                log("Last process was killed after timeout.");
            }
            else if (thr_read.joinable()) thr_read.join();
        }
    }

}