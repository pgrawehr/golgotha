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
 
#include "pch.h"
#include "image/color.h"
#include "image/image8.h"
#include "file/file.h"
#include "error/error.h"
#include "image/context.h"
#include "palette/pal.h"
//#include <memory.h>


i4_bool fli_read_rle(w8 *im, w32 w, w32 h, i4_file_class *fp)
//{{{
{
  w8 *line,c;
  sw8 size;

  for (w32 y=0; y<h; y++)
  {   
    line=im;
    im+=w;
    w8 tpackets=fp->read_8();
    for (w32 k=0; k<tpackets; k++)
    {
      size=fp->read_8();
      if (size>0)
      {
        c=fp->read_8();
        while (size)
        {
          *line=c;
          ++line;
          size--;
        }
      } else      
      {
        if (fp->read(line,-size)!=(w32)(-size))
          return i4_F;
        else line+=-size;
      }      
    }
  }
  return i4_T;
}
//}}}


i4_bool fli_read_lc(w8 *prev, w8 *im, w32 w, w32 h, i4_file_class *fp)
//{{{
{
  w32 x,y;
  w16 skip_lines,change_lines;

  w8 *line,*prev_line,tpackets,c;
  sw8 size;

  
  skip_lines=fp->read_16();
  memcpy(im,prev,skip_lines*w);
  im+=skip_lines*w;
  prev+=skip_lines*w;
  
  change_lines=fp->read_16();
  for (y=0; y<change_lines; y++)
  {
    tpackets=fp->read_8();
    line=im;
    prev_line=prev;
    im+=w;
    prev+=w;

    if (tpackets==0)
      memcpy(line,prev_line,w);
    else
    {
      for (x=0; x<tpackets; x++)
      {
        w8 skip=fp->read_8();
        memcpy(line,prev_line,skip);
        line+=skip;
        prev_line+=skip;

        size=fp->read_8();
        if (size<0)
        {
          c=fp->read_8();
          while (size)
          {
            *line=c;
            ++line;
            ++prev_line;
            size++;
          }
        } else      
        {
          if (fp->read(line,size)!=(w32)size)
            return i4_F;
          line+=size;
          prev_line+=size;
        }

      }
    }

  }

  return i4_T;
}
//}}}


i4_bool fli_read_ss2(w8 *prev, w8 *im, w32 w, w32 h, i4_file_class *fp)
//{{{
{
  w32 x,y;
  w16 skip_lines,change_lines;

  w16 *line,*prev_line,code_word,c;
  sw8 size;

  change_lines=fp->read_16();
  for (y=0; y<change_lines; y++)
  {
    do
    {
      code_word=fp->read_16();
      switch(code_word>>14)
      {
        case 0:
          break;
        case 1:
          return i4_F;

        case 2:
        {
          *(im+w-1) = code_word & 0xff;
        } break;
        
        case 3:
          skip_lines = -code_word;
          im += w*skip_lines;
          prev += w*skip_lines;
          break;
      }
    } while (code_word & 0x8000);

    line=(w16*)im;
    prev_line=(w16*)prev;

    im+=w;
    prev+=w;

    if (code_word==0)
      memcpy(line,prev_line,w-1);
    else
    {
      for (x=0; x<code_word; x++)
      {
        w8 skip=fp->read_8();
        memcpy(line,prev_line,skip);
        line+= (skip>>1);
        prev_line+= (skip>>1);

        size=fp->read_8();
        if (size<0)
        {
          c=fp->read_16();
          while (size)
          {
            *line=c;
            ++line;
            ++prev_line;
            size++;
          }
        } else      
        {
          if (fp->read(line,size*2)!=(w32)size*2)
            return i4_F;
          line+=size;
          prev_line+=size;
        }

      }
    }

  }

  return i4_T;
}
//}}}


//hacked on 5-23-97, loads a series of .pcx files
/*i4_image_class **i4_load_fli(i4_file_class *fp, w32 &frames)
{
return NULL;
}
*/

i4_image_class **i4_load_fli(i4_file_class *fp, w32 &frames)
//{{{
{
  //{{{ FLI chunk signatures
  enum 
  {
    FLI_COLOR_MAP256=4,
    FLI_SS2=7,
    FLI_COLOR_MAP=11,
    FLI_COMPRESSED_LINE=12,
    FLI_BLACK=13,
    FLI_RLE=15,
    FLI_RAW=16,
    FLI_PSTAMP=18
  } ;
  //}}}

  w32 offset,shift=0;
  w32 size,frame_size,k;
  w16 signature,
    depth,
    w, h,
    chunk_type,
    tchunks;
    
  w8 buffer[114];
  w32 chunk_size,
    frame_on;

  int error = 0;

  size=fp->read_32();
  signature=fp->read_16();

  frames=fp->read_16();
  w=fp->read_16();
  h=fp->read_16();

  depth=fp->read_16();
  if (depth!=8)
    return 0;

  if (signature==0xaf11)        // FLI signature
  {
    if (fp->read(buffer,114)!=114)
      return 0;
  }
  else if (signature==0xaf12)   // FLC signature
  {
    // read offset to first frame chunk to skip prefixes, etc.
    fp->seek(80);
    offset = fp->read_32();
    // go to first frame
    fp->seek(offset);
  }
  else
    return 0;

  //i4_pal_handle_class pal;
  i4_pixel_format f;
  
  f.default_format();
  f.pixel_depth=I4_8BIT;
  f.lookup=(w32*)I4_MALLOC(256*4,"Color Palette");
  for (int i=0 ; i<256 ; i++)
	  {
	  f.lookup[i]=(i<<24)+(i<<16)+(i<<8)+i;
	  }
  i4_lookup_pal pal(&f);
  
  i4_image8 **im;
  im=(i4_image8 **) new w32[frames];//(i4_unmatched_image8 **)i4_malloc(sizeof(i4_unmatched_image8 *)*frames,"fli frame pointers");
  
  for (frame_on=0; frame_on<frames && !error; frame_on++)
  {
    frame_size=fp->read_32();
    if (fp->read_16()!=0xf1fa)
      error = 1;
    else
    {
      im[frame_on]=new i4_image8(w,h,&pal);
      tchunks=fp->read_16();
      fp->read(buffer,8);

      for (k=0; k<tchunks && !error; k++)
      {
        chunk_size=fp->read_32();
        chunk_type=fp->read_16();
        switch ((w8)(chunk_type&0xff))
        {
          case FLI_COMPRESSED_LINE :
            //{{{
            if (!fli_read_lc((w8*)im[frame_on-1]->data/*local_sub_data(0,0)*/,
                             (w8*)im[frame_on]->data,
                             w,h,
                             fp))
              error = 1;
            break;
            //}}}
          case FLI_RAW :
            //{{{
            if (fp->read(im[frame_on]->data,64000)!=64000)
              error = 1;
            break;
            //}}}
          case FLI_RLE :
            //{{{
            if (!fli_read_rle((w8*)im[frame_on]->data, w,h, fp))
              error = 1;
            break;
            //}}}
          case FLI_COLOR_MAP :
            //{{{
            shift = 2;
            //}}}
          case FLI_COLOR_MAP256 :
            //{{{
          {
            w32 pald[256],*p;

            p=pald;
            memset(pald,0,sizeof(pald));
            w32 i,j;
            w16 change;
            w8 trip[3];
            w16 count=fp->read_16();
            for (i=0; i<count; i++)
            {
              p+=fp->read_8();
              change=fp->read_8();
              if (!change) 
                change=256;
              for (j=0; j<change; j++)
              {
                fp->read(trip,3);
                if (p>pald+256)
                  i4_error("fli gone crazy");

                *p=(trip[0] << (16+shift))|
                  (trip[1] <<  (8+shift))|
                  (trip[2] <<  shift);
                p++;

              }
              //pal=i4_pal_man.register_pal(I4_8BIT,pald);
			  memcpy(f.lookup,pald,256*4);
			  //i4_pal_man.register_pal(&f);
			  memcpy(pal.source.lookup,f.lookup,256*4);
			  i4_pal_man.register_pal(&pal.source);
            }
            shift = 0;
          } break;
            //}}}
          case FLI_BLACK :
            //{{{
          {
            i4_draw_context_class context(0,0,w-1,h-1);
            im[frame_on]->clear(0,context);
          } break;
            //}}}
          case FLI_SS2 :
            //{{{
            if (!fli_read_ss2((w8*)im[frame_on-1]->data,
                              (w8*)im[frame_on]->data,
                              w,h,
                              fp))
              error = 1;
            break;
            //}}}
          default :
            //{{{
            fp->seek(fp->tell()+chunk_size - 6);
            //}}}
        }
      }
    }
  }

  if (error)
  {
    for (w32 j=0; j<frame_on; j++)
      delete im[j];
    i4_free(im);
    return 0;
  }

  for (k=0; k<frames; k++)
    im[k]->set_pal(&pal);
  delete f.lookup;
  return (i4_image_class **)im;
}
//}}}




//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}

