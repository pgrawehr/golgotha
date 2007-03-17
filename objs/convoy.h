/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_CONVOY_HH
#define G1_CONVOY_HH

#include "objs/map_piece.h"

//a convoy is an invisible but selectable object that groups map pieces together
class g1_convoy_class :
	public g1_map_piece_class
{
protected:
	float sizex,sizey;
	float meanx,meany,meanh;    //for precalcing
public:
	enum {
		DATA_VERSION=1
	};
	g1_convoy_class(g1_object_type id, g1_loader_class *fp);
	~g1_convoy_class();
	void save(g1_saver_class *fp);

	i4_bool move(i4_float x_amount,i4_float y_amount);
	virtual void draw(g1_draw_context_class *context, i4_3d_vector& viewer_position);
	i4_bool deploy_to(i4_float x, i4_float y, g1_path_handle ph); //needed to ensure the entire convoy takes the same route
	void fire();
	void think();
	i4_bool can_attack(g1_object_class *who);
	void add_object(g1_object_class *obj);
	void remove_object(g1_object_class *obj);
	w8 mingrade(void);
	void calcsize(void);
	void damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir);
	g1_map_solver_class *prefered_solver();
	void setup(g1_object_class *firstobj);
	short get_single();
};

short single_sel_id(g1_object_class *s);


#endif
