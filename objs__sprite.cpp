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
 

#include "objs/sprite.h"
#include "error/error.h"
#include "load3d.h"
#include "string/string.h"
#include "error/alert.h"
#include "obj3d.h"
#include "objs/sprite_id.h"
#include "global_id.h"
#include "render/r1_api.h"
#include "render/tmanage.h"

extern i4_grow_heap_class *g1_object_heap;

g1_sprite_list_class g1_sprite_list_man;

// todo : add a sprite loader instead of mouching the load3d
// its mooching, jonathan. -trey

void g1_sprite_list_class::load(r1_render_api_class *tmap)
{
  i4_const_str *sprites=i4_string_man.get_array("sprite_array");
  w32 count;

  g1_quad_object_loader_class loader(g1_object_heap);

  for (count=0; !sprites[count].null(); count++)
  {  
    I4_ASSERT(count<=G1_MAX_SPRITES, "increase G1_MAX_SPRITES");

    i4_str *name=i4gets("spr_fmt").sprintf(200, &sprites[count]);
    i4_file_class *in_file=i4_open(*name);
    
    
    if (!in_file)
      i4_alert(i4gets("file_missing"),200,sprites+count);
    else
    {
      g1_loader_class *fp=g1_open_save_file(in_file);
      if (fp)
      {
        g1_quad_object_class *tmp=loader.load(fp, sprites[count], tmap->get_tmanager());
        tmp->scale(0.1);

        array[count].add_x=tmp->animation[0].vertex[0].v.x;
        array[count].add_y=tmp->animation[0].vertex[0].v.y;
        array[count].texture       = tmp->quad[0].material_ref;
        array[count].texture_scale = tmp->quad[0].texture_scale;
        array[count].extent        = tmp->extent;                
        
        r1_texture_handle material=tmp->quad[0].material_ref;
        int alen=r1_render_api_class_instance->get_tmanager()->get_animation_length(material);
        array[count].num_animation_frames = alen;

        delete fp;
      }
      else
        i4_alert(i4gets("old_model_file"),200,sprites+count);
    }
    delete name;
  }

  if (count!=G1_SPRITE_LAST)
  {
    i4_alert(i4gets("sprite_num"),100,count,G1_SPRITE_LAST);
    i4_error("");
  }

  i4_free(sprites);
}
