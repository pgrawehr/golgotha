//this file is (c) 2002 Patrick Grawehr

//#include "pch.h"
#ifndef TRANSPORT__TRANSPORT_H
#define TRANSPORT__TRANSPORT_H
#include "memory/array.h"
#include "memory/hashtable.h"
#include "string/string.h"
#include "file/file.h"
#include "g1_object.h"
#include "reference.h"
//#include "memory/stlinc.h"
#include <math.h>
#include "map_singleton.h"
#include "transport/randomize.h"


typedef w32 node_id;
typedef w32 link_id;
#include "objs/car.h"

//extern i4_array<float> quality_values(0,100);
extern i4_bool Nodes_Active;//true iff the nodes and links are
//connected to the map.
class node
	{
	friend class node_manager;
	protected:
		node_id id;
		float x,y,h;
		i4_array<link_id> in_links;
		i4_array<link_id> out_links;
	public:
		node_id get_id(){return id;};
		void get_pos(float &_x, float &_y){_x=x;_y=y;};
		float xpos(){return x;};
		float ypos(){return y;};
		node(w32 _id,float _x, float _y, float _h):id(_id),x(_x),y(_y),h(_h),
			in_links(10,10),out_links(10,10){};
		void add_in_link(link_id lid)
			{
			in_links.add(lid);
			}
		void add_out_link(link_id lid)
			{
			out_links.add(lid);
			}
		void remove_in_link(link_id lid)
			{in_links.remove(lid);}
		void remove_out_link(link_id lid)
			{out_links.remove(lid);}
		void remove_link(link_id lid)
			{
			in_links.remove(lid);//just try
			out_links.remove(lid);
			}
	};

class waiting_car
	{
	public:
	int start_time;//the time we are supposed to start
	link_id start_loc,end_loc;//the way we are supposed to go
	};

#define TTIMES_SAMPLE_TIME (15*60) //how precise do we record congestion? 
								  //-> in seconds.
#define START_TIME_BIN_SIZE (5*60)
class g2_link_quality
	{
	public:
		float start;//at this time, this sample starts
		w16 samples;//this many cars complained about this road
		float acctimes;//accumulated ttimes for today
		float quratio;//factor the length should be multiplied with for routing
	};

class g2_link
	{
	friend class link_manager;
	friend class node_manager;
	protected:
		link_id id;
		node_id from;
		node_id to;
		float length;
		float freespeed;
		float capacity;
	public:
		i4_array<g2_link_quality> quality;//pointer to table with actual speeds on heavy used links
		//depends on time
		//get the id of the link
		link_id get_id(){return id;};

		//returns the length in meters of the link
		float get_length(){return length;}

		//gets the highest allowed speed in m/s.
		//Hint: We assume everybody follows the speedlimits.
		float get_freespeed(){return freespeed;}

		//get the capacity (maximum number of cars per hour allowed)
		float get_capacity(){return capacity;}

		//Tell the link that he is bad at daytime.
		//I've expected to use extime for this link but
		//I used usedtime.
		void mark_bad(double daytime, double extime, double usedtime);

		//return the factor the length of the link should be multiplied
		//with for the given starttime. We'll take an estimate of
		//the time we pass this link from the pathlen and the freespeed
		i4_float quratio(double starttime,i4_float pathlen);

		node_id get_from(){return from;}
		node_id get_to(){return to;}
		g2_link(link_id _id,node_id _from, node_id _to,float _length,
			float _freespeed,float _capacity)
			:id(_id),from(_from),to(_to),length(_length),
			freespeed(_freespeed),capacity(_capacity),quality(0,10)
			{};
	};

typedef g1_typed_reference_class<g1_object_class> obj_ref_t;
class node_manager
	{
	protected:
	i4_hashtable<void> nodes;//actually quite a hack... but saves memory
	//i4_hashtable<obj_ref_t> road_nodes;
	public:
		i4_bool use_active;
		double minx,miny,maxx,maxy;
		double scalingx,scalingy,offsetx,offsety;
		node *get_node(node_id id);
		obj_ref_t *get_obj(node_id id);
		node *add_node(node *newnode);
		i4_bool load_nodes(const i4_const_str &filename);
		i4_bool load_nodes(i4_file_class *fp);
		i4_bool save_nodes(i4_file_class *fp);
		i4_bool save_nodes(const i4_const_str &filename);
		void remove_node(node *rem);
		void remove_node(node_id nid);
		void reset();
		void activate(g1_map_class *map);
		w32 num_nodes(){return nodes.entries();}
		void collect();//collect all nodes on the map
		node_manager();
		~node_manager();
		

	};

class link_manager
	{
	friend class node_manager;
	friend class act_manager;
	protected:
		node_manager *nman;
		i4_hashtable<g2_link> links;
	public:
		g2_link *get_link(link_id id);
		g2_link *add_link(g2_link* n);
		void remove_link(g2_link *l);
		link_manager(node_manager *_nman):nman(_nman),links(4000,0){};
		i4_bool load_links(const i4_const_str &filename,g1_loader_class *mainfp);
		i4_bool load_links(i4_file_class *fp);
		i4_bool save_links(const i4_const_str &filename,g1_saver_class *mainfp);
		i4_bool save_links(i4_file_class *fp);
		i4_bool save_qualitydata(g1_saver_class *fp);
		i4_bool load_qualitydata(g1_loader_class *fp);
		void reset();
		~link_manager();
	
	};

#define TRANSIMS_DATA_VERSION 1
#define TRANSIMS_DATA_STRING "TRANSIMS_SIMULATION_DATA"

class act_manager:public g2_singleton

	{
	public:
		double daytime;//we don't know the scaling yet.
		w32 day;
		g2_scramble_thinkers scramble;
		w32 maxwindowsize,curwindowsize;
		i4_isl_list<g2_car_object_class>::iterator nextstart,oldestwaiting;
	act_manager();
	//i4_array<waiting_car> cars; //solve with global i4_isl_list
	i4_bool load_activities(const i4_const_str &filename);
	i4_bool load_activities(i4_file_class *fp);
	void think();
	i4_bool save(g1_saver_class *fp);
	i4_bool load(g1_loader_class *fp);
	void sort_car_list(int (*sortfun)(g2_car_object_class* const *a,g2_car_object_class* const *b));
	};

node_manager *g2_node_man();
link_manager *g2_link_man();
act_manager *g2_act_man();
act_manager *g2_new_act_man();

#endif
