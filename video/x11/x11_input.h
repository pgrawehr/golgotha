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
 

#ifndef __X11_INPUT_HH
#define __X11_INPUT_HH

#include "device/device.h"
#include "time/time.h"
#include <X11/Xlib.h> 
#include <X11/Xutil.h>

class i4_display_class;
class x11_shm_extension_class;
class i4_draw_context_class;

class x11_input_class : public i4_device_class 
{
public:
  Atom                    wm_delete_window, wm_protocols;
  XVisualInfo             *my_visual;
  Display                 *display;
  i4_bool                  mouse_locked;
  Window                   mainwin;
  Window                   errorwin;
  GC                       gc;
  i4_display_class        *i4_display;
  Colormap                 xcolor_map;
  int                      screen_num;
  //set to true by an exposure event
  //to indicate when the display should be redrawn completelly.
  i4_bool					   was_exposed;

  i4_draw_context_class   *context;
  w16                      modifier_state;            // keyboard shift state
  sw32                     mouse_x, mouse_y;
  i4_time_class            last_down[3], last_up[3];
  
  void get_x_time(w32 xtick, i4_time_class &t);

  i4_time_class i4_start_time;
  i4_bool need_first_time;
  sw32 first_time;
  i4_bool repeat_on;

  XVisualInfo *find_visual_with_depth(int depth);
  char *name() { return "X11 input"; } 

  i4_bool open_display();
  void close_display();

  i4_bool create_window(sw32 x, sw32 y, w32 w, w32 h, 
                        i4_display_class *i4_display,
                        i4_bool takeup_fullscreen,
                        XVisualInfo *visual);
  int create_error_window(const char *error_msg);
  void draw_error(Display *disp, const char *error_msg, int width, int height, int screen_e_num);
  void destroy_window();

  virtual i4_bool process_events();

  virtual void resize(int w, int h) { ; }
  virtual void note_event(XEvent &xev) { ; }

  i4_bool lock_mouse_in_place(i4_bool yes_no)
  {
    mouse_locked=yes_no;
    return i4_T;
  }

  x11_input_class();
  ~x11_input_class();
};

int showerror(const char *error_msg);


#endif
