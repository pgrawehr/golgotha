#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "g1_object.h"
#include "map.h"
#include "map_man.h"
#include "lisp/li_class.h"
#include "objs/structure_death.h"
#include "objs/particle_emitter.h"
#include "objs/explode_model.h"
#include "objs/carcass.h"
#include "math/random.h"
#include "math/pi.h"

// circular distribution precomputation
//#include "obj/circle_distrib.cc"
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

static i4_float distrib[][3] =
{
	{  0.1119191f, -0.2615892f,  0.0809548f },
	{ -0.0452529f, -0.3379074f,  0.1162292f },
	{  0.0279555f, -0.3618025f,  0.1316825f },
	{  0.1749611f,  0.3536127f,  0.1556534f },
	{ -0.3699638f, -0.1603638f,  0.1625898f },
	{  0.2052169f,  0.3553498f,  0.1683875f },
	{ -0.4332598f,  0.1643958f,  0.2147400f },
	{  0.3386490f, -0.3234571f,  0.2193077f },
	{ -0.4117406f, -0.3958438f,  0.3262226f },
	{ -0.5119566f, -0.3265958f,  0.3687644f },
	{  0.0235136f, -0.6238471f,  0.3897381f },
	{ -0.4229256f, -0.4791123f,  0.4084147f },
	{ -0.6360176f,  0.2824885f,  0.4843182f },
	{  0.3013742f,  0.6421718f,  0.5032111f },
	{  0.7283822f, -0.4513522f,  0.7342594f },
	{ -0.7393920f,  0.4438775f,  0.7437278f },
	{  0.6292487f,  0.6089329f,  0.7667533f },
	{  0.6443001f, -0.6544292f,  0.8434002f },
	{ -0.5181742f, -0.7614983f,  0.8483842f },
	{ -0.1292248f,  1.0149743f,  1.0468719f },
	{ -1.0265210f, -0.0737772f,  1.0591884f },
	{  1.0388487f, -0.2767890f,  1.1558188f },
	{ -1.0874339f,  0.1317726f,  1.1998766f },
	{  0.2515588f,  1.0682873f,  1.2045196f },
	{  0.5765704f, -0.9755095f,  1.2840523f },
	{  0.0266290f,  1.1391640f,  1.2984036f },
	{ -1.1406190f, -0.2395954f,  1.3584176f },
	{ -1.0869348f, -0.4419400f,  1.3767383f },
	{ -0.4602780f,  1.1603298f,  1.5582211f },
	{  1.2395019f,  0.2034033f,  1.5777378f },
	{ -0.0542220f, -1.2694477f,  1.6144374f },
	{ -1.1491087f, -0.6274906f,  1.7141951f },
	{  0.5080261f,  1.2831039f,  1.9044461f },
	{ -1.0483423f, -0.9411163f,  1.9847215f },
	{ -1.3191282f, -0.5656053f,  2.0600087f },
	{ -1.3691046f,  0.5233776f,  2.1483714f },
	{ -0.5854948f, -1.3513192f,  2.1688677f },
	{  1.3562519f,  0.6796750f,  2.3013772f },
	{  0.2330792f, -1.5546416f,  2.4712365f },
	{  1.5848205f,  0.0088370f,  2.5117340f },
	{  0.4695513f, -1.5172555f,  2.5225426f },
	{  1.4568255f,  0.6415380f,  2.5339116f },
	{  1.5364338f, -0.4324282f,  2.5476231f },
	{  1.5816531f, -0.2210640f,  2.5504959f },
	{ -1.3291359f, -0.9233928f,  2.6192564f },
	{  0.8494235f,  1.4087872f,  2.7062018f },
	{  0.0659764f,  1.6490205f,  2.7236215f },
	{  1.5988521f,  0.5281074f,  2.8352254f },
	{ -1.0747915f, -1.3194717f,  2.8961823f },
	{ -1.2052282f, -1.2147751f,  2.9282535f },
};

static li_symbol_ref particles("particle_emitter");
static r1_texture_ref explosion2("smoke_particle");
static li_symbol_ref explode_model("explode_model");
static g1_model_ref carcass("garage_charred");

g1_particle_emitter_params t_params;
i4_float t_rad = 0.6f;
i4_float t_fac_grav = 0.002f;
i4_float t_roll = 0.01f;
i4_float t_rspeed = 0.05f;

int death_time = 50;

class global_setter_class
{
public:
	global_setter_class()
	{
		t_params.defaults();
		t_params.start_size=0.4f;
		t_params.particle_lifetime=20;
		t_params.grow_speed=-t_params.start_size/t_params.particle_lifetime;
		t_params.max_speed=0.05f;
		t_params.air_friction=0.90f;
		t_params.num_create_attempts_per_tick=1;
		t_params.creation_probability=0.50;
		t_params.speed.set(0, 0, 0.05f);
		t_params.emitter_lifetime=death_time;
	};
} global_setter;

i4_bool g1_structure_death_class::think()
{
	if (me->health<=0)
	{
		me->request_think();
		me->health--;
		me->h += i4_float(me->health)*t_fac_grav;
		me->roll += t_roll;

		if (me->health<-death_time)
		{
			me->unoccupy_location();
			me->request_remove();
		}
		return i4_F;
	}
	return i4_T;
}

void g1_structure_death_class::damage(g1_object_class * obj, int hp, i4_3d_vector damage_dir)
{
	if (me->health<=0)
	{
		return;
	}

	me->health-=hp;

	if (me->health<=0)
	{
		me->health=0;

		g1_explode_model_class * e;
		e=(g1_explode_model_class *)g1_create_object(g1_get_object_type(explode_model.get()));
		g1_explode_params params;
		params.stages[0].setup(0,  0.0008f, G1_APPLY_SING);
		params.stages[1].setup(10, 0,      G1_APPLY_SING);
		params.t_stages=2;
		params.gravity=0.05f;
		e->setup(me, i4_3d_vector(0,0,0), params);

		i4_float limit = me->occupancy_radius();
		limit *= limit;

		for (int n=0; n<sizeof(distrib)/sizeof(distrib[0]) && distrib[n][2]<limit; n++)
		{
			g1_particle_emitter_params params;
			params.defaults();
			params.start_size=0.4f;
			params.grow_speed=-params.start_size/20;
			params.max_speed=0.1f;
			params.air_friction=0.90f;
			params.num_create_attempts_per_tick=1;
			params.creation_probability=0.50f;
			params.particle_lifetime=20;
			params.speed.set(0, 0, 0.05f);

			params = t_params;

			params.emitter_lifetime += i4_rand()&0x15;

			params.texture=explosion2.get();
			g1_particle_emitter_class * emitter =
				(g1_particle_emitter_class *)g1_create_object(g1_get_object_type(particles.get()));

			i4_float
			th = i4_float_rand() *i4_2pi(),
			px = distrib[n][0] + me->x,
			py = distrib[n][1] + me->y;

			params.speed.x = t_rspeed*distrib[n][0];
			params.speed.y = t_rspeed*distrib[n][1];

			if (emitter)
			{
				emitter->setup(px,py, g1_get_map()->terrain_height(px,py), params);
			}
		}

		g1_create_carcass(me, carcass.get(), "garage_charred", -1, -1, 0);

//     g1_object_class *o=g1_create_object(g1_get_object_type("garage_charred"));
//     o->x = me->x;
//     o->y = me->y;
//     o->h = me->h;
//     o->theta = me->theta;
//     o->pitch = me->pitch;
//     o->roll = me->roll;
//     o->grab_old();
//     o->occupy_location();

		me->request_think();
	}
}
