#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.h"
#include "math/num_type.h"
#include "math/pi.h"
#include "saver.h"
#include "map.h"
#include "g1_render.h"
#include "object_definer.h"
#include "dll/dll.h"
#include "map_man.h"
#include "math/pi.h"

#include "objs/shockwave.h"
#include "objs/explosion1.h"
#include "objs/debris.h"
#include "map_vert.h"
#include "sound/sfx_id.h"

g1_object_definer<g1_shockwave_class> 
g1_shockwave_def("shockwave", g1_object_definition_class::EDITOR_SELECTABLE);

g1_object_definer<g2_water_wave_class>
g2_water_wave_def("water_wave", g1_object_definition_class::EDITOR_SELECTABLE);
                                                      

S1_SFX_DISTANCE(explode_sfx, "explosion/super_mortar.wav", S1_3D | S1_STREAMED, 100, 50);

//the following sound must be recorded first.
//S1_SFX(wave_sfx,"ambient/water_wave.wav",S1_3D,80,60); 

g1_shockwave_class::g1_shockwave_class(g1_object_type id,
                                       g1_loader_class *fp)
  : g1_object_class(id, fp), vert_map(0)
{  
  if (fp && fp->check_version(DATA_VERSION))
  {
    fp->read_format("ff2", &strength, &focal_radius, &radius);  
    alloc_vert_map();
    fp->end_version(I4_LF);
  }
  else
  {
    radius=5;
    strength=1;
    focal_radius=0;
    alloc_vert_map();
  }

  draw_params.setup("shockwave");
}

void g1_shockwave_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);  
  fp->start_version(DATA_VERSION);  
  fp->write_format("ff2", &strength, &focal_radius, &radius);  
  fp->end_version();
}

g1_shockwave_class::~g1_shockwave_class()
{
  if (vert_map)
    i4_free(vert_map);
}

void g1_shockwave_class::alloc_vert_map()
{
  if (vert_map)
    i4_free(vert_map);
    
  int d=2*radius+1;
  vert_map=(vert_offset *)I4_MALLOC(sizeof(vert_offset) * d*d, "");
}

void g1_shockwave_class::setup(const i4_3d_vector &pos,  // starting position (center)
                               float _strength,           // maximum ground distortion
                               w16 _radius)               // maximum radius in game units

{
  g1_map_class *map=g1_get_map();
 
  x=lx=pos.x;
  y=ly=pos.y;
  h=lh=pos.z;

  strength=_strength;
  radius=_radius;
  focal_radius=0;
  alloc_vert_map();

  red_intensity=1.0;
  green_intensity=1.0;
  blue_intensity=0;

  if (occupy_location())
  {
    request_think();
    g1_explosion1_class *explosion = NULL;
    
    explosion = (g1_explosion1_class *)g1_create_object(g1_get_object_type("explosion1"));
    if (explosion)
      explosion->setup(x,y,h, g1_explosion1_class::MAKE_SPHERE);

    g1_debris_class *debris = NULL;
    debris = (g1_debris_class *)g1_create_object(g1_get_object_type("debris"));
    if (debris) debris->setup(x,y,h, g1_debris_class::RING_CHUNKS);
    debris = (g1_debris_class *)g1_create_object(g1_get_object_type("debris"));
    if (debris) debris->setup(x,y,h, g1_debris_class::RING_CHUNKS);
    debris = (g1_debris_class *)g1_create_object(g1_get_object_type("debris"));
    if (debris) debris->setup(x,y,h, g1_debris_class::RING_CHUNKS);

    explode_sfx.play(x,y,h);
  }
}



i4_bool g1_shockwave_class::occupy_location()
{
  if (get_flag(MAP_OCCUPIED) || !vert_map)
    return i4_T;

  
  if (g1_object_class::occupy_location())
  {
 
    sw32 _ix = i4_f_to_i(x),
      _iy = i4_f_to_i(y);  

    float oor = 1.f/(float)radius;

    vert_offset *vo = vert_map;

    sw32 x_left,x_right,y_top,y_bottom;
  
    sw32 wx = g1_get_map()->width();
    sw32 wy = g1_get_map()->height();
  
    x_left   = _ix - radius; if (x_left<0)     x_left=0;
    x_right  = _ix + radius; if (x_right>wx-1)  x_right=wx-1;
    y_top    = _iy - radius; if (y_top<0)      y_top=0;
    y_bottom = _iy + radius; if (y_bottom>wy-1) y_bottom=wy-1;
  
    sw32 ix,iy;

    for (iy=y_top; iy<=y_bottom; iy++)
    {
      g1_map_vertex_class *v = g1_get_map()->vertex(x_left, iy);

      for (ix=x_left; ix<=x_right; ix++, vo++, v++)
      {
        i4_3d_vector dir = i4_3d_vector(x-ix, y-iy, h - v->get_height());

        i4_float dist  = dir.length();
        i4_float odist = 1.f/dist;

//         float t=dist * oor;
        float alpha=(float)fabs((dist - focal_radius)*2.0f);
        float multiplier=2 / (alpha*alpha+1);
        if (multiplier>1.0) multiplier=1.0;
        float new_height=multiplier*strength;

        sw32 new_height_value = i4_f_to_i(new_height/0.05f);

        if (new_height_value>255)
          new_height_value=255;
        else if (new_height_value<0)
          new_height_value=0;
      
        vo->displacement=new_height_value;      

        v->height+=(w8)vo->displacement;
        v->light_sum=0x80000000;
        v->normal=0x8000;

        w16 old_rgb=(w16)v->dynamic_light;
        sw32 r,g,b, o_r,og,ob;
        r=i4_f_to_i(red_intensity*multiplier*255);
        g=i4_f_to_i(green_intensity*multiplier*255);
        b=i4_f_to_i(blue_intensity*multiplier*255);
        
        o_r=(old_rgb>>16)&255;              // grab the old light values
        og=(old_rgb>>8)&255;
        ob=(old_rgb)&255;
        
        if (r+o_r>255) r=255-o_r;           // adjust for overflow
        if (g+og>255) g=255-og;
        if (b+ob>255) b=255-ob;

        vo->color = (r<<16) | (g<<8) | b;
        v->dynamic_light += vo->color;
      }
    }

    request_think();

    return i4_T;
  }
  else return i4_F;
}


void g1_shockwave_class::unoccupy_location()
{
  if (get_flag(MAP_OCCUPIED) && vert_map)
  {
    g1_object_class::unoccupy_location();
    
    sw32 _ix = i4_f_to_i(x),
         _iy = i4_f_to_i(y);

    vert_offset *vo=vert_map;

    sw32 x_left,x_right,y_top,y_bottom;
    sw32 w = g1_get_map()->width();
    sw32 h = g1_get_map()->height();

    x_left   = _ix - radius; if (x_left<0)     x_left=0;
    x_right  = _ix + radius; if (x_right>w-1)  x_right=w-1;
    y_top    = _iy - radius; if (y_top<0)      y_top=0;
    y_bottom = _iy + radius; if (y_bottom>h-1) y_bottom=h-1;  

    sw32 ix,iy;

    for (iy=y_top; iy<=y_bottom; iy++)
    {
      g1_map_vertex_class *v=g1_get_map()->vertex(x_left, iy);

      for (ix=x_left; ix<=x_right; ix++, vo++, v++)
      {
        v->height-=(w8)vo->displacement;
        v->dynamic_light-=vo->color;
        v->light_sum=0x80000000;
        v->normal=0x8000;
      }
    }
  }
}


void g1_shockwave_class::think()
{
  grab_old();
  unoccupy_location();
  strength *= 0.92f;
  focal_radius += 0.3f;
  red_intensity *= 0.95f;
  green_intensity *= 0.80f;

  if (strength<0.05f)
    request_remove();
  else
  {
    occupy_location();
    request_think();
  }
}

//void fast_transform(i4_transform_class *t,const i4_3d_vector &src, r1_3d_point_class &dst);

void g1_shockwave_class::draw(g1_draw_context_class *context)
{    
  //g1_model_draw(this, draw_params, context);
#if 0
  sw32     frequency = 4;
  
  i4_float ratio = (1.f / (float)frequency) * i4_interpolate((g1_tick_counter & (frequency-1)),
                                                              (g1_tick_counter & (frequency-1))+1,
                                                               g1_render.frame_ratio);
  if (g1_tick_counter & frequency)
    ratio = 1-ratio;

  i4_float start_a,start_r,start_g,start_b;
  i4_float end_a,  end_r,  end_g,  end_b;

  start_a = i4_interpolate(1,0.5,ratio);
  start_r = 1;
  start_g = i4_interpolate(0,1,ratio);
  start_b = 0;
      
  end_a   = i4_interpolate(0.5,1,ratio);
  end_r   = 1;
  end_g   = i4_interpolate(1,0,ratio);
  end_b   = 0;

  i4_3d_vector start_point = i4_3d_vector(x,y,h);
  i4_3d_vector end_point   = i4_3d_vector(x,y,h+20);

  r1_3d_point_class t_start_point, t_end_point;
      
  fast_transform(context->transform, start_point, t_start_point);
  fast_transform(context->transform, end_point,   t_end_point);
    
  t_start_point.x *= g1_render.scale_x;
  t_start_point.y *= g1_render.scale_y;

  t_end_point.x   *= g1_render.scale_x;
  t_end_point.y   *= g1_render.scale_y;

  i4_3d_vector v;

  v = i4_3d_vector(t_end_point.x - t_start_point.x, t_end_point.y - t_start_point.y, t_end_point.z - t_start_point.z);
    
  r1_vert      points[4];
  i4_3d_vector norm;

  norm.cross((i4_3d_vector &)t_start_point, v);
  norm.normalize();
  norm *= 2;
    
  points[0].v.x = t_start_point.x + norm.x;
  points[0].v.y = t_start_point.y + norm.y;
  points[0].v.z = t_start_point.z + norm.z;
  points[0].a   = start_a;
  points[0].r   = start_r;
  points[0].g   = start_g;
  points[0].b   = start_b;

  points[1].v.x = t_start_point.x - norm.x;
  points[1].v.y = t_start_point.y - norm.y;
  points[1].v.z = t_start_point.z - norm.z;
  points[1].a   = start_a;
  points[1].r   = start_r;
  points[1].g   = start_g;
  points[1].b   = start_b;

  norm.cross((i4_3d_vector &)t_end_point, v);
  norm.normalize();
  norm *= 2;
    
  points[2].v.x = t_end_point.x - norm.x;
  points[2].v.y = t_end_point.y - norm.y;
  points[2].v.z = t_end_point.z - norm.z;
  points[2].a   = end_a;
  points[2].r   = end_r;
  points[2].g   = end_g;
  points[2].b   = end_b;
            
  points[3].v.x = t_end_point.x + norm.x;
  points[3].v.y = t_end_point.y + norm.y;
  points[3].v.z = t_end_point.z + norm.z;
  points[3].a   = end_a;
  points[3].r   = end_r;
  points[3].g   = end_g;
  points[3].b   = end_b;

  g1_post_draw_quad_class p;

  p.vert_ref[0] = g1_render.add_post_draw_vert(points[0]);
  p.vert_ref[1] = g1_render.add_post_draw_vert(points[1]);
  p.vert_ref[2] = g1_render.add_post_draw_vert(points[2]);
  p.vert_ref[3] = g1_render.add_post_draw_vert(points[3]);
    
  g1_render.add_post_draw_quad(p);
#endif
}


/* Water Wave class definitions */

g2_water_wave_class::g2_water_wave_class(g1_object_type id,
                                       g1_loader_class *fp)
  : g1_shockwave_class(id, fp)
{  
  
}

void g2_water_wave_class::save(g1_saver_class *fp)
{
  g1_shockwave_class::save(fp);  
  //fp->start_version(DATA_VERSION);  
  //fp->write_format("ff2", &strength, &focal_radius, &radius);  
  //fp->end_version();
}


void g2_water_wave_class::setup_wave(const i4_3d_vector &pos,  // starting position (center)
                               float _strength,           // maximum ground distortion
                               w16 _radius)               // maximum radius in game units

{
  g1_map_class *map=g1_get_map();
 
  x=lx=pos.x;
  y=ly=pos.y;
  h=lh=pos.z;

  strength=_strength;
  radius=_radius;
  focal_radius=0;
  alloc_vert_map();

  red_intensity=0.0;
  green_intensity=0.0;
  blue_intensity=0;

  if (occupy_location())
  {
    request_think();
  }
}



i4_bool g2_water_wave_class::occupy_location()
{
  if (get_flag(MAP_OCCUPIED) || !vert_map)
    return i4_T;

  
  if (g1_object_class::occupy_location())
  {
 
    sw32 _ix = i4_f_to_i(x),
      _iy = i4_f_to_i(y);  

    float oor = 1.f/(float)radius;

    vert_offset *vo = vert_map;

    sw32 x_left,x_right,y_top,y_bottom;
  
    sw32 wx = g1_get_map()->width();
    sw32 wy = g1_get_map()->height();
  
    x_left   = _ix - radius; if (x_left<0)     x_left=0;
    x_right  = _ix + radius; if (x_right>wx-1)  x_right=wx-1;
    y_top    = _iy - radius; if (y_top<0)      y_top=0;
    y_bottom = _iy + radius; if (y_bottom>wy-1) y_bottom=wy-1;
  
    sw32 ix,iy;

    for (iy=y_top; iy<=y_bottom; iy++)
    {
      g1_map_vertex_class *v = g1_get_map()->vertex(x_left, iy);

      for (ix=x_left; ix<=x_right; ix++, vo++, v++)
      {
	    
	    if (!v->get_flag(g1_map_vertex_class::APPLY_WAVE_FUNCTION))
			{
			vo->displacement=0;
			continue;
			}
        i4_3d_vector dir = i4_3d_vector(x-ix, y-iy, h - v->get_height());

        i4_float dist  = dir.length();
        i4_float odist = 1.f/dist;

//         float t=dist * oor;
        float alpha=(float)fabs((dist - focal_radius)*2.0f);
        float multiplier=(2 / (alpha*alpha+1))-1;//now between -1 and +1
        //if (multiplier>1.0) multiplier=1.0;
        float new_height=multiplier*strength;

        sw32 new_height_value = v->height-i4_f_to_i(new_height/0.05f);

        if (new_height_value>3)//don't make to big waves
          new_height_value=3;
        else if (new_height_value<-3)
          new_height_value=3;
	
		//can't do this, because other shockwaves might have added something, too.
		//v->height=(w8)new_height_value;
      
        vo->displacement=new_height_value;      

        v->height+=(w8)vo->displacement;
		//no change of light here
        /*
		v->light_sum=0x80000000;
        v->normal=0x8000;

        w16 old_rgb=(w16)v->dynamic_light;
        sw32 r,g,b, o_r,og,ob;
        r=i4_f_to_i(red_intensity*multiplier*255);
        g=i4_f_to_i(green_intensity*multiplier*255);
        b=i4_f_to_i(blue_intensity*multiplier*255);
        
        o_r=(old_rgb>>16)&255;              // grab the old light values
        og=(old_rgb>>8)&255;
        ob=(old_rgb)&255;
        
        if (r+o_r>255) r=255-o_r;           // adjust for overflow
        if (g+og>255) g=255-og;
        if (b+ob>255) b=255-ob;

        vo->color = (r<<16) | (g<<8) | b;
        v->dynamic_light += vo->color;
		*/
      }
    }

    request_think();

    return i4_T;
  }
  else return i4_F;
}


void g2_water_wave_class::unoccupy_location()
{
  if (get_flag(MAP_OCCUPIED) && vert_map)
  {
    g1_object_class::unoccupy_location();
    
    sw32 _ix = i4_f_to_i(x),
         _iy = i4_f_to_i(y);

    vert_offset *vo=vert_map;

    sw32 x_left,x_right,y_top,y_bottom;
    sw32 w = g1_get_map()->width();
    sw32 h = g1_get_map()->height();

    x_left   = _ix - radius; if (x_left<0)     x_left=0;
    x_right  = _ix + radius; if (x_right>w-1)  x_right=w-1;
    y_top    = _iy - radius; if (y_top<0)      y_top=0;
    y_bottom = _iy + radius; if (y_bottom>h-1) y_bottom=h-1;  

    sw32 ix,iy;

    for (iy=y_top; iy<=y_bottom; iy++)
    {
      g1_map_vertex_class *v=g1_get_map()->vertex(x_left, iy);

      for (ix=x_left; ix<=x_right; ix++, vo++, v++)
      {
        v->height-=(w8)vo->displacement;
        //v->dynamic_light-=vo->color;
        //v->light_sum=0x80000000;
        //v->normal=0x8000;
      }
    }
  }
}


void g2_water_wave_class::think()
{
  grab_old();
  unoccupy_location();
  strength *= 0.92f;
  focal_radius += 0.01f;
  //red_intensity *= 0.95f;
  //green_intensity *= 0.80f;

  if (strength<0.01f)
    request_remove();
  else
  {
    occupy_location();
    request_think();
  }
}
