#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.h"
#include "lisp/li_class.h"
#include "g1_object.h"
#include "li_objref.h"
#include "math/random.h"
#include "object_definer.h"
#include "player.h"
#include "controller.h"
#include "objs/stank.h"
#include "objs/path_object.h"
#include "objs/bases.h"
#include "app/app.h"
#include "device/event.h"
#include "mess_id.h"
#include "device/kernel.h"
#include "math/pi.h"
#include "border_frame.h"
#include "team_api.h"

static li_symbol_ref stank("stank");

class g1_stank_factory_class : public g1_factory_class
{
public:
  g1_stank_factory_class(g1_object_type id, g1_loader_class *fp)
    : g1_factory_class(id,fp)
  {
    //flags|=THINKING;
    //set_flag(BLOCKING,0);
	set_flag(CAN_DRIVE_ON,1);
    request_think();

  }
  
  i4_bool occupy_location() 
  { 
    if (occupy_location_center())
    {
      g1_factory_list.insert(*this);
      return i4_T;
    }
    else return i4_F;
  }


  //void continue_game()
  //{
  //  
  //}

  void request_remove()
	  {
	  g1_factory_class::request_remove();

	  }

  void think()
  {
    if (!death.think())
		return;
	if (health<=0)
		{
		unoccupy_location();
		request_remove();
		}
	
    request_think();
	g1_player_info_class *pl=g1_player_man.get(player_num);
    if (!pl->get_commander() 
        /*&&
        !pl->continue_wait*/)
    {//strncmp returns 0 if the two match, but we want to continue if they don't match.
      if (pl->num_stank_lives() 
		  && pl->get_ai()->ai_name()->strncmp("ai_remote",9))
      {
        g1_object_class *o=g1_create_object(g1_get_object_type(stank.get()));
        g1_player_piece_class *stank=g1_player_piece_class::cast(o);
        if (stank)
        {
          pl->num_stank_lives()--;
          pl->calc_upgrade_level();

          stank->player_num=player_num;
          stank->x=x; stank->y=y; stank->h=h;
          stank->theta=theta-i4_pi()/2.0f;
          stank->turret->rotation.z = stank->base_angle = stank->theta;

          stank->grab_old();
          stank->occupy_location();

          pl->add_object(stank->global_id);

          pl->set_commander(stank);


          /*
          if (player_num==g1_player_man.get_local()->get_player_num() && 
              g1_current_controller.get())
          {
            if (g1_border.get())
            {
              if (g1_border->strategy_on_top)
				  {
                //li_call("strategy_toggle");
				  }
              else
                g1_current_controller->view.suggest_camera_mode(G1_ACTION_MODE,
                  stank->global_id);
            }
          }
          */

          stank->request_think();    
        }
        else delete o;
      }
      //else //this is not appropriate. 
	  //  we have only lost if (we have no more stanks) and 
	  // ((we have no more money) or (we have no stank_factory))
	  // I'll implement a special winning/loosing condition tester object.
      //{
      //  if (g1_player_man.local_player==player_num)
      //  {
      //    i4_user_message_event_class loser(G1_YOU_LOSE);
      //    i4_kernel.send_event(i4_current_app, &loser);
      //  }
      //}
    }
  }

  virtual i4_bool build(int type)
  {
    if (type!=g1_supertank_type || g1_player_man.get(player_num)->num_stank_lives()>=5)
      return i4_F;

    if (g1_player_man.get(player_num)->money()>=g1_object_type_array[type]->defaults->cost)
    {
      g1_player_man.get(player_num)->money()-=g1_object_type_array[type]->defaults->cost;
      g1_player_man.get(player_num)->num_stank_lives()++;
      return i4_T;
    }
    else return i4_F;



  }
  
};

static g1_object_definer<g1_stank_factory_class>
mainbase_def("mainbasepad", g1_object_definition_class::EDITOR_SELECTABLE|
			 g1_object_definition_class::TO_BUILDING|
			 g1_object_definition_class::TO_FACTORY);

