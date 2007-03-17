/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "video/win32/dx5_util.h"
#include "video/win32/dx5_mouse.h"
#include "image/context.h"
#include "video/win32/dx5_error.h"

#include <ddraw.h>

dx5_mouse_class::dx5_mouse_class(i4_bool page_flipped)
	: page_flipped(page_flipped)
{
	cursor.pict=0;
	cursor.hot_x=0;
	cursor.hot_y=0;
	last.save_buffer=0;
	current.save_buffer=0;
	last_was_gdi=i4_T;

}

void dx5_mouse_class::set_cursor(i4_cursor_class *c)
{
	/*if (cursor.pict)
	   {
	   delete cursor.pict;
	   cursor.pict=0;
	   }*/

	if (c && c->pict)
	{
		int cw=c->pict->width(), ch=c->pict->height();

		i4_draw_context_class context(0,0, cw-1, ch-1);
		i4_dx5_image_class *dx5_image=(i4_dx5_image_class *)cursor.pict;
		if ((!cursor.pict)||(cw!=cursor.pict->width())||ch!=cursor.pict->height())
		{
			delete cursor.pict;
			cursor.pict=0;
			dx5_image = new i4_dx5_image_class(cw, ch, DX5_SYSTEM_RAM | DX5_CLEAR);
			//currently, it is not possible to use vram, as we'll lose the cursor on
			//DDERR_SURFACELOST
			cursor      = *c;
			cursor.pict = dx5_image;
		}

		//setup the color-key value for the mouse (black)
		DDCOLORKEY colorkey;
		memset(&colorkey,0,sizeof(DDCOLORKEY));

		//i4_color converted_transparent = c->pict->pal.pal->convert(c->trans,&dx5_common.i4_fmt_565);

		colorkey.dwColorSpaceLowValue  = 0; //converted_transparent;
		colorkey.dwColorSpaceHighValue = 0; //converted_transparent;

		dx5_image->surface->SetColorKey(DDCKEY_SRCBLT,&colorkey);
		short w=dx5_image->width()-1;
		short h=dx5_image->height()-1;
		i4_draw_context_class ctx(0,0,w,h);
		//all drawing functions, including the contexts always _include_
		//the rightmost pixel
		//so if the context and the requested operation is greater than the
		//image nothing is clipped away and we silently overwrite some
		//memory. (remove the -1 up here and see how the kernel likes you....)
		dx5_image->bar(0,0,w,h, 0,ctx); //change everything to black
		dx5_image->lock();
		c->pict->put_image_trans(cursor.pict, 0,0, c->trans, context);
		dx5_image->unlock();
	}
	else
	{
		delete cursor.pict;
		cursor.pict=0; //hide cursor
	}
	i4_bool g=primary_is_gdi();
	last_was_gdi=!g;
	current.isgdi=g;
	last.isgdi=!g;
}

dx5_mouse_class::~dx5_mouse_class()
{
	if (cursor.pict)
	{
		delete cursor.pict;
		cursor.pict=0;
	}
}

i4_bool dx5_mouse_class::primary_is_gdi()
{
	//i4_bool ret;
	LPDIRECTDRAWSURFACE lpgdi=0;
	LPDIRECTDRAWSURFACE3 lpgdi3=0;
	//LPSURFACEDESC lpsd=0;
	//DDSCAPS caps;
	//dx5_common.primary_surface->GetCaps(&caps);
	//if (caps.dwCaps&DDSCAPS_PRIMARYSURFACE)
	//	return i4_T;
	//else
	//	return i4_F;
	i4_dx5_check(dx5_common.ddraw->GetGDISurface(&lpgdi));
	i4_dx5_check(lpgdi->QueryInterface(IID_IDirectDrawSurface3,(void **)&lpgdi3));
	if (lpgdi)
	{
		lpgdi->Release();
	}
	if (dx5_common.primary_surface==lpgdi3)
	{
		if (lpgdi3)
		{
			lpgdi3->Release();
		}
		return i4_T;
	}
	else
	{
		if (lpgdi3)
		{
			lpgdi3->Release();
		}
		return i4_F;
	}
}

void dx5_mouse_class::save_and_draw(int x, int y)
{
	if (!cursor.pict)
	{
		return ;
	}
//  i4_bool g=primary_is_gdi();
//  if (page_flipped&&(g==last_was_gdi)) //current frame buffer same as last: something has happenened
//	  {
//
//	  }
//i4_warning("TRACE: Mouse first part");
	if (!current.save_buffer ||
		current.save_buffer->width() != cursor.pict->width() ||
		current.save_buffer->height() != cursor.pict->height())
	{
		if (current.save_buffer)
		{
			delete current.save_buffer;
		}

		current.save_buffer=new i4_dx5_image_class(cursor.pict->width(), cursor.pict->height(),
												   DX5_VRAM);
	}

	current.x=x - cursor.hot_x;
	current.y=y - cursor.hot_y;
	if (current.x<0)
	{
		current.x=0;
	}
	if (current.y<0)
	{
		current.y=0;
	}


	RECT src;
	i4_display_class *disp=i4_current_app->get_display();
	//PG: Not having these two conditions allows the cursor
	//to move all the way to the right or bottom of the screen,
	//but may cause trouble with the BltFast operation
	if (current.x+cursor.pict->width()>disp->width())
	{
		current.x=disp->width()-cursor.pict->width()-1;
	}
	if (current.y+cursor.pict->height()>disp->height())
	{
		current.y=disp->height()-cursor.pict->height()-1;
	}
	src.left   = current.x;
	src.right  = current.x + cursor.pict->width();
	src.top    = current.y;
	src.bottom = current.y + cursor.pict->height();
	HRESULT hr=0;
	//i4_warning("TRACE: Mouse second part");
	//no i4_dx5_check here as we allways get an error when the cursor is outside the window
	if (i4_dx5_check(hr=current.save_buffer->surface->BltFast(0,0, dx5_common.back_surface, &src,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT)))
	{
		//i4_warning("TRACE: Mouse third part");

		i4_dx5_check(dx5_common.back_surface->BltFast(current.x, current.y,
													  ((i4_dx5_image_class *)cursor.pict)->surface,0,
													  DDBLTFAST_SRCCOLORKEY|DDBLTFAST_WAIT));
	}
	else
	{
		if (current.save_buffer&&current.save_buffer->surface)
		{
			current.save_buffer->surface->Restore();
		}
		if (last.save_buffer&&last.save_buffer->surface)
		{
			last.save_buffer->surface->Restore();
		}
	}

	if (page_flipped)
	{
		save_struct tmp=current;
		current=last;
		last=tmp;
		tmp.save_buffer=0;
	}

}


void dx5_mouse_class::restore()
{

	if (current.save_buffer)
	{
		RECT src;
		src.left   = 0;
		src.right  = cursor.pict->width();
		src.top    = 0;
		src.bottom = cursor.pict->height();
		i4_bool g=primary_is_gdi();
		if (page_flipped&&(g==current.isgdi)) //current frame buffer same as last: something has happenened
		{
			dx5_common.primary_surface->BltFast(current.x,current.y,current.save_buffer->surface,&src,
												DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);
			current.isgdi=!current.isgdi;
			last.isgdi=!last.isgdi;

		}
		dx5_common.back_surface->BltFast(current.x, current.y, current.save_buffer->surface, &src,
										 DDBLTFAST_NOCOLORKEY);  //DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT
	}
}
