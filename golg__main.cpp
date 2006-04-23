/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "time/profile.h"
#include "window/wmanager.h"
#include "map.h"
#include "tile.h"
#include "app/app.h"
#include "controller.h"
#include "input.h"
#include "objs/map_piece.h"
#include "player.h"
#include "math/random.h"
#include "resources.h"
#include "g1_menu.h"
#include "border_frame.h"
#include "sound/sfx_id.h"
#include "remove_man.h"
#include "saver.h"
#include "mess_id.h"
#include "sound_man.h"
#include "time/time.h"
#include "time/timedev.h"
#include "g1_speed.h"
#include "cwin_man.h"
#include "m_flow.h"
#include "g1_render.h"
#include "objs/defaults.h"
#include "net/startup.h"       // g1 stuff for start network menus
#include "net/server.h"
#include "network/net_prot.h"  // so we can choose a protocol when we start
#include "render/r1_api.h"
#include "g1_texture_id.h"
#include "statistics.h"
#include "map_man.h"
#include "render/tmanage.h"
#include "level_load.h"
#include "lisp/lisp.h"
#include "make_tlist.h"
#include "gui/image_win.h"
#include "font/anti_prop.h"
#include "lisp/li_init.h"
#include "objs/vehic_sounds.h"
#include "tick_count.h"
#include "map_view.h"
#include "demo.h"
#include "image_man.h"
#include "resource.h"
#include "status/status.h"
#include "status/gui_stat.h"

#ifdef _WINDOWS
#include "options.h"
#include "optioninfo.h"
#include "optionsound.h"
#include "optionsdialog.h"
#include "optionextras.h"
#include "optionsheet.h"
#include "eh.h"
#endif
#include "music/stream.h"
#include "main/win_main.h"
#include "gui/smp_dial.h"
#include "device/key_man.h"
#include "editor/editor.h"
#include "file/get_filename.h"

extern HWND current_window_handle;
#include <string.h>

//define this if the network component is linked
//currently, this doesn't work under linux
//It isn't complete anyway.
//This is now done by arch.h

#ifndef NETWORK_INCLUDED
//These definitions are required even if the actual network code
//is not linked.
i4_net_protocol *i4_net_protocol::first=0;

i4_str *i4_get_user_name()
{
  char buf[256];

#ifdef _WINDOWS
  DWORD s=sizeof(buf);
  GetUserName(buf, &s);
#else
  char *gl=getlogin();
  if (gl)
    strcpy(buf,gl);
  else
    strcpy(buf,"unknown");
#endif
  
  return i4_from_ascii(buf);
}
g1_network_time_manager_class *g1_network_time_man_ptr=0;
#endif

i4_profile_class pf_calc_model("calc model");
i4_profile_class pf_calc_model_1("calc model 1");
i4_profile_class pf_calc_model_2("calc model 2");
i4_profile_class pf_calc_model_3("calc model 3");
i4_profile_class pf_calc_model_4("calc model 4");
i4_profile_class pf_calc_model_5("calc model 5");
i4_profile_class pf_calc_model_6("calc model 6");
i4_profile_class pf_calc_model_7("calc model 7");
i4_profile_class pf_calc_model_8("calc model 8");
i4_profile_class pf_calc_model_9("calc model 9");


extern i4_grow_heap_class *g1_object_heap;

static i4_event_handler_reference_class<i4_parent_window_class> loading_window;

int G1_HZ=10;

static li_symbol_ref s_deterministic("deterministic");

static char first_level[80];
extern int last_draw_tick;
extern int frame_locked_mode;
extern i4_stream_wav_player *g1_music_stream;
extern g1_cwin_man_class *m1_maxtool_man;
#ifdef _WINDOWS
//MFC Code
CWnd cwnd;
#endif
class golgotha_app : public i4_application_class
//The one that controls it all
{
private:
  i4_window_class *main_menu;
  i4_bool playing_movie;
  w32 _max_memory;
  
  w32 argc;
  i4_const_str *argv;
  i4_bool need_post_play_load;
  i4_bool start_in_editor;
  i4_bool do_poll_sound_man;
  friend int Thread_Sound_Man_Poller(void *arg);//Heute verwenden wir Spagetticode
  //Thread_Sound_Man_Poller(void *arg);
  struct redraw_later_struct
  {
    i4_bool waiting;
    i4_time_device_class::id id;

    redraw_later_struct() { waiting=i4_F; }    
  } redraw_later;


  virtual w32 max_memory() { return _max_memory; }


  i4_net_protocol *protocol;

public:  
	i4_window_class *get_main_menu() {return main_menu;};
  void handle_no_displays()
  {
#ifdef _WINDOWS
    MessageBox(0,"Golgotha requires DirectX5, DirectX9, or an OpenGL capable card\n"
             "Make sure you have DX5 or later (and are in 16bit color mode)"
             "or that you have the current version of OpenGl installed",
			 "No usable display device found",MB_OK+MB_ICONSTOP);
#else
    printf("Golgotha would be happy to have an OpenGl capable card available\n");
    printf("or at least find an X-Server running somewhere. \n");
    printf("Specify -display <location> to indicate where your X-Server runs.\n");
#endif
    exit(55);
  }
  
  void pre_play_save();
  void post_play_load();
  void start_saved_game(i4_user_message_event_class *e);

  golgotha_app(w32 argc, i4_const_str *argv)
    : argc(argc), argv(argv)
  {
    start_in_editor=i4_F;
    need_post_play_load=i4_F;
    playing_movie=i4_F;
    main_menu=0;
    protocol=0;
	do_poll_sound_man=i4_F;
    _max_memory = 40*1024*1024;//What does this say? Was 10MB
  }

  virtual ~golgotha_app()
	  {

	  }


  void refresh()
  {
    g1_render.main_draw=i4_T;

    if (g1_render.r_api)
      g1_render.r_api->modify_features(R1_LOCK_CHEAT,g1_resources.lock_cheat);

    i4_application_class::refresh();

    g1_render.main_draw=i4_F;
  }

  void grab_time();



  void map_changed() 
  {     
    li_call("redraw");

    if (redraw_later.waiting)
      i4_time_dev.cancel_event(redraw_later.id);

    redraw_later.waiting=i4_T;
    i4_user_message_event_class m(G1_REDRAW_LATER);
    redraw_later.id=i4_time_dev.request_event(this, &m, 500);
  }

  void init();
  
  virtual i4_bool idle() { return i4_F; }

  void calc_model()
  {
    int fixed=li_get_value(s_deterministic.get())==li_true_sym;
	//even if only maxtool is running, we need to call next_frame
	//to finish loading textures
	if (g1_render.r_api&&g1_render.r_api->get_tmanager()) 
		{
		g1_render.r_api->get_tmanager()->next_frame();
		int k=0;
		while(g1_render.r_api->get_tmanager(k))
			{//call next_frame for all tmanagers
			g1_render.r_api->get_tmanager(k)->next_frame();
			k++;
			}
		}

    if (g1_map_is_loaded())
    {
      pf_calc_model.start();
	  if (loading_window.get())
		i4_kernel.delete_handler(loading_window.get());
	  

      i4_time_class now;
      sw32 md;
      int ticks_simulated=0;
      do
      {
        md=now.milli_diff(g1_get_map()->tick_time);
        if (fixed || md>0 || frame_locked_mode)
        {
          //We must skip the think loop if the controller is unavailable
          //for the moment (this happens in rare cases when the mode
          //is just being changed)
          if ((!g1_resources.paused)&&(g1_current_controller.get()))
          {
            pf_calc_model_1.start();
            g1_player_man.think();  // thinks player/team type thoughts
            pf_calc_model_1.stop();

            g1_map_class *map=g1_get_map();
            map->think_objects();

            g1_demo_tick();

            if ((g1_tick_counter&7)==0)
              g1_radar_update();


            g1_remove_man.process_requests();
            ticks_simulated++;
          }
          else
	      {
              g1_get_map()->tick_time.add_milli((1000/G1_HZ));
#ifdef NETWORK_INCLUDED
	      i4_network_poll();
#endif
              ticks_simulated++;
          }

          if (g1_current_controller.get())
            g1_current_controller->update_camera();
          
        }       
      } while (!frame_locked_mode && !fixed && md>0 && ticks_simulated<10);
      
      if (ticks_simulated>=10) // simulation can't catch up with display
		g1_get_map()->tick_time.get();      // catch up the time

      //if (g1_current_controller.get() &&
      //    g1_current_controller->view.get_view_mode()!=G1_EDIT_MODE)
	  if (g1_current_controller.get())
        g1_current_controller->request_redraw(i4_F);


      //g1_sound_man.poll(i4_T);             // update sound effects (play next narative sfx)
	  //Should call this as often as possible, a thread would be best.

      if (playing_movie)
      {
        if (!g1_get_map()->advance_movie_with_time())
        {
          playing_movie=i4_F;

          
          if (need_post_play_load)
          {
            if (g1_resources.paused)
              li_call("Pause");
            post_play_load();
          }
          else if (g1_current_controller.get())
            g1_current_controller->view.suggest_camera_mode(G1_ACTION_MODE);
        }

        li_call("redraw");
      }
      else 
        g1_input.reset_esc();

      pf_calc_model.stop();
    }

    g1_input.acknowledge();          // acknowledge that the keys have been examined    
  }

  void return_to_game();
  void start_new_game();
  void network_menu(); 
  void server_menu();
  void help_screen(char *screen_name, int next_id);
  void win_screen(char *screen_name, i4_str *filename);
  void plot_screen();
  void client_wait_menu();
  void do_main_menu();
  void hide_main_menu();
  void do_options();
  void open_savegame();
  void save_savegame();
  void save_savegame_ok(i4_user_message_event_class *ev);
  void receive_event(i4_event *ev);
  void uninit()
  {
	do_poll_sound_man=i4_F;
	i4_warning("Stopping background sounds");
	i4_thread_sleep(400);
    i4_kernel.unrequest_events(this,
                               i4_device_class::FLAG_DO_COMMAND |
                               i4_device_class::FLAG_END_COMMAND|
							   i4_device_class::FLAG_DISPLAY_CHANGE);
      
    if (redraw_later.waiting)
    {
      i4_time_dev.cancel_event(redraw_later.id);
      redraw_later.waiting=i4_F;
    }
	li_call("def_skys",0,0);//this ensures that even the last dx5 images 
	//get deleted before shutdown
    if (g1_map_is_loaded())
      g1_destroy_map();

    g1_global_id.init();

    g1_unload_object_defaults();

    g1_resources.cleanup();
	s1_unload();//unload sound buffers
    g1_unload_images();


    g1_cwin_man->destroy_views();//unload editor
    g1_cwin_man->uninit();
	m1_maxtool_man->destroy_views();
	m1_maxtool_man->uninit();//unload maxtool
	i4_uninit_gui_status();//shouldn't do this to early.
	hide_main_menu();
	main_menu=0;
	r1_texture_manager_class *tman=0;
    if (g1_render.r_api)
		tman=g1_render.r_api->get_tmanager();
    if (tman) 
		{
		//tman->reset();  //reset is {uninit();init();}
		tman->uninit();
		}
    r1_destroy_api(g1_render.r_api);    
	g1_render.uninstall_font();
    g1_render.r_api=0;

    g1_input.uninit();

    g1_uninitialize_loaded_objects();
    r1_cleanup_texture_lookup_table();
	if (g1_music_stream) delete g1_music_stream;
	g1_music_stream=0;
    i4_application_class::uninit();
	


  }

  void choice_first_level();

  void name(char* buffer) { static_name(buffer,"golgotha_app"); }
} ;

golgotha_app *g1_app=0;

li_object *g1_force_pause(li_object *o, li_environment *env)
	{
	//forces the system to pause, does nothing if already paused
	g1_stop_sound_averages();
	g1_resources.paused=i4_T;
	if (g1_map_is_loaded())
		g1_get_map()->tick_time.get();
	return 0;
	}

li_object *g1_is_paused(li_object *o, li_environment *env)
	{
	if (g1_resources.paused)
		return li_true_sym;
	return li_nil;
	}

li_object *g1_pause(li_object *o, li_environment *env)
{//toggles pause mode
  g1_stop_sound_averages();

  g1_resources.paused=!g1_resources.paused;
  if (g1_map_is_loaded())
    g1_get_map()->tick_time.get();
  return 0;
}

li_object *g1_set_default_level(li_object *o, li_environment *env)
{
  strcpy(first_level, li_get_string(li_eval(li_car(o,env),env),env));
  return 0;
}


void golgotha_app::pre_play_save()
{
  i4_file_class *out=i4_open(i4gets("play_savename"), I4_WRITE);
  if (out)
  {
    g1_saver_class *save=new g1_saver_class(out);

    g1_get_map()->save(save, G1_MAP_ALL);

    if (save->begin_data_write())
      g1_get_map()->save(save, G1_MAP_ALL);

    delete save;

    need_post_play_load=i4_T;
  }
}

void golgotha_app::post_play_load()
{
  i4_str *old_name=new i4_str(g1_get_map()->get_filename());
  if (g1_load_level(i4gets("play_savename"), 0))
    g1_get_map()->set_filename(*old_name);

  delete old_name; 

  need_post_play_load=i4_F;
}


void golgotha_app::do_options()
{ //This displays the wrong options-settings (Only pop-up window)
   /*if (main_menu)
     delete main_menu;

   main_menu=new g1_option_window(wm->width(), wm->height(), this, wm->get_style());
   wm->add_child(0,0,main_menu);*/
   //cwnd.Attach(current_window_handle);
#ifdef _WINDOWS
   ShowCursor(TRUE);
   COptionsheet op("Options");
   op.EnableStackedTabs(FALSE);
   op.DoModal();
   ShowCursor(FALSE);

   //todo: add options for linux
#endif
}


void golgotha_app::network_menu()
{ 
#ifdef NETWORK_INCLUDED
  hide_main_menu();

  main_menu=new g1_startup_window(wm->width(), wm->height(), wm->get_style(), protocol);
  wm->add_child(0,0,main_menu);
#endif
}

void golgotha_app::help_screen(char *screen_name, int next_id)
{
  hide_main_menu();

  main_menu=new g1_help_screen_class(wm->width(), wm->height(), wm->get_style(),
                                     i4gets(screen_name), next_id);

  wm->add_child(0,0,main_menu);
}

void golgotha_app::win_screen(char *screen_name, i4_str *filename)
{
  hide_main_menu();

  main_menu=new g1_win_screen_class(wm->width(), wm->height(), wm->get_style(),
                                     i4gets(screen_name), filename);

  wm->add_child(0,0,main_menu);
}

void golgotha_app::plot_screen()
{
  hide_main_menu();

  main_menu=new g1_help_screen_class(wm->width(), wm->height(), wm->get_style(),
                                     i4gets("plot_screen"), G1_MAIN_MENU);

  wm->add_child(0,0,main_menu);
}

void golgotha_app::server_menu()
{ 
#ifdef NETWORK_INCLUDED
  hide_main_menu();


  main_menu=new g1_server_start_window(wm->width(), wm->height(), 
                                       wm->get_style(), protocol);
  wm->add_child(0,0,main_menu);
#endif
}

void golgotha_app::client_wait_menu()
{ 
#ifdef NETWORK_INCLUDED
  hide_main_menu();

  main_menu=new g1_client_wait_window(wm->width(), wm->height(), 
                                      wm->get_style(),  protocol);
  wm->add_child(0,0,main_menu);
#endif
}



void golgotha_app::do_main_menu()
{

  sw32 ids[]={ G1_RETURN_TO_GAME,
			   G1_START_NEW_GAME,
               G1_SAVEGAME_LOAD,
               G1_SAVEGAME_SAVE,

               G1_NETWORK_MENU,
               G1_OPTIONS,
               G1_QUIT,
               -1 };
  
  hide_main_menu();
  i4_key_man.set_context("menu");
  main_menu=new g1_main_menu_class(wm->width(),
                                   wm->height(),
                                   this,
                                   ids,
                                   wm->get_style());
                                   
                                   
  wm->add_child(0,0,main_menu);
}

li_symbol_ref fo_sym("File/Open");

/*
void check_fo()
{
  li_symbol *org=fo_sym.get();
  li_symbol *news=li_get_symbol("File/Open");
  
  if (org!=news)
    i4_warning("symbols off!");
}*/

void golgotha_app::save_savegame()
{
  if (g1_get_map())
  {
    i4_create_file_save_dialog(get_style(),
                               g1_get_map()->get_filename(),
                               g1_editor_instance.get_editor_string("savegame_save_title"),
                               g1_editor_instance.get_editor_string("savegame_start_dir"),
                               g1_editor_instance.get_editor_string("savegame_file_mask"),
                               g1_editor_instance.get_editor_string("savegame_mask_name"),
                               this,
                               G1_SAVEGAME_SAVE_OK,
                               G1_FILE_OPEN_CANCEL);
  }
  else 
	  i4_message_box("Save your game","No game loaded. Cannot save.",MSG_OK);
}


void golgotha_app::open_savegame()
{
  i4_str *start_dir=0;
  
  start_dir=new i4_str(g1_editor_instance.get_editor_string("savegame_start_dir"));

  i4_create_file_open_dialog(get_style(),
                             g1_editor_instance.get_editor_string("savegame_load_title"),
                             *start_dir,
                             g1_editor_instance.get_editor_string("savegame_file_mask"),
                             g1_editor_instance.get_editor_string("savegame_mask_name"),
                             this,
                             G1_SAVEGAME_LOAD_OK,
                             G1_FILE_OPEN_CANCEL);

  delete start_dir;
}


void golgotha_app::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::DISPLAY_CHANGE :
      {
        CAST_PTR(dev, i4_display_change_event_class, ev);
        if (dev->change==i4_display_change_event_class::SIZE_CHANGE)
        {
          get_root_window()->resize(get_display()->width(),get_display()->height());        
        }
        
      } break;
    case i4_event::DO_COMMAND :
    {
      fo_sym.get();

      g1_cwin_man->receive_event(ev);      
      char *cmd=((i4_do_command_event_class *)ev)->command;
      li_symbol *s=li_get_symbol(cmd);
      if (li_get_fun(s,0))
        li_call(s); 

      g1_input.receive_event(ev);
    } break;

    case i4_event::END_COMMAND :
    {
      g1_cwin_man->receive_event(ev);
      char cmd[200];
      sprintf(cmd, "-%s",((i4_do_command_event_class *)ev)->command);
      li_symbol *s=li_get_symbol(cmd);
      if (li_get_fun(s,0))
        li_call(cmd);
      g1_input.receive_event(ev);
    } break;
      
    case i4_event::USER_MESSAGE :
    {
      CAST_PTR(uev,i4_user_message_event_class,ev);
      switch (uev->sub_type)
      {
       case G1_MAIN_MENU :
          do_main_menu();
          break;

        case G1_NETWORK_MENU :
          network_menu();  
          break;
        case G1_SERVER_MENU :
          server_menu();
          break;
        case G1_CLIENT_JOINED_MENU :
          client_wait_menu();
          break;

        case G1_YOU_LOSE :
          help_screen("youlose_screen", G1_MAIN_MENU);
          break;
          
        case G1_YOU_WIN:
			{
			CAST_PTR(fptr,i4_file_open_message_class,uev);
			win_screen("youwin_screen", fptr->filename);
			}
          break;         

        case G1_HELP_SCREEN :
          help_screen("help_screen", G1_RETURN_TO_GAME);
          break;

        case G1_START_HELP_SCREEN :
          help_screen("help_screen", G1_START_NEW_GAME);
          break;

        case G1_PLOT_SCREEN :
          plot_screen();
          break;

        case G1_START_NEW_GAME :
          start_new_game(); 
          break;

        case G1_RETURN_TO_GAME :
          return_to_game();
          break;

		case G1_SAVEGAME_SAVE:
		  save_savegame();
		break;

		case G1_SAVEGAME_SAVE_OK: 
			save_savegame_ok(uev);		//Saving a savegame must not change
									//the map's filename.
		break;

		case G1_SAVEGAME_LOAD_OK:
			start_saved_game(uev);
		break;

		case G1_SAVEGAME_LOAD:
		  open_savegame();
		break;

        case G1_OPTIONS :
          do_options();
          break;

        case G1_QUIT :
          quit();
          break;
        case G1_ESCAPE :
          do_main_menu();
          break;

        case G1_MAP_CHANGED :
          map_changed();
          break;

        case G1_GET_ROOT_IMAGE :
        {
          CAST_PTR(get, g1_get_root_image_event, ev);
          get->result=display->get_screen();
        } break;


        case G1_REDRAW_LATER :
        {
          redraw_later.waiting=i4_F;
          li_call("redraw_all");
        } break;

        case G1_PLAY_MOVIE :
          if (g1_map_is_loaded())
          {
            pre_play_save();

            if (g1_get_map()->start_movie())
            {
              playing_movie=i4_T;            
              g1_resources.paused=i4_F;
            }

          } break;

        case G1_STOP_MOVIE :
        {
          if (playing_movie)
          {
            if (g1_map_is_loaded())
            {
              g1_get_map()->stop_movie();
              
              post_play_load();
            }

            playing_movie=i4_F;

            if (g1_current_controller.get() &&
                g1_current_controller->view.get_view_mode()==G1_CAMERA_MODE)
              g1_current_controller->view.suggest_camera_mode(G1_ACTION_MODE);
          }
         
        } break;

        case G1_GET_CURRENT_MOVIE :
        {
          if (g1_map_is_loaded())
          {
            CAST_PTR(mev, g1_get_current_movie_event, ev);
            mev->mflow=g1_get_map()->current_movie;
          }

        } break;

        case G1_INTERLACE_PIXEL :
        {
          g1_cwin_man->destroy_views();
          g1_resources.render_window_expand_mode = R1_COPY_1x1_SCANLINE_SKIP;
          g1_cwin_man->create_views();            
        } break;

        case G1_DOUBLE_PIXEL :
        {
          g1_cwin_man->destroy_views();
          g1_resources.render_window_expand_mode = R1_COPY_2x2;
          g1_cwin_man->create_views();            
        } break;

        case G1_NORMAL_PIXEL :
        {
          g1_cwin_man->destroy_views();
          g1_resources.render_window_expand_mode = R1_COPY_1x1;
          g1_cwin_man->create_views();            
        } break;

      }
    } break;

    default:
      i4_application_class::receive_event(ev);
  }
}

void golgotha_app::choice_first_level()
{

  for (int i=1; i<(int)argc; i++)
    if (argv[i]=="-f")
    {
      i4_os_string(*(argv+i+1), first_level, 80);
      i++;
    }
    else if (argv[i]==i4_const_str("-edit"))
    {
      start_in_editor=1;
    }
    else if (argv[i]=="-eval")
    {
      i++;
      char buf[1000];
      i4_os_string(argv[i], buf, 1000);
      char *s=buf;
      li_eval(li_get_expression(s,0), 0);
    }
    else if (argv[i]=="-frame_lock")
      frame_locked_mode=1;
}

int Thread_Sound_Man_Poller(void *arg)
	{
	while (g1_app->do_poll_sound_man)
		{
		g1_sound_man.poll(i4_T);
		i4_thread_sleep(500);
		}
	return 0;
	}



void CheckDXVersion();

union i4_endianesstest{
	w32 anint;
	w8 chars[4];
};


void golgotha_app::init()
{
  sw32 i;
  
  
  for (i=0; i<(int)argc; i++)
  {
    if (argv[i] == "-max_memory")
    {
      i++;
      i4_const_str::iterator p(argv[i].begin());
      _max_memory = (w32)p.read_number();
    }
  }
  char buf[300];
#ifdef _WINDOWS
  LoadString(i4_win32_instance,IDS_INITIALISATION,buf,300);
#else
  printf("Starting initialisation sequence\n");
  strcpy(buf,"Initialisation, please wait...");
#endif
  i4_status_class *status=i4_create_status(buf,0);
  status->update(0.01f);

  r1_truncate_texture_file();
  i4_file_class *inittest=i4_open("resource.res",I4_READ|I4_NO_BUFFER);
  if (!inittest)
	  {
	  //i4_error("_FATAL: resource.res not found in the current directory. "
	//	  "Check that the current directory is the directory the exe-file "
	//	  "resides in.");
#ifdef _WINDOWS
	  MessageBox(0,"FATAL: resource.res not found in the current directory. "
		  "Check that the current directory is the directory the exe-file "
		  "resides in.","Golgotha: Wrong startup directory",0);
#else
	  printf("FATAL: resource.res not found in the current directory. \n"
		  "Check that the current directory is the directory the executable file \n"
		  "resides in.");
#endif
	  exit(89);
	  }
  delete inittest;
  memory_init();//calls init__init::i4_init() to init all i4_init_classes
  status->update(0.1f);
  
  i4_endianesstest end;
  end.anint=0x0ff;
  if (end.chars[0]==0xff)
  {
	i4_warning("Detected architecture is little endian.");
	if (i4_litend!=1)
	{
		i4_error("FATAL: Golgotha was compiled with big-endian settings"
    " included. Please rebuild Golgotha with the correct endianness setting in arch.h");
		exit(92);
        };
  }
  else
  {
	i4_warning("Detected architecture is big endian.");
	if (i4_litend!=0)
	{
		i4_error("FATAL: Golgotha was compiled with little-endian settings"
" included. Please rebuild Golgotha with the correct endianness setting in arch.h");
		exit(92);
	};
  }
  i4_warning("Loading resource Manager...");
  resource_init("resource.res",0);
  strcpy(first_level, "test.level");//Hardcoded first level
  li_add_function("set_default_level", g1_set_default_level);
  i4_warning("Executing scheme/start.scm");
  li_load("scheme/start.scm");
  i4_mkdir("savegame");//create the savegame directory if it doesn't exist
#ifdef _WINDOWS
  CheckDXVersion();
#endif
  status->update(0.2f);
  display_init(); 
  

  status->update(0.23f);
  printf("Display initialisation ok.\n");
  i4_image_class *im=i4_load_image("bitmaps/comic1.jpg");
  printf("If this line is drawn, the jpg loader is ok.\n");
   
  status->update(0.25f);
#ifdef _WINDOWS
  cwnd.Attach(current_window_handle);

  g1_sound_man.poll(i4_T);             // update sound effects (play next narative sfx)
  do_poll_sound_man=i4_T;
  i4_add_thread((i4_thread_func_type)Thread_Sound_Man_Poller,1000,NULL);
  g1_sound_man.loop_current_song=i4_T;
#else
//All sound is currently disabled under linux and the thread synchronization
//code isn't complete either
  do_poll_sound_man=i4_F;
  g1_sound_man.loop_current_song=i4_F;
#endif  
  //wm is the window-manager
  wm->set_background_color(0);


  if (im)//Im is the Cartoon image
  {
    loading_window=new i4_image_window_class(im, i4_T, i4_F);
    wm->add_child(wm->width()/2-loading_window->width()/2,
                  wm->height()/2-loading_window->height()/2,
                  loading_window.get());
  }
  refresh();     // show something right away
  
  status->update(0.3f);
  char *font_fname=li_get_string(li_get_value("font"),0);
  i4_image_class *fim=i4_load_image(font_fname);
  if (!fim) i4_error("image load failed : %s", font_fname);  
  i4_current_app->get_style()->font_hint->normal_font=new i4_anti_proportional_font_class(fim);  
  delete fim;
  status->update(0.4f);



  choice_first_level();
  //Attempt to initialize a rendering api suitable for the current display.
  g1_render.r_api = r1_create_api(display);
  i4_warning("Render api created successfully: %s.",g1_render.r_api->name());
  refresh();
  status->update(0.5f);
  //init the renderer!
  
  if (!g1_render.r_api)
	  {/*
    MessageBox(NULL,"Could not initialize a rendering device. \n"
	"Possible Causes:\n-You tried to run the game in a window "
	"with your desktop color depth not equal to 16Bit.\n"
	"-There's not enough video memory available for this mode.\n"
	"-Your DirectX installation is invalid.\n"
	"-Well, err...","DirectX initialisation failed",
	MB_OK+MB_ICONSTOP+MB_APPLMODAL);*/
#ifdef _WINDOWS
	  char* s=(char*)malloc(1000);
	  
	  LoadString(AfxGetResourceHandle(),IDS_RENDERINITFAILED,s,999);
	  MessageBox(NULL,s,"DirectX",MB_OK+MB_ICONSTOP+MB_APPLMODAL);
	  free(s);
	  FatalExit(99);
#else
	  printf("FATAL ERROR: Initialisation of renderer failed. Quitting.\n");
	  exit(99);
#endif
	  
	  }



  g1_input.init();  

  g1_resources.load();
  li_load("scheme/preferences.scm");
  status->update(0.6f);
  i4_bool movie_time = i4_F;

  for (i=0; i<(int)argc; i++)
  {
    if (argv[i] == i4gets("movie_option"))
      movie_time = i4_T;        
  }

  if (g1_render.r_api->get_render_device_flags() & R1_SOFTWARE)
  {
    //setup a timer and decide whether or not to do double pixel
    //default is high res mode
    int processor_speed = i4_get_clocks_per_second();
  
    g1_resources.render_window_expand_mode = R1_COPY_1x1;
    /*
    if (processor_speed > 170000000)
	    g1_resources.render_window_expand_mode = R1_COPY_1x1;
    else
    if (processor_speed > 140000000)
      g1_resources.render_window_expand_mode = R1_COPY_1x1_SCANLINE_SKIP;
    else    
      g1_resources.render_window_expand_mode = R1_COPY_2x2;

    g1_resources.radius_mode = g1_resource_class::VIEW_LOW;
    */

    if (movie_time)
      g1_resources.radius_mode = g1_resource_class::VIEW_FAR;

  }
  else
  {
    //hardware rasterizer, view distance = far by default
    g1_resources.radius_mode = g1_resource_class::VIEW_FAR;
  }

  s1_load();
  g1_load_images();

  status->update(0.7f);
  g1_player_man.init_colors(&display->get_screen()->get_pal()->source, g1_render.r_api);

  protocol=i4_get_first_protocol();

  i4_graphical_style_class *style=wm->get_style();
  

  /*
  style->color_hint->button.active.bright=(135<<8)|64;
  style->color_hint->button.active.medium=(114<<8)|49;
  style->color_hint->button.active.dark=(88<<8)|38;

  style->color_hint->button.passive.bright=(120<<8)|64;
  style->color_hint->button.passive.medium=(100<<8)|49;
  style->color_hint->button.passive.dark=(70<<8)|38;
  */

  if (!g1_cwin_man) 
    g1_cwin_man=new g1_cwin_man_class;      
  status->update(0.8f);
  g1_cwin_man->init(wm, style, display->get_screen(), display, wm);
  if (m1_maxtool_man)
	  m1_maxtool_man->init(wm,style,display->get_screen(),display,wm);
  li_add_function("Pause", g1_pause);
  li_add_function("ForcePause", g1_force_pause);
  li_add_function("is_paused",g1_is_paused);
  status->update(1.0f);
  
  hide_main_menu();
  main_menu=new g1_help_screen_class(wm->width(), wm->height(), wm->get_style(),
                                      i4gets("startup_screen"), G1_PLOT_SCREEN);
  wm->add_child(0,0,main_menu);
  
  delete status;
  status=0;
  i4_kernel.request_events(this,
                           i4_device_class::FLAG_DO_COMMAND |
                           i4_device_class::FLAG_END_COMMAND|
						   i4_device_class::FLAG_DISPLAY_CHANGE);
  do_main_menu();
  
  if (start_in_editor)
    g1_cwin_man->set_edit_mode(i4_T);

   //start game with startup screens
   //if (!movie_time)
   //{
   //  help_screen("startup_screen", G1_START_HELP_SCREEN);
   //}
   //else
   //{
   //  start_new_game();
   //}

}
  

void golgotha_app::return_to_game()
{
  if (!g1_map_is_loaded())
	  {
	  start_new_game();
	  }
  else if (main_menu)
  {
    hide_main_menu();
  }    
}

void golgotha_app::hide_main_menu()
	{
	if (main_menu)
		delete main_menu;
	i4_key_man.set_context("action");
	main_menu=0;
	}

void golgotha_app::save_savegame_ok(i4_user_message_event_class *ev)
{
  if (g1_get_map())
  {
	
    CAST_PTR(f, i4_file_open_message_class, ev);
	i4_filename_struct fsource,ftarget;

	//this (or other hack) is needed, as source may come from current dir
	//(usually \golgotha). The sprintf then generates a "/filename.scm"
	//source file, wich of course is incorrect.
	i4_str  *srcpath=i4_full_path(g1_editor_instance.get_map()->get_filename());
	i4_split_path(*srcpath,fsource);

	i4_split_path(*f->filename,ftarget);
	if (strlen(ftarget.path)==0)
		strcpy(ftarget.path,".");
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
    g1_editor_instance.save_as(*f->filename);
  }
}

void golgotha_app::start_saved_game(i4_user_message_event_class *e)
{
  //assumes file exists and is valid. Results are undefined if not.
  if (main_menu)
  {
    hide_main_menu();
  }

  g1_cwin_man->destroy_views();
  m1_maxtool_man->destroy_views();
  
  CAST_PTR(f, i4_file_open_message_class, e);
  refresh();//Redisplay the comic-Image
  if (!g1_load_level(*f->filename))//This seems actually to load the level
	  {
      i4_error("ERROR: Could not load your savegame %s. Probable cause: File corrupt or missing access rights.", f->filename->c_str());
	  g1_resources.paused=i4_T;
	  return;
	  }


  if (loading_window.get())
    i4_kernel.delete_handler(loading_window.get());
  
  g1_cwin_man->create_views();
  g1_resources.paused=i4_F;
  if (g1_map_is_loaded() && g1_get_map()->start_movie())
  {
    playing_movie=i4_T;            
    
    
    if (g1_current_controller.get())
      g1_current_controller->view.suggest_camera_mode(G1_CAMERA_MODE);

  }

  g1_sound_man.loop_current_song=i4_F;
  g1_sound_man.next_song();
}

void golgotha_app::start_new_game()
{
  hide_main_menu();
  

  g1_cwin_man->destroy_views();
  m1_maxtool_man->destroy_views();
  
  int no_demo=1;
  for (int i=1; i<(int)argc; i++)
  {
    if (argv[i]=="-no_demo")
      no_demo=1;
    if (argv[i]=="-demo")
      li_call("play_demo");
    else if (argv[i]=="-df")
    {
      i++;
      char fname[100];
      i4_os_string(argv[i], fname,256);
      li_call("play_demo", li_make_list(new li_string(fname), 0));            
    }
  }

  // demo verision for sampler CD, play demo if not started already
  if (!g1_playing_demo() && !no_demo)
    li_call("play_demo");
  refresh();//Redisplay the comic-Image
  if (!g1_playing_demo())
    if (!g1_load_level(first_level))//This seems actually to load the level
		{
        i4_error("ERROR: Could not load level %s.", first_level);
		return;
		}


  if (loading_window.get())
    i4_kernel.delete_handler(loading_window.get());
  
  g1_cwin_man->create_views();

  if (g1_map_is_loaded() && g1_get_map()->start_movie())
  {
    playing_movie=i4_T;            
    g1_resources.paused=i4_F;
    
    if (g1_current_controller.get())
      g1_current_controller->view.suggest_camera_mode(G1_CAMERA_MODE);

  }

  g1_sound_man.loop_current_song=i4_F;
  g1_sound_man.next_song();
}

void myUnexpectedExit()
	{
	i4_error("Quitting for unknown reason");
	}

void i4_main(w32 argc, i4_const_str *argv)
{  
#ifdef _WINDOWS
  set_unexpected(myUnexpectedExit);
#endif
  //golgotha_app app(argc, argv);
  g1_app=new golgotha_app(argc,argv);
  g1_app->run();  
#ifdef _WINDOWS
  cwnd.Detach();//Detach the MFC from the Main-Window, we are clearing up manually
#endif
  delete g1_app;
  g1_app=NULL;
}

li_object *run_ivcon(li_object *o, li_environment *env)
	{
	w32 r=i4_message_box("Run IVCON","IVCON modal starten?",MSG_YESNOCANCEL);
	if (r==MSG_CANCEL) return 0;
	i4_const_str s("ivcon/ivcon.exe");
	i4_exec(s,r==MSG_YES?i4_T:i4_F,i4_const_str("IVCON - 3D Graphics File Converter"));
	return 0;
	}

static li_object *debug_windows(li_object *o, li_environment *env)
	{
	g1_app->get_root_window()->debug_show();
	return 0;
	}

static li_object *main_menu(li_object *o, li_environment *env)
	{
	g1_app->do_main_menu();
	return 0;
	}
static li_object *g1app_global_options(li_object *o, li_environment *env)
	{
	g1_app->do_options();
	return 0;
	}

static li_object *show_help(li_object *o, li_environment *env)
	{
	if(!(g1_app->get_main_menu()))
		g1_app->help_screen("help_screen",G1_RETURN_TO_GAME);
	return 0;
	}

li_object *hide_main_menu(li_object *o, li_environment *env)
	{
	g1_app->hide_main_menu();
	return 0;
	}

li_automatic_add_function(main_menu,"quit_demo");
li_automatic_add_function(show_help,"Show_Help");
li_automatic_add_function(g1app_global_options,"global_options");
li_automatic_add_function(hide_main_menu,"Hide_Main_Menu");
li_automatic_add_function(run_ivcon,"Run_IVCON");
li_automatic_add_function(debug_windows,"show_all_windows");

