/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_REPAIRER_HH
#define G1_REPAIRER_HH

#include "sound/sound.h"
#include "objs/map_piece.h"
#include "objs/light_o.h"

class g1_repairer_class :
	public g1_map_piece_class
{
protected:
	g1_mini_object *turret,*boom,*tip;
	i4_float repair_angle, repair_length;
	i4_float length;

	i4_bool first, lit, repairing;
	i4_3d_vector target_pos;
	i4_float w1,w2;
	i4_color c1,c2;
	i4_float a1,a2;
	i4_float size;

	g1_typed_reference_class<g1_object_class> target;
	g1_typed_reference_class<g1_light_object_class> light, end_light;

	enum {
		NUM_ARCS=5
	};

	struct arc_point
	{
		i4_3d_vector lposition,position;
	} arc[NUM_ARCS];



	void setup_flare(g1_object_class *target);



	void arc_to(const i4_3d_vector &target);
	void copy_old_points();

	virtual void post_think();

	void create_light();
	void destroy_light();
public:
	g1_repairer_class(g1_object_type id, g1_loader_class *fp);
	virtual void save(g1_saver_class *fp);
	virtual void load(g1_loader_class *fp);
	virtual void skipload(g1_loader_class *fp);
	virtual void draw(g1_draw_context_class *context, i4_3d_vector& viewer_position);

	virtual void setup_flare_look(i4_float _size,
								  i4_float start_width,     i4_float end_width,
								  i4_color start_color,     i4_color end_color,
								  i4_float start_alpha=1.0, i4_float end_alpha=1.0,
								  i4_bool _lit=i4_F)
	{
		size = _size;
		w1 = start_width;
		w2 = end_width;
		c1 = start_color;
		c2 = end_color;
		a1 = start_alpha;
		a2 = end_alpha;
		lit = _lit;
	}

	virtual i4_bool can_attack(g1_object_class *who);
	virtual void think();
	virtual void request_remove();
};

#endif
