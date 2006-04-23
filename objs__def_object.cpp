/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "lisp/lisp.h"
#include "objs/miscobjs.h"
#include "g1_object.h"
#include "lisp/li_class.h"
#include "objs/model_id.h"
#include "saver.h"
#include "objs/model_draw.h"
#include "objs/def_object.h"
#include "objs/fire.h"
#include "li_objref.h"
#include "lisp/li_vect.h"
#include "lisp/li_dialog.h"
#include "map_man.h"
#include "map.h"
#include "objs/map_piece.h"
#include "image_man.h"
#include "player.h"

static li_symbol *s_model_name=0, *s_mini_object=0,
  *s_offset=0, *s_position=0, *s_rotation=0,
  *s_object_flags=0, *s_type_flags=0;


static li_symbol *s_ofun[G1_F_TOTAL];


////////////////////  li_def_object setup code ///////////////////////////////////////

// these flag value are assigned ot their symbol names for easy ussage in script files
static w32 g1_oflag_values[]={g1_object_class::THINKING,
                              g1_object_class::SELECTABLE,
                              g1_object_class::TARGETABLE,
                              g1_object_class::GROUND,
                              g1_object_class::UNDERWATER,
                              g1_object_class::AERIAL,
                              g1_object_class::HIT_GROUND,
                              g1_object_class::HIT_UNDERWATER,
                              g1_object_class::HIT_AERIAL,
                              g1_object_class::BLOCKING,
                              g1_object_class::DANGEROUS,
                              g1_object_class::CAN_DRIVE_ON,
							  g1_object_class::SPECIALTARGETS
};

static w32 g1_decoflag_values[]={
		g1_deco_object_class::DECO_NORMAL,
		g1_deco_object_class::DECO_INVULNERABLE,
		g1_deco_object_class::DECO_USEPOLYCOLDET,
		g1_deco_object_class::DECO_GHOST,
		g1_deco_object_class::DECO_HASOWNER
	};


static char *g1_oflag_names[]={"thinking",
                               "selectable",
                               "targetable",
                               "ground",
                               "underwater",
                               "aerial",
                               "hit_ground",
                               "hit_underwater",
                               "hit_aerial",
                               "blocking",
                               "dangerous",
                               "can_drive_on",
							   "specialtargets",
                               0};

static char *g1_decoflag_names[]={
	"deco_none",
	"deco_invulnerable",
	"deco_usepolycoldet",
	"deco_ghost",
	"deco_hasowner",
	0};


	static char *g1_ofun_names[G1_F_TOTAL]={"think",
		"damage",
		"notify_damage",
		"enter_range",
		"draw",
		"message",
		"deploy_to",
		"change_player_num",
		"occupy_location",
		"unoccupy_location", 
		"can_attack"  //only sensefull for map-pieces
		};

li_object *li_def_object(li_object *o, li_environment *env);
li_object *li_def_movable_object(li_object *o, li_environment *env);
li_object *li_def_buildings(li_object *o, li_environment *env)
{
  for (; o; o=li_cdr(o,env))
	  {
	  li_object *obj=li_car(o,env);
	  char *name=0;
	  w32 deco_flags=0;
	  if (obj->type()==LI_STRING)
		  name=li_string::get(obj,env)->value();
	  else
		  {
		  name=li_string::get(li_car(obj,env),env)->value();
		  li_object *next=li_cdr(obj,env);
		  while (next&&next!=li_nil)
			  {
			  deco_flags|=li_int::get(li_eval(li_car(next,env),env),env)->value();
			  next=li_cdr(next,env);
			  }
		  }
      g1_create_deco_object(name,deco_flags);
	  }
  return 0;
}

class g1_create_object_init : public i4_init_class
{
public:  
  void init()
  {
    int i=0;
    for (char **a=g1_oflag_names; *a; a++)
      li_set_value(*a, new li_int(g1_oflag_values[i++]));

	i=0;
	for (char **b=g1_decoflag_names; *b; b++)
		li_set_value(*b,new li_int(g1_decoflag_values[i++]));

    for (i=0; i<G1_F_TOTAL; i++)
      s_ofun[i]=li_get_symbol(g1_ofun_names[i]);
    
    li_set_value("editor_selectable", new li_int(g1_object_definition_class::EDITOR_SELECTABLE));
    li_set_value("movable", new li_int(g1_object_definition_class::MOVABLE));

    li_add_function("def_object", li_def_object);
    li_add_function("def_buildings", li_def_buildings);
	li_add_function("def_movable_object", li_def_movable_object);
	li_set_value("null_object_ref",li_g1_null_ref(),0);
	
  }

  //this init type requires to be called after the LISP_FUNCTIONS since
  //otherwise li_g1_null_ref() is not yet valid.
  int init_type() {return I4_INIT_TYPE_STRING_MANAGER;}
} g1_create_object_init_instance;



struct g1_object_def_struct
{
  char *name;  
  char *model_name;
  i4_array<g1_mini_object_def> *minis;
  li_type_number var_class;
  w32 flags;
  
  g1_object_def_struct()
  {
    name=0;
    minis=0;
    var_class=0;
    flags=0;
  }  
} ;



/////////////////////////////  Dynamic object code ///////////////////////////////

void g1_mini_object_def::assign_to(g1_mini_object *m)
{
  m->offset=offset;
  m->x=m->lx=position.x;
  m->y=m->ly=position.y;
  m->h=m->lh=position.z;

  m->rotation=m->lrotation=rotation;
  m->defmodeltype=defmodeltype;
  m->frame=0;
  m->animation=0;
}

g1_movable_dynamic_object_type_class::g1_movable_dynamic_object_type_class(li_symbol *sym)
:g1_dynamic_object_type_class(sym)
	{
	obj_flags=g1_object_class::CAN_DRIVE_ON;
	var_class=0;
	flags |=TO_MOVABLE_DYNAMIC_OBJECT|TO_MAP_PIECE;
	flags &= ~TO_DYNAMIC_OBJECT; //the type class inherits from there
	//but the object class does not.
	}
g1_dynamic_object_type_class::g1_dynamic_object_type_class(li_symbol *sym)
  : g1_object_definition_class(sym->name()->value()),
    minis(0,1)

{
  //obj_flags=g1_object_class::CAN_DRIVE_ON;
  obj_flags=0;
  var_class=0;
  flags |=  TO_DYNAMIC_OBJECT;

  memset(&funs, 0, sizeof(funs));
}


li_object *g1_dynamic_object_class::message(li_symbol *name, li_object *params, li_environment *env)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_MESSAGE])
  {
    li_ext_class_context context(vars,env,this);
    li_g1_ref _me(global_id);
      
    li_list *p=(li_list *)params;
    li_push(p, name);
    li_push(p, &_me);

    //return t->funs[G1_F_MESSAGE](p , env);
	return li_call(t->funs[G1_F_MESSAGE],p,env);
  }
  return g1_object_class::message(name,params,env);
}

li_object *g1_movable_dynamic_object_class::message(li_symbol *name, li_object *params, li_environment *env)
{
  g1_movable_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_MESSAGE])
	  {
    li_ext_class_context context(vars,env,this);
    li_g1_ref _me(global_id);
      
    li_list *p=(li_list *)params;
    li_push(p, name);
    li_push(p, &_me);

    //return t->funs[G1_F_MESSAGE](p , env);
	return li_call(t->funs[G1_F_MESSAGE],p,env);
	  }
  return g1_map_piece_class::message(name,params,env);
}

g1_draw_context_class *g1_draw_context;

// called when the object is on a map cell that is drawn
void g1_dynamic_object_class::draw(g1_draw_context_class *context)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_DRAW])
  {
    g1_draw_context=context;
	li_environment *myenv=0;
    li_ext_class_context context(vars,myenv,this);
    li_g1_ref _me(global_id);
    //t->funs[G1_F_DRAW](li_make_list(&_me,0),0);
	li_call(t->funs[G1_F_DRAW],li_make_list(&_me,0),myenv);
  }
  else
  {
    g1_model_draw_parameters mp;
    mp.model=get_type()->model;
    g1_model_draw(this, mp, context);
  }
}

void g1_movable_dynamic_object_class::draw(g1_draw_context_class *context)
{
  g1_movable_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_DRAW])
  {
    g1_draw_context=context;
	li_environment *env=0;
    li_ext_class_context context(vars,env,this);
    li_g1_ref _me(global_id);
    //t->funs[G1_F_DRAW](li_make_list(&_me,0),0);
	li_call(t->funs[G1_F_DRAW],li_make_list(&_me,0),env);
  }
  else
  {
    //g1_model_draw_parameters mp;
    //mp.model=get_type()->model;
    //g1_model_draw(this, mp, context);
    g1_map_piece_class::draw(context);
  }
}


// void g1_dynamic_object_class::note_enter_range(g1_object_class *who, g1_fire_range_type range)
// {
//   g1_dynamic_object_type_class *t=get_type();
//   if (t->funs[G1_F_ENTER_RANGE])
//   {
//     li_class_context context(vars);
//     t->funs[G1_F_ENTER_RANGE](li_make_list(new li_g1_ref(global_id),
//                                            new li_g1_ref(who->global_id),
//                                            new li_int(range), 0), 0);
//   }
// }

void g1_dynamic_object_class::damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_DAMAGE])
  {
    li_environment *env=0;
    li_ext_class_context context(vars,env,this);    
    (*t->funs[G1_F_DAMAGE])(li_make_list(new li_g1_ref(global_id),
                                      new li_g1_ref(obj->global_id),
                                      new li_int(hp),
                                      new li_vect(damage_dir), 0),env);
  }
  else
	  {
      request_think();
	  };
}

void g1_movable_dynamic_object_class::damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_DAMAGE])
  {
    li_environment *env=0;
    li_ext_class_context context(vars,env,this);    
    (*t->funs[G1_F_DAMAGE])(li_make_list(new li_g1_ref(global_id),
                                      new li_g1_ref(obj->global_id),
                                      new li_int(hp),
                                      new li_vect(damage_dir), 0),env);
  }
  else
	  {
	  g1_map_piece_class::damage(obj,hp,damage_dir);
      request_think();
	  };
}

void g1_dynamic_object_class::notify_damage(g1_object_class *obj, sw32 hp)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_NOTIFY_DAMAGE])
  {
    li_environment *env=0;
    li_ext_class_context context(vars,env,this);    
	//li_symbol has an overwritten operator()
    (*t->funs[G1_F_NOTIFY_DAMAGE])(li_make_list(new li_g1_ref(global_id),
                                             new li_g1_ref(obj->global_id),
                                             new li_int(hp),
                                             0),env);
  }
  else
    g1_object_class::notify_damage(obj, hp);
}

void g1_movable_dynamic_object_class::notify_damage(g1_object_class *obj, sw32 hp)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_NOTIFY_DAMAGE])
  {
    li_environment *env=0;
    li_ext_class_context context(vars,env,this);    
	//li_symbol has an overwritten operator()
    (*t->funs[G1_F_NOTIFY_DAMAGE])(li_make_list(new li_g1_ref(global_id),
                                             new li_g1_ref(obj->global_id),
                                             new li_int(hp),
                                             0),env);
  }
  else
    g1_map_piece_class::notify_damage(obj, hp);
}

void g1_dynamic_object_class::think()
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_THINK])
  {
    li_environment *myenv=0;
    li_ext_class_context context(vars,myenv,this,i4_T);   
    (*t->funs[G1_F_THINK])(li_make_list(new li_g1_ref(global_id), 0), myenv);
  }
}

void g1_movable_dynamic_object_class::think()
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_THINK])
  {
    li_environment *myenv=0;
    li_ext_class_context context(vars,myenv,this,i4_T);   
	

    (*t->funs[G1_F_THINK])(li_make_list(new li_g1_ref(global_id), 0), myenv);
  }
  else
	  g1_map_piece_class::think();
}
static li_float_class_member dest_x("dest_x"), dest_y("dest_y");

i4_bool g1_dynamic_object_class::deploy_to(float x, float y, g1_path_handle ph)
{
  g1_dynamic_object_type_class *t=get_type();
  li_environment *env=0;
  li_ext_class_context context(vars,env,this);

  if (t->funs[G1_F_DEPLOY_TO])
    (*t->funs[G1_F_DEPLOY_TO])(li_make_list(new li_g1_ref(global_id),
                                         new li_float(x),
                                         new li_float(y),
										 new li_int(ph==0?0:1),0), env);
  else
  {
    if (vars && vars->member_offset(dest_x)!=-1)
	{
      dest_x()=x;
      dest_y()=y;
    }
  }
  //should set ph but dynamic object are not derived from map pieces.
  g1_path_manager.free_path(ph);
  return 0;
}

i4_bool g1_movable_dynamic_object_class::deploy_to(float x, float y, g1_path_handle ph)
{
  g1_dynamic_object_type_class *t=get_type();
  li_environment *env=0;
  li_ext_class_context context(vars,env,this);

  if (t->funs[G1_F_DEPLOY_TO])
    (*t->funs[G1_F_DEPLOY_TO])(li_make_list(new li_g1_ref(global_id),
                                         new li_float(x),
                                         new li_float(y),
										 new li_int(ph==0?0:1),0), env);
  else
  {
	g1_map_piece_class::deploy_to(x,y,ph);
  }
  
  return 0;
}

i4_bool g1_movable_dynamic_object_class::can_attack(g1_object_class *who)
	{
	g1_dynamic_object_type_class *t=get_type();
	if (t->funs[G1_F_CAN_ATTACK])
		{
		li_environment *env=0;
		li_ext_class_context context(vars,env,this);
		li_object *ret=0;
		ret=(*t->funs[G1_F_CAN_ATTACK])(li_make_list(new li_g1_ref(global_id),
			new li_int(flags),new li_int(player_num),
			new li_g1_ref(who),new li_int(who->flags),
			new li_int(who->player_num),0),0);
		if (ret==li_true_sym)
			return i4_T;
		else
			return i4_F;
		}
	else
		return g1_map_piece_class::can_attack(who);
	}

void g1_dynamic_object_class::change_player_num(int new_player_num)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_CHANGE_TEAMS])
  {
    li_class_context context(vars);
    (*t->funs[G1_F_CHANGE_TEAMS])(li_make_list(new li_g1_ref(global_id),
                                            new li_int(new_player_num), 0), 0);
  }
  else
    g1_object_class::change_player_num(new_player_num);  
}

void g1_movable_dynamic_object_class::change_player_num(int new_player_num)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_CHANGE_TEAMS])
  {
    li_class_context context(vars);
    (*t->funs[G1_F_CHANGE_TEAMS])(li_make_list(new li_g1_ref(global_id),
                                            new li_int(new_player_num), 0), 0);
  }
  else
    g1_map_piece_class::change_player_num(new_player_num);  
}

i4_bool g1_dynamic_object_class::occupy_location()
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_OCCUPY_LOCATION])
  {
    li_environment *env=0;
    li_ext_class_context context(vars,env,this);

    if ((*t->funs[G1_F_OCCUPY_LOCATION])(li_make_list(new li_g1_ref(global_id),0), env))
      return i4_T;
    else return i4_F;
  }
  else
    return g1_object_class::occupy_location();  
}

i4_bool g1_movable_dynamic_object_class::occupy_location()
	{
	//not yet sure wheter everything is ok here. 
	//Is there a case where we would need the environment here
	//because a lisp function is in the call stack?
	g1_dynamic_object_type_class *t=get_type();
	li_environment *env=0;
	li_ext_class_context context(vars,env,this);
	if (t->funs[G1_F_OCCUPY_LOCATION])
		{
		
		if ((*t->funs[G1_F_OCCUPY_LOCATION])(li_make_list(new li_g1_ref(global_id),0), env))
			return i4_T;
		else return i4_F;
		}
	else
		{
		/*
		float xp=li_float::get(li_get_value("xpos",env),env)->value();
		float yp=li_float::get(li_get_value("ypos",env),env)->value();
		float hp=li_float::get(li_get_value("hpos",env),env)->value();
		mp->x=xp;
		mp->y=yp;
		mp->h=hp;
		*/
		return g1_map_piece_class::occupy_location();  
		}
	}


void g1_dynamic_object_class::unoccupy_location()
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_UNOCCUPY_LOCATION])
  {
    li_environment *env=0;
    li_ext_class_context context(vars,env,this);
    (*t->funs[G1_F_UNOCCUPY_LOCATION])(li_make_list(new li_g1_ref(global_id), 0), env);
  }
  else
    g1_object_class::unoccupy_location();  
}

void g1_movable_dynamic_object_class::unoccupy_location()
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_UNOCCUPY_LOCATION])
  {
    li_environment *env=0;
    li_ext_class_context context(vars,env,this);
    (*t->funs[G1_F_UNOCCUPY_LOCATION])(li_make_list(new li_g1_ref(global_id), 0), env);
  }
  else
    g1_map_piece_class::unoccupy_location();  
}

g1_dynamic_object_class::g1_dynamic_object_class(g1_object_type type, g1_loader_class *fp)
  : g1_object_class(type, fp)

{
  g1_dynamic_object_type_class *dtype=get_type();
       
  if (dtype->minis.size())
  {
    allocate_mini_objects(dtype->minis.size(), "");
    for (int i=0; i<dtype->minis.size(); i++)
      dtype->minis[i].assign_to(mini_objects+i);     
  }


  if (fp && fp->read_16()==VERSION)
  {
    int old_minis=fp->read_8(), i;
    int old_mini_disk_size=fp->read_32();
      
    if (old_minis==dtype->minis.size())
    {
      g1_mini_object *m=mini_objects;

      for (i=0; i<dtype->minis.size(); i++)
      {
        fp->read_format("fffffffff", 
                        &m->offset.x, &m->offset.y, &m->offset.z,
                        &m->x, &m->y, &m->h,
                        &m->rotation.x, &m->rotation.y, &m->rotation.z);
      }
    }
    else
      fp->seek(fp->tell() + old_mini_disk_size);
  }


  flags = get_type()->obj_flags;

  grab_old();
}

g1_movable_dynamic_object_class::g1_movable_dynamic_object_class(g1_object_type type, g1_loader_class *fp)
  : g1_map_piece_class(type, fp)
{
  g1_movable_dynamic_object_type_class *dtype=get_type();
  radar_type=G1_RADAR_VEHICLE;
  i4_str rname(dtype->name());
  rname="bitmaps/radar/"+rname+".tga";
  g1_team_icon_ref* ref=new g1_team_icon_ref(rname.c_str());
  ref->load();//will be the default image if load fails.
  radar_image=ref;
  if (dtype->minis.size())
  {
    allocate_mini_objects(dtype->minis.size(), "");
    for (int i=0; i<dtype->minis.size(); i++)
      dtype->minis[i].assign_to(mini_objects+i);     
  }
  draw_params.setup(dtype->model);
  if (fp && fp->read_16()==VERSION)
  {
    int old_minis=fp->read_8(), i;
    int old_mini_disk_size=fp->read_32();
      
    if (old_minis==dtype->minis.size())
    {
      g1_mini_object *m=mini_objects;

      for (i=0; i<dtype->minis.size(); i++)
      {
        fp->read_format("fffffffff", 
                        &m->offset.x, &m->offset.y, &m->offset.z,
                        &m->x, &m->y, &m->h,
                        &m->rotation.x, &m->rotation.y, &m->rotation.z);
      }
    }
    else
      fp->seek(fp->tell() + old_mini_disk_size);
  }

  flags = get_type()->obj_flags;

  grab_old();
}

g1_movable_dynamic_object_class::~g1_movable_dynamic_object_class()
	{
	delete radar_image;//must manually do this here.
	radar_image=0;
	}

void g1_dynamic_object_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);

  fp->write_16(VERSION);

  fp->write_8((w8)num_mini_objects);
  int han=fp->mark_size();

  g1_mini_object *m=mini_objects;
  for (int i=0; i<num_mini_objects; i++)
  {      
    fp->write_format("fffffffff", 
                     &m->offset.x, &m->offset.y, &m->offset.z,
                     &m->x, &m->y, &m->h,
                     &m->rotation.x, &m->rotation.y, &m->rotation.z);
	m=mini_objects+i;
  }

  fp->end_mark_size(han);
}

void g1_movable_dynamic_object_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  fp->write_16(VERSION);

  fp->write_8((w8)num_mini_objects);
  int han=fp->mark_size();

  g1_mini_object *m=mini_objects;
  for (int i=0; i<num_mini_objects; i++)
  {      
    fp->write_format("fffffffff", 
                     &m->offset.x, &m->offset.y, &m->offset.z,
                     &m->x, &m->y, &m->h,
                     &m->rotation.x, &m->rotation.y, &m->rotation.z);
	m=mini_objects+i;
  }

  fp->end_mark_size(han);
}

void g1_dynamic_object_class::load(g1_loader_class *fp)
	{
	g1_object_class::load(fp);
	fp->read_16();
	if (num_mini_objects!=fp->read_8())
		{
		i4_warning("Cannot synchronize two different objects");
		w32 s=fp->read_32();
		fp->seek(fp->tell()+s);
		return;
		}
	fp->read_32();
    g1_mini_object *m=mini_objects;
	for (int i=0; i<num_mini_objects; i++)
		{      
		fp->read_format("fffffffff", 
                     &m->offset.x, &m->offset.y, &m->offset.z,
                     &m->x, &m->y, &m->h,
                     &m->rotation.x, &m->rotation.y, &m->rotation.z);
		m=mini_objects+i;
		}
	}

void g1_movable_dynamic_object_class::load(g1_loader_class *fp)
	{
	g1_map_piece_class::load(fp);
	fp->read_16();
	if (num_mini_objects!=fp->read_8())
		{
		i4_warning("Cannot synchronize two different objects");
		w32 s=fp->read_32();
		fp->seek(fp->tell()+s);
		return;
		}
	fp->read_32();
    g1_mini_object *m=mini_objects;
	for (int i=0; i<num_mini_objects; i++)
		{      
		fp->read_format("fffffffff", 
                     &m->offset.x, &m->offset.y, &m->offset.z,
                     &m->x, &m->y, &m->h,
                     &m->rotation.x, &m->rotation.y, &m->rotation.z);
		m=mini_objects+i;
		}
	}

void g1_movable_dynamic_object_class::skipload(g1_loader_class *fp)
	{
	g1_map_piece_class::skipload(fp);
	fp->read_16();
	w32 s=fp->read_32();
	fp->seek(fp->tell()+s);	
	}

void g1_dynamic_object_class::skipload(g1_loader_class *fp)
	{
	g1_object_class::skipload(fp);
	fp->read_16();
	w32 s=fp->read_32();
	fp->seek(fp->tell()+s);	
	}


//////////////////  dynamic object type functions ///////////////////////////////

g1_object_class *g1_dynamic_object_type_class::create_object(g1_object_type type, 
                                                             g1_loader_class *fp)
{
  g1_object_class *o=new g1_dynamic_object_class(type, fp);   
  o->init();
  return o;
}

g1_object_class *g1_movable_dynamic_object_type_class::create_object(
			g1_object_type type, g1_loader_class *fp)
	{
	g1_object_class *o=new g1_movable_dynamic_object_class(type, fp);
	o->init();
	return o;
	}

static void read_vect(i4_3d_vector &v, li_object *o, li_environment *env)
{
  v.x=(float)li_get_float(li_eval(li_car(o,env), env),env);  o=li_cdr(o,env);
  v.y=(float)li_get_float(li_eval(li_car(o,env), env),env);  o=li_cdr(o,env);
  v.z=(float)li_get_float(li_eval(li_car(o,env), env),env);
}
//We cannot assume that a function has type "li_function"
//it could also have "li_user_function"
/*
static li_function_type get_function(li_object *o, li_environment *env)
{
  li_object *fun=li_get_fun(li_symbol::get(o,env),env);
  if (!fun)
    li_error(env, "USER: no registered function %O", o);

  return li_function::get(fun,env)->value();
}
*/

li_object *li_def_object(li_object *o, li_environment *env)
{
  li_symbol *sym=li_symbol::get(li_car(o,env),env);
  

  g1_dynamic_object_type_class *type=new g1_dynamic_object_type_class(sym); 


  for (li_object *l=li_cdr(o,env); l; l=li_cdr(l,env))
  {
    li_object *prop=li_car(l,env);
    li_symbol *sym=li_symbol::get(li_car(prop,env),env);
    prop=li_cdr(prop,env);

    if (sym==li_get_symbol("model_name", s_model_name))
    {
      int id=g1_model_list_man.find_handle(li_string::get(li_car(prop,env),env)->value());
      type->model=g1_model_list_man.get_model(id);
    }
    else if (sym==li_get_symbol("mini_object", s_mini_object))
    {
      g1_mini_object_def mo;
      mo.init();
          
      char *name=0;
      for (;prop; prop=li_cdr(prop,env)) // prop = ((offset 0 0 0.1) (position 0 0 0) (model_name "gunport_barrel"))
      {
        li_object *sub=li_car(prop,env);
        sym=li_symbol::get(li_car(sub,env),env); sub=li_cdr(sub,env);


        if (sym==li_get_symbol("offset", s_offset))
          read_vect(mo.offset, sub, env);
        else if (sym==li_get_symbol("position", s_position))
          read_vect(mo.position, sub, env);
        else if (sym==li_get_symbol("rotation", s_rotation))
          read_vect(mo.rotation, sub, env);
        else if (sym==li_get_symbol("model_name", s_model_name))
        {
          char *n=li_string::get(li_eval(li_car(sub,env), env),env)->value();
          mo.defmodeltype=g1_model_list_man.find_handle(n);
        }
        else li_error(env,"USER: %O should be (offset/position/rotation x y z)",sym);
      }

      type->minis.add(mo);

    }
    else if (sym==li_get_symbol("object_flags", s_object_flags))
    {
      for (;prop; prop=li_cdr(prop,env))
        type->obj_flags |= li_int::get(li_eval(li_car(prop,env),env),env)->value();
    }
    else if (sym==li_get_symbol("type_flags", s_type_flags))
    {
      for (;prop; prop=li_cdr(prop,env))
        type->flags |= li_int::get(li_eval(li_car(prop,env),env),env)->value();
    }
    else
    {
      int found=0;
      for (int i=0; i<G1_F_TOTAL-1; i++)//cannot use can_attack with non-map-piece objs
        if (sym==s_ofun[i])
        {
          //type->funs[i]=li_get_fun(li_car(prop,env),env);
		  type->funs[i]=li_symbol::get(li_car(prop,env),env);//need to store only the symbol
          found=1;
        }

      if (!found)
        li_error(env,"USER: unknown object property %O", sym);
    }      
  }   

  // if functions were not filled in see if we can find defaults
  for (int i=0; i<G1_F_TOTAL; i++)
    if (type->funs[i]==0)
    {
      char buf[200];
      sprintf(buf, "%s_%s", type->name(), g1_ofun_names[i]);
      
      li_symbol *sym=li_find_symbol(buf);
      if (sym)
      {
	      type->funs[i]=sym;
        //li_object *fun=li_function::get(sym->fun(), env);
        //if (fun)
        //  type->funs[i]=li_function::get(fun, env)->value();
		//type->funs[i]=fun;
      }
    }


  type->flags|=g1_object_definition_class::DELETE_WITH_LEVEL;
 
  return 0;
}

LI_HEADER(decrease_fire_delay)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp&&mp->fire_delay>0)
		mp->fire_delay--;
	return li_true_sym;
	}

LI_HEADER(object_flags)
	{
	g1_object_class *c=get_object_param(o,env);
	return new li_int(c->flags);
	}

LI_HEADER(object_owner)
	{
	g1_object_class *c=get_object_param(o,env);
	return new li_int(c->player_num);
	}

LI_HEADER(player_variables)
	{
	int x=li_get_int(li_eval(li_first(o,env),env),env);
	return g1_player_man.get(x)->vars.get();
	};

LI_HEADER(object_pos)
	{
	g1_object_class *c=get_object_param(o,env);
	return new li_vect(i4_3d_vector(c->x,c->y,c->h));
	}

LI_HEADER(object_orientation)
	{
	g1_object_class *c=get_object_param(o,env);
	return new li_vect(i4_3d_vector(c->theta,c->pitch,c->roll));
	}

LI_HEADER(vector_element)
	{
	i4_3d_vector v=li_vect::get(li_eval(li_second(o,env),env),env)->value();
	li_object *elem=li_eval(li_first(o,env),env);
	if (elem->type()==LI_INT)
		{
		int i=li_int::get(elem,env)->value();
		if (i==0)
			return new li_float(v.x);
		else if (i==1)
			return new li_float(v.y);
		else if(i==2)
			return new li_float(v.z);
		li_error(env,"USER: (vector_element int vect) A vector has only 3 elements!");
		return 0;
		}
	if (elem->type()==LI_SYMBOL)
		{
		if (elem==li_get_symbol("x"))
			{
			return new li_float(v.x);
			}
		if (elem==li_get_symbol("y"))
			return new li_float(v.y);
		if (elem==li_get_symbol("z"))
			return new li_float(v.z);
		if (elem==li_get_symbol("h"))
			return new li_float(v.z);//'h is synonym for z, 
						//use what's apropriate for context
		}
	li_error(env,"USER: (vector_element symbol vect) Unknown second operand %O.",elem);
	return 0;
	}

LI_HEADER(objects_in_range)
	{
	g1_object_class *c=get_object_param(o,env);
	float range=li_float::get(li_eval(li_second(o,env),env),env)->value();
	const int arrsize=1024;
	w32 object_mask_flags=0xffffffff;
	w32 type_mask_flags=0xffffffff;
    g1_object_class *array[arrsize];
	if (li_third(o,env))
		{
		object_mask_flags=li_int::get(li_eval(li_third(o,env),env),env)->value();
		if (li_fourth(o,env))
			{
			type_mask_flags=li_int::get(li_eval(li_fourth(o,env),env),env)->value();
			}
		}
	int num=g1_get_map()->get_objects_in_range(c->x,c->y,range,array,arrsize,
		object_mask_flags,type_mask_flags);

	li_object *list=0;
	for (int i=0;i<num;i++)
		{
		list=new li_list(new li_g1_ref(array[i]),list);
		}
	return list;
	}
//todo: the can_attack function should be overridable.
LI_HEADER(find_targets)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		if (mp->attack_target.valid())
			return new li_g1_ref(mp->attack_target.get());
		mp->find_target();
		if (mp->attack_target.valid())
			return new li_g1_ref(mp->attack_target.get());
		}
	return li_nil;
	}

li_object *li_g1_fire(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		if (mp->fire_delay>0)
			return li_nil;
		g1_object_class *target=li_g1_ref::get(li_eval(li_second(o,env),env),env)->value();
		if (target==0)
			{
			target=mp->attack_target.get();
			}
		li_vect *pos=li_vect::get(li_eval(li_third(o,env),env),env);
		li_vect *dir=li_vect::get(li_eval(li_fourth(o,env),env),env);
		
		mp->fire_delay = mp->defaults->fire_delay;
    
		i4_transform_class main ;// , l2w;
		//turret->calc_transform(1.0, &t);   //local transform from tank to turret 
		
		mp->calc_world_transform(1.0, &main);  //transform from world to tank
		//l2w.mult_translate(main, pos->value());  //transform from world to turret
		//l2w.transform(i4_3d_point_class(0.4f, 0, 0.11f), pos); //movement allong turret?
		i4_3d_vector glob_pos,glob_dir;
		glob_pos=pos->value()+i4_3d_vector(mp->x,mp->y,mp->h);
		//glob_dir=dir->value();
		main.transform_3x3(dir->value(),glob_dir);
    
		li_symbol *weapon=li_symbol::get(li_eval(li_fifth(o,env),env),env);
		if (weapon==0 || weapon==li_nil)
			weapon=mp->defaults->fire_type;
		g1_fire(weapon, mp, target, glob_pos, glob_dir);
		return li_true_sym;
		}
	return li_nil;
	}

LI_HEADER(create_object)
	{
	li_symbol *name=li_symbol::get(li_eval(li_first(o,env),env),env);
	char *objname=name->name()->value();
	li_vect *pos=li_vect::get(li_eval(li_second(o,env),env),env);
	li_vect *orientation=li_vect::get(li_eval(li_third(o,env),env),env);
	g1_object_class *newobj;
	newobj=g1_create_object(g1_get_object_type(objname));
	newobj->x=pos->value().x;
	newobj->y=pos->value().y;
	newobj->h=pos->value().z;
	newobj->theta=orientation->value().x;
	newobj->pitch=orientation->value().y;
	newobj->roll=orientation->value().z;
	newobj->grab_old();
	newobj->occupy_location();
	return new li_g1_ref(newobj);
	}

void update_local_values(g1_map_piece_class *for_obj, li_environment *n_env)
	{
	li_set_value("xpos",new li_float(for_obj->x),n_env);
	li_set_value("ypos",new li_float(for_obj->y),n_env);
	li_set_value("hpos",new li_float(for_obj->h),n_env);
	li_set_value("theta",new li_float(for_obj->theta),n_env);
	li_set_value("pitch",new li_float(for_obj->pitch),n_env);
	li_set_value("roll",new li_float(for_obj->roll),n_env);
	li_set_value("health",new li_int(for_obj->health),n_env);
	li_set_value("speed",new li_float(for_obj->speed),n_env);
	li_set_value("attack_target",new li_g1_ref(for_obj->attack_target.get()),n_env);
	};

li_object *li_g1_map_piece_think(li_object *o, li_environment *env)
	{
	//g1_object_class *c=li_g1_ref::get(li_first(o,env),env)->value();
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		mp->g1_map_piece_class::think();//not the current one, since
		update_local_values(mp,env);//this can change the values we
		//are calculating with (and we don't want to write back the old
		//ones after thinking)
		}
	//this will make up the parent call. 
	return 0;
	}

li_object *li_g1_map_piece_damage(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		g1_object_class *other=li_g1_ref::get(li_eval(li_second(o,env),env),env)->value();
		int hp=li_int::get(li_eval(li_third(o,env),env),env)->value();
		li_vect *damage_dir=li_vect::get(li_eval(li_fourth(o,env),env),env);
		mp->g1_map_piece_class::damage(other,hp,damage_dir->value());//not the current one, since
		update_local_values(mp,env);//this can change the values we
		//are calculating with (and we don't want to write back the old
		//ones after thinking)
		}
	return 0;
	}

li_object *li_g1_map_piece_notify_damage(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		g1_object_class *other=li_g1_ref::get(li_eval(li_second(o,env),env),env)->value();
		int hp=li_int::get(li_eval(li_third(o,env),env),env)->value();
		mp->g1_map_piece_class::notify_damage(other,hp);//not the current one, since
		update_local_values(mp,env);
		}
	return 0;
	}

li_object *li_g1_map_piece_message(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		li_symbol *msg=li_symbol::get(li_eval(li_second(o,env),env),env);
		li_object *msg_param=li_eval(li_third(o,env),env);
		mp->g1_map_piece_class::message(msg,msg_param,env);//not the current one, since
		update_local_values(mp,env);
		}
	return 0;
	}

li_object *li_g1_map_piece_deploy_to(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		float x=li_float::get(li_eval(li_second(o,env),env),env)->value();
		float y=li_float::get(li_eval(li_third(o,env),env),env)->value();
		mp->g1_map_piece_class::deploy_to(x,y,0);//not the current one, since
		update_local_values(mp,env);
		}
	return 0;
	}

li_object *li_g1_map_piece_change_player_num(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		int np=li_int::get(li_eval(li_second(o,env),env),env)->value();
		mp->g1_map_piece_class::change_player_num(np);//not the current one, since
		update_local_values(mp,env);
		}
	return 0;
	}

li_object *li_g1_map_piece_can_attack(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		g1_object_class *who=li_g1_ref::get(li_eval(li_fourth(o,env),env),env)->value();
		mp->g1_map_piece_class::can_attack(who);//not the current one, since
		update_local_values(mp,env);
		}
	return 0;
	}

li_object *li_g1_map_piece_occupy(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		{
		double xp=li_float::get(li_get_value("xpos",env),env)->value();
		double yp=li_float::get(li_get_value("ypos",env),env)->value();
		double hp=li_float::get(li_get_value("hpos",env),env)->value();
      	mp->x=(float)xp;
	    mp->y=(float)yp;
	    mp->h=(float)hp;
		if (mp->g1_map_piece_class::occupy_location())//not the current one, since
			{
			update_local_values(mp,env);//at least pitch and roll could be modified
			return li_true_sym;
			}
		}
	//this will make up the parent call. 
	return 0;
	}

li_object *li_g1_map_piece_unoccupy(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(c);
	if (mp)
		mp->g1_map_piece_class::unoccupy_location();//not the current one, since
	//this will make up the parent call. 
	return 0;
	}

li_object *li_g1_sync(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	float xp=(float)li_float::get(li_get_value("xpos",env),env)->value();
	float yp=(float)li_float::get(li_get_value("ypos",env),env)->value();
	float hp=(float)li_float::get(li_get_value("hpos",env),env)->value();
    c->x=xp;
	c->y=yp;
	c->h=hp;
	return 0;
	}

LI_HEADER(request_think)
	{
	g1_object_class *c=get_object_param(o,env);
	c->request_think();
	return 0;
	}

LI_HEADER(request_remove)
	{
	g1_object_class *c=get_object_param(o,env);
	c->request_remove();
	return 0;
	}

LI_HEADER(max_health)
	{
	g1_object_class *c=get_object_param(o,env);
	return new li_int(c->get_max_health());
	}

LI_HEADER(local_player)
	{
	return new li_int(g1_player_man.local_player);
	};


void init_vars_function(g1_object_definition_class *obj)
	{
	g1_movable_dynamic_object_type_class *type=(g1_movable_dynamic_object_type_class*)obj;
	if (type->vars)
		{
		li_class *cl=type->vars;
		//this solution does not update the functions of objects which are not redefined
		li_object *fn=new li_function(li_g1_map_piece_think);
		cl->set_value(cl->member_offset("think"),fn);
		cl->set_value(cl->member_offset("think_parent"),fn);
		fn=new li_function(li_g1_map_piece_occupy);
		cl->set_value(cl->member_offset("occupy_location"),fn);
		cl->set_value(cl->member_offset("occupy_location_parent"),fn);
		fn=new li_function(li_g1_map_piece_unoccupy);
		cl->set_value(cl->member_offset("unoccupy_location"),fn);
		cl->set_value(cl->member_offset("unoccupy_location_parent"),fn);
		fn=new li_function(li_g1_map_piece_damage);
		cl->set_value(cl->member_offset("damage"),fn);
		cl->set_value(cl->member_offset("damage_parent"),fn);
		fn=new li_function(li_g1_map_piece_notify_damage);
		cl->set_value(cl->member_offset("notify_damage"),fn);
		cl->set_value(cl->member_offset("notify_damage_parent"),fn);
		fn=new li_function(li_g1_map_piece_message);
		cl->set_value(cl->member_offset("message"),fn);
		cl->set_value(cl->member_offset("message_parent"),fn);
		fn=new li_function(li_g1_map_piece_deploy_to);
		cl->set_value(cl->member_offset("deploy_to"),fn);
		cl->set_value(cl->member_offset("deploy_to_parent"),fn);
		fn=new li_function(li_g1_map_piece_change_player_num);
		cl->set_value(cl->member_offset("change_player_num"),fn);
		cl->set_value(cl->member_offset("change_player_num_parent"),fn);
		fn=new li_function(li_g1_map_piece_can_attack);
		cl->set_value(cl->member_offset("can_attack"),fn);
		cl->set_value(cl->member_offset("can_attack_parent"),fn);
		//for all type->funs do set the nonparent members to the functions
		for(int f=0;f<G1_F_TOTAL;f++)
			{
			if (type->funs[f])
				{
				li_symbol *forsymbol=s_ofun[f];
				cl->set_value(cl->member_offset(forsymbol),type->funs[f]);
				}
			}
		}
	}
//methods to handle mini-objects.

li_object *li_get_mini_object_pos(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	int num_mini_obj=li_int::get(li_eval(li_second(o,env),env),env)->value();
	if (num_mini_obj<c->num_mini_objects)
		{
		li_object *rl=0;
		g1_mini_object *mi=&c->mini_objects[num_mini_obj];
		rl=li_make_list(new li_float(mi->x),
			new li_float(mi->y),
			new li_float(mi->h),
			new li_vect(mi->offset),
			new li_vect(mi->rotation),0);
		return rl;
		}
	li_error(env,"USER: An object of type %s doesn't have a mini-object with number %i. "
		"Be aware that the first mini-object has number 0.",
		c->name(),num_mini_obj);
	return 0;
	}

li_object *li_set_mini_object_pos(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	int num_mini_obj=li_int::get(li_eval(li_second(o,env),env),env)->value();
	if (num_mini_obj<c->num_mini_objects)
		{
		li_object *rl=li_eval(li_third(o,env),env),*ri;
		g1_mini_object *mi=&c->mini_objects[num_mini_obj];
		//rl should be a list of exactly 5 elements
		ri=li_car(rl,env);
		mi->x=li_float::get(li_eval(ri,env),env)->value();
		rl=li_cdr(rl,env);
		ri=li_car(rl,env);
		mi->y=li_float::get(li_eval(ri,env),env)->value();
		rl=li_cdr(rl,env);
		ri=li_car(rl,env);
		mi->h=li_float::get(li_eval(ri,env),env)->value();
		rl=li_cdr(rl,env);
		ri=li_car(rl,env);
		mi->offset=li_vect::get(li_eval(ri,env),env)->value();
		rl=li_cdr(rl,env);
		ri=li_car(rl,env);
		mi->rotation=li_vect::get(li_eval(ri,env),env)->value();
		return 0;
		}
	li_error(env,"USER: An object of type %s doesn't have a mini-object with number %i. "
		"Be aware that the first mini-object has number 0.",
		c->name(),num_mini_obj);
	return 0;
	}

li_object *li_set_mini_object_model(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	int num_mini_obj=li_int::get(li_eval(li_second(o,env),env),env)->value();
	if (num_mini_obj<c->num_mini_objects)
		{
		li_object *rl=li_eval(li_third(o,env),env),*ri;
		g1_mini_object *mi=&c->mini_objects[num_mini_obj];
		//rl should be a list of exactly 4 elements
		ri=li_car(rl,env);
		mi->animation=li_int::get(li_eval(ri,env),env)->value();
		rl=li_cdr(rl,env);
		ri=li_car(rl,env);
		mi->frame=li_int::get(li_eval(ri,env),env)->value();
		rl=li_cdr(rl,env);
		ri=li_car(rl,env);
		mi->defmodeltype=li_int::get(li_eval(ri,env),env)->value();
		rl=li_cdr(rl,env);
		ri=li_car(rl,env);
		mi->lod_model=li_int::get(li_eval(ri,env),env)->value();
	
		return 0;
		}
	li_error(env,"USER: An object of type %s doesn't have a mini-object with number %i. "
		"Be aware that the first mini-object has number 0.",
		c->name(),num_mini_obj);
	return 0;
	}
li_object *li_get_mini_object_model(li_object *o, li_environment *env)
	{
	g1_object_class *c=get_object_param(o,env);
	int num_mini_obj=li_int::get(li_eval(li_second(o,env),env),env)->value();
	if (num_mini_obj<c->num_mini_objects)
		{
		li_object *rl=0;
		g1_mini_object *mi=&c->mini_objects[num_mini_obj];
		rl=li_make_list(new li_int(mi->animation), //should always be 0 at the moment
			new li_int(mi->frame),
			new li_int(mi->defmodeltype),
			new li_int(mi->lod_model),0);
		return rl;
		}
	li_error(env,"USER: An object of type %s doesn't have a mini-object with number %i. "
		"Be aware that the first mini-object has number 0.",
		c->name(),num_mini_obj);
	return 0;
	}



li_object *li_def_movable_object(li_object *o, li_environment *env)
{
  li_symbol *sym=li_symbol::get(li_car(o,env),env);
  //todo: create symbols for parent calls.
  //we could do this by just adding several functions to the
  //classes type vars. Hmm... then we need to have a possiblity
  //to get the type var instance for a class object

  g1_movable_dynamic_object_type_class *type=0;

  if (g1_get_object_type(sym)!=0)
	  {
	  //the type of this object already exists, get the pointer
	  //to the previous instance.
	  //todo: check that the object really has the same type
	  type=(g1_movable_dynamic_object_type_class*)g1_object_type_array[g1_get_object_type(sym)];
	  }
  if (!type)
	type=new g1_movable_dynamic_object_type_class(sym); 

//Can't do this here, since the vars are not yet set. Need to use the init
  //function to do this.
  /*
  if (type->vars)
	  {
	  li_class *cl=type->vars;
	  li_object *fn=new li_function(li_g1_think);
	  cl->set_value(cl->member_offset("think"),fn);
	  cl->set_value(cl->member_offset("think_parent"),fn);
	  //...
	  }
  */
  type->special_init_function=&init_vars_function;
  if (type->vars)//that's just for the case of a redefinition
	  {
	  init_vars_function(type);
	  type->minis.clear();//they should be redefined
	  }

  for (li_object *l=li_cdr(o,env); l; l=li_cdr(l,env))
  {
    li_object *prop=li_car(l,env);
    li_symbol *sym=li_symbol::get(li_car(prop,env),env);
    prop=li_cdr(prop,env);

    if (sym==li_get_symbol("model_name", s_model_name))
    {
      int id=g1_model_list_man.find_handle(li_string::get(li_car(prop,env),env)->value());
      type->model=g1_model_list_man.get_model(id);
    }
    else if (sym==li_get_symbol("mini_object", s_mini_object))
    {
      g1_mini_object_def mo;
      mo.init();
          
      char *name=0;
      for (;prop; prop=li_cdr(prop,env)) // prop = ((offset 0 0 0.1) (position 0 0 0) (model_name "gunport_barrel"))
      {
        li_object *sub=li_car(prop,env);
        sym=li_symbol::get(li_car(sub,env),env); sub=li_cdr(sub,env);


        if (sym==li_get_symbol("offset", s_offset))
          read_vect(mo.offset, sub, env);
        else if (sym==li_get_symbol("position", s_position))
          read_vect(mo.position, sub, env);
        else if (sym==li_get_symbol("rotation", s_rotation))
          read_vect(mo.rotation, sub, env);
        else if (sym==li_get_symbol("model_name", s_model_name))
        {
          char *n=li_string::get(li_eval(li_car(sub,env), env),env)->value();
          mo.defmodeltype=g1_model_list_man.find_handle(n);
        }
        else li_error(env,"USER: %O should be (offset/position/rotation x y z)",sym);
      }

      type->minis.add(mo);

    }
    else if (sym==li_get_symbol("object_flags", s_object_flags))
    {
      for (;prop; prop=li_cdr(prop,env))
        type->obj_flags |= li_int::get(li_eval(li_car(prop,env),env),env)->value();
    }
    else if (sym==li_get_symbol("type_flags", s_type_flags))
    {
      for (;prop; prop=li_cdr(prop,env))
        type->flags |= li_int::get(li_eval(li_car(prop,env),env),env)->value();
    }
    else
    {
      int found=0;
      for (int i=0; i<G1_F_TOTAL; i++)
        if (sym==s_ofun[i])
        {
          //type->funs[i]=li_get_fun(li_car(prop,env),env);
		  type->funs[i]=li_symbol::get(li_car(prop,env),env);//need to store only the symbol
          found=1;
        }

      if (!found)
        li_error(env,"USER: unknown object property %O", sym);
    }      
  }   

  // if functions were not filled in see if we can find defaults
  for (int i=0; i<G1_F_TOTAL; i++)
    if (type->funs[i]==0)
    {
      char buf[200];
      sprintf(buf, "%s_%s", type->name(), g1_ofun_names[i]);
      
      li_symbol *sym=li_find_symbol(buf);
      if (sym)
      {
	      type->funs[i]=sym;
        //li_object *fun=li_function::get(sym->fun(), env);
        //if (fun)
        //  type->funs[i]=li_function::get(fun, env)->value();
		//type->funs[i]=fun;
      }
    }


  type->flags|=g1_object_definition_class::DELETE_WITH_LEVEL;
 
  return 0;
}

li_ext_class_context::li_ext_class_context(li_class *current_context,
										   li_environment *&env,
										   g1_object_class *for_obj,
										   i4_bool pos_ch)
										   :li_class_context(current_context)
	{
	g1_object_class *onstack=li_g1_ref::get(li_get_value("_object_on_stack",env),env)->value();
	if (onstack==for_obj) //don't regenerate everything if already there
		{
		n_env=env;
		obj=for_obj;
		pos_changes_allowed=pos_ch;
		return;
		}
	n_env=new li_environment(env,i4_T);
	obj=for_obj;
	pos_changes_allowed=pos_ch;
	//the objects this pointer 
	//should define_value, not set_value.
	//this creates a new value in the given environment, not in the root symbol table
	if (li_this)
		li_define_value("this",li_this,n_env);
	else
		li_define_value("this",li_nil,n_env);
	li_define_value("_object_on_stack",new li_g1_ref(for_obj->global_id),n_env);
	//the pointer to the type of the object (will contain the member functions)
	li_define_value("this_type",for_obj->get_type()->vars,n_env);
	li_define_value("xpos",new li_float(for_obj->x),n_env);
	li_define_value("ypos",new li_float(for_obj->y),n_env);
	li_define_value("hpos",new li_float(for_obj->h),n_env);
	li_define_value("theta",new li_float(for_obj->theta),n_env);
	li_define_value("pitch",new li_float(for_obj->pitch),n_env);
	li_define_value("roll",new li_float(for_obj->roll),n_env);
	li_define_value("health",new li_int(for_obj->health),n_env);
	g1_map_piece_class *mp=g1_map_piece_class::cast(for_obj);
	if (mp)
		{
		li_define_value("speed",new li_float(mp->speed),n_env);
		li_define_value("attack_target",new li_g1_ref(mp->attack_target.get()),n_env);
		}
	env=n_env;
	}

void li_ext_class_context::write_back(li_environment *env,
									  g1_object_class *to_obj)
	{
	to_obj->theta=(float)li_float::get(li_get_value("theta",env),env)->value();
	to_obj->pitch=(float)li_float::get(li_get_value("pitch",env),env)->value();
	to_obj->roll=(float)li_float::get(li_get_value("roll",env),env)->value();
	to_obj->health=(short)li_int::get(li_get_value("health",env),env)->value();
	g1_map_piece_class *mp=g1_map_piece_class::cast(to_obj);
	if (mp)
		{
		mp->speed=(float)li_float::get(li_get_value("speed",env),env)->value();
		mp->attack_target=li_g1_ref::get(li_get_value("attack_target",env),env)->value();
		}
	/* positional elements (x,y and h) are written back in occupy_location
	if (!pos_changes_allowed && 
		to_obj->get_flag(g1_object_class::MAP_OCCUPIED))//only think-methods may change the position
		return;
	float xp=li_float::get(li_get_value("xpos",env),env)->value();
	float yp=li_float::get(li_get_value("ypos",env),env)->value();
	float hp=li_float::get(li_get_value("hpos",env),env)->value();
	if (xp==to_obj->x && yp==to_obj->y && hp==to_obj->h)
		return; //need to test all of them since occupy_location()
	            //can do stuff based on the height, too.
	//warning: this may result in unexspected infinite recursion.
	if (to_obj->get_flag(g1_object_class::MAP_OCCUPIED))
		to_obj->unoccupy_location();
	to_obj->x=xp;
	to_obj->y=yp;
	to_obj->h=hp;
	to_obj->occupy_location();
	*/
	}

li_ext_class_context::~li_ext_class_context()
	{
	write_back(n_env,obj);
	}

class g1_li_fn_inits: public i4_init_class
	{
	public:
		void init()
			{
			li_add_function("request_think",li_request_think,0);
			li_add_function("request_remove",li_request_remove,0);
			li_add_function("sync",li_g1_sync,0);
			li_add_function("decrease_fire_delay",li_decrease_fire_delay);
			li_add_function("fire",li_g1_fire);
			li_add_function("find_targets",li_find_targets);
			li_add_function("max_health",li_max_health);
			li_add_function("object_owner",li_object_owner);
			li_add_function("local_player",li_local_player);
			li_add_function("object_flags",li_object_flags);
			li_add_function("objects_in_range",li_objects_in_range);
			li_add_function("object_pos",li_object_pos);
			li_add_function("object_orientation",li_object_pos);
			li_add_function("vector_element",li_vector_element);
			li_add_function("get_mini_object_model",li_get_mini_object_model);
			li_add_function("set_mini_object_model",li_set_mini_object_model);
			li_add_function("get_mini_object_pos",li_get_mini_object_pos);
			li_add_function("set_mini_object_pos",li_set_mini_object_pos);
			li_add_function("create_object", li_create_object);
			li_add_function("player_variables",li_player_variables);
			}
		int init_type() {return I4_INIT_TYPE_LISP_FUNCTIONS;}
	}g1_li_fn_inits_inst;

