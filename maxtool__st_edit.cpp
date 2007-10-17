/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
//#include "max.h"

#include "pch.h"
#include "maxtool/st_edit.h"
#include "maxtool/m1_info.h"
#include "window/window.h"
#include "render/gtext_load.h"
#include "image/image.h"
#include "maxtool/max_object.h"
#include "window/win_evt.h"
#include "device/kernel.h"
#include "maxtool/render2.h"
#include "app/app.h"
#include "window/wmanager.h"
#include "gui/text_input.h"
#include "render/tmanage.h"
#include "lisp/lisp.h"
#include "gui/smp_dial.h"

#define HANDLE_SIZE   3
#define SNAP_DISTANCE 5

i4_float m1_st_edit_window_class::twidth() const
{
	i4_float retval=255.9999f;

	if (texture)
	{
		retval=texture->width()-0.0001f;
	}
	//return (texture)? texture->width()-0.0001 : 255.9999;
	if (retval>255.9999)
	{
		retval=255.9999f;
	}
	return retval;
}

i4_float m1_st_edit_window_class::theight() const
{
	//return (texture)? texture->height()-0.0001 : 255.9999;
	i4_float retval=(texture) ? texture->height()-0.0001f : 255.9999f;

	if (retval>255.9999)
	{
		retval=255.9999f;
	}
	return retval;
}

void m1_st_edit_window_class::get_point(int poly, int num, int &x, int &y)
{
	if (m1_info.obj)
	{
		g1_quad_class * q=m1_info.obj->quad+poly;
		float tw=twidth(), th=theight();

		x=(int)(q->u[num] * tw);
		y=(int)(q->v[num] * th);
	}
}

void m1_st_edit_window_class::draw(i4_draw_context_class &context)
{
	local_image->clear(0, context);

	if (texture)
	{
		texture->put_image(local_image, 0,0, context);
	}

	if (m1_info.obj)
	{
		w32 c=0x0000ff; //(254<<16)|(2<<8)|166;
		int x1,y1,x2,y2;

		for (int j=0; j<m1_info.obj->num_quad; j++)
		{
			if (m1_info.obj->quad[j].get_flags(g1_quad_class::SELECTED))
			{
				int i; //do NOT change to unsigned!

				g1_quad_class * q=&m1_info.obj->quad[j];

				get_point(j,0,x2,y2);
				for (i=q->num_verts()-1; i>=0; i--)
				{
					get_point(j,i,x1,y1);
					local_image->line(x1,y1,x2,y2,  c, context);
					x2 = x1;
					y2 = y1;
				}

				for (i=0; i<q->num_verts(); i++)
				{
					int x,y;
					get_point(j,i,x,y);
					if (m1_info.obj->get_poly_vert_flag(j,1<<i))
					{
						local_image->bar(x-2,y-2,x+2,y+2, 0xffff00, context);
					}
					else
					{
						local_image->bar(x-2,y-2,x+2,y+2, 0x808080, context);
					}
				}
			}
		}
		if (preselect_x>=0)
		{
			local_image->bar(preselect_x-2,preselect_y-2,preselect_x+2,preselect_y+2, 0xff00ff, context);
		}
	}
}

void m1_st_edit_window_class::drag_points(int xc, int yc)
{
	m1_poly_object_class * obj=m1_info.obj;

	if (!obj)
	{
		return;
	}

	float tw=twidth(), th=theight();
	float u_change=xc/tw, v_change=yc/th;
	i4_bool change=i4_F;

	for (int i=0; i<m1_info.obj->num_quad; i++)
	{
		if (obj->quad[i].get_flags(g1_quad_class::SELECTED))
		{
			for (w32 j=0; j<obj->quad[i].num_verts(); j++)
			{
				if (obj->get_poly_vert_flag(i, 1<<j))
				{
					obj->quad[i].u[j]+=u_change;
					if (obj->quad[i].u[j]<0)
					{
						obj->quad[i].u[j]=0;
					}
					if (obj->quad[i].u[j]>1)
					{
						obj->quad[i].u[j]=1;
					}

					obj->quad[i].v[j]+=v_change;
					if (obj->quad[i].v[j]<0)
					{
						obj->quad[i].v[j]=0;
					}
					if (obj->quad[i].v[j]>1)
					{
						obj->quad[i].v[j]=1;
					}
					change=i4_T;
				}
			}
		}
	}

	if (change)
	{
		obj->calc_texture_scales();
		request_redraw(i4_F);
		if (m1_render_window.get())
		{
			m1_render_window->request_redraw(i4_F);
		}
	}
}

i4_bool m1_st_edit_window_class::verts_are_selected()
{
	if (!m1_info.obj)
	{
		return 0;
	}

	for (w32 j=0; j<m1_info.obj->num_quad; j++)
	{
		if (m1_info.obj->quad[j].get_flags(g1_quad_class::SELECTED))
		{
			for (w32 i=0; i<m1_info.obj->quad[j].num_verts(); i++)
			{
				if (m1_info.obj->get_poly_vert_flag(j,1<<i))
				{
					return i4_T;
				}
			}
		}
	}

	return i4_F;
}

void m1_st_edit_window_class::select_point(int point)
{
	if (!m1_info.obj)
	{
		return;
	}

	if (point<0)
	{
		return;
	}

	m1_poly_object_class * obj=m1_info.obj;

	if (!i4_current_app->get_window_manager()->shift_pressed())
	{
		for (int j=0; j<obj->num_quad; j++)
		{
			for (int i=0; i<4; i++)
			{
				obj->set_poly_vert_flag(j, 1<<i, 0);
			}
		}
	}

	for (int j=0; j<obj->num_quad; j++)
	{
		g1_quad_class * q=obj->quad+j;
		if (q->get_flags(g1_quad_class::SELECTED))
		{
			for (w32 i=0; i<q->num_verts(); i++)
			{
				if (q->vertex_ref[i] == point)
				{
					obj->set_poly_vert_flag(j, 1<<i, 1);
				}
			}
		}
	}

	request_redraw();
}

void m1_st_edit_window_class::change_current_verts()
{
	if (!m1_info.obj)
	{
		return;
	}

	int old_px = preselect_x;
	int old_py = preselect_y;

	preselect_x = -1;

	if (m1_info.preselect_point>=0)
	{
		for (w32 j=0; j<m1_info.obj->num_quad; j++)
		{
			if (m1_info.obj->quad[j].get_flags(g1_quad_class::SELECTED))
			{
				for (w32 i=0; i<m1_info.obj->quad[j].num_verts(); i++)
				{
					if (m1_info.obj->quad[j].vertex_ref[i] == m1_info.preselect_point)
					{
						get_point(j,i,preselect_x,preselect_y);
					}
				}
			}
		}
	}

	if (preselect_x!=old_px || preselect_y!=old_py)
	{
		request_redraw();
	}
}

void m1_st_edit_window_class::change_current_texture(i4_const_str new_name)
{
	if (m1_info.obj)
	{
		i4_file_class * fp=i4_open(new_name);
		if (!fp)
		{
			i4_message_box("File not found","Could not find the specified file. Nothing will happen.",MSG_OK);
			return;
		}
		delete fp;
		//Hint: We currently assume the textures directory is
		//at the same location as the exe file (not on a cdrom or something)
		i4_str * relpath=i4_relative_path(new_name);
		if (relpath->strstr("textures")!=relpath->begin()) //we aren't already in the textures folder
		{
			i4_filename_struct fn;
			i4_split_path(new_name,fn);
			char targetn[256];
			char targetp[256];
			char targetext[256]; //well, actually the sum must not exceed this...
			i4_os_string(fn.filename,targetn,256);
			i4_os_string(fn.extension,targetext,256);
			sprintf(targetp,"textures/%s.%s",fn.filename,fn.extension);
			if (i4_message_box("Add new texture?","The file you selected exists, but is not in the textures folder. Copy it there?",MSG_YES+MSG_CANCEL)==MSG_CANCEL)
			{
				delete relpath;
				return;
			}
			i4_copy_file(new_name,targetp);

		}
		delete relpath;
		int i;

		for (i=0; i<m1_info.obj->num_quad; i++)
		{
			if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
			{
				delete m1_info.obj->texture_names[i];
				m1_info.obj->texture_names[i]=new i4_str(new_name);
			}
		}

		m1_info.texture_list_changed();
	}
}


void m1_st_edit_window_class::receive_event(i4_event * ev)
{
	int pre_grab = grab;

	m1_poly_object_class * obj=m1_info.obj;

	if (!obj)
	{
		return ;
	}

	switch (ev->type())
	{
		case i4_event::WINDOW_MESSAGE:
			{
				CAST_PTR(wev, i4_window_message_class, ev);
				if (wev->sub_type==i4_window_message_class::GOT_DROP)
				{
					i4_kernel.send_event(tname_edit, ev);
				}
			} break;

		case i4_event::OBJECT_MESSAGE:
			{
				CAST_PTR(tc, i4_text_change_notify_event, ev);
				if (tc->object==tname_edit && tc->new_text && m1_info.obj)
				{
					i4_file_class * fp=i4_open(*tc->new_text);
					if (!fp)
					{
						return;
					}
					delete fp;
					int i;

					for (i=0; i<m1_info.obj->num_quad; i++)
					{
						if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
						{
							m1_info.obj->texture_names[i]=new i4_str(*tc->new_text);
						}
					}
					li_call("distribute_sel"); //distribute the newly inserted texture
					m1_info.texture_list_changed();
				}


			} break;

		case i4_event::MOUSE_BUTTON_DOWN:
			{
				CAST_PTR(bev, i4_mouse_button_down_event_class, ev);

				if (bev->left())
				{
					grab |= LEFT;
				}
				if (bev->right())
				{
					grab |= RIGHT;
				}
				if (bev->center())
				{
					grab |= MIDDLE;
				}

				i4_window_request_key_grab_class kgrab(this);
				i4_kernel.send_event(parent, &kgrab);

				i4_bool clear_old=i4_T;
				int sel_poly=-1, sel_vert=-1;

				if (bev->left() && obj)
				{
					for (int j=0; j<obj->num_quad; j++)
					{
						g1_quad_class * q=obj->quad+j;
						if (q->get_flags(g1_quad_class::SELECTED))
						{
							for (w32 i=0; i<q->num_verts(); i++)
							{
								int x,y;
								get_point(j,i,x,y);
								if (abs(bev->x-x)<HANDLE_SIZE && abs(bev->y-y)<HANDLE_SIZE)
								{
									if (obj->get_poly_vert_flag(j, 1<<i))
									{
										clear_old=i4_F;
									}
									else
									{
										obj->set_poly_vert_flag(j, 1<<i, 1);
									}

									sel_poly=j;
									sel_vert=i;
								}
							}
						}
					}
				}

				if (clear_old && !i4_current_app->get_window_manager()->shift_pressed())
				{
					for (int j=0; j<obj->num_quad; j++)
					{
						for (int i=0; i<4; i++)
						{
							obj->set_poly_vert_flag(j, 1<<i, 0);
						}
					}

					if (sel_poly!=-1)
					{
						obj->set_poly_vert_flag(sel_poly, 1<<sel_vert, 1);
					}
				}

				request_redraw(i4_T);
			} break;

		case i4_event::MOUSE_BUTTON_UP:
			{
				CAST_PTR(bev, i4_mouse_button_up_event_class, ev);

				if (bev->left())
				{
					grab &= ~LEFT;
				}
				if (bev->right())
				{
					grab &= ~RIGHT;
				}
				if (bev->center())
				{
					grab &= ~MIDDLE;
				}
			} break;

		case i4_event::MOUSE_MOVE:
			{
				CAST_PTR(mev, i4_mouse_move_event_class, ev);

				int old_px = preselect_x, old_py = preselect_y;

				preselect_x = -1;

				if (m1_info.obj)
				{
					if (grab && verts_are_selected())
					{
						int snap_x = mev->x, snap_y = mev->y;

						for (int j=0; j<m1_info.obj->num_quad; j++)
						{
							g1_quad_class * q=m1_info.obj->quad+j;
							if (q->get_flags(g1_quad_class::SELECTED))
							{
								for (w32 i=0; i<q->num_verts(); i++)
								{
									if (!m1_info.obj->get_poly_vert_flag(j,1<<i))
									{
										int x,y;
										get_point(j,i,x,y);
										if (abs(mev->x-x)<SNAP_DISTANCE && abs(mev->y-y)<SNAP_DISTANCE)
										{
											snap_x = x;
											snap_y = y;
										}
									}
								}
							}
						}

						drag_points(snap_x+snap_off_x-mev->lx, snap_y+snap_off_y-mev->ly);

						snap_off_x = mev->x - snap_x;
						snap_off_y = mev->y - snap_y;
					}
					else
					{
						for (int j=0; j<m1_info.obj->num_quad; j++)
						{
							g1_quad_class * q=m1_info.obj->quad+j;
							if (q->get_flags(g1_quad_class::SELECTED))
							{
								for (w32 i=0; i<q->num_verts(); i++)
								{
									int x,y;
									get_point(j,i,x,y);
									if (abs(mev->x-x)<HANDLE_SIZE && abs(mev->y-y)<HANDLE_SIZE)
									{
										preselect_x = x;
										preselect_y = y;
										snap_off_x = mev->x - preselect_x;
										snap_off_y = mev->y - preselect_y;
									}
								}
							}
						}
					}

					if (old_px!=preselect_x || old_py!=preselect_y)
					{
						request_redraw();
					}
				}

			} break;

	}
	if (!pre_grab && grab)
	{
		i4_window_request_mouse_grab_class grab_ev(this);
		i4_kernel.send_event(parent,&grab_ev);
	}
	if (pre_grab && !grab)
	{
		i4_window_request_mouse_ungrab_class grab_ev(this);
		i4_kernel.send_event(parent,&grab_ev);
	}
}



void m1_st_edit_window_class::edit_poly_changed()
{
	if (texture)
	{
		delete texture;
	}
	texture=0;

	if (m1_info.obj)
	{
		m1_poly_object_class * obj=m1_info.obj;

		int sel_poly=-1;
		for (int j=0; j<obj->num_quad; j++)
		{
			if (obj->quad[j].get_flags(g1_quad_class::SELECTED))
			{
				if (sel_poly==-1)
				{
					sel_poly=j;
				}
				else if (sel_poly>=0)
				{
					if (!(*obj->texture_names[sel_poly] == *obj->texture_names[j]))
					{
						sel_poly=-2;
					}
				}
			}
		}

		if (sel_poly==-1)
		{
			tname_edit->change_text("<None Selected>");
		}
		else if (sel_poly==-2)
		{
			tname_edit->change_text("<Multiple Selected>");
		}
		else
		{

			i4_image_class * im[10];
			w32 imid;
			//int t=r1_load_gtext(imid=r1_get_texture_id(*obj->texture_names[sel_poly]), im);
			imid=r1_get_texture_id(*obj->texture_names[sel_poly]);
			int t=0;

			for (int i=1; i<t; i++)
			{
				delete im[i];
			}

			if (t)
			{
				texture=im[0];
			}

			if (texture)
			{
				tname_edit->change_text(*obj->texture_names[sel_poly]);
			}
			else
			{
				tname_edit->change_text("<Texture not found>");
				i4_const_str * n=NULL;
				n=r1_get_texture_name(imid);
				char buf[250],buf2[250];
				i4_os_string(*n,buf,100);
				sprintf(buf2,"textures/%s.jpg",buf);
				texture=im[0]=i4_load_image(i4_const_str(buf2),NULL);
				if (!texture)
				{
					sprintf(buf2,"textures/%s.tga",buf);
					texture=im[0]=i4_load_image(i4_const_str(buf2),NULL);
				}
				delete n;
				if (texture)
				{
					tname_edit->change_text(buf2);
				}
				else
				{
					request_redraw(i4_F);
					return; //Can't execute following code if texture is null.
				}
			}
			int sizex=texture->width();
			int sizey=texture->height();
			i4_float scale;
			//need to do some proportional scaling.
			if (sizex>256||sizey>256) //if neither one is larger, nothing to do
			{
				if (sizex>=sizey)
				{
					scale=(sizex/256.0f);
					sizex=256; //The larger becomes the maximum window size
					sizey=(int) (sizey/scale); //the smaller is proportionally scaled
				}
				else
				{
					scale=(sizey/256.0f);
					sizey=256;
					sizex=(int) (sizex/scale);
				}
			}
			texture=texture->scale_image(0,sizex,sizey,texture->get_pal());
			delete im[0];
		}
	}
	li_call("update_edit_special");
	request_redraw(i4_F);
}


m1_st_edit_window_class::m1_st_edit_window_class(w16 w, w16 h,
												 i4_text_input_class * tname_edit)
	: i4_window_class(w,h),
	  tname_edit(tname_edit)
{
	texture=0;
	dragging=i4_F;
	preselect_x = -1;
	grab=0;
}

m1_st_edit_window_class::~m1_st_edit_window_class()
{
	if (texture)
	{
		delete texture;
	}
	texture=0;
}

i4_event_handler_reference_class<m1_st_edit_window_class> m1_st_edit;
