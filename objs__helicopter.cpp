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
#include "global_id.h"
#include "map_cell.h"
#include "map.h"
#include "map_man.h"
#include "objs/vehic_sounds.h"
#include "sound/sfx_id.h"
#include "objs/helicopter.h"
#include "objs/fire.h"
#include "object_definer.h"
#include "objs/path_object.h"
#include "camera.h"

#include "image_man.h"
static g1_team_icon_ref radar_im("bitmaps/radar/helicopter.tga");

static g1_model_ref model_ref("heli_body"),
shadow_ref("heli_body_shadow"),
blades_ref("heli_blades-alpha");

static i4_3d_vector blades_attach, blades_offset;

enum {
	DATA_VERSION=2
};
enum helicopter_mode
{
	TAKE_OFF=0,
	TAKE_OFF2,
	FLYING,
	DYING
};

const i4_float fly_height = 1.5;
const i4_float max_bladespeed = 0.75;

static g1_object_type missile;
void g1_helicopter_init()
{
	missile = g1_get_object_type("guided_missile");

	blades_attach.set(0,0,0);
	model_ref()->get_mount_point("Blades", blades_attach);
	blades_offset.set(0,0,0);
	blades_ref()->get_mount_point("Blades", blades_offset);
}

g1_object_definer<g1_helicopter_class>
g1_helicopter_def("helicopter",
				  g1_object_definition_class::TO_MAP_PIECE |
				  g1_object_definition_class::EDITOR_SELECTABLE |
				  g1_object_definition_class::MOVABLE|
				  g1_object_definition_class::HAS_ALPHA,
				  g1_helicopter_init);

g1_helicopter_class::g1_helicopter_class(g1_object_type id,
										 g1_loader_class *fp)
	: g1_map_piece_class(id,fp)
{
	radar_image=&radar_im;
	radar_type=G1_RADAR_VEHICLE;
	solveparams=SF_GRADE4;
	set_flag(BLOCKING      |
			 TARGETABLE    |
			 AERIAL        |
			 HIT_AERIAL    |
			 HIT_GROUND    |
			 SELECTABLE    |
			 DANGEROUS,
			 1);

	//draw_params.setup("heli_body","heli_body_shadow");

	//allocate 1 mini object
	allocate_mini_objects(1,"Helicopter Mini-Objects");
	blades = &mini_objects[0];

	//setup blades
	blades->defmodeltype = blades_ref.id();
	blades->position(blades_attach);
	blades->offset = blades_offset;
	bladespeed = 0;

	w16 ver,data_size;
	if (fp)
	{
		fp->get_version(ver,data_size);
	}
	else
	{
		ver =0;
		data_size=0;
	}

	switch (ver)
	{
		case DATA_VERSION:
			mode = 0;
			fp->read_format("1ffffff",
							&mode,

							&blades->rotation.x,&blades->rotation.x,&blades->rotation.x,
							&bladespeed,
							&vspeed,
							&upaccel);
			grab_old();
			break;
		case 1:
			mode = 0;
			if (fp->read_8())
			{
				mode=TAKE_OFF;
			}                           // take_off
			if (fp->read_8())
			{
				mode=TAKE_OFF;
			}                           // taking_off
			if (fp->read_8())
			{
				mode=FLYING;
			}                           // flying
			fp->read_8();
			fp->read_8();

			blades->rotation.x = fp->read_float();
			blades->rotation.y = fp->read_float();
			blades->rotation.z = fp->read_float();
			fp->read_float();
			fp->read_float();
			fp->read_float();

			bladespeed = fp->read_float();

			vspeed  = fp->read_float();
			upaccel = fp->read_float();
			break;
		default:
			if (fp)
			{
				fp->seek(fp->tell() + data_size);
			}
			mode = TAKE_OFF;
			blades->rotation.x = blades->lrotation.x = 0;
			blades->rotation.y = blades->lrotation.y = 0;
			blades->rotation.z = blades->lrotation.z = 0;
			bladespeed = 0;
			vspeed = upaccel = 0;
			break;
	}

	if (fp)
	{
		fp->end_version(I4_LF);
	}

	fire_delay = 0;
	blades->lod_model = blades->defmodeltype;

	blades->defmodeltype = g1_model_list_man.find_handle("heli_blades-alpha");

	draw_params.setup("heli_body","heli_body_shadow", "heli_body_lod");


	init_rumble_sound(G1_RUMBLE_HELI);

	damping_fraction = 0.02f;

	bladespeed=max_bladespeed;
}

static char *chunk_list[3]={
	"chunk_chopper_blade","chunk_chopper_body", "chunk_chopper_tail"
};

int g1_helicopter_class::get_chunk_names(char **&list)
{
	list=chunk_list;
	return 3;
}


void g1_helicopter_class::save(g1_saver_class *fp)
{
	g1_map_piece_class::save(fp);

	fp->start_version(DATA_VERSION);

	fp->write_format("1ffffff",
					 &mode,
					 &blades->rotation.x,&blades->rotation.x,&blades->rotation.x,
					 &bladespeed,
					 &vspeed,
					 &upaccel);

	fp->end_version();
}

void g1_helicopter_class::load(g1_loader_class *fp)
{
	g1_map_piece_class::load(fp);
	fp->check_version(DATA_VERSION);
	fp->read_format("1ffffff",&mode,
					&blades->rotation.x,&blades->rotation.x, &blades->rotation.x,
					&bladespeed,&vspeed,&upaccel);
	fp->end_version(I4_LF);
}

void g1_helicopter_class::skipload(g1_loader_class *fp)
{
	g1_map_piece_class::skipload(fp);
	fp->check_version(DATA_VERSION);
	fp->seek(fp->tell()+25);
	fp->end_version(I4_LF);
};

void g1_helicopter_class::fire()
{
	g1_object_class *target=attack_target.get();

	if (target)
	{
		i4_transform_class btrans,tmp1;
		i4_3d_vector pos1, pos2, dir;

		btrans.translate(x,y,h);

		tmp1.rotate_x(groundroll);
		btrans.multiply(tmp1);

		tmp1.rotate_y(groundpitch);
		btrans.multiply(tmp1);

		tmp1.rotate_z(theta);
		btrans.multiply(tmp1);

		tmp1.rotate_y(pitch);
		btrans.multiply(tmp1);

		tmp1.rotate_x(roll);
		btrans.multiply(tmp1);

		btrans.transform(i4_3d_vector(0.2f, -0.11f, -0.02f), pos1);
		btrans.transform(i4_3d_vector(0.2f,  0.11f, -0.02f), pos2);
		btrans.transform(i4_3d_vector(0.4f, -0.11f, -0.02f), dir);
		dir-=pos1;


		g1_fire(defaults->fire_type,  this, attack_target.get(), pos1, dir);
		g1_fire(defaults->fire_type,  this, attack_target.get(), pos2, dir);

		fire_delay = defaults->fire_delay;
	}
}

i4_bool g1_helicopter_class::move(i4_float x_amount, i4_float y_amount, i4_float z_amount)
{

	unoccupy_location();
	x+=x_amount;
	y+=y_amount;
	h+=z_amount;

	g1_add_to_sound_average(rumble_type, i4_3d_vector(x,y,h));
	if (occupy_location())
	{
		return i4_T;
	}

	return i4_F;
}


void g1_helicopter_class::think()
{
	if (!check_life(i4_F))
	{
		mode = DYING;
	}


	//physics
	blades->rotation.z -= bladespeed;
	h                  += vspeed;

	if (fire_delay>0)
	{
		fire_delay--;
	}


	switch (mode)
	{
		case TAKE_OFF:
			{
				g1_camera_event cev;
				cev.type=G1_WATCH_EVENT;
				cev.follow_object=this;
				g1_current_view_state()->suggest_camera_event(cev);
				mode=TAKE_OFF2;
			} break;

		case TAKE_OFF2:
			{
				if (next_path.valid())
				{
					dest_x = next_path->x - path_cos*path_len;
					dest_y = next_path->y - path_sin*path_len;
					dest_z = next_path->h - path_tan_phi*path_len;
				}

				//then raise the helicopter
				i4_float dist_to_go = dest_z - h;
				if (dist_to_go>0.05)
				{
					//if he is more than halfway away, accelerate down
					//otherwise accelerate up
					if (dist_to_go > (fly_height * 0.5))
					{
						vspeed = (vspeed<0.05f) ? vspeed+0.005f : vspeed;
					}
				}
				else
				{
					//lock these there were any small errors
					h = dest_z;
					vspeed = 0;
					upaccel = 0;

					//think one more time so the l* variables catch up
					//(otherwise he'll bob up and down)
					if (h == lh)
					{
						mode = FLYING;
					}
				}
			}

		case FLYING:
			{
				find_target();

				if (next_path.valid())
				{
					dest_x = next_path->x;
					dest_y = next_path->y;
					dest_z = next_path->h;
				}

				i4_float dist, dtheta;
				i4_3d_vector d;
				suggest_air_move(dist, dtheta, d);
				g1_object_class *blocking=0;
				i4_float save_z=d.z;
				if (check_move(d.x,d.y,d.z,blocking))
				{
					//if we're colliding with the airbase we just go up vertically.
					if (blocking && (blocking->id==g1_get_object_type("airbase")))
					{
						d.x=0;
						d.y=0;
						d.z=save_z>0.05f ? save_z : 0.05f;
					}
					if (my_solver)
					{
						if (h>terrain_height+0.3)
						{
							//fly vertical upwards if just above the floor
							move(d.x,d.y,d.z);
						}
						else
						{
							move(0,0,d.z);
						}
					}
					else
					{
						move(d.x,d.y,d.z);
					}
				}

				if (h<terrain_height)
				{
					damage(this,health,i4_3d_vector(-d.x,-d.y,-d.z));
					break;
				}
				if (dist<speed)
				{
					advance_path();
				}

				i4_float roll_to = -i4_pi()/4 * dtheta;

				i4_normalize_angle(roll_to);

				if (roll_to)
				{
					i4_rotate_to(roll,roll_to,defaults->turn_speed/4);
				}
				else
				{
					i4_rotate_to(roll,0,defaults->turn_speed/2);
				}
				//make the heli pitch forward if it moves. The amount is just pure guess, since speed is
				//in units per tick and pitch is an angle. But it seems to work
				i4_float pitch_to=speed*1.5f;
				if (i4_fabs(pitch-pitch_to)>0.05f)
				{
					//this avoids swinging.
					i4_rotate_to(pitch,pitch_to,0.05f);
				}

				if (attack_target.valid() && !fire_delay)
				{
					fire();
				}

				groundpitch = 0; //no ground when in the air (duh)
				groundroll  = 0;
			} break;

		case DYING:
			{
				i4_float &roll_speed  = dest_x;
				i4_float &theta_speed = dest_y;

				theta_speed += 0.02f;
				theta += theta_speed;

				roll_speed += 0.02f;
				roll += roll_speed;

				vspeed -= (g1_resources.gravity * 0.15f);

				if (h<=(terrain_height+0.01))
				{
					g1_map_piece_class::damage(0,health,i4_3d_vector(0,0,1));
				}                                                               // die somehow!!!

			} break;
	}

	request_think();
}

void g1_helicopter_class::damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
{
	//we dont want to explode if ppl shoot us while we're dying.. we want to
	//smash into the ground and create a nice explosion
	if (mode != DYING)
	{
		g1_map_piece_class::damage(obj,hp,_damage_dir);
		if (health<20)
		{
			i4_float &roll_speed  = dest_x;
			i4_float &theta_speed = dest_y;

			roll_speed  = 0;
			theta_speed = 0;
			health      = 20;
			set_flag(DANGEROUS,0);
			mode = DYING;
		}
	}
}
