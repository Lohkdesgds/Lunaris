#pragma once

#if _WIN32 || _WIN64 || __x86_64__ || __ppc64__
#if _WIN64
#define LSW_X64
#else
#define LSW_X32
#endif
#endif