// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS


#include "resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <atltrace.h>
#include <atlsync.h>
#include <comutil.h>
#include <atlrx.h>
#include <atltime.h>
#include <psapi.h>

using namespace ATL;

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <hash_map>
#include <list>
#include <set>
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>

typedef CAtlRECharTraitsW RegExpCharTraits;

#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

#ifdef _UNICODE
typedef std::wstringstream StringStream;
#else
typedef std::stringstream StringStream;
#endif

#ifndef ASSERT
# ifdef _DEBUG
#  define ASSERT(x) assert(x)
# else
#  define ASSERT(x)
# endif
#endif

#ifndef VERIFY
# ifdef _DEBUG
#  define VERIFY(x) ASSERT(x)
# else
#  define VERIFY(x) (x)
# endif
#endif

// static_assert is now a c++ keyword
template<bool _static_assert>
struct StaticAssert;

template<> struct StaticAssert<true> {};

#define STATIC_ASSERT(expr) do { StaticAssert<expr>(); break; } while(true)
#define ASSERT_SIZEOF(type1, type2) do { StaticAssert<sizeof(type1)==sizeof(type2)>(); break; } while(true)

//#define TRACK_MEMORY_ALLOCATION
#include "allocator.h"