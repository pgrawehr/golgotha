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
#include "threads/threads.h"
#include "video/opengl/opengl_display.h"
#include "video/win32/win32_input.h"
#include "main/win_main.h"

class i4_win32_opengl_display_class :
	public i4_opengl_display_class
{
public:
	virtual char *name()
	{
		return " OpenGL ";
	}

	win32_input_class input;
	virtual i4_device_class *local_devices()
	{
		return &input;
	}
	virtual void destroy_window()
	{
		input.destroy_window();
	}
	virtual i4_bool create_window(w32 xres,w32 yres)
	{
		return input.create_window(0,0,xres,
								   yres, this, i4_win32_startup_options.fullscreen);
	}


	virtual void get_mouse_pos(sw32 &mouse_x, sw32 &mouse_y)
	{
		mouse_x=input.mouse_x;
		mouse_y=input.mouse_y;
	}

	virtual i4_bool lock_mouse_in_place(i4_bool yes_no)
	{
		return input.lock_mouse_in_place(yes_no);
	}

	virtual unsigned long window_handle()
	{
		return (unsigned long)i4_win32_window_handle;
	}

	i4_win32_opengl_display_class(char *message) :
		input(message)
	{
	}

};

i4_win32_opengl_display_class i4_win32_opengl_display_instance("Hit Left Control + F9 to return to Golgotha");
