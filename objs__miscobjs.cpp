#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "g1_object.h"
#include "saver.h"
#include "objs/model_draw.h"
#include "objs/model_collide.h"
#include "map.h"
#include "map_man.h"
#include "objs/shrapnel.h"
#include "math/random.h"

#include "objs/miscobjs.h"

class g1_deco_definition_class;

g1_deco_type_manager_class g1_deco_type_manager;

static g1_object_type shrapnel_type=0;

g1_deco_object_class::g1_deco_object_class(g1_object_type id, g1_loader_class *fp)
	: g1_object_class(id, fp) //, death(this)  // JJ modification not to issue MSVC warning
{
	death.SetG1_Object_Class(this); // JJ modification not to issue MSVC warning

	model_name=0;
	decoflags=0;
	//health=100;//the initial health is given by the defaults
	if (fp)
	{
		w16 ver,data_size;
		fp->get_version(ver,data_size);
		switch (ver)
		{
			case 2:
				health=fp->read_16();
			case 1:
				char buf[1024];
				int len=fp->read_16();
				fp->read(buf, len);

				model_name=g1_deco_type_manager.find_name(buf);
				if (model_name)
				{
					char lod_name[256];
					sprintf(lod_name, "%s_lod", model_name);

					draw_params.setup(model_name, 0, lod_name);
				}
		}
		fp->end_version(I4_LF);
	}

	//collision_type = 1;

	if (!fp)
	{
		player_num=G1_MAX_PLAYERS-1;
	}
	set_flag(BLOCKING      |
			 //SELECTABLE    |
			 CAN_DRIVE_ON  |
			 TARGETABLE    |
			 GROUND        |
			 SHADOWED,
			 1);
}

void g1_deco_object_class::save(g1_saver_class *fp)
{
	g1_object_class::save(fp);

	fp->start_version(DATA_VERSION);
	fp->write_16(health);

	if (model_name)
	{
		int len=strlen(model_name)+1;
		fp->write_16(len);
		fp->write((void *)model_name, len);
	}
	else
	{
		fp->write_16(0);
	}

	fp->end_version();
}

i4_bool g1_deco_object_class::check_collision(g1_object_class *source,const i4_3d_vector &start, i4_3d_vector &ray)
{
	i4_3d_vector normal;
	if (decoflags&DECO_USEPOLYCOLDET)
	{
		return g1_model_collide_polygonal_ex(this, source, ray, normal);
	}
	else if (decoflags&DECO_GHOST)
	{
		return i4_F;
	}
	return g1_model_collide_radial(this, draw_params, start, ray);
}



void g1_deco_object_class::think()
{
	//if (occupancy_radius()>0.8 && !death.think())
	//  return;
	if (decoflags & DECO_INVULNERABLE)
	{
		health=get_max_health();
		return;
	}
	if (!death.think())
	{
		//occupancy_radius() might become very small
		//if objects are scalled
		return;
	}

	if (model_name==0)
	{
		unoccupy_location();
		request_remove();
	}

	if (health<0)
	{
		g1_shrapnel_class *shrapnel = NULL;
		if (!shrapnel_type)
		{
			shrapnel_type = g1_get_object_type("shrapnel");
		}
		shrapnel = (g1_shrapnel_class *)g1_create_object(shrapnel_type);
		if (shrapnel)
		{
			i4_float rh = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2f;
			shrapnel->setup(x, y, h + rh, 10, i4_T);
		}

		unoccupy_location();
		request_remove();
	}
}

void g1_deco_object_class::damage(g1_object_class *who_is_hurting,
								  int how_much_hurt, i4_3d_vector damage_dir)
{
	if (decoflags & DECO_INVULNERABLE)
	{
		return;
	}
	if (occupancy_radius()>0.8)
	{
		death.damage(who_is_hurting, how_much_hurt, damage_dir);
	}
	else
	{
		g1_object_class::damage(who_is_hurting,how_much_hurt,damage_dir);
	}
}

class g1_deco_definition_class :
	public g1_object_definition_class
{
public:
	char deco_name[40];
	w32 decoflags;

	g1_object_class *create_object(g1_object_type type,
								   g1_loader_class *fp)
	{
		g1_deco_object_class *o=new g1_deco_object_class(type, fp);

		if (!fp)
		{
			o->model_name=_name;
			if (o->model_name)
			{
				char lod_name[256];
				sprintf(lod_name, "%s_lod", o->model_name);

				o->draw_params.setup(o->model_name, 0, lod_name);
				o->set_flag(g1_object_class::SHADOWED,1);
			}

		}
		o->decoflags=decoflags; //by now, we set them to all (not only new) objects
		o->init();

		return o;
	}

	g1_deco_definition_class(char *__name, w32 _decoflags)
		: g1_object_definition_class(strcpy(deco_name, __name))
	{
		set_flag(MOVABLE,0);
		decoflags=_decoflags;
	}

	~g1_deco_definition_class()
	{
		g1_deco_type_manager.remove_type(this);
	}

};

g1_object_type g1_create_deco_object(char *name, w32 decoflags)
{
	g1_deco_definition_class *def=new g1_deco_definition_class(name, decoflags);
	g1_object_type t=def->type;
	def->flags|=g1_object_definition_class::TO_DECO_OBJECT |
				 g1_object_definition_class::DELETE_WITH_LEVEL;

	g1_deco_type_manager.add_type(def);

	return t;
}


char *g1_deco_type_manager_class::find_name(const char *name)
{
	for (int i=0; i<deco_objs.size(); i++)
	{
		if (deco_objs[i])
		{
			if (strcmp(name, deco_objs[i]->name())==0)
			{
				return (char *)deco_objs[i]->name();
			}
		}
	}

	return 0; // this object name doesn't exsist in the game anymore
}

void g1_deco_type_manager_class::uninit()
{
	deco_objs.uninit();
}
