/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "render/r1_font.h"
#include "image/context.h"
#include "render/r1_api.h"
#include "render/tmanage.h"
#include "render/r1_clip.h"

#define FONT_BACKGROUND 0x01492
static inline int i4_is_black(w32 c)
{
	//if ((c&0xff000000)==0) //Was soll der Quatsch? Schwarz heisst kein Alpha??
	//if((c&0x00ffffff)==0) //korrekt, aber leider nicht brauchbar
	if((c&0x00ffffff)==FONT_BACKGROUND)
	{
		//Das blau der Grafik
		return 1;
	}
	else
	{
		return 0;
	}
}

static inline int i4_scanline_empty(i4_image_class * im, int y)
{
	for (int x=0; x<im->width(); x++)
	{
		if (!i4_is_black(im->get_pixel(x,y)))
		{
			return 0;
		}
	}

	return 1;
}


static inline int i4_colum_empty(i4_image_class * im, int x)
{
	for (int y=0; y<im->height(); y++)
	{
		if (!i4_is_black(im->get_pixel(x,y)))
		{
			return 0;
		}
	}

	return 1;
}


static inline int i4_image_has_alpha(i4_image_class * im)
{
	for (int y=0; y<im->height(); y++)
	{
		for (int x=0; x<im->width(); x++)
		{
			w32 cl=im->get_pixel(x,y);
			if (cl>>24)
			{
				i4_warning("alpha %d", cl>>24);
			}
		}
	}

	return 1;
}

void r1_font_class::set_color(i4_color color)
{
	r=((color>>16)&0xff) * (1.0f/255.0f);
	g=((color>>8)&0xff) * (1.0f/255.0f);
	b=((color>>0)&0xff) * (1.0f/255.0f);
	col=color;
}

i4_bool r1_font_class::expand(i4_image_class * from,
							  i4_image_class * to,
							  int start_ch)
{
	//obsolete, don't use.
	int x=0, y=0;

	int y_top=0, y_bottom=from->height()-1, x1=0,x2,i;

	while (y_top<=y_bottom && i4_scanline_empty(from, y_top))
		y_top++;



	if (y_top==y_bottom-1)
	{
		i4_error("Font-image is empty");
	}

	while (i4_scanline_empty(from, y_bottom))
		y_bottom--;




	for (i=0; i<256; i++)
	{
		pos[i].w=4;
		pos[i].y=0;
		pos[i].x=255;
	}


	int char_on=start_ch;

	largest_h=y_bottom-y_top;

	i4_draw_context_class context(0,0, to->width()-1, to->height()-1);
	do
	{
		while (x1!=from->width() && i4_colum_empty(from, x1)) x1++;


		if (x1<from->width())
		{
			x2=x1+1;
			while (x2!=from->width() && !(i4_colum_empty(from, x2))) x2++;


			int w=x2-x1+1;
			if (w>longest_w)
			{
				longest_w=w;
			}


			if (w+x>to->width())
			{
				y+=largest_h;
				if (y>to->height())
				{
					return i4_F;
				}
				x=0;
			}

			pos[char_on].x=x;
			pos[char_on].y=y;
			pos[char_on].w=w;
			from->i4_image_class::put_part(to, x,y, x1,y_top, x2,y_bottom, context);

			x+=w+1;

			x1=x2+1;
		}
		char_on++;
	}
	while (x1<from->width() && char_on<255);


	//warning: image will be lost upon reloading the textures!
	texture=api->get_tmanager()->register_image(to);
	xs=1.0f/to->width();
	ys=1.0f/to->height();

	return i4_T;
}

r1_font_class::~r1_font_class()
{
	if (fontimage)
	{
		delete fontimage;
	}
	if (font)
	{
		delete font;
	}
	font=0;
	fontimage=0;
}

void r1_font_class::calculatefontdata(i4_image_class * im, int start_ch)
{
	int x=1;
	int y=0;
	int i=0;
	int startx;

	for (; i<256; i++)
	{
		posneu[i].x=0;
		posneu[i].y=0;
		posneu[i].w=1;
	}

	i=start_ch=33; //Mal sehen, damit beginnt die aktuelle Schriftart
	largest_h=im->height();
	int lw=1;
	while (x<im->width()&&(i<126)) //This should change if we want to use fonts with extended char sets
	{
		while(i4_colum_empty(im,x)) x++;


		startx=x;
		do
		{
			x++;
		}
		while (!i4_colum_empty(im,x));
		posneu[i].x=startx-1;
		if (posneu[i].x>lw)
		{
			lw=posneu[i].x;
		}
		posneu[i].w=x-startx+1;
		i++;
	}
	longest_w=lw;
}


r1_font_class::r1_font_class(r1_render_api_class * api, i4_image_class * im, int start_ch)
	: api(api)
{
	memset(pos,0, sizeof(pos));
	fontimage=0;
	font=0;
	h=im->height();
	w=im->width();
	bpp=im->pal->source.pixel_depth;
	//i4_draw_context_class mycontext(0,0,h,w);
	//font->set_pal(i4_pal_man.default_32());
	//im->put_image(font,0,0,mycontext);
	font=im->copy();
	calculatefontdata(font,start_ch);
	//fontimage=(void*)new char[(h*w*(bpp/8))+4];
	//memcpy(fontimage,im->data,h*w*(bpp/8));
	/*i4_image_class *to=i4_create_image(64, 64, i4_pal_man.default_32());

	   if (!expand(im, to, start_ch))
	   {
	   delete to;
	   to=i4_create_image(128, 128, i4_pal_man.default_32());
	   if (!expand(im, to, start_ch))
	   {
	   	delete to;
	   	to=i4_create_image(256, 256, i4_pal_man.default_32());
	   	if (!expand(im, to, start_ch))
	   	  i4_error("r1_font does not fit on a 256x256 texture");


	   }
	   }*/
}


void r1_font_class::put_character(i4_image_class * screen,
								  sw16 x, sw16 y,
								  const i4_char &c,
								  i4_draw_context_class &context)
{
	I4_ASSERT(screen,"Drawing to NULL-Screen??");
	char_def_neu chd;
	h=font->height();
	chd=posneu[c.value()];
	int xundw=chd.x+chd.w;
	w32 color;
	for (int j=0; j<h; j++)
	{
		for (int i=chd.x; i<(xundw); i++)
		{
			color=font->get_pixel(i,j);
			if (color==0xffffff)
			{
				color=col;
			}                              //Weiss->Vordergrundfarbe
			//if (c!=0x1492)
			screen->put_pixel(x+i-chd.x,y+j,color,context);
		}
	}


	screen->add_dirty(x,y,chd.w,y+h,context);
	/*
	   if (pos[ch].x!=255)
	   {
	   r1_vert v[4];

	   v[0].px=x;              v[0].py=y;
	   v[1].px=(float)x+pos[ch].w;    v[1].py=(float)y;
	   v[2].px=(float)x+pos[ch].w;    v[2].py=(float)y+largest_h;
	   v[3].px=(float)x;              v[3].py=(float)y+largest_h;


	   float xp=pos[ch].x * xs;
	   float yp=pos[ch].y * ys;
	   float wp=pos[ch].w * xs;
	   float hp=largest_h * ys;

	   v[0].s=xp; v[0].t=yp;
	   v[1].s=xp+wp; v[1].t=yp;
	   v[2].s=xp+wp; v[2].t=yp+hp;
	   v[3].s=xp; v[3].t=yp+hp;


	   float w=1.0f/r1_near_clip_z;
	   v[0].w=w; v[0].v.z=r1_near_clip_z;
	   v[1].w=w; v[1].v.z=r1_near_clip_z;
	   v[2].w=w; v[2].v.z=r1_near_clip_z;
	   v[3].w=w; v[3].v.z=r1_near_clip_z;

	   v[0].r=r; v[0].g=g; v[0].b=b;  v[0].a=1;
	   v[1].r=r; v[1].g=g; v[1].b=b;  v[1].a=1;
	   v[2].r=r; v[2].g=g; v[2].b=b;  v[2].a=1;
	   v[3].r=r; v[3].g=g; v[3].b=b;  v[3].a=1;

	   api->use_texture(texture, 256, 0);

	   i4_rect_list_class::area_iter cl=context.clip.list.begin();
	   for (; cl != context.clip.list.end(); ++cl)
	   	if (x>=cl->x1 && y>=cl->y1 && v[1].px<=cl->x2 && v[2].py<=cl->y2)
	   	  api->render_poly(4, v);
	   }
	   api->states_have_changed=i4_T;
	   api->flush_vert_buffer();
	 */
}




void r1_font_class::put_string(i4_image_class * screen,
							   sw16 x, sw16 y,
							   const i4_const_str &string,
							   i4_draw_context_class &context)
{
	//this routine needs speed improvements
	//ASSERT(screen);
	char_def_neu chd;

	h=font->height();
	int xinit=x;
	for (i4_const_str::iterator p=string.begin(); p!=string.end(); ++p)
	{
		i4_char ch=p.get();
		int i,j;
		chd=posneu[ch.value()];
		if (ch==' ')
		{
			w=4;
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
				{
					screen->put_pixel(x+i,y+j,FONT_BACKGROUND,context);
				}
			}
			x+=w;
			continue;
		}
		//put_character(screen,x,y,ch,context);
		//font->put_part_trans(screen,x,y,chd.x,chd.y,chd.w,h,0x00001492,context);
		w32 c;
		int xundw=chd.x+chd.w;
		for (j=0; j<h; j++)
		{
			for (i=chd.x; i<(xundw); i++)
			{
				c=font->get_pixel(i,j);
				if (c==0xffffff)
				{
					c=col;
				}                  //Weiss->Vordergrundfarbe
				//if (c!=0x1492)
				screen->put_pixel(x+i-chd.x,y+j,c,context);
			}
		}


		x+=(chd.w);
	}
	screen->add_dirty(xinit,y,x,y+h,context);
	//screen->bar(xinit,y,x,y+h,0x0000ff,context);

	//screen->rectangle(0,0,10,20,0x00ffff00,context);
	//api->states_have_changed=i4_T;
	//api->flush_vert_buffer();
	return;
	/*
	   r1_vert v[4];
	   ZeroMemory(v,sizeof(v));
	   if (r1_near_clip_z<=0) r1_near_clip_z=0.1f;
	   float w=0.9f/r1_near_clip_z;
	   v[0].w=w; v[0].v.z=r1_near_clip_z;
	   v[1].w=w; v[1].v.z=r1_near_clip_z;
	   v[2].w=w; v[2].v.z=r1_near_clip_z;
	   v[3].w=w; v[3].v.z=r1_near_clip_z;

	   v[0].r=r; v[0].g=g; v[0].b=b; // v[0].a=1;
	   v[1].r=r; v[1].g=g; v[1].b=b; // v[1].a=1;
	   v[2].r=r; v[2].g=g; v[2].b=b; // v[2].a=1;
	   v[3].r=r; v[3].g=g; v[3].b=b; // v[3].a=1;

	   if (!string.null())
	   {
	   api->use_texture(texture, 256, 0);
	   //api->disable_texture();

	   for (i4_const_str::iterator p=string.begin(); p!=string.end(); ++p)
	   {
	   	int ch=p.get().value();
	   	if (pos[ch].x!=255)
	   	{
	   	  i4_rect_list_class::area_iter cl=context.clip.list.begin();
	   	  for (; cl != context.clip.list.end(); ++cl)
	   	  {
	   		v[0].px=x;                  v[0].py=y;
	   		v[1].px=(float)x+pos[ch].w;        v[1].py=(float)y;
	   		v[2].px=(float)x+pos[ch].w;        v[2].py=(float)y+largest_h;
	   		v[3].px=(float)x;                  v[3].py=(float)y+largest_h;

	   		float xp=pos[ch].x * xs;
	   		float yp=pos[ch].y * ys;
	   		float wp=pos[ch].w * xs;
	   		float hp=largest_h * ys;


	   		v[0].s=xp; v[0].t=yp;
	   		v[1].s=xp+wp; v[1].t=yp;
	   		v[2].s=xp+wp; v[2].t=yp+hp;
	   		v[3].s=xp; v[3].t=yp+hp;

	   		if (x>=cl->x1 && y>=cl->y1 && v[1].px<=cl->x2 && v[2].py<=cl->y2)
	   		  api->render_poly(4, v);
	   	  }
	   	}

	   x+=pos[ch].w;

	   }
	   }
	   api->states_have_changed=i4_T;
	   api->flush_vert_buffer();
	 */
}
