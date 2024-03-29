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
#include "objs/rocktank.h"
#include "objs/peontank.h"
#include "input.h"
#include "math/pi.h"
#include "math/trig.h"
#include "math/angle.h"
#include "g1_rand.h"
#include "resources.h"
#include "saver.h"
#include "objs/vehic_sounds.h"
#include "sound/sfx_id.h"
#include "object_definer.h"
#include "g1_render.h"
#include "objs/fire.h"
#include "lisp/lisp.h"

#include "image_man.h"
static g1_team_icon_ref radar_im("bitmaps/radar/peon_tank.tga");

// Constants
enum {
	DATA_VERSION=2
};
enum {
	ORIGNAL_RUMBLE_VOLUME=I4_SOUND_VOLUME_LEVELS/8
};

static g1_model_ref model_ref("peon_body"),
shadow_ref("peon_body_shadow"),
turret_ref("peon_turret"),
lod_bot("peon_body_lod"),
lod_top("peon_turret_lod");

static g1_object_type bullet;
static i4_3d_vector turret_attach;

void g1_peon_tank_init()
{
	bullet = g1_get_object_type("b90mm");

	turret_attach.set(0,0,0);
	model_ref()->get_mount_point("Turret", turret_attach);
}

g1_object_definer<g1_peon_tank_class>
g1_peon_tank_def("peon_tank",
				 g1_object_definition_class::EDITOR_SELECTABLE |
				 g1_object_definition_class::MOVABLE |
				 g1_object_definition_class::TO_MAP_PIECE,
				 g1_peon_tank_init);


static char * chunk_list[3]={
	"chunk_peon_barrel", "chunk_peon_body", "chunk_peon_turret"
};


int g1_peon_tank_class::get_chunk_names(char * *&list)
{
	list=chunk_list;
	return 3;
}


g1_peon_tank_class::g1_peon_tank_class(g1_object_type id,
									   g1_loader_class * fp)
	: g1_map_piece_class(id, fp)
{

	radar_type=G1_RADAR_VEHICLE;
	radar_image=&radar_im;
	set_flag(BLOCKING      |
			 SELECTABLE    |
			 TARGETABLE    |
			 GROUND        |
			 HIT_GROUND    |
			 HIT_AERIAL    | //new: peon tank can attack airplanes
			 DANGEROUS,1);

	draw_params.setup(model_ref.id(),turret_ref.id(), lod_bot.id());

	allocate_mini_objects(2,"Peon Tank Mini-objects");
	turret = &mini_objects[0];

	turret->defmodeltype = turret_ref.id();
	turret->lod_model = lod_top.id();
	turret->position(turret_attach);

	w16 ver=0,data_size=0;

	if (fp)
	{
		fp->get_version(ver,data_size);
	}

	switch (ver)
	{
		case DATA_VERSION:
			fp->read_format("fff",
							&turret->rotation.x,
							&turret->rotation.y,
							&turret->rotation.z);
			turret->grab_old();
			break;

		case 1:
			turret->rotation.x = fp->read_float();
			turret->rotation.y = fp->read_float();
			turret->rotation.z = fp->read_float();

			turret->lrotation.x = fp->read_float();
			turret->lrotation.y = fp->read_float();
			turret->lrotation.z = fp->read_float();
			break;

		default:
			if (fp)
			{
				fp->seek(fp->tell() + data_size);
			}
			turret->rotation.x = 0;
			turret->rotation.y = 0;
			turret->rotation.z = 0;
			turret->lrotation = turret->rotation;
			fire_delay = 0;
			break;
	}

	if (fp)
	{
		fp->end_version(I4_LF);
	}


	init_rumble_sound(G1_RUMBLE_GROUND);

	guard_angle = 0;
	guard_time = 0;
	tread_pan=0.0;

	turret_kick = lturret_kick = 0;
}

void g1_peon_tank_class::load(g1_loader_class * fp)
{
	g1_map_piece_class::load(fp);
	fp->check_version(DATA_VERSION);
	fp->read_format("fff",&turret->rotation.x,
					&turret->rotation.y,&turret->rotation.z);
	fp->end_version(I4_LF);
};

void g1_peon_tank_class::skipload(g1_loader_class * fp)
{
	g1_map_piece_class::skipload(fp);
	float a,b,c;
	fp->check_version(DATA_VERSION);
	fp->read_format("fff",&a,&b,&c);
	fp->end_version(I4_LF);
};

void g1_peon_tank_class::save(g1_saver_class * fp)
{
	g1_map_piece_class::save(fp);

	fp->start_version(DATA_VERSION);

	fp->write_format("fff",
					 &turret->rotation.x,
					 &turret->rotation.y,
					 &turret->rotation.z);

	fp->end_version();

}


void g1_peon_tank_class::fire()
{
	g1_object_class * target=attack_target.get();

	if (target)
	{
		fire_delay = defaults->fire_delay;
		i4_3d_vector pos, dir;

		i4_transform_class t, main, l2w;
		turret->calc_transform(1.0, &t); //local transform from tank to turret
		calc_world_transform(1.0, &main); //transform from world to tank
		l2w.multiply(main, t); //transform from world to turret

		l2w.transform(i4_3d_point_class(0.4f, 0, 0.11f), pos); //movement allong turret?
		dir=l2w.x;

		g1_fire(defaults->fire_type, this, target, pos, dir);
	}
}

void g1_peon_tank_class::think()
{
	g1_map_piece_class::think();

	if (!alive())
	{
		return;
	}

	//turret_kick thing
	lturret_kick = turret_kick;
	request_think();

	if (turret_kick < 0.f)
	{
		turret_kick += 0.01f;

		if (turret_kick >= 0)
		{
			turret_kick = 0;
		}

	}

	//aim the turret
	if (attack_target.valid())
	{

		i4_float dx,dy,dangle,dz;

		i4_3d_point_class pos;

		lead_target(pos);

		//this will obviously only be true if attack_target.ref != NULL
		dx = ((x+turret->x) - pos.x);
		dy = ((y+turret->y) - pos.y);
		dz = ((h+turret->h) - pos.z);

		//aim the turret

		i4_float dist=i4_fsqrt(dx*dx+dy*dy);
		i4_float height_angle=i4_atan2(dz,dist);
		if (height_angle>i4_pi_4())
		{
			height_angle=i4_pi_4();
		}
		if (height_angle<-i4_pi_4())
		{
			height_angle=-i4_pi_4();
		}
		i4_rotate_to(turret->rotation.y,height_angle,defaults->turn_speed);

		guard_angle = i4_atan2(dy,dx) + i4_pi() - theta;
		i4_normalize_angle(guard_angle);

		dangle = i4_rotate_to(turret->rotation.z, guard_angle, defaults->turn_speed);
		if (dangle<defaults->turn_speed && dangle>-defaults->turn_speed)
		{
			if (!fire_delay)
			{
				fire();
			}
		}
		guard_time = 30;
	}
	else
	{

		if (moved())
		{
			guard_angle = 0;
			guard_time = 30;
		}
		else if (guard_time<=0)
		{
			guard_angle = g1_float_rand(83) *i4_2pi();
			guard_time = 20;
		}

		if (i4_rotate_to(turret->rotation.z, guard_angle, defaults->turn_speed)==0)
		{
			if (guard_time>0)
			{
				guard_time--;
			}
		}
	}
}

void g1_peon_tank_class::draw(g1_draw_context_class * context, i4_3d_vector& viewer_position)
{
	turret->offset.x = i4_interpolate(lturret_kick, turret_kick, g1_render.frame_ratio);

	g1_map_piece_class::draw(context, viewer_position);
}
