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
 

#ifndef __LIST_PICK_HH
#define __LIST_PICK_HH

#include "window/window.h"
#include "gui/scroll_bar.h"
class i4_menu_item_class;

class i4_list_pick : public i4_parent_window_class
{
  sw32 total_items;
  i4_window_class **items;
  i4_color background;
  w32 length;
  w32 scroll_event_id;
  sw32 start,end;
  i4_bool need_draw_all, free_items;
  i4_scroll_bar *scrollbar;

public:
	enum{
		LB_SCROLLSELF=0x80000000,
		
		};
  char *name() { return "list_pick"; }
  
  i4_list_pick(w16 width, w16 height,
               w32 total_items,
               i4_window_class **items,
               w32 scroll_event_id,//if MSB is 1, the list generates the scrollbar itself
			   //with the given id (may be 0)
               i4_color background,
               i4_bool free_item=i4_T);

  ~i4_list_pick();

  virtual void update(w32 new_total_items,i4_window_class **new_items);

  virtual void reposition_start(sw32 new_start);
  
  virtual void parent_draw(i4_draw_context_class &context);

  //should not overwrite draw in descendants of parent_window_class.
  //It even seems that the implemented draw is aequivalent to its parent's.
  //virtual void draw(i4_draw_context_class &context);

  virtual void receive_event(i4_event *ev);
};


#endif

