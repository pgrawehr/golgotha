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
#include "objs/repairer.h"
#include "input.h"
#include "math/pi.h"
#include "math/angle.h"
#include "math/trig.h"
#include "math/random.h"
#include "resources.h"
#include "saver.h"
#include "map_cell.h"
#include "map.h"
#include "map_man.h"
#include "object_definer.h"
#include "g1_rand.h"
#include "camera.h"
#include "lisp/lisp.h"

enum {DATA_VERSION=1};
static li_symbol_ref light_type("lightbulb");

g1_object_definer<g1_repairer_class>
g1_repairer_def("repairer", g1_object_definition_class::EDITOR_SELECTABLE|
				g1_object_definition_class::TO_MAP_PIECE);

g1_repairer_class::g1_repairer_class(g1_object_type id,
                                     g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  
  draw_params.setup("repair_base");
  defaults = g1_repairer_def.defaults;
  
  allocate_mini_objects(3,"repair_objects");

  turret = &mini_objects[0];
  turret->x = turret->lx = 0;
  turret->y = turret->ly = 0;
  turret->h = turret->lh = 0;
  turret->rotation.set(0,0,0);
  turret->defmodeltype = g1_model_list_man.find_handle("repair_turret");
  turret->grab_old();
  
  boom = &mini_objects[1];
  boom->x = turret->lx = turret->x;
  boom->y = turret->ly = turret->y;
  boom->h = turret->lh = turret->h;
  boom->rotation.set(0,0,0);
  boom->defmodeltype = g1_model_list_man.find_handle("repair_boom");
  boom->grab_old();
  
  tip = &mini_objects[2];
  tip->x = tip->lx = g1_resources.repairer_tip_attach.x;
  tip->y = tip->ly = g1_resources.repairer_tip_attach.y;
  tip->h = tip->lh = g1_resources.repairer_tip_attach.z;
  tip->rotation.set(0,-i4_pi()/2.0f,0);
  tip->defmodeltype = g1_model_list_man.find_handle("repair_tip");
  tip->grab_old();

  memset(arc,0,sizeof(arc_point) * NUM_ARCS);
  first = 1;
  size = 0.8f;
  w1 = w2 = 0.02f;
  a1 = a2 = 1.0f;
  c1 = 0xffffff;
  c2 = 0x8800ff;
  lit = i4_T;
  repairing=i4_F;
  
  //setup_look(0.8f, 0.05f, 0.05f, 0xff88ff, 0x8800ff, 1.0, 1.0, i4_T);
  w16 ver,data_size=0;
  if (fp)
    fp->get_version(ver,data_size);
  else
    ver =0;
  switch (ver)
  {
    case DATA_VERSION:
      break;
    default:
      if (fp) fp->seek(fp->tell() + data_size);
      health = defaults->health;
      break;
  }

  if (fp)
    fp->end_version(I4_LF);

  length = g1_resources.repairer_boom_offset;

  set_flag(BLOCKING      |
		   TARGETABLE    |
		   GROUND        |
		   HIT_GROUND    |
		   SPECIALTARGETS|
		   USES_POSTTHINK|
           SHADOWED,
           1);
}

void g1_repairer_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);
  
  fp->start_version(DATA_VERSION);

  fp->end_version();
}

void g1_repairer_class::load(g1_loader_class *fp)
	{
	g1_map_piece_class::load(fp);
	fp->check_version(DATA_VERSION);
	fp->end_version(I4_LF);
	}

void g1_repairer_class::skipload(g1_loader_class *fp)
	{
	g1_map_piece_class::skipload(fp);
	fp->check_version(DATA_VERSION);
	fp->end_version(I4_LF);
	};

void g1_repairer_class::setup_flare(g1_object_class *_target)
//{{{
{

  target = _target;

  if (target.valid())
    target_pos.set(target->x,target->y,target->h);
  
  if (lit)
    create_light();
}

i4_bool g1_repairer_class::can_attack(g1_object_class *who)
{
  if (who->player_num == player_num)
  {
    g1_map_piece_class *mp = g1_map_piece_class::cast(who);
	//this is odd: compiler complains about mp!=this because of const declarations...
    return (mp && ((void *)mp)!=((void *)this) && mp->get_flag(GROUND) && mp->health>0 && mp->health<mp->defaults->health && in_range(who));
  }
  return i4_F;
}

void g1_repairer_class::request_remove()
	{
	destroy_light();//remove lights
	g1_map_piece_class::request_remove();
	}

void g1_repairer_class::think()
{  
  find_target();

  if (health < defaults->health)
    health++;

  if (fire_delay>0)
    fire_delay--;

  pitch = 0;
  roll  = 0;
  h = terrain_height;

  if (attack_target.valid()) 
  {
    i4_float dx,dy;

    //this will obviously only be true if attack_target.ref != NULL    
    dx = (attack_target->x - x);
    dy = (attack_target->y - y);

    //aim the turet
    
    repair_angle = i4_atan2(dy,dx);
    i4_normalize_angle(repair_angle);
    repair_length = (float)sqrt(dx*dx + dy*dy);
	

  }
  else
  {
    repair_angle = 0;
    repair_length = 1.5;
  }

  int aimed;
 // i4_float dangle;
  
  aimed = (i4_rotate_to(turret->rotation.z,repair_angle,defaults->turn_speed)==0.0);
  if (length<repair_length) 
  {
    length += speed;
    if (length>repair_length) length=repair_length; 
  }
  else
  {
    length -= speed;
    if (length<repair_length) length=repair_length; 
  }
  i4_float 
    cs = (float)cos(turret->rotation.z),
    sn = (float)sin(turret->rotation.z),
    ex = length - g1_resources.repairer_boom_offset;
  boom->rotation.z = turret->rotation.z;
  tip->rotation.z = turret->rotation.z;
  boom->x = cs*ex;
  boom->y = sn*ex;
  ex += g1_resources.repairer_tip_attach.x;
  tip->x = cs*ex;
  tip->y = sn*ex;

  if (attack_target.valid() && aimed)
  {
    if (i4_rotate_to(tip->rotation.y,-i4_pi()/6.0f,0.1f)==0.0 && fire_delay==0)
    {
      g1_map_piece_class *mp = g1_map_piece_class::cast(attack_target.get());
      
      if (mp->health<mp->get_max_health() && mp->health>0)
      {
        fire_delay = defaults->fire_delay;
        mp->health+=get_type()->get_damage_map()->get_damage_for(mp->id);
        
        if (mp->health>mp->defaults->health)
			{
            mp->health = mp->defaults->health;
			attack_target=0;
			}
      }
      else
        attack_target = 0;
    }
	if (!repairing&&attack_target.valid())
		{
		setup_flare(attack_target.get());
	    repairing=i4_T;
		}
	else
		repairing=i4_F;
	
  }
  else
	  {
    // return to stopped position
	  i4_rotate_to(tip->rotation.y,-i4_pi()/2.0f,0.1f);
	  repairing=i4_F;
	  destroy_light();
	  first=1;
	  }


  // don't rethink if i'm not healing or moving
  //if (attack_target.valid() ||
  //    tip->rotation.y != tip->lrotation.y ||
  //    tip->rotation.z != tip->lrotation.z ||
  //    tip->x != tip->lx ||
  //    tip->y != tip->ly)
  request_think();
}

void g1_repairer_class::draw(g1_draw_context_class *context)
{
  //g1_model_draw(this, draw_params, context);
  g1_map_piece_class::draw(context);
  if (repairing)
	  {
	  int i;
	  i4_3d_point_class pts[g1_repairer_class::NUM_ARCS];

	  for (i=0;i<g1_repairer_class::NUM_ARCS;i++)
      pts[i].interpolate(arc[i].lposition,arc[i].position,g1_render.frame_ratio);
  
      g1_render.add_translucent_trail(context->transform,pts,
                                  g1_repairer_class::NUM_ARCS,
                                  w1,w2,a1,a2,c1,c2);
	  }
}

void g1_repairer_class::copy_old_points()
//{{{
{
  sw32 i;
  for (i=0;i<NUM_ARCS;i++)
    arc[i].lposition = arc[i].position;
}
//}}}

void g1_repairer_class::arc_to(const i4_3d_vector &target)
//{{{
{
  i4_3d_vector r,p;
  i4_float map_point_height;

  i4_3d_vector point, ray;
  
  point.set(x+tip->x,y+tip->y,h+tip->h-0.2f);
  ray = target;
  ray -= point;
  ray /= NUM_ARCS-1;

  sw32 i;

  for (i=0; i<NUM_ARCS; i++)
  {
    i4_float sin_factor = (float)sin(i4_pi()*i/(NUM_ARCS-1));

    r.x = ((g1_rand(23)&0xFFFF)/((i4_float)0xffff) - 0.5f) * size*sin_factor;
    r.y = ((g1_rand(32)&0xFFFF)/((i4_float)0xffff) - 0.5f) * size*sin_factor;
    r.z = ((g1_rand(45)&0xFFFF)/((i4_float)0xffff) - 0.5f) * size*sin_factor;
    
    p.set(point.x + ray.x*i + r.x,
          point.y + ray.y*i + r.y,
          point.z + ray.z*i + r.z);
    
    map_point_height = g1_get_map()->map_height(p.x,p.y,p.z) + 0.05f;
  
    if (p.z < map_point_height)
      p.z = map_point_height;
  
    arc[i].position = p;
  }

  if (end_light.get())
    end_light->move(target.x, target.y, target.z);
}
//}}}

void g1_repairer_class::post_think()
//{{{
{
  if (repairing)
	  {
	  copy_old_points();

      if (target.valid())
          target_pos.set(target->x,target->y,target->h);

      arc_to(target_pos);

      if (first)
        copy_old_points();

      first=0;
	  }
  request_think();
}
//}}}

void g1_repairer_class::create_light()
{

  if (!light.get())
  {
    float r=g1_resources.visual_radius();
    if (g1_current_view_state()->dist_sqrd(i4_3d_vector(x,y,h))<r*r)
    {
      light = (g1_light_object_class *)g1_create_object(g1_get_object_type(light_type.get()));
      light->setup(x,y,h+0.3f, 
                   i4_float(c1>>16&0xff)/256.0f, 
                   i4_float(c1>>8&0xff)/256.0f, 
                   i4_float(c1&0xff)/256.0f, 
                   1);
      light->occupy_location();
      
      end_light = (g1_light_object_class *)g1_create_object(g1_get_object_type(light_type.get()));
      end_light->setup(target_pos.x, target_pos.y, target_pos.z, 
                       i4_float(c2>>16&0xff)/256.0f, 
                       i4_float(c2>>8&0xff)/256.0f, 
                       i4_float(c2&0xff)/256.0f, 
                       1.0f);
      end_light->occupy_location();
    }
  }
}

void g1_repairer_class::destroy_light()
{

  if (light.get())
  {
    light->unoccupy_location();
    light->request_remove();
    light=0;
  }
    
  if (end_light.get())
  {
    end_light->unoccupy_location();
    end_light->request_remove();
    end_light=0;
  }
}
