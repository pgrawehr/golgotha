/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DX9_MOUSE_HH
#define DX9_MOUSE_HH

#include "video/win32/dx9_util.h"
#include "window/cursor.h"

class i4_cursor_class;

class dx9_mouse_class
{
  struct save_struct
  {
    i4_dx9_image_class *save_buffer;
    int x,y;
    save_struct() { save_buffer=0;x=0;y=0;}
	i4_bool isgdi;//to sync the buffer with the on - screen data (may skip frames)

    ~save_struct()
    {
      if (save_buffer)
        delete save_buffer;
      save_buffer=0;
    }
  } 
  current, last;

  i4_cursor_class cursor;
  i4_bool page_flipped;
public:
  dx9_mouse_class(i4_bool page_flipped);
  virtual void save_and_draw(int x, int y);
  virtual void restore();
  virtual void set_cursor(i4_cursor_class *cursor);
  void add_dirty_area(i4_draw_context_class* context);
  virtual ~dx9_mouse_class();
} ;


#endif
