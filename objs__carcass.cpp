/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "string/string.h"
#include "g1_object.h"
#include "objs/model_draw.h"
#include "lisp/lisp.h"
#include "render/tex_id.h"
#include "objs/particle_emitter.h"
#include "object_definer.h"
#include "saver.h"
#include "map_man.h"
#include "map.h"



static li_symbol_ref part_emit("particle_emitter");
static r1_texture_ref smoke_texture("smoke_particle");

class g1_carcass_class :
	public g1_object_class
{
public:
	int time_left;
	i4_const_str * model_name; //used to keep the model across saves/loads
	enum {
		DATA_VERSION=2
	};

	g1_carcass_class(g1_object_type id, g1_loader_class * fp)
		: g1_object_class(id, fp)
	{
		model_name=0; //just in case...
		w16 ver=0,data=0; //it is possible that there follows no data for this
		if (fp) //object
		{
			fp->get_version(ver,data);
			if (ver==DATA_VERSION)
			{
				time_left=fp->read_32();
				model_name=fp->read_counted_str();
				char buf[300];
				i4_os_string(*model_name,buf,300);
				draw_params.setup(buf);
				fp->end_version(I4_LF);
			}
			else
			{
				fp->seek(fp->tell()-4); //go back to start of obj
				x=-1;
			}
		}
		else
		{
			time_left=100;
			player_num=G1_MAX_PLAYERS-1;
		}
	};

	~g1_carcass_class()
	{
		delete model_name;
	}

	void save(g1_saver_class * fp)
	{
		g1_object_class::save(fp);
		fp->start_version(DATA_VERSION);
		fp->write_32(time_left);
		fp->write_counted_str(*model_name);
		fp->end_version();
	};

	void load(g1_loader_class * fp)
	{
		g1_object_class::load(fp);
		fp->check_version(DATA_VERSION);
		time_left=fp->read_32();
		delete model_name;
		model_name=fp->read_counted_str();
		fp->end_version(I4_LF);
	}
	void skipload(g1_loader_class * fp)
	{
		g1_object_class::skipload(fp);
		fp->check_version(DATA_VERSION);
		fp->read_32();
		i4_str * temp=fp->read_counted_str();
		delete temp;
		fp->end_version(I4_LF);
	}

	virtual void draw(g1_draw_context_class * context, i4_3d_vector& viewer_position)
	{
		g1_model_draw(this, draw_params, context, viewer_position);
		flags|=TARGETABLE | GROUND | BLOCKING | CAN_DRIVE_ON;
	};

	void setup(g1_object_class * from,
			   g1_quad_object_class * model,
			   const i4_const_str &model_n,
			   int ticks,
			   int ticks_to_smoke,
			   g1_quad_object_class * lod_model)
	{
		if (ticks_to_smoke>0)
		{
			g1_particle_emitter_class * smoke=
				(g1_particle_emitter_class *)g1_create_object(g1_get_object_type(part_emit.get()));

			g1_particle_emitter_params params;
			params.defaults();
			params.texture=smoke_texture.get();
			params.start_size=0.05f;
			params.grow_speed=0.005f;
			params.particle_lifetime=50;
			params.num_create_attempts_per_tick=1;
			params.creation_probability=0.5;
			params.speed=i4_3d_vector(0.001f, 0.001f, 0.05f);
			params.emitter_lifetime = ticks_to_smoke;

			if (smoke)
			{
				smoke->setup(from->x, from->y, from->h, params);
			}
		}

		time_left = ticks;

		theta=from->theta;
		x=from->x;
		y=from->y;
		h=from->h;
		grab_old();
		delete model_name;
		model_name=0;
		model_name=new i4_const_str(model_n);

		draw_params.setup(model, 0, lod_model);
		occupy_location();
	}


	void think()
	{
		if (time_left>=0)
		{
			time_left--;
			if (time_left<=0)
			{
				unoccupy_location();
				request_remove();
				return;
			}
		}
		if (g1_get_map()->terrain_height(x,y)<h) //may happen i.e
		{
			//if the stank exploded together with the bridge he was on
			unoccupy_location();
			h-=g1_resources.gravity;
			pitch+=0.12f; //some random values.
			roll-=0.024f;
			theta+=0.002f;
			occupy_location();
		}
		request_think();
	}
};

g1_object_definer<g1_carcass_class>
g1_carcass_def("carcass", g1_object_definition_class::EDITOR_SELECTABLE);

g1_object_class *g1_create_carcass(g1_object_class * from,
								   g1_quad_object_class * model,
								   const i4_const_str &model_n,
								   int ticks,
								   int ticks_to_smoke,
								   g1_quad_object_class * lod_model)
{
	g1_carcass_class * c=(g1_carcass_class *)g1_create_object(g1_carcass_def.type);

	c->setup(from, model, model_n, ticks, ticks_to_smoke, lod_model);
	//new definiton: setup must call occupy_location().
	//c->occupy_location();
	return c;
}
