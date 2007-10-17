/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_SOLVEMAP_HH
#define G1_SOLVEMAP_HH

#include "arch.h"
#include "math/num_type.h"

#define BLOCK_EASY_WAY 1
#define BLOCK_NO_WAY 0
#define BLOCK_HARD_WAY 2

class g1_block_map_class;
class g1_map_solver_class
{
public:
	enum {
		INVALID_VIRTUAL_BASE=0,
		IS_MAP_SOLVER=1, //exactly one of these 2 bits
		IS_GRAPH_SOLVER=2, //must always be set!
		IS_ASTAR_MAP_SOLVER=4,
		IS_GRAPH_SOLVER1,
		IS_BREADTH_SOLVER
	};
	g1_map_solver_class()
	{
		flags=INVALID_VIRTUAL_BASE;
	};
	virtual ~g1_map_solver_class()
	{
	};
	w32 flags;
	virtual void set_block_map(g1_block_map_class * _block) = 0;
	//finds a path from start to dest
	virtual i4_bool path_solve(i4_float startx, i4_float starty, i4_float destx, i4_float desty,
							   w8 sizex, w8 sizey, w8 grade,
							   i4_float * point, w16 &points) = 0;
	//is there a straigth way to the destination?
	static w32 unblocked(g1_block_map_class * block, i4_float startx, i4_float starty, i4_float destx, i4_float desty);
	static g1_map_solver_class *cast(g1_map_solver_class * sc)
	{
		//although this is the base itself, it should not be considered
		//as the implementation of the map solver.
		if (sc->flags&IS_MAP_SOLVER)
		{
			return sc;
		}
		else
		{
			return 0;
		}
	}
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
