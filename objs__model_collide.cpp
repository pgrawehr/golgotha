#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "objs/model_collide.h"
#include "math/transform.h"
#include "g1_object.h"
#include "objs/model_id.h"
#include "math/pi.h"
#include "math/angle.h"

i4_bool g1_model_collide_polygonal_ex(g1_object_class *_this,
									  g1_object_class *source,
									  i4_3d_vector &ray,
									  i4_3d_vector &hit_normal,
									  i4_float *minimum_t)
{
	if (!_this->get_flag(g1_object_class::BLOCKING))
	{
		return i4_F;
	}
	const int nc=8;
	const i4_float rad = 0.25;
	g1_object_class *hit_obj=0;
	i4_3d_vector oray=ray;
	i4_bool ret=i4_F;
	i4_3d_vector start;
	for (int i=0; i<nc; i++)
	{
		start.set(source->x+rad*(float)cos(2.0*i4_pi()*(0.5+i)/nc),source->y+rad*(float)sin(2.0*i4_pi()*(0.5+i)/nc),source->h+0.07f);

		oray = ray;
		if (g1_model_collide_polygonal(_this, _this->draw_params, start, ray, hit_normal,minimum_t))
		{
			if (hit_normal.z>0.8)
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
				// we've hit the object 'obj'

				i4_3d_vector diff(ray);
				diff -= oray;
				//hit_normal.z = 0;
				hit_normal.normalize();
				hit_normal *= diff.dot(hit_normal) + 0.05f;
				ray = oray;
				ray += hit_normal;
				if (ray.length()<0.001)
				{
					ray.x=0;
					ray.y=0;
					ray.z=0;
				}
				// should really continue in the loop, in case we collide
				// with more than one wall at the same time. (i.e we've run into
				// an edge)
				return i4_T;
			}
		}

	}
	return ret;
}

i4_bool g1_model_collide_polygonal(g1_object_class *_this,
								   g1_model_draw_parameters &params,
								   const i4_3d_vector &start,
								   i4_3d_vector &ray,
								   i4_3d_vector &hit_normal,
								   i4_float *minimum_t)
{
	i4_transform_class transform, mini_local;
	i4_3d_vector istart, iray, normal;
	i4_float t, min_t=1.0;
	int poly;

	g1_quad_object_class *model = params.model;
	if (!model)
	{
		return i4_F;
	}

	_this->calc_world_transform(1.0, &transform);

	transform.inverse_transform(start, istart);
	transform.inverse_transform_3x3(ray, iray);

	if (model->intersect(istart, iray, params.animation, params.frame, &t, &poly, &normal))
	{
		if (t<min_t)
		{
			min_t = t;
			transform.transform_3x3(normal, hit_normal);
		}
	}

	i4_3d_vector mstart, mray;
	for (int i=0; i<_this->num_mini_objects; i++)
	{
		g1_mini_object *mini = &_this->mini_objects[i];

		if (mini->defmodeltype!=0xffff)
		{
			mini->calc_transform(1.0, &mini_local);
			mini_local.inverse_transform(istart, mstart);
			mini_local.inverse_transform_3x3(iray, mray);
			model = g1_model_list_man.get_model(mini->defmodeltype);
			if (model->intersect(mstart, mray, mini->animation, mini->frame, &t, &poly, &normal))
			{
				if (t<min_t)
				{
					min_t = t;
					i4_3d_vector local_norm;
					mini_local.transform_3x3(normal, local_norm);
					transform.transform_3x3(local_norm, hit_normal);
				}
			}
		}
	}

	if (minimum_t)
	{
		*minimum_t = min_t;
	}

	if (min_t<1.0)
	{
		ray *= min_t;
		return i4_T;
	}
	return i4_F;
}

i4_bool g1_model_collide_radial(g1_object_class *_this,
								g1_model_draw_parameters &params,
								const i4_3d_vector &start,
								i4_3d_vector &ray)
{

	// intersection of a ray with a sphere from graphics gems 1  p. 388
	i4_3d_vector EO, D, P,DIFF;
	float r, disc, d, v,difflen,raylen;

	D=ray;
	D.normalize();

	r=_this->occupancy_radius();
	float z_dist=_this->h - start.z;
//   if (z_dist*2<r)        // fudge the z because some vehicles are too short
//     z_dist=0;

	EO=i4_3d_vector(_this->x - start.x, _this->y - start.y, z_dist);
	v=EO.dot(D);

	//EO_length=EO.length();

	disc=r*r - (EO.dot(EO) - v*v);

	if( disc < 0 )
	{
		return i4_F;
	}
	else
	{
		d=(float)sqrt(disc);
		P=start+D*(v-d);
		DIFF=P-start;
		//diff is colinear with ray (should be...)
		difflen=DIFF.dot(DIFF);
		raylen=ray.dot(ray);
		if (raylen<difflen)
		{
			return i4_F;
		}
		if (raylen>0.2)
		{
			ray*=0.25;
			g1_model_collide_radial(_this,params,start,ray);
			return i4_T;
		}

		//D*=(v-d);
		//ray=D;
		ray.x=0;
		ray.y=0;
		ray.z=0;
		return i4_T;
	}


//   i4_float a,b,c,d,r, gx,gy;

//   gx = _this->x - start.x;
//   gy = _this->y - start.y;
//   d  = gx*gx + gy*gy;
//   r  = _this->occupancy_radius();
//   r = r*r;

//   //first bounding radius check
//   if (d < r)
//     return i4_T;

//   //then ray/circle intersection check
//   b  = ray.x*ray.x + ray.y*ray.y;
//   a  = ray.x*gx + ray.y*gy;
//   c  = (a*a)/b;

//   return (r>d-c);
}
