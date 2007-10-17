/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef _WIN32_SPECIFIC_HH_
#define _WIN32_SPECIFIC_HH_

#include "render/software/r1_software.h"
#include <ddraw.h>

class r1_software_dd_class :
	public r1_software_class
{
	friend class r1_software_dd_render_window_class;
protected:
	sw32 current_width,current_height;
	w32 expand_depth;
public:
	r1_software_dd_class();
	char *name()
	{
		return "Software Renderer with ddraw";
	}
	~r1_software_dd_class();
	i4_bool init(i4_display_class * display);
	void uninit();
	i4_bool resize(w16 new_width,w16 new_height);
	i4_bool expand_type_supported(r1_expand_type type);
	r1_render_window_class *create_render_window(int w, int h,
												 r1_expand_type expand_type);

	void clear_area(int x1, int y1, int x2, int y2, w32 color, float z);
	i4_image_class *create_compatible_image(w16 w, w16 h);

	void copy_part(i4_image_class * im,
				   int x, int y,                            // position on screen
				   int x1, int y1,                          // area of image to copy
				   int x2, int y2);
};

class r1_software_gdi_class :
	public r1_software_class
{
protected:
	friend class r1_software_gdi_render_window_class;
	i4_image_class * render_image;
	w16 * render_image_data;
public:
	r1_software_gdi_class();
	char *name()
	{
		return "Software Renderer with plain GDI";
	}
	~r1_software_gdi_class();
	i4_bool init(i4_display_class * display);
	void uninit();
	i4_bool resize(w16 new_width,w16 new_height);
	i4_bool expand_type_supported(r1_expand_type type);
	r1_render_window_class *create_render_window(int w, int h,
												 r1_expand_type expand_type);
	void clear_area(int x1, int y1, int x2, int y2, w32 color, float z);
	i4_image_class *create_compatible_image(w16 w, w16 h);
	void copy_part(i4_image_class * im,
				   int x, int y,                            // position on screen
				   int x1, int y1,                          // area of image to copy
				   int x2, int y2);
};

class r1_software_dd_render_window_class :
	public r1_software_render_window_class
{
	//i (trey), unfortunately dont really know how this works.
	//w16 current_width,current_height;
public:
	//IDirectDrawSurface3 *surface;  //the render target must
	//depend on the renderer, not the window

	r1_software_dd_render_window_class(w16 w, w16 h,
									   r1_expand_type expand_type,
									   r1_render_api_class * api);

	~r1_software_dd_render_window_class();

	//void resize(w16 new_width, w16 new_height); //the window must not handle
	//the resizing of the render target surface. That should be done by the renderer.
	void draw(i4_draw_context_class &context);
	//void receive_event(i4_event *ev);

	void name(char * buffer)
	{
		static_name(buffer,"dd software render window");
	}
};

class r1_software_gdi_render_window_class :
	public r1_software_render_window_class
{
	//i (trey), unfortunately dont really know how this works.
public:

	r1_software_gdi_render_window_class(w16 w, w16 h,
										r1_expand_type expand_type,
										r1_render_api_class * api);


	~r1_software_gdi_render_window_class();

	void draw(i4_draw_context_class &context);
	//void receive_event(i4_event *ev);

	void name(char * buffer)
	{
		static_name(buffer,"gdi software render window");
	}
};

//extern r1_software_class r1_software_class_instance;
extern r1_software_dd_class r1_software_dd_class_instance;
extern r1_software_gdi_class r1_software_gdi_class_instance;
#endif
