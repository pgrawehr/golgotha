/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
//#include "max.h"
#include "pch.h"
//#include "app/app.h"
#include "maxtool/render2.h"
#include "obj3d.h"
#include "load3d.h"


#include "menu/textitem.h"
#include "gui/text.h"
#include "gui/button.h"
#include "window/window.h"
#include "window/colorwin.h"
#include "gui/text_input.h"
#include "device/keys.h"
#include "gui/browse_tree.h"
#include "string/string.h"
#include "gui/text_scroll.h"
#include "error/error.h"
#include "error/alert.h"
//#include "version.h"
#include "maxtool/id.h"
#include "maxtool/tupdate.h"
#include "status/gui_stat.h"
#include "render/tmanage.h"
#include "g1_limits.h"
#include "saver_id.h"
#include "render/r1_win.h"
#include "g1_tint.h"
#include "file/file.h"
#include "file/get_filename.h"
#include "loaders/dir_load.h"
#include "gui/deco_win.h"
#include "maxtool/m1_info.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "maxtool/st_edit.h"
#include "gui/image_win.h"
#include "gui/li_pull_menu.h"
#include "device/key_man.h"
#include "menu/pull.h"
#include "font/anti_prop.h"
#include "main/main.h"
#include "render/r1_font.h"
#include "cwin_man.h"
#include "controller.h"
#include "mess_id.h"
#include "editor/dialogs/debug_win.h"

//#include <string.h>

#ifdef _WINDOWS
//#include "render/dx5/r1_dx5.h"
#include "maxcomm.h"
#include "main/win_main.h"
#endif

//int G1_HZ=10;

//#include <stdio.h>


r1_render_api_class *api=0;

#ifdef MAXCOMM_HH
m1_mail_slot_class slot;
#endif

i4_str *current_model_name=0;


i4_text_scroll_window_class *m1_warn_window=0;

int m1_max_mip_level=64;
/*

//use i4_error() and i4_warning() instead of these!
int m1_warning(const char *format, va_list &ap)
{//update to use common warning queue
  char st[500];
  vsprintf(st,format,ap);

  if (m1_warn_window)
  {
    m1_warn_window->output_string(st);
    m1_warn_window->output_char((i4_char)('\n'));
  }
  else
    fprintf(stderr,"Warning : %s\n",st);

  return 0;
}
*/

/*
int m1_alert(const i4_const_str &ret)
{
  i4_const_str::iterator s=ret.begin();
  while (s!=ret.end())
  {
    m1_warn_window->output_char(s.get());
    ++s;
  }
  m1_warn_window->output_char((i4_char)('\n'));

  return 0;
}*/

void m1_add_to_list(i4_array<i4_str *> &t_arr, i4_str *tname)
{
  int found=0;
  for (int j=0; !found && j<t_arr.size(); j++)
    if ( (*t_arr[j])==(*tname))
      found=1;

  if (!found)
    t_arr.add(tname);
  else
    delete tname;  
}

class m1_utility_init_class:public i4_init_class
	{
	public:
		virtual void init(void)
			{
		  i4_string_man.load("resource/fly_util.res");
#ifdef MAXCOMM_HH
		  slot.create(i4gets("slot_name"));
#endif
		  m1_info.init(i4_global_argc,i4_global_argv);
			}
		virtual void uninit(void)
			{
			m1_info.uninit();
			}
	}m1_utility_init_class_instance;

g1_cwin_man_class *m1_maxtool_man=0;
class m1_utility_app_class : public g1_cwin_man_class
//public i4_application_class
{
protected:
  m1_poly_object_class *obj;
  i4_float theta,phi,dist;
  m1_utility_window_class *util_win;
  i4_text_input_class *tname_edit;
  i4_button_class *browse_button;
  r1_render_window_class *rwin;
  i4_display_class *display;
  i4_bool maxtool_mode;
  r1_render_api_class *api;
  i4_window_class *st_edit;
  i4_parent_window_class *deco,*deco2;
  i4_window_manager_class *wm;
  i4_pull_menu_class *menu;
  i4_bool need_save;

public:
	m1_utility_app_class()
		{
		obj=0;
		maxtool_mode=i4_F;
		tname_edit=0;
		browse_button=0;
		m1_maxtool_man=this;
//others inited on init()
		}
//   i4_bool get_display_name(char *name, int max_len)
//   {
//     strcpy(name, "Windowed GDI");
//     return i4_T;
//   }


  char *name() { return "m1_utility_app_class"; }

  void update_fly(const i4_const_str &name, i4_bool check)
  {
    util_win->set_object(name);
  }


  i4_window_class *create_st_edit()
  {    
    tname_edit=new i4_text_input_class(style,
                                       "",
                                       256,
                                       200, this,
                                       wm->get_style()->font_hint->small_font);
                                                             

    m1_st_edit=new m1_st_edit_window_class(256,256, tname_edit);

    i4_deco_window_class *st_deco=new i4_deco_window_class(m1_st_edit->width(),
                                                           m1_st_edit->height(),
                                                           i4_F,
                                                           style);

    st_deco->add_child((sw16)st_deco->get_x1(), (sw16)st_deco->get_y1(), m1_st_edit.get());
                                                           
	browse_button=new i4_button_class(0,
		new i4_text_window_class("Browse",
		    style,
			style->font_hint->small_font),
		style,
		new i4_event_reaction_class(this, 
		    new i4_user_message_event_class(M1_BUTTON_BROWSE)
		));
	browse_button->set_popup(i4_T);
    tname_edit->resize(st_deco->width()-browse_button->width(), tname_edit->height());

   
    i4_deco_window_class *tname_deco=new i4_deco_window_class(st_deco->width(),
                                                              st_deco->height() +
                                                              tname_edit->height(),
                                                              i4_T,
                                                              style);

    tname_deco->add_child((sw16)tname_deco->get_x1(), (sw16)tname_deco->get_y1(), tname_edit);
	tname_deco->add_child((sw16)tname_deco->get_x1() + tname_edit->width(),
		(sw16)tname_deco->get_y1(),browse_button);
    tname_deco->add_child((sw16)tname_deco->get_x1(), (sw16)(tname_deco->get_y1() + tname_edit->height()),
                          st_deco);
                                                              
    
    return tname_deco;
  }

  void receive_event(i4_event *ev)
  {
    switch (ev->type())
    {      
      case i4_event::DISPLAY_CHANGE :
      {
        CAST_PTR(dev, i4_display_change_event_class, ev);
        if (dev->change==i4_display_change_event_class::SIZE_CHANGE)
        {
          int nw=i4_current_app->get_display()->width();
          int nh=i4_current_app->get_display()->height();

          
          m1_warn_window->resize(nw,m1_warn_window->height());
          
          rwin->resize(nw-rwin->x(), nh-m1_warn_window->height()-rwin->y());         
        }
        
      } break;


      case i4_event::OBJECT_MESSAGE :
      {
        CAST_PTR(tc, i4_text_change_notify_event, ev);
        if (tc->object==tname_edit && tc->new_text && m1_info.obj)
          m1_st_edit->change_current_texture(*tc->new_text);

      } break;

      case i4_event::DO_COMMAND :
      {
        CAST_PTR(dev, i4_do_command_event_class, ev);
        //if (!strcmp(dev->command,"quit"))
        //  quit();
        //else 
        //{
          li_object *f=li_get_fun(li_get_symbol(dev->command), 0);
          if (f)
          {
            li_function_type fun=li_function::get(f,0)->value();
            fun(0,0);
          }
        //}

      } break;

      case i4_event::USER_MESSAGE :
      {
        CAST_PTR(uev, i4_user_message_event_class, ev);
        if (uev->sub_type >= M1_SET_PLAYER_NUMBER)
        {
          util_win->set_current_player((w8)(uev->sub_type-M1_SET_PLAYER_NUMBER));
          util_win->request_redraw(i4_F);
        } else
        {
          switch (uev->sub_type)
          {

			  case M1_BUTTON_BROWSE:
				  {
				  i4_create_file_open_dialog(style,
					  "Browse for Texture",
					  "textures",
					  "*.tga;*.jpg",
					  "Texture files",
					  this,
					  M1_BUTTON_BROWSE_OK,
					  M1_BUTTON_BROWSE_CANCEL);
				  }
				  break;
			  case M1_BUTTON_BROWSE_CANCEL://don't do anything
				  break;
			  case M1_BUTTON_BROWSE_OK:
				  {
				  CAST_PTR(file,i4_file_open_message_class,ev);
				  tname_edit->change_text(*file->filename,i4_T);
				  }
				  break;

            case M1_NEXT :        
            {
              if (m1_info.models.size())
              {
                m1_info.current_model=(m1_info.current_model+1)%m1_info.models.size();
                update_fly(*m1_info.models[m1_info.current_model], i4_T);
              } 
            } break;

            case M1_LAST :
            {
              if (m1_info.models.size())
              {
                m1_info.current_model=(m1_info.current_model+m1_info.models.size()-1) %
                  m1_info.models.size();
                update_fly(*m1_info.models[m1_info.current_model], i4_T);
              } 
            } break;

            case M1_RECENTER :
            {
              util_win->recenter();
			  util_win->request_redraw(i4_F);
            } break;

            case M1_AXIS :
              m1_info.set_flags(M1_SHOW_AXIS, ~m1_info.get_flags(M1_SHOW_AXIS));
              util_win->request_redraw(i4_F);
              break;


            case M1_QUIT :
              li_call("File/Exit");//Wrap this call (actually, M1_QUIT shan't be used)
              break;
			case G1_DEBUG_SEND_EVENT:
				{
			    CAST_PTR(mev,g1_send_debug_message_class,ev);
				if (m1_warn_window)
					{
					m1_warn_window->output_string(mev->msg);
					m1_warn_window->request_redraw();			
					}
				break;
				}
          }
        }

      } break;
    }

    g1_cwin_man_class::receive_event(ev);
  }

  virtual void init(i4_parent_window_class *_parent,
                             i4_graphical_style_class *_style,
                             i4_image_class *_root_image,
                             i4_display_class *_display,
                             i4_window_manager_class *_wm)
	  {
	  g1_cwin_man_class::init(_parent,_style,_root_image,_display,_wm);
	  display=_display;
	  wm=_wm;
      m1_info.r_api=r1_render_api_class_instance;//use current device
	  api=r1_render_api_class_instance;
	  api->install_new_tmanager(m1_info.tman_index);
	  g1_init_color_tints(api);
	  i4_image_class *im=i4_load_image("bitmaps/golg_font_18.tga");
	  if (!im) 
		  {
		  i4_error("CRITICAL: Unable to load required font file: bitmaps/golg_font_18.tga. Maxtool may show unexspected behaviour.");
		  }
      if (m1_info.r_font)
        delete m1_info.r_font;
      m1_info.r_font=new r1_font_class(api, im);
      delete im;

      theta=0;
      phi=0;
      dist=40;
      obj=0;
	  util_win=0;
	  tname_edit=0;
	  rwin=0;
	  st_edit=0;
	  maxtool_mode=i4_F;
	  deco=0;deco2=0;
	  menu=0;
	  need_save=i4_F;
	  

	  //Must check wheter following code is needed.
	  	  //if (!style->font_hint->normal_font)
//		style->font_hint->normal_font=style->font_hint->small_font;
//
//      i4_image_class *im=i4_load_image("maxtool_font.tga");
//      style->font_hint->normal_font=new i4_anti_proportional_font_class(im);
//      delete im;



	  }
//old method, used only if not included in golgotha
  //void init() //init the maxtool. Assumes i4 has been initialized successfully.
  //{
    //i4_set_max_memory_used(10*1024*1024);
    //memory_init();

    //char *cd_name="cd_image_name = golgotha.cd";
    //i4_string_man.load_buffer(cd_name, "");


    //resource_init("res/fly_util.res", 0); 


//#ifdef MAXCOMM_HH
//    slot.create(i4gets("slot_name"));
//#endif

    //m1_info.init(i4_global_argc, i4_global_argv);
    //if (m1_info.update_mode)
    //  li_call("update_all_textures");
    //else
    //{    
      //display_init();
	  //r1_dx5_class_instance=new r1_dx5_class();
      //api = r1_create_api(display);
      //if (!api)
      //  i4_error("could not create a render api for this display");
      //m1_info.r_api=r1_render_api_class_instance;//use current device


      //g1_init_color_tints(api);


      //theta=0;
      //phi=0;
      //dist=40;

      //obj=0;

      //i4_graphical_style_class *style=wm->get_style();
	  //if (!style->font_hint->normal_font)
//		style->font_hint->normal_font=style->font_hint->small_font;
//
//      i4_image_class *im=i4_load_image("maxtool_font.tga");
//      style->font_hint->normal_font=new i4_anti_proportional_font_class(im);
//      delete im;


    
      //i4_key_man.load("res/keys.res");//Actually, I don't think this is still required (we have one single keys.res file)
      //i4_key_man.set_context("maxtool");

  //We aren't the i4_application object anymore, so there's a parent doing this for us.
      //i4_kernel.request_events(this, 
      //                         i4_device_class::FLAG_DO_COMMAND |
      //                         i4_device_class::FLAG_END_COMMAND);


  //    i4_pull_menu_class *menu=li_create_pull_menu("scheme/menu_maxtool.scm");
  //    menu->show(wm,0,0);


/*      i4_color fore=wm->i4_read_color_from_resource("warning_window_fore");
      i4_color back=wm->i4_read_color_from_resource("warning_window_back");

    

      w32 l,t,r,b;
      style->get_in_deco_size(l,t,r,b);
      i4_parent_window_class *deco=new i4_deco_window_class(wm->width()-(l+r), 80, i4_T, style);

      m1_warn_window=new i4_text_scroll_window_class(wm->get_style(),
                                                     fore, back,
                                                     deco->width()-(l+r),
                                                     deco->height()-(t+b));   

      m1_warn_window->output_string("Flytool warning messages.....\n");
      deco->add_child(l,t, m1_warn_window);*/

//#ifndef __linux
      //    i4_set_warning_function(m1_warning);
//      i4_set_alert_function(m1_alert);
//#endif


    

//      i4_window_class *st_edit=create_st_edit();
//      wm->add_child(0,wm->height()-deco->height()-st_edit->height(), st_edit);


    
//      wm->add_child(0, menu->height(),
//                    new i4_deco_window_class(st_edit->width(),
//                                             wm->height()-deco->height()-st_edit->height()-menu->height(),
//                                             i4_T, style));

      //     wm->add_child(0, menu->height(), 
      //                   new i4_image_window_class(i4_load_image("fly_tool_logo.jpg"), i4_T, i4_F));


//      int rw=wm->width()-st_edit->width(), 
//        rh=wm->height()-deco->height()-menu->height();

//      rwin=api->create_render_window(rw, rh, R1_COPY_1x1);

//      util_win=new m1_utility_window_class(rw, rh, api, wm, theta, phi, dist);
//      m1_render_window=util_win;

//      util_win->init();
    
//      rwin->add_child(0,0, util_win);
//      wm->add_child(st_edit->width(), menu->height(), rwin);


      /*    i4_parent_window_class *warn=
          wm->add_mp_window(wm, control_window->width(),m1_util_win->height(),
          m1_warn_window->width(),
          m1_warn_window->height(),
          i4gets("warn_title")); */

    //    warn->add_child(0,0, m1_warn_window);
   //   wm->add_child(0,util_win->height()+
     //               menu->height(), deco);
    

     // i4_kernel.request_events(this, i4_device_class::FLAG_DISPLAY_CHANGE);

   //   i4_init_gui_status(wm, display);
    //}

//  }

virtual void create_views()
{
  if (maxtool_mode)
  {
    /*sw32 x1=0,
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
    
    g1_current_controller=views[0];*/
      i4_color fore=wm->i4_read_color_from_resource("warning_window_fore");
      i4_color back=wm->i4_read_color_from_resource("warning_window_back");

    

      w32 l,t,r,b;
      style->get_in_deco_size(l,t,r,b);
      deco=new i4_deco_window_class((w16)(wm->width()-(l+r)), 80, i4_T, style);

      m1_warn_window=new i4_text_scroll_window_class(wm->get_style(),
                                                     fore, back,
                                                     (w16)(deco->width()-(l+r)),
                                                     (w16)(deco->height()-(t+b)));   

      m1_warn_window->output_string("Maxtool is ready...\n");
      deco->add_child((sw16)l,(sw16)t, m1_warn_window);

#ifndef __linux
      //    i4_set_warning_function(m1_warning);
      //i4_set_alert_function(m1_alert);
#endif


    
      st_edit=create_st_edit();
      wm->add_child(0,wm->height()-deco->height()-st_edit->height(), st_edit);


    
      wm->add_child(0, menu->height(),
                    deco2=new i4_deco_window_class(st_edit->width(),
                                             wm->height()-deco->height()-st_edit->height()-menu->height(),
                                             i4_T, style));

      //     wm->add_child(0, menu->height(), 
      //                   new i4_image_window_class(i4_load_image("fly_tool_logo.jpg"), i4_T, i4_F));


      int rw=wm->width()-st_edit->width(), 
      rh=wm->height()-deco->height()-menu->height();

      rwin=api->create_render_window(rw, rh, R1_COPY_1x1);

      util_win=new m1_utility_window_class(rw, rh, api, wm, theta, phi, dist);
      m1_render_window=util_win;

      util_win->init();
    
      rwin->add_child(0,0, util_win);
      wm->add_child(st_edit->width(), menu->height(), rwin);


      /*    i4_parent_window_class *warn=
          wm->add_mp_window(wm, control_window->width(),m1_util_win->height(),
          m1_warn_window->width(),
          m1_warn_window->height(),
          i4gets("warn_title")); */

    //    warn->add_child(0,0, m1_warn_window);
      wm->add_child(0,util_win->height()+
                    menu->height(), deco);
	  g1_current_controller=0;//util_win;//Not shure what this pointer is needed for
    

      //i4_kernel.request_events(this, i4_device_class::FLAG_DISPLAY_CHANGE);

      //i4_init_gui_status(wm, display);
    //}
  }
  else
    g1_cwin_man_class::create_views();
}

void destroy_views()
{
  if (maxtool_mode)
  {
    /*for (w32 i=0; i<(w32)t_views; i++)
    {
      views[i]->hide_focus();
      views[i]->view=view_states[i];

      delete view_wins[i];
      views[i]=0;
      view_wins[i]=0;
    } */     
  if (m1_warn_window) delete m1_warn_window;
  m1_warn_window=0;
  if (rwin) delete rwin;
  rwin=0;
  if (st_edit) delete st_edit;
  st_edit=0;
  delete menu;
  menu=0;
  delete deco;delete deco2;
  deco=deco2=0;
  g1_current_controller=0;


  } 
  //else //does it any harm if we always call this?
  g1_cwin_man_class::destroy_views();
}
  
i4_bool is_edit_mode() {return maxtool_mode;}
virtual void set_edit_mode(i4_bool yes_no){
	//show our views, set context
	//code is quite like g1_editor_class::set_edit_mode
	if (yes_no)
		{
		i4_unlink(i4gets("play_savename"));
		i4_user_message_event_class movie_stop(G1_STOP_MOVIE);  // stop game movie if there was one
		i4_kernel.send_event(i4_current_app, &movie_stop);
		need_save=i4_T;
		g1_cwin_man_class::destroy_views();   // kill the normal game views
		maxtool_mode=i4_T;
		g1_change_key_context(G1_MAXTOOL_MODE);
		if (menu) delete menu;
		menu=0;
		menu=li_create_pull_menu("scheme/menu_maxtool.scm");
		menu->show(parent, 0,0);
		
		create_views();
		li_call("Pause");
		i4_kernel.request_events(this, 
                               i4_device_class::FLAG_DISPLAY_CHANGE|
							   i4_device_class::FLAG_USER_MESSAGE);
		}
	else
		{
		i4_kernel.unrequest_events(this,
							   i4_device_class::FLAG_DISPLAY_CHANGE|
							   i4_device_class::FLAG_USER_MESSAGE);
		if (menu) menu->hide();
		delete menu;
		menu=0;
		
		//close_windows();
		
		destroy_views();
		
		
		maxtool_mode=i4_F;
		g1_cwin_man_class::create_views();   // create the normal game views
		
		li_call("Pause");
		}
	}
  
  
  void kill_obj()
  {
    
	  if (m1_info.tman_index!=-1)
		  {
		r1_texture_manager_class *tman=api->get_tmanager(m1_info.tman_index);

		tman->reset();
		  }
      m1_info.textures_loaded=i4_F;
	if (obj)
    {
      delete obj;

      obj = 0;
    }    
  }

  virtual void uninit()
	  {
	  //This needs to be done explicitly because set_edit_mode(false)
	  //will not be called when the user quits directly from maxtool.
	  i4_kernel.unrequest_events(this,
							   i4_device_class::FLAG_DISPLAY_CHANGE|
							   i4_device_class::FLAG_USER_MESSAGE);
	  kill_obj();
	  if (menu) delete menu;
		  menu=0;
	  m1_info.uninit();
	  /*if (m1_info.r_font)
        delete m1_info.r_font;
	  m1_info.r_font=0;*/
	  g1_cwin_man_class::uninit();
	  }
  
  void m1_load_next()
  {
#ifdef MAXCOMM_HH
    if (slot.initialized())
    {
      if (slot.read_ready())
      {
        char name[128];

        name[0] = 0;
        slot.read(name,sizeof(name));

        i4_str *new_name = i4gets("name_format").sprintf(128,name);

        m1_info.set_current_filename(*new_name);
        update_fly(*new_name, i4_T);

        BringWindowToTop(i4_win32_window_handle);


        delete new_name;
      }
    }
#endif
  }
  /*
  void calc_model()
  {
    r1_texture_manager_class *tman=api->get_tmanager();
    //tman->do_collection();

    m1_load_next();

#ifdef _WINDOWS
    Sleep(0);
#endif

    i4_application_class::calc_model();

    if (!m1_info.textures_loaded)
    {
      api->get_tmanager()->load_textures();


      i4_image_class *im=i4_load_image("bitmaps/golg_font_18.tga");
      if (m1_info.r_font)
        delete m1_info.r_font;
      m1_info.r_font=new r1_font_class(api, im);
      delete im;


      m1_info.textures_loaded=i4_T;
    }
     
  }
  */
  /*
  virtual void run(int argc, i4_const_str *argv)
  {
    i4_current_app=this;
    init();
    finished=i4_F; 

    if (!m1_info.update_mode)  // bypassing graphics?
    {
      if (m1_info.models.size())
        update_fly(*m1_info.models[0], i4_T);

      do
      {
        calc_model();
        refresh();      

      
        //next frame thing      
        api->get_tmanager()->next_frame();

        do 
        {
          m1_load_next();
          get_input();
        }
        while (display->display_busy() && !finished);

      } while (!finished);
    }

    m1_info.uninit();
    uninit();
  }
  
  void uninit()
  {
    if (!m1_info.update_mode)  // bypassing graphics?
    {
      i4_uninit_gui_status();

      i4_kernel.unrequest_events(this, i4_device_class::FLAG_DISPLAY_CHANGE);
      kill_obj();

      r1_destroy_api(api);
      i4_application_class::uninit();
    }
    else
      i4_uninit();


  }*/
  virtual i4_bool is_active(void)
	  {
	  return maxtool_mode;
	  }
  
} g1_maxtool_instance;

li_object *m1_show_maxtool(li_object *o, li_environment *env)
{
  li_call("Hide_Main_Menu");//delete the menu behind.
  g1_maxtool_instance.set_edit_mode(!g1_maxtool_instance.is_active());
  return 0;
}

li_automatic_add_function(m1_show_maxtool,"Maxtool/Toggle Menu");
/*
m1_utility_app_class *m1_app;

li_object *m1_quit(li_object *o, li_environment *env)
{
  if (m1_app)
    m1_app->quit();
  return 0;
}
li_automatic_add_function(m1_quit, "quit");

void i4_main(w32 argc, i4_const_str *argv)
{
  //if (!strcmp(opt,"-no_full"))
  //  fullscreen=i4_F;
  i4_win32_startup_options.check_option("-no_full");
  m1_utility_app_class m1_app;
  m1_app.run(argc, argv);
}
*/