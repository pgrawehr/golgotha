/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h"
#include <math.h>

#include "memory/malloc.h"
#include "error/error.h"
#include "map.h"
#include "map_man.h"
#include "time/profile.h"
#include "solvemap_astar.h"
#include "math/trig.h"
#include "math/pi.h"
#include "math/angle.h"
#include "memory/bitarray2d.h"

i4_profile_class pf_solve_astar("solve_astar");

const i4_float sqrt2 = (float)sqrt(2.0);
static g1_map_class *map_ref; //for faster reference
static BitArray2D *visited_array=0;

//enum { VISITED = g1_map_cell_class::VISITED, OK = g1_map_cell_class::ROUTEOK };

inline void visit(w16 x, w16 y)
{
	//map_ref->cell(x,y)->flags |= g1_map_cell_class::VISITED;
	visited_array->SetBit(x,y);
}

inline void ok(w16 x, w16 y)
{
	map_ref->cell(x,y)->flags |= g1_map_cell_class::ROUTEOK;
}

inline i4_bool is_visited(w16 x, w16 y)
{
	//return (map_ref->cell(x,y)->flags & g1_map_cell_class::VISITED)!=0;
	return visited_array->IsBitSet(x,y);
}

inline i4_bool is_ok(w16 x, w16 y)
{
	return (map_ref->cell(x,y)->flags& g1_map_cell_class::ROUTEOK)!=0;
}

inline void set_solve_link(w16 x, w16 y, w16 from_x, w16 from_y,i4_float f_length)
{
	g1_map_cell_class *c = map_ref->cell(x,y);

	c->scratch_x = from_x;
	c->scratch_y = from_y;
	c->scratch_len=f_length;
}

inline void solve_link(w16 x, w16 y, w16 &from_x, w16 &from_y, i4_float &f_length)
{
	g1_map_cell_class *c = map_ref->cell(x,y);

	from_x = c->scratch_x;
	from_y = c->scratch_y;
	f_length=c->scratch_len;
}

typedef g1_astar_map_solver_class::cell cell_type;
int cell_compare(const i4_float *a, const cell_type *b)
{
	i4_float ca = *a, cb = b->hint + b->length;
	if (cb<ca)
	{
		return -1;
	}
	else if (cb>ca)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

i4_bool g1_astar_map_solver_class::add_link(w16 from_x,w16 from_y, w16 x,w16 y,
											i4_float from_length)
//{{{
{
	set_solve_link(x,y, from_x, from_y, from_length);
	visit(x,y);

	i4_float
	dx = (float)dest_x - x,
	dy = (float)dest_y - y,
	hint = dx*dx+dy*dy, //this is the distance estimate
	cost = hint + from_length;
	w32 loc;
	cell *p;

	if (heap.size()>0)
	{
		w32 hs = heap.size();
		i4_bsearch(&cost, loc, &heap[0], hs, cell_compare);

		p = heap.add_at(loc);
	}
	else
	{
		p = heap.add();
	}

	p->x = x;
	p->y = y;
	p->length = from_length;
	p->hint = hint;

	return i4_T;
}
//}}}

i4_bool g1_astar_map_solver_class::add_step_link(w16 from_x,w16 from_y, w16 x,w16 y,
												 i4_float from_length,w8 dir)
//{{{
{
	g1_map_class *map = map_ref;
	w16 fx,fy;
	i4_float flen;

	if (x<=1 || x>=map->width()-1 || y<=1 || y>=map->height())
	{
		return i4_F;
	}



	//if (g1_get_map()->cell(x,y)->get_solid_list())
	//  return i4_F;
	if (is_visited(x,y))
	{
		solve_link(x,y,fx,fy,flen);
		if (flen<=from_length)
		{
			return i4_F;
		}
	}

	if (block->is_blocked(x,y,dir))
	{
		return i4_F;
	}

	return add_link(from_x,from_y, x,y, from_length);
}
//}}}

i4_bool g1_astar_map_solver_class::add_path_link(w16 from_x,w16 from_y, w16 x,w16 y,
												 i4_float from_length)
//{{{
{
	//g1_map_class *map = g1_get_map();

	if (x<=1 || x>=map_ref->width()-1 || y<=1 || y>=map_ref->height())
	{
		return i4_F;
	}

	if (is_visited(x,y))
	{
		return i4_F;
	}

	return add_link(from_x,from_y, x,y, from_length);
}
//}}}

i4_bool g1_astar_map_solver_class::get_next_cell(w16 &x,w16 &y, i4_float &length)
//{{{
{
	if (heap.size()==0)
	{
		return i4_F;
	}

	cell *p = &heap[heap.size()-1];
	x = p->x;
	y = p->y;
	length = p->length;

	heap.remove(heap.size()-1);

	return i4_T;
}
//}}}

//This method resets the scratch bits. Perhaps a bit slow if often used?
void g1_astar_map_solver_class::clear_solve()
//{{{
{
	int x=0,y=0;
	g1_map_cell_class *c;
	map_ref=g1_get_map();
	for (y=0; y<map_ref->height(); y++)
	{
		c = map_ref->cell(0,y);
		for (x=0; x<map_ref->width(); x++)
		{
			c->flags &= ~(VISITED|OK);
			//c->flags &= ~OK;//this seems to show the choosen path on the map.
			c++;
		}
	}
}
//}}}

//this array is used to make backward-movements expensive
//compared to using backtracking
const i4_float costadj[8][8]=
{

	{2,1,0.1f,0,0.1f,1,2,3},    //theta=0
	{1,0.1f,0,0.1f,1,2,3,2},    //angle goes counterclockwise
	{0.1f,0,0.1f,1,2,3,2,1},
	{0,0,1,2,3,2,1,0},
	{0,1,2,3,2,1,0,0},
	{1,2,3,2,1,0,0,0},
	{2,3,2,1,0.1f,0,0.1f,1},
	{3,2,1,0.1f,0,0.1f,1,2}

};
/*
   const i4_float costadj[8][8]=
   	{

   		{1,1,0.1,0,0.1,1,1,1},//theta=0
   		{1,0.1,0,0.1,1,1,1,1},
   		{0.1,0,0.1,1,1,1,1,1},
   		{0,0.1,1,1,1,1,1,0.1},
   		{0.1,1,1,1,1,1,0.1,0},
   		{1,1,1,1,1,0.1,0,0.1},
   		{1,1,1,1,0.1,0,0.1,1},
   		{1,1,1,0.1,0,0.1,1,1}

   	};
 */


i4_bool g1_astar_map_solver_class::path_solve(i4_float startx, i4_float starty,
											  i4_float destx, i4_float desty,
											  w8 sizex, w8 sizey, w8 grade,
											  i4_float *point, w16 &points)
//{{{
{
	//I need a fast data structure to do insertions/lookups on a (x,y) key.

	block=g1_get_map()->get_block_map(grade);
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
	pf_solve_astar.start();
	map_ref=g1_get_map();
	visited_array=new BitArray2D(map_ref->width(),map_ref->height());
	visited_array->Clear();
	w16 x = (w16)startx,y = (w16)starty;
	i4_float l;

	dest_x = (w16)destx;
	dest_y = (w16)desty;

	w8 costindex;
	i4_float xdiff=destx-startx;
	i4_float ydiff=desty-starty, th;
	th=i4_atan2(ydiff,xdiff)+i4_pi_8();
	i4_normalize_angle(th);
	costindex=(w8)((th/i4_2pi())*8);

	clear_heap();
	//clear_solve();

	add_link(x, y, x, y, 0);

	i4_bool found=i4_F;

	while (!found && get_next_cell(x,y,l))
	{
		if (x==dest_x && y==dest_y)
		{
			found = i4_T;
			break;
		}
		//this order is not random! takes cache aspects into account
		add_step_link(x,y, x-1,y-1, l+2, G1_NORTH|G1_EAST); //we use the square of the real distance everywhere
		add_step_link(x,y, x,y-1, l+1, G1_NORTH);
		add_step_link(x,y, x+1,y-1, l+2, G1_NORTH|G1_WEST);

		add_step_link(x,y, x-1,y, l+1, G1_EAST);
		add_step_link(x,y, x+1,y, l+1, G1_WEST);

		add_step_link(x,y, x-1,y+1, l+2, G1_EAST|G1_SOUTH);
		add_step_link(x,y, x,y+1, l+1, G1_SOUTH);
		add_step_link(x,y, x+1,y+1, l+2, G1_WEST|G1_SOUTH);


		//g1_object_chain_class *c = map_ref->cell(x,y)->get_obj_list();
	}

	if (!found)
	{
		pf_solve_astar.stop();
		delete visited_array;
		visited_array=0;
		return i4_F;
	}

	w16 nx=(w16) dest_x,ny=(w16)dest_y;
	sw16 dx=0,dy=0;
	i4_float shift_x=destx-dest_x;
	i4_float shift_y=desty-dest_y;
	//x = nx+1;
	//y = ny+1;
	i4_float waylen=0;
	solve_link(nx,ny,x,y,waylen);
	sw16 dx2= nx-x,dy2= ny-y;
	x=nx-dx2;
	y=ny-dy2;
	//i4_float dropout;
	points = 0;
	w16 lastupdate=0,len=0;
	while (x!=nx || y!=ny)
	{
		//ok(nx,ny);
		/*if (nx-x==dx && ny-y==dy)
		   points--;
		   dx = nx-x;
		   dy = ny-y;
		   x = nx; y = ny;
		   point[points*2]=   (i4_float)x+shift_x;
		   point[points*2+1]= (i4_float)y+shift_y;
		   points++;*/
		if (nx-x!=dx || ny-y!= dy)
		{
			//if (lastupdate<(len-1))
			//	{
			point[points*2]=(i4_float)nx+shift_x;
			point[points*2+1]=(i4_float)ny+shift_y;
			points++;
			//	}
			//lastupdate=len;
			dx=nx-x;
			dy=ny-y;
		}
		x=nx;
		y=ny;
		//len++;
		solve_link(x,y, nx,ny,waylen);
	}
	delete visited_array;
	visited_array=0;
	pf_solve_astar.stop();
	if (points>0)
	{
		points--;
	}
	else
	{
		return i4_F;
	}
	//cut of start point (at end of list)
	//since that would only try to correct out rounding errors on the start point
	/*i4_bool optimizing=i4_T;
	   w16 pt=points;
	   while (points>2&&opimizing)
	   	{

	   	}
	 */

	return i4_T;
}
//}}}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
