/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

/* This is a new file from the revival project*/
#include "pch.h"
#include "editor/editor.h"
#include "controller.h"
#include "window/win_evt.h"
#include "string/string.h"
#include "draw_context.h"
#include "font/font.h"
#include "window/style.h"
#include "saver.h"
#include "menu/pull.h"
#include "resources.h"
#include "math/pi.h"
#include "mess_id.h"
#include "loaders/load.h"
#include "editor/edit_id.h"
#include "editor/contedit.h"
#include "editor/e_state.h"
#include "editor/lisp_interaction.h"
#include "m_flow.h"
#include "light.h"
#include "math/spline.h"
#include "file/get_filename.h"

#include "time/gui_prof.h"
#include "border_frame.h"
#include "status/status.h"
#include "status/gui_stat.h"
#include "editor/dialogs/debug_win.h"
#include "render/r1_api.h"
#include "g1_render.h"
#include "gui/create_dialog.h"
#include "gui/text_input.h"
#include "app/app.h"

#include "window/wmanager.h"
#include "editor/e_res.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "gui/li_pull_menu.h"
#include "gui/text_scroll.h"
#include "lisp/li_dialog.h"
#include "main/main.h"


i4_profile_class pf_lispinteraction_draw("lisp_interaction");

// sort in descending order

w8 i4_lisp_interaction_on=0;

enum { CLOSE_WIN, POLL };
class i4_lisp_interaction_class : public i4_parent_window_class, 
public li_gc_object_marker_class 
//I don't really like multiple inheritance, but here it's really usefull
{
  i4_bool update_stats;
  i4_graphical_style_class *style;
  i4_time_device_class::id poll_id;
  i4_event_reaction_class *on_close;
  w64 last_clock;
  
	protected:
  i4_text_scroll_window_class *output;
  i4_text_input_class *input;
  li_environment *window_env;
  

public:
  i4_lisp_interaction_class(w16 w, w16 h, i4_graphical_style_class *style,
                    i4_event_reaction_class *on_close)
    : i4_parent_window_class(w,h), 
      style(style),
      on_close(on_close),
	  li_gc_object_marker_class()
  {    
    i4_object_message_event_class poll(this, POLL);
    poll_id=i4_time_dev.request_event(this, &poll, 5000);  // set up a timer, usable to warn the user of potential lenghty calcs

    last_clock=i4_get_system_clock();
	//window_env=0;//dono what will happen if nonzero

    i4_color fore=i4_read_color_from_resource("lisp_foreground_color");
    i4_color back=i4_read_color_from_resource("lisp_background_color");
	
	
	input=new i4_text_input_class(style,
									"(comment enter lisp code)",//No default text
									w,//as tall as the window
									1000,//up to 1000 chars at once
									this, //inform ourselves about changes
									style->font_hint->normal_font//use large font
									);
	w16 inputh=input->height();
    output=new i4_text_scroll_window_class(style,
                                           fore, back,
                                           w,
                                           h-inputh);
	add_child(0,0,output);
	add_child(0,h-inputh,input);
    output->printf("Lisp parser ready...\n");

    i4_lisp_interaction_on=1;
	i4_kernel.request_events(this,i4_device_class::FLAG_USER_MESSAGE);
	window_env=new li_environment(0,i4_F);//let everything run in same env.
  }

  virtual void mark_objects(int set)
	  {//called by gc.
	  li_get_type(window_env->unmarked_type())->mark(window_env,set);
	  }

  void parent_draw(i4_draw_context_class &context)
  {
    //int t=0,i=0;
    

    //pf_lispinteraction_draw.start();

    local_image->clear(0, context); 
	update_stats=i4_F;
    last_clock=i4_get_system_clock();    
	i4_parent_window_class::parent_draw(context);
    //pf_lispinteraction_draw.stop();
  }

  void receive_event(i4_event *ev); 

  ~i4_lisp_interaction_class()
  {
    if (on_close)
      delete on_close;
    i4_time_dev.cancel_event(poll_id);
	delete output;
	output=0;//needed here, as there might still come events to 
	//us while being destructed
	delete input;
	input=0;
	i4_kernel.unrequest_events(this,i4_device_class::FLAG_USER_MESSAGE);
    
    i4_lisp_interaction_on=0;

	//NEWER EVER call a destructor implicitly in a destructor, gives only trouble.
	//li_gc_object_marker_class::~li_gc_object_marker_class();//called implicitly by compiler
  }

  char *name() { return "Lisp_Interaction_Window"; }
};

i4_parent_window_class *i4_lisp_int_win=0;

void i4_lisp_interaction_class::receive_event(i4_event *ev)
	{
	
	
	if (ev->type()==i4_event::OBJECT_MESSAGE)
		{
		CAST_PTR(oev, i4_object_message_event_class, ev);
		if (oev->object==this)
			{
			if (oev->sub_type==CLOSE_WIN)     // close window
				{
				i4_lisp_int_win=0;
				output=0;input=0;//deleted by kernel
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
		else 
			if (oev->object==input)
				{
				
				CAST_PTR(tc,i4_text_change_notify_event,ev);
				if (output&&tc->new_text)
					{
					char *s;
					int l=input->get_edit_string()->length();
					char *buf=(char *)I4_MALLOC(l+1,"Lisp_Eval_Buf");
					buf[l]=0;
					i4_str *str=input->get_edit_string();
					i4_os_string(*str,buf,1001);
					s=buf;
					output->printf(buf);
					output->printf("\n");
					li_object *exp,*ret;
					do
						{
						exp=li_get_expression(s, window_env);
						ret=0;
						if (exp)
							{
							li_set_value("-",exp,window_env);
							li_set_value("+++",li_get_value("++"),window_env);
							li_set_value("++",li_get_value("+"),window_env);
							ret=li_eval(exp, window_env);
							li_set_value("+",exp,window_env);
							li_set_value("***",li_get_value("**"),window_env);
							li_set_value("**",li_get_value("*"),window_env);
							li_set_value("*",ret,window_env);
							li_print(ret,window_env);
							}
						} while (exp);
					
					i4_free(buf);
					request_redraw();
					}
				}
			else
				{
				i4_parent_window_class::receive_event(ev);
				}
		}
	else if (ev->type()==i4_event::USER_MESSAGE)
		{
		CAST_PTR(mev,g1_send_debug_message_class,ev);
		if (mev->sub_type==G1_DEBUG_SEND_EVENT)
			{
			if (output) output->printf(mev->msg);//output get's deleted
			//while still messages may rest in the queue
			//output->printf("\n"); //not needed
			request_redraw();
			}
		else
			i4_parent_window_class::receive_event(ev);
		}
	else
		i4_parent_window_class::receive_event(ev);
	}

void i4_lisp_interaction_window(i4_graphical_style_class *style,
                      i4_parent_window_class *parent,                      
                      sw32 &win_x, sw32 &win_y,
                      w32 w, w32 h,
                      int open_type,  // 0==close, 1==open, 2==toggle window
                      i4_event_reaction_class *on_close)
{
  if (i4_lisp_int_win && (open_type==0 || open_type==2))
  {
    win_x=i4_lisp_int_win->get_parent()->x();
    win_y=i4_lisp_int_win->get_parent()->y();

    style->close_mp_window(i4_lisp_int_win);
    i4_lisp_int_win=0;
  }
  else if (!i4_lisp_int_win && (open_type==1 || open_type==2))
  {
    i4_lisp_interaction_class *p=new i4_lisp_interaction_class((w16)w,(w16)h, style, on_close);

    i4_event_reaction_class *re;
    re=new i4_event_reaction_class(p, new i4_object_message_event_class(p,CLOSE_WIN,i4_event::NOW));

    i4_lisp_int_win=style->create_mp_window((short) win_x, (short) win_y,(w16) w,(w16)h, 
                                        i4gets("lisp_inter_title",i4_F), re);
    i4_lisp_int_win->add_child(0,0,p);
  }

}

//An empty function
li_object *ignore_comment(li_object *o,li_environment *env)
	{
	return li_nil;
	}
extern int i4_show_events;
li_object *toggle_kernel_debug(li_object *o,li_environment *env)
	{
	i4_show_events=(i4_show_events+1)%3;//cycle var
	return new li_int(i4_show_events);
	}

li_automatic_add_function(ignore_comment,"comment");
li_automatic_add_function(toggle_kernel_debug,"show-messages");

