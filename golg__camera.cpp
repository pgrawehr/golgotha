/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "camera.h"
#include "global_id.h"
#include "g1_render.h"
#include "math/angle.h"
#include "input.h"
#include "player.h"
#include "objs/stank.h"
#include "math/trig.h"
#include "resources.h"
#include "map_man.h"
#include "map.h"
#include "m_flow.h"
#include "cwin_man.h"
#include "lisp/lisp.h"
#include "controller.h"
#include "objs/field_camera.h"
#include "objs/map_piece.h"
#include "saver.h"
#include "time/profile.h"
#include "lisp/lisp.h"
#include "sound/sfx_id.h"
#include "demo.h"
#include "main/win_main.h"
#include "lisp/li_init.h"


i4_profile_class   pf_suggest_camera_event("suggest_camera_event");

int stereoport=0x03bc;//The default stereoport

enum {
  G1_PAN_LEFT           =(1<<0),
  G1_PAN_RIGHT          =(1<<1),
  G1_PAN_FORWARD        =(1<<2),
  G1_PAN_BACKWARD       =(1<<3),
  G1_PAN_UP             =(1<<4),
  G1_PAN_DOWN           =(1<<5),
  G1_ROTATE_LEFT        =(1<<6),
  G1_ROTATE_RIGHT       =(1<<7),
  G1_ROTATE_UP          =(1<<8),
  G1_ROTATE_DOWN        =(1<<9),
  
  G1_MOVE_LEFT          =(1<<10),
  G1_MOVE_RIGHT         =(1<<11),
  G1_MOVE_FORWARD       =(1<<12),
  G1_MOVE_BACKWARD      =(1<<13),
  G1_TILT_LEFT          =(1<<14),
  G1_TILT_RIGHT         =(1<<15)
};


char *cmd_names[]={"Pan Left",
                   "Pan Right",
                   "Pan Forward",
                   "Pan Backward",
                   "Pan Up",
                   "Pan Down",
                   "Rotate Left",
                   "Rotate Right",
                   "Rotate Up",
                   "Rotate Down",
                   "Move Left",
                   "Move Right",
                   "Move Forward",
                   "Move Backward",
				   "Tilt Left",
				   "Tilt Right",
                   0};



w32 g1_scroll_bits=0;


li_object *g1_camera_key_press(li_object *o, li_environment *env)
{
  w32 flag=li_get_int(li_eval(env->current_function()),env);
  g1_scroll_bits|=flag;  
  return 0;
}

li_object *g1_camera_key_depress(li_object *o, li_environment *env)
{
  w32 flag=li_get_int(li_eval(env->current_function()),env);
  g1_scroll_bits&=~flag;  
  return 0;
}

li_object *g1_toggle_follow_mode(li_object *_o, li_environment *env)
{
  g1_object_class *o=g1_player_man.get_local()->get_commander();
  if (g1_current_controller.get() && o)
    if (g1_current_controller->view.view_mode==G1_FOLLOW_MODE)
      g1_current_controller->view.suggest_camera_mode(G1_ACTION_MODE, o->global_id);
    else
      g1_current_controller->view.suggest_camera_mode(G1_FOLLOW_MODE, o->global_id);

  return 0;
}


class g1_add_camera_funs_initer : public i4_init_class
{
public:
  void init()
  {
    char **cmd;
    int f=1;
    for (cmd=cmd_names; *cmd; cmd++)
    {
      li_set_value(*cmd, new li_int(f));
      li_add_function(*cmd, g1_camera_key_press);

      char end_cmd[200];    
      sprintf(end_cmd, "-%s", *cmd);      
      li_add_function(end_cmd, g1_camera_key_depress);
      li_set_value(end_cmd, new li_int(f));
            
      f=f<<1;
    }
  }
} g1_add_camera_funs_instance;



li_object *g1_action_mode(li_object *o, li_environment *env)
{
  if (g1_current_view_state())
    g1_current_view_state()->suggest_camera_mode(G1_ACTION_MODE);
  return 0;
}

li_object *g1_strategy_mode(li_object *o, li_environment *env)
{
  if (g1_current_view_state())
    g1_current_view_state()->suggest_camera_mode(G1_STRATEGY_MODE);
  return 0;
}

li_object *g1_follow_mode(li_object *o, li_environment *env)
{
  g1_player_piece_class *com=g1_player_man.get_local()->get_commander();
  if (g1_current_view_state() && com)
    g1_current_view_state()->suggest_camera_mode(G1_FOLLOW_MODE, com->global_id);
  return 0;
}

li_object *g1_camera_mode(li_object *o, li_environment *env)
{
  if (g1_current_view_state())
	  {
	  g1_view_mode_type curr_mode=g1_current_view_state()->view_mode;
	  if (curr_mode==G1_WATCH_MODE)
		  curr_mode=G1_ACTION_MODE;
	  else 
		  curr_mode=G1_WATCH_MODE;
	  g1_current_view_state()->suggest_camera_mode(curr_mode);
	  }
  return 0;
}

li_object *g1_set_current_camera(li_object *o, li_environment *env)
{
  if (g1_current_view_state() && g1_current_controller.get())
  {
    g1_object_class *c=g1_find_named_camera(li_get_string(li_eval(li_car(o,env),env),env));
    if (c)
    {
      g1_camera_event evt;
      evt.camera_at=c;
      evt.type=G1_WATCH_FORCE;

      g1_current_controller->view.suggest_camera_event(evt);
    }
  }
  return 0;
}


li_object *g1_set_camera_position(li_object *o, li_environment *env)
{
  if (g1_current_view_state())
  {
    float x=(float)li_get_float(li_eval(li_first(o,env), env),env);
    float y=(float)li_get_float(li_eval(li_second(o,env), env),env);
    g1_current_view_state()->set_camera_position(x,y);
  }
  
  return 0;
}


void g1_camera_info_struct::defaults()
{
  gx=gy=1;
  gz=10;

  horizon_rotate=i4_pi_2();
  ground_rotate=i4_pi_2();
  roll=0;
  scale_x=scale_y=scale_z=1.0f;

}


void g1_camera_info_struct::load(g1_loader_class *fp)
{
  fp->read_format("ffffff", &gx, &gy, &gz, &ground_rotate, &horizon_rotate, &roll);  
  scale_x=scale_y=scale_z=1.0f;//should update the version of this data
}

void g1_camera_info_struct::save(g1_saver_class *fp)
{
  fp->write_format("ffffff", &gx, &gy, &gz, &ground_rotate, &horizon_rotate, &roll);  
}


g1_camera_event::g1_camera_event()
{
  follow_object=0;
  camera_at=0;
  type=G1_WATCH_INVALID;
  min_time=10, max_time=50;   // min/max times to sustain camera shot (in ticks)
  time_elapsed=0;           // time that we've spent on this shot already (in ticks)

  
};


void g1_view_state_class::save(g1_saver_class *fp)
{
  fp->start_version(1);

  start.save(fp);
  end.save(fp);
  
  fp->write_format("ff",
                   &start_end_interpolate_fraction,
                   &start_end_interpolate_fraction_step);
  fp->write_8(view_mode);
  
  fp->write_global_id(follow_object_id);
  
  fp->end_version();
}

void g1_view_state_class::defaults()
{
  move_offset=i4_3d_vector(0,0,0);
  stereomode=STEREO_NONE;
  if (i4_win32_startup_options.stereo)
	  {
	  stereomode=STEREO_LEFT;
	  stereoport=i4_win32_startup_options.stereoport;
	  }
  watch_type=G1_WATCH_IDLE;    
  follow_object_id=g1_global_id.invalid_id();
  view_mode=G1_STRATEGY_MODE;
  mode_changed=i4_T;
  start_end_interpolate_fraction_step=0.0;
  start_end_interpolate_fraction=1.0;
  start_invalid=i4_T;
  start.defaults();
  strategy_height_offset=0;
  end.defaults();
  current.defaults();
}


void g1_camera_event::object_ids_changed()   // call after level is reloaded
{
  camera_at=0;
  follow_object=0;
}

void g1_view_state_class::load(g1_loader_class *fp)
{
  if (fp && fp->check_version(1))
  {
    start.load(fp);
    end.load(fp);
    
    fp->read_format("ff",
                     &start_end_interpolate_fraction,
                     &start_end_interpolate_fraction_step);
    
    view_mode=(g1_view_mode_type)fp->read_8();
    follow_object_id=fp->read_global_id();
  }
  else
    follow_object_id=g1_global_id.invalid_id();

  current_cam.object_ids_changed();
  next_cam.object_ids_changed();
}




g1_view_state_class::g1_view_state_class()
{
  defaults();
}


// calculates current and returns a pointer to it
g1_camera_info_struct *g1_view_state_class::get_camera()
{
  float fr=g1_render.frame_ratio;
  if (fr>1.0) fr=1.0;
  if (fr<0.0) fr=0.0;
  
  float i=start_end_interpolate_fraction + start_end_interpolate_fraction_step * fr;
  
  current.gx = i4_interpolate(start.gx, end.gx, i);
  current.gy = i4_interpolate(start.gy, end.gy, i);
  current.gz = i4_interpolate(start.gz, end.gz, i);

  current.scale_x=i4_interpolate(start.scale_x,end.scale_x,i);
  current.scale_y=i4_interpolate(start.scale_y,end.scale_y,i);
  current.scale_z=i4_interpolate(start.scale_z,end.scale_z,i);
    
  current.ground_rotate = i4_interpolate_angle(start.ground_rotate, end.ground_rotate, i);
  current.horizon_rotate = i4_interpolate_angle(start.horizon_rotate, end.horizon_rotate, i);
  current.roll = i4_interpolate_angle(start.roll, end.roll, i);
  current.ground_x_rotate = i4_interpolate_angle(start.ground_x_rotate, end.ground_x_rotate, i);
  current.ground_y_rotate = i4_interpolate_angle(start.ground_y_rotate, end.ground_y_rotate, i);

  return &current;
}

void g1_view_state_class::update_follow_mode()
{
  start=end;
  
  g1_object_class *follow = g1_global_id.check_id(follow_object_id) ?
  g1_global_id.get(follow_object_id) : 0;
  if (follow)
  {
    g1_player_piece_class *stank=g1_player_man.get_local()->get_commander();
        
    i4_float fr = g1_render.frame_ratio;
 
    i4_float itheta = follow==stank ? stank->base_angle  : follow->theta;   
        
    i4_float idist   = g1_resources.follow_camera_dist;
    i4_float iheight = g1_resources.follow_camera_height;

    i4_float ix = follow->x;
    i4_float iy = follow->y;
    i4_float ih = follow->h+ 0.1f;
      
    end.ground_rotate = g1_resources.follow_camera_rotation + itheta;

    end.gx = ix - (float)cos(end.ground_rotate) * idist;
    end.gy = iy - (float)sin(end.ground_rotate) * idist;

    float th = ih + iheight;
    float gh = g1_get_map()->terrain_height(end.gx, end.gy)+0.01f;

    if (th < gh)
    {
      g1_resources.follow_camera_height = gh - ih;
      th = gh;
    }

    end.gz = th;
      
    float xy_dist = (float)sqrt((ix - end.gx) * (ix - end.gx) + 
                         (iy - end.gy) * (iy - end.gy));

    end.horizon_rotate = i4_atan2(end.gz - ih, xy_dist);
      
    end.roll=0;
    end.ground_x_rotate=0; 
    end.ground_y_rotate=0;

    start_end_interpolate_fraction=0.0;
    start_end_interpolate_fraction_step=1.0;
  }
  else
  {
    view_mode=G1_WATCH_MODE;
    update();
  }
	mode_changed=i4_F;
}

void g1_view_state_class::update_action_mode()
{
  g1_player_piece_class *stank=g1_player_man.get_local()->get_commander();
  if (!stank)
  {
    view_mode=G1_WATCH_MODE;
    update();
  }
  else
  {
    stank->calc_action_cam(start,0);
    stank->calc_action_cam(end,1);
    start_end_interpolate_fraction=0.0;
    start_end_interpolate_fraction_step=1.0;
  }      
  mode_changed=i4_F;
}

li_symbol_ref watch_camera_turn_speed("watch_camera_turn_speed"),
              watch_camera_max_speed("watch_camera_max_speed"),
              watch_camera_accel("watch_camera_accel"); 

void g1_check_position(float &x, float &y)
{
    if (x<0) x=0;
    if (y<0) y=0;
    if (x+1>=g1_get_map()->width()) 
        x = (float)(g1_get_map()->width()-1);  // JJ Cast
    if (x+1>=g1_get_map()->height()) 
        x= (float)(g1_get_map()->height()-1);  // JJ Cast
}

void g1_view_state_class::use_next_cam()
{
  if (next_cam.camera_at.get()==current_cam.camera_at.get())   // don't snap if same camera position
    next_cam.time_elapsed=1;
  
  current_cam=next_cam;
  next_cam.type=G1_WATCH_INVALID;
  if (current_cam.type!=G1_WATCH_INVALID)
  {
    g1_object_class *watch_obj=current_cam.follow_object.get();
    if (watch_obj)
    {
      i4_3d_vector pos=i4_3d_vector(watch_obj->x, watch_obj->y, watch_obj->h);        
      g1_object_class *cam_object=g1_find_closest_field_camera(pos);
      if (cam_object)
       current_cam.camera_at=cam_object;
    }
    else if (next_cam.camera_at.get())
    {
      i4_transform_class t;

      g1_object_class *cam=next_cam.camera_at.get();
      cam->world_transform=&t;
      cam->calc_world_transform(0, &t);

      i4_3d_vector startp,endp;
      t.transform(i4_3d_vector(0,0,0), startp);
      t.transform(i4_3d_vector(1,0,0), endp);
      

      float xy_theta=i4_atan2(endp.y - startp.y, endp.x - startp.x);
      float xy_dist=(float)sqrt((endp.y - startp.y) * (endp.y - startp.y) +
                         (endp.x - startp.x) * (endp.x - startp.x));

      float z_theta=i4_atan2(endp.z - startp.z, xy_dist);
    
      end.gx=startp.x;
      end.gy=startp.y;
      end.gz=startp.z;

      end.ground_x_rotate=0; 
      end.ground_y_rotate=0;

      end.ground_rotate=xy_theta;
      end.horizon_rotate=-z_theta;
    
      end.roll = 0;

      start=end;
    }
  }  
}

void g1_view_state_class::update_watch_mode()
{
  start=end;

  i4_bool next=i4_F;
  
  
  // see if the next camera has higher priority
  if ((next_cam.type>current_cam.type && current_cam.time_elapsed>current_cam.min_time) ||
      (next_cam.type!=G1_WATCH_INVALID && current_cam.time_elapsed>current_cam.max_time))
    next=i4_T;
  
  
  if (!current_cam.follow_object.get()) // object following is gone    
    next=i4_T;

  if (current_cam.type==G1_WATCH_INVALID)
    next=i4_T;
  
  if (next)
    use_next_cam();
           
  if (current_cam.type==G1_WATCH_INVALID)
    return ;

  
  g1_object_class *obj_to_watch=current_cam.follow_object.get();
  g1_object_class *camera_at=current_cam.camera_at.get();

  if (camera_at)
  {
    end.gx=camera_at->x;
    end.gy=camera_at->y;
    end.gz=camera_at->h;    
  }

  
  if (obj_to_watch)
  {
    i4_3d_vector watch=i4_3d_vector(obj_to_watch->x, obj_to_watch->y, obj_to_watch->h);
    
    float xy_theta=i4_atan2(watch.y - end.gy, watch.x - end.gx);
    float xy_dist=(float)sqrt((watch.y - end.gy) * (watch.y - end.gy) +
                       (watch.x - end.gx) * (watch.x - end.gx));

    float z_theta=i4_atan2(watch.z - end.gz, xy_dist);
    

    float turn_speed=(float)li_get_float(li_get_value(watch_camera_turn_speed.get()),0);

    if (current_cam.time_elapsed==0)  // ifrst tick it's ok to snap
    {
      end.ground_rotate=xy_theta;
      end.horizon_rotate=-z_theta;
    }
    else
    {
      i4_rotate_to(end.ground_rotate, xy_theta, turn_speed);                        
      i4_rotate_to(end.horizon_rotate, -z_theta, turn_speed);
    }
    
    end.roll = 0;
    
  }

  current_cam.time_elapsed++;

  
  start_end_interpolate_fraction=0.0;
  start_end_interpolate_fraction_step=1.0;
  mode_changed=i4_F;
}

void g1_view_state_class::update_circle_mode()
{
  start=end;

  i4_3d_vector watch=circle.looking_at;

  // camera vel
  // x = angle of ration about object being watched
  // y = distance in from object in xy plane
  // z = height above object


  if (end.gz-watch.z<3)
  {
    end.gz+=circle.zvel;
    circle.zvel+=0.0005f;
    if (circle.zvel>0.1f)
      circle.zvel=0.1f;
  }


  circle.dist+=circle.dist_vel;
  circle.dist_vel-=0.01f;
  if (circle.dist_vel<0.0f)
    circle.dist_vel=0.0f;

  
  circle.theta+=circle.theta_vel;


  end.gx=watch.x + (float)cos(circle.theta)*circle.dist;
  end.gy=watch.y + (float)sin(circle.theta)*circle.dist;  

    
  float xy_theta=i4_atan2(watch.y - end.gy, watch.x - end.gx);
  float xy_dist=(float)sqrt((watch.y - end.gy) * (watch.y - end.gy) +
                     (watch.x - end.gx) * (watch.x - end.gx));

  float z_theta=i4_atan2(watch.z - end.gz, xy_dist);   

  end.ground_rotate=xy_theta;
  end.horizon_rotate=-z_theta;    
  end.roll = 0;
  end.ground_x_rotate=end.ground_y_rotate=0;

  start_end_interpolate_fraction=0.0;
  start_end_interpolate_fraction_step=1.0;
  mode_changed=i4_F;
}

static float g1_pan_speed=0, g1_rotate_speed=0, g1_roll_speed=0;
static float g1_span_speed=0, g1_srotate_speed=0;
static li_symbol_ref editor_pan_accel("editor_pan_accel");
static li_symbol_ref editor_rotate_accel("editor_rotate_accel");
static li_symbol_ref strategy_pan_accel("strategy_pan_accel");
static li_symbol_ref strategy_rotate_accel("strategy_rotate_accel");

void g1_view_state_class::update_strategy_mode()
{
  i4_angle a=end.ground_rotate;
  i4_angle move_speed=(i4_float)g1_srotate_speed+li_get_float(li_get_value(strategy_pan_accel.get()),0);
  
  start=end;

  //accelerate slowly
  if (g1_scroll_bits)
	  {
	  move_speed=move_speed>0.5f?0.5f:move_speed;
	  }
  else
	  move_speed=0;
  g1_srotate_speed=move_speed;
  if (mode_changed)
	  {
	  end.ground_x_rotate=end.ground_y_rotate=0;
      end.ground_rotate=i4_pi_2();
      end.horizon_rotate=i4_pi_2() - i4_pi_8();
      end.roll=0;
	  }
  mode_changed=i4_F;
      
  if (g1_scroll_bits & (G1_ROTATE_DOWN | G1_ROTATE_UP | G1_ROTATE_RIGHT | G1_ROTATE_LEFT))
    g1_rotate_speed+=(float)li_get_float(li_get_value(editor_rotate_accel.get()),0);
  else
    g1_rotate_speed=0;

  if (g1_scroll_bits & (G1_TILT_LEFT | G1_TILT_RIGHT))
    g1_roll_speed+=(float)li_get_float(li_get_value(editor_rotate_accel.get()),0);
  else
    g1_roll_speed=0;

  if (g1_scroll_bits & G1_TILT_LEFT)
	  g1_current_controller->roll(g1_roll_speed);
  if (g1_scroll_bits & G1_TILT_RIGHT)
	  g1_current_controller->roll(-g1_roll_speed);
  
  if (g1_scroll_bits & G1_PAN_DOWN)
    strategy_height_offset+=move_speed;
  
  if (g1_scroll_bits & G1_PAN_UP)
    strategy_height_offset-=move_speed;

  if (g1_scroll_bits & G1_ROTATE_LEFT)
    g1_current_controller->rotate(g1_rotate_speed, 0);
  
  if (g1_scroll_bits & G1_ROTATE_RIGHT)
    g1_current_controller->rotate(-g1_rotate_speed, 0);

  if (g1_scroll_bits & G1_ROTATE_UP)
    g1_current_controller->rotate(0, -g1_rotate_speed);
  
  if (g1_scroll_bits & G1_ROTATE_DOWN)
    g1_current_controller->rotate(0, g1_rotate_speed);

  if (g1_scroll_bits & G1_MOVE_LEFT)
    g1_current_controller->pan(-(float)sin(a)*move_speed, (float)cos(a)*move_speed, 0);

  if (g1_scroll_bits & G1_MOVE_RIGHT)
    g1_current_controller->pan((float)sin(a)*move_speed, -(float)cos(a)*move_speed, 0);

  if (g1_scroll_bits & G1_MOVE_FORWARD)
    g1_current_controller->pan((float)cos(a)*move_speed, (float)sin(a)*move_speed, 0);

  if (g1_scroll_bits & G1_MOVE_BACKWARD)
    g1_current_controller->pan(-(float)cos(a)*move_speed, -(float)sin(a)*move_speed, 0);

  end.gx+=move_offset.x;
  end.gy+=move_offset.y;
  move_offset=i4_3d_vector(0,0,0);
  
  if (end.gx<2.0) end.gx = 2.0;
  else if (end.gx>g1_map_width-2.0f) end.gx = g1_map_width-2.0f;
  if (end.gy<2.0) end.gy = 2.0;
  else if (end.gy>g1_map_height-2.0f) end.gy = g1_map_height-2.0f;
  
  if (strategy_height_offset>80)
	  strategy_height_offset=80;
  i4_float th=g1_get_map()->terrain_height(end.gx, end.gy);
  if (g1_resources.startegy_camera_dist+strategy_height_offset<0.01f)
	  {
	  strategy_height_offset= -(g1_resources.startegy_camera_dist-0.01f);
	  }
  end.gz=th + g1_resources.startegy_camera_dist+strategy_height_offset;

  start_end_interpolate_fraction=0.0;
  start_end_interpolate_fraction_step=1.0;
}    




void g1_view_state_class::update_edit_mode()
{
  start_end_interpolate_fraction=1.0;
  start_end_interpolate_fraction_step=0;
  
  i4_angle a=end.ground_rotate;  

  end.gx+=move_offset.x;
  end.gy+=move_offset.y;
  move_offset=i4_3d_vector(0,0,0);

      
  if (g1_scroll_bits & (G1_ROTATE_DOWN | G1_ROTATE_UP | G1_ROTATE_RIGHT | G1_ROTATE_LEFT))
    g1_rotate_speed+=(float)li_get_float(li_get_value(editor_rotate_accel.get()),0);
  else
    g1_rotate_speed=0;

  if (g1_scroll_bits & (G1_TILT_LEFT | G1_TILT_RIGHT))
    g1_roll_speed+=(float)li_get_float(li_get_value(editor_rotate_accel.get()),0);
  else
    g1_roll_speed=0;
  
  if (g1_scroll_bits & (G1_PAN_DOWN | G1_PAN_UP | G1_PAN_RIGHT | G1_PAN_LEFT |
                         G1_PAN_FORWARD | G1_PAN_BACKWARD))
  {
    g1_pan_speed+=(float)li_get_float(li_get_value(editor_pan_accel.get()),0);
    if (g1_pan_speed>4)
      g1_pan_speed=4;
  }
  else
    g1_pan_speed=0;

  if (g1_scroll_bits & G1_TILT_LEFT)
	  g1_current_controller->roll(g1_roll_speed);
  if (g1_scroll_bits & G1_TILT_RIGHT)
	  g1_current_controller->roll(-g1_roll_speed);

  if (g1_scroll_bits & G1_ROTATE_LEFT)
    g1_current_controller->rotate(g1_rotate_speed, 0);
  
  if (g1_scroll_bits & G1_ROTATE_RIGHT)
    g1_current_controller->rotate(-g1_rotate_speed, 0);

  if (g1_scroll_bits & G1_ROTATE_UP)
    g1_current_controller->rotate(0, -g1_rotate_speed);
  
  if (g1_scroll_bits & G1_ROTATE_DOWN)
    g1_current_controller->rotate(0, g1_rotate_speed);
  
  
  if (g1_scroll_bits & G1_PAN_DOWN)
    g1_current_controller->pan(0,0,-g1_pan_speed);
  
  if (g1_scroll_bits & G1_PAN_UP)
    g1_current_controller->pan(0,0,g1_pan_speed);

  if (g1_scroll_bits & G1_PAN_LEFT)
    g1_current_controller->pan(-(float)sin(a)*g1_pan_speed, (float)cos(a)*g1_pan_speed,0);

  if (g1_scroll_bits & G1_PAN_RIGHT)
    g1_current_controller->pan((float)sin(a)*g1_pan_speed, -(float)cos(a)*g1_pan_speed,0);

  if (g1_scroll_bits & G1_PAN_FORWARD)
    g1_current_controller->pan((float)cos(a)*g1_pan_speed, (float)sin(a)*g1_pan_speed,0);

  if (g1_scroll_bits & G1_PAN_BACKWARD)
    g1_current_controller->pan(-(float)cos(a)*g1_pan_speed, -(float)sin(a)*g1_pan_speed,0);

  mode_changed=i4_F;
}

void g1_view_state_class::update_camera_mode()
{
  g1_movie_flow_class *m=g1_get_map()->current_movie;
  g1_cut_scene_class *cut=m ? m->current() : 0;
  if (cut)
  {
    i4_float cx, cy, cz, tx, ty, tz, xy_theta, z_theta, xy_dist;

    cut->paths[g1_cut_scene_class::CAMERA]->get_point(m->get_frame(), cx,cy,cz);
    cut->paths[g1_cut_scene_class::TARGET]->get_point(m->get_frame(), tx,ty,tz);

    xy_theta=i4_atan2(ty-cy, tx-cx);
    xy_dist=(float)sqrt((ty-cy)*(ty-cy) + (tx-cx)*(tx-cx));
      
    z_theta=i4_atan2(tz-cz, xy_dist);

    end.gx=cx; end.gy=cy; end.gz=cz;

    end.ground_rotate=xy_theta;
    end.horizon_rotate=-z_theta;
    end.roll = 0;
  }
  else 
  {
    view_mode=G1_WATCH_MODE;
    update();
    return;
  }
  mode_changed=i4_F;
}

// should be called after every tick, updates end camera pos
void g1_view_state_class::update()
{
  if (&g1_current_controller->view==this)
    g1_change_key_context(view_mode);
      
  if (start_end_interpolate_fraction>=1.0)
  {
    start=end;
    start_end_interpolate_fraction_step=1.0;
    start_end_interpolate_fraction=0;
  }
  else
    start_end_interpolate_fraction+=start_end_interpolate_fraction_step;
 
  
  switch (view_mode)
  {
    case G1_EDIT_MODE :
      update_edit_mode();
      break;
      
    case G1_STRATEGY_MODE :
      update_strategy_mode();
      break;
      
    case G1_ACTION_MODE :
      update_action_mode();
      break;


    case G1_FOLLOW_MODE :
      update_follow_mode();      
      break;
    
    case G1_CAMERA_MODE :
      update_camera_mode();
      break;

    case G1_WATCH_MODE :
      update_watch_mode();
      break;

    case G1_CIRCLE_WAIT :
      update_circle_mode();
      break;
  }
  
}

/*!  
calc_tranform(). Very important code bellow!
this calculates the actual world-to-screen tranform for this window.
transformations at the top will be applied to the camera-coordinate system
transformations at the bottom of the function will be in world-coordinates
Editing anything in-between is not recommended (except you know _exactly_ 
what you are doing.
This code is called once per frame and window, so speed is not so much
an issue here.
\param transform an i4_transform_class reference that will contain the world to screen transform
*/

void g1_view_state_class::calc_transform(i4_transform_class &transform)
{
  g1_camera_info_struct *c=get_camera();

    // now calculate our actual transform
  i4_transform_class tmp;  
  transform.identity();//load the identity
  i4_float eyediffx=0;
  if (stereomode==STEREO_LEFT)
	  {
	  if (view_mode==G1_ACTION_MODE)//in first person view, 
		  {//objects get nearer, so the effect is heavy, anyway
		  eyediffx=-0.015f;
		  }
	  else
		  eyediffx=-0.03f;
	  tmp.translate(eyediffx,0,0);
	  transform.multiply(tmp);
	  }
  else if (stereomode==STEREO_RIGHT)
	  {
	  if (view_mode==G1_ACTION_MODE)
		  eyediffx=+0.015f;
	  else
		  eyediffx=+0.03f;
	  tmp.translate(eyediffx,0,0);
	  transform.multiply(tmp);
	  }

  
  
  
  //tmp.scale(c->scale_x,c->scale_y,c->scale_z);
  //transform.multiply(tmp);//what about doing this in world-coordinates?
  //doesn't work since the reference to the bounding box doesn't match.
  //Ok, we don't need to do anything with the transformation matrices,
  //only with the viewport. 
  //I'll later check what these variables really do.
  //This is even much cheaper than doing a matrix multiplication.
  //the variables are used later on in calculating the definitive 
  //world-to-view transformation of every object and vertex.
  //(We're only calculating the camera-to-world transform here)
  g1_render.scale_x*=c->scale_x;
  g1_render.scale_y*=c->scale_y;
  g1_render.ooscale_x*=(1/c->scale_x);
  g1_render.ooscale_y*=(1/c->scale_y);
  
  tmp.rotate_y(i4_pi());
  transform.multiply(tmp);//do some coordinate realignments

  tmp.rotate_x(i4_pi_2());
  transform.multiply(tmp); 
     
  tmp.rotate_z(-i4_pi_2());
  transform.multiply(tmp);                                           
  
  tmp.rotate_x(-c->roll); //adjust for the camera position
  transform.multiply(tmp);
  
  tmp.rotate_y(-c->horizon_rotate);
  transform.multiply(tmp);

  tmp.rotate_z(-c->ground_rotate);
  transform.multiply(tmp);  
 
  //action mode is the only mode that specifies these
  tmp.rotate_y(-c->ground_y_rotate); 
  transform.multiply(tmp);
  
  tmp.rotate_x(-c->ground_x_rotate);
  transform.multiply(tmp);

//  tmp.scale(c->scale_x,c->scale_y,c->scale_z);
//  transform.multiply(tmp);
  
  

  tmp.translate(-c->gx, 
	  -c->gy,
	  -c->gz);//do the final translation
  transform.multiply(tmp);//this is actually done first.
  
  //tmp.scale(c->scale_x,c->scale_y,c->scale_z);
  //transform.multiply(tmp);

  s1_set_camera_pos(transform);  
  //tmp.scale(c->scale_x,c->scale_y,c->scale_z);
  //transform.multiply(tmp);
}



void g1_view_state_class::suggest_camera_mode(g1_view_mode_type mode,
                                              w32 follow_object_global_id)
{ 
  if (mode==G1_CIRCLE_WAIT)
  {
    g1_object_class *o=g1_global_id.checked_get(follow_object_global_id);
    if (o)
    {
      circle.zvel=circle.theta=0;
      circle.theta_vel=0.05f;
      circle.dist=1;
      circle.dist_vel=0.2f;
      circle.looking_at=i4_3d_vector(o->x, o->y, o->h);
      end.gz=o->h+0.1f;
    }
    else return ;
  }

  if (view_mode!=mode)
	  mode_changed=i4_T;
  view_mode=mode;
  follow_object_id=follow_object_global_id;


  switch (mode)
  {
    case G1_STRATEGY_MODE :
		//init to sensefull values before switching mode
      g1_demo_script_add("(strategy_mode)");
	  
      break;

    case G1_ACTION_MODE :
      g1_demo_script_add("(action_mode)");
      break;

    case G1_FOLLOW_MODE :
      g1_demo_script_add("(follow_mode)");
      break;
  }

}


void g1_view_state_class::set_camera_position(float game_x, float game_y)
{
  move_offset.x=game_x-end.gx;
  move_offset.y=game_y-end.gy;   

  char cmd[50];
  sprintf(cmd, "(set_camera_pos %0.2f %0.2f)", game_x, game_y);
  g1_demo_script_add(cmd);
}

void g1_view_state_class::suggest_camera_event(g1_camera_event &event)
{
  pf_suggest_camera_event.start();

  if (event.type==G1_WATCH_FORCE)
  {
    next_cam=event;
    suggest_camera_mode(G1_WATCH_MODE, event.follow_object.get() ?
                        event.follow_object->global_id : 0 );
  }
  
//#if 0
  if (event.type>next_cam.type)
  {
    next_cam=event;
    if (event.type==G1_WATCH_FORCE)
      suggest_camera_mode(G1_WATCH_MODE, event.follow_object.get() ?
                          event.follow_object->global_id : 0 );
  }

  // see if next is same type & at the caera location
  else
  {
    i4_bool current_dead=i4_F;
    // object we are watching is dead
    g1_map_piece_class *mp;
    mp=g1_map_piece_class::cast(current_cam.follow_object.get());
    if (mp && mp->health<=0)
      current_dead=i4_T;
   
    if ((current_dead || event.type==next_cam.type) &&
        next_cam.camera_at.get()!=current_cam.camera_at.get())
    {
      g1_object_class *watch_obj=next_cam.follow_object.get();
      if (watch_obj)
      {
        i4_3d_vector pos=i4_3d_vector(watch_obj->x, watch_obj->y, watch_obj->h);        
        g1_object_class *cam_object=g1_find_closest_field_camera(pos);      
        if (cam_object && cam_object==current_cam.camera_at.get())
          current_cam.camera_at=cam_object;
      }
    }
  }
//#endif
  
  pf_suggest_camera_event.stop();
}


void g1_view_state_class::get_camera_pos(i4_3d_vector &v)
{
  v.x=current.gx;
  v.y=current.gy;
  v.z=current.gz;
}

float g1_view_state_class::dist_sqrd(const i4_3d_vector &v)
{
  float d=(end.gx-v.x)*(end.gx-v.x) +
          (end.gy-v.y)*(end.gy-v.y) +
          (end.gz-v.z)*(end.gz-v.z);
  return d;
}


void g1_view_state_class::zoom(float dist)
{
  float xy_contribute=(float)fabs(cos(end.horizon_rotate));

  i4_3d_vector move_vector((float)cos(end.ground_rotate) * xy_contribute,
                           (float)sin(end.ground_rotate) * xy_contribute,
                           -(float)sin(end.horizon_rotate));
  
  end.gx+=move_vector.x * dist;
  end.gy+=move_vector.y * dist;
  end.gz+=move_vector.z * dist;
}

void g1_view_state_class::pan(i4_float x, i4_float y, i4_float z)
{
  end.gx+=x;
  end.gy+=y;
  end.gz+=z;
}

void g1_view_state_class::rotate(i4_float about_game_z, i4_float up_down)
{
  end.horizon_rotate+=up_down;
  end.ground_rotate+=about_game_z;
}

//sets absolute, not relative values!
void g1_view_state_class::scale(float x, float y, float z)
	{
	if (x<=0 || y<=0 || z<=0)
		{//neither of these must be 0. Use scale(0,0,0) to restore default
		end.scale_x=1;
		end.scale_y=1;
		end.scale_z=1;
		}
	else
		{
		end.scale_x=x;
		end.scale_y=y;
		end.scale_z=z;
		}
	}

void g1_view_state_class::roll(i4_float r)
	{
	end.roll+=r;
	}

g1_view_state_class *g1_current_view_state()
{
  if (!g1_current_controller.get())
    return 0;
  else
    return &g1_current_controller->view;
}

li_automatic_add_function(g1_action_mode, "action_mode");  
li_automatic_add_function(g1_strategy_mode, "strategy_mode");  
li_automatic_add_function(g1_follow_mode, "follow_mode");  
li_automatic_add_function(g1_set_camera_position, "set_camera_pos");  

li_automatic_add_function(g1_camera_mode, "camera_mode");
li_automatic_add_function(g1_set_current_camera, "set_current_camera");  
li_automatic_add_function(g1_toggle_follow_mode, "toggle_follow_mode");
