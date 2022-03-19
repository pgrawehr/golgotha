/*
   2000.04.12
   i4_malloc macro changed to I4_MALLOC
   JJ FILES MERGED

 */
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
//WARNING: Most of this code is only dummy! We are using the MFC
//for memory management under Win95!
#include "pch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "arch.h"

#include "memory/malloc.h"
#include "error/error.h"
#include "threads/threads.h"
#include "time/profile.h"
#include "init/init.h"
#include "file/file.h"
#include "file/static_file.h"
#include "string/string.h"

//#include "afx.h"

//WARNING: This code is REALLY messy.
#ifdef _WINDOWS
//Wrapper functions for debugger
#undef new
#ifdef _DEBUG
//AfxAllocMemoryDebug is not defined for release builds
void * __cdecl operator new(size_t nSize, char * lpszFileName, int nLine)
{
	return AfxAllocMemoryDebug(nSize,FALSE,lpszFileName,nLine);
}



void __cdecl operator delete(void * p, char * lpszFileName, int nLine)
{
	AfxFreeMemoryDebug(p,FALSE);
}
#else

void * __cdecl operator new(unsigned int nSize, char * lpszFileName, int nLine)
{
	return malloc(nSize);
}

void __cdecl operator delete(void * p, char * lpszFileName, int nLine)
{
	free(p);
}
#endif //nodebug

#endif

// define this to override the native memcpy to check for overwriting memory leaks
//#define i4_MEMCPY_CHECK
#define i4_FREE_CHECK

//#ifdef i4_NEW_CHECK
//#undef new
//#endif

#undef i4_malloc
#undef i4_realloc

static i4_critical_section_class mem_lock;


#ifndef __MAC__

static i4_profile_class pf_malloc("i4_malloc");
static i4_profile_class pf_free("i4_free");


// declare the new()s before we include malloc.hh so that the new
// macro doesn't mess up their definition
int i4_m_instance=0;
int i4_mem_break=-1;

#if 0
#if (__linux && i4_NEW_CHECK)

void * operator new( size_t size, char * file, w32 line)
{
	return i4_malloc(size, file, line);
}


void * operator new [] (size_t size, char * file, w32 line)
{
	return i4_malloc(size, file, line);
}
#endif

#endif

/*#include <new.h>

 #undef new


   void *operator new( size_t size)
   {
   return i4_malloc(size,"unknown",0);
   }

   void operator delete(void *ptr)
   {
   i4_free(ptr);
   }*/


#endif

#ifdef i4_MEM_CHECK
#define i4_MEM_CLEAR
#endif


#include "memory/bmanage.h"


#ifdef i4_MEM_CHECK
// can be set in debugger, break mem fun will be called when this address is allocated
long break_mem_point=0;
void break_mem_fun()
{
	printf("memory breakpoint\n");
}

#ifdef i4_MEMCPY_CHECK
// can set this memory range to check for mem copies over memory
w32 i4_check_min=0, i4_check_max=0;

extern "C" void *memcpy(void * dest, const void * src,size_t n)
{
	if (((w32)dest)<i4_check_max && ((w32)dest)+n>i4_check_min)
	{
		break_mem_fun();
	}

	bcopy(src,dest,n);
}
#endif

#endif

//cannot include this earlier.
#include "memory/new.h"

i4_block_manager_class bmanage[5];
int bmanage_total=0;


void inspect_memory()
{
	mem_lock.lock();
	for (int i=0; i<bmanage_total; i++)
	{
		bmanage[i].inspect();
	}
	mem_lock.unlock();
}


void i4_malloc_uninit()
{
}


void i4_mem_cleanup(int ret, void * arg)
{
	i4_malloc_uninit();
}


int i4_malloc_max_size=20000000;
int i4_malloc_min_size=0x100000;

char * not_enough_total_memory_message=
	"Memory manager : Sorry you do not have enough memory available to\n"
	"                 run this program.\n";


void i4_set_min_memory_required(int bytes)
{
	i4_malloc_min_size=bytes;
}

void i4_set_max_memory_used(int bytes)
{
	i4_malloc_max_size=bytes;
	if (i4_malloc_min_size< i4_malloc_max_size)
	{
		i4_malloc_min_size=i4_malloc_max_size;
	}
}


void i4_malloc_init()
{
	if (bmanage_total)
	{
		i4_warning("warning : i4_malloc_init called twice\n");
	}
	else
	{
		void * mem;

		sw32 size=i4_malloc_max_size;
		for (mem=NULL; !mem && size>=i4_malloc_min_size;)
		{
			mem=malloc(size);
			if (!mem)
			{
				size-=0x100;
			}
		}

		if (mem)
		{
			bmanage[bmanage_total].init(mem,size);
			bmanage_total++;
			size-=0x1000;
		}
		else
		{
			i4_error(not_enough_total_memory_message);
		}
	}
}



class i4_memory_init_class :
	public i4_init_class
{
public:
	int init_type()
	{
		return I4_INIT_TYPE_MEMORY_MANAGER;
	}

	void init()
	{
		i4_malloc_init();
	}

	void uninit()
	{
		i4_malloc_uninit();
	}

} i4_memory_init_instance;


swptr i4_available()
{
	mem_lock.lock();
	swptr size=0;
	for (int i=0; i<bmanage_total; i++)
	{
		size+=bmanage[i].available();
	}
	mem_lock.unlock();
	return size;
}

swptr i4_largest_free_block()
{
	mem_lock.lock();
	swptr l=0;
	for (int i=0; i<bmanage_total; i++)
	{
		swptr t=bmanage[i].largest_free_block();
		if (t>l)
		{
			l=t;
		}
	}
	mem_lock.unlock();
	return l;
}

swptr i4_allocated()
{
	mem_lock.lock();

	swptr size=0;
	for (int i=0; i<bmanage_total; i++)
	{
		size+=bmanage[i].allocated();
	}

	mem_lock.unlock();
	return size;
}


void *i4_malloc(w32 size, char * file, int line)
{
	return malloc(size);
}


void i4_free(void * ptr)
{
	free(ptr);
}


void *i4_realloc(void * ptr, w32 size, char * file, int line)
{
	if (!ptr)
	{
		// malloc is already lock protected
		return i4_malloc(size, file, line);
	}

	return realloc(ptr, size);
}


void dmem_report()
{
	i4_mem_report("debug_mem.log");
}

static i4_static_file_class mem_report;

void i4_mem_report(char * filename)
{
	i4_file_class * fp=mem_report.open(filename);

	if (fp)
	{
		fp->printf("Total available=%d, allocated=%d\n", i4_available(), i4_allocated());

		for (int i=0; i<bmanage_total; i++)
		{
			bmanage[i].report(fp);
		}

	}


}

/********************************************************************** <BR>

   GROWHEAP.CPP

   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "memory/growheap.h"
#include "error/error.h"

i4_grow_heap_class::i4_grow_heap_class(w32 initial_size, w32 grow_increment)
//{{{
{
	increment=grow_increment;

	if (initial_size)
	{
		heap * h=new heap(initial_size,"initial grow heap");
		list.insert(*h);
		current_size=initial_size;
		current_offset=0;
	}
	else
	{
		current_size=0;
		current_offset=0;
	}
}
//}}}


void * i4_grow_heap_class::malloc(w32 size, char * description)
//{{{
// description not used right now
{
	if (size==0)
	{
		return 0;
	}

	if (current_offset+size<current_size)
	{
		void * ret=(void *)(((w8 *)list.begin()->data)+current_offset);
		current_offset+=size;
		return ret;
	}
	else
	{
		if (size>increment)
		{
			//i4_error("grow heap param problem");
			increment=size*2+2; //just ignore and adjust...
		}

		heap * h=new heap(increment,"grow heap");
		list.insert(*h);
		current_size=increment;
		current_offset=0;
		return malloc(size,description);
	}
}
//}}}


void i4_grow_heap_class::clear()
//{{{
{
	heap_iter p = list.begin();

	heap_iter q(p);

	current_offset=0;

	++p;

	while (p != list.end())
	{
		list.erase_after(q);
		delete &*p;
		p = q;
		++p;
	}


}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}


/********************************************************************** <BR>

   BMANAGE.CPP

   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "memory/bmanage.h"
#include "error/error.h"
#include "file/file.h"
#include <string.h>
#include <stdlib.h>

//#define i4_MEM_CLEAR 1

#ifdef i4_MEM_CHECK
static int i4_show_libc_mallocs=0;

static char *str_alloc(char * s)
{
	if (i4_show_libc_mallocs)
	{
		i4_warning("str_alloc : '%s'", s);
	}

	int l=strlen(s)+1;
	char * t=(char *)::malloc(l);
	if (!t)
	{
		i4_error("could not allocate memory for string on libc heap");
	}
	memcpy(t,s,l);
	return t;
}

static void str_free(char * s)
{
	if (!s || s[0]==0)
	{
		i4_error("trying to free libc str (0)");
	}

	if (i4_show_libc_mallocs)
	{
		i4_warning("str_free : '%s'", s);
	}

		::free(s);
}

#endif


void i4_block_manager_class::inspect()
{
}


void i4_block_manager_class::report(i4_file_class * fp)
{
}

swptr i4_block_manager_class::largest_free_block()
{
	return INT64_MAX;
}

swptr i4_block_manager_class::available()
{
	return INT64_MAX;
}

swptr i4_block_manager_class::allocated()
{
	return 0;
}

void i4_block_manager_class::init(void * block, long Block_size)
{

}

void * i4_block_manager_class::alloc(long size, char * name)
{
	return malloc(size);
}

/************************** FREE **********************************/
/*    should be called to free a pointer in the static heap       */
/*    i.e. begining of the heap                                   */
/******************************************************************/
void i4_block_manager_class::free(void * ptr)
{
	if (ptr != nullptr)
	{
		free(ptr);
	}
}
