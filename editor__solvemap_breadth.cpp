/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "solvemap_breadth.h"
#include "path_api.h"
#include "memory/malloc.h"
#include "error/error.h"
#include "map.h"
#include "map_man.h"
#include "map_cell.h"
#include "map_vert.h"
#include <math.h>

void g1_breadth_first_map_solver_class::set_block_map(g1_block_map_class *_block)
//{{{
{
  block = _block;
  if (!solve_map || block->width()!=wx || block->height()!=wy)
  {
    if (solve_map)
		{
        i4_free(solve_map);
		delete [] length_map;
		}
    wx = block->width();
    wy = block->height();
    solve_map = (w8*)I4_MALLOC(wx*wy,"solve_map");
	length_map = new i4_float[wx*wy];
  }
}
//}}}

g1_breadth_first_map_solver_class::~g1_breadth_first_map_solver_class()
//{{{
{
  if (solve_map)
	  {
      i4_free(solve_map);
	  delete [] length_map;
	  }
}
//}}}

i4_bool g1_breadth_first_map_solver_class::add_cell(w32 x,w32 y,w8 d, i4_float length, i4_float addlen)
//{{{
{
  if (d>0 &&
      (x<0 || x>=wx || y<0 || y>=wy))
    return i4_F;

   /*|| is_visited(x,y)*/

  
  //advanced method that makes blocked areas sometimes passable.
  if (block->is_blocked((w16)x,(w16)y,d))
	  {
	  if (credits<=0 || (g1_get_map()->vertex(x,y)->flags & g1_map_vertex_class::APPLY_WAVE_FUNCTION))
		  return i4_F;
	  else
		  {
		  addlen=addlen*4;		  
		  credits-=2;
		  }
	  }
	length=length+addlen;
  
  
//  if (cnl[tail]>length)
//	  {
//	  set_solve_dir(x,y,d);//this way must be shorter than any way found till now.
//      visit(x,y);
//	  set_length(x,y,length);
//	  if (--tail<0) tail=queue_length-1;
//	  cnx[tail]=x;
//	  cny[tail]=y;
//	  cnl[tail]=length;
//	  }
//  else
//	  {
	  if (!is_visited(x,y))
		  {
		  set_solve_dir(x,y,d);
		  visit(x,y);
		  set_length(x,y,length);
	      cnx[head] = x;
          cny[head] = y;
          cnl[head] = length;
	      if (++head>=queue_length) head=0;
		  }
	  else
		  if (get_length(x,y)>length)
			  {
			  set_solve_dir(x,y,d);
			  set_length(x,y,length);//is already visited
			  cnx[head]=x;
			  cny[head]=y;
			  cnl[head]=length;
			  if (++head>=queue_length) head=0;
			  }
//	  }
  

  if (head==tail)
	  {
      i4_warning("INTERNAL: Map solving queue overrun.");
	  tail++;//assume tail is aged
	  }
    
  return i4_T;
}
//}}}

i4_bool g1_breadth_first_map_solver_class::get_next_cell(w32 &x,w32 &y, i4_float &length)
//{{{
{
  if (tail==head)
    return i4_F;
  do 
	  {
	  x = cnx[tail];
	  y = cny[tail];
	  length = cnl[tail];
      if (++tail>=queue_length) tail=0;
	  } while (tail!=head && get_length(x,y)<length);//skip entry if already obsolete
  if (tail==head&&head>2) //queue underrun, this means no way.
	  {//head>2 is true if we didn't just start
	  return i4_F;
	  }
  return i4_T;
}
//}}}

i4_bool g1_breadth_first_map_solver_class::path_solve(i4_float startx, i4_float starty, 
                                                      i4_float destx, i4_float desty, 
                                                      w8 sizex, w8 sizey, w8 grade,
                                                      i4_float *point, w16 &points)
//{{{
{
  w32 x,y,xs,ys;
  i4_float l;
  set_block_map(g1_get_map()->get_block_map(grade));
  w32 b=unblocked(block,startx,starty,destx,desty);
  if (b==BLOCK_EASY_WAY)
	  {//return path in reverse order
	  point[2]=startx;
	  point[3]=starty;
	  point[0]=destx;
	  point[1]=desty;
	  points=2;
	  return i4_T;
	  }
  else if (b==BLOCK_NO_WAY)
	  {
	  points=0;
	  return i4_F;
	  }

  //Todo: Take care if object is already in blocked area.
  clear_queue();
  clear_solve();

  add_cell((w32)startx,
           (w32)starty,
           0,0,0);

  //int found=0;
  
  credits=10;
  while (get_next_cell(x,y,l) && (x!=destx || y!=desty))
  {
    add_cell(x-1,y-1,G1_NORTH|G1_EAST,l,sqrt2);
    add_cell(x,y-1,G1_NORTH,l,1);
	add_cell(x+1,y-1,G1_NORTH|G1_WEST,l,sqrt2);

    add_cell(x-1,y,G1_EAST,l,1);
	//skipping myself
	add_cell(x+1,y,G1_WEST,l,1);

	add_cell(x-1,y+1,G1_SOUTH|G1_EAST,l,sqrt2);
	add_cell(x,y+1,G1_SOUTH,l,1);
	add_cell(x+1,y+1,G1_SOUTH|G1_WEST,l,sqrt2);
	credits+=1;
  }

  x = (w32)destx;
  xs=(w32) startx;
  ys=(w32) starty;
  y = (w32)desty;

  w32 last_dir=0;
  points = 0;
  if (solve_dir(x,y)==0)
    return i4_F;
  while (x!=xs || y!=ys)
  {
    //ok(x,y);
    if (solve_dir(x,y)!=last_dir)
    {
      last_dir = solve_dir(x,y);
      point[points*2]=   i4_float(x)+0.5f;
      point[points*2+1]= i4_float(y)+0.5f;
      points++;
    }
    switch (solve_dir(x,y)) 
    {
      case G1_NORTH: y++; break;
	  case G1_NORTH|G1_EAST: x++;y++;break;
	  case G1_NORTH|G1_WEST: x--;y++;break;
	  case G1_SOUTH|G1_WEST: x--;y--;break;
	  case G1_SOUTH|G1_EAST: x++;y--;break;
      case G1_SOUTH: y--; break;
      case G1_WEST:  x--; break;
      case G1_EAST:  x++; break;
      default:
        I4_ASSERT(0,"ERROR: Busted path solving!\n");
        break;
    }
  }
  //ok(x,y);
  point[points*2]=  startx;
  point[points*2+1]=starty;
  points++;

  return i4_T;
}

w32 g1_map_solver_class::unblocked(g1_block_map_class *block,i4_float startx, i4_float starty, i4_float destx, i4_float desty)
	{//uses bresenham to check for a direct path from start to dest
	w32 x1,y1,x2,y2;
	//i4_coord cx1,cy1,cx2,cy2;
	//i4_bool skip;
	
    x1=(w32)startx;
    y1=(w32)starty;
    x2=(w32)destx;
    y2=(w32)desty;
    //skip=i4_F;
	w8 dirs=0xff;
    sw32 i,xc,yc,er,n,m,xi,yi,xcxi,ycyi,xcyi;
    unsigned dcy,dcx;
    //no clipping involved here, both points are inside for shure
	if (x1>x2)        // make sure that x1 is to the left
		{    
        i=x1; x1=x2; x2=i;  // if not swap points
        //i=y1; y1=y2; y2=i;
		}  
	
	if (y1>y2)        // make sure that y1 is on top
		{    
        //i=x1; x1=x2; x2=i;  // if not swap points
        i=y1; y1=y2; y2=i;
		}  
	if ((x2-x1)<3 && (y2-y1)<3)
		{
		//Test: As coldet now works, one should be able to get as close
		//as possible to some object.
		//Exception: What blocks is the terrain itself
		//Or perhaps we handle this case just with the terrain being 
		//extremelly expensive.
		//if (block->is_blocked(destx,desty,0xff))
		//	{
		//	return BLOCK_NO_WAY;
		//	}
		return BLOCK_EASY_WAY;
		}
	
	xi=x2; xc=x1;
	
	// assume y1<=y2 from above swap operation
	yi=y2; yc=y1;
	xc=(x2-x1); yc=(y2-y1);
	dcx=x1;dcy=y1;
	if (xc<0) xi=-1; else xi=1;
	if (yc<0) yi=-1; else yi=1;
	n=xc; m=yc; // abs(.) is not needed, they can't be negative 
	ycyi=2*yc*xi; //can't get negative (I hope...)
	er=0;
	
	if (n>m)
        {
		xcxi=2*xc*xi;
		for (i=0;i<=n;i++)
			{
            
			if (block->is_blocked(dcx,dcy,dirs))
				return BLOCK_HARD_WAY;
			
            if (er>0)
				{
				dcy+=yi;
				er-=xcxi;
				}
            er+=ycyi;
            dcx+=xi;
			}
        }
	else
        {
		xcyi=2*xc*yi;
		for (i=0;i<=m;i++)
			{
            
			if (block->is_blocked(dcx,dcy,dirs))
				return BLOCK_HARD_WAY;
			
            if (er>0)
				{
				dcx+=xi;
				er-=ycyi;
				}
            er+=xcyi;
            dcy+=yi;
			}
        }
	
    return BLOCK_EASY_WAY;
	}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
