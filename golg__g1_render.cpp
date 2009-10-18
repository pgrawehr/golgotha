/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h"
#include "controller.h"
#include "map.h"
#include "window/win_evt.h"
#include "time/profile.h"
#include "menu/menu.h"
#include "menu/textitem.h"
#include "window/style.h"
#include "time/time.h"
#include "resources.h"
#include "input.h"
#include "mess_id.h"
#include "sound_man.h"
#include "g1_speed.h"
#include "remove_man.h"
#include "image/color.h"
#include "light.h"
#include "statistics.h"
#include "render/r1_api.h"
#include "render/r1_clip.h"
#include "math/random.h"
#include "math/pi.h"
#include "math/angle.h"
#include "math/trig.h"
#include "math/d_transform.h"
#include "g1_render.h"
#include "tile.h"
#include "g1_tint.h"
#include "time/profile.h"
#include "g1_texture_id.h"
#include "g1_tint.h"
#include "objs/map_piece.h"
#include "objs/stank.h"
#include "lisp/lisp.h"
#include "map_vert.h"
#include "loaders/load.h"
#include "render/r1_font.h"
#include "player.h"
#include "octree.h"

static i4_profile_class pf_render_object_transform("render_object (transform)"),
//pf_render_object_light("render_object (light)"),
pf_render_object_pack("render_object (project/pack)"),
pf_render_sprite("g1_render::render_sprite"),
pf_render_draw_outline("g1_render::draw_outline"),
pf_post_draw_quads("g1_render::post_draw_quads"),
pf_add_translucent_trail("g1_render::add_trans_trail"),
pf_draw_building("g1_render::draw_building"),
pf_draw_octree("render_octree");



const sw32 max_post_draw_quads = 1024;
static g1_post_draw_quad_class g1_post_draw_quads[max_post_draw_quads];
static sw32 g1_num_post_draw_quads = 0;

const sw32 max_post_draw_verts = max_post_draw_quads*4;
static r1_vert g1_post_draw_verts[max_post_draw_verts];
static sw32 g1_num_post_draw_verts = 0;

struct g1_post_draw_sprite_struct
{
	r1_texture_handle tex;
	float z;
	float s1, t1, s2, t2;
	float x1,y1,x2,y2;

} g1_post_draw_sprites[G1_MAX_SPRITES];
int g1_t_post_draw_sprites=0;



g1_render_class g1_render;

//this is used for the damage/blackening effect of hurt vehicles

static w8 charred_array[8] =
{
	8|1|128|4|32|2|64,
	8|1|128|4|32|2,
	8|1|128|4|32,
	8|1|128|4,
	8|1|128,
	8|1,
	8,
	0
};

i4_bool g1_render_class::main_draw=i4_F;

i4_float g1_render_class::scale_x,
						  g1_render_class::scale_y,
						  g1_render_class::ooscale_x,
						  g1_render_class::ooscale_y,
						  g1_render_class::center_x,
						  g1_render_class::center_y;

sw8 g1_render_class::render_damage_level = -1;

r1_render_api_class * g1_render_class::r_api;


float g1_render_class::calculate_frame_ratio()
{
	if (g1_map_is_loaded())
	{
		i4_time_class now;
		if (g1_resources.paused)
		{
			frame_ratio=1.0f;
		}
		else
		{
			frame_ratio=1.0f+now.milli_diff(g1_get_map()->tick_time)*G1_HZ/1000.0f;
		}

		if (frame_ratio>1)
		{
			frame_ratio=1;
		}
	}
	else
	{
		frame_ratio=0;
	}

	return frame_ratio;
}

// this is the default function for handling tinted polygons
g1_quad_class *g1_tint_modify_function(g1_quad_class * in,
									   g1_player_type player)
{
	g1_render.r_api->set_color_tint(g1_player_tint_handles[player]);
	return in;
}

void g1_get_ambient_function(i4_transform_class * object_to_world,
							 i4_float &ar, i4_float &ag, i4_float &ab)
{
	g1_get_map()->get_illumination_light(object_to_world->t.x,
										 object_to_world->t.y, ar,ag,ab);
}


//Does this also need to be changed to double precision?
void fast_transform(i4_transform_class * t,const i4_3d_vector &src, r1_3d_point_class &dst)
{
#ifndef _WINDOWS
	dst.x = t->x.x*src.x + t->y.x*src.y + t->z.x*src.z + t->t.x;
	dst.y = t->x.y*src.x + t->y.y*src.y + t->z.y*src.z + t->t.y;
	dst.z = t->x.z*src.x + t->y.z*src.y + t->z.z*src.z + t->t.z;
#else
	_asm
	{
		mov eax, t
		mov ecx, src
		mov edx, dst

		//optimized transformation: 34 cycles
		//compute t->x.x*src.x, t->x.y*src.x, t->x.z*src.x
		fld dword ptr  [ecx+0]     ;		starts & ends on cycle 0
		fmul dword ptr [eax+0]     ;		starts on cycle 1
		fld dword ptr  [ecx+0]     ;		starts & ends on cycle 2
		fmul dword ptr [eax+4]     ;		starts on cycle 3
		fld dword ptr  [ecx+0]     ;		starts & ends on cycle 4
		fmul dword ptr [eax+8]     ;		starts on cycle 5

		//compute t->y.x*src.y, t->y.y*src.y, t->y.z*src.y
		fld dword ptr  [ecx+4]     ;		starts & ends on cycle 6
		fmul dword ptr [eax+12]    ;		starts on cycle 7
		fld dword ptr  [ecx+4]     ;		starts & ends on cycle 8
		fmul dword ptr [eax+16]    ;		starts on cycle 9
		fld dword ptr  [ecx+4]     ;		starts & ends on cycle 10
		fmul dword ptr [eax+20]    ;		starts on cycle 11

		//st0           st1           st2           st3           st4           st5
		//t->y.z*src.y  t->y.y*src.y  t->y.x*src.y  t->x.z*src.x  t->x.y*src.x  t->x.x*src.x

		fxch           st(2)       ;		no cost
		//st0           st1           st2           st3           st4           st5
		//t->y.x*src.y  t->y.y*src.y  t->y.z*src.y  t->x.z*src.x  t->x.y*src.x  t->x.x*src.x

		faddp          st(5),st(0) ;		starts on cycle 12
		//st0           st1           st2           st3           st4
		//t->y.y*src.y  t->y.z*src.y  t->x.z*src.x  t->x.y*src.x  t->x.x*src.x+
		//                                                        t->y.x*src.y

		faddp          st(3),st(0) ;		starts on cycle 13
		//st0           st1           st2           st3
		//t->y.z*src.y  t->x.z*src.x  t->x.y*src.x+ t->x.x*src.x+
		//                            t->y.y*src.y  t->y.x*src.y

		faddp          st(1),st(0) ;		starts on cycle 14
		//st0           st1           st2
		//t->x.z*src.x+ t->x.y*src.x+ t->x.x*src.x+
		//t->y.z*src.y  t->y.y*src.y  t->y.x*src.y

		//compute t->z.x*src.z, t->z.y*src.z, t->z.z*src.z
		fld dword ptr  [ecx+8]     ;		starts & ends on cycle 15
		fmul dword ptr [eax+24]    ;		starts on cycle 16
		fld dword ptr  [ecx+8]     ;		starts & ends on cycle 17
		fmul dword ptr [eax+28]    ;		starts on cycle 18
		fld dword ptr  [ecx+8]     ;		starts & ends on cycle 19
		fmul dword ptr [eax+32]    ;		starts on cycle 20

		//st0           st1           st2           st3           st4           st5
		//t->z.z*src.z  t->z.y*src.z  t->z.x*src.z  t->x.z*src.x+ t->x.y*src.x+ t->x.x*src.x+
		//                                          t->y.z*src.y  t->y.y*src.y  t->y.x*src.y
		fxch           st(2)       ;		no cost

		faddp          st(5),st(0) ;		starts on cycle 21
		//st0           st1           st2           st3           st4
		//t->z.y*src.z  t->z.z*src.z  t->x.z*src.x+ t->x.y*src.x+ t->x.x*src.x+
		//                            t->y.z*src.y  t->y.y*src.y  t->y.x*src.y+
		//                                                        t->z.x*src.z

		faddp          st(3),st(0) ;		starts on cycle 22

		//st0           st1           st2           st3
		//t->z.z*src.z  t->x.z*src.x+ t->x.y*src.x+ t->x.x*src.x+
		//              t->y.z*src.y  t->y.y*src.y+ t->y.x*src.y+
		//                            t->z.y*src.z  t->z.x*src.z

		faddp          st(1),st(0) ;		starts on cycle 23
		//st0           st1           st2
		//t->x.z*src.x+ t->x.y*src.x+ t->x.x*src.x+
		//t->y.z*src.y+ t->y.y*src.y+ t->y.x*src.y+
		//t->z.z*src.z  t->z.y*src.z  t->z.x*src.z

		fxch           st(2)       ;		no cost
		//st0            st1           st2
		//t->x.x*src.x+  t->x.y*src.x+ t->x.z*src.x+
		//t->y.x*src.y+  t->y.y*src.y+ t->y.z*src.y+
		//t->z.x*src.z   t->z.y*src.z  t->z.z*src.z

		fadd dword ptr [eax+36]    ;		starts on cycle 24
		fxch           st(1)       ;		starts on cycle 25

		//st0            st1           st2
		//t->x.y*src.x+  dest_x        t->x.z*src.x+
		//t->y.y*src.y+                t->y.z*src.y+
		//t->z.y*src.z                 t->z.z*src.z

		fadd dword ptr [eax+40]    ;		starts on cycle 26
		fxch           st(2)       ;		no cost

		//st0            st1    st2
		//t->x.z*src.x+  dest_x dest_y
		//t->y.z*src.y+
		//t->z.z*src.z
		fadd dword ptr [eax+44]    ;		starts on cycle 27
		fxch           st(1)       ;		no cost

		//st0     st1    st2
		//dest_x  dest_z dest_y

		fstp dword ptr [edx+0]     ;		starts on cycle 28, ends on cycle 29
		fstp dword ptr [edx+8]     ;		starts on cycle 30, ends on cycle 31
		fstp dword ptr [edx+4]     ;		starts on cycle 32, ends on cycle 33
	}
#endif
}


void g1_render_class::uninstall_font()
{
	if (rendered_font)
	{
		delete rendered_font;
		rendered_font=0;
	}


}

void g1_render_class::install_font()
{
	uninstall_font();
	i4_image_class * im=i4_load_image("bitmaps/golg_font_18.bmp");
	if (im)
	{
		rendered_font=new r1_font_class(r_api, im);
		delete im;
	}
}

static int g1_quad_sorter(g1_quad_class * const * a,
						  g1_quad_class * const * b)
{
	if (*a==*b)
	{
		return 0;
	}
	if (*a>*b)  //if a > b we want that a comes before b
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

i4_bool g1_render_class::prepare_octree_rendering(i4_array<g1_quad_class *> &qif,
												  g1_quad_object_class * obj,
												  g1_vert_class * src_vert,
												  i4_transform_class * tf,
												  i4_transform_class * object_to_world,
												  w8 &ANDCODE,
												  w8 &ORCODE)
{
	g1_octree * oc=obj->octree;

	//quads in frustrum
	pf_draw_octree.start();

	if (oc->DrawOctree(tf,qif,object_to_world ? 0 : 1)==i4_F)
	{
		pf_draw_octree.stop();
		return i4_F;
	}
	//now: We first sort qif, such that we can later drop duplicate
	//quads.
	//Then we iterate trough it and grab all the vertices
	//from the used quads, since we only need to transform those.

	//we have to initialize the vertices array, such that we
	//can check their validity later
	//hmmm... this is quite expensive if num_vertices is large
	//(which is very probable)
	memset(t_vertices,0,(obj->num_vertex+1)*sizeof(r1_vert));
	if (qif.empty())
	{
		pf_draw_octree.stop();
		return i4_T; //will break completelly out, as andcode is still 0xff
	}
	qif.sort(g1_quad_sorter);

	g1_quad_class * curquad;
	g1_vert_class * src_v=0;

	i4_3d_vector t_light;

	i4_float ar=0.5f,ag=0.5f,ab=0.5f;

	float dir_int=g1_lights.directional_intensity;
	// get the ambient contribution from the map at the object's center point

	i4_3d_vector light = g1_lights.direction;
	t_light=light;

	if (object_to_world)
	{
		//transform light into object space for lighting
		//Warn: this doesn't work if no map is loaded!
		get_ambient(object_to_world, ar,ag,ab);
		object_to_world->inverse_transform_3x3(light,t_light);
	}

	w32 vertex;
	w32 n_vert;
	int vertid;
	int curquadidx;
	r1_vert * v;
	w8 code;
	i4_float ooz,dot,reflected_intensity;
	for (curquadidx=0; curquadidx<qif.size(); curquadidx++)
	{
		curquad=qif[curquadidx];
		if ((curquadidx>0)&&(curquad==qif[curquadidx-1]))
		{
			continue;
		}            //skip duplicate quads

		n_vert=curquad->num_verts();
		for (vertex=0; vertex<n_vert; vertex++)
		{
			vertid=curquad->vertex_ref[vertex];
			v=&t_vertices[vertid];
			if (v->flags)
			{
				continue;
			}
			src_v=&src_vert[vertid];
			fast_transform(tf,src_v->v,v->v);
			code = r1_calc_outcode(v);
			ANDCODE &= code;
			ORCODE  |= code;
			ooz = r1_ooz(v->v.z);
			v->px = v->v.x * ooz * center_x + center_x;
			v->py = v->v.y * ooz * center_y + center_y;
			v->w  = ooz;

			dot=-src_v->normal.dot(t_light);
			reflected_intensity = dir_int * dot;


			if (reflected_intensity<0)
			{
				reflected_intensity=0;
			}

			v->r = reflected_intensity * dir_int + ar;
			if (v->r>1.0)
			{
				v->r=1.0;
			}
			v->g = reflected_intensity * dir_int + ag;
			if (v->g>1.0)
			{
				v->g=1.0;
			}
			v->b = reflected_intensity * dir_int + ab;
			if (v->b>1.0)
			{
				v->b=1.0;
			}
			v->a = 1.0;
			v->flags=1;
		}
	}
	pf_draw_octree.stop();
	return i4_T;
}

void g1_render_class::ensure_capacity(int num_vertices)
{
	if (num_vertices>max_t_vertices)
	{
		max_t_vertices=num_vertices+50;
		t_vertices=(r1_vert *)realloc(t_vertices,max_t_vertices*sizeof(r1_vert));
	}
}


void g1_render_class::render_object(g1_quad_object_class * obj,
									i4_transform_class * object_to_view,
									i4_transform_class * object_to_world,
									i4_float texture_scale,
									int player_num,
									sw32 current_frame,
									g1_screen_box * bound_box,
									w32 option_flags)
{
	if (!obj)
	{
		//its not 100% clear in wich rare cases this happens
		return;
	}
	g1_stat_counter.increment(g1_statistics_counter_class::OBJECTS);


	int i,j, /*k,*/ num_vertices;
	//r1_vert* t_vertices=0;
	//int src_quad[4];
	i4_transform_class view_transform;

	num_vertices            = obj->num_vertex;
	ensure_capacity(num_vertices+1);



	r1_vert * v              = t_vertices;
	g1_vert_class * src_vert = obj->get_verts(0, current_frame);
	w8 ANDCODE = 0xFF;
	w8 ORCODE  = 0;

	g1_vert_class * src_v=src_vert;

	//pf_render_object_transform.start();

	//get this vector before we warp the transform
	i4_3d_vector cam_in_object_space;
	object_to_view->inverse_transform(i4_3d_vector(0,0,0),cam_in_object_space);


	view_transform.x.x = object_to_view->x.x * scale_x;
	view_transform.x.y = object_to_view->x.y * scale_y;
	view_transform.x.z = object_to_view->x.z;
	view_transform.y.x = object_to_view->y.x * scale_x;
	view_transform.y.y = object_to_view->y.y * scale_y;
	view_transform.y.z = object_to_view->y.z;
	view_transform.z.x = object_to_view->z.x * scale_x;
	view_transform.z.y = object_to_view->z.y * scale_y;
	view_transform.z.z = object_to_view->z.z;
	view_transform.t.x = object_to_view->t.x * scale_x;
	view_transform.t.y = object_to_view->t.y * scale_y;
	view_transform.t.z = object_to_view->t.z;

	//if we have an octree, we need to iterate over a
	//limited set of polys and vertices only.
	quad_object_list.clear();
	i4_bool use_ot=i4_F;
	if (obj->octree)
	{
		//hiddenly modificates the t_vertices array
		use_ot=prepare_octree_rendering(quad_object_list,obj,src_vert,
										&view_transform,
										object_to_world,
										ANDCODE,ORCODE);
		if (ANDCODE&&use_ot)
		{
			return;
		}
	}
	if (!obj->octree || (use_ot==i4_F))
	{
		for (i=0; i<num_vertices; i++, src_v++, v++)
		{
			fast_transform(&view_transform,src_v->v,v->v);

			//v->v.x *= scale_x; //this is accomplished by the above matrix multiplies
			//v->v.y *= scale_y;

			w8 code = r1_calc_outcode(v);
			ANDCODE &= code;
			ORCODE  |= code;
			if (!code)
			{
				//valid point
				i4_float ooz = r1_ooz(v->v.z);

				v->px = v->v.x * ooz * center_x + center_x;
				v->py = v->v.y * ooz * center_y + center_y;


				v->w  = ooz;
				if (option_flags & RENDER_PROJECTTOPLANE)
				{
					v->w=r1_ooz(r1_near_clip_z);

				}
			}
		}


		//pf_render_object_transform.stop();

		if (ANDCODE)
		{
			//free(t_vertices);
			return;
		}

		//pf_render_object_light.start();

		//IMPORTANT
		//we need this transform to do lighting
		if (object_to_world)
		{
			//worldspace light vector
			i4_3d_vector light = g1_lights.direction;
			i4_3d_vector t_light;

			// get the ambient contribution from the map at the object's center point
			i4_float ar,ag,ab;
			get_ambient(object_to_world, ar,ag,ab);

			float dir_int=g1_lights.directional_intensity;

			//transform light into object space for lighting
			object_to_world->inverse_transform_3x3(light,t_light);

			src_vert = obj->get_verts(0,current_frame);
			v        = t_vertices;

			//We'll need to further investigate here, since it's no more
			//acceptable to iterate over all vertices if there is an octree
			//(num_vertices might be several hundred thousand)
			for (i=0; i<num_vertices; i++, v++)
			{
				//      i4_float reflected_intensity = 1.0;

				i4_float dot=-src_vert[i].normal.dot(t_light);
				i4_float reflected_intensity = g1_lights.directional_intensity * dot;


				if (reflected_intensity<0)
				{
					reflected_intensity=0;
				}

				v->r = reflected_intensity * dir_int + ar;
				if (v->r>1.0)
				{
					v->r=1.0;
				}
				v->g = reflected_intensity * dir_int + ag;
				if (v->g>1.0)
				{
					v->g=1.0;
				}
				v->b = reflected_intensity * dir_int + ab;
				if (v->b>1.0)
				{
					v->b=1.0;
				}
				v->a = 1.0;
			}
		}
		else
		{
			v = t_vertices;
			for (i=0; i<num_vertices; i++, v++)
			{
				v->r = v->g = v->b = 1.f;    //g1_lights.ambient_intensity;
				v->a = 1.0f;    /// we will decrease this... (was 1.0, just to remember)
				//ok? let's try this?
			}
		}


		if (render_damage_level != -1)
		{
			//set their lighting values to 1
			v = t_vertices;
			for (i=0; i<num_vertices; i++, v++)
			{
				if ((1<<(i&7)) & charred_array[render_damage_level])
				{
					v->r = v->g = v->b = 0;
				}
			}
		}

	}    //else not octree

	//pf_render_object_light.stop();

	pf_render_object_pack.start();
	g1_quad_class * q_ptr = obj->quad, * q;
	int allquads=obj->num_quad;
	if (use_ot)
	{
		allquads=quad_object_list.size();
		q_ptr=quad_object_list[0];
	}
	for (i=0; i<allquads; i++, q_ptr++)
	{
		if (use_ot)
		{
			q_ptr=quad_object_list[i];
			if (i>0 && q_ptr==quad_object_list[i-1])
			{
				//we've already drawn this quad.
				continue;
			}
		}
		sw32 num_poly_verts = q_ptr->num_verts();

		i4_3d_vector cam_to_pt = src_vert[q_ptr->vertex_ref[0]].v;
		cam_to_pt -= cam_in_object_space;

		float dot = cam_to_pt.dot(q_ptr->normal);

		if (dot<0||q_ptr->get_flags(g1_quad_class::BOTHSIDED))
		{
			if (g1_tint!=G1_TINT_OFF && g1_hurt_tint==0)
			{
				if ((q_ptr->get_flags(g1_quad_class::TINT)||(g1_tint==G1_TINT_ALL))
					&& player_num!=-1)
				{
					q=tint_modify(q_ptr, player_num);
				}
				else
				{
					r_api->set_color_tint(0);
					q=q_ptr;
				}
			}
			else
			{
				q=q_ptr;
			}

			// copy in the texture coordinates
			for (j=0; j<num_poly_verts; j++)
			{
				int ref=q->vertex_ref[j];
				//src_quad[j]=ref;

				v = &t_vertices[ref];
				v->s = q->u[j];
				v->t = q->v[j];
			}


			if (ORCODE==0)
			{
				float nearest_w = 0;

				if (bound_box)
				{
					for (j=0; j<num_poly_verts; j++)
					{
						r1_vert * temp_vert = &t_vertices[q->vertex_ref[j]];

						float ooz = temp_vert->w;

						if (ooz > nearest_w)
						{
							nearest_w=ooz;
						}

						if (temp_vert->px < bound_box->x1)
						{
							bound_box->x1 = temp_vert->px;
						}
						if (temp_vert->px > bound_box->x2)
						{
							bound_box->x2 = temp_vert->px;
						}

						if (temp_vert->py < bound_box->y1)
						{
							bound_box->y1 = temp_vert->py;
						}
						if (temp_vert->py > bound_box->y2)
						{
							bound_box->y2 = temp_vert->py;
						}

						if (temp_vert->v.z > bound_box->z2)
						{
							bound_box->z2 = temp_vert->v.z;
						}

						if (temp_vert->v.z < bound_box->z1)
						{
							bound_box->z1 = temp_vert->v.z;
							bound_box->w  = ooz;
						}
					}
				}
				else
				{
					for (j=0; j<num_poly_verts; j++)
					{
						r1_vert * temp_vert = &t_vertices[q->vertex_ref[j]];

						float ooz = temp_vert->w;

						if (ooz > nearest_w)
						{
							nearest_w=ooz;
						}
					}
				}

				i4_float twidth = nearest_w * q->texture_scale * texture_scale * center_x * 2;

				r_api->use_texture(q->material_ref, i4_f_to_i(twidth), current_frame);

				g1_stat_counter.increment(g1_statistics_counter_class::OBJECT_POLYS);
				g1_stat_counter.increment(g1_statistics_counter_class::TOTAL_POLYS);
				r_api->render_poly(num_poly_verts,t_vertices,q->vertex_ref);
			}
			else
			{
				r1_vert temp_buf_1[64];
				r1_vert temp_buf_2[64];
				r1_vert * clipped_poly;

				clipped_poly = r_api->clip_poly(&num_poly_verts,
												t_vertices,
												q->vertex_ref,
												temp_buf_1,
												temp_buf_2,
												R1_CLIP_NO_CALC_OUTCODE
							   );


				if (clipped_poly && num_poly_verts>=3)
				{
					float nearest_w = 0;

					if (!bound_box)
					{
						r1_vert * temp_vert = clipped_poly;

						for (j=0; j<num_poly_verts; j++, temp_vert++)
						{
							i4_float ooz = r1_ooz(temp_vert->v.z);

							temp_vert->w  = ooz;
							temp_vert->px = temp_vert->v.x * ooz * center_x + center_x;
							temp_vert->py = temp_vert->v.y * ooz * center_y + center_y;

							if (ooz > nearest_w)
							{
								nearest_w=ooz;
							}
						}
					}
					else
					{
						r1_vert * temp_vert = clipped_poly;

						for (j=0; j<num_poly_verts; j++,temp_vert++)
						{
							float ooz = r1_ooz(temp_vert->v.z);

							temp_vert->w  = ooz;
							temp_vert->px = temp_vert->v.x * ooz * center_x + center_x;
							temp_vert->py = temp_vert->v.y * ooz * center_y + center_y;

							if (ooz > nearest_w)
							{
								nearest_w=ooz;
							}

							if (temp_vert->px < bound_box->x1)
							{
								bound_box->x1 = temp_vert->px;
							}
							if (temp_vert->px > bound_box->x2)
							{
								bound_box->x2 = temp_vert->px;
							}

							if (temp_vert->py < bound_box->y1)
							{
								bound_box->y1 = temp_vert->py;
							}
							if (temp_vert->py > bound_box->y2)
							{
								bound_box->y2 = temp_vert->py;
							}

							if (temp_vert->v.z > bound_box->z2)
							{
								bound_box->z2 = temp_vert->v.z;
							}

							if (temp_vert->v.z < bound_box->z1)
							{
								bound_box->z1 = temp_vert->v.z;
								bound_box->w = ooz;
							}
						}
					}

					i4_float twidth = nearest_w * q->texture_scale * texture_scale * center_x * 2;
					r_api->use_texture(q->material_ref, i4_f_to_i(twidth), current_frame);
					r_api->render_poly(num_poly_verts,clipped_poly);

					g1_stat_counter.increment(g1_statistics_counter_class::OBJECT_POLYS);
					g1_stat_counter.increment(g1_statistics_counter_class::TOTAL_POLYS);
				}
			}
		}
	}

	if (g1_tint!=G1_TINT_OFF && g1_hurt_tint==0)
	{
		r_api->set_color_tint(0);
	}

	//free(t_vertices);
	pf_render_object_pack.stop();
}

void g1_render_class::render_object_polys(g1_quad_object_class * obj,
										  i4_transform_class * object_to_view,
										  sw32 current_frame)
{
	int i,j, /*k,*/ num_vertices;
	//r1_vert* t_vertices=0;
	//int src_quad[4];
	i4_transform_class view_transform;

	num_vertices            = obj->num_vertex;
	if (num_vertices>max_t_vertices)
	{
		max_t_vertices=num_vertices+50;
		t_vertices=(r1_vert *)realloc(t_vertices,max_t_vertices*sizeof(r1_vert));
	}
	r1_vert * v              = t_vertices;
	g1_vert_class * src_vert = obj->get_verts(0, current_frame);

	w8 ANDCODE = 0xFF;
	w8 ORCODE  = 0;

	g1_vert_class * src_v=src_vert;

	pf_render_object_transform.start();

	view_transform.x.x = object_to_view->x.x * scale_x;
	view_transform.x.y = object_to_view->x.y * scale_y;
	view_transform.x.z = object_to_view->x.z;
	view_transform.y.x = object_to_view->y.x * scale_x;
	view_transform.y.y = object_to_view->y.y * scale_y;
	view_transform.y.z = object_to_view->y.z;
	view_transform.z.x = object_to_view->z.x * scale_x;
	view_transform.z.y = object_to_view->z.y * scale_y;
	view_transform.z.z = object_to_view->z.z;
	view_transform.t.x = object_to_view->t.x * scale_x;
	view_transform.t.y = object_to_view->t.y * scale_y;
	view_transform.t.z = object_to_view->t.z;

	i4_float adjuster = -0.01f;

	for (i=0; i<num_vertices; i++, src_v++, v++)
	{
		fast_transform(&view_transform,src_v->v,v->v);

		//v->v.x *= scale_x; //this is accomplished by the above matrix multiplies
		//v->v.y *= scale_y;

		w8 code = r1_calc_outcode(v);
		ANDCODE &= code;
		ORCODE  |= code;
		if (!code)
		{
			//valid point
			i4_float ooz = r1_ooz(v->v.z);
			i4_float ooz_adjusted = r1_ooz(v->v.z + adjuster);

			v->px = v->v.x * ooz * center_x + center_x;
			v->py = v->v.y * ooz * center_y + center_y;

			v->w  = ooz_adjusted;

			v->a = v->r = v->g = v->b = 1.f;
		}
	}

	pf_render_object_transform.stop();

	if (ANDCODE)
	{
		//free(t_vertices);
		return;
	}

	pf_render_object_pack.start();

	g1_quad_class * q_ptr = obj->quad;

	for (i=0; i<obj->num_quad; i++, q_ptr++)
	{
		sw32 num_poly_verts = q_ptr->num_verts();

		if (ORCODE==0)
		{
			r_api->render_poly(num_poly_verts,t_vertices,q_ptr->vertex_ref);
		}
		else
		{
			r1_vert temp_buf_1[64];
			r1_vert temp_buf_2[64];
			r1_vert * clipped_poly;

			clipped_poly = r_api->clip_poly(&num_poly_verts,
											t_vertices,
											q_ptr->vertex_ref,
											temp_buf_1,
											temp_buf_2,
											R1_CLIP_NO_CALC_OUTCODE
						   );

			if (clipped_poly && num_poly_verts>=3)
			{
				r1_vert * temp_vert = clipped_poly;

				for (j=0; j<num_poly_verts; j++, temp_vert++)
				{
					float ooz = r1_ooz(temp_vert->v.z);
					float ooz_adjusted = r1_ooz(temp_vert->v.z + adjuster);

					temp_vert->px = temp_vert->v.x * ooz * center_x + center_x;
					temp_vert->py = temp_vert->v.y * ooz * center_y + center_y;

					temp_vert->w  = ooz_adjusted;
				}

				r_api->render_poly(num_poly_verts,clipped_poly);
			}
		}
	}
	//free(t_vertices);
	pf_render_object_pack.stop();
}

void g1_render_class::render_3d_line(const i4_3d_point_class &p1,
									 const i4_3d_point_class &p2,
									 i4_color color1,
									 i4_color color2,
									 i4_transform_class * t,
									 i4_bool draw_in_front_of_everything)
{
	r1_vert v[2];


	project_point(p1, v[0], t);
	project_point(p2, v[1], t);

	// Ignored for now, since it doesn't work (any line using this flag is always clipped away)
	//if (draw_in_front_of_everything)
	//{
	//  v[0].v.z=r1_near_clip_z;//g1_near_z_range();
	//  v[1].v.z=r1_near_clip_z;//g1_near_z_range();
	//
	//  float ooz = r1_ooz(r1_near_clip_z);//r1_ooz(g1_near_z_range());
	//
	//  v[0].w = r1_near_clip_z;
	//  v[1].w = r1_near_clip_z;
	//}

	v[0].r=g1_table_0_255_to_0_1[(color1&0xff0000)>>16];
	v[0].g=g1_table_0_255_to_0_1[(color1&0xff00)>>8];
	v[0].b=g1_table_0_255_to_0_1[(color1&0xff)>>0];
	v[0].a=1;
	v[0].s=0;
	v[0].t=0;

	v[1].r=g1_table_0_255_to_0_1[(color2&0xff0000)>>16];
	v[1].g=g1_table_0_255_to_0_1[(color2&0xff00)>>8];
	v[1].b=g1_table_0_255_to_0_1[(color2&0xff)>>0];
	v[1].a=1;
	v[1].s=0;
	v[1].t=0;

	r1_shading_type oldshade=r_api->get_shade_mode();
	r1_alpha_type oldalpha=r_api->get_alpha_mode();

	r_api->set_shading_mode(R1_COLORED_SHADING);
	r_api->set_alpha_mode(R1_ALPHA_DISABLED);
	r_api->disable_texture();

	r1_clip_render_lines(1, v, center_x, center_y, r_api);
	//restore old modes. Since lines are realy seldom used primitives,
	//this is no performance problem compared to what might happen if
	//the caller unexspectedly finds the render device in a new state.
	r_api->set_shading_mode(oldshade);
	r_api->set_alpha_mode(oldalpha);
	//r_api->use_default_texture();
	//r_api->set_constant_color(0xffffff);
}

void g1_render_class::render_2d_point(int px, int py, i4_color color)
{

	if (px>0 && py>0 && px<center_x*2-1 && py<center_y*2-1)
	{
		r_api->r1_render_api_class::clear_area(px-1,py-1,px+1,py+1,color,
											   r1_near_clip_z);
	}
}

void g1_render_class::render_3d_point(const i4_3d_point_class &p1, i4_color color, i4_transform_class * t)
{
	r1_vert v;

	project_point(p1, v, t);

	int ix = int (v.px),iy = int (v.py);
	if (ix>0 && iy>0 && ix<center_x*2-1 && iy<center_y*2-1)
	{
		r_api->r1_render_api_class::clear_area(ix-1,iy-1,ix+1,iy+1,color,
											   v.v.z);
	}
}

void g1_draw_vert_line(float x, float y1, float y2, r1_vert * v)
{
	r1_vert * v1=v, * v2=v+1;

	v1->px=x;
	v2->px=x;
	v1->py=y1;
	v2->py=y2;
	g1_render.r_api->render_lines(1, v);
}




void g1_draw_horz_line(float y, float x1, float x2, r1_vert * v)

{
	r1_vert * v1=v, * v2=v+1;

	v1->py=y;
	v2->py=y;
	v1->px=x1;
	v2->px=x2;
	g1_render.r_api->render_lines(1, v);
}
/*! The following function draws a border around a given unit
   Together with a health bar indicating the remaining health of the unit
   @param box The screen bounding box of the object (from the draw routine)
   @param for_who The object itself
 */

void g1_render_class::draw_outline(g1_screen_box * box, g1_object_class * for_who)
{
	if ((box->flags&g1_screen_box::DRAWN))
	{
		return;
	}
	box->flags|=g1_screen_box::DRAWN;
	pf_render_draw_outline.start();
	r1_vert line_points[3];
	//memset(line_points,0,sizeof(r1_vert)*3);

	sw32 width  = (sw32)(center_x*2);
	sw32 height = (sw32)(center_y*2);

	//dunno whats up w/these.. looks like the box is ending up outside the viewport somehow

	sw32 box_x1 = i4_f_to_i(box->x1);
	sw32 box_y1 = i4_f_to_i(box->y1);
	sw32 box_x2 = i4_f_to_i(box->x2);
	sw32 box_y2 = i4_f_to_i(box->y2);
	if (box_y1<6)
	{
		box_y1=6; //draw the health-bar even if the unit
		//is at the top of the screen.
		if (box_y2<8)
		{
			box_y2=8;
		}          //to avoid that the bottom becomes higher than the top
	}

	//box_x1++;
	//box_y1++;
	//box_x2--;
	//box_y2--;

	if (box_x1 < 0 || box_y1 < 0 || box_x2<0 || box_y2<0)
	{
		pf_render_draw_outline.stop();
		return;
	}

	if (box_x2 > width || box_x1>width || box_y2 > height || box_y1 > height)
	{
		pf_render_draw_outline.stop();
		return;
	}

	r_api->disable_texture();

	i4_float depth   = box->z1-0.1f;
	i4_float oodepth = box->z1-0.1f; //box->w;

	line_points[0].v.z = depth;
	line_points[0].w   = oodepth;
	line_points[0].s=0;
	line_points[0].t=0;
	line_points[1].v.z = depth;
	line_points[1].w   = oodepth;
	line_points[1].s=0;
	line_points[1].t=0;
	line_points[2].v.z = depth;
	line_points[2].w   = oodepth;
	line_points[2].s=0;
	line_points[2].t=0;
	r_api->set_alpha_mode(R1_ALPHA_DISABLED);


	if (for_who->player_num==0)
	{
		line_points[0].r   = 0.5f; //0.25f;
		line_points[0].g   = 0.5f; //0.25f;
		line_points[0].b   = 0.5f; //0.f;
		line_points[0].a   = 0.5f;
		line_points[1].r   = 0.5f;
		line_points[1].g   = 0.5f;
		line_points[1].b   = 0.5f; //0.f;
		line_points[1].a   = 0.5f;
		line_points[2].r   = 0.5f; //0.25f;
		line_points[2].g   = 0.5f; //0.25f;
		line_points[2].b   = 0.5f; //0.f;
		line_points[2].a   = 0.5f;
		r_api->set_constant_color(0xffffffff);
	}
	else
	{
		g1_player_info_class * p=g1_player_man.get(for_who->player_num);
		float r = ((p->map_player_color >> 16) & 0xff) / 255.f;
		float g = ((p->map_player_color >> 8) & 0xff) / 255.f;
		float b = (p->map_player_color & 0xff) / 255.0f;
		line_points[0].r   = r;
		line_points[0].g   = g;
		line_points[0].b   = b;
		line_points[1].r   = r;
		line_points[1].g   = g;
		line_points[1].b   = b;
		line_points[2].r   = r;
		line_points[2].g   = g;
		line_points[2].b   = b;
		r_api->set_constant_color(0xffff0000);
	}

	//dunno whats up w/these.. looks like the box is ending up outside the viewport somehow
	//box->x1 += 1;
	//box->y1 += 1;



	float width_adjust  = (float)((box_x2 - box_x1)/6);
	float height_adjust = (float)((box_y2 - box_y1)/6);

	line_points[0].px = (float)box_x1;
	line_points[0].py = box_y1 + height_adjust;

	line_points[1].px = (float)box_x1;
	line_points[1].py = (float)box_y1;

	line_points[2].px = box_x1 + width_adjust;
	line_points[2].py = (float)box_y1;


	r_api->disable_texture();
	//The box must be at least 10 pixels wide for drawing the health.
	if (for_who && (box_x1+10<=box_x2) && box_y1>=5)
	{
		//mp=g1_map_piece_class::cast(for_who);
		if (for_who->health>0)
		{
			float percent=(float) ((for_who->health - for_who->get_min_health())/(float) ( for_who->get_max_health()-for_who->get_min_health() ) );

			//protect against invalid health values
			if (percent > 1.f)
			{
				percent = 1.f;
			}
			if (percent < 0.f)
			{
				percent = 0.f;
			}

			i4_color c;

			if (percent<=0.25)
			{
				c=0xff0000;
			}                 // red
			else if (percent<=0.5)
			{
				c=0xffff00;
			}                 // yellow
			else
			{
				c=0x00ff00;
			}                 // green

			r_api->clear_area(box_x1, box_y1-5, box_x2, box_y1-2, 0, box->z1);

			int box_w=(box_x2-1)-(box_x1+1);
			r_api->clear_area(box_x1+1, box_y1-4, box_x1+1+(int)(box_w*percent), box_y1-3, c, box->z1-0.01f);
		}
	}
	//r_api->use_default_texture();
	r_api->set_shading_mode(R1_COLORED_SHADING);
	r_api->render_lines(2,line_points);

	line_points[0].px = box_x2 - width_adjust;
	line_points[0].py = (float)box_y1;

	line_points[1].px = (float)box_x2;
	line_points[1].py = (float)box_y1;

	line_points[2].px = (float)box_x2;
	line_points[2].py = box_y1 + height_adjust;

	r_api->render_lines(2,line_points);

	line_points[0].px = (float)box_x2;
	line_points[0].py = box_y2 - height_adjust;

	line_points[1].px = (float)box_x2;
	line_points[1].py = (float)box_y2;

	line_points[2].px = box_x2 - width_adjust;
	line_points[2].py = (float)box_y2;

	r_api->render_lines(2,line_points);

	line_points[0].px = box_x1 + width_adjust;
	line_points[0].py = (float)box_y2;

	line_points[1].px = (float)box_x1;
	line_points[1].py = (float)box_y2;

	line_points[2].px = (float)box_x1;
	line_points[2].py = box_y2 - height_adjust;

	r_api->render_lines(2,line_points);

	pf_render_draw_outline.stop();
}

int g1_sprite_depth_compare(const void * a, const void * b)
{
	if (((g1_post_draw_sprite_struct *)a)->z<((g1_post_draw_sprite_struct *)b)->z)
	{
		return -1;
	}
	else if (((g1_post_draw_sprite_struct *)a)->z>((g1_post_draw_sprite_struct *)b)->z)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

sw32 g1_render_class::add_post_draw_vert(r1_vert &a)
{
	g1_post_draw_verts[g1_num_post_draw_verts] = a;
	g1_num_post_draw_verts++;

	return (g1_num_post_draw_verts-1);
}

sw32 g1_render_class::add_post_draw_quad(g1_post_draw_quad_class &q)
{
	g1_post_draw_quads[g1_num_post_draw_quads] = q;
	g1_num_post_draw_quads++;

	return (g1_num_post_draw_quads-1);
}


void g1_render_class::post_draw_quads()
{
	int i,j;

	if (g1_num_post_draw_quads)
	{

		pf_post_draw_quads.start();


		r1_vert temp_buf_1[8];
		r1_vert temp_buf_2[8];

		r_api->disable_texture();
		r_api->set_alpha_mode(R1_ALPHA_LINEAR);
		//    r_api->set_shading_mode(R1_COLORED_SHADING);
		//    r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR);

		int t = g1_num_post_draw_quads;

		for (i=0; i<t; i++)
		{
			g1_post_draw_quad_class * q = &g1_post_draw_quads[i];

			sw32 num_poly_verts = (q->vert_ref[3]==0xFFFF) ? (3) : (4);

			r1_vert * clipped_poly = r_api->clip_poly(&num_poly_verts,
													  g1_post_draw_verts,
													  q->vert_ref,
													  temp_buf_1,
													  temp_buf_2,
													  0);

			if (clipped_poly && num_poly_verts>=3)
			{
				for (j=0; j<num_poly_verts; j++)
				{
					float ooz = r1_ooz(clipped_poly[j].v.z);

					clipped_poly[j].px = clipped_poly[j].v.x * ooz * center_x + center_x;
					clipped_poly[j].py = clipped_poly[j].v.y * ooz * center_y + center_y;
					clipped_poly[j].w  = ooz;
				}
				r_api->render_poly(num_poly_verts,clipped_poly);
			}
		}

		r_api->set_alpha_mode(R1_ALPHA_DISABLED);
		g1_num_post_draw_quads = 0;
		g1_num_post_draw_verts = 0;

		pf_post_draw_quads.stop();

	}

	qsort(g1_post_draw_sprites, g1_t_post_draw_sprites, sizeof(g1_post_draw_sprite_struct),
		  g1_sprite_depth_compare);

	for (i=0; i<g1_t_post_draw_sprites; i++)
	{
		g1_post_draw_sprite_struct * s=g1_post_draw_sprites + i;
		r1_clip_render_textured_rect(s->x1,s->y1,s->x2,s->y2, s->z,
									 1.0,
									 i4_f_to_i(g1_render.center_x*2),
									 i4_f_to_i(g1_render.center_y*2),
									 s->tex, 0, g1_render.r_api, s->s1, s->t1, s->s2, s->t2);
		g1_stat_counter.increment(g1_statistics_counter_class::SPRITES);
	}
	r_api->states_have_changed=i4_T;
	r_api->flush_vert_buffer();

	g1_t_post_draw_sprites=0;

}

void g1_render_class::clip_render_quad(g1_quad_class * q,
									   r1_vert * verts,
									   i4_transform_class * t,
									   int current_frame)
{
	int i;
	i4_3d_vector p;

	w8 ANDCODE = 0xFF;
	w8 ORCODE  = 0;


	for (i=0; i<4; i++)
	{
		int vref=q->vertex_ref[i];

		r1_3d_point_class * v=&verts[vref].v;

		p=i4_3d_vector(v->x, v->y, v->z);

		i4_3d_vector &temp_v = (i4_3d_vector &)verts[vref].v;
		t->transform(p, temp_v);

		verts[vref].v.x *= scale_x;
		verts[vref].v.y *= scale_y;


		w8 code = r1_calc_outcode(verts+vref);
		ANDCODE &= code;
	}

	if (ANDCODE)
	{
		return ;
	}

	sw32 num_poly_verts = 4;
	r1_vert temp_buf_1[32];
	r1_vert temp_buf_2[32];


	r1_vert * clipped_poly = r_api->clip_poly(&num_poly_verts,
											  verts,
											  q->vertex_ref,
											  temp_buf_1,
											  temp_buf_2,
											  R1_CLIP_NO_CALC_OUTCODE);


	if (clipped_poly && num_poly_verts>=3)
	{
		float nearest_w = 0;

		int j;
		for (j=0; j<num_poly_verts; j++)
		{
			float ooz = r1_ooz(clipped_poly[j].v.z);
			if (ooz > nearest_w)
			{
				nearest_w=ooz;
			}

			clipped_poly[j].px = clipped_poly[j].v.x * ooz * center_x + center_x;
			clipped_poly[j].py = clipped_poly[j].v.y * ooz * center_y + center_y;
			clipped_poly[j].w  = ooz;
		}

		if (q->material_ref)
		{
			i4_float twidth = nearest_w * q->texture_scale * center_x * 2;
			r_api->use_texture(q->material_ref, i4_f_to_i(twidth), current_frame);
		}

		r_api->render_poly(num_poly_verts, clipped_poly);
	}
}


void g1_render_class::add_translucent_trail(i4_transform_class * t,
											i4_3d_point_class * spots, int t_spots,
											float start_width, float end_width,
											float start_alpha, float end_alpha,
											w32 sc, w32 ec)

{
	if (t_spots<2)
	{
		return;
	}

	pf_add_translucent_trail.start();

	//make sure there's enough space in our arrays
	if ((t_spots+1)*3 > (max_post_draw_verts - g1_num_post_draw_verts))
	{
		//i4_warning("g1_render::no space for tail vertices");
		pf_add_translucent_trail.stop();
		return;
	}

	if ((t_spots*2)   > (max_post_draw_quads - g1_num_post_draw_quads))
	{
		//i4_warning("g1_render::no space for tail polys");
		pf_add_translucent_trail.stop();
		return;
	}

	i4_float start_r=g1_table_0_255_to_0_1[((sc>>16)&0xff)];
	i4_float start_g=g1_table_0_255_to_0_1[((sc>>8)&0xff)];
	i4_float start_b=g1_table_0_255_to_0_1[((sc>>0)&0xff)];

	i4_float end_r=g1_table_0_255_to_0_1[((ec>>16)&0xff)];
	i4_float end_g=g1_table_0_255_to_0_1[((ec>>8)&0xff)];
	i4_float end_b=g1_table_0_255_to_0_1[((ec>>0)&0xff)];

	int i;
	i4_3d_point_class proj[256];
	i4_3d_point_class * p = proj;

	for (i=0; i<t_spots; i++, p++)
	{
		t->transform(spots[i], *p);
	}

	float width      = start_width;
	float width_step = (end_width-start_width)/(t_spots-1);
	float next_width = start_width;

	float alpha_step = (end_alpha-start_alpha)/(t_spots-1);
	float r_step     = (end_r-start_r)/(t_spots-1);
	float g_step     = (end_g-start_g)/(t_spots-1);
	float b_step     = (end_b-start_b)/(t_spots-1);

	i4_2d_vector perp0( -(proj[1].y-proj[0].y),  proj[1].x-proj[0].x);

	if (perp0.x==0 && perp0.y==0)
	{
		perp0.x=1;
	}
	else
	{
		perp0.normalize();
	}

	float &ooxscale = scale_x; //ooscale_x;
	float &ooyscale = scale_y; //ooscale_y;

	int sv = g1_num_post_draw_verts;

	i4_float r = start_r;
	i4_float g = start_g;
	i4_float b = start_b;
	i4_float edge_alpha = 0.1f;

	add_smoke_vert((proj[0].x + perp0.x * width) * ooxscale,
				   (proj[0].y + perp0.y * width) * ooyscale,
				   proj[0].z, r,g,b, edge_alpha);

	add_smoke_vert((proj[0].x) * ooxscale,
				   (proj[0].y) * ooyscale,
				   proj[0].z, r,g,b, start_alpha);


	add_smoke_vert((proj[0].x - perp0.x * width) * ooxscale,
				   (proj[0].y - perp0.y * width) * ooyscale,
				   proj[0].z, r,g,b, edge_alpha);


	for (i=1; i<t_spots; i++)
	{
		next_width  += width_step;
		r           += r_step;
		g           += g_step;
		b           += b_step;
		start_alpha += alpha_step;

		i4_2d_vector perp_last, perp_next, perp;

		perp_last.x =-(proj[i].y-proj[i-1].y);
		perp_last.y = (proj[i].x-proj[i-1].x);

		if (perp_last.x==0 && perp_last.y==0)
		{
			perp_last.x=1;
		}
		else
		{
			perp_last.normalize();
		}

		if (i==t_spots-1)
		{
			perp=perp_last;
		}
		else
		{
			perp_next.x =-(proj[i+1].y-proj[i].y);
			perp_next.y = (proj[i+1].x-proj[i].x);

			if (perp_next.x==0 && perp_next.y==0)
			{
				perp_next.x=1;
			}
			else
			{
				perp_next.normalize();
			}

			perp.x=perp_last.x + perp_next.x;
			perp.y=perp_last.y + perp_next.y;

			if (perp.x==0 && perp.y==0)
			{
				perp.x=1;
			}
			else
			{
				perp.normalize();
			}
		}

		add_smoke_vert((proj[i].x + perp.x * next_width) * ooxscale,
					   (proj[i].y + perp.y * next_width) * ooyscale,
					   proj[i].z, r,g,b, edge_alpha);

		add_smoke_vert((proj[i].x) * ooxscale,
					   (proj[i].y) * ooyscale,
					   proj[i].z, r,g,b, start_alpha);

		add_smoke_vert((proj[i].x - perp.x * next_width) * ooxscale,
					   (proj[i].y - perp.y * next_width) * ooyscale,
					   proj[i].z, r,g,b, edge_alpha);

		g1_post_draw_quad_class q1(sv+ i * 3,
								   sv + i * 3+1,
								   sv + i * 3-2,
								   sv + i * 3-3);
		add_post_draw_quad(q1);

		g1_post_draw_quad_class q2(sv +i * 3+1,
								   sv + i * 3+2,
								   sv + i * 3-1,
								   sv + i * 3-2);
		add_post_draw_quad(q2);
	}

	pf_add_translucent_trail.stop();
}


inline g1_map_vertex_class *g1vmin(g1_map_vertex_class * v1,
								   g1_map_vertex_class * v2)
{
	if (v1->get_height_value()<v2->get_height_value())
	{
		return v1;
	}
	else
	{
		return v2;
	}
}


i4_bool g1_render_class::project_point(const i4_3d_point_class &p,
									   r1_vert &v,
									   i4_transform_class * transform)
{
	i4_3d_vector &temp_v = (i4_3d_vector &)v.v;
	/*i4_d3d_vector dtemp_v;
	   i4_dtransform_class dtrans(*transform);

	   dtrans.transform(i4_d3d_point_class(p),dtemp_v);
	   dtemp_v.x *= (double)scale_x;
	   dtemp_v.y *= (double)scale_y;

	   if (dtemp_v.z>0.000001)
	   {
	   double ooz = 0.999999/dtemp_v.z;
	   v.v.x = dtemp_v.x;
	   v.v.y = dtemp_v.y;
	   v.v.z = dtemp_v.z;

	   v.px = dtemp_v.x * ooz * center_x + center_x;
	   v.py = dtemp_v.y * ooz * center_y + center_y;

	   v.w = ooz;

	   return i4_T;
	   }
	   else
	   	{
	   	v.v.x=dtemp_v.x;
	   	v.v.y=dtemp_v.y;
	   	v.v.z=0.001;
	   	return i4_F;
	   	}
	 */
	//this is appropriate enough (and faster)
	transform->transform(p,temp_v);
	v.v.x *= scale_x;
	v.v.y *= scale_y;

	if (v.v.z>0.001)
	{
		float ooz = r1_ooz(v.v.z);

		v.px = v.v.x * ooz * center_x + center_x;
		v.py = v.v.y * ooz * center_y + center_y;

		v.w = ooz;

		return i4_T;
	}
	else
	{
		v.v.z=0.001f;
		return i4_F;
	}

}

w8 g1_render_class::point_classify(const i4_3d_point_class &p,
								   i4_transform_class * transform)
{

	i4_3d_vector temp;

	transform->transform(p,temp);
	//temp.x *= scale_x;  //although project_point above is exactly doing the same,
	//temp.y *= scale_y;  //this is wrong here (causes point to be off by a factor for the larger of the two)
	w8 code=0;
	if (temp.z<r1_near_clip_z)
	{
		//no further investigation ( a real vertex can't be outside of
		//all planes)
		code=0xff;
		return code;
	}
	float ooz=r1_ooz(temp.z);
	float px,py;
	px = temp.x * ooz *center_x+ center_x;
	py = temp.y * ooz *center_y+ center_y;
	//render_2d_point(px,py,0xffffff); // for testing purposes only
	if (px<0)
	{
		code|=2;
	}
	if (px>2*center_x)
	{
		code|=1;
	}
	if (py<0)
	{
		code|=8;
	}
	if (py>2*center_y)
	{
		code|=4;
	}
	//if code is non-null it is outside of at least one plane of the frustrum
	return code;
}

i4_bool g1_render_class::point_in_frustrum(const i4_3d_point_class &p,
										   i4_transform_class * transform)
{
	return (point_classify(p,transform)==0);
}

i4_bool g1_render_class::sphere_in_frustrum(const i4_3d_point_class center,
											i4_float size,
											i4_transform_class * transform)
{
	//check wheter all 8 vertices of the cube lie on the same
	//side of the frustrum (otherwise, it might be that the cube
	//is larger than the frustrum and we need to draw anyway)
	//remember that we use this code to check all nodes of the octree,
	//not only the leaves. And it is very probable that the root-node
	//is REALLY large.
	i4_float s=size/2;
	//if the andcode becomes zero at one point, at least one part
	//of the cube lies in the frustrum
	w8 ANDCODE=0xff;

	ANDCODE&=point_classify(i4_3d_point_class(center.x+s,center.y+s,center.z+s),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x-s,center.y+s,center.z+s),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x+s,center.y-s,center.z+s),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x-s,center.y-s,center.z+s),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x+s,center.y+s,center.z-s),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x-s,center.y+s,center.z-s),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x+s,center.y-s,center.z-s),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x-s,center.y-s,center.z-s),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	return i4_F;
}

i4_bool g1_render_class::cube_in_frustrum(const i4_3d_point_class center,
										  i4_float xsize,
										  i4_float ysize,
										  i4_float zsize,
										  i4_transform_class * transform)
{
	//check wheter all 8 vertices of the cube lie on the same
	//side of the frustrum (otherwise, it might be that the cube
	//is larger than the frustrum and we need to draw anyway)
	//remember that we use this code to check all nodes of the octree,
	//not only the leaves. And it is very probable that the root-node
	//is REALLY large.
	i4_float xs=xsize/2;
	i4_float ys=ysize/2;
	i4_float zs=zsize/2;
	//if the andcode becomes zero at one point, at least one part
	//of the cube lies in the frustrum
	w8 ANDCODE=0xff;

	ANDCODE&=point_classify(i4_3d_point_class(center.x+xs,center.y+ys,center.z+zs),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x-xs,center.y+ys,center.z+zs),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x+xs,center.y-ys,center.z+zs),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x-xs,center.y-ys,center.z+zs),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x+xs,center.y+ys,center.z-zs),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x-xs,center.y+ys,center.z-zs),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x+xs,center.y-ys,center.z-zs),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	ANDCODE&=point_classify(i4_3d_point_class(center.x-xs,center.y-ys,center.z-zs),
							transform);
	if (!ANDCODE)
	{
		return i4_T;
	}
	return i4_F;
}

void g1_render_class::cube_in_frustrum(const i4_3d_point_class center,
									   i4_float xsize,
									   i4_float ysize,
									   i4_float zsize,
									   i4_transform_class * transform,
									   w8 &ANDCODE,
									   w8 &ORCODE)
{
	//check wheter all 8 vertices of the cube lie on the same
	//side of the frustrum (otherwise, it might be that the cube
	//is larger than the frustrum and we need to draw anyway)
	//remember that we use this code to check all nodes of the octree,
	//not only the leaves. And it is very probable that the root-node
	//is REALLY large.
	i4_float xs=xsize/2;
	i4_float ys=ysize/2;
	i4_float zs=zsize/2;

	//if the andcode becomes zero at one point, at least one part
	//of the cube lies in the frustrum
	ANDCODE=0xff;
	ORCODE=0x0;
	w8 code;
	code=point_classify(i4_3d_point_class(center.x+xs,center.y+ys,center.z+zs),
						transform);
	ANDCODE&=code;
	ORCODE|=code;
	code=point_classify(i4_3d_point_class(center.x-xs,center.y+ys,center.z+zs),
						transform);
	ANDCODE&=code;
	ORCODE|=code;
	code=point_classify(i4_3d_point_class(center.x+xs,center.y-ys,center.z+zs),
						transform);
	ANDCODE&=code;
	ORCODE|=code;
	code=point_classify(i4_3d_point_class(center.x-xs,center.y-ys,center.z+zs),
						transform);
	ANDCODE&=code;
	ORCODE|=code;
	code=point_classify(i4_3d_point_class(center.x+xs,center.y+ys,center.z-zs),
						transform);
	ANDCODE&=code;
	ORCODE|=code;
	code=point_classify(i4_3d_point_class(center.x-xs,center.y+ys,center.z-zs),
						transform);
	ANDCODE&=code;
	ORCODE|=code;
	code=point_classify(i4_3d_point_class(center.x+xs,center.y-ys,center.z-zs),
						transform);
	ANDCODE&=code;
	ORCODE|=code;
	code=point_classify(i4_3d_point_class(center.x-xs,center.y-ys,center.z-zs),
						transform);


}


void g1_render_class::draw_rectangle(int sx1, int sy1, int sx2, int sy2, i4_color col,
									 i4_draw_context_class &context)
{
	sw32 x1,y1,x2,y2;

	if (sx1<sx2)
	{
		x1=sx1;
		x2=sx2;
	}
	else
	{
		x1=sx2;
		x2=sx1;
	}
	if (sy1<sy2)
	{
		y1=sy1;
		y2=sy2;
	}
	else
	{
		y1=sy2;
		y2=sy1;
	}

	r_api->set_shading_mode(R1_SHADE_DISABLED);

	r1_clip_clear_area(x1,y1,x2,y1, col, 0.01f, context, r_api);
	r1_clip_clear_area(x2,y1,x2,y2, col, 0.01f, context, r_api);
	r1_clip_clear_area(x1,y2,x2,y2, col, 0.01f, context, r_api);
	r1_clip_clear_area(x1,y1,x1,y2, col, 0.01f, context, r_api);
}


void g1_setup_tri_texture_coords(r1_vert * tri1, r1_vert * tri2,
								 int cell_rotation, int cell_is_mirrored)
{
	float u[4]={
		1,0,0,1
	};
	float v[4]={
		1,1,0,0
	};

	int on_remap[4]={
		1,0,3,2
	};
	int dir=cell_is_mirrored ? 1 : 3, on=on_remap[cell_rotation];

	tri1[0].s=u[on];
	tri1[0].t=v[on];
	on=(on+dir)&3;

	tri1[1].s=u[on];
	tri1[1].t=v[on];
	on=(on+dir)&3;

	tri1[2].s=u[on];
	tri1[2].t=v[on];


	on=on_remap[cell_rotation];

	tri2[0].s=u[on];
	tri2[0].t=v[on];
	on=(on+dir+dir)&3;

	tri2[1].s=u[on];
	tri2[1].t=v[on];
	on=(on+dir)&3;

	tri2[2].s=u[on];
	tri2[2].t=v[on];
}




void g1_render_class::render_sprite(const i4_3d_vector &p,
									r1_texture_handle tex,
									float sprite_width, float sprite_height,
									float s1, float t1, float s2, float t2)
{

	if (p.z >= r1_near_clip_z)
	{
		if (g1_t_post_draw_sprites<G1_MAX_SPRITES)
		{
			g1_post_draw_sprite_struct * s=g1_post_draw_sprites + g1_t_post_draw_sprites;

			float ooz = r1_ooz(p.z);

			i4_float xs = g1_render.center_x * ooz * g1_render.scale_x;
			i4_float ys = g1_render.center_y * ooz * g1_render.scale_y;

			float cx=g1_render.center_x + p.x*xs;
			float cy=g1_render.center_y + p.y*ys;
			float w=sprite_width * g1_render.center_x * ooz;
			float h=sprite_height * g1_render.center_x * ooz;

			s->x1=cx-w/2;
			s->y1=cy-h/2;
			s->x2=cx+w/2;
			s->y2=cy+h/2;

			s->z=p.z;
			s->tex=tex;
			s->s1=s1;
			s->t1=t1;
			s->s2=s2;
			s->t2=t2;

			g1_t_post_draw_sprites++;
		}
	}
}


void g1_render_class::render_near_sprite(float px, float py,
										 r1_texture_handle tex,
										 float sprite_width, float sprite_height,
										 float s1, float t1, float s2, float t2)
{
	if (g1_t_post_draw_sprites<G1_MAX_SPRITES)
	{
		g1_post_draw_sprite_struct * s=g1_post_draw_sprites + g1_t_post_draw_sprites;
		s->x1=px-sprite_width/2;
		s->y1=py-sprite_height/2;
		s->x2=px+sprite_width/2;
		s->y2=py+sprite_height/2;

		s->z=r1_near_clip_z*1.1f;
		s->tex=tex;
		s->s1=s1;
		s->t1=t1;
		s->s2=s2;
		s->t2=t2;

		g1_t_post_draw_sprites++;
	}
}
