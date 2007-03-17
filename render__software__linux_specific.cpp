#include "pch.h"

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


#include <math.h>

#include "render/r1_clip.h"
#include "render/r1_win.h"
#include "render/r1_res.h"

#include "device/processor.h"

#include "render/software/r1_software.h"
#include "render/software/r1_software_texture.h"
#include "render/software/r1_software_globals.h"
#include "render/software/span_buffer.h"
#include "render/software/inline_fpu.h"
#include "render/software/mappers.h"
#include "render/software/linux_specific.h"
#include "video/x11/x11_display.h"

#include "time/profile.h"

//static i4_profile_class pf_software_flush_spans("software::flush_spans");
static i4_profile_class pf_software_draw("software::draw()");
static i4_profile_class pf_software_blt("software::blt()");

r1_software_x11_class r1_software_x11_class_instance;

r1_software_x11_class::r1_software_x11_class()
{
	render_image=0;
}

r1_software_x11_class::~r1_software_x11_class()
{
	I4_ASSERT(render_image==0,"r1_software_x11_class: Destructor called before uninit()");
	render_image=0;
};

void r1_software_x11_class::uninit()
{
	r1_software_render_buffer_ptr=0;
	r1_software_render_buffer_is_locked=i4_F; //not available
	//(the renderer won't try to render to a non-locked surface)
	delete render_image;
	render_image=0;
	r1_software_class::uninit();
}

i4_bool r1_software_x11_class::init(i4_display_class *display)
{

	//The definition of win32_display_class is not available here,
	//therefore the compiler doesn't know that they are related.
	if (display!=(i4_display_class *)x11_display_ptr)
	{
		return i4_F;
	}
	r1_name_cache_file("x11_software");
	if (!r1_software_class::init(display))
	{
		return i4_F;
	}
	if (render_image)
	{
		//this should not have been initialized already...
		delete render_image;
	}
	render_image=i4_create_image(display->width(),display->height(),display->get_palette());
	render_image_data=(w16 *)render_image->data;
	r1_software_render_buffer_ptr=render_image_data;
	r1_software_render_buffer_bpl=render_image->bpl;
	r1_software_render_buffer_wpl=render_image->bpl>>1;
	r1_software_render_buffer_height=render_image->height();
	r1_software_render_buffer_is_locked=i4_T; //always available;

	return i4_T;
}


i4_bool r1_software_x11_class::expand_type_supported(r1_expand_type type)
{
	// to simplify, we'll demand 1x1 for a start.

	if (type==R1_COPY_1x1)
	{
		return i4_T;
	}

	return i4_F;
}





r1_software_x11_render_window_class::r1_software_x11_render_window_class(
	w16 w, w16 h,r1_expand_type expand_type,
	r1_render_api_class *api)
	: r1_software_render_window_class(w,h,expand_type,api)
{
	// reference to backing store
	// glibc-2.1.1 dies with the 'new' and 'delete' (see also destructor)
	//backing_store = (w16*) i4_malloc(sizeof(w16) * w * h, "");
	//backing_store = new w16[ w * h ];
	// We will allocate the memory later, so no problems with buggy glibc's.
	// And btw: Those bugs should have been fixed in current releases.
}

r1_software_x11_render_window_class::~r1_software_x11_render_window_class()
{
	//nothing to do
}
r1_render_window_class *r1_software_x11_class::create_render_window(int w, int h,
																	r1_expand_type expand_type)
{
	r1_render_window_class *new_window;
	if (w & 1)
	{
		w++;
	}
	if (h & 1)
	{
		h++;
	}
	new_window = new r1_software_x11_render_window_class(w,h, expand_type, this);
	return new_window;
}

/*
   void r1_software_render_window_class::receive_event(i4_event *ev)
   {
   if (expand_type==R1_COPY_2x2 && ev->type()==i4_event::MOUSE_MOVE)
   {
   	CAST_PTR(mev, i4_mouse_move_event_class, ev);

   	int o_lx=mev->lx, o_ly=mev->ly, o_x=mev->x, o_y=mev->y;

   	mev->lx/=2;
   	mev->ly/=2;
   	mev->x/=2;
   	mev->y/=2;

   	i4_parent_window_class::receive_event(ev);

   	mev->lx=o_lx;
   	mev->ly=o_ly;
   	mev->x=o_x;
   	mev->y=o_y;
   }
   else i4_parent_window_class::receive_event(ev);
   }
 */

void r1_software_x11_class::copy_part(i4_image_class *im,
									  int x, int y,         // position on screen
									  int x1, int y1,       // area of image to copy
									  int x2, int y2)
{
	// copy a segment of of the image to the display.
	i4_draw_context_class ctx(0,0,render_image->width(),render_image->height());
	im->put_part(render_image,x,y,x1,y1,x2,y2,ctx);
}

void r1_software_x11_render_window_class::draw(i4_draw_context_class &context)
{
	pf_software_draw.start();

	//r1_software_render_surface = surface;
	r1_software_render_expand_type = expand_type;


	r1_software_x11_class_instance.width_compare  = width();
	r1_software_x11_class_instance.height_compare = height();

	r1_software_x11_class_instance.modify_features(R1_PERSPECTIVE_CORRECT, 1);
	r1_software_x11_class_instance.modify_features(R1_SPANS, 1);

	clear_spans();

	r1_software_x11_class_instance.context = &context;
	r1_software_render_window_class::draw(context);
	r1_software_x11_class_instance.context = 0;

	//set this to false so that set_color_tint() actuall does work
	i4_bool spans_still_on = r1_software_x11_class_instance.is_feature_on(R1_SPANS);
	r1_software_x11_class_instance.modify_features(R1_SPANS, 0);
	flush_spans(); //I think this will flush the polygons to the software render buffer
	//clear this so that no funny business happens with leftover
	//color tint settings
	r1_software_x11_class_instance.set_color_tint(0);
	//restore whatever state it was in
	r1_software_x11_class_instance.modify_features(R1_SPANS, spans_still_on);
	//flush lines
	if (num_buffered_lines)
	{
		sw32 i;
		for (i=0; i<num_buffered_lines; i++)
		{
			r1_software_x11_class_instance.draw_line(software_lines[i].x0,
													 software_lines[i].y0,
													 software_lines[i].x1,
													 software_lines[i].y1,
													 software_lines[i].start_color);
		}
		num_buffered_lines=0;
	}
	pf_software_blt.start();
	i4_rect_list_class::area_iter cl;
	i4_rect_list_class *clip=&context.clip;
	switch (expand_type)
	{
		case R1_COPY_1x1_SCANLINE_SKIP:
		case R1_COPY_1x1:

			r1_software_x11_class_instance.render_image->put_part(local_image,
																  0,0,
																  0,0,
																  r1_software_x11_class_instance.render_image->width(),
																  r1_software_x11_class_instance.render_image->height(),
																  context);
			//}
			break;
		case R1_COPY_2x2:

			for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
			{
				r1_software_x11_class_instance.render_image->scale_image(local_image,
																		 cl->x2 - cl->x1, cl->y2 - cl->y1,0);
			}
			break;
	}
	pf_software_blt.stop();
	pf_software_draw.stop();
};

/*
   void r1_software_render_window_class::draw(i4_draw_context_class &context)
   {
   i4_window_class* p = parent;
   while ( p->get_parent() ) p = p->get_parent();

   int pwid = p->width();
   local_image->add_dirty(0,0,width()-1,height()-1,context);
   int bpp = 2; // FIX FIX FIX! 16-bit dependency
   r1_software_render_buffer_ptr       = backing_store;
   r1_software_render_buffer_bpl       = bpp * width();
   r1_software_render_buffer_wpl       = width();

   r1_software_render_buffer_height       = height();

   // start draw profiler
   pf_software_draw.start();

   r1_software_render_expand_type = expand_type;

   r1_software_class_instance.width_compare  = width();
   r1_software_class_instance.height_compare = height();

   r1_software_class_instance.modify_features(R1_PERSPECTIVE_CORRECT, 1);
   r1_software_class_instance.modify_features(R1_SPANS, 1);

   clear_spans();

   r1_software_class_instance.context = &context;
   r1_software_class_instance.context = 0;

   //set this to false so that set_color_tint() actuall does work

   i4_bool spans_still_on = r1_software_class_instance.is_feature_on(R1_SPANS);

   r1_software_class_instance.modify_features(R1_SPANS, 0);

   r1_render_window_class::draw(context);

   // start flush_span profiler
   pf_software_flush_spans.start();
   flush_spans();
   // stop flush_span profiler
   pf_software_flush_spans.stop();

   local_image->add_dirty(0,0,width()-1,height()-1,context);

   //clear this so that no funny business happens with leftover
   //color tint settings
   r1_software_class_instance.set_color_tint(0);

   //restore whatever state it was in
   r1_software_class_instance.modify_features(R1_SPANS, spans_still_on);

   //flush lines
   if (num_buffered_lines)
   {
   	sw32 i;
   	for (i=0;i<num_buffered_lines;i++)
   	{
   	  r1_software_class_instance.draw_line(software_lines[i].x0,
   										   software_lines[i].y0,
   										   software_lines[i].x1,
   										   software_lines[i].y1,
   										   software_lines[i].start_color);
   	}
   	num_buffered_lines=0;
   }

   pf_software_blt.start();
   // Bacon, Lettuce, and Tomato the image to screen
   i4_rect_list_class::area_iter cl;
   i4_rect_list_class *clip=&context.clip;

   for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl) {
   	w16* target = (w16 *)global_image->data + x() + cl->x1 + (y()+cl->y1)*pwid;
   	w16* source = backing_store + cl->x1 + (cl->y1 * width());
   	int clwid = (cl->x2 - cl->x1) + 1;
   	for ( int ln = cl->y1; ln <= cl->y2; ln++ ) {
   	  memcpy( target, source, clwid * 2); // 16-bit dep
   	  target += pwid;
   	  source += width();
   	}
   }
   pf_software_blt.stop();

   // stop draw profiler
   pf_software_draw.stop();
   }
 */

void r1_software_x11_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
{
	if ((write_mask & R1_WRITE_COLOR) == 0)
	{
		return;
	}

	if (write_mask & R1_COMPARE_W) // need to use spans to do this
	{
		w32 old_const_color = get_constant_color();
		r1_alpha_type old_alpha_mode  = get_alpha_mode();
		r1_shading_type old_shade_mode  = get_shade_mode();

		set_shading_mode(R1_CONSTANT_SHADING);
		set_alpha_mode(R1_ALPHA_DISABLED);

		set_constant_color(color);

		disable_texture();

		r1_vert v[4];

		z -= 0.5;
		if (z<r1_near_clip_z)
		{
			z = r1_near_clip_z;
		}

		v[3].px=x1;
		v[3].py=y1;
		v[3].v.z = z;
		v[3].w = 1/z;
		v[2].px=x2+1;
		v[2].py=y1;
		v[2].v.z = z;
		v[2].w = 1/z;
		v[1].px=x2+1;
		v[1].py=y2+1;
		v[1].v.z = z;
		v[1].w = 1/z;
		v[0].px=x1;
		v[0].py=y2+1;
		v[0].v.z = z;
		v[0].w = 1/z;

		render_sprite(v);

		set_shading_mode(old_shade_mode);
		set_constant_color(old_const_color);
		set_alpha_mode(old_alpha_mode);
	}
	else
	{
		// clear
		i4_draw_context_class ctx(x1,y1,x2,y2);
		render_image->clear(color,ctx);
	}

}

i4_image_class *r1_software_x11_class::create_compatible_image(w16 w, w16 h)
{
	// generate a new X pixmap of dim. wxh
	return i4_create_image(w,h,i4_pal_man.register_pal(&fmt));
}

i4_bool r1_software_x11_class::resize(w16 new_width,w16 new_height)
{
	if (new_width>render_image->width() || new_height>render_image->height())
	{
		delete render_image;
		render_image=i4_create_image(new_width,new_height,i4_pal_man.register_pal(&fmt));
		render_image_data=(w16 *)render_image->data;
		r1_software_render_buffer_ptr=render_image_data;
		r1_software_render_buffer_bpl=render_image->bpl;
		r1_software_render_buffer_wpl=render_image->bpl>>1;
		r1_software_render_buffer_height=render_image->height();
		r1_software_render_buffer_is_locked=i4_T; //always available;
	}
	r1_software_class::resize(new_width,new_height);
	return i4_T;
}


class r1_software_x11_uniniter_class :
	public i4_init_class
{
public:
	void uninit()
	{
		r1_software_x11_class_instance.uninit();
	}
} r1_software_x11_uniniter_instance;
