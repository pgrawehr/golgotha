#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/map_piece.h"
#include "map.h"
#include "map_man.h"
#include "math/pi.h"
#include "math/trig.h"
#include "math/angle.h"
#include "g1_rand.h"
#include "resources.h"
#include "objs/model_draw.h"
#include "objs/model_id.h"
#include "saver.h"
#include "objs/scream.h"
#include "sound/sfx_id.h"
#include "sound_man.h"
#include "objs/target.h"
#include "player.h"
#include "g1_texture_id.h"
#include "g1_render.h"
#include "sound/sound_types.h"
#include "objs/shrapnel.h"
#include "objs/explode_model.h"
#include "team_api.h"
#include "time/profile.h"
#include "li_objref.h"
#include "lisp/li_types.h"
#include "lisp/li_class.h"
#include "lisp/li_init.h"
#include "objs/path_object.h"
#include "tile.h"
#include "map_cell.h"
#include "tick_count.h"
#include "map_view.h"
#include "g1_tint.h"
#include "controller.h"
#include "solvemap.h"
#include "solvegraph_breadth.h"
#include "solvemap_breadth.h"
#include "editor/dialogs/path_win.h"
#include "transport/transport.h"
#include "map_vars.h"
#include "path.h"


int g1_show_list=0;
li_object *g1_toggle_show_list(li_object *o, li_environment *env)
{
  g1_show_list=!g1_show_list;
  return 0;
}

li_automatic_add_function(g1_toggle_show_list, "toggle_show_list");

enum { MAP_PIECE_DATA_VERSION=9 };

i4_profile_class pf_find_target("map piece::find target");
i4_profile_class pf_suggest_move("map_piece::suggest_move");
i4_profile_class pf_check_move("map_piece::check_move");
i4_profile_class pf_update_rumble_sound("map_piece::update_rumble");
i4_profile_class pf_map_piece_think("map_piece::think");
i4_profile_class pf_map_piece_move("map_piece::move");


//S1_SFX(scream1, "screams/scream0_22khz.wav", 0, 60);   // don't play screams as 3d
//S1_SFX(scream2, "screams/scream00_22khz.wav", 0, 60);

//static g1_object_type shrapnel_type=0, shockwave_type=0;
extern g1_object_type road_type;
extern g1_object_type car_type;

const int battle_step = 9;
i4_float position_table[battle_step] = { -2.0, -0.5, 1.0, -1.5, 2.0, -2.5, 3.0, -3.5, 4.0 };


i4_bool g1_map_piece_class::can_attack(g1_object_class *who)
{
  return (i4_bool)(who->get_team()!=get_team() &&
                   who->get_flag(TARGETABLE) &&
                   ((who->get_flag(GROUND | UNDERWATER | AERIAL) & 
                     (get_flag(HIT_GROUND | HIT_UNDERWATER | HIT_AERIAL)>>3)))!=0 &&
                   in_range(who));
}

/*
i4_bool g1_map_piece_class::in_range(g1_object_class *o) const
{
  float r=detection_range();
  r*=r;
  float d=(o->x-x)*(o->x-x) + (o->y-y)*(o->y-y);
  return (d<=r);
}
*/
//extern g1_object_type g1_supertank_type;

static g1_quad_class mp_tmp_quad;


// this is the default function for handling tinted polygons
g1_quad_class *g1_map_piece_tint_modify(g1_quad_class *in, g1_player_type player)
{
  mp_tmp_quad=*in;

  mp_tmp_quad.material_ref=g1_get_texture("charredvehicle");
  if (g1_tint!=G1_TINT_OFF)
    g1_render.r_api->set_color_tint(g1_player_tint_handles[player]);
    
  return &mp_tmp_quad;
}


void g1_dead_ambient(i4_transform_class *object_to_world, 
                             i4_float &ar, i4_float &ag, i4_float &ab)
{
  g1_get_map()->get_illumination_light(object_to_world->t.x, object_to_world->t.y, ar,ag,ab);
  
  ar *=0.4f;
  ag *=0.4f;
  ab *=0.4f;
}

void g1_map_piece_class::draw(g1_draw_context_class *context)
{
  if (g1_show_list)
  {
  if (next_object.get())
    g1_render.render_3d_line(i4_3d_point_class(x+0.1f,y+0.1f,h+0.1f),
                             i4_3d_point_class(next_object->x, 
                                               next_object->y+0.1f, 
                                               next_object->h+0.1f),
                             0xffff, 0, context->transform);
  if (prev_object.get())
    g1_render.render_3d_line(i4_3d_point_class(x-0.1f,y-0.1f,h+0.1f),
                             i4_3d_point_class(prev_object->x, 
                                               prev_object->y-0.1f, 
                                               prev_object->h+0.1f),
                             0xff00ff, 0, context->transform);
  }

  g1_model_draw(this, draw_params, context);

  if (context->draw_editor_stuff)
  {
    if (attack_target.valid())
    {
      i4_3d_point_class p1(x,y,h+0.1f), p2(attack_target->x,attack_target->y,attack_target->h+0.1f);
      g1_render.render_3d_line(p1,p2,0xff8000,0xff0000,context->transform);
    }
  }
}

void g1_map_piece_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);
  
  fp->start_version(MAP_PIECE_DATA_VERSION);

  if (path)
	  {
	  fp->write_8(99);
	  g1_path_manager.save(fp,path);
	  }
  else
	  {
	  fp->write_8(0);
	  }
  fp->write_format("ffffffffffff222",
                   &speed,&vspeed,
                   &dest_x,&dest_y,&dest_z,
                   &dest_theta,&fire_delay,
                   &path_pos, &path_len,
                   &path_cos, &path_sin, &path_tan_phi,
                   &stagger,
                   &draw_params.frame, &draw_params.animation);

  fp->write_reference(attack_target);
  fp->write_reference(next_object);
  fp->write_reference(prev_object);

  next_path.save(fp);
  if (path_to_follow)
  {
    g1_id_ref *r;
    int t=0;
    for (r=path_to_follow; r->id; r++, t++);  
    fp->write_16(t);
    for (r=path_to_follow; r->id; r++)
      r->save(fp);
  }
  else
    fp->write_16(0);
  
  fp->end_version();
}

void g1_map_piece_class::add_team_flag()
{
  int mini_index=num_mini_objects-1;

  if (mini_index>=0 && mini_objects[mini_index].defmodeltype==0)
  {
    mini_objects[mini_index].defmodeltype = g1_player_man.get(player_num)->team_flag.value;
    i4_3d_vector v;
    if (!draw_params.model->get_mount_point("Flag", v))
      v.set(0,0,0);
    
    mini_objects[mini_index].position(v);
  }
}

g1_map_solver_class *g1_map_piece_class::prefered_solver()
	{
	g1_map_solver_class *ms=0;
	if (my_solver&&(solveparams&SF_OK)) return my_solver;
	g1_map_class *map=g1_get_map();
	ms=map->get_prefered_solver();
	if (!(solveparams&SF_OK))
		{
		solve_flags st=solveparams&SF_GRADEMASK;
		solveparams=(map->solvehint& ~SF_GRADEMASK)|st;
		solveparams|=SF_OK;
		//solveparams=map->solvehint;
		};
	if (!ms && !(solveparams&SF_USE_PATHS))
		{
		if (solveparams&SF_USE_GRAPH)
			ms=map->get_graph_solver();
		else
			ms=map->get_breadth_solver(GRADE(solveparams));
		}
	
	if (ms==(g1_map_solver_class*)map->get_breadth_solver(0))
		{
		ms=(g1_map_solver_class*)map->get_breadth_solver(GRADE(solveparams));
		}
	my_solver=ms;
	return ms;
	}

g1_map_piece_class::~g1_map_piece_class()
	{
	if (path) g1_path_manager.free_path(path);
	}

g1_map_piece_class::g1_map_piece_class(g1_object_type id, 
                                       g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  rumble_type=G1_RUMBLE_GROUND;

  path_to_follow=0;
  
  w16 ver=0,data_size=0;
  solveparams=SF_GRADE1;
  my_solver=0;
  //prefered_solver();
  death=0;
  damping_fraction=0.8;
  dest_theta=0;
  defaults=g1_object_type_array[id]->defaults;

  damage_direction=i4_3d_vector(0,0,1);
  ticks_to_blink=0;

  path=0;
  path_to_follow=0;
  path_cos=0;
  path_len=0;
  path_pos=0;
  path_sin=0;
  path_tan_phi=0;
  stagger=0;
  if (fp)
    fp->get_version(ver,data_size);

  stagger=0;

  used_path_len=0.7f * li_get_float(li_get_value("world_scaling"),0);

  switch (ver)
  {
	  case MAP_PIECE_DATA_VERSION:
		{
		if (fp->read_8()==99) //meaning has path
			path=g1_path_manager.load(fp);
		}
		//NO break here
	case 8:
    {
      fp->read_format("ffffffffffff222",
                      &speed,&vspeed,
                      &dest_x,&dest_y,&dest_z,
                      &dest_theta,&fire_delay,
                      &path_pos, &path_len,
                      &path_cos, &path_sin, &path_tan_phi,
                      &stagger,
                      &draw_params.frame, &draw_params.animation);

      fp->read_reference(attack_target);
      fp->read_reference(next_object);
      fp->read_reference(prev_object);

      next_path.load(fp);
      int t=fp->read_16();
      if (t)
      {
        path_to_follow=(g1_id_ref *)I4_MALLOC(sizeof(g1_id_ref)*(t+1),"path_to_follow");
        for (int i=0; i<t; i++)
          path_to_follow[i].load(fp);

		path_to_follow[t].id=0;
      }

        
    } break;

    case 7:
    {
      draw_params.frame=fp->read_16();
      draw_params.animation=fp->read_16();
      speed=fp->read_float();
      fp->read_float();       // remove in next rev.
      fp->read_float();       // remove in next rev
      health=fp->read_16();
      fire_delay=fp->read_16();

      dest_x=fp->read_float();
      dest_y=fp->read_float();

      fp->read_32();          // fp->read_reference(attack_target); remove
      fp->read_32();          // fp->read_reference(convoy); remove

      int t=fp->read_16();
      if (t)
      {
        path_to_follow=(g1_id_ref *)I4_MALLOC(sizeof(g1_id_ref)*(t+1),"");
        for (int i=0; i<t; i++)
          path_to_follow[i].load(fp);
		path_to_follow[t].id=0;
      }
        
    } break;

    default:
    {
      if (fp) fp->seek(fp->tell() + data_size);

      health = defaults->health;
      speed=0;
      vspeed=0;
      tread_pan=0;
      dest_x = dest_y = dest_z = -1;
      dest_theta=0;
      fire_delay=0;      
      groundpitch = groundroll = 0;
      memset(&attack_target,0,sizeof(g1_typed_reference_class<g1_object_class>));
    } break;
  }

  if (fp)
    fp->end_version(I4_LF);
}

void g1_map_piece_class::load(g1_loader_class *fp)
	{
	g1_object_class::load(fp);
	w16 ver,ds;
	fp->get_version(ver,ds);
	g1_path_manager.free_path(path);
	path=0;
	if (fp->read_8()==99)
		path=g1_path_manager.load(fp);
	fp->read_format("ffffffffffff222",
                      &speed,&vspeed,
                      &dest_x,&dest_y,&dest_z,
                      &dest_theta,&fire_delay,
                      &path_pos, &path_len,
                      &path_cos, &path_sin, &path_tan_phi,
                      &stagger,
                      &draw_params.frame, &draw_params.animation);

      fp->read_reference(attack_target);
      fp->read_reference(next_object);
      fp->read_reference(prev_object);

      next_path.load(fp);
	  i4_free(path_to_follow);
	  path_to_follow=0;
      int t=fp->read_16();
      if (t)
      {
        path_to_follow=(g1_id_ref *)I4_MALLOC(sizeof(g1_id_ref)*(t+1),"path_to_follow");
        for (int i=0; i<t; i++)
          path_to_follow[i].load(fp);

		path_to_follow[t].id=0;
      }

	fp->end_version(I4_LF);
	}

void g1_map_piece_class::skipload(g1_loader_class *fp)
	{
	g1_object_class::skipload(fp);
	w16 v,d;
	fp->get_version(v,d);
	if (fp->read_8()==99)
		{
		//this is not at all the fastest way of doing this...
		g1_path_handle dumbpath=g1_path_manager.load(fp);
		g1_path_manager.free_path(dumbpath);
		dumbpath=0;
		};
	fp->seek(fp->tell()+52);
	fp->skip_reference();
	fp->skip_reference();
	fp->skip_reference();
	g1_id_ref::skip(fp);
	int t=fp->read_16();
	if (t)
		{
		for (int i=0;i<t;i++)
			g1_id_ref::skip(fp);//just needs an g1_id_ref to do this
		}
	fp->end_version(I4_LF);
	}

	

void g1_map_piece_class::change_player_num(int new_player_num)
	{
	if (new_player_num!=player_num)
		attack_target=0;//if something gets captured 
	//i.e a supergun, don't fire at current target.
	g1_object_class::change_player_num(new_player_num);
	}

w32 g1_map_piece_class::follow_path()
{  
  if (!path) return NO_PATH;

  g1_path_manager.get_position(path, dest_x, dest_y);

  // first check to see if we're already there  
  i4_float dx = (x-dest_x), dy = (y-dest_y);
  i4_float dist = dx*dx+dy*dy;
  //performance???
  //wouldn't really help either
  /*if ((g1_tick_counter+global_id)%10==0)//global_id ensures not all units do this simultaneously
	  {
	  i4_float dex,dey;
	  while (g1_path_manager.get_nth_position(path,1,dex,dey)&&
	      g1_map_solver_class::unblocked(
		  g1_get_map()->get_block_map(GRADE(solveflags)),
		  x,y,dex,dey)
		  {
		  path=g1_path_manager.advance_path(path);
		  }
	  }

*/
  //if we're there, advance to the next path position
  if (dist<=speed*speed||dist<=0.01)//usually dependent on speed, but piece may stop at end
  {
    path = g1_path_manager.advance_path(path);
    if (!path) return NO_PATH;
    
    g1_path_manager.get_position(path, dest_x, dest_y);
  }
  return (g1_path_manager.is_last_path_point(path))? FINAL_POINT : GAME_PATH;
}

inline void check_on_map(const i4_3d_vector &v)
{
  int vx=i4_f_to_i(v.x), vy=i4_f_to_i(v.y);
  if (vx<0 || vy<0 || vx>=g1_get_map()->width() || vy>=g1_get_map()->height())
    i4_error("off map");
}



void g1_map_piece_class::damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
{


  g1_object_class::damage(obj, hp, _damage_dir);
  if (attack_target.valid() && 
	  !attack_target->get_flag(DANGEROUS) &&
	  !get_flag(SPECIALTARGETS))
	  {//somebody shot us while we are attacking something 
	  //that cannot shot.
	  attack_target=obj;
	  }
  ticks_to_blink=20;

  
  if (health<=0)
  {
//     if ((g1_tick_counter + global_id)&1)
//       scream1.play();
//     else
//       scream2.play();
  }
  else
    ticks_to_blink=20;
}


void g1_map_piece_class::find_target(i4_bool unfog)
{
  pf_find_target.start();

  if (attack_target.valid() && 
      (!can_attack(attack_target.get()) || 
	  (!attack_target->get_flag(DANGEROUS) && 
	  !get_flag(SPECIALTARGETS))))
    attack_target = 0;

  if (!find_target_now())
  {
    pf_find_target.stop();
    return;
  }

  g1_map_class *map = g1_get_map();

  //find a target in range
  sw32 ix,iy,x_left,x_right,y_top,y_bottom;

  float r=detection_range();

  if (r<=0.01 && !get_flag(SPECIALTARGETS)) return;
  
  x_left   = i4_f_to_i(x-r); if (x_left<0)                x_left=0;
  x_right  = i4_f_to_i(x+r); if (x_right>=map->width())   x_right=map->width()-1;
  y_top    = i4_f_to_i(y-r); if (y_top<0)                 y_top=0;
  y_bottom = i4_f_to_i(y+r); if (y_bottom>=map->height()) y_bottom=map->height()-1;

  if (g1_player_man.local_player != player_num)
    unfog=i4_F;


  g1_object_class *potential_target=0;
  i4_float dist,target_dist=r*r;

  int fog_rect_x1=10000, fog_rect_y1=10000,
      fog_rect_x2=-1, fog_rect_y2=-1;
  

  for (iy=y_top;  iy<=y_bottom; iy++)
  {
    g1_map_cell_class *c=map->cell((w16)x_left,(w16)iy);
    for (ix=x_left; ix<=x_right;  ix++)
    {
      if (unfog && (c->flags & g1_map_cell_class::FOGGED))
      {
        c->unfog(ix, iy);
        if (ix<fog_rect_x1) fog_rect_x1=ix;
        if (ix>fog_rect_x2) fog_rect_x2=ix;
        if (iy<fog_rect_y1) fog_rect_y1=iy;
        if (iy>fog_rect_y2) fog_rect_y2=iy;
      }

      
      if (!attack_target.valid()&&r>0.0)
      {
        g1_object_chain_class *p = c->get_obj_list();
        
        while (p)
        {
          g1_object_class *o = p->object;
                    
          if (can_attack(o))
          {
		    if (o->get_flag(DANGEROUS)||get_flag(SPECIALTARGETS))
				{//if target is not dangerous attack it only if 
				//there is nothing dangerous nearby
				dist = (o->x-x)*(o->x-x) + (o->y-y)*(o->y-y);
				if (dist<target_dist)
					{
					potential_target = o;
					target_dist = dist;
					}
				}
          }
          
          p = p->next;
        }
      }
      c++;
    }
  }


  if (fog_rect_x2!=-1)
    g1_radar_refresh(fog_rect_x1, fog_rect_y1, fog_rect_x2, fog_rect_y2);

  if (!attack_target.valid() && potential_target)
  {
    attack_target = potential_target;
    request_think();
  }

  pf_find_target.stop();
}

void g1_map_piece_class::lead_target(i4_3d_point_class &lead, i4_float shot_speed)
{
  if (!attack_target.valid())
    return;

  lead.set(attack_target->x, attack_target->y, attack_target->h);

  g1_map_piece_class *mp = g1_map_piece_class::cast(attack_target.get());

  if (!mp) 
    return;

  if (shot_speed<0)
  {
    g1_object_type shot = g1_get_object_type(defaults->fire_type);
    shot_speed = g1_object_type_array[shot]->get_damage_map()->speed;
  }

  if (shot_speed>0)
  {
    i4_float 
      dx = mp->x - x,
      dy = mp->y - y,
      t = (float)sqrt(dx*dx + dy*dy)/shot_speed;

    i4_3d_vector mp_diff(mp->x - mp->lx, mp->y - mp->ly, mp->h - mp->lh);
    mp_diff*=t;
    lead += mp_diff;
  }
}



void g1_map_piece_class::init()
{

}


void g1_map_piece_class::init_rumble_sound(g1_rumble_type type)
{
  rumble_type=type;
}




static li_object_class_member links("links");
static li_symbol_ref reached("reached");

void g1_map_piece_class::unlink()
{
  g1_path_object_class *path;
  g1_map_piece_class *mp;

  if ((path = g1_path_object_class::cast(prev_object.get()))!=0)
  {
    int i=path->get_object_index(this);
    if (i>=0)
      path->link[i].object = next_object.get();
    else
      i4_warning("unlinking bad link 1!");
  }
  else if ((mp = g1_map_piece_class::cast(prev_object.get()))!=0)
    if (mp->next_path.get() == next_path.get())
      mp->next_object = next_object.get();
    else
      mp->prev_object = next_object.get();

  if ((path = g1_path_object_class::cast(next_object.get()))!=0)
  {
    int i=path->get_object_index(this);
    if (i>=0)
      path->link[i].object = prev_object.get();
    else
      i4_warning("unlinking bad link 2!");
  }
  else if ((mp = g1_map_piece_class::cast(next_object.get()))!=0)
    if (mp->next_path.get() == next_path.get())
      mp->prev_object = prev_object.get();
    else
      mp->next_object = prev_object.get();

  prev_object=0;
  next_object=0;
}

void g1_map_piece_class::link(g1_object_class *origin)
{
  g1_object_class *origin_next=0;
  g1_path_object_class *path;
  g1_map_piece_class *mp;

  if ((path = g1_path_object_class::cast(origin))!=0)
  {
    int i = path->get_path_index(g1_path_object_class::cast(next_path.get()));
    if (i>=0)
    {
      origin_next = path->link[i].object.get();
      path->link[i].object = this;
    }
    else
		{//we reached the end of the path
		
		if (g1_get_map()->solvehint&SF_BOUND_FORCE)
		    damage(0,health,i4_3d_vector(0,0,1));
		
    //  i4_warning("linking bad link!");
		}
  }
  else if ((mp = g1_map_piece_class::cast(origin))!=0)
  {
    if (mp->next_path.get() == next_path.get())
    {
      origin_next = mp->next_object.get();
      mp->next_object = this;
    }
    else
    {
      origin_next = mp->prev_object.get();
      mp->prev_object = this;
    }
  }

  if ((path = g1_path_object_class::cast(origin_next))!=0)
  {
    int i=path->get_object_index(origin);
    if (i>=0)
      path->link[i].object = this;
    else
      i4_warning("linking bad link!");
  }
  else if ((mp = g1_map_piece_class::cast(origin_next))!=0)
    if (mp->next_path.get() == next_path.get())
      mp->prev_object = this;
    else
      mp->next_object = this;

  next_object = origin_next;
  prev_object = origin;
}

void g1_map_piece_class::think()
{
  pf_map_piece_think.start();

  request_think();

  pitch = 0;
  roll  = 0;

  // limit the search for target to 1 every 4 ticks
  find_target();

  if (fire_delay>0)
    fire_delay--;
  
  if (next_path.valid())
  {
    dest_x = next_path->x;
    dest_y = next_path->y;
  }

  i4_float dist, dtheta, dx, dy, old_pathpos=path_pos;
  suggest_move(dist, dtheta, dx, dy, 0);
  if (check_move(dx,dy))
  {
    if ((g1_rand(62)&63)==0)
    {
      g1_camera_event cev;
      cev.type=G1_WATCH_IDLE;
      cev.follow_object=this;
      g1_current_view_state()->suggest_camera_event(cev);
    }
    
    move(dx,dy);
  }
  else
  {
    //get_terrain_info(); //can this have changed 
	//if we didn't move? Perhaps yes in some cases (buldozing, shockwave).
    if (next_path.get())
		{
		if (next_path->id!=road_type)
			get_terrain_info();
		}
	else
		{
		get_terrain_info();
		}
    
    path_pos = old_pathpos;
	speed=0;//hopefully, this doesn't interfere with something else...
	/*if (speed>5)
		{
		speed=speed/2;
		}
	else
		{
		speed=0;
		}*/
  }
  if (dist<speed)
    advance_path();
  
  if (h+vspeed-0.001 > terrain_height)
  {
    //he's off the ground
    //dont set these to 0, just leave them the way they were
    groundpitch = lgroundpitch;
    groundroll  = lgroundroll;

    h += vspeed;
    vspeed -= g1_resources.gravity;    
    request_think();
  }
  else
  {
    if (!get_flag(ON_WATER))
      h = terrain_height;
    else
    {
      h -= g1_resources.sink_rate;
      damage(0,g1_resources.water_damage,i4_3d_vector(0,0,1));
    }
    
    if (h!=lh)
      hit_ground();
    
    vspeed = h - lh - g1_resources.gravity;
  }

  pf_map_piece_think.stop();
}

//This function has been inlined
/*i4_bool g1_map_piece_class::find_target_now() const
{
  return (((g1_tick_counter+global_id)&3)==0);
}*/

void g1_map_piece_class::hit_ground()
{
  if (vspeed<-0.4)
    damage(0,health,i4_3d_vector(0,0,1));
}

void g1_map_piece_class::advance_path()
{
  if (path|| !(solveparams&SF_OK))
	  {
	  if (!my_solver) 
		  my_solver=prefered_solver();
	  }	  
  if (my_solver) return;
	  
  //g1_team_type type=g1_player_man.get(player_num)->get_team();
      
  if (next_path.valid())
  {
    message(reached.get(), new li_g1_ref(next_path->global_id), 0);

    unlink();

    g1_path_object_class *from = g1_path_object_class::cast(next_path.get());
	
    // find next path node to go to
    if (path_to_follow)
    {
      g1_id_ref *r;
      for (r=path_to_follow; r->id && r->id!=next_path.id; r++);
      if (r->id) r++;//Irgendetwas läuft hier schief.
      if (r->id)
        next_path=*r;
      else
		  {
		  next_path.id=0;
		  //i4_free(path_to_follow);//freeing this here avoids running 
		  //path_to_follow=0;//through this loop although we already are
		  //completelly lost
		  }
    }

    if (next_path.valid())
    {
      g1_path_object_class *next=(g1_path_object_class *)next_path.get();
	  
      path_cos = next->x - x;//from->x;
      path_sin = next->y - y;//from->y;
      path_tan_phi = next->h - h; //from->h;
	  if (from->id!=road_type)
		  {
	      stagger = g1_float_rand(8)*2.0f-1.0f;
	
		  path_pos = 0;
      
		  path_len = (float)sqrt(path_cos*path_cos + path_sin*path_sin);
		  i4_float dist_w = 1.0f/path_len;
          path_cos *= dist_w;
          path_sin *= dist_w;
          path_tan_phi *= dist_w;
	      //if (path_tan_phi>0.05)
	      //	  path_tan_phi=0.05;
	      //else if (path_tan_phi<-0.05)
	      //	  path_tan_phi=-0.05;
		  }
	  else
		  {//must be both road objects, and road len is different
		  //than euclidean distance and capacity is calculated differently 
		  //and speed is much less (scaling) and and and...
		  link_id curlink=from->link_to(G1_ALLY,next);
		  if (!curlink)//something odd (like the removal of a road)
			  {//has happened
			  speed=0;
			  path_pos=0;
			  //todo: find a new path to dest or at least inform
			  //the caller that we cannot continue due 
			  //to some link that doesn't exist any more
			  return;
			  }
		  stagger=0.01f;//can we use this to drive on the 
		  //righthand side of the road?
		  path_pos=0;
		  g2_link *ln=g2_link_man()->get_link(curlink);
		  path_len=ln->get_length();
		  i4_float dist_w=1.0f/path_len;
		  path_cos *=dist_w;
		  path_sin *=dist_w;
		  path_tan_phi *= dist_w;
		  speed=ln->get_freespeed();// * li_get_float(li_get_value("world_scaling",0),0);
		  }
      link(from);
	  return;
    }
    //else
    //{
      //unoccupy_location();//I want to see these objects once!
      //request_remove();
    //}
  }
//  else//no more valid paths
//	  {
	  if (g1_get_map()->solvehint&SF_BOUND_FORCE)
		  damage(0,health,i4_3d_vector(0,0,1));
	  else
		  {
		  solveparams&= ~SF_USE_PATHS;//cannot use paths any longer
		  enter_formation(FO_ENDPATH,0);
		  }
//	  }
}

void g1_map_piece_class::return_to_path()
	{
	//g1_team_type type=g1_player_man.get(player_num)->get_team();
	//i4_bool unlinked=i4_F;
	g1_path_object_class *from = 0;
	/*if (next_path.valid()
		&&(fabs(next_path->x-x)<1.0f)//are we far off the path?
		&&(fabs(next_path->y-y)<1.0f))
		{
		message(reached.get(), new li_g1_ref(next_path->global_id), 0);
		//assume we reached the current path object (actually we will
		//skip it and go directly to the next)
		unlink();
		unlinked=i4_T;
		
		from=g1_path_object_class::cast(next_path.get());
		
		// find next path node to go to
		if (path_to_follow)
			{
			g1_id_ref *r;
			for (r=path_to_follow; r->id && r->id!=next_path.id; r++);
			if (r->id) r++;
			if (r->id)
				next_path=*r;
			else
				next_path.id=0;
			}
		}*/
		
	if (next_path.valid())
		{
		g1_path_object_class *next=(g1_path_object_class *)next_path.get();
			
		path_cos = next->x - x;//cheat and calculate path from current position
		path_sin = next->y - y;
		path_tan_phi = next->h - h;
		stagger = g1_float_rand(8)*2.0f-1.0f;
			
		path_pos = 0;
			
		path_len = (float)sqrt(path_cos*path_cos + path_sin*path_sin);
		i4_float dist_w = 1.0f/path_len;
		path_cos *= dist_w;
		path_sin *= dist_w;
		path_tan_phi *= dist_w;
			
		//if (unlinked) link(from);
		}
	
	
	}



i4_bool g1_map_piece_class::check_turn_radius()
//{{{
{  
  i4_float cx,cy;
  i4_float r,d,d1,d2,rx,ry; 
  
  //get the radius of the circle he's currently capable of turning through  
  //make it extra-large so he doesnt make ridiculously large turns
  //check r^2... a bit faster
  r = (speed/defaults->turn_speed) * 1.5f;

  rx = r*(float)sin(theta);
  ry = r*(float)cos(theta);

  r = r*r;
  
  //check the two circles that are currently unreachable
  //if the destination lies within either circle, return false  
  cx = x - rx;
  cy = y + ry;
  d1 = (dest_x - cx);
  d1 *= d1;
  d2 = (dest_y - cy);
  d2 *= d2;
  d  = d1+d2;
  if (d<r) return i4_F;  
    
  cx = x + rx;
  cy = y - ry;
  d1 = (dest_x - cx);
  d1 *= d1;
  d2 = (dest_y - cy);
  d2 *= d2;
  d  = d1+d2;
  if (d<r) return i4_F;
  
  return i4_T;
}
//}}}

void g1_map_piece_class::request_remove()
//{{{
{
  if (path_to_follow)
    i4_free(path_to_follow);
  path_to_follow = 0;
  unlink();
  g1_object_class::request_remove();
}
//}}}

i4_bool g1_map_piece_class::suggest_move(i4_float &dist,
                                         i4_float &dtheta,
                                         i4_float &dx, i4_float &dy,
                                         i4_float braking_friction,
                                         i4_bool reversible)
	{
	if (path|| (!(solveparams&SF_OK) && prefered_solver()))
		{
		w32 path_info=follow_path();
		if (path_info==FINAL_POINT)
			braking_friction=0.1f;

		if (path_info)
			{
			i4_float angle,t ,diffangle;
			//copied from stank
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
			if (!reversible) go_reverse=i4_F;//if vehicle cannot move backwards
			
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
				
				i4_float stop_dist=1.0f;
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
						
						speed -= speed*damping_fraction;
						speed += accel;
						}
					}
				dist = speed*(float)cos(pitch);
				dx = (float)cos(theta) * dist;
				dy = (float)sin(theta) * dist;
				//dist = t;
				return i4_T;
				}
			
			}
		dtheta = 0;
		dist = 0;
		speed = 0;
		dx = 0;
		dy = 0;
		return i4_F;
		
		
		
	  }
	  if (!prev_object.get() || !next_object.get())
		  {
		  dx=dy=0;
		  dtheta=0;
		  dist=path_len-path_pos;
		  return i4_F;
		  }
	  
	  pf_suggest_move.start();
	  
	  i4_float angle, scale=0.0;
	  i4_bool on_road=i4_T;
	  if (next_path->id!=road_type)
		  {
		  speed = defaults->speed * (1.0f - damping_fraction);
		  on_road=i4_F;
		  }
	  /*
	  if (next_path->id==road_type)// (*next_path).id
		  {
		  //speed = speed + 0.01;// * (1.0f - damping_fraction);
		  on_road=i4_T;
		  }
	  else
		  {
		  speed = defaults->speed * (1.0f - damping_fraction);
		  on_road=i4_F;
		  }
	  */
	  path_pos += speed;
	  if (on_road)
		  {
		  g1_id_ref *r;
		  for (r=path_to_follow; r->id && r->id!=next_path.id; r++);
		  if (r->id) 
			  {
			  r--;
			  g1_path_object_class *pr=(g1_path_object_class*)r->get(),
				  *nx=(g1_path_object_class*)next_path.get();
		      //float reallen= sqrt((pr->x-nx->x)*(pr->x-nx->x)+(pr->y-nx->y)*(pr->y-nx->y));
			  float distx=(nx->x - pr->x);
			  float disty=(nx->y - pr->y);
			  
			  float distpt=(sqrt(distx*distx+disty*disty) )*(path_pos/path_len);
			  //float th=i4_atan2(disty,distx);
		      //float posx=((nx->x-pr->x)/path_len)*path_pos;
		      //float posy=((nx->y-pr->y)/path_len)*path_pos;
			  dx=pr->x - x + cos(theta)*distpt;
			  dy=pr->y - y + sin(theta)*distpt;
		      //dx=pr->x+posx-x;
		      //dy=pr->y+posy-y;
			  }
		  }
	  else
		  {
		  if (path_len>6.0)
			{
			if (path_pos<3.0)
			  scale = path_pos/3.0f;
			else if (path_pos>path_len-3.0)
			  scale = (path_len-path_pos)/3.0f;
			else
			  scale = 1.0f;
			}
		  dx = (dest_x - path_cos*(path_len - path_pos) - path_sin*stagger*scale - x);
		  dy = (dest_y - path_sin*(path_len - path_pos) + path_cos*stagger*scale - y);
		  }
	  
	  
	  //aim the vehicle    
	  angle = i4_atan2(dy,dx);
	  i4_normalize_angle(angle);    
	  
	  dtheta = angle - theta;
	  if (dtheta<-i4_pi()) 
		  dtheta += i4_2pi();
	  else if (dtheta>i4_pi()) 
		  dtheta -= i4_2pi();
	  
	  if (dtheta<-defaults->turn_speed) 
		  dtheta = -defaults->turn_speed;
	  else if (dtheta>defaults->turn_speed) 
		  dtheta = defaults->turn_speed;
	  theta += dtheta;
	  i4_normalize_angle(theta);
	  
	  dist = path_len-path_pos;
	  
	  pf_suggest_move.stop();
	  return (dist==0);
}



i4_bool g1_map_piece_class::suggest_air_move(i4_float &dist,
                                             i4_float &dtheta,
                                             i4_3d_vector &d)
	{
	pf_suggest_move.start();
	
	if (path|| (!(solveparams&SF_OK) && prefered_solver()))
		{
		w32 path_info=follow_path();
		i4_float braking_friction;
		if (path_info==FINAL_POINT)
			{
			braking_friction=0.2f;
			}
		else
			{
			braking_friction=0;
			}
		i4_float dx=0,dy=0,dz=0;
		if (path_info)
			{
			i4_float angle,t ,diffangle;
			i4_bool can_get_there = i4_T;
			
			dx = (dest_x - x);
			dy = (dest_y - y);
			i4_float height=g1_get_map()->terrain_height(x,y);
			if (h<(height+FLY_HEIGHT-4*VSPEED))
				{
				dz=VSPEED;
				}
			else if (h>(height+FLY_HEIGHT+4*VSPEED))
				{
				dz=-VSPEED;
				}
			//avoid crashing in the ground
			if (h<(height+1.7))
				braking_friction=0.2f;
			
			
			//aim the vehicle    
			angle = i4_atan2(dy,dx);
			i4_normalize_angle(angle);    
			
			diffangle = angle - theta;
			if (diffangle<-i4_pi()) diffangle += 2*i4_pi();
			else if (diffangle>i4_pi()) diffangle -= 2*i4_pi();
			
			//it's a plane, so there's no big point in going backwards
			//Actually, the helicopter would be able to do this, but not here.
			//dont worry about turn radii unless he's actually ready for forward movement
			can_get_there = check_turn_radius();
			dtheta = diffangle;
			
			//distance to move squared
			t = dx*dx + dy*dy;
			
			//how far will the vehicle go if he slows down from his maximum speed?
			if (t>speed*speed+0.0025 || braking_friction==0.0)
				{
				if (dtheta<-defaults->turn_speed) dtheta = -defaults->turn_speed;
				else if (dtheta>defaults->turn_speed) dtheta = defaults->turn_speed;
				theta += dtheta;
				i4_normalize_angle(theta);
				
				i4_float stop_dist=1.0f;
				if (braking_friction>0.0)
					stop_dist = (float)fabs(speed)/braking_friction; // geometric series of braking motion
				
				if (braking_friction>0.0 && t<=(stop_dist*stop_dist))
					{            
					//just in case our calculations were off, dont let him slow to less than 0.01
					//hm, on roads, due to scaling, these constants must be much less.
					//we don't consider planes here
					if (speed <= 0.01)
						speed = 0.01f;
					else
						speed *= (1.0f-braking_friction);
					}
				else
					{
					
					i4_float /*target_speed,*/ accel;
					
					if (!can_get_there)
						{
						if (braking_friction<=0.1f)
							braking_friction=0.1f;
						speed *= (1.0f-braking_friction);
						}
					else 
						{
						accel = defaults->accel;
						
						accel += (float)sin(pitch)*g1_resources.gravity;
						
						//speed -= speed*damping_fraction;
						speed += accel;
						}
					}
				if ((speed*1.1)>defaults->speed)
					speed=defaults->speed*1.1;
				dist = speed*(float)cos(pitch);
				dx = (float)cos(theta) * dist;
				dy = (float)sin(theta) * dist;
				d.x=dx;
				d.y=dy;
				d.z=dz;
				//dist = t;
				pf_suggest_move.stop();
				return i4_T;
				}
			
			}
		dtheta = 0;
		dist = 0;
		speed = 0;
		//dx = 0;
		//dy = 0;
		d.x=0;
		d.y=0;
		d.z=0;
		pf_suggest_move.stop();
		return i4_F;
		
	  }
	  if (!prev_object.get() || !next_object.get())
		  {//we don't have a global assigned path nor a local command
		  d.x=d.y=d.z=0;
		  dist=0;
		  dtheta=0;
		  speed=0;
		  pf_suggest_move.stop();
		  return i4_F;
		  }
	//code bellow for following paths only

	i4_float angle, scale=0.0,pl,curlen;
	i4_bool faroff=i4_F;
	if (speed<defaults->speed)
		{
		speed+=defaults->accel;
		}
	
		/*if (path_len>6.0)
		{
		if (path_pos<3.0)
		scale = path_pos/3.0f;
		else if (path_pos>path_len-3.0)
		scale = (path_len-path_pos)/3.0f;
		else
		scale = 1.0f;
		
	}*/
	
	path_pos += speed;
	pl=path_len-path_pos;
	//d.x = cos(theta)*speed;
	d.x=(dest_x - path_cos*pl - path_sin*stagger*scale - x);
	//d.y = sin(theta)*speed;
	d.y=(dest_y - path_sin*pl + path_cos*stagger*scale - y);
	d.z = (dest_z - path_tan_phi*pl - h);
	curlen=d.x*d.x+d.y*d.y;
	if (curlen>(2*defaults->speed*defaults->speed))
		{
		faroff=i4_T;//might happen if object unexspectedly looses its target
		}
	//aim the vehicle    
	angle = i4_atan2(d.y,d.x);
	i4_normalize_angle(angle);    
	
	dtheta = angle - theta;
	if (dtheta<-i4_pi()) dtheta += i4_2pi();
	else if (dtheta>i4_pi()) dtheta -= i4_2pi();
	
	if (dest_theta<0&&dtheta>0) dtheta=0;//don't flip sign of dtheta
	//in just one time step
	if (dest_theta>0&&dtheta<0) dtheta=0;
	i4_float dft=defaults->turn_speed;//abbreviation
	if (dtheta<-dft) 
		{
		//tanb=tan(dtheta);
		//dtheta = -dft;
		//tand=tan(dtheta);
		//if (tanb<4*tand||tanb>0)
		//  faroff=i4_T;//we are facing quite the other direction we want to go
		if (dtheta<-3*dft)
			faroff=i4_T;
		dtheta=-dft;
		
		}
	else if (dtheta>dft) 
		{
		//tanb=tan(dtheta);
		//dtheta = dft;
		//tand=tan(dtheta);
		//if (tanb>4*tand||tanb<0) //should also be dependent on the
		//distance to the target
		//  faroff=i4_T;
		if (dtheta>3*dft)
			faroff=i4_T;
		dtheta=dft;
			
		}
	else 
			//try to avoid overshoot. (right-left-right flipping)
		{
		if (dtheta>0)
			dtheta=dtheta>(dft/8)?dtheta/4:0;
		else if (dtheta<0)
			dtheta=dtheta<(-dft/8)?dtheta/4:0;
		}
		
		
		dist = pl;
		//if (dtheta!=0) faroff=i4_T;
		if (faroff)//we must calculate the way on the actual position.
			{
			d.x=(float)cos(theta)*speed;
			d.y=(float)sin(theta)*speed;
			//if (d.z+h<terrain_height+1.0f)
			//	  {
			//	  d.z+=0.01;
			//	  }
			dist=1.0f;//just in case...
			if (next_path.valid())
				{
				g1_path_object_class *next=(g1_path_object_class *)next_path.get();
				
				path_cos = next->x - x;//cheat and calculate path from current position
				path_sin = next->y - y;
				path_tan_phi = next->h - h;
				//stagger = g1_float_rand(8)*2.0f-1.0f;
				
				path_pos = 0;
				
				path_len = (float)sqrt(path_cos*path_cos + path_sin*path_sin);
				i4_float dist_w = 1.0f/path_len,
					r=(float)fabs(speed/dtheta);//aproximation of minimum turn radius
				//calculated from:
				//u=2pi*speed/dtheta; r=u/2pi
				dist = path_len;
				if (fabs(path_cos)<=2*r&&fabs(path_sin)<=2*r)
					dist=speed/4;//to be shure caller calls advance_path() soon
				path_cos *= dist_w;
				path_sin *= dist_w;
				path_tan_phi *= dist_w;
				//if (path_tan_phi>0.05)
				//  path_tan_phi=0.05;
				//else if (path_tan_phi<-0.05)
				//  path_tan_phi=-0.05;
				
				}
			}
		if (d.z>0.1f)
			{
			d.z=0.1f;
			}
		else if (d.z<-0.1f)
			{
			d.z=-0.1f;
			}
		theta += dtheta;
		i4_normalize_angle(theta);
		if (dtheta!=0)
			{
			dest_theta=dtheta;//save last theta (actually, we only need the sign)
			}
		else 
			{
			dest_theta=0;
			}
		pf_suggest_move.stop();
		return (dist==0);
	}
/*
g1_map_piece_class *g1_map_piece_class::cast(g1_object_class *obj)
{
    if (!obj || !(obj->get_type()->get_flag(g1_object_definition_class::TO_MAP_PIECE)))
      return 0;
    return (g1_map_piece_class*)obj;
}
*/
i4_bool g1_map_piece_class::can_enter_link(g1_path_object_class *from, g1_path_object_class *to)
	{
	int i=from->get_path_index(to);
	g1_map_piece_class *v;
	if (i>=0)
		{
		v=g1_map_piece_class::cast(from->link[i].object.get());
		if (v && (v->path_pos <= (v->used_path_len*1.5f)) && 
			(v->path_pos<=22.0f))// ?????
			{
			return i4_F;
			}
		return i4_T;
		}
	return i4_F;//shan't happen...
	}

g2_link *g1_map_piece_class::link_on()
	{
	g1_path_object_class *last;
	g1_id_ref *r=path_to_follow;
	//if (!g2_link_man() || path_to_follow->id==0) 
	//	return 0;
	w32 searchid=next_path.id;
	//for (;r->id && r->id != next_path.id;r++);
	for (;r->id!=searchid;r++);
	r--;
	if (r->id)
		{
		last=(g1_path_object_class*)r->get();
		link_id lid=last->link_to(G1_ALLY,(g1_path_object_class*)next_path.get());
		return g2_link_man()->get_link(lid);
		}
	return 0;
	}

i4_bool g1_map_piece_class::check_move(i4_float &dx,i4_float &dy)
{
  pf_check_move.start();
  i4_bool ret=i4_T;
  i4_bool calced=i4_F;
  //check_collision(from,to);//that function can only be tested to see wheter 
  //we hit a given (known) target
  g1_object_class *blocking=0;
  //sw32 _ix=x,_iy=y;
  /*
  if (g1_get_map()->check_collision(x,y,
	  occupancy_radius(),dx,dy,_ix,_iy, this, blocking))
	  {
	  //the "blocking" object blocks our way.
	  dx=0;dy=0;
	  pf_check_move.stop();
	  return i4_F;
	  }
  */
  float olddx=dx,olddy=dy;
  if ( g1_get_map()->check_poly_collision(x,y,
	  occupancy_radius(),dx,dy,this,blocking))
	  {
	  //i4_warning("Hit object %i. Will move by (%f/%f) instead of (%f/%f)",blocking->global_id,dx,dy,olddx,olddy);
	  // TODO
	  //if the new dx/dy pair points far from the original
	  //direction, return false
	  }
  //g2_link *own, *other;
  if (next_object.valid())
  {
    g1_path_object_class *path;
    g1_map_piece_class *mp;
    
    if ((path = g1_path_object_class::cast(next_object.get()))!=0)
    {
	  //First case: The object ahead is a path object (probably an intersection)  
	 
	  //PG: Fixme: Units may also use the wrong direction
	  //for their team if:
	  //- It is built in a (captured) enemy factory
	  //- The path is just a move-out-of-factory indicator
	  //- We use traffic_sim. Here, all paths go in G1_ALLY
	  //  direction.
	 
	  //I do not yet know what consequences this error has.
	  //Perhaps it will be eliminated if complete collision detection
	  //is added.
      g1_team_type team=g1_player_man.get(player_num)->get_team();
    
      // check branches
      for (int i=0; i<path->total_links(team); i++)
      {
        mp = g1_map_piece_class::cast(path->get_object_link(team,i));

        if (mp && mp!=this)
        {
          i4_float dist=path_len - path_pos;
          
          if (mp->player_num!=player_num)
          {
            // enemy vehicle
            dist+=mp->path_len-mp->path_pos;
            if (dist<(2+used_path_len))
              ret=i4_F;
          }
          else
          {
            // allied vehicle
		    // perhaps we can simulate parallel update by
		    // increasing the required dist if my speed is 
		    // zero and the speed of the guy before me is not.
		    // or: If check_move() fails we don't just stop but
		    // decrease our speed significantly.
		  //we should also make the required distance dependent
		  //on the actual (not only the relative) speed of myself
            dist += mp->path_pos;
			if (path!=mp->prev_object.get())//must be the last on that link
				continue;
			//other=mp->link_on();
			//if (own && own!=other)//the other vehicle is going the other way
			//	continue;

            if (dist<used_path_len)
              ret=i4_F;
			else if (dist<used_path_len*2 && !calced)
				{//this code is experimental, it may leed to 
				//unstable conditions
				calced=i4_T;
				if (speed>mp->speed)
					{//the guy before me is slower
					speed=speed*0.8;//above statement implies speed>0
					dx=dx*0.8;//exact??
					dy=dy*0.8;
					}
				else if (mp->speed < 8)
					{
					ret=i4_F;
					//speed=speed*0.9;
					//dx=dx*0.9;
					//dy=dy*0.9;
					}
				else
					{
					speed=speed*0.9;
					dx=dx*0.9;
					dy=dy*0.9;
					}

				}
          }
        }
      }
    }
    else if ((mp = g1_map_piece_class::cast(next_object.get()))!=0)
    {
	  //Case 2: There's another vehicle ahead.
      i4_float dist = -path_pos;
      
      if (mp->player_num!=player_num)
      {
        // enemy vehicle
        dist+=mp->path_len-mp->path_pos;
        if (dist<(2+used_path_len))
          ret=i4_F;
      }
      else
      {
        // allied vehicle
        dist += mp->path_pos;
		//other=mp->link_on();//very slow: need to optimize this
		//own=link_on();
		
		//if (own && own!=other)//the other vehicle is going the other way
		//	ret=i4_T;
		if (car_type==id && next_path.id!=mp->next_path.id)
			ret=i4_T;
        else if (dist<used_path_len)
          ret=i4_F;
		//trying to use more realistic code that relies on the
		//two seconds rule
		else if (dist<(used_path_len+(2*speed)))
			//2seconds * 22.0m/s+8m=44m+8m=52m suggested distance
			{//this code is experimental, it may leed to 
			//unstable conditions
			//calced=i4_T;
			if (speed>mp->speed)
				{//the guy before me is slower
				speed=speed*0.8;//above statement implies speed>0
				//dx=dx*0.8;//exact??
				//dy=dy*0.8;
				dx=fabs(cos(theta)) * 0.8 * dx;//dx already has a sign
				dy=fabs(sin(theta)) * 0.8 * dy;
				}
			else if (mp->speed < 8)//required for delayed start
				{
				ret=i4_F;
				//speed=speed*0.9;
				//dx=dx*0.9;
				//dy=dy*0.9;
				}
			else
				{
				speed=speed*0.9;
				dx=fabs(cos(theta))*dx*0.9;
				dy=fabs(sin(theta))*dy*0.9;
				
				}
			}

      }
    }
		
  }
  else
	{//no more valid paths and can't find any
	}
	
  pf_check_move.stop();
  return ret;
}

i4_bool g1_map_piece_class::check_life(i4_bool remove_if_dead)
{
  if (ticks_to_blink)
    ticks_to_blink--;

  if (health<=0)
  {
    //they were just killed. free their resources
    if (remove_if_dead)
    {
      unoccupy_location();
      request_remove();

      //keep thinking so that death scenes can be played out
    }

	//This happens way to often...
    //char msg[100];
    //sprintf(msg, "Unit Lost : %s", name());
    //g1_player_man.show_message(msg, 0xff0000, player_num);
    
    return i4_F;
  }
  return i4_T;
}

static li_symbol_ref explode_model("explode_model");


 
static li_symbol_ref set_course("set_course");

void g1_map_piece_class::set_path(g1_path_handle new_path)
{  
  if (path)
    g1_path_manager.free_path(path);
  path=new_path;
}

i4_bool g1_map_piece_class::deploy_to(float destx, float desty, g1_path_handle ph)
{
  i4_float points[MAX_SOLVEPOINTS*2];
  w16 t_points;
  //g1_path_handle ph=0;

  //if (g1_get_map()->find_path(x,y, destx,desty, points,t_points) && t_points>1)
  //  ph=g1_path_manager.alloc_path(t_points-1, points);

  //when the unit gets a individual command for the first time
  //it is disconnected from the path
  solveparams&=~SF_USE_PATHS;
  unlink();//does this hurt if we are already unlinked?
  g1_map_solver_class *ps=prefered_solver();
  if (ps&&!ph)
	  {
	  if (ps->path_solve(x,y,destx,desty,1,1,GRADE(solveparams),points,
		  t_points)&&t_points>0)
		  {
		  
		  ph=g1_path_manager.alloc_path(t_points,points);
		  }
	  }
      
  set_path(ph);

  return (ph!=0);
}
void g1_map_piece_class::set_path(g1_id_ref *list)
{
  if (path_to_follow)
    i4_free(path_to_follow);
      
  path_to_follow=list;
  next_path=list[0].id;
  
  advance_path();
  //if (!next_path.valid())
  //  i4_warning("i can't get there!");
  ltheta = theta = i4_atan2(path_sin,path_cos);
  request_think();
}



i4_bool g1_map_piece_class::move(i4_float x_amount,
                             i4_float y_amount)
{
  pf_map_piece_move.start();  

  unoccupy_location();
  
  x += x_amount;
  y += y_amount;
  
  if (!occupy_location())
  {
    pf_map_piece_move.stop();
    return i4_F;  
  }

  g1_add_to_sound_average(rumble_type, i4_3d_vector(x,y,h));


  pf_map_piece_move.stop();

  return i4_T;
}

void g1_map_piece_class::get_terrain_info()
{
  sw32 ix,iy;
  
  ix=i4_f_to_i(x);
  iy=i4_f_to_i(y);
  g1_map_cell_class *cell_on=g1_get_map()->cell((w16)ix,(w16)iy);
  
  w16 handle=cell_on->type;
//  i4_float newheight;
  
  g1_get_map()->calc_height_pitch_roll(this, x,y,h, terrain_height, groundpitch, groundroll);

  g1_tile_class *t = g1_tile_man.get(handle);
  damping_fraction = t->friction_fraction;
  g1_block_map_class *bmc=g1_get_map()->get_block_map(GRADE(solveparams));
  if (bmc->ready()&&bmc->is_blocked(ix,iy,G1_ALL_DIRS))
	  damping_fraction=damping_fraction*1.7;
  set_flag(ON_WATER, (t->flags & g1_tile_class::WAVE)!=0 && 
           terrain_height<=g1_get_map()->terrain_height(x,y));
}

//implemented because the subclasses have it virtual
void g1_map_piece_class::fire()
	{

	}

void g1_map_piece_class::calc_world_transform(i4_float ratio, i4_transform_class *t)
{
  if (!t)
    t = world_transform;

  i4_float z_rot        = i4_interpolate_angle(ltheta,theta, ratio);
  i4_float y_rot        = i4_interpolate_angle(lpitch,pitch, ratio);
  i4_float x_rot        = i4_interpolate_angle(lroll ,roll , ratio);
  
  i4_float ground_x_rot = i4_interpolate_angle(lgroundroll,  groundroll, ratio);
  i4_float ground_y_rot = i4_interpolate_angle(lgroundpitch, groundpitch, ratio);

  i4_float tx=i4_interpolate(lx,x,ratio);
  i4_float ty=i4_interpolate(ly,y,ratio);
  i4_float tz=i4_interpolate(lh,h,ratio);

  
  t->translate(tx,ty,tz);  
  t->mult_rotate_x(ground_x_rot);
  t->mult_rotate_y(ground_y_rot);
  t->mult_rotate_z(z_rot);
  t->mult_rotate_y(y_rot);
  t->mult_rotate_x(x_rot);
}
//The different commands are now registered in the bomb_truck class initer
static li_symbol_ref commands_ask("commands-ask"),
	commands_exec("commands-exec"),
	command_stop("command-stop");
li_object *g1_map_piece_class::message(li_symbol *message, li_object *message_param, li_environment *env)
	{
	if (message==commands_ask.get())
		return li_make_list(command_stop.get(),g1_object_class::message(message,message_param,env),0);
	if (message==commands_exec.get())
		{
		if (message_param==command_stop.get())
			{
			solveparams&=~SF_USE_PATHS;
		    unlink();
            //g1_map_solver_class *ps=
			prefered_solver();//called for its side-effects.
			set_path((g1_path_class *)NULL);
			return li_true_sym;
			}
		else
			{
			return g1_object_class::message(message,message_param,env);
			}
		}
	return g1_object_class::message(message,message_param,env);
	}
 

