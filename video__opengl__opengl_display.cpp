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
#include "image/image.h"
#include "image/context.h"
#include "image/image16.h"
#include "image/image32.h"
#include "video/opengl/opengl_display.h"

#include <stdlib.h>
#ifndef _WINDOWS
#include <X11/Xlib.h> //no X11 support available on windows (and doesn't make sense anyway)
#include <X11/Xutil.h>
#include <GL/glx.h>
#endif

//#define GL_NOREALPAGEFLIP
static void prepare_fb_write() {

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);
  glDepthMask(GL_FALSE);
  glDisable(GL_BLEND);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

}

static void done_fb_write() {

  glPopAttrib();

}

i4_opengl_display_class *i4_opengl_display=0;


i4_opengl_display_class::i4_opengl_display_class()
{
  i4_opengl_display = this;

  mcursor = 0;
  prev_mouse_save = 0;
  mouse_save1 = mouse_save2 = 0;
  prev_mouse_x = prev_mouse_y = 40;

  modes = NULL;
  n_modes = 0;
  mode_pointer = 0;

  back_screen = 0;
  back_pal = 0;
}

i4_bool i4_opengl_display_class::set_mouse_shape(i4_cursor_class *cursor)
{
  if (mcursor) {
    delete mcursor;
	mcursor = 0;
  }

  if (cursor)
	mcursor = cursor->copy(get_palette());

  return i4_T;
}

// this was used to add a rectangular region to screen
void i4_opengl_display_class::remove_cursor(sw32 x, sw32 y, i4_image_class *mouse_save)
{
  if (mcursor && mouse_save) {

	sw32 w = mouse_save->width();
	sw32 h = mouse_save->height();

	sw32 yoffset = 0;

	sw32 x1 = x;
	sw32 y1 = y;
	sw32 y2 = y + h - 1;
	sw32 x2 = x + w - 1;

	if (y < 0) {
	  y1 = 0;
	  yoffset = -y;

	}
	if (y2 < y1)
	  y2 = y1;

	glDrawBuffer(GL_BACK);
  	glRasterPos2d( oo_half_width * (float)x1 - 1.f, oo_half_height * (float)y1 - 1.f);
	glDrawPixels(x2 - x1 + 1, y2 - y1 + 1,
				 GL_RGB,GL_UNSIGNED_SHORT_5_6_5,(GLvoid *)((w16*)mouse_save->data + yoffset * w));

  }

}

// save fb under region where cursor is to be drawn
void i4_opengl_display_class::save_cursor(sw32 x, sw32 y, i4_image_class *&save) {

 if (mcursor) {

   sw32 w = mcursor->pict->width();
   sw32 h = mcursor->pict->height();

   if (save && (save->width() != w ||
				save->height() != h ))
	 {
	   delete save;
	   save = 0;
	 }

   if (!save)
	 save = i4_create_image(w, h, pal);

	sw32 x1 = x;
	sw32 y1 = y;
	sw32 x2 = x + w - 1;
	sw32 y2 = y + h - 1;
	sw32 yoffset = 0;

	if (y < 0) {
	  y1 = 0;
	  yoffset = -y;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadBuffer(GL_BACK);
	glReadPixels(x1, y1, x2 - x1 + 1, y2 - y1 + 1,
				 GL_RGB,GL_UNSIGNED_SHORT_5_6_5,(GLvoid *)((w16*)save->data + yoffset * w));

 }


}
	


void i4_opengl_display_class::draw_cursor(sw32 x, sw32 y)
{
  if (mcursor) {

	w32 w = mcursor->pict->width();
	w32 h = mcursor->pict->height();

	i4_draw_context_class no_clip_context(0,0, w-1, h-1);

	// get region under cursor
	i4_image_class *back = 0;
	save_cursor(x,y,back);

	// flip back image vertically
	//w8 tmp[back->bpl];
 //MIPSPRO compiler doesn't like this
	w8 *tmp;
	tmp=(w8*)malloc(back->bpl);
	for (w32 i=0; i<h/2; i++) {
	  w16 *ptr1 = (w16 *)back->data + i * w;
	  w16 *ptr2 = (w16 *)back->data + (h - 1 - i) * w;
	  memcpy(tmp,ptr1,back->bpl);
	  memcpy(ptr1,ptr2,back->bpl);
	  memcpy(ptr2,tmp,back->bpl);
	}
	free(tmp);
	tmp=0;

	mcursor->pict->put_image_trans((i4_image16 *)back,0,0,mcursor->trans,no_clip_context);

	// we're zooming it
	y += h;
	glPixelZoom(1.f,-1.f);

	glRasterPos2f( oo_half_width * (float)x - 1.f, oo_half_height * (float)y - 1.f);
	glDrawPixels(w,h,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,(GLvoid *)back->data);

	glPixelZoom(1.f,1.f);

	delete back;
  }
} 

void i4_opengl_display_class::flush()
{

  sw32 w = width();
  
  context->both_dirty->intersect_area(0,0,width()-1,height()-1);

  i4_rect_list_class::area_iter a=context->both_dirty->list.begin();
  i4_rect_list_class *use_list=0;
#ifdef GL_NOREALPAGEFLIP
  use_list=context->both_dirty;
#else
  for (;a!=context->both_dirty->list.end();++a)
    next_frame_copy.add_area(a->x1, a->y1, a->x2, a->y2);
  use_list=&next_frame_copy;
#endif
  prepare_fb_write();

  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
  glPixelZoom(1,-1);

  a=use_list->list.begin();
  if (input_exposed())
  {
  	glRasterPos2f(-1.0f,1.0f);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	glDrawPixels(width(),height(),GL_RGB,GL_UNSIGNED_SHORT_5_6_5,
		(GLvoid*)fake_screen->data);
  }
  else
  {
  for (;a!=use_list->list.end();++a)
  {  

	glRasterPos2f( (GLfloat)a->x1 * oo_half_width - 1.0 , 1.0 - (GLfloat)a->y1 * oo_half_height );
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, a->x1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, a->y1);

	glDrawPixels(a->x2-a->x1+1, a->y2-a->y1+1, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
				 (GLvoid *)fake_screen->data);
  }
  }
  glPixelZoom(1.f,-1.f);

  glPopClientAttrib();
#ifndef GL_NOREALPAGEFLIP
  next_frame_copy.swap(context->both_dirty);
#endif
  context->both_dirty->delete_list();

  sw32 mouse_x, mouse_y;
  get_mouse_pos(mouse_x, mouse_y);

  if (mcursor) {
	mouse_x -= mcursor->hot_x;
	mouse_y -= mcursor->hot_y + mcursor->pict->height();
  }

  i4_image_class **cur_save;

  if (!prev_mouse_save || prev_mouse_save == mouse_save2)
	cur_save = &mouse_save1;
  else
	cur_save = &mouse_save2;

  save_cursor(mouse_x, mouse_y, *cur_save);
  draw_cursor(mouse_x, mouse_y);

  glFlush();
  swap_buffers();


  if (prev_mouse_save)
	remove_cursor(prev_mouse_x, prev_mouse_y, prev_mouse_save);

  prev_mouse_save = *cur_save;
  prev_mouse_x = mouse_x;
  prev_mouse_y = mouse_y;
  done_fb_write();

  //glClear(GL_DEPTH_BUFFER_BIT);

}


i4_bool i4_opengl_display_class::close()
{

  next_frame_copy.delete_list();

  if (back_screen) {
	delete back_screen;
	back_screen = 0;

	//must not delete any paletes, this is done by the pal manager
	//if (back_pal) {
	//  delete back_pal;
	//  back_pal = 0;
	//}
	
  }

  if (fake_screen)
  {
    delete fake_screen;
    fake_screen=0;

    if (context)
    {
      delete context;
      context=0;
    }
  }
  return i4_T;
}

i4_display_class::mode *i4_opengl_display_class::get_first_mode(int driver_id)
{
  if (!n_modes)
	return NULL;
  mode_pointer=0;
  return &modes[0];
}

i4_display_class::mode *i4_opengl_display_class::get_next_mode()
{
  mode_pointer++;
  if (mode_pointer >= n_modes)
  {
	mode_pointer = 0;
	return 0;
  }
  return &modes[mode_pointer];
}

void i4_opengl_display_class::init()
{
  i4_display_list_struct *s=new i4_display_list_struct;
  s->add_to_list("OpenGL (native)", 0x8000, /*Priority*/ 245, this, i4_display_list);
}

i4_bool i4_opengl_display_class::initialize_mode()
	{
		//memcpy(&cur_mode, &amode, sizeof(cur_mode));
	    i4_warning("Opening X window for OpenGL");
        // this should open an X window
        //return create_window(cur_mode.xres, cur_mode.yres);
	    return initialize_mode(&modes[mode_pointer]);
	}

i4_bool i4_opengl_display_class::initialize_mode(mode *m)
{

  destroy_window();

  if (!create_window(m->xres, m->yres)) {
	i4_alert("OpenGL display: could not initialize mode\n",100,NULL);
	return i4_F;
  }

  oo_half_width = 2.0 / (float)m->xres;
  oo_half_height = 2.0 / (float)m->yres;

  // create image object for reading back buffer
  i4_pixel_format back_f;
  back_f.pixel_depth = I4_16BIT;
  back_f.red_mask =   31 << 11;
  back_f.green_mask = 63 << 5;
  back_f.blue_mask =  31;
  back_f.alpha_mask = 0;
  back_f.calc_shift();

  //if (back_pal)
  //	delete back_pal;
  back_pal = i4_pal_man.register_pal(&back_f);

  if (back_screen)
	delete back_screen;
  back_screen = i4_create_image(m->xres, m->yres, back_pal);

  i4_pixel_format f;
  f.lookup      = 0;
  f.pixel_depth = I4_16BIT;
  f.red_mask  =  31 << 11;
  f.green_mask = 63 << 5;
  f.blue_mask =  31;
  f.alpha_mask = 0;
  f.calc_shift();

  pal = i4_pal_man.register_pal(&f);

  fake_screen=i4_create_image(m->xres, m->yres, pal);
    
  context=new i4_draw_context_class(0,0, m->xres-1, m->yres-1);
  context->both_dirty=new i4_rect_list_class;
  context->single_dirty=new i4_rect_list_class;

  memcpy(&cur_mode, m, sizeof(mode));

  return i4_T;
}

i4_image_class *i4_opengl_display_class::lock_frame_buffer(i4_frame_buffer_type type, i4_frame_access_type access)
{

  if (access == I4_FRAME_BUFFER_READ) {

	if (type == I4_FRONT_FRAME_BUFFER) {
	  i4_warning("attempting to read from front buffer");
	}
	else if (type == I4_BACK_FRAME_BUFFER) {

	  glPixelStorei(GL_PACK_ALIGNMENT,1);
	  glReadBuffer(GL_BACK);
	  glReadPixels(0,0,width(),height(),
	  			   GL_RGB,GL_UNSIGNED_SHORT_5_6_5,
	  			   (GLvoid *)back_screen->data);
	  //glReadPixels(width(),height(),0,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,(GLvoid*) back_screen->data);
	  //back_screen->bpl=0;
	  i4_image_class *tim=back_screen->copy();
	  //almost working: The middle row is missing. But is it the first or the last?
	  for (int i=0;i<back_screen->height();i++)
	  {
	  	memcpy(back_screen->data+(2*i*back_screen->width()),
			tim->data+(2*(back_screen->height()-i-1)*back_screen->width()),
			back_screen->width());
	  }
	  delete tim;
	  return back_screen;
	}
  }

  else if (access == I4_FRAME_BUFFER_WRITE) {

	if (type == I4_FRONT_FRAME_BUFFER)
	  i4_warning("attempting to write to front buffer");

	if (type == I4_BACK_FRAME_BUFFER)
	  i4_warning("attempting to write to back buffer");

  }

  return fake_screen;
}

void i4_opengl_display_class::unlock_frame_buffer(i4_frame_buffer_type type)
{
  return;
}

