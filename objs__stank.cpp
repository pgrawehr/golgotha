/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "pch.h"
#include "sound_man.h"
#include "input.h"
#include "math/pi.h"
#include "math/angle.h"
#include "math/random.h"
#include "resources.h"
#include "global_id.h"
#include "map.h"
#include "map_cell.h"
#include "map_man.h"
#include "sound/sfx_id.h"
#include "g1_speed.h"
#include "math/trig.h"
#include "g1_render.h"
#include "render/r1_api.h"
#include "tile.h"
#include "object_definer.h"
#include "player.h"
#include "human.h"
#include "controller.h"
#include "g1_render.h"
#include "statistics.h"
#include "human.h"
#include "border_frame.h"

#include "objs/model_id.h"
#include "objs/stank.h"

#include "objs/explosion1.h"
#include "objs/shrapnel.h"
#include "objs/fire.h"
#include "lisp/lisp.h"
#include "lisp/li_load.h"
#include "tick_count.h"

#include "time/profile.h"
#include "map_cell.h"
#include "render/r1_clip.h"
#include "lisp/li_error.h"
#include "objs/vehic_sounds.h"
#include "map_view.h"

#include "image_man.h"

#include "g1_tint.h"
#include "objs/carcass.h"
#include "solvemap.h"
#include "solvemap_astar.h"
#include "map_vert.h"
#include "objs/shockwave.h"
#ifdef NETWORK_INCLUDED
#include "net/server.h"
#endif
//static i4_float droll = 0.0, ddroll = 0.0, 
static i4_float hurt=0.0f;

static g1_model_ref model_ref("supertankbase"),
  turret_ref("supertanktop"),
  turret_full_ref("supertanktop_full"),
  stank_carcass("chunk_supertankbase");

static i4_3d_vector turret_attach, muzzle_exit;

static g1_team_icon_ref radar_im0("bitmaps/radar/supertank_0.tga");
static g1_team_icon_ref radar_im2("bitmaps/radar/supertank_2.tga");
static g1_team_icon_ref radar_im4("bitmaps/radar/supertank_4.tga");
static g1_team_icon_ref radar_im6("bitmaps/radar/supertank_6.tga");
static g1_team_icon_ref radar_im8("bitmaps/radar/supertank_8.tga");
static g1_team_icon_ref radar_im10("bitmaps/radar/supertank_10.tga");
static g1_team_icon_ref radar_im12("bitmaps/radar/supertank_12.tga");
static g1_team_icon_ref radar_im14("bitmaps/radar/supertank_14.tga");

S1_SFX(supertank_lost, "narrative/supertank_lost_22khz.wav", S1_STREAMED, 200);
S1_SFX(return_to_repair, "computer_voice/return_to_base_for_repairs_22khz.wav", S1_STREAMED, 200);
S1_SFX(refuel_sfx, "misc/supertank_refuel_lp.wav", S1_3D, 200);

i4_profile_class pf_stank_think("stank::think");

g1_object_type g1_supertank_type;

static g1_object_type main_base_type;

void g1_supertank_init();

g1_object_definer<g1_player_piece_class>
g1_supertank_def("stank",
                 g1_object_definition_class::EDITOR_SELECTABLE|
				 g1_object_definition_class::MOVABLE |
                 g1_object_definition_class::TO_MAP_PIECE |
                 g1_object_definition_class::TO_PLAYER_PIECE,
                 g1_supertank_init);

void g1_supertank_init()
{
  g1_supertank_type  = g1_supertank_def.type;
  main_base_type     = g1_get_object_type("mainbasepad");

  turret_attach = g1_resources.player_top_attach;
  model_ref()->get_mount_point("Turret", turret_attach);
  muzzle_exit.set(0.4f, 0, 0.11f);
  turret_ref()->get_mount_point("Muzzle", muzzle_exit);
}


void g1_player_piece_class::save_base_info(g1_saver_class *fp)
{
  fp->start_version(BASE_DATA_VERSION);  
  fp->write_format("ffff44", 
                   &base_angle, &lbase_angle, &strafe_speed, &cur_top_attach, 
                   &move_tick,
                   &upgrade_level_when_built);
  fp->end_version();
}

void g1_player_piece_class::load_base_info(g1_loader_class *fp)
{
  if (fp && fp->check_version(BASE_DATA_VERSION))
  {
    fp->read_format("ffff44", 
                    &base_angle, &lbase_angle, &strafe_speed, &cur_top_attach, 
                    &move_tick,
                    &upgrade_level_when_built);
    fp->end_version(I4_LF);

  }
  else
  {
    base_angle=lbase_angle=0;
    strafe_speed=0;
    cur_top_attach=g1_resources.player_top_attach.z;
    move_tick=0;
  }
}


void g1_player_piece_class::save_mouse_look_info(g1_saver_class *fp)
{
  fp->start_version(MOUSE_LOOK_DATA_VERSION);

  fp->write_float(mouse_look_increment_x);
  fp->write_float(mouse_look_increment_y);
  fp->write_float(mouse_look_x);
  fp->write_float(mouse_look_y);

  fp->write_float(turret->rotation.z);

  fp->end_version();
}

void g1_player_piece_class::load_mouse_look_info(g1_loader_class *fp)
{
  if (fp && fp->check_version(MOUSE_LOOK_DATA_VERSION))
  {
    mouse_look_increment_x=fp->read_float();
    mouse_look_increment_y=fp->read_float();
    mouse_look_x=fp->read_float();
    mouse_look_y=fp->read_float();

    turret->rotation.z=fp->read_float();
    fp->end_version(I4_LF);
  }
  else
  {
    mouse_look_increment_x=mouse_look_increment_y=0;
    mouse_look_x=mouse_look_y=0;
    turret->rotation.z=0;
  }
}


void g1_player_piece_class::save_ammo_info(g1_saver_class *fp)
{
  fp->start_version(AMMO_DATA_VERSION);
  for (int i=0; i<MAX_WEAPONS; i++)
    ammo[i].save(fp);
  fp->end_version();
}

void g1_player_piece_class::load_ammo_info(g1_loader_class *fp)
{
  if (fp && fp->check_version(AMMO_DATA_VERSION))
  {
    for (int i=0; i<MAX_WEAPONS; i++)
      ammo[i].load(fp);
    fp->end_version(I4_LF);
  }
}

void g1_player_piece_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  int han=fp->mark_size();

  fp->start_version(DATA_VERSION);
  fp->write_32(stank_flags);
  fp->write_float(vspeed);
  fp->end_version();

  save_base_info(fp);
  save_mouse_look_info(fp);
  save_ammo_info(fp);

  fp->end_mark_size(han);
}


void g1_stank_ammo_info_struct::setup(li_symbol *sym)
{
  ammo_type=g1_find_stank_ammo_type(sym);
  if (ammo_type)
    amount=ammo_type->max_amount;
  else
    li_error(0,"USER: s_tank : no weapon called %O", sym);
  
  delay_remaining=0;   
  need_refresh=i4_T;    
  last_fired_object=0;  
}

i4_str g1_player_piece_class::frame_image_name(int &num_entries)
    {
    num_entries=6;
    g1_player_info_class *p=g1_player_man.get(player_num);
    int level=p->supertank_upgrade_level;
    i4_str name="bitmaps/stank/status_stank_level_";
    i4_char c(level+'0');
    name.insert(name.end(),c);
    name.insert(name.end(),".jpg");
    return name;
    }

void g1_player_piece_class::frame_amount(int entry_number,
                                         int &current_amount,
                                         int &max_amount,
                                         g1_amount_display_class *window)
    {
    
    g1_player_info_class *p=g1_player_man.get(player_num);
    int level=p->supertank_upgrade_level;
    li_object *o,*a;
	li_symbol *s;
	g1_stank_ammo_type_struct *am;
	for (o=li_get_value("upgrade_levels");o&&level;level--) 
		o=li_cdr(o,0);
	o=li_car(o,0);
    max_amount=ammo[entry_number-2].ammo_type->max_amount;
    current_amount=ammo[entry_number-2].amount;
	if (o)
		{
        switch (entry_number)
            {
            case 2:
                //main
		        a=li_second(o,0);
		        s=li_symbol::get(a,0);
		        am=g1_find_stank_ammo_type(s);
		        window->change_icon(am->icon,am->dark_icon);
                break;
            case 3:
                //missiles
		        a=li_third(o,0);
		        s=li_symbol::get(a,0);
		        am=g1_find_stank_ammo_type(s);
		        window->change_icon(am->icon,am->dark_icon);
                break;
            case 4:
                //chaingun
		        a=li_fourth(o,0);
		        s=li_symbol::get(a,0);
		        am=g1_find_stank_ammo_type(s);
		        window->change_icon(am->icon,am->dark_icon);
                break;
            case 5:
                //health
		        a=li_fifth(o,0);
		        s=li_symbol::get(a,0);
		        am=g1_find_stank_ammo_type(s);
		        window->change_icon(am->icon,am->dark_icon);
                current_amount=health;
                break;
            default:
                current_amount=0;
                max_amount=-1;
            };
		}

    };

void g1_player_piece_class::find_weapons()
{
  int level=g1_player_man.get_local()->supertank_upgrade_level;
  if (level<0) level=0;
  upgrade_level_when_built=level;

  
  li_object *o;
  for (o=li_get_value("upgrade_levels"); o && li_cdr(o,0) && level; level--)
    o=li_cdr(o,0);
  
  if (o)
  {
    o=li_cdr(li_car(o,0),0);

    for (int i=0; i<MAX_WEAPONS; i++)
    {
      ammo[i].setup(li_symbol::get(li_car(o,0),0));
      o=li_cdr(o,0);
    }    
  }      
}



void g1_stank_ammo_info_struct::save(g1_saver_class *fp)
{
  if (ammo_type)
    li_save_object(fp, ammo_type->weapon_type,0);
  else
    li_save_object(fp, li_nil,0);
  
                   
  fp->write_16(amount);
  fp->write_16(delay_remaining);
  fp->write_8(need_refresh);
  
  fp->write_reference(last_fired_object);
}

void g1_stank_ammo_info_struct::load(g1_loader_class *fp)
{
  li_symbol *wtype=li_symbol::get(li_load_object(fp, fp->li_remap, 0),0);
  
  ammo_type=g1_find_stank_ammo_type(wtype);
  amount=fp->read_16();
  delay_remaining=fp->read_16();
  need_refresh=fp->read_8();
  fp->read_reference(last_fired_object);      
}
  

void g1_player_piece_class::load(g1_loader_class *fp)
	{
	g1_map_piece_class::load(fp);
	int size=fp->read_32();
	fp->check_version(DATA_VERSION);
	stank_flags = fp->read_32();
    vspeed = fp->read_float();     
    fp->end_version(I4_LF);

    //find_weapons();
      
    load_base_info(fp);
    load_mouse_look_info(fp);
    load_ammo_info(fp);
	};

void g1_player_piece_class::skipload(g1_loader_class *fp)
	{
	g1_map_piece_class::load(fp);
	int size= fp->read_32();
	fp->check_version(DATA_VERSION);
	fp->read_32();
	fp->read_float();
	fp->end_version(I4_LF);
    fp->seek(fp->tell()+28+24);
	if (fp && fp->check_version(AMMO_DATA_VERSION))
	{
		for (int i=0; i<MAX_WEAPONS; i++)
			ammo[MAX_WEAPONS].load(fp);//just ignore what's there... (we have added an additional entry)
        fp->end_version(I4_LF);
	}

	};

g1_player_piece_class::g1_player_piece_class(g1_object_type id,
                                             g1_loader_class *fp)
  : g1_map_piece_class(id, fp), refuel_handle()
{   
  //w16 ver,data_size;
  i4_bool defaults=i4_T;

  draw_params.setup(model_ref.id());
  solveparams=SF_GRADE4;
  my_solver=g1_get_map()->get_astar_solver();

  allocate_mini_objects(1,"Super Tank Mini Objects");
  turret = &mini_objects[0];
  turret->defmodeltype = turret_ref.id();
  turret->position(turret_attach);

  if (fp)
  {
    int total_size=fp->read_32();

    if (fp->check_version(DATA_VERSION))
    {
      stank_flags = fp->read_32();
      vspeed = fp->read_float();     
      fp->end_version(I4_LF);

      find_weapons();
      
      load_base_info(fp);
      load_mouse_look_info(fp);
      load_ammo_info(fp);
      defaults=i4_F;
    }
    else 
      fp->seek(fp->tell() + total_size);   
  }
  
  if (defaults)
  {
    find_weapons();
    stank_flags=0;
    vspeed=0;

    load_base_info(0);
    load_mouse_look_info(0);
    load_ammo_info(0);

    health=ammo[3].amount;
  }
    
  turret_kick = lturret_kick = 0;

  // clear user data
  accel_ratio = strafe_accel_ratio = dtheta = 0;
  memset(fire,0,sizeof(fire));

  init_rumble_sound(G1_RUMBLE_STANK);

  radar_image=&radar_im0;
  radar_type=G1_RADAR_STANK;
  //initialize these guys. somewhat important
  set_flag(GROUND        | 
           BLOCKING      |
           SELECTABLE    |
           TARGETABLE    |
           GROUND        | 
           HIT_AERIAL    |//stank can attack anything
           HIT_GROUND    |
		   HIT_UNDERWATER|
           DANGEROUS, 1);

}

g1_map_solver_class *g1_player_piece_class::prefered_solver()
	{
	solveparams=SF_OK|SF_GRADE4|SF_FORCE_ASTAR;
	return my_solver=g1_get_map()->get_astar_solver();
	}

i4_bool g1_player_piece_class::suggest_move(i4_float &dist,
                                            i4_float &dtheta,
                                            i4_float &dx, i4_float &dy,
                                            i4_float braking_friction)
{
  i4_float angle,t ,diffangle;
  
  i4_bool go_reverse    = i4_F;
  i4_bool can_get_there = i4_T;

  dx = (dest_x - x);
  dy = (dest_y - y);

  //aim the vehicle    
  angle = i4_atan2(dy,dx);
  i4_normalize_angle(angle);    
  
  diffangle = angle - theta;
  if (diffangle<-i4_pi()) diffangle += 2*i4_pi();
  else if (diffangle>i4_pi()) diffangle -= 2*i4_pi();

  go_reverse = (diffangle>i4_pi()/2) || (diffangle<-i4_pi()/2);

  //dont worry about turn radii unless he's actually ready for forward movement
  if (go_reverse)
    dtheta = diffangle;
  else
  {
    can_get_there = check_turn_radius();
    dtheta = diffangle;
  }

  //distance to move squared
  t = dx*dx + dy*dy;
  
  //how far will the vehicle go if he slows down from his maximum speed?
  if (t>speed*speed+0.0025 || braking_friction==0.0)
  {
    if (dtheta<-defaults->turn_speed) dtheta = -defaults->turn_speed;
    else if (dtheta>defaults->turn_speed) dtheta = defaults->turn_speed;
    theta += dtheta;
    i4_normalize_angle(theta);

    i4_float stop_dist=0;
    if (braking_friction>0.0)
      stop_dist = (float)fabs(speed)/braking_friction; // geometric series of braking motion

    if (!go_reverse && braking_friction>0.0 && t<=(stop_dist*stop_dist))
    {            
      //just in case our calculations were off, dont let him slow to less than 0.01
      if (speed <= 0.01)
        speed = 0.01f;
      else
        speed *= (1.0f-braking_friction);
    }
    else
    {
      // calculate speed changes

      // uA = coefficient of sliding friction (removed for now)
      // th = rising pitch
      // uB = damping friction (damping_fraction = 1/uB)
      // C = exp( -uB * t) (damping_e)

      // K = (accel - (uA*cos(th) + sin(th))*g
      // finalvel = K / uB

      // vel = finalvel - (speed - finalvel)*exp(-uB * step_time) 

      i4_float /*target_speed,*/ accel;

      if (!can_get_there && !go_reverse)
        speed *= (1.0f-braking_friction);
      else 
      {
        if (go_reverse) 
          accel = -defaults->accel*0.30f;
        else
          accel = defaults->accel;

        accel += (float)sin(pitch)*g1_resources.gravity;
        speed += accel;
        speed -= speed*damping_fraction;
      }
    }
    dist = speed*(float)cos(pitch);
    dx = (float)cos(theta) * dist;
    dy = (float)sin(theta) * dist;
    dist = t;
  }
  else
  {
    dtheta = 0;
    dist = 0;
    speed = 0;
    dx = 0;
    dy = 0;
  }    

  // Oli's debug statement
//   if (g1_object_class::selected())
//     g1_debugxy(0,1,"(%5.2f,%5.2f)-(%5.2f,%6f)", 
//                x, y, 
//                pitch*180/i4_pi(), speed);

  return (dist==0);
}

int g1_player_piece_class::move(i4_float x_amount, i4_float y_amount)
{
  int x1,y1, x2,y2, ix,iy;
  i4_bool ret = MV_OK;

  if (x_amount==0.0 && y_amount==0.0)
    return MV_OK;

  i4_3d_vector start, ray(x_amount,y_amount,0), normal, offset, oray;
//  i4_float dist;

  /*
  offset = ray;
  dist = offset.length();
  offset *= 0.1/dist;

  ray += offset;
  */

  const int nc=8;
  const i4_float rad = 0.25;
  g1_object_class *hit_obj=0;
  for (int i=0; i<nc; i++)
  {
    start.set(x+rad*(float)cos(2.0*i4_pi()*(0.5+i)/nc),y+rad*(float)sin(2.0*i4_pi()*(0.5+i)/nc),h+0.07f);

    if (ray.x<0) { x2 = i4_f_to_i(start.x); x1 = i4_f_to_i(start.x+ray.x); }
    else         { x1 = i4_f_to_i(start.x); x2 = i4_f_to_i(start.x+ray.x); }
    if (ray.y<0) { y2 = i4_f_to_i(start.y); y1 = i4_f_to_i(start.y+ray.y); }
    else         { y1 = i4_f_to_i(start.y); y2 = i4_f_to_i(start.y+ray.y); }
    
    for (iy=y1; iy<=y2; iy++)
      for (ix=x1; ix<=x2; ix++)
      {
        g1_object_chain_class *objlist = g1_get_map()->cell(ix,iy)->get_obj_list();
        
        while (objlist)
        {
          g1_object_class *obj=objlist->object;
//          i4_float dist, dx,dy;
        
          if (obj->get_flag(BLOCKING) && obj!=this)
          {
            oray = ray;
            if (g1_model_collide_polygonal(obj, obj->draw_params, start, ray, normal))
            {
              if (normal.z>0.8)
              {
                // incline more than 45 degrees.  let it go
                ray = oray;
              }
              else
              {
                // slide perp. to the normal
                // NOTE: we really should resubmit the new movement vector to objects
                //       in the list we already tried to collide against for more robust
                //       sliding.  just hope we catch most of the errors on the next frame
			    // we've hit the object 'obj'
			   
				if (!(obj->get_type()->flags&g1_object_definition_class::MOVABLE))
						hit_obj=obj;
                i4_3d_vector diff(ray);
                diff -= oray;
                normal.z = 0;
                normal.normalize();
                normal *= diff.dot(normal) + 0.05f;
                ray = oray;
                ray += normal;
                ret = MV_CORRECTED;
              }
            }
          }
        
          objlist = objlist->next_solid();
        }
      }
  }
  if (hit_obj && ray.length()<0.001)
	  return MV_CANNOTCONT;
  unoccupy_location();
  /*
  ray -= offset;
  */
  x += ray.x;
  y += ray.y;
  if (x<1) x=1; else if (x>g1_get_map()->width()-2) x=(float)g1_get_map()->width()-2;
  if (y<1) y=1; else if (y>g1_get_map()->height()-2) y=(float)g1_get_map()->height()-2;
  g1_map_vertex_class *wvert=g1_get_vertex((int)x,(int)y);
  if (((g1_tick_counter&0x01f) ==0 ) && 
	  (wvert->get_flag(g1_map_vertex_class::APPLY_WAVE_FUNCTION))
	  && speed != 0)
	  {
	  g2_water_wave_class *wv=(g2_water_wave_class*) g1_create_object(g1_get_object_type("water_wave"));
	  if (wv)
		  wv->setup_wave(i4_3d_vector(x,y,h),0.2f,10);
	  }
  occupy_location();

  return ret;
}

void g1_player_piece_class::lead_target(i4_3d_point_class &lead, int slot_number)
{
  g1_stank_ammo_info_struct *a=ammo+slot_number;

  int wtype=g1_get_object_type(a->ammo_type->weapon_type);
  if (wtype)
  {
    i4_float shot_speed = g1_object_type_array[wtype]->get_damage_map()->speed;
    g1_map_piece_class::lead_target(lead,shot_speed);
  }
}

void g1_player_piece_class::check_if_turret_in_ground()
{
  if (player_num != g1_player_man.local_player)
    return;

  i4_3d_vector pos, dir;
  float h;
  //PG: Bugfix: if the stank position is really weird, this loop newer quits.
  do
  {
    get_bullet_exit(pos, dir);
    h=pos.z-g1_get_map()->map_height(pos.x, pos.y, pos.z);

    if (h<0)
      turret->rotation.y-=0.01f;
  } while ((h<0) && (turret->rotation.y>-10));
}
  
 
void g1_player_piece_class::check_for_refuel()
{
  g1_object_chain_class *objlist = g1_get_map()->cell(i4_f_to_i(x), i4_f_to_i(y))->get_solid_list();
  i4_bool on_base=i4_F;

  while (objlist && !on_base)
  {
    g1_object_class *obj=objlist->object;
//    i4_float dist, dx,dy;

    if (obj->id == main_base_type && obj->player_num==player_num)
      on_base = i4_T;

    objlist=objlist->next;
  }

  if (on_base)
  {
    if (get_stank_flag(g1_player_piece_class::ST_REFUELING)==0)
    {      
      for (int i=0; i<MAX_WEAPONS; i++)
      {
        int mx=ammo[i].ammo_type->max_amount;
        if (mx==0) mx=1;
        ammo[i].refuel_delay_remaining=ammo[i].ammo_type->refuel_delay/mx;
      }
      
      set_stank_flag(g1_player_piece_class::ST_REFUELING,1);
    }

    health++;
    if (health>ammo[3].ammo_type->max_amount)
    {
      health=ammo[3].ammo_type->max_amount;
	  
      if (refuel_handle.playing) s1_end_looping(refuel_handle);
    }
    else
    {
//       g1_sound_man.update_dynamic_3d_sound(refuel_sound,   sfxfix
//                                            i4_3d_vector(x,y,h),
//                                            i4_3d_vector(0,0,0),1.0);
    }

    for (int i=0; i<MAX_WEAPONS; i++)
    {
      if (ammo[i].refuel_delay_remaining)
        ammo[i].refuel_delay_remaining--;
      else
      {
        if (ammo[i].amount<ammo[i].ammo_type->max_amount)
        {
          ammo[i].amount++;
		  refuel_sfx.play_looping(refuel_handle);
          // jc:fixme  add refuel sfx here
        }

        int mx=ammo[i].ammo_type->max_amount;
        if (mx==0) mx=1;
        ammo[i].refuel_delay_remaining=ammo[i].ammo_type->refuel_delay/mx;
      }
    }

  }
  else
  {    
    if (get_stank_flag(ST_REFUELING))
      //stop refueling
      set_stank_flag(ST_REFUELING,0);
    
	if (refuel_handle.playing) s1_end_looping(refuel_handle);
//     if (refuel_sound && refuel_sound->is_playing())   sfxfix
//       refuel_sound->stop();
  }

}





void g1_player_piece_class::check_for_powerups()
{
  i4_3d_vector v=i4_3d_vector(x,y,h);
  float r=2.0;

  int fog_rect_x1=10000, fog_rect_y1=10000,
      fog_rect_x2=-1, fog_rect_y2=-1, ix,iy;
  
  g1_map_class *map=g1_get_map();

  int x_left   = i4_f_to_i(v.x-r); if (x_left<0)                x_left=0;
  int x_right  = i4_f_to_i(v.x+r); if (x_right>=map->width())   x_right=map->width()-1;
  int y_top    = i4_f_to_i(v.y-r); if (y_top<0)                 y_top=0;
  int y_bottom = i4_f_to_i(v.y+r); if (y_bottom>=map->height()) y_bottom=map->height()-1;  

  for (iy=y_top;  iy<=y_bottom; iy++)
  {
    g1_map_cell_class *c=map->cell(x_left,iy);
    for (ix=x_left; ix<=x_right;  ix++, c++)
    {
      for (g1_object_chain_class *o=c->get_obj_list(); o; o=o->next)
      {
        g1_object_class *obj=o->object;
        if (!obj->get_flag(g1_object_class::SCRATCH_BIT))
        {
          obj->set_flag(g1_object_class::SCRATCH_BIT, 1);
          obj->note_stank_near(this);
        }
      }
    }
  }


  for (iy=y_top;  iy<=y_bottom; iy++)
  {
    g1_map_cell_class *c=map->cell(x_left,iy);
    for (ix=x_left; ix<=x_right;  ix++, c++)
      for (g1_object_chain_class *o=c->get_obj_list(); o; o=o->next)
        o->object->set_flag(g1_object_class::SCRATCH_BIT, 0);
  }
}


short g1_player_piece_class::get_max_health()
	{
	return ammo[3].ammo_type->max_amount;
	}

void g1_player_piece_class::think()
{
  pf_stank_think.start();

  if (health>ammo[3].ammo_type->max_amount)//since the static defaults->health
	  health=ammo[3].ammo_type->max_amount;//of an stank is very high.
  // Save old values of position & orientation
  lbase_angle  = base_angle;    
  lturret_kick  = turret_kick;

  if (!check_life())
  {
    //    g1_sound_man.play(STATIC_3D_SOUND,g1_sfx_explosion_ground_vehicle,i4_3d_vector(x,y,h));  sfxfix

//     if (refuel_sound)  sfxfix
//     {
//       g1_sound_man.free_dynamic_3d_sound(refuel_sound);
//       refuel_sound=0;
//     }
    supertank_lost.play();
    pf_stank_think.stop();
    return;
  }

  check_for_refuel();
  check_for_powerups();
  
  i4_bool process_input = i4_T;
  i4_bool remote_stank=i4_F;//true if this stank belongs to one with an ai_remote

  if (i4_const_str("ai_remote")== *(g1_player_man.get(player_num)->get_ai()->ai_name()))
	  {
	  remote_stank=i4_T;
	  process_input=i4_F;
	  }

  w32 pathinfo = follow_path();
  

  if (!g1_current_controller.get() ||
       (g1_current_controller->view.get_view_mode()!=G1_ACTION_MODE) &&
       g1_current_controller->view.get_view_mode()!=G1_FOLLOW_MODE &&
       g1_current_controller->view.get_view_mode()!=G1_WATCH_MODE
       )
      process_input = i4_F;

  if (g1_human->team()!=player_num)
	  process_input=i4_F;
  // Process real user movement
  
  // User Input
  if (process_input) 
  {
    // Process mouse movement for looking around
    i4_normalize_angle(mouse_look_increment_x);

    mouse_look_x -= mouse_look_increment_x;
    i4_normalize_angle(mouse_look_x);    
    base_angle   -= mouse_look_increment_x;
    i4_normalize_angle(base_angle);

    if (mouse_look_increment_y!=0)
      mouse_look_y = turret->rotation.y;
      
    mouse_look_y += mouse_look_increment_y;
    i4_normalize_angle(mouse_look_y);

    if (mouse_look_y < 3*i4_pi()/2 && mouse_look_y > i4_pi()/2)
    {
      if (mouse_look_increment_y>0)
        mouse_look_y = i4_pi()/2;
      else
        mouse_look_y = 3*i4_pi()/2;
    }
  }

  mouse_look_increment_x = 0;
  mouse_look_increment_y = 0;

  // Movement
  if (pathinfo && (player_num==g1_player_man.local_player ||!attack_target.valid()) )
  {
    // Move along the set path
    i4_float dist, dtheta, dx, dy, brakes=0.0;

    if (pathinfo==FINAL_POINT)
      brakes=0.1f;
    
    suggest_move(dist, dtheta, dx, dy, brakes);
    int mvres=move(dx,dy);
	if (mvres==MV_CANNOTCONT)
		{
		set_path((g1_path_class*)NULL);
		brakes=0.1f;
		}

    if (speed*speed>=dist)
    {
      i4_3d_vector a,b,c;

      path = g1_path_manager.advance_path(path);

      a.set(x,y,0);
      b.z=c.z=0;
      if (g1_path_manager.get_nth_position(path,0,b.x,b.y) &&
          g1_path_manager.get_nth_position(path,1,c.x,c.y))
      {
        c -= b;
        b -= a;
        b.normalize();
        c.normalize();
        brakes = (b.dot(c)<0.8f) ? 0.1f : 0.0f;
      }
    }

    if (!process_input)
      base_angle = theta;
    
    g1_add_to_sound_average(G1_RUMBLE_STANK, i4_3d_vector(x,y,h), i4_3d_vector(x-lx, y-ly, h-lh));
  }
  else 
  {
    i4_angle desired_base = base_angle;
  
    //if (process_input) 
    //{
      // process turning
      base_angle += dtheta; //dtheta is used by the ai to control the stank
      i4_normalize_angle(base_angle);
    //}

    if (!get_stank_flag(ST_INFLIGHT))
    {
      if (accel_ratio)
      {
        // process acceleration
        if (!get_stank_flag(ST_SLIDING))
          speed += defaults->accel*accel_ratio;
        speed += (float)sin(pitch)*g1_resources.gravity;
        speed -= speed*damping_fraction;
      }

      if (strafe_accel_ratio)
      {
        // do strafing
        strafe_speed += g1_resources.player_accel * strafe_accel_ratio;
        strafe_speed -= strafe_speed*damping_fraction;
      }

      desired_base = base_angle;
      if (strafe_accel_ratio<0)
      {
        if (accel_ratio<0)        desired_base = 3*i4_pi()/4+base_angle;
        else if (accel_ratio>0)   desired_base=i4_pi()/4+base_angle;
        else                      desired_base=i4_pi()/2+base_angle;
    
        i4_normalize_angle(desired_base);
      }
      else if (strafe_accel_ratio>0) 
      {
        if (accel_ratio<0)        desired_base=5*i4_pi()/4+base_angle;
        else if (accel_ratio>0)   desired_base=7*i4_pi()/4+base_angle;
        else                      desired_base=3*i4_pi()/2+base_angle;    
        
        i4_normalize_angle(desired_base);
      }
    }

    if (!get_stank_flag(ST_INFLIGHT))
    {
      // Process braking
      if (accel_ratio==0) 
      {
        speed = speed*g1_resources.player_stop_friction;
        if (speed<0.01 && speed>-0.01) 
          speed=0;        
      }
      if (strafe_accel_ratio==0)
      {
        strafe_speed = strafe_speed*g1_resources.player_stop_friction;
        if (strafe_speed<0.01 && strafe_speed>-0.01) 
          strafe_speed=0;
      }
    }

    dtheta=0;
    accel_ratio=0;
    strafe_accel_ratio=0;

    // Do desired rotation
    i4_rotate_to(theta,desired_base,g1_resources.player_turn_speed+0.1f);

    // Move me
    i4_float
      mv_speed = (float)cos(groundpitch)*speed,
      mv_strafe_speed = (float)cos(groundroll)*strafe_speed;        

    i4_float
      dx = (float)cos(base_angle)*mv_speed + (float)cos(base_angle-i4_pi()/2)*mv_strafe_speed,
      dy = (float)sin(base_angle)*mv_speed + (float)sin(base_angle-i4_pi()/2)*mv_strafe_speed;

#if 0
    if (groundpitch<-0.5 || groundpitch>0.5 || groundroll<-0.5 || groundroll>0.5)
    {
      dx += sin(groundpitch)*g1_resources.gravity;
      dy -= sin(groundroll)*g1_resources.gravity;
      set_stank_flag(ST_SLIDING,1);
    }
    else
      set_stank_flag(ST_SLIDING,0);
#endif

    move(dx,dy);
    
    //if (turret->h != cur_top_attach)
    //  turret->h = cur_top_attach;
  }

  switch ((int)(base_angle*8/(2*i4_pi())))
  {
    case 0 : radar_image=&radar_im0; break;
    case 1 : radar_image=&radar_im2; break;
    case 2 : radar_image=&radar_im4; break;
    case 3 : radar_image=&radar_im6; break;
    case 4 : radar_image=&radar_im8; break;
    case 5 : radar_image=&radar_im10; break;
    case 6 : radar_image=&radar_im12; break;
    case 7 : radar_image=&radar_im14; break;
  }
          


  // Calculate height physics & pitch & roll
  
  i4_float newground = g1_get_map()->map_height(x,y,lh);

  if (h-0.00001+vspeed > newground)
  {    
    //he's off the ground
    //dont set these to 0, just leave them the way they were
    groundpitch = lgroundpitch;
    groundroll  = lgroundroll;

    h += vspeed;
    vspeed -= g1_resources.gravity;

    set_stank_flag(ST_INFLIGHT);
  }
  else
  {
    // steady on the ground

    vspeed = newground - h - g1_resources.gravity;

    h = newground;

    i4_3d_vector z2, z3;

    z2.set(x+0.1f, y, 0);
    z3.set(x-0.1f, y, 0);
    z2.z = g1_get_map()->map_height(z2.x,z2.y,lh);
    z3.z = g1_get_map()->map_height(z3.x,z3.y,lh);
    groundpitch = -i4_atan2(z2.z - z3.z,0.5);

    z2.set(x, y+0.1f, 0);
    z3.set(x, y-0.1f, 0);
    z2.z = g1_get_map()->map_height(z2.x,z2.y,lh);
    z3.z = g1_get_map()->map_height(z3.x,z3.y,lh);
    groundroll =  i4_atan2(z2.z - z3.z,0.5);

    pitch = 0;

    if (player_num != g1_player_man.local_player)
      roll  = 0;
    else
      roll = 0.8f*roll;

    set_stank_flag(ST_INFLIGHT,0);
  }

  if (hurt>0.0)
  {
    g1_hurt_tint = i4_f_to_i((G1_NUM_HURT_TINTS-1)*hurt)+1;
    hurt-=0.05f;
    if (hurt<0.0)
      hurt = 0.0;
  }
  else
    g1_hurt_tint = 0;

  if (vspeed>0.3)  // don't jump too high
    vspeed=0.3f;
  
  // Orient turrent
  if (process_input)
  {
    turret->rotation.x = 0;
    turret->rotation.y = mouse_look_y;
    turret->rotation.z = base_angle;
    
    check_if_turret_in_ground();
  }
  else
  {
    //turret->rotation.x = 0;
    //turret->rotation.y = turret->lrotation.y = 0;
    //turret->rotation.x = turret->lrotation.x = 0;
  }
  
  // Do firing stuff
  if (turret_kick < 0.f)
    turret_kick += 0.02f;
  
  // Update delays
  for (int del=0; del<MAX_WEAPONS; del++)
    if (ammo[del].delay_remaining)
      ammo[del].delay_remaining--;

  look_for_targets();

  find_target(i4_F);
  //if (player_num!=g1_player_man.local_player)
  //  find_target();
  //else
  if (player_num==g1_player_man.local_player)
	g1_unfog_radius(i4_3d_vector(x,y,h), 7.0);//Only unfog for the local stank

//  if (process_input)
//  {
    for (int i=0; i<MAX_WEAPONS-1; i++)
      if (fire[i])
      {
        fire[i]=0;
		if (!remote_stank)
			fire_weapon(i);//don't fire with remote stanks. They will send us info about
		//bullets.
      }
//  }
//  else
  if (!process_input && g1_player_man.local_player==player_num)
  {
    // Auto-fire
    if (attack_target.valid()) 
    {
      g1_object_class *mp = attack_target.get();
  
      i4_float dx,dy,angle, dangle, dh, hangle, dist;
     
      dx = (mp->x - (x+turret->x));
      dy = (mp->y - (y+turret->y));
	  dh = ((mp->h+0.7f) - (h + turret->h)); 
	  dist= i4_fsqrt(dx*dx+dy*dy);
      //aim the turret
  
      angle = i4_atan2(dy,dx);    
      hangle = i4_atan2(dh, dist);
	  if (hangle>i4_pi_4())
		  hangle=i4_pi_4();
	  else if (hangle<-i4_pi_4())
		  hangle=-i4_pi_4();
	  i4_rotate_to(turret->rotation.x,hangle,0.2f);
      //snap it
      i4_normalize_angle(angle);    
      
      dangle = i4_rotate_to(turret->rotation.z, angle, defaults->turn_speed);
	  if (dangle<defaults->turn_speed && dangle>-defaults->turn_speed && in_range(2,mp))
		  {
		  fire_weapon(2);
		  }
    }
    else
    {
	  //find_target();
      if (!moved())
      {
        if (g1_tick_counter & 128)
          turret->rotation.z += 
            defaults->turn_speed * (float)sin(i4_2pi() * (float)(g1_tick_counter & 63) / 64.f);
        else
          turret->rotation.z -= 
            defaults->turn_speed * (float)sin(i4_2pi() * (float)(g1_tick_counter & 63) / 64.f);
      }
    }
  }
  
  request_think();
  if (process_input)
	  {
	  set_flag(NEEDS_SYNC,1);//always needs sync
      }
#ifdef NETWORK_INCLUDED
   g1_network_time_man_ptr->updatecomplete(global_id,g1_tick_counter);
#endif

  pf_stank_think.stop();
}

i4_bool g1_player_piece_class::can_attack(g1_object_class *who)
{
  return (i4_bool)(who->get_team()!=get_team() &&    //is an enemy
                   who->get_flag(TARGETABLE)  &&     //and is targetable
                   ((who->id==id&&in_range(1,who))|| //and ((is another stank and in range of missiles)
				   in_range(0,who)||in_range(2,who)));// or (can be attacked with either maingun or chaingun))
}


void g1_player_piece_class::track_base(i4_float desired_angle)
{
  i4_float d;
  
  d = i4_angle_diff(desired_angle, theta);
  
  if (d<g1_resources.strafe_turn_speed)
  {
    theta=desired_angle;
    return ;
  }

  if (i4_angle_minus(desired_angle, theta) < i4_pi())
  {
    theta += g1_resources.strafe_turn_speed;
    if (theta>=i4_2pi()) theta-=i4_2pi();
  }
  else
  {
    theta -= g1_resources.strafe_turn_speed;
    if (theta<0)
      theta += i4_2pi();
  }
}


//extern w32 g1_num_objs_in_view;
//extern i4_array<g1_object_class*> g1_objs_in_view_dyn;

g1_object_class *g1_player_piece_class::find_view_target(const i4_3d_vector &view_pos,
                                                         const i4_3d_vector &view_dir,
                                                         find_view_type type,
                                                         float max_dist)
{
  sw32 i;
  i4_float best;
  i4_3d_vector vdir=view_dir;
  vdir.normalize();

  
  if (type==USE_SLOP)
    best = (float)cos(i4_deg2rad(g1_resources.player_fire_slop_angle));
  else
    best = 0.0;

  g1_object_class *fire_at_this = 0;

  if (g1_objs_in_view_dyn.size()<= 0) return 0;
  w32 objs_to_check=g1_objs_in_view_dyn.size()>256?256:g1_objs_in_view_dyn.size();
  for (i=0;i<(int)objs_to_check;i++)
  {
    if (g1_objs_in_view_dyn[i])
    {
      g1_object_class *p = g1_objs_in_view_dyn[i];

      if (can_attack(p) && p->get_flag(DANGEROUS) && p!=(g1_object_class *)this)
      {
        i4_3d_vector source_to_target = i4_3d_vector(p->x, p->y, p->h);
        source_to_target -= view_pos;

        float dist=source_to_target.length();
        source_to_target/=dist;

        i4_float dot = (float)fabs(source_to_target.dot(vdir));
        if (dot > best && dist<max_dist)
        {          
          best = dot;
          fire_at_this = p;
        }
      }
    }
  }
  
  return fire_at_this;
}


i4_bool g1_player_piece_class::in_range(int slot_number, g1_object_class *o) const
{
  if (!o) return i4_F;

  int wtype=g1_get_object_type(ammo[slot_number].ammo_type->weapon_type);
  if (wtype==0) return i4_F;

  i4_float range=g1_object_type_array[wtype]->get_damage_map()->range;
  range *= range;
  i4_float dist = (o->x-x)*(o->x-x) + (o->y-y)*(o->y-y);

  return dist<range;
}

i4_bool g1_player_piece_class::fire_weapon(int slot_number)
{
  g1_stank_ammo_info_struct *a=ammo+slot_number;
  if (a->delay_remaining)
    a->delay_remaining--;
  else
  {
    if (a->amount)
    {  
      i4_3d_point_class bpos, bdir;  
      get_bullet_exit(bpos, bdir);

      float range=0;
      int wtype=g1_get_object_type(a->ammo_type->weapon_type);
      if (wtype)
        range=g1_object_type_array[wtype]->get_damage_map()->range;
      
      g1_object_class *fire_at=a->current_target.get();

#if 0
      if (!fire_at)
        fire_at = attack_target.get();
#endif

      // adjust fire direction to point directly at the object
	  // This doesn't make much sense for the stank, since we know
	  // exactly in which direction we're firing. 
      //if (fire_at)
      //{
      //  i4_3d_vector lead;

      //  attack_target = a->current_target.get();
      //  lead_target(bdir, slot_number);
      //  bdir -= bpos;
      //  bdir.z += fire_at->occupancy_radius()/3;
      //}
  
      g1_object_class *weapon = g1_fire(a->ammo_type->weapon_type, this, 
                                        fire_at, 
                                        bpos,
                                        bdir,
                                        a->last_fired_object.get());
      if (weapon)
      {
        a->amount--;
        a->delay_remaining=a->ammo_type->fire_delay;
        a->last_fired_object = weapon;
		if (g1_player_man.local_player==player_num)
			{
			weapon->set_flag(NEWER_SYNC,0);//a remote system cannot (and must not) know about these
			weapon->set_flag(NEEDS_SYNC,1);
			weapon->request_think();
			}
      }

      return i4_T;
    }
  }

  return i4_F;
}



void fast_transform(i4_transform_class *t,const i4_3d_vector &src, r1_3d_point_class &dst);


static r1_texture_ref target_cursor_sprite("target_cursor");
//static r1_texture_ref target_gun_cursor_sprite("gun_target_cursor");
static r1_texture_ref compass_sprite("compass");
static g1_model_ref lock_on("lock_on"), lock_on_guns("lock_on_guns");



void draw_spining_tris(float px, float py, float z,
                              float theta,
                              g1_quad_object_class *o,                            
                              int ticks)
{
  g1_vert_class *v=o->get_verts(0,0);

  i4_transform_class t;
  t.identity();
  t.mult_translate(px,py,0);
  
  float s=g1_render.center_x / z;

  if (ticks<10)
  {
    ticks=11-ticks;
    while (ticks--)      
      s*=1.1f;
  }
    
  t.mult_scale(s,s,1);
  t.mult_rotate_z(theta);
  t.mult_rotate_x(i4_pi()/2.0f);

  
  
  r1_render_api_class *api=g1_render.r_api;

  
  r1_vert tv[4];
  for (int k=0; k<4; k++)
  {
    tv[k].v.z=r1_near_clip_z*2;//newer draw anything exactly at r1_near_clip_z
	//since this would overlap with any windows drawn on top of the render window.
    tv[k].a=1;
    tv[k].w=r1_ooz(tv[k].v.z);
    tv[k].r=1;
    tv[k].g=1;
    tv[k].b=1;
  }
  
  for (int i=0; i<o->num_quad; i++)
  {
    i4_bool off=i4_F;
    
    g1_quad_class *q=o->quad+i;
    int t_verts=q->num_verts();
    for (int j=0; j<t_verts; j++)
    {
      t.transform(v[q->vertex_ref[j]].v, tv[j].v);
      tv[j].px=tv[j].v.x;
      tv[j].py=tv[j].v.y;
      
      if (tv[j].v.x<0 || tv[j].v.x>g1_render.center_x*2 ||
          tv[j].v.y<0 || tv[j].v.y>g1_render.center_y*2)
        off=i4_T;        
    }
 
    if (!off)
    {
      g1_render.r_api->use_texture(q->material_ref, 8, 0);
      g1_render.r_api->render_poly(t_verts, tv);
    }
  }
}


void g1_player_piece_class::draw_target_cursors(g1_draw_context_class *context)
{
  float z=r1_near_clip_z*1.1f;
  float w_mult=z/g1_render.center_x;
  static i4_float compass_pos=0.004f;
  static i4_float compass_z=z;
  g1_render.render_sprite(i4_3d_vector(0,0,z),
                          target_cursor_sprite.get(), 33 * w_mult, 33 * w_mult);

  i4_transform_class tr;
  tr.identity();//we want to render the line relative to the camera coordinate
  //system, so no transformation is needed.
  g1_render.render_sprite(i4_3d_vector(compass_pos,-compass_pos,compass_z),
	  compass_sprite.get(),
	  33*w_mult, 33*w_mult);
  g1_render.render_3d_line(i4_3d_vector(compass_pos,-compass_pos,compass_z),
	  i4_3d_vector((compass_pos+(0.0005f*cos(-theta))),-(compass_pos+(0.0005f*sin(theta))),compass_z),
	  0x00ffffff,0,&tr,i4_T);
  r1_vert v;

  float ctheta=(g1_tick_counter + g1_render.frame_ratio)/4.0f;
  
  if (ammo[1].current_target.get())
  {
    g1_object_class *t=ammo[1].current_target.get();
    i4_3d_vector p=i4_3d_vector(t->x, t->y, t->h+ t->occupancy_radius()/3.0f);
    i4_3d_vector lp=i4_3d_vector(t->lx, t->ly, t->lh+ t->occupancy_radius()/3.0f);
    i4_3d_vector ip;
    ip.interpolate(lp, p, g1_render.frame_ratio);

    if (g1_render.project_point(ip, v, context->transform))
      draw_spining_tris(v.px, v.py, v.v.z, ctheta,
                        lock_on_guns.get(),
                        ammo[1].ticks_this_has_been_my_current_target);
  }

  ctheta+=0.4f;
  if (ammo[2].current_target.get())
  {   
    g1_object_class *t=ammo[2].current_target.get();

    i4_3d_vector p=i4_3d_vector(t->x, t->y, t->h+ t->occupancy_radius()/3.0f);
    i4_3d_vector lp=i4_3d_vector(t->lx, t->ly, t->lh+ t->occupancy_radius()/3.0f);
    i4_3d_vector ip;
    ip.interpolate(lp, p, g1_render.frame_ratio);

    if (g1_render.project_point(ip, v, context->transform))
      draw_spining_tris(v.px, v.py, v.v.z, ctheta,
                        lock_on.get(),
                        ammo[1].ticks_this_has_been_my_current_target);
  }
  
}

void g1_player_piece_class::look_for_targets()
{
  if (!g1_player_man.get_local() ||
      g1_player_man.get_local()->get_player_num()!=player_num)
  {
    ammo[0].current_target=0;
    if (attack_target.valid())
      ammo[1].current_target = attack_target.get();
    ammo[2].current_target=0;
    return;
  }

  i4_3d_vector bpos, bdir;  
  get_bullet_exit(bpos, bdir);

  for (int i=0; i<MAX_WEAPONS; i++)
  {
    if (ammo[i].amount)
    {
      float range=0;
      int wtype=g1_get_object_type(ammo[i].ammo_type->weapon_type);
      if (wtype)
        range=g1_object_type_array[wtype]->get_damage_map()->range;

      g1_object_class *t=find_view_target(bpos, bdir, i ? USE_SCREEN : USE_SLOP, range);
    
      if (t==ammo[i].current_target.get())      
        ammo[i].ticks_this_has_been_my_current_target++;
      else
      {
        ammo[i].ticks_this_has_been_my_current_target=0;
        ammo[i].current_target=t;
      }
    }
    else
    {
      ammo[i].ticks_this_has_been_my_current_target=0;
      ammo[i].current_target=0;
    }
  } 
}

void g1_player_piece_class::draw(g1_draw_context_class *context)
{
  g1_player_piece_class *local_stank=g1_player_man.get_local()->get_commander();
    
  i4_bool in_local_stank=i4_F;

  if (g1_current_controller.get() &&
      this==local_stank && g1_current_controller->view.get_view_mode()==G1_ACTION_MODE &&
	  ((g1_current_controller->view.follow_object_id==global_id) ||
      (g1_current_controller->view.follow_object_id==0)))
      {
      in_local_stank=i4_T;
      }

  if (in_local_stank)
    draw_target_cursors(context);

  
  sw8 damage_level;

  if (health > defaults->health)
    damage_level = 7;
  else
  if (health < 0)
    damage_level = 0;
  else
    damage_level = (sw8)i4_f_to_i(7.f * health / (float)defaults->health);
  
  g1_render.set_render_damage_level(damage_level);

  g1_screen_box *bbox=0;

  if (get_flag(g1_object_class::SELECTABLE | g1_object_class::TARGETABLE))
  { 
    if (g1_render.current_selectable_list)
      bbox=g1_render.current_selectable_list->add();
    if (bbox)
    {
      bbox->x1 = 2048;
      bbox->y1 = 2048;
      bbox->x2 = -1;
      bbox->y2 = -1;
      bbox->z1 = 999999;
      bbox->z2 = -999999;
      bbox->w  = 1.0f/999999.0f;
      bbox->object_id=global_id;
	  bbox->flags=0;
    }
  }

  
  //draw with this model
  g1_quad_object_class *model = draw_params.model;

  //3d vectors
  i4_transform_class out;
  i4_transform_class *old = context->transform;  
  context->transform = &out;

  out.multiply(*old, *world_transform);

  if (!in_local_stank)
  {
    g1_render.render_object(model,
                            &out,
                            world_transform,
                            1,                     
                            player_num,
                            draw_params.frame,
                            bbox,
                            0);
  }
  
  turret->offset.x = i4_interpolate(lturret_kick, turret_kick, g1_render.frame_ratio);


  // draw barrel with alpha if in action mode and on the local supertank
  if (in_local_stank)
  {
    
    //g1_render.r_api->set_alpha_mode(R1_ALPHA_CONSTANT);
    //g1_render.r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR);
    //g1_render.r_api->set_shading_mode(R1_COLORED_SHADING);
    //g1_render.r_api->set_constant_color((w32)(0.80 * 255) << 24);
	turret->defmodeltype=turret_ref.id();
    i4_3d_vector r     = turret->rotation;
    i4_3d_vector old_r = turret->lrotation;
    
    turret->rotation.z  = turret->rotation.z  - theta;
    turret->lrotation.z = turret->lrotation.z - ltheta;

    i4_transform_class turret_local;
    turret->calc_transform(g1_render.frame_ratio, &turret_local);

    turret->draw(context,
                world_transform,
                bbox,
                player_num,
                &turret_local);
    
    turret->rotation  = r;
    turret->lrotation = old_r;

    //g1_render.r_api->set_alpha_mode(R1_ALPHA_DISABLED);
    //g1_render.r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_W | R1_WRITE_COLOR);
  }
  else
  {
    turret->defmodeltype=turret_full_ref.id();
    i4_3d_vector r     = turret->rotation;
    i4_3d_vector old_r = turret->lrotation;
    
    turret->rotation.x  = 0;
    turret->lrotation.x = 0;
    
    //turret->rotation.y  = 0;
    //turret->lrotation.y = 0;

    turret->rotation.z  = turret->rotation.z  - theta;
    turret->lrotation.z = turret->lrotation.z - ltheta;

    turret->draw(context,
                world_transform,
                bbox,
                player_num);
    
    turret->rotation  = r;
    turret->lrotation = old_r;
  }

  //get back the old context transform  
  context->transform = old;

  g1_render.set_render_damage_level(-1);

  if (path&&(g1_current_controller->view.get_view_mode()==G1_EDIT_MODE))  
	  //no idea why this should be drawn in action mode.
  {
    i4_3d_point_class a(x,y,h+0.1f),b(dest_x,dest_y,0),c;

    b.z = g1_get_map()->terrain_height(b.x,b.y)+0.1f;
    c = b;
    g1_path_manager.get_nth_position(path,1,c.x,c.y);
    c.z = g1_get_map()->terrain_height(c.x,c.y)+0.1f;
    g1_render.render_3d_line(a,b,0x00ff00,0x00ff00,context->transform);
    g1_render.render_3d_line(b,c,0x00ff00,0x00ff00,context->transform);
  }

}


void g1_player_piece_class::get_bullet_exit(i4_3d_vector &pos,
                                            i4_3d_vector &dir)
{
#if 1
  i4_3d_vector r  = turret->rotation;
  i4_3d_vector lr = turret->lrotation;
  
  turret->rotation.z  = turret->rotation.z  - theta;
  turret->lrotation.z = turret->lrotation.z - ltheta;

  i4_transform_class t, main, l2w;
  turret->calc_transform(1.0, &t);    
  calc_world_transform(1.0, &main);
  l2w.multiply(main, t);
    
  l2w.transform(muzzle_exit, pos);
  dir=l2w.x;

  turret->rotation = r;
  turret->lrotation = lr;
#else
  i4_transform_class spare_transform;

  i4_transform_class temp_transform;

  spare_transform.identity();

  temp_transform.rotate_x(groundroll);
  spare_transform.multiply(temp_transform);

  temp_transform.rotate_y(groundpitch);
  spare_transform.multiply(temp_transform);
                 
  temp_transform.rotate_z(turret->rotation.z);
  spare_transform.multiply(temp_transform);

  temp_transform.rotate_y(turret->rotation.y);
  spare_transform.multiply(temp_transform);

  i4_3d_vector bullet_pos;
  i4_3d_vector bullet_dir;
  i4_3d_vector tmp;
  
  bullet_pos.x = g1_resources.player_turret_radius;
  bullet_pos.y = 0.023;
  bullet_pos.z = 0.17;

  bullet_dir.x = g1_resources.bullet_speed;
  bullet_dir.y = 0;
  bullet_dir.z = 0;
  
  spare_transform.transform(bullet_pos, pos);

  pos.x += x;
  pos.y += y;
  pos.z += h;
  
  spare_transform.transform(bullet_dir, dir); 
#endif
}

g1_player_piece_class::~g1_player_piece_class()
{
  if (player_num==g1_player_man.local_player)
    g1_hurt_tint=0;
}


static int first_hurt=1;

void g1_player_piece_class::damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
{
  int dowait=0;
  if (player_num == g1_player_man.local_player)
  {
    if (hp>10)
    {
      i4_angle dir = i4_atan2(_damage_dir.y, _damage_dir.x);
      
      dir -= theta;
      i4_normalize_angle(dir);
      
      if (dir>i4_pi())
        roll = 0.08f;
      else
        roll = -0.08f;
    }

    hurt +=(hp)/100.0f;
    hurt = (hurt>0.99999f)?0.99999f : hurt;

    if (first_hurt)
    {
      first_hurt=0;
      return_to_repair.play();
    }
    if (health!=0 && health-hp<=0)
    {
      if (g1_current_controller.get())
      {
	    //only switch to circle_wait if action or camera mode
	    //otherwise, this is usually of no use.
	    if (((g1_current_controller->view.follow_object_id==0)  ||
            (g1_current_controller->view.follow_object_id==global_id ) ) &&
            ((g1_current_controller->view.get_view_mode()==G1_ACTION_MODE) ||
			(g1_current_controller->view.get_view_mode()==G1_CAMERA_MODE)))
			{
			g1_current_controller->view.suggest_camera_mode(G1_CIRCLE_WAIT,
                                                         global_id);
			dowait=1;
			}
        g1_current_controller->scroll_message("Death cometh. Returning to base...");        
      }
    }
  }

  if (health!=0 && health-hp<=0)
  {
    if (dowait)
		g1_player_man.get(player_num)->continue_wait=1;
    theta = base_angle;
    g1_create_carcass(this, stank_carcass.get(), "chunk_supertankbase");
  }

  if ((stank_flags & ST_GODMODE)==0)    
    g1_map_piece_class::damage(obj, hp, _damage_dir);
  
}

void g1_player_piece_class::notify_damage(g1_object_class *obj, sw32 hp)
{
  // did we kill this guy?
  if (obj->health-hp<=0 && obj->get_flag(DANGEROUS))
    g1_player_man.get(player_num)->add_points(obj->get_type()->defaults->cost);
}

void g1_player_piece_class::calc_action_cam(g1_camera_info_struct &cam,
                                            float fr)
{  
  i4_transform_class cam_transform;
  
  i4_3d_vector interp_pos,tmp;
  interp_pos.interpolate(i4_3d_vector(lx,ly,lh), i4_3d_vector(x,y,h), fr);

  cam.ground_rotate = i4_interpolate_angle(turret->lrotation.z, turret->rotation.z, fr);
  cam.horizon_rotate = i4_interpolate_angle(turret->lrotation.y, turret->rotation.y, fr);
  cam.roll  = i4_interpolate_angle(lroll, roll, fr);
  cam.ground_x_rotate = i4_interpolate_angle(lgroundroll,  groundroll, fr);
  cam.ground_y_rotate = i4_interpolate_angle(lgroundpitch, groundpitch, fr);
    
  cam_transform.translate(interp_pos.x, interp_pos.y, interp_pos.z);

  cam_transform.mult_rotate_x(cam.ground_x_rotate);
  cam_transform.mult_rotate_y(cam.ground_y_rotate);
  cam_transform.mult_rotate_z(cam.ground_rotate);
  cam_transform.mult_rotate_y(cam.horizon_rotate);
  
  //hardcoded position of the camera, relative to (0,0,0) of the turret model
  cam_transform.transform(i4_3d_vector(0.15f, 0.023f, 0.12f), tmp);

  cam.gx = tmp.x;
  cam.gy = tmp.y;
  cam.gz = tmp.z;
}
