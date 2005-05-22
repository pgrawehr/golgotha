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
void* __cdecl operator new(unsigned int nSize, char *lpszFileName, int nLine)
	{
	return AfxAllocMemoryDebug(nSize,FALSE,lpszFileName,nLine);
	}



void __cdecl operator delete(void* p, char *lpszFileName, int nLine)
	{
	AfxFreeMemoryDebug(p,FALSE);
	}
#else

void* __cdecl operator new(unsigned int nSize, char *lpszFileName, int nLine)
	{
	return malloc(nSize);
	}

void __cdecl operator delete(void* p, char *lpszFileName, int nLine)
	{
	free(p);
	}
#endif//nodebug

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

void *operator new( size_t size, char *file, w32 line)
{  
  return i4_malloc(size, file, line);
}


void *operator new [](size_t size, char *file, w32 line)
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

extern "C" void *memcpy(void* dest, const void *src,size_t n)
{
  if (((w32)dest)<i4_check_max && ((w32)dest)+n>i4_check_min)
    break_mem_fun();

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
  for (int i=0;i<bmanage_total;i++)
    bmanage[i].inspect();
  mem_lock.unlock();
}


void small_allocation_summary(int &total, int *&list)
{
  int size=1;
  total=JM_SMALL_SIZE/4;
  list=(int *)i4_malloc(total*sizeof(int), __FILE__, __LINE__);
  
  for (;size<total;size++)
  {
    list[size]=0;
    int i,x;
    for (i=0;i<bmanage_total;i++)
    {
      small_block *s=bmanage[i].sblocks[size];
      while (s) 
      { 
        for (x=0;x<32;x++)
          if (s->alloc_list&(1<<x))
            list[size]++; 

        s=s->next; 
      }
    }
  }
}


void i4_malloc_uninit()
{
#ifdef i4_MEM_CHECK
  i4_mem_report("end_mem.log");
#endif

  mem_lock.lock();

  for (int i=0;i<bmanage_total;i++)
  {
    free(bmanage[i].addr);
  }
  bmanage_total=0;
  mem_lock.unlock();
}


void i4_mem_cleanup(int ret, void *arg)
{
  i4_malloc_uninit(); 
}

 
int i4_malloc_max_size=20000000;
int i4_malloc_min_size=0x100000;

char *not_enough_total_memory_message=
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
    i4_malloc_min_size=i4_malloc_max_size;
}


void i4_malloc_init()
{
  if (bmanage_total)
    i4_warning("warning : i4_malloc_init called twice\n");
  else
  {
    void *mem;

    sw32 size=i4_malloc_max_size;
    for (mem=NULL;!mem && size>=i4_malloc_min_size;)
    {
      mem=malloc(size);
      if (!mem) size-=0x100;        
    }

    if (mem)
    {
      bmanage[bmanage_total].init(mem,size);
      bmanage_total++; 
      size-=0x1000;
    }  
    else
      i4_error(not_enough_total_memory_message);
  }
}



class i4_memory_init_class : public i4_init_class
{
public:
  int init_type() { return I4_INIT_TYPE_MEMORY_MANAGER; }

  void init() { i4_malloc_init(); }

  void uninit() { i4_malloc_uninit(); }

} i4_memory_init_instance;


swptr i4_available()
{
  mem_lock.lock();
  swptr size=0;
  for (int i=0;i<bmanage_total;i++) 
    size+=bmanage[i].available();
  mem_lock.unlock();
  return size;
}

swptr i4_largest_free_block()
{
  mem_lock.lock();
  swptr l=0;
  for (int i=0;i<bmanage_total;i++)
  {
    swptr t=bmanage[i].largest_free_block();
    if (t>l)
      l=t;
  }
  mem_lock.unlock();
  return l;
}

swptr i4_allocated()
{
  mem_lock.lock();
  
  swptr size=0;
  for (int i=0;i<bmanage_total;i++) 
    size+=bmanage[i].allocated();
  
  mem_lock.unlock();
  return size;
}


void *i4_malloc(w32 size, char *file, int line)
{
  pf_malloc.start();

  if (size==0)
    i4_warning("tried to malloc 0 bytes");

  i4_m_instance++;
  if (i4_m_instance == i4_mem_break)
    i4_warning("i4 mem break");

#ifdef i4_MEM_CHECK
  char reason[200];

  sprintf(reason, "%s:%d (inst=%d)", file, line, i4_m_instance);
#else
  char *reason="unknown";
#endif

//   printf("Instance %d: %s\n", i4_m_instance, reason);

//  I4_TEST(i4_is_initialized(), "i4_malloc called before i4_init");

  if (!bmanage_total) 
  {
    void *ret=malloc(size);
    return ret;
  }

  size=(size+3)&(0xffffffff-3);
  mem_lock.lock();


    for (int i=0;i<bmanage_total;i++)
    {
      void *a=bmanage[i].alloc(size, reason);
      if (a) 
      {
        mem_lock.unlock();
        pf_malloc.stop();
        return a;
      }
    }

    mem_lock.unlock();
    i4_mem_report("no_mem.log");

	i4_error("CRITICAL: i4_memory_manager: Out of memory!");
	return 0;
  
}


void i4_free(void *ptr)
{
  if (!ptr)
	  return;
  pf_free.start();

  if (!i4_is_initialized())
	  {
	  
	  i4_warning("INTERNAL: i4_free called after i4_uninit()");
	  free(ptr);//we can only hope that we guess right here.
	  return;
	  }

  if (!bmanage_total) 
  { 
    free(ptr); 
    pf_free.stop();
    return ; 
  }

#ifdef i4_FREE_CHECK
  //I4_ASSERT(valid_ptr(ptr), "invalid free!");
#endif

  mem_lock.lock();
  for (int i=0;i<bmanage_total;i++)
    if (ptr>=(void *)bmanage[i].sfirst)  // is the pointer in this block?
    {
      if (ptr<=(void *)bmanage[i].slast)  // is it in static space?
      {
        bmanage[i].free(ptr);
        mem_lock.unlock();
        pf_free.stop();
        return ;
      } 
    }
 
  mem_lock.unlock();
  
  //i4_error("INTERNAL: i4_free() : bad pointer.\n");
  //delete ptr;
  free(ptr);
  ptr=NULL;
  pf_free.stop();
}


void *i4_realloc(void *ptr, w32 size, char *file, int line)
{  
  if (!ptr) 
  {
    // malloc is already lock protected
    return i4_malloc(size, file, line);
  }


  if (!bmanage_total) 
  { 
    // thread protect the c library realloc
    mem_lock.lock();
    void *ret=realloc(ptr,size); 
    mem_lock.unlock();
    return ret;
  }

  if (size==0) 
  { 
    // free is already lock protected
    i4_free(ptr); 
    return NULL; 
  }

  swptr old_size=0;
  for (int i=0;i<bmanage_total;i++)
    if (ptr>=(void *)bmanage[i].sfirst && 
        ptr<=(void *)(((char *)bmanage[i].sfirst)+bmanage[i].block_size))
    {
      old_size=bmanage[i].pointer_size(ptr);  
      if (ptr<=(void *)bmanage[i].slast)
      {
        void *nptr=i4_malloc(size, file, line);
        if ((sw32)size>old_size)
          memcpy(nptr,ptr,old_size);
        else memcpy(nptr,ptr,size);

        bmanage[i].free(ptr);


        return nptr;
      }
    }

  i4_error("ERROR: i4_realloc() : bad pointer");
  return NULL;
}


void dmem_report()
{
  i4_mem_report("debug_mem.log");
}

static i4_static_file_class mem_report;

void i4_mem_report(char *filename)
{
  i4_file_class *fp=mem_report.open(filename);

  if (fp)
  {
    for (int i=0;i<bmanage_total;i++)
      bmanage[i].report(fp);
  
    fp->printf("Total available=%d, allocated=%d\n", i4_available(), i4_allocated());    
  }
  
}


swptr small_ptr_size(void *ptr)
{
  return ((small_block *)(((swptr *)ptr)[-1]))->size;
}


int valid_ptr(void *ptr)
{
  if (!bmanage_total) { return 0; }
  for (int i=0;i<bmanage_total;i++)
    if (ptr>=(void *)bmanage[i].sfirst)  // is the pointer in this block?
    {
      if (ptr<=(void *)bmanage[i].slast)
      {
        int ret=bmanage[i].valid_ptr(ptr);
        return ret;
      }
    }

  return 0;
}




int valid_memory(void *ptr)
{
  for (int i=0; i<bmanage_total; i++)
    if (ptr>=(void *)bmanage[i].sfirst)  // is the pointer in this block?
      if (ptr<=(void *)bmanage[i].slast)
        return 1;

  return 0;
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
    heap *h=new heap(initial_size,"initial grow heap");
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


void *i4_grow_heap_class::malloc(w32 size, char *description)           
//{{{
// description not used right now
{
  if (size==0) 
    return 0;

  if (current_offset+size<current_size)
  {
    void *ret=(void *)(((w8 *)list.begin()->data)+current_offset);
    current_offset+=size;
    return ret;
  } else
  {
    if (size>increment)
		{
        //i4_error("grow heap param problem");
		increment=size*2+2;//just ignore and adjust...
		}

    heap *h=new heap(increment,"grow heap");
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

static char *str_alloc(char *s) 
{
  if (i4_show_libc_mallocs)
    i4_warning("str_alloc : '%s'", s);

  int l=strlen(s)+1;
  char *t=(char *)::malloc(l);
  if (!t)
    i4_error("could not allocate memory for string on libc heap");
  memcpy(t,s,l);
  return t;
}

static void str_free(char *s)
{
  if (!s || s[0]==0)
    i4_error("trying to free libc str (0)");

  if (i4_show_libc_mallocs)
    i4_warning("str_free : '%s'", s);

  ::free(s);
}

#endif

int i4_block_manager_class::valid_ptr(void *ptr)
{
  void *next=(void *)(*(((swptr *)ptr)-1));
  if (next && next<ptr)                        // small allocation
  {
    small_block *s=(small_block *)next;
    if (s->size<=0) return 0;

    small_block *c=sblocks[s->size];
    while (c && c!=s) c=c->next;
    if (!c) return 0;
    return 1;
  }

  memory_node *o=(memory_node *)(((char *)ptr)-sizeof(memory_node));
  memory_node *f=sfirst;
  while (f && f!=o) f=f->next;
  if (f) return 1;
  else return 0;
}


void i4_block_manager_class::inspect()
{
  memory_node *f=sfirst;
  for (;f;f=f->next);               // scan through static big list

  int i,bit=1;
  for (i=0;i<JM_SMALL_SIZE;i++)
  {
    for (small_block *s=sblocks[i];s;s=s->next)
    {
      char *addr=((char *)(s+1));
      bit = 1;
      for (int j=0;j<32;j++)
      {
        if (s->alloc_list&bit)
        {
          void *next=(void *)(*(((swptr *)addr)));
          if (next!=s)
          {
            i4_warning("inspect : bad pointer\n");
            return;
          }
        }
        bit=bit<<1;
        addr+=s->size+4;
      }
    }
  }
}


void i4_block_manager_class::report(i4_file_class *fp)
{
  fp->printf("************** Block size = %d ***************\n",block_size);
  int i=0;
  memory_node *f=sfirst;
  swptr f_total=0, a_total=0;

  for (;f;f=f->next,i++)
  {    
    fp->printf("%4d   %p (%d) %4d      ",i,f,((char *)f-(char *)sfirst),f->size);
    if (f->size>0)
      a_total+=f->size;
    else
      f_total+=-f->size;

#ifdef i4_MEM_CHECK
    if (f->size>0)    
      fp->printf("%s",f->name);
    else 
      fp->printf("FREE");
#endif
    fp->printf("\n");
  }    
  for (i=0;i<JM_SMALL_SIZE;i++)
  {
    for (small_block *s=sblocks[i];s;s=s->next)
    {      
      fp->printf("*** Small Block size = %d ***\n",i);      
      unsigned long bit=1;
      char *addr=((char *)(s+1));
      for (int j=0;j<32;j++)
      {
	fp->printf("%p   ",addr);
	if (s->alloc_list&bit)
	{
#ifdef i4_MEM_CHECK
	  fp->printf("%s\n",s->name[j]);
#else
	  fp->printf("allocated\n");
#endif	  
	} else fp->printf("FREE\n");
	bit=bit<<1;
	addr+=s->size+4;
      }
    }
  }

  fp->printf("**************** Block summary : %d free, %d allocated\n", f_total, a_total);

}

swptr i4_block_manager_class::pointer_size(void *ptr)
{
  swptr ret;
  void *next=(void *)(*(((swptr *)ptr)-1));
  if (next>ptr)
    ret=((memory_node *)(((char *)ptr)-sizeof(memory_node)))->size;
  else
    ret=((small_block *)next)->size;  

  return ret;
}


swptr i4_block_manager_class::largest_free_block()
{
  swptr l=0;
  memory_node *f;
  for (f=sfirst;f;f=f->next)
    if (-f->size>l)
      l=-f->size;

  return l;
}

swptr i4_block_manager_class::available()
{
  swptr size=0;
  memory_node *f;
  for (f=sfirst;f;f=f->next)
    if (f->size<0) size-=f->size;

  return size;
}

swptr i4_block_manager_class::allocated()
{
  swptr size=0;
  memory_node *f;
  for (f=sfirst;f;f=f->next)
    if (f->size>0) size+=f->size;

  return size;
}

void i4_block_manager_class::init(void *block, long Block_size)
{
  block_size=Block_size;
  addr=block;
  /* 
     I'm padding each block, because I'm comparing pointers against size
     in jfree to determine weither a pointer is too a small object or a large alloc
     and it must always be true that the address of the pointer is > JM_SMALL_SIZE 
     All systems I know start pointer address pretty high, but this is a porting consern.     
  */
  
  slast=sfirst=(memory_node *)(((char *)block)+JM_SMALL_SIZE);   
  sfirst->size=-(block_size-(sw32)sizeof(memory_node)-(sw32)JM_SMALL_SIZE);
  sfirst->next=NULL;
  memset(sblocks,0,sizeof(sblocks));

}

void *i4_block_manager_class::alloc(long size, char *name)
{
  if (size<JM_SMALL_SIZE)
  {
    small_block *s=sblocks[size];
    for (;s && s->alloc_list==0xffffffff;s=s->next);
    if (!s)
    {
      s=(small_block *)i4_block_manager_class::alloc((size+4)*32+sizeof(small_block),"small_block");
      if (!s) return NULL;   // not enough room for another small block
      s->alloc_list=1;
      s->next=sblocks[size];
      sblocks[size]=s;
      s->size=size;
#ifdef i4_MEM_CHECK
      s->name[0]=str_alloc(name);
#endif      
      swptr *addr=(long *)(((char *)s)+sizeof(small_block));
      *addr=(swptr)s;
      return (void *)(addr+1);  // return first block
    } else
    {
      int bit=1,i=0;
      char *addr=((char *)s)+sizeof(small_block);
      while (1)        // we already know there is a bit free
      {
	if ((s->alloc_list&bit)==0)
	{
	  s->alloc_list|=bit;
#ifdef i4_MEM_CHECK
	  s->name[i]=str_alloc(name);
#endif      	 
	  *((swptr *)addr)=(swptr)s;

	  return (void *)(addr+4);
	}
	i++;
	bit=bit<<1;
	addr+=size+4;
      }      
    }                
  }


  memory_node *s=sfirst;
  if (!s) return NULL;
  for (;s && -s->size<size;s=s->next);  
  if (!s) return NULL;
  s->size=-s->size;

  if (s->size-size>sizeof(memory_node)+4)  // is there enough space to split the block?
  {    
    memory_node *p=(memory_node *)((char *)s+sizeof(memory_node)+size);
    if (s==slast)
      slast=p;
    p->size=-(s->size-size-(sw32)sizeof(memory_node));
#ifdef i4_MEM_CLEAR
    memset( ((memory_node *)p)+1,0,-p->size);
#endif
    p->next=s->next;
    s->next=p;
    s->size=size;
  }
#ifdef i4_MEM_CHECK
  s->name=str_alloc(name);
#endif
  return (void *)(((char *)s)+sizeof(memory_node));
}


int i4_show_frees=0;

/************************** FREE **********************************/
/*    should be called to free a pointer in the static heap       */
/*    i.e. begining of the heap                                   */
/******************************************************************/
void i4_block_manager_class::free(void *ptr)
{
  // see if this was a small_block allocation
  void *next=(void *)(*(((swptr *)ptr)-1));
  if (next && next<ptr)  // small allocation
  {
    small_block *s=(small_block *)next;
    if (s->size<=0)
    {
      i4_warning("i4_free : bad pointer\n");
      return ;
    }
#ifdef i4_MEM_CLEAR
    memset(ptr,0,s->size);
#endif

    swptr field=(((char *)ptr)-((char *)s)-sizeof(small_block))/(s->size+4);
#ifdef i4_MEM_CHECK
    if (i4_show_frees)
      i4_warning("small free : %s",s->name[field]);
    str_free(s->name[field]);
#endif
    s->alloc_list&=(0xffffffff-(1<<field));
    if (s->alloc_list==0)
    {
      small_block *l=NULL;
      small_block *n=sblocks[s->size];
      for (;n!=s;n=n->next) l=n;
//#ifdef i4_MEM_CHECK
      if (!n) 
		  { 
		  i4_warning("Free small block error\n"); 
		  return;
		  }
//#endif
      if (!l)
      sblocks[s->size]=s->next;
      else l->next=s->next;
      i4_block_manager_class::free(s);
    }      
  } else
  {
    memory_node *o=(memory_node *)(((char *)ptr)-sizeof(memory_node)),*last=NULL;
#ifdef i4_MEM_CHECK
    if (i4_show_frees)
      i4_warning("big free : %s",o->name);
    str_free(o->name);
#endif
#ifdef i4_MEM_CLEAR
    memset(ptr,0,o->size);
#endif

    if (o->next && o->next->size<0)   // see if we can add into next block
    {
      if (o->next==slast)
        slast=o;
      o->size+=-o->next->size+sizeof(memory_node);
      o->next=o->next->next;
    }

    memory_node *n=sfirst;
    for (;n && n!=o;n=n->next) 
		{
		last=n;
		}
//#ifdef i4_MEM_CHECK
    if (!n) 
		{ 
		i4_warning("Free big block error. Already freed?\n"); 
		return;
		}
//#endif
    
    if (last && last->size<0)
    {
      if (o==slast) {
        slast=last;
      }
      last->next=o->next;
      last->size-=o->size+sizeof(memory_node);	
    } else o->size=-o->size;            
  }  
}
