/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map.h"
#include "visible.h"
#include "solvemap_astar.h"
#error "This file is obsolete"
i4_bool g1_map_class::find_path(i4_float start_x, i4_float start_y,
                                i4_float destx, i4_float desty,
                                i4_float *points, w16 &t_nodes)
{
  if (destx<3) destx=3; else if (destx>=w-3) destx = (float)w-4;
  if (desty<3) desty=3; else if (desty>=h-3) desty = (float)h-4;

  // jc: fixme
  // Grade is not known here (should be given by caller, as he knows
  //the capablities of the current vehicle)
  //  i4_float tt=get_block_map(grade)->line_of_sight(c->x,c->y,destx,desty, rad_x, rad_y);
  //  i4_float tc=c->calculate_size(destx - c->x,desty - c->y)-0.001;
  if (0) //tt>=tc)
  {
    // if the destination is visible, then path is just a line
    points[0]=destx;
    points[1]=desty;
    //    points[2]=c->x;
    //    points[3]=c->y;
    t_nodes=2;
  }
  else
  {
    if (!astar_solver->path_solve(start_x, start_y,
                            destx, desty, 1, 1, 4,
                            points, t_nodes))
    {
      t_nodes=0;
      return i4_F;
    }
  }
  return i4_T;
}
