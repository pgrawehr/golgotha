/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_ASYNC_READ_HH
#define I4_ASYNC_READ_HH

// This class should probably only be used by files in "i4/files/*"

// This class is a portable implemented of async file reading
// A thread gets created during init() which runs as long as
// there is stuff to read and then blocks for a signal from the main
// program so if nothing needs reading, it runs efficiently.  Request
// to be read are qued up (max of 16).  The request are processed
// serially.  If you want to read from multiple devices in parallel,
// you should create two i4_async_reads, one for each device.

#include "memory/dynque.h"
#include "threads/threads.h"
#include "init/init.h"
#include "file/file.h"

class i4_async_reader : public i4_init_class
{
  volatile i4_bool stop;
  i4_signal_object sig;

  i4_bool emulation;
	public:
  struct read_request
  {
    sw32 fd;
    w32 size;
    void *buffer;
    i4_file_class::async_callback callback;
    void *context;
	w32 prio;
    read_request(sw32 fd, void *buffer,
                 w32 size, i4_file_class::async_callback callback, 
                 void *context, w32 prio)
      : fd(fd), buffer(buffer), size(size), callback(callback), 
	  context(context), prio(prio) {}
    read_request() { ; }
  };
	  protected:
  enum { MAX_REQUEST=32 };
  enum { STACK_SIZE=8096 };
  i4_critical_section_class que_lock;
  i4_dynamic_que<read_request, 0, MAX_REQUEST> request_que;
  static volatile w32 num_pending;
  void emulate_speeds(read_request &r);

protected:

  virtual w32 read(sw32 fd, void *buffer, w32 count) = 0;
  
#ifdef _WINDOWS
  DWORD hPRIVATE_thread;
#else
  w32 hPRIVATE_thread;
#endif

public:
	virtual w32 max_priority()=0;//the maximum priority this reader should handle
  // name is just some unique name.  Windows requires this for Semaphores
  i4_async_reader(char *name);

  void init();   // creates thread (called by i4_init()
  void uninit(); // waits for thread to die (called by i4_uninit()
  static i4_bool is_idle()
	  {return num_pending==0;}

  // ques up a request
  i4_bool start_read(int fd, void *buffer, w32 size, 
                     i4_file_class::async_callback call,
                     void *context, w32 priority=255);

  void PRIVATE_thread();  // don't call this!
};


#endif
