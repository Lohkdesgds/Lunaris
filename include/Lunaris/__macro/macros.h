#pragma once

/*
This is a tool for every include thing in the Lunaris lib.
*/

#define WIN32_LEAN_AND_MEAN

// This is set by BUILDCOUNTER
#define LUNARIS_BUILD_NUMBER 1079

// https://stackoverflow.com/questions/240353/convert-a-preprocessor-token-to-a-string
#define __LUNARIS_STRINGIFY(x) #x
#define __LUNARIS_TOSTRING(x) __LUNARIS_STRINGIFY(x)

#ifdef LUNARIS_HEADER_ONLY
// -- HEADER ONLY -- //
#define LUNARIS_DECL inline
// -- ENDOF HEADER ONLY -- //
#else
// -- USING LIB -- //
#define LUNARIS_DECL
#ifdef _DEBUG
#pragma comment (lib, "LunarisLib-d.lib")
#elif LUNARIS_VERBOSE_BUILD
#pragma comment (lib, "LunarisLib-rv.lib")
#else
#pragma comment (lib, "LunarisLib.lib")
#endif
// -- ENDOF USING LIB -- //
#endif

// User optional things

#ifdef LUNARIS_AUDIO_SAMPLE_AMOUNT
#define LUNARIS_AUTOSET_AUDIO_SAMPLE_AMOUNT LUNARIS_AUDIO_SAMPLE_AMOUNT
#else
#define LUNARIS_AUTOSET_AUDIO_SAMPLE_AMOUNT 8
#endif

#include <string>
#ifdef LUNARIS_VERBOSE_BUILD 
#include <stdarg.h>
#include <mutex>
#include <thread>
#endif

namespace Lunaris {

	const char ___lunaris_date[] = __TIMESTAMP__;

#define ___LUNARIS_VERSION_SHORT ("Lunaris B" __LUNARIS_TOSTRING(LUNARIS_BUILD_NUMBER))
#define ___LUNARIS_VERSION_LONG ((std::string("Lunaris build #" __LUNARIS_TOSTRING(LUNARIS_BUILD_NUMBER) " | Date: ") + Lunaris::___lunaris_date + " Brazil/Sao_Paulo").c_str())
#define ___LUNARIS_VERSION_DATE (Lunaris::___lunaris_date)

#define LUNARIS_VERSION_SHORT ___LUNARIS_VERSION_SHORT
#define LUNARIS_VERSION_LONG ___LUNARIS_VERSION_LONG
#define LUNARIS_VERSION_DATE ___LUNARIS_VERSION_DATE

	/// <summary>
	/// Template for non copyable classes (in the works)
	/// </summary>
	class NonCopyable {
	public:
		NonCopyable() = default;
		~NonCopyable() = default;

		NonCopyable(const NonCopyable&) = delete;
		void operator=(const NonCopyable&) = delete;
		NonCopyable(NonCopyable&&) = default;
		void operator=(NonCopyable&&) noexcept {}
	};

	/// <summary>
	/// <para>Template for non movable classes (in the works)</para>
	/// <para>Note: std::move will result in copy.</para>
	/// </summary>
	class NonMovable {
	public:
		NonMovable() = default;
		~NonMovable() = default;

		NonMovable(NonMovable&&) = delete;
		void operator=(NonMovable&&) = delete;
		NonMovable(const NonMovable&) = default;
		void operator=(const NonMovable&) {};
	};

#ifdef LUNARIS_VERBOSE_BUILD // easy verbose

	inline std::recursive_mutex __g_verbose_lock;

	inline void __PRINT_DEBUG(const char* const _file, const size_t _line, const char* const _format, ...)
	{
		std::unique_lock<std::recursive_mutex>(__g_verbose_lock);

		va_list args;
		va_start(args, _format);
		{
			const char* __rf1 = strrchr(_file, '/');
			const char* __rf2 = strrchr(_file, '\\');
			const char* __rff = __rf1 ? (__rf1 + 1) : (__rf2 ? (__rf2 + 1) : _file);
			const auto __tid = std::hash<std::thread::id>()(std::this_thread::get_id());
#ifdef _WIN32
			printf_s("(%020zu)[%s|%04zu] ", __tid, __rff, _line);
			if (_format) {
				vprintf_s(_format, args);
				printf_s("\n");
			}
#else
			printf("(%020zu)[%s|%04zu] ", __tid, __rff, _line);
			if (_format) {
				vprintf(_format, args);
				printf("\n");
			}
#endif
		}
		va_end(args);
	}

// if FORMAT null no breakline, just format
#define PRINT_DEBUG(_FORMAT, ...) Lunaris::__PRINT_DEBUG(__FILE__, __LINE__, _FORMAT, __VA_ARGS__)
#else
#define PRINT_DEBUG(_FORMAT, ...)
#endif
}