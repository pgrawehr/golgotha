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


#ifndef __MITSHM_HPP_
#define __MITSHM_HPP_

// this module can be compiled seperately from x11_display_class
// if a system does not support IPX do not compile this module
// otherwise it will link itself with the x11_display_class on init()

#include "arch.h"
#include "image/image.h"
#include "init/init.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

class x11_shm_extension_class;

class x11_shm_image_class
{
public:
	w8 * data;
	virtual i4_bool copy_part_to_vram(x11_shm_extension_class * use,
									  i4_coord x, i4_coord y,
									  i4_coord x1, i4_coord y1,
									  i4_coord x2, i4_coord y2) = 0;
} ;

class x11_shm_extension_class :
	public i4_init_class
{
public:
	i4_bool need_sync_event;
	int shm_base,shm_error_base,shm_finish_event;



	virtual void note_event(XEvent &ev) = 0;
	x11_shm_extension_class();
	virtual i4_bool available(Display * display, char * display_name) = 0;
	virtual x11_shm_image_class *create_shm_image(Display * display,
												  Window window,
												  GC gc,
												  Visual * X_visual,
												  int visual_depth,
												  w16 &width, w16 &height) = 0;

	virtual void destroy_shm_image(Display * display, x11_shm_image_class * im) = 0;
	virtual void shutdown(Display * display) = 0;
} ;

x11_shm_extension_class *create_x11_shm_extension_class();


#endif
