/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h" //just to remember: this must always be the first and
//only the first include file in the list. So newer include within a .h file
#include "objs/path_object.h"
#include "object_definer.h"
#include "lisp/li_init.h"
#include "lisp/li_class.h"
#include "li_objref.h"
#include "map_man.h"
#include "map.h"
#include "saver.h"
#include "li_objref.h"
#include "player.h"
#include "g1_render.h"
#include "objs/map_piece.h"
#include "isllist.h"
#include "objs/bases.h"
#include "sound/sfx_id.h"
#include "render/r1_clip.h"
#include "resources.h"
#include "objs/road_object.h"

extern int g1_show_list;  // defined in map_piece.cc

g2_breadth_first_road_solver_class g2_breadth_solver;
g2_astar_road_solver_class g2_astar_solver;

enum
{
	DATA_VERSION1=1,
	DATA_VERSION2=2,
	DATA_VERSION3=3
};




static li_symbol_class_member active("active");
static li_symbol_ref on("on"), yes("yes"), no("no"), already_attached("already_attached");

static li_symbol_ref off("off"), s_add_link("add_link"), s_remove_link("remove_link");
static li_g1_ref_class_member start("start");
static li_symbol_class_member bridgeable_spot("bridgeable_spot");

i4_isl_list<g1_path_object_class> g1_path_object_list;


static li_g1_ref_list_class_member links("links"), enemy_links("enemy_links"),
controlled_objects("controlled_objects");

static li_int_class_member warning_level("warning_level");


int g1_path_object_class::bomb_warning_level()
{
	return vars->get(warning_level);
}

g1_path_object_class::bridge_status_type g1_path_object_class::get_bridge_status()
{
	li_symbol * s=vars->get(bridgeable_spot);

	if (s==yes.get())
	{
		return NO_BRIDGE;
	}
	else if (s==no.get())
	{
		return NOT_BRIDGABLE;
	}
	else
	{
		return HAS_BRIDGE;
	}
}

int g1_path_object_class::total_links(g1_team_type team)
{
	return (abs(link_index[team+1]) - abs(link_index[team]));
}

int g1_path_object_class::total_links()
{
	return link_index[G1_MAX_TEAMS]; //should (?) contain the end of the list
}

g1_path_object_class::g1_path_object_class(g1_object_type id, g1_loader_class * fp)
	: g1_object_class(id, fp),
	  link(8,16)
{
	int i;
	w16 ver=0,data_size=0;

	next=0;
	if (fp)
	{
		fp->get_version(ver,data_size);
	}

	switch (ver)
	{
		case DATA_VERSION2:
			{
				for (i=0; i<G1_MAX_TEAMS; i++)
				{
					last_selected_tick[i]=fp->read_32();
				}

				link_index[0]=0;
				for (i=0; i<G1_MAX_TEAMS; i++)
				{
					link_index[i+1] = fp->read_8();
				}

				for (i=0; i<link_index[G1_MAX_TEAMS]; i++)
				{
					link_class * l = link.add();

					l->path.load(fp);
					l->object.load(fp);
					l->link_desc=0;
				}
			} break;
		case DATA_VERSION3:
			{
				for (i=0; i<G1_MAX_TEAMS; i++)
				{
					last_selected_tick[i]=fp->read_32();
				}

				link_index[0]=0;
				for (i=0; i<G1_MAX_TEAMS; i++)
				{
					link_index[i+1] = fp->read_8();
				}

				for (i=0; i<link_index[G1_MAX_TEAMS]; i++)
				{
					link_class * l = link.add();

					l->path.load(fp);
					l->object.load(fp);
					l->link_desc=fp->read_32();
				}
			} break;

		default:
			{
				if (fp)
				{
					fp->seek(fp->tell() + data_size);
				}

				link_index[0]=0;
				for (i=0; i<G1_MAX_TEAMS; i++)
				{
					last_selected_tick[i]=1;
					link_index[i+1]=0;
				}
			} break;
	}
	if (fp)
	{
		fp->end_version(I4_LF);
	}

	draw_params.setup("blackred");
	set_flag(SELECTABLE | TARGETABLE, 1);
}

void g1_path_object_class::validate()
{
	for (int a=0; a<G1_MAX_TEAMS; a++)
	{
		for (int i=total_links((g1_team_type)a)-1; i>=0; i--)
		{
			link_class * l = &link[link_index[a]+i];
			if (!l->path.valid() || !l->object.valid())
			{
				link.remove(link_index[a] + i);
				for (int t=a; t<G1_MAX_TEAMS; t++)
				{
					link_index[t+1]--;
				}
			}
		}
	}
}

void g1_path_object_class::save(g1_saver_class * fp)
{
	int i;

	g1_object_class::save(fp);

	fp->start_version(DATA_VERSION3);

	for (i=0; i<G1_MAX_TEAMS; i++)
	{
		fp->write_32(last_selected_tick[i]);
	}

	for (i=0; i<G1_MAX_TEAMS; i++)
	{
		fp->write_8(link_index[i+1]);
	}

	for (i=0; i<link.size(); i++)
	{
		link[i].path.save(fp);
		link[i].object.save(fp);
		fp->write_32(link[i].link_desc);
	}

	fp->end_version();
}

i4_bool g1_path_object_class::occupy_location()
{

//  int a,i;

	if (occupy_location_corners())
	{
		g1_path_object_list.insert(*this);
		return i4_T;
	}
	else
	{
		return i4_F;
	}
}


void g1_path_object_class::unoccupy_location()
{
	if (get_flag(MAP_OCCUPIED))
	{
		g1_object_class::unoccupy_location();
		g1_path_object_list.find_and_unlink(this);
	}
}

void g1_path_object_class::draw(g1_draw_context_class * context, i4_3d_vector& viewer_position)
{
	if (g1_show_list)
	{
		int a=0;
		for (int i=0; i<link.size(); i++)
		{
			while (i>=link_index[a+1]) a++;

			// determine team number
			i4_float offs = a*0.2f-0.1f;
			i4_color col = (a==0) ? 0xffff : 0xff00ff;
			g1_object_class * o = link[i].get_object();
			if (o)
			{
				g1_render.render_3d_line(i4_3d_point_class(x+offs,y+offs,h+0.1f),
										 i4_3d_point_class(o->x+offs, o->y+offs, o->h+0.1f),
										 col, 0, context->transform);
			}
		}
	}

#if 0
	if (controlled_objects()->size())
	{
		g1_model_draw(this, draw_params, context);
	}
	else
#endif
	g1_editor_model_draw(this, draw_params, context, viewer_position);

}


void g1_path_object_class::add_controlled_object(g1_object_class * o)
{
	li_class_context context(vars);
	li_g1_ref_list * list=controlled_objects()->clone();
	vars->set_value(controlled_objects.offset, list);
	if (list->find(o)<0)
	{
		list->add(o);
	}
}

void g1_path_object_class::remove_controlled_object(g1_object_class * o)
{
	li_class_context context(vars);
	li_g1_ref_list * list=controlled_objects()->clone();
	vars->set_value(controlled_objects.offset,list);
	if (list->find(o)>=0)
	{
		list->remove(o);
	}
}

void g1_path_object_class::add_link(g1_team_type team, g1_path_object_class * o, link_id lid)
{
	if (get_path_index(team, o)<0)
	{
		link_class * l = link.add_at(link_index[team+1]);
		l->link_desc=lid;
		l->path = o;
		l->object = o;
		for (int i=team; i<G1_MAX_TEAMS; i++)
		{
			link_index[i+1]++;
		}
	}
}

i4_bool g1_path_object_class::remove_link(g1_team_type team, g1_path_object_class * p)
{
	int loc = get_path_index(team, p);

	if (loc>=0)
	{
		link.remove(link_index[team] + loc);
		for (int i=team; i<G1_MAX_TEAMS; i++)
		{
			link_index[i+1]--;
		}
		return i4_T;
	}
	return i4_F;
}

void g1_path_object_class::request_remove()
{
	int team=0;

	for (int i=0; i<link.size(); i++)
	{
		while (i>=link_index[team+1]) team++;

		// determine team number

		g1_object_class * o = link[i].get_object();
		g1_map_piece_class * mp;

		while ((mp = g1_map_piece_class::cast(o))!=0)
		{
			if (mp->next_path.get() == this)
			{
				o = mp->prev_object.get();
			}
			else
			{
				o = mp->next_object.get();
			}
			mp->unlink();
		}

		g1_path_object_class * path = link[i].get_path();


		I4_TEST(o == path, "Invalid Linked List!");

		if (path)
		{
			path->remove_link((team==G1_ALLY) ? G1_ENEMY : G1_ALLY, this);
		}
	}
	link.clear();
	g1_object_class::request_remove();

}

li_object * g1_path_object_class::message(li_symbol * message_name,
										  li_object * message_params,
										  li_environment * env)
{
	li_class_context context(vars);

	if (message_name==on.get() || message_name==off.get())
	{
		active() = on.get();
	}
	else if (message_name==s_add_link.get())
	{
		g1_object_class * o=li_g1_ref::get(message_params,env)->value();
		g1_path_object_class * path = g1_path_object_class::cast(o);

		if (path)
		{
			add_link(G1_ALLY, path);
			path->add_link(G1_ENEMY, this);
		}
	}
	else if (message_name==s_remove_link.get())
	{
		g1_object_class * o=li_g1_ref::get(message_params,env)->value();
		g1_path_object_class * path = g1_path_object_class::cast(o);

		if (path)
		{
			remove_link(G1_ALLY, path);
			path->remove_link(G1_ENEMY, this);
		}
	}

	return 0;
}

int g1_path_object_class::get_path_index(g1_team_type team, g1_path_object_class * o) const
{
	for (int i=link_index[team]; i<link_index[team+1]; i++)
	{
		if (link[i].get_path() == o)
		{
			return i-link_index[team];
		}
	}

	return -1;
}

int g1_path_object_class::get_path_index(g1_path_object_class * o) const
{
	for (int i=0; i<link.size(); i++)
	{
		if (link[i].get_path() == o)
		{
			return i;
		}
	}

	return -1;
}

int g1_path_object_class::get_object_index(g1_object_class * o) const
{
	for (int i=0; i<link.size(); i++)
	{
		if (link[i].get_object() == o)
		{
			return i;
		}
	}

	return -1;
}

g1_path_object_class * g1_path_object_class::link_class::get_path()
{
	return g1_path_object_class::cast(path.get());
	/*
	   	g1_object_class *trypath=path.get();
	   	//g1_path_object_class *apath;
	   	g1_path_object_class *castedpath=0;
	   	if (trypath)//Ok, we at least got some object, try to convert to a path.
	   	{
	   		castedpath=g1_path_object_class::cast(trypath);
	   		if (!castedpath)
	   		{
	   		//No path, but object, try to convert anyway.
	   		castedpath=(g1_path_object_class *)trypath;
	   		}
	   	}
	   	else
	   	{//Path was invalid, try something else
	   		//first try wheter object is something valid
	   		trypath=object.get();
	   		if (trypath)
	   			{
	   			castedpath=g1_path_object_class::cast(trypath);
	   			if (!castedpath)
	   			castedpath=(g1_path_object_class *)trypath;
	   			}
	   		//no, let's look for some other waypoint anywhere.
	   		else
	   		castedpath=0;
	   		//while (!castedpath && trypath!=0)
	   		//{
	   		//	castedpath=g1_path_object_class::cast(apath.path.get());
	   		//	apath=apath.next;
	   		//}
	   		//}
	   	}
	   	return castedpath;
	 */
}

static g1_team_type g1_path_cur_team;
int links_sorter(g1_path_object_class * const * a, g1_path_object_class * const * b)
{
	//must define an ABSOLUTE order on the elements
	g1_path_object_class * a1= *a,* b1= *b;

	if (a1->last_selected_tick[g1_path_cur_team]>
		b1->last_selected_tick[g1_path_cur_team]) //meaning: a is more recent than b
	{
		return 1;
	}
	else if (a1->last_selected_tick[g1_path_cur_team]<
			 b1->last_selected_tick[g1_path_cur_team])
	{
		return -1;
	}
	else if (a1->global_id>b1->global_id) //if ticks are equal, use id
	{
		//there would be a problem if they weren't unique...
		return 1;
	}
	else if (a1==b1)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

g1_path_object_class * g1_path_object_class::get_recent_road(g1_team_type team,
															 g1_path_object_class * last_used)
{
	//Return any path that can be found
	g1_path_object_class * best=0;
	//w32 max_allowed=last_used ? last_used->last_selected_tick[team] : 0x7fffffff-1;

	int t=total_links();
	int best_tick=0;

	//int past_it= last_used?0:1;//set to 1 if first evaluation for this node
	i4_array<g1_path_object_class *> links(10,10);
	int i;
	for (i=0; i<t; i++)
	{
		g1_path_object_class * p=get_link(i);
		if (p && !p->get_flag(SCRATCH_BIT))
		{
			links.add(p);
		}
	}
	g1_path_cur_team=team; //currently, there's no better solution for this
	links.sort(links_sorter);
	i4_bool found=last_used ? i4_F : i4_T;
	for (i=0; i<links.size(); i++) //size() might not be equal to t
	{
		if (found)
		{
			return links[i];
		}
		if (links[i]==last_used)
		{
			//so we'll use the next entry
			found=1;
		}
	}

	return best;
}

g1_path_object_class * g1_path_object_class::get_recent_link(g1_team_type team,
															 g1_path_object_class * last_used)
{
	g1_path_object_class * best=0;
	//Fixed bug in the following line, Maximum was 0xffffffff casted to signed...
	w32 max_allowed=last_used ? last_used->last_selected_tick[team] : 0x7fffffff-1;

	w32 t=total_links(team);
	w32 best_tick=0;
	int past_it=0;

	for (int i=0; i<(int)t; i++)
	{
		g1_path_object_class * p=get_link(team,i);
		if (p)
		{
			int tick=p->last_selected_tick[team];

			if (tick<=(int)max_allowed)
			{
				if (tick<(int)max_allowed && tick>(int)best_tick)
				{
					best=p;
					best_tick=tick;
				}
				else if (tick==(int)max_allowed && past_it && best_tick!=max_allowed)
				{
					best=p;
					best_tick=tick;
				}
			}

			if (p==last_used)
			{
				past_it=1;
			}
		}
	}

	return best;
}

link_id g1_path_object_class::link_to(g1_team_type team, g1_path_object_class * obj)
{
	//link_id ret=0;
	int t=total_links(); //we actually ignore the team

	//g1_path_object_class *c;
	for (int i=0; i<t; i++)
	{
		if (obj==get_link(i))
		{
			return get_link_id(i);
		}
	}
	return 0;
}

int g1_path_object_class::total_controlled_objects()
{
	return li_g1_ref_list::get(vars->get(controlled_objects),0)->size();
}

g1_object_class * g1_path_object_class::get_controlled_object(int object_num)
{
	return li_g1_ref_list::get(vars->get(controlled_objects),0)->value(object_num);
}

// returns the total destinations found (banks & etc that are attached to the path)
int g1_path_object_class::find_path_destinations(g1_object_class * * list,
												 int list_size,
												 g1_team_type team)
{
	i4_array<g1_object_class *> objects_to_unmark(128,128);
	i4_array<g1_path_object_class *> unvisited_nodes(128,128);
	char * c1="links";
	char * c2="enemy_links";
	int off=vars->member_offset(team==G1_ALLY ? c1 : c2);
	unvisited_nodes.add(this);
	int unvisited_head=0;
	int t_in_list=0;

	while (unvisited_head<unvisited_nodes.size())
	{
		g1_path_object_class * p=unvisited_nodes[unvisited_head++];

		if (p && !p->get_flag(SCRATCH_BIT))
		{
			p->set_flag(SCRATCH_BIT, 1);
			objects_to_unmark.add(p);

			li_g1_ref_list * l=li_g1_ref_list::get(p->vars->value(off),0);
			int t=l->size(), i;
			for (i=0; i<t; i++)
			{
				g1_object_class * o=l->value(i);
				if (o)
				{
					unvisited_nodes.add(g1_path_object_class::cast(o));
				}
			}

			if (li_g1_ref_list::get(p->vars->get(controlled_objects),0)->size())
			{
				list[t_in_list++]=p;
			}
		}
	}

	for (int i=0; i<objects_to_unmark.size(); i++)
	{
		objects_to_unmark[i]->set_flag(SCRATCH_BIT, 0);
	}

	return t_in_list;
}


int g1_path_object_class::find_path(g1_team_type team,
									g1_path_object_class * * stack,
									int stack_size)
{
	int t=0;
	g1_path_object_class * o=this;

	//int numlinks=0;
	do
	{
		stack[t++]=o;
		//numlinks++;
		if (o)
		{
			o=o->get_recent_link(team, 0);
		}
	}
	while (o && t<stack_size);

	if (t<2) //can't go that way, try the other
	{
		o=this;
		t=0;
		if (team==G1_ALLY)
		{
			team=G1_ENEMY;
		}
		else
		{
			team=G1_ALLY;
		}
		do
		{
			stack[t++]=o;
			if(o)
			{
				o=o->get_recent_link(team,0);
			}
		}
		while (o && t<stack_size);
	}
	I4_ASSERT(t<stack_size, "ERROR: Either paths are too long or path loop encountered!");

	return t;
}


int g1_path_object_class::find_path(g1_team_type team, g1_path_object_class * dest,
									g1_path_object_class * * stack,
									int stack_size)
{
	g1_path_object_class * visited[25000];
	w32 most_recently_selected[2000];

	I4_ASSERT(stack_size<=2000, "WARNING: Can't find paths greater than 2000.  bump this up, if needed.");

	int depth=0, num_visited=0;

	enum {
		BIG_NUM=0xffffffff
	};

	stack[depth] = this;
	stack[depth+1] = 0;
	most_recently_selected[depth] = BIG_NUM;
	set_flag(g1_object_class::SCRATCH_BIT,1);
	visited[num_visited++] = this;
	//g1_team_type curteam=team;
	do
	{
		g1_path_object_class * o = stack[depth]->get_recent_link(team, stack[depth+1]);
		//if (!o&&curteam==team)
		//	{
		//	if (team==G1_ALLY) team==G1_ENEMY; else team==G1_ALLY;
		//	o=stack[depth]->get_recent_link(team, stack[depth]);
		//	}
		if (o&& !o->get_flag(SCRATCH_BIT))
		{
			o->set_flag(g1_object_class::SCRATCH_BIT,1);
			visited[num_visited++] = o;

			depth++;
			stack[depth] = o;
			stack[depth+1] = 0;
		}
		else
		{
			depth--;
		}
	}
	while (depth>=0 && stack[depth]!=dest && depth+1<stack_size);

	for (int i=0; i<num_visited; i++)
	{
		visited[i]->set_flag(g1_object_class::SCRATCH_BIT,0);
	}

	if (depth<0 || depth>stack_size-2)
	{
		depth=0;
	}

	if (depth)
	{
		while (stack[depth] && depth<stack_size-2) // whatsthis? && stack[depth]->total_links(type)>0)
		{
			g1_path_object_class * o = stack[depth]->get_recent_link(team, 0);
			stack[++depth] = o;
		}

		stack[++depth]=0;
	}

	for (int j=0; j<depth; j++)
	{
		if (!stack[j])
		{
			return j;
		}
	}

	return depth;
}

bool g1_path_object_class::repair(int how_much)
{
	//Repairing path objects repairs all attached
	//objects to full health (ignoring the argument, since capturing them also restores
	//them)
	int t=total_controlled_objects();

	for (int i=0; i<t; i++)
	{
		g1_object_class * o=get_controlled_object(i);
		if (o && o->player_num==player_num)
		{
			o->repair(o->get_max_health());
		}
	}
	return true;
}

void g1_path_object_class::change_player_num(int new_player)
{
	g1_object_class::change_player_num(new_player);
	int t=total_controlled_objects();
	for (int i=0; i<t; i++)
	{
		g1_object_class * o=get_controlled_object(i);
		if (o && o->player_num!=new_player)
		{
			char msg[100];
			w32 color;
			if (new_player==g1_player_man.local_player)
			{
				sprintf(msg, "Building Captured: %s", o->name());
				color=0x00ff00;
			}
			else
			{
				sprintf(msg, "Building Lost: %s", o->name());
				color=0xff0000;
			}

			g1_player_man.show_message(msg, color, g1_player_man.local_player);

			o->change_player_num(new_player);
		}

	}
}

g1_path_object_class * g1_path_object_class::find_next(g1_team_type team,
													   g1_path_object_class * dest)
{
	g1_path_object_class * path[256];

	find_path(team, dest, path, 256);
	return path[1];
}

void g1_path_object_class::editor_draw(g1_draw_context_class * context)
{
	int i;

	for (i=0; i<total_links(G1_ALLY); i++)
	{
		g1_object_class * o = get_link(G1_ALLY,i);
		if (o)
		{
			g1_render.render_3d_line(i4_3d_point_class(x,y,h+0.1f),
									 i4_3d_point_class(o->x, o->y, o->h+0.1f),
									 0xffffff, 0, context->transform);
		}
	}

	for (i=0; i<total_links(G1_ENEMY); i++)
	{
		g1_object_class * o = get_link(G1_ENEMY,i);
		if (o)
		{
			g1_render.render_3d_line(i4_3d_point_class(o->x, o->y, o->h+0.1f),
									 i4_3d_point_class(x,y,h+0.1f),
									 0xffffff, 0, context->transform);
		}
	}
	li_g1_ref_list::get(vars->get(controlled_objects),0)->draw(this, 0xff0000, context);
}

g1_object_definer<g1_path_object_class>
g1_path_object_def("path_object",
				   g1_object_definition_class::EDITOR_SELECTABLE |
				   g1_object_definition_class::TO_PATH_OBJECT);

g1_object_definer<g1_road_object_class>
g1_road_object_def("road_object",
				   g1_object_definition_class::EDITOR_SELECTABLE|
				   g1_object_definition_class::TO_PATH_OBJECT);

const int ROAD_DATA_VERSION=1;

g1_road_object_class::g1_road_object_class(g1_object_type id, g1_loader_class * fp)
	: g1_path_object_class(id,fp)
{
	w16 version,data_size;

	ref=0;
	rlen=0;
	if (fp)
	{
		fp->get_version(version,data_size);
	}
	else
	{
		version=0;
	}
	switch (version)
	{
		case  ROAD_DATA_VERSION:
			{
				nid=fp->read_32();
				fp->read_32(); //reserved
				fp->end_version(I4_LF);
			}
			break;
		case 0:
			{
				nid=0; // a setup() will come shortly
			}
			break;
	}
	draw_params.setup("smallcrossroad");
	set_flag(TARGETABLE,0);

}

void g1_road_object_class::save(g1_saver_class * fp)
{
	g1_path_object_class::save(fp);
	fp->start_version(ROAD_DATA_VERSION);
	fp->write_32(nid);
	fp->write_32(0); //reserved
	fp->end_version();
}

void g1_road_object_class::setup(node_id thisid,i4_float _x, i4_float _y, i4_float _h)
{
	x=_x;
	y=_y;
	h=_h;
	if (x<0)
	{
		x=0;
	}
	if (x>g1_get_map()->width())
	{
		x=g1_get_map()->width()-1;
	}
	//cause scaling may fail because of floating point rounding errors
	if (y<0)
	{
		y=0;
	}
	if (y>g1_get_map()->height())
	{
		y=g1_get_map()->height()-1;
	}
	nid=thisid;
	if (!occupy_location())
	{
		g1_path_object_class::request_remove(); //don't remove from list. Caller knows
		//that this may fail.
	}
}

i4_bool g1_road_object_class::occupy_location()
{
	//todo: change this
	return g1_path_object_class::occupy_location();
}

void g1_road_object_class::editor_draw(g1_draw_context_class * context)
{

}

void g1_road_object_class::draw(g1_draw_context_class * context, i4_3d_vector& viewer_position)
{
	//g1_path_object_class::draw(context);
	//editor_draw(context);
	//don't do anything right now. later, we draw here the roads
	int i;

	r1_vert v[2];

	i4_3d_point_class p(x,y,h);
	g1_object_class * o;
	if (g1_render.project_point(p, v[0], context->transform))
	{
		if ((v[0].v.z * v[0].v.z)>g1_resources.lod_disappear_dist)
		{
			return;
		}
		r1_clip_clear_area((sw32)v[0].px-1, (sw32)v[0].py-1,
						   (sw32)v[0].px+1, (sw32)v[0].py+1, 0x000000ff, v[0].v.z, *context->context,
						   g1_render.r_api);

		//r1_set_color(&v[0],0xffff0000);
		//r1_clip_render_points(1,&v[0],g1_render.r_api);
	}
	;
	/*	for (i=0; i<total_links(G1_ALLY); i++)
	   {
	   o=get_link(G1_ALLY,i);
	   if (o)
	   {
	   if (g1_render.project_point(i4_3d_point_class(o->x,o->y,o->h),v[1], context->transform))
	   {
	   //r1_clip_render_lines draws projected points, but
	   //calculates on world coordinates.
	   //possible fix: check v.v.z to be large enough.
	   r1_clip_render_lines(1,v,g1_render.center_x,
	   g1_render.center_y,
	   g1_render.r_api);
	   r1_clip_clear_area((sw32)v[1].px-1, (sw32) v[1].py-1,
	   (sw32)v[1].px,(sw32)v[1].py,0x0000ff00,v[1].v.z, *context->context,
	   g1_render.r_api);
	   }
	   }
	   }
	   }*/
	//i4_transform_class *trans= new i4_transform_class();
	//g1_map_piece_class *obj;
	for (i=0; i<total_links(G1_ALLY); i++)
	{
		o = get_link(G1_ALLY,i);
		if (o)
		{
			g1_render.render_3d_line(p,
									 i4_3d_point_class(o->x, o->y, o->h),
									 0xff808080, 0xff808080, context->transform, i4_F);
			//obj=g1_map_piece_class::cast(get_object_link(G1_ALLY,i));
			//while (obj)
			//	{
			//obj->world_transform=trans;//not needed
			//obj->calc_world_transform(g1_render.frame_ratio)
			//	obj->draw(context);
			//obj->world_transform=0;
			//	obj=g1_map_piece_class::cast(obj->next_object.get());
			//	}

			/* //disabled because the cars are not managed right now
			   g1_object_class *origin_next,*origin=this;
			   g1_path_object_class *path;
			   g1_map_piece_class *mp;

			   if (path = g1_path_object_class::cast(origin))
			   	{
			   	int i,j = path->get_path_index(g1_path_object_class::cast(o));
			   	for (i=0;i<j;i++)
			   		{
			   		mp=g1_map_piece_class::cast(path->link[i].object.get());
			   		if (mp)
			   			mp->draw(context);
			   		}
			   	}

			 */
		}
	}
	//delete trans;
	//todo: add code to draw all cars on the outgoing links
}

int g1_road_object_class::find_path(g1_team_type team, g1_path_object_class * dest, g1_path_object_class * * stack,
									int stack_size)
{
	int res;

	//for performance analysis, we use both now.
	//g2_breadth_solver.path_solve(team,this,
	//	(g1_road_object_class*) dest,stack,stack_size);

	res=g2_astar_solver.path_solve(team,this,
								   (g1_road_object_class *) dest,stack,stack_size);
	return res;

	//should return the "best" path to destination
	//currently returns the most recent seen path to dest
	//g1_path_object_class *visited[25000];
	//w32 most_recently_selected[2000];

	//I4_ASSERT(stack_size<=2000, "WARNING: Can't find paths greater than 2000.  bump this up, if needed.");

	//int depth=0, num_visited=0;





	//enum {BIG_NUM=0xffffffff };

	//Doing it the right way now, since finding the bug bellow uses to
	//much time and the use of that routing code is questionable
	/*stack[depth] = this;
	   stack[depth+1] = 0;
	   //most_recently_selected[depth] = BIG_NUM;
	   set_flag(g1_object_class::SCRATCH_BIT,1);
	   visited[num_visited++] = this;
	   //g1_team_type curteam=team;
	   do
	   {
	   g1_path_object_class *o = stack[depth]->get_recent_road(team, stack[depth+1]);
	   //if (!o&&curteam==team)
	   //	{
	   //	if (team==G1_ALLY) team==G1_ENEMY; else team==G1_ALLY;
	   //	o=stack[depth]->get_recent_link(team, stack[depth]);
	   //	}
	   if (o&& !o->get_flag(SCRATCH_BIT))
	   {
	   	o->set_flag(g1_object_class::SCRATCH_BIT,1);
	   	visited[num_visited++] = o;

	   	depth++;
	   	stack[depth] = o;
	   	stack[depth+1] = 0;
	   }
	   else if (!o)
	   	  {
	   	  depth--;//bug: can't do this only because we already were here
	   	  //there also must be no other solution to get_recent_road
	   	  }
	   else
	   	  {
	   	  stack[depth+1]=o;//gotta find next solution
	   	  }

	   } while (depth>=0 && stack[depth]!=dest && (depth+1)<stack_size);

	   for (int i=0; i<num_visited; i++)
	   visited[i]->set_flag(g1_object_class::SCRATCH_BIT,0);

	   if (depth<0)
	   	{
	   	i4_warning("WARNING: Could not find a way there.");
	   	depth=0;
	   	}

	   if (depth)
	   {
	   while (stack[depth] && depth<stack_size-2) // whatsthis? && stack[depth]->total_links(type)>0)
	   {
	   	g1_path_object_class *o = stack[depth]->get_recent_road(team, 0);
	   	stack[++depth] = o;
	   }

	   stack[++depth]=0;
	   }
	   stack[++depth]=0;

	   for (int j=0; j<depth; j++)
	   if (!stack[j])
	   	return j;
	 */
	//return depth;

}

void g1_road_object_class::think()
{

}

i4_bool g1_road_object_class::build(int type)
{
	return i4_F;
}

int g1_road_object_class::find_path(g1_team_type team,
									g1_path_object_class * * stack,
									int stack_size)
{
	//return the recently used path, but don't return any path loops
	int t=0,cl;

	g1_path_object_class * o=this,* next;

	//int numlinks=0;
	do
	{
		stack[t++]=o;
		//numlinks++;
		if (o)
		{
			o->set_flag(SCRATCH_BIT,1);
			//finds any link, but the most recently selected of this team
			next=0;
			cl=o->total_links();
			do
			{
				next=o->get_recent_link(team, next);
				//next->set_flag(SCRATCH_BIT,1);
				cl--;
				if (cl<0)
				{
					next=0;
				}            //no more usable paths
			}
			while (next && next->get_flag(SCRATCH_BIT));
			o=next;

		}

	}
	while (o && t<(stack_size-1));
	int th=0;
	for (; th<t; th++)
	{
		stack[th]->set_flag(SCRATCH_BIT,0); //reset scratch bits
	}

	//Can only go in ally-direction here (all vehicles will)
	//the maximum way length is stack_size (is almost always fully used here,
	//as we do not indicate an end of the route.)
	//I4_ASSERT(t<stack_size, "ERROR: Path seems to be too long.");

	return t;

}

void g1_road_object_class::request_remove()
{
	g1_path_object_class::request_remove();
	g2_node_man()->remove_node(nid);
}

g1_road_object_class::~g1_road_object_class()
{
}

/*static int compare_road_nodes(const g1_road_object_class::link_class *a,
   							  const g1_road_object_class::link_class *b)
   {
   // smallest length last
   link *la=g2_link_man()->get_link(a->link_desc);
   link *lb=g2_link_man()->get_link(b->link_desc);
   if (lb->length >
   	  la->length)
   	return 1;
   else if (lb->length <
   	  la->length)
   	return -1;
   else
   	return 0;
   }*/

int road_compare_nodes(const g2_breadth_first_road_solver_class::solve_node * a,
					   const g2_breadth_first_road_solver_class::solve_node * b)
{
	// smallest length last
	if (b->length > a->length)
	{
		return 1;
	}
	else if (b->length < a->length)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

i4_bool g2_breadth_first_road_solver_class::add_node(g1_road_object_class * node, g1_road_object_class * from,
													 i4_float len)
{
	if (node->ref && node->rlen <len)
	{
		// found better one already
		return i4_F;
	}

	// store current path graph
	node->ref = from;
	node->rlen = len;

	// add into heap
	w32 loc;

	solve_node test(node,len);
	if (heap.size())
	{
		i4_bsearch(&test, loc, &heap[0], (w32)heap.size(), road_compare_nodes);
		heap.add_at(test, loc);
	}
	else
	{
		heap.add(test);
	}

	return i4_T;
}


i4_bool g2_breadth_first_road_solver_class::get_next_node(g1_road_object_class *&node, i4_float &len)
{
	w32 loc;

	if ((loc=heap.size())==0)
	{
		// nothing left in heap
		return i4_F;
	}

	// get last node (shortest) & remove
	loc--;
	node = heap[loc].nref;
	len = heap[loc].length;
	heap.remove(loc);

	return i4_T;
};

int g2_breadth_first_road_solver_class::path_solve(g1_team_type team, g1_road_object_class * start, g1_road_object_class * dest,
												   g1_path_object_class * * path, w32 stack_size)
{
	g1_road_object_class * node;
	i4_float len; //actually we now use exspected time

	clear_heap();
	clear_solve();

	if (!start || !dest)
	{
		return i4_F;
	}

	add_node(start, 0, 0);
	link_manager * lman=g2_link_man();
	len=0;
	w32 starttime=(w32)g2_act_man()->daytime;
	while (get_next_node(node, len))
	{
		//g1_road_object_class *c =0;
		for (int i=0; i<node->total_links(G1_ALLY); i++) //ally means outgoing
		{
			g2_link * l=lman->get_link(node->get_link_id(G1_ALLY,i));
			i4_float extime=l->quratio(starttime,len);
			//i4_float extime=l->get_length()/l->get_freespeed();
			add_node((g1_road_object_class *)node->get_link(G1_ALLY,i), node,
					 len + extime);
			//else we skip.
		}
	}

	w32 points=0;

	if (dest->ref==0)
	{
		return 0;
	}

	// count nodes
	node = dest;
	points = 0;
	while (node!=start&&points<(stack_size))
	{
		/*point[points*2+0] = i4_float(graph->critical[node].x)+0.5f;
		   point[points*2+1] = i4_float(graph->critical[node].y)+0.5f;
		   points++;
		   node = solve_graph[node].ref;*/
		path[points++]=node;
		node=node->ref;
	}
	path[points++]=start; //it seems paths need this entry to work correctly

	//point[points*2+0] = i4_float(graph->critical[start_node].x)+0.5f;
	//point[points*2+1] = i4_float(graph->critical[start_node].y)+0.5f;
	//points++;

	return points;
}

void g2_breadth_first_road_solver_class::clear_solve()
{
	g1_object_class * olist[G1_MAX_OBJECTS];
	w32 num=g1_get_map()->make_object_list(olist,G1_MAX_OBJECTS);
	w32 road_id=g1_get_object_type("road_object");

	for (int i=0; i<num; i++)
	{
		if (olist[i]->id==road_id)
		{
			((g1_road_object_class *)olist[i])->ref=0;
		}
	}
};

g2_breadth_first_road_solver_class::~g2_breadth_first_road_solver_class()
{
};

int road_compare_weighted_nodes(const g2_breadth_first_road_solver_class::solve_node * a,
								const g2_breadth_first_road_solver_class::solve_node * b)
{
	// smallest length last
	if (b->weight > a->weight)
	{
		return 1;
	}
	else if (b->weight < a->weight)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

i4_bool g2_astar_road_solver_class::add_weighted_node(g1_road_object_class * node, g1_road_object_class * from,
													  i4_float len)
{
	if (node->ref && node->rlen <len)
	{
		// found better one already
		return i4_F;
	}

	// store current path graph
	node->ref = from;
	node->rlen = len;

	// add into heap
	w32 loc;
	//doesn't work, would need to have the real, not the map coordinates
	//and an estimate of the allowed travel speed on that route.
	//i4_float d=(node->x-destx)*(node->x-destx)+(node->y-desty)*(node->y-desty);
	//i4_float w=len+sqrt(d);

	//this is a best-first estimate.
	i4_float w=(node->x-destx)*(node->x-destx)+(node->y-desty)*(node->y-desty);
	solve_node test(node,len,w);
	if (heap.size())
	{
		i4_bsearch(&test, loc, &heap[0], (w32)heap.size(), road_compare_weighted_nodes);
		heap.add_at(test, loc);
	}
	else
	{
		heap.add(test);
	}

	return i4_T;
}


i4_bool g2_astar_road_solver_class::get_next_weighted_node(g1_road_object_class *&node, i4_float &len)
{
	w32 loc;

	if ((loc=heap.size())==0)
	{
		// nothing left in heap
		return i4_F;
	}

	// get last node (shortest) & remove
	loc--;
	//if (heap[loc].weight=0)
	//	  return i4_F;
	node = heap[loc].nref;
	len = heap[loc].length;
	heap.remove(loc);

	return i4_T;
};

int g2_astar_road_solver_class::path_solve(g1_team_type team, g1_road_object_class * start, g1_road_object_class * dest,
										   g1_path_object_class * * path, w32 stack_size)
{
	g1_road_object_class * node;
	i4_float len; //actually we now use exspected time

	clear_heap();
	clear_solve();

	if (!start || !dest)
	{
		return i4_F;
	}

	destx=dest->x;
	desty=dest->y;
	add_node(start, 0, 0);
	link_manager * lman=g2_link_man();
	len=0;
	w32 starttime=(w32)g2_act_man()->daytime;
	while (get_next_node(node, len))
	{
		//g1_road_object_class *c =0;
		if (node==dest)
		{
			break;
		}
		for (int i=0; i<node->total_links(G1_ALLY); i++) //ally means outgoing
		{
			g2_link * l=lman->get_link(node->get_link_id(G1_ALLY,i));
			i4_float extime=l->quratio(starttime,len);
			//i4_float extime=l->get_length()/l->get_freespeed();
			add_node((g1_road_object_class *)node->get_link(G1_ALLY,i), node,
					 len + extime);
			//else we skip.
		}
	}

	w32 points=0;

	if (dest->ref==0)
	{
		return 0;
	}

	// count nodes
	node = dest;
	points = 0;
	while (node!=start&&points<(stack_size))
	{
		/*point[points*2+0] = i4_float(graph->critical[node].x)+0.5f;
		   point[points*2+1] = i4_float(graph->critical[node].y)+0.5f;
		   points++;
		   node = solve_graph[node].ref;*/
		path[points++]=node;
		node=node->ref;
	}
	path[points++]=start; //it seems paths need this entry to work correctly

	//point[points*2+0] = i4_float(graph->critical[start_node].x)+0.5f;
	//point[points*2+1] = i4_float(graph->critical[start_node].y)+0.5f;
	//points++;

	return points;
}
