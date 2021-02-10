#pragma once

/*
This will include ALL libraries
- You can actually include only a fraction of it, but sincerely I prefer to include them all.

Dependency order is:
- Between same folder
- One folder up

Folder order:
- Handling
- Tools
- Interface
- Work

The libraries are compiled with static ALLEGRO (for release) and /MT or /MTd. You have to match this so it works.
*/

/*
- - - - - - - - - - -|>========== SETUP MACROS POSSIBLE ==========<|- - - - - - - - - - -
-> LSW_NOT_USING_LIB : If you're not using the .lib but the .cpp's, define this before including any header.
### NOT FULLY DONE
-> LSW_AVOID_NON_LIB : This disables including .ipp files, so any source code is from the .lib aka no new templates can be generated.
*/

#if defined( LSW_NOT_USING_LIB ) && defined( LSW_AVOID_NON_LIB )
#error "You can't set both!"
#endif

#include "_Macros/system_work.h"

#define WIN32_LEAN_AND_MEAN

#include "Handling/Handling.h"
#include "Tools/Tools.h"
#include "Interface/Interface.h"
#include "Work/Work.h"

#ifndef LSW_NOT_USING_LIB

#ifdef _DEBUG

#ifdef LSW_X64
#pragma comment (lib, "LSW_dynamic_debug.lib")
#else
#pragma comment (lib, "LSW_dynamic_debug_x86.lib")
#endif

#else

#ifdef LSW_X64
#pragma comment (lib, "LSW_static_release.lib")
#else
#pragma comment (lib, "LSW_static_release_x86.lib")
#endif

#endif // _DEBUG

#endif // LSW_NOT_USING_LIB