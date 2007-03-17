/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef _LINUX_SPECIFIC_HH_
#define _LINUX_SPECIFIC_HH_

#include "render/software/r1_software.h"

/*! \brief This class implements the renderer for plain x11.

   This contains the implementation of the system dependent
   part of the X11 display interface. The class is very short, since
   most of the functionality is in r1_software_class, the base class for
   all software renderers. This class is very similar to the windows gdi
   renderer.
 */
class r1_software_x11_class :
	public r1_software_class
{
	friend class r1_software_x11_render_window_class;
protected:
	//! The render target surface.
	i4_image_class *render_image;
	//! This pointer to the image data is used as the render target.
	w16 *render_image_data;
public:
	r1_software_x11_class();
	char *name()
	{
		return "Software Renderer for X11";
	}
	~r1_software_x11_class();
	i4_bool init(i4_display_class *display);
	void uninit();
	i4_bool resize(w16 new_width,w16 new_height);
	i4_bool expand_type_supported(r1_expand_type type);
	r1_render_window_class *create_render_window(int w, int h,
												 r1_expand_type expand_type);

	void clear_area(int x1, int y1, int x2, int y2, w32 color, float z);
	i4_image_class *create_compatible_image(w16 w, w16 h);

	void copy_part(i4_image_class *im,
				   int x, int y,                            // position on screen
				   int x1, int y1,                          // area of image to copy
				   int x2, int y2);
};



class r1_software_x11_render_window_class :
	public r1_software_render_window_class
{
	//i (trey), unfortunately dont really know how this works.
	//I (PG) do. Each renderer has a specific render window class associated.
	//Just do it the way this and the win32 implementation do it.
public:

	r1_software_x11_render_window_class(w16 w, w16 h,
										r1_expand_type expand_type,
										r1_render_api_class *api);


	~r1_software_x11_render_window_class();

	void draw(i4_draw_context_class &context);
	//void receive_event(i4_event *ev);

	char *name()
	{
		return "x11 software render window";
	}
};

//extern r1_software_class r1_software_class_instance;
extern r1_software_x11_class r1_software_x11_class_instance;
#endif
