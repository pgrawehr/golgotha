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
 

#include "arch.h"
#include "quantize/median.h"
#include "quantize/histogram.h"
#include <stdlib.h>
#include "memory/malloc.h"

// counts for each color, global so that it can be accessed by qsort function
static w32 *counts;   


static int red_compare(const void *a, const void *b)
{
  w16 color1=(w16)counts[*((w16 *)a)],
      color2=(w16)counts[*((w16 *)b)];

  return ((sw32) ((color1 & (  (1 | 2 | 4 | 8 | 16) << 11))>>11))-
    ((sw32) ((color2 & (  (1 | 2 | 4 | 8 | 16) << 11))>>11));
}


static int green_compare(const void *a, const void *b)
{
  w16 color1=(w16)counts[*((w16 *)a)],
      color2=(w16)counts[*((w16 *)b)];

  return ((sw32) (color1 & (  (1 | 2 | 4 | 8 | 16 | 32) << 5)))-
         ((sw32) (color2 & (  (1 | 2 | 4 | 8 | 16 | 32) << 5)));
}


static int blue_compare(const void *a, const void *b)
{
  w16 color1=(w16)counts[*((w16 *)a)],
      color2=(w16)counts[*((w16 *)b)];

  return ((sw32) (color1 & (  (1 | 2 | 4 | 8 | 16) << 0)))-
         ((sw32) (color2 & (  (1 | 2 | 4 | 8 | 16) << 0)));
}


struct box
{
  w32 index;
  w32 colors;
  w32 sum;
};

static int box_compare(const void *a, const void *b)
{
  return ((box *)b)->sum-((box *)a)->sum;
}

inline void _16_to_rgb(w32 rgb, w8 &r, w8 &g, w8 &b)
{
  b=(w8)(rgb & (1 | 2 | 4 | 8 | 16))<<3;
  rgb>>=5;

  g=(w8)(rgb & (1 | 2 | 4 | 8 | 16 | 32))<<2;
  rgb>>=6;

  r=(w8)(rgb & (1 | 2 | 4 | 8 | 16))<<3;
}

void i4_median_cut(i4_histogram_class *hist, 
                   w32 skip_colors,
                   w32 t_colors,
                   w32 *return_palette)
                                  
{
  enum { HIST_SIZE=0x10000,  // 16 bit histogram table
         MAX_COLORS=256 };
  
  counts=hist->counts; 

  box *box_list=(box *)I4_MALLOC(sizeof(box) * MAX_COLORS, "boxes");

  // setup the initial box
  w32 total_boxes    = 1;
  box_list[0].index  = 0;
  box_list[0].colors = hist->tcolors;
  box_list[0].sum    = hist->total_pixels;

  w32 box_index;
  w32 loop_count;

  // split boxes until we have all the colors
  while ( total_boxes < t_colors-skip_colors )
  {
    // Find the first splittable box.
    for ( box_index = 0; box_index < total_boxes; ++box_index )
      if (box_list[box_index].colors >= 2)
        break;

    if ( box_index == total_boxes)
      break;	/* ran out of colors! */


    w32 start = box_list[box_index].index;
    w32 end   = start + box_list[box_index].colors;

    w8  min_r=0xff, max_r=0x00,
        min_g=0xff, max_g=0x00,
        min_b=0xff, max_b=0x00,
        r,g,b;

    
    // now find the minimum and maximum r g b values for this box
    for (loop_count = start; loop_count<end; loop_count++)
    {
      _16_to_rgb(hist->reference[loop_count],r,g,b);

      if (r>max_r) max_r=r;
      if (r<min_r) min_r=r;
      
      if (g>max_g) max_g=g;
      if (g<min_g) min_g=g;

      if (b>max_b) max_b=b;
      if (b<min_b) min_b=b;
    }


    // Find the largest dimension, and sort by that component. 
    if (((max_r - min_r) >= (max_g - min_g)) && ((max_r - min_r) >= (max_b - min_b)))
      qsort(hist->reference + start,
          end-start,             // total elements to sort
          sizeof(w16),
          red_compare);
    else if ( (max_g - min_g) >= (max_b - min_b) )
      qsort(hist->reference + start,
          end-start,             // total elements to sort
          sizeof(w16),
          green_compare);
    else
      qsort(hist->reference + start,
          end-start,             // total elements to sort
          sizeof(w16),
          blue_compare);



    // now find the division which closest divides into an equal number of pixels
    w32 low_count= counts[hist->reference[start]];
    w32 mid_number=box_list[box_index].sum/2;

    for (loop_count = start+1; loop_count<end-1 && low_count<mid_number ; loop_count++)
      low_count+=counts[hist->reference[loop_count]];


    // now split the box
    box_list[total_boxes].index  = loop_count;
    box_list[total_boxes].colors = end-loop_count;
    box_list[total_boxes].sum    = box_list[box_index].sum-low_count;
    total_boxes++;

    box_list[box_index].colors= loop_count-start;
    box_list[box_index].sum   = low_count;

    // sort to bring the biggest boxes to the top
    qsort(box_list, total_boxes, sizeof(box), box_compare );    

    /*    for (int z=0;z<total_boxes;z++)
    {
      printf("box #%d : index = %d, colors= %d, sum=%d\n",z, 
             box_list[z].index, box_list[z].colors, box_list[z].sum);
    } */

  }


  // we should have 256 boxes, we now need to choose a color for each box
  // we do this by averaging all the colors within the box
  w32 r_tot, g_tot, b_tot;
  for (box_index = 0; box_index<total_boxes-skip_colors; box_index++)
  {
    r_tot = g_tot = b_tot = 0;

    w32 start = box_list[box_index].index;
    w32 end   = start + box_list[box_index].colors;

    w8 r,g,b;
    for (loop_count = start; loop_count < end; loop_count++)
    {
      _16_to_rgb(hist->reference[loop_count],r,g,b);
      
      r_tot+=r;
      g_tot+=g;
      b_tot+=b;
    }

    r_tot/=(end-start);
    g_tot/=(end-start);
    b_tot/=(end-start);

    return_palette[box_index+skip_colors] = (r_tot<<16)|
      (g_tot<<8) |
      b_tot;
  }

  for (;box_index + skip_colors<t_colors; box_index++)
    return_palette[box_index+skip_colors] = 0;


  i4_free(box_list);

}

