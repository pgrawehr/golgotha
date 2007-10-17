/**********************************************************************
   	Golgotha Forever - A portable, free 3D strategy and FPS game.
   	Copyright (C) 1999 Golgotha Forever Developers
   	Sources contained in this distribution were derived from
   	Crack Dot Com's public release of Golgotha which can be
   	found here:  http://www.crack.com
   	All changes and new works are licensed under the GPL:
   	This program is free software; you can redistribute it and/or modify
   	it under the terms of the GNU General Public License as published by
   	the Free Software Foundation; either version 2 of the License, or
   	(at your option) any later version.
   	This program is distributed in the hope that it will be useful,
   	but WITHOUT ANY WARRANTY; without even the implied warranty of
   	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   	GNU General Public License for more details.
   	You should have received a copy of the GNU General Public License
   	along with this program; if not, write to the Free Software
   	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   	For the full license, see COPYING.
 ***********************************************************************/

#include "pch.h"
//#include <stdio.h>
//#ifdef __linux
//includes pthread_mutexattr_settype()
//Dumb: In my linux distro, declaring this would require symbols which
//are declared nowhere... (Mandrake 8.1)
//#define __USE_UNIX98
//#endif
#include <pthread.h>
#include "error/error.h"
#include "threads/threads.h"
#include "init/init.h"
#include "main/main.h"
#include <sched.h>
#include <semaphore.h>
static volatile w32 i4_thread_count=0;
static i4_critical_section_class i4_thread_start_lock;
static i4_critical_section_class i4_thread_lock;
static volatile i4_thread_func_type i4_thread_to_start;
struct thread_node
{
	pthread_t handle;
	w32 thread_id;
	thread_node * next;
	void * base, * top;
	thread_node(pthread_t handle, w32 thread_id, void * base, void * top,
				thread_node * next) :
		handle(handle),
		thread_id(thread_id),
		next(next),
		base(base),
		top(top) {
	}
};
static volatile thread_node * thread_list=0;
//volatile thread_node *p=0;
int i4_main_thread_id;
static int i4_thread_size;

void i4_wait_threads()  // waits for all threads to terminate
						// (don't call from a thread!)
{
	//i4_warning("Waiting threads");
	while (i4_thread_count!=0)
		i4_thread_yield();


}
void remove_thread(int id)
{
	i4_thread_start_lock.lock();
	volatile thread_node * p=0;
	if (thread_list->thread_id==id)
	{
		p=thread_list;
		thread_list=thread_list->next;
	}
	else
	{
		volatile thread_node * q;
		for (q=thread_list; q->next->thread_id!=id; q=q->next)
		{
			;
		}
		p=q->next;
		q->next=p->next;
	}
	pthread_cancel(p->handle);
	delete p;
	i4_thread_start_lock.unlock();
}

void *i4_thread_starter(void * arg)
{
	//i4_warning("Inside thread");
	i4_thread_func_type start=i4_thread_to_start;
	int size=i4_thread_size; //not very safe...

	size-=200;
	i4_thread_to_start=0;
	i4_thread_start_lock.lock();
	w32 thread_id=(w32)pthread_self(); //under some os's, this returns a handle
	pthread_t handle;
	thread_node * p=new thread_node(handle, thread_id,
									(void *)&size,
									(void *)(((char *)&size)-size),
									(thread_node *) thread_list);
	thread_list=p;
	i4_thread_start_lock.unlock();

	start(arg);
	remove_thread(p->thread_id);

	i4_thread_lock.lock();
	i4_thread_count--;
	i4_thread_lock.unlock();
	//i4_warning("Exiting thread");
	pthread_exit(0);

	return 0;
}
void i4_add_thread(i4_thread_func_type fun, w32 stack_size, void * arg_list)
{
	while (i4_thread_to_start!=0)
		i4_thread_yield();


	//i4_warning("Add thread");
	i4_thread_to_start=fun;
	i4_thread_size=stack_size;
	i4_thread_lock.lock();
	i4_thread_count++;
	i4_thread_lock.unlock();
	//i4_warning("Creating thread");
	pthread_t temphandle;
	volatile thread_node * old=thread_list;
	pthread_create(&temphandle, 0, i4_thread_starter, arg_list);
	while(thread_list==old)
	{
		i4_thread_yield();
		i4_thread_sleep(1);
	}
	; //wait until p is modified (only means thread
	  //is doing something, the write to it might not be atomic, but
	  //that thread now at least has got the lock, so we wait for the lock)
	i4_thread_start_lock.lock();
	thread_list->handle=temphandle; //write back (now p is valid)
	i4_thread_start_lock.unlock();
}
void i4_thread_sleep(w32 dwMilli)
{
	usleep(dwMilli*1000);
	sched_yield();
}
int i4_exec(const i4_const_str &cmdline, const i4_bool modal,
			const i4_const_str &title)
{
	int i;

	i=fork();
	char buf[MAX_PATH];
	i4_os_string(cmdline,buf,MAX_PATH);
	if (i==0) //I am the child
	{
		execl(buf,buf,0);
		//if we are still running, something went wrong...
		i4_error("FATAL: Executing child process failed after fork(). Child is killing itself.");
	}
	return i4_T;
}

void i4_thread_yield()
{
	//i4_warning("Thread yielding");
	sched_yield();
}
int i4_get_thread_id()
{
	//i4_warning("Get_thread_id");
	return (w32) pthread_self();
}
int i4_get_main_thread_id()
{
	//i4_warning("Get main thread");
	return i4_main_thread_id;
}
i4_bool i4_get_first_thread_id(int &id);
//cannot just return the master thread since that one is the last in the list
int i4_get_first_thread_id()
{
	return thread_list->thread_id;
}

i4_bool i4_get_next_thread_id(int last_id, int &id);
i4_bool i4_get_next_thread_id(int last_id, int &id)
{
	//i4_warning("Get next thread id");
	for (volatile thread_node * p=thread_list; p; p=p->next)
	{
		if (p->thread_id==last_id)
		{
			if (p->next)
			{
				id=p->next->thread_id;
				return i4_T;
			}
			else
			{
				return i4_F;
			}
		}
	}
	return i4_F;
}
void i4_suspend_other_threads()
{
	//i4_warning("Suspend other threads");
	i4_thread_start_lock.lock();
	w32 thread_id=(w32)pthread_self();
	for (volatile thread_node * p=thread_list; p; p=p->next)
	{
		if (p->thread_id!=thread_id)
		{
			//i4_warning("I should be suspending this thread");
		}
	}
	i4_thread_start_lock.unlock();
}
void i4_resume_other_threads()
{
	//i4_warning("Resume other threads");
	i4_thread_start_lock.lock();
	w32 thread_id=(w32)pthread_self();
	for (volatile thread_node * p=thread_list; p; p=p->next)
	{
		if (p->thread_id!=thread_id)
		{
			//i4_warning("I should be resuming this thread");
		}
	}
	i4_thread_start_lock.unlock();
}
void i4_get_thread_stack(int thread_id, void *&base, void *&top)
{
	//i4_warning("Get thread stack");
	base=0;
	top=0;
	if (thread_id==i4_main_thread_id)
	{
		base=i4_stack_base;
		if (i4_get_thread_id()!=i4_main_thread_id)
		{
			i4_error("Can't get main thread stack from thread");
		}

		int t;
		top=(void *)(&t);
	}
	else
	{
		for (volatile thread_node * p=thread_list; p; p=p->next)
		{
			if (p->thread_id==thread_id)
			{
				base=p->base;
				top=p->top;
			}
		}
	}
}
i4_bool i4_threads_supported()
{
	return i4_T;
}
class pthreads_init :
	public i4_init_class
{
public:
	virtual int init_type()
	{
		return I4_INIT_TYPE_THREADS;
	}
	void init()
	{
		//i4_warning("Init threads");
		i4_main_thread_id=(w32)pthread_self();
		thread_list=new thread_node(0, i4_main_thread_id,
									i4_stack_base, i4_stack_base, 0);
	}
	void uninit()
	{
		//i4_warning("UnInit threads");
		//don't attempt to kill the main thread. (will leave a small
		//memory leak, but who cares..?)
		//remove_thread(i4_main_thread_id);
	}
} pthread_initer_instance;
void i4_set_thread_priority(int thread_id, i4_thread_priority_type priority)
{
	//i4_warning("Set thread priority");
}

// Critical section (mutexes)
//PTHREAD_MUTEX_RECURSIVE_NP=1 (at least under linux)
//#ifdef __sgi
//pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
//#else//currently, this case is for linux, will perhaps change, too.
//pthread_mutex_t mutex={0,0,0, 1, {0,0}};
//#endif
//static volatile int recursive_lock=0;
//static volatile int thread_lock_id=0;

/*
   //pthread_mutexattr_settype() has some real portability problems
   //it exists under all known unixes, but has different names and
   //parameters.
   //see the first paragraph of qmutex_unix.cpp of some Qt distribution...
 #ifdef __linux
   //These definitions come from /usr/include/pthread.h
 #define I4_MUTEX_SETTYPE(a,b) pthread_mutexattr_settype((a),(b))
   //this is embarassing. gcc 2.95 needs it, gcc 3.3 has it's own
   //definition for this...
 #ifndef __USE_UNIX98
   extern "C" int pthread_mutexattr_settype(pthread_mutexattr_t *m,int kind);
 #endif
 #define I4_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
 #else
 #define I4_MUTEX_SETTYPE(a,b) pthread_mutexattr_setkind_np((a),(b))
 #define I4_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
 #endif

 #ifdef __bsd
 #undef I4_MUTEX_RECURSIVE
 #define I4_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE
 #endif
 */

#ifndef __USE_UNIX98
extern "C" int pthread_mutexattr_settype(pthread_mutexattr_t * m,int kind);
#endif


i4_critical_section_class::i4_critical_section_class(void)
{
//  i4_warning("Init mutex");
//#ifdef __sgi
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
//pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	I4_MUTEX_SETTYPE(&attr,I4_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutex,&attr);
//#endif
}
i4_critical_section_class::~i4_critical_section_class(void)
{
	//i4_warning("Destroying Mutex");
	pthread_mutex_destroy(&m_mutex);
}
//PG: Need to examine carefully all possible cases
/*void i4_critical_section_class::lock(void)
   {
   //i4_warning("Locking Mutex");
   //This should not give trouble with concurency.  if (pthread_self()==thread_lock_id)  {  	recursive_lock++;  	return;  }  pthread_mutex_lock(&mutex);
   recursive_lock++;  thread_lock_id=pthread_self();}
 */
void i4_critical_section_class::lock(void)
{
	pthread_mutex_lock(&m_mutex);
};
/*void i4_critical_section_class::unlock(void)
   {
   //i4_warning("Unlocking Mutex");
   recursive_lock--;  if (recursive_lock==0)  {
   	pthread_mutex_unlock(&mutex);
   	thread_lock_id==0;	  };}*/

void i4_critical_section_class::unlock(void)
{
	pthread_mutex_unlock(&m_mutex);
};
// Signals
//sem_t semaphore; //this is now included in the objects
i4_signal_object::i4_signal_object(char * name,
								   w32 dwMaxCount,
								   w32 dwInitialCount)
{
	//i4_warning("Create non-default signal object");

	sem_init(&semaphore, 0, dwMaxCount);

	//decrease the object as often as required
	while(dwInitialCount<dwMaxCount)
	{
		sem_wait(&semaphore);
		dwInitialCount++;
	}
}
i4_signal_object::i4_signal_object(char * name)
{
	//i4_warning("Create signal object");
	sem_init(&semaphore, 0, 0);
}
i4_bool i4_signal_object::wait_signal()
{
	//i4_warning("Wait signal..");
	sem_wait(&semaphore);
	return i4_T;
}
i4_bool i4_signal_object::wait_signal(w32 dwMillisecs)
{
	while(sem_trywait(&semaphore)!=0)
	{
		dwMillisecs--;
		i4_thread_sleep(1);
		if (dwMillisecs<=0)
		{
			return i4_F;
		}
	}
	return i4_T;
}
void i4_signal_object::signal()
{
	//i4_warning("Signal...");
	sem_post(&semaphore);
}
i4_signal_object::~i4_signal_object()
{
	//i4_warning("Destroying signal object");
	sem_destroy(&semaphore);
}
