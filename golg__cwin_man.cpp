/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "cwin_man.h"
#include "resources.h"
#include "controller.h"
#include "border_frame.h"
#include "map_view.h"
#include "player.h"
#include "image/image.h"
#include "window/win_evt.h"
#include "device/kernel.h"
#include "window/style.h"
#include "gui/deco_win.h"
#include "gui/image_win.h"
#include "map.h"
#include "render/r1_win.h"
#include "g1_render.h"
#include "objs/stank.h"
#include "saver.h"
#include "math/pi.h"
#include "device/key_man.h"
#include "input.h"
#include "map_man.h"
#include "lisp/lisp.h"
#include "loaders/load.h"


g1_cwin_man_class *g1_cwin_man=0;



void g1_change_key_context(w8 view_mode)
{
  if (i4_key_man.current_context()==i4_key_man.get_context_id("menu"))
	  return;
  switch (view_mode)
  {
    case G1_EDIT_MODE :
      i4_key_man.set_context("editor"); break;

    case G1_STRATEGY_MODE :
      i4_key_man.set_context("strategy"); break;

    case G1_ACTION_MODE :
      i4_key_man.set_context("action"); break;

    case G1_FOLLOW_MODE :
      i4_key_man.set_context("follow"); break;

    case G1_WATCH_MODE :
      //i4_key_man.set_context("strategy"); //leave context as is.
	  break;
	case G1_MAXTOOL_MODE:
	  i4_key_man.set_context("maxtool");break;
  }
}




void g1_cwin_man_class::save_views(g1_saver_class *fp)
{
  fp->mark_section("Game view setting V1");
  view_state.save(fp);
}

void g1_cwin_man_class::load_views(g1_loader_class *fp)
{
  if (fp && !fp->goto_section("Game view setting V1"))
    fp=0;     // load defaults if not in file

  view_state.load(fp);
}
i4_window_class *blank_screen=0;

void g1_cwin_man_class::init(i4_parent_window_class *_parent,
                             i4_graphical_style_class *_style,
                             i4_image_class *_root_image,
                             i4_display_class *display,
                             i4_window_manager_class *wm)
{
  style=_style;
  parent=_parent;
  root_image=_root_image;
  pal=_root_image->get_pal(); 
}

g1_cwin_man_class::g1_cwin_man_class()
{
  parent=0;
  root_image=0;
  style=0;
}

void g1_cwin_man_class::create_views()

{    
  delete blank_screen;
  blank_screen=NULL;
  if (!g1_border.get())
  {
    g1_strategy_screen=new g1_strategy_screen_class();

    g1_border = new g1_border_frame_class(); 
    //    g1_current_controller->view=view_state;
    parent->add_child(0,0,g1_border.get());
	if (!g1_map_is_loaded())
		{
		  parent->add_child(0, 20, 
                         blank_screen=new i4_image_window_class(i4_load_image("bitmaps/editor/nomap.jpg"), i4_T, i4_F));

		}

  }
}


void g1_cwin_man_class::destroy_views()
{
  if (g1_border.get())
  {
    i4_kernel.delete_handler(g1_strategy_screen.get());
    i4_kernel.delete_handler(g1_border.get());
  }
  if (blank_screen) delete blank_screen;
  blank_screen=0;
}

void g1_cwin_man_class::receive_event(i4_event *ev)
{  
//   if (view.get())
//     g1_input.receive_event(ev);
}


void g1_cwin_man_class::map_changed()
{
  g1_player_type p=g1_default_player;
  g1_radar_recalculate_backgrounds();
}

