// gui/butbox.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "pch.h"
#include "gui/butbox.h"
#include "error/error.h"
#include "device/kernel.h"
#include "device/event.h"
#include "app/app.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "menu/textitem.h"

void i4_button_box_class::parent_draw(i4_draw_context_class &context)
{
  // this code swiped from color_window
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);

  child_clip.swap(&context.clip);
  
  i4_graphical_style_class *style=i4_current_app->get_style();

  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
    style->deco_neutral_fill(local_image, c->x1, c->y1, c->x2, c->y2, context);

  child_clip.swap(&context.clip);
  i4_parent_window_class::parent_draw(context);
}

void i4_button_group_class::parent_draw(i4_draw_context_class &context)
	{
	
	w32 left=0,top=0,right=0,bottom=0;
	i4_graphical_style_class *style=i4_current_app->get_style();
	i4_font_class *fnt=style->font_hint->normal_font;
	if (!(flags&NOBORDER))
		{
		style->get_in_deco_size(left,top,right,bottom);
		style->draw_in_deco(local_image,0,0,width(),height(),i4_F,context);
		}
	if (!(flags&NOTEXT))
		{
		fnt->put_string(local_image,(short)left+1,(short)top+1,text,context);
		}
	i4_button_box_class::parent_draw(context);
	}

i4_button_box_class::i4_button_box_class(i4_event_handler_class *receiver,
                                         i4_bool require_one_down)
  : receiver(receiver),
    require_one_down(require_one_down)
{
  current_down=0;
}

i4_button_group_class::i4_button_group_class(i4_const_str &_text,
											 w16 _width,w16 _height,
											 i4_event_handler_class *receiver,
											 w32 _flags)
											 :i4_button_box_class( receiver,
											 (_flags&i4_checkbox_class::TYPE_MASK)==i4_checkbox_class::CHECKBOX),
											 text(_text)
											 
											 
	{
	
	flags=_flags;
	resize(_width,_height);
	}
						

void i4_button_box_class::add_child(i4_coord x, i4_coord y, i4_window_class *child) 
{ 
  i4_error("INTERNAL: You must use add_button() to add buttons to a button_box."); 
}

void i4_button_box_class::expand_if_needed()
{
  win_iter c=children.begin();
  w32 w=width(),h=height();
  for (;c!=children.end(); ++c)
  {
    if (c->x()+c->width()-x()>(sw32)w)
      w=c->x()+c->width()-x();
    if (c->y()+c->height()-y()>(sw32)h)
      h=c->y()+c->height()-y();
  }
  if ((w!=width() || h!=height()))
    resize((w16)w,(w16)h);
}

// when adding a child, enlarge the button box window if nessary to encompass it
void i4_button_box_class::add_button(i4_coord _x, i4_coord _y, i4_button_class *child)
{
  i4_parent_window_class::add_child(_x,_y,child);
  child->set_menu_parent(this);
  expand_if_needed();
}

//void i4_button_group_class::add_button(i4_coord _x, i4_coord _y, i4_checkbox_class *child);
//	{
//	i4_botton_box_class::add_button(_x,_y,child);
//	}

void i4_button_box_class::arrange_right_down()
{
  i4_parent_window_class::arrange_right_down();
  expand_if_needed();
}
 
void i4_button_box_class::arrange_down_right()
{
  i4_parent_window_class::arrange_down_right();
  expand_if_needed();
}

void i4_button_group_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                             i4_event_reaction_class *ev,   // who it was to 
                                             i4_menu_item_class::reaction_type type)
{
  i4_checkbox_class *but=(i4_checkbox_class *)who;

  if ((flags&i4_checkbox_class::TYPE_MASK)!=i4_checkbox_class::CHECKBOX)
	  {
	  return;
	  }
  if (type==i4_menu_item_class::PRESSED)
  {
    if (current_down)
    {
      if (current_down!=but)  // see if we need to depress the current button
      {
        i4_checkbox_class *old_down=(i4_checkbox_class*)current_down;
        current_down=but;
        old_down->set_state(0);
      }  
    } else current_down=but;
  }
  else if (type==i4_menu_item_class::DEPRESSED)
  {
    if (but==current_down)
      ((i4_checkbox_class *)current_down)->set_state(1);  // sorry we need you to stay down
  }
}

void i4_button_box_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                             i4_event_reaction_class *ev,   // who it was to 
                                             i4_menu_item_class::reaction_type type)
{
  i4_button_class *but=(i4_button_class *)who;

  if (type==i4_menu_item_class::PRESSED)
  {
    if (current_down)
    {
      if (current_down!=but)  // see if we need to depress the current button
      {
        i4_button_class *old_down=(i4_button_class*)current_down;
        current_down=but;
        old_down->do_depress();
      }  
    } else current_down=but;
  }
  else if (type==i4_menu_item_class::DEPRESSED)
  {
    if (but==current_down && require_one_down)
      current_down->do_press();         // sorry we need you to stay down
  }
}

void i4_button_group_class::push_button(i4_button_class *which, i4_bool send_event)
	{
	//Remember: Only checkboxes are supposed to be added to the group.
	if (!which) return;
	i4_checkbox_class *w=(i4_checkbox_class *) which;
	if (current_down!=which)
		{
		((i4_checkbox_class*)current_down)->set_state(0);
		}
	current_down=w;
	w->set_state(1);
	if (send_event)
		current_down->send_event(current_down->send.press, i4_menu_item_class::PRESSED); 
	}


void i4_button_box_class::push_button(i4_button_class *which, i4_bool send_event)
{
  if (!which) return;

  if (current_down)
  {
    if (current_down!=which)
    {
      current_down->do_depress(); 
      current_down=which;    
      current_down->do_press();
    }
  } else
  {
    current_down=which;
    current_down->do_press();
  }

  if (send_event)
    current_down->send_event(current_down->send.press, i4_menu_item_class::PRESSED); 
}

// gui/button.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/button.h"
#include "device/kernel.h"
#include "device/keys.h"
#include "window/win_evt.h"

i4_button_class::i4_button_class(const i4_const_str *idle_context_help,  // can be null
                                 i4_window_class *child, 
                                 i4_graphical_style_class *hint,
                                 i4_event_reaction_class *press,
                                 i4_event_reaction_class *depress,
                                 i4_event_reaction_class *activate,
                                 i4_event_reaction_class *deactivate) :
  i4_menu_item_class(idle_context_help,
                     hint,
                     child ? child->width()+4 : 0,
                     child ? child->height()+4 : 0,
                     press,depress,
                     activate,
                     deactivate),
  decore(child)
{
  popup=i4_F;
  state=WAIT_CLICK;
  repeat_down=i4_F;
  repeat_event=0;
  grabbing=i4_F;
  if (child)
    add_child(2,2,child);
}

i4_button_class::~i4_button_class()
{
  if (decore)
  {
    remove_child(decore);
    delete decore;

    if (grabbing)
    {
      I4_ASSERT(parent, "~button/grab/and no parent");
        
      i4_window_request_mouse_ungrab_class ungrab(this);
      i4_kernel.send_event(parent,&ungrab);
    }
  }
  if (state==WAIT_REPEAT||state==WAIT_DELAY)
	  {
	  i4_time_dev.cancel_event(time_id);//we still have an event set up.
	  state=WAIT_CLICK;
	  }
  if (repeat_event) delete repeat_event;
}

void i4_button_class::reparent(i4_image_class *draw_area, i4_parent_window_class *par)
{
  if (grabbing && !draw_area)
  {
    grabbing=i4_F;
    i4_window_request_mouse_ungrab_class ungrab(this);
    i4_kernel.send_event(parent,&ungrab);
  }
  i4_menu_item_class::reparent(draw_area, par);
}


void i4_button_class::receive_event(i4_event *ev)
{
  if (!disabled)
  {
    if (ev->type()==i4_event::OBJECT_MESSAGE)
    {
      CAST_PTR(oev,i4_object_message_event_class,ev);
      if (oev->object==this)
      {
        i4_kernel.not_idle();
        
        if (repeat_event)
          send_event(repeat_event, PRESSED);
        else
          send_event(send.press, PRESSED);

		if (state==WAIT_REPEAT||state==WAIT_DELAY)
			i4_time_dev.cancel_event(time_id);
		i4_object_message_event_class omec1(this);
        time_id=i4_time_dev.request_event(this, 
                                          &omec1,
                                          hint->time_hint->button_repeat);
        state=WAIT_REPEAT;

      } else i4_menu_item_class::receive_event(ev);
    }
    else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
    {
      CAST_PTR(b,i4_mouse_button_down_event_class,ev);
      if (b->but==i4_mouse_button_down_event_class::LEFT)
      {        
        if (pressed)
        {
          do_depress();

          send_event(send.depress, DEPRESSED);
        }
        else 
        {
          do_press();
          send_event(send.press, PRESSED);

          if (!grabbing)
          {
            i4_window_request_mouse_grab_class grab(this);
            i4_kernel.send_event(parent,&grab);
            grabbing=i4_T;
          }
         
          if (repeat_down)
          {         
            i4_kernel.not_idle();
			if (state==WAIT_REPEAT||state==WAIT_DELAY)
				i4_time_dev.cancel_event(time_id);

			i4_object_message_event_class omec2(this);
            time_id=i4_time_dev.request_event(this, 
                                              &omec2,
                                              hint->time_hint->button_delay);
            state=WAIT_DELAY;
          }

        }
      }
    } else if (ev->type()==i4_event::MOUSE_BUTTON_UP)
    {
      CAST_PTR(b,i4_mouse_button_down_event_class,ev);
      if (b->but==i4_mouse_button_down_event_class::LEFT)
      {  
        if (grabbing)
        {
          grabbing=i4_F;
          i4_window_request_mouse_ungrab_class ungrab(this);
          i4_kernel.send_event(parent,&ungrab);
        }
      
        if (state==WAIT_DELAY || state==WAIT_REPEAT)
        {
          i4_time_dev.cancel_event(time_id);
          state=WAIT_CLICK;
        }

        if (pressed && popup)
        {
          do_depress();
          send_event(send.depress, DEPRESSED);
        }

      }
    }
    else if (ev->type()==i4_event::KEY_PRESS)
    {
      CAST_PTR(k,i4_key_press_event_class,ev);
      if (k->key==I4_ENTER)
      {
        if (pressed)
        {
          do_depress();
          send_event(send.depress, DEPRESSED);

        }
        else 
        {
          do_press();
          send_event(send.press, PRESSED);

        }
      } else if (k->key==I4_TAB)
      {
        i4_window_message_class t(i4_window_message_class::REQUEST_NEXT_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &t);
      }
      else if (k->key==I4_LEFT)
      {
        i4_window_message_class l(i4_window_message_class::REQUEST_LEFT_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &l);
      }
      else if (k->key==I4_RIGHT)
      {
        i4_window_message_class r(i4_window_message_class::REQUEST_RIGHT_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &r);
      }
      else if (k->key==I4_UP)
      {
        i4_window_message_class u(i4_window_message_class::REQUEST_UP_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &u);
      }
      else if (k->key==I4_DOWN)
      {
        i4_window_message_class d(i4_window_message_class::REQUEST_DOWN_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &d);
      }
    } else 
      i4_menu_item_class::receive_event(ev);
  } else i4_menu_item_class::receive_event(ev);
}


void i4_button_class::parent_draw(i4_draw_context_class &context)
{
  local_image->add_dirty(0,0,width()-1,height()-1,context);
    
  i4_color_hint_class::bevel *color;
  if (active)
    color=&hint->color_hint->button.active;
  else 
    color=&hint->color_hint->button.passive;
    
  i4_color b,d,m=color->medium;
  if (!pressed)
  {
    b=color->bright;
    d=color->dark;
  } else 
  {
    b=color->dark;
    d=color->bright;
  }

  local_image->bar(0,0,width()-1,0,b,context);
  local_image->bar(0,0,0,height()-1,b,context);    
  local_image->bar(1,1,width()-2,1,m,context);
  local_image->bar(1,1,1,height()-2,m,context);

  local_image->bar(2,height()-2,width()-2,height()-2,d,context);
  local_image->bar(width()-2,2,width()-2,height()-2,d,context);
  local_image->bar(0,height()-1,width()-1,height()-1,hint->color_hint->black,context);
  local_image->bar(width()-1,0,width()-1,height()-1,hint->color_hint->black,context);


  local_image->bar(2,2,width()-3,height()-3,color->medium,context);
}


void i4_button_class::set_popup(i4_bool value)
{
  popup=value;
}

void i4_button_class::set_repeat_down(i4_bool value, i4_event_reaction_class *_repeat_event)
{
  if (!value && (state==WAIT_DELAY || state==WAIT_REPEAT))
    i4_time_dev.cancel_event(time_id);
  state=WAIT_CLICK;
  repeat_down=value;

  if (repeat_event) delete repeat_event;
  repeat_event=_repeat_event;
}

// gui/create_dialog.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.h"
#include "device/event.h"
#include "gui/text.h"
#include "gui/text_input.h"
#include "window/colorwin.h"
#include "error/alert.h"
#include "gui/button.h"
#include "gui/butbox.h"
#include "gui/deco_win.h"

char *tokens[]={"=", "(", ")",
                "[", "]", "right", "down", "up_deco", "down_deco", "'", "text_input", "text",
                "button", "butbox", "obj_ev", "user_ev", "x+", "y+", "checkbox", 0};

enum {
  TK_EQUAL, TK_LPAREN, TK_RPAREN,
  TK_LBRACE, TK_RBRACE, TK_RIGHT, TK_DOWN, TK_UP_DECO, TK_DOWN_DECO, TK_TICK, 
  TK_TEXT_INPUT, TK_TEXT,
  TK_BUTTON, TK_BUTBOX, TK_OBJ_EV,  TK_USER_EV,
  TK_XPLUS, TK_YPLUS, TK_CHECKBOX, TK_NUMBER, TK_POINTER, TK_NONE };
  


void i4_expected(char *why, i4_const_str::iterator i, const i4_const_str &s)
{
  i4_str *er=new i4_str(i4gets("expected"),2000);
  i4_str::iterator x=er->end();
  er->insert(x,why);
  er->insert(er->end()," parsing ");
  er->insert(er->end(),s);
  
 /* while (i!=s.end())
  {
    x.set(i.get());
    ++i;
    ++x;
    er->set_length(er->length()+1);
  }

  i4_warning(why);
  i4_alert(*er, 2000);*/
  i4_error(er->c_str());
  delete er;
}

int i4_is_space(i4_const_str::iterator i)
{
  if (i.get().value()==' ' ||
      i.get().value()=='\t' ||
      i.get().value()=='\n' ||
      i.get().value()=='\r')
    return 1;
  else return 0;
}

int i4_cmp_char_str(i4_const_str::iterator i, const i4_const_str &s, char *c)
{
  while (*c && i!=s.end())
  {
    if (i.get().value()!=*c)
      return 0;

    c++;
    ++i;
  }

  if (*c)
    return 0;
  else return 1;

}

#ifdef _MANAGED
#pragma unmanaged
#endif

int i4_read_dlg_token(i4_const_str::iterator &i,
                      const i4_const_str &fmt,
                      sw32 &num,
                      void *&ptr,
                      va_list &ap)
{
  while (i!=fmt.end() && i4_is_space(i))
    ++i;

  if (i==fmt.end()) return TK_NONE;

  if (i.get().value()=='%')
  {
    ++i;
    if (i.get().value()=='d')
    {
      ++i;
      num=va_arg(ap,int);
      return TK_NUMBER;
    }
    else if (i.get().value()=='p')
    {
      ++i;
      ptr=va_arg(ap,void *);
      return TK_POINTER;
    }
    else i4_error("expecting p or d after %");
  }
  else
  {
    if ((i.get().value()>='0' && i.get().value()<='9') || i.get().value()=='-')
    {
      int neg=0;
      if (i.get().value()=='-')
      {
        neg=1;
        ++i;
      }

      num=0;
      while (i!=fmt.end() && !i4_is_space(i) && i.get().value()>='0' && i.get().value()<='9')
      {
        num=num*10+i.get().value()-'0';
        ++i;
      }

      if (neg) num=-num;
      return TK_NUMBER;
    }

    for (int j=0; tokens[j]; j++)
      if (i4_cmp_char_str(i, fmt, tokens[j]))
      {
        int l=strlen(tokens[j]);
        while (l) 
        {
          ++i;
          l--;
        }
            
        return j;
      }

    i4_expected("unknown token", i, fmt);

  }
  return TK_NONE;
}

#ifdef _MANAGED
#pragma managed
#endif

int i4_next_token_is_rbrace(i4_const_str::iterator i,
                            const i4_const_str &fmt)
{
  while (i!=fmt.end() && i4_is_space(i))
    ++i;

  if (i==fmt.end()) return 1;

  if (i.get().value()==']') return 1;
  else return 0;
}


i4_str *i4_read_str(i4_const_str::iterator &i, const i4_const_str &fmt, va_list &ap)
{
  sw32 n; void *p;

  if (i4_read_dlg_token(i, fmt, n, p, ap)!=TK_TICK)
    i4_expected("'",i,fmt);

  i4_str *s=new i4_str();
  while (i!=fmt.end() && i.get().value()!='\'')
  {
	s->insert(s->end(),i.get());
    ++i;
  }
  ++i;

  //i4_warning(s->c_str());
  i4_str *ret=s->vsprintf(200, ap);
  //i4_warning(ret->c_str());
  delete s;
  return ret;
}

i4_event_reaction_class *i4_read_reaction(i4_const_str::iterator &i, 
                                          const i4_const_str &fmt, 
                                          va_list &ap)
{
  sw32 x,id; void *p, *from, *to;
  int t=i4_read_dlg_token(i, fmt, x, p, ap);

  if (t==TK_OBJ_EV)
  {
    if (i4_read_dlg_token(i, fmt, x, p, ap)!=TK_LPAREN)
      i4_expected("(",i,fmt);

    if (i4_read_dlg_token(i, fmt, x, from, ap)!=TK_POINTER)
      i4_expected("pointer",i,fmt);

    if (i4_read_dlg_token(i, fmt, x, to, ap)!=TK_POINTER)
      i4_expected("pointer",i,fmt);

    if (i4_read_dlg_token(i, fmt, id, p, ap)!=TK_NUMBER)
      i4_expected("number",i,fmt);
  
    if (i4_read_dlg_token(i, fmt, x, p, ap)!=TK_RPAREN)
      i4_expected(")",i,fmt);
    
    i4_object_message_event_class *om;
    om=new i4_object_message_event_class((i4_event_handler_class *)from, id);
    return new i4_event_reaction_class((i4_event_handler_class *)to, om);
  }
  else if (t==TK_USER_EV)
  {
    if (i4_read_dlg_token(i, fmt, x, p, ap)!=TK_LPAREN)
      i4_expected("(",i,fmt);

    if (i4_read_dlg_token(i, fmt, x, to, ap)!=TK_POINTER)
      i4_expected("pointer",i,fmt);

    if (i4_read_dlg_token(i, fmt, id, p, ap)!=TK_NUMBER)
      i4_expected("number",i,fmt);

    if (i4_read_dlg_token(i, fmt, x, p, ap)!=TK_RPAREN)
      i4_expected(")",i,fmt);
    
    i4_user_message_event_class *uev;
    uev=new i4_user_message_event_class(id);

    return new i4_event_reaction_class((i4_event_handler_class *)to, uev);
  }
  else 
  {
    i4_expected("obj_ev or user_ev",i,fmt);
    return 0;
  }
}

i4_window_class *i4_read_object(i4_parent_window_class *parent,
                                i4_graphical_style_class *style,
                                sw32 &cx, sw32 &cy,
                                i4_const_str::iterator &i, const i4_const_str &fmt,
                                va_list &ap,
                                int in_buttonbox)
{
  sw32 x=0;
  void *p=0;
  i4_const_str::iterator start_i=i;
  i4_window_class *ret=0;


  int token=i4_read_dlg_token(i, fmt, x, p, ap);
  switch (token)
  {
    case TK_POINTER :
    {
      token=i4_read_dlg_token(i, fmt, x, p, ap);
      if (token!=TK_EQUAL)
        i4_expected("expected = after %p", start_i, fmt);

      i4_window_class *r=i4_read_object(parent, style, cx, cy, i, fmt, ap, in_buttonbox);
      *((i4_window_class **)p)=r;
      ret=r;
    } break;

    case TK_LBRACE :
    {
      int dir=i4_read_dlg_token(i, fmt, x, p, ap);
      if (dir!=TK_RIGHT && dir!=TK_DOWN)
        i4_expected("right or down after [", start_i, fmt);
     
      sw32 ncx=cx, ncy=cy;
      int max_w=0, max_h=0;
      i4_window_class *r;
      while (!i4_next_token_is_rbrace(i, fmt))
      {
        r=i4_read_object(parent, style, ncx, ncy, i, fmt, ap, in_buttonbox);
        if (r)
        {
          if (!ret) ret=r;
          if (dir==TK_RIGHT)
            ncx+=r->width();
          else ncy+=r->height();
        }        
      }
      i4_read_dlg_token(i, fmt, x, p, ap);

    } break;

    case TK_RIGHT :
    case TK_DOWN :
    case TK_NUMBER :
    case TK_RBRACE :
      i4_expected("out of place token",start_i,fmt);
      break;

    case TK_XPLUS :
    {
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);

      cx+=x;
    } break;

    case TK_YPLUS :
    {
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);
      cy+=x;

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);

    } break;

    case TK_UP_DECO :
    {
      sw32 w,h;
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, w, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, h, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      i4_deco_window_class *cw=new i4_deco_window_class((w16)w,(w16)h, i4_T, style);

      sw32 ncx=0, ncy=0;
      i4_window_class *r=i4_read_object(cw, style, ncx, ncy, i, fmt, ap, 0);
      
      parent->add_child((short)cx,(short)cy, cw);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);

      ret=cw;
    } break;

    case TK_TEXT :
    {
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      i4_str *s=i4_read_str(i, fmt, ap);
      if (s)
      {
        i4_text_window_class *tw=new i4_text_window_class(*s, style);
        delete s;
        parent->add_child((w16)cx, (w16)cy, tw);
        ret=tw;
      }

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);

    } break;

    case TK_TEXT_INPUT :
    {
      sw32 w;
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, w, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      i4_str *s=i4_read_str(i, fmt, ap);
      if (s)
      {
        i4_text_input_class *ti=new i4_text_input_class(style, *s, w, 256);
        delete s;
        parent->add_child((short)cx, (short)cy, ti);
        ret=ti;
      }

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);
    } break;

      
    case TK_BUTTON :
    {
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      i4_color_window_class tmp_win(0,0,0,style);
      i4_window_class *r=i4_read_object(&tmp_win, style, cx, cy, i, fmt, ap, in_buttonbox);      
      i4_event_reaction_class *re=i4_read_reaction(i, fmt, ap);
      
      if (r)
      {
        tmp_win.remove_child(r);

        i4_button_class *b=new i4_button_class(0, r, style, re);
        
        if (in_buttonbox)
          ((i4_button_box_class *)parent)->add_button((short)cx, (short)cy, b);
        else
        {
          b->set_popup(i4_T);
          parent->add_child((short)cx,(short)cy, b);
        }
        ret=b;
      }

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);


    } break;


    case TK_BUTBOX :
    {
      sw32 def_down;

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, def_down, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      
      i4_button_box_class *bbox=new i4_button_box_class(0);
      
      sw32 ncx=0,ncy=0;
      i4_window_class *r=i4_read_object(bbox, style, ncx, ncy, i, fmt, ap, 1);
      bbox->resize_to_fit_children();

      i4_button_class *b=(i4_button_class *)bbox->get_nth_window(def_down);
      if (b)
        bbox->push_button(b,0);

      parent->add_child((short)cx, (short)cy, bbox);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);
      
      return bbox;
    } break;

  }
  return ret;
}

void i4_create_dialog(const i4_const_str &fmt,
                      i4_parent_window_class *parent,
                      i4_graphical_style_class *style,
                      ...)
{
  va_list ap;
  va_start(ap, style);


  i4_const_str::iterator i=fmt.begin();

  sw32 cx=0, cy=0;

  i4_read_object(parent, style, cx, cy, i, fmt, ap, 0);

  va_end(ap);

}

// gui/divider.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/divider.h"
#include "window/style.h"
#include "window/win_evt.h"

void i4_divider_class::reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
{
  if (parent && (w_attached && width()!=parent->width() ||
                 h_attached && height()!=parent->height()))
  {
    int nw=w_attached ? parent->width() : width();
    int nh=h_attached ? parent->height() : height();

    resize(nw,nw);
  }
                 
  i4_parent_window_class::reparent(draw_area, parent);
}

void i4_divider_class::resize(w16 new_width, w16 new_height)
{
  if (w_attached && parent)
    new_width=parent->width();

  if (h_attached && parent)
    new_height=parent->height();

  i4_parent_window_class::resize(new_width, new_height);

  int x1,y1,x2,y2;
  get_drag_area(x1,y1,x2,y2);

  if (split_up_down)
  {   
    if (w1.get())
    {
      remove_child(w1.get());
      if (y1<0) y1=0;
      w1->resize(new_width, y1);
      add_child(0,0,w1.get());
    }
    
    if (w2.get())
    {
      remove_child(w2.get());
      int nh=new_height-y2;
      if (nh<0) nh=0;

      w2->resize(w, nh);
      add_child(0,y2+1, w2.get());
    }
  }
  else
  {
    if (w1.get())
    {
      remove_child(w1.get());
      if (x1<0) x1=0;
      w1->resize(x1, new_height);
      add_child(0,0,w1.get());
    }
    
    if (w2.get())
    {
      remove_child(w2.get());
      int nw=new_width-x2;
      if (nw<0) nw=0;
      w2->resize(nw, new_height);
      add_child(x2+1,0, w2.get());
    }
  }  
}

void i4_divider_class::get_drag_area(int &x1, int &y1, int &x2, int &y2)
{
  w32 l,r,t,b;
  style->get_out_deco_size(l,t,r,b);

  if (split_up_down)
  {
    int vbh=t+b;
    y1=split_value-vbh/2;
    y2=y1+vbh-1; 
    x1=0; x2=width()-1;
  }
  else
  {
    int vbw=l+r;
    x1=split_value-vbw/2;
    x2=x1+vbw-1;
    y1=0;
    y2=height()-1;
  }
}

i4_divider_class::i4_divider_class(int w, int h,
                                   i4_bool split_up_down,
                                   int split_x_or_y,
                                   i4_window_class *window1,
                                   i4_window_class *window2,
                                   i4_graphical_style_class *style,
                                   int window1_min_size,
                                   int window2_min_size)
  : split_up_down(split_up_down),
    split_value(split_x_or_y),
    style(style),
    i4_parent_window_class(w,h),
    min1(window1_min_size),
    min2(window2_min_size)    
{
  w1=window1;
  w2=window2;

  if (w==-1)
  {
    w_attached=i4_T;
    w=0;
  }
  else
    w_attached=i4_F;

  if (h==-1)
  {
    h=0;
    h_attached=i4_T;
  }
  else
    h_attached=i4_F;


  dragging=i4_F;
  if (split_x_or_y==-1)
  {
    if (split_up_down)
      split_value=h/2;
    else
      split_value=w/2;
  }
  
  w1=window1;
  w2=window2;

  add_child(0,0, window1);
  add_child(0,0, window2);
  resize(w,h);
}


void i4_divider_class::parent_draw(i4_draw_context_class &context)
{
  int x1,y1,x2,y2;
  get_drag_area(x1,y1,x2,y2);
  local_image->add_dirty(x1,y1,x2,y2, context);
  style->draw_out_deco(local_image, x1,y1, x2,y2, i4_F, context);
}


void i4_divider_class::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::MOUSE_BUTTON_DOWN :
    {
      CAST_PTR(bev, i4_mouse_button_down_event_class, ev);
      if (bev->left())
      {
        int x1,y1,x2,y2;
        get_drag_area(x1,y1,x2,y2);
        if (bev->x>=x1 && bev->x<=x2 &&
            bev->y>=y1 && bev->y<=y2)
        {
          dragging=i4_T;
          i4_window_request_mouse_grab_class grab(this);
          i4_kernel.send_event(parent, &grab);
        }
        else i4_parent_window_class::receive_event(ev);
      }
      else i4_parent_window_class::receive_event(ev);
    } break;


    case i4_event::MOUSE_BUTTON_UP :
    {        
      CAST_PTR(bev, i4_mouse_button_up_event_class, ev);
      if (dragging && bev->left())
      {
        dragging=i4_F;
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);        
      }
      else i4_parent_window_class::receive_event(ev);
    } break;

    case i4_event::MOUSE_MOVE :
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
      if (dragging)
      {
        if (split_up_down)
        {
          if (mev->y!=split_value && mev->y>min1 && height()-mev->y>min2)
          {
            split_value=mev->y;
            resize(width(), height());
          }
        }
        else if (mev->y!=split_value && mev->x>min1 && width()-mev->x>min2)
        {
          split_value=mev->x;
          resize(width(), height());
        }
      }
      else i4_parent_window_class::receive_event(ev);
    } break;

    default:
      i4_parent_window_class::receive_event(ev);
  }
}

// gui/gradiant.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.h"
#include "palette/pal.h"
#include "image/context.h"

void i4_gradiant_bar(i4_image_class *im, int x1, int y1, int x2, int y2,
                     i4_color start_color, i4_color end_color,
                     i4_draw_context_class &context)
{
  context.add_both_dirty(x1,y1,x2,y2);

  float sr=(float)((start_color>>16)&0xff), sg=(float)((start_color>>8)&0xff), sb=(float)((start_color)&0xff);
  float er=(float)((end_color>>16)&0xff), eg=(float)((end_color>>8)&0xff), eb=(float)((end_color)&0xff);
  
  im->add_dirty(x1,y1,x2,y2, context);

  float t=1.0f/(x2-x1+1);
  float r_step=(er-sr)*t;
  float g_step=(eg-sg)*t;
  float b_step=(eb-sb)*t;


  int w=(x2-x1+1);
  int h=(y2-y1+1);
  if (w*h*4<32*1024)  // do it fast if it's small than 32k
  {        
    i4_image_class *fast=i4_create_image(w,h, i4_pal_man.default_no_alpha_32());
    
    for (int x=0; x<w; x++)
    {
      w32 c=(((int)sr)<<16) | (((int)sg)<<8) | (((int)sb));

      w32 *sl=((w32 *)fast->data)+x;
      for (int y=0; y<h; y++)
      {
        *sl=c;
        sl+=w;
      }

      sr+=r_step;  sg+=g_step;  sb+=b_step;
    }

    fast->put_image(im, x1,y1, context);
    delete fast;
  }
  else
  { 
    for (int x=x1; x<=x2; x++)
    {
      w32 c=(((int)sr)<<16) | (((int)sg)<<8) | (((int)sb));
      im->bar(x,y1,x,y2, c, context);

      sr+=r_step;  sg+=g_step;  sb+=b_step;
    }
  }
}

// gui/image_win.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/image_win.h"

#include "window/window.h"
#include "window/win_evt.h"

#include "image/image.h"


void i4_image_window_class::reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
{
  if (draw_area)
  {
    if (darken_im)
      delete darken_im;

    if (darken && im)
    {
      int x,y, w=im->width(), h=im->height();
      
      i4_image_class *d=i4_create_image(w, h, draw_area->get_pal());

      for (y=0; y<h; y++)
        for (x=0; x<w; x++)
        {
          i4_color c=im->get_pixel(x,y);
          int r= (((c>>16)&0xff)*3/4)<<16;
          int g= (((c>>8)&0xff)*3/4)<<8;
          int b= (((c>>0)&0xff)*3/4)<<0;

          d->put_pixel(x,y, r|g|b);
        }

      darken_im=d;

      
    }
    else darken_im=0;
  }
  i4_parent_window_class::reparent(draw_area, parent);
}


void i4_image_window_class::change_image(i4_image_class *new_im)
{
  if (del && im)
    delete im;

  im = new_im;
  reparent(local_image, parent);
}


i4_image_window_class::i4_image_window_class(i4_image_class *im,
                                             i4_bool delete_on_destructor,
                                             i4_bool dark_when_not_active)
  : i4_parent_window_class(im->width(),im->height()),
    im(im),
    del(delete_on_destructor),
    darken(dark_when_not_active)
{
  active=i4_F;
  darken_im=0;
}

void i4_image_window_class::parent_draw(i4_draw_context_class &context)
{
  if (darken && !active && darken_im)
    darken_im->put_image(local_image, 0,0, context);
  else
  {
    i4_rect_list_class child_clip(&context.clip,0,0);
    child_clip.intersect_list(&undrawn_area);

    child_clip.swap(&context.clip);

    i4_rect_list_class::area_iter c;
    for (c=context.clip.list.begin(); c!=context.clip.list.end();++c)
      {
      i4_coord nx1,ny1,nx2,ny2;
      nx1=c->x1;
      nx2=c->x2;
      ny1=c->y1;
      ny2=c->y2;
      im->put_part(local_image, nx1, 
          ny1, nx1, ny1, nx2, ny2, 
	  context);
      }

    
    child_clip.swap(&context.clip);
    i4_parent_window_class::parent_draw(context);

  }

}

void i4_image_window_class::receive_event(i4_event *ev)
{
  CAST_PTR(wev, i4_window_message_class, ev);
  if (ev->type()==i4_event::WINDOW_MESSAGE && 
      wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
  {
    active=i4_T;
    if (darken && darken_im)
      request_redraw(i4_F);
  }
  else if (ev->type()==i4_event::WINDOW_MESSAGE && 
           wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
  {
    active=i4_F;
    if (darken && darken_im)
      request_redraw(i4_F);

  }
  
  i4_parent_window_class::receive_event(ev);
}

i4_image_window_class::~i4_image_window_class()
{
  if (del && im)
    delete im;
  if (darken_im)
    delete darken_im;
}

//gui/li_pull_menu.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "menu/pull.h"
#include "menu/key_item.h"
#include "lisp/lisp.h"
#include "app/app.h"
#include "menu/textitem.h"
#include "gui/seperate.h"
#include "device/key_man.h"
#include "window/wmanager.h"

i4_event_handler_reference_class<i4_pull_menu_class> li_pull;

static li_object *li_add_sub(li_object *o, li_environment *env)
{
  i4_graphical_style_class *style=i4_current_app->get_style();

  i4_menu_item_class *top_name;
  top_name=new i4_text_item_class(li_string::get(li_eval(li_car(o,env),env),env)->value(),
                                  style,
                                  &li_pull->menu_colors,                                   
                                  0,0,0,0,0,  5,3);

  i4_menu_class *sub=style->create_menu(i4_T);

  for (o=li_cdr(o,env); o; o=li_cdr(o,env))
  {
    if (li_car(o,env)->type()!=LI_LIST)
    {
      sub->add_item(new i4_seperator_line_class(style,
                                                li_pull->menu_colors.text_background,
                                                3,4));
    }
    else
    {
      li_object *s=li_car(o,env);
      char *view_name=li_string::get(li_eval(li_car(s,env),env),env)->value(); 
      s=li_cdr(s,env);
      char *com_name=li_string::get(li_eval(li_car(s,env),env),env)->value();
      char *check_name=0;
      s=li_cdr(s,env);
      if (s)
          {
          check_name=li_string::get(li_eval(li_car(s,env),env),env)->value();
          }

      int command_id=i4_key_man.get_command_id(com_name);

      i4_do_command_event_class *do_cmd=new i4_do_command_event_class(com_name, command_id);
      i4_event_reaction_class *command=new i4_event_reaction_class(i4_current_app, do_cmd);

      i4_end_command_event_class *end_cmd=new i4_end_command_event_class(com_name, command_id);
      i4_event_reaction_class *end_command=new i4_event_reaction_class(i4_current_app, end_cmd);


      i4_text_item_class *ki=new i4_text_item_class(view_name, 
                                                    style,
                                                   &li_pull->menu_colors, 
                                                    0,
                                                   command, end_command,
                                                   0,0,
                                                   10,3,
                                                   check_name);
      sub->add_item(ki);
    }
  }


  li_pull->add_sub_menu(top_name, sub);
  return 0;
}

LI_HEADER(clear_subs)
	{
	li_pull->remove_all_menus();
	return 0;
	}

i4_pull_menu_class *li_create_pull_menu(char *filename)
{
  if (li_pull.get())
      {
      i4_warning("WARN: Pull menu already created");
      li_clear_subs(0,0);
      }


  i4_graphical_style_class *style=i4_current_app->get_style();
  li_pull=new i4_pull_menu_class(style, i4_current_app->get_window_manager());

  li_environment *env=new li_environment(0, i4_T);
  //li_add_function("add_sub_menu", li_add_sub, env);//declare at load-time
  li_load(filename, env);
	
  return li_pull.get();
}



//if possible, delete the assigned menu instance complete.
li_automatic_add_function(li_clear_subs,"remove_sub_menus");//Clears out the menu-bar

li_automatic_add_function(li_add_sub,"add_sub_menu");

// gui/list_box.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/list_box.h"
#include "gui/button.h"
#include "gui/deco_win.h"
#include "gui/image_win.h"

void i4_list_box_class:: set_top(i4_menu_item_class *item)
{
  if (top)
    delete top;

  top=item->copy();

  int h=top->height() >= height()-(t+b) ? top->height() : height()-(t+b);

  top->set_menu_parent(this);
  top->resize((w16)(width()-(l+r)-down->width()), (w16)h);
  top->do_deactivate();

  i4_parent_window_class::add_child((short)l,(short)t, top);
}

i4_list_box_class::~i4_list_box_class()
{
  hide();

  for (int i=0; i<entries.size(); i++)
    delete entries[i];

  if (top)
    delete top;
  if (reaction)
      delete reaction;

}

i4_list_box_class::i4_list_box_class(w16 w,         
                                     i4_graphical_style_class *style,
                                     i4_parent_window_class *root_window,
                                     i4_event_reaction_class *react)

  : i4_menu_class(i4_F), entries(0,8), style(style), 
  root_window(root_window),reaction(react)
{
  i4_image_class *down_im=style->icon_hint->down_icon;

  down=new i4_button_class(0, new i4_image_window_class(down_im), style,
                           0,
                           new i4_event_reaction_class(this, 1));
  down->set_popup(i4_T);

  l=1;  r=1;  t=1;  b=1;

  resize((w16)w, (w16)(down->height()+(t+b)));

  i4_parent_window_class::add_child((short)(w-down->width()-r), (short)t, down);
  top=0;
  pull_down=0;
  current=0;
}


void i4_list_box_class::parent_draw(i4_draw_context_class &context)
{
  local_image->clear(0, context);
  //  style->draw_in_deco(local_image, 0,0,width()-1, height()-1, i4_F, context);
}


void i4_list_box_class::add_item(i4_menu_item_class *item)
{  


  if (item->height()+t+b>height())
    resize((w16)width(), (w16)(item->height()+t+b));

  if (entries.size()==0)
    set_top(item);

  entries.add(item);

  item->set_menu_parent(this);
}


void i4_list_box_class::set_current_item(int entry_num)
{
  if (entry_num>=0 && entry_num<entries.size())
  {
    set_top(entries[entry_num]);
    current=entry_num;
  }
}

class i4_list_pull_down_class : public i4_deco_window_class
{
  i4_window_class *buddy;
public:
  i4_list_pull_down_class(w16 w, w16 h, i4_window_class *buddy, i4_graphical_style_class *style) 
    : i4_deco_window_class(w,h, i4_T, style), buddy(buddy)
  {
  }
 
  void grab()
  {
    i4_window_request_mouse_grab_class grab(this);
    i4_kernel.send_event(parent, &grab);
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
      if (mev->x<0 || mev->y<0 || mev->x>=width() || mev->y>=height())
      {        
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);

        i4_user_message_event_class uev(1);
        i4_kernel.send_event(buddy, &uev);
      }
    }

    i4_deco_window_class::receive_event(ev);
  }

  void name(char* buffer) { static_name(buffer,"list_box_pull_down"); }
};

void i4_list_box_class::show(i4_parent_window_class *show_on, i4_coord px, i4_coord py)
{
  if (!pull_down)
  {

    int i, y=0, x;

    for (i=0; i<entries.size(); i++)
      y+=entries[i]->height();

    pull_down=new i4_list_pull_down_class((w16)(width()-(l+r)), (w16)y, this, style);
    y=pull_down->get_y1();
    x=pull_down->get_x1();

    for (i=0; i<entries.size(); i++)
    {
      entries[i]->resize(width(), entries[i]->height());

      pull_down->add_child(x,y, entries[i]);
      y+=entries[i]->height();
    }

    show_on->add_child(px,py, pull_down);
    pull_down->grab();
  }
}


void i4_list_box_class::hide()
{
  if (pull_down)
  {

    for (int i=0; i<entries.size(); i++)
      pull_down->remove_child(entries[i]);

    root_window->remove_child(pull_down);
    delete pull_down;
    pull_down=0;
  }
}



void i4_list_box_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    if (!pull_down) 
      show(root_window, x(), y()+height());
    else hide();
  }
  else 
    i4_menu_class::receive_event(ev);
}


void i4_list_box_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                           i4_event_reaction_class *ev,   // who it was to 
                                           i4_menu_item_class::reaction_type type)
{
  if (type==i4_menu_item_class::PRESSED)
  {
    if (who==top)
      show(root_window, x(), y()+height());
    else
    {
      for (int i=0; i<entries.size(); i++)
          if (entries[i]==who)        
              set_current_item(i);
      
          //if this is a listbox that should immediatelly trigger
          //an action if something changed (like in the file-open dialog)
          //we send this event around.
      if (reaction&&reaction->handler_reference.get())
          {
          i4_kernel.send_event(reaction->handler_reference.get(),
            reaction->event);
          }
      hide();
    }
  }
}

// gui/scroll_bar.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/scroll_bar.h"
#include "window/colorwin.h"
#include "image/image.h"
#include "gui/button.h"
#include "gui/image_win.h"
#include "window/win_evt.h"



inline void draw_out_deco(i4_image_class *screen,
                          i4_draw_context_class &context,
                          i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                          i4_color bright, i4_color medium, i4_color dark, i4_color black)
{
  screen->add_dirty(x1,y1,x2,y2,context);

  screen->bar(x1,y1,x2-1,y1, bright, context);
  screen->bar(x1,y1,x1,y2-1, bright, context);
  screen->bar(x1,y2-1,x2-1,y2-1, dark, context);
  screen->bar(x2-1,y1+1,x2-1,y2-1, dark, context);

  screen->bar(x1,y2,x2,y2, black, context);
  screen->bar(x2,y1,x2,y2, black, context);
  screen->bar(x1+1, y1+1, x2-2, y2-2, medium, context);
  
}


class i4_scroll_button : public i4_window_class
{
  i4_bool active,dragging;
  i4_scroll_bar *buddy;

  void reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
  {
    i4_window_class::reparent(draw_area, parent);
    if (parent)
      fit_parent();
  }

  i4_bool vertical() { return buddy->vertical; }
  i4_graphical_style_class *style() { return buddy->style; }



public:
  void fit_parent()
  {   
    if (vertical())
        {
        w16 he=parent->height();
        //must not divide if we have zero elements in the list.
        if (buddy->total_scroll_objects)
            he=(w16)(parent->height() * buddy->total_visible_objects / 
                buddy->total_scroll_objects);
        //otherwise the scroll button might vanish.
        if (he<=10)
            he=10;
        resize((w16)parent->width(), he);
        }
    else
        {
        w16 wi=parent->width();
        if (buddy->total_scroll_objects)
            wi=(w16)(parent->width() * buddy->total_visible_objects / 
                buddy->total_scroll_objects);
        if (wi<=10)
            wi=10;
        resize(wi,
             (w16)parent->height());
        }
  }

  void name(char* buffer) { static_name(buffer,"scroll_button"); }


  i4_scroll_button(i4_scroll_bar *buddy)
    : i4_window_class(0,0), 
      buddy(buddy)
  {
    active=i4_F;
    dragging=i4_F;
  }

  void draw(i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    if (active)
      color=&style()->color_hint->window.active;
    else 
      color=&style()->color_hint->window.passive;

    draw_out_deco(local_image,context,
                  0,0,width()-1,height()-1,
                  color->bright,
                  color->medium,
                  color->dark,
                  style()->color_hint->black);
  }

  void receive_event(i4_event *ev)
  {
    switch (ev->type())
    {
      case i4_event::WINDOW_MESSAGE :
      {
        CAST_PTR(wev,i4_window_message_class,ev);
        if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
        {
          active=i4_T;
          request_redraw();
        }
        else if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
        {
          active=i4_F;
          request_redraw();
        }
      } break;
      case i4_event::MOUSE_MOVE :
      {
        CAST_PTR(mev,i4_mouse_move_event_class,ev);
        if (dragging)
        {
          if (vertical())
          {
            i4_coord new_y=mev->y+y()-parent->y()-height()/2;  // convert to parent coords
            if (new_y<0)
              new_y=0;
            else if (new_y+height()>=parent->height())
              new_y=parent->height()-height();

            new_y=new_y+parent->y()-y();  // convert back to our coord system
            if (new_y!=y())
            {
              move(0,new_y);
              buddy->calc_pos();
              buddy->send_position();
            }
          }
          else
          {
            i4_coord new_x=mev->x+x()-parent->x()-width()/2;  // convert to parent coords
            if (new_x<0)
              new_x=0;
            else if (new_x+width()>=parent->width())
              new_x=parent->width()-width();

            new_x=new_x+parent->x()-x();  // convert back to our coord system
            if (new_x!=x())
            {
              move(new_x, 0); 
              buddy->calc_pos();
              buddy->send_position();
            }
          }
        }
      } break;

      case i4_event::MOUSE_BUTTON_DOWN :
      {
        CAST_PTR(bev,i4_mouse_button_down_event_class,ev);
        if (bev->but==i4_mouse_button_down_event_class::LEFT)
        {
          dragging=i4_T;
          i4_window_request_mouse_grab_class grab(this);
          i4_kernel.send_event(parent,&grab);
        }
      } break;

      case i4_event::MOUSE_BUTTON_UP :
      {
        CAST_PTR(bev,i4_mouse_button_up_event_class,ev);
        if (bev->but==i4_mouse_button_up_event_class::LEFT)
        {
          dragging=i4_F;
          i4_window_request_mouse_ungrab_class ungrab(this);
          i4_kernel.send_event(parent,&ungrab);
        }
      } break;


    }
  }

} ;


// if total items under control changes
void i4_scroll_bar::set_new_total(int total)
{
  total_scroll_objects=total;
  if (total_scroll_objects<=0)
    total_scroll_objects=1;

  scroll_but->fit_parent();
}

i4_button_class *i4_scroll_bar::create_button(i4_button_class *&b, i4_image_class *im)
{
  b=new i4_button_class(0, new i4_image_window_class(im), style);

  b->send.press=new i4_event_reaction_class(this, new i4_object_message_event_class(b));
  b->set_repeat_down(i4_T);
  b->set_popup(i4_T);
  
  return b;
}

i4_scroll_bar::i4_scroll_bar(i4_bool vertical,
                             int max_dimention_size,
                             int total_visible_objects,
                             int total_scroll_objects,    // total number of objects
                             w32 message_id,
                             i4_event_handler_class *send_to,
                             i4_graphical_style_class *style)

  : i4_parent_window_class(0,0),
    vertical(vertical),
    total_scroll_objects(total_scroll_objects),
    total_visible_objects(total_visible_objects),
    id(message_id),
    send_to(send_to),
    style(style)
{
  pos=0;
  if (vertical)
  {
    add_child(0,0, create_button(up_but, style->icon_hint->up_icon));

    create_button(down_but, style->icon_hint->down_icon);
    add_child(0, max_dimention_size-down_but->height(), down_but);

    left_but=right_but=0;

    resize(up_but->width(), max_dimention_size);
  }
  else
  {
    add_child(0,0, create_button(left_but, style->icon_hint->left_icon));

    create_button(right_but, style->icon_hint->right_icon);
    add_child(max_dimention_size-right_but->width(),0, right_but);
    up_but=down_but=0;    

    resize(max_dimention_size,  left_but->height());
  }


  int x=vertical ? 0 : left_but->width(),
    y=vertical ? up_but->height() : 0;
  int sa_w=vertical ? width() : width() - left_but->width() - right_but->height(),
    sa_h=vertical ? height() - up_but->width() - down_but->height() : height();

  // area where scroll grab button resides
  scroll_area=i4_add_color_window(this,
                                  style->color_hint->window.passive.dark,
                                  style, x,y, sa_w, sa_h);


  scroll_but=new i4_scroll_button(this);
  scroll_area->add_child(0,0,scroll_but);

}


  
void i4_scroll_bar::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(oev,i4_object_message_event_class,ev);
    if (oev->object==down_but || oev->object==right_but)
    {    
      if (pos<total_scroll_objects-1)
      {
        pos+=1;
        set_bar_pos(pos);
        send_position();
      }           
    } else if (oev->object==up_but || oev->object==left_but)
    {
      if (pos>0)
      {
        pos-=1;
        set_bar_pos(pos);
        send_position();
      }           
    }

  } else i4_parent_window_class::receive_event(ev);
}

void i4_scroll_bar::set_bar_pos(sw32 pos)
{ 
  sw32 reverse_pos;
  sw32 realpos;
  if (vertical)
  {
    if (total_scroll_objects<=1)
      reverse_pos=0;
    else
      reverse_pos=pos*(scroll_area->height()-scroll_but->height())/(total_scroll_objects-1);
    //This would actually not be needed if the caller always correctly
    //computed the size of the scrollbar. But it might fix a small 
    //inconvenience in case where the caller repeadedly increases
    //the size of the controlled contents (i.e a terminal window).
    //In such cases it might otherwise happen that the scroll button
    //vanishes after an automatic scroll. 
    if (reverse_pos>(scroll_area->height()-scroll_but->height()))
        reverse_pos=(scroll_area->height()-scroll_but->height());
    realpos=reverse_pos - (scroll_but->y()- scroll_area->y());
    scroll_but->move(0,realpos);
  }
  else
  {
    if (total_scroll_objects<=1)
      reverse_pos=0;
    else
      reverse_pos=pos*(scroll_area->width()-scroll_but->width())/(total_scroll_objects-1);

    if (reverse_pos>(scroll_area->width()- scroll_but->width()))
        reverse_pos=(scroll_area->width()- scroll_but->width());
    scroll_but->move(reverse_pos- (scroll_but->x()-scroll_area->x()),0);
  }
                   
}

void i4_scroll_bar::set_pos(sw32 _pos)
    {
    pos=_pos;
    set_bar_pos(pos);
    }

void i4_scroll_bar::calc_pos()
{
  if (total_visible_objects>=total_scroll_objects || total_scroll_objects<=1)
    pos=0;
  else if (vertical)
    pos=(scroll_but->y()-scroll_area->y()) * (total_scroll_objects-1) / 
      (scroll_area->height()-scroll_but->height());
  else
    pos=(scroll_but->x()-scroll_area->x()) * (total_scroll_objects-1) / 
      (scroll_area->width()-scroll_but->width());

}

void i4_scroll_bar::send_position()
{
  if (send_to)
  {
    //    i4_vscroll_button *b=(i4_vscroll_button *)scroll_but;
    i4_scroll_message message(pos, total_scroll_objects, id);
    i4_kernel.send_event(send_to,&message);
  }  
}

// gui_slider.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/slider.h"
#include "window/style.h"
#include "window/win_evt.h"

i4_slider_class::i4_slider_class(sw32 width, 
                                 sw32 initial_start,
                                 i4_event_handler_class *notify,
                                 w32 milli_delay,
                                 i4_graphical_style_class *style)
  : i4_window_class((w16)width,style->font_hint->normal_font->largest_height()+2),
    style(style),
    notify(notify),
    milli_delay(milli_delay)
{    
  off=initial_start;
  grab=i4_F;
  active=i4_F;
  w32 ol,o_r,ot,ob;//gcc doesn't like or as identifier
  style->get_out_deco_size(ol,ot,o_r,ob);
  bw=ol+o_r+2;
  need_cancel=i4_F;
}

void i4_slider_class::draw(i4_draw_context_class &context)
{
  w32 il,ir,it,ib;
  w32 med=style->color_hint->window.passive.medium;
  local_image->clear(med, context);

  style->get_in_deco_size(il,it,ir,ib);
  style->draw_in_deco(local_image, 0,(short)(height()/2-it),width()-1,(short)(height()/2+ib), active, context);

  w32 ol,o_r,ot,ob;
  style->get_out_deco_size(ol,ot,o_r,ob);
  style->draw_out_deco(local_image, (short)off,0,(short)(off+bw-1),height()-1, active, context);  
  local_image->bar((short)(off+ol), (short)ot, (short)(off+bw-o_r), (short)(height()-ob), med, context);
}


void i4_slider_class::send_change()
{
  if (notify)
  {
    if (need_cancel)
    {
      i4_time_dev.cancel_event(t_event);
      need_cancel=i4_F;
    }

    i4_slider_event ev(this, off, width()-bw);

    if (milli_delay)
    {
      t_event=i4_time_dev.request_event(notify, &ev, milli_delay);
      need_cancel=i4_T;
    } else i4_kernel.send_event(notify, &ev);
  }
}


i4_slider_class::~i4_slider_class()
{
  if (need_cancel)
  {
    milli_delay=0;
    send_change();
  }
}


void i4_slider_class::set_off_from_mouse()
{
  w32 ol,o_r,ot,ob;
  sw32 loff=off;

  style->get_out_deco_size(ol,ot,o_r,ob);
  off=lx-bw/2;
  if (off<0) off=0;
  if (off+bw>=width())
    off=width()-bw;

  if (loff!=off)
  {
    send_change();
    request_redraw();
  }
}

void i4_slider_class::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::WINDOW_MESSAGE :
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      {
        active=i4_T;
        request_redraw();
      }
      else if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      {
        active=i4_F;
        request_redraw();
      }
    } break;

    case i4_event::MOUSE_MOVE :
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
      lx=mev->x;
      ly=mev->y;
        
      if (grab)
        set_off_from_mouse();                

    } break;

    case i4_event::MOUSE_BUTTON_DOWN :
    {
      CAST_PTR(bev, i4_mouse_button_down_event_class, ev);
      if (bev->left() && !grab)
      {
        grab=i4_T;
        i4_window_request_mouse_grab_class grab(this);
        i4_kernel.send_event(parent,&grab);
        set_off_from_mouse();
      }        

    } break;
    case i4_event::MOUSE_BUTTON_UP :
    {
      CAST_PTR(bev, i4_mouse_button_up_event_class, ev);
      if (grab && bev->left())
      {
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent,&ungrab);
        grab=i4_F;
      }
    } break;
  }
}

// gui/smp_dial.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/smp_dial.h"
#include "window/window.h"
#include "device/event.h"
#include "device/kernel.h"
#include "window/win_evt.h"
#include "image/image.h"
#include "window/style.h"
#include "gui/button.h"
#include "gui/text.h"

g1_message_box *g1_message_box::dialog_active=0;
w32 g1_message_box::modal_result=0;
//i4_str *g1_message_box::text=0;
g1_message_box::g1_message_box(w16 w, w16 h, i4_graphical_style_class *style,
							   //i4_const_str &title, i4_const_str &message,
							   w32 flags)
							   : style(style),
							   i4_color_window_class(w,h,style->color_hint->neutral(),style),
							   bg_color(style->color_hint->window.active.medium),
							   flags(flags)
	{
	modal_result=0;
	inputwindow=0;
	dialog_active=this;
	textual_event=0;
	checkbox_event=0;
	send_to=0;
	modal=(flags&MSG_NOTMODAL)?0:1;
	//text=0;
	};

g1_message_box::~g1_message_box()
	{
	//i4_color_window_class::~i4_color_window_class();
	//text=0;
	//modal_result=0;//will be read just after destruction of the window.
	//dialog_active=0;
	}

i4_str g1_message_box::get_text()
    {
	if (inputwindow)
        return *inputwindow->get_edit_string();
    else 
        return i4_str("");
    }


void g1_message_box::receive_event(i4_event *ev)
	{
	
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      /*if (uev->sub_type==MSG_INPUTBOX)
      {
        text=*inputwindow->get_edit_string();
		return;
      }*/
      
	  modal_result=uev->sub_type;
	  //if (inputwindow) text=inputwindow->get_edit_string();
	  //else text=0;
	  if (flags&MSG_NOTMODAL) 
		  {
		  style->close_mp_window(parent);
		  dialog_active=0;
		  }

    } 
	else
	{
		if (ev->type()==i4_event::OBJECT_MESSAGE) 
			{
			CAST_PTR(oev,i4_object_message_event_class,ev);
			if (oev->object==inputwindow)
				{
				//text=inputwindow->get_edit_string();
				return;
				}
			}

		i4_parent_window_class::receive_event(ev);
	}
	};

class i4_simple_dlg_class : public i4_parent_window_class
{
public:
  enum { YES, NO };

  void name(char* buffer) { static_name(buffer,"simple dlg"); }
  i4_event_handler_class *send_to;
  i4_event *yes_event, *no_event;
  i4_color bg_color;
  i4_graphical_style_class *style;
    
  i4_simple_dlg_class(w16 w, w16 h,
                      i4_graphical_style_class *style,
                      i4_event_handler_class *send_to,
                      i4_event *yes_event, i4_event *no_event)
    : i4_parent_window_class(w,h),
      yes_event(yes_event), no_event(no_event),
      style(style),
      send_to(send_to),
      bg_color(style->color_hint->window.passive.medium)
  {}

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      if (uev->sub_type==YES)
      {
        i4_kernel.send_event(send_to, yes_event);        
        style->close_mp_window(parent);
      }
      else if (uev->sub_type==NO)
      {
        i4_kernel.send_event(send_to, no_event);
        style->close_mp_window(parent);
      }
    } else i4_parent_window_class::receive_event(ev);
  }

  void parent_draw(i4_draw_context_class &context)
  {
    local_image->clear(bg_color, context);
  }

};

i4_button_class *i4_simple_create_button(const i4_const_str &name, 
                                                i4_event_handler_class *send_to,
                                                i4_event *ev,
                                                i4_graphical_style_class *style)
{
  i4_button_class *b=new i4_button_class(0,
                                         new i4_text_window_class(name, style),
                                         style,
                                         new i4_event_reaction_class(send_to, ev));
  b->set_popup(i4_T);
  return b;

}

i4_parent_window_class *i4_create_yes_no_dialog(i4_parent_window_class *parent,
                                                i4_graphical_style_class *style,
                                                const i4_const_str &title,
                                                const i4_const_str &message,
                                                const i4_const_str &yes, const i4_const_str &no,
                                                i4_event_handler_class *send_to,
                                                i4_event *yes_event, i4_event *no_event)
{
  i4_parent_window_class *root=parent;


  i4_simple_dlg_class *sd=new i4_simple_dlg_class(100,50,
                                                  style,
                                                  send_to,
                                                  yes_event, no_event);


  i4_button_class *y=i4_simple_create_button(yes, sd, 
                                            new i4_user_message_event_class(0), style);
  i4_button_class *n=i4_simple_create_button(no, sd,
                                            new i4_user_message_event_class(1), style);
  i4_text_window_class *t=new i4_text_window_class(message, style);

  sw32 w=t->width(),h=t->height()+2+y->height();
  sw32 bw=y->width()+n->width();
  if (w<bw)
    w=bw;

  sd->resize((w16)w,(w16)h);

  i4_parent_window_class *p; 
  p=style->create_mp_window(parent->width()/2-w/2, parent->height()/2-h/2, (w16)w,(w16)h, title);



  sd->add_child(p->width()/2-bw/2, 0, y);
  sd->add_child(p->width()/2-bw/2+y->width(), 0, n);
  sd->add_child(p->width()/2-t->width()/2, y->height()+1, t);

  p->add_child(0,0,sd);
  return p;
}

char *button_names[MSG_LASTBUTTON+1]=//only the power-of-two values are used
	{
	"Nobutton",//0
	"yes",//1
	"no",//2
	"yesno",
	"ok",//4
	"yesok",
	"okno",
	"okyesno",
	"cancel",//8
	"yescancel",
	"nocancel",
	"yesnocancel",
	"",
	"",
	"",
	"",
	"ignore",//16
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"abort"//32
	};

w32 i4_input_box(const i4_const_str &title, const i4_const_str &message, const i4_const_str &preset, i4_str &rettext, w32 flags)
{
  i4_parent_window_class *root=i4_current_app->get_root_window();
  i4_graphical_style_class *style=i4_current_app->get_style();

  if (flags==0) flags=4; //no buttons-> ok button
  i4_button_class *buttons[MSG_LASTBUTTON+1];//much too large, i know, but 
  //this is only an array of pointers, so there's no trouble in wasting space.
  
  g1_message_box *sd=new g1_message_box(50,100,
                                         style,
                                         flags);

  sw32 w=0,h=0,bw=2;
  i4_button_class *cb;
  int numbuttons=0;
  int i=0;
  for (;i<MSG_LASTBUTTON+1;i++)
	  {
	  buttons[i]=0;//zero array out
	  }
  i=1;
  while (i<=MSG_LASTBUTTON)
	  {
	  if (i&flags) 
		  {
		  numbuttons++;
		  i4_const_str s=i4gets(button_names[i]);
		  cb=i4_simple_create_button(s,sd,new i4_user_message_event_class(i),style);
	      buttons[i]=cb;
	      bw+=2+cb->width();
	      if (cb->height()>h) h=cb->height();
		  }
	  i<<=1;
	  }
  if (numbuttons==0) 
	  {
	  i4_error("INTERNAL: i4_message_box: No buttons requested");
	  return 0;
	  }

  //i4_button_class *y=i4_simple_create_button(yes, sd, 
  //                                          new i4_user_message_event_class(0), style);
  //i4_button_class *n=i4_simple_create_button(no, sd,
  //                                          new i4_user_message_event_class(1), style);
  i4_text_window_class *t=new i4_text_window_class(message, style, 0, 300);
  w=t->width()+2;
  if (w<150) w=150;//avoid that the input-box gets to small. 
  sd->inputwindow=new i4_text_input_class(style,preset,w-4,300,
	  sd);
  h+=sd->inputwindow->height()+4;

  
  h+=t->height()+4;
  
  if (w<bw)
    w=bw;

  sd->resize((w16)w,(w16)h);

  i4_parent_window_class *p; 
  p=style->create_modal_window(root->width()/2-w/2, root->height()/2-h/2, (w16)w,(w16)h, title,
	  false);


  i=0;
  sw32 x=(w/2)-(bw/2)-2,y=t->height()+4+sd->inputwindow->height();
  while (i<MSG_LASTBUTTON+1)
	  {
	  if (buttons[i])
		  {
		  sd->add_child((short)x,(short)y,buttons[i]);
		  x+=buttons[i]->width()+2;
		  }
	  i++;
	  }

  //sd->add_child(p->width()/2-bw/2, 0, y);
  //sd->add_child(p->width()/2-bw/2+y->width(), 0, n);
  sd->add_child(p->width()/2-t->width()/2, 1, t);
  sd->add_child(1,t->height()+2,sd->inputwindow);

  p->add_child(0,0,sd);
  if (flags&MSG_NOTMODAL)
	  {
	  return 0;
	  }
  else
	  {
	  i4_current_app->refresh();
	  int result=0;
	  
	  while (result==0)
		  {
		  i4_current_app->get_input();//force message to be modal
		  //let's try wheter this helps. (at least it's modal to current context)
		  i4_current_app->refresh();

	      rettext=sd->get_text();
          //this will automatically close and delete the window
          //when the return value is non-zero.
		  result= g1_message_box::get_result();
		  }
	  
	  return result;

	  }
  
}

w32 i4_message_box(const i4_const_str &title, const i4_const_str &message, w32 flags)
{
  i4_parent_window_class *root=i4_current_app->get_root_window();
  i4_graphical_style_class *style=i4_current_app->get_style();

  if (flags==0) flags=4; //no buttons-> ok button
  i4_button_class *buttons[MSG_LASTBUTTON+1];//much too large, i know, but 
  //this is only an array of pointers, so there's no trouble in wasting space.
  
  g1_message_box *sd=new g1_message_box(50,100,
                                         style,
                                         flags);

  sw32 w=0,h=0,bw=2;
  i4_button_class *cb;
  int numbuttons=0;
  int i=0;
  for (;i<MSG_LASTBUTTON+1;i++)
	  {
	  buttons[i]=0;//zero array out
	  }
  i=1;
  while (i<=MSG_LASTBUTTON)
	  {
	  if (i&flags) 
		  {
		  numbuttons++;
		  i4_const_str s=i4gets(button_names[i]);
		  cb=i4_simple_create_button(s,sd,new i4_user_message_event_class(i),style);
	      buttons[i]=cb;
	      bw+=2+cb->width();
	      if (cb->height()>h) h=cb->height();
		  }
	  i<<=1;
	  }
  if (numbuttons==0) 
	  {
	  i4_error("INTERNAL: i4_message_box: No buttons requested");
	  return 0;
	  }

  //i4_button_class *y=i4_simple_create_button(yes, sd, 
  //                                          new i4_user_message_event_class(0), style);
  //i4_button_class *n=i4_simple_create_button(no, sd,
  //                                          new i4_user_message_event_class(1), style);
  i4_text_window_class *t=new i4_text_window_class(message, style, 0, 300);

  w=t->width()+2;
  h+=t->height()+4;
  
  if (w<bw)
    w=bw;

  sd->resize((w16)w,(w16)h);

  i4_parent_window_class *p; 
  p=style->create_modal_window(root->width()/2-w/2, root->height()/2-h/2, (w16)w,(w16)h, title,false);


  i=0;
  sw32 x=(w/2)-(bw/2)-2,y=t->height()+2;
  while (i<MSG_LASTBUTTON+1)
	  {
	  if (buttons[i])
		  {
		  sd->add_child((short)x,(short)y,buttons[i]);
		  x+=buttons[i]->width()+2;
		  }
	  i++;
	  }

  //sd->add_child(p->width()/2-bw/2, 0, y);
  //sd->add_child(p->width()/2-bw/2+y->width(), 0, n);
  sd->add_child(p->width()/2-t->width()/2, 1, t);

  p->add_child(0,0,sd);
  if (flags&MSG_NOTMODAL)
	  {
	  return 0;
	  }
  else
	  {
	  i4_current_app->refresh();
	  int result=0;
	  while (result==0)
		  {
		  i4_current_app->get_input();//force message to be modal
		  //let's try wheter this helps. (at least it's modal to current context)
		  i4_current_app->refresh();
		  result= g1_message_box::get_result();
		  }
	  return result;

	  }
  
}


// gui/tab_bar.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/tab_bar.h"
#include "memory/array.h"
#include "window/style.h"

struct tab
{
  i4_menu_item_class *tab_top;
  i4_window_class *tab_body;
  int desired_width;    
};

enum { LEFT_SPACING=5,
       RIGHT_SPACING=5,
       TAB_TOP_MARGIN=6,
       TAB_BOTTOM_MARGIN=4,
       TAB_LEFT_MARGIN=3,
       TAB_RIGHT_MARGIN=3 };
       
       

class i4_tab_bar_data
{
public:
  i4_array<tab> tabs;
  int current_tab;
  int top_height;
  i4_graphical_style_class *style;

  i4_tab_bar_data()
    : tabs(0,8)
  {
    top_height=0;
  }

};


i4_tab_bar::i4_tab_bar(int width, int height, i4_graphical_style_class *style)
  : i4_menu_class(i4_F)
{
  data=new i4_tab_bar_data();
  data->style=style;
  private_resize(width, height);
    
}

i4_tab_bar::~i4_tab_bar()
{
  delete data;  
 
}


void i4_tab_bar::private_resize(w16 new_width, w16 new_height)
{
  i4_parent_window_class::private_resize(new_width, new_height);
  if (!data->tabs.size())
    return ;
 
 
  int i, t_tabs=data->tabs.size();
  remove_child(data->tabs[data->current_tab].tab_body);

  for (i=0; i<t_tabs; i++)
    remove_child(data->tabs[i].tab_top);




  int available_size=width()-LEFT_SPACING-RIGHT_SPACING-t_tabs*(TAB_LEFT_MARGIN+TAB_RIGHT_MARGIN);

  int suggested_size=available_size / t_tabs;
  int leftover_size=0;
  
  for (i=0; i<t_tabs; i++)
    if (data->tabs[i].desired_width<suggested_size)
      leftover_size+=suggested_size-data->tabs[i].desired_width;

  leftover_size+=available_size-suggested_size*t_tabs;


  int client_w=width()-4;
  int client_h=height()-data->top_height-TAB_TOP_MARGIN-TAB_BOTTOM_MARGIN;
  
  int xpos=LEFT_SPACING + TAB_LEFT_MARGIN;
  int ypos=TAB_TOP_MARGIN-2;
  for (i=0; i<t_tabs; i++)
  {
    int new_w=suggested_size;

    int additional_size_needed=data->tabs[i].desired_width-suggested_size;
    if (additional_size_needed>0)
    {      
      if (additional_size_needed>leftover_size)
      {
        new_w=suggested_size+leftover_size;        
        leftover_size=0;
      }
      else
      {
        leftover_size-=additional_size_needed;
        new_w=data->tabs[i].desired_width;
      }
    }
    else new_w=data->tabs[i].desired_width;
      

    data->tabs[i].tab_top->resize(new_w, data->top_height);
    i4_parent_window_class::add_child(xpos, ypos, data->tabs[i].tab_top);

    data->tabs[i].tab_body->resize(client_w, client_h);
    if (i==data->current_tab)
      i4_parent_window_class::add_child(2, data->top_height+TAB_TOP_MARGIN, data->tabs[i].tab_body);

    xpos+=new_w+TAB_LEFT_MARGIN+TAB_RIGHT_MARGIN;
  }
}



void i4_tab_bar::add_tab(i4_menu_item_class *tab_top, i4_window_class *tab_body)
{
  i4_parent_window_class::add_child(0,0, tab_top);
  if (!data->tabs.size())
  {
    data->current_tab=0;   
    i4_parent_window_class::add_child(0,0, tab_body);
  }

  if (tab_top->height()>data->top_height)
    data->top_height=tab_top->height();


  tab t;
  t.tab_top=tab_top;
  t.tab_body=tab_body;
  t.desired_width=tab_top->width();  
  data->tabs.add(t);

  tab_top->set_menu_parent(this);

  resize(width(), height());
}



void i4_tab_bar::set_current_tab(int tab_number)
{
  if (!data->tabs.size() || tab_number==data->current_tab)
    return ;

  i4_window_class *client=data->tabs[data->current_tab].tab_body;
  int cx=client->x()-x(), cy=client->y()-y();


  remove_child(client);
  if (tab_number<0) tab_number=0;
  if (tab_number>=data->tabs.size())
    tab_number=data->tabs.size()-1;

  data->current_tab=tab_number;
  i4_parent_window_class::add_child(cx, cy, data->tabs[tab_number].tab_body);
}


void i4_tab_bar::note_reaction_sent(i4_menu_item_class *who,
                                    i4_event_reaction_class *ev,
                                    i4_menu_item_class::reaction_type type)
{
  if (type==i4_menu_item_class::PRESSED)
  {
    for (int i=0; i<data->tabs.size(); i++)
      if (who==data->tabs[i].tab_top)
        set_current_tab(i);
  }
}




void i4_tab_bar::parent_draw(i4_draw_context_class &context)
{ 
  data->style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);


  w32 black=0, 
    bright=data->style->color_hint->button.passive.bright,
    med=data->style->color_hint->button.passive.medium,
    dark=data->style->color_hint->button.passive.dark;
    
  i4_image_class *im=local_image;

  int dx=LEFT_SPACING, dy=0;
  int cur=data->current_tab;


  int client_y=TAB_TOP_MARGIN+data->top_height;
  int client_h=data->tabs[0].tab_body->height();

  im->bar(0, client_y-2, width()-1, client_y-2, bright, context);
  im->bar(1, client_y-1, width()-2, client_y-1, med, context);

  im->bar(0, client_y-2, 0, client_y+client_h+2, bright, context);
  im->bar(1, client_y-1, 1, client_y+client_h+1, med, context);

  im->bar(width()-2, client_y, width()-2, client_y+client_h+1, dark, context);
  im->bar(width()-1, client_y-1, width()-1, client_y+client_h+2, black, context);

  im->bar(2, client_y+client_h+1, width()-2, client_y+client_h+1, dark, context);
  im->bar(1, client_y+client_h+2, width()-1, client_y+client_h+2, black, context);

  

  for (int i=0; i<data->tabs.size(); i++)
  {
    int x1=dx, y1=dy+2, x2=dx+data->tabs[i].tab_top->width()+TAB_LEFT_MARGIN+TAB_RIGHT_MARGIN-1, 
      y2=dy+TAB_TOP_MARGIN+data->top_height-3;

    if (i==cur)
    {
      y1-=2;
      y2+=2;
      x1+=1;
      x2+=1;
    }

    if (i-1!=cur)
      im->bar(x1, y1+2, x1, y2, bright, context);    // left edge
    im->bar(x1+1, y1+1, x1+1, y1+1, bright, context);    // round off left edge to top


    im->bar(x1+2, y1, x2-(i+1==cur ? 1 : 2), y1, bright, context);  // top edge
     

    if (i+1!=cur)
    {
      im->bar(x2-1, y1, x2-1, y2, dark, context);
      im->bar(x2, y1+1, x2, y2, black, context);
    }

    if (i==cur)
      data->style->deco_neutral_fill(im, x1+1, y2-2, x2-2, y2, context);
          
    dx+=data->tabs[i].tab_top->width()+TAB_LEFT_MARGIN+TAB_RIGHT_MARGIN;
  }
}

// gui/text_input.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "gui/text_input.h"

#include "time/timedev.h"
#include "device/event.h"
#include "window/win_evt.h"
#include "device/keys.h"
#include "device/key_man.h"

class cursor_blink_class : public i4_object_message_event_class
{
  public :
  cursor_blink_class(void *object) : i4_object_message_event_class(object) {}
  virtual i4_event  *copy() { return new cursor_blink_class(object); }  
} ;

sw32 i4_text_input_class::get_number()
{
  i4_str::iterator is=get_edit_string()->begin();
  return is.read_number();
}

i4_text_input_class::i4_text_input_class(i4_graphical_style_class *style,
                                         const i4_const_str &default_string,
                                         w32 width,       // width in pixels of window
                                         w32 _max_width,
                                         i4_event_handler_class *change_notify,
                                         i4_font_class *_font)
  : style(style),
    i4_window_class(0,0),
    change_notify(change_notify)

{
  if (!_font)
    font=style->font_hint->normal_font;
  else
    font=_font;

  max_width=_max_width;
  sent_change=i4_F;
  changed=i4_F;
  selected=i4_F;
  cursor_on=i4_F;
  selecting=i4_F;
  need_cancel_blink=i4_F;
    
  w32 l,t,r,b;
  style->get_in_deco_size(l,t,r,b);
  private_resize((w16)(width+l+r), (w16)(font->height(default_string)+1+t+b));

  if (max_width<(int)default_string.length())         // need a string at least as long as default
    max_width=default_string.length();

  st=new i4_str(default_string,max_width+1);

  cursor_x=default_string.length();

  hi_x1=0;   // nothing hilighted by default
  hi_x2=0;
  left_x=0;
  lastcontext=-1;
}


void i4_text_input_class::draw_deco(i4_draw_context_class &context)
{
  style->draw_in_deco(local_image, 0,0, width()-1, height()-1, selected, context);
}

void i4_text_input_class::draw(i4_draw_context_class &context)
{        
  sw32 cx1,cy1,cy2;
  w32 char_on=0;
  w32 x;
  i4_color fg;
  i4_const_str::iterator c=st->begin();
  w32 l,t,r,b;

  local_image->add_dirty(0,0, width()-1, height()-1, context);

  style->get_in_deco_size(l,t,r,b);
  cx1=l;
  cy1=t;
  cy2=height()-1-b;


  for (x=0; x<(w32)left_x; x++)
  {
    ++c;
    char_on++;
  }

  style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
  //  local_image->clear(style->color_hint->text_background,context);
  w32 proper_hi_x1=hi_x1>hi_x2 ? hi_x2 : hi_x1;
  w32 proper_hi_x2=hi_x1>hi_x2 ? hi_x1 : hi_x2;
      

  for (x=0; x<width() && c!=st->end(); )
  {

    if (char_on>=proper_hi_x1 && char_on<proper_hi_x2)
    {
      fg=style->color_hint->selected_text_foreground;
      local_image->bar((short)(cx1+x),                      (short)(cy1+1),
                       (short)(cx1+x+font->width(c.get())), (short)(cy2),
                       style->color_hint->selected_text_background,
                       context);
    } else
      fg=style->color_hint->text_foreground;

    font->set_color(fg);
    if (c!=st->end())
    {
      font->put_character(local_image,(short)(cx1+x),(short)(cy1+1),c.get(),context);
      if (cursor_on && cursor_x==(sw32)char_on)
        local_image->bar((short)(cx1+x),(short)(cy1+1),
                         (short)(cx1+x),(short)(cy2-2),style->color_hint->white,context);

      x+=font->width(c.get());
      ++c;
    }
    else if (cursor_on && cursor_x==(sw32)char_on)
        local_image->bar((short)(cx1+x),(short)(cy1+1), (short)(cx1+x),(short)(cy2-2),style->color_hint->white,context);



    char_on++;
  }

  if (cursor_on && cursor_x==(sw32)char_on)
    local_image->bar((short)(cx1+x),(short)(cy1+1),
                     (short)(cx1+x),(short)(cy2-2),style->color_hint->white,context);



  draw_deco(context);

}


// this will find the character the mouse is on, this should work for non-constantly spaced
// character strings as well
w32 i4_text_input_class::mouse_to_cursor()
{
  i4_const_str::iterator c=st->begin();

  w32 char_on=0;
  w32 x,fw;
  if (last_x<0)
    last_x=0;

  for (x=0; x<(w32)left_x && c!=st->end(); x++)
  {
    ++c;
    char_on++;
  }
    
  for (x=0; x<width() && c!=st->end(); )
  {
    fw=font->width(c.get());
    if (last_x<=(sw16)(x+fw/2+1))
      return char_on;

    x+=fw;
    char_on++;
    ++c;
  }
  return char_on;
}

void i4_text_input_class::request_blink()
{
  if (!need_cancel_blink)
  {
	cursor_blink_class cbs(this);//the event will be ->copy()ed before inserting in the event list.
    blink_timer=i4_time_dev.request_event(this,&cbs,
                                          //new cursor_blink_class(this),
                                          500);
    need_cancel_blink=i4_T;
  }
}

void i4_text_input_class::stop_blink()
{
  if (need_cancel_blink)
  {
    i4_time_dev.cancel_event(blink_timer);
    need_cancel_blink=i4_F;
  }
}

void i4_text_input_class::sustain_cursor()
{
  cursor_on=i4_T;
  request_redraw();
}

void i4_text_input_class::del_selected()
{
        
  if (hi_x1!=hi_x2)
  {
    i4_coord swap;
    if (hi_x1>hi_x2)
    {
      swap=(short)hi_x1;
      hi_x1=hi_x2;
      hi_x2=swap;
    }

    cursor_x=hi_x1;

    i4_str::iterator start=st->begin(),end=st->begin();
    while (hi_x1)
    {
      ++start;
      hi_x2--;
      hi_x1--;
    }
    end=start;
    while (hi_x2)
    {
      ++end;
      hi_x2--;
    }
    st->remove(start,end);
  }
  request_redraw();
}

void i4_text_input_class::move_cursor_right()
//{{{
{
  if (cursor_x<(sw32)st->length())
  {
    cursor_x++;

    i4_const_str::iterator c=st->begin();
    w32 cur_left,x;

    for (x=0; x<(w32)left_x && c!=st->end(); x++)
      ++c;
 

    for (cur_left=cursor_x-left_x; cur_left && c!=st->end(); cur_left--)
    {
      x+=font->width(c.get());
      ++c;      
    }
    if (x>=width())
    {
      left_x+=4;
    }
    sustain_cursor();
  }
}
//}}}


void i4_text_input_class::move_cursor_left()
{
  if (cursor_x)
  {
    cursor_x--;
    if (cursor_x<left_x)
    {
      if (left_x>4)
        left_x-=4;
      else left_x=0;
    }
    sustain_cursor();
  }
}


void i4_text_input_class::move_cursor_end()
//{{{
{
  w32 left_pos, pos;
  i4_const_str::iterator c;

  cursor_x = st->length();
  left_x = 0;
  pos = 0;
  for (c=st->begin(); c!=st->end(); ++c)
    pos += font->width(c.get());
  left_pos = pos - width();

  pos = 0;
  for (c=st->begin(); c!=st->end() && pos<left_pos; ++c)
  {
    pos += font->width(c.get());
    left_x++;
  }

  sustain_cursor();
}
//}}}


void i4_text_input_class::become_active()
{
  i4_window_request_key_grab_class kgrab(this);
  send_to_parent(&kgrab);
  int cn=i4_key_man.current_context();
  if (cn!=i4_key_man.get_context_id("menu"))
	  lastcontext=cn;
  else 
	  {
	  if (lastcontext==-1) 
		  lastcontext=i4_key_man.get_context_id("action");
	  }
  i4_key_man.set_context("menu");//we don't want any key to have some side-effect during textual input
  
  //be shure we are only requesting the events once.
  i4_kernel.unrequest_events(this,i4_device_class::FLAG_CHAR_SEND);
  i4_kernel.request_events(this,i4_device_class::FLAG_CHAR_SEND);
  selected=i4_T;
  request_redraw();
  if (!cursor_on)      // make sure cursor is not draw when we are not in focus
    cursor_on=i4_T;
  request_blink();
}

void i4_text_input_class::become_unactive()
{
  selected=i4_F;
  request_redraw();
  if (lastcontext!=-1) i4_key_man.set_context(lastcontext);
  //lastcontext=-1;
  i4_kernel.unrequest_events(this,i4_device_class::FLAG_CHAR_SEND);
  if (cursor_on)      // make sure cursor is not draw when we are not in focus
    cursor_on=i4_F;

  if (hi_x1!=hi_x2)
  {
    hi_x1=hi_x2;
    request_redraw();
  }

  stop_blink();

  if (selecting)
  {
    i4_window_request_mouse_ungrab_class ungrab(this);
    send_to_parent(&ungrab);
    selecting=i4_F;
  }

}
  
void i4_text_input_class::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::WINDOW_MESSAGE :        
    {
      CAST_PTR(wev,i4_window_message_class,ev);
      switch (wev->sub_type)
      {
        case i4_window_message_class::GOT_DROP :
        {
          CAST_PTR(dev, i4_window_got_drop_class, ev);

          if (dev->drag_info.drag_object_type==i4_drag_info_struct::FILENAMES)
          {
            if (dev->drag_info.t_filenames==1)
            {
              change_text(*dev->drag_info.filenames[0]);
              if (change_notify)
              {
                i4_text_change_notify_event o(this, st);
                i4_kernel.send_event(change_notify, &o);                
                sent_change=i4_T;
              }
            }
          }

        } break;

        case i4_window_message_class::GOT_MOUSE_FOCUS :
        {//set_cursor will make a copy itself, so don't do a ->copy() here
          set_cursor(style->cursor_hint->text_cursor());
        } break;

        case i4_window_message_class::LOST_MOUSE_FOCUS :
        {
          set_cursor(0);
        } break;


        case i4_window_message_class::LOST_KEYBOARD_FOCUS :
        {
          become_unactive(); 

          if (change_notify && changed && !sent_change)
          {
            i4_text_change_notify_event o(this, st);
            i4_kernel.send_event(change_notify, &o);
            sent_change=i4_T;
          }
        } break;
          
        default :          
          i4_window_class::receive_event(ev);
      }
    } break;

    case i4_event::OBJECT_MESSAGE :
    {
      CAST_PTR(oev,i4_object_message_event_class,ev);
      if (oev->object==this)                              // was this an event we created?
      {
        need_cancel_blink=i4_F;
        cursor_on=(i4_bool)(!cursor_on);
        request_redraw();
        request_blink();
      }
    } break;

    case i4_event::MOUSE_BUTTON_DOWN :
    {
      CAST_PTR(bev,i4_mouse_button_down_event_class,ev);
      if (bev->but==i4_mouse_button_down_event_class::LEFT)
      {
        become_active();

        selecting=i4_T;
          
        hi_x1=mouse_to_cursor();
        hi_x2=hi_x1;
        cursor_x=hi_x1;

        i4_window_request_mouse_grab_class grab(this);
        send_to_parent(&grab);


      }

    } break;

    case i4_event::MOUSE_BUTTON_UP :
    {       
      CAST_PTR(bev,i4_mouse_button_up_event_class,ev);
      if (bev->but==i4_mouse_button_up_event_class::LEFT && selecting)
      {
        selecting=i4_F;
        i4_window_request_mouse_ungrab_class ungrab(this);
        send_to_parent(&ungrab);

      }
    } break;


    case i4_event::MOUSE_MOVE : 
    {
      CAST_PTR(mev,i4_mouse_move_event_class,ev);
      last_x=mouse_x;
      last_y=mouse_y;
      mouse_x=mev->x;
      mouse_y=mev->y;
      if (selecting)
      {
        w32 old_hi_x2=hi_x2;
        hi_x2=mouse_to_cursor();
        if (old_hi_x2!=(w32)hi_x2)
        {
          request_redraw();
          cursor_x=hi_x2;
        }
      }
    } break;

    case i4_event::KEY_PRESS :
    {
      CAST_PTR(kev,i4_key_press_event_class,ev);

      switch (kev->key)
      {
        case I4_BACKSPACE :
        {
          if (hi_x1!=hi_x2)
            del_selected();
          else if (cursor_x)
          {
            cursor_x--;
            w32 cur=cursor_x;
            i4_str::iterator cursor1=st->begin(),cursor2=st->begin();
            while (cur)
            {
              cur--;
              ++cursor1;
            }
            cursor2=cursor1;
            ++cursor2;
            st->remove(cursor1,cursor2);
			if (cursor_x<left_x)
				left_x=cursor_x;
            sustain_cursor();
          }
          note_change();
        

        } break;
		//case I4_ESC:
		//	do_main_menu();
		//  break;
        
        case I4_LEFT :
        {
          move_cursor_left();
        } break;

        case I4_RIGHT :
        {
          move_cursor_right();
        } break;

        case I4_HOME :
        {
          cursor_x = 0;
          left_x = 0;
          sustain_cursor();
        } break;

        case I4_END :
        {
          //cursor_x = 0;
          //left_x = 0;
          //sustain_cursor();
		  move_cursor_end();
        } break;

        case I4_ENTER :
        {
          if (change_notify /*&& changed && !sent_change*/)
          {//if user want's input more than once, why not let him do?
            i4_text_change_notify_event o(this, st);
            i4_kernel.send_event(change_notify, &o);
            sent_change=i4_T; 
          }
        } break;

        default :
        {//For Keys, we wait for CHAR_SEND events
          //if (kev->key>=' ' && kev->key<='~')
		/*
		  if (kev->key>=' ' && kev->key<=255) //Why are letters like הצü not allowed?
          {
            note_change();
        
            if (hi_x1!=hi_x2)
              del_selected();
            w32 cur=cursor_x;
            i4_str::iterator cursor=st->begin();
            while (cur)
            {
              cur--;
              ++cursor;
            }

            st->insert(cursor,(w8)kev->key);
            move_cursor_right();
          }*/

        } break;
      }
    } break;

	case i4_event::CHAR_SEND:
		{//For Textual input, this is more accurate than KEY_PRESS
		//(Considers Keyboard mappings)
		CAST_PTR(cev,i4_char_send_event_class,ev);
		if (cev->character>=' ' && cev->character<=255) 
          {
            note_change();
        
            if (hi_x1!=hi_x2)
              del_selected();
            w32 cur=cursor_x;
            i4_str::iterator cursor=st->begin();
            while (cur)
            {
              cur--;
              ++cursor;
            }

            st->insert(cursor,(w8)cev->character);
            move_cursor_right();
          }
		}break;
    case i4_event::QUERY_MESSAGE :
    {
      CAST_PTR(qev,i4_query_text_input_class,ev);
      qev->copy_of_data=new i4_str(*st,(w16)st->length()+1);
    } break;

	case i4_event::USER_MESSAGE://something similar to query:
		//when receiving an user_message, we will sent the text
		{//to the target regardless of what's happened (perhaps we 
		//just don't know?
		if (change_notify /*&& changed && !sent_change*/)
          {//if user want's input more than once, why not let him do?
            i4_text_change_notify_event o(this, st);
            i4_kernel.send_event(change_notify, &o);
            sent_change=i4_T; 
          }
		}
		break;

    default :
      i4_window_class::receive_event(ev);
  }
}

i4_text_input_class::~i4_text_input_class()
{
  if (lastcontext!=-1) i4_key_man.set_context(lastcontext);
  if (change_notify && changed && !sent_change)
  {
    i4_text_change_notify_event o(this, st);
    i4_kernel.send_event(change_notify, &o);
    sent_change=i4_T; 
  }
  i4_kernel.unrequest_events(this,i4_device_class::FLAG_CHAR_SEND);
  stop_blink();
  delete st;
  st=0;
}


i4_window_class *i4_create_text_input(i4_graphical_style_class *style,
                                      const i4_const_str &default_string,
                                      w32 width,        // width in pixels of window
                                      w32 max_width)
{
  i4_text_input_class *ti=new i4_text_input_class(style,
                                                  default_string,
                                                  width,
                                                  max_width);



  return ti;
}

void i4_text_input_class::change_text(const i4_const_str &new_st, i4_bool just_post)
{
  delete st;
  st=new i4_str(new_st, max_width);
  request_redraw();
  left_x=0;
  cursor_x=0;
  sent_change=i4_F;
  if (just_post)
	  {
	  if (change_notify /*&& changed && !sent_change*/)
          {//if user want's input more than once, why not let him do?
            i4_text_change_notify_event o(this, st);
            i4_kernel.send_event(change_notify, &o);
            sent_change=i4_T; 
          }
	  }
}

// gui/text_scroll.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _MANGLE_INC
#include "gui/TEXT_~ZG.HH"
#else
#include "gui/text_scroll.h"
#endif
#include "window/style.h"
#include "area/rectlist.h"


static inline int fmt_char(char c)
{
  if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
    return 1;
  return 0;
}

void i4_text_scroll_window_class::printf(char *fmt, ...)
{
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
          output_string(str);
        } break;          

        case 'd' :
        case 'i' :
        case 'x' :
        case 'X' :
        case 'c' :
        case 'o' :
        {
          ::sprintf(out,f,va_arg(ap,int));
          output_string(out);
        } break;

        case 'f' :
        case 'g' :
          ::sprintf(out,f,va_arg(ap,double));
          output_string(out);
          break;

        default :
          ::sprintf(out,f,va_arg(ap,void *));
          output_string(out);
          break;
      }
      fmt=fmt_end;
      if (*fmt)
        fmt++;
    }
    else
    {
      output_char(*fmt);
      fmt++;
    }
  }
  va_end(ap);
}



i4_text_scroll_window_class::i4_text_scroll_window_class(i4_graphical_style_class *style,
                                                         i4_color text_foreground,
                                                         i4_color text_background,
                                                         w16 width, w16 height,    // in pixels
                                                         w32 scrollback)
  : i4_parent_window_class(width, height),
    style(style),
    fore(text_foreground),
    back(text_background),
    max_scrollback(scrollback),
    scroll_pos(-1) //scroll once when the first line is drawn. 
{
  

  i4_font_class *fnt=style->font_hint->normal_font;

  term_height=height/fnt->largest_height();
  if (max_scrollback<term_height)
      max_scrollback=term_height;
  curr_scrollback=1;
  scrollbar=new i4_scroll_bar(i4_T,
      height, //drawing height of scrollbar
      1,1, //visible/total number of lines (total is all - visible)
      0, //message id
      this,  //event handler
      style);
  add_child(width-scrollbar->width(),0,scrollbar);
  width=width-scrollbar->width();
  //term_width will be >=1
  term_width=(width/fnt->largest_width() + 1)*2;

  //if (!term_size)
  //  term_size=512;
  term_size=term_width*max_scrollback;

  term_out=(i4_char *)I4_MALLOC(term_size, "terminal chars");
  term_end=term_out+term_size;

  //start at the last line in the window.
  draw_start=term_out+term_size-(term_width*term_height);
  line_height=fnt->largest_height()+1;
  memset(term_out,0,term_size);
  dx=tdx=tdy=0;
  dy=max_scrollback-1;
  need_clear=i4_T;
  used_size = 0;
  used_lines= 0;
}

i4_text_scroll_window_class::~i4_text_scroll_window_class()
{
	i4_free(term_out);
	term_out=0;
	term_end=0;
	draw_start=0;
	used_size=0;
	used_lines=0;
}

void i4_text_scroll_window_class::skip_first_line()
{
/*
  i4_font_class *fnt=style->font_hint->normal_font;
  w32 x=0, wd=width(), count=0;
  i4_char *s=term_out;
    
  while (s->value() && x<wd)
  {
    if (s->value()=='\n')
      x=wd;
    else
      x+=fnt->width(*s);
    count++;
    s++;
  }
  memmove(term_out, s, used_size-count+1);
  used_size-=count;
  */
}

void i4_text_scroll_window_class::output_string(char *string)
{
  while (*string)
  {
    output_char( (i4_char)(*string));
    string++;
  }
}

void i4_text_scroll_window_class::scroll_text_up()
    {
    memmove(term_out,term_out+term_width,term_size-term_width);
    memset(term_end-term_width,0,term_width);
    if (used_lines<max_scrollback)
        {
        used_lines++;
        if (used_lines>term_height)
            {
            curr_scrollback++;
            }
        }
    scrollbar->set_new_total(curr_scrollback);
    //set to the end.
    //scrollbar->set_bar_pos(curr_scrollback);
    if (scroll_pos>=scrollbar->get_pos()-1)
        {
        scrollbar->set_pos(curr_scrollback);
        scroll_pos++;
        }
    dy=max_scrollback-1;
    dx=0;
    tdx=0;
    need_clear=i4_T;
    request_redraw(i4_T);
    }

void i4_text_scroll_window_class::receive_event(i4_event *ev)
    {
    if (ev->type()==i4_event::USER_MESSAGE)
        {
        CAST_PTR(uev, i4_user_message_event_class, ev);
      
        if (uev->sub_type==0)  // SCROLLBAR message
            {      
            CAST_PTR(sbm, i4_scroll_message, ev);
            scroll_pos=sbm->amount;
            need_clear=i4_T;
            request_redraw(i4_F);
            return;
            }
        }
    i4_parent_window_class::receive_event(ev);
    }

void i4_text_scroll_window_class::output_char(const i4_char &ch)
{
  //we need a font for this to work
  if (!style->font_hint || !style->font_hint->normal_font) return;
  i4_font_class *fnt=style->font_hint->normal_font;

  /*
  if (used_size+2>term_size)
  {
    term_size+=100;
    i4_char *old_spot=term_out;
    term_out=(i4_char *)i4_realloc(term_out, term_size, "terminal chars");
    dx=dy=0;
    draw_start=term_out;        
    need_clear=i4_T;    
  }
  */

  if (!redraw_flag)
    request_redraw();

  sw32 tempdx=dx+1;
  sw32 temptdx=tdx+fnt->width(ch);
  if (tempdx>=term_width ||temptdx>=width()-scrollbar->width()|| ch=='\n')
      {
      scroll_text_up();
      }
  if (ch!='\n')
      {
      term_out[dy*term_width+dx]=ch;
      dx++;
      tdx+=fnt->width(ch);
      }
  /*
  if (tdx>=width() || ch=='\n')
  {
    tdx=0;
    tdy+=line_height;
    if (tdy+line_height>height())
    {
      skip_first_line();
      tdy-=line_height;
      dx=dy=0;
      draw_start=term_out;        
      need_clear=i4_T;
    }
  }

  term_out[used_size++]=ch;  
  term_out[used_size]=0;
  */
}

void i4_text_scroll_window_class::clear()
{
  draw_start=term_out+term_size-(term_width*term_height);
  memset(term_out,0,term_size);
  scrollbar->set_pos(1);
  scrollbar->set_new_total(1);
  
  curr_scrollback=1;
  dx=dy=tdx=tdy=0;
  need_clear=i4_T;
  used_size = 0;
  used_lines=0;
  request_redraw(i4_F);
  request_redraw(i4_T);//redraw the scrollbar
}

void i4_text_scroll_window_class::resize(w16 new_width, w16 new_height)
{
  i4_font_class *fnt=style->font_hint->normal_font;
  term_height=new_height/fnt->largest_height();
  term_width=new_width/fnt->largest_width();
  scrollbar->resize(scrollbar->width(),new_height);
  scrollbar->set_pos(1);
  scrollbar->set_new_total(max_scrollback-term_height+1);
  int new_size=term_width+max_scrollback;
  if (new_size>(int)term_size)
  {
      term_size=new_size;

      term_out=(i4_char *)I4_REALLOC(term_out, term_size, "terminal chars");
      term_end=term_out+term_size;
  }

  dx=dy=tdx=tdy=0;
  used_size = 0;

  i4_parent_window_class::resize(new_width, new_height);
}


void i4_text_scroll_window_class::parent_draw(i4_draw_context_class &context)
{
  i4_font_class *fnt=style->font_hint->normal_font;

  if (!undrawn_area.empty())
      {
      need_clear=i4_T;
      }
  if (need_clear)
  {
    if (back==style->color_hint->neutral())
      style->deco_neutral_fill(local_image, 0,0, width()-1-scrollbar->width(), height()-1, context);
    else
      local_image->bar(0,0,width()-1-scrollbar->width(),height()-1,back,context);

    need_clear=i4_F;
  }
  fnt->set_color(fore);

  sw32 pos=scroll_pos-1;
  sw32 first_scroll_line=max_scrollback-used_lines+pos;
  //pos*=term_width;
  pos=term_width*first_scroll_line;
  i4_char *cpos=term_out+pos;
  i4_char *linestart=cpos;
  sw32 xpos=0,ypos=0;
  while (cpos<term_end)
      {
      i4_char c=*cpos;
      if (c==0 || c=='\n')
          {
          xpos=0;
          ypos+=fnt->largest_height();
          linestart+=term_width;
          cpos=linestart;
          }
      else
          {
          fnt->put_character(local_image, (short)xpos,(short)ypos, c, context);
          xpos+=fnt->width(c);
          cpos++;
          }
      }
  /*
  if (!undrawn_area.empty())
  {
    draw_start=term_out;
    dx=dy=0;
    need_clear=i4_T;
  }
    
  if (need_clear)
  {
    if (back==style->color_hint->neutral())
      style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
    else
      local_image->clear(back, context);

    need_clear=i4_F;
  }
  fnt->set_color(fore);

  while (draw_start->value())
  {
    if (draw_start->value()=='\n')
      dx=width()+1;
    else
    {
      fnt->put_character(local_image, (short)dx,(short)dy, *draw_start, context);
      dx+=fnt->width(*draw_start);
    }

    if (dx>=width())
    {
      dx=0;
      dy+=fnt->largest_height();
    } 

    draw_start++;  
  } 
  tdx=dx;
  tdy=dy;
  */
  i4_parent_window_class::parent_draw(context);
}

i4_bool i4_text_scroll_window_class::need_redraw() 
{ 
  if (draw_start->value()) 
    return i4_T;
  else 
    return i4_parent_window_class::need_redraw();
}      







