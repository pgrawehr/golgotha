/********************************************************************** 

    Golgotha Forever - A portable, free 3D strategy and FPS game.
    Copyright (C) 1999 Golgotha Forever Developers

    Sources contained in this distribution were derived from
    Crack Dot Com's public release of Golgotha which can be
    found here:  http://www.crack.com

    All changes and new works are licensed under the GPL:

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For the full license, see COPYING.

***********************************************************************/
#include "pch.h"
#include <stdio.h>
#include <GL/gl.h>
//#include <GL/glu.h>
#include "image/image.h"
#include "render/r1_api.h"
#include "video/opengl/opengl_display.h"
#include "render/opengl/opengl_tman.h"
#include "render/r1_clip.h"
#include "render/r1_win.h"
#include "render/r1_res.h"


static struct static_info_struct
{
  float smul, tmul;    // texture coordinate multiplers
  float a_mul, w_mul;            // scales the w value to best fit application's range  
  float xoff, yoff;    // window offset additions
} static_info = {0.f, 0.f, 255.f, 1.f, 0.f, 0.f };

//class r1_opengl_render_class;
//extern r1_opengl_render_class r1_opengl_render;

// Only activate this one if really required. Is extremelly slow. 
//#define OGL_ERROR_CHECK() check_error(__FILE__,__LINE__)
#define OGL_ERROR_CHECK() 

class r1_opengl_render_window_class : public r1_render_window_class
{
public:
  r1_opengl_render_window_class *next_rw;
  i4_bool need_flip_update;
  r1_opengl_render_window_class(w16 w, w16 h, r1_expand_type expand_type, r1_render_api_class *api);
  ~r1_opengl_render_window_class();
  void draw(i4_draw_context_class &context);
  void request_redraw(i4_bool for_a_child)
  {
	if (!for_a_child)
    {
	  if (children.begin() != children.end())
		children.begin()->request_redraw(i4_F);
	}
	need_flip_update = i4_T;
	r1_render_window_class::request_redraw(for_a_child);
  }
  char *name() { return "OpenGL render window"; }
};

//r1_opengl_render_window_class *r1_render_window_list = 0;

r1_opengl_render_window_class::r1_opengl_render_window_class(w16 w, w16 h,
															 r1_expand_type expand_type,
															 r1_render_api_class *api)
  : r1_render_window_class(w,h,expand_type, api)
{
  need_flip_update = i4_T;
  next_rw = 0; //r1_render_window_list;
  //r1_render_window_list = this;
}

r1_opengl_render_window_class::~r1_opengl_render_window_class()
{
  /*if (this == r1_render_window_list)
	r1_render_window_list = next_rw;
  else
  {
	r1_opengl_render_window_class *last = 0;
	for (r1_opengl_render_window_class *p=r1_render_window_list; p && p!=this;)
    {
	  last = p;
	  p = p->next_rw;
	}
	last->next_rw = next_rw;
  }*/
}

void r1_opengl_render_window_class::draw(i4_draw_context_class &_context)
{
  volatile static i4_bool in_render_rec=i4_F;
  //r1_opengl_render.x_off=_context.xoff;
  //r1_opengl_render.y_off=_context.yoff;
  //r1_render_api_class::context = &_context;
  if (in_render_rec)
  {
  	r1_render_window_class::draw(_context);
	return;
  }
  in_render_rec=i4_T;
  static_info.xoff = _context.xoff;
  static_info.yoff = _context.yoff;
  clip_with_z(_context, RENDER_DEFAULT_NEAR_DISTANCE,RENDER_DEFAULT_FAR_DISTANCE);//are the values ok?
  
  r1_render_window_class::draw(_context);

  

  if (need_flip_update)
  {
	request_redraw(i4_T);

	for (win_iter i=children.begin(); i!=children.end(); ++i)
	  i->request_redraw(i4_F);

	need_flip_update = i4_F;
  }
  in_render_rec=i4_F;
}

  static float opengl_z_scale=0,opengl_w_scale=0;
class r1_opengl_render_class : public r1_render_api_class
{
private:

  static i4_display_class       *display;
  float oo_half_width, oo_half_height; // inverse of half display dimensions

  r1_alpha_type       pre_holy_alpha_mode;
  r1_write_mask_type  pre_holy_write_mask;

  i4_bool texture_mode;
  i4_bool holy_mode;
  i4_float min_z,max_z;

i4_bool pass_verticies(int t_verts, r1_vert *src)
{
  float x, y, z, r=0, g=0, b=0, a,w;
  float w_fact;
  while (t_verts--)
  {
	x = src->px + static_info.xoff;
	y = src->py + static_info.yoff;

	//z = 1.f/src->w;
	//z = (z - r1_near_clip_z) / (r1_far_clip_z - r1_near_clip_z) * 2.f - 1.f;
	//z = src->w+1.f;
	//if (z<1.0)
	//	z=1.0f;
	z = (src->w);
	z = (z-r1_near_clip_z) / (r1_far_clip_z-r1_near_clip_z);
	w = src->w;
	if (z<min_z)
		min_z=z;
	if (z>max_z)
		max_z=z;
	//w_fact= 1.0f/(r1_far_clip_z-r1_near_clip_z);
	w = 1.0f/z;
	//if (w<0)
	//	w=0;
	a = src->a;

	if (shade_mode == R1_WHITE_SHADING)
    {
	  float base = src->r;
	  r = base * r_tint_mul;
	  g = base * g_tint_mul;
	  b = base * b_tint_mul;
	}

	else if (shade_mode == R1_SHADE_DISABLED)
    {
	  r = r_tint_mul;
	  g = g_tint_mul;
	  b = b_tint_mul;
	}

	else if (shade_mode == R1_CONSTANT_SHADING)
    {
	  float const_r = ((const_color >> 16) & 0xff) / 255.f;
	  float const_g = ((const_color >> 8) & 0xff) / 255.f;
	  float const_b = (const_color & 0xff) / 255.0;
	  r = const_r * r_tint_mul;
	  g = const_g * g_tint_mul;
	  b = const_b * b_tint_mul;
	}

	else if (shade_mode == R1_COLORED_SHADING)
    {
	  r = src->r * r_tint_mul;
	  g = src->g * g_tint_mul;
	  b = src->b * b_tint_mul;
	}

	glColor4f(r,g,b,a);
	if (texture_mode)
	  glTexCoord2f(src->s,src->t);
	glVertex4f( w*(oo_half_width * x - 1.0) , 
			w*(1.0 - oo_half_height * y) , w*z, w);
	src++;
  }

  return i4_T;
}


public:

  i4_float x_off,y_off;

  void disable_texture();
  void enable_texture();
  void check_error(char *file, int line);

  void set_write_mode(r1_write_mask_type mask) {

	if (holy_mode == i4_T)
	  disable_holy();

	w32 diff = mask ^ get_write_mask();

	if (diff & R1_WRITE_W) {
	  if (mask & R1_WRITE_W)
		glDepthMask(GL_TRUE);
	  else
		glDepthMask(GL_FALSE);
	}
	OGL_ERROR_CHECK();

	if (diff & R1_COMPARE_W) {
	  if (mask & R1_COMPARE_W) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);
	  }
	  else
		glDisable(GL_DEPTH_TEST);
	}
	OGL_ERROR_CHECK();

	if (diff & R1_WRITE_COLOR) {
	  if (mask & R1_WRITE_COLOR) {
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	  }
	  else {
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	  }
	}
	OGL_ERROR_CHECK();

	r1_render_api_class::set_write_mode(mask);

  }


  virtual r1_texture_manager_class *install_new_tmanager(w32 &index)
  {
  	r1_texture_manager_class *new_tman=0;
	new_tman=new r1_opengl_texture_manager_class(tmanager->get_pal());
	new_tman->is_master_tman=i4_F;
	for (int k=0;k<tmanagers.size();k++)
		{
		if (tmanagers[k]==0) 
			{
			tmanagers[k]=new_tman;
			index=k;
			return new_tman;
			}
		}
	index=tmanagers.add(new_tman);
	return new_tman;
  }
  void set_filter_mode(r1_filter_type type) {

	switch(type) {

	case R1_NO_FILTERING:
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	  break;

	case R1_BILINEAR_FILTERING:
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  break;
	}
    OGL_ERROR_CHECK();
	r1_render_api_class::set_filter_mode(type);
  }

  r1_opengl_render_class()
  {

	holy_mode = i4_F;
	write_mask = R1_COMPARE_W | R1_WRITE_W | R1_WRITE_COLOR;
	shade_mode = R1_SHADE_DISABLED;
	alpha_mode = R1_ALPHA_DISABLED;

	pre_holy_write_mask = write_mask;
	pre_holy_alpha_mode = alpha_mode;

	// currently only R1_SOFTWARE
	render_device_flags = 0;
	x_off=0;
	y_off=0;
	min_z=10000;
	max_z=-1000;
  }

  // destructor
  virtual ~r1_opengl_render_class()
  {

  }

  // return false if display not compatible with render api
  virtual i4_bool init(i4_display_class *display);
  
  void enable_holy();

  void disable_holy() {

	if (holy_mode != i4_F) {
	  holy_mode = i4_F;
	  set_alpha_mode(pre_holy_alpha_mode);
	  set_write_mode(pre_holy_write_mask);
	  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	}
	OGL_ERROR_CHECK();

  }

  void uninit()
  {
	if (tmanager)
    	{
	  delete tmanager;
	  tmanager = 0;
	}
	r1_render_api_class::uninit();

  }

  virtual void set_constant_color(w32 color) {

	  r1_render_api_class::set_constant_color(color);

	  float a = ( (const_color & 0xff000000) >> 24 ) / 255.0;
	  float r = ( (const_color & 0xff0000) >> 16) / 255.0;
	  float g = ( (const_color & 0xff00) >> 8 ) / 255.0;
	  float b = (const_color & 0xff) / 255.0;
	  //glBlendColorEXT(r,g,b,a);
	  glColor4f(r,g,b,a);
	  OGL_ERROR_CHECK();

  }

  virtual void clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
  {

	w32 old_constant_color = get_constant_color();
	if (z<r1_near_clip_z)
		z=r1_near_clip_z;
	if (z>r1_far_clip_z)
		z=r1_far_clip_z;
	set_constant_color(color);
	i4_bool renable_texture = texture_mode;
	
	//x1+=context->xoff;//adjust for window position
	//x2+=context->xoff;
	//y1+=context->yoff;
	//y2+=context->yoff;

	r1_shading_type old_shade_mode = get_shade_mode();
	set_shading_mode(R1_CONSTANT_SHADING);

	disable_texture();

	r1_vert v[4];

	v[0].px = x1;     v[0].py = y1;
	v[1].px = x1;     v[1].py = y2+1;
	v[2].px = x2+1;   v[2].py = y2+1;
    v[3].px = x2+1;   v[3].py = y1;

	v[0].w = v[1].w = v[2].w = v[3].w = 0;

	v[0].a = v[1].a = v[2].a = v[3].a = 0.f;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	render_poly(4,v);
	glDepthFunc(GL_GREATER);
	OGL_ERROR_CHECK();
	//glClearColor(0,0,0,0);
	//glClear(GL_COLOR_BUFFER_BIT);
	glClearDepth(0);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (renable_texture == i4_T)
	  enable_texture();

	set_shading_mode(old_shade_mode);
	set_constant_color(old_constant_color);
  }


  virtual void set_z_range(float nearvalue, float farvalue)
  {
	opengl_z_scale=0.999f/farvalue;
	opengl_w_scale=nearvalue*0.999f;  
	r1_near_clip_z = nearvalue;
	r1_far_clip_z = farvalue;
  }

  virtual void set_alpha_mode(r1_alpha_type type)
  {


	switch (type) {
	case R1_ALPHA_DISABLED:
	  glDisable(GL_BLEND);
	  break;

	case R1_ALPHA_CONSTANT:
	  //printf("constant alpha\n");
	  glEnable(GL_BLEND);
	  glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA_SATURATE);
	  break;

	case R1_ALPHA_LINEAR:
	  glEnable(GL_BLEND);
	  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	  break;

	}
	OGL_ERROR_CHECK();
	r1_render_api_class::set_alpha_mode(type);
  }
 


  virtual void render_poly(int t_verts, r1_vert *verts)
  {
	glBegin(GL_TRIANGLE_FAN);
	pass_verticies(t_verts,verts);
	glEnd();
	OGL_ERROR_CHECK();
  }

  virtual void render_lines(int t_lines, r1_vert *verts)
  {
	glBegin(GL_LINE_STRIP);
	pass_verticies(t_lines,verts);
	glEnd();
	OGL_ERROR_CHECK();
  }

  virtual void render_pixel(r1_vert *pixel)
  {
	glBegin(GL_POINTS);
	pass_verticies(1,pixel);
	glEnd();
	OGL_ERROR_CHECK();
  }
  void render_pixel(int t_points, r1_vert *pixel)
  {
  	glBegin(GL_POINTS);
	pass_verticies(t_points,pixel);
	glEnd();
	OGL_ERROR_CHECK();
  }

  r1_render_window_class *create_render_window(int visw, int vish, r1_expand_type type)
  {
	return new r1_opengl_render_window_class(visw,vish,R1_COPY_1x1, this);
  }

  virtual i4_image_class *create_compatible_image(w16 w, w16 h)
  {
	return i4_create_image(w,h,display->get_palette());
  }


  char *name() { return "Golgotha OpenGL render class"; }

  void copy_part(i4_image_class *im, int x, int y, int x1, int y1, int x2, int y2);

  void use_texture(r1_texture_handle material_ref, sw32 desired_width, w32 frame);
  void use_texture(w32 index, r1_texture_handle material_ref, sw32 desired_width, w32 frame);
  

};

r1_opengl_render_class r1_opengl_render;
i4_display_class *r1_opengl_render_class::display=0;

void r1_opengl_render_class::copy_part(i4_image_class *im,
			   int x, int y,       // position on screen
			   int x1, int y1,     // image area to copy
			   int x2, int y2) {

  i4_warning("copy_part() called");
  if (display) {

	int width = x2 - x1 + 1;
	int height = y2 - y1 + 1;

	i4_image_class *dst_im = i4_create_image( width, height, display->get_palette());

	if (dst_im) {

	  i4_draw_context_class no_clip_context(0,0, width - 1, height - 1);
	  im->put_part(dst_im,0,0,x1,y1,x2,y2,no_clip_context);

	  glPushAttrib(GL_ALL_ATTRIB_BITS);

	  glDrawBuffer(GL_BACK);

	  glDisable(GL_TEXTURE_2D);
	  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	  glDisable(GL_DEPTH_TEST);
	  glDepthMask(GL_FALSE);
	  glDisable(GL_BLEND);
	  OGL_ERROR_CHECK();
	  glPixelZoom(1.,-1.);
	  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	  glRasterPos2f( oo_half_width * (float)x - 1.0, 1. - oo_half_height * (float)y );
	  glDrawPixels(width,height,GL_RGBA,GL_RGB16,(GLubyte*)dst_im->data);

	  glPopAttrib();

	  delete dst_im;
	  glEnable(GL_DEPTH_TEST);
	  glDepthMask(GL_TRUE);
	  glEnable(GL_TEXTURE_2D);
	  glEnable(GL_BLEND);
	  OGL_ERROR_CHECK();
	}

  }
}

void r1_opengl_render_class::check_error(char* file, int line)
{
	GLenum e=glGetError();
	if (e==GL_NO_ERROR)
		return;
	char *s="";
	switch(e)
	{
		case GL_INVALID_ENUM:
			s="GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			s="GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			s="GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			s="GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			s="GL_STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			s="GL_OUT_OF_MEMORY";
			break;
		default:
			s="UNKNOWN_GL_ERROR";
			break;					
			
	}
	i4_warning("Opengl Error in File %s, line %d: %s\n",file,line,s);
}

void r1_opengl_render_class::disable_texture() {

  if (texture_mode != i4_F) {
	glDisable(GL_TEXTURE_2D);
	texture_mode = i4_F;
	last_node = 0;
  }
  OGL_ERROR_CHECK();

}

void r1_opengl_render_class::enable_texture() {

  if (texture_mode != i4_T) {
	glEnable(GL_TEXTURE_2D);
	texture_mode = i4_T;
	last_node = 0;
  }
  OGL_ERROR_CHECK();

}

i4_bool r1_opengl_render_class::init(i4_display_class *_display)
    {
	if (_display == i4_opengl_display) {
	  display = _display;

	  // initial texture state
	  texture_mode = i4_T;
	  glEnable(GL_TEXTURE_2D);
	  last_node = 0;

	  // initial depth buffer state
	  glDepthMask(GL_TRUE);
	  glEnable(GL_DEPTH_TEST);
	  glDepthFunc(GL_GREATER);
	  
	  OGL_ERROR_CHECK();

	  // set up glBlendColor
	  set_constant_color(get_constant_color());

	  glViewport(0, 0, display->width(), display->height());
	  oo_half_width = 2.f / (float)display->width();
	  oo_half_height = 2.f /(float)display->height();

	  glDepthRange(0.f,1.f);
	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  OGL_ERROR_CHECK();
	  
	  glMatrixMode(GL_TEXTURE);
	  glLoadIdentity();
	  OGL_ERROR_CHECK();

	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
	  OGL_ERROR_CHECK();
	  
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
	  OGL_ERROR_CHECK();
	  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	}
	else
        { //it's not an opengl capable display driver that was instantiated
	  return i4_F;
	}

	tmanager = new r1_opengl_texture_manager_class(display->get_palette());

	r1_name_cache_file("opengl");
	return i4_T;
    }

void r1_opengl_render_class::enable_holy() 
    {
    
    if (holy_mode != i4_T) {
        pre_holy_alpha_mode = get_alpha_mode();
        pre_holy_write_mask = get_write_mask();
        
        set_write_mode(R1_WRITE_W | R1_COMPARE_W | R1_WRITE_COLOR);
        glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        set_alpha_mode(R1_ALPHA_LINEAR);
        OGL_ERROR_CHECK();
        holy_mode = i4_T;
        }
    }

void r1_opengl_render_class::use_texture(w32 index, r1_texture_handle handle, sw32 desired_width, w32 frame)
{

  if (!tmanagers[index]->valid_handle(handle)) {
	disable_texture();
	return;
  }

  enable_texture();

  sw32 w, h;
  r1_miplevel_t *mip = (tmanagers[index])->get_texture(handle,frame,desired_width,w,h);

  if (mip && mip != last_node) {

	last_node = mip;

	((r1_opengl_texture_manager_class *)tmanagers[index])->select_texture((r1_local_texture_handle_type)mip->vram_handle,static_info.smul,static_info.tmul);
	switch (get_filter_mode()) {

	case R1_NO_FILTERING:
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	  break;

	case R1_BILINEAR_FILTERING:
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  break;

	}
	OGL_ERROR_CHECK();

	if (mip->entry->is_alphatexture() == i4_T || mip->entry->is_transparent())
	  enable_holy();
	else
	  disable_holy();

  }

}

void r1_opengl_render_class::use_texture(r1_texture_handle handle,
										 sw32 desired_width, w32 frame)
{

  if (!tmanager->valid_handle(handle)) {
	disable_texture();
	return;
  }

  enable_texture();

  sw32 w, h;
  r1_miplevel_t *mip = tmanager->get_texture(handle,frame,desired_width,w,h);

  if (mip && mip != last_node) {

	last_node = mip;

	((r1_opengl_texture_manager_class *)tmanager)->select_texture((r1_local_texture_handle_type)mip->vram_handle,static_info.smul,static_info.tmul);
	switch (get_filter_mode()) {

	case R1_NO_FILTERING:
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	  break;

	case R1_BILINEAR_FILTERING:
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  break;

	}
	OGL_ERROR_CHECK();
	if (mip->entry->is_alphatexture() == i4_T || mip->entry->is_transparent())
	  enable_holy();
	else
	  disable_holy();

  }

}
