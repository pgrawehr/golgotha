/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h"
#include "sky.h"
#include "init/init.h"
#include "device/kernel.h"
#include "app/app.h"
#include "lisp/lisp.h"
#include "lisp/li_dialog.h"
#include "g1_render.h"
#include "render/r1_api.h"
#include "render/tmanage.h"
#include "map_man.h"
#include "map.h"
#include "loaders/load.h"
#include "window/window.h"
#include "window/win_evt.h"
#include "window/style.h"
#include "gui/scroll_bar.h"
#include "gui/text.h"
//#include "gui/text_input.h"
#include "resources.h"
#include "objs/model_id.h"
#include "math/vector.h"
#include "math/angle.h"
#include "math/pi.h"
#include "cwin_man.h"
#include "render/gtext_load.h"
#include "camera.h"
#include "draw_context.h"
#include "time/profile.h"
#include "render/r1_clip.h"
#include "render/r1_win.h"
#include "tick_count.h"
#include "lisp/li_class.h"
#include "map_vars.h"
#include "g1_tint.h"
#include "tile.h"
#include "file/file.h"
#include "editor/editor.h"
#include "editor/e_res.h"

li_object_class_member top_cloud_layer("top_cloud_layer"), bottom_cloud_layer("bottom_cloud_layer");
li_float_class_member li_red("red"), li_green("green"), li_blue("blue"), li_alpha("alpha");
li_string_class_member li_cloudname("texture_name");

i4_profile_class pf_draw_sky("g1_draw_sky");

i4_array<i4_str *> g1_sky_list(0,16);
li_object *g1_def_skys(li_object * o, li_environment * env);
static r1_texture_ref sky_texture("cloud2");

li_type_number li_g1_sky_type=0;


class li_g1_sky_function :
	public li_type_function_table
{
public:
	virtual void print(li_object * o, i4_file_class * stream)
	{
		li_g1_sky * s=li_g1_sky::get(o,0);

		li_get_type(s->value()->type())->print(s->value(),stream);
	};
	virtual int equal(li_object * o1, li_object * o2)
	{
		if (o1->type()!=o2->type())
		{
			return i4_F;
		}
		return li_get_type(li_g1_sky::get(o1,0)->value()->type())->equal(
				   li_g1_sky::get(o1,0)->value(),
				   li_g1_sky::get(o2,0)->value());
	}
	virtual char *name()
	{
		return "sky";
	}
	virtual li_object *create(li_object * params, li_environment * env)
	{
		li_object * o=li_eval(li_first(params,env),env);
		li_string * s=li_string::get(o,env);

		if (((li_object *)s==li_nil)||(s==0))
		{
			s=new li_string("");
		}
		return new li_g1_sky(s);
	}
	virtual void save_object(i4_saver_class * fp, li_object * o, li_environment * env)
	{
		li_g1_sky * t=li_g1_sky::get(o,env);

		li_save_object(fp,t->value(),env);
	}
	virtual li_object *load_object(i4_loader_class * fp, li_type_number * type_remap,li_environment * env)
	{
		li_string * s=0;

		s=li_string::get(li_load_object(fp,type_remap,env),env);
		return new li_g1_sky(s);
	}
	virtual void mark(li_object * o,int set)
	{
		li_g1_sky * t=(li_g1_sky *)o;    //must do a force-cast

		//because o might be marked
		o->mark(set);
		li_get_type(t->value()->type())->mark(t->value(),set);
	}
};

g1_sky_view::~g1_sky_view()
{
	//i4_kernel.delete_handler(eh);
	eh=0;

}

void g1_sky_view::draw(i4_draw_context_class &context)
{
	i4_bool valid=i4_F;

	if (off()>=0 && off()<g1_sky_list.size())
	{
		valid=i4_T;
	}

	r1_render_api_class * api=g1_render.r_api;
	api->flush_vert_buffer();
	r1_texture_handle han;
	char tname[256], tname1[256];

	if (valid)
	{
		i4_os_string(*g1_sky_list[off()], tname, 256);
		sprintf(tname1, "%s1", tname);

		i4_bool loaded;

		han=api->get_tmanager()->query_texture(tname, &loaded);
		if (!loaded)
		{
			han=api->get_tmanager()->query_texture(tname1, &loaded);
		}
		if (!loaded)
		{
			han=0;
		}
		//han=api->get_tmanager()->find_texture(r1_get_texture_id(tname));
		//if (!han)
		//  han=api->get_tmanager()->find_texture(r1_get_texture_id(tname1));


	}
	else
	{
		han=-1;
	}



	api->default_state();
	api->set_shading_mode(R1_SHADE_DISABLED);

	if (active)
	{
		api->set_constant_color(0x0000ff);
	}
	else
	{
		api->set_constant_color(0x9f9f9f);
	}

	if (han>=0)
	{
		api->use_texture(han, width(), 0);
	}
	else
	{
		if (valid==i4_T)
		{
			i4_os_string(*g1_sky_list[off()],tname,256);
			i4_warning("Missing sky texture %s, cannot display.", tname);
		}
	}

	r1_vert v[3];
	setup_vert(0,0, v[0]);
	setup_vert(0,1, v[1]);
	setup_vert(1,1, v[2]);
	api->render_poly(3, v);
	//api->flush_vert_buffer();

	setup_vert(0,0, v[0]);
	setup_vert(1,1, v[1]);
	setup_vert(1,0, v[2]);
	api->render_poly(3, v);
	api->flush_vert_buffer();
	request_redraw(); //keep drawing, otherwise the display will lose sync with the scrollbar
};

void g1_sky_view::receive_event(i4_event * ev)
{
	if (ev->type()==i4_event::WINDOW_MESSAGE)
	{
		CAST_PTR(wev, i4_window_message_class, ev);
		if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
		{
			active=i4_T;
			request_redraw(i4_F);
		}
		else if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
		{
			active=i4_F;
			request_redraw(i4_F);
		}
	}
	else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
	{
		CAST_PTR(bev, i4_mouse_button_down_event_class, ev);

		if (bev->left() && off()>=0 && off()<g1_sky_list.size())
		{
			if (eh)
			{
				eh->receive_event(
					new i4_do_command_event_class((*g1_sky_list[off()]).c_str(),off()));
			}
			else
			{
				g1_editor_instance.add_undo(G1_MAP_SKY);

				if (g1_get_map()->sky_name)
				{
					delete g1_get_map()->sky_name;
				}

				g1_get_map()->sky_name=new i4_str(*g1_sky_list[off()]);
			}
			request_redraw(i4_F);
			request_redraw(i4_T);
			li_call("redraw");
		}


	}


};


g1_sky_picker_class::g1_sky_picker_class(i4_graphical_style_class * style,
										 i4_event_handler_class * handler)
	: i4_color_window_class(0,0, style->color_hint->neutral(), style),
	  render_windows(0,32),
	  eh(handler)
{
	int t_vis=5, t_obj=g1_sky_list.size();
	int obj_w=64, obj_h=64, i,x=0, y=0, x1=0;

	for (i=0; i<t_vis; i++)
	{
		render_windows.add(g1_render.r_api->create_render_window(obj_w, obj_h, R1_COPY_1x1));
		render_windows[i]->add_child(0,0, new g1_sky_view(obj_w, obj_h, i,eh));
		add_child(x1,y, render_windows[i]);
		x1+=render_windows[i]->width();
	}
	y+=obj_h;

	i4_scroll_bar * sb=new i4_scroll_bar(i4_F, t_vis * obj_w, t_vis, t_obj, 0, this, style);
	add_child(x,y, sb);

	resize_to_fit_children();
}
int g1_sky_view::sky_scroll_offset=0;

void g1_sky_picker_class::receive_event(i4_event * ev)
{
	i4_color_window_class::receive_event(ev);
	if (ev->type()==i4_event::USER_MESSAGE)
	{
		CAST_PTR(sev, i4_scroll_message, ev);
		g1_sky_view::sky_scroll_offset=sev->amount;
		for (int i=0; i<render_windows.size(); i++)
		{
			render_windows[i]->request_redraw(i4_F);
			render_windows[i]->request_redraw(i4_T);
		}
		request_redraw(i4_T);
		request_redraw(i4_F);
	}
}

g1_sky_picker_class::~g1_sky_picker_class()
{
	//i4_kernel.delete_handler(eh);
	eh=0;

}


class li_sky_change_button_class :
	public i4_button_class
{
public:
	i4_text_window_class * show_name;
	i4_const_str current_name;
	void name(char * buffer)
	{
		static_name(buffer,"sky_change_button");
	}
	void update_name()
	{
		//char buf[200];
		//memset(buf,0,200);

		//sprintf(buf,current_name.c_str());
		//if (strlen(buf)>0)
		//	buf[strlen(buf)-1]=0;

		if (show_name)
		{
			show_name->set_text(new i4_str(current_name.c_str()));
			show_name->request_redraw();
		}

	};


	li_sky_change_button_class(const i4_const_str &help,
							   i4_window_class * child,
							   i4_graphical_style_class * style,
							   i4_text_window_class * show_name,
							   const i4_const_str &_current_name)
		: i4_button_class(&help, child, style),
		  show_name(show_name),
		  current_name(_current_name)
	{
		set_popup(i4_T);
		update_name();
	}


	void do_press()
	{
		g1_sky_view::sky_scroll_offset=0;
		g1_sky_picker_class * sp=new g1_sky_picker_class(i4_current_app->get_style(),this);

		g1_editor_instance.create_modal(sp->width(), sp->height(), "set_sky_title");
		g1_editor_instance.modal_window.get()->add_child(0,0,sp);
		//The name of the button doesn't change, does it?
		i4_button_class::do_press();
		update_name();
	}

	void receive_event(i4_event * ev)
	{
		//A new sky was choosen
		if (ev->type()==i4_event::DO_COMMAND)
		{
			CAST_PTR(dcmd,i4_do_command_event_class,ev);
			current_name=dcmd->command;
			update_name();
			g1_editor_instance.close_modal();
			request_redraw(i4_T);
			request_redraw();
			return;
		}


		i4_button_class::receive_event(ev);
	}
	~li_sky_change_button_class()
	{
		show_name=0; //DO NOT delete this one
	}
};


class li_sky_editor :
	public li_type_edit_class
{
public:
	virtual int create_edit_controls(i4_str name,
									 li_object * o,
									 li_object * property_list,
									 i4_window_class * * windows,
									 int max_windows,
									 li_environment * env)
	{
		if (max_windows<3)
		{
			return 0;
		}
		i4_graphical_style_class * style=i4_current_app->get_style();
		windows[0]=new i4_text_window_class(name, style);
		li_string * str=li_g1_sky::get(o,env)->value();
		//char buf[300];
		//i4_ram_file_class rf(buf, 260);
		//li_get_type(str->type())->print(str, &rf);
		//buf[rf.tell()]=0;
		//buf[rf.tell()-1]=0;
		i4_text_window_class * ti=new i4_text_window_class(str->value(),style);
		if (ti->width()<200)
		{
			ti->resize(200,ti->height());
		}
		windows[1]=ti;
		windows[2]=new li_sky_change_button_class(
			g1_ges("sky_change_help"),
			new i4_text_window_class(g1_ges("sky_change_name"),style),
			style,
			ti,
			str->value());
		return 3;

	};
	virtual li_object *apply_edit_controls(li_object * o,
										   li_object * property_list,
										   i4_window_class * * windows,
										   li_environment * env)
	{
		return new li_g1_sky(((i4_text_window_class *)windows[1])->get_text()->c_str());
	}
} li_sky_edit_instance;

class li_g1_sky_initer :
	public i4_init_class
{
public:
	virtual int init_type()
	{
		return I4_INIT_TYPE_LISP_FUNCTIONS;
	}
	void init()
	{
		li_type_function_table * ty=new li_g1_sky_function;

		li_g1_sky_type=li_add_type(ty);
		li_get_type(li_find_type("sky"))->set_editor(&li_sky_edit_instance);
	};
} li_g1_sky_initer_instance;

void scale_copy(i4_image_class * src, i4_image_class * dst, int sx1, int sy1, int sx2, int sy2)
{

	int dest_x, dest_y;
	float source_x, source_y;
	float source_xstep=(float)(sx2-sx1+1) / (float)dst->width();
	float source_ystep=(float)(sy2-sy1+1) / dst->height();

	i4_draw_context_class c1(0,0, src->width()-1, src->height()-1);
	i4_draw_context_class c2(0,0, dst->width()-1, dst->height()-1);

	int dest_h=dst->height(), dest_w=dst->width();

	for (source_y=(float)sy1, dest_y=0; dest_y<dest_h; dest_y++,  source_y+=source_ystep)
	{
		for (source_x=(float)sx1, dest_x=0; dest_x<dest_w; dest_x++,  source_x+=source_xstep)
		{
			w32 color=src->get_pixel((short)i4_f_to_i(source_x), (short)i4_f_to_i(source_y), c1);
			dst->put_pixel(dest_x, dest_y, color, c2);
		}
	}

}

class g1_sky_class :
	public i4_init_class
{
public:
	i4_str * current_sky_name;
	i4_image_class * sky_im;
	g1_quad_object_class * sky_model;
	i4_time_class sky_time, start_time;

	void init()
	{
		current_sky_name=0;
		li_add_function("def_skys", g1_def_skys);
	}

	void reset()
	{
		for (int i=0; i<g1_sky_list.size(); i++)
		{
			delete g1_sky_list[i];
		}

		g1_sky_list.clear();

		if (current_sky_name)
		{
			delete current_sky_name;
		}

		current_sky_name=0;
		sky_model=0;
		if (sky_im)
		{
			delete sky_im;
		}
		sky_im=0;
	}

	void uninit()
	{
		//important: This uninit() must be called before the directx
		//interface is released (if present) because otherwise
		//the sky_im pointer would already be dangling...
		if (current_sky_name)
		{
			delete current_sky_name;
		}
		current_sky_name=0;
		for (int i=0; i<g1_sky_list.size(); i++)
		{
			delete g1_sky_list[i];
		}
		g1_sky_list.uninit();
		if (sky_im)
		{
			delete sky_im;
		}
		sky_model=0;
	}

	i4_file_class *find_sky_file(i4_const_str fname)
	{
		if (fname.null())
		{
			return 0;
		}

		i4_filename_struct fn;
		i4_split_path(fname, fn);

		// try the skys directory first
		char aname[256];
		sprintf(aname, "textures/%s.jpg", fn.filename);
		i4_file_class * fp=i4_open(aname);

		if (fp)
		{
			return fp;
		}


		// try the directory the level is in
		i4_filename_struct lev_split;
		i4_split_path(g1_get_map()->get_filename(), lev_split);

		sprintf(aname, "%s/%s.jpg", lev_split.path, fn.filename);
		fp=i4_open(aname);


		if (fp)
		{
			return fp;
		}


		return fp;
	}


	i4_bool update(i4_const_str &sky_name, i4_window_class * w, int use_blits)
	{
		r1_render_api_class * api=g1_render.r_api;

		if (use_blits && sky_im && (sky_im->width()!=w->width() || sky_im->height()!=w->height()*2))
		{
			delete current_sky_name;
			current_sky_name=0;
		}

		// see if the sky name has changed, if not then no need to try to load stuff
		if (current_sky_name && *current_sky_name==sky_name)
		{
			if (use_blits)
			{
				return sky_im ? i4_T : i4_F;
			}
			else
			{
				return sky_model ? i4_T : i4_F;
			}
		}

		// assign the new name and clean up old stuff
		if (current_sky_name)
		{
			delete current_sky_name;
		}
		current_sky_name=new i4_str(sky_name);


		if (sky_im)
		{
			delete sky_im;
			sky_im=0;
		}
		sky_model=0;



		if (use_blits)
		{
			i4_file_class * fp=find_sky_file(*current_sky_name);
			i4_image_class * im=0;

			if (!fp)
			{
				i4_image_class * images[10];
				w32 id=r1_get_texture_id(*current_sky_name);

				int t=r1_load_gtext(id, images);
				if (t)
				{
					im=images[0];
					for (int i=1; i<t; i++)
					{
						delete images[i];
					}
				}
				else
				{
					return i4_F;
				}

			}

			if (!im)
			{
				im=i4_load_image(fp);
			}

			delete fp;

			if (!im)
			{
				return i4_F;
			}

			sky_im = api->create_compatible_image(w->width(), w->height()*2);
			if (!sky_im)
			{
				delete im;
				im=0;
				return i4_F;
			}

			api->lock_image(sky_im);


			scale_copy(im, sky_im, 0,0, im->width()-1, im->height()-1);

			// add black lines for interlace mode
			if (g1_resources.render_window_expand_mode==R1_COPY_1x1_SCANLINE_SKIP)
			{
				i4_draw_context_class context(0,0, sky_im->width()-1, sky_im->height()-1);

				for (int i=0; i<=im->height()-1; i+=2)
				{
					sky_im->bar(0, i, sky_im->width()-1, i, 0, context);
				}
			}

			api->unlock_image(sky_im);
			delete im;
		}
		else
		{
			char sky_name[200];
			i4_os_string(*current_sky_name, sky_name,200);

			g1_model_id_type model_id=g1_model_list_man.find_handle("sky");
			if (!model_id)
			{
				return i4_F;
			}

			sky_model=g1_model_list_man.get_model(model_id);

			i4_bool loaded;
			char sname[256], sname1[256], sname2[256];
			i4_os_string(*current_sky_name, sname, 256);

			sprintf(sname1, "%s1", sname);
			sprintf(sname2, "%s2", sname);


			r1_texture_handle sky1, sky2;


			sky1=api->get_tmanager()->query_texture(sname1, &loaded);
			if (!loaded)
			{
				sky1=sky2=api->get_tmanager()->query_texture(sname, &loaded);
			}
			else
			{
				sky2=api->get_tmanager()->query_texture(sname2, &loaded);
				if (!loaded)
				{
					sky2=sky1;
				}
			}
			if (sky1==-1 || sky2==-1)
			{
				sky1=sky2=0;
			}
			//w32 idname1=g1_tile_man.get_texture(r1_get_texture_id(sname1));
			//w32 idname2=g1_tile_man.get_remap(r1_get_texture_id(sname2));
			//w32 idname=r1_get_texture_id(sname);
			//sky1=api->get_tmanager()->find_texture(idname1);
			//if (!sky1)
			//	  sky1=api->get_tmanager()->find_texture(idname);
			//sky2=api->get_tmanager()->find_texture(idname2);
			//if (!sky2)
			//  sky2=sky1;


			if (!sky1)
			{
				return i4_F;
			}

			for (int i=0; i<sky_model->num_quad; i++)
			{
				int tinted=sky_model->quad[i].get_flags(g1_quad_class::TINT);

				if (!tinted)
				{
					sky_model->quad[i].material_ref=sky1;
				}
				else
				{
					sky_model->quad[i].material_ref=sky2;
				}
			}
		}

		return i4_T;
	}

} g1_sky;


li_object *g1_def_skys(li_object * o, li_environment * env)
{
	g1_sky.reset();

	for (o=li_cdr(o,env); o; o=li_cdr(o,env))
	{
		char * name=li_string::get(li_eval(li_car(o,env), env),env)->value();
		g1_render.r_api->get_tmanager()->register_texture(name, "sky name");
		g1_sky_list.add(new i4_str(name));
	}

	return 0;
}


int force_blits=0;


void generate_poly(i4_3d_vector * points, w16 * indexes,
				   i4_transform_class &transform,
				   float s, float t,
				   r1_vert * v,
				   float r, float g, float b, float a)
{
	for (int i=0; i<4; i++)
	{
		int x=indexes[i];

		i4_3d_vector p=points[x];
		transform.transform(p, v[i].v);
		v[i].v.x*=g1_render.scale_x;
		v[i].v.y*=g1_render.scale_y;
		v[i].r=r;
		v[i].g=g;
		v[i].b=b;
		v[i].a=a;
	}

	v[0].s=s;
	v[0].t=t;
	v[1].s=s+0.5f;
	v[1].t=t;
	v[2].s=s+0.5f;
	v[2].t=t+0.5f;
	v[3].s=s;
	v[3].t=t+0.5f;
}



static float cam_z=8;
void draw_clouds(g1_camera_info_struct &current_camera,
				 i4_transform_class &transform,
				 g1_draw_context_class * context)

{
	int i,j;
	r1_vert v[2*2+20]; //, *p;


	i4_3d_vector pts[4*4], * pt;

	float cloud_scale=15;

	int repeat_length = (int)(cloud_scale*2*16);
	float start_t=(float)fmod((g1_tick_counter+g1_render.frame_ratio)/2000.0f+current_camera.gy/200.0f,0.5f);
	float start_s=(float)fmod(current_camera.gx*(1/200.0f), 0.5f);

	float r[2],g[2],b[2],a[2];

	li_class * bottom_layer=(li_class *)g1_map_vars.vars()->get(bottom_cloud_layer);

	r[0]=bottom_layer->get(li_red);
	g[0]=bottom_layer->get(li_green);
	b[0]=bottom_layer->get(li_blue);
	a[0]=bottom_layer->get(li_alpha);
	char * bottom_layer_name=bottom_layer->get(li_cloudname);

	li_class * top_layer=(li_class *)g1_map_vars.vars()->get(top_cloud_layer);
	r[1]=top_layer->get(li_red);
	g[1]=top_layer->get(li_green);
	b[1]=top_layer->get(li_blue);
	a[1]=top_layer->get(li_alpha);
	char * top_layer_name=top_layer->get(li_cloudname);

	for (int k=1; k>=0; k--)
	{

		pt=pts;

		for (float y=-1.5; y<=1.5; y+=1)
		{
			for (float x=-1.5; x<=1.5; x+=1, pt++)
			{
				*pt=i4_3d_vector(x * cloud_scale + current_camera.gx,
								 y * cloud_scale + current_camera.gy,
								 (float)(cam_z+current_camera.gz*0.9f-(fabs(x)+fabs(y))*cloud_scale/8.0f+k) );
			}
		}


		r1_texture_handle han=0;
		i4_bool loaded=i4_F;
		han=g1_render.r_api->get_tmanager()->query_texture(
			k==1 ? top_layer_name : bottom_layer_name,&loaded);
		if (loaded==i4_F)
		{
			han=sky_texture.get();
		}
		g1_render.r_api->use_texture(han, 256,0);

		for (j=0; j<3; j++)
		{
			for (i=0; i<3; i++)
			{
				w16 indexes[4];
				indexes[0]=i+j*4;
				indexes[1]=indexes[0]+1;
				indexes[2]=indexes[0]+5;
				indexes[3]=indexes[0]+4;

				generate_poly(pts, indexes, transform, start_s, start_t, v, r[k], g[k], b[k], a[k]);


				if (i==0)
				{
					v[0].a=0;
					v[3].a=0;
				}
				else if (i==2)
				{
					v[1].a=0;
					v[2].a=0;
				}

				if (j==0)
				{
					v[0].a=0;
					v[1].a=0;
				}
				else if (j==2)
				{
					v[2].a=0;
					v[3].a=0;
				}





				r1_vert buf1[20], buf2[20], * pv;
				sw32 t_verts=4;
				w16 v_index[4]={
					0,1,2,3
				};
				pv=g1_render.r_api->clip_poly(&t_verts, v, v_index, buf1, buf2, 0);

				if (t_verts)
				{

					for (int j=0; j<t_verts; j++)
					{
						r1_vert * v = &pv[j];

						float ooz = r1_ooz(v->v.z);

						v->px = v->v.x * ooz * g1_render.center_x + g1_render.center_x;
						v->py = g1_render.center_y + v->v.y * ooz * g1_render.center_y;
						v->w  = ooz;
					}

					g1_render.r_api->render_poly(t_verts, pv);
				}
			}
		}


	}
}



void g1_draw_sky(i4_window_class * window,
				 g1_camera_info_struct &current_camera,
				 i4_transform_class &transform,
				 g1_draw_context_class * context)

{
	pf_draw_sky.start();

	r1_render_api_class * api=g1_render.r_api;

	int use_blits=(api->get_render_device_flags() & R1_SOFTWARE) ? 1 : 0;
	if (force_blits)
	{
		use_blits=1;
	}
	if (g1_get_map()->sky_name &&
		g1_get_map()->sky_name->null())
	{
		delete g1_get_map()->sky_name;
		g1_get_map()->sky_name=0;
	}

	if (g1_get_map()->sky_name &&
		g1_sky.update(*g1_get_map()->sky_name, window, use_blits))
	{

		if (use_blits)
		{
			int window_xoff=context->context->xoff, window_yoff=context->context->yoff;

			i4_image_class * sky_im = g1_sky.sky_im;

			i4_normalize_angle(current_camera.ground_rotate);
			i4_float horz_cap = -current_camera.horizon_rotate;

			i4_normalize_angle(horz_cap);

			if (horz_cap > i4_pi_3_2())
			{
				horz_cap = 0;
			}
			if (horz_cap > i4_pi_2())
			{
				horz_cap = i4_pi_2();
			}

			sw32 h = sky_im->height() / 2;

			sw32 x_offs = i4_f_to_i(current_camera.ground_rotate * 4 *
									sky_im->width() / i4_2pi()) % sky_im->width();

			sw32 y_offs = i4_f_to_i(horz_cap * h/(i4_pi_2()));

			if (y_offs > h)
			{
				y_offs = h;
			}
			if (y_offs < 0)
			{
				y_offs = 0;
			}

			//y_offs -= (h/4);

			y_offs = h - y_offs;

			sw32 y2 = y_offs+h-1;
			if (y2>sky_im->height()-1)
			{
				y2 = sky_im->height()-1;
			}

			//y cffset must be even for interlaced
			if (g1_resources.render_window_expand_mode==R1_COPY_1x1_SCANLINE_SKIP)
			{
				y_offs = y_offs & (~1);
			}

			if (g1_hurt_tint>0)
			{
				// solid red fill
				i4_color col =
					(i4_f_to_i(g1_hurt_tint_data[g1_hurt_tint].r*255)<<16) |
					(i4_f_to_i(g1_hurt_tint_data[g1_hurt_tint].g*255)<<8) |
					(i4_f_to_i(g1_hurt_tint_data[g1_hurt_tint].b*255));

				api->clear_area(0,0, window->width()-1, window->height()-1, col, r1_far_clip_z);
			}
			else
			{
				// blit sky image
				//api->context=context;//different types, cannot convert.
				api->put_image(sky_im,
							   x_offs,
							   0,
							   0, y_offs,
							   sky_im->width()-x_offs-1, y2);

				if (x_offs>0)
				{
					api->put_image(sky_im,
								   0,0,
								   sky_im->width()-x_offs, y_offs,
								   sky_im->width()-1, y2);
				}
			}
		}
		else
		{
			i4_transform_class out, scale, trans;
			out.identity();

			trans.translate(current_camera.gx, current_camera.gy, -4);
			out=transform;
			out.multiply(trans);

			api->set_filter_mode(R1_BILINEAR_FILTERING);
			api->set_write_mode(  R1_WRITE_COLOR | R1_COMPARE_W );
			g1_render.render_object(g1_sky.sky_model,
									&out,
									0,
									100,
									g1_default_player,
									0,
									0,
									0);

			api->set_write_mode(  R1_WRITE_COLOR | R1_WRITE_W | R1_COMPARE_W );


			draw_clouds(current_camera, transform, context);

			//default state is filter enabled
//      api->set_filter_mode(R1_NO_FILTERING);

		}
	}
	//else  //that only gives trouble since the buffer is inited already
	//and there might already be something in it.
	//perhaps we should adjust the Blt() to do a z-buffer blit?
	//  api->clear_area(0,0, window->width()-1, window->height()-1, 0, r1_far_clip_z);

	pf_draw_sky.stop();
}
