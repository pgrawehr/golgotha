
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
 

#ifndef OPENGL_DISPLAY_HH
#define OPENGL_DISPLAY_HH

#include "video/display.h"
#include "error/alert.h"
#include "app/app.h"
#include <GL/gl.h>
//#include <GL/glu.h>

class i4_cursor_class;

class i4_opengl_display_class : public i4_display_class
{

  float oo_half_width;
  float oo_half_height;

  i4_draw_context_class *context;

  i4_cursor_class *mcursor;
  i4_image_class *prev_mouse_save, *mouse_save1, *mouse_save2;
  sw32 prev_mouse_x, prev_mouse_y;

  void draw_cursor(sw32 x, sw32 y);
  void save_cursor(sw32 x, sw32 y, i4_image_class *&save);
  void remove_cursor(sw32 x, sw32 y, i4_image_class *mouse_save);

  i4_image_class *fake_screen; // passed by get_screen()
  i4_image_class *back_screen; // passed by lock_frame_buffer() for reading
  i4_pal *back_pal;
  i4_rect_list_class next_frame_copy;
  sw32 last_mouse_x, last_mouse_y;

protected:
  //i4_display_list_struct me;

  // It will be up to the system-specific implementation to use the
  // following mode variables.
  mode *modes;
  int n_modes, mode_pointer;
  virtual void setup_modes() = 0;
  mode cur_mode; // will hold the current mode as passed to initialize_mode()

public:

  virtual i4_device_class *local_devices() = 0;
  virtual void destroy_window() = 0;
  virtual i4_bool create_window(w32 xres, w32 yres) = 0;
  virtual void get_mouse_pos(sw32 &mouse_x, sw32 &mouse_y) = 0;
  virtual void swap_buffers() = 0;
  virtual i4_bool input_exposed() {return i4_F;}//usefull way to communicate special requests.
  virtual i4_bool lock_mouse_in_place(i4_bool yes_no) = 0;

  void get_pl_pos(int pixel_x, int pixel_y, float &glx, float &gly)
  {
    glx = pixel_x *2.0 / (float)(width()) -1;
    gly = ((height()-pixel_y) *2.0/  (float)height() -1 );
  }

  virtual i4_image_class *lock_frame_buffer(i4_frame_buffer_type type,i4_frame_access_type access);
  virtual void unlock_frame_buffer(i4_frame_buffer_type type);

  virtual i4_refresh_type update_model() { return I4_PAGE_FLIP_REFRESH; }

  virtual char *name() { return "OpenGL display"; }
  virtual i4_draw_context_class *get_context() { return context; }
  virtual i4_image_class *get_screen() { return fake_screen; }
  i4_bool set_mouse_shape(i4_cursor_class *cursor);
  virtual void flush();
  virtual w16 width() const { return fake_screen->width(); }
  virtual w16 height() const { return fake_screen->height(); }

  virtual mode *current_mode() { return &cur_mode; }  
  virtual mode *get_first_mode(int driver_id); 
  virtual mode *get_next_mode();
  virtual i4_bool initialize_mode(mode *m);
  virtual i4_bool initialize_mode();
  //{ return initialize_mode(&modes[0]); } // ?????!!!!!
  virtual void init();

  virtual i4_bool close();
  //  virtual i4_bool realize_palette(i4_pal_handle_class pal_id) { return i4_F; }


  i4_opengl_display_class();

};

extern i4_opengl_display_class *i4_opengl_display;

#endif
