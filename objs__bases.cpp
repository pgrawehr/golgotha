#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "lisp/li_class.h"
#include "g1_object.h"
#include "li_objref.h"
#include "math/random.h"
#include "math/pi.h"
#include "object_definer.h"
#include "player.h"
#include "controller.h"
#include "objs/path_object.h"
#include "objs/bases.h"
#include "sound/sfx_id.h"
#include "player.h"
#include "image_man.h"
#include "map.h"
#include "map_man.h"
#include "map_cell.h"
#ifdef NETWORK_INCLUDED
#include "net/server.h"
#endif

i4_isl_list<g1_factory_class> g1_factory_list;


static li_symbol_ref build("build");


S1_SFX(sfx_built, "assembled/vehicle_ready_22khz.wav", S1_STREAMED, 100);

static li_int_class_member ticks_till_next_deploy("ticks_till_next_deploy"),
reset_time_in_ticks("reset_time_in_ticks"),
path_color("path_color"),
selected_path_color("selected_path_color");

static li_g1_ref_class_member start("start");
static li_object_class_member can_build("can_build");
static g1_team_icon_ref airbase_radar_im("bitmaps/radar/airbase.tga");
static g1_team_icon_ref garage_radar_im("bitmaps/radar/garage.tga");
static g1_team_icon_ref mainbasepad_radar_im("bitmaps/radar/base_golg.tga");

void g1_factory_class::set_start(g1_path_object_class * _start)
{
	li_class_context context(vars);
	start();
	vars->set_value(start.offset, new li_g1_ref(_start));
}

g1_path_object_class * g1_factory_class::get_start()
{
	g1_object_class * o=li_g1_ref::get(vars->get(start),0)->value();

	if (!o)
	{
		return 0;
	}
	return g1_path_object_class::cast(o);
}


w32 g1_factory_class::get_path_color()
{
	return vars->get(path_color);
}


w32 g1_factory_class::get_selected_path_color()
{
	return vars->get(selected_path_color);
}


g1_factory_class::g1_factory_class(g1_object_type g1_ot_id, g1_loader_class * g1_lc_fp)
	: g1_building_class(g1_ot_id,g1_lc_fp) //, death(this) // JJ modification not to issue MSVC warning
{
	//death.SetG1_Object_Class(this); // JJ modification not to issue MSVC warning
	char * n=name();
	char lod_n[256];

	sprintf(lod_n, "%s_lod", n);
	draw_params.setup(n,0,lod_n);

	set_flag(BLOCKING      |
			 CAN_DRIVE_ON  |
			 TARGETABLE    |
			 GROUND
			 ,1);
	if (!strcmp(n, "garage"))
	{
		radar_image=&garage_radar_im;
		set_flag(CAN_DRIVE_ON,0);
	}
	else if (!strcmp(n, "airbase"))
	{
		radar_image=&airbase_radar_im;
		set_flag(CAN_DRIVE_ON,0); //this must not be true, otherwise planes might crash into it when starting
	}
	else if (!strcmp(n, "mainbasepad"))
	{
		radar_image=&mainbasepad_radar_im;
	}

	radar_type=G1_RADAR_BUILDING;

}

i4_bool g1_factory_class::occupy_location()
{
	if (g1_object_class::occupy_location())
	{
		g1_factory_list.insert(*this);
		return i4_T;
	}
	else
	{
		return i4_F;
	}
}

void g1_factory_class::unoccupy_location()
{
	g1_object_class::unoccupy_location();
	g1_factory_list.find_and_unlink(this);
}

g1_map_piece_class * g1_factory_class::create_object(int type)
{
	if (start()->value())
	{
		g1_object_class * o=g1_object_type_array[type]->create_object(type,0);
		if (o)
		{
			o->set_flag(NEEDS_SYNC,1);
			o->request_think(); //Be shure we think at least one time
#ifdef NETWORK_INCLUDED
			g1_network_time_man_ptr->updatecomplete(o->global_id,g1_tick_counter);
#endif
			g1_map_piece_class * mp=g1_map_piece_class::cast(o);
			if (!mp)
			{
				mp->request_remove();
			}
			else
			{
				mp->player_num=player_num;
				mp->x=x;
				mp->y=y;
				mp->h=h;
				mp->theta=theta;
				mp->add_team_flag();

				mp->grab_old();
				mp->occupy_location();

				g1_player_man.get(mp->player_num)->add_object(mp->global_id);
				return mp;
			}
		}
	}
	return 0;
}

i4_bool g1_factory_class::build(int type)
{
	li_class_context context(vars);

	g1_path_object_class * startp=get_start();
	if (startp)
	{
		// is this a object type we can build?
		int found=0;
		for (li_object * blist=can_build(); !found && blist; blist=li_cdr(blist,0))
		{
			li_object * val=li_get_value(li_symbol::get(li_car(blist,0),0));
			li_int * anint=li_int::get(val,0);
			if (anint&&anint->value()==type)
			{
				found=1;
			}
		}

		if (!found)
		{
			return 0;
		}

		int cost=g1_object_type_array[type]->defaults->cost;

		if (cost<=g1_player_man.get(player_num)->money() && !deploy_que.full())
		{
			if (player_num==g1_player_man.local_player)
			{
				sfx_built.play();
			}

			g1_player_man.get(player_num)->money()-=cost;
			g1_build_item item;
			item.type=type;

			g1_path_object_class * list[400];

			if (startp)
			{
				int t=startp->find_path(g1_player_man.get(player_num)->get_team(), list, 400);
				item.path=(g1_id_ref *)I4_MALLOC(sizeof(g1_id_ref) *(t+1), "");
				for (int i=0; i<t; i++)
				{
					item.path[i]=list[i];
				}
				item.path[t].id=0;
			}
			else
			{
				item.path=0;
			}

			deploy_que.que(item);

			request_think();
			return i4_T;
		}
	}
	return i4_F;
}

void g1_factory_class::request_remove()
{
	while (!deploy_que.empty())
	{
		g1_build_item item;
		deploy_que.deque(item);
		i4_free(item.path);
	}
	g1_building_class::request_remove();
}

void g1_factory_class::think()
{
	if (!death.think())
	{
		return;
	}

	if (ticks_till_next_deploy())
	{
		ticks_till_next_deploy()--;
		request_think();
	}
	else if (!deploy_que.empty())
	{

		g1_object_chain_class * c=g1_get_map()->cell(x,y)->get_solid_list();
		while (c)
		{
			if (c->object!=this)
			{
				request_think();
				return; //G1_BUILD_OCCUPIED to be tested here, otherwise
				//the queue can newer be larger than one single object
			}
			c=c->next_solid();
		}

		ticks_till_next_deploy()=reset_time_in_ticks();

		g1_build_item item;
		deploy_que.deque(item);

		g1_map_piece_class * o=create_object(item.type);
		if (o)
		{
			o->set_path(item.path);
			char msg[100];
			sprintf(msg, "Vehicle advancing: %s", o->name());
			g1_player_man.show_message(msg, 0x00ff00, player_num);
		}
		else
		{
			i4_free(item.path);
		}

		request_think();
	}
}

g1_building_class::g1_building_class(g1_object_type id, g1_loader_class * fp)
	: g1_object_class(id,fp)
{
	death.SetG1_Object_Class(this);
	set_flag(TARGETABLE|BLOCKING,1);
}

//void g1_building_class::think()
//	{
//death.think();
//	}

void g1_building_class::damage(g1_object_class * obj, int hp, i4_3d_vector damage_dir)
{
	death.damage(obj, hp, damage_dir);
}

static g1_object_definer<g1_factory_class>
garage_def("garage", g1_object_definition_class::EDITOR_SELECTABLE|
		   g1_object_definition_class::TO_BUILDING|
		   g1_object_definition_class::TO_FACTORY);

static g1_object_definer<g1_factory_class>
airbase_def("airbase", g1_object_definition_class::EDITOR_SELECTABLE|
			g1_object_definition_class::TO_BUILDING|
			g1_object_definition_class::TO_FACTORY);
