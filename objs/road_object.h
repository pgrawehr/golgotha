//a revival file

#ifndef OBJS__ROAD_OBJECT_H
#define OBJS__ROAD_OBJECT_H
#include "g1_object.h"
#include "objs/path_object.h"
#include "solvegraph.h"

class g1_road_object_class;

//well, actually this is a dijkstra solver, not a breadth first solver.
class g2_breadth_first_road_solver_class :
	public g1_graph_solver_class,
	public i4_init_class
{
public:
	class solve_node
	{
public:
		g1_road_object_class * nref;
		i4_float length,weight;

		solve_node()
		{
		}
		//for dy
		solve_node(g1_road_object_class * node, i4_float length) :
			nref(node),
			length(length)
		{
			weight=length;
		}
		solve_node(g1_road_object_class * node, i4_float length, i4_float weight) :
			nref(node),
			length(length),
			weight(weight)
		{
		}
		solve_node& operator=(const solve_node &a)
		{
			nref=a.nref;
			length=a.length;
			weight=a.weight;
			return *this;
		}
	};

protected:
	//g1_critical_graph_class *graph;


	//solve_node *solve_graph;
	w32 nodes;

	// Breadth first heap
	i4_array<solve_node> heap;

	virtual void clear_heap()
	{
		heap.clear();
	}
	void init()
	{
		nodes=0;
	}

	void uninit()
	{
		heap.uninit();
	};
	i4_bool add_node(g1_road_object_class * node, g1_road_object_class * from, i4_float len);
	i4_bool get_next_node(g1_road_object_class *&node, i4_float &len);

	void clear_solve(); //reset rlen and ref of all road objects

public:

	g2_breadth_first_road_solver_class() :
		heap(0,200)
	{
	}
	//g1_breadth_first_graph_solver_class(g1_critical_graph_class *_graph)
	//  : heap(100,20), solve_graph(0) { set_graph(_graph); }
	~g2_breadth_first_road_solver_class();

	void set_graph(g1_critical_graph_class * _graph)
	{
	};
	i4_bool path_solve_nodes(g1_graph_node start_node, g1_graph_node end_node,
							 w8 group_size, w8 grade,
							 i4_float * point, w16 &points)
	{
		points=0;
		return i4_F;
	};                        //this solver is not applicable like this

	virtual int path_solve(g1_team_type team, g1_road_object_class * start, g1_road_object_class * dest,
						   g1_path_object_class * * path, w32 stack_size);

};

class g2_astar_road_solver_class :
	public g2_breadth_first_road_solver_class
{
protected:
	i4_float destx,desty;
	//i4_array<solve_node> closed_heap;
	/*
	   virtual void clear_heap()
	   	{
	   	closed_heap.clear();
	   	g2_breadth_first_road_solver_class::clear_heap();
	   	}
	 */
public:
	/*
	   void init()
	   	{
	   	g2_breadth_first_road_solver_class::init();
	   	}
	   void uninit()
	   	{
	   	closed_heap.uninit();
	   	g2_breadth_first_road_solver_class::uninit();
	   	}
	 */
	g2_astar_road_solver_class()
	{
	};
	i4_bool add_weighted_node(g1_road_object_class * node, g1_road_object_class * from, i4_float len);
	i4_bool get_next_weighted_node(g1_road_object_class *&node, i4_float &len);
	virtual int path_solve(g1_team_type team, g1_road_object_class * start, g1_road_object_class * dest,
						   g1_path_object_class * * path, w32 stack_size);
};


class g1_road_object_class :
	public g1_path_object_class
{
protected:
	w32 last_path_tick;
	g1_typed_reference_class<g1_path_object_class> last_dest;
public:
	friend class node_manager;
	friend class link_manager;
	i4_float rlen; //for solver
	g1_road_object_class * ref; //dito
	//The path-finding is a bit different on these roads
	//than on usual ones
	int find_path(g1_team_type type, g1_path_object_class * * stack, int stack_size);
	int find_path(g1_team_type type, g1_path_object_class * dest,
				  g1_path_object_class * * stack, int stack_size); //from this to dest
	virtual void draw(g1_draw_context_class * context, i4_3d_vector& viewer_position); //with (outgoing) links
	void calc_world_transform(i4_float ratio, i4_transform_class * transform=0)
	{
	};     //not needed for these objects
	void editor_draw(g1_draw_context_class * context); //same
	g1_road_object_class(g1_object_type id, g1_loader_class * fp);
	~g1_road_object_class();
	void save(g1_saver_class * fp);
	//i4_float occupancy_radius() const;//with all neighbours
	i4_bool occupy_location(); //such that the graphics fits.
	void setup(node_id thisid, i4_float x, i4_float y, i4_float h);
	void request_remove();
	void think(); //inserts cars on nearby links
	i4_bool build(int type); //requests a build at this node
	node_id nid;
	//class ext_link_class:public g1_path_object_class::link_class
	//	{
	//	public:
	//		link *linkdesc;
	//	}
};

#endif
