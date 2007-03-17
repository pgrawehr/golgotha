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


#include "image/image.h"
#include "error/error.h"
#include "memory/malloc.h"
#include "image/context.h"
#include <memory.h>

inline void argb_split(w32 c, int &a, int &r, int &g, int &b)
{
	a=(c&0xff000000)>>24;
	r=(c&0xff0000)>>16;
	g=(c&0xff00)>>8;
	b=(c&0xff)>>0;
}

inline w32 pack_argb(int a, int r, int g, int b)
{
	return (a<<24) | (r<<16) | (g<<8) | b;
}

void i4_dither_quantize(i4_image_class *source, i4_image_class *dest)
{
	int w=source->width(), h=source->height(), x,y;

	I4_ASSERT(source && dest && w==dest->width() && h==dest->height(), "source size !=dest");

	int size=(w+1)*(h+1)*4;
	w32 *er_im=(w32 *)I4_MALLOC(size,"");
	memset(er_im, 0, size);
	w32 *cur_er=er_im;

	i4_draw_context_class context(0,0, w-1, h-1);
	for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++, cur_er++)
		{
			w32 c=source->get_pixel(x,y, context);
			int r,g,b,a, ea,er,eg,eb;

			argb_split(c, a,r,g,b);
			argb_split(*cur_er, ea, er, eg,eb);

			ea-=127;
			er-=127;
			eg-=127;
			eb-=127;

			a+=ea;
			if (a<0)
			{
				a=0;
			}
			if (a>255)
			{
				a=255;
			}
			r+=er;
			if (r<0)
			{
				r=0;
			}
			if (r>255)
			{
				r=255;
			}
			g+=eg;
			if (g<0)
			{
				g=0;
			}
			if (g>255)
			{
				g=255;
			}
			b+=eb;
			if (b<0)
			{
				b=0;
			}
			if (b>255)
			{
				b=255;
			}


			dest->put_pixel(x,y, pack_argb(a,r,g,b), context);
			w32 d=dest->get_pixel(x,y, context); // see what the differance is
			argb_split(d, ea, er, eg, eb);

			ea-=a;
			er-=r;
			eg-=g;
			eb-=b;

			cur_er[1]+=pack_argb( (ea/4)+127, (er/4)+127, (eg/4)+127, (eb/4)+127);
			cur_er[w+1]+=pack_argb( (ea/2)+127, (er/2)+127, (eg/2)+127, (eb/2)+127);
			cur_er[w+1+1]+=pack_argb( (ea/4)+127, (er/4)+127, (eg/4)+127, (eb/4)+127);
		}

		cur_er++;
	}

	i4_free(er_im);
}
