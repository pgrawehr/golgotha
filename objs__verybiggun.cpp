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
 

#include "sound_man.h"
#include "objs/model_id.h"
#include "objs/model_draw.h"
#include "objs/rocktank.h"
#include "objs/verybiggun.h"
#include "input.h"
#include "math/pi.h"
#include "math/trig.h"
#include "math/angle.h"
#include "objs/bullet.h"
#include "resources.h"
#include "saver.h"
#include "map_cell.h"
#include "map.h"
//#include "cell_id.h"
#include "sound/sfx_id.h"
#include "object_definer.h"
#include "objs/fire.h"
#include "lisp/lisp.h"

static li_symbol_ref bullet("napalm");

void g1_very_big_gun_init()
{
  //bullet = g1_get_object_type("bullet");
}

g1_object_definer<g1_very_big_gun_class>
g1_very_big_gun_def("verybiggun", 
					g1_object_definition_class::EDITOR_SELECTABLE|
					g1_object_definition_class::TO_MAP_PIECE, 
					g1_very_big_gun_init);

g1_fire_range_type g1_very_big_gun_class::range()
{
  return G1_FIRE_RANGE_1;
}

g1_very_big_gun_class::g1_very_big_gun_class(g1_object_type id,
                                             g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  
  draw_params.setup("turret");
  defaults = g1_very_big_gun_def.defaults;
  
  w16 ver,data_size;
  if (fp)
  {  
    fp->get_version(ver,data_size);
    if (ver==DATA_VERSION)
    {
    }
    else
    {
      fp->seek(fp->tell() + data_size);
      fire_delay = 15;
      health     = 20;
    }

    fp->end_version(I4_LF);
  }
  else
  {
    fire_delay = 15;
    health     = 20;
  }

  health = 200;

  //no sound for now
  //init_rumble_sound(g1_moving_engineering_vehicle_wav);  
  
  //maxspeed   = 0; //doesnt move
  //turn_speed = defaults->turn_speed;

  radar_type=G1_RADAR_BUILDING;
  set_flag(BLOCKING      |
           SELECTABLE    |
           TARGETABLE    |
           GROUND        | 
           HIT_GROUND    |
           SHADOWED      |
		   DANGEROUS, 1);

}

void g1_very_big_gun_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  
  fp->start_version(DATA_VERSION);
    
  fp->end_version();
  
}

void g1_very_big_gun_class::load(g1_loader_class *fp)
	{
	g1_map_piece_class::load(fp);
	fp->check_version(DATA_VERSION);
	fp->end_version(I4_LF);
	};

void g1_very_big_gun_class::skipload(g1_loader_class *fp)
	{
	g1_map_piece_class::skipload(fp);
	fp->check_version(DATA_VERSION);
	fp->end_version(I4_LF);
	};


void g1_very_big_gun_class::fire()
{
  if (attack_target.valid())
  {    
    //i4_float tmp_x,tmp_y,tmp_z;
  
    fire_delay = defaults->fire_delay;

	i4_3d_point_class bpos, bdir;
    i4_transform_class btrans, tmp1, tmp2;
  
    tmp1.rotate_y(0);
    tmp1.t.set(0,0,0.3f);

    tmp2.rotate_z(theta);
    tmp2.t.set(x,y,h);

    btrans.multiply(tmp2,tmp1);

    btrans.transform(i4_3d_point_class(0.1f,0,0),bpos);

    bdir.set((float)cos(theta), (float)sin(theta), 0);
  

    /*tmp_x = 0.4;
    tmp_y = 0;    
    tmp_z = 0.2;

    i4_transform_class btrans,tmp1;  

    i4_angle ang = theta;
  
    btrans.translate(0,0,0);

    tmp1.rotate_z(ang);
    btrans.multiply(tmp1);

    tmp1.rotate_y(pitch);
    btrans.multiply(tmp1);

    tmp1.rotate_x(roll);
    btrans.multiply(tmp1);

    i4_3d_point_class tpoint;
 
    btrans.transform(i4_3d_point_class(tmp_x,tmp_y,tmp_z),tpoint);
    //i4_float bx,by,bz;

    //bx = tpoint.x + x;
    //by = tpoint.y + y;
    //bz = tpoint.z + h;    

    //i4_float b_dx,b_dy,b_dz;

    btrans.transform(i4_3d_point_class(g1_resources.bullet_speed,0,0),tpoint);
	*/
    //b_dx = tpoint.x;
    //b_dy = tpoint.y;
    //b_dz = tpoint.z;    

    //g1_bullet_class *b = (g1_bullet_class *)g1_create_object(bullet);
	g1_fire(bullet.get(),this,0,bpos,bdir,0);
    /*if (b)
    {
      b->setup(bx, by, bz, b_dx, b_dy, b_dz, ang,pitch,roll,player_num,
               i4_F,
               defaults->damage,
               defaults->fire_range
               );

      b->set_owner(this);    
    }*/
  }
}


void g1_very_big_gun_class::think()
{  
  check_life();
  

  if (health < 200)
    health++;

  find_target();
  pitch = groundpitch*cos(theta) - groundroll *sin(theta);
  roll  = groundroll *cos(theta) + groundpitch*sin(theta);

  if (fire_delay>0)
    fire_delay--;

  request_think();
  //aim the turet
  if (attack_target.valid()) 
  {
    

    i4_float dx,dy,angle;

    //this will obviously only be true if attack_target.ref != NULL    
    dx = (attack_target->x - x);
    dy = (attack_target->y - y);

    //aim the turet
    
    angle = i4_atan2(dy,dx);
    
    //snap it
    i4_normalize_angle(angle);    
    
    if (i4_rotate_to(theta,angle,defaults->turn_speed)<0.2f)
      if (!fire_delay)
        fire();
  }  
}
