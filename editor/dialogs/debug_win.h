/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DEBUG_WIN_HH
#define DEBUG_WIN_HH

#include "window/style.h"
#include "device/event.h"
#include "string/string.h"
#include "mess_id.h"

//These functions are also valueable if not in debug - mode.
//#ifdef _DEBUG 
// debugging enabled

class i4_debug_message_device_class : public i4_device_class
{


public:
	i4_debug_message_device_class():i4_device_class(){};
  virtual void broadcast_message(char *message);
  virtual void broadcast_message(i4_const_str &message);
  virtual i4_bool process_events(){return i4_F;};       // returns true if an event was dispatched
  virtual void name(char* buffer) { static_name(buffer,"Debug Message Device"); }
  //virtual device_flags reports_events() { return 0; }  // does not report common events
};

extern i4_debug_message_device_class i4_debug_dev;

class g1_send_debug_message_class : public i4_user_message_event_class
{ 
public:
  char *msg;
  i4_bool del_on_destruct;
  virtual dispatch_time when() { return NOW; }  
  virtual void name(char* buffer) {strncpy(buffer,msg,127);}
  g1_send_debug_message_class(i4_const_str &message) 
    : i4_user_message_event_class(G1_DEBUG_SEND_EVENT)
      
	  {
	  w32 l=message.ascii_length();
	  msg=new char[l];
	  i4_os_string(message,msg,message.ascii_length());
	  del_on_destruct=i4_T;
	  }
  g1_send_debug_message_class(char *message_text)
	  :i4_user_message_event_class(G1_DEBUG_SEND_EVENT)
	  {
	  msg=message_text;
	  del_on_destruct=i4_F;
	  }
  virtual ~g1_send_debug_message_class()
	  {
	  if (del_on_destruct)
		  delete msg;
	  }

  virtual i4_event  *copy() { return new g1_send_debug_message_class(msg); }    
};



#define g1_debug   g1_debug_printf
#define g1_debugxy g1_debug_printfxy

int g1_debug_close(i4_graphical_style_class *style);
int g1_debug_open(i4_graphical_style_class *style, 
		  i4_parent_window_class *parent, 
		  const i4_const_str &title,
		  int status_lines,
		  i4_event_reaction_class *closed);
int g1_debug_printf(const char *s, ...);
int g1_debug_printfxy(int x, int y, const char *s, ...);
//#else
// no more!

//#define g1_debug               if (0)
//#define g1_debugxy             if (0)
//#define g1_debug_close(style)  (0)
//#define g1_debug_open(a,b,c,d,e) (0) 
//#define g1_debug_printf        if (0)
//#define g1_debug_printfxy      if (0)
//#endif

#endif


