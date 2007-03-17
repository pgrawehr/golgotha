/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//{{{ Interface 4
//
//  System dependant #defines and macros
//  Endianess macros
//    short_swap,long_swap  - exchange endianess
//    lstl, lltl            - little short to local, little long to local (swaps to x86 endianess)
//
/*
   The following macros should be defined by the make tool for the corresponding
   machines. _DEBUG and NDEBUG can always be optionally added

   The set of constants after the arrow is defined internally for use in i4 code.
   Throughout the code, you should use the most common macro possible for
   a given os-specific function.

   Windows (MSVC): _WIN32, _WINDOWS, ... some others -> NETWORK_INCLUDED, I4_FASTCALL
   	little endian
   Mac: __MAC__ -> (unknown, untested)
   	unknown endianess (big?)
   Linux: __linux -> I4_UNIX
   	little endian
   Sun: SUN4 -> I4_UNIX I4_WORDALIGN
   	big endian
   Sgi: __sgi -> I4_UNIX I4_WORDALIGN
   	big endian


   I4_UNIX is defined for unix derivats (running an X11 type display)
   I4_WORDALIGN is defined for architectures which need word-aligned memory
   	accesses. (Most RISC architectures do, intel does not).
 */

#ifndef __ARCH_HPP_
#define __ARCH_HPP_

#ifdef __MAC__
#include <Types.h>
#include <Files.h>
#include <Errors.h>
#endif

// The following section defines for each architecture and compiler
// alias names from the builtin data types to the i4 portable
// data types
// 32 bit architectures
#ifdef _WINDOWS
typedef unsigned long w32;
typedef unsigned short w16;
typedef unsigned char w8;
typedef signed long sw32;
typedef signed short sw16;
typedef signed char sw8;

#define snprintf _snprintf
#undef I4_UNIX /*Windows is not unix*/

#ifdef __WATCOMC__                   // visual c has __int64
typedef struct declw64 {
	long int hi;
	long int lo;
} w64;
#else
typedef unsigned __int64 w64;
typedef __int64 sw64;
#endif
#endif //Windows

#ifdef I4_UNIX
typedef unsigned long w32;
typedef unsigned short w16;
typedef unsigned char w8;
typedef signed long sw32;
typedef signed short sw16;
typedef signed char sw8;
typedef unsigned long long w64;
typedef long long sw64;

//the following definitions are needed to make some windows-parts
//compile under linux.
//these definitions are (C) Microsoft
#define MAX_PATH        256
typedef void *HINSTANCE;
typedef unsigned long HRESULT;
typedef void *HWND;
typedef struct _GUID {
	unsigned long Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[ 8 ];
} GUID;

//The following may still result in porting problems with some
//compilers internally declaring this. Gotta change the jpg files
//sometime, I suppose.
typedef long int INT32; //for jpg loader


// #if (SUN4 || __sgi)
// #define I4_WORDALIGN
// #endif

#define ZeroMemory(a,b) memset(a,0,b)
#endif


// 16 and 64 bit architectures are not defined yet
// but can be added here as long as the compiler supports 32 bit words

//The following macros are *everything* that should need
//changing due to endianess
#define w16_swap(x) (((((w16) (x)))<<8)|((((w16) (x)))>>8))
#define w32_swap(x) ((( ((w32)(x)) )>>24)|((( ((w32)(x)) )&0x00ff0000)>>8)| ((( ((w32)(x)) )&0x0000ff00)<<8)|(( ((w32)(x)) )<<24))

//All little endian systems go here
//of course, if you wan't to compile for linux, say on a sparc,
//you don't wanna go here. Preferably use some other switch, then.
#ifndef WORDS_BIGENDIAN

enum {
	i4_bigend=0, i4_litend=1
};

#define s_to_lsb(x) (x)
#define l_to_lsb(x) (x)
#define s_to_msb(x) w16_swap(x)
#define l_to_msb(x) w32_swap(x)

#else

enum {
	i4_bigend=1, i4_litend=0
};

#define s_to_lsb(x) w16_swap(x)
#define l_to_lsb(x) w32_swap(x)
#define s_to_msb(x) (x)
#define l_to_msb(x) (x)

#endif

//some generic typedefs
typedef sw16 i4_coord;  // use this type for all x & y positions withing images
typedef w32 i4_color;  // reserved as 32bits in case we expand to 32 bit color

#define i4_null 0

#ifdef _WINDOWS
#ifndef _MANAGED
#define I4_FAST_CALL __fastcall
#else
#define I4_FAST_CALL
#endif
#else
#define I4_FAST_CALL
#endif

typedef w8 i4_bool;

enum  {
	i4_F=0,
	i4_T=1
};

//Are we compiling for a 64 bit target?
//This is msvc-specific, you might want to add other tests here for other compilers.
#ifdef _M_IA64
#define I4_64BITCPU 1
#endif

//use the type wptr instead of w32 wherever a value might be assigned a pointer.
#ifdef I4_64BITCPU
typedef w64 wptr;
typedef sw64 swptr;
#else
#if defined (_MSC_VER) && (_MSC_VER>=1300)
typedef __w64 unsigned long wptr;
typedef __w64 signed long swptr;
#else
typedef w32 wptr;
typedef sw32 swptr;
#endif
#endif


// use this mainly for events, to cast to a known event type
#define CAST_PTR(var_name, type, source_obj) type *var_name=(type *)(source_obj)

#ifdef I4_64BITCPU
inline void *ALIGN_FORWARD(void *addr)
{
	return (void *)(((w64)addr+7)&~7);
}

inline void *ALIGN_BACKWARD(void *addr)
{
	return (void *)(((w64)addr)&~7);
}
#else
#ifdef _WINDOWS
#pragma warning(disable : 4311 4312) //pointer truncation for IA64.
#endif
inline void *ALIGN_FORWARD(void *addr)
{
	return (void *)(((w32)addr+3)&~3);
}

inline void *ALIGN_BACKWARD(void *addr)
{
	return (void *)(((w32)addr)&~3);
}
#ifdef _WINDOWS
#pragma warning(3 : 4311 4312)
#endif
#endif
#ifdef _WINDOWS
//Apply some modifications to the warning levels of the compiler
#include "pragma.h"
#endif

//There follow some macro definitions that are mainly for convenience.
//There's now one place where all names are that differ for the different
//versions (otherwise, stuff like different builds using the same
//texture cache file may happen).

//I4_FILE_PREFIX should be different for each system golgotha
//runs on. The data representation in these files *is*
//hardware dependent, unlike most other files.
//The texture cache is one usage of it.
//Hint: Failure to do so might result in weird texture errors on some systems.
#ifdef _WINDOWS
#define I4_FILE_PREFIX "win32"
#define GOLGOTHA_WINDOW_TITLE "Golgotha Milestone 5"
#endif

/*
 #else
 #ifdef __bsd
   //This must come before linux, since __bsd implies __linux
 #define I4_FILE_PREFIX "bsd"
 #define GOLGOTHA_WINDOW_TITLE "Golgotha Milestone 5 FreeBSD native build"
 #else
 #ifdef __linux
 #define I4_FILE_PREFIX "linux"
 #define GOLGOTHA_WINDOW_TITLE "Golgotha Milestone 5 linux native build"
 #else
 #ifdef SUN4
 #define I4_FILE_PREFIX "sun4"
 #define GOLGOTHA_WINDOW_TITLE "Golgotha Milestone 5 SUN Sparc Solaris native"
 #else
 #ifdef __sgi
 #define I4_FILE_PREFIX "sgi"
 #define GOLGOTHA_WINDOW_TITLE "Golgotha Milestone 5 SGI native"
 #else
 #define I4_FILE_PREFIX "unkn"
 #define GOLGOTHA_WINDOW_TITLE "Golgotha Milestone 5 unknown architecture build"
 #endif //sgi
 #endif //sun4
 #endif //linux
 #endif //bsd
 #endif //windows
 */
#endif
//the file arch.h
