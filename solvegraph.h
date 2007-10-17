/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/



#ifndef G1_SOLVEGRAPH_HH
#define G1_SOLVEGRAPH_HH

#include "critical_graph.h"
#include "math/num_type.h"
#include "solvemap.h"

class g1_graph_solver_class :
	public g1_map_solver_class
{
public:
	g1_graph_solver_class()
	{
		flags=IS_GRAPH_SOLVER;
	}

	virtual void set_graph(g1_critical_graph_class * _graph) = 0;
	void set_block_map(g1_block_map_class * _block)
	{
		i4_error("INTERNAL: Attempt to assign a block map to a graph solver");
	}
	virtual i4_bool path_solve_nodes(g1_graph_node start_node, g1_graph_node end_node,
									 w8 group_size, w8 grade,
									 i4_float * point, w16 &points) = 0;
	i4_bool path_solve(i4_float startx, i4_float starty, i4_float destx, i4_float desty,
					   w8 sizex, w8 sizey, w8 grade,
					   i4_float * point, w16 &points);
	static g1_graph_solver_class *cast(g1_map_solver_class * sc)
	{
		if (sc->flags&IS_GRAPH_SOLVER)
		{
			return (g1_graph_solver_class *) sc;
		}
		else
		{
			return 0;
		}
	}
};

#endif
