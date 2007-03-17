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

#include "global_id.h"
#include "input.h"
#include "math/pi.h"
#include "math/trig.h"
#include "math/angle.h"
#include "g1_rand.h"
#include "resources.h"
//#include "saver.h"
#include "map_cell.h"
#include "map.h"
#include "map_man.h"
//#include "sfx_id.h"
#include "objs/bomber.h"
#include "objs/fire.h"
#include "object_definer.h"
#include "objs/path_object.h"

#include "image_man.h"
static g1_team_icon_ref radar_im("bitmaps/radar/plane.tga");

enum {
	DATA_VERSION=1
};

enum
{
	TAKE_OFF=0,
	FLYING,
	DYING
};



void g1_bomber_init()
//{{{
{
}
//}}}

g1_object_definer<g1_bomber_class>
g1_bomber_def("bomber",
			  g1_object_definition_class::TO_MAP_PIECE |
			  g1_object_definition_class::EDITOR_SELECTABLE |
			  g1_object_definition_class::MOVABLE,
			  g1_bomber_init);


g1_bomber_class::g1_bomber_class(g1_object_type id,
								 g1_loader_class *fp)
	: g1_map_piece_class(id, fp)
//{{{
{
	draw_params.setup("bomber","bomber_shadow", "bomber_lod");

	damping_fraction = 0.02f;

	radar_type=G1_RADAR_VEHICLE;
	radar_image=&radar_im;
	set_flag(BLOCKING      |
			 TARGETABLE    |
			 AERIAL        |
			 HIT_GROUND    |
			 DANGEROUS,1);

	defaults=g1_bomber_def.defaults;
	ticks_still_bombing=0;
	last_target=0;

	w16 ver=0,data_size=0;
	if (fp)
	{
		fp->get_version(ver,data_size);
	}
	switch (ver)
	{
		case DATA_VERSION:
			fp->read_format("1f4",
							&mode,
							&vspeed,
							&sway);
			break;
		default:
			if (fp)
			{
				fp->seek(fp->tell() + data_size);
			}
			mode = 0;
			vspeed = 0;
			sway = 0;
			break;
	}

	if (fp)
	{
		fp->end_version(I4_LF);
	}
}
//}}}

void g1_bomber_class::save(g1_saver_class *fp)
//{{{
{
	g1_map_piece_class::save(fp);

	fp->start_version(DATA_VERSION);

	fp->write_format("1f4",
					 &mode,
					 &vspeed,
					 &sway);

	fp->end_version();
}
//}}}

void g1_bomber_class::load(g1_loader_class *fp)
{
	g1_map_piece_class::load(fp);
	fp->check_version(DATA_VERSION);
	fp->read_format("1f4",&mode,&vspeed,&sway);
	fp->end_version(I4_LF);
};

void g1_bomber_class::skipload(g1_loader_class *fp)
{
	g1_map_piece_class::skipload(fp);
	fp->check_version(DATA_VERSION);
	w8 m;
	float v;
	w32 s;
	fp->read_format("1f4",&m,&v,&s);
	fp->end_version(I4_LF);
};

void g1_bomber_class::fire()
//{{{
{
	fire_delay = g1_bomber_def.defaults->fire_delay;

	i4_3d_vector pos(x,y,h);

	pos.x += g1_float_rand(3)*0.4f-0.2f;
	pos.y += g1_float_rand(2)*0.4f-0.2f;

	g1_fire(defaults->fire_type,
			this, attack_target.get(), pos,
			i4_3d_vector(x-lx,y-ly,h-lh));
}
//}}}

i4_bool g1_bomber_class::move(i4_float x_amount, i4_float y_amount)
//{{{
{
	i4_float
	newx = x+x_amount,
	newy = y+y_amount;

	unoccupy_location();
	if (newx>0 && newx<g1_get_map()->width() && newy>0 && newy<g1_get_map()->height())
	{
		//if (check_move(newx,newy))
		//for bombers, this is quite complicated, since they
		//cannot stop.
		//	{
		//	}
		x = newx;
		y = newy;

		if (occupy_location())
		{
			return i4_T;
		}
	}
	request_remove();
	return i4_F;
}
//}}}

i4_bool g1_bomber_class::can_attack(g1_object_class *who)
{
	i4_float dx,dy,angle,dtheta;
	if (who==last_target.get())
	{
		return i4_F;
	}
	dx=who->x-x;
	dy=who->y-y;

	angle = i4_atan2(dy,dx);
	dtheta=angle-theta;
	i4_normalize_angle(dtheta);
	if ((dtheta>i4_pi_2()&&dtheta<i4_pi_3_2())||(fabs(dx)<1&&fabs(dy)<1))
	{
		if (attack_target.valid())
		{
			ticks_still_bombing=20;
			last_target=attack_target.get();
			attack_target=0;
			return_to_path();
		}
		return i4_F; //cannot attack anything behind me.
	}
	return g1_map_piece_class::can_attack(who);
}

void g1_bomber_class::think()
//{{{
{
	if (h<(terrain_height-0.1))
	{
		damage(this,health,i4_3d_vector(0,0,1));
	}
	if (!check_life())
	{
		return;
	}

	find_target();

	if (fire_delay>0)
	{
		fire_delay--;
	}

	//if (h<=terrain_height)//we unfortunatelly crashed
//	  {
//	  mode=DYING;
//	  }


	h += vspeed;
	if (speed<0.01)
	{
		speed+=0.01;
	}

	switch (mode)
	{
		case TAKE_OFF:
			{
				if (next_path.valid())
				{
					dest_x = next_path->x - path_cos*path_len;
					dest_y = next_path->y - path_sin*path_len;
					dest_z = next_path->h - path_tan_phi*path_len;
				}

				//then raise the bomber
				i4_float dist_to_go = dest_z - h;
				if (dist_to_go>0.05)
				{
					//if he is more than halfway away, accelerate down
					//otherwise accelerate up
					if (dist_to_go > (FLY_HEIGHT * 0.5))
					{
						vspeed = (vspeed<0.05f) ? vspeed+0.005f : vspeed;
					}
					else
					{
						vspeed+=0.01f;
					}
				}
				else
				{
					//lock these in case there were any small errors
					h = dest_z;
					vspeed = 0;

					//think one more time so the l* variables catch up
					//(otherwise he'll bob up and down)
					if (h == lh)
					{
						mode = FLYING;
					}
				}
			} break;

		case FLYING:
			{
				i4_3d_vector d;
				// i4_float angle,t;

				// sway over terrain
				sway++;

				i4_float dist, dtheta=0;

				if (ticks_still_bombing>0)
				{
					ticks_still_bombing--;
					if (fire_delay==0)
					{
						fire();
					}
				}

				if (attack_target.valid())
				{
					if (fire_delay==0)
					{
						fire();
					}
					//dest_x=attack_target->x;
					//dest_y=attack_target->y;
					//dest_z=attack_target->h+2.0f;
					//suggest_air_move(dist,dtheta,d);//update internal values
					if (h<attack_target->h+2.0)
					{
						h+=0.2;
					}
					//move(d.x,d.y);
					i4_float dx,dy,angle;
					dx=attack_target->x-x;
					dy=attack_target->y-y;

					angle = i4_atan2(dy,dx);
					dtheta=angle-theta;
					i4_normalize_angle(dtheta);
					ticks_still_bombing=20; //drop more bombs, we are just over the target
					if ((dtheta>i4_pi()/2&&dtheta<i4_pi_3_2())||(fabs(dx)<1&&fabs(dy)<1))
					{
						last_target=attack_target.get();
						attack_target=0; //cannot attack this one longer
						return_to_path();
					}
					else
					{
						i4_rotate_to(theta,angle,defaults->turn_speed);
					}

					move(cos(theta)*speed,sin(theta)*speed);
					sway=0;

				}
				else
				if (ticks_still_bombing>10)
				{
					move(cos(theta)*speed,sin(theta)*speed); //move straight ahead
					dist=path_len-speed;
				}
				else
				if (next_path.valid())
				{
					dest_x = next_path->x;
					dest_y = next_path->y;
					dest_z = next_path->h;
					suggest_air_move(dist, dtheta, d);
					move(d.x,d.y);
					h += d.z + 0.02f*(float)sin(i4_pi()*sway/15.0f);
					if (dist<speed)
					{
						advance_path();
					}
				}
				else
				{
					move(cos(theta)*speed,sin(theta)*speed);
				}


				i4_float roll_to=0,pitch_to=0;
				//i4_float roll_to  = 0.02f*(float)sin(i4_pi()*sway/17.0f);
				//i4_float pitch_to = 0.02f*(float)sin(i4_pi()*sway/13.0f);




				//i4_normalize_angle(roll_to);
				if (speed>0.001 || dtheta!=0.0)
				{
					//dtheta-=i4_2pi();//denormalize
					if (dtheta<i4_pi())
					{
						roll_to = -i4_pi_2() * (dtheta);
					}                     //don't roll 'till vertical
					else
					{
						roll_to = -i4_pi_2() * (i4_2pi()-(dtheta));
					}
					pitch_to = -i4_pi_4() * speed;
				}

				i4_rotate_to(roll,roll_to,defaults->turn_speed/4);
				i4_rotate_to(pitch,pitch_to,defaults->turn_speed/4);

				groundpitch = 0; //no ground in the air (duh)
				groundroll  = 0;
			} break;

		case DYING:
			{
				i4_float &roll_speed  = dest_x;
				i4_float &pitch_speed = dest_y;

				pitch_speed += 0.004f;
				pitch += pitch_speed;

				roll_speed += 0.01f;
				roll += roll_speed;

				vspeed -= (g1_resources.gravity * 0.1f);

				i4_float dx,dy;
				dx = speed*(float)cos(theta);
				dy = speed*(float)sin(theta);
				move(dx,dy);

				if (h<=terrain_height)
				{

					g1_map_piece_class::damage(0,health,i4_3d_vector(0,0,1));
					// die somehow!!!
				}
			} break;
	}

	// have to keep thinking to sway
	request_think();
}
//}}}

void g1_bomber_class::damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
//{{{
{
	//we dont want to explode if ppl shoot us while we're dying.. we want to
	//smash into the ground and create a nice explosion

//do not take damage from a dropped bomb.
	if (obj->id==g1_get_object_type(defaults->fire_type))
	{
		return;
	}
	if (mode != DYING)
	{
		if (health<20)
		{
			i4_float &roll_speed  = dest_x;
			i4_float &pitch_speed = dest_y;

			roll_speed  = 0;
			pitch_speed = 0;
			health = 20;
			set_flag(DANGEROUS,0);
			mode = DYING;
		}
		g1_map_piece_class::damage(obj,hp,_damage_dir);
	}
}
//}}}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
