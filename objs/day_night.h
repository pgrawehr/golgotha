/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_DAY_NIGHT_HH
#define G1_DAY_NIGHT_HH

#include "g1_object.h"



class g1_mapsingle_class :
	public g1_object_class
{
	i4_bool needsdeleteonnexttick;
public:
	g1_mapsingle_class(g1_object_type id, g1_loader_class *fp);
	virtual void think();
	//singleton objects don't take damage
	virtual void damage(g1_object_class *who_is_hurting,
						int how_much_hurt, i4_3d_vector damage_dir);
	virtual void draw(g1_draw_context_class *context,
					  i4_3d_vector &viewer_position);
};

class g1_day_night_class :
	public g1_mapsingle_class
{
	//position of the sun, in degrees
	i4_float currentpos;
	i4_bool first;
public:
	g1_day_night_class(g1_object_type id, g1_loader_class *fp);
	//all variables are defined in the lisp class, so no specific
	//safe and load code is needed.
	//virtual void save(g1_saver_class *fp);
	//virtual void load(g1_loader_class *fp);
	//virtual void skipload(g1_loader_class *fp);

	virtual void think();
	//void damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir);
	void object_changed_by_editor(g1_object_class *who, li_class *old_values);
	i4_float calc_intensity(i4_bool atday, i4_float pos);
	i4_bool isday(int tick);
};

#endif
