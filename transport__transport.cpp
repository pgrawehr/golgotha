#include "pch.h"
#include "transport/transport.h"
#include "init/init.h"
#include "main/main.h"
//#include <conio.h>
#include "lisp/li_all.h"
#include "map.h"
#include "map_man.h"
#include "map_vars.h"
#include "window/window.h"
#include "window/style.h"
#include "gui/smp_dial.h"
#include "init/init.h"
#include "objs/car.h"
#include "remove_man.h"
#include "resources.h"

node_manager * node_man;
link_manager * link_man;
act_manager * act_man;
//i4_array<float> quality_values(0,100);

node_manager *g2_node_man()
{
	return node_man;
}
link_manager *g2_link_man()
{
	return link_man;
};

act_manager *g2_act_man()
{
	return act_man;
};

act_manager *g2_new_act_man()
{
	act_man=new act_manager();
	return act_man;
}


class transport_initer :
	public i4_init_class
{
public:
	void init()
	{
		node_man=0;
		link_man=0;
	}
	void uninit()
	{
		delete node_man;
		delete link_man;
		node_man=0;
		link_man=0;
	}

} transport_initer_instance;

//void *i4_stack_base=0;
//HINSTANCE i4_win32_instance=0;
/*int main(int argc, char **argv)
   	{
   	long t=0;
   	i4_stack_base= (void*) (&t);
   	i4_init();
   	node_man=new node_manager();
   	node_man->load_nodes("TNodes.dat");
   	link_man=new link_manager(node_man);
   	link_man->load_links("TLinks.dat");
   	//scanf("\n");
   	_getch();
   	delete link_man;
   	delete node_man;
   	i4_uninit();
   	return 0;
   	}
 */

//todo: Add a function that reinitializes node manager, act manager
//and link manager on an existing map.

/*LISPFUN*
   Name: load_from_transims
   Syntax: (load_from_transims)
   Purpose: (Internal) Loads the traffic sim files (nodes, links, activities)
   Returns: t or 0
 */



LI_HEADER(load_from_transims) {
	if (!g1_map_is_loaded())
	{
		return 0;
	}
	li_string_class_member nodes("traffic_sim_nodes");
	li_string_class_member links("traffic_sim_links");
	li_string_class_member veh("traffic_sim_vehicles");
	li_string_class_member acts("traffic_sim_acts");
	li_symbol_class_member can_load("traffic_sim");
	li_class_context ctx(g1_map_vars.vars());
	if (can_load()==li_nil)
	{
		if (o)
		{
			return 0;
		}               //any parameter means non-interactive
		w32 ans=i4_message_box("Missing data", "Please fill in the level-vars form and choose \"traffic_sim\". Go there?",MSG_YES+MSG_NO);
		if (ans==MSG_YES)
		{
			li_call("edit_level_vars");
		}
		return 0;

	}
	/*if (node_man)
	   	delete node_man;
	   if (link_man)
	   	delete link_man;
	   node_man=0;
	   link_man=0;*/
	li_call("unload_transims");
	li_call("remove_all_paths");
	node_man=new node_manager();
	if (!node_man->load_nodes(nodes()))
	{
		i4_error("ERROR: Cannot load %O as nodes. ",nodes());
		delete node_man;
		node_man=0;
		return 0;
	}
	link_man=new link_manager(node_man);
	if (!link_man->load_links(links(),0))
	{
		i4_error("ERROR: Cannot load %O as links. ",links());
		delete node_man;
		delete link_man;
		node_man=0;
		link_man=0;
		return 0;
	}

	node_man->activate(g1_get_map());
	act_man=new act_manager();
	if (!act_man->load_activities(acts()))
	{
		i4_error("ERROR: Cannot load %O as activities. ", acts());
		return li_nil;
	}
	return li_true_sym;

}


/*LISPFUN*
   Name: unload_transims
   Syntax: (unload_transims)
   Purpose: (Internal) Remove the objects used for traffic sim.
   Returns: nil
 */



LI_HEADER(unload_transims) {
	i4_isl_list<g2_car_object_class>::iterator i=g2_car_object_list.begin(),j;
	if (i!=0)
	{
		j=i;
		i++;
		while(i!=g2_car_object_list.end()&&j!=g2_car_object_list.end())
		{
			j->unoccupy_location(); //removing i.th entry would destroy the iterator
			j->request_remove();
			j=i;
			i++;
		}
		j->unoccupy_location(); //remove last element
		j->request_remove();
	}
	//i=g2_car_object_list.begin();
	//i->unoccupy_location();//and first (previously skipped)
	//i->request_remove();
	g1_remove_man.process_requests();
	delete node_man;
	delete link_man;
	delete act_man;
	node_man=0;
	link_man=0;
	act_man=0;
	return li_nil;
}

/*LISPFUN*
   Name: adjust_lod_dist
   Syntax: (adjust_lod_dist [disappear_dist [switch_dist [extra_disappear_dist]]])
   Returns: t
 */

LI_HEADER(adjust_lod_dist) {
	li_object * ob=li_car(o,env);

	if (ob)
	{
		g1_resources.lod_disappear_dist=li_get_float(li_eval(ob,env),env);
	}
	ob=li_second(o,env);
	if (ob)
	{
		g1_resources.lod_switch_dist=li_get_float(li_eval(ob,env),env);
	}
	ob=li_third(o,env);
	if (ob)
	{
		g1_resources.lod_nolodmodel_disappear_dist=li_get_float(li_eval(ob,env),env);
	}
	else
	{
		g1_resources.lod_nolodmodel_disappear_dist=g1_resources.lod_disappear_dist*5;
	}
	return li_true_sym;
}

li_automatic_add_function(li_load_from_transims,"load_from_transims");
li_automatic_add_function(li_unload_transims,"unload_transims");
li_automatic_add_function(li_adjust_lod_dist,"adjust_lod_dist");
