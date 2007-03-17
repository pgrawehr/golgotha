/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


/*

   Example ussage of threads :

   void test(void *arg)
   {
   .. do something..
   }

   my_main()
   {
   i4_add_thread(test,10*1024);
   }

 */

#ifndef I4_THREADS_HH
#define I4_THREADS_HH

#include "arch.h"
#include "string/string.h"
#ifdef I4_UNIX
#include <semaphore.h>
#include <pthread.h>
#endif
//#include "windows.h" //this would be required for CRITICAL_SECTION
//but makes sbr - Files >1.3MB each!

//the interface your threads should support
typedef void (*i4_thread_func_type)(void *context);

i4_bool i4_threads_supported();

// this will start the thread right away, regardless of how many threads are running
void i4_add_thread(i4_thread_func_type fun, w32 stack_size=50 *1024, void *context=0);

//give up your time slice
void i4_thread_yield();

//sleep some time (not to be used for syncronization)
void i4_thread_sleep(w32 ms);
void i4_wait_threads();  // waits for all threads to terminate (don't call from a thread!)
int  i4_get_thread_id();
int  i4_get_main_thread_id();

//executes an external program
int i4_exec(const i4_const_str &cmdline, const i4_bool modal, const i4_const_str &title);


void i4_suspend_other_threads();  // stops all of threads from running
void i4_resume_other_threads();   // resumes execution of other threads

int i4_get_first_thread_id();
i4_bool i4_get_next_thread_id(int last_id, int &id);
void i4_get_thread_stack(int thread_id, void *&base, void *&top);


enum i4_thread_priority_type {
	I4_THREAD_PRIORITY_HIGH,
	I4_THREAD_PRIORITY_NORMAL,
	I4_THREAD_PRIORITY_LOW
};

void i4_set_thread_priority(int thread_id, i4_thread_priority_type priority);


class i4_critical_section_class
{
protected:
	void *cs; // to store operating system depandant data, enlarge if needed
#ifdef I4_UNIX
	pthread_mutex_t m_mutex;
	int recursive;
#endif
public:
	i4_critical_section_class();
	void I4_FAST_CALL lock();
	void I4_FAST_CALL unlock();
	~i4_critical_section_class();
};


class i4_signal_object
{
protected:
#ifdef _WINDOWS
	w32 data[1]; // to store operating system dependent data, enlarge if needed
	//windows requires only one word (a handle)
	//but linux needs a whole struct
#else
	sem_t semaphore;
#endif
public:

	//Creates unsignaled object
	i4_signal_object(char *name);

	//specify dwMaxCount=dwInitialCount=1 if you want to have a mutex that is
	//initially free.
	i4_signal_object(char *name, w32 dwMaxCount, w32 dwInitialCount);

	//WaitForSingleObject()
	i4_bool wait_signal();

	//returns i4_F if cannot get hold of
	//lock in specified amount of time.
	i4_bool wait_signal(w32 dwMillisecs);

	//ReleaseSemaphore()
	void signal();
	~i4_signal_object();
};


#endif
