#pragma once

/*
This is a tool for every include thing in the Lunaris lib.
*/

#define WIN32_LEAN_AND_MEAN

// This is set by BUILDCOUNTER
#define LUNARIS_BUILD_NUMBER 957

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
}