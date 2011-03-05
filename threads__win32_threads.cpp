#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "threads/threads.h"
#include "error/error.h"
#include "init/init.h"
#include "main/main.h"

#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <windowsx.h>
static w32 i4_thread_count=0;
static i4_critical_section_class i4_thread_lock;
static i4_critical_section_class thread_list_lock;

struct thread_node
{
	HANDLE h;
	w32 thread_id;
	thread_node * next;
	void * base, * top;

	thread_node(HANDLE h, w32 thread_id, void * base, void * top, thread_node * next)
		: h(h),
		  thread_id(thread_id),
		  next(next),
		  base(base),
		  top(top) {
	}

};

static thread_node * thread_list=0;

int i4_main_thread_id;

void i4_wait_threads()  // waits for all threads to terminate (don't call from a thread!)
{
	while (i4_thread_count!=0)
		i4_thread_yield();


}



static i4_thread_func_type i4_thread_to_start=0;
static int i4_thread_size;

void remove_thread(int id)
{
	thread_list_lock.lock();

	thread_node * p=0;
	if (thread_list->thread_id==(w32)id)
	{
		p=thread_list;
		thread_list=thread_list->next;
	}
	else
	{
		thread_node * q;
		for (q=thread_list; q->next->thread_id!=(w32)id; q=q->next)
		{
			;
		}
		p=q->next;
		q->next=p->next;
	}

	CloseHandle(p->h);
	delete p;
	thread_list_lock.unlock();
}

//! This function ensures the operating system really commited the memory for the stack.
//! Failing to call this might cause exceptions on Windows 7 during gc runs, as the garbage collector doesn't know how far the stack really goes at that point.
void commit_stack(int size)
{
	char* p=(char*)_alloca(size);
	for (int i=0;i<size;i++)
	{
		*p=0;
	}
}

void i4_thread_starter(void * arg)
{
	i4_thread_func_type start=i4_thread_to_start;
	int size=i4_thread_size;
	commit_stack(size);
	size-=200;
	i4_thread_to_start=0;

	thread_list_lock.lock();
	w32 thread_id=GetCurrentThreadId();

	HANDLE h;
	DuplicateHandle(GetCurrentProcess(),
					GetCurrentThread(),
					GetCurrentProcess(),
					&h,
					DUPLICATE_SAME_ACCESS,
					FALSE,
					DUPLICATE_SAME_ACCESS);


	thread_node * p=new thread_node(h, thread_id,
									(void *)&size,
									(void *)(((char *)&size)-size),
									thread_list);


	thread_list=p;
	thread_list_lock.unlock();



	start(arg);

	remove_thread(p->thread_id);

	i4_thread_lock.lock();
	i4_thread_count--;
	i4_thread_lock.unlock();

	_endthread();
}

void i4_add_thread(i4_thread_func_type fun, w32 stack_size, void * arg_list)

{
	while (i4_thread_to_start!=0)
		i4_thread_yield();



	i4_thread_to_start=fun;
	i4_thread_size=stack_size;

	i4_thread_lock.lock();
	i4_thread_count++;
	i4_thread_lock.unlock();

	_beginthread(i4_thread_starter, stack_size, arg_list);
}

void i4_thread_sleep(w32 ms)
{
	Sleep(ms);
}

void i4_thread_yield()
{
	Sleep(0);
}


int i4_get_thread_id()
{
	return GetCurrentThreadId();
}


int i4_exec(const i4_const_str &cmdline, const i4_bool modal, const i4_const_str &title)
{
	//pass null as title if creating gdi or unknown process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&pi,sizeof(pi));
	char tbuf[100];
	char cmdbuf[500];
	i4_os_string(cmdline,cmdbuf,500);
	if (!title.null())
	{
		i4_os_string(title,tbuf,100);
	}
	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(si);
	si.lpReserved=0;
	si.lpDesktop=0;
	si.lpTitle=tbuf;
	si.dwX=si.dwY=0;
	si.dwXSize=si.dwYSize=200;
	si.dwXCountChars=80;
	si.dwYCountChars=50;
	si.dwFlags=STARTF_USECOUNTCHARS;
	if (!CreateProcess(NULL,cmdbuf,NULL,
					   NULL,FALSE,0,
					   NULL,NULL,&si,&pi))
	{
		i4_error("ERROR: CreateProcess() failed.");
		return 0;
	}
	if (modal)
	{
		WaitForSingleObject(pi.hProcess,INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	//How to close the Handles if we run asynchronously?
	return 1;
}

void i4_suspend_other_threads()
{
	thread_list_lock.lock();

	w32 thread_id=GetCurrentThreadId();
	for (thread_node * p=thread_list; p; p=p->next)
	{
		if (p->thread_id!=thread_id)
		{
			SuspendThread(p->h);
		}
	}

	thread_list_lock.unlock();
}

void i4_resume_other_threads()
{
	thread_list_lock.lock();

	w32 thread_id=GetCurrentThreadId();
	for (thread_node * p=thread_list; p; p=p->next)
	{
		if (p->thread_id!=thread_id)
		{
			ResumeThread(p->h);
		}
	}

	thread_list_lock.unlock();
}


int i4_get_main_thread_id()
{
	return i4_main_thread_id;
}

i4_bool i4_get_first_thread_id(int &id);
i4_bool i4_get_next_thread_id(int last_id, int &id);

int i4_get_first_thread_id()
{
	return thread_list->thread_id;
}

i4_bool i4_get_next_thread_id(int last_id, int &id)
{
	for (thread_node * p=thread_list; p; p=p->next)
	{
		if (p->thread_id==(w32)last_id)
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


void i4_get_thread_stack(int thread_id, void *&base, void *&top)
{
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
		for (thread_node * p=thread_list; p; p=p->next)
		{
			if (p->thread_id==(w32)thread_id)
			{
				base=p->base;
				top=p->top;
			}
		}
	}
}



class thread_initer :
	public i4_init_class
{
public:
	virtual int init_type()
	{
		return I4_INIT_TYPE_THREADS;
	}
	void init()
	{
		i4_main_thread_id=i4_get_thread_id();
		HANDLE h;
		DuplicateHandle(GetCurrentProcess(),
						GetCurrentThread(),
						GetCurrentProcess(),
						&h,
						DUPLICATE_SAME_ACCESS,
						FALSE,
						DUPLICATE_SAME_ACCESS);

		thread_list=new thread_node(h, i4_main_thread_id,
									i4_stack_base, i4_stack_base, 0);
	}

	void uninit()
	{
		remove_thread(i4_main_thread_id);
	}

};

static thread_initer thread_initer_instance;



////////////// Critical section stuff
i4_critical_section_class::i4_critical_section_class()
{
	cs=(void *) malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection((RTL_CRITICAL_SECTION *)cs);
}

void I4_FAST_CALL i4_critical_section_class::lock()
{
	EnterCriticalSection((RTL_CRITICAL_SECTION *)cs);
/*  __asm
   { //Wie soll das funktionieren??? Woher ist ecx bekannt??
   	start:
   	  bts [ecx], 0
   	  jnc success
   	  call i4_thread_yield   // give up our time-slice
   	  jmp start
   	success:
   }*/
}

void I4_FAST_CALL i4_critical_section_class::unlock()
{
	//__asm mov [ecx], 0
	LeaveCriticalSection((RTL_CRITICAL_SECTION *)cs);
}

i4_critical_section_class::~i4_critical_section_class()
{
	DeleteCriticalSection((RTL_CRITICAL_SECTION *)cs);
	free(cs);
	cs=NULL;
}


///////////// signal stuff
i4_signal_object::i4_signal_object(char * name)
{
	char buf[100];

	sprintf(buf,"%s-%d",name,_getpid());  // make sure name doesn't interfere with other
	*((HANDLE *)data)=CreateSemaphore(0, 0, 1, buf); // processes using this library
}

i4_signal_object::i4_signal_object(char * name, w32 dwMaxCount, w32 dwInitialCount)
{
	char buf[100];

	sprintf(buf,"%s-%d",name,_getpid());
	*((HANDLE *)data)=CreateSemaphore(0,dwInitialCount,dwMaxCount,buf);
}

i4_bool i4_signal_object::wait_signal()
{
	WaitForSingleObject(*((HANDLE *)data), INFINITE);
	return i4_T;
}

i4_bool i4_signal_object::wait_signal(w32 dwMillisecs)
{
	if (WaitForSingleObject(*((HANDLE *)data),dwMillisecs)==WAIT_TIMEOUT)
	{
		return i4_F;
	}
	return i4_T;
}



void i4_signal_object::signal()
{
	ReleaseSemaphore(*((HANDLE *)data), 1, 0);
}

i4_signal_object::~i4_signal_object()
{
	CloseHandle(* ((HANDLE *)data));
}

i4_bool i4_threads_supported()
{
	return i4_T;
}


void i4_set_thread_priority(int thread_id, i4_thread_priority_type priority)
{
	thread_list_lock.lock();

	thread_node * p;
	for (p=thread_list; p && p->thread_id!=(w32)thread_id; p=p->next)
	{
		;
	}
	if (p)
	{
		switch (priority)
		{
			case I4_THREAD_PRIORITY_HIGH:
				SetThreadPriority(p->h, THREAD_PRIORITY_HIGHEST);
				break;

			case I4_THREAD_PRIORITY_NORMAL:
				SetThreadPriority(p->h, THREAD_PRIORITY_NORMAL);
				break;

			case I4_THREAD_PRIORITY_LOW:
				SetThreadPriority(p->h, THREAD_PRIORITY_BELOW_NORMAL);
		}
	}

	thread_list_lock.unlock();
}
