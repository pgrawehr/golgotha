/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "map.h"
#include "g1_object.h"
#include "height_info.h"
#include "map_cell.h"
#include "time/profile.h"
#include "objs/map_piece.h"
#include "objs/bases.h"
#include "math/pi.h"
#include "init/init.h"

static const int nc=8; //just use only 4 corners doesn't work correctly.
static const i4_float rad=2.0f/((float)nc);

static float radcostable[nc];
static float radsintable[nc];

class radtablescalcer :
	public i4_init_class
{
public:
	virtual void init()
	{
		int i;
		for(i=0; i<nc; i++)
		{
			radcostable[i]=float (rad*cos(2.0*i4_pi()*(0.5+i)/nc));
			radsintable[i]=float (rad*sin(2.0*i4_pi()*(0.5+i)/nc));
		}
	};
} radtablescalcer_instance;

i4_profile_class g1_map_pf_check_collision("map::check_collision");

/** Generic collision checking entry point.
 *Checks wheter a given object can move in the given direction.
 *Returns True if a collision occured and false if not.
 *the dx/dy values might be adjusted to something that works
 *after this function returns.
 *Idee für verbesserten Algorithmus: Verwende Hashtabelle
 *mit bijektiv-konkateniertem Key je 2er Objekte um inkrementell
 *den Abstand zwischen den zwei Objekten zu bestimmen.

 *Weitere Verbesserung: Die while-Schleife nach aussen ziehen,
 *da es Bedingungen gibt, die nur 1x getestet werden müssen.
   @param x The x coordinate of @a _this
   @param y The y coordinate of @a _this
   @param dx The amount we want to go in x direction
   @param dy The amount we want to go in y direction
   @param _this The this pointer of the object being tested
   @param hit If a collision occured, this returns the other object.
   Be aware that it might be that this is not the only object that collides
   with the current one.
   @param occupancy_radius The occupancy radius of _this. Passed as its
   own parameter for historical reasons.
   @returns True for hit, False for miss.
 */


/*
   int g1_map_class::check_poly_collision(i4_float x, i4_float y,
   									   i4_float occupancy_radius,
   									   i4_float &dx, i4_float &dy,
   									   g1_object_class *_this,
   									   g1_object_class *&hit)
   	{

   	int x1,y1, x2,y2, ix,iy;
   	//i4_bool ret = i4_F;

   	if (dx==0.0 && dy==0.0)
   		return i4_F;
   	g1_map_pf_check_collision.start();
   	i4_3d_vector start, ray(dx,dy,0), normal, offset, oray;
   	//ray.normalize();  //perhaps model.intersect relies on this?
   	//const int nc=8;
   	//const i4_float rad = 0.25;
   	g1_map_piece_class *mp=g1_map_piece_class::cast(_this),*othermp;
   	i4_bool this_is_on_path=(mp&&mp->get_next_object());
   	i4_bool other_is_on_path;
   	const int NUM_CHECK_ENTRIES=20;
   	g1_object_class *alreadychecked[NUM_CHECK_ENTRIES+1];//for a few cases, remember wheter we
   	//have alread done them.
   	int checkedindex,ind;
   	float speed2=dx*dx+dy*dy;
   	float z=_this->h;
   	i4_bool brk;
   	for (int i=0; i<nc; i=i+1)
   		//instead of 0 to 7 or 0,2,4,6 we would like to use 1,3,5,7
   		//but that doesn't work either.
   		{
   		//start.set(x+rad*(float)cos(2.0*i4_pi()*(0.5+i)/nc),y+rad*(float)sin(2.0*i4_pi()*(0.5+i)/nc),_this->h+0.07f);
   		//The z+0.07 is not appropriate enough. First, it may completelly
   		//fail if the scalling is not 1/10 and then, some objects are
   		//much flatter than this, so this will newer hit.
   		start.set(x+radcostable[i],y+radsintable[i],z+0.07f);

   		if (ray.x<0)
   			{
   			x2 = i4_f_to_i(start.x);
   			x1 = i4_f_to_i(start.x+ray.x);
   			}
   		else
   			{
   			x1 = i4_f_to_i(start.x);
   			x2 = i4_f_to_i(start.x+ray.x);
   			}
   		if (ray.y<0)
   			{
   			y2 = i4_f_to_i(start.y);
   			y1 = i4_f_to_i(start.y+ray.y);
   			}
   		else
   			{
   			y1 = i4_f_to_i(start.y);
   			y2 = i4_f_to_i(start.y+ray.y);
   			}
   		checkedindex=0;
   		alreadychecked[0]=0;
   		for (iy=y1; iy<=y2; iy++)
   			for (ix=x1; ix<=x2; ix++)
   				{
   				g1_object_chain_class *objlist = cell(ix,iy)->get_obj_list();

   				while (objlist)
   					{
   					g1_object_class *obj=objlist->object;
   					//          i4_float dist, dx,dy;
   					othermp=g1_map_piece_class::cast(obj);
   					other_is_on_path=(othermp && othermp->get_next_object());
   					if ((obj==_this) || (this_is_on_path &&
   						other_is_on_path ))
   						{//shortcut
   						objlist=objlist->next_solid();
   						continue;
   						}

   					if (obj->get_flag(g1_object_class::BLOCKING))
   						{
   						ind=0;
   						brk=i4_F;
   						while(ind<checkedindex)
   							{
   							if (alreadychecked[ind]==obj)
   								{
   								objlist=objlist->next_solid();
   								brk=i4_T;
   								break;
   								}
   							ind++;
   							};
   						if (brk)
   							continue;//we have to do this
   						//in this complicated fashion because
   						//we want to continue the outer loop.
   						if (checkedindex<NUM_CHECK_ENTRIES)
   							{
   							checkedindex=checkedindex+1;
   							alreadychecked[checkedindex]=obj;
   							};
   						oray = ray;
   						//before testing for polygons
   						//we can do an occupancy_radius test.
   						float xd,yd,zd;
   						//double s=(obj->x*x+obj->y*y+obj->h*_this->h);
   						float s,rcomp;
   						//xd=(obj->x*obj->x)-(x*x);//??
   						//yd=(obj->y*obj->y)-(y*y);
   						//zd=(obj->h*obj->h)-(z*z);
   						xd=(x-obj->x);
   						xd=xd*xd;
   						yd=(y-obj->y);
   						yd=yd*yd;
   						zd=(z-obj->h);
   						zd=zd*zd;
   						s=xd+yd+zd;//square of the distance between objects
   						rcomp=(obj->occupancy_radius()+_this->occupancy_radius());
   						rcomp=rcomp*rcomp;//square of the sum of their radius
   						if (s>rcomp)
   							{
   							objlist=objlist->next_solid();
   							continue;
   							}

   						//if (othermp && mp && (rcomp<2))
   						//	{
   							//if other is map_piece and this too,
   							//occupancy_radius should suffice as test
   						//	ray.x=0;
   						//	ray.y=0;
   						//	hit=obj;
   						//	continue;
   						//	}


   						if (g1_model_collide_polygonal(obj, obj->draw_params, start, ray, normal))
   							{
   							if (normal.z>0.8)
   								{
   								// incline more than 45 degrees.  let it go
   								ray = oray;
   								}
   							else
   								{
   								// slide perp. to the normal
   								// NOTE: we really should resubmit the new movement vector to objects
   								//       in the list we already tried to collide against for more robust
   								//       sliding.  just hope we catch most of the errors on the next frame
   								i4_3d_vector diff(ray);
   								diff -= oray;
   								normal.z = 0;
   								normal.normalize();
   								normal *= diff.dot(normal) + 0.05f;
   								ray = oray;
   								ray += normal;
   								hit=obj;
   								//ret = i4_T;
   								}
   							}
   						}

   					objlist = objlist->next_solid();
   					}
   				}
   		}
   	dx=ray.x;
   	dy=ray.y;
   	g1_map_pf_check_collision.stop();
   	return (hit!=0);
   	}

 */

/*

   int g1_map_class::check_collision(i4_float x, i4_float y,
   								  i4_float occupancy_radius,
   								  i4_float &dx, i4_float &dy,
   								  sw32 _ix, sw32 _iy,
   								  g1_object_class *_this,
   								  g1_object_class*& hit) const
   	{
   	g1_map_pf_check_collision.start();
   	i4_float gx,gy,r;
   	i4_float a,b,c,d;
   	i4_float ldx,ldy;
   	i4_float model_corr_factor;
   	if (dx<0)
   		ldx=dx-1;
   	else
   		ldx=dx+1;
   	if (dy<0)
   		ldy=dy-1;
   	else
   		ldy=dy+1;

   	hit = 0;
   	i4_float min_dist=0;

   	w32 x_left,x_right,y_top,y_bottom,ix,iy;

   	if (
   		(cell((w16)_ix,(w16)_iy)->is_blocking()) ||
   		(x<0)    || (y<0)    || (x>=w)    || (y>=h) ||
   		(x+dx<0) || (y+dy<0) || (x+dx>=w) || (y+dy>=h)
   		)
   		{
   		dx = 0;
   		dy = 0;
   		g1_map_pf_check_collision.stop();
   		return -1;
   		}
   	model_corr_factor=0.8f*1.0f/g1_model_list_man.get_scaling();
   	occupancy_radius*=model_corr_factor;
   	//for unsigned values, anything larger than the max means negative
   	x_left   = w32(x - i4_fabs(ldx)); if (x_left>w)     x_left=0;
   	x_right  = w32(x + i4_fabs(ldx)); if (x_right>w-1)  x_right=w-1;
   	y_top    = w32(y - i4_fabs(ldy)); if (y_top>h)      y_top=0;
   	y_bottom = w32(y + i4_fabs(ldy)); if (y_bottom>h-1) y_bottom=h-1;


   	for (ix=x_left; ix<=x_right;  ix++)
   		for (iy=y_top;  iy<=y_bottom; iy++)
   			{
   			if (ix>=0 && iy>=0 && ix<w && iy<h)
   				{
   				g1_object_chain_class *p = cell((w16)ix,(w16)iy)->get_solid_list();
   				while (p)
   					{
   //					if its an exact collision (exactly same x's and y's) its probably the same object,
   //						so ignore it
   					if (!p->object->get_flag(g1_object_class::BLOCKING) ||
   						(p->object->get_flag(g1_object_class::CAN_DRIVE_ON)
   						&& !g1_building_class::cast(p->object))
   						)
   						{
   						p=p->next_solid();
   						continue;
   						}
   						//if (p->object->x!=x || p->object->y!=y)
   						if (p->object!=_this)
   							{

   							gx = p->object->x - x;
   							gy = p->object->y - y;
   							//is correction needed here, too?
   							a  = dx*gx + dy*gy;
   							b  = dx*dx + dy*dy;
   							c  = (a*a)/b;

   							d  = (gx*gx + gy*gy);
   							r  = model_corr_factor*p->object->occupancy_radius();

   							if ((r*r)>d-c)
   								{
   								i4_bool x_check=i4_F,y_check=i4_F;
   								min_dist=d;
   								if (dx>0)
   									{
   									if ((p->object->x+r > x) && (p->object->x-r < (x+dx))) x_check = i4_T;
   									}
   								else
   									{
   									if ((p->object->x-r < x) && (p->object->x+r > (x+dx))) x_check = i4_T;
   									}

   								if (dy>0)
   									{
   									if ((p->object->y+r > y) && (p->object->y-r < (y+dy))) y_check = i4_T;
   									}
   								else
   									{
   									if ((p->object->y-r < y) && (p->object->y+r > (y+dy))) y_check = i4_T;
   									}

   								if (x_check && y_check)
   									{
   									//it might hit.
   									//now do a close check.
   									hit =p->object;
   									if (_this->h < hit->h)
   										{
   										if ( (_this->h+occupancy_radius)<
   											(hit->h - hit->occupancy_radius()))
   											{
   											hit=0;
   											}

   										}
   									else
   										{
   										if (( _this->h-occupancy_radius) >
   											(hit->h + hit->occupancy_radius()))
   											{
   											hit=0;
   											}
   										}
   									g1_map_piece_class *mpthis,*mpother;
   									if ((mpthis=g1_map_piece_class::cast(_this)) && (mpother=g1_map_piece_class::cast(hit)))
   										{//both go to map piece-> if they are both on a path, just skip here
   										if (mpthis->get_next_object() && mpother->get_next_object())
   											{
   											hit=0;
   											}
   										}
   									//otherwise, we'll just do a radial test anyway.
   									if (hit&&(hit->occupancy_radius()>0.5))
   										{
   										i4_3d_vector pos(x,y,_this->h);
   										i4_3d_vector ray(dx,dy,0),newray;
   										newray=ray;
   										//ray.normalize();
   										//lets see...
   										//The stank works, so this is probably a problem of
   										//the parameters only.
   										//Perhaps we have to pass the edge of our model as pos?
   										//see stank.
   										if (!hit->check_collision(pos,
   											newray))
   											hit=0;
   										//attempt 1: just bad
   										//if (newray!=ray)
   										//	{
   										//	hit=0;
   										//	}
   										//attempt 2: doesn't work
   										//if((newray.length()+0.00001)<ray.length())
   										//	{
   										//	hit=p->object;
   										//	dx=newray.x;
   										//	dy=newray.y;
   										//	}
   										//attempt 3: check wheter
   										//the two point in the same direction
   										i4_float cosphi;
   										//calculate angle for arbitrary direction
   										cosphi=ray.dot(newray)/(newray.length()*ray.length());
   										if (cosphi<0.998 && (cosphi>0))
   											{
   											hit=p->object;
   											dx=newray.x;
   											dy=newray.y;
   											}
   										//dx=newray.x;
   										//dy=newray.y;
   										}
   									}
   								}
   							}
   						p = p->next_solid();
   						if (hit)
   							{
   							g1_map_pf_check_collision.stop();
   							return 1;//as soon as we have one, we can return
   							}
   					}
   				}
   			}
   		g1_map_pf_check_collision.stop();
   		if (hit)
   			return 1;

   		return 0;
   	}
 */

i4_bool g1_map_class::check_collision(g1_object_class *obj,
									  i4_float &dx, i4_float &dy,
									  i4_float &dz,
									  g1_object_class *&hit) const
{
	hit=0;
	if (dx==0 && dy==0)
	{
		return i4_F;
	}
	g1_map_pf_check_collision.start();
	//for now, it should be safe to assume that this cast is
	//always possible, since non-map-piece objects cannot move and
	//therefore also cannot collide.
	g1_map_piece_class *mp=g1_map_piece_class::cast(obj);
	if (!mp)
	{
		g1_map_pf_check_collision.stop();
		return i4_F;
	}
	i4_bool this_is_on_path=mp->get_next_object() ? i4_T : i4_F;
	g1_object_class *other=0;
	g1_map_piece_class *other_mp=0;
	i4_bool other_is_on_path=i4_F;
	i4_float occupancy_radius=mp->occupancy_radius();
	g1_object_class *objs_arr[20];
	sw32 num_objs=get_objects_in_range(mp->x,mp->y,
									   occupancy_radius*2+1,objs_arr,20);
	i4_3d_vector start(mp->x,mp->y,mp->h);
	i4_3d_vector ray(dx,dy,dz);
	i4_bool ret=i4_F;
	for(int i=0; i<num_objs; i++)
	{
		other=objs_arr[i];
		//don't check collision against myself.
		if (other==obj)
		{
			continue;
		}
		if (other->check_collision(obj,start,ray))
		{
			dx=ray.x;
			dy=ray.y;
			dz=ray.z;
			hit=objs_arr[i];
			ret=i4_T;
			break;
		}
	}
	g1_map_pf_check_collision.stop();
	return ret;
}


int g1_map_class::check_non_player_collision(g1_object_class *source,
											 g1_player_type player_num,
											 const i4_3d_vector &point,
											 i4_3d_vector &ray,
											 g1_object_class *& hit) const
{
	hit = 0;

	sw32 x_left,x_right,y_top,y_bottom, ix,iy;

	if ((point.x<0) || (point.y<0) || (point.x>=w) || (point.y>=h) ||
		(point.x+ray.x<0) || (point.y+ray.y<0) || (point.x+ray.x>=w) || (point.y+ray.y>=h))
	{
		ray.x=0;
		ray.y=0;
		ray.z=0;
		return -1;
	}

	i4_3d_vector final(point);
	final += ray;
	i4_float height = terrain_height(final.x,final.y);
	if (final.z<height)
	{
		// hit the ground
		ray.z = height - point.z;
		return 1;
	}

	if (ray.x<0)
	{
		x_left   = i4_f_to_i(point.x + ray.x);
		x_right  = i4_f_to_i(point.x);
	}
	else
	{
		x_left   = i4_f_to_i(point.x);
		x_right  = i4_f_to_i(point.x + ray.x);
	}
	if (ray.y<0)
	{
		y_top    = i4_f_to_i(point.y + ray.y);
		y_bottom = i4_f_to_i(point.y);
	}
	else
	{
		y_top    = i4_f_to_i(point.y);
		y_bottom = i4_f_to_i(point.y + ray.y);
	}

	if (x_left<0)
	{
		x_left=0;
	}
	if (x_right>(sw32)w-1)
	{
		x_right=w-1;
	}
	if (y_top<0)
	{
		y_top=0;
	}
	if (y_bottom>(sw32)h-1)
	{
		y_bottom=h-1;
	}

	for (ix=x_left; ix<=x_right;  ix++)
	{
		for (iy=y_top;  iy<=y_bottom; iy++)
		{
			g1_object_chain_class *p = cell(ix,iy)->get_solid_list();

			while (p)
			{
				// if the object is not on our team or so not dangerous (probably a building)

				// Let's see... If I guess right p->object->check_collision()
				// checks wheter the given vector "ray" starting at "point"
				// intersects a poly of "p->object" (if handled as poly collision handler)
				if ((p->object->player_num!=player_num ||
					 !p->object->get_flag(g1_object_class::DANGEROUS)) &&
					p->object->check_collision(source,point, ray))
				{
					hit = p->object;
					return 1;
				}

				p = p->next_solid();
			}
		}
	}

	//if (hit)
	//  return 1;

	return 0;
}


// int g1_map_class::check_cell(i4_float x, i4_float y,
//                              i4_float occupancy_radius,
//                              sw32 ix, sw32 iy) const
// {
//   i4_float dx, dy, r;

//   if (!cell(ix,iy).get_solid_list())
//     return 1;
//   return 0;
// }


int g1_map_class::check_terrain_location(i4_float x, i4_float y, i4_float z,
										 i4_float rad, w8 grade, w8 dir) const
{
	i4_float x_left,x_right,y_top,y_bottom;

	x_left   = x - rad;
	if (x_left<0)
	{
		return 0;
	}
	x_right  = x + rad;
	if (x_right>=w)
	{
		return 0;
	}
	y_top    = y - rad;
	if (y_top<0)
	{
		return 0;
	}
	y_bottom = y + rad;
	if (y_bottom>=h)
	{
		return 0;
	}

	sw32 xl = i4_f_to_i(x_left);
	sw32 xr = i4_f_to_i(x_right);
	sw32 yt = i4_f_to_i(y_top);
	sw32 yb = i4_f_to_i(y_bottom);
//code bellow mainly for old maps
	g1_map_cell_class *c;
	const g1_block_map_class *block = get_block_map(grade);
	c = cell(xl,yt);

	if (c->is_blocking() || block->is_blocked((w16)xl,(w16)yt,dir))
	{
		return 0;
	}

	c = cell(xr,yt);

	if (c->is_blocking() || block->is_blocked((w16)xr,(w16)yt,dir))
	{
		return 0;
	}

	c = cell(xl,yb);

	if (c->is_blocking() || block->is_blocked((w16)xl,(w16)yb,dir))
	{
		return 0;
	}

	c = cell(xr,yb);

	if (c->is_blocking() || block->is_blocked((w16)xr,(w16)yb,dir))
	{
		return 0;
	}

	return 1;
}
