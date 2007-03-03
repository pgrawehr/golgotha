/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "controller.h"
#include "objs/stank.h"
#include "map.h"
#include "window/win_evt.h"
#include "time/profile.h"
#include "menu/menu.h"
#include "menu/textitem.h"
#include "window/style.h"
#include "time/time.h"
#include "math/pi.h"
#include "resources.h"
#include "input.h"
#include "math/angle.h"
#include "mess_id.h"
#include "sound_man.h"
#include "math/random.h"
#include "g1_speed.h"
#include "remove_man.h"
#include "image/color.h"
#include "light.h"
#include "statistics.h"
#include "g1_render.h"
#include "render/r1_api.h"
#include "player.h"
#include "m_flow.h"
#include "math/trig.h"
#include "time/profile.h"
#include "app/app.h"
#include "loaders/load.h"
#include "g1_texture_id.h"
#include "render/tmanage.h"
#include "saver.h"
#include "human.h"
#include "objs/explode_model.h"
#include "objs/def_object.h"
#include "objs/miscobjs.h"
#include "tick_count.h"
#include "render/r1_clip.h"
#include "status/status.h"
#include "lisp/lisp.h"
#include "lisp/li_dialog.h"
#include "lisp/li_types.h"
#include "lisp/li_init.h"
#include "device/key_man.h"
#include "window/window.h"
#include "window/wmanager.h"
#include <stdlib.h>

enum {
  G1_CONTROLLER_WIREFRAME=1491,//should be unique
  G1_CONTROLLER_TEXTUREMAP,
  G1_CONTROLLER_WHITE_LIGHT,
  G1_CONTROLLER_320X200,
  G1_CONTROLLER_400X400,
  G1_CONTROLLER_640X480,
  G1_CONTROLLER_SCROLL
};

static li_symbol_ref li_mouse_left_button("mouse_left_button"),
                     li_mouse_right_button("mouse_right_button");
  
i4_event_handler_reference_class<g1_object_controller_class> g1_current_controller;

i4_profile_class pf_draw_overhead("controller::draw_overhead"),
  pf_test_clip("controller::test_clip");

extern sw32 g1_cur_num_map_objects;

//  View Frustrum Points
enum { NUM_VIEW_POINTS=5 };
static i4_3d_vector view_point[NUM_VIEW_POINTS];

class view_point_init_class : public i4_init_class
{
public:
	i4_float val;
  void init()
  {
	  val=200;
    view_point[0].set(   0,   0,   0 );
    view_point[1].set( 200, 200, 200 );
    view_point[2].set(-200, 200, 200 );
    view_point[3].set(-200,-200, 200 );
    view_point[4].set( 200,-200, 200 );
  }
  void set(i4_float x)
  {
	  val=x;
	  view_point[0].set(0,0,0);
	  view_point[1].set(x,x,x);
	  view_point[2].set(-x,x,x);
	  view_point[3].set(-x,-x,x);
	  view_point[4].set(x,-x,x);
  }
  void set_to_val()
  {
	  set(val);
  }
};
static view_point_init_class view_point_init;


void g1_controller_global_id_reset_notifier::reset()
{
  for_who->reset_global_ids();
}

void g1_object_controller_class::reset_global_ids()
{
  selectable_list.t_recent=0;
}

i4_bool g1_object_controller_class::add_spin_event(char *model_name, w8 lost)
{
  spin_event e(g1_model_list_man.find_handle(model_name), lost);
  return spin_events.que(e);
}

g1_player_piece_class *g1_object_controller_class::get_track()
{
  return g1_player_man.get(team_on())->get_commander();
}


g1_player_type g1_object_controller_class::team_on() const
{ 
  return (g1_human)? g1_human->team() : g1_default_player;
}


void g1_object_controller_class::get_pos(i4_3d_vector& pos) 
{ 
  view.get_camera_pos(pos); 
}

g1_object_controller_class::g1_object_controller_class(w16 w, w16 h, 
                                                       i4_graphical_style_class *style)
  : i4_parent_window_class(w,h),
    style(style),
    notifier(0)
{
  scroll_active=i4_F;
  for (int i=0; i<MAX_MESSAGES; i++)
    messages[i].text[0]=0;

  notifier.for_who=this;
  first_frame=i4_T;

  cursor_state=255;
  flags=NEED_CLEAR;

  context_menu=0;

  last_frame_draw_time.get();
  last_mouse_x=last_mouse_y=0;
  frames_to_count=8;
  frame_rate=0;

  mouse_mask=0;
  follow=0;
  stat_inc=0;
  stat_time=0;
  stat_type=0;
  radar_view=0;
}

g1_object_controller_class::~g1_object_controller_class()
{
  stop_scroll();
}


void g1_object_controller_class::setup_context(i4_draw_context_class &i4_context)
{
  g1_context.window_setup(0,0,width(), height(),
                          g1_render.center_x, g1_render.center_y,
                          g1_render.scale_x, g1_render.scale_y,
                          g1_render.ooscale_x, g1_render.ooscale_y); 

  g1_context.transform=&transform;
  g1_context.context=&i4_context;
  g1_context.screen=local_image;
  g1_context.style=style;

  g1_render.calculate_frame_ratio();

  selectable_list.t_recent=0;
  g1_render.current_selectable_list=&selectable_list;

}

#undef MAX
#define MAX(x,y) (x)>(y)?(x):(y)

extern w32 g1_disable_all_drawing;

extern i4_3d_point_class listentemp;

i4_profile_class pf_controller_draw("controller::parent_draw");

void g1_object_controller_class::draw_counts()
{
//#if 0      // not working anymore - fixme:jc
  //i4_float z=r1_near_clip_z*1.1f;
  int xoff,yoff,x=0;
  int icon_sx = g1_resources.stat_pad_size_x/2;
  int icon_sy = g1_resources.stat_pad_size_x/2;
  
  xoff = g1_resources.stat_pad_start_x;
  yoff = g1_resources.stat_pad_start_y;

  if (yoff>0) yoff += 5;

  if (g1_resources.render_window_expand_mode==R1_COPY_2x2)
  {
    xoff = xoff*3/4;
    yoff = yoff/2;
  }
  if (xoff<0) xoff+= width();
  if (yoff<0) yoff+= height();

  g1_render.r_api->set_shading_mode(R1_COLORED_SHADING);
  g1_render.r_api->set_write_mode(R1_WRITE_COLOR);



  if (current_spin.model==0)
    if (spin_events.deque(current_spin))
    {
      last_stat_time.get();
      stat_x = (i4_float(x) - g1_render.center_x)/(g1_render.center_x*g1_render.scale_x);
      stat_y = (i4_float(yoff) - g1_render.center_y)/(g1_render.center_y*g1_render.scale_y);
      if (current_spin.type==spin_event::GAINED)
        stat_time = 4000;
      else
        stat_time = 1000;
    }


  if (current_spin.model)
  {
    i4_time_class now;
    stat_time -= now.milli_diff(last_stat_time);
    last_stat_time.get();

    if (stat_time<=0)    // we are done with this spin
      current_spin.model=0;
    else
    {
      g1_quad_object_class *model = g1_model_list_man.get_model(current_spin.model);

      if (model)
      {
        i4_3d_vector pos(0,0,0);
        i4_float w=1.0;

        if (current_spin.type==spin_event::GAINED)
        {
          if      (stat_time>3000) 
			  { w = 1/(1.0f + i4_float(stat_time-3000.0f)*0.02f); }
          //else
			//  if (stat_time>1000)  { }  // sit for a while
          //else                   
          //{
          //  i4_float frac = 1.0-i4_float(stat_time)/500.0;
          //  pos.x = i4_interpolate(pos.x,stat_x,frac);
          //  pos.y = i4_interpolate(pos.y,stat_y,frac);
          //  w = (0.2 - 1.0)*frac + 1.0;
          //}
        }
        else
        {
		  //original code, didn't work anymore
          //i4_float frac = 1.0-i4_float(stat_time)/500.0;
          //pos.x = i4_interpolate(stat_x,-1.0,frac);
          //pos.y = stat_y;
          //w = i4_interpolate(0.2, 1.0, frac);
		  i4_float frac=1.0f- i4_float(stat_time)/500.0f;
		  pos.x=i4_interpolate(stat_x,-1.0f,frac);
		  pos.y=i4_interpolate(stat_y,-1.0f,frac);
		  w=(0.2f - 1.0f)*frac + 1.0f;
        }
        pos.z = 0.3f;

        i4_transform_class scale, rot, transform;

        transform.translate(pos);
        transform.mult_rotate_y(stat_time*i4_2pi()/1000.0f);
        transform.mult_rotate_x(i4_pi_2());
        transform.mult_uniscale(w);

        i4_float save_ambient = g1_lights.ambient_intensity;

        g1_lights.ambient_intensity = 1.0f;
        g1_render.render_object(model, &transform, 0, w);
        g1_lights.ambient_intensity = save_ambient;
      }
    }

  }
//#endif
}

int last_draw_tick=0;
int frame_locked_mode=0;

void g1_object_controller_class::editor_pre_draw(i4_draw_context_class &context)
	{
    if (view.get_view_mode()==G1_STRATEGY_MODE)
		{
		g1_render.r_api->set_write_mode(R1_WRITE_COLOR | R1_WRITE_W);
		r1_clip_clear_area(0,0,width()-1, height()-1, 0, r1_far_clip_z, context, g1_render.r_api);
		//g1_render.r_api->set_write_mode(R1_WRITE_COLOR | R1_WRITE_W | R1_WRITE_COLOR);
		}
	}

void g1_object_controller_class::editor_post_draw(i4_draw_context_class &context)
	{
	//if (view.get_view_mode()==G1_ACTION_MODE)
	//	{
	//	g1_render.render_sprite(here,texhandle);
	//	}
	}

//returns true if rendering is enabled after call
li_object *li_toggle_rendering(li_object *o, li_environment *env)
	{
	g1_disable_all_drawing= (g1_disable_all_drawing+1) % 3;
	if (g1_disable_all_drawing)
		return li_nil;
	return li_true_sym;
	}

li_automatic_add_function(li_toggle_rendering,"toggle_rendering");

void g1_object_controller_class::parent_draw(i4_draw_context_class &context)
{
  if (frame_locked_mode)
  {
    if( (w32)last_draw_tick != g1_tick_counter )
      last_draw_tick=g1_tick_counter;
    else
      return ;
  }
  
  if (!g1_render.main_draw)
  {
    if (clear_if_cannot_draw())
    {
      g1_render.r_api->default_state();
      g1_render.r_api->clear_area(0,0, width()-1, height()-1, 0, r1_far_clip_z);
    }

    request_redraw(i4_F);
    return ;
  }

  if (!g1_map_is_loaded())
  {
    local_image->clear(0,context);
    return ;
  }

  //i4_parent_window_class::parent_draw(context);
   redraw_flag=i4_T; //The controller is always dirty. (should be redrawed with every frame)
   request_redraw(i4_T);
  
  //update_cursor();

  if (g1_disable_all_drawing>0) 
	  return;  

  pf_controller_draw.start();

  setup_context(context);


  frames_to_count--;
  if (frames_to_count==0)
  {
    frames_to_count=FRAME_MEASURE_INTERVAL;
    i4_time_class now;
    frame_rate=1.0f/(float)(now.milli_diff(last_frame_draw_time)/(1000.0*FRAME_MEASURE_INTERVAL));
    last_frame_draw_time.get();
  }  
  

  i4_bool repeat;
  do
  {
    repeat=i4_F;

    g1_render.r_api->default_state();

	//g1_near_z_range and far_z_range are obsolete anyway
    //g1_render.r_api->set_z_range(g1_near_z_range(), g1_far_z_range()+0.01f);
	//g1_render.r_api->set_z_range(r1_near_clip_z,r1_far_clip_z+0.01f);

    if (g1_render.draw_mode==g1_render_class::WIREFRAME)
      g1_render.r_api->clear_area(0,0, width()-1, height()-1, 0, r1_far_clip_z);


    g1_player_piece_class *track=get_track();

    editor_pre_draw(context);


    g1_render.r_api->set_write_mode(R1_WRITE_COLOR | R1_WRITE_W | R1_COMPARE_W);
    g1_render.r_api->set_shading_mode(R1_COLORED_SHADING);
    g1_render.r_api->set_alpha_mode(R1_ALPHA_DISABLED);
    g1_render.r_api->set_color_tint(0);
  
    view.calc_transform(transform);
    setup_clip();
	//i4_transform_class xtmp;
	//xtmp.scale(view.get_camera()->scale_x,
	//	view.get_camera()->scale_y,view.get_camera()->scale_z);
	//transform.multiply(xtmp);
  
    //   //are we using a software rasterizer?
    //   if (view_mode==STRATEGY_MODE && !camera_is_zooming())
    //   {
    //     g1_render.r_api->modify_features(R1_PERSPECTIVE_CORRECT, 0);
    //     g1_render.r_api->modify_features(R1_SPANS, 1);
    //   }

    g1_render.r_api->modify_features(R1_PERSPECTIVE_CORRECT, 1);
    g1_render.r_api->modify_features(R1_SPANS, 1);

    draw_overhead(&g1_context);

    update_cursor();
    editor_post_draw(context);

    g1_render.post_draw_quads();


    sw32 cx1=context.xoff,
      cy1=context.yoff;
    sw32 cx2=cx1+width()-1,
      cy2=cy1+height()-1;

    if (view.get_view_mode()==G1_ACTION_MODE ||
        view.get_view_mode()==G1_STRATEGY_MODE ||
        view.get_view_mode()==G1_FOLLOW_MODE)     
      draw_counts();
	g1_render.flush_vert_buffer();
    g1_render.r_api->get_tmanager()->next_frame();
    g1_stat_counter.t_frames++;

    //g1_render.current_selectable_list=0;


    if (first_frame && 
        g1_render.r_api->get_tmanager()->texture_resolution_change())
    {
      g1_render.r_api->clear_area(0,0, width()-1, height()-1, 0, r1_far_clip_z);
      repeat=i4_T;
      
      for (i4_idle_class *p=i4_idle_class::first; p;p=p->next)
        p->idle();
    }

    // keep drawing till everything is rez-ed in
  } while (repeat);
  first_frame=i4_F;


  int tx=0, ty=0;
  for (int i=0; i<MAX_MESSAGES; i++)
  {    
    if (messages[i].text[0])
    {
      g1_render.rendered_font->set_color(messages[i].color);
      g1_render.rendered_font->put_string(local_image, tx,ty, messages[i].text, context);
      ty+=g1_render.rendered_font->height(messages[i].text);
    }
  }

  pf_controller_draw.stop();
}

void g1_object_controller_class::start_scroll()
{
  if (!scroll_active)
  {
    scroll_active=i4_T;
    //i4_event *ev=new i4_user_message_event_class(G1_CONTROLLER_SCROLL);
    //message_scroll_id=i4_time_dev.request_event(this,  ev, 6000);
	//delete ev;

	//request_event() is always copying the given event, so 
	//using the stack to create the event is faster.
	i4_user_message_event_class ev(G1_CONTROLLER_SCROLL);
	message_scroll_id=i4_time_dev.request_event(this,&ev,6000);
	
  }
}

void g1_object_controller_class::stop_scroll()
{
  if (scroll_active)
  {
    i4_time_dev.cancel_event(message_scroll_id);
    scroll_active=i4_F;
  }
}


void g1_object_controller_class::scroll_message(const i4_const_str &m, i4_color color)
{
  start_scroll();

  int i;
  for (i=0; i<MAX_MESSAGES; i++)
  {
    w32 c=messages[i].color;
    c=((((c>>16)&0xff)>>1)<<16)|
      ((((c>>8)&0xff)>>1)<<8)|
      ((((c>>0)&0xff)>>1)<<0);
    messages[i].color=c;  
    

    if (messages[i].text[0]==0)
    {
      i4_os_string(m, messages[i].text, sizeof(messages[i].text));
      messages[i].color=color;
	  request_redraw();
      return ;
    }
  }

  for (i=0; i>MAX_MESSAGES-1; i++)
    memcpy(&messages[i], &messages[i+1], sizeof(message));

  i4_os_string(m, messages[MAX_MESSAGES-1].text, sizeof(messages[0]));
  messages[MAX_MESSAGES-1].color=color;

  request_redraw();
}

void g1_object_controller_class::window_event(i4_window_message_class *wev)
{
  switch (wev->sub_type)
  {
    case i4_window_message_class::LOST_KEYBOARD_FOCUS :
      set_flag(KEY_FOCUS, i4_F);
      request_redraw();
      break;
  }
}

void g1_object_controller_class::do_command_event(i4_do_command_event_class *ev)
{
  if (!strcmp(ev->command, "Toggle Stats"))
    g1_stat_counter.show();  
  else if (!strcmp(ev->command, "Toggle Texture Loading"))
    g1_render.r_api->get_tmanager()->toggle_texture_loading();
  else if (!strcmp(ev->command, "Show_Help"))
  {
    i4_user_message_event_class u(G1_HELP_SCREEN);
    i4_kernel.send_event(i4_current_app, &u);
  }  
  else
    g1_input.receive_event(ev);
}

void g1_object_controller_class::end_command_event(i4_end_command_event_class *ev)
{
  g1_input.receive_event(ev);
}

g1_object_class *dummy_obj = NULL;

void g1_object_controller_class::change_mouse_mask(int new_mask)
{
  
  //i4_bool lock=i4_F;
  //if (new_mask & MOUSE_RIGHT) 
  //{
  //  if (mouse_mask & MOUSE_LEFT) // don't ack right if left is already down
  //   return ;

  //  lock=i4_T;
  //}

//   if (new_mask && !mouse_mask)
//   {
//     i4_window_request_mouse_grab_class grab(this, lock);
//     i4_kernel.send_event(parent, &grab);
//   }
//   else if (!new_mask && mouse_mask)
//   {
//     i4_window_request_mouse_ungrab_class ungrab(this);
//     i4_kernel.send_event(parent, &ungrab);
//   }

  mouse_mask=new_mask;
}


void g1_object_controller_class::reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
{
  change_mouse_mask(0);
  i4_parent_window_class::reparent(draw_area, parent);
}

void g1_object_controller_class::update_cursor()
{
  //last_mouse_x=last_mouse_x();
  //last_mouse_y=last_mouse_y();
  g1_object_class *o=find_object_under_mouse(last_mouse_x, last_mouse_y,0);
  w8 new_cursor=g1_human->determine_cursor(o);

  if (new_cursor!=cursor_state)
  {
    set_cursor(g1_resources.big_cursors[new_cursor]);
    cursor_state=new_cursor;
  }
}


void g1_object_controller_class::receive_event(i4_event *ev)
{
  i4_parent_window_class::receive_event(ev);

  switch (ev->type())
  {
    case i4_event::USER_MESSAGE :
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      if (uev->sub_type==G1_CONTROLLER_SCROLL)
      {
        for (int i=0; i<MAX_MESSAGES-1; i++)
        {    
          memcpy(&messages[i], &messages[i+1], sizeof(message));
          w32 c=messages[i].color;
          c=((((c>>16)&0xff)>>1)<<16)|
            ((((c>>8)&0xff)>>1)<<8)|
            ((((c>>0)&0xff)>>1)<<0);
          messages[i].color=c;  

        }

        messages[MAX_MESSAGES-1].text[0]=0;

        request_redraw();

        scroll_active=i4_F;
        if (messages[0].text[0])
          start_scroll(); 
      }

    } break;

    case i4_event::WINDOW_MESSAGE :
    {
      CAST_PTR(wev,i4_window_message_class,ev);
      window_event(wev);

    } break;

    case i4_event::MOUSE_BUTTON_UP :
    {
      CAST_PTR(mev,i4_mouse_button_up_event_class,ev);
	  if (mev->left())
		  mouse_mask &=~MOUSE_LEFT;
	  else if (mev->right())
		  mouse_mask &=~MOUSE_RIGHT;
	  if (view.get_view_mode()==G1_EDIT_MODE)
		  break;
	  if (mouse_mask==0)
		  {
		  i4_window_request_mouse_ungrab_class ungrab(this);
		  i4_kernel.send_event(parent,&ungrab);
		  }
	  g1_human->player_clicked(0,(float)mev->x,(float)mev->y,g1_human_class::END_DRAG);
      if (view.get_view_mode()==G1_ACTION_MODE)
      {
        if (mev->left())
        {
          char *s=li_string::get(li_get_value(li_mouse_left_button.get()),0)->value();
          i4_end_command_event_class cmd(s, i4_key_man.get_command_id(s), mev->time);
          g1_input.receive_event(&cmd);
        }
        else if (mev->right())
        {
          char *s=li_string::get(li_get_value(li_mouse_right_button.get()),0)->value();
          i4_end_command_event_class cmd(s, i4_key_man.get_command_id(s), mev->time);
          g1_input.receive_event(&cmd);
        }
      }

	  //suggested behaviour: 
	  //Left click on own object: select object(s)
	  //right click on own object: Open object window
	  //left click on enemy object: fire main gun (only if in command)
	  //right click on enemy object: if (something is selected) 
	  //   command troop to attack target or/and fire secondary weapon
	  //left click on terrain: fire and send unit there
	  //right click on terrain: command selected unit(s)
	  //left click on outside terrain: deselect unit(s)
	  if ((abs(begin_dragx - mev->x)<3) && (abs(begin_dragy - mev->y)<3) && !get_flag(DRAGGING))
		  {//was the user dragging?
		  //No.
		  g1_object_class *obj=find_object_under_mouse(mev->x,mev->y,mev->left()?MOUSE_LEFT:0);
	  if (mev->left())
		  {
		  
		  if (obj)
			  {
			  //if (obj->player_num==g1_human->team())
				g1_human->player_clicked(obj,obj->x,obj->y,g1_human_class::SELECT);
			  //else
			  //  g1_human->player_clicked(obj,obj->x,obj->y,g1_human_class::ATTACK);
			  }
		  else 
			  {
			  i4_float gotox,gotoy,dirx,diry;
			  if (!view_to_game(mev->x,mev->y,gotox,gotoy,dirx,diry)&&view.get_view_mode()!=G1_ACTION_MODE) 
				  {//in action mode, it is unavoidable to sometimes drag the rectangle over the sky.
				  g1_human->clear_selected();
				  break;
				  }
			  g1_human->player_clicked(0,gotox,gotoy,g1_human_class::SELECT_PATH);
			  }
		  }
	  if (mev->right())
		  {
			  i4_float gotox,gotoy,dirx,diry;
			  if (!view_to_game(mev->x,mev->y,gotox,gotoy,dirx,diry)) 
				  {
				  //g1_human->clear_selected();
				  break;
				  }
			  if (obj&&obj->player_num!=g1_human->team())
				  {
				  g1_human->player_clicked(obj,gotox,gotoy,g1_human_class::ATTACK);
				  break;
				  }
			  
			  g1_human->player_clicked(0,gotox,gotoy,g1_human_class::GOTO);
		  }
		  }
	  else
		  {//the user was dragging a selection rectangle
		  if (mev->left())
			  {
			  g1_human->player_clicked(0,(float) mev->x,(float) mev->y,g1_human_class::END_DRAG);
		      find_objects_in_rectangle(begin_dragx,begin_dragy,mev->x,mev->y,!get_flag(DRAGGING));
			  }
		  }
	  set_flag(DRAGGING,0);


    } break;

    case i4_event::MOUSE_BUTTON_DOWN :
    {
      CAST_PTR(mev,i4_mouse_button_down_event_class,ev);
	  last_mouse_x=mev->x;
	  last_mouse_y=mev->y;
	  begin_dragx=mev->x;
	  begin_dragy=mev->y;
	  if (mev->left())
		  mouse_mask |=MOUSE_LEFT;
	  else if (mev->right())
		  mouse_mask |=MOUSE_RIGHT;
	  if (view.get_view_mode()==G1_EDIT_MODE)
		  break;
	  i4_window_request_mouse_grab_class grab(this);
	  i4_kernel.send_event(parent,&grab);
      if (view.get_view_mode()==G1_ACTION_MODE)
      {
        if (mev->left())
        {
          char *s=li_string::get(li_get_value(li_mouse_left_button.get()),0)->value();
          i4_do_command_event_class cmd(s, i4_key_man.get_command_id(s), mev->time);
          g1_input.receive_event(&cmd);
        }
        else if (mev->right())
        {
          char *s=li_string::get(li_get_value(li_mouse_right_button.get()),0)->value();
          i4_do_command_event_class cmd(s, i4_key_man.get_command_id(s), mev->time);
          g1_input.receive_event(&cmd);
        }
      }
	  g1_human->player_clicked(0,(float)begin_dragx,(float)begin_dragy,g1_human_class::START_DRAG);
	  
    } break;

	case i4_event::MOUSE_MOVE:
		{
		CAST_PTR(mmev,i4_mouse_move_event_class,ev);
		if (mouse_mask==MOUSE_LEFT&&view.get_view_mode()!=G1_EDIT_MODE)
			{
			i4_bool cleartostart=!get_flag(DRAGGING);
			if ((abs(begin_dragx- mmev->x)>=3) && (abs(begin_dragy- mmev->y)>=3))
				{
				g1_human->player_clicked(0,(float)begin_dragx,(float)begin_dragy,g1_human_class::START_DRAG);
				set_flag(DRAGGING);
				}
			g1_human->player_clicked(0,(float)last_mouse_x,(float)last_mouse_y,g1_human_class::DRAGGING);
			find_objects_in_rectangle(begin_dragx,begin_dragy,mmev->x,mmev->y,cleartostart);
			}
		
		last_mouse_x=mmev->x; 
        last_mouse_y=mmev->y;
		}
		break;


    case i4_event::DO_COMMAND :
    {
      CAST_PTR(kev, i4_do_command_event_class, ev);
      do_command_event(kev);
    } break;

    case i4_event::END_COMMAND :
    {
      CAST_PTR(kev, i4_end_command_event_class, ev);
      end_command_event(kev);
    } break;

  }
  i4_window_class::receive_event(ev);
}

void g1_object_controller_class::delete_context_menu()
{
  if (context_menu)
  {
    delete context_menu;
    context_menu=0;
    note_undrawn(0,0,width()-1,height()-1);
    request_redraw();
  }
}



void g1_object_controller_class::resize(w16 new_width, w16 new_height)
{
  set_flag(NEED_CLEAR,i4_T);
  i4_parent_window_class::resize(new_width,new_height); 
}

  



void g1_object_controller_class::update_camera()
{
  if (view.get_view_mode()!=G1_EDIT_MODE)
  {
    view.update();
    request_redraw(i4_F);
  }
  else    // see if the camera moved
  {
    g1_camera_info_struct old_camera=*view.get_camera();
    view.update();

    if (memcmp(view.get_camera(), &old_camera, sizeof(g1_camera_info_struct))!=0)
      request_redraw(i4_F);
  }
}



i4_bool g1_object_controller_class::view_to_game(sw32 mouse_x, sw32 mouse_y,
                                                 i4_float &game_x, i4_float &game_y,
                                                 i4_float &dir_x, i4_float &dir_y)
{
  i4_3d_vector c_pos, c_dir, c_plane;


  i4_transform_class tran=transform;
  
  i4_float max_dim = width() > height() ? (i4_float)width() : (i4_float)height();
  max_dim/=2;

  i4_float mx=(float)(mouse_x - width()/2);
  i4_float my=(float)(mouse_y - height()/2);  

  tran.inverse_transform(i4_3d_vector(0,0,0),c_pos);
  tran.inverse_transform(i4_3d_vector(mx,my, max_dim),c_dir);  

  c_dir -= c_pos;  	

  i4_float t = -(c_pos.z/c_dir.z); 

  dir_x = c_dir.x;
  dir_y = c_dir.y;

  i4_float dist = (float)sqrt(dir_x*dir_x + dir_y*dir_y);
  dir_x /= dist;
  dir_y /= dist;

  if (t<0)
    return i4_F;

  //where the camera/mouse ray intersects the ground
  i4_float g_intersect_x, g_intersect_y;

  g_intersect_x = c_pos.x + c_dir.x * t;
  g_intersect_y = c_pos.y + c_dir.y * t;
  i4_float z    = c_pos.z + c_dir.z * t;
  if (z>0.1 || z<-0.1)
  {
    i4_float a=0;
    //    i4_warning("bad view_to_game calculation");
  }

  i4_float d_x = g_intersect_x - c_pos.x;
  i4_float d_y = g_intersect_y - c_pos.y;
  i4_float d_z = -c_pos.z;
  i4_float biggest = (float)fabs(d_x);
  if (fabs(d_y)>biggest) biggest = (float)fabs(d_y);
  
  //dont check for big z step. height only changes on
  //x and y steps

  biggest *= 4;
  
  d_x /= biggest;
  d_y /= biggest;
  d_z /= biggest;

  i4_float px=c_pos.x, py=c_pos.y, pz=c_pos.z;

  if (px>=get_map()->width() || py>=get_map()->height() || px<0 || py<0 ||
      get_map()->terrain_height(px,py)>pz)
  {
    game_x = px;
    game_y = py;
    return i4_T;
  }
  for (t=0;t<biggest;t+=1)
  {
    px += d_x;
    py += d_y;
    pz += d_z;
    if (px<get_map()->width() && py<get_map()->height() && px>=0 && py>=0)
    {
      if (get_map()->terrain_height(px,py)>pz)
      {
        d_x /= 4;
        d_y /= 4;
        d_z /= 4;
        while (get_map()->terrain_height(px,py)>pz)
        {
          px -= d_x;
          py -= d_y;
          pz -= d_z;        
        }
        game_x = px;
        game_y = py;
        return i4_T;
      }
    }
    else
    {
      game_x = px - d_x;
      game_y = py - d_y;
      return i4_T;
    }
  }
  game_x = px;
  game_y = py;

  return i4_T;
}

void g1_object_controller_class::find_objects_in_rectangle(w32 x1, w32 y1, w32 x2, w32 y2, i4_bool cleartostart)
	{
	g1_screen_box *a=selectable_list.recent, *current=0;
	w32 t;
	if (cleartostart&& !i4_current_app->get_window_manager()->shift_pressed())
		{
		g1_human->clear_selected();
		}
	if (x1>x2)//flip edges
		{
		t=x1;
		x1=x2;
		x2=t;
		}
	if (y1>y2)
		{
		t=y1;
		y1=y2;
		y2=t;
		}
	for (int i=0; i<(int)selectable_list.t_recent;i++,a++)
		{
		//i4_rect_list_class rc((short)b->x1,(short)b->y1,(short)b->x2,(short)b->y2);
		//that class is for window lists, but why not reuse?
		//because it is very complicated and slow.
		//rc.intersect_area((short)x1,(short)y1,(short)x2,(short)y2);
		//if (rc.empty())
		//	continue;
		if (!(x2<a->x1 || y2<a->y1 || x1>a->x2 || y1>a->y2))
			{
			if (g1_global_id.check_id(a->object_id))
				g1_human->player_clicked(g1_global_id.get(a->object_id),0,0,g1_human_class::ADD_TO_LIST);
			
			} 
		}
	
	
	}


g1_object_class *g1_object_controller_class::find_object_under_mouse(sw32 mx, sw32 my, w32 flags)
{
  g1_screen_box *b=selectable_list.recent, *closest=0;

  for (int i=0; i<(int)selectable_list.t_recent; i++, b++)
	{
	if (mx>=b->x1 && mx<=b->x2 && 
        my>=b->y1 && my<=b->y2)
		{
		if (!closest || closest->z1>b->z1)
			closest=b;
		}
	}//won't add an object without a global_id. So these are per se
  //not selectable!
  //exception: The buttons are a bit special
  if (closest && closest->flags&g1_screen_box::BUTTON 
	  && g1_global_id.check_id(closest->object_id))
	  {
	  if (flags!=g1_object_controller_class::MOUSE_LEFT)
		  return 0; //if just collision testing, the buttons don't count.
	  g1_object_class *obj=g1_global_id.get(closest->object_id);
	  char *cmdname=command_lookup_table.get(closest->commandnum)->commandname;
	  I4_ASSERT(cmdname, "INTERNAL: Unknown special command identifier found.");
	  obj->message(li_get_symbol("commands-exec"), 
		  li_get_symbol(cmdname), 
		  0);
	  return 0;
	  }

  if (closest && g1_global_id.check_id(closest->object_id))
	  {
	  g1_object_class *obj=g1_global_id.get(closest->object_id);
	  g1_render.draw_outline(closest,obj);
	  return obj;
	  }
  //else if (closest && closest->flags&g1_screen_box::BUTTON)
//	  {
	  //is a button-type object (i.e the user clicked "stop")
	  //problem: how do we know what button was clicked?
//	  }

  return 0;
}


void g1_object_controller_class::set_exclusive_mode(i4_bool yes_no)
{
  if (!parent)
    i4_error("call set exclusive when window is parented");

  i4_window_request_key_grab_class kgrab(this);
  i4_kernel.send_event(parent, &kgrab);

  if (!kgrab.return_result)
    i4_error("key grab failed");

  request_redraw();
}



void g1_object_controller_class::zoom(i4_float dist)
{
  view.zoom(dist);
  request_redraw(i4_F);
}

void g1_object_controller_class::pan(i4_float x, i4_float y, i4_float z)
{
  view.pan(x,y,z);
  request_redraw(i4_F);
}


void g1_object_controller_class::rotate(i4_float about_game_z, i4_float up_down)
{
  view.rotate(about_game_z, up_down);
  request_redraw(i4_F);
}

void g1_object_controller_class::scale(i4_float x, i4_float y, i4_float z)
	{
	view.scale(x,y,z);
	request_redraw(i4_F);
	}

static li_float_class_member cx("x"),cy("y"),cz("z"),scx("scalex"),
scy("scaley"),scz("scalez"),rotx("rotatex"),roty("rotatey"),rotz("rotatez");

//The next two methods are friend of g1_view_state_class, therefore they are allowed
//to set the members directly. 
li_object *g1_set_camera_params(li_object *o, li_environment *env)
{
  if (o==0)
	  return 0;
  li_class *c=li_class::get(li_car(o, env),env);
  li_class_context ctx(c);
  g1_current_controller->scale(scx(),scy(),scz());
  g1_current_controller->view.end.gx=cx();
  g1_current_controller->view.end.gy=cy();
  g1_current_controller->view.end.gz=cz();
  g1_current_controller->view.end.ground_rotate=rotx();
  g1_current_controller->view.end.horizon_rotate=roty();
  g1_current_controller->view.end.roll=rotz();
  g1_current_controller->view.start=g1_current_controller->view.end;
  g1_current_controller->request_redraw(i4_F);
  return 0;
}



li_object *g1_edit_camera_params(li_object *o, li_environment *env)
{
  li_object *cl=li_new("camera_params",o,env);
  li_class_context ctx(li_class::get(cl,env));
  scx()=g1_current_controller->view.current.scale_x;
  scy()=g1_current_controller->view.current.scale_y;
  scz()=g1_current_controller->view.current.scale_z;
  cx()=g1_current_controller->view.end.gx;
  cy()=g1_current_controller->view.end.gy;
  cz()=g1_current_controller->view.end.gz;
  rotx()=g1_current_controller->view.end.ground_rotate;
  roty()=g1_current_controller->view.end.horizon_rotate;
  rotz()=g1_current_controller->view.end.roll;
  g1_current_controller->view.start=g1_current_controller->view.end;
  li_create_dialog("Camera parameters", cl, 0, g1_set_camera_params);
  return 0;
};

//This zooms the view to a factor of 3.0 or back to normal (sensefull only in action mode). Hint: Changing the view state
//to something other than action mode resets the camera to normal. 
li_object *g1_toggle_glasses(li_object *o, li_environment *env)
{
	if (g1_current_controller->view.current.scale_y<1.1)
	{
		g1_current_controller->scale(3.0f,3.0,g1_current_controller->view.current.scale_z);
	}
	else
	{
		g1_current_controller->scale(1.0f,1.0f,1.0f);
	}
	g1_current_controller->view.start=g1_current_controller->view.end;
	g1_current_controller->request_redraw(i4_F);
	return li_true_sym;
}

li_automatic_add_function(g1_edit_camera_params, "edit_camera");
li_automatic_add_function(g1_toggle_glasses,"toggle_glasses");


void g1_object_controller_class::roll(i4_float r)
	{
	view.roll(r);
	request_redraw(i4_F);
	}

inline float fmin(float a, float b) { return (a<b)?a:b; }
inline float fmax(float a, float b) { return (a>b)?a:b; }

void g1_object_controller_class::setup_clip()
//  precalculate constants used to clip orthogonal bounding boxes from the view frustrum
{
  i4_3d_vector a,b;
  int i;
  // view_point_init.set_to_val();
  // transform
  for (i=0; i<NUM_VIEW_POINTS; i++)
    transform.inverse_transform(view_point[i], camera_point[i]);

  // Calculate bbox of view
  bbox.min = camera_point[0];
  bbox.max = camera_point[0];
  for (i=1; i<NUM_VIEW_POINTS; i++)
  {
    bbox.min.x = fmin(bbox.min.x, camera_point[i].x);
    bbox.min.y = fmin(bbox.min.y, camera_point[i].y);
    bbox.min.z = fmin(bbox.min.z, camera_point[i].z);
    bbox.max.x = fmax(bbox.max.x, camera_point[i].x);
    bbox.max.y = fmax(bbox.max.y, camera_point[i].y);
    bbox.max.z = fmax(bbox.max.z, camera_point[i].z);
  }

  // Calculate plane equations and determine N & P points for planes
  int j=4;
  for (i=0; i<4; i++)
  {
    a = camera_point[j];
    a -= camera_point[i];
      
    j = i+1;
    b = camera_point[j];
    b -= camera_point[i];
      
    plane[i].normal.cross(a,b);
	//? perhaps use the 5th point here? 
    plane[i].D = -plane[i].normal.dot(camera_point[4]);
    plane[i].calc_np();
  }
}

int g1_object_controller_class::test_clip(const i4_3d_vector& min, const i4_3d_vector& max)
// test bounding region of lod_node against the view frustrum
{
//  pf_test_clip.start();

  int i;
  int ret=-1;
  
  if (max.x<bbox.min.x || min.x>bbox.max.x || 
      max.y<bbox.min.y || min.y>bbox.max.y ||
      max.z<bbox.min.z || min.z>bbox.max.z)
    // bounding box check
    ret = -1;
  else
  {
    ret = 1;
    i4_3d_vector corner[8];
    
    // create indexed array for bounding box corners
    corner[0].set(min.x,min.y,min.z);
    corner[1].set(max.x,min.y,min.z);
    corner[2].set(min.x,max.y,min.z);
    corner[3].set(max.x,max.y,min.z);
    corner[4].set(min.x,min.y,max.z);
    corner[5].set(max.x,min.y,max.z);
    corner[6].set(min.x,max.y,max.z);
    corner[7].set(max.x,max.y,max.z);
    
    // check the bounding box against the 4 view frustrum planes
    for (i=0; i<4; i++)
    {
      if (plane[i].evaluate(corner[plane[i].p]) < 0)
      {
        // completely outside this plane
        ret = -1;
		//  ret=0;
        break;
      }

      if (plane[i].evaluate(corner[plane[i].n]) < 0)
        // not completely inside this plane
        ret = 0;
    }
  }

//  pf_test_clip.stop();
  return ret;
}

