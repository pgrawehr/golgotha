/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "font/font.h"
#include "math/num_type.h"


i4_bool i4_font_class::get_line(const i4_const_str &string,
                                i4_const_str::iterator &s, i4_const_str::iterator &e, 
                                int &t_words, int &t_spaces,
                                int &chars_width, int line_width)
{
  t_words=0;
  chars_width=0;
  t_spaces=0;


  int x=0, last_word_width=0, last_word_spaces=0;
  if (line_width<0) line_width=1000000;
  int in_a_word=0;
  i4_const_str::iterator word_start=s;

  while (e!=string.end() && e.get().ascii_value()!='\n')
  {
    i4_char c=e.get();

    int c_width=width(c);

    if (x+c_width>line_width)      // we exceeded the space available for this line
    {
      if (in_a_word)
      {
        chars_width=last_word_width;
        e=word_start;
        t_spaces=last_word_spaces;
      }

      return t_words ? i4_T : i4_F;
    }


    x+=c_width;

    if (!c.is_space())
    {
      if (!in_a_word)
      {
        last_word_width=chars_width;
        word_start=e;
       
      }

      in_a_word=1;
      chars_width+=c_width;
    }
    else
    {
      if (in_a_word)
      {
        in_a_word=0;
        t_words++;
        last_word_spaces=t_spaces;
      }

      t_spaces++;
    }


    ++e;
  }

  if (in_a_word)
    t_words++;

  if (e!=string.end() && e.get().ascii_value()=='\n')
    ++e;

  return t_words ? i4_T : i4_F;

}

void i4_font_class::put_line(i4_image_class *screen,
                             i4_const_str::iterator s, i4_const_str::iterator e, 
                             int x, int y,
                             float space_width,
                             i4_draw_context_class &context)
{
  float fx=(float)x;
  while (s!=e)
  {
    put_character(screen, (short)i4_f_to_i(fx), (short)y, s.get(), context);
    if (s.get().is_space())
      fx+=space_width;
    else
      fx+=width(s.get());
    ++s;
  }
}

void i4_font_class::put_paragraph(i4_image_class *screen, 
                                  sw16 x, sw16 y, 
                                  const i4_const_str &string, 
                                  i4_draw_context_class &context,
                                  int space_between_lines,
                                  justification_type justification,
                                  int line_width_in_pixel)
{

  i4_const_str::iterator start=string.begin(),end=string.begin();
  int t_words, chars_width;

  if (line_width_in_pixel==-1 && justification!=LEFT)
    i4_error("cannot do justification without line width");

  int space_width=width(i4_const_str(" "));

  int t_spaces=0;
  while (get_line(string, start,end, t_words, t_spaces, chars_width, line_width_in_pixel))
  {
    switch (justification)
    {
      case LEFT : put_line(screen, start, end, x, y, (float)space_width, context); break;
      case RIGHT : 
        put_line(screen, start, end, 
                 x+line_width_in_pixel-(chars_width + space_width * t_spaces), 
                 y, (short)space_width, context); break;

      case CENTER :
        put_line(screen, start, end, 
                 x+line_width_in_pixel/2-(chars_width + space_width * t_spaces)/2, 
                 y, (short)space_width, context); break;

      case FULL :
        float sw;
        if (t_words>1)
          sw=(line_width_in_pixel-chars_width)/(float)t_spaces;
        else
          sw=0;

        put_line(screen, start, end, x, y, sw, context);
        break;
    }

    if (end!=string.end() && end.get().is_space())
    {
      start=end;
      ++start;
      end=start;
    }
    else start=end;

    y+=largest_height() + space_between_lines;
  }

}

// PLAIN.CPP
    
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "font/plain.h"
#include <string.h>

i4_plain_font_class::~i4_plain_font_class()
{
  delete bitmap;
}

i4_plain_font_class::i4_plain_font_class(i4_image_class *bitmap)
{
  i4_plain_font_class::bitmap=bitmap->copy();
  w=bitmap->width()/32;
  h=bitmap->height()/8;
}

void i4_plain_font_class::set_color(i4_color color)
{
  /*  w32 p[256];
  memset(p,0,sizeof(p));
  w8 r,g,b;

  i4_pixel_format *f=&pal.pal->source;
  w32 rgb=(((color & f->red_mask)>>f->red_shift)<<(16+(8-f->red_bits))) |
    (((color & f->green_mask)>>f->green_shift)<<(8+(8-f->green_bits))) |
    (((color & f->blue_mask)>>f->blue_shift)<<(16+(8-f->blue_bits)));

  p[1]=rgb;

  i4_pixel_format fmt;
  fmt.pixel_depth=I4_8BIT;
  fmt.lookup=p;

  bitmap->set_pal(i4_pal_man.register_pal(&fmt));
  return i4_T; */
}

void i4_plain_font_class::put_string(i4_image_class *screen, 
                                     sw16 x, sw16 y, 
                                     const i4_const_str &string, 
                                     i4_draw_context_class &context)
{
  if (!string.null())
  {
    i4_const_str::iterator p=string.begin();

    while (p!=string.end())
    {
      char ch=(char)p.get().value();
      i4_coord x1=((ch)%32)*w;
      i4_coord y1=((ch)/32)*h;

      bitmap->put_part_trans(screen,x,y,x1,y1,x1+w-1,y1+h-1,0,context);
      x+=w;

      ++p;
    }
  }
} 

void i4_plain_font_class::put_character(i4_image_class *screen, 
                                        sw16 x, sw16 y, 
                                        const i4_char &c, 
                                        i4_draw_context_class &context)
{
  char ch=(char)c.value();
  i4_coord x1=((ch)%32)*w;
  i4_coord y1=((ch)/32)*h;
  bitmap->put_part_trans(screen,x,y,x1,y1,x1+w-1,y1+h-1,0,context);
}

// ANTI_PROP.CPP

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "font/anti_prop.h"
#include "image/image.h"
#include "palette/pal.h"
#include "image/context.h"

/*  Printable english characters 
  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z   {   |   }

  "
  */

static inline int i4_is_black(w32 c)
{
  if ((c&0xff000000)==0)
    return 1;
  else return 0;
}

static inline int i4_scanline_empty(i4_image_class *im, int y)
{
  for (int x=0; x<im->width(); x++)  
    if (!i4_is_black(im->get_pixel(x,y)))
      return 0;

  return 1;
}



static inline int i4_colum_empty(i4_image_class *im, int x)
{
  for (int y=0; y<im->height(); y++)
    if (!i4_is_black(im->get_pixel(x,y)))
      return 0;

  return 1;
}


static inline int i4_image_has_alpha(i4_image_class *im)
{
  for (int y=0; y<im->height(); y++)
    for (int x=0; x<im->width(); x++)   
    {
      w32 cl=im->get_pixel(x,y);
      if (cl>>24)
        i4_warning("alpha %d", cl>>24);
    }

  return 1;
}



class i4_anti_image_class : public i4_image_class
{
public:

  w32 current_color;


  virtual i4_image_class *copy() 
  {
    i4_anti_image_class *i=new i4_anti_image_class(w,h);
    memcpy(i->data, data, w*h);
    return i;
  }
  
  virtual w16 width() { return w; }
  virtual w16 height() { return h; }

  void set_color(w32 c) { current_color=c; }

  virtual void put_pixel(i4_coord x, i4_coord y, w32 color)
  {
    *((w8 *)data+x+y*w)= (w8)(color>>24);
  }

  virtual w32  get_pixel(i4_coord x, i4_coord y)
  {
    return ((*((w8 *)data+x+y*w))<<24) | current_color;
  }

  i4_anti_image_class(w16 _w, w16 _h)
  {
    w=_w;
    h=_h;
    bpl=w;
    i4_pixel_format fmt;
    fmt.default_format();
    pal=i4_pal_man.register_pal(&fmt);
    data=(w8*)I4_MALLOC(w*h,"");
    
    current_color=0xffffff;
  }

  ~i4_anti_image_class()
  {
    i4_free(data);
  }

};

void i4_anti_proportional_font_class::put_string(i4_image_class *screen, 
                                                 sw16 x, sw16 y, 
                                                 const i4_const_str &string, 
                                                 i4_draw_context_class &context)
{
  if (!string.null())
  {
    for (i4_const_str::iterator p=string.begin(); p!=string.end(); ++p)
    {
      char ch=p.get().ascii_value();
	  if (ch=='\n')
	  {
		  y+=height(ch);
		  x=0;
	  }
      else if (offsets[ch])
      {
        aim->put_part(screen, x,y, offsets[ch], 0, offsets[ch]+widths[ch]-1, aim->height()-1, 
                      context);

        x+=widths[ch];
      }
      else x+=4;
    }
  }
}


void i4_anti_proportional_font_class::put_character(i4_image_class *screen, 
                                                    sw16 x, sw16 y, 
                                                    const i4_char &c, 
                                                    i4_draw_context_class &context)
{
  int ch=c.ascii_value();
  int xo=offsets[ch];
  if (xo)
    aim->put_part(screen, x,y, xo,0,xo+widths[ch]-1,aim->height(), context);
}


i4_anti_proportional_font_class::i4_anti_proportional_font_class(i4_image_class *im, 
                                                                 int start_ch)
{
  memset(offsets, 0, sizeof(offsets));
  longest_w=2;

  
  //  i4_image_has_alpha(im);

  int y_top=0, y_bottom=im->height()-1, x1=0,x2,i;
  for (i=0; i<256; i++)
    widths[i]=4;

  while (y_top<=y_bottom && i4_scanline_empty(im, y_top))
    y_top++;

  if (y_top==y_bottom-1)
    i4_error("image is empty");

  while (i4_scanline_empty(im, y_bottom))
    y_bottom--;

  int char_on=start_ch;
  int x=1;

  do
  {
    while (x1!=im->width() && i4_colum_empty(im, x1)) x1++;
    if (x1<im->width())
    {
      x2=x1+1;
      while (x2!=im->width() && 
             !(i4_colum_empty(im, x2) && i4_colum_empty(im, x2+1))) x2++;
      widths[char_on]=x2-x1+1;
      if (widths[char_on]>longest_w)
        longest_w=widths[char_on];

      offsets[char_on]=x;
      x+=x2-x1+1;
      x1=x2+2;
    }     
    char_on++;
  } while (x1<im->width());

  h=y_bottom-y_top+1;
  aim=new i4_anti_image_class(x, h);

  x1=0; x=1;
  i4_draw_context_class context(0,0, aim->width()-1, aim->height()-1);
  char_on=start_ch;
  do
  {
    while (x1!=im->width() && i4_colum_empty(im, x1)) x1++;
    if (x1<im->width())
    {
      x2=x1+1;
      while (x2!=im->width() && 
             !(i4_colum_empty(im, x2) && i4_colum_empty(im, x2+1))) x2++;

      im->i4_image_class::put_part(aim, x, 0, x1, y_top, x2, y_bottom, context);
      x+=x2-x1+1;
      x1=x2+2;
    }     
    char_on++;
  } while (x1<im->width());
}


void i4_anti_proportional_font_class::set_color(i4_color color)
{
  aim->set_color(color);
}

i4_anti_proportional_font_class::~i4_anti_proportional_font_class() 
{ 
  delete aim; 
}
