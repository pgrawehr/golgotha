/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ File & File Manager Classes
//
//$Id: file.cc,v 1.48 1998/07/15 21:16:39 jc Exp $
#include "pch.h"
#include "memory/malloc.h"
#include "memory/array.h"

#include "error/error.h"

#include "time/time.h"
#include "time/profile.h"

#include "string/string.h"

#include "threads/threads.h"

#include "device/device.h"
#include "device/kernel.h"

#include "main/win_main.h"

#include "window/style.h"
#include "window/window.h"
#include "window/colorwin.h"
#include "gui/text_input.h"
#include "status/status.h"

#include "file/file.h"
#include "file/file_man.h"
#include "file/async.h"
#include "file/buf_file.h"
#include "file/ram_file_man.h"
#include "file/ram_file.h"
#include "file/sub_section.h"
#include "file/get_filename.h"

#include "lisp/lisp.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WINDOWS
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#else
#ifdef __bsd
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#else
#ifdef __linux
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/io.h>
#include <dirent.h>
#else
//For instance SUN4 goes here
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif
#endif

#endif

//#define file_load_debug /*Activate to see "open failed..." and "opened file ..." messages*/


static i4_profile_class pf_tell("File tell");
static i4_profile_class pf_open("File Open");
static i4_profile_class pf_sopen("System file Open");
static i4_profile_class pf_close("File Close");
static i4_profile_class pf_read("File Read");
static i4_profile_class pf_con_buf("::buffered file");

i4_temp_file_class::i4_temp_file_class()
	{
	bs=0x10000;
	incsize=0x10000;
	//buf=(w8*) malloc(bs);
	buf=new w8[bs];
	if (!buf)
		i4_error("CRITICAL: Ups, we just ran out of Memory!");
	offset=0;
	fs=0;
	}

i4_temp_file_class::i4_temp_file_class(w32 startsize, w32 _incsize)
	{
	bs=startsize;
	incsize=_incsize;
	//buf=(w8*) malloc(bs);
	buf=new w8[bs];
	if (!buf)
		i4_error("CRITICAL: Ups, we just ran out of Memory!");
	offset=0;
	fs=0;
	}

w32 i4_temp_file_class::read(void *buffer, w32 size)
	{
	int bytesread=size;
	if ((offset+size)>fs) bytesread=fs-offset;
	if (bytesread==0) return 0;
	memcpy(buffer,buf+offset,bytesread);
	offset+=bytesread;
	return bytesread;
	}

w32 i4_temp_file_class::write(const void *buffer, w32 size)
	{
	w32 newsize=offset+size;
	if (newsize>bs)
		{
		w8 *nbuf;
		int by=0;
		if (newsize<(bs+incsize)) newsize=bs+incsize;
		nbuf=(w8*) realloc(buf,newsize);
		if (!nbuf)
			{
			i4_error("Out of memory during memory realloc");
			by=bs-fs;
			memcpy(buf+offset,buffer,by);
			fs=bs;
			return (by);
			}
		buf=nbuf;
		bs=newsize;
		}
	memcpy(buf+offset,buffer,size);
	offset+=size;
	if (offset>fs) fs=offset;
	//ASSERT(fs<=bs);
	return size;
	}



w32 i4_temp_file_class::seek(w32 _offset)
	{
	if (_offset>fs) _offset=fs;
	offset=_offset;
	return offset;
	}
i4_temp_file_class::~i4_temp_file_class()
	{
	delete buf;
	buf=0;
	}

w32 i4_ram_file_class::read (void *buffer, w32 size)
  {
    if (size>bs-offset)
      size=bs-offset;
    memcpy(buffer, buf+offset, size);
    offset+=size;
    return size;
  }

w32 i4_ram_file_class::write(const void *buffer, w32 size)
  {
    if (size>bs-offset)
      size=bs-offset;
    memcpy(buf+offset, buffer, size);
    offset+=size;
    return size;
  }

i4_ram_file_class::i4_ram_file_class(void *buffer, int buffer_size)
    : buf((w8 *)buffer), bs(buffer_size), offset(0) 
	{ ; 
	}

i4_bool i4_file_class::async_write(const void *buffer, w32 size, 
                                   async_callback call,
                                   void *context)
{
  (*call)(write(buffer, size), context);

  return i4_T;
}


i4_bool i4_file_class::async_read (void *buffer, w32 size, 
                                   async_callback call,
                                   void *context, w32 priority)
{
  (*call)(read(buffer, size), context);
  return i4_T;
}


class file_string : public i4_str
{
public:
  file_string(w16 size) : i4_str(size) {}

  char *buffer() { return (char *)ptr; }
  void set_len(w16 _len) { len = _len; }
};


i4_str* i4_file_class::read_str(w32 len)
{
  file_string *str = new file_string((w16)len);   
  len = read(str->buffer(),len);
  str->set_len((w16)len);

  return str;
}

i4_str* i4_file_class::read_counted_str()
{
  w16 len = read_16();
  return read_str(len);
}

/*
class file_write_string : public i4_const_str
{
public:
  char *buffer() { return (char *)ptr; }
};
*/

w32 i4_file_class::write_str(const i4_const_str &str)
{
  //file_write_string *s = (file_write_string *)&str;

  return write(str.c_str(), str.length());
}

w32 i4_file_class::write_counted_str(const i4_const_str &str)
{
  w32 count;

  count = write_16((w16)str.length());
  if (str.length())      
    return (count + write_str(str));
  else
    return count;
}



static inline int fmt_char(char c)
{
  if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
    return 1;
  return 0;
}

// same as fprintf, but with the addition %S is a i4_const_str *
int i4_file_class::printf(char *fmt, ...)
{
  w32 start=tell();

  va_list ap;
  va_start(ap, fmt);

  while (*fmt)
  {
    if (*fmt=='%')
    {
      char *fmt_end=fmt;
      while (!fmt_char(*fmt_end) && *fmt_end) fmt_end++;
      char f[10], out[500]; 
      memcpy(f, fmt, fmt_end-fmt+1);
      f[fmt_end-fmt+1]=0;
      out[0]=0;

      switch (*fmt_end)
      {
        case 's' : 
        {
          char *str=va_arg(ap,char *);
          write(str,strlen(str));
        } break;          
        case 'S' : 
        {
          i4_const_str *s=va_arg(ap,i4_const_str *); 
          write_str(*s);
        } break;
        case 'd' :
        case 'i' :
        case 'x' :
        case 'X' :
        case 'c' :
        case 'o' :
        {
          ::sprintf(out,f,va_arg(ap,int));
          write(out,strlen(out));
        } break;

        case 'f' :
        case 'g' :
          ::sprintf(out,f,va_arg(ap,double));
          write(out,strlen(out));
          break;

        default :
          ::sprintf(out,f,va_arg(ap,void *));
          write(out,strlen(out));
          break;
      }
      fmt=fmt_end;
      if (*fmt)
        fmt++;
    }
    else
    {
      write_8(*fmt);
      fmt++;
    }
  }
  va_end(ap);

  return tell()-start;
}




int i4_file_class::write_format(char *format, ...)
{
  char *f=format;
  va_list ap;
  va_start(ap, format);

  int start=tell();
  while (*f)
  {
    switch (*f)
    {
      case '1' : write_8(*(va_arg(ap,     w8 *))); break;
      case '2' : write_16(*(va_arg(ap,    w16 *))); break;
      case '4' : write_32(*(va_arg(ap,     w32 *))); break;
      case 'f' : write_float(*(va_arg(ap,  float *))); break;
      case 'S' : write_counted_str(  *(va_arg(ap, i4_const_str *))); break;
    }
    f++;
  }
  va_end(ap);

  return tell()-start;
}


// format is same as write_format, returns number of fields written
int i4_file_class::read_format(char *format, ...)
{
  char *f=format;
  va_list ap;
  va_start(ap, format);

  int start=tell();
  while (*f)
  {
    switch (*f)
    {
      case '1' : *(va_arg(ap,   w8 *))=read_8(); break;
      case '2' : *(va_arg(ap,  w16 *))=read_16(); break;
      case '4' : *(va_arg(ap,  w32 *))=read_32(); break;
      case 'f' : *(va_arg(ap, float*))=read_float(); break;
      case 'S' : *(va_arg(ap, i4_str **))=read_counted_str(); break;
    }
    f++;
  }
  va_end(ap);

  return tell()-start;
}


// returns NULL if unable to open file
i4_file_class *i4_open(const i4_const_str &name, w32 flags)
{//Scans for the file using an iterator over file-Managers! Not over directories(?)
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
  {    
    i4_file_class *fp=m->open(name,flags);
    if (fp) 
      return fp;
  }

  return 0;
}

// return i4_F on failure
i4_bool i4_unlink(const i4_const_str &name)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->unlink(name))
      return i4_T;
  return i4_F;
}


// returns i4_F if file does not exsist
i4_bool i4_get_status(const i4_const_str &filename, 
                      i4_file_status_struct &return_stat)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->get_status(filename, return_stat))
      return i4_T;

  return i4_F;
}

// return i4_F on failure
i4_bool i4_mkdir(const i4_const_str &name)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->mkdir(name))
      return i4_T;

  return i4_F;
}

// returns i4_F on failure (disk full, target dir inexistent...)
i4_bool i4_copy_file(const i4_const_str &source, const i4_const_str &dest)
	{
	i4_file_class *src=i4_open(source,I4_READ);
	if (!src) return i4_F;
	i4_file_class *dst=i4_open(dest,I4_WRITE);
	if (!dst)
		{
		delete src;
		return i4_F;
		}
	w8 *buf=new w8[2000];
	w32 inbytes=0;
	w32 outbytes=0;
	inbytes=src->read(buf,2000);
	while (inbytes>0)
		{
		outbytes=dst->write(buf,inbytes);
		if (outbytes!=inbytes) 
			{
			delete src;
			delete buf;
			delete dst;
			i4_unlink(dest);
			return i4_F;
			}
		inbytes=src->read(buf,2000);
		}
	delete buf;
	delete src;
	delete dst;
	return i4_T;
	}


// returns i4_F if path is bad (tfiles and tdirs will be 0 as well)
// you are responsible for deleting both the array of strings and each string in the array
// file_status is a pointer to an array of file_status's that will be created, you
// must free these as well.  file_status may be 0 (default), in which case no array is created

i4_bool i4_get_directory(const i4_const_str &path, 
                         i4_directory_struct &dir_struct,
                         i4_bool get_status,
                         i4_status_class *status)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->get_directory(path, dir_struct, get_status, status)) 
      return i4_T;

  return i4_F;
}

// returns i4_F if path cannot be split
i4_bool i4_split_path(const i4_const_str &name, i4_filename_struct &fname_struct)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->split_path(name, fname_struct))
      return i4_T;

  return i4_F;
}

// return 0 if full path cannot be determined
i4_str *i4_full_path(const i4_const_str &relative_name)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
  {
    i4_str *s=m->full_path(relative_name);
    if (s) 
        return s;
  }

  return 0;
}


i4_file_manager_class *i4_file_manager_class::first=0;

void i4_add_file_manager(i4_file_manager_class *fman, i4_bool add_front)
{
  if (add_front)
  {
    fman->next=i4_file_manager_class::first;
    i4_file_manager_class::first=fman;
  }
  else
  {
    i4_file_manager_class *last=0, *p=i4_file_manager_class::first;
    while (p)
    {
      last=p;
      p=p->next;
    }

    if (!last) 
      i4_add_file_manager(fman, i4_T);
    else 
    {
      last->next=fman;
      fman->next=0;
    }
  }
}

void i4_remove_file_manger(i4_file_manager_class *fman)
{
  i4_file_manager_class *last=0, *p=i4_file_manager_class::first;
  while (p && p!=fman)
  {
    last=p;
    p=p->next;
  }

  if (p!=fman)
    i4_error("unable to find file manager");
  else if (last)
    last->next=fman->next;
  else
    i4_file_manager_class::first=fman->next;   
}




i4_bool i4_file_manager_class::split_path(const i4_const_str &name, i4_filename_struct &fn)
{
  char buf[512];
  i4_os_string(name, buf, 512);


  char *p=buf, *last_slash=0, *last_dot=0, *q;

  while (*p)
  {
    if (*p=='/' || *p=='\\')
      last_slash=p;
    else if (*p=='.')
      last_dot=p;
    p++;
  }

 
  if (last_dot)
  {
    q=fn.extension;    
    for (p=last_dot+1; *p; )
      *(q++)=*(p++);
    *q=0;
  }
  else last_dot=p;
      
  
  if (last_slash)
  {
    q=fn.path;
    for (p=buf; p!=last_slash; )
      *(q++)=*(p++);
    *q=0;
    last_slash++;
  }
  else last_slash=buf;


  q=fn.filename;
  for (p=last_slash; p!=last_dot;)
    *(q++)=*(p++);
  *q=0;


  return i4_T;
}



i4_directory_struct::~i4_directory_struct()
{
  int i;

  for (i=0; i<(int)tfiles; i++)
    delete files[i];

  if (files) 
    i4_free(files);

  for (i=0; i<(int)tdirs; i++)
    delete dirs[i];

  if (dirs)
    i4_free(dirs);


  if (file_status)
    i4_free(file_status);

}



static char convert_slash(char c)
{
  if (c=='\\') return '/';
  else return c;
}

i4_str *i4_relative_path(const i4_const_str &path)
{
  i4_str *full_path=i4_full_path(path);
  i4_str *full_current=i4_full_path(i4_const_str("."));
  
  char full[512], current[512], ret[512];

  i4_os_string(*full_path, full, 512);
  i4_os_string(*full_current, current, 512);

  delete full_path;
  delete full_current;


  // files are on different drives  
  if (full[1]==':' && current[1]==':' && full[0]!=current[0])
    return new i4_str(full);

  // one files is on a network drive and one is local
  if ((full[1]==':' && current[1]!=':') ||
      (full[1]!=':' && current[1]==':'))
    return new i4_str(full);


  int start=0;
  while (convert_slash(full[start])==convert_slash(current[start]))
    start++;


  char *c=current+start;
  if (*c==0)
    return new i4_str(full+start+1);

  strcpy(ret,"../");
  while (*c)
  {
    if (convert_slash(*c)=='/')
      strcpy(ret+strlen(ret),"../");

    c++;
  }

  strcpy(ret+strlen(ret), full+start);
  return new i4_str(ret);
}

// ASYNC.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

int i4_async_priority_sorter(const i4_async_reader::read_request *a,
							 const i4_async_reader::read_request *b)
	{
	if (a->prio < b->prio) 
		return -1;
	if (a->prio > b->prio)
		return 1;
	return 0;
	}

volatile w32 i4_async_reader::num_pending=0;

i4_async_reader::i4_async_reader(char *name) : sig(name), 
request_que(i4_async_priority_sorter)  
{ 
  emulation=i4_F; 
  stop=i4_F;
}

void i4_async_reader_thread_start(void *arg)
{
  ((i4_async_reader *)arg)->PRIVATE_thread();
}

void i4_async_reader::init()
{
  if (i4_threads_supported())
  {
    stop=i4_T;
    i4_add_thread(i4_async_reader_thread_start, STACK_SIZE, this);
  }
}

void i4_async_reader::uninit()
{
  if (i4_threads_supported())
  {
    while (stop==i4_T)       // wait to thread is read to be stopped
      i4_thread_yield();
  
    stop=i4_T;
    sig.signal();
    while (stop&&hPRIVATE_thread)//if the thread isn't running yet, don't wait.
		{
      i4_thread_yield();
#ifdef _WINDOWS
	  WaitForSingleObject((HANDLE)hPRIVATE_thread,INFINITE);
#else
	  i4_thread_sleep(1000);//for systems wich don't support waiting for a thread
	  //(at least I don't know)
#endif
		}
	request_que.reset();//must be uninited before the memman goes down.
  }
}

i4_bool i4_async_reader::start_read(int fd, void *buffer, w32 size, 
                                    i4_file_class::async_callback call,
                                    void *context, w32 priority)
{
  //if (!i4_threads_supported())
  //  i4_error("FATAL: Threads are not supported in this OS. Check your makefile.");

  que_lock.lock();

  /*if (request_que.size()>=MAX_REQUEST)//Otherwise, we'll soon have infinite requests pending
	  {
	  que_lock.unlock();
	  return i4_F;
	  }
  */
  read_request r(fd, buffer, size, call, context, priority);
  if (!request_que.insert(r))
  {
    que_lock.unlock();
    return i4_F;
  }
  num_pending++;
  que_lock.unlock();

  sig.signal();
  return i4_T;
}


void i4_async_reader::emulate_speeds(read_request &r)
{    
  if (emulation) 
  {
    i4_time_class now, start;
    while (start.milli_diff(now) < (sw32)r.size*1000/(1000*1024) + 20*1000)
    {
      i4_thread_yield();
      now.get();
    }
  }
}


void i4_async_reader::PRIVATE_thread()
{
  read_request r;
  sw32 amount;
  stop=i4_F;
#ifdef _WINDOWS
  hPRIVATE_thread=GetCurrentThreadId();
#else
  hPRIVATE_thread=1;//non-null value
#endif
  do
  {
    while (request_que.empty())   // if no more request to satisfy, wait for main process signal
    {
      //stop=i4_F;
	  if (stop) break;
      sig.wait_signal();
    }

    if (!stop)
    {
      que_lock.lock();

      if (request_que.deque(r))
      {
        que_lock.unlock();
        emulate_speeds(r);

        amount = read(r.fd, r.buffer, r.size);          
        r.callback(amount, r.context);
		num_pending--;
      }
      else que_lock.unlock();
    }

  } while (!stop);
#ifndef _WINDOWS
  hPRIVATE_thread=0;//to signal the parent we quitted
#endif
  stop=i4_F;
}
// buf_file
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/




w32 i4_buffered_file_class::read(void *buffer, w32 size)
{
  w32 total_read=0;
  while (size)
  {
    if (offset>=buf_start && offset<buf_end)
    {
      w32 copy_size;
      if (buf_end-offset<size)
        copy_size=buf_end-offset;
      else copy_size=size;
      memcpy(buffer,((w8 *)buf)+offset-buf_start,copy_size);
      size-=copy_size;
      buffer=(void *)(((w8 *)buffer)+copy_size);
      offset+=copy_size;
      total_read+=copy_size;
    } else if (offset==buf_end)      // sequentially read more into the buffer
    {
      buf_start=offset;
      buf_end=offset+from->read(buf,buf_size);
      if (buf_end==buf_start)
        return total_read;
    } else                          // need to seek from file to a new spot
    {
      from->seek(offset);
      buf_start=buf_end=offset;
    }
  }
  return total_read;
}


w32 i4_buffered_file_class::write(const void *buffer, w32 size)
{
  w32 total_write=0;

  while (size)
  {      
    write_file=i4_T;
    if (offset>=buf_start && offset<=buf_end)
    {
      w32 copy_size;
      if (offset+size<buf_start+buf_size)
        copy_size=size;
      else
        copy_size=buf_start+buf_size-offset;

      memcpy(((w8 *)buf)+offset-buf_start,buffer,copy_size);

      size-=copy_size;
      buffer=((const w8 *)buffer)+copy_size;
      offset+=copy_size;
      total_write+=copy_size;

      if (offset>buf_end)
      {
        buf_end=offset;
        if (buf_end-buf_start==buf_size)
        {
          from->write(buf, buf_end-buf_start);
          buf_start=buf_end;
        }
      }

    }
    else if (buf_end!=buf_start)      // flush the buffer
    {
      from->write(buf, buf_end-buf_start);
      buf_start=buf_end;
    } else
    {
      from->seek(offset);
      buf_start=buf_end=offset;
    }
  }

  return total_write;
}


w32 i4_buffered_file_class::seek (w32 offset)
{
  i4_buffered_file_class::offset=offset;
  return offset;
}


w32 i4_buffered_file_class::size ()
{
  return from->size();
}


w32 i4_buffered_file_class::tell ()
{
  return offset;
}


i4_buffered_file_class::~i4_buffered_file_class()
{
  if (write_file && buf_start!=buf_end)
    from->write(buf, buf_end-buf_start);

  delete from;
  i4_free(buf);
}


i4_buffered_file_class::i4_buffered_file_class(i4_file_class *from, 
                                               w32 buffer_size,
                                               w32 current_offset)
  : from(from), buf_size(buffer_size), offset(current_offset)
{
  write_file=i4_F;
  buf=I4_MALLOC(buf_size,"file buffer");
  buf_start=buf_end=0;
}

struct callback_context
{
  i4_bool in_use;
  w32 prev_read;
  void *prev_context;
  i4_file_class::async_callback prev_callback;
  i4_buffered_file_class *bfile;
} ;


// Maximum number async reads going on at the same time
enum { MAX_ASYNC_READS = 4 };
static callback_context contexts[MAX_ASYNC_READS];
static w32 t_callbacks_used=0;

void i4_async_buf_read_callback(w32 count, void *context)
{
  callback_context *c=(callback_context *)context;
  c->bfile->offset+=count;

  i4_file_class::async_callback call=c->prev_callback;
  count += c->prev_read;
  void *ctext=c->prev_context;
  c->in_use=i4_F;

  t_callbacks_used--;
  
  call(count, ctext);
}



i4_bool i4_buffered_file_class::async_read (void *buffer, w32 size, 
                                            async_callback call,
                                            void *context, w32 priority)
{
  if (!(offset>=buf_start && offset<buf_end))
  {
    from->seek(offset);
    buf_start=buf_end=0;
  }


  if (t_callbacks_used>=MAX_ASYNC_READS)
    return i4_file_class::async_read(buffer, size, call, context,priority);
  else
  {    
    w32 avail_size;

    if (offset>=buf_start && offset<buf_end)    
      avail_size=buf_end-offset;
    else
      avail_size=0;

    if (avail_size < size)
    {
      callback_context *c=0;
      for (w32 i=0; !c && i<MAX_ASYNC_READS; i++)
        if (!contexts[i].in_use)
        {
          c=contexts+i;
          c->in_use=i4_T;
        }

      if (c==0)
        i4_error("didn't find a free context");

      t_callbacks_used++;

      if (avail_size)
        c->prev_read=read(buffer,avail_size);
      else
        c->prev_read=0;

      c->prev_context=context;
      c->prev_callback=call;
      c->bfile=this;
      return from->async_read((w8 *)buffer + avail_size, size-avail_size, 
                              i4_async_buf_read_callback, c,priority);
    }
    else
    {
      call(read(buffer,avail_size), context);
      return i4_T;
    }

  }
}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}

// RAM_FILE_MAN.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


i4_openable_ram_file_info *i4_flist=0;

class i4_ram_file_manager_class : public i4_file_manager_class
{  
public: 

  virtual i4_file_class *open(const i4_const_str &name, w32 flags)
  {
    if (flags==I4_READ)
    {
      char buf[256];
      i4_os_string(name, buf, 256);
//wo sucht der? Wirklich nur im aktuellen Verzeichnis?
      for (i4_openable_ram_file_info *f=i4_flist; f; f=f->next)
        if (strcmp(buf, f->filename)==0)
          return new i4_ram_file_class(f->data, f->data_size);         
    }

    return 0;
  }

  i4_ram_file_manager_class()
  {
    i4_add_file_manager(this, i4_F);
  }

  ~i4_ram_file_manager_class()
  {
    i4_remove_file_manger(this);
  }
} i4_ram_file_manager_instance;

i4_openable_ram_file_info::i4_openable_ram_file_info(char *filename, void *data, w32 data_size)
  : filename(filename), data(data), data_size(data_size)
{ 
  next=i4_flist;
  i4_flist=this;
}
// SUB_SECTION.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



w32 i4_sub_section_file::read (void *buffer, w32 rsize)
{
  if( foffset-fstart+rsize > (w32)fsize )  // don't allow reads past the sub section
    rsize=fstart+fsize-foffset;

  int rs=fp->read(buffer, rsize);
  foffset+=rs;
  return rs;
}

w32 i4_sub_section_file::write(const void *buffer, w32 size)
{
  i4_error("don't write here");
  return 0;
}


w32 i4_sub_section_file::seek(w32 offset)
{
  if( offset >= (w32)fsize )
    offset=fsize-1;
  
  foffset=fstart+offset;
  fp->seek(foffset);
  return foffset;
}

w32 i4_sub_section_file::size()
{
  return fsize;
}

w32 i4_sub_section_file::tell()
{
  return foffset-fstart;
}


i4_sub_section_file::i4_sub_section_file(i4_file_class *_fp, int start, int size)
{
  fp=_fp;
  if (!fp)
    i4_error("no file");

  fp->seek(start);
  fsize=size;
  fstart=start;
  foffset=start;
}

i4_sub_section_file::~i4_sub_section_file()
{
  if (fp)
    delete fp;
}


i4_bool i4_sub_section_file::async_read (void *buffer, w32 size, 
                                         async_callback call,
                                         void *context, w32 priority)
{
  if (foffset-fstart+size> (w32)fsize)  // don't allow reads past the sub section
    size=fstart+fsize-foffset;


  foffset+=size;  
  return fp->async_read(buffer, size, call, context, priority);
}
// WIN32\WIN_FILE.CPP

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _WINDOWS
i4_profile_class pf_win32_seek("Win32File::Seek");

i4_profile_class pf_win32_read("Win32File::Read");
i4_profile_class pf_win32_write("Win32File::Write");

class i4_win32_async_reader : public i4_async_reader
{
	public:
		w32 max_priority(){return 100;};//the maximum priority that is handled differently
public:
  i4_win32_async_reader(char *name) : i4_async_reader(name) {}
  virtual w32 read(sw32 fd, void *buffer, w32 count) 
  { 
    w32 res = _read(fd,buffer,count);
    return res;
  }
} i4_win32_async_instance("hd_async_reader_2");

class i4_win32_async_priority_reader : public i4_win32_async_reader
	{
	public:
		w32 max_priority(){return 100;};
		i4_win32_async_priority_reader(char *name):i4_win32_async_reader(name){};
	}i4_win32_async_priority_instance("hd_async_reader_3");


#else

#ifndef __sgi
extern "C" {
// libpthreads doesn't have this defined (it's not thread-safe)
#ifndef __P         // glibc symbol
#define __P(p) p
#endif
extern char *realpath __P ((__const char *__name, char *__resolved));
           };
#endif

i4_critical_section_class realpath_lock;

enum { S_IRWUGO=(S_IRUSR|S_IRGRP|S_IROTH| S_IWUSR|S_IWGRP|S_IWOTH) };
enum { S_ALL   =(S_IRUSR|S_IRGRP|S_IROTH| S_IWUSR|S_IWGRP|S_IWOTH | S_IXUSR|S_IXGRP|S_IXOTH) };

class i4_unix_async_reader : public i4_async_reader
{
public:
		w32 max_priority(){return 0xffffffff;};
  i4_unix_async_reader(char *name) : i4_async_reader(name) {}
  virtual w32 read(sw32 fd, void *buffer, w32 count) 
  { 
    return ::read(fd,buffer,count);
  }
} i4_unix_async_instance("hd_async_reader");

#ifndef SUN4
class i4_unix_async_priority_reader:public i4_unix_async_reader
	{
	public:
		w32 max_priority(){return 100;};
		i4_unix_async_priority_reader(char *name):i4_unix_async_reader(name){};
	}i4_unix_async_priority_instance("hd_async_reader_4");
#endif

#endif

////////////////////////////////////////////////////////////////////////
//
//  Normal Win32 File Class
//
#ifdef _WINDOWS
class i4_win32_file_class : public i4_file_class
{
protected:
  w32 fd;
public:

  i4_win32_file_class(w32 fd) : fd(fd) {}

  w32 read (void *buffer, w32 size)
  {
    pf_win32_read.start();
    w32 res = _read(fd,buffer,size);
    pf_win32_read.stop();
    return res;
  }

  w32 write(const void *buffer, w32 size)
  {
    pf_win32_write.start();
    w32 res = _write(fd,buffer,size);
    pf_win32_write.stop();
    return res;
  }

  w32 seek (w32 offset)
  {  
    pf_win32_seek.start();
    w32 res = lseek(fd, offset, SEEK_SET);
    pf_win32_seek.stop();
    return res;
  }

  w32 size ()                       
  { 
    w32 len = _filelength(fd);

    /*
    w32 cur = lseek(fd,0,SEEK_CUR);
      ]  len = lseek(fd,0,SEEK_END);
    lseek(fd,cur,SEEK_SET);
    */
    return len;
  }

  w32 tell ()
  {
    return _tell(fd);
  }

  ~i4_win32_file_class()
  { 
#ifdef file_load_debug
  static int recurs=0;
  //This is a problem when at the same time logging is enabled
  if (recurs==0)
	  {
	  recurs++;
	  i4_warning("Closing file with fd=%i.",fd);
	  recurs--;
	  }
#endif
    _close(fd); 
  }


  // returns i4_F if an immediate error occured
  i4_bool async_read (void *buffer, w32 size, 
                              async_callback call,
                              void *context=0, w32 priority=255)
  {
    if (i4_threads_supported())
		if (i4_win32_async_priority_instance.max_priority()>priority)
			return i4_win32_async_priority_instance.start_read(fd,buffer,size,call,context,priority);
		else
            return i4_win32_async_instance.start_read(fd, buffer, size, call, context,priority);
    else
      call(read(buffer,size),context);
    return i4_T;
  }

};

#else

////////////////////////////////////////////////////////////////////////
//
//  Normal UNIX File Class
//

class i4_normal_file_class : public i4_file_class
{
protected:
  w32 fd;
public:

  i4_normal_file_class(w32 fd) : fd(fd) {}


  virtual i4_bool async_read (void *buffer, w32 size, async_callback call, void *context, w32 priority=255)
  {

    if (i4_threads_supported())
    {
#ifdef SUN4
	return i4_unix_async_instance.start_read(fd,buffer,size,call,context,priority);
#else
		if (i4_unix_async_priority_instance.max_priority()>priority)
			return i4_unix_async_priority_instance.start_read(fd,buffer,size,call,context,priority);
		else
		    return i4_unix_async_instance.start_read(fd, buffer, size, call, context, priority);
#endif
    }
    else 
      (*call)(read(buffer, size), context);
    return i4_T;
  }

  virtual w32 read (void *buffer, w32 size) 
  {
    int ret=::read(fd,buffer,size);
    if (ret<0) ret=0;
    return ret;
  }
  
  virtual w32 write(const void *buffer, w32 size) 
  { 
    w32 ret=::write(fd,buffer,size); 
    return ret;
  }
  
  virtual w32 seek (w32 offset)
  { 
    w32 ret=lseek(fd,offset,SEEK_SET); 
    return ret;
  }
  
  virtual w32 size ()                       
  { 
    w32 cur=lseek(fd,0,SEEK_CUR);
    w32 len=lseek(fd,0,SEEK_END);
    lseek(fd,cur,SEEK_SET);
    return len;
  }
  
  virtual w32 tell ()                       
  { 
    w32 ret=lseek(fd,0,SEEK_CUR); 
    return ret;
  }
  
  ~i4_normal_file_class()
  { 
    close(fd); 
  }
};





#endif


////////////////////////////////////////////////////////////////////////
//
//  File Manager Methods
//


// see file/file.hh for a description of what each of these functions do
#ifdef _WINDOWS
class i4_win32_file_manager_class : public i4_file_manager_class
{
public:   
  virtual i4_file_class *open(const i4_const_str &name, w32 flags)
  {
    sw32 f=0;
    i4_bool no_buffer=i4_F;
    flags &= ~I4_SUPPORT_ASYNC;     // don't have to do anything special for these
    
    if (flags & I4_NO_BUFFER)
    {
      flags=(flags & (~I4_NO_BUFFER));
      no_buffer=i4_T;
    }

    f=O_BINARY;       // open all files in binary mode
    char sbuf[256];
	char newbuf[256];
	char drivebuf[5];
	int res=0;
	char *whatfor;//used for debugging.
    switch (flags)
    {
      case I4_READ: 
        f|=O_RDONLY;
		whatfor="reading only";
        break;

      case I4_WRITE: 
        f |= O_WRONLY | O_CREAT;
		whatfor="writting as a new file";
        res=_unlink(i4_os_string(name,sbuf,sizeof(sbuf)));
		if (res!=0)
			whatfor="writting as a new file (but deleting failed)";
        break;

      case I4_WRITE|I4_READ: 
        f |= O_RDWR | O_CREAT;
		whatfor="randomly accessing as new file" ;
        res=_unlink(i4_os_string(name,sbuf,sizeof(sbuf)));
		if (res!=0)
			whatfor="randomly accessing as new file (but deleting failed)";
        break;

      case I4_APPEND:
	  case I4_WRITE|I4_APPEND:
		  f|= O_WRONLY|O_CREAT|O_APPEND;
		  whatfor="appending and writing";
		  //Warning: O_APPEND makes it impossible to write 
		  //somewhere except at the end.
		  break;
      case I4_WRITE|I4_APPEND|I4_READ:
        f |= O_RDWR|O_CREAT|O_RANDOM;
		whatfor="reading and writing";
		//Taking this case to solve the above problem
        break;

      default: 
        i4_warning("i4_file_class::Bad open flags!");
		whatfor="actually nothing";
        return NULL;     
    }

    int fd;
	i4_os_string(name,sbuf,sizeof(sbuf));
	strcpy(newbuf,sbuf);
	//ULARGE_INTEGER bytesfree,totalbytes,totalfree;
	//must move the following check somewhere else as
	//this code is time-critical
	/*if (GetDiskFreeSpaceEx(NULL,&bytesfree,&totalbytes,&totalfree)!=0)
	//GetDiskFreeSpaceEx fails on Win95A (See SDK)
		{
		while (bytesfree.QuadPart<5000000)
		{

		int a=MessageBox(i4_win32_window_handle,
			"I4_FILE_MANAGER: The free Disk Space on the current drive has fallen below 5MB\n"
			"Please free up some space before continuing or the system may fail.\n"
			"Warning: Pressing abort immediatelly quits.",
			"Disk space critical",MB_ABORTRETRYIGNORE+MB_ICONSTOP+MB_DEFBUTTON2);
		if (a==IDABORT) i4_error("Quitting at user request");
		if (a==IDIGNORE) break;	
		GetDiskFreeSpaceEx(NULL,&bytesfree,&totalbytes,&totalfree);
		}
		}*/
	fd=::_open(newbuf,f,_S_IREAD | _S_IWRITE);
	if (strstr(newbuf,"tex_cache")==NULL)//Diese Datei NIE auf der CD suchen
		//Aber: Erzeuge für jeden Videomodus eine andere Cachedatei
		//und versuche, mehrere verschiedene g_decompressed/ Ordner zu benützen
		{
	if (fd<0){//Gefunden in aktuellem Pfad?

		if (f&O_CREAT) 
			{
			//The creation of the file has failed
			MessageBox(0,
				"Unable to create a file in the current directory. Check wheter you have the correct permissions.",
				"File system permission problem",MB_OK+MB_ICONSTOP+MB_SYSTEMMODAL);
			return NULL;
			}
		if (!get_first_cd_letter(drivebuf))
			{
#ifdef file_load_debug
			i4_warning("i4_win32_file_manager_class::open() failed for %s and no CDROM Drive available.",i4_os_string(name,sbuf,sizeof(sbuf)));
#endif
			return NULL;
			}
//HIER: Suchen nach der Datei, evtl auf der CD, in einem anderen Verzeichnis...
		do
			{
			
			sprintf(newbuf,"%s%s",drivebuf,sbuf);
			//Stammverzeichnis der CD
			fd=::_open(newbuf,f,_S_IREAD);//_S_IWRITE is rather useless when accessing a CD
			if (fd>=0)
				{
				
				break;
				}
			sprintf(newbuf,"%sgolgotha/%s",drivebuf,sbuf);
			//Verzeichnis "\Golgotha" der CD
			fd=::_open(newbuf,f,_S_IREAD);
			if (fd>=0)
				{
				
				break;
				}
			
			fd=-1;
			}
		
		while((get_next_cd_letter(drivebuf))!=0);
		
		}
		}
    if (fd<0)
    {
#ifdef file_load_debug
      i4_warning("i4_win32_file_manager_class::open() failed for %s\n",i4_os_string(name,sbuf,sizeof(sbuf)));
#endif
      return NULL;
    }
#ifdef file_load_debug
	static int recursing=0;
	if (recursing==0)
		{
		recursing++;
		i4_warning("File %s (fd=%i)opened for %s\n",i4_os_string(name,sbuf,sizeof(sbuf)),fd,whatfor);
		//if (strcmp(sbuf,"texture/skycloudyblue.jpg")==0)
		//	{
		//	int a=0;
		//	a+=27;
		//	}
		recursing--;
		}
#endif

    i4_file_class *ret_fp;
    if (!no_buffer)
      ret_fp=new i4_buffered_file_class(new i4_win32_file_class(fd));
    else
      ret_fp=new i4_win32_file_class(fd);

    return ret_fp;
  }



  virtual i4_bool unlink(const i4_const_str &name)
  {
    char buf[MAX_PATH];
    return _unlink(i4_os_string(name,buf,sizeof(buf)))==0;
  }


  virtual i4_bool mkdir(const i4_const_str &name)
  {  
    char buf[MAX_PATH];
    return ::_mkdir(i4_os_string(name,buf,sizeof(buf)))==0;
  }

  i4_bool get_status(const i4_const_str &filename, i4_file_status_struct &return_stat)
  {
    i4_bool error=i4_F;
    struct _stat times;
    char buf[256];

    return_stat.flags=0;
    if (_stat(i4_os_string(filename,buf,sizeof(buf)),&times)==0)      
    {
      return_stat.last_modified=times.st_mtime;
      return_stat.last_accessed=times.st_atime;
      return_stat.created=times.st_ctime;
	  return_stat.size=times.st_size;
	  return_stat.flags=0;

      if (times.st_mode &  _S_IFDIR)
        return_stat.flags=I4_FILE_STATUS_DIRECTORY;     
    }
    else 
      error=i4_T;

    return (i4_bool)(!error);
  }

  virtual i4_bool get_directory(const i4_const_str &path, 
                                i4_directory_struct &dir_struct,
                                i4_bool get_status,
                                i4_status_class *status)

  {
    _finddata_t fdat;
    char os_str[512],buf[512];
    sprintf(os_str,"%s/*.*",i4_os_string(path,buf,sizeof(buf)));

    long handle=_findfirst(os_str,&fdat),done;
    if (handle==-1)
      return i4_F;
  
    i4_array<i4_file_status_struct> stats(64,64);

    do
    {
      if (fdat.attrib & _A_SUBDIR)
      {
        dir_struct.tdirs++;
        dir_struct.dirs=(i4_str **)i4_realloc(dir_struct.dirs,
                                              sizeof(i4_str *)*dir_struct.tdirs,"dir list");
        dir_struct.dirs[dir_struct.tdirs-1]=new i4_str(i4_const_str(fdat.name));
      }
      else
      {
        i4_file_status_struct *s=stats.add();
        s->last_accessed=fdat.time_access;
        s->last_modified=fdat.time_write;
        s->created=fdat.time_create;
		s->flags=fdat.attrib;
		s->size=fdat.size;

        dir_struct.tfiles++;
        dir_struct.files=(i4_str **)i4_realloc(dir_struct.files,
                                               sizeof(i4_str *)*dir_struct.tfiles,"dir list");
        dir_struct.files[dir_struct.tfiles-1]=new i4_str(i4_const_str(fdat.name));
      }

      done=_findnext(handle, &fdat);
    } while (done!=-1);

	_findclose(handle);
    if (get_status)
    {    
      if (dir_struct.tfiles)
      {
        i4_file_status_struct *sa;
        sa=(i4_file_status_struct *)I4_MALLOC(sizeof(i4_file_status_struct)*dir_struct.tfiles,"");
        for (int j=0; j< (int)dir_struct.tfiles; j++)
          sa[j]=stats[j];

        dir_struct.file_status=sa;
      }
    }

    return i4_T;
  }

  virtual i4_str *full_path(const i4_const_str &relative_name)
  {
    char buf[256], buf2[256];
	buf2[0]=0;
	i4_os_string(relative_name, buf, 256);
    if (! ::_fullpath(buf2, buf, 256))
		{
		return 0;
		}
    return new i4_str(buf2);
  }

  

  i4_win32_file_manager_class()
  {
	FindCDDrives();
    i4_add_file_manager(this, i4_F);    
  }

  ~i4_win32_file_manager_class()
  {
    i4_remove_file_manger(this);
  }
  //Get Type of Drive
  //The return value specifies the type of drive. It can be one of the following values. 

 
//0, DRIVE_UNKNOWN The drive type cannot be determined. 
//1, DRIVE_NO_ROOT_DIR The root path is invalid. For example, no volume is mounted at the path. 
//2, DRIVE_REMOVABLE The disk can be removed from the drive. 
//3, DRIVE_FIXED The disk cannot be removed from the drive. 
//4, DRIVE_REMOTE The drive is a remote (network) drive. 
//5, DRIVE_CDROM The drive is a CD-ROM drive. 
//6, DRIVE_RAMDISK The drive is a RAM disk. 

//0=A, 1=B... be shure 0<=letter<=26
  virtual w8 Get_Drive_Type(w8 letter)
	  {
	    return Drives[letter];
	  }
  virtual void FindCDDrives()
	  {
	  UINT t;
	  char s[5];
	  BOOL foundcd=false;
	  firstcd=2;
	  //Scan for Drive-Types
		for(int i=0;i<=26;i++)
			{
			sprintf(s,"%c:\\",(i+'A'));
			t=GetDriveType(s);
			if (t==DRIVE_CDROM && foundcd==false)
				{
				firstcd=t;
				foundcd=true;
				}
			Drives[i]=t;
			}
			
		if (!foundcd){
			i4_warning("No CDROM Drive found.");
			}
		currentcd=firstcd;
	  }
  virtual int get_first_cd_letter(char *str)
	  {
	  if (firstcd<3)
		  {
		  return false;
		  }
	  sprintf(str,"%c:\\",(firstcd-1+'A'));
	  currentcd=firstcd;
	  return true;
	  }
  virtual int get_next_cd_letter(char *str)
	  {
	  int i=currentcd+1;
	  while (i<=26)
		  {
		  if (Drives[i]==DRIVE_CDROM)
			  {
			  currentcd=i;
			  sprintf(str,"%c:\\",(currentcd-1+'A'));
	          return true;
			  }
		  i++;
		  }
	  return false;
	  }

  protected:
	  w8 Drives[27];
	  w8 firstcd;
	  w8 currentcd;

} i4_win32_file_man;


i4_bool i4_rmdir(const i4_const_str &path)
{
  char p[256];
  i4_os_string(path, p, 256);
  if (_rmdir(p)==0)
    return i4_T;
  else return i4_F;
}

i4_bool i4_chdir(const i4_const_str &path)
{
  char p[256];
  i4_os_string(path, p, 256);
  if (_chdir(p)==0)
    return i4_T;
  else return i4_F;
}

#else
//UNIX File manager

char *i4_linux_filename_filter(const i4_const_str &name, char *obuf, int olen)
{
  char buf[256], *s, *d;
  i4_os_string(name, buf, 256);
  int changed=0;

  if (olen) olen--;     // save space for null

  for (s=buf, d=obuf; olen && *s; )
  {
    if (*s=='\\')
    {
      *d='/';
      changed=1;
    }
    else if (s[0]=='c' && s[1]==':' && s[2]=='/')
    {
      *d='/';
      s+=2;
      changed=1;
    }
    else if (s[0]=='x' && s[1]==':')
    {
      d[0]='/';
      d[1]='u';
      d++;
      olen--;
      s++;
      changed=1;
    }
    else *d=*s;

    s++;
    d++;
    olen--;
  }
  *d=0;
  
  return obuf;
}


class i4_linux_file_manager_class : public i4_file_manager_class
{

public:
  virtual i4_file_class *open(const i4_const_str &name, w32 flags=I4_READ)
  {
    char buf[256];
    i4_file_class *fp=0;

    sw32 f=0;
    i4_bool no_buffer=i4_F;

    flags &= ~(I4_SUPPORT_ASYNC);
    
    if (flags & I4_NO_BUFFER)
    {
      flags=(flags & (~I4_NO_BUFFER));
      no_buffer=i4_T;
    }


    switch (flags)
    {
      case I4_READ | I4_APPEND :
      case I4_READ | I4_WRITE | I4_APPEND :
        f |= O_RDWR | O_CREAT | O_APPEND;
        break;
        
      case I4_READ: 
        f|=O_RDONLY;
        break;

      case I4_WRITE: 
        f |= O_WRONLY | O_CREAT;
        ::unlink(i4_linux_filename_filter(name, buf, sizeof(buf)));
        break;

      case I4_WRITE|I4_READ: 
        f |= O_RDWR | O_CREAT;
        ::unlink(i4_linux_filename_filter(name, buf, sizeof(buf)));
        break;

      case I4_APPEND:
      case I4_WRITE|I4_APPEND:
        f |= O_WRONLY|O_CREAT|O_APPEND;
        break;

      default: 
        i4_warning("i4_file_class::Bad open flags!");
        return NULL;     
    }

    int fd;


    if (flags & I4_WRITE)
      fd=::open(i4_linux_filename_filter(name, buf, sizeof(buf)),f,S_IRWUGO);
    else
      fd=::open(i4_linux_filename_filter(name, buf, sizeof(buf)),f);

    if (fd<0)
      {
#ifdef file_load_debug
      i4_warning("i4_unix_file_class::open() failed for %s.",i4_os_string(name,buf,256));
#endif
      return 0;
      }

    if (!no_buffer)
      fp=new i4_buffered_file_class(new i4_normal_file_class(fd));
    else
      fp=new i4_normal_file_class(fd);

#ifdef file_load_debug
      i4_warning("i4_unix_file_class::open() suceeded for %s.",i4_os_string(name,buf,256));
#endif
    return fp;
    
  }

  virtual i4_bool unlink(const i4_const_str &name)
  {
    char buf[MAX_PATH];
    return ::unlink(i4_linux_filename_filter(name, buf, sizeof(buf)))==0;
  }

  virtual i4_bool mkdir(const i4_const_str &name)
  {
    char buf[MAX_PATH];
    return ::mkdir(i4_linux_filename_filter(name,buf,sizeof(buf)),S_ALL)==0;
  }

  i4_bool get_status(const i4_const_str &filename, i4_file_status_struct &return_stat)
  {
    char buf[256];
    i4_bool error=i4_F;
    struct stat times;

    return_stat.flags=0;

    if (stat(i4_linux_filename_filter(filename,buf,sizeof(buf)),&times)==0)     
    {
      return_stat.last_modified=times.st_mtime;
      return_stat.last_accessed=times.st_atime;      
      return_stat.created=times.st_ctime;
	  return_stat.size=times.st_size;
	  return_stat.flags=0;

      if (times.st_mode &  S_IFDIR)
        return_stat.flags=I4_FILE_STATUS_DIRECTORY;     
    }
    else 
      error=i4_T;

    return (i4_bool)(!error);
  }

  virtual i4_bool get_directory(const i4_const_str &path, 
                                i4_directory_struct &dir_struct,
                                i4_bool get_status,
                                i4_status_class *status)
  {
    char buf[256];
    struct dirent *de;

    i4_linux_filename_filter(path,buf,sizeof(buf));
    //buf[strlen(buf)+1]=0;
    //buf[strlen(buf)]='/';
    DIR *d=opendir(buf);
    if (!d) 
    {
    	i4_warning("Cannot get contents of directory %s. Error %i.",buf,errno);
    	return i4_F;
    }

    i4_str **tlist=NULL;
    sw32 t=0;
    char curdir[256];
    getcwd(curdir,256);
    if (chdir(buf)!=0)
    {
    	i4_warning("Cannot change to directory %s.",buf);
    	closedir(d);
      return i4_F;
    }

    do
    {
      de=readdir(d);

      if (status) 
        status->update(0.5);

      if (de && de->d_name[0])
      {
        t++;
        tlist=(i4_str **)i4_realloc(tlist,sizeof(i4_str *)*t,"tmp file list");
        tlist[t-1]=new i4_str(de->d_name);
      }
    } while (de);
    closedir(d);
    d=0;

    i4_file_status_struct *sa=0;
    
    if (t)
      sa=(i4_file_status_struct *)I4_MALLOC(sizeof (i4_file_status_struct) * t, "stat array");
   
    for (int i=0;i<t;i++)
    {
      if (status) 
        status->update(i/(float)t);


      struct stat s;
      stat(i4_linux_filename_filter(*tlist[i],buf,sizeof(buf)), &s);
                                                                                   
      
      if (S_ISDIR(s.st_mode))
      {
        dir_struct.tdirs++;
        dir_struct.dirs=(i4_str **)i4_realloc(dir_struct.dirs,
                                              sizeof(i4_str *)*dir_struct.tdirs,"dir list");
        dir_struct.dirs[dir_struct.tdirs-1]=tlist[i];
      } else
      {
        int on=dir_struct.tfiles++;
        
        dir_struct.files=(i4_str **)i4_realloc(dir_struct.files,
                                               sizeof(i4_str *)*dir_struct.tfiles,"dir list");
        dir_struct.files[on]=tlist[i];

        sa[on].last_accessed=s.st_atime;
        sa[on].last_modified=s.st_mtime;
        sa[on].created=s.st_ctime;
		sa[on].size=s.st_size;
		sa[on].flags=0;
      }
    }
    
    if (t)
      i4_free(tlist);

    if (get_status)
      dir_struct.file_status=sa;
    else if (sa)
      i4_free(sa);

    
    chdir(curdir);
    return i4_T;
  }

  virtual i4_str *full_path(const i4_const_str &relative_name)
  {
    char buf[256], buf2[256];
    realpath_lock.lock();
    ::realpath(i4_linux_filename_filter(relative_name, buf, 256), buf2);
    realpath_lock.unlock();
    return new i4_str(i4_const_str(buf2));
  }


  i4_linux_file_manager_class()
  {
    i4_add_file_manager(this, i4_F);
  }

  ~i4_linux_file_manager_class()
  {
    i4_remove_file_manger(this);
  }

} i4_linux_file_manager;



i4_bool i4_rmdir(const i4_const_str &path)
{
  char p[256];
  i4_os_string(path, p, 256);
  if (rmdir(p)==0)
    return i4_T;
  else return i4_F;
}

i4_bool i4_chdir(const i4_const_str &path)
{
  char p[256];
  i4_os_string(path, p, 256);
  if (chdir(p)==0)
    return i4_T;
  else return i4_F;
}

#endif
// WIN32\WIN_OPEN.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#ifdef _WINDOWS

/*
class open_string : public i4_str
{
public:
  open_string(char *fname)
    : i4_str(fname)
  {
    //len=strlen(fname);
    //memcpy(ptr, fname, len);
  }
};
*/

#ifndef _CONSOLE
long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );



UINT APIENTRY win32_dialog_hook( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch( message )
  {
    case WM_LBUTTONUP :

    case WM_RBUTTONUP :

    case WM_MBUTTONUP :
    case WM_ACTIVATEAPP :


    case WM_SYSKEYUP :
    case WM_KEYUP :
      WindowProc(hWnd, message, wParam, lParam);
      break;
  }

  return FALSE;
}

#endif
void i4_create_file_open_dialog(i4_graphical_style_class *style,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  OPENFILENAME ofn;

  char mname[256], m[256], tname[256], idir[256], fname[256], curdir[256];

  ShowCursor(TRUE);

  _getcwd(curdir,256);

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));


  char *af="All files(*.*)\0*.*\0\0";

  int i=strlen(mname);
  int l=strlen(m) + 1;
  mname[i++]='(';
  memcpy(mname + i, m, l);
  i+=l;
  mname[i-1]=')';
  mname[i++]=0;
  memcpy(mname+i,m,l);
  i+=l;
  l=20;
  memcpy(mname + i, af, l);
  

  fname[0]=0;

  ofn.lStructSize = sizeof(OPENFILENAME);
#ifdef _CONSOLE
  ofn.hwndOwner=0;
#else
  ofn.hwndOwner = i4_win32_window_handle;
#endif
  ofn.hInstance = i4_win32_instance;
  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = fname;
  ofn.nMaxFile = 256;

  ofn.nMaxFileTitle = 0;
  ofn.lpstrFileTitle = NULL;//tname;

  ofn.lpstrInitialDir = idir;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lCustData = 0L;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;
#ifndef _CONSOLE
  ofn.lpfnHook = win32_dialog_hook;
#endif

  ofn.lpstrFilter = mname;
  ofn.lpstrDefExt = "level";
  ofn.lpstrTitle = tname;
  ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER |OFN_FILEMUSTEXIST ;
#ifndef _CONSOLE
  WindowProc(i4_win32_window_handle, WM_LBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_RBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_MBUTTONUP, 0,0);


  li_call("show_gdi_surface");//Be shure gdi-surface is on top
#endif
  if (GetOpenFileName(&ofn)) 
  {	
    i4_file_open_message_class o(ok_id, new i4_str(ofn.lpstrFile));
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }

  _chdir(curdir);

  ShowCursor(FALSE);
}



void i4_create_file_save_dialog(i4_graphical_style_class *style,
                                const i4_const_str &default_name,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  OPENFILENAME ofn;

  char mname[256], m[256], tname[256], idir[256], fname[256], curdir[256];

  _getcwd(curdir,256);

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));


  char *af="All files(*.*)\0*.*\0\0";

  int i=strlen(mname);
  int l=strlen(m) + 1;
  mname[i++]='(';
  memcpy(mname + i, m, l);
  i+=l;
  mname[i-1]=')';
  mname[i++]=0;
  memcpy(mname+i,m,l);
  i+=l;
  l=20;
  memcpy(mname + i, af, l);

  fname[0]=0;

  ofn.lStructSize = sizeof(OPENFILENAME);
#ifndef _CONSOLE
  ofn.hwndOwner = i4_win32_window_handle;
#else
  ofn.hwndOwner=0;
#endif
  ofn.hInstance = i4_win32_instance; 

  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = fname;
  ofn.nMaxFile = 256;

  ofn.nMaxFileTitle = 0;
  ofn.lpstrFileTitle =NULL;

  ofn.lpstrInitialDir = idir;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lCustData = 0L;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;
#ifndef _CONSOLE
  ofn.lpfnHook = win32_dialog_hook;
#endif
  ofn.lpstrFilter = mname;
  ofn.lpstrDefExt = m;
  while (*ofn.lpstrDefExt && *ofn.lpstrDefExt!='.')
    ofn.lpstrDefExt++;

  ofn.lpstrTitle = tname;
  ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER |OFN_OVERWRITEPROMPT;
#ifndef _CONSOLE
  WindowProc(i4_win32_window_handle, WM_LBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_RBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_MBUTTONUP, 0,0);

  li_call("show_gdi_surface");//Be shure gdi-surface is on top if page-flipped
#endif
  if (GetSaveFileName(&ofn)) 
  {	
    i4_file_open_message_class o(ok_id, new i4_str(ofn.lpstrFile));
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }

  _chdir(curdir);
}
#else

//Unix file open dialog

#include "gui/create_dialog.h"
#include "window/window.h"

/*
class open_string : public i4_str
{
public:
  open_string(char *fname)
    : i4_str(fname)
  {
    //len=strlen(fname);
    //memcpy(ptr, fname, len);

  }
};
*/


void i4_create_file_open_dialog(i4_graphical_style_class *style,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  char mname[256], m[256], tname[256], idir[256], pcmd[1000];

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));

  char *display=getenv("DISPLAY");
  if (!display || display[0]==0)
    display=":0.0";

  char *tmp_name="/tmp/open_filename.tmp";
  sprintf(pcmd, "xgetfile -title \"%s\" -pattern \"%s\" -path \"%s\" -popup -display %s > %s",
          tname, m, idir, display, tmp_name);

  unlink(tmp_name);
  system(pcmd);
  FILE *fp=fopen(tmp_name,"rb");
  if (!fp || !fgets(m, 256, fp))
    m[0]=0;
  else
    while (m[strlen(m)-1]=='\n')
      m[strlen(m)-1]=0;
  
  if (fp) fclose(fp);
  unlink(tmp_name);


  char *s=m;

  if (*s==0 || *s=='\n')
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_file_open_message_class o(ok_id, new i4_str(s));
    i4_kernel.send_event(tell_who, &o);
  }


};


class i4_get_save_dialog : public i4_color_window_class
{
  i4_text_input_class *ti;
  i4_graphical_style_class *style;
  w32 ok_id, cancel_id;
  i4_event_handler_class *tell_who;
  i4_str ext_default_name;
public:
  i4_parent_window_class *mp_window;

  enum { OK, CANCEL, CLOSE };

//don't use references here, as local stays in memory while caller will not.
  i4_get_save_dialog(i4_const_str default_name, i4_const_str start_dir, 
                     i4_graphical_style_class *style,
                     w32 ok_id, w32 cancel_id,
                     i4_event_handler_class *tell_who)
    : i4_color_window_class(0,0, style->color_hint->neutral(), style),
      style(style),
      ok_id(ok_id),
      cancel_id(cancel_id),
      tell_who(tell_who),
	ext_default_name(start_dir)
  {    
    mp_window=0;
    ext_default_name.insert(ext_default_name.end(),default_name);
    i4_create_dialog(i4gets("get_savename_dialog"), 
                     this,
                     style,
                     &ti,
                     &ext_default_name,
                     this, OK,
                     this, CANCEL);
                     
    resize_to_fit_children();
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      if (uev->sub_type==OK)          // tell who-ever what the user typed
      {
        i4_file_open_message_class o(ok_id,ti->get_edit_string());
        i4_kernel.send_event(tell_who, &o);
      }
      else
      {
        i4_user_message_event_class c(cancel_id); // tell whoever the user canceled
        i4_kernel.send_event(tell_who, &c);
      }

      style->close_mp_window(mp_window);   // close ourself
    } 
    else i4_parent_window_class::receive_event(ev);
  }
  
  char *name() { return "save dialog"; }
};



void i4_create_file_save_dialog(i4_graphical_style_class *style,
                                const i4_const_str &default_name,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  i4_get_save_dialog *dlg=new i4_get_save_dialog(default_name, start_dir, style, 
                                                 ok_id, cancel_id, tell_who);

  i4_event_reaction_class *re;
  re=new i4_event_reaction_class(dlg, 
                                 new i4_user_message_event_class(i4_get_save_dialog::CLOSE));


  i4_parent_window_class *p;

  p=style->create_mp_window(-1, -1, dlg->width(), dlg->height(),
                            title_name,
                            re);

  p->add_child(0,0, dlg);
  dlg->mp_window=p;
}

#endif

