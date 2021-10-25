#pragma once

/*
This is a tool for every include thing in the Lunaris lib.
*/

#define WIN32_LEAN_AND_MEAN

// This is set by BUILDCOUNTER
#define LUNARIS_BUILD_NUMBER 221

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
