/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef SHOCKWAVE_HH
#define SHOCKWAVE_HH

#include "g1_object.h"
#include "player.h"
#include "path.h"
#include "objs/model_draw.h"

class g1_solid_class;
class g1_map_piece_class;

class g1_shockwave_class :
	public g1_object_class
{
protected:
	struct vert_offset
	{
		w32 color;
		sw32 displacement;
	} *vert_map;

	float focal_radius;

	float strength, red_intensity, green_intensity, blue_intensity;
	w16 radius;


	enum {
		DATA_VERSION=1
	};

	void alloc_vert_map();
public:

	virtual void draw(g1_draw_context_class *context, i4_3d_vector& viewer_position);
	virtual void think();

	g1_shockwave_class(g1_object_type id, g1_loader_class *fp);
	virtual void save(g1_saver_class *fp);

	//making these virtual doesn't make much sense as you call this
	//function immediatelly after creation of the object
	void setup(const i4_3d_vector &pos, // starting position (center)
			   float strength=0.5,                           // maximum ground distortion
			   w16 radius=5);                                // maximum radius in game units

	~g1_shockwave_class();

	virtual i4_bool occupy_location();
	virtual void unoccupy_location();
};

class g2_water_wave_class :
	public g1_shockwave_class
{
protected:
	enum {
		WATER_WAVE_DATA_VERSION=1
	};
public:
	virtual void think();
	g2_water_wave_class(g1_object_type id, g1_loader_class *fp);
	virtual void save(g1_saver_class *fp);
	void setup_wave(const i4_3d_vector &pos, float strength, w16 radius);
	//~g2_water_wave_class();
	virtual i4_bool occupy_location();
	virtual void unoccupy_location();

};
#endif
