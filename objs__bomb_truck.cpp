#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/map_piece.h"
#include "object_definer.h"
#include "lisp/lisp.h"
#include "objs/explode_model.h"
#include "li_objref.h"
#include "objs/path_object.h"
#include "map_man.h"
#include "map.h"
#include "saver.h"
#include "objs/shockwave.h"
#include "image_man.h"
#include "tick_count.h"
#include "sound/sfx_id.h"
#include "controller.h"
#include "human.h"

static g1_team_icon_ref radar_im("bitmaps/radar/bomb_truck.tga");

S1_SFX(bomb_coming, "computer_voice/bomb_truck_approaching_22khz.wav", S1_STREAMED, 200);

S1_SFX(bleep, "misc/bleep_22khz.wav", 0, 100);


static li_symbol_ref reached("reached");
static li_symbol_ref commands_ask("commands-ask");
static li_symbol_ref commands_exec("commands-exec");
static li_symbol_ref command_self_destruct("command-self_destruct");
li_symbol_ref shockwave("shockwave");

class g1_bomb_truck_class : public g1_map_piece_class
{
public:
  int warning_level;
  i4_bool user_warned;

  g1_bomb_truck_class(g1_object_type id, g1_loader_class *fp)
    : g1_map_piece_class(id,fp)
  {
    warning_level=0;
    draw_params.setup("bomb_truck",0,"bomb_truck_lod");

    allocate_mini_objects(1,"Bomb Truck Mini-Objects");
    user_warned=i4_F;

    radar_image=&radar_im;
    radar_type=G1_RADAR_VEHICLE;
    set_flag(BLOCKING      |
             TARGETABLE    |
             GROUND        | 
             SHADOWED      |
			 SELECTABLE    |
			 SPECIALTARGETS|
             DANGEROUS, 1);
  }

  i4_bool can_attack(g1_object_class *who)
	  {
	  if (who==this)
		  return i4_T;
	  return i4_F;
	  }

  void think()
  {
    if (attack_target.get()==this)
		damage(0,health,i4_3d_vector(0,0,1));
    g1_map_piece_class::think();
    if (warning_level>9)
      warning_level=9;

    if (g1_player_man.local_player!=player_num &&
        warning_level && (g1_tick_counter+global_id)%(10-warning_level)==0)
      bleep.play();
      
  }
  


  void damage(g1_object_class *who_is_hurting,
              int how_much_hurt, i4_3d_vector damage_dir)  
  {
    g1_object_class::damage(who_is_hurting, how_much_hurt, damage_dir);
    if (health<=0)
    {
      g1_shockwave_class *shock = NULL;
      if (controled())
          {
          g1_current_controller->view.suggest_camera_mode(
               G1_CIRCLE_WAIT,global_id);
          }
      shock = (g1_shockwave_class *)g1_create_object(g1_get_object_type(shockwave.get()));
      if (shock)
        shock->setup(i4_3d_vector(x,y,h), 0.5);

      g1_apply_damage(this, this, 0, i4_3d_vector(0,0,1));
    }
  }


  li_object *message(li_symbol *message_name,
                             li_object *message_params, 
                             li_environment *env)
  {
    if (message_name==commands_ask.get())
		{
		//the caller is requesting the list of special options for
		//this unit. We add ourselves to what the parent returns.
		return li_make_list(command_self_destruct.get(),
			g1_map_piece_class::message(message_name,message_params,env),
			0);
		};
	if (message_name==commands_exec.get())
		{
		//execute a command. If it is known, we do it and return true
		//such that the caller knows that the command was processed
		//if we don't know this command, we pass it on. 
		//If the caller gets a NULL-Response nobody knew how to handle the request
		//which should actually not happen. 
		if (message_params==command_self_destruct.get())
			{
			damage(0,health+1,i4_3d_vector(0,0,1));
			return li_true_sym;
			}
		return g1_map_piece_class::message(message_name,message_params,env);
		}
    if (message_name==reached.get())
    {
      g1_object_class *who=li_g1_ref::get(message_params,env)->value();
      if (who)
      {
        g1_path_object_class *po=g1_path_object_class::cast(who);
        
        if (po->bomb_warning_level()>warning_level)
          warning_level=po->bomb_warning_level();

        if (warning_level && g1_player_man.local_player!=player_num &&
            !user_warned)
        {
          user_warned=i4_T;
          if (g1_current_controller.get())
            g1_current_controller->scroll_message(i4gets("bomb_coming"));
          bomb_coming.play();
        }

        if (po && !po->total_links(get_team()))//only valid for last point of path
        {//lets do a self-destruction to destroy this building
          /*for (int i=0; i<po->total_controlled_objects(); i++)
          {
            g1_object_class *who_found=po->get_controlled_object(i);
            
            int e_type=g1_get_object_type("explode_model");
            g1_explode_model_class *e = (g1_explode_model_class *)g1_create_object(e_type);
            if (e && who && who->draw_params.model)
            {
              g1_explode_params params;              
              e->setup(who, i4_3d_vector(who->x, who->y, who->h), params);

              who->unoccupy_location();
              who->request_remove();
            }
          }*/
		  if (!g1_get_map()->find_object_by_id(g1_get_object_type("floorpad"),0))
			  {
			  damage(0,health+1,i4_3d_vector(0,0,1));//create nice boum
			  who->change_player_num(player_num);//but only on path-bound maps
			  }
        }
      }
    
    }
  
    return g1_map_piece_class::message(message_name, message_params, env);
  }
};

void g1_bomb_truck_init()
	{
	special_command_entry *sce=0;
	const w32 NUMCOMMANDS=5;
	char *cmdnames[NUMCOMMANDS]={"command-stop",
        "command-self_destruct",
        "command-control",
        "command-toggle_switch",
        "command-to_far_switch"};
	for (int i=0;i<NUMCOMMANDS;i++)
		{
		li_get_symbol(cmdnames[i]);//to be sure the names pop up in the symbol table
		sce=new special_command_entry;
		strcpy(sce->commandname,cmdnames[i]);
		if (!command_lookup_table.insert_if_single(
			i4_check_sum32(cmdnames[i],strlen(cmdnames[i])),sce))
			{
			delete sce;//already present->drop again.
			sce=0;
			};
		}
	};

g1_object_definer<g1_bomb_truck_class>
g1_bomb_truck_def("bomb_truck", 
                  g1_object_definition_class::EDITOR_SELECTABLE |
                  g1_object_definition_class::TO_MAP_PIECE |
                  g1_object_definition_class::MOVABLE,
				  g1_bomb_truck_init);


