// editor.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "math/trig.h"
#include "app/app.h"
#include "editor/editor.h"
#include "editor/edit_id.h"
#include "editor/pmenu.h"
#include "editor/contedit.h"
#include "editor/e_state.h"
#include "editor/lisp_interaction.h"
#include "editor/e_res.h"
#include "main/main.h"
#include "controller.h"
#include "math/pi.h"
#include "math/angle.h"
#include "math/random.h"
#include "map.h"
#include "tile.h"
#include "window/window.h"
#include "window/win_evt.h"
#include "window/style.h"
#include "window/wmanager.h"
#include "window/colorwin.h"
#include "string/string.h"
#include "draw_context.h"
#include "font/font.h"
#include "resources.h"
#include "saver.h"
#include "menu/pull.h"
#include "menu/textitem.h"
#include "device/event.h"
#include "device/keys.h"
#include "device/kernel.h"
#include "device/device.h"

#include "editor/dialogs/e_time.h"
#include "editor/dialogs/obj_win.h"
#include "editor/dialogs/scene.h"
#include "editor/dialogs/tile_picker.h"
#include "editor/dialogs/path_win.h"
#include "editor/dialogs/debug_win.h"
#include "editor/dialogs/object_picker.h"
#include "editor/dialogs/d_time.h"
#include "editor/dialogs/d_light.h"
#include "editor/dialogs/pick_win.h"
#include "editor/dialogs/scroll_picker.h"
#include "editor/dialogs/tile_win.h"
#include "editor/commands/fill.h"

#include "mess_id.h"
#include "loaders/load.h"
#include "loaders/write.h"
#include "file/get_filename.h"
#include "m_flow.h"
#include "input.h"
#include "light.h"
#include "math/spline.h"
#include "time/gui_prof.h"
#include "border_frame.h"
#include "remove_man.h"
#include "status/status.h"
#include "status/gui_stat.h"
#include "level_load.h"
#include "map_vert.h"

#include "render/r1_api.h"
#include "render/r1_win.h"
#include "render/r1_clip.h"
#include "g1_render.h"
#include "gui/create_dialog.h"
#include "gui/text_input.h"
#include "gui/li_pull_menu.h"
#include "gui/button.h"
#include "gui/butbox.h"
#include "gui/text.h"
#include "gui/deco_win.h"
#include "gui/image_win.h"
#include "gui/smp_dial.h"
#include "gui/slider.h"
#include "gui/scroll_bar.h"

#include "g1_object.h"
#include "remove_man.h"
#include "g1_limits.h"
#include "g1_tint.h"
#include "g1_speed.h"
#include "tick_count.h"
#include "sky.h"
#include "obj3d.h"


#include "critical_graph.h"
#include "editor/mode/e_tile.h"
#include "editor/mode/e_object.h"
//#include "editor/mode/e_game.h"
#include "editor/mode/e_ai.h"
#include "editor/mode/e_mode.h"
#include "editor/mode/e_camera.h"
#include "editor/mode/e_light.h"
#include "dll/dll_man.h"

#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "lisp/li_dialog.h"
#include "lisp/li_class.h"

#include "player.h"
#include "map_view.h"
#include "map_data.h"
#include "map_vars.h"

#include "solvemap_astar.h"
#include "solvemap_astar2.h"
#include "solvegraph_breadth.h"
#include "solvemap_breadth.h"

#include "font/font.h"
#include <stdarg.h>

#include "g1_speed.h"
#include "objs/stank.h"
#include "image/color.h"

#include "f_tables.h"
#include "map_vert.h"
#include "map_man.h"
#include "map_cell.h"
#include "map_light.h"


#include "error/error.h"
#include "light.h"
#include "cwin_man.h"

#include "objs/path_object.h"
#include "objs/map_piece.h"
#include "objs/def_object.h"
#include "objs/light_o.h"
#include "objs/bases.h"

#include "render/r1_win.h"
#include "render/r1_api.h"
#include "render/tmanage.h"

#include "file/get_filename.h"
#include "file/file_open.h"
#include "saver_id.h"

#include "image/image32.h"
#include "team_api.h"

#include "time/time.h"


g1_editor_class g1_editor_instance;


int g1_controller_edit_class::get_current_splines(i4_spline_class **buffer, int buf_size)
{
  return g1_editor_instance.get_current_splines(buffer, buf_size);
}

void g1_editor_class::create_radar()
{
  if (!radar_parent.get())
  {
    g1_map_class *map=g1_get_map();
    i4_parent_window_class *mv=g1_create_radar_view(map->width(), map->height(),
                                                    G1_RADAR_CLICK_HOLDS_VIEW |
                                                    G1_RADAR_DRAW_ALL_PATHS);



    i4_parent_window_class *p;
    int y=i4_current_app->get_window_manager()->height()-mv->height()-30;
    p=i4_current_app->get_style()->create_mp_window(0, y,
                                                    mv->width(), mv->height(),
                                                    g1_ges("radar_title"));
    p->add_child(0,0, mv);
    radar_parent=p;
  }  
}

int g1_editor_class::get_current_splines(i4_spline_class **buffer, int buf_size)
{
  if (!have_map() || !get_map()->current_movie || !get_map()->current_movie->t_cut_scenes)
    return 0;

  g1_cut_scene_class *s=get_map()->current_movie->current();

  if (s && buf_size>g1_cut_scene_class::T_PATHS)
  {
    for (int i=0; i<g1_cut_scene_class::T_PATHS; i++)
      buffer[i]=s->paths[i];

    return g1_cut_scene_class::T_PATHS;
  }

  return 0;
}




void g1_editor_class::create_views()
{
  if (edit_mode)
  {
    sw32 x1=0,
      x2=parent->width()-1-g1_edit_state.tools_width(),
      y1=style->font_hint->normal_font->largest_height()+8;

    int y2=parent->height()-1;
    if (time_line) y2-=time_line->height();

    w32 div;
    for (div=1; div*div<(w32)t_views; div++);

    sw32 x=x1, y=y1, w=(x2-x1+1)/div, h=(y2-y1+1)/div;


    for (w32 i=0; i<(w32)t_views; i++)
    {
      r1_expand_type expand=(r1_expand_type) g1_resources.render_window_expand_mode;

      view_wins[i]=g1_render.r_api->create_render_window(w,h, expand);
                                                                         
      views[i]=new g1_controller_edit_class(view_wins[i]->render_area_width(),
                                            view_wins[i]->render_area_height(), style);

      view_wins[i]->add_child(0,0,views[i]);
      parent->add_child_front((short)x,(short)y, view_wins[i]);

      if (i==1)
      {
        x=x1;
        y+=h;
      }
      else x+=w;

      views[i]->view=view_states[i];
      views[i]->view.suggest_camera_mode(G1_EDIT_MODE);
    }
    
    g1_current_controller=views[0];
  }
  else
    g1_cwin_man_class::create_views();
}

void g1_editor_class::destroy_views()
{
  if (edit_mode)
  {
    for (w32 i=0; i<(w32)t_views; i++)
    {
      views[i]->hide_focus();
      views[i]->view=view_states[i];

      delete view_wins[i];
      views[i]=0;
      view_wins[i]=0;
    }      


  } 
  //else 
  g1_cwin_man_class::destroy_views();
}
  



void g1_editor_class::close_windows()
{
  scene_edit.hide();

  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
    mc->cleanup();

  if (radar_parent.get())
    i4_kernel.delete_handler(radar_parent.get());
}




void g1_editor_class::set_edit_mode(i4_bool yes_no)
{
  if (yes_no)
  {
    // remove play save name so game doesn't try to load movie start
    i4_unlink(i4gets("play_savename"));
    i4_user_message_event_class movie_stop(G1_STOP_MOVIE);  // stop game movie if there was one
    i4_kernel.send_event(i4_current_app, &movie_stop);

    changed();
	li_call("Hide_Main_Menu");//hide the main menu if visible
    g1_cwin_man_class::destroy_views();   // kill the normal game views
    edit_mode=i4_T;

    g1_change_key_context(G1_EDIT_MODE);

    if (menu) delete menu;
    menu=li_create_pull_menu("scheme/menu.scm");
    menu->show(parent, 0,0);
	//this code would be ok, but if its nowhere re-enabled we run into trouble...
	//i4_menu_item_class *mn;
	//menu->get_sub_menu("File/Save",&mn,1);
	//mn->do_disable();
    
    time_line=new g1_time_line(parent, style, &g1_edit_state);

    //t_views=1;// set in constructor. Maintains last user setting.

    int th=parent->height();
    if (time_line) th-=time_line->height();
    g1_edit_state.show_tools(parent, th);
    
    create_views();    // create sup'd up edit controllers

    g1_change_key_context(G1_EDIT_MODE);

    li_call("ForcePause");  // stop the game from running while the editor is going

  } else
  {
    menu->hide();
    delete menu;
    menu=0;
    
    close_windows();

    destroy_views();

    edit_mode=i4_F;

    if (time_line)
      delete time_line;
	time_line=0;

    g1_edit_state.hide_tools();
    
    g1_cwin_man_class::create_views();   // create the normal game views

    li_call("Pause");     // start the game running again
  }
}


g1_editor_class::g1_editor_class()
{
  memset(&movement, 0, sizeof(movement));
  t_views=1;//show only 1 view by default
  for (int i=0; i<MAX_VIEWS; i++)
      {
      //must do it manually here, since some prerequisites
      //are not yet met when we first use this constructor. 
      //view_states[i].suggest_camera_mode(G1_EDIT_MODE);
      view_states[i].set_view_mode(G1_EDIT_MODE);
      //view_states[i].mode_changed=i4_T;
      }

  vert_noise_amount=3;
  use_view_state=i4_F;
  debug_view=0;
  lisp_interaction_view=0;

  delete_icon=0;

  tool_window=0;
  time_line=0;

  menu=0;

  edit_mode=i4_F;
  need_save=i4_F;
  can_undo=i4_F;
  can_redo=i4_F;
  selection=i4_F;
  profile_view=i4_F;
  paste_buffer_valid=i4_F;

  objects_window=0;
  views[0]=0;

  modal_window=0;

  g1_cwin_man=this;
}

void g1_editor_class::changed()
{
  need_save=i4_T;
}


void g1_editor_key_item::action()
{
  g1_editor_instance.do_command(command_id);
}

void g1_editor_class::save_as(const i4_const_str &fname)
{
  if (have_map())
  {
    get_map()->recalc_static_stuff();
    
    i4_bool restore_edit_mode=i4_T;
    
    g1_edit_state.hide_focus();
    get_map()->recalc_static_stuff();

    i4_file_class *out=i4_open(fname, I4_WRITE);
    if (out)
    {
      g1_saver_class *save=new g1_saver_class(out);

      get_map()->save(save, G1_MAP_ALL);

      if (save->begin_data_write())
        get_map()->save(save, G1_MAP_ALL);

      delete save;
    }
	else
		i4_message_box("Saving your map","Could not open the file for output. Permission denied?",MSG_OK);


    g1_edit_state.show_focus();

    need_save=i4_F;
  }
}

//save_as(get_map()->get_filename()) is same
void g1_editor_class::save()
{
  save_as(get_map()->get_filename());
  /*if (have_map())
  {
    get_map()->recalc_static_stuff();

    i4_bool restore_edit_mode=i4_T;
    
    g1_edit_state.hide_focus();
    get_map()->recalc_static_stuff();

    i4_file_class *out=i4_open(get_map()->get_filename(), I4_WRITE);
    if (out)
    {
      g1_saver_class *save=new g1_saver_class(out);

      get_map()->save(save, G1_MAP_ALL);

      if (save->begin_data_write())
        get_map()->save(save, G1_MAP_ALL);

      delete save;
    }
	else
		i4_message_box("Saving your map","Could not open the file for output. Permission denied?",MSG_OK);


    g1_edit_state.show_focus();

    need_save=i4_F;
  }*/
}

void g1_editor_class::do_command(w16 command_id)
{
  switch (command_id)
  {
    case G1_EDITOR_UNDO :
      do_undo();
      break;

    case G1_EDITOR_REDO :
      do_redo();
      break;
      
    case G1_EDITOR_NEW :
      open_new_level_window();
      break;

    case G1_EDITOR_SAVE ://save under the level filename
    {
      save_as(get_map()->get_filename());      
    } break;

	case G1_EDITOR_QUICKSAVE: 	//save under hardcoded quicksave filename
		{
		i4_filename_struct fsource,ftarget;

	//this (or other hack) is needed, as source may come from current dir
	//(usually \golgotha). The sprintf then generates a "/filename.scm"
	//source file, wich of course is incorrect.
		i4_str *srcpath=i4_full_path(get_map()->get_filename());
		i4_split_path(*srcpath,fsource);
		i4_split_path(i4_const_str("savegame/quicksave.level"),ftarget);
		i4_const_str fmt("%s/%s.scm");
		i4_str *src=fmt.sprintf(300,fsource.path,fsource.filename);
		i4_str *dst=fmt.sprintf(300,ftarget.path,ftarget.filename);
		if (!i4_copy_file(*src,*dst))
			{
			i4_error("ERROR: Could not copy SCM file. Source file corrupt? Permissions?");
			delete src;
			delete dst;
			delete srcpath;
			return;
			}
		delete src;
		delete dst;
		delete srcpath;
		save_as(i4_const_str("savegame/quicksave.level"));
		}
	break;

	case G1_EDITOR_QUICKLOAD://pass the request to the application
		{
		i4_file_status_struct fstat;
		if (!i4_get_status("savegame/quicksave.level",fstat))
			{
			i4_message_box("Quickload impossible","Quicksave file does not exist. Try quicksaving before quickloading.",MSG_OK);
			break;
			}
		i4_file_open_message_class o(G1_SAVEGAME_LOAD_OK, new i4_str("savegame/quicksave.level"));
		i4_kernel.send_event(i4_current_app, &o);
		}
	break;

    case G1_EDITOR_SAVEAS ://save under user-defined name, change name of map
    {
      open_saveas();
    } break;
      
	

	case G1_EDITOR_OPEN_ANIM:
		open_anim();
		break;
    case G1_EDITOR_LOAD :
      open_file();
      break;

    case G1_EDITOR_OPEN_DLL :
      open_dll();
      break;

    case G1_EDITOR_EXIT :
    {
      i4_user_message_event_class ex(G1_QUIT);
      i4_kernel.send_event(i4_current_app, &ex);
    } break;

    case G1_EDITOR_TOGGLE :
      set_edit_mode((i4_bool)!edit_mode);
      break;


    case G1_EDITOR_TILE_PICKER :
      if (g1_map_is_loaded())
        g1_e_tile.open_picker();
      break;


    case G1_EDITOR_WINDOW_RADAR :
      if (g1_map_is_loaded())
      {
        if (radar_parent.get())
          i4_current_app->get_style()->close_mp_window(radar_parent.get());
        else
          create_radar();
      }
      break;

    case G1_EDITOR_WINDOW_OBJECTS :
      if (g1_map_is_loaded())
        li_call("toggle_object_picker");

      break;


    
     case G1_EDITOR_AI_WINDOW:
     {
       if (!path_window)
       {
         enum { MAX_NUM=40 };
		 
         get_map()->get_critical_graph()->expand_critical_graph();
         i4_image_class *img[MAX_NUM];

         i4_const_str::iterator i1=get_editor_string("path_start").begin();
         img[0] = g1_edit_state.get_icon(i1.read_number());

         i1=get_editor_string("path_dest").begin();
         img[1] = g1_edit_state.get_icon(i1.read_number());

         i1=get_editor_string("path_critical").begin();
         img[2] = g1_edit_state.get_icon(i1.read_number());

         path_window = new g1_path_window_class(get_map(), img);
		 

         // Create MPWindow
         i4_parent_window_class *mpw;
         i4_user_message_event_class *close=
           new i4_user_message_event_class(G1_EDITOR_AI_WINDOW_CLOSED);
         mpw=style->
           create_mp_window(-1,-1,
                            path_window->width()+40,
							path_window->height(),
                            e_strs.get("ai_window_title"),
                            new i4_event_reaction_class(this, close));
         mpw->add_child(40,0, path_window);
		 
         // Create Toolbar
         i4_const_str *nums;
         i4_const_str *help_names[MAX_NUM];
         nums = get_editor_array("path_tool_win_icons");
         if (nums)
         {
           int n=0;
           for (i4_const_str* p=nums;  (!p->null() && n<MAX_NUM);  n++)
           {
             i4_const_str::iterator i1=p->begin();
             img[n] = g1_edit_state.get_icon(i1.read_number());
             ++p;
             help_names[n] = p;
             ++p;
           }

           g1_path_tool_window_class *path_tool = 
             new g1_path_tool_window_class(style, path_window, n, img, help_names );
           mpw->add_child(0,0, path_tool);
           mpw->resize_to_fit_children();

           i4_free(nums);
         }
       }
     } break;


    case G1_EDITOR_RESIZE_MAP :
      open_resize_level_window();
      break;

    case G1_EDITOR_WINDOW_SCENES :
      if (have_map() && get_map()->get_current_movie())
      {
        scene_edit.show(parent, get_map()->get_current_movie(), style);
      }
      break;

    case G1_EDITOR_SELECT_ALL_VERTS :
      select_all_verts();
      break;

    case G1_EDITOR_1_VIEW:
    {
      destroy_views();
      t_views=1;
      create_views();
    } break;

    case G1_EDITOR_TICK_MAP:
    {
      if (edit_mode && have_map())
      {
	    g1_player_man.think(); 
        get_map()->think_objects();
        g1_remove_man.process_requests();
        li_call("redraw");
      }
    } break;

    case G1_EDITOR_PROFILE:
    {
      sw32 x=0, y=0;
      i4_user_message_event_class *ue=new i4_user_message_event_class(G1_EDITOR_PROFILE_CLOSED);
      i4_event_reaction_class *prof_closed;
      prof_closed=new i4_event_reaction_class(this, ue);
      i4_profile_watch(style, parent, x,y, 200,280,!profile_view, prof_closed);
      profile_view=!profile_view;
    } break;

	case G1_EDITOR_LISP_EDIT:
		{
		sw32 x=50, y=70, w=400, h=300;
		if (lisp_interaction_view)
			{
			i4_lisp_interaction_window(style,parent,x,y,w,h,0,0);
			lisp_interaction_view=0;
			}
		else
			{
			i4_user_message_event_class *ue=new i4_user_message_event_class(G1_EDITOR_LISP_CLOSED);
			i4_event_reaction_class *lisp_closed;
			lisp_closed=new i4_event_reaction_class(this,ue);
			i4_lisp_interaction_window(style,parent,x,y,w,h,1,lisp_closed);
			lisp_interaction_view=1;
			}
		} break;

    case G1_EDITOR_DEBUG:
    {
      if (debug_view) 
		  {
		  g1_debug_close(style);
		  debug_view = 0;
		  }
      else 
		  {
		  i4_user_message_event_class *ue=
			  new i4_user_message_event_class(G1_EDITOR_DEBUG_CLOSED);
		  i4_event_reaction_class *debug_closed=
			  new i4_event_reaction_class(this, ue);
		  g1_debug_open(style, parent, g1_ges("debug_title"), 3, debug_closed);
		  debug_view=1;
		  }
    } break;

    case G1_EDITOR_4_VIEWS:
    {
      destroy_views();
      t_views=4;
      create_views();
    } break;

    case G1_EDITOR_RECALC_LIGHT :
//       if (map)
//         calc_map_lighting(0,0, get_map()->width(), get_map()->height());
      break;

    case G1_EDITOR_MERGE_TERRAIN :
      merge_terrain();
      break;

    case G1_EDITOR_FLATTEN_TERRAIN :
      flatten_terrain();
      break;

    case G1_EDITOR_SMOOTH_TERRAIN :
      smooth_terrain();
      break;

    case G1_EDITOR_NOISE_TERRAIN :
      noise_terrain();
      break;

    case G1_EDITOR_LOAD_HEIGHT_MAP :
      load_height_bitmap();
      break;

    case G1_EDITOR_SAVE_HEIGHT_MAP :
      save_height_bitmap();
      break;

    case G1_EDITOR_FOG_ALL :
      break;
      
    case G1_EDITOR_FOG_NONE :
      break;

    case G1_EDITOR_SET_SKY :
      create_sky_window();      // defined in editor/sky.cc
      break;

    case G1_EDITOR_SNAP_CENTER :
      g1_edit_state.snap=g1_edit_state_class::SNAP_CENTER;
      break;

    case G1_EDITOR_NO_SNAP :
      g1_edit_state.snap=g1_edit_state_class::NO_SNAP;
      break;

    case G1_EDITOR_SNAP_ORIGIN :
      g1_edit_state.snap=g1_edit_state_class::SNAP_ORIGIN;
      break;


  }
}

void g1_editor_class::close_modal()
{
  if (modal_window.get())
  {
    style->close_mp_window(modal_window.get());
    modal_window=0;
  }
}



char *cmd_2_enum[]=
{
  "File/New",                    // G1_EDITOR_NEW,
  "File/Save",                   // G1_EDITOR_SAVE,
  "File/Save As",                // G1_EDITOR_SAVEAS,
  "File/Open",                   // G1_EDITOR_LOAD,
  "",                            // G1_EDITOR_MERGE_TERRAIN,
  "File/Exit",                   // G1_EDITOR_EXIT,

  "Edit/Undo",                   // G1_EDITOR_UNDO,
  "Edit/Redo",                   // G1_EDITOR_REDO,
  "",                            // G1_EDITOR_CUT,
  "",                            // G1_EDITOR_COPY,
  "",                            // G1_EDITOR_PASTE,
  "Edit/Toggle Menu",            // G1_EDITOR_TOGGLE,
  "Edit/No Snap",                // G1_EDITOR_NO_SNAP,
  "Edit/Snap Cell Center",       // G1_EDITOR_SNAP_CENTER,
  "Edit/Snap Cell Origin",       // G1_EDITOR_SNAP_ORIGIN,


  "Tools/Objects",               // G1_EDITOR_WINDOW_OBJECTS,
  "Tools/Scenes",                // G1_EDITOR_WINDOW_SCENES,
  "Tools/Radar",                 // G1_EDITOR_WINDOW_RADAR,
  "View/1 View",                 // G1_EDITOR_1_VIEW,
  "View/4 Views",                // G1_EDITOR_4_VIEWS,
  "Tools/Tiles",                 // G1_EDITOR_TILE_PICKER,

  "",                            // G1_EDITOR_RECALC_LIGHT,
  "Terrain/Select All",          // G1_EDITOR_SELECT_ALL_VERTS,
  "Map/Resize",                  // G1_EDITOR_RESIZE_MAP,
  "Map/Change Sky",              // G1_EDITOR_SET_SKY,

  "Map/Fog Out",                 // G1_EDITOR_FOG_ALL,
  "Map/Unfog",                   // G1_EDITOR_FOG_NONE,

  "Map/Simulate Tick",           // G1_EDITOR_TICK_MAP,
  "Tools/Profile",               // G1_EDITOR_PROFILE,
  "Tools/Debug",                 // G1_EDITOR_DEBUG,
  "Tools/AI",                    // G1_EDITOR_AI_WINDOW,

  "Terrain/Load Image Heightmap",// G1_EDITOR_LOAD_HEIGHT_MAP,
  "Terrain/Save Image Heightmap",// G1_EDITOR_SAVE_HEIGHT_MAP,

  "Terrain/Flatten Selected",    // G1_EDITOR_FLATTEN_TERRAIN,
  "Terrain/Smooth Selected",     // G1_EDITOR_SMOOTH_TERRAIN,
  "Terrain/Add Noise to Selected",  // G1_EDITOR_NOISE_TERRAIN,

  "File/Open DLL",               // G1_EDITOR_OPEN_DLL,
  "File/Open Anim",				//G1_EDITOR_OPEN_ANIM,
  "Tools/Lisp Interaction",      //G1_EDITOR_LISP_EDIT
  "File/Quicksave",				//G1_EDITOR_QUICKSAVE
  "File/Quickload",				//G1_EDITOR_QUICKLOAD
  //direct lisp implementation for these
  //"load_from_transims",         //G1_EDITOR_TRANSIMS_LOAD
  //"save_for_transims",          //G1_EDITOR_TRANSIMS_SAVE
  0};

  

// these ID's are defined in mess_id.hh
void g1_editor_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    CAST_PTR(uev, i4_user_message_event_class, ev);
    switch (uev->sub_type)
    {            
      case G1_EDITOR_TERRAIN_NOISE_OK :
        noise_terrain_ok();
        break;


      case G1_EDITOR_SET_SKY_OK :
        set_sky();                // defined in editor/sky.cc
        break;

      case G1_TMERGE_FILE_OPEN_OK :
        merge_terrain_ok(uev);
        break;

      case G1_FILE_OPEN_OK :
        open_file_ok(uev);
        break;

      case G1_OPEN_DLL_OK :
        open_dll_ok(uev);
        break;

	  case G1_ANIM_OPEN_OK:
		  open_anim_ok(uev);
		  break;

      case G1_FILE_SAVEAS_OK :
        open_saveas_ok(uev);
        break;

      case G1_EDITOR_LOAD_TERRAIN_HEIGHT_OK :
        load_height_bitmap_ok(ev);
        break;

      case G1_EDITOR_SAVE_TERRAIN_HEIGHT_OK :
        save_height_bitmap_ok(ev);
        break;
          
      case G1_EDITOR_RESIZE_PLACE_LT :
      case G1_EDITOR_RESIZE_PLACE_CT :
      case G1_EDITOR_RESIZE_PLACE_RT :
      case G1_EDITOR_RESIZE_PLACE_LC :
      case G1_EDITOR_RESIZE_PLACE_CC :
      case G1_EDITOR_RESIZE_PLACE_RC :
      case G1_EDITOR_RESIZE_PLACE_BL :
      case G1_EDITOR_RESIZE_PLACE_BC :
      case G1_EDITOR_RESIZE_PLACE_BR :
        resize_dialog.orient=uev->sub_type-G1_EDITOR_RESIZE_PLACE_LT;
        break;


      case G1_EDITOR_RESIZE_MAP_OK :
        resize_level();
        break;

      case G1_EDITOR_NEW_OK :
        new_level_from_dialog();
        break;
        

      case G1_EDITOR_MODAL_BOX_CANCEL :
        close_modal();
        break;
      
      case G1_EDITOR_MODAL_CLOSED :
        modal_window=0;
        break;

        
      case G1_EDITOR_AI_WINDOW_CLOSED :
        path_window=0;
        break;
        
	  case G1_EDITOR_LISP_CLOSED://Nothing to do.
		  lisp_interaction_view=0;
		  break;
		
      case G1_EDITOR_PROFILE_CLOSED :
      {
        profile_view=0;
      } break;

      case G1_EDITOR_DEBUG_CLOSED :
      {
        debug_view=0;
      } break;
    }
  }
  else
  {      
    g1_cwin_man_class::receive_event(ev);
  }

}

i4_image_class *g1_editor_class::load_compatiable_image(const i4_const_str &fname)
{
  i4_image_class *im=i4_load_image(fname);
  if (im)
  {
    i4_draw_context_class c(0,0, im->width()-1, im->height()-1);

    i4_image_class *i=g1_render.r_api->create_compatible_image(im->width(), im->height());
    if (!i)
    {
      delete im;
      return 0;
    }

    im->put_image(i, 0,0, c);
    delete im;
    return i;     
  }
  return 0;
}


li_object *g1_old_pull_menu_command(li_object *o, li_environment *env)
{
  char *cmd_name=li_symbol::get(env->current_function(),env)->name()->value();
  
  int id=0;
  for (char **cmd=cmd_2_enum; *cmd; cmd++, id++)
    if (strcmp(*cmd, cmd_name)==0)
      g1_editor_instance.do_command(id);
  
  return 0;
}

void g1_editor_class::redraw_all()
{
  for (int i=0; i<t_views; i++)
    if (views[i])
      views[i]->request_redraw(i4_F);
}

li_object *g1_editor_redraw_all(li_object *o, li_environment *env)
{
  g1_editor_instance.redraw_all();
  return 0;
}

void g1_editor_class::init(i4_parent_window_class *_parent,
                           i4_graphical_style_class *_style,
                           i4_image_class *_root_image,
                           i4_display_class *display,
                           i4_window_manager_class *wm)

{ 
  li_add_function("redraw_all_views", g1_editor_redraw_all);

  for (char **cmd=cmd_2_enum; *cmd; cmd++)
    li_add_function(*cmd, g1_old_pull_menu_command);  
  
  g1_cwin_man_class::init(_parent, _style, _root_image, display, wm);

  e_strs.load(i4gets("editor_strs"));


  pick_act=load_compatiable_image(e_strs.get("pick_act"));
  pick_pass=load_compatiable_image(e_strs.get("pick_pass"));

  views[0]=0;

  menu=0;

  delete_icon=i4_load_image(e_strs.get("delete_icon"));  


  i4_bool determantistic=i4_F;
  for (int i=0; i<(int)i4_global_argc; i++)
    if (i4_global_argv[i]=="-deterministic")
      determantistic=i4_T;

  if (!determantistic)
    i4_init_gui_status(wm, display);

}

void g1_editor_class::uninit()
{
  close_modal();

  //i4_uninit_gui_status();

  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
    mc->cleanup();

  if (lisp_interaction_view)
	  {
	  sw32 a=0,b=0;
	  i4_lisp_interaction_window(style,0,a,
		  b,0,0,0,0);
	  lisp_interaction_view=0;
	  }

  if (debug_view) {
    g1_debug_close(style);
    debug_view = 0;
  }


  if (pick_act)
    delete pick_act;

  if (pick_pass)
    delete pick_pass;  

  delete menu; 
  menu=0;
  if (time_line) delete time_line;
  time_line=0;
  delete delete_icon;
   
  g1_edit_state.uninit();  
}

void g1_controller_edit_class::changed()
{
  g1_editor_instance.changed();   // so the user can save
}



void g1_editor_class::set_tool_window(i4_window_class **window)
{
  if (tool_window && tool_window!=window)
  {
    delete *tool_window;
    *tool_window=0;
    tool_window=0;
  }

  if (parent)
  {
    parent->add_child(parent->width()-(*window)->width(),
                      0,
                      (*window));
    tool_window=window;
  } else 
  {
    delete *window;
    window=0;
  }
}


void g1_controller_edit_class::display_edit_win()  // defined in editor.cc
{
  g1_editor_instance.set_tool_window(&edit_win);
}


void g1_controller_edit_class::move_selected_heights(sw32 z_change)
{
  g1_editor_instance.unmark_all_selected_verts_for_undo_save();
  g1_editor_instance.mark_selected_verts_for_undo_save();
  g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);


  w32 w=(get_map()->width()+1), h=(get_map()->height()+1);//,c;
  g1_map_vertex_class *v=get_map()->vertex(0,0);

  //Having a status window here is more anoying than helpfull 
  //because this is a short, but repeating operation
  //i4_status_class *status = i4_create_status(g1_ges("moving vertices"));

  for (sw32 vy=0; vy<(sw32)h; vy++)
  {
    //if (status)
    //  status->update(vy/(float)h);

    for (sw32 vx=0; vx<(sw32)w; vx++, v++)
    {
      if (v->is_selected())
      {
        w8 nh;

        if ((sw32)v->get_height_value()+z_change<0)
          nh=0;
        else if ((sw32)v->get_height_value()+z_change>255)
          nh=255;
        else
          nh=(w8)(v->get_height_value()+z_change);

        get_map()->change_vert_height(vx,vy, nh);
      }
    }
  }

  //if (status)
  //  delete status;

  changed();
  refresh();
}


void g1_controller_edit_class::change_light_direction()
{
  if (g1_map_is_loaded())
  {
    g1_editor_instance.add_undo(G1_MAP_VERTS | G1_MAP_LIGHTS);


    i4_3d_vector one_z(0,0,-1), center, light;
    transform.transform(one_z, light);
    transform.transform(i4_3d_vector(0,0,0), center);
    light-=center;



    changed();
    refresh();
  }  
}

void g1_editor_class::select_all_verts()
{
  add_undo(G1_MAP_VERTS);

  sw32 w=get_map()->width(), h=get_map()->height();

  for (int j=0; j<=h; j++)
    for (int i=0; i<=w; i++)
      get_map()->vertex(i,j)->set_is_selected(i4_T);

  changed();
  li_call("redraw");
}

void g1_editor_class::open_anim()
	{
	i4_str *start_dir=0;
	if (!start_dir)
    start_dir=new i4_str(get_editor_string("open_anim_start_dir"));
	//TODO: start_dir gets leaked (but mustn't delete bellow, since it's copied to an i4_const_str)
	i4_create_file_open_dialog(style,
		get_editor_string("open_anim_title"),
		*start_dir,
		get_editor_string("open_anim_file_mask"),
		get_editor_string("open_anim_mask_name"),
		this,
		G1_ANIM_OPEN_OK,
		G1_FILE_OPEN_CANCEL);
	}

void g1_editor_class::open_file()
{
  i4_str *start_dir=0;

  if (g1_map_is_loaded() && !g1_get_map()->get_filename().null())
  {
    i4_filename_struct fn;
    i4_split_path(g1_get_map()->get_filename(), fn);

    if (fn.path[0])
      start_dir=new i4_str(fn.path);
  }

  if (!start_dir)
    start_dir=new i4_str(get_editor_string("open_start_dir"));

  i4_create_file_open_dialog(style,
                             get_editor_string("open_title"),
                             *start_dir,
                             get_editor_string("open_file_mask"),
                             get_editor_string("open_mask_name"),
                             this,
                             G1_FILE_OPEN_OK,
                             G1_FILE_OPEN_CANCEL);

  delete start_dir;
}

void g1_editor_class::open_saveas()
{
  if (have_map())
  {
    i4_create_file_save_dialog(style,
                               get_map()->get_filename(),
                               get_editor_string("saveas_title"),
                               get_editor_string("open_start_dir"),
                               get_editor_string("open_file_mask"),
                               get_editor_string("open_mask_name"),
                               this,
                               G1_FILE_SAVEAS_OK,
                               G1_FILE_OPEN_CANCEL);
    
  }
}



void g1_editor_class::open_dll() 
{
  i4_create_file_open_dialog(style,
                             get_editor_string("open_dll_title"),
                             i4_const_str(i4_dll_dir),
                             get_editor_string("open_dll_file_mask"),
                             get_editor_string("open_dll_mask_name"),
                             this,
                             G1_OPEN_DLL_OK,
                             G1_FILE_OPEN_CANCEL);
}


void g1_editor_class::open_anim_ok(i4_user_message_event_class *ev)
	{
	CAST_PTR(f, i4_file_open_message_class, ev);
	char tmp[MAX_PATH];
	i4_os_string(*f->filename,tmp,MAX_PATH);
	li_load(tmp);
	}

void g1_editor_class::open_file_ok(i4_user_message_event_class *ev)
{
  CAST_PTR(f, i4_file_open_message_class, ev);

  g1_load_level(*f->filename, 1, 0);

}

void g1_editor_class::open_dll_ok(i4_user_message_event_class *ev)
{
  CAST_PTR(f, i4_file_open_message_class, ev);
  if (f->filename)
    i4_dll_man.load(*f->filename);
}

void g1_editor_class::open_saveas_ok(i4_user_message_event_class *ev)
{
  if (have_map())
  {

    CAST_PTR(f, i4_file_open_message_class, ev);
	i4_filename_struct fsource,ftarget;

	//this (or other hack) is needed, as source may come from current dir
	//(usually \golgotha). The sprintf then generates a "/filename.scm"
	//source file, wich of course is incorrect.
	i4_str *srcpath=i4_full_path(get_map()->get_filename());
	i4_split_path(*srcpath,fsource);
	i4_split_path(*f->filename,ftarget);
	i4_const_str fmt("%s/%s.scm");
	i4_str *src=fmt.sprintf(300,fsource.path,fsource.filename);
	i4_str *dst=fmt.sprintf(300,ftarget.path,ftarget.filename);
	if (!i4_copy_file(*src,*dst))
		{
		i4_error("ERROR: Could not copy SCM file. Source file corrupt? Permissions?");
		delete src;
		delete dst;
		delete srcpath;
		return;
		}
	delete src;
	delete dst;
	delete srcpath;
    get_map()->set_filename(*f->filename);
    save_as(get_map()->get_filename());    
  }
}


i4_parent_window_class *g1_editor_class::create_modal(w32 w, w32 h, char *title_res)
{
  close_modal();

  i4_object_message_event_class *closed_modal;
  closed_modal=new i4_object_message_event_class(this, G1_EDITOR_MODAL_CLOSED);


  modal_window=style->create_modal_window(-1, -1, (w16)w, (w16)h,
                                       g1_ges(title_res), true,
                                       new i4_event_reaction_class(this, closed_modal));

  return modal_window.get();
}


void g1_editor_class::mark_selected_verts_for_undo_save()
{
  if (have_map())
  {
    int t=(get_map()->width()+1) * (get_map()->height()+1);
    g1_map_vertex_class *v=get_map()->verts;

    for (int i=0; i<t; i++, v++)
      if (v->is_selected())
        v->set_need_undo(i4_T);
  }
}


void g1_editor_class::unmark_all_selected_verts_for_undo_save()
{
  int t=(get_map()->width()+1) * (get_map()->height()+1);
  g1_map_vertex_class *v=get_map()->verts;

  for (int i=0; i<t; i++, v++)
    v->set_need_undo(i4_F);
}




void g1_editor_class::save_views(g1_saver_class *fp)
{
  g1_cwin_man_class::save_views(fp);

  for (int i=0; i<t_views; i++)
  {
    char section_name[50];
    sprintf(section_name, "editor view %d v1", i);
    fp->mark_section(section_name);

    if (views[i])
      view_states[i]=views[i]->view;

    view_states[i].save(fp);
  }
}

void g1_editor_class::load_views(g1_loader_class *fp)
{
  g1_cwin_man_class::load_views(fp);

  if (!fp)
  {
    for (int i=0; i<MAX_VIEWS; i++)
    {
      if (views[i])
        views[i]->view.load(0);
      else
        view_states[i].load(0);
    }
  }
  else
  {
    for (int i=0; ; i++)
    {
      char section_name[50];
      sprintf(section_name, "editor view %d v1", i);
      if (!fp->goto_section(section_name))
        return ;

      view_states[i].load(fp);
      if (views[i])
        views[i]->view=view_states[i];
    }
  }
}


const i4_const_str &g1_ges(char *res)
{ 
  return g1_editor_instance.get_editor_string(res); 
}

g1_controller_edit_class *g1_editor_class::get_current_view()
{
  for (int i=0; i<MAX_VIEWS; i++)
    if (views[i]==g1_current_controller.get())
      return views[i];
 
  return 0;  
}

// ce_event.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



void g1_controller_edit_class::receive_event(i4_event *ev)
{

  

  if (!g1_map_is_loaded()) 
	  {
	  g1_object_controller_class::receive_event(ev);
	  return ;
	  }
  if (!active())
  {
    g1_object_controller_class::receive_event(ev);
    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      if (wev->sub_type==i4_window_message_class::LOST_KEYBOARD_FOCUS)
        unfocused();
      return;
    }              
    else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)      
    {
      focused();
      g1_current_controller=this;

      request_redraw();
      setup_context();
      if (!mode->pass_through_focus_click())
        return;
    }
    else
    {
      if (ev->type()==i4_event::MOUSE_MOVE)
      {
        CAST_PTR(mev, i4_mouse_move_event_class, ev);
        last_mouse_x=mev->x;
        last_mouse_y=mev->y;

        if (abs((sw32)last_mouse_x-(sw32)g1_edit_state.context_help.mx)>15 ||
            abs((sw32)last_mouse_y-(sw32)g1_edit_state.context_help.my)>15)
          g1_edit_state.context_help.hide();

      }

      update_cursor();
      return;
    }
  }


  switch (ev->type())
  {
    case i4_event::IDLE_MESSAGE :
    {
      if (active())
        mode->idle();
    } break;


    case i4_event::MOUSE_MOVE :
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
      setup_context();
      mode->mouse_move(mev->x, mev->y);
      //last_mouse_x, y set by parent class

      if (abs((sw32)last_mouse_x-(sw32)g1_edit_state.context_help.mx)>15 ||
          abs((sw32)last_mouse_y-(sw32)g1_edit_state.context_help.my)>15)
        g1_edit_state.context_help.hide();

    } break;
    
    case i4_event::MOUSE_BUTTON_DOWN :
    {
      CAST_PTR(bev, i4_mouse_button_down_event_class, ev);
      if (bev->left())
      {
        setup_context();
        mode->mouse_down();
      }
    } break;

    case i4_event::MOUSE_BUTTON_UP :
    {
      CAST_PTR(bev, i4_mouse_button_up_event_class, ev);
      if (bev->left())
      {
        setup_context();
        mode->mouse_up();
      }
    } break;

    case i4_event::KEY_PRESS :
    {
      CAST_PTR(kev, i4_key_press_event_class, ev);
      setup_context();
      mode->key_press(kev);
    } break;

    case i4_event::WINDOW_MESSAGE :
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      switch (wev->sub_type)
      {
        case i4_window_message_class::LOST_KEYBOARD_FOCUS :
          unfocused();
          break;

        case i4_window_message_class::LOST_MOUSE_FOCUS :
          mode->hide_focus();
          break;

        case i4_window_message_class::GOT_MOUSE_FOCUS :
          mode->show_focus();
          break;

      }
    } break;

 
  }
  g1_object_controller_class::receive_event(ev);
}

// ce_movie.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



i4_spline_class::point *g1_controller_edit_class::find_spline_point(sw32 mx, sw32 my, 
                                                                    w32 instance)
{
  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);


  r1_vert rv;
  i4_spline_class::point *first=0, *last=0;


  for (w32 i=0; i<(w32)t; i++)
  {
    i4_spline_class::point *c=s[i]->begin();
    for (;c;c=c->next)
    {
      if (g1_render.project_point(i4_3d_point_class(c->x, c->y, c->z),
                                  rv,
                                  g1_context.transform))
      {
        if (abs((sw32)rv.px-mx)<3 && abs((sw32)rv.py-my)<3)
        {
          if (instance)
            instance--;
          else
            return c;
        }
      }          
    }        
  }
  return 0;
}


void g1_controller_edit_class::clear_selected_points()
{
  i4_bool change=i4_F;

  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);

  i4_spline_class::point *c;

  for (int i=0; i<t; i++)
    for (c=s[i]->begin(); c; c=c->next) 
      if (c->selected)
      {
        c->selected=i4_F;
        change=i4_T;
      }
  
  if (change)
    changed();
}

void g1_controller_edit_class::select_points_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                                     g1_mode_handler::select_modifier)
{
/*
  clear_selected_points();

  if (!map->current_movie)
    return ;

  g1_movie_flow_class *flow=map->current_movie;

  sw32 dx1,dy1,dx2,dy2;
  drag_area(dx1,dy1,dx2,dy2);
  i4_bool change=i4_F;

  float x,y;
  i4_spline_class::point *first=0, *last=0;
  

  if (flow && flow->current())
  {
    for (w32 i=0; i<2; i++)
    {
      i4_spline_class::point *c;
      if (i==0) 
        c=flow->current()->camera.begin();
      else
        c=flow->current()->target.begin();

      for (;c;c=c->next)
      {
        if (project_point(i4_3d_point_class(c->x, c->y, c->z),
                          x,y,
                          &g1_context))
        {
          if (x>=dx1 && y>=dy1 && x<=dx2 && y<=dy2)
          {
            if (!c->selected)
            {
              c->selected=i4_T;
              change=i4_T;
            }
          }
        }
      }
    }
  }
 
  if (change)
  {
    refresh();  
    changed();
  }*/
}


void g1_controller_edit_class::add_movie_control_point(int list_number)
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  //w32 list_number_2;
  int t=get_current_splines(s,MAX_SPLINE_EDIT);
  
  i4_float h=0;
  if (list_number == g1_cut_scene_class::CAMERA)
    h=2;
  else if (list_number == g1_cut_scene_class::TARGET)
    h=0.2f;

  if (list_number<t)
  {    
    i4_float gx,gy, dx,dy;
    if (!view_to_game(last_mouse_x, last_mouse_y, gx,gy, dx,dy))
      return;

    h+=get_map()->terrain_height(gx, gy);

#if 0 //movie hack that no longer works
    if (g1_input.button_1() && list_number==g1_cut_scene_class::CAMERA)
    {
      
      list_number_2 = g1_cut_scene_class::TARGET;

      g1_player_piece_class *p;
      p=(g1_player_piece_class *)get_map()->find_object_by_id(g1_supertank_type, 
                                                              g1_default_player);
      gx = p->x;
      gy = p->y;
      h  = p->h;

      i4_3d_point_class view_dir,view_adj;

      i4_transform_class tank_trans,tmp;
      tank_trans.identity();      
      
      p->cam_pitch = p->groundpitch;//*sin(p->base_angle) + p->groundroll*cos(p->base_angle);
      p->cam_roll  = p->groundroll;// *sin(p->base_angle) - p->groundpitch*cos(p->base_angle);
      
      tmp.rotate_z((p->base_angle));
      tank_trans.multiply(tmp);

      tmp.rotate_y((p->cam_pitch));
      tank_trans.multiply(tmp);

      tmp.rotate_x((p->cam_roll));
      tank_trans.multiply(tmp);

      tank_trans.transform(i4_3d_point_class(1,0,0),view_dir);
      tank_trans.transform(i4_3d_point_class(0.25,0.023,0.15),view_adj);
      
      gx += view_adj.x;
      gy += view_adj.y;
      h  += view_adj.z;

      view_dir.x += gx;
      view_dir.y += gy;
      view_dir.z += h;

      if (s[list_number_2]->total())
      {
        w32 last_time=s[list_number_2]->get_control_point(s[list_number_2]->total()-1)->frame;
        s[list_number_2]->add_control_point(view_dir.x, view_dir.y, view_dir.z, last_time + G1_MOVIE_HZ);
      }
      else
        s[list_number_2]->add_control_point(view_dir.x, view_dir.y, view_dir.z, 0);
    }
#endif

    if (s[list_number]->total())
    {
      w32 last_time=s[list_number]->get_control_point(s[list_number]->total()-1)->frame;
      s[list_number]->add_control_point(gx, gy, h, last_time + G1_MOVIE_HZ);
    }
    else
      s[list_number]->add_control_point(gx, gy, h, 0);

    refresh();
    changed();
  }
}

void g1_controller_edit_class::delete_selected_points()
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);
  for (int i=0; i<t; i++)    
    s[i]->delete_selected();

  refresh();
  changed();
}

void g1_controller_edit_class::insert_control_points()
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);

  for (int i=0; i<t; i++)    
    s[i]->insert_control_points();

  refresh();
  changed();
}

void g1_controller_edit_class::move_selected_points(i4_float xa, i4_float ya, i4_float za)
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);

  for (int i=0; i<t; i++)
  {
    i4_spline_class::point *c=s[i]->begin();

    for (;c;c=c->next)
    {
      if (c->selected)
      {
        c->x+=xa;
        c->y+=ya;
        c->z+=za;
      }
    }

    changed();
    refresh();  
  }  
}

// contedit.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



void g1_controller_edit_class::update_cursor() 
{ 
  if (!active())
  {
    if (cursor_state!=G1_X_CURSOR)
    {
      cursor_state=G1_X_CURSOR;
      set_cursor(g1_resources.big_cursors[cursor_state]);
    }
  }
  else if (mode) 
    mode->update_cursor(); 
}



void g1_controller_edit_class::setup_context()
{  
  g1_context.window_setup(0,0,width(), height(),
                          g1_render.center_x, g1_render.center_y,
                          g1_render.scale_x, g1_render.scale_y,
                          g1_render.ooscale_x, g1_render.ooscale_y);
}

void g1_controller_edit_class::setup_mode()
{
  if (mode && active())
    mode->hide_focus();

  if (mode)
  {
    delete mode;   
  }

  mode=0;


  char buf[250];
  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
  {
	  mc->name(buf);
    if (strcmp(g1_edit_state.major_mode, buf)==0)
	{
      mode=mc->create_mode_handler(this);
	}
  }

  if (mode && active())
    mode->show_focus();

  refresh();


}

void g1_controller_edit_class::show_focus()
{
  if (!focus_visible)
  {
    mode->show_focus();
    focus_visible=i4_T;
  }
}

void g1_controller_edit_class::hide_focus()
{
  if (focus_visible)
  {
    mode->hide_focus();
    focus_visible=i4_F;
  }
}

void g1_controller_edit_class::refresh()
{
  i4_user_message_event_class ch(G1_MAP_CHANGED);
  i4_kernel.send_event(i4_current_app, &ch);
}

g1_controller_edit_class::~g1_controller_edit_class()
{  
  g1_edit_state.context_help.hide();

  if (active())
    unfocused();

  if (mode)
  {
    delete mode;   
    mode=0;
  }
}


void g1_controller_edit_class::unfocused()
{
  hide_focus();

  update_cursor();

  if (g1_current_controller.get()==this)
    g1_current_controller=0;
  

  request_redraw();
}

void g1_controller_edit_class::focused()
{
  setup_mode();

  g1_current_controller=this;
  
  i4_window_request_key_grab_class kgrab(this);
  i4_kernel.send_event(parent, &kgrab);

  show_focus();
  request_redraw();

  cursor_state=G1_DEFAULT_CURSOR;
  set_cursor(g1_resources.big_cursors[cursor_state]);
}

void g1_replace_cell(g1_map_class *map,
                     g1_map_cell_class &original, 
                     g1_map_cell_class &new_cell)
{
  g1_map_cell_class old;

  while (original.get_obj_list())
  {
    g1_object_class *o=original.object_list->object;
    o->unoccupy_location();

    old=original;
    original=new_cell;
    o->occupy_location();
    new_cell=original;
    original=old;        
  }

  old=original;
  original=new_cell;
  new_cell=old;
  
  I4_ASSERT(new_cell.object_list==0, "new cell list should be null");

}

void g1_controller_edit_class::replace_cell(g1_map_class *map,
                                            g1_map_cell_class &original, 
                                            g1_map_cell_class &new_cell)
{
  g1_replace_cell(map, original, new_cell);
}

void g1_controller_edit_class::restore_cell()
{
  if (need_restore_cell)
  {
    if (g1_map_is_loaded())
    {
      replace_cell(get_map(),
                   *get_map()->cell((w16)cell_x, (w16)cell_y),
                   saved_cell);

      need_restore_cell=i4_F;

      li_call("redraw");
    }
  }
}

void g1_controller_edit_class::save_and_put_cell()
{
  if (g1_map_is_loaded())
  {
    saved_cell=*get_map()->cell((w16)cell_x, (w16)cell_y);
    saved_cell.object_list=0;                    // don't keep the object list

    g1_map_cell_class new_cell;
    new_cell.init(g1_e_tile.get_cell_type(), 
                  g1_e_tile.get_cell_rotation(),
                  g1_e_tile.get_mirrored());

    replace_cell(get_map(), *get_map()->cell((w16)cell_x, (w16)cell_y), new_cell);
  
    need_restore_cell=i4_T;

    li_call("redraw");
  }
}

class g1_map_fill_class : public g1_flood_fill_class
{

public:
  g1_map_class *map;
  g1_map_class *get_map() { return map; }
  w16 type, block_type;
  g1_rotation_type rotation;
  g1_player_type owner;
  i4_bool mirror;

  virtual void get_clip(sw32 &x1, sw32 &y1, sw32 &x2, sw32 &y2)
  {
    x1=y1=0;
    x2=get_map()->width()-1;
    y2=get_map()->height()-1;
  }

  i4_bool blocking(sw32 x, sw32 y)
  {
    w16 on_type=get_map()->cell((w16)x,(w16)y)->type;
    if (on_type==block_type)
      return i4_F;
    else return i4_T;
  }

  void fill_block(sw32 x, sw32 y, sw32 startx, sw32 starty)
  { 
    g1_map_cell_class c;
    c.init(type, rotation, mirror);

    g1_replace_cell(map, *get_map()->cell((w16)x,(w16)y), c);
  }

  g1_map_fill_class(g1_map_class *map, w16 type, 
                    g1_rotation_type rotation, 
                    i4_bool mirror,
                    g1_player_type owner,
                    w16 block_type)
    : map(map), type(type), rotation(rotation), 
      mirror(mirror),
      block_type(block_type), owner(owner)
  {}
};

void g1_controller_edit_class::fill_map()
{
  if (g1_map_is_loaded())
  {
    g1_editor_instance.add_undo(G1_MAP_CELLS);

    if (active())
      hide_focus();    


    w32 type_on=get_map()->cell((w16)cell_x, (w16)cell_y)->type;

    if (type_on!=g1_e_tile.get_cell_type())
    {
      g1_map_fill_class f(get_map(), 
                          g1_e_tile.get_cell_type(),
                          g1_e_tile.get_cell_rotation(),
                          g1_e_tile.get_mirrored(),
                          g1_edit_state.current_team,
                          (w16)type_on);

      f.fill(cell_x, cell_y);
    }


    if (active())
      show_focus();

    changed();
  }
}

void g1_controller_edit_class::change_map_cell()
{
  g1_editor_instance.add_undo(G1_MAP_CELLS);

  graph_changed=i4_T;
  saved_cell.init(g1_e_tile.get_cell_type(), 
                  g1_e_tile.get_cell_rotation(),
                  g1_e_tile.get_mirrored());
  changed();
}


void g1_controller_edit_class::draw_3d_line(const i4_3d_point_class &p1,
                                            const i4_3d_point_class &p2,
                                            i4_color color1,
                                            i4_color color2,
                                            i4_image_class *local_image,
                                            g1_draw_context_class *context)
{
  g1_render.render_3d_line(p1, p2, color1, color2, context->transform);
}


void g1_controller_edit_class::draw_3d_point(sw32 w,
                                             i4_color color,
                                             const i4_3d_point_class &p,
                                             i4_image_class *local_image,
                                             g1_draw_context_class *context)
{
  r1_vert v;
  if (g1_render.project_point(p, v, context->transform))
    r1_clip_clear_area((sw32)v.px-w, (sw32)v.py-w, 
                       (sw32)v.px+w, (sw32)v.py+w, color, v.v.z, *context->context,
                       g1_render.r_api);
}


void g1_controller_edit_class::draw_spline(i4_image_class *local_image,
                                           g1_draw_context_class *g1_context, 
                                           i4_color cpoint_color,
                                           i4_color line_color,
                                           i4_color spline_color,
                                           i4_spline_class *s,
                                           w32 cur_frame)
{
  sw32 i;
  //i4_float lx,ly,x,y,z;
  if (!s->total()) return;

  i4_spline_class::point *p;    
  i4_3d_point_class cur_p, last_p;

  for (i=0; i<(sw32)s->total(); i++)
  {
    p=s->get_control_point(i);     

    cur_p=i4_3d_point_class(p->x, p->y, p->z);

    if (i!=0)
      draw_3d_line(cur_p, last_p, line_color, line_color, local_image, g1_context);
    
    last_p=cur_p;
  }


  p=s->get_control_point(0);

  w32 frame=0;

  while (p->next)
  {
    while (frame<p->next->frame)
    {
      i4_float x1,y1,z1,x2,y2,z2;

      i4_3d_point_class q1,q2;
      s->get_point(frame, x1,y1,z1);
          

      if (g1_e_camera.should_show_frames())  
        draw_3d_point(1, 0x0000ff, i4_3d_point_class(x1,y1,z1), local_image, g1_context);

      if (frame==cur_frame)
        draw_3d_point(1, 0xff00ff, i4_3d_point_class(x1,y1,z1), local_image, g1_context);

      frame++;

      s->get_point(frame, x2,y2,z2);
      draw_3d_line(i4_3d_point_class(x1,y1,z1), 
                   i4_3d_point_class(x2,y2,z2),
                   spline_color, spline_color,
                   local_image, g1_context);


    }
    p=p->next;
  }

  for (i=0; i<(sw32)s->total(); i++)
  {
    i4_3d_point_class q;

    i4_spline_class::point *p=s->get_control_point(i);

    if (p->selected)
      draw_3d_point(1, 0xffff00,
                    i4_3d_point_class(p->x,p->y,p->z), local_image, g1_context);
    else
      draw_3d_point(1, 
                    cpoint_color,
                    i4_3d_point_class(p->x,p->y,p->z), local_image, g1_context);
  }


}

void g1_controller_edit_class_tile_post_cell_draw(sw32 x, sw32 y, void *context)
{
  ((g1_controller_edit_class *)context)->tile_cell_draw(x,y);
}


void g1_controller_edit_class_object_post_cell_draw(sw32 x, sw32 y, void *context)
{
  ((g1_controller_edit_class *)context)->object_cell_draw(x,y);
}

void g1_controller_edit_class::object_cell_draw(sw32 x, sw32 y)
{
  g1_map_class *map=get_map();

   int bgrade=-1;
   //code for old maps only
   for (int j=0; j<G1_GRADE_LEVELS; j++)
   {
     g1_block_map_class *bmap=map->get_block_map(j);
     if (bmap && bmap->is_blocked((w16)x,(w16)y, G1_NORTH | G1_SOUTH | G1_WEST | G1_EAST))
       bgrade=j;
   }
  //end of old code
  if ((map->cell((w16)x,(w16)y)->flags & g1_map_cell_class::IS_GROUND)==0)
    bgrade=0;

  if (map->cell((w16)x,(w16)y)->get_solid_list())
    bgrade=1;
  else
    bgrade=-1;

  if (bgrade!=-1)
  {
    w32 c[5]={0xff0000, 0x00ff00, 0x0000ff, 0xffff00, 0xff00ff };

    float r=0.01f;
    draw_3d_line(i4_3d_point_class((float)x,(float)y, map->terrain_height((float)x,(float)y)+r),
                 i4_3d_point_class((float)(x+1),(float)(y+1), map->terrain_height((float)(x+1),(float)(y+1))+r),
                 c[bgrade], c[bgrade], local_image, &g1_context);

    draw_3d_line(i4_3d_point_class((float)(x+1),(float)y, map->terrain_height((float)(x+1),(float)y)+r),
                 i4_3d_point_class((float)x,(float)(y+1), map->terrain_height((float)x,(float)(y+1))+r),
                 c[bgrade], c[bgrade], local_image, &g1_context);
  }

}

void g1_controller_edit_class::tile_cell_draw(sw32 x, sw32 y)
{
  r1_vert rv;
  g1_map_class *map=get_map();

  g1_map_vertex_class *v[4];
  v[0]=map->vertex(x,y);
  v[1]=v[0]+1;
  v[2]=v[0]+map->width()+1;
  v[3]=v[0]+map->width()+1+1;

  int vt[8]={x,y,x+1,y,x,y+1,x+1,y+1};

  object_cell_draw(x,y);

  for (w32 i=0; i<4; i++)
  {
    if (v[i]->is_selected())
    { 
      i4_3d_point_class cp((float)vt[i*2],(float)vt[i*2+1],v[i]->get_height());

      if (g1_render.project_point(cp, rv, g1_context.transform))
        r1_clip_clear_area((sw32)rv.px-1, (sw32)rv.py-1, (sw32)rv.px+1, (sw32)rv.py+1, 
                           0xffff00, rv.v.z, *g1_context.context,
                           g1_render.r_api);
    }

    v[i]->set_flag(g1_map_vertex_class::WAS_DRAWN_LAST_FRAME,1);
  }

}

void g1_controller_edit_class::editor_pre_draw(i4_draw_context_class &context)
{
  g1_render.frame_ratio=0;

  if (g1_map_is_loaded() && active())
  {
    int w=(get_map()->width()+1) * (get_map()->height()+1);
    g1_map_vertex_class *v=get_map()->vertex(0,0);

    for (int i=0; i<w; i++, v++)
      v->set_flag(g1_map_vertex_class::WAS_DRAWN_LAST_FRAME, 0);
  }

  if (strcmp(g1_edit_state.major_mode,"TILE")==0 && get_map())
    get_map()->set_post_cell_draw_function(g1_controller_edit_class_tile_post_cell_draw, this);

  if (strcmp(g1_edit_state.major_mode,"OBJECT")==0 && get_map())
    get_map()->set_post_cell_draw_function(g1_controller_edit_class_object_post_cell_draw, this);


  if (g1_resources.paused)
    g1_context.draw_editor_stuff=i4_T;

  if (view.get_view_mode()==G1_EDIT_MODE)
  {
    g1_render.r_api->set_write_mode(R1_WRITE_COLOR | R1_WRITE_W);
    r1_clip_clear_area(0,0,width()-1, height()-1, 0, r1_far_clip_z, context, g1_render.r_api);
    g1_render.r_api->set_write_mode(R1_WRITE_COLOR | R1_WRITE_W | R1_WRITE_COLOR);
  }

}

void g1_controller_edit_class::editor_post_draw(i4_draw_context_class &context)
{

  g1_context.draw_editor_stuff=i4_F;

  if (g1_map_is_loaded())
    get_map()->set_post_cell_draw_function(0,0);

  mode->post_draw(context);

  if (active())
    g1_render.draw_rectangle(0,0, width()-1, height()-1, 0xffff00, context);
  else
    g1_render.draw_rectangle(0,0, width()-1, height()-1, 0, context);
}



void g1_controller_edit_class::process_input(i4_time_class tick_time)
{
  /*  if (have_focus)
  {
    if (!get_map()) return;
   
    i4_angle a=camera.ground_rotate, move_speed=1.0, pan_speed=0.2;

    if (camera.gx>=0 && camera.gy>=0 && 
        camera.gx<get_map()->width() && camera.gy<get_map()->height())
    {
      i4_float th=get_map()->terrain_height(camera.gx, camera.gy);
      i4_float camera_dist=camera.gz-th;

      if (camera_dist<6)
      {
        pan_speed*=camera_dist/6.0;
        move_speed*=camera_dist/6.0;
      }
    }



    if (g1_editor_instance.movement.rotate_left)
      rotate(0.08, 0);

    if (g1_editor_instance.movement.rotate_right)
      rotate(-0.08, 0);

    if (g1_editor_instance.movement.rotate_up)
      rotate(0, -0.08);

    if (g1_editor_instance.movement.rotate_down)
      rotate(0, 0.08);


    if (g1_editor_instance.movement.pan_down)
      pan(0,0,-pan_speed);

    if (g1_editor_instance.movement.pan_up)
      pan(0,0,pan_speed);

    if (g1_editor_instance.movement.pan_left)
      pan(-sin(a)*move_speed, cos(a)*move_speed,0);

    if (g1_editor_instance.movement.pan_right)
      pan(sin(a)*move_speed, -cos(a)*move_speed,0);

    if (g1_editor_instance.movement.pan_forward)
      pan(cos(a)*move_speed, sin(a)*move_speed,0);

    if (g1_editor_instance.movement.pan_backward)
      pan(-cos(a)*move_speed, -sin(a)*move_speed,0);


      } */
}

g1_map_vertex_class *g1_controller_edit_class::find_map_vertex(sw32 x, sw32 y,
                                                               sw32 &vx, sw32 &vy)
{
  sw32 sx,sy,ex,ey, tx,ty;
  sx=(sw32)0;  sy=(sw32)0;
  ex=(sw32)get_map()->width();  ey=(sw32)get_map()->height();

  g1_map_vertex_class *mv=get_map()->vertex(0,0);

  if (!g1_context.transform)
    return 0;

  // transform all the points in area and test proximity to mouse x,y
  for (ty=sy; ty<=ey; ty++)
    for (tx=sx; tx<=ex; tx++)
    {
      if (mv->get_flag(g1_map_vertex_class::WAS_DRAWN_LAST_FRAME))
      {
        r1_vert v;

        if (g1_render.project_point(i4_3d_point_class((float)tx, (float)ty, mv->get_height()), 
                                    v, g1_context.transform))
        {
          if (abs((sw32)v.px-x)<=3 && abs((sw32)v.py-y)<=3)
          {
            vx=tx; vy=ty;
            return mv;
          }
        }      
      }
      mv++;
    }

  return 0;
}




void g1_controller_edit_class::clear_selected_verts()
{
  if (!i4_current_app->get_window_manager()->control_pressed())
  {
    i4_bool change=i4_F;

    sw32 w=(get_map()->width()+1)*(get_map()->height()+1);
    g1_map_vertex_class *v=get_map()->vertex(0,0);


    for (w32 x=0; x<(w32)w; x++, v++)
      if (v->is_selected())
      {
        change=i4_T;
        v->set_is_selected(i4_F);
      }

    if (change)
    {
      changed();
      refresh();
    }
  }
}


void g1_controller_edit_class::select_verts_in_area(sw32 x1, sw32 y1, 
                                                    sw32 x2, sw32 y2, 
                                                    g1_mode_handler::select_modifier mod)
{
  sw32 w=(get_map()->width()+1),h=(get_map()->height()+1),x,y;
  g1_map_vertex_class *v=get_map()->vertex(0,0);
  
  i4_bool change=i4_F;

  g1_editor_instance.add_undo(G1_MAP_VERTS);


  for (y=0; y<h; y++)
    for (x=0; x<w; x++, v++)
    {
      if (v->get_flag(g1_map_vertex_class::WAS_DRAWN_LAST_FRAME))
      {
        r1_vert vt;
        if (g1_render.project_point(i4_3d_point_class((float)x, (float)y, v->get_height()),
                                    vt,
                                    g1_context.transform))
        {
          if (vt.px>=x1 && vt.px<=x2 && vt.py>=y1 && vt.py<=y2)
          {
            change=i4_T;

            if (mod==g1_mode_handler::SUB_FROM_OLD)
              v->set_is_selected(i4_F);
            else
              v->set_is_selected(i4_T);


          }
          else if (mod==g1_mode_handler::CLEAR_OLD_IF_NO_SELECTION)
          {
            if (v->is_selected())
            {
              change=i4_T;
              v->set_is_selected(i4_F);
            }
          }
        } else if (mod==g1_mode_handler::CLEAR_OLD_IF_NO_SELECTION)
        {
          if (v->is_selected())
          {
            change=i4_T;
            v->set_is_selected(i4_F);
          }
        }
      }
    }

  if (change)
  {
    changed();
    refresh();
  }
}


g1_controller_edit_class::g1_controller_edit_class(w16 w, w16 h,
                                                   i4_graphical_style_class *style)
  : g1_object_controller_class(w,h, style)
{    
  focus_visible=i4_F;

  move_vert_x=move_vert_y=0;
  move_vert_z=0;
  move_verts=i4_F;

  edit_win=0;
  drag_select=i4_F;

  graph_changed=i4_F;
  need_restore_cell=i4_F;     
  place_object=0;
  //selected_object=0;

  place_object_on_map=i4_F;
  cell_x=cell_y=0;
  last_mouse_x=last_mouse_y=0;
    
  mode=0;
  setup_mode();

}

// e_state.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



g1_edit_state_class g1_edit_state;

g1_controller_edit_class *g1_edit_state_class::current_focus()
{
  if (g1_editor_instance.in_editor() &&  g1_current_controller.get())
    return ((g1_controller_edit_class *)g1_current_controller.get());
  else
    return 0;
}

void g1_edit_state_class::hide_focus()
{
  if (current_focus())
    current_focus()->hide_focus();
}


void g1_edit_state_class::show_focus()
{
  if (current_focus())
    current_focus()->show_focus();
}

i4_graphical_style_class *g1_edit_state_class::get_style()
{
  return i4_current_app->get_style();
}


i4_button_class *g1_edit_state_class::create_button(char *res_name, 
                                                    w32 evalue, 
                                                    i4_bool popup,
                                                    i4_event_handler_class *send_to,
                                                    i4_event *send_event)
{
  if (send_to==0)
    send_to=this;

  i4_const_str s=g1_editor_instance.get_editor_string(res_name);
  if (s.null())
    i4_error("missing %s from resource/editor.res",res_name);

  i4_const_str::iterator i=s.begin();
  sw32 icon_num=i.read_number();

  load_icons();

  if (icon_num<0 || icon_num>=total_icons)
    i4_error("bad icon #, %d",icon_num); 

  // change this to an icon at some point
  i4_image_window_class *t=new i4_image_window_class(icons[icon_num]);

  if (!send_event)
    send_event=new i4_object_message_event_class(send_to, evalue);

  i4_event_reaction_class *r=new i4_event_reaction_class(send_to, send_event);

  char help[30];
  sprintf(help,"%s_help",res_name);
  i4_button_class *b=new i4_button_class(&g1_ges(help), t, get_style(), r);


  if (popup)
    b->set_popup(popup);


  return b;
}

void g1_edit_state_class::add_but(i4_button_box_class *box,                     
                                  char *res_name, 
                                  w32 evalue, 
                                  i4_bool down,
                                  i4_event *send_event)
{
  char help[30];
  sprintf(help,"%s_help",res_name);
  i4_button_class *b=create_button(res_name, evalue, i4_F, 0, send_event);

  box->add_button(0,0, b);
  
  if (down)
    box->push_button(b, i4_F);

}


i4_window_class *g1_edit_state_class::create_buttons(w32 height)
{
  i4_graphical_style_class *style=get_style();

  i4_button_box_class *box=new i4_button_box_class(this);

  int i=0;
  g1_mode_creator *mc, *cur=0;
  char buf[250];
  for (mc=g1_mode_creator::first; mc; mc=mc->next, i++)
  {
	  mc->name(buf);
    if (strcmp(major_mode, buf)==0)
      cur=mc;
  
    add_but(box, buf, 0, (i4_bool) (cur==mc), new g1_set_major_mode_event(buf));
  }

  if (cur)
  {
    box->arrange_right_down();

    i4_parent_window_class *minor_container=new i4_color_window_class(0, (w16)height,
                                                                      style->color_hint->neutral(),
                                                                      style);
    cur->create_buttons(minor_container);
    minor_container->resize_to_fit_children();
    
    i4_deco_window_class *d;
    d=new i4_deco_window_class(box->width() + minor_container->width(), (w16)height, i4_F, style);


    d->add_child((short)d->get_x1(), (short)d->get_y1(), box);
    d->add_child((short)(d->get_x1() + box->width()), (short)d->get_y1(), minor_container);
    
    return d;
  }

  if (box)
    delete box;

  return 0;
}

void g1_edit_state_class::show_tools(i4_parent_window_class *p, 
                                     w32 _win_h)
{
  win_h=_win_h;
  parent=p;

  if (tools)
    delete tools;
  tools=create_buttons(win_h);
  parent->add_child(parent->width()-tools->width(), 0, tools);
  
}

void g1_edit_state_class::hide_tools()
{
  if (tools)
  {
    delete tools;
    tools=0;
    parent=0;
  }

  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
    mc->cleanup();
}

void g1_edit_state_class::snap_point(i4_3d_point_class &p, int do_z)
{
  switch (snap)
  {
    case SNAP_CENTER : 
    {            
      p.x=((int)(p.x-0.5f))+0.5f; 
      p.y=((int)(p.y-0.5f))+0.5f; 
      if (do_z)
        p.z=((int)(p.z-0.5f))+0.5f; 
    } break;

    case SNAP_ORIGIN :
    {            
      int a;
      a = ((int)p.x); p.x = (float)a;  
      a = ((int)p.y); p.y = (float)a;  
      if (do_z)
      {
        a = ((int)p.z); p.z = (float)a;  // JJ  HMM... Maybe it's Error why not 'z' ? 
		// was p.y
      }  
    } 
    break;
  }
}

void g1_edit_state_class::set_current_team(int team_num)
{
  current_team=team_num;
}

g1_edit_state_class::g1_edit_state_class()
{
  current_team=0;

  snap=NO_SNAP;
  icons=0;
  strcpy(major_mode, "LIGHT");
  tools=0;
  parent=0;
 }

g1_mode_creator *g1_edit_state_class::get_major_mode()
{
	char buf[250];
  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
  {

	  mc->name(buf);
    if (strcmp(major_mode, buf)==0)
      return mc;
  }

  return 0;
}

i4_bool g1_edit_state_class::set_major_mode(char *mode_name)
{ 
  hide_focus();

  char buf[250];
  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
  {
	  mc->name(buf);
    if (strcmp(mode_name, buf)==0)
    {
      strcpy(major_mode, buf);

      if (tools)
        delete tools;

      tools=create_buttons(win_h);
      parent->add_child(parent->width()-tools->width(), 0, tools);

      show_focus();      
  
      g1_editor_instance.major_mode_change();

      return i4_T;
    }
  }     
  return i4_F;
}

i4_bool g1_edit_state_class::set_minor_mode(char *major_mode_name, w32 minor_mode)
{
  i4_bool ret;
  hide_focus();


  if (strcmp(major_mode, major_mode_name)==0 || set_major_mode(major_mode_name))
  {
    ret=get_major_mode()->set_minor_mode(minor_mode);
   
    if (tools)
    {
      delete tools;
      tools=create_buttons(win_h);
      parent->add_child(parent->width()-tools->width(), 0, tools);
    }
  }
  else
    ret=i4_F;
  
  show_focus();

  return ret;
}

void g1_edit_state_class::receive_event(i4_event *ev)
{
  CAST_PTR(oev, i4_object_message_event_class, ev);

  if (ev->type()==i4_event::USER_MESSAGE)
  {
    CAST_PTR(uev, i4_user_message_event_class, ev);
    if (uev->sub_type==G1_SET_MAJOR_MODE)
    {
      CAST_PTR(sm, g1_set_major_mode_event, ev);
      set_major_mode(sm->name);
    }
    else if (uev->sub_type==G1_SET_MINOR_MODE)
    {      
      CAST_PTR(sm, g1_set_minor_mode_event, ev);
      set_minor_mode(sm->name, sm->minor_mode);
    }
  }
}


void g1_edit_state_class::load_icons()
{
  if (icons) return;

  i4_const_str *e_cons=g1_editor_instance.get_editor_array("e_icons"), *e;
  e=e_cons;

  for (total_icons=0; !e->null(); total_icons++, e++);
  e=e_cons;

  icons=(i4_image_class **)I4_MALLOC(sizeof(i4_image_class *) * total_icons, "icons");

  for (w32 i=0; i<(w32)total_icons; i++, e++)
  {
    if (e->null())
      i4_error("not enough icons in e_icons array");

    icons[i]=i4_load_image(*e);
  }

  i4_free(e_cons);
}


void g1_edit_state_class::uninit()
{
  hide_tools();
  int i;
  

  for (i=0; i<total_icons; i++)
    if (icons[i])
    {
      delete icons[i];
      icons[i]=0;
    }

  if (icons)
    i4_free(icons);
  icons=0;
  total_icons=0;
}

w32 g1_edit_state_class::tools_width()
{
  if (tools)
    return tools->width();
  else return 0;
}




void g1_edit_state_class::context_help_struct::show(const i4_const_str &help, 
                                                    int _mx, int _my)
{
  hide();
  mx=_mx; my=_my;
  window=i4_current_app->get_style()->create_quick_context_help(_mx, _my+15, help);
}

void g1_edit_state_class::context_help_struct::hide()
{
  if (window.get())
  {
    i4_kernel.delete_handler(window.get());
    window=0;
  }
}

li_object *g1_edit_selected(li_object *o, li_environment *env)
{
  if (g1_edit_state.current_focus())
  {
    if (g1_edit_state.current_focus()->get_mode())
      g1_edit_state.current_focus()->get_mode()->edit_selected();
  }
  
  return 0;
  
}

li_automatic_add_function (g1_edit_selected, "edit_selected");

/////////////////////////////////////
// MODE DIRECTORY 
//////////////////////////////////////

// mode/e_camera.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



g1_camera_params g1_e_camera;

void g1_camera_mode::edit_selected()
{
  i4_spline_class *s[g1_controller_edit_class::MAX_SPLINE_EDIT];
  int t=c->get_current_splines(s,g1_controller_edit_class::MAX_SPLINE_EDIT);

  w32 cur_frame=0;

  for (int i=0; i<t; i++)
  {
    i4_spline_class::point *sp=s[i]->begin();
    for (;sp;sp=sp->next)
    {
      if (sp->selected)
        cur_frame=sp->frame;
    }
  }

  i4_parent_window_class *p=g1_create_time_edit_window(c->style, cur_frame);
  g1_e_camera.set_edit_window(c->parent->root_window(), lx() + c->x(), ly() + c->y());
}

g1_mode_handler::state g1_camera_mode::current_state()
{
  w8 remap[]={ ROTATE, ZOOM, OTHER, OTHER, DRAG_SELECT, DRAG_SELECT };

  I4_ASSERT(g1_e_camera.get_minor_mode()<=sizeof(remap), "state too big");
  return (g1_mode_handler::state)remap[g1_e_camera.get_minor_mode()];
}

void g1_camera_mode::mouse_down()
{
  if (g1_e_camera.get_minor_mode()==g1_camera_params::ADD_CAMERA)
    c->add_movie_control_point(0);
  else if (g1_e_camera.get_minor_mode()==g1_camera_params::ADD_TARGET)
    c->add_movie_control_point(1);
  else if (g1_e_camera.get_minor_mode()==g1_camera_params::ADD_OBJECT)
    c->add_movie_control_point(2);

  g1_mode_handler::mouse_down();
}


i4_bool g1_camera_mode::select_object(sw32 mx, sw32 my, 
                                      i4_float &ox, i4_float &oy, i4_float &oz,
                                      select_modifier mod)
{
  i4_spline_class::point *sp=c->find_spline_point(mx, my, 0);

  if (sp)
  {
    
    if ((sp->selected && mod==SUB_FROM_OLD) ||
        (!sp->selected && (mod==ADD_TO_OLD || mod==CLEAR_OLD_IF_NO_SELECTION)))
    {
      g1_editor_instance.add_undo(G1_MAP_MOVIE);

      if (mod == CLEAR_OLD_IF_NO_SELECTION)
        c->clear_selected_points();
      sp->selected = !sp->selected;

      c->changed();
      c->refresh();
    }
         
    ox=sp->x;    oy=sp->y;    oz=sp->z;
    return i4_T;
  }
  else if (mod == CLEAR_OLD_IF_NO_SELECTION)
  {
    g1_editor_instance.add_undo(G1_MAP_MOVIE);
    
    c->clear_selected_points();
  }

  return i4_F;
}

void g1_camera_mode::move_selected(i4_float xc, i4_float yc, i4_float zc,
                                   sw32 mouse_x, sw32 mouse_y)
{
  if (g1_e_camera.get_minor_mode()==g1_camera_params::MOVE)
    c->move_selected_points(xc,yc,zc);
}

void g1_camera_mode::post_draw(i4_draw_context_class &context)
{
  i4_spline_class *s[g1_controller_edit_class::MAX_SPLINE_EDIT];
  int t=c->get_current_splines(s,g1_controller_edit_class::MAX_SPLINE_EDIT);

  i4_color pas_colors[3]={0x7f0000, 0x007f00, 0x00007f };
  i4_color act_colors[3]={0xff0000, 0x00ff00, 0x0000ff };
                          

  for (int i=0; i<t; i++)
  {
    w32 cf=c->get_map()->get_current_movie()->get_frame();
    
    c->draw_spline(c->local_image, &c->g1_context, 0xffffff,
                   pas_colors[i], act_colors[i], s[i], cf);
  }

  g1_mode_handler::post_draw(context);
}

void g1_camera_mode::select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                            select_modifier add_modifier)
{
  if (!c->get_map()->current_movie)
    return ;

  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  if (!add_modifier)
    c->clear_selected_points();

  g1_movie_flow_class *flow=c->get_map()->current_movie;

  i4_bool change=i4_F;

  r1_vert rv;
  i4_spline_class::point *first=0, *last=0;
  
  i4_spline_class *s[g1_controller_edit_class::MAX_SPLINE_EDIT];
  int t=c->get_current_splines(s,g1_controller_edit_class::MAX_SPLINE_EDIT);


  for (w32 i=0; i<(w32)t; i++)
  {
    i4_spline_class::point *sp=s[i]->begin();

    for (;sp;sp=sp->next)
    {
      if (g1_render.project_point(i4_3d_point_class(sp->x, sp->y, sp->z),
                           rv,
                           c->g1_context.transform))
      {
        if (rv.px>=x1 && rv.py>=y1 && rv.px<=x2 && rv.py<=y2)
        {
          if (add_modifier==CLEAR_OLD_IF_NO_SELECTION || add_modifier==ADD_TO_OLD)
          {
            if (!sp->selected)
            {
              sp->selected=i4_T;
              change=i4_T;
            } 
          } else if (add_modifier==SUB_FROM_OLD)
          {
            sp->selected=i4_F;
            change=i4_T;
          }         
        }
      }     
    }
  }
 
  if (change)
  {
    c->refresh();  
    c->changed();
  }
}

void g1_camera_mode::delete_selected()
{
  c->delete_selected_points();
}



g1_camera_params::g1_camera_params()
{ 
  edit_win=0;
  minor_mode=MOVE;
}



void g1_camera_params::create_buttons(i4_parent_window_class *container)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"cROTATE", "cZOOM", 
              "cADD_CAMERA", "cADD_TARGET", "cADD_OBJECT",
              "cMODIFY", "cSELECT", 0};
  w32 i=ROTATE;
  for (char **a=rn; *a; a++, i++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==minor_mode, new g1_set_minor_mode_event("CAMERA",(w8)i));
                          

  box->arrange_right_down();
  container->add_child(0,0, box);
}



void g1_camera_params::cleanup()
{
  if (edit_win.get())
  {
    get_style()->close_mp_window(edit_win.get());
    edit_win=0;
  }
}

void g1_camera_params::set_edit_window(i4_parent_window_class *p, sw32 x, sw32 y)
{
  cleanup(); 
  edit_win=get_style()->create_mp_window((short)x,(short)y,
                                   p->width(), p->height(), 
                                   g1_ges("edit_time"), 0);
  edit_win->add_child(0,0,p);
}

// mode/e_light.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



g1_light_params g1_e_light;

g1_mode_handler::state g1_light_mode::current_state()
{
  w8 remap[]={ ROTATE, ZOOM, DRAG_SELECT, DRAG_SELECT, OTHER, OTHER };
  I4_ASSERT(g1_e_light.minor_mode<=sizeof(remap), "state too big");
  return (g1_mode_handler::state)remap[g1_e_light.get_minor_mode()];
}

void g1_light_params::create_buttons(i4_parent_window_class *container)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"lROTATE", "lZOOM", 0 }, **a;
  w32 i=ROTATE;

  for (a=rn; *a; i++, a++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==minor_mode,
                          new g1_set_minor_mode_event("LIGHT",(w8)i));

  box->arrange_right_down();
  box->resize_to_fit_children();


  container->add_child(0,0,box);


  char *indiv_buts[]={"lGDARKEN", "lGBRIGHTEN", "lDDARKEN", "lDBRIGHTEN", "lAMBIENT", 0};
  int but_ids[]={ GDARKEN, GBRIGHTEN, DDARKEN, DBRIGHTEN, AMBIENT };
  int rep_ids[]={ GDARKEN_REP, GBRIGHTEN_REP, DDARKEN_REP, DBRIGHTEN_REP, 0 };


  int y=box->height()+5;

  
  for (i=0, a=indiv_buts; *a; i++, a++)
  {
    i4_button_class *b=g1_edit_state.create_button(*a, but_ids[i], i4_T, this);
    if (rep_ids[i])
      b->set_repeat_down(i4_T, new i4_event_reaction_class(this, rep_ids[i]));

    container->add_child(0, y, b);
    y+=b->height();
  }
}


void g1_light_params::receive_event(i4_event *ev)
{
  int sub_type=-1;
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(oev,i4_object_message_event_class,ev);
    if (oev->object==this)
      sub_type=oev->sub_type;
  }
  else if (ev->type()==i4_event::USER_MESSAGE)
    sub_type=((i4_user_message_event_class *)ev)->sub_type;

  if (sub_type!=-1)
  {
    int recalc=0;
    float increment=1/64.0;

    switch (sub_type)
    {
      case GDARKEN :
        li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));
        
      case GDARKEN_REP :
      {
        if (g1_lights.ambient_intensity - increment>=0)
        {
          recalc=1;
          g1_lights.set_ambient_intensity(g1_lights.ambient_intensity-increment);
        }
      } break;

      case GBRIGHTEN :
        li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));

      case GBRIGHTEN_REP :
      {
        if (g1_lights.ambient_intensity+increment <= 1.0)
        {
          recalc=1;
          g1_lights.set_ambient_intensity(g1_lights.ambient_intensity+increment);
        }
      } break;

      case DDARKEN :
        li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));

      case DDARKEN_REP :
      {
        if (g1_lights.directional_intensity - increment>=0)
        {
          recalc=1;
          g1_lights.set_directional_intensity(g1_lights.directional_intensity-increment);
        }
      } break;

      case DBRIGHTEN :
        li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));
      case DBRIGHTEN_REP :
      {
        if (g1_lights.directional_intensity+increment <= 1.0)
        {
          recalc=1;
          g1_lights.set_directional_intensity(g1_lights.directional_intensity+increment);
        }
      } break;

      
      case AMBIENT :
      {
        if (g1_current_controller.get())
        {
          recalc=1;
          li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));

          i4_transform_class *t=&g1_current_controller->transform;
          i4_3d_vector cam1, cam2; 
          t->inverse_transform(i4_3d_vector(0,0,0), cam1);
          t->inverse_transform(i4_3d_vector(0,0,1), cam2);
          cam2-=cam1;

          g1_lights.direction=cam2;
        }
      } break;

    }
    
    if (recalc)
    {
      g1_calc_static_lighting();
      g1_editor_instance.changed();

      li_call("redraw");
    }
  }
}

// mode/e_mode.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



sw32 g1_mode_handler::lx() { return c->last_mouse_x; }
sw32 g1_mode_handler::ly() { return c->last_mouse_y; }

sw32 g1_mode_handler::x() { return c->x(); }
sw32 g1_mode_handler::y() { return c->y(); }


g1_mode_creator *g1_mode_creator::first=0;

void g1_mode_handler::uninit()
{
}

i4_bool g1_mode_handler::active()
{
  return c->active();
}

g1_mode_creator::g1_mode_creator()
{
  minor_mode=0;
  next=first;
  first=this;
}

g1_mode_creator::~g1_mode_creator()
{
  if (first==this)
    first=first->next;
  else
  {
    g1_mode_creator *last=0;
    for (g1_mode_creator *f=first; f!=this;)
    {
      last=f;
      f=f->next;
    }
    last->next=next;
  }
} 


i4_graphical_style_class *g1_mode_creator::get_style()
{
  return i4_current_app->get_style();
}

i4_transform_class *g1_mode_handler::get_transform()
{
  return &c->transform;
}

g1_mode_handler::g1_mode_handler(g1_controller_edit_class *c) 
  : c(c) 
{
  mouse_down_flag=i4_F;
  
  drag=i4_F;
  drag_select=i4_F;
  cursor_type=G1_DEFAULT_CURSOR;
}

void g1_mode_handler::update_cursor()
{
  float x,y,z;
  w8 nc=G1_DEFAULT_CURSOR;
  
 

  if (g1_map_is_loaded() && current_state()==DRAG_SELECT) 
  {
    if (drag)
      nc=G1_MOVE_CURSOR;
    else if (select_object(lx(), ly(), x,y,z, FOR_CURSOR_HINT))
      nc=G1_SELECT_CURSOR;
  }

  if (nc!=cursor_type)
  {
    cursor_type=nc;
    c->set_cursor(g1_resources.big_cursors[cursor_type]);
  }
}


void g1_mode_handler::mouse_move(sw32 mx, sw32 my)
{
  switch (current_state())
  {
    case DRAG_SELECT :
    {
      if (drag)   // are we dragging game stuff around?
      {
        i4_transform_class *t=&c->transform;

        i4_3d_point_class control_point;   

        t->transform(pos, control_point);

        i4_3d_vector is(1,0,0), js(0,1,0), center, i,j;
        t->inverse_transform(i4_3d_vector(0,0,0), center);
        t->inverse_transform(is, i);
        i-=center;
        t->inverse_transform(js, j); 
        j-=center;

        // i & j now for the camera plane

        i4_float csx=g1_render.scale_x;
        i4_float csy=g1_render.scale_y;



        i4_float sx=(mx-lx())/(csx*g1_render.center_x) * control_point.z;
        i4_float sy=(my-ly())/(csy*g1_render.center_y) * control_point.z;
    

        i*=sx;
        j*=sy;
   
        i4_float xc=i.x+j.x, yc=i.y+j.y, zc=i.z+j.z;

		//todo: add code to avoid overshot in z direction
		if (zc>0.5)
			zc=0.5f;
		if (zc<-0.5)
			zc=-0.5f;
        pos.x+=xc; pos.y+=yc; pos.z+=zc;
        i4_3d_point_class current_snap_pos=pos;
        g1_edit_state.snap_point(current_snap_pos);

        move_selected(current_snap_pos.x-snap_pos.x,
                      current_snap_pos.y-snap_pos.y,
                      current_snap_pos.z-snap_pos.z,
                      mx-lx(), my-ly());    

        snap_pos=current_snap_pos;
      }
      else if (mouse_down_flag && drag_select)
      {
        sx2=mx;
        sy2=my;
        c->request_redraw();
      }
    } break;
    
    case ZOOM :
      if (mouse_down_flag)
        if (i4_current_app->get_window_manager()->shift_pressed())
          c->rotate((mx-lx())*0.01f, (ly()-my)*0.01f);
        else
          c->zoom((ly()-my)/50.0f);
      
      break;
      
    case ROTATE :
      if (mouse_down_flag)
        if (i4_current_app->get_window_manager()->shift_pressed())
          c->zoom((ly()-my)/50.0f);
        else
          c->rotate((mx-lx())*0.01f, (ly()-my)*0.01f);
          break; 
  }
}

void g1_mode_handler::mouse_up()
{
  if (mouse_down_flag)
  {
    ungrab_mouse();

    if (drag)
    {
      update_cursor();
      drag=i4_F;
    }
    else if (current_state()==DRAG_SELECT && drag_select)
    {
      sw32 dx1,dy1,dx2,dy2;
      if (sx1<sx2) { dx1=sx1; dx2=sx2; } else { dx1=sx2; dx2=sx1; }
      if (sy1<sy2) { dy1=sy1; dy2=sy2; } else { dy1=sy2; dy2=sy1; }

      select_modifier mod;
      if (i4_current_app->get_window_manager()->shift_pressed())
        mod=ADD_TO_OLD;
      else if (i4_current_app->get_window_manager()->alt_pressed())
        mod=SUB_FROM_OLD;
      else
        mod=CLEAR_OLD_IF_NO_SELECTION;

      select_objects_in_area(dx1,dy1,dx2,dy2, mod);
      c->request_redraw();
      drag_select=i4_F;
      update_cursor();
    }

  }
}

void g1_mode_handler::grab_mouse()
{
  mouse_down_flag=i4_T;
}

void g1_mode_handler::ungrab_mouse()
{
  mouse_down_flag=i4_F;
}

void g1_mode_handler::do_command(i4_do_command_event_class *cmd)
{
  if (!strcmp(cmd->command, "edit_selected"))
    edit_selected();  
}

void g1_mode_handler::end_command(i4_end_command_event_class *cmd)
{
  
}


void g1_mode_handler::mouse_down()
{ 
  grab_mouse();

  if (current_state()==DRAG_SELECT)
  {
    select_modifier mod;
    if (i4_current_app->get_window_manager()->shift_pressed())
      mod=ADD_TO_OLD;
    else if (i4_current_app->get_window_manager()->alt_pressed())
      mod=SUB_FROM_OLD;
    else
      mod=CLEAR_OLD_IF_NO_SELECTION;


    if (select_object(lx(), ly(), pos.x, pos.y, pos.z, mod))
    {
      snap_pos=pos;
      i4_time_class now;
      if ( (w32)now.milli_diff(last_click_time) < c->style->time_hint->double_click )
        edit_selected();
      else
        drag=i4_T;
    }
    else
    {
      drag_select=i4_T;
      sx2=sx1=lx();
      sy2=sy1=ly();
    }

    update_cursor();
  }

  last_click_time.get();
}

void g1_mode_handler::post_draw(i4_draw_context_class &context)
{
  if (current_state()==DRAG_SELECT && mouse_down_flag && !drag)
    g1_render.draw_rectangle(sx1,sy1,sx2,sy2, 0xffffff, context);
}

i4_bool g1_mode_handler::pass_through_focus_click()
{
  switch (current_state())
  {
    case ZOOM :
    case ROTATE : return i4_T; break;
  }
  return i4_F;
}


li_object *g1_delete_selected(li_object *o, li_environment *env)
{
  g1_controller_edit_class *v=g1_editor_instance.get_current_view();
  if (v)
  {
    v->get_mode()->delete_selected();
    return li_true_sym;
  }
  
  return 0;
}

li_automatic_add_function(g1_delete_selected, "Map/Delete Selected");

// mode/e_object.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


static g1_typed_reference_class<g1_object_class> preselected_object;
static li_symbol *g1_last_link_var=0;
static g1_typed_reference_class<g1_object_class> editing_object;
static li_symbol_ref path_object_type("path_object");

g1_object_params g1_e_object;


li_object *g1_object_changed(li_object *o, li_environment *env)
{
  if (editing_object.get())
  {
    if (o)
    {
      editing_object->vars=li_class::get(li_car(o,env),env);

               
      g1_object_class *list[G1_MAX_OBJECTS];
      int t=g1_get_map()->make_object_list(list, G1_MAX_OBJECTS); 
      li_class *old=li_class::get(li_second(o,env),env);
      for (int i=0; i<t; i++)
        list[i]->object_changed_by_editor(editing_object.get(), old);

    }

    g1_e_object.close_edit_window();
  }

  return 0;
}

li_automatic_add_function(g1_object_changed, "object_changed");



g1_mode_handler::state g1_object_mode::current_state()
{
  w8 remap[]={ ROTATE,
               ZOOM,
               OTHER,
               DRAG_SELECT,
               DRAG_SELECT,
               DRAG_SELECT };

  I4_ASSERT(g1_e_object.minor_mode<=sizeof(remap), "state too big");
  return (g1_mode_handler::state)remap[g1_e_object.get_minor_mode()];
}


void g1_object_mode::hide_focus()
{
  if (add.get())
  {
    add->unoccupy_location();
    add->request_remove();
    g1_remove_man.process_requests();
    add=0;
  }
}


      

void g1_object_mode::show_focus()
{
  if (!add.get() && g1_e_object.get_minor_mode()==g1_object_params::ADD)
  {
    w32 type=g1_e_object.get_object_type();
    if (g1_object_type_array[type])
    {
      add=g1_create_object((short)type);


      if (add.get())
      {

        add->player_num=g1_edit_state.current_team;

        i4_float gx,gy, dx,dy;
        if (!c->view_to_game(lx(),ly(), gx,gy, dx,dy))
        { gx=2; gy=2; }
    
        if (gx<0 || gy<0 ||  gx>=c->get_map()->width() || gy>=c->get_map()->height())
        { gx=2; gy=2; }

        i4_3d_point_class pos(gx,gy,c->get_map()->terrain_height(gx,gy));
        g1_edit_state.snap_point(pos);

        add->x=pos.x;
        add->y=pos.y;
        add->h=pos.z;
        add->grab_old();

        if (add->id==g1_get_object_type("lightbulb"))
        {
          add->h+=2;
          g1_light_object_class *l=g1_light_object_class::cast(add.get());
          l->setup(add->x, add->y, add->h, 1,1,1,1);
        }

        add->player_num=g1_edit_state.current_team;

        add->grab_old();

        if (!add->occupy_location())
        {
          g1_remove_man.process_requests();        
          add=0;
        }
        else
        {

          g1_object_class *list[G1_MAX_OBJECTS];
          int t=g1_get_map()->make_object_list(list, G1_MAX_OBJECTS); 

          // if we are creating a path-object, and a path object was previously selected, form a
          // link between the two
          g1_path_object_class *po=g1_path_object_class::cast(add.get()), *p2;
          if (po)
          {

            for (int i=0; i<t; i++)
            {
              if (list[i]->get_flag(g1_object_class::SELECTED))
              {
                p2=g1_path_object_class::cast(list[i]);
                if (p2)
                {
                  p2->add_link(G1_ALLY, po);
                  po->add_link(G1_ENEMY, p2);
                }
                else 
                {
                  po->add_controlled_object(list[i]);
                  for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
                  {
                    if (f==list[i])
                      f->set_start(po);
                  }
                }
              }
            }
          } else
          {
            // see if a path object was previously selected, if so add ourself to it's
            // controlled-object list

            for (int i=0; i<t; i++)
            {
              if (list[i]->get_flag(g1_object_class::SELECTED))
              {
                po=g1_path_object_class::cast(list[i]);
                if (po)
                {
                  po->add_controlled_object(add.get());
                  for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
                  {
                    if (f==add.get())
                      f->set_start(po);
                  }
                }
              }
            }
          }

          add->grab_old();          
        }

        c->refresh();
      }
    }
  }
}

void g1_object_mode::mouse_down()
{
  if (add.get())
  {
    hide_focus();
    g1_editor_instance.add_undo(G1_MAP_OBJECTS);
    show_focus();

    g1_object_class *list[G1_MAX_OBJECTS];
    int t=g1_get_map()->make_object_list(list, G1_MAX_OBJECTS);           
    for (int i=0; i<t; i++)
      list[i]->set_flag(g1_object_class::SELECTED, 0);    
    add->set_flag(g1_object_class::SELECTED, 1);




    add->request_think();
    g1_player_man.get(add->player_num)->add_object(add->global_id);
    add=0;
    show_focus();
  }

  g1_mode_handler::mouse_down();
}



void g1_object_mode::mouse_move(sw32 mx, sw32 my)
{
  if (!add.get())
    show_focus();

  if (add.get())
  {
    i4_float gx,gy, dx,dy;
    if (c->view_to_game(mx,my, gx,gy, dx,dy) &&
        (!(gx<0 || gy<0 ||  gx>=c->get_map()->width() || gy>=c->get_map()->height())))
    {      

      i4_3d_point_class pos(gx,gy, c->get_map()->terrain_height(gx,gy));
      g1_edit_state.snap_point(pos);

      if (pos.x!=add->x || pos.y!=add->y || pos.z!=add->h)
      {
        add->unoccupy_location();
        add->player_num=g1_edit_state.current_team;
    
        add->x=pos.x;
        add->y=pos.y;
        add->h=pos.z;
        add->grab_old();
      
        if (!add->occupy_location())
        {
          g1_remove_man.process_requests();        
          add=0;
        }
      
        c->refresh();
      }
    }
  }

  g1_mode_handler::mouse_move(mx,my);
}

static li_symbol_ref s_add_link("add_link"), s_remove_link("remove_link");

li_object *g1_add_link(li_object *o, li_environment *env)
{
  if (preselected_object.get())
  {
    li_call("redraw");
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);    
    li_g1_ref *who=new li_g1_ref(preselected_object.get()->global_id);    
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
        olist[i]->message(s_add_link.get(), who, 0);
  }
  
  return 0;
}
li_automatic_add_function(g1_add_link, "add_link");


li_object *g1_remove_link(li_object *o, li_environment *env)
{
  if (preselected_object.get())
  {
    li_call("redraw");
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
      
    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);    
    li_g1_ref *who=new li_g1_ref(preselected_object.get()->global_id);    
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
        olist[i]->message(s_remove_link.get(), who, 0);
  }
  
  return 0;
}
li_automatic_add_function(g1_remove_link, "remove_link");


li_object *g1_fix_forward_link(li_object *o, li_environment *env)
{
  if (preselected_object.get())
  {
    li_call("redraw");
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);    
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
      {
        g1_map_piece_class *mp = g1_map_piece_class::cast(olist[i]);
        if (mp)
          mp->fix_forward_link(preselected_object.get());
      }
  }
  
  return 0;
}
li_automatic_add_function(g1_fix_forward_link, "fix_forward_link");


li_object *g1_fix_previous_link(li_object *o, li_environment *env)
{
  if (preselected_object.get())
  {
    li_call("redraw");
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);    
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
      {
        g1_map_piece_class *mp = g1_map_piece_class::cast(olist[i]);
        if (mp)
          mp->fix_previous_link(preselected_object.get());
      }
  }
  
  return 0;
}
li_automatic_add_function(g1_fix_previous_link, "fix_previous_link");


li_object *g1_fix_path_link(li_object *o, li_environment *env)
{
  g1_path_object_class *p2 = g1_path_object_class::cast(preselected_object.get());
  g1_map_piece_class *mp = 0;
  g1_path_object_class *p1 = 0;

  if (p2)
  {
    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);

    for (int i=0; i<t; i++)
      if (olist[i]->selected())
      {
        g1_map_piece_class *_mp = g1_map_piece_class::cast(olist[i]);
        g1_path_object_class *_p1 = g1_path_object_class::cast(olist[i]);
        if (_mp)
          mp = _mp;
        if (_p1)
          p1 = _p1;
      }

    if (p1 && mp)
    {
      int index = p1->get_path_index(p2);

      if (index>=0)
        p1->link[index].object = mp;
    }
  }
  
  return 0;
}
li_automatic_add_function(g1_fix_path_link, "fix_path_link");


li_object *g1_fix_tick_count(li_object *o, li_environment *env)
{
  if (o)
  { 
    w32 new_tick = li_get_int(li_eval(li_car(o,env), env),env);
    g1_tick_counter = new_tick;
  }
  
  return 0;
}
li_automatic_add_function(g1_fix_tick_count, "fix_tick_count");


void g1_object_mode::key_press(i4_key_press_event_class *kev)
{
  int i,t;
  g1_object_class *olist[G1_MAX_OBJECTS];
  
  switch (kev->key)
  {
    case 'A' :
    {
      hide_focus();
      t = c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
      for (i=0; i<t; i++)  
        olist[i]->flags |= g1_object_class::SELECTED;      
      show_focus();
      c->refresh();
      break;
    }

    case '[' :
    {
      if (g1_e_object.get_object_type()>0)
      {
        int find=-1;
        for (int j=g1_e_object.get_object_type()-1; 
             find==-1 && j>0;
             j--)
        {
          if (g1_object_type_array[j] && g1_object_type_array[j]->editor_selectable())
            find=j;
        }

        if (find)
        {
          hide_focus();        
          g1_e_object.set_object_type(find);
          show_focus();
        }

      } break;
    } break;

    case ']' :
    {      
      if (g1_e_object.get_object_type()<=g1_last_object_type)
      {
        int find=-1;
        for (int j=g1_e_object.get_object_type()+1; 
             find==-1 && j<=g1_last_object_type;
             j++)
        {
          if (g1_object_type_array[j] && g1_object_type_array[j]->editor_selectable())
            find=j;
        }
          
        
        if (find)
        {
          hide_focus();
          g1_e_object.set_object_type(find);
          show_focus();
        }

      } break;
    } break;


  
  }
  g1_mode_handler::key_press(kev);
}

void g1_object_mode::delete_selected()
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (int i=0; i<t; i++)
    if (olist[i]->selected())
    {
      olist[i]->unoccupy_location();
      olist[i]->request_remove();
      g1_remove_man.process_requests();
    }

  c->refresh();
}

float g1_object_mode::get_snap_theta()
{
  return real_object_theta-(float)fmod(real_object_theta, i4_pi_2());
}

void g1_object_mode::move_selected(i4_float xc, i4_float yc, i4_float zc,
                                   sw32 mouse_x, sw32 mouse_y)
{
  if (!no_more_move_undos)
  {
    g1_editor_instance.add_undo(G1_MAP_OBJECTS);
    no_more_move_undos=i4_T;
  }



  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);


  for (int i=0; i<t; i++)
    if (olist[i]->flags&g1_object_class::SELECTED)
    {
      olist[i]->unoccupy_location();

      switch (g1_e_object.get_minor_mode())
      {
        case g1_object_params::MOVE :
        {
          g1_object_class *o=olist[i];
          o->x+=xc;
          o->y+=yc;
          o->h+=zc;


        } break;

        case g1_object_params::OBJECT_ROTATE :
        {
          real_object_theta += mouse_y/300.0f * 2*3.14f;

          if (g1_edit_state.snap==g1_edit_state_class::SNAP_CENTER ||
              g1_edit_state.snap==g1_edit_state_class::SNAP_ORIGIN)
            olist[i]->theta = get_snap_theta();
          else
            olist[i]->theta = real_object_theta;

        }
        break;

      }

      olist[i]->grab_old();
      olist[i]->occupy_location();
    }
  
  c->changed();
  c->refresh();
}

i4_bool g1_object_mode::select_object(sw32 mx, sw32 my, 
                                      i4_float &ox, i4_float &oy, i4_float &oz,
                                      select_modifier mod)
{
  no_more_move_undos=i4_F;

  //if (mod!=FOR_CURSOR_HINT)
  //  g1_editor_instance.add_undo(G1_MAP_OBJECTS);
  //I don't think selecting needs an undo. 1st this is very expensive on
  //large maps and 2nd there are other things that are more important to 
  //be able to undo...

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  int i;
  i4_bool ret=i4_F;
  i4_bool change=i4_F;

  preselected_object=0;
  if (!c->g1_context.transform)
    return i4_F;
  
  int set_team=-1;

  for (i=0; i<t && !ret; i++)
  {
    r1_vert rv;
    if (g1_render.project_point(i4_3d_point_class(olist[i]->x, olist[i]->y, olist[i]->h), 
                         rv, c->g1_context.transform))
      if (abs((sw32)rv.px-mx)<3 && abs((sw32)rv.py-my)<3)
      {
        ox=olist[i]->x;
        oy=olist[i]->y;
        oz=olist[i]->h;
        real_object_theta=olist[i]->theta;

        if ((olist[i]->flags & g1_object_class::SELECTED)==0 &&
            mod==CLEAR_OLD_IF_NO_SELECTION)
        {
          for (int j=0; j<t; j++)
            olist[j]->flags &= ~g1_object_class::SELECTED;
          change=i4_T;
        }


        if (mod==CLEAR_OLD_IF_NO_SELECTION || mod==ADD_TO_OLD)
        {
          olist[i]->flags |= g1_object_class::SELECTED;
          change=i4_T;

          set_team=olist[i]->player_num;
          
        }
        else if (mod==SUB_FROM_OLD)
        {
          olist[i]->flags &= ~g1_object_class::SELECTED;       
          change=i4_T;
        }
        else if (mod==FOR_CURSOR_HINT)
          preselected_object=olist[i];

        ret=i4_T;        
      }
  }

  if (set_team!=-1)
    g1_edit_state.set_current_team(set_team);


  if (change)
  {
    c->changed();
    c->refresh();
  }


  return ret;
}

void g1_object_mode::select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                            select_modifier mod)
{
  no_more_move_undos=i4_F;

  g1_editor_instance.add_undo(G1_MAP_OBJECTS);

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
 
  for (int i=0; i<t; i++)
  {
    r1_vert rv;
    if (g1_render.project_point(i4_3d_point_class(olist[i]->x, 
                                           olist[i]->y, 
                                           olist[i]->h), rv, c->g1_context.transform))
    {
      if (rv.px>=x1 && rv.px<=x2 && rv.py>=y1 && rv.py<=y2)
      {
        if (mod==SUB_FROM_OLD)
          olist[i]->flags&=~g1_object_class::SELECTED;
        else
          olist[i]->flags|=g1_object_class::SELECTED;        
      }
      else if (mod==CLEAR_OLD_IF_NO_SELECTION)
        olist[i]->flags&=~g1_object_class::SELECTED;
    }
  } 

  c->changed();
  c->refresh();
}

li_symbol_ref dbug_objs("dbug_objects");

void g1_object_mode::post_draw(i4_draw_context_class &context)
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  int w=1;
  for (int i=0; i<t; i++)
  {
    r1_vert rv;
    i4_3d_point_class v(olist[i]->x, olist[i]->y, olist[i]->h);
    if (g1_render.project_point(v, rv, c->g1_context.transform))
    {
      w32 color=norm_color;

      li_object *dbug=li_get_value(dbug_objs.get());
      for (;dbug; dbug=li_cdr(dbug,0))
        if ((w32)li_get_int(li_car(dbug,0),0)==olist[i]->global_id)
          color=0xff0000;


      if (olist[i]->flags & g1_object_class::SELECTED)
      {
        color=sel_color;
        i4_3d_point_class floor(v.x, v.y, c->get_map()->terrain_height(v.x,v.y));
        g1_render.render_3d_line(v,floor,sel_color,norm_color,c->g1_context.transform);
      }
  
      r1_clip_clear_area((sw32)rv.px-w, (sw32)rv.py-w, (sw32)rv.px+w, (sw32)rv.py+w, 
                         color, 0.01f, *c->g1_context.context, g1_render.r_api);


    }
  }

  g1_mode_handler::post_draw(context);
}

g1_object_mode::g1_object_mode(g1_controller_edit_class *c) : g1_mode_handler(c)  
{
  add=0;
  sel_color=0xffff00;
  norm_color=0x7f7f7f;
  no_more_move_undos=i4_F;
}

void g1_object_mode::edit_selected()
{
  g1_object_class *olist[G1_MAX_OBJECTS], *first=0;
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS), tsel=0;
  
  g1_e_object.close_edit_window();

  
  for (int i=0; i<t; i++)
  {
    if (olist[i]->selected())
    {
      editing_object=olist[i];
      tsel++;
    }        
  }

  if (tsel==1)    
  {
    i4_window_class *w=g1_object_type_array[editing_object->id]->create_edit_dialog();
    if (w)
      g1_e_object.set_edit_window(w);
  }
  else
    editing_object=0;
}

void g1_object_mode::idle()
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  int closest=-1, closest_d=10000;

  for (int i=0; i<t; i++)
  {
    r1_vert rv;
    if (g1_render.project_point(i4_3d_point_class(olist[i]->x, olist[i]->y, olist[i]->h), 
                         rv, c->g1_context.transform))
      if (abs((sw32)rv.px-lx())<8 && abs((sw32)rv.py-ly())<8)
      {
        int d=abs((sw32)rv.px-lx()) + abs((sw32)rv.py-ly());
        if (d<closest_d)
        {
          closest=i;
          closest_d=d;
        }
      }
  }

  if (closest!=-1)
  {
    i4_str *help=olist[closest]->get_context_string();

    i4_str *h2=i4_const_str("gid=%d %S").sprintf(200,olist[closest]->global_id, help);
    g1_edit_state.context_help.show(*h2, x()+lx(), y()+ly());

    if (help)
      delete help;
    delete h2;

  } 
}



void g1_object_params::create_buttons(i4_parent_window_class *containter)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"oROTATE", "oZOOM", "oADD", "oSELECT", "oMOVE", "oOBJECT_ROTATE", 0 };
  w32 i=ROTATE;
  for (char **a=rn; *a; a++, i++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==minor_mode,
                          new g1_set_minor_mode_event("OBJECT",(w8)i));

  box->arrange_right_down();
  containter->add_child(0,0, box);
}


void g1_object_params::cleanup()
{
  if (edit_win.get())
  {
    get_style()->close_mp_window(edit_win.get());
    edit_win=0;
  }

  if (g1_object_picker_mp.get())
  {
    get_style()->close_mp_window(g1_object_picker_mp.get());
    g1_object_picker_mp=0;
  }

}


void g1_object_params::close_edit_window()
{
  if (edit_win.get())
  {
    get_style()->close_mp_window(edit_win.get());
    edit_win=0;
  }
}

void g1_object_params::set_edit_window(i4_window_class *p)
{
  close_edit_window();
  edit_win=get_style()->create_mp_window(0,0,
                                   p->width(), p->height(), 
                                   g1_ges("edit_object"), 0);
  edit_win->add_child(0,0,p);
}


g1_object_params::g1_object_params()
{
  minor_mode=MOVE; 

  current_object_type=0;
  edit_win=0;

}

g1_mode_handler *g1_object_params::create_mode_handler(g1_controller_edit_class *c)
{  
  return new g1_object_mode(c);
}



li_object *g1_join_objects(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  g1_object_class *olist[G1_MAX_OBJECTS];
  int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i,j;

  for (i=0; i<t; i++)
  {
    if (olist[i]->get_flag(g1_object_class::SELECTED))
    {
      g1_path_object_class *p=g1_path_object_class::cast(olist[i]);
      if (p)
      {
        for (j=0; j<t; j++)
        {
          if (j!=i && olist[j]->get_flag(g1_object_class::SELECTED))
          {
            g1_path_object_class *p2=g1_path_object_class::cast(olist[j]);

            if (!p2)
            {
              p->add_controlled_object(olist[j]);
              
              for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
              {
                if (f==olist[j])
                  f->set_start(p);
              }
            }
            else
            {
              // join to path objects if we can determine the direction
              if ((p->total_links(G1_ENEMY)>0 || p->total_links(G1_ALLY)>0) &&
                  p->total_links(G1_ALLY)==0 &&
                  p2->total_links(G1_ENEMY)==0 &&
                  p->get_path_index(p2)<0)
				{
                p->add_link(G1_ALLY, p2);
                p2->add_link(G1_ENEMY, p);
				}
			  else if (p->total_links(G1_ENEMY)==0 && 
				  p2->total_links(G1_ALLY)==0 &&
				  p2->get_path_index(p)<0)
				  { //not shure wheter this is ok.
				  p->add_link(G1_ENEMY, p2);
				  p2->add_link(G1_ALLY,p);
				  }
			  else
				  {
				  i4_const_str s("Yes=Node %d to Node %d, No=Other way, Cancel=No Op");
				  i4_str *st=s.sprintf(200,p->global_id,p2->global_id);
				  w32 ans=i4_message_box("Choose Direction",*st,MSG_YES+MSG_NO+MSG_CANCEL);
				  delete st;
				  if (ans==MSG_CANCEL)
					  return 0;
				  if (ans==MSG_YES)
					  {
					  p->add_link(G1_ALLY, p2);
					  p2->add_link(G1_ENEMY, p);
					  
					  }
				  else
					  {
					  p->add_link(G1_ENEMY, p2);
					  p2->add_link(G1_ALLY, p);
					  }

				  }
            }
			return 0;
          }
        }          
      }

    }
  }

  return 0;
}

li_object *g1_unjoin_objects(li_object *o, li_environment *env)
	{
	li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
	
	g1_object_class *olist[G1_MAX_OBJECTS];
	int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i,j,k;
	
	for (i=0; i<t; i++)
		{
		if (olist[i]->get_flag(g1_object_class::SELECTED))
			{
			g1_path_object_class *p=g1_path_object_class::cast(olist[i]);
			if (p)
				{
				for (j=0; j<t; j++)
					{
					if (j!=i && olist[j]->get_flag(g1_object_class::SELECTED))
						{
						g1_path_object_class *p2=g1_path_object_class::cast(olist[j]);
						
						if (!p2)
							{
							p->remove_controlled_object(olist[j]);
							
							
							for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
								{
								if (f==olist[j])
									f->set_start(0);
								}
							}
						else
							{//check direction
							int r=p->total_links(G1_ENEMY), swap=0;
							for (k=0; k<r; k++)
								{
								if (p->get_link(G1_ENEMY,k)==p2)
									swap=1;
								}
							if (swap)
								{
								g1_path_object_class *tmp=p;
								p=p2;
								p2=tmp;
								}
							p->remove_link(G1_ALLY,p2);
							p2->remove_link(G1_ENEMY,p);
								
								
							}
						}
					}          
				}
			
			}
		}
	
	return 0;
	}


li_object *g1_remove_all_paths(li_object *o, li_environment *env)
	{
	li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
	g1_object_class *olist[G1_MAX_OBJECTS];
	int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;
	g1_path_object_class *p;
	for (i=0;i<t;i++)
		{
		p=g1_path_object_class::cast(olist[i]);
		if (p)
			{
			p->stop_thinking();
			p->unoccupy_location();
			p->request_remove();
			g1_remove_man.process_requests();
			}
		}
	return 0;
	}

li_automatic_add_function(g1_remove_all_paths,"remove_all_paths");

li_object *g1_insert_path_object(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  g1_object_class *olist[G1_MAX_OBJECTS];
  int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;//,j;
  g1_path_object_class *p1=0, *p2=0;

  for (i=0; i<t; i++)
  {
    if (olist[i]->get_flag(g1_object_class::SELECTED))
    {
      if (!p1)
        p1=g1_path_object_class::cast(olist[i]);
      else if (!p2)
        p2=g1_path_object_class::cast(olist[i]);
      else
		  {
		  i4_message_box("Insert Path Object","Must have exaclty two path nodes selected.",MSG_OK);
          return 0;
		  }
    }
  }

  if (p1 && p2)
  {
    // determine if we should swap the two
    int t=p1->total_links(G1_ENEMY), swap=0;
    for (i=0; i<t; i++)
      if (p1->get_link(G1_ENEMY,i)==p2)
        swap=1;

    if (swap)
    {
      g1_path_object_class *t=p1;
      p1=p2;
      p2=p1;
    }

    g1_path_object_class *newp;
    newp=(g1_path_object_class *)g1_create_object(g1_get_object_type(path_object_type.get()));
    if (newp)


    if (!p1->remove_link(G1_ALLY, p2) ||
        !p2->remove_link(G1_ENEMY, p1))
		{
		i4_message_box("Insert Path Object","The two selected nodes aren't adjacent.",MSG_OK);
        return 0;
		}
      

    p1->add_link(G1_ALLY, newp);
    p2->add_link(G1_ENEMY, newp);
    newp->add_link(G1_ENEMY, p1);
    newp->add_link(G1_ALLY, p2);

    newp->x=(p1->x + p2->x)/2.0f;
    newp->y=(p1->y + p2->y)/2.0f;
    newp->h=(p1->h + p2->h)/2.0f;
    newp->grab_old();

    g1_player_man.get(newp->player_num)->add_object(newp->global_id);
    newp->occupy_location();
    
  }
  else
	  i4_message_box("Insert Path Node","Must have exactly two adjacent nodes selected",MSG_OK);

  return 0;
}

li_automatic_add_function(g1_unjoin_objects, "unjoin_path_ends");
li_automatic_add_function(g1_join_objects, "join_path_ends");
li_automatic_add_function(g1_insert_path_object, "insert_path_object");


static float move_v=0;
static void move_objs()
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (int i=0; i<t; i++)
  {
    if (olist[i]->get_flag(g1_object_class::SELECTED))
    {
      olist[i]->unoccupy_location();
      olist[i]->h+=move_v;
      olist[i]->grab_old();
      olist[i]->occupy_location();
    }
  }
  li_call("redraw");
}



li_object *g1_move_object_up_start(li_object *o, li_environment *env)
{
  move_v+=0.05f;
  move_objs();
  return 0;
}

li_object *g1_move_object_up_end(li_object *o, li_environment *env)
{
  move_v=0;
  return 0;
}


li_object *g1_move_object_down_start(li_object *o, li_environment *env)
{
  move_v-=0.05f;
  move_objs();
  return 0;
}

li_object *g1_move_object_down_end(li_object *o, li_environment *env)
{
  move_v=0;
  return 0;
}


li_automatic_add_function(g1_move_object_up_start, "move_selected_up");
li_automatic_add_function(g1_move_object_up_end, "-move_selected_up");
li_automatic_add_function(g1_move_object_down_start, "move_selected_down");
li_automatic_add_function(g1_move_object_down_end, "-move_selected_down");

// mode/e_team.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



static li_object *set_team(int num)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);

  for (int i=0; i<t; i++)
    if (olist[i]->selected())
      olist[i]->change_player_num(num);


  g1_edit_state.current_team=num;

  li_call("redraw");
    
  return 0;
}


static li_object *team_0(li_object *o, li_environment *env) { return set_team(0); }
static li_object *team_1(li_object *o, li_environment *env) { return set_team(1); }
static li_object *team_2(li_object *o, li_environment *env) { return set_team(2); }
static li_object *team_3(li_object *o, li_environment *env) { return set_team(3); }
static li_object *team_4(li_object *o, li_environment *env) { return set_team(4); }


static li_object *tint_none(li_object *o, li_environment *env)
{
  g1_tint=G1_TINT_OFF;
  li_call("redraw");
  return 0;
}

static li_object *tint_polys(li_object *o, li_environment *env)
{
  g1_tint=G1_TINT_POLYS;
  li_call("redraw");
  return 0;
}

static li_object *tint_all(li_object *o, li_environment *env)
{
  g1_tint=G1_TINT_ALL;
  li_call("redraw");
  return 0;
}


static int team_editing;
static li_string_class_member ai("ai");

LI_HEADER(set_local_player)
	{
	li_int *i=new li_int(g1_player_man.local_player);
	g1_player_man.local_player=li_int::get(li_first(o,env),env)->value();
	return i;
	}
li_object *g1_set_player(li_object *o, li_environment *env)
{
  if (!o) //the window was closed using cancel. 
      return 0;
  g1_player_info_class *p=g1_player_man.get(team_editing);
  p->vars=li_car(o, env);
  
  li_class_context c(li_class::get(p->vars.get(),env));
  if ((strcmp(ai(),"human")==0)&&g1_player_man.local_player!=team_editing)
	  {
	  //if (i4_message_box("Setting human player","Would you like to make this player the local player?",MSG_YES|MSG_NO)==MSG_YES)
	  //Not setting this doesn't work.
		  g1_player_man.local_player=team_editing;
	  }
  g1_team_api_class *newai=g1_create_ai(ai(),0);
  p->set_ai(newai);
  newai->init();//we can immediatelly do this since we exspect the map to be loaded
  return 0;
} 


static li_object *edit_team(int num)
{
  team_editing=num;
  char buf[100];
  i4_os_string(*(g1_player_man.get(num)->get_ai()->ai_name()),buf,100);
  
  //be shure that the display name matches the actually set ai.
  li_class::get(g1_player_man.get(num)->vars.get(),0)->set(ai,
	  new li_string(buf));
  li_create_dialog("Player Vars", g1_player_man.get(num)->vars.get(),
                   0, g1_set_player);
  return 0;
}

li_object *edit_team_0(li_object *o, li_environment *env) { return edit_team(0); }
li_object *edit_team_1(li_object *o, li_environment *env) { return edit_team(1); }
li_object *edit_team_2(li_object *o, li_environment *env) { return edit_team(2); }
li_object *edit_team_3(li_object *o, li_environment *env) { return edit_team(3); }
li_object *edit_team_4(li_object *o, li_environment *env) { return edit_team(4); }


li_automatic_add_function(team_0, "team_0");
li_automatic_add_function(team_1, "team_1");
li_automatic_add_function(team_2, "team_2");
li_automatic_add_function(team_3, "team_3");
li_automatic_add_function(team_4, "team_4");

li_automatic_add_function(edit_team_0, "edit_team_0");
li_automatic_add_function(edit_team_1, "edit_team_1");
li_automatic_add_function(edit_team_2, "edit_team_2");
li_automatic_add_function(edit_team_3, "edit_team_3");
li_automatic_add_function(edit_team_4, "edit_team_4");

li_automatic_add_function(tint_none, "tint_none");
li_automatic_add_function(tint_polys, "tint_polys");
li_automatic_add_function(tint_all, "tint_all");

li_automatic_add_function(li_set_local_player,"set_local_player");
// mode/e_tile.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


g1_tile_params g1_e_tile;

void g1_tile_mode::show_focus()
{
  if (active() && !focus_visible)
  {
    switch (g1_e_tile.get_minor_mode())
    {
      case g1_tile_params::FILL :
      case g1_tile_params::PLACE :
        c->save_and_put_cell(); 
        break;      
    }

    focus_visible=i4_T;
  }
}


void g1_tile_mode::hide_focus()
{
  if (active() && focus_visible)
  {
    switch (g1_e_tile.get_minor_mode())
    {
      case g1_tile_params::FILL :
      case g1_tile_params::PLACE :
        c->restore_cell(); 
        break;      
    }

    focus_visible=i4_F;
  }
}
void g1_tile_mode::mouse_down()
{
	if (g1_e_tile.get_minor_mode()==g1_tile_params::FILL)
	{
		g1_get_map()->mark_for_recalc(G1_RECALC_WATER_VERTS);
		c->fill_map();
	}
	else if (g1_e_tile.get_minor_mode()==g1_tile_params::PLACE)
	{
		g1_get_map()->mark_for_recalc(G1_RECALC_WATER_VERTS);
		c->change_map_cell();
	}
	else if (g1_e_tile.get_minor_mode()==g1_tile_params::INFO)
	{
		i4_const_str msg("Position (%i,%i), Texture %S rotated by %i deg%s. Characteristics: "
			"%s%sfriction %f, damage %i");
		g1_map_cell_class *cell= c->get_map()->cell((w16)c->cell_x,(w16)c->cell_y);
		i4_const_str *texname=0;
		//It seems we don't need the remap here
		//texname=g1_tile_man.get_name_from_tile(g1_tile_man.get_remap(cell->type));
		texname=g1_tile_man.get_name_from_tile(cell->type);
		g1_tile_class* tile=g1_tile_man.get(cell->type);
		i4_str *m=msg.sprintf(500,(w32)c->cell_x,(w32)c->cell_y,texname,
			(w32)cell->get_rotation()*90,cell->mirrored()?" and mirrored":"",
			(tile->flags&g1_tile_class::BLOCKING)?"blocking, ":"",
			(tile->flags&g1_tile_class::WAVE)?"wave, ":"",
			tile->friction_fraction,(w32)(tile->damage));
		//i4_warning(m);
		i4_message_box("Tile Info",*m,MSG_OK);
		delete texname;
		delete m;
		return; //don't pass on to parent, since that event is outdated when 
		//this method returns.
	}

	g1_mode_handler::mouse_down();

}

void g1_tile_mode::mouse_move(sw32 mx, sw32 my)
{
  
  switch (g1_e_tile.get_minor_mode())
  {
	case g1_tile_params::INFO :
    case g1_tile_params::FILL :
    case g1_tile_params::PLACE :
    {
      i4_float gx,gy, dx,dy;
      if (!c->view_to_game(mx,my, gx,gy, dx,dy))
        return;

      if (gx<0 || gy<0 ||  gx>=c->get_map()->width() || gy>=c->get_map()->height())
        return;

      if ((w32)gx!=c->cell_x ||  (w32)gy!=c->cell_y)
      {
        hide_focus();
        c->cell_x=(w32)gx;
        c->cell_y=(w32)gy;     
        show_focus();

        // if the mouse button is down, replaced the saved cell
        if (mouse_down_flag&&g1_e_tile.get_minor_mode()!=g1_tile_params::INFO)
          c->change_map_cell();
      }     
    } break;        
  }

  g1_mode_handler::mouse_move(mx,my);

}


g1_mode_handler::state g1_tile_mode::current_state()
{
  w8 remap[]={ ROTATE, ZOOM, OTHER, OTHER, DRAG_SELECT, OTHER };
  I4_ASSERT(g1_e_tile.minor_mode<=sizeof(remap), "state too big");
  return (g1_mode_handler::state)remap[g1_e_tile.get_minor_mode()];
}

void g1_tile_mode::move_selected(i4_float xc, i4_float yc, i4_float zc,
                                 sw32 mouse_x, sw32 mouse_y)
{
  //vert_exact_z-=mouse_y/50.0f;
  double aequidist=((li_float*)li_call("get_height_aequidistance",0,0))->value();
  //int z_int=(sw32)(vert_exact_z/(aequidist*10));
  int z_diff=zc/aequidist;
  if (zc>0 && z_diff==0)
	z_diff=1;
  else if (zc<0 && z_diff==0)
	  z_diff=-1;
  c->move_selected_heights(z_diff);

}

i4_bool g1_tile_mode::select_object(sw32 mx, sw32 my, 
                                    i4_float &ox, i4_float &oy, i4_float &oz,
                                    select_modifier mod)
{
  

  sw32 x,y;
  g1_map_vertex_class *v=c->find_map_vertex(mx, my, x, y);

  if (mod!=FOR_CURSOR_HINT)
  {
    g1_editor_instance.unmark_all_selected_verts_for_undo_save();
    if (v)
      v->set_need_undo(i4_T);

    g1_editor_instance.mark_selected_verts_for_undo_save();
    g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);    
  }


  if (v)
  {
    ox=(float)x; oy=(float)y;
    oz=c->get_map()->vertex(x,y)->get_height();

    if (!v->is_selected() && mod==CLEAR_OLD_IF_NO_SELECTION)
    {
      c->clear_selected_verts();

      c->changed();
      c->refresh();
    }

    if (mod==CLEAR_OLD_IF_NO_SELECTION || mod==ADD_TO_OLD)
    {
      v->set_is_selected(i4_T);
      c->changed();
      c->refresh();

      move_pivot=v;
      vert_exact_z=c->get_map()->vertex(x,y)->get_height();
    }     


    return i4_T;
  }
  else
  {
    if (mod==CLEAR_OLD_IF_NO_SELECTION)
      c->clear_selected_verts();

    return i4_F;
  }
}

void g1_tile_mode::select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                          select_modifier mod)
{
  c->select_verts_in_area(x1,y1,x2,y2, mod);
}


void g1_tile_mode::key_press(i4_key_press_event_class *kev)
{
  switch (kev->key)
  {
    case '1' :
    {
      hide_focus();
      int lsize=g1_get_map()->width()*g1_get_map()->height();
      g1_map_cell_class *c=g1_get_map()->cell(0,0);

      for (int i=0; i<lsize; i++, c++)
      {
        int rot=c->get_rotation();
        int remap[4]={1,0,3,2};    
        rot=remap[rot];
        c->set_rotation((g1_rotation_type)rot);
      }
	  i4_warning("rotation = %d, mirrored = %d", c->get_rotation(), c->mirrored());

      show_focus();
    } break;

    case '3' :
    {
      hide_focus();

      g1_map_cell_class *cell=c->get_map()->cell((w16)c->cell_x, (w16)c->cell_y);

      g1_e_tile.set_cell_type(cell->type);
      g1_e_tile.set_cell_rotation(cell->get_rotation());
      g1_e_tile.set_mirrored(cell->mirrored());
      
      if (g1_e_tile.get_minor_mode()!=g1_tile_params::PLACE &&
          g1_e_tile.get_minor_mode()!=g1_tile_params::FILL)
        g1_e_tile.set_minor_mode(g1_tile_params::PLACE);

      i4_warning("rotation = %d, mirrored = %d", cell->get_rotation(), cell->mirrored());
      
      show_focus();
    } break;


    case '4':
    {
      hide_focus();
      g1_e_tile.set_mirrored((i4_bool)!g1_e_tile.get_mirrored());
	  i4_warning("New rotation = %d, mirrored = %d", g1_e_tile.get_cell_rotation(), g1_e_tile.get_mirrored());

      show_focus();
    } break;

    case '9' :
    {
      if (g1_e_tile.get_minor_mode()==g1_tile_params::PLACE)
      {
        if (g1_e_tile.get_cell_type()>0)
        {
          hide_focus();
          g1_e_tile.set_cell_type(g1_e_tile.get_cell_type()-1);
          show_focus();
        } 
      }

      if (g1_e_tile.get_minor_mode()==g1_tile_params::HEIGHT)
        c->move_selected_heights(-1);

    } break;

    case '0' :
    {
      if (g1_e_tile.get_minor_mode()==g1_tile_params::PLACE)
      {
        if ((w32)(g1_e_tile.get_cell_type()+1) < g1_tile_man.total())
        {
          hide_focus();
          g1_e_tile.set_cell_type(g1_e_tile.get_cell_type()+1);
          show_focus();
        } 
      }
      if (g1_e_tile.get_minor_mode()==g1_tile_params::HEIGHT)
        c->move_selected_heights(1);

    } break;

    
    case '2' :    
    {
      hide_focus();
      if (g1_e_tile.get_cell_rotation()==G1_ROTATE_270)
        g1_e_tile.set_cell_rotation(G1_ROTATE_0);
      else      
        g1_e_tile.set_cell_rotation((g1_rotation_type)(g1_e_tile.get_cell_rotation()+1));
      show_focus();
	  i4_warning("New rotation = %d, mirrored = %d", g1_e_tile.get_cell_rotation(), g1_e_tile.get_mirrored());


    } break;

  }
}

g1_tile_mode::g1_tile_mode(g1_controller_edit_class *c)
  : g1_mode_handler(c)
{
  c->cell_x=0;
  c->cell_y=0;
  focus_visible=i4_F;
}


g1_tile_params::g1_tile_params() 
{ 
  vert_move_snap=1;
  picker_mp_window=0;
  picker=0;

  minor_mode=ROTATE;
  current_cell_type=0;
  current_cell_rotation=G1_ROTATE_0;   
  mirrored=i4_F;
}


void g1_tile_params::refresh_picker()
{
  if (picker.get())
    picker->refresh();
}


void g1_tile_params::create_buttons(i4_parent_window_class *container)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"tROTATE", "tZOOM", "tPLACE", "tFILL", "tHEIGHT", "tINFO", 0 };
  w32 i=ROTATE;
  for (char **a=rn; *a; a++, i++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==minor_mode,
                          new g1_set_minor_mode_event("TILE",(w8)i));

  box->arrange_right_down();
  container->add_child(0,0, box);
}


void g1_tile_params::cleanup()
{
  if (mp_window.get())
  {
    get_style()->close_mp_window(mp_window.get());
    picker=0;
  }
  current_cell_rotation=G1_ROTATE_0;   
  mirrored=i4_F;
}

void g1_tile_params::open_picker()
{
  cleanup();

  if (g1_e_tile.allow_picker_creation())
  {
    picker=new g1_tile_picker_class(get_style(), &picker_info, 0,0);
    picker->create_windows();

    i4_parent_window_class *mpw;
    mpw=get_style()->create_mp_window(picker_info.win_x, picker_info.win_y,
                                      picker->width(), picker->height(),
                                      g1_ges("tile_pick_title"));

    mpw->add_child(0,0, picker.get());
    mp_window=mpw;

  }
}

/////////////////////////////////////
// DIALOGS DIRECTORY 
//////////////////////////////////////

// dialogs/scene.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



class g1_scene_change_order_event : public i4_object_message_event_class
{
public:
  sw32 o,n;
  g1_scene_change_order_event(void *object, 
                              w32 sub_type, 
                              w32 old_order,
                              w32 new_order)
    : i4_object_message_event_class(object, sub_type),
      o(old_order),
      n(new_order)            {}

  virtual dispatch_time when()  { return LATER; }
  virtual i4_event  *copy() { return new g1_scene_change_order_event(object, sub_type, o,n); }
};



class g1_delete_scene_event_class : public i4_object_message_event_class
{
public:
  sw32 sn;
  g1_delete_scene_event_class(void *object,
                              w32 sub_type,
                              sw32 scene_number)
    : i4_object_message_event_class(object, sub_type),
      sn(scene_number)
  {}
  virtual dispatch_time when()  { return LATER; }
  virtual i4_event  *copy() { return new g1_delete_scene_event_class(object, sub_type, sn); }
};




class g1_cut_scene_editor_class : public i4_parent_window_class
{
  sw32 scene_number, start_time;

  g1_cut_scene_class *cs;
  i4_graphical_style_class *style;
  i4_text_input_class *scene_number_input;
  i4_text_input_class *scene_name_input;
  i4_text_input_class *scene_wav_input;
  i4_event_handler_class *eh;

  i4_window_class *dialog_active;

public:
  enum { DELETE_ME,
         DELETE_YES,
         DELETE_NO };
  
  void name(char* buffer) { static_name(buffer,"cut_scene_editor"); }

  ~g1_cut_scene_editor_class()
  {
    if (dialog_active)
      delete dialog_active;
  }

  sw32 tbox_width() { return 50; }
  sw32 name_width() { return 200; }
  sw32 wav_width() { return 150; }

  void send_delete_me()
  {
    g1_delete_scene_event_class del(eh, g1_scene_editor_class::DELETE_SCENE, scene_number);
    i4_kernel.send_event(eh, &del);
  }

  void create_delete_confirm_window()
  {
    dialog_active=i4_create_yes_no_dialog(root_window(),
                            style,
                            g1_ges("delete_title"),
                            g1_ges("delete_message"),
                            g1_ges("yes"), g1_ges("no"),
                            this,
                            new i4_object_message_event_class(this, DELETE_YES),
                            new i4_object_message_event_class(this, DELETE_NO));    
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::OBJECT_MESSAGE)
    {
      CAST_PTR(oev, i4_object_message_event_class, ev);
    
      if (oev->object==this)
      {
        switch (oev->sub_type)
        {
          case DELETE_ME :
            create_delete_confirm_window(); break;
          case DELETE_NO :
            dialog_active=0;
            break;

          case DELETE_YES :
            dialog_active=0;
            send_delete_me();
            break;                        
        }
      }
      else if (oev->object==scene_number_input)
      {
        CAST_PTR(tev, i4_text_change_notify_event, ev);
        i4_const_str::iterator i=tev->new_text->begin();
        sw32 sn=i.read_number();
        if (sn!=scene_number)
        {
          g1_scene_change_order_event co(eh, g1_scene_editor_class::CHANGE_SCENE_NUMBER,
                                         scene_number, sn);
          i4_kernel.send_event(eh, &co);
        }
      }
      else if (oev->object==scene_name_input)
      {
        CAST_PTR(tev, i4_text_change_notify_event, ev);
        if (cs->name) delete cs->name;
        cs->name=new i4_str(*tev->new_text);
      }
      else if (oev->object==scene_wav_input)
      {
        CAST_PTR(tev, i4_text_change_notify_event, ev);
        if (cs->wave_file) delete cs->wave_file;
        cs->wave_file=new i4_str(*tev->new_text);
      }


    }
    else if (!dialog_active || ev->type()!=i4_event::MOUSE_BUTTON_DOWN)
      i4_parent_window_class::receive_event(ev);
  }

  g1_cut_scene_editor_class(g1_cut_scene_class *cs,                             
                            sw32 scene_number,
                            sw32 start_time,          // in frames
                            i4_graphical_style_class *style,
                            i4_event_handler_class *eh)
    : cs(cs),
      scene_number(scene_number),
      start_time(start_time),
      style(style),
      i4_parent_window_class(0,0),
      eh(eh)
  {
    dialog_active=0;

    w32 l,r,t,b,xon=0;
    style->get_in_deco_size(l,t,r,b);

    i4_font_class *f=style->font_hint->normal_font;
    //resize(300, f->largest_height()+t+b+2);

    i4_str *fn=g1_ges("frame_format").sprintf(10,scene_number);

    scene_number_input=new i4_text_input_class(style,*fn,24,4, this);
    add_child((short)xon,0, scene_number_input); xon+=scene_number_input->width();

    xon+=tbox_width()*2;

	//this shows the same as the next window, it's of no use.
    //i4_text_window_class *tw=new i4_text_window_class(*cs->name,style);
    //add_child((short)xon,0, tw);
	//xon+=tw->width();

    scene_name_input=new i4_text_input_class(style,*cs->name,name_width(), 100, this);
    add_child((short)xon, 0, scene_name_input);   xon+=scene_name_input->width();

    i4_str *s=cs->wave_file;
    if (!s)     
      scene_wav_input=new i4_text_input_class(style,g1_ges("null_string"),wav_width(), 100, this);
    else
      scene_wav_input=new i4_text_input_class(style,*s,wav_width(), 100, this);
    add_child((short)xon, 0, scene_wav_input);  xon+=scene_wav_input->width();

    // delete button
    i4_object_message_event_class *om;
    om=new i4_object_message_event_class(this, DELETE_ME);
    i4_image_class *del_icon=g1_editor_instance.delete_icon;
    i4_button_class *del=new i4_button_class(&g1_ges("delete_scene_help"),
                                             new i4_image_window_class(del_icon),
                                             style,
                                             new i4_event_reaction_class(this, om));
    del->set_popup(i4_T);
    add_child((short)xon, 0, del); xon+=del->width();

    resize((w16)xon,(w16)(f->largest_height()+t+b+2));

  }

  void draw_time_box(sw32 x, sw32 y, 
                     sw32 t, sw32 wid,
                     i4_draw_context_class &context)
                     
  {
    sw32 t_sec=t/G1_MOVIE_HZ;
    sw32 t_usec=(t-t_sec*G1_MOVIE_HZ)*60/G1_MOVIE_HZ;
    w32 l,r,top,b;

    style->get_in_deco_size(l,top,r,b);
    style->draw_in_deco(local_image, (short)x,(short)y, x+wid-1, height()-1, i4_F, context);
    local_image->bar((short)(x+l),(short)(y+top), (short)(x+wid-r), (short)(y+height()-b), 
                     style->color_hint->window.passive.medium,
                     context);

    i4_str *s=g1_ges("time_format").sprintf(30, t_sec, t_usec);
    i4_font_class *f=style->font_hint->normal_font;

    f->set_color(style->color_hint->text_foreground);
    f->put_string(local_image, (short)(x+l), (short)(y+top+1),  *s, context);
    delete s;
  }


  void parent_draw(i4_draw_context_class &context)
  {
    local_image->clear(style->color_hint->window.passive.medium, context);

    draw_time_box(scene_number_input->width(), 0, start_time, tbox_width(), context);
    draw_time_box(scene_number_input->width()+tbox_width(), 0, cs->total_frames(),
                  tbox_width(), context);       
  }

};



void g1_scene_editor_class::show(i4_parent_window_class *parent_window,
                                 g1_movie_flow_class *_movie,
                                 i4_graphical_style_class *_style)
                                 
{
  movie=_movie;
  style=_style;

  if (parent.get())
    hide();

  i4_color_window_class *holder=new i4_color_window_class(0,0,0, style);

  w32 t=0;
  i4_window_class *addt=new i4_text_window_class(g1_ges("new"), style);
  i4_event_reaction_class *add_reaction;
  i4_event *add_event=new i4_object_message_event_class(this, ADD_SCENE);
  add_reaction=new i4_event_reaction_class(this, add_event);
  i4_button_class *addb=new i4_button_class(0, addt, style, add_reaction);
  addb->set_popup(i4_T);
  holder->add_child(0,0, addb);


  for (w32 i=0; i<movie->t_cut_scenes; i++)
  {    
    g1_cut_scene_editor_class *ce=new g1_cut_scene_editor_class(movie->set[i], i+1,t, style, this);
    t+=movie->set[i]->total_frames();

    holder->add_child(0,0, ce);
  }

  holder->arrange_right_down();
  holder->resize_to_fit_children();

  w32 w=holder->width(), h=holder->height();

  i4_event_reaction_class *re;
  re=new i4_event_reaction_class(this, new i4_object_message_event_class(this, WINDOW_CLOSED));
  parent=style->create_mp_window((short)wx, (short)wy, (w16)w,(w16)h, g1_ges("scene_win_title"), re);

  holder->transfer_children(parent.get(), 0,0);
  delete holder;
                            
}

void g1_scene_editor_class::hide()
{
  if (parent.get())
  {
    if (parent->get_parent())
    {
      wx=parent->get_parent()->x();
      wy=parent->get_parent()->y();
    }

    style->close_mp_window(parent.get());

    parent=0;
  }
}


void g1_scene_editor_class::last_scene()
{
 
  if (movie->t_cut_scenes==1)
    movie->set_scene(0);
  else
  {
    if (movie->get_scene()!=0)
      movie->set_scene(movie->get_scene()-1);
  }
}

void g1_scene_editor_class::receive_event(i4_event *ev)
{
  CAST_PTR(oev, i4_object_message_event_class, ev);
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    if (oev->object==this)
    {
      switch (oev->sub_type)
      {
        case ADD_SCENE :
        {
          g1_editor_instance.add_undo(G1_MAP_MOVIE);

          i4_parent_window_class *parent_window=0;

          if (parent.get() && parent->get_parent() && parent->get_parent()->get_parent())
            parent_window=parent->get_parent()->get_parent();

          hide();
          g1_cut_scene_class *cs=movie->add_cut_scene(g1_ges("new_scene"));
          show(parent_window, movie, style);

          g1_editor_instance.changed();
        } break;

        case WINDOW_CLOSED :
        {
          parent=0;
        } break;

        case CHANGE_SCENE_NUMBER :
        {
          g1_editor_instance.add_undo(G1_MAP_MOVIE);

          i4_parent_window_class *parent_window=0;
          if (parent.get() && parent->get_parent() && parent->get_parent()->get_parent())
            parent_window=parent->get_parent()->get_parent();

          hide();

          CAST_PTR(co, g1_scene_change_order_event, ev);
          co->n--;
          co->o--;

          if ( (w32)co->n >= movie->t_cut_scenes )
            co->n=movie->t_cut_scenes-1;
          else if (co->n<0)
            co->n=0; 

          sw32 i;
          g1_cut_scene_class *c=movie->set[co->o];
          for (i=co->o; i<(sw32)movie->t_cut_scenes-1; i++)
            movie->set[i]=movie->set[i+1];

          for (i=movie->t_cut_scenes-1; i>co->n; i--)
            movie->set[i]=movie->set[i-1];
          
          movie->set[co->n]=c;

          show(parent_window, movie, style);

          g1_editor_instance.changed();
        } break;

        case DELETE_SCENE :
        {
          g1_editor_instance.add_undo(G1_MAP_MOVIE);
    
          i4_parent_window_class *parent_window=0;
          if (parent.get() && parent->get_parent() && parent->get_parent()->get_parent())
            parent_window=parent->get_parent()->get_parent();

          hide();

          CAST_PTR(dev, g1_delete_scene_event_class, ev);
          dev->sn--;

          last_scene();
          g1_cut_scene_class *c=movie->set[dev->sn];
          
          for (w32 i=dev->sn; i<movie->t_cut_scenes-1; i++)
            movie->set[i]=movie->set[i+1];

          movie->t_cut_scenes--;

          delete c;

          show(parent_window, movie, style);

          g1_editor_instance.changed();
        } break;

      }
    }
  }
}
// dialogs/d_light.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



static i4_parent_window_class *g1_create_slider(i4_event_handler_class *notify,
                                                w32 milli_delay,
                                                w32 width,
                                                w32 start,
                                                const i4_const_str name,
                                                i4_graphical_style_class *style)
{
  i4_slider_class *s=new i4_slider_class(width, start, notify, milli_delay, style);
  i4_text_window_class *t=new i4_text_window_class(name, style);

  i4_parent_window_class *c=new i4_color_window_class((w16)width, s->height()+t->height(),
                                                      style->color_hint->neutral(),
                                                      style);
  c->add_child(0,0,s);
  c->add_child(0,s->height(), t);
  c->resize_to_fit_children();

  return c;

}
                        

class g1_bulb_ewin : public i4_color_window_class
{
  i4_parent_window_class *r,*g,*b, *c3;
  i4_float o_r,o_g,o_b,o_c3;

  w32 selected_objects[G1_MAX_OBJECTS];
  int t_sel;

public:


  void name(char* buffer) { static_name(buffer,"bulb edit"); }

  g1_bulb_ewin(i4_graphical_style_class *style,
               g1_object_class *def)

    : i4_color_window_class(0,0,style->color_hint->neutral(), style)
  {
    g1_light_object_class *d=g1_light_object_class::cast(def);
    if (d)
    {
      o_r=d->r;
      o_g=d->g;
      o_b=d->b;
      o_c3=d->c3;   

      r=g1_create_slider(this, 1000, 200, (sw32)(o_r*200), g1_ges("red"), style);
      add_child(0,0,r);

      g=g1_create_slider(this, 1000, 200, (sw32)(o_g*200), g1_ges("green"), style);
      add_child(0,r->height()+5,g);

      b=g1_create_slider(this, 1000, 200, (sw32)(o_b*200), g1_ges("blue"), style);
      add_child(0,r->height() + g->height() + 10,b);

      c3=g1_create_slider(this, 1000, 200, (sw32)((1.0-(o_c3-0.05)/(1.0-0.05))*200),
                          g1_ges("brightness"), style);
      add_child(0,r->height() + g->height() + b->height() + 20, c3);

      resize_to_fit_children();
    }

    t_sel=g1_get_map()->make_selected_objects_list(selected_objects, G1_MAX_OBJECTS);
  }

  void update_lights()
  {
    g1_map_class *m=g1_editor_instance.get_map();
    
   
    g1_editor_instance.add_undo(G1_MAP_OBJECTS | G1_MAP_CELLS);

    for (int i=0; i<t_sel; i++)
    {
      if (g1_global_id.check_id(selected_objects[i]))
      {
        g1_light_object_class *l;

        if ((l=g1_light_object_class::cast(g1_global_id.get(selected_objects[i])))!=0)
          l->setup(l->x, l->y, l->y, o_r, o_g, o_b, 0.01f, 0.25f, o_c3);
      } 
    }

    g1_editor_instance.changed();

    li_call("redraw");
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::OBJECT_MESSAGE)
    {
      CAST_PTR(sev, i4_slider_event, ev);
      if (r->isa_child((i4_window_class *)sev->object))
        o_r=sev->x/(i4_float)sev->divisor;
      else if (g->isa_child((i4_window_class *)sev->object))
        o_g=sev->x/(i4_float)sev->divisor;
      else if (b->isa_child((i4_window_class *)sev->object))
        o_b=sev->x/(i4_float)sev->divisor;
      else if (c3->isa_child((i4_window_class *)sev->object))
        o_c3=1.0f-(sev->x/(i4_float)sev->divisor)*(1.0f-0.05f);
      else i4_parent_window_class::receive_event(ev);
      
      update_lights();
    }
    else i4_parent_window_class::receive_event(ev);
   
  }
};

i4_parent_window_class *g1_create_bulb_edit(i4_graphical_style_class *style,
                                            g1_object_class *def)
{
  return new g1_bulb_ewin(style, def);

}

// dialogs/d_time.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



class g1_time_edit_window : public i4_color_window_class
{
  i4_text_input_class *si, *mi;
  w32 t_sec, t_msec;

public:
  void name(char* buffer) { static_name(buffer,"time edit"); }

  g1_time_edit_window(i4_graphical_style_class *style, w32 cur_frame)
    : i4_color_window_class(0,0,style->color_hint->neutral(), style)
  {
    i4_text_window_class *st,*mt;
    i4_str *sec_str, *msec_str;

    st=new i4_text_window_class(g1_ges("sec"),style);
    mt=new i4_text_window_class(g1_ges("msec"),style);

    t_sec=cur_frame/G1_MOVIE_HZ;
    t_msec=(cur_frame-t_sec*G1_MOVIE_HZ)*60/G1_MOVIE_HZ;


    sec_str=g1_ges("sec_fmt").sprintf(10,t_sec);
    si=new i4_text_input_class(style, *sec_str, 40, 8, this);

    msec_str=g1_ges("msec_fmt").sprintf(10,t_msec);
    mi=new i4_text_input_class(style, *msec_str, 40, 8, this);

    delete sec_str;
    delete msec_str;

    add_child(0,3,st);
    add_child(st->width(),0,si);

    add_child(st->width() + si->width(),3,mt);
    add_child(st->width() + si->width() + mt->width(),0,mi);

    resize_to_fit_children();
  }

  void change_time()
  {      
    i4_spline_class *s[g1_controller_edit_class::MAX_SPLINE_EDIT];
    int t=g1_editor_instance.get_current_splines(s,g1_controller_edit_class::MAX_SPLINE_EDIT);
    
    int nf=t_sec*G1_MOVIE_HZ+t_msec*G1_MOVIE_HZ/60;

    for (int i=0; i<t; i++)
    {
      i4_spline_class::point *sp=s[i]->begin(), *last=0;
      for (;sp;sp=sp->next)
      {
        if (sp->selected)
          if( sp->frame < (w32)(nf || (!last || last->frame< (w32)nf)) )
          {
            int advance=nf-sp->frame;

            for (i4_spline_class::point *q=sp; q; q=q->next)
              q->frame+=advance;
          }            
        last=sp;
      }
    }     
  }

  void receive_event(i4_event *ev)
  {
    CAST_PTR(tev, i4_text_change_notify_event, ev);

    if (tev->type()==i4_event::OBJECT_MESSAGE)
    {
      if (tev->object==si)
      {
        i4_const_str::iterator i=tev->new_text->begin();
        t_sec=i.read_number();
        change_time();
      }

      if (tev->object==mi)
      {
        i4_const_str::iterator i=tev->new_text->begin();
        t_msec=i.read_number();
        change_time();
      }
    }
    i4_color_window_class::receive_event(ev);
  }
};

i4_parent_window_class *g1_create_time_edit_window(i4_graphical_style_class *style,
                                                   w32 cur_frame)
{
  return new g1_time_edit_window(style, cur_frame);
}

// dialogs/debug_win.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



//The output should also be visible if not a debug build
//#ifdef _DEBUG

class g1_debug_window_class : public i4_parent_window_class
{
  i4_color fore,back;
  char *screen;
  sw32 cx, cy, starty;
  sw32 cols, lines, col_width, line_height;

  i4_graphical_style_class *style;
  i4_bool need_clear;

public:
  void name(char* buffer) { static_name(buffer,"debug_window"); }

  g1_debug_window_class(i4_graphical_style_class *style,
			i4_color text_foreground,
			i4_color text_background,
			w16 width, w16 height, // in pixels
			sw32 starting_line = 0);
  virtual ~g1_debug_window_class();
  virtual void receive_event(i4_event *ev);
  void newline();
  void putch(char ch);
  void puts(char *string);
  void putsxy(int x, int y, char *string);

  virtual void parent_draw(i4_draw_context_class &context);
};

i4_event_handler_reference_class<g1_debug_window_class> debug_window;
i4_event_handler_reference_class<i4_parent_window_class> debug_mp;
i4_debug_message_device_class i4_debug_dev;

g1_debug_window_class::g1_debug_window_class(i4_graphical_style_class *style,
					     i4_color text_foreground,
					     i4_color text_background,
					     w16 width, w16 height,
					     sw32 starting_line)
  : i4_parent_window_class(width, height),
    style(style),
    fore(text_foreground),
    back(text_background),
    starty(starting_line),
    need_clear(i4_T)
{
  i4_font_class *fnt=style->font_hint->small_font;

  col_width   = fnt->largest_height()+1;
  line_height = fnt->largest_width()+1;
  cols = width/col_width;
  lines = height/line_height;
  starty=lines-1; //3 lines means line 0 1 2
  screen = (char *)I4_MALLOC(cols*lines, "debug screen");
  memset(screen, 0, cols*lines*sizeof(char));
  cx = 0;
  cy = starty;
  i4_kernel.request_events(this,i4_device_class::FLAG_USER_MESSAGE);
}

void g1_debug_window_class::receive_event(i4_event *ev)
	{
	if (ev->type()==i4_event::USER_MESSAGE)
		{
		CAST_PTR(mev,g1_send_debug_message_class,ev);
		if (mev->sub_type==G1_DEBUG_SEND_EVENT)
			{
			puts(mev->msg);
			return;
			}
		}
	i4_parent_window_class::receive_event(ev);
	}

g1_debug_window_class::~g1_debug_window_class()
	{
	i4_kernel.unrequest_events(this,i4_device_class::FLAG_USER_MESSAGE);
	}

void g1_debug_window_class::newline()
{
  cx = 0;
  if (++cy>=lines)
	  {
	  memmove(screen,screen+cols,(lines-1)*cols);//scroll window up
	  memset(screen+(cols*(lines-1)),0,cols);
	  cy = starty;
	  }

  if (!redraw_flag)
    request_redraw();
}

void g1_debug_window_class::putch(char ch)
{
  I4_ASSERT(cy>=0 && cy<lines && cx>=0 && cx<cols, 
	    "g1_debug_window_class::putch - Cursor out of bounds");

  switch (ch) {
  case '\n': 
    newline(); 
    break;
  default :   
    screen[cy*cols+cx] = ch;
    if (++cx>=cols) 
      newline();
    break;
  }

  if (!redraw_flag)
    request_redraw();
}

void g1_debug_window_class::puts(char *s)
{
  while (*s)
    putch(*s++);
}

void g1_debug_window_class::putsxy(int x, int y, char *s)
{
  while (*s && x<cols)
    if (x<cols)
      screen[y*cols + x++] = *s++;

  if (!redraw_flag)
    request_redraw();
}

void g1_debug_window_class::parent_draw(i4_draw_context_class &context)
{
  i4_font_class *fnt=style->font_hint->small_font;

  if (!undrawn_area.empty())
    need_clear=i4_T;

  need_clear=i4_F;
    
  local_image->clear(back, context);
  fnt->set_color(fore);

  char *p;
  for (sw16 y=0; y<lines; y++) {
    p = screen + cols*y;
    for (sw16 x=0; x<cols; x++)
      fnt->put_character(local_image, (sw16)(x*col_width), (sw16)(y*line_height), p[x], context);
  }
  i4_parent_window_class::parent_draw(context);
}

int g1_debug_printf(const char *s, ... )
//{{{
{
  char buf[1000];
  va_list arg;
  int ret=0;
  static int recursion=0;//this function is subject to recurse, but mustn't.
  if (recursion==0)
	  {
	//if (debug_window.get()==0)
	//	return 0;
	recursion=1;

	va_start(arg,s);
	ret = vsprintf(buf, s, arg);
	va_end(arg);
	i4_debug_dev.broadcast_message(buf);
	//debug_window->puts(buf);
	recursion=0;
	  }
  return ret;
}

class i4_debug_device_adder_class : public i4_init_class
{
  public :
  void init() 
  { 
    i4_kernel.add_device(&i4_debug_dev);
  }
} i4_debug_device_adder_instance;

void i4_debug_message_device_class::broadcast_message(char *message)
	{
	g1_send_debug_message_class msg(message);
	send_event_to_agents(&msg,FLAG_USER_MESSAGE);
	}

void i4_debug_message_device_class::broadcast_message(i4_const_str &message)
	{
	g1_send_debug_message_class msg(message);
	send_event_to_agents(&msg,FLAG_USER_MESSAGE);
	}

int g1_debug_printfxy(int x,int y, const char *s, ...)
{//do not use this function
  char buf[256];
  va_list arg;
  int ret=0;
  static int recursion=0;
  if (recursion==0)
	  {
	if (debug_window.get()==0)
		return 0;
	recursion=1;
	va_start(arg,s);
	ret = vsprintf(buf, s, arg);
	va_end(arg);
	debug_window->putsxy(x,y,buf);
	  }
  return ret;
}

int g1_debug_open(i4_graphical_style_class *style,
		  i4_parent_window_class *parent, 
		  const i4_const_str &title,
		  int status_lines,
		  i4_event_reaction_class *debug_closed)
{
  sw32 x=0, y=0, w = 600, h=60;
	
  if (debug_mp.get())
    return 0;

  debug_mp=style->create_mp_window((w16)x, (w16)y, (sw16)w, (sw16)h, title, debug_closed);
  debug_window = new g1_debug_window_class(style, 0xffffff, 0, (w16) w, (w16) h, status_lines);
  debug_mp->add_child(0,0,debug_window.get());
  
  return 1;
}

int g1_debug_close(i4_graphical_style_class *style)
{
  if (debug_mp.get()==0)
    return 0;

  style->close_mp_window(debug_mp.get());

  return 1;
}
//}}}
//#endif

// dialogs/e_time.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



class g1_time_show : public i4_window_class
{
  i4_graphical_style_class *style;
  i4_event_handler_reference_class<g1_time_line> tl;
public:
  g1_time_show(i4_graphical_style_class *style,
               g1_time_line *tl) 
    : i4_window_class(style->font_hint->normal_font->largest_width()*6,
                      style->font_hint->normal_font->largest_height()),
      style(style)
  {
    tl=tl;
  }

  void draw(i4_draw_context_class &context)
  {
    style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);

    if (!tl.get()) return;

    i4_font_class *f=style->font_hint->normal_font;
    f->set_color(style->color_hint->text_foreground);

    i4_float t=tl->current_frame()/(i4_float)(G1_MOVIE_HZ);
    sw32 sec=(sw32)t;
    sw32 msec=(sw32)((t-sec)*100);

    i4_str *out=g1_editor_instance.get_editor_string("time_format").sprintf(20, sec, msec);
    f->put_string(local_image, 0,0, *out, context);
    delete out;
  }

  void name(char* buffer) { static_name(buffer,"time show"); }
};

class g1_frame_show : public i4_text_input_class
{
  i4_event_handler_reference_class<g1_time_line> tl;
public:
  g1_frame_show(i4_graphical_style_class *style,
               g1_time_line *tl)
    : i4_text_input_class(style,
                          g1_editor_instance.get_editor_string("0"),
                          50,
                          8)
  {
    tl=tl;
  }

  virtual void become_unactive()
  {
    i4_const_str::iterator i=st->begin();
    w32 v=i.read_number();    
    i4_text_input_class::become_unactive();
  }

  void receive_event(i4_event *ev)
  {
    if (!tl.get()) return;

    CAST_PTR(kev, i4_key_press_event_class, ev);
    if (ev->type()==i4_event::KEY_PRESS && kev->key==I4_ENTER)
    {
      become_unactive();

      i4_const_str::iterator i=st->begin();
      sw32 x=i.read_number();

      if (tl->selected_spline() && x>0)
      {
        g1_editor_instance.add_undo(G1_MAP_MOVIE);

        i4_spline_class::point *p;
        w32 frame=tl->current_frame();
        p=tl->selected_spline()->get_control_point_previous_to_frame(frame);

        sw32 add=x-(p->next->frame-p->frame);
        p=p->next;


        for (;p;p=p->next)
          p->frame+=add;

        g1_editor_instance.changed();

       
      }


    }
    else i4_text_input_class::receive_event(ev);
  }
    
  void update()
  {
    i4_str *st;
    if (!tl.get()) return;
    if (tl->selected_spline())
    {
      i4_spline_class::point *p;
      w32 frame=tl->current_frame();
      p=tl->selected_spline()->get_control_point_previous_to_frame(frame);

      if (p && p->next)
        st=g1_editor_instance.get_editor_string("frame_format").sprintf(20,
                                                                        p->next->frame-p->frame);
      else
        st=new i4_str(g1_editor_instance.get_editor_string("no_next"));
    }
    else
      st=new i4_str(g1_editor_instance.get_editor_string("not_sel"));

    change_text(*st);
    delete st;
  }
    
  void name(char* buffer) { static_name(buffer,"time show"); }
};

class g1_time_scroller : public i4_parent_window_class
{
  i4_event_handler_reference_class<g1_time_line> tl;
  i4_graphical_style_class *style;
  sw32 mx,my;
  i4_bool mouse_down;

public:
  void name(char* buffer) { static_name(buffer,"time sroller"); }

  g1_time_scroller(w16 w, w16 h, 
                   i4_graphical_style_class *style,
                   g1_time_line *tl) 
    : i4_parent_window_class(w,h),
      style(style)
  {
    tl=tl;
    mouse_down=i4_F;
  }
    
  void parent_draw(i4_draw_context_class &context)
  {
    style->draw_in_deco(local_image, 0,0, width()-1, height()-1, i4_F, context);

    w32 l,r,t,b;
    style->get_in_deco_size(l,r,t,b);
    local_image->bar((short)l,(short)r, (short)(width()-r),(short)(height()-t), style->color_hint->window.active.medium, context);

    local_image->line((short)l,(short)(height()/2),(short)(width()-r), (short)(height()/2), 0xffffffff, context);
    
    if (!tl.get()) return;
    int tf=tl->total_frames();
    if (tf)
    {
      w32 o=tl->current_frame()*(width()-l-r)/tf+l;
      local_image->line((short)o,(short)t,(short)o,(short)(height()-b), 0xffffffff, context);
    }
  }


  void mouse_change_time()
  {
    if (!tl.get()) return;

    w32 l,r,t,b;
    style->get_in_deco_size(l,r,t,b);
    if ( mx >= (sw32)(width()-r) )
      tl->set_current(tl->total_frames()-1, i4_T);
    else if (mx<(sw32)l)
      tl->set_current(0, i4_T);
    else
      tl->set_current( (mx-l)*tl->total_frames()/(width()-l-r), i4_T);
    request_redraw();

  }

  void g1_time_scroller::receive_event(i4_event *ev)
  {
   
    switch (ev->type())
    {
      case i4_event::MOUSE_MOVE :
      {       
        CAST_PTR(mev, i4_mouse_move_event_class, ev);
        mx=mev->x;
        my=mev->y;
        if (mouse_down)
          mouse_change_time();
      } break;

      case i4_event::MOUSE_BUTTON_DOWN :
      {
        CAST_PTR(mev, i4_mouse_button_down_event_class, ev);
        if (mev->but==i4_mouse_button_down_event_class::LEFT)
        {
          i4_window_request_mouse_grab_class grab(this);
          i4_kernel.send_event(parent,&grab);
          mouse_down=i4_T;
          mouse_change_time();
        }
      } break;

      case i4_event::MOUSE_BUTTON_UP :
      {
        CAST_PTR(mev, i4_mouse_button_up_event_class, ev);
        if (mev->but==i4_mouse_button_up_event_class::LEFT && mouse_down)
        {
          mouse_down=i4_F;
          i4_window_request_mouse_ungrab_class grab(this);
          i4_kernel.send_event(parent,&grab);
        }
      } break;
    }
  }
   
};



i4_button_class *g1_time_line::create_img_win(char *icon_res_name,
                                              w32 mess_id,
                                              i4_graphical_style_class *style)

{
  i4_image_class *im=i4_load_image(g1_editor_instance.get_editor_string(icon_res_name));
  I4_ASSERT(im,"icon missing");
  
  char help[30];
  sprintf(help,"%s_help",icon_res_name);

  i4_object_message_event_class *omes=new i4_object_message_event_class(this, mess_id);
  i4_event_reaction_class *press=new i4_event_reaction_class(this, omes);
  i4_button_class *b=new i4_button_class(&g1_ges(help),
                                         new i4_image_window_class(im, i4_T),
                                         style,
                                         press);
  return b;
}

void g1_time_line::create_time_win(i4_graphical_style_class *style)
{ 
  frame_show=new g1_frame_show(style, this);
  frame_show->update();

  i4_const_str next_str=g1_editor_instance.get_editor_string("next");
  i4_window_class *frame_text=new i4_text_window_class(next_str, style);

  sec_win=new g1_time_show(style, this);

  w32 w=frame_text->width()+10+frame_show->width();
  w32 h=frame_show->height() + 2 + sec_win->height();

  i4_deco_window_class *d=new i4_deco_window_class((w16)w,(w16)h, i4_F, style);  
  time_win=d;

  w32 x1=d->get_x1(), y1=d->get_y1();

  time_win->add_child((short)x1,(short)(y1+2), frame_text);
  time_win->add_child((short)(x1+frame_text->width()+5), (short)y1, frame_show.get());
  time_win->add_child((short)x1,(short)(y1+frame_show->height()+1), sec_win.get());
}

g1_time_line::~g1_time_line()
{
  if (g1_frame_change_notify)
  {
    delete g1_frame_change_notify;
    g1_frame_change_notify=0;
  }
  
  if (g1_scene_change_notify)
  {
    delete g1_scene_change_notify;
    g1_scene_change_notify=0;
  }

  if (g1_movie_stop_notify)
  {
    delete g1_movie_stop_notify;
    g1_movie_stop_notify=0;
  }

  if (scroller.get())
      delete scroller.get();
      
  if (time_win.get())
    delete time_win.get();

  if (bbox.get())
    delete bbox.get();

  if (last_scene.get())
    delete last_scene.get();

  if (next_scene.get())
    delete next_scene.get();

  if (scene_number_input.get())
    delete scene_number_input.get();
}

g1_time_line::g1_time_line(i4_parent_window_class *parent, 
                           i4_graphical_style_class *style,
                           g1_edit_state_class *state)
  : state(state)
{
  i4_object_message_event_class *o;
  o=new i4_object_message_event_class(this,FRAME_CHANGED);
  g1_frame_change_notify=new i4_event_reaction_class(this, o);
                                                          
  o=new i4_object_message_event_class(this,SCENE_CHANGED);                            
  g1_scene_change_notify=new i4_event_reaction_class(this, o);

  o=new i4_object_message_event_class(this,MOVIE_STOPPED);
  g1_movie_stop_notify=new i4_event_reaction_class(this, o);

  bbox=new i4_button_box_class(this, i4_F);
  

  i4_button_class *left, *right, *rewind, *fforward;
  left=create_img_win("e_left", LAST_TIME, style);  
  left->set_repeat_down(i4_T);
  left->set_popup(i4_T);

  right=create_img_win("e_right", NEXT_TIME, style);  
  right->set_repeat_down(i4_T);
  right->set_popup(i4_T);

  play=create_img_win("e_play", PLAY, style);
  i4_user_message_event_class *stop=new i4_user_message_event_class(G1_STOP_MOVIE);
  play->send.depress=new i4_event_reaction_class(i4_current_app, stop);
                                                 

  rewind=create_img_win("e_rewind", REWIND, style);
  fforward=create_img_win("e_fforward", FFORWARD, style);

  bbox->add_button(0,0,rewind);
  bbox->add_button(0,0,left);
  bbox->add_button(0,0,play.get());
  bbox->add_button(0,0,right);
  bbox->add_button(0,0,fforward);

  bbox->arrange_down_right();

  h=bbox->height();
  

  create_time_win(style);
  if (time_win->height()>h)
    h=time_win->height();

  last_scene=create_img_win("e_left", LAST_SCENE, style);  
  last_scene->set_repeat_down(i4_T);
  last_scene->set_popup(i4_T);

  next_scene=create_img_win("e_right", NEXT_SCENE, style);  
  next_scene->set_repeat_down(i4_T);
  next_scene->set_popup(i4_T);
  
  i4_const_str fmt=g1_editor_instance.get_editor_string("frame_format");
  w32 scene_d=current_movie() ? current_movie()->get_scene()+1 : 1;
  i4_str *scene_number=fmt.sprintf(25, scene_d);
  scene_number_input=new i4_text_input_class(style,*scene_number, 40, 8, this);
  delete scene_number;

  sw32 xon=0;
  parent->add_child((short)xon, parent->height()-last_scene->height()-1, last_scene.get());
  xon+=last_scene->width();

  parent->add_child((short)xon, parent->height()-scene_number_input->height()-1, 
                    scene_number_input.get());
  xon+=scene_number_input->width();

  parent->add_child((short)xon, parent->height()-next_scene->height()-1, next_scene.get());
  xon+=next_scene->width();


  scroller=new g1_time_scroller( (w16)(parent->width() - xon - bbox->width() - time_win->width()),
                                 (w16)h, style, this);
  

  parent->add_child((short)xon, parent->height()-scroller->height()-1, scroller.get());
  xon+=scroller->width();

  parent->add_child((short)xon, parent->height()-bbox->height()-1, bbox.get());
  xon+=bbox->width();

  parent->add_child((short)xon, parent->height()-time_win->height()-1, time_win.get());
}


void g1_time_line::update()
{
  scroller->request_redraw(i4_F);
  frame_show->update();
  sec_win->request_redraw(i4_F);
}

i4_spline_class *g1_time_line::selected_spline()
{
  return 0;
}


g1_movie_flow_class *g1_time_line::current_movie()
{
  g1_get_current_movie_event gm;
  i4_kernel.send_event(i4_current_app, &gm);
  return gm.mflow;
}

void g1_time_line::set_current(w32 frame, i4_bool stop_play)
{  
  g1_movie_flow_class *m=current_movie();
  if (m)
  {
    g1_editor_instance.add_undo(G1_MAP_MOVIE);

    m->set_frame(frame);

    i4_user_message_event_class c(G1_MAP_CHANGED);
    i4_kernel.send_event(i4_current_app, &c);
    update();   
    
    if (stop_play)
      play->do_depress();
  }
}


w32 g1_time_line::current_frame()
{
  g1_movie_flow_class *m=current_movie();
  if (m)
    return m->get_frame();
  else
    return 0;
}

w32 g1_time_line::total_frames()
{
  g1_movie_flow_class *m=current_movie();
  if (m && m->current())
    return m->current()->total_frames();
  else return 0;
}

void g1_time_line::update_scene()
{
  g1_movie_flow_class *m=current_movie();

  i4_const_str fmt=g1_editor_instance.get_editor_string("frame_format");
  i4_str *scene_number=fmt.sprintf(10, m->get_scene()+1);
  scene_number_input->change_text(*scene_number);
  delete scene_number;
}

void g1_time_line::set_current_scene(sw32 scene)
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  g1_movie_flow_class *m=current_movie();
  m->set_scene(scene);
  m->set_frame(0);

  update_scene();

  i4_user_message_event_class ch(G1_MAP_CHANGED);
  i4_kernel.send_event(i4_current_app, &ch);
}

void g1_time_line::unpress_play()
{
  if (play.get())
    play->do_depress();
}

void g1_time_line::receive_event(i4_event *ev)
{
  g1_movie_flow_class *m=current_movie();

  CAST_PTR(oev, i4_object_message_event_class, ev);

  if (!m || oev->type()!=i4_event::OBJECT_MESSAGE)
    return ;

  if (oev->object==this)
  {
    switch (oev->sub_type)
    {
      case LAST_TIME :
        if (m->get_frame())
          set_current(m->get_frame()-1, i4_T);
        break;

      case NEXT_TIME :
        if (m->current() && m->get_frame()+1<m->current()->total_frames())          
          set_current(m->get_frame()+1, i4_T);          
        break;

      case FFORWARD :
        if (m->current())
          set_current(m->current()->total_frames()-1, i4_T);
        break;

      case REWIND :
        set_current(0, i4_T);
        break;

      case PLAY :
      {
        i4_user_message_event_class m(G1_PLAY_MOVIE);
        i4_kernel.send_event(i4_current_app, &m);
      } break;

      case LAST_SCENE :      
        if (m->current() && m->get_scene())
         set_current_scene(m->get_scene()-1);
        break;

      case NEXT_SCENE :
        if (m->current() && m->get_scene()<m->t_cut_scenes-1)
         set_current_scene(m->get_scene()+1);
        break;        

      case SCENE_CHANGED :
        update_scene();
        break;

      case FRAME_CHANGED :
        update();
        break;

      case MOVIE_STOPPED :
        unpress_play();
        break;
    }
  } else if (oev->object==scene_number_input.get())
  {
    CAST_PTR(tc, i4_text_change_notify_event, ev);
    i4_const_str::iterator i=tc->new_text->begin();
    sw32 n=i.read_number()-1;
    if (n>=0 && ((w32)n < m->t_cut_scenes) )
      set_current_scene(n);
  }
}
// dialogs/obj_win.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



void g1_3d_object_window::do_idle()
{
  if (object.valid())
  {    
    if (!context_help_window.get())
    {
      i4_graphical_style_class *style=i4_current_app->get_style();
      i4_const_str help_str=i4gets((char *)object->name(),i4_F);
      if (!help_str.null())
        context_help_window=style->create_quick_context_help(x(), y()+height()+5, help_str);
      else
        context_help_window=style->create_quick_context_help(x(), y()+height()+5,
                                                         i4_const_str((char *)object->name()));
    }
  }
    
}

void g1_3d_object_window::set_object_type(g1_object_type type, 
                                          w16 _array_index)
{
  if (object_type==type)
	  {
	  request_redraw(i4_F);
	  return;//don't reload object if selecting same obj again.
	  }
  if (object.valid())
  {
    object->request_remove();
    g1_remove_man.process_requests();
    object=0;
  }

  if (type>=0)
  {
    object=g1_create_object(type);
    if (object.valid())
    {
      object->x=object->lx=object->y=object->ly=object->h=object->lh=0;
      object->player_num=g1_edit_state.current_team;
    }
	camera.init();//reset camera to default location
  }

  object_type=type;
  array_index=_array_index;
  request_redraw(i4_F);
}

g1_3d_object_window::g1_3d_object_window(w16 w, w16 h,
                                         g1_object_type obj_type,
                                         w16 _array_index,
                                         g1_3d_pick_window::camera_struct &camera,
                                         i4_image_class *active_back,
                                         i4_image_class *passive_back,
                                         i4_event_reaction_class *reaction)
  :  g1_3d_pick_window(w,h,
                       active_back, passive_back,
                       camera,
                       reaction),
     object_type(-1) //set to invalid (will cause set_object_type() bellow to load always)
{
  object=0;
  set_object_type(obj_type, _array_index);
}

g1_3d_object_window::~g1_3d_object_window()
	{
	if (object.valid())
		{
		object->request_remove();
		g1_remove_man.process_requests();
		object=0;
		}
	}

void g1_3d_object_window_ambient(i4_transform_class *object_to_world,
                                 i4_float &ar, i4_float &ag, i4_float &ab)
{
  ar = 1.0;
  ag = 1.0;
  ab = 1.0;
}


void g1_3d_object_window::draw_object(g1_draw_context_class *context)
{
  if (object.valid())
  {
    r1_render_api_class *render_api=g1_render.r_api;

    i4_transform_class spare_transform;
	spare_transform.identity();
    object->world_transform = &spare_transform;

    object->player_num=g1_edit_state.current_team;
    object->calc_world_transform(g1_render.frame_ratio);
    
    //setup an ambient function that returns 1.0
    g1_get_ambient_function_type last_ambient_func = g1_render.get_ambient;
    g1_render.get_ambient = g1_3d_object_window_ambient;

	//g1_far_z_range is obsolete for r1_far_clip_z
    g1_render.r_api->clear_area(0,0, width()-1, height()-1, 0, r1_far_clip_z);

      
    li_class_context c(object->vars);
	i4_3d_vector pos(0,0,0); //not checked anyway when FORCEDRAW is on. 
	w32 oldflags=object->draw_params.flags;
	object->draw_params.flags|=g1_model_draw_parameters::FORCEDRAW;//draw this object regardless of its position
    object->draw(context,pos);
    object->draw_params.flags=oldflags;
    //put the old ambient function back
    g1_render.get_ambient = last_ambient_func;

    
  }
  else
    local_image->clear(0, *context->context);
}

i4_bool g1_3d_object_window::selected()
{
  return (i4_bool)(g1_e_object.get_object_type() == object_type);
}

void g1_3d_object_window::do_press()
{
  if (object_type>=0 && g1_object_type_array[object_type])
  {
    g1_edit_state.hide_focus();

    g1_e_object.set_object_type(object_type);

    if (strcmp(g1_edit_state.major_mode, "OBJECT"))
      g1_edit_state.set_major_mode("OBJECT");

    if (g1_e_object.get_minor_mode() != g1_object_params::ADD)
      g1_edit_state.set_minor_mode("OBJECT", g1_object_params::ADD);

    g1_edit_state.show_focus();
  }
}


// dialogs/object_picker.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



static int to_upper(int ch) { return ch>='a' && ch<='z' ? ch-'a' : ch; }

int obj_name_compare(const g1_object_type *a, const g1_object_type *b)
{
  return strcmp(g1_object_type_array[*a]->name(), g1_object_type_array[*b]->name());
}

class g1_object_picker_class : public i4_color_window_class
{
  g1_3d_object_window *obj_view;

  i4_array<g1_object_type> selectable;
  i4_array<i4_text_item_class *> text;
  int offset;
  w32 bg_color;


public:
  enum { W=128,H=400 };

  void recolor_text()
  {
    for (int i=0; i<text.size(); i++)
    {     
      w32 new_color;
      if (i+offset>=selectable.size() ||
          selectable[i+offset]==g1_e_object.get_object_type())
        new_color=0xffff00;
      else
        new_color=bg_color;

      if (text[i]->bg_color!=new_color)
      {
        text[i]->bg_color=new_color;
        text[i]->request_redraw(i4_F);
      }        
    }
  }

  void reorient_text()
  {
    for (int i=0; i<text.size(); i++)
    {
      if (i+offset<selectable.size())
        text[i]->change_text(g1_object_type_array[selectable[i+offset]]->name());
      else
        text[i]->change_text("");

    }

    recolor_text();
  }

  void refresh()
  {
    obj_view->set_object_type(g1_e_object.get_object_type(), 0);

    if (obj_view->object.valid())
      obj_view->camera.view_dist=obj_view->object->occupancy_radius()*1.25f;

    request_redraw(i4_T);
  }

  void name(char* buffer) { static_name(buffer,"object_picker_class"); }
  g1_object_picker_class()
    : i4_color_window_class(W, H, i4_current_app->get_style()->color_hint->neutral(),
                            i4_current_app->get_style()),
      selectable(0,32),
      text(0,32)
  {

    offset=0;

    g1_3d_pick_window::camera_struct camera;
    camera.init();

    i4_graphical_style_class *style=i4_current_app->get_style();


    w32 l,t,r,b, ow, oh;
    style->get_in_deco_size(l,t,r,b);

    // put the render window in a deco window to make it stand out and look nice
    ow=W-(l+r)-2;
    oh=W-(t+b);

    i4_deco_window_class *deco=new i4_deco_window_class((w16)ow, (w16)oh, i4_T, style);
    

    // object view must reside in a render window
    r1_render_window_class *rwin;
    rwin=g1_render.r_api->create_render_window(ow, oh, R1_COPY_1x1);

    obj_view=new g1_3d_object_window((w16)ow, (w16)oh,
                                     g1_e_object.get_object_type(),
                                     0,
                                     camera,
                                     0,0, 0);
    
    rwin->add_child(0,0, obj_view);
    deco->add_child((short)deco->get_x1(), (short)deco->get_y1(), rwin);   
    add_child((short)l,(short)t, deco);



    // find out which objects are visible to the editor user
    int i=0, y;
    for (i=0; i<=g1_last_object_type; i++)
      if (g1_object_type_array[i] && 
          g1_object_type_array[i]->flags & g1_object_definition_class::EDITOR_SELECTABLE)
        selectable.add(i);

    selectable.sort(obj_name_compare);


    bg_color=style->color_hint->neutral();

    // create text fields so we know how many items fit on the screen
    for (t=0, y=deco->height()+2; y < height() && t < (w16)selectable.size(); y++)
    {
      text.add(new i4_text_item_class(g1_object_type_array[selectable[t]]->name(),
                                      style, style->color_hint, style->font_hint->small_font,
                                      new i4_event_reaction_class(this, t+1)));

      if (selectable[t]==g1_e_object.get_object_type())
        text[t]->bg_color=0xffff00;
      else
        text[t]->bg_color=bg_color;

        

      add_child(0,y, text[t]);
      y+=text[t]->height();
      t++;
    }
      

    // now add a scroll bar
    i4_scroll_bar *sb=new i4_scroll_bar(i4_T, height()-deco->height(), 
                                        t, selectable.size(), 0, this, style);

    add_child(width()-sb->width(), deco->height()+2, sb);

    // new resize the text fields to fit with the scroll bar
    for (i=0; i<text.size(); i++)
      text[i]->resize(W-sb->width(), text[i]->height());
  }


  void select_type(int type)
  {
    g1_edit_state.hide_focus();

    g1_e_object.set_object_type(type);

    if (strcmp(g1_edit_state.major_mode, "OBJECT"))
      g1_edit_state.set_major_mode("OBJECT");

    if (g1_e_object.get_minor_mode() != g1_object_params::ADD)
      g1_edit_state.set_minor_mode("OBJECT", g1_object_params::ADD);

    recolor_text();

    g1_edit_state.show_focus();
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      
      if (uev->sub_type==0)  // SCROLLBAR message
      {      
        CAST_PTR(sbm, i4_scroll_message, ev);
        offset=sbm->amount;
        reorient_text();
      }
      else                  // selected one of the text fields
		  {
		  sw32 index=(sw32) (uev->sub_type-1+offset);
		  if (index>=selectable.size())
			  {
			  //invalid entry choosen, perhaps we should ask the user for a new object
			  //to add to the list
			  }
		  else
			  select_type(selectable[index]);
		  }
    }
    else if (ev->type()==i4_event::KEY_PRESS)
    {
      CAST_PTR(kev, i4_key_press_event_class, ev);
      if (kev->key_code==I4_UP)
      {
        if (offset) { offset--; reorient_text(); }       
      }
      else if (kev->key_code==I4_DOWN)
      {
        if (offset<selectable.size()-1)
        {
          offset++;
          reorient_text();
        }
      }

      for (int i=0; i<selectable.size(); i++)
        if (to_upper(*g1_object_type_array[selectable[i]]->name())==kev->key_code)
        {
          offset=i;
          select_type(selectable[i]);
          return ;
        }
    }
    else 
      i4_color_window_class::receive_event(ev);
  }

};


i4_event_handler_reference_class<i4_parent_window_class> g1_object_picker_mp;
i4_event_handler_reference_class<g1_object_picker_class> g1_object_picker;

void g1_refresh_object_picker()
{
  if (g1_object_picker.get())
    g1_object_picker->refresh();
}


li_object *g1_toggle_object_picker(li_object *o, li_environment *env)
{
  i4_graphical_style_class *style=i4_current_app->get_style();
  if (g1_object_picker.get())
    style->close_mp_window(g1_object_picker.get());
  else
  {
    g1_object_picker=new g1_object_picker_class;
    g1_object_picker_mp=style->create_mp_window(-1,-1, 
                                                g1_object_picker->width(),
                                                g1_object_picker->height(),
                                                g1_ges("object_pick_title"));

    g1_object_picker_mp->add_child(0,0,g1_object_picker.get());
  }

  return 0;
}

li_automatic_add_function(g1_toggle_object_picker, "toggle_object_picker");

// dialogs/objref_edit.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



class li_objref_change_button_class : public i4_button_class
{
public:
  i4_text_window_class *show_name;
  w32 current_id;
  void name(char* buffer) { static_name(buffer,"objref_change_button"); }
  void update_name()
  {
    char buf[200];

    if (g1_global_id.check_id(current_id))
      sprintf(buf, "%s, %d", g1_global_id.get(current_id)->name(), current_id);
    else
      sprintf(buf, "0");

    if (show_name)
      show_name->set_text(new i4_str(buf));
  }


  li_objref_change_button_class(const i4_const_str &help,
                                i4_window_class *child,
                                i4_graphical_style_class *style,
                                i4_text_window_class *show_name,
                                w32 current_id)
    : i4_button_class(&help, child, style),
      show_name(show_name),
      current_id(current_id)
  {
    set_popup(i4_T);
    update_name();
  }

                      
  void do_press()
  {
    w32 selected_objects[G1_MAX_OBJECTS];
    int t_sel=g1_get_map()->make_selected_objects_list(selected_objects, G1_MAX_OBJECTS);
    if (t_sel)
      current_id=selected_objects[0];
    else
      current_id=g1_global_id.invalid_id();

    i4_button_class::do_press();
    update_name();
  }
};



class li_objref_list_controls : public i4_parent_window_class
{
public:
  i4_text_window_class *show_name;

  enum { ADD, DEL, CLEAR };

  li_object_pointer c;
  
  li_g1_ref_list *get() { return (li_g1_ref_list *)c.get(); }

  i4_graphical_style_class *style() { return i4_current_app->get_style(); }

  i4_button_class *create_but(char *name, int id)
  {
    char help_name[50];
    sprintf(help_name, "%s_help", name);

    i4_button_class *b=0;
    b=new i4_button_class(&g1_ges(help_name), 
                          new i4_text_window_class(g1_ges(name), style()),
                          style(),
                          new i4_event_reaction_class(this, id));
    b->set_popup(i4_T);
    return b;
  }

  li_objref_list_controls(li_g1_ref_list *o,
                          i4_text_window_class *show_name)
    : show_name(show_name),
      i4_parent_window_class(0,0)
  {
    c=new li_g1_ref_list;
    for (int i=0; i<o->size(); i++)
    {
      g1_object_class *obj=o->value(i);
      if (obj)
        get()->add(obj);
    }


    add_child(0,0, create_but("add_links", ADD));
    add_child(0,0, create_but("del_links", DEL));
    add_child(0,0, create_but("clear_links", CLEAR));
    arrange_down_right();
    resize_to_fit_children();
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      w32 sel[G1_MAX_OBJECTS];
      int t_sel=g1_get_map()->make_selected_objects_list(sel, G1_MAX_OBJECTS);

      switch (((i4_user_message_event_class *)ev)->sub_type)
      {
        case ADD :
        {
          for (int i=0; i<t_sel; i++)
            if (get()->find(sel[i])==-1)
              get()->add(sel[i]);
        } break;

        case DEL:
        {
          for (int i=0; i<t_sel; i++)
            if (get()->find(sel[i])!=-1)
              get()->remove(sel[i]);
        } break;

        case CLEAR :
        {
          while (get()->size())
            get()->remove(get()->get_id(0));
        } break;
      }

      if (show_name)
        show_name->set_text(i4_const_str("%d").sprintf(20, get()->size()));

    }
    else i4_parent_window_class::receive_event(ev);
  }

  void name(char* buffer) { static_name(buffer,"objref_list_controls"); }

};

class li_object_reference_edit_class : public li_type_edit_class
{
public:

  virtual int create_edit_controls(i4_str name,
                                   li_object *o, 
                                   li_object *property_list,
                                   i4_window_class **windows, 
                                   int max_windows,
                                   li_environment *env)
  {
    if (max_windows<3) return 0;
    
    char buf[200];
    sprintf(buf, "Name of object linked to");


    windows[0]=new i4_text_window_class(name, i4_current_app->get_style());
   
    i4_text_window_class *ti=new i4_text_window_class(buf, i4_current_app->get_style());
    windows[2]=ti;

    i4_graphical_style_class *style=i4_current_app->get_style();

    w32 id=li_g1_ref::get(o,env)->id();
    windows[1]=new li_objref_change_button_class(g1_ges("change_link_help"),
                                                 new i4_text_window_class(g1_ges("change_link"), 
                                                                          style),
                                                 style,
                                                 ti, id);
       
    return 3;
  }

  virtual li_object *apply_edit_controls(li_object *o, 
                                         li_object *property_list,
                                         i4_window_class **windows,
                                         li_environment *env)
  {
    return new li_g1_ref(((li_objref_change_button_class *)windows[1])->current_id);
  }

} li_object_reference_edit_instance;


class li_object_list_reference_edit_class : public li_type_edit_class
{
public:
  virtual int create_edit_controls(i4_str name,
                                   li_object *o, 
                                   li_object *property_list,
                                   i4_window_class **windows, 
                                   int max_windows,
                                   li_environment *env)
  {
    if (max_windows<3) return 0;

    li_g1_ref_list *r=li_g1_ref_list::get(o,env);
    
    char buf[200];
    sprintf(buf, "%d : %s", r->size(), r->size() && r->value(0) ?
            r->value(0)->name() : 0);

    windows[0]=new i4_text_window_class(name, i4_current_app->get_style());
   
    i4_text_window_class *ti=new i4_text_window_class(buf, i4_current_app->get_style());
    windows[2]=ti;

    i4_graphical_style_class *style=i4_current_app->get_style();

    windows[1]=new li_objref_list_controls(li_g1_ref_list::get(o,env),
                                           ti);
       
    return 3;
  }

  virtual li_object *apply_edit_controls(li_object *o, 
                                         li_object *property_list,
                                         i4_window_class **windows,
                                         li_environment *env)
  {
    return ((li_objref_list_controls *)windows[1])->get();
  }

} li_object_list_reference_edit_instance;


class li_objref_edit_initer : public i4_init_class
{
public:
  void init() 
  {
    li_get_type(li_find_type("object_ref"))->set_editor(&li_object_reference_edit_instance);
    li_get_type(li_find_type("object_ref_list"))->set_editor(&li_object_list_reference_edit_instance);
  }

} i_objref_edit_instance;

// dialogs/pick_win.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



void g1_3d_pick_window::parent_draw(i4_draw_context_class &context)
{    
  i4_transform_class  tmp;
  i4_transform_class  transform;

  request_redraw(i4_T);
  request_redraw(i4_F);

  transform.identity();

  tmp.translate(0,0,camera.view_dist);  transform.multiply(tmp);    

  tmp.rotate_y(camera.theta);           transform.multiply(tmp);
  tmp.rotate_x(i4_pi()-camera.phi);   transform.multiply(tmp);


  tmp.translate(-camera.center_x,
                -camera.center_y,
                -camera.center_z);      
  transform.multiply(tmp);




  //  context.add_both_dirty(0,0,local_image->width()-1,local_image->height()-1);

  g1_draw_context_class gc;
  gc.window_setup(0,0, width(), height(),
                  g1_render.center_x, g1_render.center_y,
                  g1_render.scale_x, g1_render.scale_y,
                  g1_render.ooscale_x, g1_render.ooscale_y);

  r1_render_api_class *render_api=g1_render.r_api;


  render_api->default_state();

  render_api->set_shading_mode(R1_SHADE_DISABLED);

  render_api->clear_area(0,0, width()-1, height()-1, 
                         active ? 0xffffff : 0x9f9f9f, r1_far_clip_z);


  gc.transform=&transform;
  gc.screen=local_image;
  gc.context=&context;
  gc.draw_editor_stuff=i4_T;
  g1_render.frame_ratio=1;
  
  //r1_far_clip_z=5000;


  if (active && !camera.stopped)
  {
    i4_time_class now;
    camera.zrot+=now.milli_diff(start)/2000.0f;
    start.get();

    
    i4_float d=(float)fabs(cos(camera.zrot)*4)-i4_pi();
    camera.theta=d;

    if (camera.theta>2*i4_pi())
      camera.theta-=2*i4_pi();
    if (camera.theta<0)
      camera.theta+=2*i4_pi();
  }
  else
    start.get();


  //r1_far_clip_z=5000;
        
  render_api->disable_texture();
  render_api->clear_area(0,0,width()-1,height()-1,0,r1_far_clip_z);    

  draw_object(&gc);

  //r1_texture_manager_class *tman=render_api->get_tmanager();

}


void g1_3d_pick_window::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
  {
    request_redraw(i4_T);

    do_press();
    i4_kernel.send(reaction);
    camera.stopped=i4_T;

    CAST_PTR(mev, i4_mouse_button_down_event_class, ev);
    if (!grabr && mev->but==i4_mouse_button_down_event_class::RIGHT)
    {
      grabr=i4_T;

      if (!grabl)
      {
        i4_window_request_mouse_grab_class grab(this);
        i4_kernel.send_event(parent, &grab);
      }
    }
    else if (!grabl && mev->but==i4_mouse_button_down_event_class::LEFT)
    {
      grabl=i4_T;

      if (!grabr)
      {
        i4_window_request_mouse_grab_class grab(this);
        i4_kernel.send_event(parent, &grab);
      }
      do_press();

    }

  }
  else if (ev->type()==i4_event::MOUSE_BUTTON_UP)
  {
    CAST_PTR(mev, i4_mouse_button_up_event_class, ev);
    if (grabr && mev->but==i4_mouse_button_up_event_class::RIGHT)
    {
      grabr=i4_F;

      if (!grabl)
      {
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);
      }
    }
    else if (grabl && mev->but==i4_mouse_button_up_event_class::LEFT)
    {
      grabl=i4_F;

      if (!grabr)
      {
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);
      }

    }
    
  } else if (ev->type()==i4_event::MOUSE_MOVE)
  {
    CAST_PTR(mev, i4_mouse_move_event_class, ev);
    if (grabl)
    {
      camera.theta -= 0.01f*(mev->x - last_mx);
      camera.phi -= 0.01f*(mev->y - last_my);

      if (camera.theta<0.0)
        camera.theta += i4_pi()*2;
      else if (camera.theta>i4_pi()*2)
        camera.theta -= i4_pi()*2;

      if (camera.phi<0.0)
        camera.phi = 0.0;
      else if (camera.phi>i4_pi())
        camera.phi = i4_pi();

      request_redraw(i4_F);
    }
    if (grabr)
    {
      camera.view_dist += (mev->y - last_my)*0.1f;
      request_redraw(i4_F);
    }

    last_mx=mev->x;
    last_my=mev->y;
  }
  else if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(kev, i4_key_press_event_class, ev);
    if (kev->key=='x')
      camera.theta+=0.2f;
    else if (kev->key=='X')
      camera.theta-=0.2f;
    if (kev->key=='y')
      camera.phi+=0.2f;
    else if (kev->key=='Y')
      camera.phi-=0.2f;    
  }

  i4_menu_item_class::receive_event(ev);
}

// dialogs/scroll_picker.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



void g1_scroll_picker_class::create_windows()
{
  for (int i=0; i<windows.size(); i++)
    delete render_windows[i];
    
  windows.clear();
  render_windows.clear();

  int y=start_y;
  int obj_size=info->object_size;

  int on=info->scroll_offset;

  int tb=0;

  int objs_per_line=info->max_object_size/obj_size;
  int done=0;

  while (y+obj_size<=height() && !done)
  {
    int x=0;
    while (x+obj_size<=info->max_object_size && !done)
    {

      i4_menu_item_class *w=create_window(obj_size, obj_size, on);
      if (!w)
        done=1;
      else
      {
        r1_render_window_class *rwin;
        rwin=g1_render.r_api->create_render_window(obj_size, obj_size, R1_COPY_1x1);

        windows.add(w);
        render_windows.add(rwin);

        rwin->add_child(0,0, w);

        add_child(x, y, rwin);
        on++;
        tb++;
        x+=obj_size;      
      }
    }

    y+=obj_size;
  }

  if (scroll_bar)
    scroll_bar->set_new_total((total_objects()+objs_per_line-1)/objs_per_line);  
}

g1_scroll_picker_class::g1_scroll_picker_class(i4_graphical_style_class *style,
                                               w32 option_flags,
                                               g1_scroll_picker_info *info,
                                               int total_objects)

  : i4_color_window_class(0,0, style->color_hint->neutral(), style),
    info(info),
    windows(16,16),
    render_windows(16,16)
{
  if (option_flags & (1<<ROTATE))
    add_child(0,0, g1_edit_state.create_button("tp_rotate", ROTATE, i4_T, this));

  if (option_flags & (1<<MIRROR))
    add_child(0,0, g1_edit_state.create_button("tp_mirror", MIRROR, i4_T, this));

  if (option_flags & (1<<GROW))
    add_child(0,0, g1_edit_state.create_button("tp_grow",   GROW, i4_T,  this));

  if (option_flags & (1<<SHRINK))
    add_child(0,0, g1_edit_state.create_button("tp_shrink", SHRINK, i4_T, this));
  
  if (option_flags & (1<<ADD))
	  add_child(0,0, g1_edit_state.create_button("tp_add", ADD, i4_T, this));

  if (option_flags & (1<<REMOVE))
	  add_child(0,0, g1_edit_state.create_button("tp_remove", REMOVE, i4_T, this));

  if (option_flags & (1<<EDIT))
	  add_child(0,0, g1_edit_state.create_button("tp_edit", EDIT, i4_T, this));


  arrange_down_right();
  
  resize_to_fit_children();

  int scroll_area_height=info->max_object_size * info->max_objects_down, 
    scroll_area_width=info->max_object_size;

  start_y=height();
  int tb=0;

  if (option_flags & (1<<SCROLL))
  {
    scroll_bar = new i4_scroll_bar(i4_T, scroll_area_height,
                                   scroll_area_height/info->object_size,
                                   total_objects,
                                   SCROLL, this, style);

    scroll_area_width += scroll_bar->width();
  }
  else
    scroll_bar = 0;
  


  resize(scroll_area_width > width() ? scroll_area_width : width(),  
         scroll_area_height + height());

  if (scroll_bar)
    add_child(width()-scroll_bar->width(), start_y, scroll_bar);

}

void g1_scroll_picker_class::parent_draw(i4_draw_context_class &context)
{
  i4_color_window_class::parent_draw(context);

  r1_texture_manager_class *tman=g1_render.r_api->get_tmanager();

  g1_render.r_api->flush_vert_buffer();

  tman->next_frame();
}

void g1_scroll_picker_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {   
    CAST_PTR(sm, i4_scroll_message, ev);
	switch (sm->sub_type)
	{
	case SCROLL:
		{
		int obj_size=info->object_size;
		int objs_per_line=info->max_object_size/obj_size;

		int off=sm->amount * objs_per_line;

		if (off>=total_objects())
			off-=objs_per_line;

		info->scroll_offset=off;

		for (int i=0; i<windows.size(); i++)
			change_window_object_num(windows[i], off+i);
	                                   
		}
		break;
	case G1_OPEN_DLL_OK:
		{
			CAST_PTR(f, i4_file_open_message_class, ev);
			if (f->filename)
			{
				i4_filename_struct fname_struct;
				i4_split_path(*f->filename,fname_struct);
				add(fname_struct.filename);
			}
			refresh(i4_T);
		}break;
	}
  }
  else if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(om, i4_object_message_event_class, ev);
    switch (om->sub_type)
    {
      case GROW :
      {
        if (info->object_size*2<=info->max_object_size)
        {
          info->scroll_offset=0;
          info->object_size*=2;
          create_windows();
        }
      } break;

      case SHRINK :
      {
        if (info->object_size/2>=info->min_object_size)
        {
          info->scroll_offset=0;
          info->object_size/=2;
          create_windows();
        }

      } break;

      case MIRROR :
      {
        mirror();
        refresh();
      } break;

      case ROTATE :
      {
        rotate();
        refresh();
      } break;
	  case EDIT:
		  {
			  for (int win=0;win<windows.size();win++)
			  {
				  if (windows[win]->selected())
				  {
					  if (edit(windows[win]))
						  refresh();
				  }
			  }
			  
		  }break;
	  case ADD:
		  i4_create_file_open_dialog(style,
			  g1_editor_instance.get_editor_string("new_tile_texture_title"),
			  "textures",  //cannot be changed
			  g1_editor_instance.get_editor_string("new_tile_texture_mask"),
			  g1_editor_instance.get_editor_string("new_tile_texture_mask_name"),
			  this,
			  G1_OPEN_DLL_OK,
			  G1_FILE_OPEN_CANCEL);
		  break;
	  
	  case REMOVE:
		  {
			  for (int win=0;win<windows.size();win++)
			  {
				  if (windows[win]->selected())
				  {
					  if (remove(windows[win]))
						  refresh(i4_T);
				  }
			  }
		  }
    }
   
  }
  else
    i4_parent_window_class::receive_event(ev);
}

void g1_scroll_picker_class::refresh(i4_bool list_has_changed)
{
  if (list_has_changed)
  {
	  create_windows();
  }
  else
  {
	for (int i=0; i<windows.size(); i++)
		windows[i]->request_redraw(i4_F);
  }
}

// dialogs/sky_picker.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


//The classes here where moved to sky.h
//class g1_sky_picker_class
//class g1_sky_view

void g1_editor_class::create_sky_window()
{
  if (get_map())
  {
  g1_sky_view::sky_scroll_offset=0;
    g1_sky_picker_class *sp=new g1_sky_picker_class(i4_current_app->get_style());

    create_modal(sp->width(), sp->height(), "set_sky_title");
    modal_window.get()->add_child(0,0,sp);
  }
}

void g1_editor_class::set_sky()
{  
   //i4_os_string(*sky_dialog.name->get_edit_string(), 
   //             get_map()->sky_name, 
   //             sizeof(get_map()->sky_name));
   if (get_map()->sky_name) delete get_map()->sky_name;
   get_map()->sky_name=new i4_str(*sky_dialog.name->get_edit_string());
   //g1_sky_class::update() needs to get this string.
   //but we don't have a window handle available right now
   //char name[200];
   //i4_os_string(get_map()->sky_name,name,200);
   //get_map()->sky_model=g1_model_list_man.get_model(g1_model_list_man.find_handle(name));
   
   close_modal();
}

// dialogs/tile_picker.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



g1_tile_picker_class::g1_tile_picker_class(i4_graphical_style_class *style, 
                                           g1_scroll_picker_info *info,
                                           i4_image_class *active_back,
                                           i4_image_class *passive_back)
  :
    g1_scroll_picker_class(style,
                           (1<<ROTATE) | (1<<MIRROR) | (1<<GROW) | 
						   (1<<SHRINK) | (1<<SCROLL)| (1<<ADD) |
						   /*(1<<REMOVE) | */ (1<<EDIT),
                           info,
                           g1_tile_man.remap_size()
                           ),
    active_back(active_back),
    passive_back(passive_back),
	edit_blocking(0),
	edit_fract(0),
	edit_tile_damage(0),
	edit_wave(0)
    
{

}
 
int g1_tile_picker_class::total_objects()
{
  return g1_tile_man.remap_size();
}

i4_menu_item_class *g1_tile_picker_class::create_window(w16 w, w16 h, int scroll_object_num)
{
  int t=g1_tile_man.remap_size();
  if (scroll_object_num>=t)
    return 0;

  g1_3d_pick_window::camera_struct tile_state;
  tile_state.init();

  g1_3d_tile_window *neww;

  neww=new g1_3d_tile_window(w,h, scroll_object_num, tile_state,
                               active_back, passive_back, 0);
  //neww->set_context_help("somehelp");
  return neww;
}


void g1_tile_picker_class::change_window_object_num(i4_window_class *win, 
                                                    int new_scroll_object_num)
{
  g1_3d_tile_window *twin=((g1_3d_tile_window *)win);

  //g1_3d_pick_window::camera_struct old_camera = twin->camera;  // JJ COmment not used
  
  int old_num=twin->tile_num;

  twin->set_tile_num(new_scroll_object_num);

}


void g1_tile_picker_class::rotate()
{
  if (g1_e_tile.get_cell_rotation()==G1_ROTATE_270)
    g1_e_tile.set_cell_rotation(G1_ROTATE_0);
  else      
    g1_e_tile.set_cell_rotation((g1_rotation_type)(g1_e_tile.get_cell_rotation()+1));
  i4_warning("New rotation = %d, mirrored = %d", g1_e_tile.get_cell_rotation(), g1_e_tile.get_mirrored());


}

void g1_tile_picker_class::mirror()
{
  g1_e_tile.set_mirrored((i4_bool)(!g1_e_tile.get_mirrored()));
  i4_warning("New rotation = %d, mirrored = %d", g1_e_tile.get_cell_rotation(), g1_e_tile.get_mirrored());

}

i4_bool g1_tile_picker_class::remove(i4_menu_item_class *window)
{
	// This is currently not supported -> Would cause mayor headaches,
	// since we first had to make sure it's not used and then the handles
	// for the remaining textures had to be changed all over the place. 
	g1_3d_tile_window *tile=(g1_3d_tile_window*)window;
	int t=g1_tile_man.get_remap(tile->tile_num);
	//g1_tile_man.get(t)->filename_checksum=0; //Mark as empty
	//g1_tile_man.get(t)->texture=0;
	return i4_F;
}

li_object* tile_edit_callback(li_object* o, li_environment *env)
{
	if (o==NULL)
		return 0;
	li_class *c=li_class::get(li_car(o, env),env);
	li_class_context ctx(c);
	int t=g1_tile_man.get_remap(li_int::get(c->value("hidden_id"),0)->value());
	g1_tile_class *tile=g1_tile_man.get(t);
	tile->set_friction(li_float::get(c->value("friction_fraction"),0)->value());
	tile->damage=li_int::get(c->value("damage"),0)->value();
	tile->flags|=(c->value("wave")==li_true_sym)?(g1_tile_class::WAVE):0;
	tile->flags|=(c->value("block")==li_true_sym)?(g1_tile_class::BLOCKING):0;
	return 0;
}

void g1_tile_picker_class::add(i4_str name)
{
	if (g1_tile_man.add_new(new li_string(name),0)==false)
	{
		i4_message_box("Tile editor","The new tile texture could not be added. Verify it doesn't exist already in the list");
		return;
	}
	li_call("reload_main_textures");
}
i4_bool g1_tile_picker_class::edit(i4_menu_item_class *window)
{
	g1_3d_tile_window *tilewin=(g1_3d_tile_window*)window;
	int t=g1_tile_man.get_remap(tilewin->tile_num);
	g1_tile_class *tile=g1_tile_man.get(t);
	
	li_class *dlg=(li_class*)li_new("tile_edit_dialog");
	li_class_context ctx(dlg);
	dlg->set("friction_fraction",new li_float(tile->friction_fraction));
	dlg->set("damage",new li_int(tile->damage));
	li_object *v=(tile->flags&g1_tile_class::BLOCKING)?li_true_sym:li_nil;
	dlg->set("block", v);
	v=(tile->flags&g1_tile_class::WAVE)?li_true_sym:li_nil;
	dlg->set("wave",v);
	dlg->set("hidden_id",new li_int(tilewin->tile_num));
	i4_const_str *s=g1_tile_man.get_name_from_tile(t);
	if (s==NULL)
	{
		return i4_F;
	}
	li_create_dialog(*s,dlg,0,&tile_edit_callback,0);
	delete s;
	return i4_T;
}

// dialogs/tile_win.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


g1_3d_tile_window::g1_3d_tile_window(w16 w, w16 h,
                                     int tile_num,
                                     g1_3d_pick_window::camera_struct &camera,
                                     i4_image_class *active_back,
                                     i4_image_class *passive_back,
                                     i4_event_reaction_class *reaction)
  :  g1_3d_pick_window(w,h,
                       active_back, passive_back,
                       camera,
                       reaction),
     object(0),
     tile_num(tile_num)

{
  w32 min_screen = w<h ? w : h;
  
}


inline void g1_setup_vert(r1_vert *v, float px, float py, float z)
{
  v->px=px;
  v->py=py;
  v->v.z=z;
  v->w=1.0f/z;
}

void g1_3d_tile_window::do_press()
{
  if (tile_num<g1_tile_man.remap_size())
  {
    g1_e_tile.set_cell_type(g1_tile_man.get_remap(tile_num));

    if (strcmp(g1_edit_state.major_mode,"TILE"))
      g1_edit_state.set_major_mode("TILE");

    if (g1_e_tile.get_minor_mode() != g1_tile_params::PLACE)
      g1_edit_state.set_minor_mode("TILE",g1_tile_params::PLACE);
  }
}

void g1_3d_tile_window::do_edit()
{
	//Todo: Needs body. 
}

i4_bool g1_3d_tile_window::selected()
{
	if (tile_num>=g1_tile_man.remap_size())
		return i4_F; //can't select an entry beyond the amount of tiles.
	if (g1_tile_man.get_remap(tile_num)==g1_e_tile.get_cell_type())
		return i4_T;
	else
		return i4_F;
}

void g1_3d_tile_window::draw_object(g1_draw_context_class *context)
{
  if (tile_num<g1_tile_man.remap_size())
  {
    i4_const_str *s=g1_tile_man.get_name_from_tile(g1_tile_man.get_remap(tile_num));
	set_context_help(s);
	delete s;
    r1_texture_handle han=g1_tile_man.get_texture(g1_tile_man.get_remap(tile_num)); 
    r1_render_api_class *render_api=g1_render.r_api;

    if (active)
      render_api->set_constant_color(0xffffff);
    else               
      render_api->set_constant_color(0x9f9f9f);

    if (tile_num<g1_tile_man.remap_size())
    {
      render_api->use_texture(han, width(), 0);

      r1_vert v1[3],v2[3];
      g1_setup_tri_texture_coords(v1, v2, g1_e_tile.get_cell_rotation(), g1_e_tile.get_mirrored());

      g1_setup_vert(v1,   0, (float)(height()-1), 50);
      g1_setup_vert(v1+1, (float)(width()-1),(float)(height()-1), 50);
      g1_setup_vert(v1+2, (float)(width()-1),0, 50);

      render_api->render_poly(3, v1);

      g1_setup_vert(v2,   0, (float)(height()-1), 50);
      g1_setup_vert(v2+1, (float)(width()-1), 0, 50);
      g1_setup_vert(v2+2, 0, 0, 50);

      render_api->render_poly(3, v2);
    }
  }
}

/////////////////////////////////////
// COMMANDS DIRECTORY 
//////////////////////////////////////

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



li_object *g1_reload_map(li_object *o, li_environment *env)
{
  i4_str *fname=new i4_str(g1_get_map()->get_filename());
  
  g1_load_level(*fname, 0, G1_MAP_VIEW_POSITIONS);
  delete fname;

  li_call("redraw_all");

  return 0;
}


li_object *g1_full_reload_map(li_object *o, li_environment *env)
{
  i4_str *fname=new i4_str(g1_get_map()->get_filename());
  
  g1_load_level(*fname, 1, G1_MAP_VIEW_POSITIONS);
  delete fname;

  li_call("redraw_all");

  return 0;
}


li_automatic_add_function(g1_reload_map, "reload_level");
li_automatic_add_function(g1_full_reload_map, "full_reload_level");

// commands/fill.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



void g1_flood_fill_class::fill(sw32 x, sw32 y)
{
  if (blocking(x,y))
    return ;
      
  fill_rec *recs=0,*r;

  sw32 startx=x,starty=y;
  sw32 clip_x1, clip_y1, clip_x2, clip_y2;
  get_clip(clip_x1, clip_y1, clip_x2, clip_y2);

  do
  {
    if (recs)
    { 
      r=recs;
      recs=recs->last;
      x=r->x; y=r->y;
      delete r;
    }


    if (!blocking(x,y))
    {
      while (x>clip_x1 && !blocking(x,y)) 
        x--;

      if (blocking(x,y) && x<clip_x2)
        x++;


      if (y>clip_y1 && !blocking(x,y-1))
        recs=new fill_rec((short)x,(short)(y-1),recs);

      if (y<clip_y2 && !blocking(x, y+1))
        recs=new fill_rec((short)x,(short)(y+1),recs);

      do
      {
        fill_block(x, y, startx, starty);

        if (y>clip_y1 && x>clip_x1 && blocking(x-1, y-1) && !blocking(x, y-1))
          recs=new fill_rec((short)x,(short)(y-1),recs);


        if (y<clip_y2 && x>clip_x1 && blocking(x-1, y+1) && !blocking(x, y+1))
          recs=new fill_rec((short)x,(short)(y+1),recs);

        x++;
      } while (!blocking(x,y) && x<clip_x2);


      x--;
      if (y>clip_y1 && !blocking(x, y-1))
        recs=new fill_rec((short)x,(short)(y-1),recs);

      if (y<clip_y2 && !blocking(x, y+1))
        recs=new fill_rec((short)x,(short)(y+1),recs);
    }
  } while (recs);
}

// commands/map_dump.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/




static float tri1_s[8*3]={0,1,1, 1,1,0, 1,0,0, 0,0,1, 0,0,1, 1,0,0, 1,1,0, 0,1,1};
static float tri1_t[8*3]={1,1,0, 1,0,0, 0,0,1, 0,1,1, 1,0,0, 1,1,0, 0,1,1, 0,0,1};

static float tri2_s[8*3]={0,1,0, 1,0,0, 1,0,1, 0,1,1, 0,1,1, 1,0,1, 1,0,0, 0,1,0};
static float tri2_t[8*3]={1,0,0, 1,0,1, 0,1,1, 0,1,0, 1,0,1, 1,0,0, 0,1,0, 0,1,1};


class map_renderer_class : public i4_window_class
{
public:
  i4_bool do_it_again;
  int ax1, ay1, ax2, ay2;

  map_renderer_class(int w, int h) : i4_window_class(w,h)
  {
    do_it_again=i4_T;
  }

  void request_redraw(i4_bool for_a_child=i4_F) 
  { 
    i4_window_class::request_redraw(for_a_child);
    do_it_again=i4_T;
  }

  void draw(i4_draw_context_class &context)
  {
    r1_render_api_class *api=g1_render.r_api;
    r1_texture_manager_class *tman=api->get_tmanager();
    
    do_it_again=i4_F;
    
    float s, t, s_step, t_step;
    s_step=(width())/(float)(ax2-ax1);//t and s must be [0..256) ??
    t_step=(height())/(float)(ay2-ay1);
    //there's a bug somewhere here, such that the map looses the last edge.
    api->default_state();
    api->clear_area(0,0,width()-1,height()-1, 0x0, RENDER_DEFAULT_FAR_DISTANCE);  

    api->set_filter_mode(R1_NO_FILTERING);

    int x,y;
    t=0;
    for (y=ay1; y<ay2; y++, t+=t_step)
    {
      s=0;
      for (x=ax1; x<ax2; x++, s+=s_step)
      {
        g1_map_cell_class *c=g1_cells + g1_map_width*y+x;
        g1_map_vertex_class *v1=g1_verts + (g1_map_width+1)*y+x, *v2,*v3,*v4;
        v2=v1+1;
        v3=v2+g1_map_width+1;  // order is: v1 -- v2
        v4=v3-1;               //           |      |
                               //           v4 -- v3 (counterclockwise for drawing)
        int texture=g1_tile_man.get(c->type)->texture;
		if (texture!=g1_tile_man.get_pink()||t==0)
			//if terrain is opaque, we use the neighbour texture on the map
			//this avoids pink mountains far away.
			//the second condition is only valid if the corner is pink
			api->use_texture(texture, (int)s_step, 0);
		//unconventional way of doing things: This (together with 
		//the texture manager ignoring the desired with of textures)
		//automatically preloads all landscape textures.
		//(This feature is currently disabled, as it may cause an unexspected exit)
		api->flush_vert_buffer();
		api->get_tmanager()->next_frame(); 

        int st_index=c->get_rotation();
        if (c->mirrored())
          st_index+=4;

        st_index*=3;

        r1_vert v[4];
  
        v[0].px=s;              v[0].py=t;
        v[1].px=s+s_step;       v[1].py=t;
        v[2].px=s+s_step;       v[2].py=t+t_step;
        v[3].px=s;              v[3].py=t+t_step;
  

        v[0].s=tri1_s[st_index];      v[0].t=tri1_t[st_index];
        v[1].s=tri1_s[st_index+1];    v[1].t=tri1_t[st_index+1];
        v[2].s=tri1_s[st_index+2];    v[2].t=tri1_t[st_index+2];
        v[3].s=tri2_s[st_index+2];    v[3].t=tri2_t[st_index+2];

        float z=1.0f;
        float w=1.0f/z;
        v[0].w=w; v[0].v.z=z;
        v[1].w=w; v[1].v.z=z;
        v[2].w=w; v[2].v.z=z;
        v[3].w=w; v[3].v.z=z;

        v[0].a=v[1].a=v[2].a=v[3].a=1;

#if 0
        v[0].r=v[0].g=v[0].b=v1->get_non_dynamic_ligth_intensity(x,y);
        v[1].r=v[1].g=v[1].b=v2->get_non_dynamic_ligth_intensity(x+1,y);
        v[2].r=v[2].g=v[2].b=v3->get_non_dynamic_ligth_intensity(x+1,y+1);
        v[3].r=v[3].g=v[3].b=v4->get_non_dynamic_ligth_intensity(x,y+1);
#else
        v[0].r = v[0].g = v[0].b = 1.0;
        v[1].r = v[1].g = v[1].b = 1.0;
        v[2].r = v[2].g = v[2].b = 1.0;
        v[3].r = v[3].g = v[3].b = 1.0;
#endif
        api->render_poly(4, v);
      }
    }
	api->states_have_changed=i4_T;//ensure that everything gets drawn now.
	api->flush_vert_buffer();
    api->set_filter_mode(R1_BILINEAR_FILTERING);
  }
  void name(char* buffer) { static_name(buffer,"map_renderer"); }
};

void render_map_to_image(int x1, int y1, int x2, int y2, int im_w, int im_h, i4_image_class* image)
{
	r1_render_api_class *api=g1_render.r_api;
	r1_texture_manager_class *tman=api->get_tmanager();
	float s, t, s_step, t_step;
	s_step=(im_w)/(float)(x2-x1);//t and s must be [0..256) ??
	t_step=(im_h)/(float)(y2-y1);
	int x,y;
	
	float t_fract_step=i4_fract(t_step);
	float s_fract_step=i4_fract(s_step);
	
	i4_image_class *current_texture=0;
	w32 col=0;
	i4_draw_context_class context(0,0,im_w-1, im_h-1);
	sw32 act_w=0,act_h=0;
	int s_corr;
	int t_corr;
	t=0;
 	g1_rotation_type rotation_remap[4]={G1_ROTATE_270,G1_ROTATE_0,G1_ROTATE_90,G1_ROTATE_180};
	g1_rotation_type rotation_remap_mirror[4]={G1_ROTATE_90,G1_ROTATE_180,G1_ROTATE_270,G1_ROTATE_0};
	g1_rotation_type rot;
	g1_map_cell_class *c;
	g1_tile_class* tile;
	int texture;
	for (y=y1; y<y2; y++, t+=t_step)  
	{
		s=0;
		t_corr=0;
		//Copy an additional pixel if the next step would skip one or it's the last element
		//to be copied (this is due to rounding errors)
		if (i4_fract(t)+t_fract_step>=1.0f || y==(y2-1))
		{
			t_corr=1;
		}
		for (x=x1; x<x2; x++, s+=s_step)
		{
			s_corr=0;
			if (i4_fract(s)+s_fract_step>=1.0f || x==(x2-1))
			{
				s_corr=1;
			}
			c=g1_cells + g1_map_width*y+x;

			tile=g1_tile_man.get(c->type);
			texture=tile->texture;
			//Get the lowest miplevel, this is better than average color and certainly loaded
			//at this point. 
			if (c->mirrored())
			{
				rot=rotation_remap_mirror[c->get_rotation()];
			}
			else
			{
				rot=rotation_remap[c->get_rotation()];
			}
			current_texture=tman->get_texture_image(texture,0,8); 
			//The following code can be used to "graphically" debug the creation of the
			//landscape lod-texture (creates a whole bunch of files in the golgotha directory!)
			/*i4_str text_name_etc("tex_image_%d_%d_%d_%d.tga");
			i4_str *file_name=text_name_etc.sprintf(200,x1,y1,(w32)s,(w32)t);
			i4_write_tga(current_texture,*file_name,false);
			delete file_name;*/
			current_texture->copy_image_to(image,(i4_coord)s,(i4_coord)t,
				(sw32)s_step+s_corr,(sw32)t_step+t_corr,rot,c->mirrored());
			delete current_texture;
		}
	}
	/*i4_str str=i4_str("temp_image_%d_%d.tga");
	i4_str *res_str=str.sprintf(100,x1,y1);
	i4_write_tga(image,*res_str,false);
	delete res_str;*/
}

i4_image_class *render_map_section(int x1, int y1, int x2, int y2, int im_w, int im_h)
{
	//BUG: The last few pixels of the given rectangle (lower right corner)
	//always stay black. FIXED
 // r1_render_api_class *api=g1_render.r_api;

 // r1_render_window_class *rwin=api->create_render_window(im_w, im_h);
 // map_renderer_class *map_r=new map_renderer_class(im_w, im_h);

 // map_r->ax1=x1;
 // map_r->ay1=y1;
 // map_r->ax2=x2;
 // map_r->ay2=y2;


 // rwin->add_child(0,0, map_r);
 // i4_current_app->get_window_manager()->add_child(0,0,rwin);
 // 
 // 
 i4_draw_context_class context(0,0,im_w-1, im_h-1);

 // i4_display_class *display=i4_current_app->get_display();
 // 
 // int tries=0;
 // r1_texture_manager_class *tman=api->get_tmanager();
 // do
 // {
 //   tman->next_frame();
 //   rwin->draw(context);
 //   display->flush();

 //   tries++;
	//i4_thread_sleep(200);//wait a bit for the loader to load up some data
 //   // repeat until textures have rez-ed in
 //   // or it doens't look like it'll happen
	//// Need at least two tries to be sure the entire display chain is up to date.
 // } while ((map_r->do_it_again && tries<100) || tries<2);
 // tman->next_frame();


  i4_pixel_format fmt;
  fmt.default_format();
  fmt.alpha_mask=0;
  fmt.calc_shift();
  const i4_pal *pal=i4_pal_man.register_pal(&fmt);

  //i4_image_class *fb;
  i4_image_class *to=0;

  //fb=display->lock_frame_buffer(I4_BACK_FRAME_BUFFER, I4_FRAME_BUFFER_READ);
    to = i4_create_image(im_w, im_h, pal);
    to->bar(0,0,im_w-1,im_h-1,0x0007000,context);//perhaps another color 
	render_map_to_image(x1,y1,x2,y2,im_w,im_h,to);
	
    //(the second last element) would prove more usefull for debugging?
    //fb->put_part(to, 0,0, 0,0, im_w-1, im_h-1, context);
    //display->unlock_frame_buffer(I4_BACK_FRAME_BUFFER);

  //delete rwin;

  return to;
}


struct area
{
  int x1,y1,x2,y2;
  area(int x1, int y1, int x2, int y2) : x1(x1),y1(y1),x2(x2),y2(y2) {}
  area() {}
};

static i4_array<area *> *list;

static void split_gather(int x1, int y1, int x2, int y2, int level)
{
  if (list->size()>=6)
    return ;

  
  int xd=x2-x1+1, yd=y2-y1+1;
  if (xd>yd)
  {
    int xs=(x2+x1)/2;
    list->add(new area(x1, y1, xs, y2));
    list->add(new area(xs, y1, x2, y2));

    if (level!=1)
    {
      split_gather(x1, y1, xs, y2, level+1);
      split_gather(xs, y1, x2, y2, level+1);
    }
  }
  else
  {
    int ys=(y2+y1)/2;

    list->add(new area(x1, y1, x2, ys));
    list->add(new area(x1, ys, x2, y2));

    if (level!=1)
    {
      split_gather(x1, y1, x2, ys, level+1);
      split_gather(x1, ys, x2, y2, level+1);
    }
  }
}

li_object *g1_dump_level(li_object *o, li_environment *env)
{
  i4_file_class *fp=i4_open("dump_level.dat", I4_WRITE);
  if (!fp) 
    return 0;
  g1_map_class *map=g1_get_map();

  int w=map->width(), h=map->height(),i,x,y;
  fp->write_32(0xabcf);   // version
  fp->write_16(w);
  fp->write_16(h);

  // save off a 1 pixel bitmap
  g1_map_cell_class *c=g1_cells;
  for (y=0; y<h; y++)
    for (x=0; x<w; x++, c++)
    {
      int type=g1_tile_man.get_texture(c->type);
      w32 color= g1_render.r_api->get_tmanager()->average_texture_color(type, 0);
      g1_map_vertex_class *v=g1_verts+x+y*(w+1);

      float tr,tg,tb;
      v->get_rgb(tr,tg,tb, x,y);

      int r=int(((color>>16)&0xff) * tr);
      int g=int(((color>> 8)&0xff) * tg);
      int b=int(((color>> 0)&0xff) * tb);


      fp->write_32((r<<16)|(g<<8)|b);
    }
  


  // save vert hights and normals
  g1_map_vertex_class *v=g1_verts;
  for (y=0; y<=h; y++)
    for (x=0; x<=w; x++, v++)
    {
      i4_3d_vector normal;
      v->get_normal(normal, x,y);

      fp->write_float(normal.x);
      fp->write_float(normal.y);
      fp->write_float(normal.z);
      fp->write_float(v->get_height());
      

      float r,g,b;
      v->get_rgb(r,g,b, x,y);
      fp->write_float(r);
      fp->write_float(g);
      fp->write_float(b);
    }


  i4_array<area *> mlist(0,32);

  // save cell texture names
  c=g1_cells;
  for (i=0; i<w*h; i++, c++)
  {
    int type=g1_tile_man.get_texture(c->type);
    char *tname=g1_render.r_api->get_tmanager()->get_texture_name(type);
    int len=strlen(tname)+1;
    fp->write_16(len);
    fp->write(tname,len);

    int flags=c->get_rotation();
    if (c->mirrored())
      flags|=4;

    fp->write_8(flags);
  }


  list=&mlist;
  int x1=0,y1=0, x2=g1_map_width, y2=g1_map_height;
  split_gather(x1,y1,x2,y2,0 );
  
  for (i=2; i<6; i++)
  {
    int x1=mlist[i]->x1, y1=mlist[i]->y1, x2=mlist[i]->x2, y2=mlist[i]->y2;

    //i4_warning("%d %d %d %d\n",x1,y1,x2,y2);
    i4_image_class *to;

    if ((to = render_map_section(x1,y1,x2,y2, 256,256))!=0)
    {
      char fn[MAX_PATH];
      sprintf(fn,"mapdebug/%d.tga", i-2);
      i4_file_class *mfp=i4_open(fn, I4_WRITE);
      i4_write_tga(to, mfp);
	  delete mfp;
      delete to;
    }
  }

  for (i=0; i<mlist.size(); i++)
    delete mlist[i];

  delete fp;
  return 0;
}

li_automatic_add_function(g1_dump_level, "dump_level");

// commands/map_misc.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



li_object *g1_fog_map(li_object *, li_environment *env)
{  
  li_call("add_undo", li_make_list(new li_int(G1_MAP_CELLS | G1_MAP_VERTS)));
  g1_map_cell_class *c=g1_cells;
  int t=g1_map_width * g1_map_height, i;

  for (i=0; i<t; i++, c++)
    c->flags |= g1_map_cell_class::FOGGED;
  

  g1_map_vertex_class *v=g1_verts;
  t=(g1_map_width+1) * (g1_map_height+1);

  for (i=0; i<t; i++, v++)
	  v->set_flag(g1_map_vertex_class::FOGGED);


  li_call("redraw");
  return 0;
}


li_object *g1_unfog_map(li_object *, li_environment *env)
{  
  li_call("add_undo", li_make_list(new li_int(G1_MAP_CELLS | G1_MAP_VERTS)));
  g1_map_cell_class *c=g1_cells;
  int t=g1_map_width * g1_map_height, i;

  for (i=0; i<t; i++, c++)
    c->flags &= ~g1_map_cell_class::FOGGED;
  

  g1_map_vertex_class *v=g1_verts;
  t=(g1_map_width+1) * (g1_map_height+1);

  for (i=0; i<t; i++, v++)
    v->set_flag(g1_map_vertex_class::FOGGED,0);


  li_call("redraw");
  return 0;
}

static void select_rest(g1_path_object_class *p, g1_team_type team)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  if (p && !p->get_flag(g1_object_class::SCRATCH_BIT))
  {
    p->set_flag(g1_object_class::SCRATCH_BIT | g1_object_class::SELECTED, 1);
    
    int t=p->total_links(team);
    for (int j=0; j<t; j++)
      select_rest(p->get_link(team, j), team);

    p->set_flag(g1_object_class::SCRATCH_BIT, 0);
  }
}

li_object *g1_select_restof_path(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  for (g1_path_object_class *p=g1_path_object_list.first(); p; p=p->next)
  {
    if (p->get_flag(g1_object_class::SELECTED))
    {
      p->set_flag(g1_object_class::SCRATCH_BIT, 1);

      for (g1_team_type team=G1_ALLY; team<G1_MAX_TEAMS; team=(g1_team_type)(team+1))
      {
        int t=p->total_links(team);
        for (int j=0; j<t; j++)
          select_rest(p->get_link(team, j), team);
      }

      p->set_flag(g1_object_class::SCRATCH_BIT, 0);
    }
  }

  return 0;
}

li_object *g1_add_cloud_shadow(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  g1_object_class *c=g1_create_object(g1_get_object_type("cloud_shadow"));
  c->x=(float)(g1_map_width/2);
  c->y=(float)(g1_map_height/2);
  c->h=(float)(g1_get_map()->terrain_height(c->x, c->y)+0.1);
  c->occupy_location();

  li_call("redraw");
  return 0;
}

li_object *g1_remove_cloud_shadow(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
  
  int type=g1_get_object_type("cloud_shadow");
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (int i=0; i<t; i++)
  {
    if (olist[i]->id==type)
    {
      olist[i]->unoccupy_location();
      olist[i]->request_remove();     
    }
  }

  li_call("redraw");
  return 0;
}



li_automatic_add_function(g1_fog_map, "fog_map");
li_automatic_add_function(g1_unfog_map, "unfog_map");
li_automatic_add_function(g1_select_restof_path, "select_restof_path");

li_automatic_add_function(g1_add_cloud_shadow, "add_cloud_shadow");
li_automatic_add_function(g1_remove_cloud_shadow, "remove_cloud_shadow");

// commands/merge_terrain.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void g1_editor_class::merge_terrain()
{
  i4_create_file_open_dialog(style,
                             get_editor_string("merge_ter_title"),
                             get_editor_string("merge_ter_start_dir"),
                             get_editor_string("merge_ter_file_mask"),
                             get_editor_string("merge_ter_mask_name"),
                             this,
                             G1_TMERGE_FILE_OPEN_OK,
                             G1_TMERGE_FILE_OPEN_CANCEL);


}

i4_bool g1_editor_class::merge_terrain_ok(i4_user_message_event_class *ev)
{
  CAST_PTR(f, i4_file_open_message_class, ev);

 
  i4_bool ret=i4_F;
  if (get_map())
  {
    i4_file_class *in=i4_open(*f->filename);
    if (in)
    {
      g1_loader_class *l=g1_open_save_file(in);
    
      if (l)
      {
        g1_editor_instance.add_undo(G1_MAP_CELLS | G1_MAP_VERTS);

        get_map()->load(l, G1_MAP_CELLS | G1_MAP_VERTS);
        delete l;
        ret=i4_T;
      }

      delete in;
    } 
  }

  return ret;
}

// commands/move.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



li_object *g1_drop_selected(li_object *o, li_environment *env)
{
  if (!g1_map_is_loaded())
    return 0;

  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);

  for (int i=0; i<t; i++)
  {
    if (olist[i]->selected())
    {
      olist[i]->unoccupy_location();
      olist[i]->h=olist[i]->lh=g1_get_map()->terrain_height(olist[i]->x, olist[i]->y);
      olist[i]->occupy_location();
    }
  }
  li_call("redraw");
  
  return 0;
}



li_object *g1_floor_selected(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;

  float lowest=1000;
  for (i=0; i<t; i++)
    if (olist[i]->selected())
      if (olist[i]->h<lowest)
        lowest=olist[i]->h;

  for (i=0; i<t; i++)
    if (olist[i]->selected())
    {
      olist[i]->h=lowest;
      olist[i]->grab_old();
    }

  li_call("redraw");
  
  return 0;
}



li_object *g1_ceil_selected(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;

  float heighest=0;
  for (i=0; i<t; i++)
    if (olist[i]->selected())
      if (olist[i]->h>heighest)
        heighest=olist[i]->h;

  for (i=0; i<t; i++)
    if (olist[i]->selected())
    {
      olist[i]->h=heighest;
      olist[i]->grab_old();
    }

  li_call("redraw");
  
  return 0;
}

li_object *g1_select_game_pieces(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;
  for (i=0; i<t; i++)    
    if (g1_map_piece_class::cast(olist[i]))
      olist[i]->set_flag(g1_object_class::SELECTED,1);
    else
      olist[i]->set_flag(g1_object_class::SELECTED,0);

  li_call("redraw");
  
  return 0;
}


li_object *g1_select_similar(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;

  w8 *sel_types=(w8 *)I4_MALLOC(g1_last_object_type,"");
  memset(sel_types, 0, g1_last_object_type);
  for (i=0; i<t; i++)
    if (olist[i]->get_flag(g1_object_class::SELECTED))
      sel_types[olist[i]->id]=1;

  for (i=0; i<t; i++)
    if (sel_types[olist[i]->id])
      olist[i]->set_flag(g1_object_class::SELECTED,1);

  i4_free(sel_types);
  
  li_call("redraw");
  
  return 0;
}


li_automatic_add_function(g1_drop_selected, "Objects/Drop Selected");
li_automatic_add_function(g1_select_similar, "Objects/Select Similar");
li_automatic_add_function(g1_select_game_pieces, "Objects/Select Game Pieces");
li_automatic_add_function(g1_floor_selected, "Map/Floor Selected");
li_automatic_add_function(g1_ceil_selected, "Map/Ceil Selected");

//commands/new_level.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void g1_editor_class::open_new_level_window()
{
  enum { MIN_DIM=10, MAX_DIM=150 };
  int uw=g1_map_is_loaded() ? get_map()->width() : 100;
  int uh=g1_map_is_loaded() ? get_map()->height() : 100;

  i4_color_window_class *cw=new i4_color_window_class(500,500, 
                                                      style->color_hint->neutral(), style);
  i4_create_dialog(g1_ges("new_level_dialog"), 
                   cw,
                   style,
                   &new_dialog.name,
                   MIN_DIM, MAX_DIM,  &new_dialog.w, uw,
                   MIN_DIM, MAX_DIM,  &new_dialog.h, uh,
                   this, G1_EDITOR_NEW_OK,
                   this, G1_EDITOR_MODAL_BOX_CANCEL);

  cw->resize_to_fit_children();
  create_modal(cw->width(), cw->height(), "new_title");
  modal_window.get()->add_child(0,0,cw);  
}



g1_map_class *g1_editor_class::create_default_map(int w, int h,
                                                  const i4_const_str &name)
{
  g1_map_class *map=new g1_map_class(i4gets("tmp_savename"));


  map->verts=(g1_map_vertex_class *)malloc((w+1) * (h+1) * 
                                                    sizeof (g1_map_vertex_class));

  map->cells=(g1_map_cell_class *)malloc(w * h * sizeof (g1_map_cell_class));
  map->w=w;
  map->h=h;

  g1_set_map(map);


  int default_tile_type=g1_tile_man.get_default_tile_type();


  w32 x,y,mw=map->width(),mh=map->height();
  for (x=0; x<mw; x++)
  {
    for (y=0; y<mh; y++)
    {
      g1_map_cell_class *c=map->cell((w16)x,(w16)y);
      c->init(default_tile_type, G1_ROTATE_0, i4_F);

      if (x==0 || y==0 || x==mw-1 || y==mh-1)
        c->flags=0;
      else
        c->flags=g1_map_cell_class::IS_GROUND;
    }
  }

  for (x=0; x<mw+1; x++)
    for (y=0; y<mh+1; y++)
      map->vertex(x,y)->init();

  map->current_movie=new g1_movie_flow_class;
  map->current_movie->set_scene(0);

  for (g1_map_data_class *md=g1_map_data_class::first; md; md=md->next)
    md->load(0, 0xffffffff);//set up the required instances

  int old_allow=undo.allow;
  undo.allow=0;
  map->recalc_static_stuff();
  undo.allow=old_allow;

  for (int i=0; i<MAX_VIEWS; i++)
  {
    view_states[i].defaults();
    view_states[i].suggest_camera_mode(G1_EDIT_MODE);

    if (views[i])
      views[i]->view=view_states[i];
  }

  map->set_filename(name);

  map->sky_name=new i4_str("skycloudyblue");

  map->init_lod();

  g1_cwin_man->map_changed();

  g1_lights.defaults();

  g1_player_man.load(0);

  g1_object_type factory_pad=g1_get_object_type("mainbasepad");
  if (factory_pad && w>25 && h>25)
  {
    g1_object_class *o;

    o=g1_create_object(factory_pad);   
    if (o)
    {
      o->x=o->lx=(float)(w/2);
      o->y=o->ly=(float)(h/2);
      o->h=o->lh=map->terrain_height(o->x, o->y);
      o->player_num=g1_player_man.get_local()->get_player_num();

      o->occupy_location();
      o->request_think();
      g1_player_man.get(o->player_num)->add_object(o->global_id);
    }
  }

  i4_user_message_event_class light_event(g1_light_params::GBRIGHTEN);
  i4_kernel.send_event(&g1_e_light,&light_event);
  return map;
}




void g1_editor_class::new_level_from_dialog()
{

  int w,h;

  i4_str::iterator is=new_dialog.w->get_edit_string()->begin();
  w=is.read_number();
  is=new_dialog.h->get_edit_string()->begin();
  h=is.read_number();


  if (!(w>=G1_MIN_MAP_DIMENSION && h>=G1_MIN_MAP_DIMENSION 
        && w<=G1_MAX_MAP_DIMENSION && h<=G1_MAX_MAP_DIMENSION))
  {
    
	
    //create_modal(300, 50, "bad_w_h_title");
    //i4_create_dialog(g1_ges("bad_map_w_h_dialog"), 
    //                 modal_window.get(), style, w, h, 
    //                 G1_MIN_MAP_DIMENSION,
    //                 G1_MAX_MAP_DIMENSION,
    //                 this, G1_EDITOR_MODAL_BOX_CANCEL);
	//modal_window->resize_to_fit_children();
	if (w<G1_MIN_MAP_DIMENSION||h<G1_MIN_MAP_DIMENSION)
		{
		i4_message_box(g1_ges("bad_w_h_title"),g1_ges("bad_w_h_title_too_small"),MSG_OK);
		close_modal();
		return;
		}
	const i4_const_str &txt=g1_ges("bad_w_h_text");
	w32 mapuses=0,totalrec=0;
	mapuses=sizeof(g1_map_class)+(sizeof(g1_map_cell_class)+sizeof(g1_map_vertex_class))*w*h;
	totalrec=(mapuses+65536*1024)/(1024*1024);
	i4_str *exp=txt.sprintf(300,mapuses,totalrec);
	w32 re=i4_message_box(g1_ges("bad_w_h_warning"),*exp,MSG_YES|MSG_NO);
	delete exp;
	if (re==MSG_NO)
		{
		close_modal();
		return;
		}
  }
  
    add_undo(G1_MAP_ALL);

    close_windows();
      
    changed();

    if (g1_map_is_loaded())
      g1_destroy_map();

    i4_str *res_name=g1_get_res_filnename(*new_dialog.name->get_edit_string());
    i4_file_class *fp=i4_open(*res_name);
    
    
    if (fp)
    {
      g1_load_res_info(0,fp,0);
      delete fp;
    }
	else //just create one by copying over the default file.
		{
		i4_copy_file("test.scm",*res_name);
		fp=i4_open(*res_name);
		if (fp)
			{
			li_load("scheme/map_init.scm");
			g1_load_res_info(0,fp,0);
			delete fp;
			}
		else
			{
			i4_message_box("Error copying scm file",
				"While copying the file test.scm for the new level, something odd happened.",MSG_OK);
			}

		
		}
	delete res_name;
	res_name=0;


    g1_initialize_loaded_objects();

    g1_map_class *map=create_default_map(w,h, 
                                         *new_dialog.name->get_edit_string());
  
  
  close_modal();
}

// commands/objects.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


li_object *g1_full_health(li_object *o, li_environment *env)
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);

  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  for (int i=0; i<t; i++)
	  {
      if (olist[i]->selected())
          olist[i]->health=olist[i]->get_max_health();
	  }

  li_call("redraw");

  return 0;
}

class g1_set_health_dialog_class : public i4_color_window_class
{
protected:
  i4_text_window_class *caption;
  i4_text_input_class *input;
public:
  g1_set_health_dialog_class(i4_graphical_style_class *style)
    : i4_color_window_class(300,50,style->color_hint->neutral(),style)
  {
    sw32 health = 0;
    g1_object_class *olist[G1_MAX_OBJECTS];
    sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
        health = olist[i]->health;

    char buf[256];
    sprintf(buf, "%d", health);
        
    caption = new i4_text_window_class(       "Health:",style);
    add_child(10,8,caption); 
    input   = new i4_text_input_class (style, buf,100,40, this);
    add_child(100,5,input);
  }

  void name(char* buffer) { static_name(buffer,"set health dialog"); }

  virtual void receive_event(i4_event *ev)
  {
    switch (ev->type())
    {
      case i4_event::OBJECT_MESSAGE:
      {
        CAST_PTR(tev, i4_text_change_notify_event, ev);
        int found=-1;
        
        i4_str::iterator p = tev->new_text->begin();
        sw32 health = p.read_number();

        g1_object_class *olist[G1_MAX_OBJECTS];
        sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
        
        li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

        for (int i=0; i<t; i++)
          if (olist[i]->selected())
            olist[i]->health=(w16)health;

        li_call("redraw");

        style->close_mp_window(parent);
      } break;

      default:
        i4_color_window_class::receive_event(ev);
        break;
    }
  }
};

static i4_event_handler_reference_class<g1_set_health_dialog_class> g1_set_health_dialog;

li_object *g1_set_health(li_object *o, li_environment *env)
{
  if (!g1_set_health_dialog.get())
  {
    i4_graphical_style_class *style = i4_current_app->get_style();
    g1_set_health_dialog = new g1_set_health_dialog_class(style);
    
    style->create_mp_window(-1,-1, 
                            g1_set_health_dialog->width(),
                            g1_set_health_dialog->height(),
                            "Set Health")
      ->add_child(0,0,g1_set_health_dialog.get());
  }
  return 0;
}
li_automatic_add_function(g1_set_health,"set_health");
li_automatic_add_function(g1_full_health,"full_health");

// commands/resize_level.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void g1_editor_class::open_resize_level_window()
{
  if (get_map())
  {
    create_modal(300, 250, "resize_title");

    i4_create_dialog(g1_ges("resize_dialog"), 
                     modal_window.get(),
                     style,
                     get_map()->width(), get_map()->height(), 

                     G1_MIN_MAP_DIMENSION, G1_MAX_MAP_DIMENSION,
                     &resize_dialog.w, get_map()->width(), 

                     G1_MIN_MAP_DIMENSION, G1_MAX_MAP_DIMENSION,
                     &resize_dialog.h, get_map()->height(),

                     this, G1_EDITOR_RESIZE_PLACE_LT,
                     this, G1_EDITOR_RESIZE_PLACE_CT,
                     this, G1_EDITOR_RESIZE_PLACE_RT,
                     this, G1_EDITOR_RESIZE_PLACE_LC,
                     this, G1_EDITOR_RESIZE_PLACE_CC,
                     this, G1_EDITOR_RESIZE_PLACE_RC,
                     this, G1_EDITOR_RESIZE_PLACE_BL,
                     this, G1_EDITOR_RESIZE_PLACE_BC,
                     this, G1_EDITOR_RESIZE_PLACE_BR,

                     this, G1_EDITOR_RESIZE_MAP_OK,
                     this, G1_EDITOR_MODAL_BOX_CANCEL);
    resize_dialog.orient=4;
  }

}

void g1_editor_class::resize_level() 
{
  int w,h;

  i4_str::iterator is=resize_dialog.w->get_edit_string()->begin();
  w=is.read_number();
  is=resize_dialog.h->get_edit_string()->begin();
  h=is.read_number();
  
  
  if (!(w>=G1_MIN_MAP_DIMENSION && h>=G1_MIN_MAP_DIMENSION 
        && w<=G1_MAX_MAP_DIMENSION && h<=G1_MAX_MAP_DIMENSION))
  {
    //close_modal();
    /*create_modal(300, 50, "bad_w_h_title");
    i4_create_dialog(g1_ges("bad_map_w_h_dialog"), 
                     modal_window.get(), style, w, h, 
                     G1_MIN_MAP_DIMENSION,
                     G1_MAX_MAP_DIMENSION,
                     this, G1_EDITOR_MODAL_BOX_CANCEL);
					 */
	if (w<G1_MIN_MAP_DIMENSION||h<G1_MIN_MAP_DIMENSION)
		{
		i4_message_box(g1_ges("bad_w_h_title"),g1_ges("bad_w_h_title_too_small"),MSG_OK);
		close_modal();
		return;
		}
	const i4_const_str &txt=g1_ges("bad_w_h_text");
	w32 mapuses=0,totalrec=0;
	mapuses=sizeof(g1_map_class)+(sizeof(g1_map_cell_class)+sizeof(g1_map_vertex_class))*w*h;
	totalrec=(mapuses+65536*1024)/(1024*1024);
	i4_str *exp=txt.sprintf(300,mapuses,totalrec);
	w32 re=i4_message_box(g1_ges("bad_w_h_warning"),*exp,MSG_YES|MSG_NO);
	delete exp;
	if (re==MSG_NO)
		{
		close_modal();
		return;
		}
  }
   
  
    if (g1_map_is_loaded())
    {
      int sx1,sy1,sx2,sy2, dx1,dy1,dx2,dy2, x,y, i;
      int dir=resize_dialog.orient;
    
      if (w>=get_map()->width())
      {
        if (dir==0 || dir==3 || dir==6)
          dx1=0;
        else if (dir==1 || dir==4 || dir==7)
          dx1=w/2-get_map()->width()/2;
        else
          dx1=w-get_map()->width();

        dx2=dx1+w-1;

        sx1=0; sx2=get_map()->width()-1;
      }
      else                                    // need to chop width
      {
        if (dir==0 || dir==3 || dir==6)       // chop off the right
          sx1=0;
        else if (dir==1 || dir==4 || dir==7)  // chop off left and right
          sx1=get_map()->width()/2-w/2;
        else 
          sx1=get_map()->width()-w;                // chop off left

        sx2=sx1+w-1;
        dx1=0; dx2=w-1;
      }

    
      if (h>=get_map()->height())
      {
        if (dir==6 || dir==7 || dir==8)
          dy1=0;
        else if (dir==3 || dir==4 || dir==5)
          dy1=h/2-get_map()->height()/2;
        else
          dy1=w-get_map()->height();

        dy2=dy1+h-1;

        sy1=0; sy2=get_map()->height()-1;
      }
      else                                    // need to chop height
      {
        if (dir==6 || dir==7 || dir==8)       // chop off the bottom
          sy1=0;
        else if (dir==3 || dir==4 || dir==5)  // chop off top and bottom
          sy1=get_map()->height()/2-h/2;
        else
          sy1=get_map()->height()-h;                // chop off top

        sy2=sy1+h-1;
        dy1=0; dy2=h-1;
      }


      g1_map_cell_class *ncells=(g1_map_cell_class *)malloc(w * h *sizeof(g1_map_cell_class));

      g1_map_vertex_class *nverts=(g1_map_vertex_class *)malloc((w+1) * (h+1) *
                                                                   sizeof(g1_map_vertex_class)
                                                                   );
      
      // first initial all the new stuff
      g1_map_cell_class *c1=ncells, *c2;
      for (y=0; y<h; y++)
        for (x=0; x<w; x++, c1++)
        {
          c1->init(0, G1_ROTATE_0, i4_F);
          if (x==0 || y==0 || x==w-1 || y==h-1)
            c1->flags=0;
          else
            c1->flags=g1_map_cell_class::IS_GROUND;
        }
      
      g1_map_vertex_class *v1=nverts,*v2;
      for (i=0; i<(w+1)*(h+1); i++, v1++)
        v1->init();

      // copy old area
      int yl=sy2-sy1+1;
      for (y=0; y<yl; y++)
      {
        c1=ncells + (dy1+y)*w + dx1;
        c2=get_map()->cells + (sy1+y)*get_map()->width() + sx1;

        for (x=sx1; x<=sx2; x++, c1++, c2++)
        {
          *c1=*c2;
          c1->object_list=0;
        }
      }

      yl=sy2-sy1+2;
      for (y=0; y<yl; y++)
      {
        v1=nverts + (dy1+y)*(w+1) + dx1;
        v2=get_map()->verts + (sy1+y)*(get_map()->width()+1) + sx1;

        for (x=sx1; x<=sx2+1; x++, v1++, v2++)
          *v1=*v2;
      }   


      // take objects off old map and move them
      g1_object_class *olist[G1_MAX_OBJECTS];
      sw32 t=get_map()->make_object_list(olist, G1_MAX_OBJECTS);
      for (i=0; i<t; i++)
      {
        g1_object_class *o=olist[i];
        o->unoccupy_location();
        o->x+=(dx1-sx1);
        o->y+=(dy1-sy1);

        if (o->x<0 || o->y<0 || o->x>=w || o->y>=h)
        {
          get_map()->request_remove(o);
          g1_remove_man.process_requests();
          olist[i]=0;

        }
        else
        {
          o->lx=o->x;
          o->ly=o->y;
          o->lh=o->h;
        }
      }

      free(get_map()->cells);
      free(get_map()->verts);



      get_map()->cells=ncells;            // swap data with new stuff
      get_map()->verts=nverts;
      get_map()->w=w;
      get_map()->h=h;
      
      // add objects onto the map
      for (i=0; i<t; i++)
        if (olist[i])
          olist[i]->occupy_location();

      // move the movie
      g1_movie_flow_class *movie=get_map()->current_movie;
      if (movie)
      {
        for (i=0; i<(sw16)movie->t_cut_scenes; i++)
          movie->set[i]->move((float)(dx1-sx1), (float)(dy1-sy1), 0);
      }

      get_map()->mark_for_recalc(0xffffff);

    }

    close_modal();
  
}

// commands/rotate_level.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


int g1_rotate_remap[4]={0, 1, 2, 3};

li_object *g1_rotate_map_90(li_object *o, li_environment *env)
{
  if (!g1_map_is_loaded()) return 0;  
  g1_map_class *map=g1_get_map();

  
  int ow=map->width(), oh=map->height(), nw=map->height(), nh=map->width(), size;

  size=sizeof(g1_map_cell_class)*nw*nh;
  //if (size>i4_largest_free_block())
  //{
  //  i4_warning("not enough memory");
  //  return 0;
  //}
  
  g1_map_cell_class *ncells=(g1_map_cell_class *)malloc(size);


  size=sizeof(g1_map_vertex_class)*(nw+1)*(nh+1);
  //if (size>i4_largest_free_block())
  //{
  //  i4_free(ncells);
  //  i4_warning("not enough memory");
  //  return 0;
  //}     
  g1_map_vertex_class *nverts=(g1_map_vertex_class *)malloc(size);

  li_call("add_undo", li_make_list(new li_int(G1_MAP_ALL)));
 
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=map->make_object_list(olist, G1_MAX_OBJECTS), i,x,y;

  for (i=0; i<t; i++)
    olist[i]->unoccupy_location();

  

  // move the cells
  g1_map_cell_class *ocells=map->cell(0,0);
  for (y=0; y<oh; y++)
    for (x=0; x<ow; x++, ocells++)
    {
      int ns=y+x*nw;
      ncells[ns]=*ocells;

      ncells[ns].set_rotation((g1_rotation_type)((g1_rotate_remap[ncells[ns].get_rotation()])));
    }

  // move the vertexes
  g1_map_vertex_class *overts=map->vertex(0,0);
  for (y=0; y<oh+1; y++)
    for (x=0; x<ow+1; x++, overts++)
    {
      int ns=y+x*(nw+1);
      nverts[ns]=*overts;      
    }


  map->change_map(nw, nh, ncells, nverts);

  // move the objects and rotate them
  for (i=0; i<t; i++)
  {
    float x=olist[i]->x;
    olist[i]->x=olist[i]->y;
    olist[i]->y=x;

    olist[i]->theta += i4_pi()/3.0f/2.0f;
    i4_normalize_angle(olist[i]->theta);
    
    olist[i]->grab_old();
  }
  

  for (i=0; i<t; i++)
    olist[i]->occupy_location();

  
  li_call("editor_refresh");
  li_call("editor_changed");
  return 0;
}


li_automatic_add_function(g1_rotate_map_90, "Map/Rotate 90");

// commands/terrain.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void g1_editor_class::flatten_terrain()
{
  g1_editor_instance.unmark_all_selected_verts_for_undo_save();
  g1_editor_instance.mark_selected_verts_for_undo_save();
  g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);


  g1_edit_state.hide_focus();

  int w=get_map()->width()+1, h=get_map()->height()+1;

  w16 lowest=0xffff;

  int t=0,x,y,i;

  i4_status_class *status=i4_create_status(g1_ges("applying_flatten"));

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 to=get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (i=0; i<to; i++)
    olist[i]->unoccupy_location();

  for (y=0; y<h; y++)
  {
    for (x=0; x<w; x++)
    {
      g1_map_vertex_class *v=get_map()->vertex(x,y);
      if (v->is_selected())
      {
        if (v->get_height_value()<lowest)
          lowest=v->get_height_value();

        t++;
      }
    }
  }


  if (t)
  {
    for (y=0; y<h; y++)
    {
      if (status)
        status->update(y/(float)h);

      for (x=0; x<w; x++)
      {
        g1_map_vertex_class *v=get_map()->vertex(x,y);
        if (v->is_selected())
          get_map()->change_vert_height(x,y, (w8)lowest);
      }
    }
  }
  
  if (status)
    delete status;

  for (i=0; i<to; i++)
    olist[i]->occupy_location();

  g1_edit_state.show_focus();


  changed();

  li_call("redraw");
}

void g1_editor_class::smooth_terrain()
{
  g1_editor_instance.unmark_all_selected_verts_for_undo_save();
  g1_editor_instance.mark_selected_verts_for_undo_save();
  g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);

  g1_edit_state.hide_focus();

  int w=get_map()->width()+1, h=get_map()->height()+1;

  w16 lowest=0xffff;

  int t=0,x,y,i;

  i4_status_class *status=i4_create_status(g1_ges("applying_smooth"));

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 to=get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (i=0; i<to; i++)
    olist[i]->unoccupy_location();


  for (y=0; y<h; y++)
  {
    if (status)
      status->update(y/(float)h);
    for (x=0; x<w; x++)
    {
      g1_map_vertex_class *v=get_map()->vertex(x,y);
      if (v->is_selected())
      {
        int t=1;
        float sum=v->get_height(); 
        
        if (x>0) { t++; sum+=v[-1].get_height(); }
        if (x<w-1) { t++; sum+=v[1].get_height(); }

        if (y>0) { t++; sum+=v[-w].get_height(); }
        if (y<h-1) { t++; sum+=v[w].get_height(); }

        sw32 avg=(sw32)(sum/(t  * 0.05));
        if (avg<0) avg=0;
        if (avg>255) avg=255;

        get_map()->change_vert_height(x,y, (w8)avg);
      }
    }
  }

  if (status)
    delete status;

  for (i=0; i<to; i++)
    olist[i]->occupy_location();

  g1_edit_state.show_focus();


  changed();

  li_call("redraw");

}


void g1_editor_class::noise_terrain()
{

  create_modal(300, 150, "terrain_noise_title");

   i4_create_dialog(g1_ges("terrain_noise_dialog"), 
                    modal_window.get(),
                    style,
                    &terrain_noise_dialog.amount,
                    vert_noise_amount,
                    this, G1_EDITOR_TERRAIN_NOISE_OK,
                    this, G1_EDITOR_MODAL_BOX_CANCEL);  
}


void g1_editor_class::noise_terrain_ok()
{
  g1_editor_instance.unmark_all_selected_verts_for_undo_save();
  g1_editor_instance.mark_selected_verts_for_undo_save();
  g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);



  g1_edit_state.hide_focus();

  i4_str::iterator i=terrain_noise_dialog.amount->get_edit_string()->begin();
  int am=i.read_number(),x,y;
  int w=get_map()->width()+1, h=get_map()->height()+1, j;

  i4_status_class *status=i4_create_status(g1_ges("applying_noise"));

  
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 to=get_map()->make_object_list(olist, G1_MAX_OBJECTS);

  for (j=0; j<to; j++)
    olist[j]->unoccupy_location();

  
  if (am>=1 && am<=64)
  {
    for (y=0; y<h; y++)
    {
      if (status)
        status->update(y/(float)h);

      for (x=0; x<w; x++)
      {
        g1_map_vertex_class *v=get_map()->vertex(x,y);
        if (v->is_selected())
        {
          int h=(int)v->get_height_value() + (i4_rand() % am) - am/2;
          if (h<0) h=0;
          if (h>255) h=255;
          get_map()->change_vert_height(x,y, h);
        }
      }
    }
  }


  for (j=0; j<to; j++)
    olist[j]->occupy_location();

  if (status)
    delete status;

  g1_edit_state.show_focus();
  changed();
  li_call("redraw");
  close_modal();
}

// commands/terrain_bitmap.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void g1_editor_class::load_height_bitmap()
{
  i4_create_file_open_dialog(style,
                             g1_ges("load_height_map_title"),
                             g1_ges("load_height_start_dir"),
                             g1_ges("load_height_file_mask"),
                             g1_ges("load_height_mask_name"),
                             this,
                             G1_EDITOR_LOAD_TERRAIN_HEIGHT_OK,
                             G1_EDITOR_NO_OP);
}

void g1_editor_class::save_height_bitmap()
{
  if (get_map())
  {
    i4_filename_struct fn;
    i4_split_path(get_map()->get_filename(), fn);

    char dname[256];
    sprintf(dname, "%s.tga", fn.filename);
    
    i4_create_file_save_dialog(style,
                               dname,
                               g1_ges("save_height_map_title"),
                               g1_ges("save_height_start_dir"),
                               g1_ges("save_height_file_mask"),
                               g1_ges("save_height_mask_name"),
                               this,
                               G1_EDITOR_SAVE_TERRAIN_HEIGHT_OK,
                               G1_EDITOR_NO_OP);
  }
}


void g1_editor_class::save_height_bitmap_ok(i4_event *ev)
{
//  int i;
  CAST_PTR(fo, i4_file_open_message_class, ev);
  
  if (fo->filename && get_map())
  {
    i4_file_class *fp=i4_open(*fo->filename, I4_WRITE);
    if (!fp)
    {
      create_modal(500, 100, "save_height_bad_file_tile");
      i4_create_dialog(g1_ges("save_height_bad_file_dialog"), 
                         modal_window.get(), style, fo->filename, 
                         this, G1_EDITOR_MODAL_BOX_CANCEL);
    }
    else
    {
      i4_pixel_format fmt;
      fmt.default_format();
      
      const i4_pal *pal=i4_pal_man.register_pal(&fmt);

      int mw=get_map()->width()+1, mh=get_map()->height()+1;

      i4_image_class *im=i4_create_image(mw, mh, pal);
      w32 *i32=(w32 *)im->data;

      g1_map_vertex_class *v=get_map()->verts;
      int x,y;
      for (y=0; y<mh; y++)
        for (x=0; x<mw; x++, v++)
        {
          *i32=get_map()->vertex(x,mh-y-1)->get_height_value();
          i32++;
        }


      i4_write_tga(im,fp);
      delete fp;
      delete im;

    }
  }
}




void g1_editor_class::load_height_bitmap_ok(i4_event *ev)
{
  int i;
  CAST_PTR(fo, i4_file_open_message_class, ev);
  
  if (fo->filename && get_map())
  {
    int mw=get_map()->width()+1, mh=get_map()->height()+1;

    
    i4_status_class *stat=i4_create_status(g1_ges("loading_terrain_bitmap"));

    i4_image_class *im=i4_load_image(*fo->filename, stat);

    if (stat)
      delete stat;

    

    if (im)
    {

      if (im->width()!=mw && im->height()!=mh)
      {
        create_modal(500, 100, "load_height_bad_size_title");
        i4_create_dialog(g1_ges("load_height_bad_size_dialog"), 
                         modal_window.get(), style, fo->filename, 
                         im->width(), im->height(), get_map()->width()+1, get_map()->height()+1,
                         this, G1_EDITOR_MODAL_BOX_CANCEL);
        delete im;
      }
      else
      {
        i4_status_class *stat=i4_create_status(g1_ges("applying_terrain_map"));

        const i4_pixel_format *fmt=&im->get_pal()->source;
        i4_draw_context_class c(0,0,mw-1, mh-1);
        g1_map_vertex_class *v=get_map()->verts;

        g1_object_class *olist[G1_MAX_OBJECTS];
        sw32 t=get_map()->make_object_list(olist, G1_MAX_OBJECTS);
        for (i=0; i<t; i++)
          olist[i]->unoccupy_location();

        int x,y;
        for (y=0; y<mh; y++)
        {
          if (stat)
            stat->update(y/(float)mh);

          for (x=0; x<mw; x++, v++)
          {
            w8 h=(w8)(i4_pal_man.convert_32_to(im->get_pixel(x,y,c), fmt) & 0xff);
            get_map()->change_vert_height(x,mh-y-1, h);
          }
          
        }

        if (stat)
          delete stat;
        delete im;

        for (i=0; i<t; i++)
          olist[i]->occupy_location();
      }
    }
    else 
    {
      create_modal(500, 100, "couldn't_load_image_title");
      i4_create_dialog(g1_ges("couldn't_load_image_dialog"), 
                       modal_window.get(), style, fo->filename, this, G1_EDITOR_MODAL_BOX_CANCEL);
    }
  }
}
// commands/undo
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void g1_editor_class::show_undo_state()
{
  int t=undo.tail, t_undos=0, t_redos=0, rt=undo.redo_tail;
  while (t!=undo.head)
  {
    t_undos++;
    t=(t+1)%undo.max;
  }

  while (rt!=undo.head)
  {
    t_redos++;
    rt=(rt+1)%undo.max;
  }   
}


void g1_editor_class::save_undo_info(w32 sections, i4_const_str &fname)
{
  li_call("redraw");

  changed();  // note that map is about to be changed so the user can save if they want

  i4_file_class *fp=i4_open(fname, I4_WRITE|I4_NO_ERROR);
  if (!fp)
  {
    i4_mkdir(g1_ges("undo_dir"));
    fp=i4_open(fname, I4_WRITE);
  }
    
  if (fp)
  {
    g1_saver_class *sfp=new g1_saver_class(fp, i4_T);

    sfp->mark_section(G1_SECTION_MAP_SECTIONS_V1);
    sfp->write_32(sections);            
    get_map()->save(sfp, sections);
    sfp->begin_data_write();

    sfp->mark_section(G1_SECTION_MAP_SECTIONS_V1);
    sfp->write_32(sections);
    get_map()->save(sfp, sections);      
    delete sfp;
  }
}

static int g1_editor_undo_disabled=0;

void g1_editor_class::add_undo(w32 sections)
{
  if (g1_map_is_loaded() && undo.allow &&!g1_editor_undo_disabled)
  {
    sections&=(~G1_MAP_VIEW_POSITIONS);   // don't save changes to the camera positions

    if (sections & G1_MAP_OBJECTS)      // if saving objects, save player info because it has 
      sections |= G1_MAP_PLAYERS;       // object references
    
    
    if (sections & (G1_MAP_CELLS 
                    | G1_MAP_VERTS 
                    | G1_MAP_OBJECTS 
                    | G1_MAP_LIGHTS 
                    | G1_MAP_SELECTED_VERTS))
      get_map()->recalc |= G1_RECALC_RADAR_VIEW;

    if (sections & (G1_MAP_CELLS | G1_MAP_VERTS | G1_MAP_SELECTED_VERTS))
      get_map()->recalc |= G1_RECALC_WATER_VERTS;


    if (!sections) return;

    g1_edit_state.hide_focus();

    if (((undo.head+1) % undo.max) == undo.tail)
      undo.tail = (undo.tail+1)%undo.max;

    int cur_undo=undo.head;

    i4_str *undo_file=g1_ges("undo_file").sprintf(100,cur_undo);

    save_undo_info(sections, *undo_file);


    delete undo_file;

    can_undo=i4_T;
    can_redo=i4_F;

    undo.head=(undo.head+1) % undo.max;
    undo.redo_tail=undo.head;

    g1_edit_state.show_focus();
  }

  show_undo_state();
}


li_object *g1_add_undo(li_object *o, li_environment *env)
{
  g1_editor_instance.add_undo(li_get_int(li_eval(li_car(o,env),env),env));
  return 0;
}

li_object *g1_set_undo(li_object *o, li_environment *env)
	{
	li_object *ob=li_eval(li_car(o,env),env);
	if (ob && ob->type()==LI_INT)
		{
		if (li_int::get(ob,env)->value()==0)
			{
			g1_editor_undo_disabled=i4_T;
			}
		else g1_editor_undo_disabled=i4_F;
		}
	else
		g1_editor_undo_disabled= g1_editor_undo_disabled?0:1;
	return new li_int(g1_editor_undo_disabled);
	}

li_automatic_add_function(g1_add_undo, "add_undo");
li_automatic_add_function(g1_set_undo, "set_undo");


void g1_editor_class::do_undo()
{


  if (g1_map_is_loaded() && undo.redo_tail!=undo.tail && !g1_editor_undo_disabled)
  {
    i4_str *old_name=new i4_str(g1_get_map()->get_filename());

    g1_edit_state.hide_focus();

    undo.redo_tail=(undo.redo_tail + undo.max - 1) % undo.max;

    i4_str *undo_file=g1_ges("undo_file").sprintf(100,undo.redo_tail);
    i4_file_class *fp=i4_open(*undo_file);
    if (fp)
    {
      g1_loader_class *lfp=g1_open_save_file(fp, i4_T);
      if (lfp->goto_section(G1_SECTION_MAP_SECTIONS_V1))
      {
        w32 sections=lfp->read_32();
     
        i4_str *redo_file=g1_ges("redo_file").sprintf(100,undo.redo_tail);        
        save_undo_info(sections, *redo_file);

        delete redo_file;

        undo.allow=0;

        if ((sections|G1_MAP_VIEW_POSITIONS)==G1_MAP_ALL)
          g1_load_level(*undo_file, 1, 0);   // need to reload textures for this one
        else
          get_map()->load(lfp, sections);

        undo.allow=1;
      }

      delete lfp;
    }
    delete undo_file;

    g1_get_map()->set_filename(*old_name);
    delete old_name;

    if (undo.redo_tail==undo.tail)
      can_undo=i4_F;

    can_redo=i4_T;
    li_call("redraw");

    g1_edit_state.show_focus();
  }

  show_undo_state();
}

void g1_editor_class::do_redo()
{
  if (g1_map_is_loaded() && undo.redo_tail!=undo.head)
  {
    i4_str *old_name=new i4_str(g1_get_map()->get_filename());

    g1_edit_state.hide_focus();

    i4_str *redo_file=g1_ges("redo_file").sprintf(100,undo.redo_tail);
    i4_file_class *fp=i4_open(*redo_file);
    if (fp)
    {    
      g1_loader_class *lfp=g1_open_save_file(fp, i4_T);
      if (lfp)
      {
        if (lfp->goto_section(G1_SECTION_MAP_SECTIONS_V1))
        {
          w32 sections=lfp->read_32();
          undo.allow=0;

          if (sections==G1_MAP_ALL)
            g1_load_level(*redo_file, 1, 0);   // need to reload textures for this one
          else
            get_map()->load(lfp, sections);

          undo.allow=1;
        }
        delete lfp;
      }
    }
    delete redo_file;

    g1_get_map()->set_filename(*old_name);
    delete old_name;


    undo.redo_tail = (undo.redo_tail + 1) % undo.max;
    
    if (undo.redo_tail==undo.head)
    {
      can_redo=i4_F;
    }

    li_call("redraw");

    g1_edit_state.show_focus();
  }

  show_undo_state();
}



//path_win.cc
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _WINDOWS
#pragma warning(once:4244)
#endif

enum {
  P1_SET_START, //0
  P1_SET_DESTINATION, //1
  P1_SET_CRITICAL,//2
  P1_LOAD_HOTSPOTS,//3

  P1_SET_GRADE1,//4
  P1_SET_GRADE2,//5
  P1_SET_GRADE3,//6
  P1_SET_GRADE4,//7

  P1_SET_SIZE1,//8
  P1_SET_SIZE3,//9

  P1_RELOAD_MAP,//10

  P1_SET_BLOCK,//11
  //P1_LAY_OBJECT,//12  //Currently, I can't imagine, what these were
  //intended for.

//  P1_STOP,
//  P1_STEP,
  P1_CHOOSE_MAP_SOLVER,//12
  P1_CHOOSE_GRAPH_SOLVER,//13

  P1_RESET_MAP_FLAGS,//14

  P1_FOG_MAP,//15 Restores fog to radar view.

  P1_UNSET_BLOCK_EVENT
};




g1_path_tool_window_class::g1_path_tool_window_class(i4_graphical_style_class *style, 
                                                     i4_event_handler_class *send_to,
                                                     int buttons, i4_image_class **img, 
                                                     i4_const_str **help_names)
  : i4_button_box_class(send_to, i4_F)
//{{{
{
  int x=0,y=0,ny=0;
  for (int i=0; i<buttons; i++)
  {
    if (img[i])
    {
      i4_button_class *b;
        
      i4_user_message_event_class *uev=new i4_user_message_event_class(i);
      i4_event_reaction_class *re=new i4_event_reaction_class(send_to, uev);
        
      b=new i4_button_class(help_names[i], new i4_image_window_class(img[i]), style, re);
      b->set_popup(i4_T);

      if (x+b->width()>40)
      {
        x=0;
        y+=ny;
        ny=0;
      }
      add_button(x,y, b);
      x += b->width();
      ny = b->height()>ny ? b->height() : ny;
    }
  }
    
  resize_to_fit_children();
}
//}}}

w16 g1_path_window_class::map_size_x(g1_map_class *map)
{
	w16 ret=map->width()*CELL_SIZE+RADAR_SIZEX+2;
	if (ret>MAX_MAP_SIZE)
		return MAX_MAP_SIZE;
	return ret;
}
w16 g1_path_window_class::map_size_y(g1_map_class *map)
{
	w16 ret=map->height()*CELL_SIZE>RADAR_SIZEY?map->height()*CELL_SIZE:RADAR_SIZEY;
	if (ret>MAX_MAP_SIZE)
		return MAX_MAP_SIZE;
	return ret;
}
g1_path_window_class::g1_path_window_class(g1_map_class *_map, 
                                           i4_image_class **icons)
  : map(_map), 
    critical_graph(_map->get_critical_graph()),
    i4_parent_window_class(map_size_x(_map),map_size_y(_map))
//{{{
{
  start_icon = icons[0];
  dest_icon = icons[1];
  crit_icon = icons[2];
  
  bitmap=0;
  map_changed = 1;

  grade = 0;
  size = 2;
  tofrom = 0;
  points = 0;
  top_point=0;
  left_point=0;
  maker = map->get_map_maker();
  //maker->make_criticals(map, critical_graph);//should already be ok
  solvemap = new g1_breadth_first_map_solver_class(map->get_block_map(0));
  //solvemap=new Path();
  solvemap_astar=map->get_astar_solver();  //new g1_astar_map_solver_class();
  
  
  solvegraph =map->get_graph_solver(); //new g1_breadth_first_graph_solver_class(critical_graph);
  solvegraph->set_graph(critical_graph);
  solve_mode=SOLVEMODE_MAP;
  solve();
  i4_parent_window_class* radar_overview=
	  g1_create_radar_view(RADAR_SIZEX,RADAR_SIZEY,
	  G1_RADAR_NO_MAP_EVENTS|
	  G1_RADAR_SUPRESS_STATUS|
	  G1_RADAR_CLICK_HOLDS_VIEW|
	  G1_RADAR_EDIT_MODE);
  radar_x_pos=width()-RADAR_SIZEX+1;
  add_child(radar_x_pos,0,radar_overview);
}
//}}}

i4_bool g1_path_window_class::within_map(w16 x, w16 y)
{
	if (x>radar_x_pos-1)
		return i4_F;
	if (x<0)
		return i4_F;
	if (y>height())
	{
		return i4_F;
	}
	if (y<0)
		return i4_F;
	if (bitmap&&x>(bitmap->width()-left_point))
		return i4_F;
	if (bitmap&&y>(bitmap->height()-top_point))
		return i4_F;
	return i4_T;
}

i4_bool g1_path_window_class::within_radar(w16 x, w16 y)
{
	if (x>width())
		return i4_F;
	if (x<radar_x_pos)
		return i4_F;
	if (y>RADAR_SIZEY)
		return i4_F;
	if (y<0)
		return i4_F;
	return i4_T;
}

void g1_path_window_class::map_position(w16& x_pos, w16& y_pos)
{
	float map_scalex=((float)map->width()/(float)RADAR_SIZEX)*CELL_SIZE;
	float map_scaley=((float)map->height()/(float)RADAR_SIZEY)*CELL_SIZE;
	float map_scale=map_scaley<map_scalex?map_scaley:map_scalex;
	x_pos-=radar_x_pos;
	x_pos*=map_scale;
	y_pos*=map_scale;
}

g1_path_window_class::~g1_path_window_class()
//{{{
{
  //if (maker)
  //  delete maker;
  if (solvemap)
    delete solvemap;
  //if (solvegraph)
  //  delete solvegraph;
  if (bitmap)
    delete bitmap;
}
//}}}

w32 g1_path_window_class::critical_color(w16 x, w16 y)
//{{{
{
  w32 crit_col[6] = { 0x200000, 0x002000, 0x000020,
                      0x400000, 0x004000, 0x000040 };
  w32 col=0;
  w16 crit;

  crit = map->cell(x,y)->nearest_critical[grade][tofrom][0];
    
  for (int i=0; i<6; i++)
    if (crit & (1<<i))
      col |= crit_col[i];
    
  return col;
}
//}}}

//g1_breadth_first_map_solver_class *g1_solvemap=0; //(OLI) Debug hack

void g1_path_window_class::solve()
//{{{
{
  i4_time_class start_time;
  i4_time_class end_time;
  
  //w16 throwawaypoints=0;
  //float throwawaypoint[MAX_SOLVEPOINTS*2];
  if (solve_mode&SOLVEMODE_MAP)
	  {
	  start_time.get();
	  solvemap->path_solve(start.x, start.y, dest.x, dest.y, size, size, (w8) grade, point, points);
	  end_time.get();
	  i4_warning("Used %d ms to solve breadth first. Got %d points.",end_time.milli_diff(start_time),(w32)points);
	  end_time.get();
	  solvemap_astar->path_solve(start.x,start.y,dest.x,dest.y,size,size,(w8) grade, 
		  astar_point, astar_points);//indicated in the map, so throw actual result away.
	  start_time.get();
	  i4_warning("Used %d ms to solve A*. Got %d points.",start_time.milli_diff(end_time),(w32)astar_points);
	  }
  else
	  {
	  start_time.get();
	  g1_graph_node start_node = map->cell(start.x,start.y)->nearest_critical[grade][tofrom][0];
      g1_graph_node dest_node = map->cell(dest.x,dest.y)->nearest_critical[grade][tofrom][0];
      solvegraph->path_solve_nodes(start_node, dest_node, size, (w8) grade, point, points);
	  point[0]=dest.x;
	  point[1]=dest.y;
	  end_time.get();
	  i4_warning("Used %d ms to solve using the critical graph. Got %d points.",end_time.milli_diff(start_time),(w32)points);
	  }
//  end_time.get();

  //g1_solvemap = solvemap; //(OLI) Debug hack
  //update to use max corridor size of all path segments
  //i4_warning("Corridor size %f!\n",
  //       map->get_block_map((w8)grade)->line_of_sight(start.x,start.y,dest.x,dest.y));
  //g1_solvemap = 0; //(OLI) Debug hack
}
//}}}
/*
enum { VISITED=g1_map_cell_class::VISITED, 
	    OK=g1_map_cell_class::ROUTEOK,
		VISIBLE=g1_map_cell_class::FOGGED};

*/
static inline i4_bool is_visited(w16 x, w16 y) 
{
return (g1_get_map()->cell(x,y)->flags & g1_map_cell_class::VISITED)!=0; 
}

static inline i4_bool is_ok(w16 x, w16 y)
	{
	return (g1_get_map()->cell(x,y)->flags&g1_map_cell_class::ROUTEOK)!=0;
	}

void g1_path_window_class::draw_to_bitmap()
//{{{
{
  map_changed=0;

  int bmw=map->width()*CELL_SIZE, bmh=map->height()*CELL_SIZE, x,y, i,j, 
    mw=map->width(), mh=map->height(), px, py, ry;

  if (bitmap && (bitmap->width() !=  bmw  ||  bitmap->height() != bmh))
  {
    delete bitmap;
    bitmap=0;
  }

  if (!bitmap)
  {
    i4_pixel_format fmt;
	
    fmt.default_format();
    bitmap=new i4_image32(bmw, bmh, i4_pal_man.register_pal(&fmt)); 
	i4_draw_context_class context(0,0,bmw,bmh);
	bitmap->clear(0,context);
  }

  

  //i4_image32::iterator block_pixel=bitmap->create_iterator(0,0), pixel;
  //w32 *pixel=0;//Must check wether this correctly replaces above line.
  //w32 *block_pixel=bitmap->paddr(0,0);

  
  g1_block_map_class *block_map = map->get_block_map((w8)grade);
  for (ry=0; ry<mh; ry++)
  {
    y = mh-1-ry;// The map was upside down, but why? 
    
    for (x=0; x<mw; x++)
    {
      w32 color = map->vertex(x,y)->get_height_value();
      
      //color |= critical_color(x,y);
      color |=0xff000000;//This is fully visible
      if (is_visited(x,y))
        color |= 0x004f00;
      if (is_ok(x,y))
        color = 0xffff0000;

      if (block_map->is_blocked(x,y, G1_NORTH | G1_SOUTH | G1_EAST | G1_WEST))
        color |= 0x800000;

      //w32 half_color = (color & 0xfefefefe)>>1;
	  //half_color=half_color&0xff000000;

      int blockN, blockW, blockE, blockS;

      //blockN = (
      //          block_map->is_blocked(x,y,G1_NORTH) ||
      //          block_map->is_blocked(x,y,G1_SOUTH));
      //blockW = (
      //          block_map->is_blocked(x,y,G1_WEST) ||
      //          block_map->is_blocked(x,y,G1_EAST));
	  blockN=block_map->is_blocked(x,y,G1_NORTH);
	  blockE=block_map->is_blocked(x,y,G1_EAST);
	  blockS=block_map->is_blocked(x,y,G1_SOUTH);
	  blockW=block_map->is_blocked(x,y,G1_WEST);
	  w32 pixcol=color;
	  const w32 white=0xffffffff;
	  py=(y*CELL_SIZE)+1;
	  px=(x*CELL_SIZE)+1;
	  if (px>bmw-1) px=bmw-1;
	  if (py>bmh-1) py=bmh-1;
	  py=bmh-1-py;//mirror the map
	  //X++ -> E
	  //Y++ -> S
	  bitmap->put_pixel(px-1,py-1,(blockN==1&&blockW==1)?white:pixcol);
	  bitmap->put_pixel(px,py-1,(blockN==1)?white:pixcol);
	  bitmap->put_pixel(px+1,py-1,(blockN==1&&blockE==1)?white:pixcol);
	  bitmap->put_pixel(px-1,py,(blockW==1)?white:pixcol);
	  bitmap->put_pixel(px,py,pixcol);
	  bitmap->put_pixel(px+1,py,(blockE==1)?white:pixcol);
      bitmap->put_pixel(px-1,py+1,(blockW==1&&blockS==1)?white:pixcol);
	  bitmap->put_pixel(px,py+1,(blockS==1)?white:pixcol);
	  bitmap->put_pixel(px+1,py+1,(blockS==1&&blockS==1)?white:pixcol);

	  //old code
	  /*bitmap->put_pixel(px-1,py-1,half_color);
	  bitmap->put_pixel(px,py-1,(blockN==1)?0xffffffff:pixcol);
	  bitmap->put_pixel(px+1,py-1,half_color);
	  bitmap->put_pixel(px-1,py,(blockW==1)?0xffffffff:pixcol);
	  bitmap->put_pixel(px,py,pixcol);
	  bitmap->put_pixel(px+1,py,(blockW==1)?0xffffffff:pixcol);
      bitmap->put_pixel(px-1,py+1,half_color);
	  bitmap->put_pixel(px,py+1,(blockN==1)?0xffffffff:pixcol);
	  bitmap->put_pixel(px+1,py+1,half_color);*/

      //pixel=block_pixel;
	  /*
      for (py=y; py>(y-CELL_SIZE); py--)
      {
        for (px=x; px<(x+CELL_SIZE); px++, ++pixel)
			{
			w32 temp=color;
          //bitmap->iterator_store(pixel, (  (px == 0 & blockW)? 0xffffff 
          //                               : (py == 0 & blockN)? 0xffffff
          //                               : (px|py == 0)? half_color 
          //                               : color ));
			if (blockW && (px==x)) temp=0xffffff;
			if (blockN && (py==y)) temp=0xffffff;
			bitmap->put_pixel(px,py,temp);
		  //bitmap->put_pixel(py,px,(((px==x) && (blockW))?0xffffff
		  //:((py==y) && (blockN))?0xffffff
		  //:((px|py)==0)?half_color:color));
		//bitmap->put_pixel(py,px,color);
			}
        pixel += bitmap->width() - CELL_SIZE;
      }*/

      //block_pixel += CELL_SIZE;

    }
    //block_pixel += (CELL_SIZE-1) * bitmap->width();
  }

  i4_draw_context_class tmp_context(0,0,bitmap->width()-1,bitmap->height()-1);
/*
//#if 0
  //{{{ Draw Section Boundaries
  //Seems to be VERY obsolete code
  test_block_map::CBounds::CBlockPoint *p,*q;
  w16 pi=map->bounds.begin(); 
  while (map->bounds.next_point(pi)) 
  {
    p = map->bounds.get_point(pi);
    for (int l=0; l<4; l++) 
    {
      if (p->edge[l]>pi) {
        q = map->bounds.get_point(p->edge[l]);
        bitmap->line(int(p->x*CELL_SIZE),int((mh-p->y)*CELL_SIZE-1), 
                     int(q->x*CELL_SIZE),int((mh-q->y)*CELL_SIZE-1), 
                     0x808080, tmp_context);
      }
      bitmap->put_pixel(int(p->x*CELL_SIZE),int((mh-p->y)*CELL_SIZE)-1, 0x0080ff, tmp_context);
    }
  }
  //}}}
//#endif
*/
  for (j=1; j<critical_graph->criticals; j++)
  {
    g1_critical_graph_class::critical_point_class *crit = &critical_graph->critical[j];
    for (i=0; i<crit->connections; i++) 
    {
      if (crit->connection[i].size[grade])
      {
        w32 k=crit->connection[i].ref;
        i4_float x1,y1, x2,y2, x3,y3, x4,y4;
        x1 = crit->x*CELL_SIZE + CELL_SIZE/2;
        y1 = (mh*CELL_SIZE-1) - (crit->y*CELL_SIZE + CELL_SIZE/2);
        x4 = critical_graph->critical[k].x*CELL_SIZE + CELL_SIZE/2;
        y4 = (mh*CELL_SIZE-1) - (critical_graph->critical[k].y*CELL_SIZE + CELL_SIZE/2);
        
        x2 = (x4-x1)*0.3f + x1;
        y2 = (y4-y1)*0.3f + y1;
        x3 = (x4-x1)*0.7f + x1;
        y3 = (y4-y1)*0.7f + y1;
        
        //bitmap->line((w16)x1,(w16)y1,(w16)x2,(w16)y2, 
        //             0x000800*(crit->connection[i].size[grade]), tmp_context);
        //bitmap->line((w16)x2,(w16)y2,(w16)x3,(w16)y3, 0x404020, tmp_context);
        bitmap->line(x1,y1,x2,y2,0xff836353,tmp_context);
        bitmap->line(x2,y2,x3,y3,0xfffbab5b,tmp_context);
      }
    }
  }

  point[points*2]=start.x;//re-add first point (not set by solver, since
  point[points*2+1]=start.y;//trying to go there would not make sense
  
  //one more point included bellow
  for (j=points; j>0; j--)    // points solved in backwards order
  {
    w32
      x1 = w32(point[j*2-2])*CELL_SIZE + CELL_SIZE/2,
      y1 = (mh*CELL_SIZE-1) - w32(point[j*2-1])*CELL_SIZE - CELL_SIZE/2,
      x2 = w32(point[j*2+0])*CELL_SIZE + CELL_SIZE/2,
      y2 = (mh*CELL_SIZE-1) - w32(point[j*2+1])*CELL_SIZE - CELL_SIZE/2;

    bitmap->line((short)x1,(short)y1,(short)x2,(short)y2,0xffff0000, tmp_context);
  }

  astar_point[astar_points*2]=start.x;
  astar_point[astar_points*2+1]=start.y;

  for (j=astar_points; j>0; j--)    // points solved in backwards order
  {
    w32
      x1 = w32(astar_point[j*2-2])*CELL_SIZE + CELL_SIZE/2,
      y1 = (mh*CELL_SIZE-1) - w32(astar_point[j*2-1])*CELL_SIZE - CELL_SIZE/2,
      x2 = w32(astar_point[j*2+0])*CELL_SIZE + CELL_SIZE/2,
      y2 = (mh*CELL_SIZE-1) - w32(astar_point[j*2+1])*CELL_SIZE - CELL_SIZE/2;

    bitmap->line((short)x1,(short)y1,(short)x2,(short)y2,0xff00ffcc, tmp_context);
  }

}
//}}}

void g1_path_window_class::parent_draw(i4_draw_context_class &context)
//{{{
{
  int mw=map->width(), mh=map->height();

  if (map_changed)
	  {
	  if (bitmap)
		  {
		  i4_draw_context_class help_context(0,0,bitmap->width()-1,bitmap->height()-1);
	      bitmap->clear(0x0,help_context);
		  }
	  draw_to_bitmap();
	  }
  local_image->clear(0x0,context);  
  bitmap->put_image(local_image, -left_point, -top_point, context);
  

  int x,y;
  x = start.x * CELL_SIZE + CELL_SIZE/2+1 - start_icon->width()/2;
  y = (mh-1-start.y) * CELL_SIZE + CELL_SIZE/2+1 - start_icon->height()/2;

  start_icon->put_image_trans(local_image, x-left_point, y-top_point, 0, context);

  x = dest.x * CELL_SIZE + CELL_SIZE/2+1 - dest_icon->width()/2;
  y = (mh-1-dest.y) * CELL_SIZE + CELL_SIZE/2+1 - dest_icon->height()/2;

  dest_icon->put_image_trans(local_image, x-left_point, y-top_point, 0, context);

  for (int i=0; i<critical_graph->criticals; i++)
  {
    x = sw32(critical_graph->critical[i].x)*CELL_SIZE
      + CELL_SIZE/2+1 - crit_icon->width()/2;
    y = (mh-1-sw32(critical_graph->critical[i].y))*CELL_SIZE
      + CELL_SIZE/2+1 - crit_icon->height()/2;
      
    crit_icon->put_image_trans(local_image, x-left_point, y-top_point, 0, context);
  }
  local_image->bar(radar_x_pos,RADAR_SIZEY,width(),height(),0x0,context);
}
//}}}
  
void g1_path_window_class::receive_event(i4_event *ev)
{
    
  switch (ev->type()) 
  {
    case i4_event::MOUSE_MOVE: 
      //{{{
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
        
      last_x = mev->x; last_y = mev->y;
    } break;
    //}}}
    case i4_event::MOUSE_BUTTON_UP: 
      //{{{
    {
      CAST_PTR(mbev, i4_mouse_button_up_event_class, ev);
	  if (within_map(last_x,last_y))
	  {
        
		int cell_x=(last_x+left_point)/CELL_SIZE,
			cell_y=map->height()-1-((last_y+top_point)/CELL_SIZE);
	        
		// determine type
	        
		switch (mode)
		{
			case P1_SET_START :
			//{{{
			{
			if (mbev->left())
			{
				start.x=cell_x;
				start.y=cell_y;
			}
			else
			{
				dest.x=cell_x;
				dest.y=cell_y;
			}
			solve();
			changed();
			} break;
			//}}}
			case P1_SET_DESTINATION :
			//{{{
			{
			dest.x=cell_x;
			dest.y=cell_y;
			solve();
			changed();
			} break;
			//}}}
			case P1_SET_BLOCK:
			//{{{
			{
			int flags=G1_NORTH | G1_EAST | G1_WEST | G1_SOUTH;
	              
			if (mbev->left())
				map->get_block_map((w8)grade)->block(cell_x, cell_y, flags);
			else
				map->get_block_map((w8)grade)->unblock(cell_x, cell_y, flags);
	              
			//maker->make_criticals(map, critical_graph);
			//solvegraph->set_graph(critical_graph);
			solve();
			changed();
			} break;
			//}}}
			case P1_SET_CRITICAL:
			//{{{
			{
			g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);
			if (map) map->mark_for_recalc(G1_RECALC_CRITICAL_DATA);

			if (mbev->left() && mbev->right())
				critical_graph->criticals=0;
			else if (mbev->left()) 
				critical_graph->add_critical_point((float)cell_x, (float)cell_y);
			else if (mbev->right())
			{
				critical_graph->remove_critical_point((float)cell_x, (float)cell_y);
	              
			}
	            
			request_redraw();
			} break;
			
		
		}
	  }
	  else if (within_radar(last_x,last_y))
	  {	
		  w16 relocate_to_x=last_x,relocate_to_y=last_y;
		  map_position(relocate_to_x,relocate_to_y);
		  top_point=relocate_to_y-height()/2;
		  left_point=relocate_to_x-(radar_x_pos-1)/2;
		  if (top_point<0)
			  top_point=0;
		  if (left_point<0)
			  left_point=0;
		  request_redraw();
	  }

	        
	} break;

    case i4_event::USER_MESSAGE:
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      switch (uev->sub_type) 
      {
        case P1_SET_BLOCK :
        //case P1_LAY_OBJECT :
        case P1_SET_START :
        case P1_SET_DESTINATION :
        case P1_SET_CRITICAL :
        //case P1_STOP : 
        //case P1_STEP :
          mode=uev->sub_type;
          break;

        case P1_SET_GRADE1:
        case P1_SET_GRADE2:
        case P1_SET_GRADE3:
        case P1_SET_GRADE4:
          if (grade == uev->sub_type - P1_SET_GRADE1)
            tofrom = !tofrom;
          else
          {
            grade = (w16) uev->sub_type - P1_SET_GRADE1;
            solvemap->set_block_map(map->get_block_map((w8)grade));
			//Function does not exist
            solve();
          }
          changed();
          break;

        case P1_SET_SIZE1:
        case P1_SET_SIZE3:
          size = (w8) uev->sub_type - P1_SET_SIZE1 + 1;
          solve();
          changed();
          break;

        case P1_RELOAD_MAP:
          //{{{
        {
          g1_editor_instance.add_undo(G1_MAP_CRITICAL_DATA);
          critical_graph = map->get_critical_graph();
          maker->make_criticals(map, critical_graph);
		  solvegraph->set_graph(critical_graph);
		  map->mark_for_recalc(G1_RECALC_BLOCK_MAPS);
		  map->recalc_static_stuff();
          changed();
        } break;
        //}}}

        case P1_LOAD_HOTSPOTS:
          //{{{
        {
          int i,j,n;
          g1_map_cell_class *c=map->cell(0,0);

          int g1_takeover_pad_type=g1_get_object_type("takeover_pad");

          g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);
          for (j=0; j<map->height(); j++)
			  {
			  //c=map->cell(0,j);
            for (i=0; i<map->width(); i++, c++)
            {
              for (g1_object_chain_class *obj=c->get_obj_list(); obj; obj=obj->next)
              {
                if (obj->object->id == g1_takeover_pad_type)
                {
                  int found=0;
                  for (n=0; n<critical_graph->criticals; n++)
                    if (critical_graph->critical[n].x==i &&
                        critical_graph->critical[n].y==j)
                      found = 1;
                  if (!found)
                  {
                    critical_graph->add_critical_point((i4_float)i,(i4_float)j);
                    //critical_graph->critical[critical_graph->criticals].x = (float)i;
                    //critical_graph->critical[critical_graph->criticals].y = (float)j;
                    //critical_graph->criticals++;
                  }
                }
              }
			  }
			  }
              
          changed();
        } break;
		case P1_CHOOSE_MAP_SOLVER:
			solve_mode=SOLVEMODE_MAP;
			solve();
			changed();
			break;
		case P1_CHOOSE_GRAPH_SOLVER:
			solve_mode=SOLVEMODE_GRAPH;
			g1_astar_map_solver_class::clear_solve();
			solve();
			changed();
			break;
		case P1_RESET_MAP_FLAGS:
			g1_astar_map_solver_class::clear_solve();
			//points=0;
			//solve();
			changed();
			break;
		case P1_FOG_MAP:
			{
			//Todo: Check this code, do the inverse.
			int y,x,mw=map->width(),mh=map->height();
			g1_map_cell_class *c;
			for (y=0;y<mh;y++)
				{
				c=map->cell(0,y);
				for (x=0;x<mw;x++,c++)
					{
					c->flags|= g1_map_cell_class::FOGGED;
					}
				}
			g1_map_vertex_class *v=map->vertex(0,0);
			mh++;//one vertex per row more than cells
			mw++;
			for(y=0;y<mh;y++)
				{
				for (x=0;x<mw;x++,v++)
					{
					v->set_flag(g1_map_vertex_class::FOGGED,1);
					}
				}
			
			}
			break;
        //}}}
      }      
    }
    //}}}
  }
}



//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}





