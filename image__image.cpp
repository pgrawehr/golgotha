/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "error/error.h"

#include "palette/pal.h"

#include "area/rectlist.h"

#include "memory/malloc.h"

#include "image/image.h"
#include "image/image8.h"
#include "image/image16.h"
#include "image/image32.h"
#include "image/image24.h"
#include "image/context.h"
#include "math/num_type.h"

#include <string.h>

#ifndef abs
#define abs(x) ((x)<0 ? -(x) : (x))
#endif


void i4_image_class::widget(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, i4_color bright, i4_color med, i4_color dark, i4_draw_context_class &context)
{
  add_dirty(x1,y1,x2,y2,context);      // to keep from creating a dirty for each operation below

  bar(x1,y1,x2,y1,bright,context);
  bar(x1,y1+1,x1,y2,bright,context);
  bar(x2,y1+1,x2,y2,dark,context);
  bar(x1+1,y2,x2-1,y2,dark,context);
  bar(x1+1,y1+1,x2-1,y2-1,med,context);
}

void i4_image_class::put_pixel(i4_coord x, i4_coord y, w32 color, i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {      
    if (x>=c->x1 && x<=c->x2 && y>=c->y1 && y<=c->y2)
		{
      put_pixel(x + context.xoff, y + context.yoff, color);
	  break; //We draw one point at most
		}
  }
}


w32 i4_image_class::get_pixel(i4_coord x, i4_coord y,  i4_draw_context_class &context)
{
  return get_pixel(x + context.xoff, y + context.yoff);
}

  
void i4_image_class::xor_bar(i4_coord x1,    i4_coord y1, 
                             i4_coord x2,    i4_coord y2, 
                             i4_color color, i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {
    i4_coord lx1,ly1,lx2,ly2;

    if (x1<c->x1) lx1=c->x1; else lx1=x1;
    if (y1<c->y1) ly1=c->y1; else ly1=y1;
    if (x2>c->x2) lx2=c->x2; else lx2=x2;
    if (y2>c->y2) ly2=c->y2; else ly2=y2;

    if (!(lx1>lx2 || ly1>ly2))
    {
      add_dirty(lx1,ly1,lx2,ly2,context);
      for (;ly1<=ly2; ly1++)
        for (int x=lx1; x<=lx2; x++)
          put_pixel(x + context.xoff, ly1 + context.yoff, get_pixel(x,ly1)^0xffffff);

    }
  }
}



void i4_image_class::bar(i4_coord x1,    i4_coord y1, 
                         i4_coord x2,    i4_coord y2, 
                         i4_color color, i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {    
    i4_coord lx1,ly1,lx2,ly2;

    if (x1<c->x1) lx1=c->x1; else lx1=x1;
    if (y1<c->y1) ly1=c->y1; else ly1=y1;
    if (x2>c->x2) lx2=c->x2; else lx2=x2;
    if (y2>c->y2) ly2=c->y2; else ly2=y2;

    if (!(lx1>lx2 || ly1>ly2))
    {
      add_dirty(lx1,ly1,lx2,ly2,context);
      for (;ly1<=ly2; ly1++)
        for (int x=lx1; x<=lx2; x++)
          put_pixel(x+ context.xoff, ly1+ context.yoff, color);

    }
  }
}

void i4_image_class::line(i4_coord ox1, i4_coord oy1, 
                          i4_coord ox2, i4_coord oy2, 
                          i4_color color, i4_draw_context_class &context)
{//draws a line using the bresenham algorithm. (fastest known algo
//for line-drawing)
  i4_coord x1,y1,x2,y2;
  i4_coord cx1,cy1,cx2,cy2;
  i4_bool skip;

  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  { 
    x1=ox1;
    y1=oy1;
    x2=ox2;
    y2=oy2;
    skip=i4_F;

    i4_coord i,xc,yc,er,n,m,xi,yi,xcxi,ycyi,xcyi;
    unsigned dcy,dcx;
    // check to make sure that both endpoint are on the screen

    cx1=c->x1;
    cy1=c->y1;
    cx2=c->x2;
    cy2=c->y2;

    // check to see if the line is completly clipped off
    if (!((x1<cx1 && x2<cx1) || (x1>cx2 && x2>cx2) || 
          (y1<cy1 && y2<cy1) || (y1>cy2 && y2>cy2)))
    {
 
      if (x1>x2)        // make sure that x1 is to the left
      {    
        i=x1; x1=x2; x2=i;  // if not swap points
        i=y1; y1=y2; y2=i;
      }  

      // clip the left side
      if (x1<cx1)
      {  
        int my=(y2-y1);       
        int mx=(x2-x1),b;
        if (!mx) skip=i4_T;
        if (my)
        {
          b=y1-(y2-y1)*x1/mx;      
          y1=my*cx1/mx+b;
          x1=cx1;      
        }
        else x1=cx1;
      }

      // clip the right side
      if (x2>cx2)
      {  
        int my=(y2-y1);       
        int mx=(x2-x1),b;
        if (!mx) skip=i4_T;
        else if (my)
        {
          b=y1-(y2-y1)*x1/mx;      
          y2=my*cx2/mx+b;
          x2=cx2;      
        }
        else x2=cx2;
      }

      if (y1>y2)        // make sure that y1 is on top
      {    
        i=x1; x1=x2; x2=i;  // if not swap points
        i=y1; y1=y2; y2=i;
      }  

      // clip the bottom
      if (y2>cy2)
      {  
        int mx=(x2-x1);       
        int my=(y2-y1),b;
        if (!my)
          skip=i4_T;
        else if (mx)
        {
          b=y1-(y2-y1)*x1/mx;      
          x2=(cy2-b)*mx/my;
          y2=cy2;
        }
        else y2=cy2;
      }

      // clip the top
      if (y1<cy1)
      {  
        int mx=(x2-x1);       
        int my=(y2-y1),b;
        if (!my) 
          skip=i4_T;
        else if (mx)
        {
          b=y1-(y2-y1)*x1/mx;      
          x1=(cy1-b)*mx/my;
          y1=cy1;
        }
        else y1=cy1;
      }


      // see if it got cliped into the box, out out
      if (x1<cx1 || x2<cx1 || x1>cx2 || x2>cx2 || y1<cy1 || y2 <cy1 || y1>cy2 || y2>cy2)
        skip=i4_T;     

      if (x1>x2)
      { xc=x2; xi=x1; }
      else { xi=x2; xc=x1; }

      if (!skip)
      {
        // assume y1<=y2 from above swap operation
        yi=y2; yc=y1;

        add_dirty(xc,yc,xi,yi,context);
        dcx=x1+context.xoff; dcy=y1+context.yoff;
        xc=(x2-x1); yc=(y2-y1);
        if (xc<0) xi=-1; else xi=1;
        if (yc<0) yi=-1; else yi=1;
        n=abs(xc); m=abs(yc);
        ycyi=abs(2*yc*xi);
        er=0;
      
        
        if (n>m)
        {
          xcxi=abs(2*xc*xi);
          for (i=0;i<=n;i++)
          {
            put_pixel(dcx, dcy, color);

            if (er>0)
            {
              dcy+=yi;
              er-=xcxi;
            }
            er+=ycyi;
            dcx+=xi;
          }
        }
        else
        {
          xcyi=abs(2*xc*yi);
          for (i=0;i<=m;i++)
          {
            put_pixel(dcx, dcy, color);

            if (er>0)
            {
              dcx+=xi;
              er-=ycyi;
            }
            er+=xcyi;
            dcy+=yi;
          }
        }
      }
    }
  }
}


void i4_image_class::put_part(i4_image_class *to, 
                              i4_coord _x,  i4_coord _y,                              
                              i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, 
                              i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  { 
    i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;

    if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
    if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
    if (x2>=width())  lx2=width()-1;   else lx2=x2;
    if (y2>=height()) ly2=height()-1;  else ly2=y2;
  
    if (!(lx1>lx2 || ly1>ly2))
    {
      if (x<c->x1)
      { lx1+=(c->x1-x); x=c->x1; }

      if (y<c->y1)
      { ly1+=(c->y1-y); y=c->y1; }


      if (x+lx2-lx1+1>c->x2)
        lx2=c->x2-x+lx1; 

      if (y+ly2-ly1+1>c->y2)
        ly2=c->y2-y+ly1; 


      const i4_pixel_format *from_format=&get_pal()->source;
      const i4_pixel_format *to_format=&to->get_pal()->source;
      if (from_format->alpha_mask==0 || to_format->alpha_mask)
      {
        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
          while (ly1<=ly2)
          {
            int tx=x;
            for (int fx=lx1;fx<=lx2; fx++, tx++)
            {     
              w32 color_32=get_pixel(fx, ly1);          
              to->put_pixel(tx+ context.xoff, y+ context.yoff, color_32);
            }
            ly1++;
            y++;
          }
        }
      }
      else
      {
        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
          while (ly1<=ly2)
          {
            int tx=x;
            for (int fx=lx1;fx<=lx2; fx++, tx++)
            {     
              w32 c=get_pixel(fx, ly1);
              float a=(float)(c>>24);
              if (a)
              {
                a/=255.0;

                int r=(c>>16)&0xff, g=(c>>8)&0xff, b=c&0xff;
                w32 tc=to->get_pixel(tx, y, context);
                int tr=(tc>>16)&0xff, tg=(tc>>8)&0xff, tb=tc&0xff;
                
                r=(int)((r-tr)*a + tr);
                g=(int)((g-tg)*a + tg);
                b=(int)((b-tb)*a + tb);

                to->put_pixel(tx+ context.xoff, y+ context.yoff, (r<<16) | (g<<8) | b);
              }
            }
            ly1++;
            y++;
          }
        }
      }
    }
  }
}



void i4_image_class::put_part_trans(i4_image_class *to, 
                                    i4_coord _x,  i4_coord _y,                              
                                    i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                                    i4_color trans_color,
                                    i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  { 
    i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;

    if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
    if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
    if (x2>=width())  lx2=width()-1;   else lx2=x2;
    if (y2>=height()) ly2=height()-1;  else ly2=y2;
  
    if (!(lx1>lx2 || ly1>ly2))
    {
      if (x<c->x1)
      { lx1+=(c->x1-x); x=c->x1; }

      if (y<c->y1)
      { ly1+=(c->y1-y); y=c->y1; }


      if (x+lx2-lx1+1>c->x2)
        lx2=c->x2-x+lx1; 

      if (y+ly2-ly1+1>c->y2)
        ly2=c->y2-y+ly1; 


      const i4_pixel_format *from_format=&get_pal()->source;
      const i4_pixel_format *to_format=&to->get_pal()->source;

      if (!(lx1>lx2 || ly1>ly2))
      {
        to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
        while (ly1<=ly2)
        {
          int tx=x;
          for (int fx=lx1;fx<=lx2; fx++, tx++)
          {     
            w32 c=get_pixel(fx, ly1);
            if (c!=trans_color)
              to->put_pixel(tx+context.xoff, y+ context.yoff, c);
          }
          ly1++;
          y++;
        }
      }
    }
  }
}



i4_image_class *i4_create_image(int width, int height, const i4_pal *pal)
{
  switch (pal->source.pixel_depth)
  {
    case I4_32BIT :
      return new i4_image32(width, height, pal);
      break;

	case I4_24BIT:
	  return new i4_image24(width,height,pal);
	  break;

    case I4_16BIT :
      return new i4_image16(width, height, pal);
      break;
      
      
    case I4_8BIT :
      return new i4_image8(width, height, pal);
      break;
      
    default:      
      i4_error("i4_create_image: Invalid Pixel depth given.");      
  }
  


  return 0;
}

i4_image_class *i4_create_image(int width, int height,
                                const i4_pal *pal,
                                void *data,
                                int bpl)
{
  switch (pal->source.pixel_depth)
  {
    case I4_32BIT :
      return new i4_image32(width, height, pal, data, bpl);
      break;

	case I4_24BIT :
	  return new i4_image24(width, height, pal, data, bpl);
	  break;

    case I4_16BIT :
      return new i4_image16(width, height, pal, data, bpl);
      break;
      
    case I4_8BIT :
      return new i4_image8(width, height, pal, data, bpl);
      break;
      
    default:      
      i4_error("don't know how to handle this video mode.");      
  }
  return 0;
}

i4_image_class *i4_image_class::copy()
{
  i4_image_class *im=i4_create_image(width(), height(), pal);

  for (int y=0; y<h; y++)
    for (int x=0; x<w; x++)
      im->put_pixel(x,y, get_pixel(x,y));

  return im;
}


void i4_image_class::add_single_dirty(i4_coord x1, i4_coord y1, i4_coord x2, 
                                      i4_coord y2, i4_draw_context_class &context)
{
  context.add_single_dirty(x1+context.xoff,y1+context.yoff,
                           x2+context.xoff,y2+context.yoff);
}
  
void i4_image_class::add_dirty(i4_coord x1, i4_coord y1, i4_coord x2, 
                               i4_coord y2, i4_draw_context_class &context)
{
  context.add_both_dirty(x1+context.xoff,y1+context.yoff,
                         x2+context.xoff,y2+context.yoff);
}

void i4_image_class::rectangle(i4_coord x1, i4_coord y1,
                               i4_coord x2, i4_coord y2, i4_color color, 
                               i4_draw_context_class &context)
{
  bar(x1,y1,x1,y2,color,context);
  bar(x2,y1,x2,y2,color,context);
  bar(x1+1,y1,x2-1,y1,color,context);
  bar(x1+1,y2,x2-1,y2,color,context);
}

i4_image_class *i4_image_class::scale_image(i4_image_class *to,i4_coord newx,i4_coord newy, 
											const i4_pal *_pal)
	{
	i4_image_class *r=0;
	const i4_pal *newpal;
	if (to)
		{
		if (to->width()!=newx||to->height()!=newy) return NULL;
		r=to;
		newpal=to->pal;
		}
	else
		{
		if (_pal==0) newpal=pal; else newpal=_pal;
		r=i4_create_image(newx,newy,newpal);
		}
	double base_width=w;
	double base_height=h;
	
	double width_ratio  = (double)base_width  / (double)newx;
	double height_ratio = (double)base_height / (double)newy;
	
	//now scale the old to fit the new, convert from source format to target
	sw32 i,j;
	
	for (j=0; j<newy; j++)
		for (i=0; i<newx; i++)
			{    
			w32 c = get_pixel((i4_coord) i4_f_to_i((float) i * (float) width_ratio),
				(i4_coord) i4_f_to_i((float) j * (float) height_ratio));
			//get_pixel() always returns a 32-Bit color value, so conversion is not 
			//needed here. 
			//w32 c2=get_pal()->convert(c,&newpal->source);
			r->put_pixel(i,j,c);
			}
		
	return r;
	}

	void i4_image_class::put_image(i4_image_class *to, i4_coord x, i4_coord y)
	{
		i4_draw_context_class ctx(0,0,to->width(),to->height());
		put_image(to,x,y,ctx);
	}

	i4_image_class* i4_image_class::rotate_image(g1_rotation_type rotation, i4_bool mirror)
	{
		int neww=width();
		int newh=height();
		bool bReverse=i4_T; //true if x and y are exchanged (is true by default, 
		                    //for unknown historical reasons)
		//I4_ASSERT(neww==newh,"Width not equal height");
		if (rotation==G1_ROTATE_90 || rotation == G1_ROTATE_270)
		{
			bReverse=!bReverse;
			neww=height();
			newh=width();
		}
		if (mirror)
		{
			bReverse=!bReverse;
			int temp=newh;
			newh=neww;
			neww=temp;
		}
		i4_image_class *ret=i4_create_image(neww,newh,this->get_pal());
		int y,x;
		int outerloop=newh;
		int innerloop=neww;
		if (bReverse)
		{
			outerloop=neww;
			innerloop=newh;
		}
		for (y=0;y<outerloop;y++)
		{
			for (x=0;x<innerloop;x++)
			{
				int newxpos=0;
				int newypos=0;
				if (!mirror)
				{
					switch(rotation)
					{
					case G1_ROTATE_0:
						{
							newypos=newh-x-1; //always fast-incrementing line first
							newxpos=neww-y-1;
						}break;
					case G1_ROTATE_90:
						{
							newxpos=neww-x-1;
							newypos=y;
						}break;
					case G1_ROTATE_180:
						{
							newypos=x;
							newxpos=y;
						}break;
					case G1_ROTATE_270:
						{
							newxpos=x;
							newypos=newh-y-1;
						}break;
					}
				}
				else
				{
					switch(rotation)
					{
					case G1_ROTATE_0: //The very easy case. 
						{
							newxpos=x;
							newypos=y;
						}break;
					case G1_ROTATE_90:
						{
							newypos=x;
							newxpos=neww-y-1;
						}break;
					case G1_ROTATE_180:
						{
							newxpos=neww-x-1;
							newypos=newh-y-1;
						}break;
					case G1_ROTATE_270:
						{
							newypos=newh-x-1;
							newxpos=y;
						}break;
					}
				}
				w32 color=get_pixel(x,y);
				I4_ASSERT(newxpos<ret->width() && newypos< ret->height() &&
					(newxpos>=0) && (newypos>=0) ,"SEVERE: Image rotation index transposition confusion");
				ret->put_pixel(newxpos,newypos,color);
			}
		}
		return ret;
		
	}

	void i4_image_class::copy_image_to(i4_image_class *to, 
		i4_coord new_xpos, i4_coord new_ypos, 
		sw32 xsize, sw32 ysize, 
		g1_rotation_type rotation, i4_bool mirror)
	{
		i4_image_class *temp_im1=NULL, *temp_im2=NULL;
		//Rotate image and mirror image
		temp_im1=rotate_image(rotation,mirror);
		//Scale image to target size and convert color depth. 
		//Copy image to location in target image.
		
		temp_im2=temp_im1->scale_image(NULL,xsize,ysize,to->get_pal());
		delete temp_im1;
		temp_im2->put_image(to,new_xpos,new_ypos);
		delete temp_im2;
	}

// IMAGE8
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/




i4_color i4_image8::get_pixel(i4_coord x, i4_coord y)
{    
  return i4_pal_man.convert_to_32(*(typed_data() + bpl*y + x), pal);
}
  
void i4_image8::put_pixel(i4_coord x, i4_coord y, w32 color)
{
  *(typed_data() + bpl*y + x)=(w8)(i4_pal_man.convert_32_to(color, &pal->source));
}


i4_image8::i4_image8(w16 _w, w16 _h, const i4_pal *_pal)
{
  w=_w;
  h=_h;
  bpl=_w;
  set_pal(_pal);
  data=new w8[w*h];//I4_MALLOC(w*h,"");  
}

i4_image8::i4_image8(w16 _w, w16 _h, const i4_pal *_pal,
                       void *_data, int _bpl)
{
  data=(w8*)_data;
  bpl=_bpl;
  pal=_pal;
  w=_w;
  h=_h;

  dont_free_data=i4_T;
}


i4_image8::~i4_image8()
{
  if (!dont_free_data)
    delete data;
}


i4_image_class *i4_image8::copy()
{
  i4_image_class *im=i4_create_image(width(), height(), pal);

  
  for (int y=0; y<h; y++)
    memcpy(((w8 *)im->data) + y*im->bpl,
           ((w8 *)data) + y*bpl,
           w);

  return im;
}
// IMAGE16
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


inline int iabs(int val) { return val>0?val:-val; }

i4_color i4_image16::get_pixel(i4_coord x, i4_coord y)
{    
  return i4_pal_man.convert_to_32(*paddr(x,y), pal);
}
  
void i4_image16::put_pixel(i4_coord x, i4_coord y, w32 color)
{
  w16 *addr=paddr(x,y);
  *addr=(w16)i4_pal_man.convert_32_to(color, &pal->source);
}




i4_image16::i4_image16(w16 _w, w16 _h, const i4_pal *_pal)
{
  w=_w;
  h=_h;
  bpl=_w*2;
  set_pal(_pal);
  data=new w8[w*h*2];//I4_MALLOC(w*h*2, "");
  dont_free_data=i4_F;
}

i4_image16::i4_image16(w16 _w, w16 _h, const i4_pal *_pal,
                       void *_data, int _bpl)
{
  data=(w8*)_data;
  bpl=_bpl;
  pal=_pal;
  w=_w;
  h=_h;

  dont_free_data=i4_T;
}

i4_image_class *i4_image16::copy()
{
  i4_image_class *im=i4_create_image(width(), height(), pal);

  //w32 size=w*h*2;
  //for (int y=0; y<h; y++)
  //  for (int x=0; x<w; x++)
  //    im->put_pixel(x,y, get_pixel(x,y));
  memcpy(im->data,data,w*h*2);

  return im;
}


i4_image16::~i4_image16()
{
  if (!dont_free_data)
    delete data;
}



void i4_image16::line(i4_coord ox1, i4_coord oy1, 
                          i4_coord ox2, i4_coord oy2, 
                          i4_color color, i4_draw_context_class &context)
{
  i4_coord x1,y1,x2,y2;
  i4_coord cx1,cy1,cx2,cy2;
  i4_bool skip;

  w16 tcolor=(w16)i4_pal_man.convert_32_to(color, &pal->source);

  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  { 
    x1=ox1;
    y1=oy1;
    x2=ox2;
    y2=oy2;
    skip=i4_F;

    i4_coord i,xc,yc,er,n,m,xi,yi,xcxi,ycyi,xcyi;
    unsigned dcy,dcx;
    // check to make sure that both endpoint are on the screen

    cx1=c->x1;
    cy1=c->y1;
    cx2=c->x2;
    cy2=c->y2;

    // check to see if the line is completly clipped off
    if (!((x1<cx1 && x2<cx1) || (x1>cx2 && x2>cx2) || 
          (y1<cy1 && y2<cy1) || (y1>cy2 && y2>cy2)))
    {
 
      if (x1>x2)        // make sure that x1 is to the left
      {    
        i=x1; x1=x2; x2=i;  // if not swap points
        i=y1; y1=y2; y2=i;
      }  

      // clip the left side
      if (x1<cx1)
      {  
        int my=(y2-y1);       
        int mx=(x2-x1),b;
        if (!mx) skip=i4_T;
        if (my)
        {
          b=y1-(y2-y1)*x1/mx;      
          y1=my*cx1/mx+b;
          x1=cx1;      
        }
        else x1=cx1;
      }

      // clip the right side
      if (x2>cx2)
      {  
        int my=(y2-y1);       
        int mx=(x2-x1),b;
        if (!mx) skip=i4_T;
        else if (my)
        {
          b=y1-(y2-y1)*x1/mx;      
          y2=my*cx2/mx+b;
          x2=cx2;      
        }
        else x2=cx2;
      }

      if (y1>y2)        // make sure that y1 is on top
      {    
        i=x1; x1=x2; x2=i;  // if not swap points
        i=y1; y1=y2; y2=i;
      }  

      // clip the bottom
      if (y2>cy2)
      {  
        int mx=(x2-x1);       
        int my=(y2-y1),b;
        if (!my)
          skip=i4_T;
        else if (mx)
        {
          b=y1-(y2-y1)*x1/mx;      
          x2=(cy2-b)*mx/my;
          y2=cy2;
        }
        else y2=cy2;
      }

      // clip the top
      if (y1<cy1)
      {  
        int mx=(x2-x1);       
        int my=(y2-y1),b;
        if (!my) 
          skip=i4_T;
        else if (mx)
        {
          b=y1-(y2-y1)*x1/mx;      
          x1=(cy1-b)*mx/my;
          y1=cy1;
        }
        else y1=cy1;
      }


      // see if it got cliped into the box, out out
      if (x1<cx1 || x2<cx1 || x1>cx2 || x2>cx2 || y1<cy1 || y2 <cy1 || y1>cy2 || y2>cy2)
        skip=i4_T;     

      if (x1>x2)
      { xc=x2; xi=x1; }
      else { xi=x2; xc=x1; }

      if (!skip)
      {
        // assume y1<=y2 from above swap operation
        yi=y2; yc=y1;

        add_dirty(xc,yc,xi,yi,context);
        dcx=x1+context.xoff; dcy=y1+context.yoff;
        xc=(x2-x1); yc=(y2-y1);
        if (xc<0) xi=-1; else xi=1;
        if (yc<0) yi=-1; else yi=1;
        n=iabs(xc); m=iabs(yc);
        ycyi=iabs(2*yc*xi);
        er=0;
      
        
        if (n>m)
        {
          xcxi=iabs(2*xc*xi);
          for (i=0;i<=n;i++)
          {
            *paddr(dcx, dcy)=tcolor;

            if (er>0)
            {
              dcy+=yi;
              er-=xcxi;
            }
            er+=ycyi;
            dcx+=xi;
          }
        }
        else
        {
          xcyi=iabs(2*xc*yi);
          for (i=0;i<=m;i++)
          {
            *paddr(dcx, dcy)=tcolor;

            if (er>0)
            {
              dcx+=xi;
              er-=ycyi;
            }
            er+=xcyi;
            dcy+=yi;
          }
        }
      }
    }
  }
}


void i4_image16::put_part(i4_image_class *to, 
                              i4_coord _x,  i4_coord _y,                              
                              i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, 
                              i4_draw_context_class &context)
{
  const i4_pixel_format *from_format=&get_pal()->source;
  const i4_pixel_format *to_format=&to->get_pal()->source;
  i4_bool success=i4_F;
  
  if ((to_format->pixel_depth!=I4_16BIT ||
      to_format->red_mask != from_format->red_mask ||
      to_format->green_mask != from_format->green_mask ||
      to_format->blue_mask != from_format->blue_mask) &&
      (from_format->alpha_mask==0 || to_format->alpha_mask))
	  {
		if (!success)
		{
			// depths aren't the same and I dono a fast conversion
			// do it the slow way
			// simple conversions in bitdepth (like from 565 to 1444 or something)
			// should be very rare anyway
			i4_image_class::put_part(to, _x, _y, x1,y1,x2,y2, context);
			return;
		}
	  }
  else
  {  
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();
         c!=context.clip.list.end();
         ++c)
    { 
      i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;

      if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
      if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
      if (x2>=width())  lx2=width()-1;   else lx2=x2;
      if (y2>=height()) ly2=height()-1;  else ly2=y2;
  
      if (!(lx1>lx2 || ly1>ly2))
      {
        if (x<c->x1)
        { lx1+=(c->x1-x); x=c->x1; }

        if (y<c->y1)
        { ly1+=(c->y1-y); y=c->y1; }


        if (x+lx2-lx1+1>c->x2)
          lx2=c->x2-x+lx1; 

        if (y+ly2-ly1+1>c->y2)
          ly2=c->y2-y+ly1; 


        w16 *source=paddr(lx1, ly1);
        w16 *dest=((i4_image16 *)to)->paddr(x + context.xoff,
                                            y + context.yoff);
        
        int copy_width=((lx2-lx1)+1)*2;

        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
          while (ly1<=ly2)
          {              
            memcpy(dest, source, copy_width);
            dest=((w16 *)((w8 *)dest + to->bpl));
            source=((w16 *)((w8 *)source + bpl));
            ly1++;
          }
        }
      }
    }
  }
}



void i4_image16::put_part_one_pixel_at_a_time(i4_image_class *to, 
                                              i4_coord _x,  i4_coord _y,
                                              i4_coord x1, i4_coord y1,
                                              i4_coord x2, i4_coord y2, 
                                              i4_draw_context_class &context)
{
  const i4_pixel_format *from_format=&get_pal()->source;
  const i4_pixel_format *to_format=&to->get_pal()->source;

  
  if ((to_format->pixel_depth!=I4_16BIT ||
      to_format->red_mask != from_format->red_mask ||
      to_format->green_mask != from_format->green_mask ||
      to_format->blue_mask != from_format->blue_mask) &&
      (from_format->alpha_mask==0 || to_format->alpha_mask))
    // depths aren't the same do it the slow way
    i4_image_class::put_part(to, _x, _y, x1,y1,x2,y2, context);
  else
  {  
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();
         c!=context.clip.list.end();
         ++c)
    { 
      i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;

      if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
      if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
      if (x2>=width())  lx2=width()-1;   else lx2=x2;
      if (y2>=height()) ly2=height()-1;  else ly2=y2;
  
      if (!(lx1>lx2 || ly1>ly2))
      {
        if (x<c->x1)
        { lx1+=(c->x1-x); x=c->x1; }

        if (y<c->y1)
        { ly1+=(c->y1-y); y=c->y1; }


        if (x+lx2-lx1+1>c->x2)
          lx2=c->x2-x+lx1; 

        if (y+ly2-ly1+1>c->y2)
          ly2=c->y2-y+ly1; 


        w16 *source=paddr(lx1, ly1);
        w16 *dest=((i4_image16 *)to)->paddr(x + context.xoff,
                                            y + context.yoff);
        
        int copy_width=((lx2-lx1)+1)*2;

        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
          while (ly1<=ly2)
          {
            for (int fx=0; fx<copy_width/2; fx++)
              dest[fx]=source[fx];
              
            dest=((w16 *)((w8 *)dest + to->bpl));
            source=((w16 *)((w8 *)source + bpl));
            ly1++;
          }
        }
      }
    }
  }
}



void i4_image16::put_part_trans(i4_image_class *to, 
                                    i4_coord _x,  i4_coord _y,                              
                                    i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                                    i4_color trans_color,
                                    i4_draw_context_class &context)
{
  const i4_pixel_format *from_format=&get_pal()->source;
  const i4_pixel_format *to_format=&to->get_pal()->source;

  
  if ((to_format->pixel_depth!=I4_16BIT ||
      to_format->red_mask != from_format->red_mask ||
      to_format->green_mask != from_format->green_mask ||
      to_format->blue_mask != from_format->blue_mask) &&
      (from_format->alpha_mask==0 || to_format->alpha_mask))
    // depths aren't the same do it the slow way
    i4_image_class::put_part(to, _x, _y, x1,y1,x2,y2, context);
  else
  {
    i4_color tcolor=i4_pal_man.convert_32_to(trans_color, &pal->source);
    
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();
         c!=context.clip.list.end();++c)
    { 
      i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;

      if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
      if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
      if (x2>=width())  lx2=width()-1;   else lx2=x2;
      if (y2>=height()) ly2=height()-1;  else ly2=y2;
  
      if (!(lx1>lx2 || ly1>ly2))
      {
        if (x<c->x1)
        { lx1+=(c->x1-x); x=c->x1; }

        if (y<c->y1)
        { ly1+=(c->y1-y); y=c->y1; }


        if (x+lx2-lx1+1>c->x2)
          lx2=c->x2-x+lx1; 

        if (y+ly2-ly1+1>c->y2)
          ly2=c->y2-y+ly1; 


        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);

          w16 *source=paddr(lx1, ly1);
          w16 *dest=((i4_image16 *)to)->paddr(x + context.xoff,
                                              y + context.yoff);
          int w=(lx2-lx1+1);
          
          for (; ly1<=ly2; ly1++)
          {
            for (int j=0; j<w; j++)
            {
              w16 c=source[j];
              if (c!=tcolor)
                dest[j]=c;
            }
            
            source=(w16 *)((w8 *)source + bpl);
            dest=(w16 *)((w8 *)dest + to->bpl);
          }
        }
      }
    }
  }
}


void i4_image16::bar(i4_coord x1,    i4_coord y1, 
                     i4_coord x2,    i4_coord y2, 
                     i4_color color, i4_draw_context_class &context)
{
  w16 tcolor=(w16)i4_pal_man.convert_32_to(color, &pal->source);
    
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {    
    i4_coord lx1,ly1,lx2,ly2;

    if (x1<c->x1) lx1=c->x1; else lx1=x1;
    if (y1<c->y1) ly1=c->y1; else ly1=y1;
    if (x2>c->x2) lx2=c->x2; else lx2=x2;
    if (y2>c->y2) ly2=c->y2; else ly2=y2;

    if (!(lx1>lx2 || ly1>ly2))
    {
      w16 *d=paddr(lx1 + context.xoff, ly1+context.yoff);
      int count=lx2-lx1+1;
      
      add_dirty(lx1,ly1,lx2,ly2,context);
      for (;ly1<=ly2; ly1++)
      {
        for (int x=0; x<count; x++)
          d[x]=tcolor;
        
        d=(w16 *)((w8 *)d + bpl);
      }
    }
  }
}
// IMAGE32.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


i4_color i4_image32::get_pixel(i4_coord x, i4_coord y)
{    
  return i4_pal_man.convert_to_32(*paddr(x,y), pal);
}
  
void i4_image32::put_pixel(i4_coord x, i4_coord y, w32 color)
{
  *paddr(x,y)=i4_pal_man.convert_32_to(color, &pal->source);
}

i4_image32::i4_image32(w16 _w, w16 _h, const i4_pal *_pal)
{
  w=_w;
  h=_h;
  bpl=_w*4;
  set_pal(_pal);
  data=new w8[w*h*4];  
  dont_free_data=i4_F;
}

i4_image32::i4_image32(w16 _w, w16 _h, const i4_pal *_pal,
                       void *_data, int _bpl)
{
  data=(w8*)_data;
  bpl=_bpl;
  pal=_pal;
  w=_w;
  h=_h;

  dont_free_data=i4_T;
}


i4_image32::~i4_image32()
{
  if (!dont_free_data)
    delete data;
}

void i4_image32::put_part(i4_image_class *to, i4_coord _x, i4_coord _y, i4_coord x1,
	  i4_coord y1, i4_coord x2, i4_coord y2, i4_draw_context_class &context)
	{
	//i4_image_class::put_part(to,_x,_y,x1,y1,x2,y2,context);
	//return;
	const i4_pixel_format *from_format=&get_pal()->source;
	const i4_pixel_format *to_format=&to->get_pal()->source;
	i4_bool success=i4_F;
	
	if (to_format->pixel_depth!=I4_32BIT)
		{		
		if (to_format->pixel_depth==I4_16BIT)
			{
			if (to_format->alpha_bits==0&&from_format->alpha_bits==0)
				{//32 to 16 reduction, no alpha.
								
				for (i4_rect_list_class::area_iter c=context.clip.list.begin();
				c!=context.clip.list.end();
				++c)
					{ 
					i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;
					
					if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
					if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
					if (x2>=width())  lx2=width()-1;   else lx2=x2;
					if (y2>=height()) ly2=height()-1;  else ly2=y2;
					
					if (!(lx1>lx2 || ly1>ly2))
						{
						if (x<c->x1)
							{ lx1+=(c->x1-x); x=c->x1; }
						
						if (y<c->y1)
							{ ly1+=(c->y1-y); y=c->y1; }
						
						
						if (x+lx2-lx1+1>c->x2)
							lx2=c->x2-x+lx1; 
						
						if (y+ly2-ly1+1>c->y2)
							ly2=c->y2-y+ly1; 
						
						
						w32 *source=paddr(lx1, ly1),*xsoffs;
						w16 *dest=((i4_image16 *)to)->paddr(x + context.xoff,
							y + context.yoff);
						w16 *xdoffs;
						w32 col,cold;
						int copy_width=((lx2-lx1)+1)*4;
						
						if (to_format->green_bits==6)
							{
							//convert 0888 to 565
							if (!(lx1>lx2 || ly1>ly2))
								{
								to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
								while (ly1<=ly2)
									{              
									//memcpy(dest, source, copy_width);
									xdoffs=dest;
									for(xsoffs=source;((w8*)xsoffs)<((w8*)source+copy_width);xsoffs++)
										{
										col=*xsoffs;
										cold=(col>>(19-11))&0xf800;
										cold+=((col>>5)&0x07e0);
										cold+=((col>>3)&0x001f);
										*xdoffs=(w16)cold;
										xdoffs++;
										}
									dest=((w16 *)((w8 *)dest + to->bpl));
									source=((w32 *)((w8 *)source + bpl));
									ly1++;
									}
								}
							success=i4_T;
							}
						else if (to_format->green_bits==5)
							{
							if (!(lx1>lx2 || ly1>ly2))
								{
								to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
								while (ly1<=ly2)
									{              
									//memcpy(dest, source, copy_width);
									xdoffs=dest;
									for(xsoffs=source;(w8*)xsoffs<((w8*)source+copy_width);xsoffs++)
										{
										col=*xsoffs;
										cold=(col>>(19-10))&0x7c00;
										cold+=((col>>6)&0x03e0);
										cold+=((col>>3)&0x001f);
										*xdoffs=(w16)cold;
										xdoffs++;
										}
									dest=((w16 *)((w8 *)dest + to->bpl));
									source=((w32 *)((w8 *)source + bpl));
									ly1++;
									}
								}
							success=i4_T;
							}
						}
					else
						{
						success=i4_T;//we couldn't copy, but we know
						//why, so don't try again
						}
					}//for clip_list
				
				
				}//if alpha
			}//if target==16
		if (!success)
			{
			// depths aren't the same and I dono a fast conversion
			// do it the slow way
			// simple conversions in bitdepth (like from 565 to 1444 or something)
			// should be very rare anyway
			i4_image_class::put_part(to, _x, _y, x1,y1,x2,y2, context);
			return;
			};
		}
		else //source and target bitdepth are the same
			{  
			for (i4_rect_list_class::area_iter c=context.clip.list.begin();
			c!=context.clip.list.end();
			++c)
				{ 
				i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;
				
				if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
				if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
				if (x2>=width())  lx2=width()-1;   else lx2=x2;
				if (y2>=height()) ly2=height()-1;  else ly2=y2;
				
				if (!(lx1>lx2 || ly1>ly2))
					{
					if (x<c->x1)
						{ lx1+=(c->x1-x); x=c->x1; }
					
					if (y<c->y1)
						{ ly1+=(c->y1-y); y=c->y1; }
					
					
					if (x+lx2-lx1+1>c->x2)
						lx2=c->x2-x+lx1; 
					
					if (y+ly2-ly1+1>c->y2)
						ly2=c->y2-y+ly1; 
					
					
					w32 *source=paddr(lx1, ly1);
					w32 *dest=((i4_image32 *)to)->paddr(x + context.xoff,
						y + context.yoff);
					
					int copy_width=((lx2-lx1)+1)*4;
					
					if (!(lx1>lx2 || ly1>ly2))
						{
						to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
						while (ly1<=ly2)
							{              
							memcpy(dest, source, copy_width);
							dest=((w32 *)((w8 *)dest + to->bpl));
							source=((w32 *)((w8 *)source + bpl));
							ly1++;
							}
						}
					}
				}
			}
		
	}


i4_image_class *i4_image32::copy()
{
  i4_image_class *im=i4_create_image(width(), height(), pal);

  memcpy(im->data,data,w*h*4);
  //for (int y=0; y<h; y++)
  //  for (int x=0; x<w; x++)
  //    im->put_pixel(x,y, get_pixel(x,y));

  return im;
}

inline w32 color_dist(w32 c1, w32 c2)
{
  sw32 r1=(c1 & 0xff0000)>>16;
  sw32 r2=(c2 & 0xff0000)>>16;

  sw32 g1=(c1 & 0xff00)>>8;
  sw32 g2=(c2 & 0xff00)>>8;

  sw32 b1=(c1 & 0xff);
  sw32 b2=(c2 & 0xff);

  return (r1-r2)*(r1-r2) + (g1-g2)*(g1-g2) + (b1-b2)*(b1-b2);
}

inline w16 _32_to_16_565(w32 c)
{
  w8 r = (w8)(((c&0xff0000)>>16)>>3);

  w8 g = (w8)(((c&0xff00)>>8)>>2);

  w8 b = (w8)((c&0xff)>>3);

  return (r<<11) | (g<<5) | b;    
}


i4_image_class *i4_image32::quantize(const i4_pal *pal,
                                     w32 skip_colors,
                                     i4_coord x1, i4_coord y1,
                                     i4_coord x2, i4_coord y2)
{

  if (pal->source.pixel_depth!=I4_8BIT)
    i4_error("palette handle, should be 8 bit");

  // first check to make sure the sub image is located within this one
  if (x1<0) x1=0;
  if (y1<0) y1=0;

  if (x2>=width())  
    x2 = width()-1;

  if (y2>=height()) 
    y2 = height()-1;  

  if (x2<x1 || y2<y1) 
    return 0;


  i4_image8 *im8 = new i4_image8(x2-x1+1,
                                 y2-y1+1,
                                 pal);
      

  w32 *pixel32 = (w32 *)((w8 *)data + x1*4 + y1*bpl);
  w8 *pixel8 = (w8 *)im8->data;

  w32 skip_32 = bpl-(x2-x1+1);   // pixels to skip per line
  w32 x,y;

  w8  closest_color=0;
  w32 closest_distance,
      distance,
      *color_index;

  w32 *pal_data=pal->source.lookup;

  
  // maps 16 bits color space into closest 8 bit color
  w8 *lookup_table=(w8 *)I4_MALLOC(0x10000, "lookup");
   
  // indicates if above color has been calculated yet
  w8 *table_calced=(w8 *)I4_MALLOC(0x10000, "table_calced");
  
  memset(table_calced,0,0x10000);     // initially no mappings are calculated

  for (y=y2-y1+1; y; y--)
  {
    for (x=x2-x1+1; x; x--)
    {
      i4_color color=*pixel32;


      w16 c16 = _32_to_16_565(color);
      if (table_calced[c16])           // have we found the closest color to this yet?
        *pixel8=lookup_table[c16];
      else
      {
        // find the closest color to this pixel
        color_index = pal_data+skip_colors;
        closest_distance = 0xffffffff;

        for (w32 c=skip_colors; c<256; c++)
        {
          distance=color_dist(*color_index,color);
          if (distance<closest_distance)
          {
            closest_distance=distance;
            closest_color=(w8)c;
          }
          color_index++;
        }

        table_calced[c16]=1;
        lookup_table[c16]=closest_color;
        *pixel8=closest_color;
      }

      ++pixel32;
      ++pixel8;
    }

    pixel32+=skip_32;
  }

  i4_free(lookup_table);
  i4_free(table_calced);

  return im8;
}

// IMAGE24.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


i4_color i4_image24::get_pixel(i4_coord x, i4_coord y)
{    
  return i4_pal_man.convert_to_32(*paddr(x,y), pal);
}
  
void i4_image24::put_pixel(i4_coord x, i4_coord y, w32 color)
{
  //i4_color c=i4_pal_man.convert_24_to(color, &pal->source)
  w8* a=(w8*)paddr(x,y);//Wir müssen (leider) byteweise zugreifen
  *a=(w8)(color);//r
  a++;
  *a=(w8)(color>>8);//g
  a++;
  *a=(w8)(color>>16);//b
  /*char *d=(char*)data;
  w32 b=bpl;
  __asm
	  {
	  push ecx;
	  movzx eax,y;
	  mul b;
	  mov ecx,3
	  mov ebx,eax;
	  movzx eax,x;
	  mul ecx;//Uhh, teuer...
	  add ebx,eax;
	  add ebx,d;
	  mov ecx,dword ptr [ebx];
	  and ecx,0xff;
	  mov edx,color;
	  shl edx,8;
	  or ecx,edx;
	  mov dword ptr[ebx],ecx;
	  pop ecx;
	  }*/

}

i4_image24::i4_image24(w16 _w, w16 _h, const i4_pal *_pal)
{
  w=_w;
  h=_h;
  bpl=_w*3;
  set_pal(_pal);
  data=new w8[(w*h*3)+4];
  dont_free_data=i4_F;
}

i4_image24::i4_image24(w16 _w, w16 _h, const i4_pal *_pal,
                       void *_data, int _bpl)
{
  data=(w8*)_data;
  bpl=_bpl;
  pal=_pal;
  w=_w;
  h=_h;

  dont_free_data=i4_T;
}


i4_image24::~i4_image24()
{
  if (!dont_free_data)
    delete data;
}
/*  
inline w32 color_dist(w32 c1, w32 c2)//Gleich wie bei 32 Bit
{
  sw32 r1=(c1 & 0xff0000)>>16;
  sw32 r2=(c2 & 0xff0000)>>16;

  sw32 g1=(c1 & 0xff00)>>8;
  sw32 g2=(c2 & 0xff00)>>8;

  sw32 b1=(c1 & 0xff);
  sw32 b2=(c2 & 0xff);

  return (r1-r2)*(r1-r2) + (g1-g2)*(g1-g2) + (b1-b2)*(b1-b2);
}
*/
inline w16 _24_to_16_565(w32 c)
{
  w8 r = (w8)(((c&0xff0000)>>16)>>3);

  w8 g = (w8)(((c&0xff00)>>8)>>2);

  w8 b = (w8)((c&0xff)>>3);

  return (r<<11) | (g<<5) | b;    
}

i4_image_class *i4_image24::copy()
{
  i4_image_class *im=i4_create_image(width(), height(), pal);

  memcpy(im->data,data,w*h*3);
  //for (int y=0; y<h; y++)
  //  for (int x=0; x<w; x++)
  //    im->put_pixel(x,y, get_pixel(x,y));

  return im;
}

typedef unsigned char w24[3];
i4_image_class *i4_image24::quantize(const i4_pal *pal,
                                     w32 skip_colors,
                                     i4_coord x1, i4_coord y1,
                                     i4_coord x2, i4_coord y2)
{

  if (pal->source.pixel_depth!=I4_8BIT)
    i4_error("palette handle, should be 8 bit");

  // first check to make sure the sub image is located within this one
  if (x1<0) x1=0;
  if (y1<0) y1=0;

  if (x2>=width())  
    x2 = width()-1;

  if (y2>=height()) 
    y2 = height()-1;  

  if (x2<x1 || y2<y1) 
    return 0;


  i4_image8 *im8 = new i4_image8(x2-x1+1,
                                 y2-y1+1,
                                 pal);
      

  w32 *pixel24 = (w32 *)((w8 *)data + x1*3 + y1*bpl);
  w8 *pixel8 = (w8 *)im8->data;

  w32 skip_24 = bpl-(x2-x1+1);   // pixels to skip per line
  w32 x,y;

  w8  closest_color=0;
  w32 closest_distance,
      distance,
      *color_index;

  w32 *pal_data=pal->source.lookup;

  
  // maps 16 bits color space into closest 8 bit color
  w8 *lookup_table=(w8 *)I4_MALLOC(0x10000, "lookup");
   
  // indicates if above color has been calculated yet
  w8 *table_calced=(w8 *)I4_MALLOC(0x10000, "table_calced");
  
  memset(table_calced,0,0x10000);     // initially no mappings are calculated

  for (y=y2-y1+1; y; y--)
  {
    for (x=x2-x1+1; x; x--)
    {
      i4_color color=(w32)*pixel24;


      w16 c16 = _24_to_16_565(color);
      if (table_calced[c16])           // have we found the closest color to this yet?
        *pixel8=lookup_table[c16];
      else
      {
        // find the closest color to this pixel
        color_index = pal_data+skip_colors;
        closest_distance = 0xffffffff;

        for (w32 c=skip_colors; c<256; c++)
        {
          distance=color_dist(*color_index,color);
          if (distance<closest_distance)
          {
            closest_distance=distance;
            closest_color=(w8)c;
          }
          color_index++;
        }

        table_calced[c16]=1;
        lookup_table[c16]=closest_color;
        *pixel8=closest_color;
      }

      //++pixel24;
	  w32 help=(w32)pixel24;
	  help+=3;
	  pixel24=(w32*)help;
      ++pixel8;
    }

    pixel24+=skip_24;
  }

  i4_free(lookup_table);
  i4_free(table_calced);

  return im8;
}
