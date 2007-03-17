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


#include "player.h"
#include "objs/def_object.h"
#include "lisp/li_class.h"
#include "lisp/li_init.h"
#include "controller.h"


static li_float_class_member damage_multiplier_fraction("damage_multiplier_fraction"),
spin_speed("spin_speed");



li_object *g1_armor_occupy(li_object *o, li_environment *env)
{
	g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_first(o,env),env);

	if (me->g1_object_class::occupy_location())
	{
		g1_player_man.get(me->player_num)->damage_multiplier() *= damage_multiplier_fraction();
		return li_true_sym;
	}
	return 0;
}


li_object *g1_armor_unoccupy(li_object *o, li_environment *env)
{
	g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_first(o,env),env);
	me->g1_object_class::unoccupy_location();
	g1_player_man.get(me->player_num)->damage_multiplier() /= damage_multiplier_fraction();
	return 0;
}

li_object *g1_armor_think(li_object *o, li_environment *env)
{
	g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_first(o,env),env);
	me->mini_objects[0].rotation.z+=spin_speed();
	me->request_think();
	return 0;
}



li_object *g1_armor_change_player_num(li_object *o, li_environment *env)
{
	g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_first(o,env),env);
	int new_team=li_int::get(li_second(o,env),env)->value();

	if (new_team!=me->player_num)
	{
		g1_player_man.get(new_team)->damage_multiplier()*=damage_multiplier_fraction();
		g1_player_man.get(me->player_num)->damage_multiplier()/=damage_multiplier_fraction();


		if (new_team==g1_player_man.local_player && g1_current_controller.get())
		{
			g1_current_controller->add_spin_event("powerup_shields", 0);
		}

		if (me->player_num==g1_player_man.local_player && g1_current_controller.get())
		{
			g1_current_controller->add_spin_event("powerup_shields", 1);
		}

		me->g1_object_class::change_player_num(new_team);
	}

	return 0;
}

li_automatic_add_function(g1_armor_occupy, "armor_building_occupy_location");
li_automatic_add_function(g1_armor_unoccupy, "armor_building_unoccupy_location");
li_automatic_add_function(g1_armor_think, "armor_building_think");
li_automatic_add_function(g1_armor_change_player_num, "armor_building_change_player_num");
