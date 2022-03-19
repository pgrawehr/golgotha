/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//This obviously causes a circular reference, but in this
//special case, this is intended.
//Note that there is no include guard on this file, but there
//is one on malloc.h
#include "memory/malloc.h"

#ifdef _DEBUG
//#ifdef i4_NEW_CHECK
//#ifndef new
//#define new new(__FILE__,__LINE__)
#ifdef _WINDOWS
#ifndef new
void * __cdecl operator new(size_t nSize, char * lpszFileName, int nLine);
void __cdecl operator delete(void * p, char * lpszFileName, int nLine);
#define new new (__FILE__,__LINE__)
#endif
#endif
//#endif

#else

#ifdef i4_NEW_CHECK
#ifndef new
#define new new (__FILE__,__LINE__)
#endif
#endif/*i4_NEW_CHECK*/

#endif
