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
 

#include "objs/def_object.h"
#include "lisp/li_init.h"
#include "lisp/li_class.h"
#include "li_objref.h"
#include "map_man.h"
#include "map.h"
#include "math/pi.h"
#include "objs/map_piece.h"
#include "object_definer.h"

static li_int_class_member explode_health("explode_health"),
  fire_delay("fire_delay"), total_burst("total_burst"),  burst_delay("burst_delay");

static li_symbol_class_member fire_type("fire_type");

static g1_model_ref barrel_ref("gunport_barrel");

void g1_gunport_init()
	{
	};
class g1_gunport_class:public g1_map_piece_class
	{

	protected:
		g1_mini_object *barrel;
	public:
		enum {DATA_VERSION=1};
		g1_gunport_class(g1_object_type id,
                                     g1_loader_class *fp);
		virtual void think();
		virtual void save(g1_saver_class *fp);

	};
g1_object_definer<g1_gunport_class>
g1_gunport_def("gunport", g1_object_definition_class::EDITOR_SELECTABLE|
				g1_object_definition_class::TO_MAP_PIECE,
				g1_gunport_init);

g1_gunport_class::g1_gunport_class(g1_object_type id, g1_loader_class *fp)
:g1_map_piece_class(id,fp)
	{
	defaults=g1_gunport_def.defaults;
	draw_params.setup("gunport_base");
	allocate_mini_objects(1,"gunport mini objects");
	barrel=&mini_objects[0];
	barrel->x=0;
	barrel->y=0;
	barrel->h=0.1f;
	barrel->rotation.y=i4_pi();
	barrel->defmodeltype = barrel_ref.id();
	radar_type=G1_RADAR_BUILDING;
	set_flag(SELECTABLE|
		TARGETABLE|
		DANGEROUS|
		GROUND|
		HIT_GROUND|
		BLOCKING|
		HIT_UNDERWATER|
		HIT_AERIAL,1);
	w16 ver, data_size;
	if (fp)
		{
		fp->get_version(ver,data_size);//actally, we don't exspect any data for us
		fp->seek(fp->tell() + data_size);
		fp->end_version(I4_LF);
		}
	else ver=0;
	}

//li_object *g1_gunport_damage(li_object *o, li_environment *env)
//{
//  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o,env),env);
//  int hp=li_int::get(li_car(li_cdr(li_cdr(o,env),env),env),env)->value();
//  health()-=hp;
//  me->request_think();
//  return 0;
//}

//li_object *g1_gunport_enter_range(li_object *o, li_environment *env)
//{
//  return 0;
//}

void g1_gunport_class::save(g1_saver_class *fp)
	{
	g1_map_piece_class::save(fp);
	fp->start_version(DATA_VERSION);
    fp->end_version();
	}

void g1_gunport_class::think()
	{

  if (health < 0)
  {
    if (health < explode_health())
    {
      unoccupy_location();
      request_remove();
    }
    else
    {
      set_flag(g1_object_class::DANGEROUS,0);
      health--;
      request_think();
    }
	return;
  }
  theta+=defaults->turn_speed;
}


//static li_float_class_member turn_speed("turn_speed"), speed("speed"), 
//  max_speed("max_speed"), hill_scale("hill_scale"), damage_speedup("damage_speedup");
/*
li_object *g1_speed_damage(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o));
  int hp=li_int::get(li_car(li_cdr(li_cdr(o))))->value();

  speed()+=hp * damage_speedup();

  me->request_think();
  return 0;
}
*/
/*li_object *g1_circle_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o,env),env);


  me->grab_old();
  me->unoccupy_location();
  
  float s=me->defaults->speed;

  me->x += cos(me->theta)*s;
  me->y += sin(me->theta)*s;

  float h=g1_get_map()->terrain_height(me->x, me->y);
  
  if (h<me->h)   // going down hill speed up
  {
    s += (me->h-h) * hill_scale();

    if (s>max_speed())
      s=max_speed();

    speed()=s;
    me->h=h;
  }
  else if (h>me->h)  // going up hill, slow down
  {
    s += (me->h-h) * hill_scale();
    if (s<0)
    {
      me->theta+=i4_pi();     // turn around
      s=0.001;
    }

    speed()=s;
    me->h=h;
  }

  me->theta += turn_speed();

  if (me->occupy_location())
    me->request_think();


  return 0;
}
*/
//li_automatic_add_function(g1_speed_damage, "speed_damage");
//li_automatic_add_function(g1_circle_think, "circle_think");
//li_automatic_add_function(g1_gunport_damage, "gunport_damage");
//li_automatic_add_function(g1_gunport_think, "gunport_think");
//li_automatic_add_function(g1_gunport_enter_range, "gunport_enter_range");

