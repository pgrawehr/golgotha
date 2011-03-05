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
r1_software_dd_class r1_software_dd_class_instance;

static i4_profile_class pf_software_flush_spans("software::flush_spans");
static i4_profile_class pf_software_draw("software::draw()");
static i4_profile_class pf_software_blt("software::blt()");


void r1_software_dd_lock_render_buffer()
{
	if (r1_software_render_buffer_is_locked)
	{
		return;
	}

	D3DLOCKED_RECT locked_rect;
	memset(&locked_rect,0,sizeof(locked_rect));

	//if (r1_software_render_surface->IsLost()==DDERR_SURFACELOST)
	//{
	//	r1_software_render_surface->Restore(); //don't bother about lost contents
	//}

	i4_dx9_check(r1_software_render_surface->LockRect(&locked_rect,NULL,DDLOCK_WAIT | DDLOCK_NOSYSLOCK));

	r1_software_render_buffer_ptr       = (w16 *)locked_rect.pBits;
	r1_software_render_buffer_bpl       = locked_rect.Pitch;
	r1_software_render_buffer_wpl       = locked_rect.Pitch>>1;

	if (r1_software_render_expand_type==R1_COPY_1x1_SCANLINE_SKIP)
	{
		r1_software_render_buffer_wpl *= 2;
	}

	r1_software_render_buffer_is_locked = i4_T;
}

void r1_software_dd_unlock_render_buffer()
{
	if (!r1_software_render_buffer_is_locked)
	{
		return;
	}
	i4_dx9_check(r1_software_render_surface->UnlockRect());
	r1_software_render_buffer_is_locked = i4_F;
	r1_software_render_buffer_ptr=0; //just to get sure it makes bang on error.
};

r1_software_dd_class::r1_software_dd_class()
	: r1_software_class()
{
	current_width=0;
	current_height=0;
};

r1_software_gdi_class::r1_software_gdi_class()
	: r1_software_class()
{
	render_image=0;
};

r1_software_dd_class::~r1_software_dd_class()
{
	I4_ASSERT(r1_software_render_surface==0,"r1_software_dd_class: Destructor called before uninit()");
};

void r1_software_dd_class::uninit()
{
	if (r1_software_render_surface)
	{
		r1_software_render_surface->Release();
	}
	r1_software_render_surface=0;
	r1_software_class::uninit(); //don't forget this call...
}


i4_bool r1_software_dd_class::init(i4_display_class * display)
{
	if (display!=i4_dx9_display)
	{
		return i4_F;
	}
	r1_name_cache_file("dd_software");
	if (!r1_software_class::init(display))
	{
		return i4_F;
	}
	//when everything was fine till here, we create the off-screen
	//render target for the software renderer
	sw32 wi,hi;
	wi=display->width();
	hi=display->height();

	r1_software_render_surface = dx9_common.create_surface(DX9_SURFACE, wi,hi, 0);

	r1_software_render_buffer_height=hi;

	if (r1_software_render_surface==0)
	{
		//this cannot be an error, since we are probing devices, and
		//it's just exspected that some devices may fail to init.
		i4_warning("SEVERE: Unable to create surface for r1_software_dd_render_window_class");
		return i4_F;
	}

	DDBLTFX fx;
	memset(&fx,0,sizeof(DDBLTFX));
	fx.dwSize = sizeof(DDBLTFX);
	current_width=wi;
	current_height=hi;
	fx.dwFillColor = 0x0000;

	// TODO: GRAP
	// r1_software_render_surface->Blt(NULL,NULL,NULL,DDBLT_WAIT | DDBLT_COLORFILL,&fx);
	/*DDPIXELFORMAT ddpf;
	ZeroMemory(&ddpf,sizeof(ddpf));
	ddpf.dwSize=sizeof(ddpf);
	*/
	expand_depth=0;
	D3DSURFACE_DESC d3ddesc;
	memset(&d3ddesc,0,sizeof(d3ddesc));
	//r1_software_render_surface->GetPixelFormat(&ddpf);
	r1_software_render_surface->GetDesc(&d3ddesc);
	r1_software_render_buffer_height=d3ddesc.Height;
	if (d3ddesc.Format != D3DFORMAT::D3DFMT_R5G6B5)
	{
		i4_warning("Primary surface bitdepth is not 16. Performance "
				   "of software renderer might decrease significantly.");
		if (d3ddesc.Format==D3DFORMAT::D3DFMT_R8G8B8)
		{
			expand_depth=3;
		}
		else if (d3ddesc.Format==D3DFORMAT::D3DFMT_A8B8G8R8)
		{
			expand_depth=4;
		}
		else
		{
			i4_error("SEVERE: Primary display bitdepth of %i unsupported.",
					d3ddesc.Format);
		}
	}
	return i4_T;
}

i4_bool r1_software_dd_class::expand_type_supported(r1_expand_type type)
{
	// Assume any recent card can do that.
	return i4_T;
	/*if (type==R1_COPY_2x2)
	{
		D3DCAPS9 card_caps;
		memset(&card_caps, 0, sizeof(card_caps));

		dx9_common.device->GetDeviceCaps(&card_caps);

		if (card_caps.Caps & DDCAPS_BLTSTRETCH)
		{
			return i4_T;
		}
		else
		{
			return i4_F;
		}
	}

	if (type==R1_COPY_1x1 || type==R1_COPY_1x1_SCANLINE_SKIP)
	{
		return i4_T;
	}

	return i4_F;*/
}




r1_software_dd_render_window_class::r1_software_dd_render_window_class(
	w16 w, w16 h,r1_expand_type expand_type,
	r1_render_api_class * api)
	: r1_software_render_window_class(w,h,expand_type,api)
{
/*
   sw32 wi,hi;
   w32  flags;

   if (expand_type==R1_COPY_2x2)      // if stretching determine if driver support bltstretch
   {
   	DDCAPS card_caps, HELcaps;
   	memset(&card_caps, 0, sizeof(card_caps));
   	card_caps.dwSize=sizeof(DDCAPS);
   	memset(&HELcaps, 0, sizeof(HELcaps));
   	HELcaps.dwSize=sizeof(DDCAPS);
   	dx5_common.ddraw->GetCaps(&card_caps, &HELcaps);

   	if (card_caps.dwCaps & DDCAPS_BLTSTRETCH)
   	{
   	  flags = DX5_VRAM;
   	  wi = w>>1;
   	  hi = h>>1;
   	}
   	else
   	{
   	  flags = DX5_SYSTEM_RAM;
   	  wi = w>>1;
   	  hi = h>>1;
   	}
   }
   else
   {
   	wi = w;
   	hi = h;
   	flags = DX5_VRAM;
   }

   surface = dx5_common.create_surface(DX5_SURFACE, wi,hi, flags);

   if (surface==0)
   	  {
   	  i4_error("SEVERE: Unable to create surface for r1_software_dd_render_window_class");
   	  return;
   	  }

   DDBLTFX fx;
   memset(&fx,0,sizeof(DDBLTFX));
   fx.dwSize = sizeof(DDBLTFX);
   current_width=wi;
   current_height=hi;
   fx.dwFillColor = 0x0000;
   surface->Blt(NULL,NULL,NULL,DDBLT_WAIT | DDBLT_COLORFILL,&fx);
 */
}

r1_render_window_class * r1_software_dd_class::create_render_window(int w, int h,
																	r1_expand_type expand_type)
{
	r1_software_render_window_class * new_window;

	if (w & 1)
	{
		w++;
	}
	if (h & 1)
	{
		h++;
	}
	new_window = new r1_software_dd_render_window_class(w,h, expand_type, this);
	return new_window;
}


/*
   void r1_software_dd_render_window_class::resize(w16 new_width,w16 new_height)
   	{
   	if ((new_width<=current_width) && (new_height<=current_height))
   		{
   		r1_software_render_window_class::resize(new_width,new_height);
   		return;
   		}
   	if (surface)
   		surface->Release();
   	surface=0;
   	sw32 wi,hi;
   	w32  flags;

   	if (expand_type==R1_COPY_2x2)      // if stretching determine if driver support bltstretch
   		{
   		DDCAPS card_caps, HELcaps;
   		memset(&card_caps, 0, sizeof(card_caps));
   		card_caps.dwSize=sizeof(DDCAPS);
   		memset(&HELcaps, 0, sizeof(HELcaps));
   		HELcaps.dwSize=sizeof(DDCAPS);
   		dx5_common.ddraw->GetCaps(&card_caps, &HELcaps);

   		if (card_caps.dwCaps & DDCAPS_BLTSTRETCH)
   			{
   			//flags = DX5_VRAM;
   			flags=DX5_SYSTEM_RAM;
   			wi = w>>1;
   			hi = h>>1;
   			}
   		else
   			{
   			flags = DX5_SYSTEM_RAM;
   			wi = w>>1;
   			hi = h>>1;
   			}
   		}
   	else
   		{
   		wi = w;
   		hi = h;//don't try to use vram, when used mainly by the cpu, this is slow
   		flags = DX5_SYSTEM_RAM;
   		}
   	current_width=wi;
   	current_height=hi;
   	surface = dx5_common.create_surface(DX5_SURFACE, wi,hi, flags);

   	if (surface==0)
   		{
   		i4_error("SEVERE: Unable to create surface for r1_software_dd_render_window_class");
   		return;
   		}
   	r1_software_render_window_class::resize(new_width,new_height);
   	}
 */

i4_bool r1_software_dd_class::resize(w16 new_width,w16 new_height)
{
	if (new_width>current_width || new_height>current_height)
	{
		r1_software_class::resize(new_width,new_height);
		return i4_F; //todo: fix this if needed
	}
	r1_software_class::resize(new_width,new_height);
	return i4_T;
}



void r1_software_dd_class::copy_part(i4_image_class * im,
									 int x, int y,          // position on screen
									 int x1, int y1,        // area of image to copy
									 int x2, int y2)
{
	//this better be true


	RECT srcRect;

	srcRect.top     = y1;
	srcRect.bottom  = y2+1;
	srcRect.left    = x1;
	srcRect.right   = x2+1;

	i4_bool lock_check = r1_software_render_buffer_is_locked;

//  IDirectDrawSurface3 *temp_surf=0;
//  r1_software_render_surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&temp_surf);
//  switch (r1_software_render_expand_type)
//  {
//    case R1_COPY_1x1:
//    case R1_COPY_2x2:
//    case R1_COPY_1x1_SCANLINE_SKIP:
	if (lock_check)
	{
		r1_software_dd_unlock_render_buffer();
	}


	r1_software_render_surface->BltFast(x,
										y,
										dx5_common.get_surface(im),
										&srcRect,
										DDBLTFAST_NOCOLORKEY);

	if (lock_check)
	{
		r1_software_dd_lock_render_buffer();
	}
//    break;
//  }

//  temp_surf->Release();
}


void r1_software_dd_render_window_class::draw(i4_draw_context_class &context)
{
	/*
	   if (GetAsyncKeyState('L') & 1)
	   {
	   do_amd3d = !do_amd3d;

	   i4_cpu_info_struct s;
	   i4_get_cpu_info(&s);

	   	if ((s.cpu_flags & i4_cpu_info_struct::AMD3D)==0)
	   	do_amd3d = i4_F;

	   	  r1_software_class_instance.initialize_function_pointers(do_amd3d); //amd3d functions

	   		if (do_amd3d)
	   		i4_warning("amd3d optimizations on");
	   		else
	   		i4_warning("amd3d optimizations off");
	   		}
	 */
	pf_software_draw.start();

	//r1_software_render_surface = surface;
	//r1_software_render_expand_type = expand_type;

	//#ifdef DEBUG
	//  r1_software_class_instance.lock_cheat = i4_F;
	//#endif

	r1_software_dd_lock_render_buffer();

	if (r1_software_dd_class_instance.lock_cheat)
	{
		r1_software_dd_unlock_render_buffer();
	}

	r1_software_dd_class_instance.width_compare  = width();
	r1_software_dd_class_instance.height_compare = height();

	r1_software_dd_class_instance.modify_features(R1_PERSPECTIVE_CORRECT, 1);
	r1_software_dd_class_instance.modify_features(R1_SPANS, 1);

	clear_spans();

	r1_software_dd_class_instance.context = &context;
	r1_software_render_window_class::draw(context);
	r1_software_dd_class_instance.context = 0;

	//set this to false so that set_color_tint() actuall does work

	i4_bool spans_still_on = r1_software_dd_class_instance.is_feature_on(R1_SPANS);

	r1_software_dd_class_instance.modify_features(R1_SPANS, 0);

	//pf_software_dd_flush_spans.start();
	flush_spans();
	//pf_software_dd_flush_spans.stop();

	//clear this so that no funny business happens with leftover
	//color tint settings
	r1_software_dd_class_instance.set_color_tint(0);

	//restore whatever state it was in
	r1_software_dd_class_instance.modify_features(R1_SPANS, spans_still_on);

	//flush lines
	if (num_buffered_lines)
	{
		sw32 i;
		for (i=0; i<num_buffered_lines; i++)
		{
			r1_software_dd_class_instance.draw_line(software_lines[i].x0,
													software_lines[i].y0,
													software_lines[i].x1,
													software_lines[i].y1,
													software_lines[i].start_color);
		}
		num_buffered_lines=0;
	}

	//if (!r1_software_dd_class_instance.lock_cheat)
	//	r1_software_dd_unlock_render_buffer();

	RECT srcRect;
	RECT destRect;

	pf_software_blt.start();


	IDirectDrawSurface3 * surface3=r1_software_render_surface;
	//surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&surface3);

	i4_rect_list_class::area_iter cl;
	i4_rect_list_class * clip=&context.clip;

	if (r1_software_dd_class_instance.expand_depth>2)
	{
		DDSURFACEDESC ddsd;
		memset(&ddsd,0,sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		i4_dx5_check(dx5_common.back_surface->Lock(0,&ddsd,DDLOCK_WAIT,0));

		for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
		{
			//srcRect.left   = cl->x1;
			//srcRect.right  = cl->x2+1;
			//srcRect.top    = cl->y1;
			//srcRect.bottom = cl->y2+1;
			if (r1_software_dd_class_instance.expand_depth==4)
			{
				//Todo: Finish this.
				//this "hack" works almost. Requires:
				//(a) That the sky uses ordinary skydomes, not blit
				//(b) That some conversion in the other direction is done
				//    where the map is read back to main memory.
				//i4_dx5_check(dx5_common.back_surface->BltFast(context.xoff+cl->x1,context.yoff+cl->y1,
				//                             surface3,&srcRect,
				//                             DDBLTFAST_NOCOLORKEY));
				int ix,iy;
				w16 * src;
				w32 * dst;
				w32 srccol;
				for (iy=cl->y1; iy<=cl->y2; iy++)
				{
					//the last *4 on the next line is needed because the calcs
					//use byte offsets.
					dst=(w32 *)(((w8 *)ddsd.lpSurface)+ddsd.lPitch*(iy+context.yoff)+(context.xoff*4));
					src=(r1_software_render_buffer_ptr+r1_software_render_buffer_wpl*iy);
					for (ix=cl->x1; ix<=cl->x2; ix++,src++,dst++)
					{
						srccol=(w32)*src;
						srccol=(srccol&0xf800)<<8 | (srccol&0x07e0)<< 5 | (srccol&0x001f) << 3;
						*dst=srccol;
					}
				}
			}
			else if (r1_software_dd_class_instance.expand_depth==3)
			{
				H();
				i4_warning("Software DirectDraw renderer: Unimplemented blit required");
			}
		}
		i4_dx5_check(dx5_common.back_surface->Unlock(0));
		pf_software_blt.stop();
		pf_software_draw.stop();
		if (!r1_software_dd_class_instance.lock_cheat)
		{
			r1_software_dd_unlock_render_buffer();
		}
		return;
	}
	if (!r1_software_dd_class_instance.lock_cheat)
	{
		r1_software_dd_unlock_render_buffer();
	}
	//  if (surface3)  //always true.
	switch (expand_type)
	{
		case R1_COPY_1x1_SCANLINE_SKIP:
		case R1_COPY_1x1:
			for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
			{
				srcRect.left   = cl->x1;
				srcRect.right  = cl->x2+1;
				srcRect.top    = cl->y1;
				srcRect.bottom = cl->y2+1;

				i4_dx5_check(dx5_common.back_surface->BltFast(context.xoff+cl->x1,context.yoff+cl->y1,
															  surface3,&srcRect,
															  DDBLTFAST_NOCOLORKEY));
			}
			break;


		case R1_COPY_2x2:
			for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
			{
				srcRect.left   = cl->x1/2;
				srcRect.right  = (cl->x2+2)/2;
				srcRect.top    = cl->y1/2;
				srcRect.bottom = (cl->y2+2)/2;

				destRect.left   = context.xoff+cl->x1;
				destRect.top    = context.yoff+cl->y1;
				destRect.right  = (context.xoff+cl->x2+1)&(~1);
				destRect.bottom = (context.yoff+cl->y2+1)&(~1);

				i4_dx5_check(dx5_common.back_surface->Blt(&destRect, surface3, &srcRect, DDBLT_ASYNC | DDBLT_WAIT, NULL));
			}
			break;
	}
	//surface3->Release();

	pf_software_blt.stop();

	//r1_software_render_surface = 0;
	pf_software_draw.stop();
}

r1_software_dd_render_window_class::~r1_software_dd_render_window_class()
{

}


void r1_software_dd_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
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
		RECT dst;
		dst.left=x1; // + context->xoff;
		dst.right=x2 /* + context->xoff*/+1;
		dst.top=y1; // + context->xoff;
		dst.bottom=y2 /* + context->yoff*/+1;

		DDBLTFX fx;
		memset(&fx,0,sizeof(DDBLTFX));
		fx.dwSize = sizeof(DDBLTFX);

		fx.dwFillColor = color;

		i4_bool lock_check = r1_software_render_buffer_is_locked;

		if (lock_check)
		{
			r1_software_dd_unlock_render_buffer();
		}

		r1_software_render_surface->Blt(&dst,NULL,NULL,DDBLT_WAIT | DDBLT_COLORFILL,&fx);

		if (lock_check)
		{
			r1_software_dd_lock_render_buffer();
		}
	}

}

i4_image_class * r1_software_dd_class::create_compatible_image(w16 w, w16 h)
{
	i4_image_class * a = dx5_common.create_image(w,h, DX5_VRAM);

	if (!dx5_common.get_surface(a))
	{
		delete a;
		a = dx5_common.create_image(w,h, DX5_SYSTEM_RAM);
		if (!dx5_common.get_surface(a))
		{
			delete a;
			return NULL;
		}
	}

	return a;
}


class r1_software_win_dd_uniniter_class :
	public i4_init_class
{
public:
	r1_software_win_dd_uniniter_class()
	{
	};
	virtual void init()
	{
		i4_init_class::init();
	}
	virtual void uninit()
	{
		r1_software_dd_class_instance.uninit();
		i4_init_class::uninit();
	}
}
r1_software_win_dd_uniniter_class_instance;

