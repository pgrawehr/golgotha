/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef _R1_DX5_HH_
#define _R1_DX5_HH_

#include "render/r1_api.h"
#include "render/r1_win.h"
#include <ddraw.h>
#include <d3d.h>
#include "video/win32/dx5_util.h"

inline DWORD FtoDW( FLOAT f )
{
	return *((DWORD *)&f);
};

class CR1_dx5_render_window_class :
	public r1_render_window_class
{
public:
	CR1_dx5_render_window_class(w16 w, w16 h, r1_expand_type expand_type, r1_render_api_class * api);
	~CR1_dx5_render_window_class();

	void    draw( i4_draw_context_class &context );
	void name(char * buffer)
	{
		static_name(buffer,"dx5 render window");
	};
//  I4_EVENT_NAME("dx5 render window"); //COMMENTED BY JJ 2000.04.10

};

class r1_dx5_class :
	public r1_render_api_class
{

	// position on screen  // area of image to copy
	void    copy_part(i4_image_class * im, int x, int y, int x1, int y1, int x2, int y2);
protected:
	DWORD m_dwFogColor,m_dwFogMode;
	FLOAT m_fFogStartValue,m_fFogEndValue,m_fFogDensity;
	BOOL m_bRangeBasedFog;
public:

	friend inline void make_d3d5_verts(D3DTLVERTEX * dx_v,r1_vert * r1_v,r1_dx5_class * c,int total);
	r1_dx5_class();
	~r1_dx5_class();

	// returns false if display is not compatible with render_api, i.e. if you pass
	// the directx display to the glide render api it return false
	// init will create the texture manager, which can be used after this call
	// text_mem_size if size of buffer to hold compressed textures (in system memory)
	i4_bool init(i4_display_class * display);

	// this will delete the texture manager(s) (and free textures associated with) created by init
	void uninit();

	//Reinit the renderer after a size change (i.e change zbuf surface)
	virtual i4_bool resize(w16 newx, w16 newy);

	virtual i4_bool redepth(w16 new_bitdepth);

	virtual void set_fogging_mode(w32 fogcolor, i4_float startvalue, i4_float endvalue);

	//creates a new texture manager (returns its handle)
	virtual r1_texture_manager_class *install_new_tmanager(w32& index);
	void set_z_range(i4_float near_z, i4_float far_z);


	void set_filter_mode(r1_filter_type type);
	void set_alpha_mode(r1_alpha_type type);
	void set_write_mode(r1_write_mask_type mask);

	// draws the polygon at the end of the frame (during end_render())
	virtual void render_poly(int t_verts, r1_vert * verts);
	void render_pixel(int t_points, r1_vert * pixel);
	void render_lines(int t_lines, r1_vert * verts);
	void clear_area(int x1, int y1, int x2, int y2, w32 color, float w);

	// creates an image of the same bit depth and palette of screen (for use with put_image)
	i4_image_class *create_compatible_image(w16 w, w16 h);

	// texture handle is obtained from the texture manager, this is enables texture mapping
	void use_texture(r1_texture_handle material_ref,
					 sw32 desired_width,
					 w32 frame);
	void use_texture(w32 index, r1_texture_handle material_ref,
					 sw32 desired_width,
					 w32 frame);

	r1_alpha_type pre_holy_alpha_mode;
	r1_write_mask_type pre_holy_write_mask;

	//i4_bool states_have_changed; //now defined in parent - Class
	i4_bool holy_mode;
	i4_bool texture_mode;

	void enable_holy();
	void disable_holy();

	virtual void flush_vert_buffer();

	// drawing will the constant color to render with if textures are disabled
	void disable_texture();

	r1_render_window_class *create_render_window(int visable_w, int visable_h,
												 r1_expand_type type);

	i4_float x_off,y_off;

	i4_bool hardware_tmapping;
	i4_bool needs_square_textures;

	char dd_driver_name[128];
	char d3d_driver_name[128];

	char *name()
	{
		return d3d_driver_name;
	}

	IDirectDrawSurface3 * zbuffer_surface;
	IDirect3D2 * d3d;
	IDirect3DDevice2 * d3d_device;
	IDirect3DViewport2 * d3d_viewport;
	dx5_d3d_info * info;
};

extern r1_dx5_class r1_dx5_class_instance;

#endif
