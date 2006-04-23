//found  profile compare 2 times !!!
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "pch.h"
#include "isllist.h"

#include "init/init.h"

#include "error/error.h"

#include "file/file.h"

#include "memory/malloc.h"

#include "string/string.h"

#include "threads/threads.h"

#include "device/kernel.h"
#include "device/event.h"

#include "lisp/li_init.h"
#include "lisp/lisp.h"

#include "time/time.h"
#include "time/timedev.h"
#include "time/profile.h"
#include "time/profile_stack.h"


#include "window/window.h"
#include "window/style.h"
#include "window/win_evt.h"

#ifdef _WINDOWS
#include <windows.h>
#include <windef.h>
#include <winbase.h>
#endif


i4_time_device_class i4_time_dev;
extern int i4_show_events;

static i4_isl_list<i4_time_device_class::timed_event> events;

i4_time_device_class::timed_event::~timed_event()
{
  delete event;
}

i4_time_device_class::timed_event::timed_event(i4_event_handler_class *send_to,
                                               i4_event *event,
                                               w32 milli_wait)
  : send_to(send_to),
    event(event->copy()),
    milli_wait(milli_wait)
{
}

class i4_time_device_adder_class : public i4_init_class
{
  public :
	  int init_type(){return I4_INIT_TYPE_BEFORE_OTHER;}
  void init() 
  { 
    i4_kernel.add_device(&i4_time_dev);
  }
  void uninit()
	  {
	  i4_kernel.remove_device(&i4_time_dev);
	  //delete all pending events
	  i4_isl_list<i4_time_device_class::timed_event>::iterator it=events.begin(),
		  del=events.end(), last=events.end();
	  for (;it!=events.end();)
		  {
		  if (last==events.end())
		    events.erase();
		  else
            events.erase_after(last);

          del=it;
          ++it;

          //del->send_to->receive_event(del->event);
          delete &*del;

		  }
	  }
} i4_time_device_adder_instance;



i4_bool i4_time_device_class::cancel_event(const id &handle)
{
  i4_isl_list<timed_event>::iterator i=events.begin(),last=events.end();
  for (;i!=events.end();++i)
  {
    if (handle.reference==&*i)
    {
      if (last==events.end())
        events.erase();
      else
        events.erase_after(last);
      delete &*i;
      return i4_T;
    }
    last=i;
  }
  return i4_F;
}

i4_bool i4_time_device_class::process_events()       // returns true if an event was dispatched
{
  if (events.begin()==events.end()) 
    return i4_F;

  i4_time_class current;
  i4_bool ret=i4_F;

  i4_isl_list<timed_event>::iterator i=events.begin(),del=events.end(), last=events.end();

  for (;i!=events.end();)
  {
    if (current.milli_diff(i->start_time)>(sw32)i->milli_wait)
    {
      if (last==events.end())
        events.erase();
      else
        events.erase_after(last);

#ifndef I4_RETAIL
      if (i4_show_events)
	  {
		  char buf[i4_event::MAX_NAME_BUFFER_SIZE];
		  char win_name[i4_event::MAX_NAME_BUFFER_SIZE];
		  i->event->name(buf);
		  i->send_to->name(win_name);
        i4_warning("sending : '%s' to '%s'",buf, win_name);
	  }
#endif
      del=i;
      ++i;

      del->send_to->receive_event(del->event);
      delete &*del;

      ret=i4_T;
    }
    else 
    {
      last=i;
      ++i;
    }

  }
  last=events.end();
  return ret;
}

i4_time_device_class::id 
i4_time_device_class::request_event(i4_event_handler_class *send_to, // who to send the event to
                                       i4_event *event,                 // what event to send
                                       w32 milli_wait) // how much time to wait (in milli-seconds)
{
  // first make sure this is not an event that needs to be sent right now
  // these events usually are two-way events that have return codes inside
  if (event->when()==i4_event::NOW)
    i4_error("INTERNAL: Cannot send NOW events throught the time device!");

  timed_event *ev=new timed_event(send_to,event,milli_wait);
  events.insert_end(*ev);
  return id(ev);
}
// WINTIME.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _WINDOWS
sw32 i4_win32_start_clock=0;

//This is useless, since instances of the time class get allocated long before
//this code would be executed (because there are static instances or fields of static instances
//with this type). Initialisation sequence of statics cannot be determined and must be assumed
//as random. 
//class i4_start_time_init_class:public i4_init_class
//{
//public:
//	virtual void init()
//	{
//		i4_win32_start_clock=GetTickCount();
//	}
//	virtual int init_type() 
//	{
//		//this one is completelly independent from anything else, so this is valid. 
//		return I4_INIT_TYPE_MEMORY_MANAGER;
//	}
//}i4_start_time_init_class_inst;

void i4_time_class::get()
{
  if(i4_win32_start_clock==0)
  {
	  i4_win32_start_clock=GetTickCount();
  }
  time.win32_time.clock=GetTickCount()-i4_win32_start_clock;
  time.win32_time.overflow=0;
}

void i4_time_class::add_milli(sw32 milli_sec)
{
  time.win32_time.clock+=milli_sec;
}



i4_bool i4_time_class::operator <(const i4_time_class &other) const 
{ 
  return (other.time.win32_time.clock<time.win32_time.clock);
}



i4_bool i4_time_class::operator >(const i4_time_class &other) const 
{ 
  return (other.time.win32_time.clock<time.win32_time.clock);
}


sw32 i4_time_class::milli_diff(const i4_time_class &past_time) const
{
  return (sw32)time.win32_time.clock-(sw32)past_time.time.win32_time.clock;
}

i4_time_class::i4_time_class(sw32 milli_sec)
{
  time.win32_time.clock=milli_sec;
  time.win32_time.overflow=0;
}


w64 i4_get_system_clock()
{
  w32 lo, hi;
  __asm
    {
      __emit 0x0F
      __emit 0x31
      mov lo, eax
      mov hi, edx
    };
  return (((w64)hi)<<32) | lo;
}

int i4_win_clocks_per_sec = -1;

int i4_get_clocks_per_second()
{
  if (i4_win_clocks_per_sec==-1)
  {
    w64 _start = i4_get_system_clock();
  
    i4_time_class now,start;
    while (now.milli_diff(start) < 1000) now.get();
  
    w64 end = i4_get_system_clock();

    i4_win_clocks_per_sec = (int)(end - _start);
  }
  
  return i4_win_clocks_per_sec;
}

void i4_sleep(int seconds) { Sleep(seconds*1000); }
void i4_milli_sleep(int milli_seconds) { Sleep(milli_seconds); }
#endif

// PROFILE.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#ifdef I4_PROFILING


i4_profile_stack_struct i4_profile_stack;
i4_profile_stack_item i4_profile_stack_struct::stack[I4_MAX_PROFILE_STACK_DEPTH];
i4_profile_class *i4_profile_class::list;
int i4_profile_on=0;
int i4_profile_stack_top=0;



void i4_profile_stack_struct::overflow()
{
  i4_debug->printf("items on stack end :\n");
  for (int i=top()-1; i>top()-30; i--)
    i4_debug->printf("  '%s'\n",stack[i].item->name);
  
  i4_error("profile stack overflow");



}

void i4_profile_stack_struct::underflow()
{
  i4_error("INTERNAL: Unbalanced profile start/stop");
}

i4_profile_class::i4_profile_class(char *debug_name)
  : name(debug_name)
{  
  for (i4_profile_class *p=list; p; p=p->next)
    if (p==this)
      i4_error("already in list?");

  total_time=0;
  next=list;
  list=this;
  active=1;
}




void i4_profile_class::called_start()
{
  w64 current_clock=i4_get_system_clock();
  if (i4_get_thread_id()==i4_get_main_thread_id())
  {
    i4_profile_stack_item *top=i4_profile_stack.get_top();
    if (top)
      top->item->total_time+=current_clock-top->start_clock;


    i4_profile_stack.push(this, current_clock);
  }

}

void i4_profile_class::called_stop()
{
  w64 current_clock=i4_get_system_clock();

  if (i4_get_thread_id()==i4_get_main_thread_id())
  {
    i4_profile_stack_item *me=i4_profile_stack.pop();
    if (me->item!=this)
      i4_error("profile stop unmatched %s (%s on stack)", 
               name, me->item->name);

    total_time+=current_clock - me->start_clock;
    
    i4_profile_stack_item *top=i4_profile_stack.get_top();
    if (top)
      top->start_clock=current_clock;
  }

}



i4_profile_class::~i4_profile_class()
{
  if (this==list)
    list=next;
  else
  {
    i4_profile_class *last=0;
    i4_profile_class *p;

    for (p=list; p && p!=this; )
    {
      last=p;
      p=p->next;
    }

    if (p!=this)      
      i4_warning("could not find profile entry to unlink %s", name);
    else
      last->next=next;
  }
  
}


// clear out any profile timing information we have so far
void i4_profile_clear()
{
  i4_profile_stack.flush_stack();
  for (i4_profile_class *p=i4_profile_class::list; p; p=p->next)
    p->total_time=0;

}

static int profile_compare(const void *a, const void *b)
{
  i4_profile_class *ap=*((i4_profile_class **)a);
  i4_profile_class *bp=*((i4_profile_class **)b);

  if (ap->total_time<bp->total_time)
    return 1;
  else if (ap->total_time>bp->total_time)
    return -1;
  else return 0;               
}

static int prev_on;
void i4_profile_report_start()
{
  i4_profile_clear();
  prev_on=i4_profile_on;
  i4_profile_on=i4_T;
}


// print a report to about timing since the last clear or program start
void i4_profile_report(char *filename)
{
  i4_profile_stack.flush_stack();

  double total_time=0;
  int t=0,i=0;
  i4_profile_class *p;

  for (p=i4_profile_class::list; p; p=p->next, t++)
    total_time+=(double)(sw64)p->total_time;    

  // put them all in a list and sort them by clocks
  i4_profile_class **plist;
  plist=(i4_profile_class **)I4_MALLOC(sizeof(i4_profile_class *)*t,"");
  for (p=i4_profile_class::list; p; p=p->next)
    plist[i++]=p;


  qsort(plist, t, sizeof(i4_profile_class *), profile_compare);


  double oo_total=1.0/total_time;
  char buf[100];

  i4_file_class *fp=i4_open(filename, I4_WRITE);
  if (fp)
  {
    sprintf(buf,"%2.2f total clocks\n", total_time);
    fp->write(buf, strlen(buf));

    for (i=0; i<t; i++)
    {
      double percent= (double)((sw64)plist[i]->total_time * oo_total);
      double t_clocks=(double)((sw64)plist[i]->total_time);

      sprintf(buf,"%2.2f%%  %2.0f clocks  %s\n", percent * 100.0, t_clocks, plist[i]->name);
      fp->write(buf, strlen(buf));
    }
    delete fp;
  }
  



  i4_free(plist);
  i4_profile_on=prev_on;
}

#endif

// LI_PROFILE.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#ifndef _CONSOLE


li_object *li_profile(li_object *o, li_environment *env)
{
  li_object *r=li_eval(li_car(o,env),env);
  if (r->type()!=LI_STRING)
    i4_warning("profile expects a string arg");
  else
  {
    i4_profile_clear();
    i4_profile_stack_top=0;

    for (i4_profile_class *c=i4_profile_class::list; c; c=c->next)
    {
      if (strstr(c->name, li_get_string(r,env))!=0)
        c->active=1;
      else
        c->active=0;
    }
  }

  
  return 0;
}

li_object *li_profile_report(li_object *o, li_environment *env)
{
  li_object *r=li_eval(li_car(o,env),env);
  if (r->type()!=LI_STRING)
    i4_warning("profile expects a string arg");
  else
    i4_profile_report(li_get_string(r,env));

  
  return 0;
}


li_automatic_add_function(li_profile,"profile");
li_automatic_add_function(li_profile_report,"profile_report");

#endif
// GUI_PROFILE.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _CONSOLE
//2345678*012345678*012345678*012345678*012345678*012345678*012345678*012345678*012345678*012345678*
i4_profile_class pf_profile_draw("profile window refresh");

// sort in descending order



enum { CLOSE_WIN, POLL };
class i4_prof_win_class : public i4_parent_window_class
{
  i4_bool update_stats;
  i4_graphical_style_class *style;
  i4_time_device_class::id poll_id;
  i4_event_reaction_class *on_close;
  w64 last_clock;

public:
  i4_prof_win_class(w16 w, w16 h, i4_graphical_style_class *style,
                    i4_event_reaction_class *on_close)
    : i4_parent_window_class(w,h), 
      style(style),
      on_close(on_close)
  {    
    i4_object_message_event_class poll(this, POLL);
    poll_id=i4_time_dev.request_event(this, &poll, 5000);  // update once every 5 secs

    last_clock=i4_get_system_clock();

      
    // count how many profile classes are in memory
    for (i4_profile_class *p=i4_profile_class::list; p; p=p->next)
      p->total_time=0;

    i4_profile_on=1;
  }

  void parent_draw(i4_draw_context_class &context)
  {
    int t=0,i=0;
    double total_time_measured=0.0;
    i4_profile_class *p=i4_profile_class::list;
    // count how many profile classes are in memory
    for (; p; p=p->next)
    {
      total_time_measured+=(double)(sw64)p->total_time;
      t++;
    }

    pf_profile_draw.start();

    local_image->clear(0, context);
    if (update_stats)
    {
      // put them all in a list and sort them by clocks
      i4_profile_class **plist;
      plist=(i4_profile_class **)I4_MALLOC(sizeof(i4_profile_class *)*t,"pl");
      for (p=i4_profile_class::list; p; p=p->next)
      {        
        plist[i]=p;
        i++;
      }
      qsort(plist, t, sizeof(i4_profile_class *), profile_compare);

      
      char buf[200];

      int y=0, h=height();
      int x=50;

      w64 current_clock = i4_get_system_clock();
      double oo_total = 1.0/(double)((sw64)current_clock - (sw64)last_clock);
      last_clock=i4_get_system_clock();    

      i4_font_class *fnt=style->font_hint->small_font;
      fnt->set_color(0x0000ff);   
      sprintf(buf,"%2.2f Total", ((double)(sw64)total_time_measured * oo_total) * 100.0);
      fnt->put_string(local_image, x+1, y, buf, context);
      y+=fnt->height(buf);


      fnt->set_color(0xffff00);



      for (i=0; i<t; i++)
      {
        double percent = (sw64)plist[i]->total_time * oo_total;

        sprintf(buf, "%2.2f %s", percent * 100.0, plist[i]->name);

        int th=fnt->height(buf);
        local_image->bar((short)(x-(sw32)(percent * x)), y, x, y+th-1, 0xffff, context);

        plist[i]->total_time=0;
        fnt->put_string(local_image, x+1, y, buf, context);
        
        y+=th;
        if (y>h)
          break;
      }

      i4_free(plist);
      update_stats=i4_F;
    } 
    else last_clock=i4_get_system_clock();    

    pf_profile_draw.stop();
  }

  void receive_event(i4_event *ev); 

  ~i4_prof_win_class()
  {
    if (on_close)
      delete on_close;
    i4_time_dev.cancel_event(poll_id);

    i4_profile_stack_top=0;
    i4_profile_on=0;
  }

  void name(char* buffer) { static_name(buffer,"profile window"); }
};

i4_parent_window_class *i4_prof_win=0;

void i4_prof_win_class::receive_event(i4_event *ev)
{
  CAST_PTR(oev, i4_object_message_event_class, ev);

  if (ev->type()==i4_event::OBJECT_MESSAGE && oev->object==this)
  {
    if (oev->sub_type==CLOSE_WIN)     // close window
    {
      i4_prof_win=0;
      
      i4_kernel.send(on_close);
    }
    else if (oev->sub_type==1)  // update statics
    {
      update_stats=i4_T;
      request_redraw();
      
      i4_object_message_event_class poll(this, POLL);
      poll_id=i4_time_dev.request_event(this, &poll, 5000);  // update once every 5 secs
    }
  }
  else i4_parent_window_class::receive_event(ev);
}

void i4_profile_watch(i4_graphical_style_class *style,
                      i4_parent_window_class *parent,                      
                      sw32 &win_x, sw32 &win_y,
                      w32 w, w32 h,
                      int open_type,  // 0==close, 1==open, 2==toggle window
                      i4_event_reaction_class *on_close)
{
  if (i4_prof_win && (open_type==0 || open_type==2))
  {
    win_x=i4_prof_win->get_parent()->x();
    win_y=i4_prof_win->get_parent()->y();

    style->close_mp_window(i4_prof_win);
    i4_prof_win=0;
  }
  else if (!i4_prof_win && (open_type==1 || open_type==2))
  {
    i4_prof_win_class *p=new i4_prof_win_class((w16)w,(w16)h, style, on_close);

    i4_event_reaction_class *re;
    re=new i4_event_reaction_class(p, new i4_object_message_event_class(p,CLOSE_WIN,i4_event::NOW));

    i4_prof_win=style->create_mp_window((short) win_x, (short) win_y,(w16) w,(w16)h, 
                                        i4gets("prof_win_title",i4_F), re);
    i4_prof_win->add_child(0,0,p);
  }

}
#endif
