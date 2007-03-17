/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef __R1_DX5_TEXTURE_HH__
#define __R1_DX5_TEXTURE_HH__

#include <ddraw.h>
#include <d3d.h>
#include "memory/lalloc.h"
#include "render/tmanage.h"
#include "render/dx5/r1_dx5.h"
#include "render/tex_no_heap.h"

class r1_dx5_texture_class :
	public r1_texture_manager_class
{
	friend r1_dx5_class;

public:

	class used_node
	{
public:
		R1_TEX_NO_HEAP_USED_NODE_DATA
		IDirectDrawSurface3 *vram_surface;
		IDirectDrawSurface3 *system_surface;
		D3DTEXTUREHANDLE texture_handle;
		//used for async loading
		w8 *data;
		i4_file_class *async_fp;
		r1_dx5_texture_class *self_tman; //Back reference to our tman.
	};

	r1_texture_no_heap_class *tex_no_heap;

	r1_dx5_texture_class(const i4_pal *pal);

	~r1_dx5_texture_class()
	{
		uninit();
	}

	void init();

	void uninit();

	void select_texture(r1_local_texture_handle_type handle, float &smul, float &tmul);

	used_node *make_surfaces_for_load(IDirectDrawSurface3 *&vram_surface,
									  IDirectDrawSurface3 *&system_surface,
									  r1_mip_load_info *&load_info,
									  sw32 actual_w, sw32 actual_h,
									  w8 node_alloc_flags=0);


	r1_miplevel_t *get_texture(r1_texture_handle handle,
							   w32 frame_counter,
							   sw32 desired_width,
							   sw32 &w, sw32 &h);

	//!Returns the texture data for the given handle in a new image.
	i4_image_class *get_texture_image(r1_texture_handle handle, int frame_counter, int desired_width);

	//!Changes the given texture to the new image
	int set_texture_image(r1_texture_handle handle, i4_image_class *im);

	i4_pal *MatchingPal(LPDDSURFACEDESC ddsd);

	i4_bool immediate_mip_load(r1_mip_load_info *load_info);
	i4_bool async_mip_load(r1_mip_load_info *load_info);

	i4_critical_section_class array_lock;

	i4_array<used_node *> finished_array;

	void async_load_finished(used_node *u);

	void free_mip(void *vram_handle);

	void next_frame();


};


#endif
