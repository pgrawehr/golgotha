/**********************************************************************

   	Golgotha Forever - A portable, free 3D strategy and FPS game.
   	Copyright (C) 1999 Golgotha Forever Developers

   	Sources contained in this distribution were derived from
   	Crack Dot Com's public release of Golgotha which can be
   	found here:  http://www.crack.com

   	All changes and new works are licensed under the GPL:

   	This program is free software; you can redistribute it and/or modify
   	it under the terms of the GNU General Public License as published by
   	the Free Software Foundation; either version 2 of the License, or
   	(at your option) any later version.

   	This program is distributed in the hope that it will be useful,
   	but WITHOUT ANY WARRANTY; without even the implied warranty of
   	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   	GNU General Public License for more details.

   	You should have received a copy of the GNU General Public License
   	along with this program; if not, write to the Free Software
   	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   	For the full license, see COPYING.

 ***********************************************************************/


#include "pch.h"
#include "gui/list_pick.h"
#include "menu/menuitem.h"
#include "image/image.h"
#include "gui/scroll_bar.h"
#include "app/app.h"


i4_list_pick::i4_list_pick(w16 my_width, w16 my_height,
						   w32 total_items,
						   i4_window_class **my_items,
						   w32 my_scroll_event_id,
						   i4_color background,
						   i4_bool free_items)

	: i4_parent_window_class(my_width, my_height),
	  total_items(total_items),
	  items(my_items),
	  background(background),
	  scroll_event_id(my_scroll_event_id),
	  free_items(free_items)
{
	need_draw_all=i4_T;
	scrollbar=0;
	start=0;
	end=-1;
	length=0;
	for (w32 i=0; i<total_items; i++)
	{
		if (items[i]->width()>length)
		{
			length=items[i]->width();
		}
	}
	length+=10;

	sw32 visible_items=0;
	if (total_items>0)
	{
		visible_items=height()/items[0]->height();
	}                                         //assume all items have same size
	else
	{
		visible_items=1;
	}
	if (scroll_event_id&LB_SCROLLSELF)
	{
		scrollbar=new i4_scroll_bar(i4_T,
									height(),
									visible_items,
									total_items,
									scroll_event_id,
									this,
									i4_current_app->get_style());
		//that would be a nasty idea. We shan't resize our parent.
		//get_parent()->resize(get_parent()->width()+scrollbar->width(),get_parent()->height());
		//resize(width()+scrollbar->width(),height());
		add_child(width()-scrollbar->width(),0,scrollbar);
		length=length-8+scrollbar->width();
	}
	reposition_start(0);
}

void i4_list_pick::update(w32 new_total_items,i4_window_class **new_items)
{
	total_items=new_total_items;
	items=new_items;
	if (scrollbar)
	{
		scrollbar->set_new_total(total_items);
	}
}


i4_list_pick::~i4_list_pick()
{
	//if (scrollbar) delete scrollbar;
	//scrollbar=0;
	for (w32 i=0; i<total_items; i++)
	{
		if (items[i]->get_parent()==this)
		{
			remove_child(items[i]);
		}                        //remove all childs (except the scrollbar)
	}
	if (free_items)
	{
		for (w32 i=0; i<total_items; i++)
		{
			delete items[i];
			items[i]=0;
		}
		//i4_free(items);
	}

	//do not pass call to parent as our children must not be deleted here.
	//parent has no idea which ones are currently being displayed.

	//unfortunatelly, we cannot stop a virtual destructor from executing
	//the parents code.
}

void i4_list_pick::reposition_start(sw32 new_start)
{
	sw32 old_end=end,old_start=start,i;
	i4_coord x1=x(),y1=y(),x2=x()+width(),y2=y()+height();

	if (new_start!=start)
	{
		need_draw_all=i4_T;
	}

	i4_window_class *c;

	for (i=new_start; x1<x2 && y1<y2 && i<(sw32)total_items;)
	{
		c=items[i];


		//This code would allow multiple rows in a list, but
		//I couldn't find a use for this at the moment.
		//if (c->height()+y1>=y2)
		//{
		//  x1+=(short)length;
		//  y1=y();
		//}

		if (i>=end || i<start)
		{
			//      i4_warning("add    %d",i);
			add_child(x1-x(),y1-y(),c);
		}
		else
		{
			//      i4_warning("move   %d",i);
			c->move(x1-c->x(),y1-c->y());
		}

		y1+=c->height();
		i++;
	}

	start=new_start;
	end=i;

	if (old_start<start && old_end>=start)
	{
		for (i=old_start; i<start; i++)
		{
			//      i4_warning("remove %d",i);
			remove_child(items[i]);
		}
	}

	if (old_start<=end && old_end>end)
	{
		for (i=end; i<old_end; i++)
		{
			//      i4_warning("remove %d",i);
			remove_child(items[i]);
		}
	}


	if (old_end<start || old_start>end)
	{
		for (i=old_start; i<old_end; i++)
		{
			//      i4_warning("remove %d",i);
			remove_child(items[i]);
		}
	}
	if (scrollbar)
	{
		scrollbar->bring_to_front();
	}
	request_redraw();
}

void i4_list_pick::parent_draw(i4_draw_context_class &context)
{
	if (need_draw_all)
	{
		local_image->add_dirty(0,0, width()-1, height()-1, context);
		local_image->clear(background,context);
		need_draw_all=i4_F;
	}
	else
	{
		i4_rect_list_class child_clip(&context.clip,0,0);
		child_clip.intersect_list(&undrawn_area);

		child_clip.swap(&context.clip);

		local_image->clear(background,context);

		child_clip.swap(&context.clip);
	}
}


void i4_list_pick::receive_event(i4_event *ev)
{
	if (ev->type()==i4_event::USER_MESSAGE)
	{
		CAST_PTR(uev, i4_user_message_event_class, ev);
		if (uev->sub_type==scroll_event_id)
		{
			CAST_PTR(sc_msg, i4_scroll_message, ev);
			if (sc_msg->scroll_total > 0)
			{
				reposition_start(sc_msg->amount*total_items/sc_msg->scroll_total);
			}
			else
			{
				reposition_start(0);
			}
		}
		else
		{
			//user events have no meaning for parent_window_class
			//i4_parent_window_class::receive_event(ev);

			//send event (somebody clicked on us) to parent
			//actually, the items[] event-handler is usually the parent itself
			if (parent)
			{
				i4_kernel.send_event(parent,ev);
			}

		}
	}
	else
	{
		i4_parent_window_class::receive_event(ev);
	}
}

void i4_list_pick_cpitems::receive_event(i4_event *ev)
{
	i4_list_pick::receive_event(ev);
}

void i4_list_pick_cpitems::update(w32 new_total_items,
								  i4_window_class **new_items)
{
	w32 i;
	for (i=0; i<total_items; i++)
	{
		if (items[i]->get_parent()==this)
		{
			remove_child(items[i]);
		}                          //remove all childs (except the scrollbar)
	}
	if (free_items)
	{
		for (w32 i=0; i<total_items; i++)
		{
			delete items[i];
			items[i]=0;
		}
	}
	if (new_items!=0)
	{
		items=(i4_window_class **)realloc(items,new_total_items*sizeof(i4_window_class*));
		total_items=new_total_items;
		memcpy(items,new_items,new_total_items*sizeof(i4_window_class*));
	}
	else
	{
		if (items)
		{
			free(items);
		}
		items=0;
		total_items=0;
	}
	need_draw_all=i4_T;
	start=0;
	end=-1;
	length=0;
	for (i=0; i<total_items; i++)
	{
		if (items[i]->width()>length)
		{
			length=items[i]->width();
		}
	}
	length+=10;

	sw32 visible_items=0;
	if (total_items>0)
	{
		visible_items=height()/items[0]->height();
	}                                         //assume all items have same size
	else
	{
		visible_items=1;
	}
	if (scrollbar)
	{
		scrollbar->set_new_total(total_items);
		scrollbar->bring_to_front();
		scrollbar->set_pos(0);
	}
	reposition_start(0);
	request_redraw();
	request_redraw(i4_T);
}

// draw should redraw supplied area of self
/*
   void i4_list_pick::draw(i4_draw_context_class &context)
   {
   if (!undrawn_area.empty() || redraw_flag)
   {
   	redraw_flag=i4_F;
   	parent_draw(context);
   	undrawn_area.delete_list();
   }


   if (child_need_redraw)
   {
   	child_need_redraw=i4_F;
   	win_iter c=children.begin();
   	for (;c!=children.end();++c)
   	{
   	  if (c->need_redraw())
   	  {
   		// copy the clip list, and move it to local coordinates
   		i4_rect_list_class child_clip(&context.clip,
   									  -(c->x()-x()),
   									  -(c->y()-y()));

   		// intersect the clip with what our area is supposed to be
   		child_clip.intersect_area(0,0,
   								  c->width()-1,
   								  c->height()-1);

   		sw16 old_xoff=context.xoff,old_yoff=context.yoff;   // save the old context xoff and yoff
   		context.xoff+=(c->x()-x());                // move the context offset to the child's x,y
   		context.yoff+=(c->y()-y());

   		child_clip.swap(&context.clip);

   		c->draw(context);         // the child is ready to draw

   		context.xoff=old_xoff;    // restore the context's x & y offsets
   		context.yoff=old_yoff;


   		child_clip.swap(&context.clip);   // restore the old clip list
   	  }
   	}
   }
   }
 */
