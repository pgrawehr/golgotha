#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//This file supports the really weird option of using a dx5 display
//together with software rendering.
#include <windows.h>
#include <ddraw.h>
#include <math.h>
#include <memory.h>

#include "render/r1_clip.h"
#include "render/r1_win.h"
#include "render/r1_res.h"

#include "device/processor.h"

#include "render/software/r1_software.h"
#include "render/software/r1_software_texture.h"
#include "render/software/r1_software_globals.h"
#include "render/software/mappers.h"
#include "render/software/span_buffer.h"
#include "render/software/inline_fpu.h"
#include "render/software/win32_specific.h"

#include "video/win32/dx9.h"
#include "video/win32/dx9_error.h"
#include "video/win32/dx9_util.h"
#include "video/win32/win32_input.h"

#include "time/profile.h"

//The actual instances of the render devices.
r1_software_gdi_class r1_software_gdi_class_instance;

static i4_profile_class pf_software_flush_spans("software::flush_spans");
static i4_profile_class pf_software_draw("software::draw()");
static i4_profile_class pf_software_blt("software::blt()");


r1_software_gdi_class::r1_software_gdi_class()
	: r1_software_class()
{
	render_image=0;
};


r1_software_gdi_class::~r1_software_gdi_class()
{
	I4_ASSERT(render_image==0,"r1_software_gdi_class: Destructor called before uninit()");
	render_image=0;
};

i4_bool r1_software_gdi_class::init(i4_display_class * display)
{
	//The definition of win32_display_class is not available here,
	//therefore the compiler doesn't know that they are related.
	if (display!=(i4_display_class *)win32_display_ptr)
	{
		return i4_F;
	}
	r1_name_cache_file("gdi_software");
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

void r1_software_gdi_class::uninit()
{
	r1_software_render_buffer_ptr=0;
	r1_software_render_buffer_is_locked=i4_F; //no more available.
	delete render_image;
	render_image=0;
	r1_software_class::uninit();
}



i4_bool r1_software_gdi_class::expand_type_supported(r1_expand_type type)
{
	//GDI can do stretch-blt, but I dono wheter the flush operation
	//supports it. And it's slow.
	return i4_T;
}


r1_software_gdi_render_window_class::r1_software_gdi_render_window_class(
	w16 w, w16 h, r1_expand_type expand_type, r1_render_api_class * api)
	: r1_software_render_window_class(w,h,expand_type,api)
{
	//render target will be local_image, so it seems we don't need
	//any additional space.
	//i4_draw_context_class ctx(0,0,local_image->width(),local_image->height());
	//local_image->clear(0,ctx);
}

r1_software_gdi_render_window_class::~r1_software_gdi_render_window_class()
{

}


r1_render_window_class * r1_software_gdi_class::create_render_window(int w,
																	 int h,
																	 r1_expand_type expand_type)
{
	r1_software_render_window_class * new_window;

	if (w&1)
	{
		w++;
	}
	if (h&1)
	{
		h++;
	}
	new_window=new r1_software_gdi_render_window_class(w,h,expand_type,this);
	return new_window;
}


i4_bool r1_software_gdi_class::resize(w16 new_width,w16 new_height)
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

void r1_software_gdi_class::copy_part(i4_image_class * im,
									  int x, int y, int x1,
									  int y1, int x2, int y2)
{
	i4_draw_context_class ctx(0,0,render_image->width(),render_image->height());
	im->put_part(render_image,x,y,x1,y1,x2,y2,ctx);
}

void r1_software_gdi_render_window_class::draw(i4_draw_context_class &context)
{
	pf_software_draw.start();

	//r1_software_render_surface = surface;
	r1_software_render_expand_type = expand_type;


	r1_software_gdi_class_instance.width_compare  = width();
	r1_software_gdi_class_instance.height_compare = height();

	r1_software_gdi_class_instance.modify_features(R1_PERSPECTIVE_CORRECT, 1);
	r1_software_gdi_class_instance.modify_features(R1_SPANS, 1);

	clear_spans();

	r1_software_dd_class_instance.context = &context;
	r1_software_render_window_class::draw(context);
	r1_software_dd_class_instance.context = 0;

	//set this to false so that set_color_tint() actuall does work

	i4_bool spans_still_on = r1_software_gdi_class_instance.is_feature_on(R1_SPANS);

	r1_software_gdi_class_instance.modify_features(R1_SPANS, 0);

	//pf_software_dd_flush_spans.start();
	flush_spans(); //I think this will flush the polygons to the software render buffer
	//pf_software_dd_flush_spans.stop();

	//clear this so that no funny business happens with leftover
	//color tint settings
	r1_software_gdi_class_instance.set_color_tint(0);

	//restore whatever state it was in
	r1_software_gdi_class_instance.modify_features(R1_SPANS, spans_still_on);

	//flush lines
	if (num_buffered_lines)
	{
		sw32 i;
		for (i=0; i<num_buffered_lines; i++)
		{
			r1_software_gdi_class_instance.draw_line(software_lines[i].x0,
													 software_lines[i].y0,
													 software_lines[i].x1,
													 software_lines[i].y1,
													 software_lines[i].start_color);
		}
		num_buffered_lines=0;
	}

	//if (!r1_software_dd_class_instance.lock_cheat)
	///  r1_software_dd_unlock_render_buffer();

	//RECT		srcRect;
	//RECT		destRect;

	pf_software_blt.start();

	//IDirectDrawSurface3 *surface3=0;
	//surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&surface3);

	i4_rect_list_class::area_iter cl;
	i4_rect_list_class * clip=&context.clip;

	//if (surface3)
	switch (expand_type)
	{
		case R1_COPY_1x1_SCANLINE_SKIP:
		case R1_COPY_1x1:
			//for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
			//{
			//srcRect.left   = cl->x1;
			//srcRect.right  = cl->x2+1;
			//srcRect.top    = cl->y1;
			//srcRect.bottom = cl->y2+1;

			//dx5_common.back_surface->BltFast(context.xoff+cl->x1,context.yoff+cl->y1,
			//                                 surface3,&srcRect,
			//                                 DDBLTFAST_NOCOLORKEY);
			r1_software_gdi_class_instance.render_image->put_part(local_image,
																  0,0,
																  0,0,
																  r1_software_gdi_class_instance.render_image->width(),
																  r1_software_gdi_class_instance.render_image->height(),
																  context);
			//}
			break;


		case R1_COPY_2x2:

			for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
			{
				/*
				   srcRect.left   = cl->x1/2;
				   srcRect.right  = (cl->x2+2)/2;
				   srcRect.top    = cl->y1/2;
				   srcRect.bottom = (cl->y2+2)/2;

				   destRect.left   = context.xoff+cl->x1;
				   destRect.top    = context.yoff+cl->y1;
				   destRect.right  = (context.xoff+cl->x2+1)&(~1);
				   destRect.bottom = (context.yoff+cl->y2+1)&(~1);
				 */
				r1_software_gdi_class_instance.render_image->scale_image(local_image,
																		 cl->x2 - cl->x1, cl->y2 - cl->y1,0);
				//dx5_common.back_surface->Blt(&destRect, surface3, &srcRect, DDBLT_ASYNC | DDBLT_WAIT, NULL);
			}
			break;
	}
	//surface3->Release();

	pf_software_blt.stop();

	//r1_software_render_surface = 0;
	pf_software_draw.stop();
}


void r1_software_gdi_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
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

		v[3].px=(float)x1;
		v[3].py=(float)y1;
		v[3].v.z = z;
		v[3].w = 1/z;
		v[2].px=(float)x2+1;
		v[2].py=(float)y1;
		v[2].v.z = z;
		v[2].w = 1/z;
		v[1].px=(float)x2+1;
		v[1].py=(float)y2+1;
		v[1].v.z = z;
		v[1].w = 1/z;
		v[0].px=(float)x1;
		v[0].py=(float)y2+1;
		v[0].v.z = z;
		v[0].w = 1/z;

		render_sprite(v);

		set_shading_mode(old_shade_mode);
		set_constant_color(old_const_color);
		set_alpha_mode(old_alpha_mode);
	}
	else
	{
		i4_draw_context_class ctx(x1,y1,x2,y2);
		//local_image->clear(color,ctx);
		render_image->clear(color,ctx);
	}
}


i4_image_class * r1_software_gdi_class::create_compatible_image(w16 w, w16 h)
{
	return i4_create_image(w,h,i4_pal_man.register_pal(&fmt));
};


class r1_software_win_uniniter_class :
	public i4_init_class
{
public:
	r1_software_win_uniniter_class()
	{
	};
	virtual void init()
	{
		i4_init_class::init();
	}
	virtual void uninit()
	{
		r1_software_gdi_class_instance.uninit();
		i4_init_class::uninit();
	}
}
r1_software_uniniter_class_instance;
