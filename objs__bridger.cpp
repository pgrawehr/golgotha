#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "object_definer.h"
#include "objs/map_piece.h"
#include "lisp/li_class.h"
#include "map_man.h"
#include "map.h"
#include "li_objref.h"
#include "camera.h"
#include "objs/path_object.h"
#include "saver.h"
#include "map_cell.h"
#include "image_man.h"

static li_symbol_class_member bridgeable_spot("bridgeable_spot");
static li_symbol_ref bridger("bridger"), yes("yes"), already_attached("already_attached");
static li_g1_ref_class_member marker_attached_to("marker_attached_to");
static g1_team_icon_ref radar_imb("bitmaps/radar/bridger.tga");

class g1_bridger_object_class : public g1_map_piece_class
{
public:
  g1_typed_reference_class<g1_object_class> mark;
  g1_typed_reference_class<g1_object_class> last_on;
  
  g1_bridger_object_class(g1_object_type id, g1_loader_class *fp)
    : g1_map_piece_class(id,fp)
  {
    allocate_mini_objects(1, "builder mini objects");
    mini_objects[0].defmodeltype = g1_model_list_man.find_handle("bridger_wheels");
    draw_params.setup("bridger_road", 0, "bridger_road");
	draw_params.flags|=g1_model_draw_parameters::FORCEDRAW;
	mark=0;
	last_on=0;
	radar_image=&radar_imb;
    radar_type=G1_RADAR_VEHICLE;
	used_path_len=4 * used_path_len; //still relative to the world scaling
    set_flag(BLOCKING      |
             TARGETABLE    |
             GROUND        | 
			 //SELECTABLE    |
             DANGEROUS, 1);
  }


  i4_bool occupy_location()
  {
    li_class_context context(vars);
    if (!marker_attached_to()->value())
      return g1_map_piece_class::occupy_location();
    else
      return occupy_location_center();
  }

  i4_bool can_attack(g1_object_class *who)
	  {
	  return i4_F;
	  }

  
  void think()
  {
    if (!marker_attached_to()->value())
    {
      g1_map_piece_class::think();
      stagger=0;                      // I want bridges to be in a straight line
      
      if (!alive())
        return;
    

      //g1_object_class *mark=0;
      int bridgers_found=0;

      mark=0;
      g1_object_chain_class *chain=g1_get_map()->cell((int)x, (int)y)->get_obj_list();
      for (g1_object_chain_class *c=chain; c; c=c->next)
        if (g1_path_object_class::cast(c->object))
          mark=c->object;


      if (mark.valid() && mark.get()->vars->get(bridgeable_spot)==yes.get())
      {
        g1_camera_event cev;
        cev.type=G1_WATCH_IDLE;
        cev.follow_object=this;
        g1_current_view_state()->suggest_camera_event(cev);
        
        unoccupy_location();

        unlink();
        set_flag(CAN_DRIVE_ON, 1);
        set_flag(DANGEROUS,0);

        x=mark->x;
        y=mark->y;
        h=mark->h;
        
        mark->vars->get(bridgeable_spot)=already_attached.get();
        vars->get(marker_attached_to)=new li_g1_ref(mark->global_id);
        set_flag(CAN_DRIVE_ON, 1);
        
        occupy_location();
              
        pitch = roll = 0;
        groundpitch = 0;
        groundroll = 0;
        grab_old();        

      }      
    }
    else if (!check_life())
    {
      unoccupy_location();
      request_remove();      
    }
  }

  //this needs to be overwritten, since a bridger is not round enough
  //for the easy case to work (you won't be able to fire on the bridge 
  //if you use radial tests only!)
  i4_bool check_collision(const i4_3d_vector &start, 
                                         i4_3d_vector &ray)
	  {
	  i4_3d_vector normal;
      return g1_model_collide_polygonal(this, draw_params, start, ray, normal);
	  }

  void request_remove()
  {
    //g1_object_class *marker=li_g1_ref::get(vars->get(marker_attached_to),0)->value();
    //if (marker)
    //  marker->vars->get(bridgeable_spot)=yes.get();
  if (mark.valid())//fixme: Crashes if marker was removed before us
    mark.get()->vars->get(bridgeable_spot)=yes.get();
    
    g1_map_piece_class::request_remove();
  }
 
};


g1_object_definer<g1_bridger_object_class>
g1_bridger_def("bridger", 
               g1_object_definition_class::EDITOR_SELECTABLE |
               g1_object_definition_class::TO_MAP_PIECE |
               g1_object_definition_class::MOVABLE);



