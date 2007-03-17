/* This is new file of the revival project.
   It contains code to simulate day and night changes
 */

#include "pch.h"
#include "g1_object.h"
#include "object_definer.h"
#include "objs/day_night.h"
#include "map.h"
#include "gui/smp_dial.h"
#include "lisp/li_class.h"
#include "light.h"
#include "sky.h"
#include "math/pi.h"
#include "math/num_type.h"
#include "map_light.h"

const float ticksperday=864000;
//I assume you can't see light vector changes of less than aprox 2 degrees.
//The sun will probably be moved in smaller steps across the sky.
const float adjustlightdiff=0.07f;
void g1_day_night_init()
{
};

g1_object_definer<g1_day_night_class>
g1_day_night_def("day_night_change",
				 g1_object_definition_class::EDITOR_SELECTABLE,
				 g1_day_night_init);



g1_mapsingle_class::g1_mapsingle_class(g1_object_type id, g1_loader_class *fp)
	: g1_object_class(id,fp)
{
	g1_object_class *other=g1_get_map()->find_object_by_id(id,0);
	needsdeleteonnexttick=i4_F;
	if (other)
	{
		char buf[200];
		g1_object_definition_class *def=other->get_type();
		sprintf(buf,"The map already contains an object of type %s "
					"as ID %d (Location: x=%d y=%d). "
					"Only one such object is allowed at once.",
				def->name(),
				other->global_id,
				other->x,other->y);
		i4_warning(buf);
		needsdeleteonnexttick=i4_T;
		vars=0;    //nothing valid here

	}
	draw_params.setup("sun_and_moon");
};

void g1_mapsingle_class::think()
{
	if (needsdeleteonnexttick)
	{
		stop_thinking();
		request_remove();
	}
};

void g1_mapsingle_class::damage(g1_object_class *who_is_hurting,
								int how_much_hurt, i4_3d_vector damage_dir)
{
};

void g1_mapsingle_class::draw(g1_draw_context_class *context, i4_3d_vector &viewer_position)
{
	if (needsdeleteonnexttick==i4_F)
	{
		g1_editor_model_draw(this,draw_params,context, viewer_position);
	}
}


g1_day_night_class::g1_day_night_class(g1_object_type id, g1_loader_class *fp)
	: g1_mapsingle_class(id,fp)
{
	currentpos=0;
	first=i4_T;
};


static li_symbol_class_member active("active");
static li_object_class_member daysky("daysky");
static li_object_class_member nightsky("nightsky");
static li_int_class_member ticks("advance_ticks"),
current_time("current_time");
static li_float_class_member darkness_factor("darkness_factor");

void g1_day_night_class::think()
{
	li_class_context ctx(this->vars);
	i4_bool day=isday(current_time());
	i4_bool nextday=isday(current_time()+ticks());
	if ((day!=nextday)||first)
	{
		if (nextday)
		{
			//it's getting day
			if (g1_get_map()->sky_name)
			{
				delete g1_get_map()->sky_name;
			}
			g1_get_map()->sky_name=new i4_str(li_g1_sky::get(daysky(),0)->value()->value());

		}
		else
		{
			//it's dusk
			if (g1_get_map()->sky_name)
			{
				delete g1_get_map()->sky_name;
			}
			g1_get_map()->sky_name=new i4_str(li_g1_sky::get(nightsky(),0)->value()->value());

		}
		first=i4_F;
	}
	day=nextday;
	current_time()+=ticks();
	//now we would actually need to calculate the position of
	//the sun (or the moon) for the current time. This depends on:
	// - location on the planet
	// - date (including year, to be precise)
	// - time

	//For demonstrative purposes, we only use the time now and assume
	//we are at the aequator on 21 of september.
	i4_float circum=1*2 *i4_pi(); //the circumference of the unit circle.
	i4_float pos=circum *current_time()/ticksperday;
	if (i4_fabs(pos-currentpos)>adjustlightdiff)
	{
		currentpos=pos;
		i4_float rads=0;
		if (day)
		{
			//create the light vector for day
			rads=pos;
			i4_3d_vector daylight;
			daylight.x=0;
			daylight.y=-cosf(rads);
			daylight.z=-sinf(rads);
			g1_lights.direction=daylight;
		}
		else
		{
			//create the light vector for night.
			rads=pos;
			i4_3d_vector moonlight;
			moonlight.x=0;
			moonlight.y=-cosf(rads);
			moonlight.z=sinf(rads); //is already inverted once
			g1_lights.direction=moonlight;
		}
		g1_lights.set_ambient_intensity(0.3f*calc_intensity(day,pos));
		g1_lights.set_directional_intensity(calc_intensity(day,pos));
		// The following function needs to be changed in such a way
		// that it changes only part of the map each tick, since
		// the entire update takes way to much time.
		g1_calc_static_lighting();
	}
	request_think();
}

i4_float g1_day_night_class::calc_intensity(i4_bool atday, i4_float pos)
{
	//if (!atday)
	//	pos-=i4_pi()/2;
	return sinf(pos);
}

i4_bool g1_day_night_class::isday(int tick)
{
	if (tick<ticksperday/2)
	{
		return i4_T;
	}
	return i4_F;
}

void g1_day_night_class::object_changed_by_editor(g1_object_class *who, li_class *old_values)
{
	if (who==this)
	{
		li_class_context ctx(vars);
		int t=current_time();
		think(); //think sets the current sky.
		current_time()=t;
		first=i4_T;
	}
}
