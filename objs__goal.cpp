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
 

#include "objs/goal.h"
#include "object_definer.h"
#include "lisp/li_init.h"
#include "lisp/li_class.h"
#include "li_objref.h"
#include "map_man.h"
#include "map.h"
#include "saver.h"

#include <stdio.h>

static g1_object_type stank;

static void g1_goal_init()
{
  stank = g1_get_object_type("stank");
}

g1_object_definer<g1_goal_class> 
g1_goal_def("goal", g1_object_definition_class::EDITOR_SELECTABLE, g1_goal_init);

static li_int_class_member type("type");
static li_int_class_member ticks_to_think("ticks_to_think");
static li_int_class_member think_delay("think_delay");

static li_float_class_member range("range");

g1_goal_class::g1_goal_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  draw_params.setup("lightbulb");
}

void g1_goal_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);
}

void g1_goal_class::draw(g1_draw_context_class *context)
{  
  g1_editor_model_draw(this, draw_params, context);
}

void g1_goal_class::think()
{
  if (ticks_to_think())
    ticks_to_think()--;
  else
  {
//#if 0
//    for (int i=0; i<G1_MAX_PLAYERS; i++)
//      power[i] = 0;
//
//    ticks_to_think() = think_delay();

//    g1_map_class::range_iterator p;
    
//    p.begin(x,y,range());
//    p.mask(g1_object_class::DANGEROUS);
    
//    while (!p.end())
//#else
    g1_object_class *objs[1024], **p;

    int num = g1_get_map()->get_objects_in_range(x, y, range(), 
                                                 objs, 1024, g1_object_class::DANGEROUS);
    p = objs;
    for (int i=0; i<num; i++)
//#endif
    {
      g1_object_class *o = *p;
      int n= o->player_num;

      if (o->id == stank)
        power[n] += 10;
      else
        power[n]++;

      ++p;
    }
	for (int k=0;k<G1_MAX_PLAYERS;k++)
		{
		if (power[k]>power[player_num])
			{
			change_player_num(k);
			}
		}
	ticks_to_think()=50; //Update every five seconds
  }
  request_think();
}
