#include "pch.h"

//This file belongs to the revival project
//A convoy object

#include "sound_man.h"
#include "objs/model_id.h"
#include "objs/model_draw.h"
#include "objs/convoy.h"
#include "input.h"
#include "math/pi.h"
#include "math/trig.h"
#include "math/angle.h"
#include "resources.h"
#include "saver.h"
#include "map_cell.h"
#include "map.h"
#include "map_man.h"
#include "objs/vehic_sounds.h"
#include "sound/sfx_id.h"
#include "objs/fire.h"
#include "object_definer.h"
#include "objs/path_object.h"
#include "li_objref.h"
#include "g1_render.h"
#include "objs/map_piece.h"
#include "solvemap.h"
#include "solvemap_breadth.h"
#include "solvegraph_breadth.h"
#include "lisp/li_class.h"
#include "editor/dialogs/path_win.h"

#include "image_man.h"


static li_g1_ref_list_class_member units("units");
static li_symbol_class_member formation("user_formation");

void g1_convoy_init(void)
	{
	};

g1_object_definer<g1_convoy_class>
g1_convoy_def("convoy",g1_object_definition_class::TO_MAP_PIECE|
			  g1_object_definition_class::EDITOR_SELECTABLE|
			  g1_object_definition_class::MOVABLE,
			  g1_convoy_init);

g1_convoy_class::g1_convoy_class(g1_object_type id, g1_loader_class *fp)
:g1_map_piece_class(id,fp)
	{
	w16 ver=0,data_size=0;
	if (fp) fp->get_version(ver,data_size);
	//switch (ver)
	//	{
	//	case DATA_VERSION:
	//	default:
	//		break;
	//	}
	//nothing to be read: the entrire status is stored in the lisp part.
	if (fp) fp->end_version(I4_LF);
	draw_params.setup("blackred");
	set_flag(SELECTABLE,1);
	}

g1_convoy_class::~g1_convoy_class()
	{
	
	}

void g1_convoy_class::save(g1_saver_class *fp)
	{
	g1_map_piece_class::save(fp);
	fp->start_version(DATA_VERSION);
	fp->end_version();
	}

void g1_convoy_class::add_object(g1_object_class *obj)
	{
	li_class_context c(vars);
	li_g1_ref_list *list=units()->clone();//be shure every instance has its own list
	vars->set_value(units.offset,list);
	if (list->find(obj)<0)
		list->add(obj);
	}

void g1_convoy_class::remove_object(g1_object_class *obj)
	{
	li_class_context c(vars);
	li_g1_ref_list *list=units()->clone();
	vars->set_value(units.offset,list);
	if (list->find(obj)>=0)
		list->remove(obj);
	//units()->remove(obj);
	}

void g1_convoy_class::draw(g1_draw_context_class *context, i4_3d_vector& viewer_position)
	{
	
	if (context->draw_editor_stuff)
		{
		li_class_context c(vars);
		int t=units()->size(),i;
		g1_map_piece_class *mp=0;
		for (i=0;i<t;i++)
			{
			mp=g1_map_piece_class::cast(units()->value(i));
			if (mp)
				{
				g1_render.render_3d_line(i4_3d_vector(x,y,h),i4_3d_vector(mp->x,mp->y,mp->h),
					0xffffff00,0xffffff00,context->transform,i4_F);
				}
			}
		}
	}

i4_bool g1_convoy_class::can_attack(g1_object_class *who)
	{//returns T if at least one unit can attack this target
	//return i4_T;//the objects must decide themselves afterwards
	li_class_context ctx(vars);
	for (int i=0;i<units()->size();i++)
		{
		g1_object_class *o=units()->value(i);
		if (o&&o->can_attack(who))
			return i4_T;
		}
	return i4_F;
	}

void g1_convoy_class::damage(g1_object_class *obj,int hp,i4_3d_vector dam)
	{//a convoy object cannot take damage;

	};

void g1_convoy_class::think()
	{
	units()->compact();
	if (units()->size()==0)
		{
		unoccupy_location();
		request_remove();
		return;
		}
	request_think();
	unoccupy_location();
	//todo: add code to build up formations
	calcsize();
	x=meanx;
	y=meany;
	h=meanh;
	if (attack_target.valid())
		{
		for (int i=0;i<units()->size();i++)
			{
			g1_map_piece_class *mp=g1_map_piece_class::cast(units()->value(i));
			if (mp&&mp->can_attack(attack_target.get()))
				{
				mp->attack_target=attack_target.get();
				}
			}
		attack_target=0;
		}
	if (!occupy_location())
		//cannot really fail if all other units are within the map
		{
		request_remove();
		}
	};

w8 g1_convoy_class::mingrade()
	{
	w8 ret=0;
	w32 param;
	li_class_context c(vars);
	units()->compact();
	for (int i=0;i<units()->size();i++)
		{
		g1_map_piece_class *mp=g1_map_piece_class::cast(units()->value(i));
		if (!mp) 
			continue;
		param=GRADE(mp->solveparams);
		if (ret<param)
			ret=(w8)param;
		}
	return ret;
	}

void g1_convoy_class::calcsize()
	{
	float minx=1E10,maxx=0;//certainly greater than any map
	float miny=1E10,maxy=0;
	float minh=1E10,maxh=-1E10;
	//if (formation()==li_get_symbol("none",li_none))
	//	{
	//	sizex=1;
	//	sizey=1;
	//	}
	g1_map_piece_class *mp;
	li_class_context ctx(vars);
	for (int i=0;i<units()->size();i++)
		{
		mp=g1_map_piece_class::cast(units()->value(i));
		if (!mp)
			{
			if (get_single())
				{
				meanx=units()->value(0)->x;
				meany=units()->value(0)->y;
				meanh=units()->value(0)->h;
				sizex=1;
				sizey=1;
				return;
				}
			continue;
			}
		if (mp->x<minx) 
			minx=mp->x;
		if (mp->x>maxx)
			maxx=mp->x;
		if (mp->y<miny)
			miny=mp->y;
		if (mp->y>maxy)
			maxy=mp->y;
		if (mp->h<minh)
			minh=mp->h;
		if (mp->h>maxh)
			maxh=mp->h;

		}
	if (maxx<minx) //this can oldy happen if nothing is found
		{
		meanx=meany=meanh=0;
		sizex=sizey=0;
		return;
		}
	meanx=(minx+maxx)/2.0f;
	meany=(miny+maxy)/2.0f;
	meanh=(minh+maxh)/2.0f;
	sizex=maxx-minx;
	if (sizex>10) sizex=10;
	sizey=maxy-miny;
	if (sizey>10) sizey=10;
	}

g1_map_solver_class *g1_convoy_class::prefered_solver()
	{
	if (g1_get_map()->has_graph())
		return g1_get_map()->get_graph_solver();
	return g1_get_map()->get_breadth_solver(mingrade());
	}


i4_bool g1_convoy_class::move(i4_float x_amount,i4_float y_amount)
	{
	return i4_T;
	}

void g1_convoy_class::setup(g1_object_class *firstobj)
	{
	add_object(firstobj);
	calcsize();
	x=meanx;
	y=meany;
	h=meanh;
	grab_old();
	occupy_location();
	request_think();
	}

i4_bool g1_convoy_class::deploy_to(i4_float destx,i4_float desty, g1_path_handle ph)
	{
	i4_float point[MAX_SOLVEPOINTS*2];
	w16 tpoints=0;
	li_class_context c(vars);
	g1_map_piece_class *mp;
	w16 single=get_single();
	if (single)
		{
		
		return units()->value(0)->deploy_to(destx,desty,ph);//use the objects solver
		//or special deploy handling (like for superguns)
		//should change the function deploy_to to take a third argument,
		//a path handle. If it's 0, it calculates its own path
		//otherwise the given path is used. 
		//The Problem is that calling deploy_to() on every object
		//in the list causes each to use its own path, not very useful.
		}
	g1_map_solver_class *s=prefered_solver();
	
	if (s)
		{
		g1_path_handle ph1=ph;
		if (!ph)
			s->path_solve(x,y,destx,desty,(w8)sizex,(w8)sizey,mingrade(),point,tpoints);
		if (tpoints)
			{
			//assign this path with small modifications to each unit.
			float xdiff,ydiff;
			
			units()->compact();
			for (int i=0;i<units()->size();i++)
				{
				mp=g1_map_piece_class::cast(units()->value(i));
				if (!mp)
					{
					
					continue;
					}
				xdiff=mp->x-x;
				ydiff=mp->y-y;
				point[0]=destx-xdiff;
				point[1]=desty-ydiff;
				ph1=g1_path_manager.alloc_path(tpoints,point);
				mp->deploy_to(destx,desty,ph1);
				//mp->solveparams&=~SF_USE_PATHS;
				//mp->my_solver=0;
				//mp->unlink();
				//mp->set_path(ph);
				}
			return i4_T;
			}
		else
			return i4_F;//can't find a way
		}
	return i4_F;//cannot deploy without a solver
	};

void g1_convoy_class::fire()
	{
	}

short g1_convoy_class::get_single()
	{
	li_class_context ctx(vars);
	if (units()->size()==1)
		{
		g1_object_class *c=units()->value(0);
		if (c)
			return c->id;
		else
			return 0;
		}
	return 0;
	}

short single_sel_id(g1_object_class *s)
	{
	g1_map_piece_class *mp=g1_map_piece_class::cast(s);
	if (mp&&mp->id==g1_get_object_type("convoy"))
		{
		return ((g1_convoy_class *)mp)->get_single();
		}
	return 0;
	}