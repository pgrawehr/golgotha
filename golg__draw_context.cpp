/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "draw_context.h"
#include "g1_render.h"

g1_draw_context_class::g1_draw_context_class()
{ 
  style=0;
  context=0;
  screen=0;
  transform=0;
  dtransform=0;

  texture_scale=1;
  top=0;
  light=0;
  draw_editor_stuff=i4_F;

  default_render_bits=0;

}


void g1_draw_context_class::window_setup(w32 win_x, w32 win_y, w32 win_w, w32 win_h,
                                         float &center_x, float &center_y,
                                         float &scale_x, float &scale_y,
                                         float &ooscale_x, float &ooscale_y)
{
  w32 max_dim=win_w > win_h ? win_w : win_h;
    
  camera_scale_x=(float)max_dim;
  camera_scale_y=(float)max_dim;

  center_x=win_x + (i4_float)win_w/2.0f;
  center_y=win_y + (i4_float)win_h/2.0f;
  //static float scalefact=1.0f;

  float x_ratio=((float)max_dim/(float)win_w);
  float y_ratio=((float)max_dim/(float)win_h);
  //scale_x = (float)max_dim/win_w;
  //scale_y = (float)max_dim/win_h;
  scale_x=x_ratio;
  scale_y=y_ratio;

  //ooscale_x = (float)win_w/(float)max_dim;
  //ooscale_y = (float)win_h/(float)max_dim;
  ooscale_x=1/x_ratio;
  ooscale_y=1/y_ratio;
}
