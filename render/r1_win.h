/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_WIN_HH
#define R1_WIN_HH

#include "window/window.h"
#include "r1_api.h"

class r1_render_window_class : public i4_parent_window_class
{  
protected:
  r1_render_api_class *api;
  r1_expand_type expand_type;

public:
	//Called if window changes
  void resize(w16 new_width, w16 new_height);
	//Called if bitdepth has changed
  void redepth(w16 new_bitdepth);

  r1_render_window_class(w16 w, w16 h,
                         r1_expand_type expand_type,
                         r1_render_api_class *api)
    : i4_parent_window_class(w,h),
      expand_type(expand_type),
      api(api)
  {
  }

  int render_area_width();
  int render_area_height();

  // use this to clip out child windows using a z buffer, also sets z-range for scene
  void clip_with_z(i4_draw_context_class &context, i4_float near_z_range, i4_float far_z_range);

  virtual void begin_render() {}
  virtual void end_render() {}
};

#endif
