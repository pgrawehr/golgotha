#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sound_man.h"
#include "objs/model_id.h"
#include "objs/model_draw.h"
#include "input.h"
#include "math/pi.h"
#include "math/trig.h"
#include "math/angle.h"
#include "objs/bullet.h"
#include "resources.h"
#include "saver.h"
#include "map_cell.h"
#include "map.h"
#include "math/angle.h"
#include "objs/vehic_sounds.h"
#include "sound/sfx_id.h"
#include "objs/trike.h"
#include "objs/shrapnel.h"
#include "math/random.h"
#include "object_definer.h"
#include "objs/path_object.h"


static g1_model_ref model_ref("trike_body"),
  shadow_ref("trike_shadow"),
  wheel_ref("trike_wheels"),
  lod_ref("trike_lod");

static g1_object_type shrapnel_type;
static i4_3d_vector wheel_attach, wheel_offset;

void g1_trike_init()
{
  shrapnel_type = g1_get_object_type("shrapnel");

  wheel_attach.set(0.1f,0,0.03f);
  model_ref()->get_mount_point("Wheel", wheel_attach);

  wheel_offset.set(0.1f,0,0.03f);
  wheel_ref()->get_mount_point("Wheel", wheel_offset);
  wheel_offset.reverse();
}

g1_object_definer<g1_trike_class>
g1_trike_def("trike",
             g1_object_definition_class::TO_MAP_PIECE |
             g1_object_definition_class::MOVABLE |
             g1_object_definition_class::EDITOR_SELECTABLE, 
             g1_trike_init);


//think and draw functions

g1_trike_class::g1_trike_class(g1_object_type id, 
                                     g1_loader_class *fp)
  : g1_map_piece_class(id, fp)
{  
  radar_type=G1_RADAR_VEHICLE;
  set_flag(BLOCKING      |
           TARGETABLE    |
           GROUND        | 
           HIT_GROUND    |
		   SELECTABLE    |
           DANGEROUS, 1);

  draw_params.setup(model_ref.id(), 0, lod_ref.id());
        
  allocate_mini_objects(2,"Trike Mini-Objects");

  wheels = &mini_objects[0];
  wheels->defmodeltype = wheel_ref.id();
  wheels->position(wheel_attach);
  wheels->offset = wheel_offset;

  w16 ver,data_size;
  if (fp)
  {
    fp->get_version(ver,data_size);
    if (ver==DATA_VERSION)
    {
      wheels->rotation.x = fp->read_float();
      wheels->rotation.y = fp->read_float();
      wheels->rotation.z = fp->read_float();
  
      wheels->lrotation.x = fp->read_float();
      wheels->lrotation.y = fp->read_float();
      wheels->lrotation.z = fp->read_float();  
    }
    else
    {
      fp->seek(fp->tell() + data_size);
    }

    fp->end_version(I4_LF);
  }
  else
  {
    wheels->rotation.x = 0;
    wheels->rotation.y = 0;
    wheels->rotation.z = 0;  
    wheels->lrotation = wheels->rotation;
  }  

  init_rumble_sound(G1_RUMBLE_GROUND);
}
   
void g1_trike_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  fp->start_version(DATA_VERSION);

  fp->write_float(wheels->rotation.x);
  fp->write_float(wheels->rotation.y);
  fp->write_float(wheels->rotation.z);

  fp->write_float(wheels->lrotation.x);
  fp->write_float(wheels->lrotation.y);
  fp->write_float(wheels->lrotation.z);

  fp->end_version();
}

void g1_trike_class::load(g1_loader_class *fp)
	{
	g1_map_piece_class::load(fp);
	fp->check_version(DATA_VERSION);
	wheels->rotation.x=fp->read_float();
	wheels->rotation.y=fp->read_float();
	wheels->rotation.z=fp->read_float();
	wheels->lrotation.x=fp->read_float();
	wheels->lrotation.y=fp->read_float();
	wheels->lrotation.z=fp->read_float();
	fp->end_version(I4_LF);

	}

void g1_trike_class::skipload(g1_loader_class *fp)
	{
	g1_map_piece_class::skipload(fp);
	fp->seek(fp->tell()+6*4);
	fp->end_version(I4_LF);
	};

void g1_trike_class::think()
    {
    
    
    if (!alive())
        {
        unoccupy_location();
        request_remove();
        return;
        }
    
    if (attack_target.valid())
        {    
        request_think();
        
        dest_x = attack_target->x;
        dest_y = attack_target->y;
		stagger=0; 

		path_cos = dest_x - x;//cheat and calculate path from current position
		path_sin = dest_y - y;
		path_tan_phi = attack_target->h - h;
		stagger = 0;

		path_pos = 0;

		path_len = (float)sqrt(path_cos*path_cos + path_sin*path_sin);
		i4_float dist_w = 1.0f/path_len;
		path_cos *= dist_w;
		path_sin *= dist_w;
		path_tan_phi *= dist_w;

        i4_float dist=0, dtheta=0, dx=0, dy=0 ,dz=0;
		speed*=1.3f;
        suggest_move(dist, dtheta, dx, dy, 0);
		g1_object_class *blocking=0;
        check_move(dx,dy,dz,blocking);
        move(dx,dy,dz);
        
        const i4_float KILL_RADIUS=0.5;
        //The trike is a self-destructing attacker
        //if we've run into the guy
        if (x > dest_x-KILL_RADIUS  &&  y > dest_y-KILL_RADIUS  &&
            x < dest_x+KILL_RADIUS  &&  y < dest_y+KILL_RADIUS)
            {
            int damage=g1_trike_def.get_damage_map()->get_damage_for(attack_target.get()->id);
            
            //hurt the dude
            attack_target->damage(this,damage,i4_3d_vector(dx,dy,0));
            
            //throw some gibs
            g1_shrapnel_class *shrapnel = NULL;
            shrapnel = (g1_shrapnel_class *)g1_create_object(shrapnel_type);
            if (shrapnel)
                {
                i4_float rx,ry,rh;
                rx = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2f;
                ry = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2f;
                rh = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2f;
                shrapnel->setup(x+rx,y+ry,h + rh,6,i4_T);
                }
            //unoccupy_location();
            //request_remove();
            //health=0;//be shure that we aren't still alive for the next tick
			this->damage(this,health,i4_3d_vector(-dx,-dy,0));
            return;
            }      
		return;   
        }
	else if (next_path->valid())
	{
		//Need a new target because our's just blew away without our help...
		if (speed>(defaults->speed+defaults->accel))
			speed-=defaults->accel; //slow down again. 
		g1_path_object_class *next=(g1_path_object_class *)next_path.get();
		if (dest_x!=next->x || dest_y != next->y)
		{
			return_to_path(); //Maybe called a bit too often, but shoudn't hurt. 
		}
	}
	g1_map_piece_class::think();
    }

