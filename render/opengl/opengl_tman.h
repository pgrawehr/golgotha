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
#ifndef OPENGL_TMAN_HH
#define OPENGL_TMAN_HH

#include "memory/lalloc.h"
#include "render/tmanage.h"
#include "render/tex_no_heap.h"
#include "render/tnode.h"
#include <GL/gl.h>
#include <GL/glu.h>

#define R1_OPENGL_TEXFORMAT_HOLY 1   // 1555
#define R1_OPENGL_TEXFORMAT_ALPHA 2  // 4444

class r1_opengl_texture_manager_class : public r1_texture_manager_class
{
public:

  class used_node
  {
  public:
	R1_TEX_NO_HEAP_USED_NODE_DATA
	GLuint gltexname;
	w8 texformatflags;
	// use for async loading
	w8 *data;
	i4_file_class *async_fp;
	r1_opengl_texture_manager_class *self_tman; //for back-reference 
  };

  r1_texture_no_heap_class *tex_no_heap;

  r1_opengl_texture_manager_class(const i4_pal *pal);

  ~r1_opengl_texture_manager_class()
  {
	uninit();
  }

  void init();

  void uninit();

  //!Returns the texture data for the given handle in a new image.
  virtual i4_image_class *get_texture_image(r1_texture_handle handle);

  //!Changes the given texture to the new image
  virtual int set_texture_image(r1_texture_handle handle, i4_image_class *im);
  

  void select_texture(r1_local_texture_handle_type handle, float &smul, float &tmul);

  r1_miplevel_t *get_texture(r1_texture_handle handle,
							 w32 frame_counter,
							 sw32 desired_width,
							 sw32 &w, sw32 &h);

  i4_bool immediate_mip_load(r1_mip_load_info *load_info);
  i4_bool async_mip_load(r1_mip_load_info *load_info);

  i4_critical_section_class array_lock;

  i4_array<used_node *> finished_array;

  void async_load_finished(used_node *u);

  void free_mip(void *vram_handle);

  void next_frame();

//  sw32 bytes_loaded;
//  sw32 no_of_textures_loaded;

private:

  GLvoid *scratch; // currently used for gluScaleImage()

  void teximage2d(used_node *u);

  used_node *make_new_used_node(r1_mip_load_info *&load_info, w8 node_alloc_flags = 0);

};
extern r1_opengl_texture_manager_class *r1_opengl_texture_manager_class_instance;

#endif


