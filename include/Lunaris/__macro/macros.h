#pragma once

/*
This is a tool for every include thing in the Lunaris lib.
*/

#define WIN32_LEAN_AND_MEAN

// This is set by BUILDCOUNTER
#define LUNARIS_BUILD_NUMBER 422

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

namespace Lunaris {

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