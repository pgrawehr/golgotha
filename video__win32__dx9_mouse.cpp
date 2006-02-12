/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "video/win32/dx9_util.h"
#include "video/win32/dx9_mouse.h"
#include "image/context.h"
#include "video/win32/dx9_error.h"

#include <ddraw.h>

dx9_mouse_class::dx9_mouse_class(i4_bool page_flipped)
  : page_flipped(page_flipped)
{
  cursor.pict=0;
  cursor.hot_x=0;
  cursor.hot_y=0;
  last.save_buffer=0;
  current.save_buffer=0;

}

void dx9_mouse_class::set_cursor(i4_cursor_class *c)
{
  

  if (c && c->pict)
  {
    int cw=c->pict->width(), ch=c->pict->height();

    i4_draw_context_class context(0,0, cw-1, ch-1);
    i4_dx9_image_class *dx9_image=(i4_dx9_image_class*)cursor.pict;
	if ((!cursor.pict)||(cw!=cursor.pict->width())||ch!=cursor.pict->height())
		{
		delete cursor.pict;
		cursor.pict=0;
		dx9_image = new i4_dx9_image_class(cw, ch, DX9_SYSMEM);
		//currently, it is not possible to use vram, as we'll lose the cursor on
		//DDERR_SURFACELOST
		
		}
    cursor      = *c;
	cursor.pict = dx9_image;
    //setup the color-key value for the mouse (black)
    //DDCOLORKEY colorkey;
    //memset(&colorkey,0,sizeof(DDCOLORKEY));

    //i4_color converted_transparent = c->pict->pal.pal->convert(c->trans,&dx9_common.i4_fmt_565);

    //colorkey.dwColorSpaceLowValue  = 0;//converted_transparent;
    //colorkey.dwColorSpaceHighValue = 0;//converted_transparent;

    //dx9_image->surface->SetColorKey(DDCKEY_SRCBLT,&colorkey);
	short w=dx9_image->width()-1;
	short h=dx9_image->height()-1;
    i4_draw_context_class ctx(0,0,w,h);
	//all drawing functions, including the contexts always _include_
	//the rightmost pixel
	//so if the context and the requested operation is greater than the 
	//image nothing is clipped away and we silently overwrite some
	//memory. (remove the -1 up here and see how the kernel likes you....)
	dx9_image->bar(0,0,w,h,	0,ctx); //change everything to black
    dx9_image->lock();
    c->pict->put_image_trans(cursor.pict, 0,0, c->trans, context);
    dx9_image->unlock();    
  }
  else 
	  {
	  delete cursor.pict;
	  cursor.pict=0;//hide cursor
	  }

}

dx9_mouse_class::~dx9_mouse_class()
{
  if (cursor.pict)
  {
    delete cursor.pict;
    cursor.pict=0;
  }
}


void dx9_mouse_class::save_and_draw(int x, int y)
{
  if (!cursor.pict) return ;
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
      delete current.save_buffer;

    current.save_buffer=new i4_dx9_image_class(cursor.pict->width(), cursor.pict->height(),
                                               DX9_SYSMEM);
  }

  current.x=x - cursor.hot_x;  current.y=y - cursor.hot_y;
  if (current.x<0) current.x=0;
  if (current.y<0) current.y=0;
  

  RECT src;
  i4_display_class *disp=i4_current_app->get_display();
  //PG: Not having these two conditions allows the cursor
  //to move all the way to the right or bottom of the screen,
  //but may cause trouble with the BltFast operation
  w16 cwidth=cursor.pict->width();
  w16 cheight=cursor.pict->height();
  if (current.x+cwidth>disp->width())
	  {
	  current.x=disp->width()-cwidth-1;
	  }
  if (current.y+cheight>disp->height())
	  {
	  current.y=disp->height()-cheight-1;
	  }
  src.left   = current.x;
  src.right  = current.x + cwidth;
  src.top    = current.y;
  src.bottom = current.y + cheight;
  int sfcby=2;
  if (cursor.pict->get_pal()->source.pixel_depth==I4_24BIT)
	  sfcby=3;
  if (cursor.pict->get_pal()->source.pixel_depth==I4_32BIT)
	  sfcby=4;
  HRESULT hr=0;
  //i4_warning("TRACE: Mouse second part");
  //no i4_dx9_check here as we allways get an error when the cursor is outside the window
  //TODO: This doesn't work...
  D3DLOCKED_RECT lock;
  i4_dx9_check(dx9_common.back_surface->LockRect(&lock,&src,D3DLOCK_NOSYSLOCK));
  
  int linelen=lock.Pitch;
  current.save_buffer->lock();
  //Step 1: Copy backbuffer to savebuffer
  int yp,xp;
  
  for (yp=0;yp<cheight;yp++)
  {
	  memcpy(current.save_buffer->data+yp*sfcby*cwidth,
		  yp*linelen+(w8*)lock.pBits,
		  sfcby*cwidth);
  }
  current.save_buffer->unlock();
  //Step 2: Copy Cursor to backbuffer
  int cbpl=cursor.pict->bpl;
  for (yp=0;yp<cheight;yp++)
  {
	  w16 c16;
	  w32 c32;
	  w16 *d,*s;
	  w32 *d2,*s2;
	  if (sfcby==2)
	  {
		  for (xp=0;xp<cwidth;xp++)
		  {
		    s=(w16*)(((w8*)cursor.pict->data)+(xp*2+yp*cbpl));
			//c16=*(((w16*)cursor.pict->data)+xp+yp*(cbpl/2));
			c16=*s;
			//c16=cursor.pict->get_pixel(x,y);
			d=(w16*)(((w8*)lock.pBits)+(linelen*yp+xp*2));
			if (c16!=0)
				*d=c16;
		  }
	  }
	  if (sfcby==4) //3 is probably not possible with dx9
	  {
		  for (xp=0;xp<cwidth;xp++)
		  {
            
			//c32=*(((w32*)cursor.pict->data)+(xp+yp*cbpl));
			//d2=(w32*)(((w8*)lock.pBits)+(linelen*yp+xp*4));
            s2=(w32*)(((w8*)cursor.pict->data)+(xp*4+yp*cbpl));
            c32=*s2;
            d2=(w32*)(((w8*)lock.pBits)+(linelen*yp+xp*4));
			if ((c32&0xffffff)!=0)
				*d2=c32;
		  }
	  }
  }
  dx9_common.back_surface->UnlockRect();
  if (page_flipped)
  {
    save_struct tmp=current;
    current=last;
    last=tmp;
    tmp.save_buffer=0;  
  }

}


void dx9_mouse_class::restore()
{  

	if (current.save_buffer)
	{
		RECT src;
		src.left   = current.x;
		src.right  = current.x+cursor.pict->width();
		src.top    = current.y;
		src.bottom = current.y+cursor.pict->height();

		D3DLOCKED_RECT lock;
		dx9_common.back_surface->LockRect(&lock,&src,D3DLOCK_NOSYSLOCK);
		int sfcby=2;
		if (cursor.pict->get_pal()->source.pixel_depth==I4_24BIT)
			sfcby=3;
		if (cursor.pict->get_pal()->source.pixel_depth==I4_32BIT)
			sfcby=4;
		int linelen=lock.Pitch;
		current.save_buffer->lock();
		//Step 3: copy savebuffer back to the backbuffer
		int yp;
		for (yp=0;yp<cursor.pict->height();yp++)
		{
			w8* dest=yp*linelen+(w8*)lock.pBits;
			int bytes=sfcby*cursor.pict->width();
			w8* src=current.save_buffer->data+yp*bytes;
			memcpy(dest,src,bytes);
		}
		//This code is for testing only. 
		/*for (int yp=0;yp<cursor.pict->height();yp++)
		{
			for (int xp=0;xp<cursor.pict->width();xp++)
			{
				w8* p=yp*linelen+xp*sfcby+(w8*)lock.pBits;
				w32* p32=(w32*)p;
				*p32=0x00FF00FF;
			}
		}*/
		current.save_buffer->unlock();
		dx9_common.back_surface->UnlockRect();
	}
}

void dx9_mouse_class::add_dirty_area(i4_draw_context_class* context)
{
	int mouse_x=current.x;
	int mouse_y=current.y;
	context->both_dirty->add_area(mouse_x,mouse_y,
		mouse_x+cursor.pict->width(),mouse_y+cursor.pict->height());
	i4_rect_list_class::area_iter cl;
	for (cl = context->render_area->list.begin(); cl!= context->render_area->list.end(); ++cl)
	{
		context->both_dirty->remove_area(cl->x1,cl->y1,cl->x2,cl->y2);
	}
}
