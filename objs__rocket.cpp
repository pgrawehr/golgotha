#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "g1_object.h"
#include "objs/model_draw.h"
#include "math/num_type.h"
#include "objs/rocket.h"
#include "tile.h"
#include "objs/explosion1.h"
#include "saver.h"
#include "objs/fire_angle.h"
#include "math/pi.h"
#include "objs/map_piece.h"
#include "map.h"
#include "map_man.h"
#include "math/angle.h"
#include "math/trig.h"
#include "image/image.h"
#include "objs/smoke_trail.h"
#include "object_definer.h"
#include "g1_render.h"
#include "g1_rand.h"

static g1_object_type shrapnel_type, explosion_type, smoke_trail_type;
void g1_rocket_init()
{
	shrapnel_type = g1_get_object_type("shrapnel");
	explosion_type = g1_get_object_type("explosion1");
	smoke_trail_type = g1_get_object_type("smoke_trail");
}

g1_object_definer<g1_rocket_class>
g1_rocket_def("rocket",
			  g1_object_definition_class::MOVABLE,
			  g1_rocket_init);

g1_rocket_class::g1_rocket_class(g1_object_type id,
								 g1_loader_class *fp)
	: g1_object_class(id,fp)
{
	draw_params.setup("missile");

	if (fp && fp->check_version(G1_ROCKET_DATA_VERSION))
	{
		zv=fp->read_float();
		speed=fp->read_float();

		fp->read_reference(who_fired_me);
		fp->read_reference(track_object);
		i4_warning("loaded rocket that does no damage");
		fp->end_version(I4_LF);
	}
	else
	{
		zv=0;
		speed=0;
	}

	damage = 0;

	radar_type=G1_RADAR_WEAPON;
	set_flag(AERIAL        |
			 HIT_GROUND    |
			 SHADOWED, 1);
}

void g1_rocket_class::save(g1_saver_class *fp)
{
	// save data associated with base classes
	g1_object_class::save(fp);

	fp->start_version(G1_ROCKET_DATA_VERSION);

	fp->write_float(zv);
	fp->write_float(speed);

	fp->write_reference(who_fired_me);
	fp->write_reference(track_object);

	fp->end_version();
}
void g1_rocket_class::load(g1_loader_class *fp)
{
	g1_object_class::load(fp);
	fp->check_version(G1_ROCKET_DATA_VERSION);
	zv=fp->read_float();
	speed=fp->read_float();
	fp->skip_reference();
	fp->skip_reference();
	//Cannot read these two, as the ids might not match
	//fp->read_reference(who_fired_me);
	//fp->read_reference(track_object);
	fp->end_version(I4_LF);
};

void g1_rocket_class::skipload(g1_loader_class *fp)
{
	g1_object_class::skipload(fp);
	fp->check_version(G1_ROCKET_DATA_VERSION);
	fp->read_float();
	fp->read_float();
	fp->skip_reference();
	fp->skip_reference();
	fp->end_version(I4_LF);
};

void g1_rocket_class::fire_at(i4_float sx, i4_float sy, i4_float sz,
							  i4_float angle,
							  g1_map_piece_class *kill_target,
							  g1_map_piece_class *owner,
							  w32 _damage)
{
	damage = _damage;

	lx=x=sx;
	ly=y=sy;
	lh=h=sz;

	i4_float x_y_dist = (float)sqrt((kill_target->x - sx)*(kill_target->x - sx) +
									(kill_target->y - sy)*(kill_target->y - sy));


	i4_float angle_with_ground;

	if (!g1_calc_fire_angle(x_y_dist, sz, kill_target->h, g(), vo(), angle_with_ground))
	{
		angle_with_ground=i4_pi()/4;
	}


	speed=(float)cos(angle_with_ground) *vo();
	zv=(float)sin(angle_with_ground) *vo();


	ltheta=theta=angle;
	pitch = lpitch = (float)atan(-zv/speed);
	player_num=owner->player_num;

	track_object = kill_target;
	who_fired_me = owner;

	occupy_location();
	request_think();
}

void g1_rocket_class::fire(i4_float sx, i4_float sy, i4_float sz,
						   i4_angle angle,
						   g1_map_piece_class *owner,
						   w32 _damage)
{
	damage = _damage;

	i4_float angle_with_ground=i4_pi()/4;

	speed=(float)cos(angle_with_ground) *vo();
	zv=(float)sin(angle_with_ground) *vo();

	theta=angle;

	who_fired_me = owner;
	player_num=owner->player_num;

	occupy_location();
	request_think();
}

void i4_normalize_angle(i4_float &angle);


void g1_rocket_class::think()
{
	zv-=g();
	pitch = (float)atan(-zv/speed);
	i4_normalize_angle(pitch);

	move((float)cos(theta) * speed, (float)sin(theta) * speed, zv);
}

void g1_rocket_class::draw(g1_draw_context_class *context, i4_3d_vector& viewer_position)
{
	g1_model_draw(this, draw_params, context, viewer_position);

	if (smoke_trail.valid())
	{
		smoke_trail->draw(context, viewer_position);
	}
}

i4_bool g1_rocket_class::move(i4_float x_amount,
							  i4_float y_amount,
							  i4_float z_amount)
{

	g1_smoke_trail_class *s;
	if (!smoke_trail.valid())
	{
		s=(g1_smoke_trail_class *)g1_create_object(smoke_trail_type);
		if (s)
		{
			s->setup(x, y, h, 0.01f, 0.1f, 0xffd000, 0xffffff); // orange to white
			s->occupy_location();
			smoke_trail=s;
		}
	}
	else
	{
		s=(g1_smoke_trail_class *)smoke_trail.get();
	}


	g1_object_class *hit = NULL;

	unoccupy_location();

	i4_3d_vector ray(x_amount, y_amount, z_amount);

	if (!g1_get_map()->check_non_player_collision(this,player_num,
												  i4_3d_vector(x,y,h),
												  ray,
												  hit))
	{
		x += x_amount;
		y += y_amount;
		h += z_amount;

		request_think();
		occupy_location();

		if (s)
		{
			s->update_head(x,y,h);
		}

		return i4_T;
	}
	else
	{
		if (s) // delete the smoke trail
		{
			s->unoccupy_location();
			s->request_remove();
		}

		// explode
		if (hit)
		{

			if (fabs(hit->h - h)<0.2)
			{
				hit->damage(this, damage, i4_3d_vector(x_amount,y_amount,z_amount));
				g1_explosion1_class *explosion;
				explosion=(g1_explosion1_class *)g1_create_object(explosion_type);

				i4_float rx,ry,rh;
				rx = g1_float_rand(34) *0.2f - 0.1f;
				ry = g1_float_rand(10) *0.2f - 0.1f;
				rh = g1_float_rand(02) *0.2f - 0.1f;

				explosion->setup(hit->x+rx,hit->y+ry,hit->h + rh);
				x = hit->x + rx;
				y = hit->y + ry;
				h = hit->h + rh;
				request_remove();
				return i4_F;
				;
			}

			x += x_amount;
			y += y_amount;
			h += z_amount;

			request_think();
			occupy_location();
			return i4_T;
		}
		else if (who_fired_me.valid())
		{
			g1_explosion1_class *explosion=(g1_explosion1_class *)g1_create_object(explosion_type);

			i4_float rx,ry,rh;

			rx = g1_float_rand(34) *0.2f - 0.1f;
			ry = g1_float_rand(10) *0.2f - 0.1f;
			rh = g1_float_rand(02) *0.2f - 0.1f;


			explosion->setup(lx+rx,ly+ry,lh+rh);
		}

		request_remove();

		return i4_F;
	}
}
