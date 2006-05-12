// WINDOW.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "area/rectlist.h"
#include "device/kernel.h"
#include "error/error.h"
#include "image/image.h"
#include "time/profile.h"
#include "string/string.h"
#include "memory/malloc.h"
#include "palette/pal.h"
#include "arch.h"
#include "loaders/load.h"
#include "image/image.h"
#include "image/context.h"
#include "app/app.h"
#include "menu/boxmenu.h"

#include "font/plain.h"
#include "font/anti_prop.h"

#include "gui/text.h"
#include "gui/deco_win.h"
#include "gui/gradiant.h"

#include "window/window.h"
#include "window/win_evt.h"
#include "window/cursor.h"
#include "window/wmanager.h"
#include "window/style.h"
#include "window/dragwin.h"
#include "window/colorwin.h"

//extern HWND      i4_win32_window_handle; 

i4_profile_class pf_child_draw_prepare("window::child_prepare"),
  pf_parent_draw_prepare("window::parent_prepare");

#include <stdlib.h>

i4_parent_window_class *i4_window_class::root_window()
{
  if (parent)
    return parent->root_window();
  else return 0;
}

i4_parent_window_class *i4_parent_window_class::root_window()
{
  if (parent)
    return parent->root_window();
  else return this;
}

i4_bool i4_window_class::isa_parent(i4_window_class *who)
{
  if (who==0)
    return i4_F;
  else if (who==parent)
    return i4_T;
  else if (parent)
    return parent->isa_parent(who);
  else return i4_F;
}

void i4_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_MOVE)
  {
    CAST_PTR(move, i4_mouse_move_event_class, ev);
    last_x=mouse_x;
    last_y=mouse_y;
    mouse_x=move->x;
    mouse_y=move->y;
  }

  if (cursor && parent)  // if we have a cursor we need to load when we get a mouse focus
  {
    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(wev,i4_window_message_class,ev);
      if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      {
        CAST_PTR(mev, i4_window_got_mouse_focus_class, ev);

        last_x=mouse_x=mev->x;
        last_y=mouse_y=mev->y;

        i4_window_change_cursor_class c(this,cursor);
        i4_kernel.send_event(parent,&c); 
      } else if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      {
        i4_window_change_cursor_class c(this,0);   // unload the cursor
        i4_kernel.send_event(parent,&c); 
      }
    }     
  }
}

void i4_window_class::set_cursor(i4_cursor_class *Cursor)
{ 
  if (cursor)
    delete cursor;
  
  if (Cursor)
    cursor=Cursor->copy();
  else
    cursor=0;

  if (parent)
  {
    i4_window_change_cursor_class c(this,cursor,i4_F);
    i4_kernel.send_event(parent,&c);
  }
}

i4_window_class::~i4_window_class()
{  
  if (cursor)
    delete cursor;
  
  if (parent)
    parent->remove_child(this);
}

void i4_window_class::redepth(w16 new_bitdepth)
	{
	if (parent)
		{
		i4_window_notify_redepth_class rd(this, new_bitdepth);
		i4_kernel.send_event(parent, &rd);
		}
	}

void i4_window_class::resize(w16 new_width, w16 new_height)
{
  //static i4_array<void *> sent_to(0,16);//hack to make shure the parent gets 
  //notifyed only once.
  if (parent)
  {
    i4_window_notify_resize_class resize(this,new_width,new_height);
    i4_kernel.send_event(parent,&resize);
  }
  private_resize(new_width,new_height);
}

void i4_parent_window_class::resize(w16 new_width, w16 new_height)
{
  i4_window_class::resize(new_width, new_height);

  i4_window_notify_resize_class resize(this,new_width,new_height);

  for (win_iter c=children.begin(); c!=children.end(); ++c)   
    send_event_to_child(&*c,&resize);
}


void i4_window_class::private_resize(w16 new_width, w16 new_height)
{
  i4_rect_list_class parent_dirty;
  if (width() && height())
    parent_dirty.add_area(0,0,width()-1, height()-1);

  w=new_width;
  h=new_height;
  parent_dirty.remove_area(0,0,width()-1,height()-1);

  if (width() && height())
    note_undrawn(0,0,width()-1,height()-1);

    
  if (parent)
  {
    for (i4_rect_list_class::area_iter a=parent_dirty.list.begin();
         a!=parent_dirty.list.end(); ++a)
    {
      parent->note_undrawn(a->x1 + x() - parent->x(),
                           a->y1 + y() - parent->y(),
                           a->x2 + x() - parent->x(),
                           a->y2 + y() - parent->y());
                           
    }
  }

  request_redraw();
}


void i4_window_class::reparent(i4_image_class *draw_area, i4_parent_window_class *Parent)
{
  parent=Parent;
  if (parent)
  {
    last_x=mouse_x=parent->last_mouse_x()+parent->x()-x();
    last_y=mouse_y=parent->last_mouse_y()+parent->y()-y();
  }
  else
  {
    last_x=mouse_x=-10000;
    last_y=mouse_y=-10000;
  }

  //if (local_image) //do not delete
    local_image=0;

  global_image=draw_area;
  if (global_image)
  {
    local_image=global_image;
    
    if (width() & height())
      note_undrawn(0,0,width()-1,height()-1);

    request_redraw();
  }
}

void i4_window_class::private_move(i4_coord x_offset, i4_coord y_offset)
{
  global_x+=x_offset;
  global_y+=y_offset; 

  if (width() && height())
    note_undrawn(0,0,width()-1,height()-1);


  request_redraw();
}

void i4_window_class::move(i4_coord x_offset, i4_coord y_offset,  i4_bool draw_under)
{
  private_move(x_offset,y_offset);
  if (parent)
  {
    i4_window_notify_move_class move(this,x_offset,y_offset,draw_under);
    i4_kernel.send_event(parent,&move);
  }
}

void i4_window_class::draw(i4_draw_context_class &context)
{
  redraw_flag=i4_F;
}

void i4_window_class::request_redraw(i4_bool for_a_child)
{
  redraw_flag=i4_T; 

    
  if (parent) 
  {
    if (transparent() && width() && height())
    {
      int x1=x()-parent->x(), y1=y()-parent->y();
      int x2=x1+width()-1, y2=y1+height()-1;

      parent->note_undrawn(x1,y1,x2,y2, i4_F);
    }

    parent->request_redraw(i4_T);
  }
}

void i4_window_class::note_undrawn(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                                   i4_bool propogate_to_children)
{
  request_redraw(i4_F);
}



void i4_parent_window_class::parent_draw(i4_draw_context_class &context)
{
  redraw_flag=i4_F;
}

// draw should redraw supplied area of self
void i4_parent_window_class::draw(i4_draw_context_class &context)

{  
  if (!undrawn_area.empty() || redraw_flag)
  {   
    pf_parent_draw_prepare.start();

    redraw_flag=i4_F;
    // copy the clip list, which should already be in our coordinates
    i4_rect_list_class child_clip(&context.clip,0,0);  
    
    // remove the area of other children above us    
    win_iter d=children.begin();
    for (;d!=children.end();++d)
    {
      if (!d->transparent())
        child_clip.remove_area(d->x()-x(),
                               d->y()-y(),
                               d->x()+d->width()-1-x(),
                               d->y()+d->height()-1-y());
    }

    child_clip.swap(&context.clip);

    pf_parent_draw_prepare.stop();

    parent_draw(context);

    pf_parent_draw_prepare.start();
    undrawn_area.delete_list();

    child_clip.swap(&context.clip);
    pf_parent_draw_prepare.stop();

  }

  if (child_need_redraw)
  {
    child_need_redraw=i4_F;
    win_iter c=children.begin();
    for (;c!=children.end();++c)   
    {
      if (c->need_redraw())
      {
        pf_child_draw_prepare.start();

		i4_coord x1=c->x()-x(),
          y1=c->y()-y(),
          x2=c->x()+c->width()-1-x(),
          y2=c->y()+c->height()-1-y();

        // copy the clip list, and move it to local coordinates
        i4_rect_list_class child_clip(&context.clip,
                                      -(c->x()-x()),
                                      -(c->y()-y()));

        // intersect the clip with what our area is supposed to be
        child_clip.intersect_area(0,0,
                                  c->width()-1,
                                  c->height()-1);   

        // save the old context xoff and yoff
        sw16 old_xoff=context.xoff,old_yoff=context.yoff;   
          
        // move the context offset to the child's x,y
        context.xoff+=(c->x()-x());                         
        context.yoff+=(c->y()-y());

	  // remove the area of other children above us
        i4_window_class *d=c->next;
        for (;d&&(!child_clip.empty());d=d->next)
          child_clip.remove_area(d->x()-c->x(), 
                                 d->y()-c->y(),
                                 d->x()+d->width()-1-c->x(),
                                 d->y()+d->height()-1-c->y()); 

        c->call_stack_counter++;  // make sure window doesn't get deleted during it's draw

        // the child is covered completely by other windows
        // tell it this in case it depends on draw()
        if (child_clip.empty())
          c->forget_redraw();   
        else
        {
          child_clip.swap(&context.clip);
          
          child_rerequested_redraw = i4_F;
         
          pf_child_draw_prepare.stop();

          c->draw(context);         // the child is ready to draw

          pf_child_draw_prepare.start();
          if (!child_rerequested_redraw)          
            c->i4_window_class::forget_redraw();

          child_clip.swap(&context.clip);   // restore the old clip list
        }
        
        context.xoff=old_xoff;    // restore the context's x & y offsets
        context.yoff=old_yoff;

        c->call_stack_counter--;
        pf_child_draw_prepare.stop();

      }      
    }
  } 

}
    
//#if ( __linux || __sgi)
//extern void db_show();
//#else
//void db_show() { ; }
//#endif

void i4_window_class::show_context(i4_draw_context_class &context)
{
  i4_draw_context_class nc(0,0,local_image->width(),local_image->height());
  local_image->clear(0xffff,nc);
  i4_rect_list_class::area_iter c=context.clip.list.begin();
  for (; c!=context.clip.list.end(); ++c)
    local_image->rectangle(c->x1,c->y1,c->x2,c->y2,0,nc);

  //  db_show();
}


i4_window_class::i4_window_class(w16 w, w16 h) : w(w),h(h)
{
  last_x=mouse_x=-10000;
  last_y=mouse_y=-10000;

  global_x=0; 
  global_y=0;
  local_image=0;
  global_image=0;
  parent=0;
  cursor=0;
  next=0;
}

void i4_window_class::bring_to_front()
    {
    if (parent)
        parent->bring_to_front(this);
    }


i4_parent_window_class::i4_parent_window_class(w16 w, w16 h) : i4_window_class(w,h)
{
  have_mouse_focus=i4_F;
  mouse_focus_grabbed=i4_F;
  key_focus=children.end();
  mouse_focus=children.end();
  drag_drop_focus=children.end();
}

void i4_parent_window_class::bring_to_front(i4_window_class *wnd)
    {
    win_iter c=children.begin();
    for (;c!=children.end();++c)
        {
        if (c.get()==wnd)
            {
            children.swap(c,children.end());
            }
        }
    }

i4_parent_window_class::win_iter i4_parent_window_class::find_window(i4_coord global_mouse_x, 
                                                                     i4_coord global_mouse_y)
{
  /*RECT r;
  GetClientRect(i4_win32_window_handle,&r);
  MapWindowPoints(i4_win32_window_handle,NULL,(LPPOINT)&r,2);
  if (global_mouse_x<r.left) global_mouse_x=(sw16)r.left;
  if (global_mouse_x>r.right) global_mouse_x=(sw16)r.right;
  if (global_mouse_y<r.top)global_mouse_x=(sw16)r.top;
  if (global_mouse_y>r.bottom) global_mouse_y=(sw16)r.bottom;*/
  win_iter c=children.begin(),
    find=children.end();
  
  for (;c!=children.end();++c)
  {
    if (global_mouse_x>=c->x() &&  
        global_mouse_y>=c->y() && 
        global_mouse_x<c->x()+c->width() && 
        global_mouse_y<c->y()+c->height())
		{
		find=c;
		}
  }
  return find;
}

void i4_parent_window_class::drag_drop_move(i4_event *ev)
{
  CAST_PTR(move, i4_window_drag_drop_move_class, ev);
  i4_coord real_mx=move->x+x(),
    real_my=move->y+y();

  win_iter find=find_window(real_mx, real_my);
  if (find != drag_drop_focus)
  {
    if (drag_drop_focus != children.end())
    {
      i4_window_lost_drag_drop_focus_class lost(this);
      send_event_to_child(&*drag_drop_focus, &lost);
    }

    drag_drop_focus = find;
    if (find != children.end())
    {
      i4_window_class *prev_from=move->from_window;
      move->from_window=this;

      send_event_to_child(&*find, move);

      move->from_window=prev_from;
    }
  }

  if (drag_drop_focus != children.end())
    send_event_to_child(&*drag_drop_focus,ev);
}


i4_bool i4_parent_window_class::find_new_mouse_focus()
{
  if (!mouse_focus_grabbed)  // see if we need to change the mouse focus
  {
    win_iter new_mouse_focus=find_window(mouse_x + x(), mouse_y + y());
	i4_window_class *nmf,*modwin;
	nmf=&*new_mouse_focus;
	//cmf=&*mouse_focus;
	modwin=i4_kernel.modal();
	if (modwin)//avoid setting the focus to something else if a modal window
		{//is open
		
		if (!nmf||((nmf!=modwin)&&(nmf!=this)&&(!nmf->isa_parent(modwin))))
			{
			/*if (cmf != modwin&&modwin!=this)//how do we do this?
				{
				change_mouse_focus(modwin);
				return i4_T;
				}*/
			return i4_F;
			}
		}
    if (new_mouse_focus!=mouse_focus)
    {
      //change_mouse_focus(&*new_mouse_focus);
	  change_mouse_focus(nmf);
      return i4_T;
    }

    else return i4_F;
  }
  else return i4_F;
}

void i4_parent_window_class::mouse_move(i4_event *ev)
{
  CAST_PTR(move, i4_mouse_move_event_class, ev);
  last_x=mouse_x;
  last_y=mouse_y;
  mouse_x=move->x;
  mouse_y=move->y;

  find_new_mouse_focus();

  if (mouse_focus!=children.end())
    send_event_to_child(&*mouse_focus,ev);
}

void i4_parent_window_class::send_event_to_child(i4_window_class *w, i4_event *ev)
{
  sw32 xo=(sw32)x()-(sw32)w->x(), yo=(sw32)y()-(sw32)w->y();

  ev->move(xo, yo);
  i4_kernel.send_event(w, ev);
  ev->move(-xo, -yo);
}

// passes events to mouse_focus or key_focus depending on the event type
void i4_parent_window_class::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::IDLE_MESSAGE :     // pass idle message to current mouse focus
    {
      if (mouse_focus!=children.end())
        send_event_to_child(&*mouse_focus,ev);
    } break;

    case i4_event::MOUSE_MOVE :
    {
      mouse_move(ev);
    } break;

    case i4_event::MOUSE_BUTTON_DOWN :
    {
      if (mouse_focus!=children.end())
        send_event_to_child(&*mouse_focus,ev);
    } break;

    case i4_event::MOUSE_BUTTON_UP :
    {
      if (mouse_focus!=children.end())
        send_event_to_child(&*mouse_focus,ev);
    } break;

    case i4_event::KEY_PRESS :
    case i4_event::KEY_RELEASE :
    {
      if (key_focus!=children.end())
        send_event_to_child(&*key_focus,ev);
      else if (mouse_focus!=children.end())
        send_event_to_child(&*mouse_focus,ev);
    } break;


    case i4_event::WINDOW_MESSAGE :
    {
      CAST_PTR(mess,i4_window_message_class,ev);
      switch (mess->sub_type)
      {	
        case i4_window_message_class::GOT_DROP :
        {
          if (drag_drop_focus != children.end())
          {
            send_event_to_child(&*drag_drop_focus, ev);
            drag_drop_focus=children.end();
          }
          else
          {
            CAST_PTR(dev, i4_window_got_drop_class, ev);

            i4_parent_window_class::win_iter w=find_window(dev->drag_info.x+x(),
                                                           dev->drag_info.y+y());
            if (w!=children.end())
              send_event_to_child(&*w, ev);
          }
        } break;

        case i4_window_message_class::REQUEST_DRAG_DROP_START :
        case i4_window_message_class::REQUEST_DRAG_DROP_END :
        {
          if (parent)
            parent->receive_event(ev);
        } break;
        case i4_window_message_class::DRAG_DROP_MOVE :
        {
          drag_drop_move(ev);
        } break;

        case i4_window_message_class::CHANGE_CURSOR :
        {
          CAST_PTR(cc,i4_window_change_cursor_class,ev);
          if (parent && (!cc->only_if_active || cc->from()==&*mouse_focus))
          {
            i4_window_change_cursor_class scope(this,cc->cursor,cc->only_if_active);
            parent->receive_event(&scope);
          }

        } break;
	case i4_window_message_class::REQUEST_KEY_GRAB :
	{
          CAST_PTR(grab,i4_window_request_key_grab_class,ev);
          grab->return_result=i4_T;

          if (parent)
          {
            i4_window_request_key_grab_class ask_parent(this);
            i4_kernel.send_event(parent,&ask_parent);
            if (ask_parent.return_result==i4_F)
              grab->return_result=i4_F;
          }

          if (grab->return_result)
          {
            win_iter c=children.begin();
            for (;c!=children.end() && c!=mess->from();++c);
            if (c!=children.end())
            {
              if (c != key_focus)
                change_key_focus(&*c);
            }
            else
              i4_warning("got key grab from unknown child");
          }
	} break;
	case i4_window_message_class::REQUEST_DELETE :
	{
	  remove_child(mess->from());
	  delete mess->from();
	} break;
	
	case i4_window_message_class::REQUEST_NEXT_KEY_FOCUS : next_key_focus(); break;
	case i4_window_message_class::REQUEST_LEFT_KEY_FOCUS : left_key_focus(); break;
	case i4_window_message_class::REQUEST_RIGHT_KEY_FOCUS : right_key_focus(); break;
	case i4_window_message_class::REQUEST_UP_KEY_FOCUS : up_key_focus(); break;
	case i4_window_message_class::REQUEST_DOWN_KEY_FOCUS : down_key_focus(); break;
	case i4_window_message_class::REQUEST_MOUSE_GRAB :
	{
	  CAST_PTR(grab,i4_window_request_mouse_grab_class,ev);

          grab->return_result=i4_T;    // defulat return is false

	  if (!mouse_focus_grabbed)  // see if we need to change the mouse focus	  
	  {
	    if (parent)
	    {
	      i4_window_request_mouse_grab_class ask_parent(this);
	      i4_kernel.send_event(parent,&ask_parent);
	      if (ask_parent.return_result==i4_F)
	        grab->return_result=i4_F;
              else grab->return_result=i4_T;
	    }

	    if (grab->return_result)
            {
              mouse_focus_grabbed=i4_T;
  
              if (&(*mouse_focus) != grab->from())
                change_mouse_focus(grab->from());
            }

	  } else 
	    grab->return_result=i4_F;
	} break;

	case i4_window_message_class::REQUEST_MOUSE_UNGRAB :
	{
	  //if (mouse_focus_grabbed)
          {
            mouse_focus_grabbed=i4_F;
            if (parent)
              i4_kernel.send_event(parent,ev);	  

            find_new_mouse_focus();
          }
        //  else i4_warning("mouse not grabbed");

	} break;


	case i4_window_message_class::NOTIFY_RESIZE :
	{
	  CAST_PTR(resize,i4_window_notify_resize_class,ev);
          i4_window_class *cf=resize->from();
		  
          if (cf!=parent)    // if this is from our parent, ignore it
          {
            i4_rect_list_class dirty;
            if (cf->width() && cf->height())
              dirty.add_area   (cf->x(),cf->y(),cf->x()+cf->width()-1,cf->y()+cf->height()-1);

            if (resize->new_width && resize->new_height)
              dirty.remove_area(cf->x(),cf->y(),
                                cf->x()+resize->new_width-1,
                                cf->y()+resize->new_height-1);

            if (width() && height())
              dirty.intersect_area(x(),y(),x()+width()-1,y()+height()-1);

            if (resize->draw_covered)
            {
              // check to see if any child were under this window 
              // will need to be redraw because of this
              win_iter c=children.begin();
              for (;c!=children.end() && c!=cf;++c)
              {                 
                if (c->width() && c->height() &&
                    !dirty.clipped_away(c->x(),c->y(),c->x()+c->width()-1,c->y()+height()-1))
                {
                  i4_rect_list_class::area_iter a=dirty.list.begin();
                  for (;a!=dirty.list.end();++a)
                  {
                    if (c->width() && c->height())
                    {
                      i4_coord x1=a->x1-c->x(),y1=a->y1-c->y(),x2=a->x2-c->x(),y2=a->y2-c->y();

                      if (x1<0) x1=0;
                      if (y1<0) y1=0;
                      if (x2>=c->width()) x2=c->width()-1;
                      if (y2>=c->height()) y2=c->height()-1;
                      if (x1<=x2 && y1<=y2)
                        c->note_undrawn(x1,y1,x2,y2);
                    }
                  }
                }
              }

              // add this dirty area to the parent's undrawn_area list, 
              // so the parent knows what part of itself it needs to redraw
              i4_rect_list_class::area_iter a=dirty.list.begin();
              for (;a!=dirty.list.end();++a)
                undrawn_area.add_area(a->x1-x(),    // make sure we add in parent-local coordinates
                                      a->y1-y(),
                                      a->x2-x(),
                                      a->y2-y());
            }
          }
          else 
            request_redraw(i4_F);
	} break;

	case i4_window_message_class::NOTIFY_MOVE :
	{
	  CAST_PTR(move_event,i4_window_notify_move_class,ev);
	  i4_window_class *child=move_event->from();
	  i4_coord old_x=child->x()-move_event->x_offset,old_y=child->y()-move_event->y_offset;

	  i4_coord new_x=child->x(),new_y=child->y();

	  if (move_event->draw_covered)
	  {
	    i4_rect_list_class dirty;

            if (child->width() && child->height())
              dirty.add_area(old_x,old_y,old_x+child->width()-1,old_y+child->height()-1);

            if (width() && height())
              dirty.intersect_area(x(),y(),x()+width()-1,y()+height()-1);

            if (child->width() && child->height())
              dirty.remove_area(new_x,new_y,new_x+child->width()-1,new_y+child->height()-1);
	    

      // check to see if any child were under this window will need to be redraw because of this
	    win_iter c=children.begin();
	    for (;c!=children.end() && c!=child;++c)
            {
              if (c->width() && c->height() &&
                  !dirty.clipped_away(c->x(),c->y(),c->x()+c->width()-1,c->y()+height()-1))
              {
                i4_rect_list_class::area_iter a=dirty.list.begin();
                for (;a!=dirty.list.end();++a)
                {
                  if (c->width() && c->height())
                  {
                    i4_coord x1=a->x1-c->x(),y1=a->y1-c->y(),x2=a->x2-c->x(),y2=a->y2-c->y();

                    if (x1<0) x1=0;
                    if (y1<0) y1=0;
                    if (x2>=c->width()) x2=c->width()-1;
                    if (y2>=c->height()) y2=c->height()-1;
                    if (x1<=x2 && y1<=y2)
                      c->note_undrawn(x1,y1,x2,y2);
                  }
                }
              }
            }

	    // add this dirty area to the parent's dirty_area list, 
            // so the parent knows what part of itself it needs to redraw
	    i4_rect_list_class::area_iter a=dirty.list.begin();
	    for (;a!=dirty.list.end();++a)
	      undrawn_area.add_area(a->x1-x(),   // add in local coordinates
                                    a->y1-y(),
                                    a->x2-x(),
                                    a->y2-y());
	  }

	} break;

        case i4_window_message_class::GOT_MOUSE_FOCUS :
        {
          have_mouse_focus=i4_T;

          CAST_PTR(mev, i4_window_got_mouse_focus_class, ev);

          last_x=mouse_x=mev->x;
          last_y=mouse_y=mev->y;

          i4_window_change_cursor_class c(this,cursor);
          i4_kernel.send_event(parent,&c); 

          find_new_mouse_focus();
        } break;


        case i4_window_message_class::LOST_DROP_FOCUS :
        {
          if (drag_drop_focus!=children.end())
          {
            send_event_to_child(&*drag_drop_focus,ev);
            drag_drop_focus=children.end();
          }
          i4_window_class::receive_event(ev);
        } break;

        case i4_window_message_class::LOST_MOUSE_FOCUS :
        {
          if (mouse_focus!=children.end())
          {
            send_event_to_child(&*mouse_focus,ev);
            mouse_focus=children.end();
          }
          i4_window_class::receive_event(ev);
          have_mouse_focus=i4_F;

        } break;
        case i4_window_message_class::LOST_KEYBOARD_FOCUS :
        {
          change_key_focus(0);
        } break;
        default :
          i4_window_class::receive_event(ev);
          
      }
    } break;
    default :
      i4_window_class::receive_event(ev);
          
  }
}

void i4_parent_window_class::change_key_focus(i4_window_class *new_focus)
{
  if (key_focus!=children.end())
  {
    i4_window_message_class lost(i4_window_message_class::LOST_KEYBOARD_FOCUS,this);
    send_event_to_child(&*key_focus,&lost);    
  }

  key_focus=new_focus;

  if (new_focus)
  {
    i4_window_message_class got(i4_window_message_class::GOT_KEYBOARD_FOCUS,this);
    send_event_to_child(&*key_focus,&got);  
  }
}


void i4_parent_window_class::change_mouse_focus(i4_window_class *new_focus)
{
  if (mouse_focus!=children.end())
  {
    i4_window_lost_mouse_focus_class lost(this, new_focus);
    send_event_to_child(&*mouse_focus,&lost);    
  }
//#ifdef _DEBUG
//  if (new_focus && new_focus->next==(void*)0xdddddddd)
//	  {
//	  i4_warning("Alert: Tried to set the focus to a tooltip-control");
//	  return;
//	  }
//#endif
  mouse_focus=new_focus;

  if (mouse_focus!=children.end())
  {
    i4_window_got_mouse_focus_class got(this, mouse_x, mouse_y);
    send_event_to_child(&*mouse_focus,&got);  
  }
}


void i4_parent_window_class::next_key_focus()
{
  if (key_focus!=children.end())
    change_key_focus(key_focus->next);
}


void i4_parent_window_class::left_key_focus()
{
  if (key_focus!=children.end())
  {
    win_iter c=children.begin(),closest=children.end();
    w32 closest_distance=0xffffffff;
    i4_coord cx1=key_focus->x()+key_focus->width()/2,
      cy1=key_focus->y()+key_focus->height()/2;


    for (;c!=children.end();++c)
    {
      if (c->x()<key_focus->x())
      {
	i4_coord cx2=c->x()+c->width()/2,cy2=c->y()+c->height()/2;

	w32 dist=(cx2-cx1)*(cx2-cx1)+(cy2-cy1)*(cy2-cy1);
	if (dist<closest_distance)
	{
	  closest_distance=dist;
	  closest=c;
	}
      }
    }

    if (closest!=children.end())
      change_key_focus(&*closest);
  }
}


void i4_parent_window_class::right_key_focus()
{
  if (key_focus!=children.end())
  {
    win_iter c=children.begin(),closest=children.end();
    w32 closest_distance=0xffffffff;
    i4_coord cx1=key_focus->x()+key_focus->width()/2,
      cy1=key_focus->y()+key_focus->height()/2;


    for (;c!=children.end();++c)
    {
      if (c->x()>cx1)
      {
	i4_coord cx2=c->x()+c->width()/2,
          cy2=c->y()+c->height()/2;       
	w32 dist=(cx2-cx1)*(cx2-cx1)+(cy2-cy1)*(cy2-cy1);
	if (dist<closest_distance)
	{
	  closest_distance=dist;
	  closest=c;
	}
      }
    }

    if (closest!=children.end())
      change_key_focus(&*closest);
  }
}


void i4_parent_window_class::up_key_focus()
{
  if (key_focus!=children.end())
  {
    win_iter c=children.begin(),closest=children.end();
    w32 closest_distance=0xffffffff;
    i4_coord cx1=key_focus->x()+key_focus->width()/2,
      cy1=key_focus->y()+key_focus->height()/2;


    for (;c!=children.end();++c)
    {
      if (c->y()<key_focus->y())
      {
	i4_coord cx2=c->x()+c->width()/2,
          cy2=c->y()+c->height()/2;       
	w32 dist=(cx2-cx1)*(cx2-cx1)+(cy2-cy1)*(cy2-cy1);
	if (dist<closest_distance)
	{
	  closest_distance=dist;
	  closest=c;
	}
      }
    }

    if (closest!=children.end())
      change_key_focus(&*closest);
  }
}


void i4_parent_window_class::down_key_focus()
{
  if (key_focus!=children.end())
  {
    win_iter c=children.begin(),closest=children.end();
    w32 closest_distance=0xffffffff;
    i4_coord cx1=key_focus->x()+key_focus->width()/2,
      cy1=key_focus->y()+key_focus->height()/2;

    for (;c!=children.end();++c)
    {
      if (c->y()>key_focus->y())
      {
	i4_coord cx2=c->x()+c->width()/2,
          cy2=c->y()+c->height()/2;       
	w32 dist=(cx2-cx1)*(cx2-cx1)+(cy2-cy1)*(cy2-cy1);
	if (dist<closest_distance)
	{
	  closest_distance=dist;
	  closest=c;
	}
      }
    }

    if (closest!=children.end())
      change_key_focus(&*closest);
  }
}

void i4_parent_window_class::reparent(i4_image_class *draw_area, i4_parent_window_class *_parent)
{
  // reparent children first, in case the have a mouse grab and need to tell our
  // original parent to ungrab
  for (win_iter c=children.begin();c!=children.end();++c)
    c->reparent(draw_area,this);

  i4_window_class::reparent(draw_area,_parent);

  if (have_mouse_focus)
    find_new_mouse_focus();
}

void i4_parent_window_class::add_child(i4_coord x_, i4_coord y_, i4_window_class *child)
{
  children.insert_end(*child);
  child->reparent(global_image,this);
  child->private_move(x()+x_-child->x(),y()+y_-child->y());

  if (have_mouse_focus)
    find_new_mouse_focus();
}

void i4_parent_window_class::add_child_front(i4_coord x_, i4_coord y_, i4_window_class *child)
{
  children.insert(*child);
  child->reparent(global_image,this);
  child->private_move(x()+x_-child->x(),y()+y_-child->y());

  if (have_mouse_focus)
    find_new_mouse_focus();
}


void i4_parent_window_class::forget_redraw()
{
  undrawn_area.delete_list();  
  for (win_iter c=children.begin();c!=children.end();++c)
    c->forget_redraw();
}

void i4_parent_window_class::transfer_children(i4_parent_window_class *other_parent, 
                                               i4_coord x_offset, i4_coord y_offset)
{ 
  while (children.begin() != children.end())
  {
    i4_window_class *c=&*children.begin();
    i4_coord xn=c->x()-x();
    i4_coord yn=c->y()-y();

    remove_child(c);
    other_parent->add_child(xn+x_offset,
                            yn+y_offset,c);
  }

  if (have_mouse_focus)
    find_new_mouse_focus();

  if (other_parent->has_mouse_focus())
    other_parent->find_new_mouse_focus();
}

void i4_parent_window_class::remove_child(i4_window_class *child)
{
  if (&*key_focus==child)
    change_key_focus(0); 

  if (&*mouse_focus==child)
  {
    if (mouse_focus_grabbed)
    {
      if (parent)
      {
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);
      }

      mouse_focus_grabbed=i4_F;
    }
  }

  if (child==&*children.begin())
    children.erase();
  else
  {
    win_iter f=children.begin(),last=children.end();
    for (;f!=children.end() && f!=&*child;)
    {
      last=f;
      ++f;
    }
    if (f!=children.end())
      children.erase_after(last);
    else
		{
        i4_error("INTERNAL: i4_window_class::remove_child(): Child not found");
		return;
		}
  }

  if (child->width() && child->height())
  {
    note_undrawn(child->x()-x(),
                 child->y()-y(),
                 child->x()+child->width()-1-x(),
                 child->y()+child->height()-1-y());
  }

  if (&*mouse_focus==child)
  {
    i4_window_lost_mouse_focus_class lost(this, 0);
    send_event_to_child(child, &lost);    
    mouse_focus=children.end();
  }


  child->reparent(0,0);  // tell the child it doesn't have a parent or a draw_area anymore

  if (have_mouse_focus)
    find_new_mouse_focus();

}

void i4_parent_window_class::request_redraw(i4_bool for_a_child)
{
  if (for_a_child)
  {
    child_need_redraw=i4_T;
    child_rerequested_redraw=i4_T;
  }
  else
    redraw_flag=i4_T;
  if (parent)
    parent->request_redraw(i4_T);
}

void i4_parent_window_class::note_undrawn(i4_coord x1, i4_coord y1, 
                                          i4_coord x2, i4_coord y2,
                                          i4_bool propogate_to_children)
{

  undrawn_area.add_area(x1,y1,x2,y2);
  request_redraw();
   
  win_iter c=children.begin();


  x1+=x();  y1+=y();     // transform to global coordinates
  x2+=x();  y2+=y();
  
  if (propogate_to_children)
  {
    for (;c!=children.end();++c)
    {
      if (c->width() && c->height())
      {
        if (!(c->x()>x2 || 
              c->y()>y2 || 
              c->x()+c->width()-1  < x1 || 
              c->y()+c->height()-1 < y1))
        {      
          i4_coord ax1=x1-c->x(),
            ay1=y1-c->y(),
            ax2=x2-c->x(),
            ay2=y2-c->y();
          if (ax1<0) ax1=0;
          if (ay1<0) ay1=0;
          if (ax2>=c->width()) ax2=c->width()-1;
          if (ay2>=c->height()) ay2=c->height()-1;
          if (ax1<=ax2 && ay1<=ay2)
            c->note_undrawn(ax1,ay1,ax2,ay2);
        }
      }
    }
  }
}



void i4_parent_window_class::private_move(i4_coord x_offset, i4_coord y_offset)
{
  i4_window_class::private_move(x_offset,y_offset);
  win_iter c=children.begin();
  for (;c!=children.end();++c)
  {
    c->private_move(x_offset,y_offset);  
  }
  // use private move so child doesn't tell us it moved (we know!)
}


void i4_parent_window_class::redraw_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2)
{
  if (!(x2<0 || y2<0 || x1>=width() || y1>=height()))
    undrawn_area.add_area(x1,y1,x2,y2);
}

// arranges child windows from left to right then down
void i4_parent_window_class::arrange_right_down()  
{
  i4_coord x1=x(),y1=y(),mh=0;
  win_iter c=children.begin();
  for (;c!=children.end();++c)
  {

    if (c->width()+x1>x()+width())
    {
      y1+=mh;
      x1=x();
      mh=0;
    }

    c->move(x1-c->x(),y1-c->y());

    if (c->height()>mh)
      mh=c->height();

    x1+=c->width();


  }
}

void i4_parent_window_class::resize_to_fit_children()
{
  i4_coord x2=0,y2=0;
  win_iter c=children.begin();
  for (;c!=children.end();++c)
  {
    if (c->x()-x()+c->width()>x2)    
      x2=c->x()-x()+c->width();
    if (c->y()-y()+c->height()>y2)
      y2=c->y()-y()+c->height();
  }

  resize(x2,y2);
}

void i4_parent_window_class::arrange_down_right()                                 // arranges child windows from top to bottom then right
{
  i4_coord x1=x(),y1=y(),mw=0;
  win_iter c=children.begin();
  for (;c!=children.end();++c)
  {
    if (c->height()+y1>=height())
    {
      x1+=mw;
      y1=y();
      mw=0;
    }

    c->move(x1-c->x(),y1-c->y());
    y1+=c->height();

    if (c->width()>mw)
      mw=c->width();


  }
}


i4_parent_window_class::~i4_parent_window_class()
{
  while (children.begin() != children.end())
  {
    win_iter c=children.begin();
    children.erase();
    c->reparent(0,0);
    
    i4_kernel.delete_handler(&*c);
  }  
  
}

i4_window_class *i4_parent_window_class::get_nth_window(w32 win_num)
{
  win_iter i=children.begin();
  while (win_num && i!=children.end())
  {
    ++i;
    win_num--;
  }
  if (i==children.end())
    return 0;
  else return &*i;
}


//#ifndef I4_RETAIL
void i4_window_class::debug_show()
{
	char debug_buf[MAX_NAME_BUFFER_SIZE];
	name(debug_buf);
  i4_warning("- %s",debug_buf);
  //if (parent)
  //  parent->debug_show();  
}
//#endif

void i4_parent_window_class::debug_show()
{
	char debug_buf[MAX_NAME_BUFFER_SIZE];
	name(debug_buf);
	i4_warning("%s has the following children:",debug_buf);
	win_iter c=children.begin();
	w32 nochildren=0;
	for (;c!=children.end();++c)
		{
		c->debug_show();
		nochildren++;
		}
	i4_warning("----end of children for %s (total %i)----",debug_buf,nochildren);
}

i4_bool i4_parent_window_class::isa_child(i4_window_class *w)
{
  win_iter c=children.begin();
  for (;c!=children.end();++c)
    if (&*c==w)
      return i4_T;

  return i4_F;
}
// WMANAGER.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/




i4_color i4_window_manager_class::i4_read_color_from_resource(char *resource)
{
  i4_const_str::iterator i=i4gets(resource).begin();
  i4_color red=i.read_number();
  i4_color grn=i.read_number();
  i4_color blue=i.read_number();


  return i4_pal_man.convert_32_to( (red<<16)|(grn<<8)|(blue),
                                   &local_image->get_pal()->source);

}

void i4_window_manager_class::set_background_color(w32 color) 
{ 
  background_color=color; 
}

void i4_window_manager_class::parent_draw(i4_draw_context_class &context)
{     
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);   // intersect clipped area with area that needs to be drawn

  child_clip.swap(&context.clip);                 // make this the display's new clip list

  local_image->clear(background_color,context);          // clear out this area

  child_clip.swap(&context.clip);                 // restore the display's clip list
}

void i4_window_manager_class::root_draw()  // called by application
{
//   i4_draw_context_class c(0,0,639,439);
//   display->get_screen()->clear(0, c);

  i4_parent_window_class::draw(*display->get_context());

  display->flush();                                   // tell the display to page-flip or copy-dirty rects
}
 
  // a window manager does not actually get associated with anything until prepare_for_mode
i4_window_manager_class::i4_window_manager_class() : i4_parent_window_class(0,0)
{
  drag_drop.active=i4_F;
  display=0;
  devices_present=0;
  style=0;
  default_cursor=0;
  background_color=0;
  no_cursor_installed=i4_T;
  key_modifiers_pressed=0;
}

i4_window_manager_class::~i4_window_manager_class()
{
  cleanup_old_mode();
}

void i4_window_manager_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(cc,i4_window_change_cursor_class,ev);
    if (cc->sub_type==i4_window_message_class::CHANGE_CURSOR)
    {
      if (cc->cursor)
      {
        display->set_mouse_shape(cc->cursor);
        no_cursor_installed=i4_F;
      }
      else if (default_cursor)  // make sure we have default cursor first
        display->set_mouse_shape(default_cursor);
    }
    else if (cc->sub_type==i4_window_message_class::REQUEST_DRAG_DROP_START)
    {
      CAST_PTR(drag, i4_window_request_drag_drop_start_class, ev);
      drag_drop.active=i4_T;
      drag_drop.reference_id=drag->reference_id;
      drag_drop.further_info=drag->further_info;
      drag_drop.originator=drag->from();
      if (drag->drag_cursor)
        display->set_mouse_shape(drag->drag_cursor);
    } 
    else if (cc->sub_type==i4_window_message_class::REQUEST_DRAG_DROP_END)
    {
      i4_error("fixme");
//       drag_drop.active=i4_F;
//       if (default_cursor)
//         display->set_mouse_shape(default_cursor);

//       i4_window_class *prev_from=drag_frop

//       i4_window_got_drop_class drop(drag_drop.originator,
//                                     drag_drop.reference_id,
//                                     drag_drop.further_info);

//       i4_parent_window_class::receive_event(&drop);
    }
  
    else if (cc->sub_type==i4_window_message_class::REQUEST_MOUSE_GRAB)
    {
      CAST_PTR(grab,i4_window_request_mouse_grab_class,ev);
      
      i4_parent_window_class::receive_event(ev);
    }
    else if (cc->sub_type==i4_window_message_class::REQUEST_MOUSE_UNGRAB)
    {
      CAST_PTR(ungrab,i4_window_request_mouse_ungrab_class,ev);
      
      i4_parent_window_class::receive_event(ev);
      ungrab->return_result=i4_T;
    }  
    else i4_parent_window_class::receive_event(ev);

  } else if (ev->type()==i4_event::DISPLAY_CHANGE)
  {

    i4_image_class *im=display->get_screen();
    private_resize(im->width(),im->height());         // in case the resolution changed
    reparent(im,0);                           // grab the new image

  } 
  else
  {
    switch (ev->type())
    {
      case i4_event::KEY_PRESS :
        key_modifiers_pressed=((i4_key_press_event_class *)ev)->modifiers;
        break;

      case i4_event::KEY_RELEASE :
        key_modifiers_pressed=((i4_key_release_event_class *)ev)->modifiers;
        break;

      case i4_event::MOUSE_BUTTON_DOWN :
      case i4_event::MOUSE_BUTTON_UP :
      {
        CAST_PTR(bev, i4_mouse_button_event_class,ev);
        if ((w32)bev->time.milli_diff(bev->last_time)<style->time_hint->double_click)
          bev->double_click=i4_T;

      } break;
    }

    i4_parent_window_class::receive_event(ev);
  }


  if (drag_drop.active && ev->type()==i4_event::MOUSE_MOVE)
  {
    CAST_PTR(mouse_move, i4_mouse_move_event_class, ev);
    i4_window_drag_drop_move_class move(drag_drop.originator,
                                        mouse_move->x,
                                        mouse_move->y,
                                        drag_drop.reference_id,
                                        drag_drop.further_info);
    i4_parent_window_class::receive_event(&move);
                                        
  }
}

void i4_window_manager_class::set_default_cursor(i4_cursor_class *cursor)
{
  if (default_cursor)
    delete default_cursor;

  default_cursor=cursor->copy();

  if (no_cursor_installed)
    display->set_mouse_shape(cursor);
}


i4_graphical_style_class *i4_window_manager_class::get_style()
{
  return style;
}

void i4_window_manager_class::prepare_for_mode(i4_display_class *display, 
                                               i4_display_class::mode *mode)
{
  cleanup_old_mode();

  i4_window_manager_class::display=display;

  const i4_const_str s=i4gets("window_manager",i4_F);
  
  if (!s.null())
  {
    char buf[100];
    i4_const_str::iterator i=s.begin();
    i.read_ascii(buf,100);

    style=i4_style_man.find_style(buf);
  }
  else style=0;

  if (!style)
    style=i4_style_man.first_style();


  if (!style)
     i4_error("FATAL: No window styles installed");




  i4_image_class *im=display->get_screen();
  if (!im)
	  {
	  i4_error("FATAL: No usable display device found");
	  return;
	  }
  resize(im->width(),im->height());
  reparent(im,0);


  style->prepare_for_mode(local_image->get_pal(), mode);

  i4_kernel.request_events(this,
                           i4_device_class::FLAG_MOUSE_BUTTON_UP|
                           i4_device_class::FLAG_MOUSE_BUTTON_DOWN|
                           i4_device_class::FLAG_MOUSE_MOVE|
                           i4_device_class::FLAG_KEY_PRESS|
                           i4_device_class::FLAG_KEY_RELEASE|
                           i4_device_class::FLAG_DISPLAY_CHANGE|
                           i4_device_class::FLAG_DRAG_DROP_EVENTS|
                           i4_device_class::FLAG_IDLE);

  default_cursor=style->cursor_hint->normal_cursor()->copy();
  display->set_mouse_shape(default_cursor);


  i4_const_str::iterator col=i4gets("root_background_color").begin();
  w32 r=col.read_number();
  w32 g=col.read_number();
  w32 b=col.read_number();


  background_color=i4_pal_man.convert_32_to( (r<<16)|(g<<8)|(b),  
                                            &local_image->get_pal()->source);
}

void i4_window_manager_class::cleanup_old_mode()
{
  if (style)  // has a previous mode been used?
  {    
    i4_kernel.unrequest_events(this,
                               i4_device_class::FLAG_MOUSE_BUTTON_UP|
                               i4_device_class::FLAG_MOUSE_BUTTON_DOWN|
                               i4_device_class::FLAG_MOUSE_MOVE|
                               i4_device_class::FLAG_KEY_PRESS|
                               i4_device_class::FLAG_KEY_RELEASE|
                               i4_device_class::FLAG_DISPLAY_CHANGE |
                               i4_device_class::FLAG_DRAG_DROP_EVENTS|
                               i4_device_class::FLAG_IDLE
                               );
    if (default_cursor)
    {
      delete default_cursor;
      default_cursor=0;
      no_cursor_installed=i4_T;      
    }
  }
}


// WIN_EVT
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void i4_drag_info_struct::copy_to(i4_drag_info_struct &drag_struct)
{
  drag_struct.t_filenames=t_filenames;
  if (t_filenames)
  {
    drag_struct.filenames=(i4_str **)I4_MALLOC(sizeof(i4_str *) * t_filenames, "");
    for (int i=0; i<t_filenames; i++)
      drag_struct.filenames[i]=new i4_str(*filenames[i]);
  } else drag_struct.filenames=0;

}

i4_drag_info_struct::~i4_drag_info_struct()
{
  if (t_filenames)
  {
    for (int i=0; i<t_filenames; i++)
      delete filenames[i];
    i4_free(filenames);
  }
}

// STYLE.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


i4_style_manager_class i4_style_man;

i4_icon_hint_class::i4_icon_hint_class()
{
  close_icon=i4_load_image(i4gets("close_icon"));
  up_icon=i4_load_image(i4gets("up_icon"));
  down_icon=i4_load_image(i4gets("down_icon"));
  left_icon=i4_load_image(i4gets("left_icon"));
  right_icon=i4_load_image(i4gets("right_icon"));
  plus_icon=i4_load_image(i4gets("plus_icon"));
  minus_icon=i4_load_image(i4gets("minus_icon"));
  background_bitmap=i4_load_image(i4gets("background_bitmap"));
  ok_icon=i4_load_image(i4gets("ok_icon"));
  cancel_icon=i4_load_image(i4gets("cancel_icon"));
}

i4_time_hint_class::i4_time_hint_class()
{
  i4_const_str::iterator d=i4gets("button_delay").begin();
  i4_const_str::iterator r=i4gets("button_repeat").begin();
  i4_const_str::iterator dc=i4gets("double_click").begin();
  button_delay=d.read_number();
  button_repeat=r.read_number();
  double_click=dc.read_number();
}

void i4_graphical_style_class::cleanup()
{
  if (color_hint)
  { 
    delete color_hint;
    color_hint=0;
  }

  if (font_hint)
  {
    delete font_hint;
    font_hint=0;
  }

  if (cursor_hint)
  {
    delete cursor_hint;
    cursor_hint=0;
  }

  if (icon_hint)
  {
    delete icon_hint;
    icon_hint=0;
  }

  if (time_hint)
  {
    delete time_hint;
    time_hint=0;
  }
}

void i4_graphical_style_class::uninit()
{
  cleanup();
}


void i4_graphical_style_class::init()
{
  i4_style_man.add_style(this);
}

void i4_style_manager_class::add_style(i4_graphical_style_class *which)
{
  which->next=list;
  list=which;
}

i4_graphical_style_class *i4_style_manager_class::find_style(char *name)
{
  i4_graphical_style_class *f=list;
  for (;f;f=f->next)
     if (!strcmp(f->name(),name))
       return f;
  return 0;
}

i4_graphical_style_class *i4_style_manager_class::first_style()
{ return list; }

i4_graphical_style_class *i4_style_manager_class::next_style(i4_graphical_style_class *last)
{  return last->next; }


i4_color i4_read_color_from_resource(char *name)
{
  i4_const_str::iterator i=i4gets(name).begin();

  i4_color red=i.read_number();
  i4_color grn=i.read_number();
  i4_color blue=i.read_number();

  return (red<<16)|(grn<<8)|(blue);
}



i4_color_hint_class::i4_color_hint_class()
{
  window.active.bright=i4_read_color_from_resource("window_active_bright");
  window.active.medium=i4_read_color_from_resource("window_active_medium");
  window.active.dark=i4_read_color_from_resource("window_active_dark");

  window.passive.bright=i4_read_color_from_resource("window_passive_bright");
  window.passive.medium=i4_read_color_from_resource("window_passive_medium");
  window.passive.dark=i4_read_color_from_resource("window_passive_dark");

  button=window;

  text_foreground=i4_read_color_from_resource("text_foreground");
  text_background=i4_read_color_from_resource("text_background");

  selected_text_foreground=i4_read_color_from_resource("selected_text_foreground");
  selected_text_background=i4_read_color_from_resource("selected_text_background");

  black = 0;
  white = 0xffffff;
}

i4_font_hint_class::i4_font_hint_class()
{
  i4_image_class *im=i4_load_image(i4gets("small_font"));
  normal_font=small_font=new i4_anti_proportional_font_class(im, '!');
  delete im;
}


i4_cursor_hint_class::i4_cursor_hint_class()
{
  normal=i4_load_cursor("normal_cursor", &i4_string_man);
  text=i4_load_cursor("text_cursor", &i4_string_man);
}


void i4_graphical_style_class::prepare_for_mode(const i4_pal *pal, 
                                                i4_display_class::mode *mode)
{
  cleanup();

  time_hint=new i4_time_hint_class();
  color_hint=new i4_color_hint_class();
  font_hint=new i4_font_hint_class();
  cursor_hint=new i4_cursor_hint_class();
  icon_hint=new i4_icon_hint_class();

}

i4_font_hint_class::~i4_font_hint_class() 
{  
  delete normal_font;  
  if (small_font && small_font!=normal_font)
    delete small_font;
}

i4_icon_hint_class::~i4_icon_hint_class()
{
  delete close_icon;
  delete up_icon;
  delete down_icon;
  delete left_icon;
  delete right_icon;
  delete plus_icon;
  delete minus_icon;
  delete background_bitmap;
  delete ok_icon;
  delete cancel_icon;
}

// MWSTYLE.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


class i4_mwm_context_help : public i4_window_class
{
  i4_graphical_style_class *style;
  i4_str *text;
public:
  i4_event_handler_reference_class<i4_window_class> shadow;

  void draw(i4_draw_context_class &context)
  {
    i4_font_class *f=style->font_hint->small_font;
    local_image->clear(style->color_hint->text_background, context);
    local_image->rectangle(0,0, width()-1, height()-1, 0, context);
    
    f->set_color(style->color_hint->text_foreground);
    f->put_string(local_image, 2,2, *text, context);
  }

  i4_mwm_context_help(i4_graphical_style_class *style, const i4_const_str text)
    :  style(style),
       i4_window_class(style->font_hint->small_font->width(text)+4,
                       style->font_hint->small_font->height(text)+4),
       text(new i4_str(text))
  {    
  }

  ~i4_mwm_context_help() 
  { 
    delete text; 
    if (shadow.get())
      i4_kernel.delete_handler(shadow.get());
  }

  //if a context help window receives an event, it is most
  //certainly in the way. 
  void receive_event(i4_event *ev)
      {
      call_stack_counter++;
      i4_kernel.delete_handler(this);
      call_stack_counter--;
      }

  void name(char* buffer) { static_name(buffer,"i4_mwm_context_help"); }
};

class i4_mwm_style_class : public i4_graphical_style_class
{
public:
  i4_image_class *close_icon;

  virtual void uninit()
  {
    if (close_icon)
    {
      delete close_icon;
      close_icon=0;
    }
    i4_graphical_style_class::uninit();
  }

  char *name() { return "MWM_STYLE_IMPLEMENTATOR"; }
  virtual i4_parent_window_class *create_mp_window(i4_coord x, i4_coord y, 
                                                   w16 w, w16 h, 
                                                   const i4_const_str &title,
                                                   i4_event_reaction_class *on_delete=0
                                                   );

  virtual i4_parent_window_class *create_modal_window(i4_coord x, i4_coord y, 
                                                   w16 w, w16 h, 
                                                   const i4_const_str &title,
												   i4_bool show_close_button,
                                                   i4_event_reaction_class *on_delete=0
                                                   );

  virtual i4_bool close_mp_window(i4_parent_window_class *created_window)
  {
    if (!created_window) return i4_F;
    i4_parent_window_class *to_del=created_window->get_parent();
    if (!to_del) return i4_F;
    i4_parent_window_class *to_del_parent=to_del->get_parent();
    if (!to_del_parent) return i4_F;
    
    i4_kernel.delete_handler(to_del);

    return i4_T;
  }

  virtual i4_bool available_for_display(i4_display_class *whom) { return i4_T; }

  virtual i4_menu_class *create_menu(i4_bool hide_on_pick)
  { 
    return new i4_box_menu_class(this,hide_on_pick);
  }


  virtual void get_in_deco_size(w32 &left, w32 &top, w32 &right, w32 &bottom)
  {
    left=right=top=bottom=2;
  }

  virtual void get_out_deco_size(w32 &left, w32 &top, w32 &right, w32 &bottom)
  {
    left=right=top=bottom=2;
  }

  virtual void deco_neutral_fill(i4_image_class *screen,
                                 sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                 i4_draw_context_class &context)
  {
    

    i4_rect_list_class sub_clip(&context.clip,0,0);
    sub_clip.intersect_area((short)x1,(short)y1,(short)x2,(short)y2);
    sub_clip.swap(&context.clip);

    i4_image_class *im=icon_hint->background_bitmap;
    if (im)
    {
      int iw=im->width(), ih=im->height();
      int dx1=-context.xoff;
      int dy1=-context.yoff;
      while (dx1+iw<x1) dx1+=iw;
      while (dy1+ih<y1) dy1+=ih;

      int x,y;
      for (y=dy1; y<=y2; y+=ih)
        for (x=dx1; x<=x2; x+=iw)
          im->put_image(screen, x,y, context);        
    }
    else
      screen->clear(color_hint->neutral(), context);


    sub_clip.swap(&context.clip);
  }

  // draw a decoration around an area that looks like it's pressed into the screen
  virtual void draw_in_deco(i4_image_class *screen, 
                            i4_coord x1, i4_coord y1,
                            i4_coord x2, i4_coord y2,
                            i4_bool active,
                            i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    if (active)
      color=&color_hint->window.active;
    else 
      color=&color_hint->window.passive;

    screen->add_dirty(x1,y1,x2,y1+1,context);
    screen->add_dirty(x1,y1+2,x1+1,y2,context);
    screen->add_dirty(x1+2,y2-1,x2,y2,context);
    screen->add_dirty(x1-1,y1+2,x2,y2-2,context);

    screen->bar(x1,y1,x2,y1,color->dark,context);
    screen->bar(x1,y1,x1,y2,color->dark,context);

    screen->bar(x1+1,y1+1,x2-1,y1+1,color_hint->black,context);
    screen->bar(x1+1,y1+1,x1+1,y2-1,color_hint->black,context);

    screen->bar(x1+1,y2,x2,y2,color->bright,context);
    screen->bar(x2,y1+1,x2,y2,color->bright,context);

    screen->bar(x1+2,y2-1,x2-1,y2-1,color->medium,context);
    screen->bar(x2-1,y1+1,x2-1,y2-1,color->medium,context);   
  }

  // draw a decoration around an area that looks like it sticks out the screen
  virtual void draw_out_deco(i4_image_class *screen, 
                             i4_coord x1, i4_coord y1,
                             i4_coord x2, i4_coord y2,
                             i4_bool active,
                             i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    if (active)
      color=&color_hint->window.active;
    else 
      color=&color_hint->window.passive;


    screen->add_dirty(x1,y1,x2,y1+1,context);
    screen->add_dirty(x1,y1+2,x1+1,y2,context);
    screen->add_dirty(x1+2,y2-1,x2,y2,context);
    screen->add_dirty(x1-1,y1+2,x2,y2-2,context);

    screen->bar(x1,y1,x2,y1,color->bright,context);
    screen->bar(x1,y1,x1,y2,color->bright,context);

    screen->bar(x1+1,y1+1,x2-1,y1+1,color->medium,context);
    screen->bar(x1+1,y1+1,x1+1,y2-1,color->medium,context);

    screen->bar(x1+1,y2,x2,y2,color->dark,context);
    screen->bar(x2,y1+1,x2,y2,color->dark,context);

    screen->bar(x1+2,y2-1,x2-1,y2-1,color_hint->black,context);
    screen->bar(x2-1,y1+1,x2-1,y2-1,color_hint->black,context);
  
  }

  // this will create a temporary (quick) context help window at the mouse cursor
  // you are responsible for deleting the window
  virtual i4_window_class *create_quick_context_help(int mouse_x, int mouse_y,
                                                     const i4_const_str &str)
  {
    i4_parent_window_class *parent=i4_current_app->get_root_window();
    i4_mwm_context_help *w=new i4_mwm_context_help(this, str);

    if (w->width()+mouse_x+3 > parent->width())
      mouse_x=parent->width()-w->width()-3;
    if (w->height()+mouse_y+3 > parent->height())
      mouse_y=parent->height()-w->height()-3;

    i4_window_class *cw=new i4_color_window_class(w->width(), w->height(), 0, this);
    w->shadow=cw;

    parent->add_child(mouse_x+3, mouse_y+3, cw);   
    parent->add_child(mouse_x, mouse_y, w);



    return w;
  }

} mwm_style;


class i4_mwm_event_class : public i4_user_message_event_class
{
public:
    enum 
    {
        CLOSE_YOURSELF,
        MAXIMIZE_YOURSELF,
		RESTORE_YOURSELF,
		GRAB_FOCUS
    };
    i4_window_class *from;

    i4_mwm_event_class(w32  sub_type, i4_window_class *from) : i4_user_message_event_class(sub_type),from(from) {}
    

    virtual i4_event  *copy() 
    { 
        return new i4_mwm_event_class(sub_type,from); 
    }

};


static void widget(i4_image_class *im,
                   i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                   i4_color bright, i4_color med, i4_color dark,
                   i4_draw_context_class &context)
{
  // to keep from creating a dirty for each operation below
  im->add_dirty(x1,y1,x2,y2,context);      

  im->bar(x1,y1,x2,y1,bright,context);
  im->bar(x1,y1+1,x1,y2,bright,context);
  im->bar(x2,y1+1,x2,y2,dark,context);
  im->bar(x1+1,y2,x2-1,y2,dark,context);
  im->bar(x1+1,y1+1,x2-1,y2-1,med,context);
}


class i4_mwm_close_button_class : public i4_window_class
{  
protected:
  i4_graphical_style_class *hint;
  i4_bool active;

public:
  virtual void name(char* buffer) { static_name(buffer,"close_button"); }

  i4_mwm_close_button_class(w16 w, w16 h,i4_graphical_style_class *hint) 
    : i4_window_class (w,h),hint(hint)
  { 
    active=i4_F;
  }

  void activate(i4_bool yes)
  {
    active=yes;
    request_redraw();
  }
    

  virtual void receive_event(i4_event *ev)
  {
    i4_mwm_event_class c(i4_mwm_event_class::CLOSE_YOURSELF,this);
    if (ev->type()==i4_event::MOUSE_BUTTON_DOWN && parent)
      i4_kernel.send_event(parent,&c);
  }
 
  virtual void draw(i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    /*    if (active)
      color=&hint->color_hint->window.active;
    else */
    color=&hint->color_hint->window.passive;

    local_image->add_dirty(0,0,width()-1,height()-1,context);



    widget(local_image, 0,0,width()-2,height()-2,color->bright,color->medium,color->dark,context);
    local_image->bar(0,height()-1,width()-1,height()-1,hint->color_hint->black,context);
    local_image->bar(width()-1,0,width()-1,height()-1,hint->color_hint->black,context);

    mwm_style.icon_hint->close_icon->put_image(local_image,2,2,context);    
  }

  virtual void show_self(w32 indent) 
  {    
    char fmt[50];
    sprintf(fmt,"%%%ds mwm_close_button",indent);
    i4_warning(fmt," ");
  }
} ;

// A close button that cannot be pressed
class i4_mwm_passive_close_button_class : public i4_mwm_close_button_class
{  

public:
	virtual void name(char* buffer) { static_name(buffer,"passive_close_button"); }

	i4_mwm_passive_close_button_class(w16 w, w16 h,i4_graphical_style_class *hint) 
		: i4_mwm_close_button_class (w,h,hint)
	{ 
		active=i4_F;
	}


	virtual void receive_event(i4_event *ev)
	{
		//No reaction whatsoever
	}

	virtual void draw(i4_draw_context_class &context)
	{
		i4_color_hint_class::bevel *color;
		/*    if (active)
		color=&hint->color_hint->window.active;
		else */
		color=&hint->color_hint->window.passive;

		local_image->add_dirty(0,0,width()-1,height()-1,context);



		widget(local_image, 0,0,width()-2,height()-2,color->bright,color->medium,color->dark,context);
		local_image->bar(0,height()-1,width()-1,height()-1,hint->color_hint->black,context);
		local_image->bar(width()-1,0,width()-1,height()-1,hint->color_hint->black,context);

		//mwm_style.icon_hint->close_icon->put_image(local_image,2,2,context);    
	}

	virtual void show_self(w32 indent) 
	{    
		char fmt[50];
		sprintf(fmt,"%%%ds mwm_passive_close_button",indent);
		i4_warning(fmt," ");
	}
} ;


static inline int get_res_num(char *name, int def)
{
  i4_const_str s=i4_string_man.get(name);
  if (s.null()) return def;
  i4_const_str::iterator i=s.begin();
  int r=i.read_number();
  int g=i.read_number();
  int b=i.read_number();
  return (r<<16)|(g<<8)|b;
}

// this is the top draggable part of a window
class i4_mwm_drag_bar_class : public i4_window_class
{
private:
  i4_graphical_style_class *hint;
  i4_bool active,dragging;
public:
  i4_const_str title;

  void name(char* buffer) { static_name(buffer,"drag_bar"); }

  enum 
  {
    MIN_WIDTH=15 
  };

  i4_mwm_drag_bar_class(w16 w, w16 h, const i4_const_str &title, i4_graphical_style_class *hint) : 
    i4_window_class(w,h), hint(hint), title(title)
  { 
    active=i4_F;
    dragging=i4_F;
  }

  void activate(i4_bool yes)
  {
    active=yes;
    request_redraw();
  }

  void draw(i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    if (active)
      color=&hint->color_hint->window.active;
    else 
      color=&hint->color_hint->window.passive;


    local_image->add_dirty(0,0,width()-1,height()-1,context);
    local_image->rectangle(0,0,width()-1,height()-1,hint->color_hint->black,context);

    i4_color sc,ec;

    if (active)
    { 
      sc=get_res_num("drag_bar_gradiant_active_start", 0x80); 
      ec=get_res_num("drag_bar_gradiant_active_end", 0x20); 
    }
    else
    {
      sc=get_res_num("drag_bar_gradiant_start", 0x707070); 
      ec=get_res_num("drag_bar_gradiant_end", 0x202020); 
    }

    i4_gradiant_bar(local_image, 1,1,width()-2,height()-2, sc, ec, context);


//     if (dragging)
//       local_image->widget(1,1,width()-2,height()-2,
//                           color->dark,color->medium,color->bright,context);
//     else
//       local_image->widget(1,1,width()-2,height()-2,
//                           color->bright,color->medium,color->dark,context);

    i4_font_class *font=hint->font_hint->normal_font;
    w16 strw=font->width(title);
    w16 strh=font->height(title);

    font->set_color(0xffffff);

    font->put_string(local_image,width()/2-strw/2,height()/2-strh/2,title,context);
  }
    
  virtual void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
    {
      if (!dragging)
      {
	i4_window_request_drag_start_class drag(this);
	i4_kernel.send_event(parent,&drag);
	if (drag.return_result)
	{
	  dragging=i4_T;
	  request_redraw();
	}
      }
    } else if (ev->type()==i4_event::MOUSE_BUTTON_UP)
    {
      if (dragging)
      {
        i4_window_request_drag_end_class end_drag(this);
	i4_kernel.send_event(parent,&end_drag);
	dragging=i4_F;
	request_redraw();
      }
    } 
  }

};


class i4_mwm_window_class : public i4_draggable_window_class
{
protected:
  i4_graphical_style_class *hint;
  i4_mwm_drag_bar_class *drag;
  i4_mwm_close_button_class *close_button;
  i4_bool active,draw_frame;
  w16 left,right,top,bottom;
  i4_parent_window_class *user_area;
  i4_event_reaction_class *on_delete;
 
  w32 close_button_width(i4_graphical_style_class *style)   
  { return 2+style->icon_hint->close_icon->width()+3; }

  w32 close_button_height(i4_graphical_style_class *style)   
  { return 2+style->icon_hint->close_icon->height()+3; }

  
  w32 dragbar_height(i4_graphical_style_class *style, const i4_const_str &title)
  {    
    return hint->font_hint->normal_font->height(title)+4+1;
  }

public:
  ~i4_mwm_window_class()
  {
    if (on_delete)
      delete on_delete;
    on_delete = 0;
  }
  void name(char* buffer) { static_name(buffer,"mwm window"); }
  i4_parent_window_class *user_window() { return user_area; }

  i4_mwm_window_class(w16 width, w16 height, 
                      const i4_const_str &title, 
					  i4_bool show_close_button,
                      i4_graphical_style_class *hint,
                      i4_event_reaction_class *on_delete)
    : i4_draggable_window_class(width,height), hint(hint),
      on_delete(on_delete)
  {   
    left=2;
    if (close_button_height(hint)>dragbar_height(hint,title))
      top=(w16)close_button_height(hint);
    else
      top=(w16)dragbar_height(hint,title);

    top+=2;  // top border

    right=2;
    bottom=2;

    
    active=i4_F;
    draw_frame=i4_T;
    
    resize(w+left+right,h+top+bottom);
    
    drag=new i4_mwm_drag_bar_class((w16)(w-left-right-close_button_width(hint)),
                                   top-2,
                                   title,hint);
    add_child(2,2,drag);

	if (show_close_button)
	{
		close_button=new i4_mwm_close_button_class((w16)close_button_width(hint),
												(w16)close_button_height(hint),
												hint);

		add_child((short)(w-right-close_button_width(hint)),2,close_button);
	}
	else
	{
		close_button=new i4_mwm_passive_close_button_class((w16)close_button_width(hint),
			(w16)close_button_height(hint),
			hint);

		add_child((short)(w-right-close_button_width(hint)),2,close_button);
	}

    user_area=i4_add_color_window(this, hint->color_hint->window.passive.medium,
                                  hint,
                                  left, top, 
                                  width, height);
  } 

  virtual void parent_draw(i4_draw_context_class &context)
  {    
    if (draw_frame ||
	!undrawn_area.clipped_away(0,0,0+width()-1,0+1)     ||
	!undrawn_area.clipped_away(0,0,0+1,0+height()-1)  ||
	!undrawn_area.clipped_away(0+1,0+height()-2,0+width()-1,0+height()-1)  ||
	!undrawn_area.clipped_away(0+width()-2,0,0+width()-1,0+height()-1))
	
    {
      i4_color_hint_class::bevel *color;
      if (active)
        color=&hint->color_hint->window.active;
      else 
        color=&hint->color_hint->window.passive;


      local_image->add_dirty(0,0,width()-1,1,context);
      local_image->add_dirty(0,2,1,height()-1,context);
      local_image->add_dirty(2,height()-2,width()-1,height()-1,context);
      local_image->add_dirty(width()-2,2,width()-1,height()-3,context);

      local_image->bar(0,0,width()-1,0,color->medium,context);
      local_image->bar(0,1,0,height()-1,color->medium,context);

      local_image->bar(1,1,width()-2,1,color->bright,context);
      local_image->bar(1,2,1,height()-2,color->bright,context);

      local_image->bar(width()-2,2,width()-2,height()-2,color->medium,context);
      local_image->bar(2,height()-2,width()-3,height()-2,color->medium,context);

      local_image->bar(width()-1,0,width()-1,height()-1,hint->color_hint->black,context);
      local_image->bar(0,height()-1,width()-2,height()-1,hint->color_hint->black,context);
      draw_frame=i4_F;
    }


    if (!undrawn_area.empty())
      local_image->bar(2,2,width()-3,height()-3,hint->color_hint->window.passive.medium,context);

  }

  virtual i4_bool need_redraw() 
  { 
	  return (i4_bool)(i4_parent_window_class::need_redraw()|draw_frame); 
  }

  void receive_event(i4_event *ev)
  {
	  if (ev->type()==i4_event::WINDOW_MESSAGE)
	  {
		  CAST_PTR(f,i4_window_message_class,ev);
		  if (f->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
		  {
			  active=i4_T;
			  drag->activate(i4_T);
			  close_button->activate(i4_T);
			  draw_frame=i4_T;

		  }
		  else if (f->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
		  {
			  drag->activate(i4_F);
			  close_button->activate(i4_F);
			  active=i4_F;
			  draw_frame=i4_T;

		  } 
		  else if (f->sub_type==i4_window_message_class::NOTIFY_RESIZE)
		  {
			  CAST_PTR(res,i4_window_notify_resize_class,ev);

			  //Fixme: Must not add a window in answer to notify_resize
			  //or fix the resize() recursion;
			  //actually, we can just safely ignore this message.
			  //It would be usefull for sizeable windows, but
			  //mwm_windows are not sizeable (currently).

			  //i4_mwm_window_class *new_parent=new i4_mwm_window_class(res->new_width,
			  //                                                        res->new_height,
			  //                                                        drag->title,
			  //                                                        hint,
			  //                                                        on_delete ? 
			  //                                                        on_delete->copy() : 0);

			  //user_area->transfer_children(new_parent->user_area,0,0);

			  // this will be post-poned
			  //i4_kernel.delete_handler(this);
			  //parent->add_child(x(),y(),new_parent);

			  //resize(res->new_width,res->new_height);

		  }

		  i4_draggable_window_class::receive_event(ev);
	  } else if (ev->type()==i4_event::USER_MESSAGE)
	  {
		  CAST_PTR(f,i4_mwm_event_class,ev);
		  if (f->sub_type==i4_mwm_event_class::CLOSE_YOURSELF)
		  {
			  if (on_delete)
				  i4_kernel.send(on_delete);

			  i4_kernel.delete_handler(this);
		  }
	  } else i4_draggable_window_class::receive_event(ev);    
  }

} ;

class i4_mwm_modal_window_class:public i4_mwm_window_class
	{
	public:
	i4_mwm_modal_window_class(w16 width, w16 height,
		const i4_const_str &title,
		i4_bool show_close_button, 
		i4_graphical_style_class *hint,
		i4_event_reaction_class *on_delete)
		:i4_mwm_window_class(width,height, 
                      title, show_close_button, 
                      hint,
                      on_delete)
		{
		i4_kernel.set_modal(this);
		//change_mouse_focus(this);//this doesn't seem to work at all
		//need to find a way to automatically get the focus when opened
		i4_mwm_event_class self_ev(i4_mwm_event_class::GRAB_FOCUS,this);
		i4_kernel.send_event(this,&self_ev);
		}

	virtual void receive_event(i4_event *ev)
		{
		CAST_PTR(mev,i4_mwm_event_class,ev);
		if (mev->type()==i4_event::USER_MESSAGE&&mev->sub_type==i4_mwm_event_class::GRAB_FOCUS)
			{
			//change_mouse_focus(this);
			i4_mouse_move_event_class mmev(mouse_x,mouse_y,x()+1,y()+1);
			i4_kernel.broadcast_event_type(&mmev,i4_device_class::FLAG_MOUSE_MOVE);
			return;
			}
		i4_mwm_window_class::receive_event(ev);
		}

	~i4_mwm_modal_window_class()
		{
		i4_kernel.end_modal(this);
		}
	virtual void name(char* buffer) 
	{
		static_name(buffer, "i4_mwm_modal_window");
	};
	};


i4_parent_window_class *i4_mwm_style_class::create_mp_window(i4_coord x, i4_coord y, 
                                                             w16 w, w16 h, 
                                                             const i4_const_str &title,
                                                             i4_event_reaction_class *on_delete)
{
    i4_parent_window_class *parent=i4_current_app->get_root_window();

  i4_mwm_window_class *win=new i4_mwm_window_class(w,h,title,i4_T,this,on_delete);
  if (x==-1) 
    x=parent->width()/2-w/2;
  if (y==-1)
    y=parent->height()/2-h/2;

  parent->add_child(x,y,win);
  return win->user_window();
}

i4_parent_window_class *i4_mwm_style_class::create_modal_window(i4_coord x, i4_coord y, 
                                                             w16 w, w16 h, 
                                                             const i4_const_str &title,
															 i4_bool show_close_button,
                                                             i4_event_reaction_class *on_delete)
	{
	i4_parent_window_class *root=i4_current_app->get_root_window();

    i4_mwm_modal_window_class *win=new i4_mwm_modal_window_class(w,h,title,
		show_close_button,this,on_delete);
    if (x==-1) 
      x=root->width()/2-w/2;
    if (y==-1)
      y=root->height()/2-h/2;

    root->add_child(x,y,win);
    return win->user_window();
	}
// DRAGWIN
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


class i4_drag_frame_class : public i4_window_class
{
  i4_coord ghost_x,ghost_y;

  public :
  i4_bool need_remove_ghost,
    draw_self;

  void name(char* buffer) { static_name(buffer,"drag_frame"); }

  i4_drag_frame_class(w16 w, w16 h) : 
    i4_window_class(w,h)
  {
    need_remove_ghost=i4_F;
    draw_self=i4_T;
  }

  virtual void draw(i4_draw_context_class &context)
  {
    // this draw is special in that it should not be subject to clipping and
    // window coordinate transformations

    i4_rect_list_class no_clip(0,0,local_image->width()-1,local_image->height()-1);
    context.clip.swap(&no_clip);
    i4_coord old_xoff=context.xoff,old_yoff=context.yoff;
    context.xoff=0;
    context.yoff=0;

    if (need_remove_ghost)
    {
      i4_coord x1=ghost_x,y1=ghost_y;
      i4_coord x2=x1+width()-1,y2=y1+height()-1;

      i4_color c=0xffffffff;

      local_image->xor_bar(x1,y1,x2,y1,c,context);
      local_image->xor_bar(x2,y1,x2,y2,c,context);
      local_image->xor_bar(x1,y1,x1,y2,c,context);
      local_image->xor_bar(x1,y2,x2,y2,c,context);      
    }

    if (draw_self)
    {
      i4_coord x1=x(),y1=y();
      i4_coord x2=x1+width()-1,y2=y1+height()-1;
      i4_color c=0xffffffff;

      local_image->xor_bar(x1,y1,x2,y1,c,context);
      local_image->xor_bar(x2,y1,x2,y2,c,context);
      local_image->xor_bar(x1,y1,x1,y2,c,context);
      local_image->xor_bar(x1,y2,x2,y2,c,context);
      ghost_x=x();
      ghost_y=y();
    }
    draw_self=i4_T;
    need_remove_ghost=i4_T;

    context.xoff=old_xoff;
    context.yoff=old_yoff;
    context.clip.swap(&no_clip);
  }

  virtual void show_self(w32 indent) 
  {    
    char fmt[50];
    sprintf(fmt,"%%%ds drag_window",indent);
    i4_warning(fmt," ");
  }

} ;


i4_draggable_window_class::i4_draggable_window_class(w16 w, w16 h) : i4_parent_window_class(w,h)
{
  drag_frame=0;
  last_mouse_x=0;
  last_mouse_y=0;
}

void i4_draggable_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(message,i4_window_request_drag_start_class,ev);
    // must have a parent in order to move/be dragged
    if (message->sub_type==i4_window_message_class::REQUEST_DRAG_START && parent)  
    {
      // see if we can grab the mouse     
      i4_window_request_mouse_grab_class grab(message->from());

      // ask ourself, we will check with our parent..
      i4_kernel.send_event(this,&grab);

      if (grab.return_result)
      {
        drag_frame=new i4_drag_frame_class(width(),height());
        parent->add_child(x(),y(),drag_frame);
        
        message->return_result=i4_T;      
      }
      else 
      {
        message->return_result=i4_F;
        i4_warning("draggable window:: unable to grab mouse");
      } 
    }
    // must have a parent in order to move/be dragged
    else if (message->sub_type==i4_window_message_class::REQUEST_DRAG_END && parent)  
    {
      if (!drag_frame)
        i4_error("got drag end message and not dragging");

      i4_window_request_mouse_ungrab_class ungrab(this);
      i4_kernel.send_event(this,&ungrab);

      // move ourself to the final position of the drag frame
      move(drag_frame->x()-x(),drag_frame->y()-y());
      request_redraw();

      // tell ourselves to delete the dragger next tick, 
      // so it gets a chance to clear itself off the screen
	  drag_frame->call_stack_counter++;
      i4_kernel.delete_handler(drag_frame);
	  drag_frame->call_stack_counter--;
      drag_frame->need_remove_ghost=i4_T;
      drag_frame->draw_self=i4_F;
      drag_frame=0;   // assume it is deleted now
    }
    else i4_parent_window_class::receive_event(ev);
  } else if (ev->type()==i4_event::MOUSE_MOVE)
  { 
    CAST_PTR(move,i4_mouse_move_event_class,ev);
    current_mouse_x=move->x+x();
    current_mouse_y=move->y+y();
    if (drag_frame &&
	((current_mouse_x!=last_mouse_x ||
          current_mouse_y!=last_mouse_y)))
    {
      if (current_mouse_x<0)
        current_mouse_x=0;
      if (current_mouse_y<0)
        current_mouse_y=0;
      i4_coord xo=current_mouse_x-last_mouse_x,yo=current_mouse_y-last_mouse_y;
      drag_frame->move(xo,yo,i4_F);
      drag_frame->request_redraw();
    }

    last_mouse_x=current_mouse_x;
    last_mouse_y=current_mouse_y;
    i4_parent_window_class::receive_event(ev);
  } else i4_parent_window_class::receive_event(ev);
}
// CURSOR.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



i4_cursor_class *i4_load_cursor(char *cursor_name,
                                i4_string_manager_class *sman)
{
  i4_const_str st=sman->get(cursor_name);    // looks up the resource
  sw32 trans,hot_x,hot_y;

  i4_const_str::iterator s=st.begin();
  i4_str *fn=s.read_string();
  trans=s.read_number();
  hot_x=s.read_number();
  hot_y=s.read_number();


  i4_image_class *im=i4_load_image(*fn);  // try to load up the cursor image
  if (!im) 
  {
    delete fn;  
    return 0;
  }
  else 
  {
    i4_cursor_class *c=new i4_cursor_class(im,(i4_color)trans,(short)hot_x,(short)hot_y);
    delete im;
    delete fn;
    return c;
  }

}



i4_cursor_class::i4_cursor_class(i4_image_class *_pict, 
                                 i4_color trans, 
                                 i4_coord hot_x, i4_coord hot_y,
                                 const i4_pal *convert_to) 
: trans(trans),
  hot_x(hot_x),hot_y(hot_y)
{
  pict=0;
  if (convert_to)
  {
    int w=_pict->width(), h=_pict->height();
    pict=i4_create_image(w,h, convert_to);
      
    i4_draw_context_class context(0,0, w-1,h-1);
    pict->clear(trans, context);
    _pict->put_image_trans(pict, 0,0, trans, context);
  }
  else pict=_pict->copy();
}

i4_cursor_class::~i4_cursor_class() 
{
  if (pict)
    delete pict;
}

i4_cursor_class::i4_cursor_class()
{
  pict = 0;
}

i4_cursor_class *i4_cursor_class::copy(const i4_pal *convert_to)
{
  return new i4_cursor_class(pict, trans, hot_x, hot_y, convert_to);
}

// COLORWIN.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



void i4_color_window_class::parent_draw(i4_draw_context_class &context)
{     
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);

  child_clip.swap(&context.clip);

  
  if (color==style->color_hint->neutral())
  {
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
      style->deco_neutral_fill(local_image, c->x1, c->y1, c->x2, c->y2, context);
  }
  else local_image->clear(color,context);

  child_clip.swap(&context.clip);
  i4_parent_window_class::parent_draw(context);
}

i4_parent_window_class *i4_add_color_window(i4_parent_window_class *parent, i4_color color, 
                                            i4_graphical_style_class *style,
                                            i4_coord x, i4_coord y, w16 w, w16 h)
{
  i4_color_window_class *cw=new i4_color_window_class(w,h,color, style);
  if (parent)
    parent->add_child(x,y,cw);
  return cw;
}







