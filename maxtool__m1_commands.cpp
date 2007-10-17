/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
//#include "max.h"
#include "pch.h"
#include "maxtool/m1_info.h"
#include "maxtool/st_edit.h"
#include "maxtool/render2.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "maxtool/max_object.h"
#include "file/file.h"
#include "error/error.h"
#include "gui/smp_dial.h"
#include "octree.h"

li_object *m1_wireframe_toggle(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_WIREFRAME, ~m1_info.get_flags(M1_WIREFRAME));
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_toggle_octree(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_SHOW_OCTREE_SUBDIVISIONS,
					  ~m1_info.get_flags(M1_SHOW_OCTREE_SUBDIVISIONS));
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_select_none(li_object * o, li_environment * env)
{
	// deselect all the old polys
	if (m1_info.obj==NULL)
	{
		return 0;
	}                             //No object loaded, cannot unselect anything.
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		m1_info.obj->quad[i].set_flags(g1_quad_class::SELECTED,0);
	}

	m1_render_window->request_redraw();
	m1_st_edit->edit_poly_changed();

	return 0;
}

li_object *m1_select_all(li_object * o, li_environment * env)
{
	// select all polys (usefull command if object has no textures jet)
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		m1_info.obj->quad[i].set_flags(g1_quad_class::SELECTED,g1_quad_class::ON);
	}

	m1_render_window->request_redraw();
	m1_st_edit->edit_poly_changed();

	return 0;
}

li_object *m1_select_similar(li_object * o, li_environment * env)
{
	i4_str * texture=0;
	int i,j;

	if (m1_info.obj==NULL)
	{
		return 0;
	}                              //useless command if no object loaded
	for (i=0; i<m1_info.obj->num_quad; i++)
	{
		if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
		{
			texture = m1_info.obj->texture_names[i];

			// non-optimal search (lots of reselection)
			for (j=0; j<m1_info.obj->num_quad; j++)
			{
				if (*m1_info.obj->texture_names[j] == *texture)
				{
					m1_info.obj->quad[j].set_flags(g1_quad_class::SELECTED);
				}
			}
		}
	}

	if (!texture)
	{
		return 0;
	}

	m1_render_window->request_redraw();
	m1_st_edit->edit_poly_changed();

	return 0;
}

li_object *m1_rotate_texture_selected(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		g1_quad_class * q = &m1_info.obj->quad[i];
		if (q->get_flags(g1_quad_class::SELECTED))
		{
			i4_float u=q->u[0],v=q->v[0], nu,nv;
			for (int j=q->num_verts()-1; j>=0; j--)
			{
				nu = q->u[j];
				nv = q->v[j];
				q->u[j] = u;
				q->v[j] = v;
				u = nu;
				v = nv;
			}
		}
	}
	m1_info.obj->calc_texture_scales();
	m1_render_window->request_redraw();


	return 0;
}

li_object *m1_reverse_texture_selected(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		g1_quad_class * q = &m1_info.obj->quad[i];
		if (q->get_flags(g1_quad_class::SELECTED))
		{
			int j = q->num_verts()-1;
			i4_float t;

			// swap uv coords
			t = q->u[1];
			q->u[1] = q->u[j];
			q->u[j] = t;
			t = q->v[1];
			q->v[1] = q->v[j];
			q->v[j] = t;
		}
	}
	m1_render_window->request_redraw();
	m1_info.obj->calc_texture_scales();

	return 0;
}

li_object *m1_toggle_bothsided_selected(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		g1_quad_class * q = &m1_info.obj->quad[i];
		if (q->get_flags(g1_quad_class::SELECTED))
		{
			if (q->get_flags(g1_quad_class::BOTHSIDED))
			{
				q->set_flags(g1_quad_class::BOTHSIDED,0);
			}
			else
			{
				q->set_flags(g1_quad_class::BOTHSIDED);
			}
		}
	}
	m1_info.obj->calc_texture_scales();
	m1_render_window->request_redraw();

	return 0;
}

li_object *m1_flip_normal_selected(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		g1_quad_class * q = &m1_info.obj->quad[i];
		if (q->get_flags(g1_quad_class::SELECTED))
		{
			int j = q->num_verts()-1;
			i4_float t;
			w16 ref;

			// swap references
			ref = q->vertex_ref[1];
			q->vertex_ref[1] = q->vertex_ref[j];
			q->vertex_ref[j] = ref;

			// swap uv coords
			t = q->u[1];
			q->u[1] = q->u[j];
			q->u[j] = t;
			t = q->v[1];
			q->v[1] = q->v[j];
			q->v[j] = t;
		}
	}
	m1_info.obj->calc_vert_normals();
	m1_info.obj->calc_texture_scales();
	m1_render_window->request_redraw();

	return 0;
}

li_object *m1_recalc_normals(li_object * o, li_environment * env)
{
	m1_info.obj->calc_extents();
	m1_info.obj->calc_vert_normals();
	m1_info.obj->calc_texture_scales();
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_default_coordinates(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		g1_quad_class * q = &m1_info.obj->quad[i];
		if (q->get_flags(g1_quad_class::SELECTED))
		{
			q->u[0] = 0;
			q->v[0] = 0;
			q->u[1] = 1;
			q->v[1] = 0;
			q->u[2] = 1;
			q->v[2] = 1;
			q->u[3] = 0;
			q->v[3] = 1;
		}
	}
	m1_info.obj->calc_texture_scales();
	m1_render_window->request_redraw();

	return 0;
}

li_object *m1_tint_selected(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
		{
			m1_info.obj->quad[i].set_flags(g1_quad_class::TINT);
		}
	}
	m1_render_window->request_redraw();

	return 0;
}

li_object *m1_untint_selected(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
		{
			m1_info.obj->quad[i].set_flags(g1_quad_class::TINT,0);
		}
	}
	m1_render_window->request_redraw();

	return 0;
}

li_object *m1_get_object_info(li_object * o, li_environment * env)
{
	if (!m1_info.obj)
	{
		return 0;
	}
	i4_warning("Current object has %i vertices and %i faces.",m1_info.obj->num_vertex,m1_info.obj->num_quad);
	i4_warning("Currently editing frame %i of %i from animation %i of %i.",
			   m1_info.current_frame+1, //show 1 of 1 instead of 0 of 1 if single frame
			   m1_info.obj->animation[m1_info.current_animation].num_frames,
			   m1_info.current_animation+1,
			   m1_info.obj->num_animations);

	return 0;
}

li_object *m1_add_anim(li_object * o, li_environment * env)
{
	if (!m1_info.obj)
	{
		return 0;
	}
	i4_str name("default");
	if (i4_input_box("Create new animation","Enter the name of the new animation","untitled_animation",name,MSG_INPUTBOX+MSG_OKCANCEL)==MSG_CANCEL)
	{
		return 0;
	}
	m1_info.obj->add_anim(name);
	m1_info.current_animation=m1_info.obj->num_animations-1;
	m1_info.current_frame=0;
	return 0;
}

li_object *m1_rename_anim(li_object * o, li_environment * env)
{
	if (!m1_info.obj)
	{
		return 0;
	}
	i4_str * oldptr=m1_info.obj->animation_names[m1_info.current_animation];
	i4_str oldname(*oldptr);
	i4_str newname("newnametemp");
	if (i4_input_box("Rename animation","Rename the animation",oldname,newname,MSG_INPUTBOX+MSG_OKCANCEL)==MSG_CANCEL)
	{
		return 0;
	}
	m1_info.obj->change_animation_name(m1_info.current_animation,newname);
	return 0;
}

li_object *m1_add_mount(li_object * o, li_environment * env)
{
	if (!m1_info.obj)
	{
		return 0;
	}
	i4_str name("default");
	if (i4_input_box("Create new mount point","Enter the name of the new mount point","untitled_mount",name,MSG_INPUTBOX+MSG_OKCANCEL)==MSG_CANCEL)
	{
		return li_nil;
	}
	m1_info.obj->add_mount(name);
	return li_true_sym;
}

i4_3d_vector m1_vector_input(const i4_const_str &msg, i4_3d_vector oldvalue)
{
	i4_3d_vector retval;

	i4_str strvectout("out");
	i4_const_str afl("( %f , %f , %f )");
	i4_3d_vector &v=oldvalue;
	i4_str * strvectin=afl.sprintf(100,v.x,
								   v.y,v.z);

	if (i4_input_box("Vector",msg,*strvectin,
					 strvectout,MSG_OK+MSG_CANCEL)==MSG_CANCEL)
	{
		return oldvalue;
	}
	char buf[200];
	i4_float x=0,y=0,z=0;
	i4_os_string(strvectout,buf,200);
	if (sscanf(buf,"( %f , %f , %f )",&x,&y,&z)<3)
	{
		i4_message_box("Invalid Input","You entered something that cannot be interpreted as a vector.",MSG_OK);
		return oldvalue;
	}
	retval.x=x;
	retval.y=y;
	retval.z=z;
	return retval;
}

li_object *m1_scale_object(li_object * o, li_environment * env)
{
	if (!m1_info.obj)
	{
		return 0;
	}
	i4_3d_vector scale=m1_vector_input(
		"Enter the tripplet to scale the object with",
		i4_3d_vector(1.0f,1.0f,1.0f));
	//This one needs to be regenerated afterwards.
	li_call("remove_octree",0,0);
	g1_vert_class * v=m1_info.obj->get_verts(m1_info.current_animation,
											 m1_info.current_frame);
	for (int i=0; i<m1_info.obj->num_vertex; i++,v++)
	{
		v->v.x*=scale.x;
		v->v.y*=scale.y;
		v->v.z*=scale.z;
	}
	return li_true_sym;
}

li_object *m1_move_object(li_object * o, li_environment * env)
{
	if (!m1_info.obj)
	{
		return 0;
	}
	i4_3d_vector move=m1_vector_input(
		"Enter the amount you want the object to move",
		i4_3d_vector(0,0,0));
	//This one needs to be regenerated afterwards.
	li_call("remove_octree",0,0);
	g1_vert_class * v=m1_info.obj->get_verts(m1_info.current_animation,
											 m1_info.current_frame);
	for (int i=0; i<m1_info.obj->num_vertex; i++,v++)
	{
		v->v.x+=move.x;
		v->v.y+=move.y;
		v->v.z+=move.z;
	}
	return li_true_sym;
}



li_object *m1_delete_anim(li_object * o, li_environment * env)
{
	if (!m1_info.obj)
	{
		return 0;
	}
	int num=m1_info.current_animation;
	if (m1_info.obj->num_animations==1)
	{
		i4_message_box("Remove animation","Cannot remove last animation of object.",MSG_CANCEL);
		return 0;
	}
	if (num==0)
	{
		i4_message_box("Remove animation","Cannot remove the default animation.",MSG_CANCEL);
		return 0;
	}
	if (i4_message_box("Remove animation", "Delete current animation including all frames?",MSG_YESNO)==MSG_NO)
	{
		return 0;
	}
	m1_info.obj->remove_anim(num);
	m1_info.current_animation=0;
	return 0;
}

li_object *m1_add_vertex(li_object * o, li_environment * env)
{
	if (!m1_info.obj)
	{
		return 0;
	}
	int vertno=m1_info.obj->add_vertex();
	m1_info.set_flags(M1_SHOW_ORPHANS);
	m1_info.num_invalid_vertices++;
	i4_warning("Added vertex number %i",vertno);
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_remove_vertex(li_object * o, li_environment * env)
{
	int vertno=0;

	if (!m1_info.obj)
	{
		return 0;
	}
	if (m1_info.preselect_point>=0)
	{
		vertno=m1_info.preselect_point;
	}
	else
	{
		if (m1_info.selected_index>0)
		{
			vertno=m1_info.selected_points[m1_info.selected_index-1];
		}
		else
		{
			i4_message_box("Delete Vertex","No vertex selected",MSG_CANCEL);
			return 0;
		}
	}
	if (i4_message_box("Delete Vertex","Remove the selected vertex together with all associated quads?",MSG_YESNO)==MSG_NO)
	{
		return 0;
	}

	m1_info.preselect_point=-1;
	if (vertno<0)
	{
		return 0;
	}
	m1_info.obj->remove_vertex(vertno);
	i4_warning("Removed vertex %i",vertno);
	m1_render_window->request_redraw();
	return 0;
}


li_object *m1_add_quad(li_object * o, li_environment * env)
//adds a quad, this is a face of an object.
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	m1_poly_object_class * obj=m1_info.obj;
	int num_points=0;
	int points[4]={
		0,0,0,0xffff
	};

	num_points=m1_info.selected_index;
	if (num_points<3||num_points>4)
	{
		i4_error("USER: add_quad: Must select 3 or 4 points (shift-click to select)");
		return 0;
	}
	points[0]=m1_info.selected_points[0];
	points[1]=m1_info.selected_points[1];
	points[2]=m1_info.selected_points[2];
	if (num_points==4)
	{
		points[3]=m1_info.selected_points[3];
	}

	m1_info.selected_index=0;
	int myquadindex=obj->add_quad();
	obj->quad[myquadindex].set(points[0],points[1],points[2],points[3]);
	obj->quad_store[myquadindex].flags=0;
	obj->quad_store[myquadindex].set_material(0);
	obj->quad_store[myquadindex].texture_scale = 1.0;
	obj->quad_store[myquadindex].u[0] = 0.0;
	obj->quad_store[myquadindex].v[0] = 0.0;
	obj->quad_store[myquadindex].u[1] = 1.0;
	obj->quad_store[myquadindex].v[1] = 0.0;
	obj->quad_store[myquadindex].u[2] = 1.0;
	obj->quad_store[myquadindex].v[2] = 1.0;
	obj->quad_store[myquadindex].u[3] = 0.0;
	obj->quad_store[myquadindex].v[3] = 1.0;
	delete obj->texture_names[myquadindex]; //delete empty entry generated by add_quad
	obj->texture_names[myquadindex]=new i4_str("textures/solid_white.jpg");
	//texture_names[i] belongs to quad[i], so adding it here will give same index
	i4_warning("Info: Added a side with vertices %i %i %i %i.",points[0],points[1],points[2],points[3]);
	obj->calc_texture_scales();
	obj->calc_vert_normals();
	obj->calc_extents();

	m1_info.texture_list_changed();
	for (int i=0; i<obj->num_quad; i++)
	{
		obj->quad[i].set_flags(g1_quad_class::SELECTED,0);
	}
	obj->quad[myquadindex].set_flags(g1_quad_class::SELECTED);
	m1_st_edit->edit_poly_changed();
	li_call("navigate"); //switch back to navigate mode with new quad selected
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_delete_selected_quad(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	if (i4_message_box("Delete Quad","This will delete all selected quads. Go ahead?",MSG_YESNO)==MSG_NO)
	{
		return 0;
	}
	m1_poly_object_class * obj=m1_info.obj;
	if (obj)
	{
		int shift=0;

		for (int i=0; i<obj->num_quad; i++)
		{
			/*if (obj->quad[i].get_flags(g1_quad_class::SELECTED))
			   shift++;
			   else
			   if (shift>0)
			   {
			   	obj->quad[i-shift]=obj->quad[i];
			   	obj->texture_names[i-shift]=obj->texture_names[i];
			   }*/
			if (obj->quad[i].get_flags(g1_quad_class::SELECTED))
			{
				obj->remove_quad(i);
				//numquads has decreased, retry on same index
				i4_warning("Deleted quad %i.",i--);
			}
		}
		//obj->num_quad-=shift;

		m1_render_window->request_redraw();
	}
	return 0;
}

li_object *m1_join_coords(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	m1_poly_object_class * obj=m1_info.obj;

	i4_float u=0,v=0;
	int set=0;

	if (obj)
	{
		for (int j=0; j<obj->num_quad; j++)
		{
			g1_quad_class * q=obj->quad+j;
			if (q->get_flags(g1_quad_class::SELECTED))
			{
				for (w32 i=0; i<q->num_verts(); i++)
				{
					if (obj->get_poly_vert_flag(j, 1<<i))
					{
						if (set)
						{
							q->u[i] = u;
							q->v[i] = v;
						}
						else
						{
							set = 1;
							u = q->u[i];
							v = q->v[i];
						}
					}
				}
			}
		}

		m1_render_window->request_redraw();
	}
	return 0;
}


li_object *m1_team1(li_object * o, li_environment * env)
{
	m1_info.current_team=1;
	m1_render_window->request_redraw();
	return 0;
}


li_object *m1_team2(li_object * o, li_environment * env)
{
	m1_info.current_team=2;
	m1_render_window->request_redraw();
	return 0;
}



li_object *m1_team3(li_object * o, li_environment * env)
{
	m1_info.current_team=3;
	m1_render_window->request_redraw();
	return 0;
}



li_object *m1_team4(li_object * o, li_environment * env)
{
	m1_info.current_team=4;
	m1_render_window->request_redraw();
	return 0;
}


li_object *m1_team_default(li_object * o, li_environment * env)
{
	m1_info.current_team=0;
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_back_black(li_object * o, li_environment * env)
{
	m1_info.bg_color=0;
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_back_red(li_object * o, li_environment * env)
{
	m1_info.bg_color=0xff0000;
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_back_white(li_object * o, li_environment * env)
{
	m1_info.bg_color=0xffffff;
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_back_blue(li_object * o, li_environment * env)
{
	m1_info.bg_color=0xff;
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_back_darkblue(li_object * o, li_environment * env)
{
	m1_info.bg_color=0x2f;
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_back_green(li_object * o, li_environment * env)
{
	m1_info.bg_color=0xff00;
	m1_render_window->request_redraw();
	return 0;
}


li_object *m1_recenter(li_object * o, li_environment * env)
{
	m1_render_window->recenter();
	return 0;
}

li_object *m1_axis_toggle(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_SHOW_AXIS, ~m1_info.get_flags(M1_SHOW_AXIS));
	m1_render_window->request_redraw();
	return 0;
}


li_object *m1_toggle_shading(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_SHADING, ~m1_info.get_flags(M1_SHADING));
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_toggle_names(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_SHOW_FACE_NAMES, ~m1_info.get_flags(M1_SHOW_FACE_NAMES));
	m1_render_window->request_redraw();
	return 0;
}



li_object *m1_toggle_numbers(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_SHOW_FACE_NUMBERS, ~m1_info.get_flags(M1_SHOW_FACE_NUMBERS));
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_toggle_origin(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_SHOW_ORIGIN, ~m1_info.get_flags(M1_SHOW_ORIGIN));
	m1_render_window->request_redraw();
	return 0;
}


li_object *m1_toggle_vnumbers(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_SHOW_VERT_NUMBERS, ~m1_info.get_flags(M1_SHOW_VERT_NUMBERS));
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_toggle_orphans(li_object * o, li_environment * env)
{
	m1_info.set_flags(M1_SHOW_ORPHANS, ~m1_info.get_flags(M1_SHOW_ORPHANS));
	m1_render_window->request_redraw();
	return 0;
}

li_object *m1_swap_polynums(li_object * o, li_environment * env)
{
	int q1=-1,q2=-1;

	if (m1_info.obj==NULL)
	{
		return 0;
	}
	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		g1_quad_class * q = &m1_info.obj->quad[i];
		if (q->get_flags(g1_quad_class::SELECTED))
		{
			if (q1==-1)
			{
				q1=i;
			}
			else if (q2==-1)
			{
				q2=i;
			}
			else
			{
				return 0;
			}
		}
	}

	if (q1!=-1 && q2!=-1)
	{
		m1_poly_object_class * obj=m1_info.obj;

		i4_str * tmp=obj->texture_names[q1];
		obj->texture_names[q1]=obj->texture_names[q2];
		obj->texture_names[q2]=tmp;


		g1_quad_class tq;
		tq=obj->quad_store[q1];
		obj->quad_store[q1]=obj->quad_store[q2];
		obj->quad_store[q2]=tq;

		obj->quad[q1]=obj->quad_store[q1];
		obj->quad[q2]=obj->quad_store[q2];
	}
	return 0;
}


li_object *m1_dump_polys(li_object * o, li_environment * env)
{
	if (m1_info.obj==NULL)
	{
		return 0;
	}
	i4_file_class * fp=i4_open("dump.scm", I4_WRITE);
	if (!fp)
	{
		return 0;
	}

	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		g1_quad_class * q = &m1_info.obj->quad[i];

		char tname[256];
		i4_os_string(*m1_info.obj->texture_names[i], tname,  256);

		int first=1, j;
		if (q->u[0]>=0.47 &&
			q->u[1]>=0.47 &&
			q->u[2]>=0.47 &&
			q->u[3]>=0.47)
		{
			first=0;
		}


		for (j=0; j<4; j++)
		{
			if (first)
			{
				q->u[j]*=2;
				m1_info.obj->texture_names[i]=new i4_str("x:\\crack\\golgotha\\textures\\nuk_sky1.tga");
			}
			else
			{
				q->u[j]=q->u[j]*2.0f-1.0f;
				m1_info.obj->texture_names[i]=new i4_str("x:\\crack\\golgotha\\textures\\nuk_sky1.tga");
			}
		}


		fp->printf("(setup_quad %d %s %f %f  %f %f  %f %f  %f %f)\n",
				   i, tname,
				   q->u[0], q->v[0],
				   q->u[1], q->v[1],
				   q->u[2], q->v[2],
				   q->u[3], q->v[3]);
	}
	delete fp;
	return 0;
}

//reload maxtool textures (should be called if something returns DDERR_SURFACELOST)
li_object *m1_reload_textures(li_object * o, li_environment * env)
{
	r1_texture_manager_class * t;

	t=m1_info.r_api->get_tmanager(m1_info.tman_index);

	if (t&&(m1_info.textures.size()>0))
	{
		t->reopen();
		m1_info.textures_loaded=i4_F;
		i4_warning("Attempting to reload maxtool textures");
		t->textures_loaded=i4_F;
		if (!t->load_textures())
		{
			i4_warning("Reloading failed");
			return li_nil;
		}
		t->textures_loaded=i4_T;
		i4_warning("Reloaded maxtool textures");
		return li_true_sym;
	}
	return li_nil;
}

//reload the main texture manager (to be called if something returns
//DDERR_SURFACELOST)
li_object *g1_reload_textures(li_object * o, li_environment * env)
{
	r1_texture_manager_class * t;

	t=m1_info.r_api->get_tmanager();
	i4_warning("Attempting to reload main textures");
	t->reopen();
	t->textures_loaded=i4_F;
	if (!t->load_textures())
	{
		i4_warning("Reloading failed");
		return li_nil;
	}
	t->textures_loaded=i4_T;
	i4_warning("Reloaded main textures");
	return li_true_sym;
}

li_object *m1_animation_rewind(li_object * o, li_environment * env)
//{{{
{
	m1_info.current_animation = 0;
	m1_info.current_frame=0; //also reset this, or we might point to an inexistent frame
	m1_info.time = 0;
	m1_render_window->update_object(m1_info.time);
	m1_st_edit->edit_poly_changed();
	m1_render_window->request_redraw();

	return 0;
}
//}}}

li_object *m1_animation_advance(li_object * o, li_environment * env)
//{{{
{
	m1_poly_object_class * obj = m1_info.obj;

	if (!obj)
	{
		return 0;
	}
	if (++m1_info.current_animation==obj->num_animations)
	{
		m1_info.current_animation = 0;
	}
	m1_info.current_frame=0;
	m1_info.time = 0;
	m1_render_window->update_object(m1_info.time);
	m1_st_edit->edit_poly_changed();
	m1_render_window->request_redraw();

	return 0;
}
//}}}

li_object *m1_animation_back(li_object * o, li_environment * env)
//{{{
{
	m1_poly_object_class * obj = m1_info.obj;

	if (!obj)
	{
		return 0;
	}
	if (--m1_info.current_animation<0)
	{
		m1_info.current_animation = obj->num_animations-1;
	}

	m1_info.time =0;
	m1_info.current_frame=0;
	m1_render_window->update_object(m1_info.time);
	m1_st_edit->edit_poly_changed();
	m1_render_window->request_redraw();

	return 0;
}
//Todo: Check that these functions have unique names (no overlapping with level-editor)


li_object *m1_update_textures(li_object * o, li_environment * env)
{
	m1_info.texture_list_changed();
	return 0;
}
//li_automatic_add_function(m1_update_textures, "update_textures");


li_object *m1_build_octree(li_object * o, li_environment * env)
{
	if (m1_info.obj==0)
	{
		return 0;
	}
	delete m1_info.obj->octree;
	m1_info.obj->octree=0;
	m1_info.obj->octree=g1_octree::Build(m1_info.obj);
	g1_octree * oc=m1_info.obj->octree;
	if (oc)
	{
		i4_warning("Octree built.");
	}
	else
	{
		i4_warning("Octree NOT built, to few vertices in object.");
	}
	return li_true_sym;
}

li_object *m1_remove_octree(li_object * o, li_environment * env)
{
	if (m1_info.obj && m1_info.obj->octree)
	{
		delete m1_info.obj->octree;
		m1_info.obj->octree=0;
	}
	return 0;
}

class li_add_m1_commands_class :
	public i4_init_class
{
	int init_type()
	{
		return I4_INIT_TYPE_LISP_FUNCTIONS;
	}
	void init()
	{
		li_add_function("build_octree",m1_build_octree);
		li_add_function("remove_octree",m1_remove_octree);
		li_add_function("dump_polys", m1_dump_polys);
		li_add_function("update_textures",m1_update_textures);
		li_add_function("axis_toggle", m1_axis_toggle);
		li_add_function("recenter", m1_recenter);
		li_add_function("select_none", m1_select_none);
		li_add_function("select_all", m1_select_all);
		li_add_function("select_similar", m1_select_similar);
		li_add_function("wireframe", m1_wireframe_toggle);
		li_add_function("rotate_texture_sel", m1_rotate_texture_selected);
		li_add_function("reverse_texture_sel", m1_reverse_texture_selected);
		li_add_function("flip_normal_sel", m1_flip_normal_selected );
		li_add_function("toggle_bothsided_sel", m1_toggle_bothsided_selected);
		li_add_function("default_coords", m1_default_coordinates);
		li_add_function("tint_sel", m1_tint_selected);
		li_add_function("untint_sel", m1_untint_selected);
		li_add_function("add_quad", m1_add_quad);
		li_add_function("delete_sel", m1_delete_selected_quad);
		li_add_function("join_coords", m1_join_coords);
		li_add_function("m1_recalc", m1_recalc_normals);

		li_add_function("add_vertex", m1_add_vertex);
		li_add_function("delete_vertex", m1_remove_vertex);
		li_add_function("add_mount", m1_add_mount);

		li_add_function("add_animation", m1_add_anim);
		li_add_function("delete_animation", m1_delete_anim);
		li_add_function("rename_animation", m1_rename_anim);
		li_add_function("animation_advance", m1_animation_advance);
		li_add_function("animation_rewind",m1_animation_rewind);
		li_add_function("animation_back", m1_animation_back);

		li_add_function("m1_team_1", m1_team1);
		li_add_function("m1_team_2", m1_team2);
		li_add_function("m1_team_3", m1_team3);
		li_add_function("m1_team_4", m1_team4);
		li_add_function("no_tint", m1_team_default);
		li_add_function("toggle_shading", m1_toggle_shading);

		li_add_function("back_black", m1_back_black);
		li_add_function("back_red", m1_back_red);
		li_add_function("back_white", m1_back_white);
		li_add_function("back_blue", m1_back_blue);
		li_add_function("back_darkblue", m1_back_darkblue);
		li_add_function("back_green", m1_back_green);


		li_add_function("toggle_names", m1_toggle_names);
		li_add_function("toggle_numbers", m1_toggle_numbers);
		li_add_function("toggle_vnumbers", m1_toggle_vnumbers);
		li_add_function("toggle_origin",m1_toggle_origin);
		li_add_function("toggle_orphans", m1_toggle_orphans);
		li_add_function("swap_polynums", m1_swap_polynums);
		li_add_function("toggle_octree", m1_toggle_octree);

		li_add_function("m1_scale_object",m1_scale_object);
		li_add_function("m1_move_object",m1_move_object);

		li_add_function("reload_max_textures", m1_reload_textures);
		li_add_function("reload_main_textures", g1_reload_textures);

		li_add_function("max_get_object_info", m1_get_object_info);


	}

} li_add_m1_commands_class_instance;
