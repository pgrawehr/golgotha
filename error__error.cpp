// ERROR.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "file/file.h"
#include "init/init.h"
#include "error/error.h"
#include "error/alert.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef __sgi
#include <unistd.h>
#endif 
const char *i4_error_file_on;
int i4_error_line_on;
#ifdef __linux
#include <unistd.h>
#endif
i4_file_class *i4_debug=0;//&default_debug;     // stream you can print debug messages to

class i4_debug_file_class : public i4_file_class
{
public:
  i4_debug_file_class() 
    {
 	if (!i4_debug)
		i4_debug=this;
    }
  virtual w32 read (void *buffer, w32 size)
  {
    return fread(buffer, size, 1, stdin);
  }
  virtual w32 write(const void *buffer, w32 size) 
  {
    return fwrite(buffer, size, 1, stdout);
  }
  virtual w32 seek (w32 offset) { return 0; }
  virtual w32 size () { return 0; }
  virtual w32 tell () { return 0; }
}default_debug;
//static i4_debug_file_class default_debug;

FILE *i4_error_mirror_file=0;

int i4_default_error(const char *st)
{
  static int died=0;

  setbuf(stdout, 0);
  if (i4_debug)
  { 
  i4_debug->printf("*****************************************************************\n"
                   "Error (%s:%d) : %s\n"
                   "*****************************************************************\n",
                   i4_error_file_on, i4_error_line_on, st);
  }
  else
	  printf("Error %s\n",st);
  if (died)
    return 1;
  died = 1;


  if (getenv("FORCE_CORE_DUMP"))
		return *((int *)0xDeadBeef);  // cause a memory fault to stop debugger
  _exit(0);
  return -1;
}


int i4_default_warning(const char *st)
{
  i4_debug->printf("Warning (%s:%d) : %s\n", i4_error_file_on, i4_error_line_on, st);

  return 0;
}

i4_error_function_type i4_error_function=i4_default_error;
i4_error_function_type i4_warning_function=i4_default_warning;


int i4_error_old(const char *format, ...)
{
  va_list ap;
  char st[1000];

  va_start(ap, format);
  vsprintf(st, format, ap);
  int ret = (*i4_error_function)(st);
  //i4_debug->printf("Error : %s\n",st);
  va_end(ap);

  return ret;
}



int i4_warning_old(const char *format, ...)
{
  va_list ap;
  char st[500];

  va_start(ap, format);
  vsprintf(st, format, ap);
  int ret = (*i4_warning_function)(st);
  //i4_debug->printf("Warning : %s\n",st);
  va_end(ap);

  return ret;
}

i4_error_pointer_type i4_get_error_function_pointer(const char *file, int line)
{
  i4_error_file_on=file;
  i4_error_line_on=line;
  return i4_error_old;
}

i4_error_pointer_type i4_get_warning_function_pointer(const char *file, int line)
{
  i4_error_file_on=file;
  i4_error_line_on=line;
  return i4_warning_old;
}



void i4_set_error_function(i4_error_function_type fun)
{
  i4_error_function=fun;
}

i4_error_function_type i4_get_error_function()
{
  return i4_error_function;
}

void i4_set_warning_function(i4_error_function_type fun)
{
  i4_warning_function=fun;
}


i4_error_function_type i4_get_warning_function()
{
  return i4_warning_function;
}

// ALERT.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



extern FILE *i4_error_mirror_file;

int i4_default_alert(const i4_const_str &ret)
//{{{
{
#ifdef __MAC__
  printf("Alert : ");
  i4_const_str::iterator s=ret.begin();
  while (s!=ret.end())
  {
    printf("%c",s.get().value());
    ++s;
  }
  printf("\n");
#else
  fprintf(stderr,"Alert : ");
  i4_const_str::iterator s=ret.begin();
  while (s!=ret.end())
  {
    fprintf(stderr,"%c",s.get().value());
    ++s;
  }
  fprintf(stderr,"\n");
#endif

  return 0;
}
//}}}

i4_alert_function_type i4_alert_function=i4_default_alert;

void i4_alert(const i4_const_str &format, w32 max_length, ...)
{
  va_list ap;
  va_start(ap, max_length);

  i4_str *ret=format.vsprintf(500,ap);  
  (*i4_alert_function)(*ret);

#if defined(WIN32)
  if (!i4_error_mirror_file)
    i4_error_mirror_file = fopen("error.out","wt");
  if (i4_error_mirror_file)
  {
    fprintf(i4_error_mirror_file,"Alert : ");
    i4_const_str::iterator s=ret->begin();
    while (s!=ret->end())
    {
      fprintf(i4_error_mirror_file,"%c",s.get().value());
      ++s;
    }
    fprintf(i4_error_mirror_file,"\n");
  }
#endif

  delete ret;
  va_end(ap);
}


void i4_set_alert_function(i4_alert_function_type fun)
{
  i4_alert_function=fun;
}

// MSVC_EXEC.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



//#ifdef _WINDOWS
//What is this code good for? Bogus hacks?
#if 0

void null_function() {}

w32 MSVC_p[20];
w32 MSVC_params=0;
void (*MSVC_func)()=&null_function;
static char FP_SAVE[120];

typedef void (*func0)(void);
typedef void (*func1)(w32 a);
typedef void (*func2)(w32 a,w32 b);
typedef void (*func3)(w32 a,w32 b,w32 c);
typedef void (*func4)(w32 a,w32 b,w32 c,w32 d);
typedef void (*func5)(w32 a,w32 b,w32 c,w32 d,w32 e);

void MSVC_exec()
{
  __asm {
    pushfd
    pushad
    fsave FP_SAVE
    frstor FP_SAVE    

    push ebp
    mov  ebp, esp
    push ecx
    push ebx
    push esi
    push edi
  }

  switch (MSVC_params)
  {
    case 1: (*((func1)MSVC_func))(MSVC_p[0]); break;
    case 2: (*((func2)MSVC_func))(MSVC_p[0],MSVC_p[1]); break;
    case 3: (*((func3)MSVC_func))(MSVC_p[0],MSVC_p[1],MSVC_p[2]); break;
    case 4: (*((func4)MSVC_func))(MSVC_p[0],MSVC_p[1],MSVC_p[2],MSVC_p[3]); break;
    case 5: (*((func5)MSVC_func))(MSVC_p[0],MSVC_p[1],MSVC_p[2],MSVC_p[3],MSVC_p[4]); break;
    default:(*((func0)MSVC_func))(); break;
  }
  MSVC_func = &null_function;
  MSVC_params = 0;

  __asm {
    pop edi
    pop esi
    pop ebx
    pop ecx
    mov esp, ebp
    pop ebp

    frstor FP_SAVE    
    popad
    popfd
  }
}
#endif


