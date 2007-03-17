#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "objs/model_draw.h"
#include "math/transform.h"
#include "g1_object.h"
#include "draw_context.h"
#include "objs/model_id.h"
#include "math/pi.h"
#include "math/angle.h"
#include "g1_render.h"
#include "g1_texture_id.h"
#include "render/r1_api.h"
#include "render/r1_clip.h"
#include "objs/map_piece.h"
#include "resources.h"
#include "lisp/lisp.h"
#include "map_man.h"
#include "map.h"
#include "tick_count.h"
#include "controller.h"

void g1_model_draw_parameters::setup(g1_quad_object_class *_model,
									 g1_quad_object_class *_shadow_model,
									 g1_quad_object_class *_lod_model)
{
	model=_model;
	shadow_model=_shadow_model;
	lod_model=_lod_model;
}


float g1_model_draw_parameters::extent() const    // gets extents of model_id from model_draw
{
	if (model)
	{
		return model->extent;
	}
	else
	{
		return 0;
	}
}

void g1_model_draw_parameters::setup(w16 _model_id, w16 _shadow_model, w16 _lod_model)
{
	model=g1_model_list_man.get_model(_model_id);

	if (shadow_model)
	{
		shadow_model=g1_model_list_man.get_model(_model_id);
	}
	else
	{
		shadow_model=0;
	}

	if (_lod_model)
	{
		lod_model=g1_model_list_man.get_model(_lod_model);
	}
	else
	{
		lod_model=0;
	}
}

void g1_model_draw_parameters::setup(char *model_name, char *shadow_model_name,
									 char *lod_model_name)
{
	//char *m=model_name;
	int id=g1_model_list_man.find_handle(model_name);
	model = g1_model_list_man.get_model(id);

	if (shadow_model_name)
	{
		id=g1_model_list_man.find_handle(shadow_model_name);
		if (id)
		{
			shadow_model = g1_model_list_man.get_model(id);
		}
		else
		{
			shadow_model = 0;
		}
	}
	else
	{
		shadow_model = 0;
	}

	if (lod_model_name)
	{
		id=g1_model_list_man.find_handle(lod_model_name);
		if (id)
		{
			lod_model = g1_model_list_man.get_model(id);
		}
		else
		{
			lod_model = 0;
		}
	}
	else
	{
		lod_model=0;
	}

}

static li_symbol_ref team_icons("team_icons");

//BSPCODE
//Here goes it:
//This function is actually responsible for drawing the object.
//It calls the functions of the g1_render global instance to render
//polygonal objects depending on the distance from the viewer.
//the g1_model_draw_parameters structure contains the pointer(s)
//to the previously loaded objects, so if you add your code to
//the g1_render.render_object() and g1_render.render_object_polys() methods
//if model.bsp is !=0. What's the difference between these two functions?
//I don't know, check yourself...

void g1_model_draw(g1_object_class *_this,
				   g1_model_draw_parameters &params,
				   g1_draw_context_class *context,
				   i4_3d_vector& viewer_position)
{


	//this needs fixing, as it is not what we want if multiple viewports are available.
	//set this to a very small value if forcedraw, so that we always use the default model
	float dist_sqrd=1.0f;
	if (!(params.flags&g1_model_draw_parameters::FORCEDRAW))
	{
		dist_sqrd=(viewer_position.x-_this->x)*(viewer_position.x-_this->x)+
				   (viewer_position.y-_this->y)*(viewer_position.y-_this->y)+
				   (viewer_position.z-_this->h)*(viewer_position.z-_this->h);


		if (dist_sqrd>g1_resources.lod_disappear_dist)
		{
			return ;
		}

		if (dist_sqrd>g1_resources.lod_nolodmodel_disappear_dist && !params.lod_model)
		{
			return ;
		}
	}

	g1_screen_box *bbox=0;

	i4_transform_class view_transform;

	i4_transform_class *old = context->transform;
	context->transform = &view_transform;

	view_transform.multiply(*old,*(_this->world_transform));

	if (_this->get_flag(g1_object_class::SELECTABLE | g1_object_class::TARGETABLE))
	{
		if (g1_render.current_selectable_list)
		{
			bbox=g1_render.current_selectable_list->add();
		}
		if (bbox)
		{
			bbox->x1 = 2048.0f;
			bbox->y1 = 2048.0f;
			bbox->x2 = -1.0f;
			bbox->y2 = -1.0f;
			bbox->z1 = 999999.0f;
			bbox->z2 = -999999.0f;
			bbox->w  = 1.0f/999999.0f;
			bbox->object_id = _this->global_id;
			bbox->flags=0;
		}
	}

	int max_health=_this->get_type()->defaults->health;
	int damage_level;

	if (_this->health >= max_health)
	{
		damage_level = 7;
	}
	else
	if (_this->health < 0)
	{
		damage_level = 0;
	}
	else
	{
		damage_level = i4_f_to_i(7.f * _this->health / (float)max_health);
	}

	g1_render.set_render_damage_level(damage_level);


	if (dist_sqrd<g1_resources.lod_switch_dist)
	{
		if (params.shadow_model)
		{
			g1_quad_object_class *model = params.shadow_model;

			g1_render.r_api->disable_texture();
			g1_render.r_api->set_shading_mode(R1_CONSTANT_SHADING);
			g1_render.r_api->set_alpha_mode(R1_ALPHA_CONSTANT);
			g1_render.r_api->set_constant_color(0x7F000000);
			g1_render.r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR);

			i4_transform_class t=*(_this->world_transform), shadow_view_transform;
			// drop shadow to ground
			// draw it a tiny bit above the ground, or we'll get
			// z-buffer artefacts.
			t.t.z=g1_get_map()->map_height(_this->x, _this->y, _this->h)+0.001;

			shadow_view_transform.multiply(*old,t);


			g1_render.render_object_polys(model,
										  &shadow_view_transform,
										  params.frame);

			g1_render.r_api->set_shading_mode(R1_COLORED_SHADING);
			g1_render.r_api->set_alpha_mode(R1_ALPHA_DISABLED);
			g1_render.r_api->set_write_mode(R1_WRITE_W | R1_COMPARE_W | R1_WRITE_COLOR);
		}

		g1_quad_object_class *model = params.model;
		if (model)
		{
			if (!(params.flags & g1_model_draw_parameters::SUPPRESS_SPECIALS) && model->num_special>0)
			{
				model->update(i4_float(g1_tick_counter + _this->global_id) + g1_render.frame_ratio);
			}

			g1_render.render_object(model,
									&view_transform,
									params.flags & g1_model_draw_parameters::NO_LIGHTING ?
									0 : _this->world_transform,
									1,
									_this->player_num,
									params.frame,
									bbox,
									0);

			for (int i=0; i<_this->num_mini_objects; i++)
			{
				_this->mini_objects[i].draw(context,
											_this->world_transform,
											bbox,
											_this->player_num);
			}
		}
	}
	else
	{
		g1_render.r_api->set_filter_mode(R1_NO_FILTERING);

		g1_quad_object_class *mod=params.lod_model;
		if (mod==NULL)
		{
			mod=params.model;
		}
		g1_render.render_object(mod,
								&view_transform,
								params.flags & g1_model_draw_parameters::NO_LIGHTING ?
								0 : _this->world_transform,
								1,
								_this->player_num,
								params.frame,
								bbox,
								0);

		for (int i=0; i<_this->num_mini_objects; i++)
		{
			_this->mini_objects[i].draw(context,
										_this->world_transform,
										bbox,
										_this->player_num,
										0, i4_T, i4_T);
		}

		g1_render.r_api->set_filter_mode(R1_BILINEAR_FILTERING);

	}

	g1_render.set_render_damage_level(-1);

	context->transform = old;
	if (bbox&& (bbox->x1>bbox->x2)) //we are off screen, remove the bounding box again
	{
		g1_render.current_selectable_list->drop_last();
	}
}

void g1_editor_model_draw(g1_object_class *_this,
						  g1_model_draw_parameters &params,
						  g1_draw_context_class *context,
						  i4_3d_vector& viewer_position)
{
	if (context->draw_editor_stuff)
	{
		g1_model_draw(_this, params, context, viewer_position);
	}
}
