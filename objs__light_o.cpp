#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/light_o.h"
#include "saver.h"
#include "objs/model_draw.h"
#include "map.h"
#include "math/num_type.h"
#include "object_definer.h"
#include "map_man.h"
#include "flare.h"
#include "time/profile.h"
#include "map_vert.h"
#include "gui/create_dialog.h"
#include "gui/text_input.h"
#include "gui/smp_dial.h"
#include "window/colorwin.h"
#include "editor/editor.h"
#include "editor/e_res.h"

static i4_profile_class pf_light_occupy("light::occupy"), pf_light_unoccupy("light::unoccupy");

g1_object_type g1_lightbulb_type;

void g1_light_object_init();

class g1_light_obj_edit_class : public i4_color_window_class
{ 
	w32 selected_objects[G1_MAX_OBJECTS];
	int t_sel;
	i4_text_input_class 
		*ti_r,
		*ti_g,
		*ti_b,
		*ti_c1,
		*ti_c2,
		*ti_c3;
public:

	

	enum { 
		OK=1, CANCEL=2};

		char *name() { return "light_obj editor"; }
		g1_light_obj_edit_class(i4_graphical_style_class *style)

			: i4_color_window_class(400, 200, style->color_hint->neutral(), style)
		{
			t_sel=g1_get_map()->make_selected_objects_list(selected_objects, G1_MAX_OBJECTS);

			g1_light_object_class *light;
			if (!t_sel)
				return;
			light=g1_light_object_class::cast(g1_global_id.get(selected_objects[0]));
			if (light)
			{
				

				i4_create_dialog(g1_ges("light_object_dialog"), this, style,
					&ti_r, int(light->r*255),  
					&ti_g, int(light->g*255),
					&ti_b, int(light->b*255),
					&ti_c1, int(light->c1*255),//Standard values for these are (0.05, 0.25, 0.5)
					&ti_c2, int(light->c2*255), 
					&ti_c3, int(light->c3*255),
					this, OK,
					this, CANCEL);
			}
		}

		void receive_event(i4_event *ev)
		{
			i4_color_window_class::receive_event(ev);

			if (ev->type()==i4_event::USER_MESSAGE)
			{
				CAST_PTR(uev, i4_user_message_event_class, ev);
				switch (uev->sub_type)
				{
				

				case OK :
					{
						i4_float f_r=((float)ti_r->get_number())/255;
						i4_float f_g=((float)ti_g->get_number())/255;
						i4_float f_b=((float)ti_b->get_number())/255;
						i4_float f_c1=((float)ti_c1->get_number())/255;
						i4_float f_c2=((float)ti_c2->get_number())/255;
						i4_float f_c3=((float)ti_c3->get_number())/255;
						if (f_r<0 || f_r>1 || 
							f_g<0 || f_g>1 ||
							f_b<0 || f_b>1 ||
							f_c1<0 || f_c1>1||
							f_c2<0 || f_c2>1||
							f_c3<0 || f_c3>1
							)
						{
							i4_message_box("Invalid input", "All intensity values must be between 0 and 255");
						}
						else
						{
							g1_light_object_class *light;

							for (int i=0; i<t_sel; i++)
								if (g1_global_id.check_id(selected_objects[i]))
								{
									light=g1_light_object_class::cast(g1_global_id.get(selected_objects[i]));
									light->setup(light->x,light->y,light->h,f_r,
										f_g,f_b,0,f_c1,f_c2,f_c3);
								}
							li_call("object_changed");
						}

						

					} break;

				case CANCEL :
					li_call("object_changed");
					break;

				}
			}

		}

};

class g1_light_object_def_class:
	public g1_object_definer<g1_light_object_class>
{
public:
	g1_light_object_def_class(char *name,
		w32 type_flags=0,
		function_type _init = 0,
		function_type _uninit = 0)
		: g1_object_definer<g1_light_object_class>(name, type_flags ,_init, _uninit) {}
	
		i4_window_class *create_edit_dialog()

		{    
			return new g1_light_obj_edit_class(i4_current_app->get_style());
		}
}
g1_light_object_def("lightbulb", 
                    g1_object_definition_class::EDITOR_SELECTABLE,
                    g1_light_object_init);

void g1_light_object_init()
{
  g1_lightbulb_type = g1_light_object_def.type;
}


void g1_light_object_class::setup(float _x, float _y, float _h, 
                                  float _r, float _g, float _b, float _white,
                                  float min_light_contribute, 
                                  float linear_contribute, 
                                  float geometric_contribute)
{
  r=_r;
  g=_g;
  b=_b;
  x=lx=_x;
  y=ly=_y;
  h=lh=_h;

  int occ=get_flag(MAP_OCCUPIED);

  if (occ)
    unoccupy_location();

  if (add_intensities)
    i4_free(add_intensities);


  c1=min_light_contribute;
  c2=linear_contribute;
  c3=geometric_contribute;

  if (c3<0.0016)  // maximum radius is 25 squares for now
    c3=0.0016f;

  change_radius=i4_f_to_i((float)sqrt((32-c1)/c3));
  
  sw32 w=change_radius*2+1,h=change_radius*2+1;

  add_intensities=(w32 *)I4_MALLOC(w*h*sizeof(w32),
                                   "light map restore");  
  
  if (occ)
    occupy_location();
  
}

g1_light_object_class::~g1_light_object_class()
{
  if (add_intensities)
    i4_free(add_intensities);
}

g1_light_object_class::g1_light_object_class(g1_object_type id,
                                             g1_loader_class *fp)
  : g1_object_class(id,fp)
{
  add_intensities=0;

  if (fp && fp->check_version(DATA_VERSION))
  {
    float _r, _g, _b, _white, _c1, _c2, _c3;
    fp->read_format("fffffff", &_r, &_g, &_b, &_white, &_c1, &_c2, &_c3);
    setup(x,y,h, _r, _g, _b, _white, _c1, _c2, _c3);

    fp->end_version(I4_LF);
  }
  else
  {
    r=g=b=1; 
    white=1;    
    c1=0.1f;    c2=0.25f;    c3=0.5f;    
    h=2;
  }

  draw_params.setup("lightbulb");
}


void g1_light_object_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);
  fp->start_version(DATA_VERSION);
  fp->write_format("fffffff", &r,&g,&b, &white, &c1,&c2,&c3);
  fp->end_version();
}

void g1_light_object_class::draw(g1_draw_context_class *context)
{  
  g1_editor_model_draw(this, draw_params, context);
}


void g1_light_object_class::move(float nx, float ny, float nh)
{
  unoccupy_location();
  lx=x; ly=y; lh=h;
  x=nx; y=ny; h=nh;

  occupy_location();
}


void g1_light_object_class::think()
{
}

i4_bool g1_light_object_class::occupy_location()
{
  if (!add_intensities)
  {
    i4_warning("call light::setup before occupy_location");
    
    return i4_F;
  }
  
  pf_light_occupy.start();
  if (g1_object_class::occupy_location())
  {

    
    sw32 ix=i4_f_to_i(x), iy=i4_f_to_i(y);
    w32 *a=add_intensities;

    for (sw32 ty=-change_radius+iy; ty<=change_radius+iy; ty++)
    {
      sw32 start_x=ix-change_radius;
      if (start_x<0) start_x=0;
      g1_map_vertex_class *v=g1_get_map()->vertex(start_x,ty);

      for (int tx=-change_radius+ix; tx<=change_radius+ix; tx++, a++)
      {
        if (tx>=0 && ty>=0 && tx<=g1_get_map()->width() && ty<=g1_get_map()->height())
        {
          i4_3d_vector normal;
          i4_float tz;
          w32 old_rgb=v->dynamic_light;

          v->get_normal(normal, tx, ty);
          tz=v->get_height();

          i4_3d_vector dir=i4_3d_vector(x-tx,
                                        y-ty, 
                                        h-tz);

          i4_float dist=(float)sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
          i4_float odist=1.0f/dist;

          dir.x*=odist;  // normalize the light direction vector
          dir.y*=odist;
          dir.z*=odist;

          i4_float ndl = normal.dot(dir);
          i4_float atten =  1.0f/(c1 + c3*dist*dist);

          if (atten>1) atten=1;

          i4_float intensity=ndl*atten;
          if (intensity<0) intensity=0;

          sw32 ra,ga,ba, o_r,og,ob;

          ra=i4_f_to_i(intensity * r * 255);    // calculate how much to add to the current light
          ga=i4_f_to_i(intensity * g * 255);
          ba=i4_f_to_i(intensity * b * 255);

          o_r=(old_rgb>>16)&255;              // grab the old light values
          og=(old_rgb>>8)&255;
          ob=(old_rgb)&255;
      
          if (ra+o_r>255) ra=255-o_r;           // adjust for overflow
          if (ga+og>255) ga=255-og;
          if (ba+ob>255) ba=255-ob;

          *a=(ra<<16)|(ga<<8)|ba;           // store the added amount so we can subtract out later
        
          v->dynamic_light=((ra+o_r)<<16) | ((ga+og)<<8) | (ba+ob);
          v->light_sum|=0x80000000;
          v++;
        }
      }
    }
    pf_light_occupy.stop();
    return i4_T;
  }
  else
  {
    pf_light_occupy.stop();
    return i4_F;
  }
}

void g1_light_object_class::unoccupy_location()
{ 
  if (!add_intensities)
    return ;

  pf_light_unoccupy.start();
  g1_object_class::unoccupy_location();

  sw32 w=change_radius*2+1,h=change_radius*2+1;
  sw32 ix=i4_f_to_i(x), iy=i4_f_to_i(y);
  w32 *a=add_intensities;

  for (sw32 ty=-change_radius+iy; ty<=change_radius+iy; ty++)
  {
    sw32 start_x=ix-change_radius;
    if (start_x<0) start_x=0;
    g1_map_vertex_class *v=g1_get_map()->vertex(start_x,ty);
    for (int tx=-change_radius+ix; tx<=change_radius+ix; tx++, a++)
    {
      if (tx>=0 && ty>=0 && tx<=g1_get_map()->width() && ty<=g1_get_map()->height())      
      {
        v->dynamic_light-=*a;
        v++;
        v->light_sum=0x80000000;
      }
    }
  }
  
  pf_light_unoccupy.stop();
}

