/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h"
#include "solvegraph_breadth.h"
#include "search.h"
#include "editor/dialogs/path_win.h"
#include "map_man.h"
#include "map.h"
#include "map_cell.h"

void g1_breadth_first_graph_solver_class::set_graph(g1_critical_graph_class * _graph)
{
	graph=_graph;
	if (!graph || !solve_graph || graph->criticals!=nodes)
	{
		if (solve_graph)
		{
			i4_free(solve_graph);
		}

		nodes = graph->criticals;
		solve_graph = (solve_node *)I4_MALLOC(nodes*sizeof(solve_node), "solve_graph");
	}
}

g1_breadth_first_graph_solver_class::~g1_breadth_first_graph_solver_class()
{
	if (solve_graph)
	{
		i4_free(solve_graph);
	}
}


int compare_nodes(const g1_breadth_first_graph_solver_class::solve_node * a,
				  const g1_breadth_first_graph_solver_class::solve_node * b)
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

i4_bool g1_breadth_first_graph_solver_class::add_node(g1_graph_node node, g1_graph_node from,
													  i4_float len)
{
	if (solve_graph[node].ref && solve_graph[node].length<len)
	{
		// found better one already
		return i4_F;
	}

	// store current path graph
	solve_graph[node].ref = from;
	solve_graph[node].length = len;

	// add into heap
	w32 loc;
	solve_node test(node,len);

	if (heap.size())
	{
		i4_bsearch(&test, loc, &heap[0], (w32)heap.size(), compare_nodes);
		heap.add_at(test, loc);
	}
	else
	{
		heap.add(test);
	}

	return i4_T;
}

i4_bool g1_breadth_first_graph_solver_class::get_next_node(g1_graph_node &node, i4_float &len)
{
	w32 loc;

	if ((loc=heap.size())==0)
	{
		// nothing left in heap
		return i4_F;
	}

	// get last node (shortest) & remove
	loc--;
	node = heap[loc].ref;
	len = heap[loc].length;
	heap.remove(loc);

	return i4_T;
}

i4_bool g1_breadth_first_graph_solver_class::path_solve_nodes(g1_graph_node start_node,
															  g1_graph_node end_node,
															  w8 group_size, w8 grade,
															  i4_float * point, w16 &points)
{
	g1_graph_node node;
	i4_float len;

	clear_heap();
	clear_solve();

	if (!start_node || !end_node)
	{
		return i4_F;
	}

	add_node(start_node, 0, 0);

	while (get_next_node(node, len))
	{
		g1_critical_graph_class::connection_class * c = graph->critical[node].connection;
		for (int i=0; i<graph->critical[node].connections; i++, c++)
		{
			if (group_size<=c->size[grade])
			{
				add_node(c->ref, node, len + c->dist);
			}
		}
	}

	points=0;

	if (solve_graph[end_node].ref==0)
	{
		return i4_F;
	}

	// count nodes
	node = end_node;
	points = 1; //the first entry will be the destination.
	//it will be modified later
	point[0]=graph->critical[end_node].x;
	point[1]=graph->critical[end_node].y;
	while (node!=start_node&&points<(MAX_SOLVEPOINTS-8))
	{
		point[points*2+0] = i4_float(graph->critical[node].x)+0.5f;
		point[points*2+1] = i4_float(graph->critical[node].y)+0.5f;
		points++;
		node = solve_graph[node].ref;
	}
	//point[points*2+0] = i4_float(graph->critical[start_node].x)+0.5f;
	//point[points*2+1] = i4_float(graph->critical[start_node].y)+0.5f;
	//points++;

	return i4_T;
}

i4_bool g1_graph_solver_class::path_solve(i4_float startx, i4_float starty, i4_float destx, i4_float desty,
										  w8 sizex, w8 sizey, w8 grade,
										  i4_float * point, w16 &points)
{
	g1_block_map_class * block=g1_get_map()->get_block_map(grade);
	w32 block_type;

	block_type=unblocked(block, startx,starty,destx,desty);
	if (block_type==BLOCK_NO_WAY)
	{
		points=0;
		return i4_F;
	}
	if (block_type==BLOCK_EASY_WAY)
	{
		point[2]=startx;
		point[3]=starty;
		point[0]=destx;
		point[1]=desty;
		points=2;
		return i4_T;
	}
	const w8 tofrom=0;
	g1_graph_node start_node = g1_get_map()->cell(startx,starty)->nearest_critical[grade][tofrom][0];
	g1_graph_node dest_node = g1_get_map()->cell(destx,desty)->nearest_critical[grade][tofrom][0];
	i4_bool ret=path_solve_nodes(start_node, dest_node, sizex>=sizey ? sizex : sizey, grade, point, points);
	if (ret==i4_F)
	{
		return i4_F;
	}
	point[0]=destx; //exact destination location
	point[1]=desty;
	point[points*2]=  startx;
	point[points*2+1]=starty;
	points++;
	return i4_T;
}
