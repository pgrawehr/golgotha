//This file is the main source for precompiled headers
//This has no great advantage in compilling speed 
//versus the need to recompile everything when one of
//the include files changes.
#ifndef __PCH_H
#define __PCH_H

#include <stdlib.h>
#include <string.h>
#ifdef _WINDOWS
//Those are needed only for windows
//Stdafx includes windows.h
#include "StdAfx.h"
#include <ddraw.h>
#define NETWORK_INCLUDED
//#include <d3d.h>
#else
//include the file created by the configure script
#include "config.h"
#define I4_UNIX


#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif



#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#include <sys/time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#endif
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif

#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif



#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif

#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETINET_IP_H
//This header doesn't compile on some os's. 
//#include <netinet/ip.h>
#endif

#if (HAVE_NETINET_IN_H && HAVE_ARPA_INET_H)
#define NETWORK_INCLUDED
#endif

#ifdef WORDS_BIGENDIAN
#define I4_WORDALIGN
#endif

#ifdef HAVE_PTHREAD_MUTEX_RECURSIVE 
#define I4_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE
#endif
#ifdef HAVE_PTHREAD_MUTEX_RECURSIVE_NP
#undef I4_MUTEX_RECURSIVE
#define I4_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif
#ifndef I4_MUTEX_RECURSIVE
#error Initializer for recursive mutexes not found. 
#endif

#ifdef NEED_PTHREAD_MUTEXATTR_SETKIND_NP_DEF
extern "C" int pthread_mutexattr_setkind_np(void *attr, int __kind);
#endif
#ifdef NEED_PTHREAD_MUTEXATTR_SETTYPE_DEF
extern "C" int pthread_mutexattr_settype(void *attr, int __kind);
#endif

#ifdef HAVE_PTHREAD_MUTEXATTR_SETKIND_NP
#define I4_MUTEX_SETTYPE(a,b) pthread_mutexattr_setkind_np((a),(b))
#else
#ifdef HAVE_PTHREAD_MUTEXATTR_SETTYPE 
#define I4_MUTEX_SETTYPE(a,b) pthread_mutexattr_settype((a),(b))
#else
#error pthread_mutexattr_set...() not found on this system. Check your POSIX implementation
#endif
#endif

//One thing that probably needs to be done:
//If the compiler is sgi MIPSPRO, #define __sgi 
//That one is merely a compiler specific directive, not system specific.

#endif
//include architecture specifics
#include "arch.h"
#include <math.h>
#include "time/profile.h"
#include "app/app.h"
#include "math/random.h"
#include "resource.h"
#include "resources.h"
#include "time/time.h"
#include "time/timedev.h"



#endif




