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
 

#include "quantize/histogram.h"
#include "error/error.h"
#include "image/image.h"
#include <memory.h>
#include "palette/pal.h"
#include "file/file.h"

//this class needs testing
i4_histogram_class::i4_histogram_class()
{
  memset(reference,0,sizeof(reference));
  memset(counts,0,sizeof(counts));
  tcolors=0;
  total_pixels=0;
}

void i4_histogram_class::add_image_colors(i4_image_class *im, int counts_per_pixel)
{
  if (im->get_pal()->source.pixel_depth!=I4_32BIT)
  {
    i4_warning("add_image_colors : image type not supported");
    return ;
  }
  
  

  w32 loop_count=im->width()*im->height();

  // this will iterate through all the pixels
  w32 *pixel=(w32 *)im->data;
  
  for (; loop_count; loop_count--)
  {
    i4_color c=*pixel;

    // convert the color to 16 bit
    c=  
      (((c&0xff0000) >> (16+3)) << (6+5)) |
      (((c&0x00ff00) >> (8+2))  << (5+0)) |
      (((c&0x0000ff) >> (0+3))  << (0+0)) ;

    increment_color((short)c,counts_per_pixel);

    // move to the next pixel in the image
    ++pixel;
  }

}

void i4_histogram_class::save(i4_file_class *fp)
{
  w32 i;
  fp->write_32(tcolors);
  fp->write_32(total_pixels);

  for (i=0; i<HIST_SIZE; i++)
  {
    reference[i]=s_to_lsb(reference[i]);
    counts[i]=l_to_lsb(counts[i]);
  }

  fp->write(reference, sizeof(reference));
  fp->write(counts, sizeof(counts));

  for (i=0; i<HIST_SIZE; i++)
  {
    reference[i]=s_to_lsb(reference[i]);
    counts[i]=l_to_lsb(counts[i]);
  }
}

void i4_histogram_class::load(i4_file_class *fp)
{
  w32 i;
  tcolors=fp->read_32();
  total_pixels=fp->read_32();

  fp->read(reference, sizeof(reference));
  fp->read(counts, sizeof(counts));

  for (i=0; i<HIST_SIZE; i++)
  {
    reference[i]=s_to_lsb(reference[i]);
    counts[i]=l_to_lsb(counts[i]);
  }
}

