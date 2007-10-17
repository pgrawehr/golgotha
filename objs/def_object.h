/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_DEF_OBJECT_HH
#define G1_DEF_OBJECT_HH

#include "g1_object.h"
#include "lisp/li_types.h"
#include "li_objref.h"
#include "objs/map_piece.h"

class li_class;


struct g1_mini_object_def
{
	i4_3d_vector offset;
	i4_3d_vector position;
	i4_3d_vector rotation;       // contains rotation information
	g1_model_id_type defmodeltype; //model used by this mini object. can be overridden with draw()

	void init()
	{
		memset(this,0,sizeof(* this));
	}
	void assign_to(g1_mini_object *m);
};

enum {
	G1_F_THINK,
	G1_F_DAMAGE,
	G1_F_NOTIFY_DAMAGE,
	G1_F_ENTER_RANGE,
	G1_F_DRAW,
	G1_F_MESSAGE,
	G1_F_DEPLOY_TO,
	G1_F_CHANGE_TEAMS,
	G1_F_OCCUPY_LOCATION,
	G1_F_UNOCCUPY_LOCATION,
	G1_F_CAN_ATTACK,
	G1_F_TOTAL
};


class g1_dynamic_object_type_class :
	public g1_object_definition_class
//public li_gc_object_marker_class
{
public:

	g1_quad_object_class * model;
	i4_array<g1_mini_object_def>  minis;
	w32 obj_flags;
	//because the symbols are always in the global symbol list
	//they should not be garbage collected.
	li_symbol * funs[G1_F_TOTAL];


	g1_dynamic_object_type_class(li_symbol * sym);
	g1_object_class *create_object(g1_object_type type, g1_loader_class * fp);
	//void gc_mark(int set);
};

class g1_movable_dynamic_object_type_class :
	public g1_dynamic_object_type_class
{
public:
	g1_movable_dynamic_object_type_class(li_symbol * sym);
	g1_object_class *create_object(g1_object_type type, g1_loader_class * fp);
};

class g1_dynamic_object_class :
	public g1_object_class
{
	enum {
		VERSION=0xdef1
	};
public:

	static g1_dynamic_object_class *cast(g1_object_class * o)
	{
		if (o && o->get_type()->get_flag(g1_object_definition_class::TO_DYNAMIC_OBJECT))
		{
			return (g1_dynamic_object_class *)o;
		}
		else
		{
			return 0;
		}
	}


	g1_dynamic_object_type_class *get_type() const
	{
		return (g1_dynamic_object_type_class *)g1_object_type_array[id];
	}

	li_class *type_vars()
	{
		return get_type()->vars;
	}

	// called when the object is on a map cell that is drawn
	virtual void draw(g1_draw_context_class * context, i4_3d_vector& viewer_position);
	void think();
	li_object *message(li_symbol * name, li_object * params, li_environment * env);

	void change_player_num(int new_player_num); // don't change player_num directly
	i4_bool occupy_location();
	void unoccupy_location();


	void damage(g1_object_class * obj, int hp, i4_3d_vector damage_dir);
	void notify_damage(g1_object_class * obj, sw32 hp);
	i4_bool deploy_to(float x, float y, g1_path_handle ph);

	g1_dynamic_object_class(g1_object_type type, g1_loader_class * fp);
	void save(g1_saver_class * fp);

	void load(g1_loader_class * fp);
	void skipload(g1_loader_class * fp);
	li_object *value(li_symbol * sym);
	void set(li_symbol * sym, li_object * value);

	static g1_dynamic_object_class *get(li_object * o, li_environment * env)
	{
		return g1_dynamic_object_class::cast(li_g1_ref::get(o,env)->value());
	}
};

class g1_movable_dynamic_object_class :
	public g1_map_piece_class
{
	enum {
		VERSION=0xeef0
	};
public:

	static g1_movable_dynamic_object_class *cast(g1_object_class * o)
	{
		if (o && o->get_type()->get_flag(g1_object_definition_class::TO_MOVABLE_DYNAMIC_OBJECT))
		{
			return (g1_movable_dynamic_object_class *)o;
		}
		else
		{
			return 0;
		}
	}

	g1_movable_dynamic_object_type_class *get_type() const
	{
		return (g1_movable_dynamic_object_type_class *)g1_object_type_array[id];
	}

	li_class *type_vars()
	{
		return get_type()->vars;
	}

	// called when the object is on a map cell that is drawn
	virtual void draw(g1_draw_context_class * context, i4_3d_vector& viewer_position);
	void think();
	i4_bool can_attack(g1_object_class * who);
	li_object *message(li_symbol * name, li_object * params, li_environment * env);

	void change_player_num(int new_player_num); // don't change player_num directly
	i4_bool occupy_location();
	void unoccupy_location();


	void damage(g1_object_class * obj, int hp, i4_3d_vector damage_dir);
	void notify_damage(g1_object_class * obj, sw32 hp);
	i4_bool deploy_to(float x, float y, g1_path_handle ph);

	g1_movable_dynamic_object_class(g1_object_type type, g1_loader_class * fp);
	~g1_movable_dynamic_object_class();
	void save(g1_saver_class * fp);
	void load(g1_loader_class * fp);
	void skipload(g1_loader_class * fp);

	li_object *value(li_symbol * sym);
	void set(li_symbol * sym, li_object * value);

	static g1_movable_dynamic_object_class *get(li_object * o, li_environment * env)
	{
		return g1_movable_dynamic_object_class::cast(li_g1_ref::get(o,env)->value());
	}
};

inline g1_object_class *get_object_param(li_object * o, li_environment * env)
{
	return li_g1_ref::get(li_eval(li_first(o,env),env),env)->value();
}

class g1_draw_context_class;
extern g1_draw_context_class * g1_draw_context;


#endif
