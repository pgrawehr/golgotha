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
 

#include "objs/def_object.h"
#include "lisp/li_init.h"
#include "lisp/li_class.h"
#include "li_objref.h"
#include "map_man.h"
#include "map.h"
#include "math/pi.h"
#include "objs/model_draw.h"
#include "math/angle.h"
#include "sound_man.h"
#include "object_definer.h"
#include "resources.h"
#include "image_man.h"
#include "sound/sfx_id.h"
#include "objs/map_piece.h"
#include "g1_rand.h"

#pragma warning(disable: 4305)

S1_SFX(bird_sfx, "ambient/owl1_22khz.wav", S1_3D, 5);
S1_SFX(bird_eagle_sfx, "ambient/eagle1_22khz.wav", S1_3D, 5);
static li_symbol_class_member flapping("flapping");

static li_int_class_member ticks_to_think("ticks_to_think");

static li_float_class_member 
  //xy_v("xy_v"), zv("zv"), 
  flap_theta("flap_theta"),
  dest_x_m("dest_x"),dest_y_m("dest_y");

static g1_model_ref model_ref("bird_body-original"),
  shadow_ref("bird_shadow"),
  wingl_ref("bird_wingl-original"),
  wingr_ref("bird_wingr-original"),
  model_2_ref("bird_body"),
  wingl_2_ref("bird_wingl"),
  wingr_2_ref("bird_wingr");
// current destination of the bird is set by def_object automatically if the
// object has these variables when deploy_to is called
//static li_float_class_member dest_x("dest_x"), dest_y("dest_y");     

static i4_3d_vector wingl_attach, wingr_attach,
wingl_offset,wingr_offset;
enum {DATA_VERSION=1};
enum bird_mode
	{TAKE_OFF=0,
	FLYING,
	LANDING,
	EATING,
	WAITING,
	TURNING
	};
static li_symbol *sym_yes=0, *sym_no=0;    // cache the pointer to the "yes" symbol

void g1_bird_init()
	{
	wingl_attach.set(0,0.002f,0);
	wingl_offset.set(0,0,0);
	wingr_attach.set(0,-0.002f,0);
	wingr_offset.set(0,0,0);
	}

class g1_bird_class : public g1_map_piece_class
{
protected:
  w8 mode;                                        //internal modes
  i4_float dest_x,dest_y,xy_v,zv;
  i4_bool first_time;
  g1_mini_object      *wingl,*wingr;// = &mini_objects[0];
  s1_sfx_ref *my_sfx;
public:
  g1_bird_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
  
  //virtual void fire();
  virtual void think();

  virtual i4_bool occupy_location();
  virtual void unoccupy_location();
        
  //void damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir);
};


class g1_bird_eagle_class: public g1_bird_class
	{
	public:
		g1_bird_eagle_class(g1_object_type id, g1_loader_class *fp);
	};

g1_object_definer<g1_bird_class>
g1_bird_def("bird",
			g1_object_definition_class::EDITOR_SELECTABLE|
			g1_object_definition_class::TO_MAP_PIECE| 
			g1_object_definition_class::MOVABLE,
			g1_bird_init);

g1_object_definer<g1_bird_eagle_class>
g1_bird_eagle_def("bird_eagle",
			g1_object_definition_class::EDITOR_SELECTABLE|
			g1_object_definition_class::TO_MAP_PIECE| 
			g1_object_definition_class::MOVABLE,
			g1_bird_init);

g1_bird_eagle_class::g1_bird_eagle_class(g1_object_type id,g1_loader_class *fp)
:g1_bird_class(id,fp)
	{
	draw_params.setup("bird_body","bird_shadow");
	wingl=&mini_objects[0];
	wingl->defmodeltype=wingl_2_ref.id();
	wingl->position(wingl_attach);
	wingl->offset=wingl_offset;
	wingr=&mini_objects[1];
	wingr->defmodeltype=wingr_2_ref.id();
	wingr->position(wingr_attach);
	wingr->offset=wingr_offset;
	my_sfx=&bird_eagle_sfx;
	}

g1_bird_class::g1_bird_class(g1_object_type id, g1_loader_class *fp)
:g1_map_piece_class(id,fp)
	{
	radar_type=G1_RADAR_NONE;//Cannot see a bird on the radar
	set_flag(
		BLOCKING|
		AERIAL,1);
	my_sfx=&bird_sfx;
	defaults=g1_bird_def.defaults;
	draw_params.setup("bird_body-original","bird_shadow");
	allocate_mini_objects(2,"Bird wings");
	wingl=&mini_objects[0];
	wingl->defmodeltype=wingl_ref.id();
	wingl->position(wingl_attach);
	wingl->offset=wingl_offset;
	wingr=&mini_objects[1];
	wingr->defmodeltype=wingr_ref.id();
	wingr->position(wingr_attach);
	wingr->offset=wingr_offset;
	w16 ver,data_size;
	if (fp)
		fp->get_version(ver,data_size);
    else
		ver =0;
	mode=1;
    switch (ver)
		{
        case DATA_VERSION:
		fp->read_format("1ffff",
			&mode,
			&dest_x,
			&dest_y,
			&xy_v,
			&zv);
		first_time=i4_F;
		break;
		default:
			dest_x=x;
			dest_y=y;
			xy_v=defaults->speed;
			first_time=i4_T;
			zv=0;			
		break;
		}
	if (fp)
		fp->end_version(I4_LF);
	
	}

void g1_bird_class::save(g1_saver_class *fp)
	{
	g1_map_piece_class::save(fp);
	fp->start_version(DATA_VERSION);
	fp->write_format("1ffff",
		&mode,
		&dest_x,
		&dest_y,
		&xy_v,
		&zv);
	fp->end_version();
	}

i4_bool g1_bird_class::occupy_location()
	{
	if (first_time)
		{
		h+=1.0f;
		lh=h;
		}
	first_time=i4_F;
	
	return g1_map_piece_class::occupy_location();
	}

void g1_bird_class::unoccupy_location()
	{
	g1_map_piece_class::unoccupy_location();
	}


void g1_bird_class::think()
{
  
  grab_old();
  unoccupy_location();
  


  x += cos(theta) * xy_v;
  y += sin(theta) * xy_v;

  if ((abs(x-dest_x)>=0.4)&&(abs(y-dest_y)>=0.4))
  {
    float desired_theta=atan2(dest_y-y, dest_x-x);
    //theta = i4_interpolate_angle(theta, desired_theta, 0.1);
	i4_rotate_to(theta,desired_theta,defaults->turn_speed);
    if (ticks_to_think()<200)
		ticks_to_think()++;
	else
		my_sfx->play(x,y,h);
  }
  else
	  {
      theta+=defaults->turn_speed;
	  ticks_to_think()--;
	  if (ticks_to_think()<=0)
		  {
		  my_sfx->play(x,y,h);
		  if (dest_x!=dest_x_m()&&dest_y!=dest_y_m())
			  {
			  dest_x=(float)(g1_rand(10)%(g1_get_map()->width()-10))+5;
	          dest_y=(float)(g1_rand(11)%(g1_get_map()->height()-10))+5;
			  }
		  else
			  {
			  dest_x=dest_x_m();
			  dest_y=dest_y_m();
			  }
		  }
	
	  }

  xy_v *= 0.99;     // air friction
  
  if (flapping()==li_get_symbol("yes", sym_yes))
  {
    zv += 0.001;
    if (zv>0.03)
      zv=0.03;
    xy_v += 0.001;
  }
  else
  {
    zv += -0.001;
    if (zv<-0.05)
      zv=-0.05;

    xy_v += 0.001;
    if (xy_v>0.2)
      xy_v=0.2;
  }

  float z_vel=zv;
  pitch = z_vel * (i4_pi()/8) / 0.2;
  h += zv;

  float agh=height_above_ground();

  if (agh<1.5)
    flapping()=li_get_symbol("yes", sym_yes);
  if (agh>3.0)
    flapping() = li_get_symbol("no", sym_no);
  if (agh<0.7)
	  {
	  zv=0.03;
	  xy_v-=0.005;
	  dest_x=(float)(g1_rand(10)%g1_get_map()->width());
	  dest_y=(float)(g1_rand(11)%g1_get_map()->height());
			
	  }
  if (agh<0)
	  {
	  request_remove();
	  return;//must not request_think() if just request_remove()'d
	  }


  if (flapping()==li_get_symbol("yes", sym_yes))
  {
    flap_theta()+=1.0;
    
    wingl->rotation.x=cos(flap_theta()) * i4_pi()/8 + i4_pi()/16;
    wingr->rotation.x=cos(flap_theta()) * -i4_pi()/8 + i4_pi()/16;
  }
  else
  {
    wingl->rotation.x *= 0.5;
    wingr->rotation.x *= 0.5;
  }
    

  if (occupy_location())
  {
    //if (ticks_to_think())
    //{
    //  ticks_to_think()--;
      request_think();
    //}
  }
};

/*
li_object *g1_draw_and_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o,env),env);


  g1_model_draw_parameters mp;
  mp.model=me->get_type()->model;
  g1_model_draw(me, mp, g1_draw_context);

  if (me->get_flag(g1_object_class::MAP_OCCUPIED))
  {
    ticks_to_think()=20;
    me->request_think();
  }

  return 0;
}
*/
/*
li_object *g1_bird_message(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o,env),env);
  return 0;
}
*/
/*
li_object *g1_bird_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o,env),env);
  
  me->grab_old();
  me->unoccupy_location();
  


  me->x += cos(me->theta) * xy_v();
  me->y += sin(me->theta) * xy_v();

  if (dest_x()>=0.0)
  {
    float desired_theta=atan2(me->y-dest_y(), me->x-dest_x());
    me->theta = i4_interpolate_angle(me->theta, desired_theta, 0.1);
  }
  else
    me->theta+=turn_speed();

  xy_v() *= 0.99;     // air friction
  
  if (flapping()==li_get_symbol("yes", sym_yes))
  {
    zv() += 0.001;
    if (zv()>0.03)
      zv()=0.03;
    xy_v() += 0.001;
  }
  else
  {
    zv() += -0.001;
    if (zv()<-0.05)
      zv()=-0.05;

    xy_v() += 0.001;
    if (xy_v()>0.2)
      xy_v()=0.2;
  }

  float z_vel=zv();
  me->pitch = z_vel * (i4_pi()/8) / 0.2;
  me->h += zv();

  float agh=me->height_above_ground();

  if (agh<1.5)
    flapping()=li_get_symbol("yes", sym_yes);
  else if (agh>2.0)
    flapping() = li_get_symbol("no", sym_no);


  if (flapping()==li_get_symbol("yes", sym_yes))
  {
    flap_theta()+=1.0;
    
    me->mini_objects[0].rotation.x=cos(flap_theta()) * i4_pi()/8 + i4_pi()/16;
    me->mini_objects[1].rotation.x=cos(flap_theta()) * -i4_pi()/8 + i4_pi()/16;
  }
  else
  {
    me->mini_objects[0].rotation.x *= 0.5;
    me->mini_objects[1].rotation.x *= 0.5;
  }
    

  if (me->occupy_location())
  {
    if (ticks_to_think())
    {
      ticks_to_think()--;
      me->request_think();
    }
  }

  return 0;
}
*/
//li_automatic_add_function(g1_bird_message, "bird_message");
//li_automatic_add_function(g1_bird_think, "bird_think");
//li_automatic_add_function(g1_draw_and_think, "draw_and_think");


