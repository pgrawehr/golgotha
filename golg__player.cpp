/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "player.h"
#include "team_api.h"
#include "error/error.h"
#include "g1_limits.h"
#include "image/color.h"
#include "map.h"
#include "g1_object.h"
#include "objs/map_piece.h"
#include "objs/stank.h"
#include "palette/pal.h"
#include "saver.h"
#include "border_frame.h"
#include "map_man.h"
#include "dll/dll_man.h"
#include "time/profile.h"
#include "lisp/li_class.h"
#include "lisp/li_load.h"
#include "li_objref.h"
#include "lisp/li_init.h"
#include "objs/path_object.h"
#include "lisp/li_dialog.h"
#include "controller.h"
#include "g1_tint.h"
#include "human.h"

enum 
{
  DATA_VERSION1=1,
  DATA_VERSION2,
  DATA_VERSION3,
  DATA_VERSION                  // added ai saving
};


g1_player_manager_class g1_player_man;

static li_symbol_ref li_upgrade_kill_ratio("upgrade_kill_ratio");
static li_int_class_member li_money("money"), li_income_rate("income_rate"),
  li_num_points("num_points"), li_num_stank_deaths("num_stank_deaths"),
  li_num_stank_lives("num_stank_lives");


static li_float_class_member li_damage_multiplier("damage_multiplier");
static li_g1_ref_class_member current_taveover_target("current_taveover_target");

static li_string_class_member li_team_flag("team_flag");
static li_symbol_ref stank("stank");

int &g1_player_info_class::num_points() { return get_vars()->get(li_num_points); }
int &g1_player_info_class::num_stank_deaths() { return get_vars()->get(li_num_stank_deaths); }
int &g1_player_info_class::num_stank_lives() { return get_vars()->get(li_num_stank_lives); }


int &g1_player_info_class::money() { return get_vars()->get(li_money); }
int &g1_player_info_class::income_rate() { return get_vars()->get(li_income_rate); }
float &g1_player_info_class::damage_multiplier() { return get_vars()->get(li_damage_multiplier);}


void g1_player_manager_class::show_message(const i4_const_str &message, w32 color, int player_for)
{
  if (player_for==local_player && g1_current_controller.get())
    g1_current_controller->scroll_message(message, color);
}

g1_player_info_class::g1_player_info_class() 
  : ai(0),
    owned_objects(0,32)
{
  refresh_money=1;
}

void g1_player_info_class::remove_object(w32 global_id)
{
  if (ai) ai->object_lost(global_id);
  
  g1_object_class *o=g1_global_id.checked_get(global_id);

  if (o==commander.get())
    set_commander(0);
  
  int t=owned_objects.size();
  for (int i=0; i<t; i++)
    if (owned_objects[i] == global_id) 
    {
      owned_objects[i]=g1_global_id.invalid_id();
      return ;
    }

  i4_warning("Object doesn't belong to current player");
}

void g1_player_info_class::add_object(w32 global_id)
{
  //if (ai)
  //  ai->object_added(global_id);

  g1_object_class *o=g1_global_id.get(global_id);
  if (!commander.get() && o->id==g1_get_object_type(stank.get()))
    set_commander(g1_player_piece_class::cast(o));
  
  int t=owned_objects.size();
  for (int i=0; i<t; i++)
    if (!g1_global_id.check_id(owned_objects[i]))
    {
      owned_objects[i]=global_id;
      return;
    }

  owned_objects.add(global_id);
}

float g1_player_info_class::kill_ratio()
{
  int d=num_stank_deaths();
  if (!d) d=1;
  return num_points() / (float)d;
}

void g1_player_info_class::calc_upgrade_level()
{
  supertank_upgrade_level=0;
        
  // run through the list "upgrage_time" in player_vars which has times to
  // upgrade

  int points=i4_f_to_i(kill_ratio());
    
  li_object *l = li_get_value(li_upgrade_kill_ratio.get());
    
  while (l)
  {
    int required_number = li_int::get(li_car(l,0),0)->value();
      
    if (required_number<=points)   // do we have enough kills for this level
      supertank_upgrade_level++;
      
    l = li_cdr(l,0); // next item in list
  }
    
  if (supertank_upgrade_level<0)
    supertank_upgrade_level=0;
}


    
static w32 convert_color_to_player(w32 player_number, i4_color color,
                                   const i4_pixel_format *fmt)
{
  sw32 r,g,b;
  r=(color>>16)&0xff;
  g=(color>>8)&0xff;
  b=(color>>0)&0xff;

  r=i4_f_to_i(g1_player_tint_data[player_number].r * r);
  if (r>255) r=255;

  g=i4_f_to_i(g1_player_tint_data[player_number].g * g);
  if (g>255) g=255;

  b=i4_f_to_i(g1_player_tint_data[player_number].b * b);
  if (b>255) b=255;


  return (r<<16)|(g<<8)|b;
}


g1_player_manager_class::g1_player_manager_class()
{
}

void g1_player_info_class::init_colors(w32 player_num, const i4_pixel_format *fmt)
{
  map_player_building=convert_color_to_player(player_num, 0x404040, fmt);
  map_player_color=convert_color_to_player(player_num, 0xffffff, fmt);
  map_attacking_player_color=convert_color_to_player(player_num, 0xdcdcdc ,fmt);
  map_select_player_color=(200<<16) | (200<<8) | 0;
}

void g1_player_info_class::new_game(g1_player_type num)
{
  player_num = num;  
  supertank_upgrade_level = 0;
  if (num==1)
    team=G1_ALLY;
  else
    team=G1_ENEMY;
}

void g1_player_info_class::set_ai(g1_team_api_class *_ai)
{
  if (ai&&ai!=g1_human)//newer delete the human interface, it is singleton
	  {
      delete ai;
	  ai=0;
	  }
  if (_ai==0)//uninit in progess
	  {
	  if (ai!=g1_human)//still not delete human interface, might be in use
		delete ai;
	  ai=0;
	  return;
	  }

  ai = _ai;
  li_string_class_member aiv("ai");
  if (ai)
	  {
	  ai->player = this;
	  }
  if (!vars.get()) return;
  li_class_context c(li_class::get(vars.get(),0));
  if (ai)
	  {
	  i4_const_str *a=ai->ai_name();
	  char buf[100];
	  i4_os_string(*a,buf,100);
	  li_class::get(vars.get(),0)->set(aiv,new li_string(buf));
	  }
  else
	  li_class::get(vars.get(),0)->set(aiv,new li_string("human"));
}


i4_profile_class pf_player_info_class_ai_think("player_info:: ai->think()");

void g1_player_info_class::add_points(int points)
{
  num_points()+=points;
  calc_upgrade_level();
}

void g1_player_info_class::supertank_died(g1_object_class *guy_who_killed_him)
{
  num_stank_deaths()++;
  calc_upgrade_level();
}

void g1_player_info_class::think()
{
  li_class_context vars_context(li_class::get(vars.get(),0));

  pf_player_info_class_ai_think.start();
  if (ai)
  {
    ai->think();
    ai->post_think();
  }
  pf_player_info_class_ai_think.stop();

  li_money() += li_income_rate();

  if (li_money() > 99999)
    li_money() = 99999;
  else if (li_money() < 0)
    li_money() = 0;

  if (g1_border.get())
    g1_border->update();
}


i4_bool g1_player_info_class::load(g1_loader_class *fp)
{  
  w16 ver=0,data_size=0;
  i4_bool ret=i4_F;
  vars=0;
  //continue_wait=i4_F;

  if (fp)
    fp->get_version(ver, data_size);

  if (!ai) i4_error("INTERNAL: no ai in player_info load");
  ai->init();

  ret = i4_T;
  switch (ver)
  {
    case DATA_VERSION:
      vars=li_class::get(li_load_typed_object("player_vars", fp, fp->li_remap,0),0);
      ai->load(fp);
      break;
    case DATA_VERSION3:
      vars=li_class::get(li_load_typed_object("player_vars", fp, fp->li_remap,0),0);
      break;
    default:
      if (fp)
        fp->seek(fp->tell() + data_size);
      ret = i4_F;
      break;
  }
 
  if (!vars.get()) 
    vars=li_class::get(li_new("player_vars"), 0);

  li_class_context c(get_vars());
  team_flag.set_name(li_team_flag());
  calc_upgrade_level();

  return ret;
}

void g1_player_info_class::save(g1_saver_class *fp)
{
  fp->start_version(DATA_VERSION);    
  li_save_object(fp, vars.get(), 0);
  ai->save(fp);
  fp->end_version();
}

//(OLI) temporary hack to allow DLL reloading of AI
//{{{
extern void (*g1_reload_dll)(void);

void reload_ais()
{
  i4_dll_man.load(i4_const_str("ai_joe.dll"), i4_T);
}
//}}}

void g1_player_manager_class::set_default_ai(const char *name)
{
  default_ai=name;

  if (default_ai)
    for (w32 i=0; i<G1_MAX_PLAYERS; i++)
    {
      if (list[i] && !list[i]->get_ai())
        list[i]->set_ai(g1_create_ai(default_ai));
    }
}

void g1_player_manager_class::set_ai(w32 player_num, const char *name)
	{
	if (player_num<G1_MAX_PLAYERS)
		list[player_num]->set_ai(g1_create_ai(name));//deletes old one itself
	}

void g1_player_manager_class::init()
{
  for (w32 i=0; i<G1_MAX_PLAYERS; i++)
  {
    list[i] = new g1_player_info_class();
    list[i]->new_game((w8)i);
  }

  local_player=1;  //(OLI) this needs to be changed to the appropriate number

  list[local_player]->set_ai(g1_create_ai("human"));

  if (default_ai)
    for (w32 i=0; i<G1_MAX_PLAYERS; i++)
    {
      if (list[i] && !list[i]->get_ai())
        list[i]->set_ai(g1_create_ai(default_ai));
    }

  //(OLI)
  g1_reload_dll = reload_ais;
}

void g1_player_manager_class::uninit()
{

  for (w32 i=0; i<G1_MAX_PLAYERS; i++)
  {
    if (list[i])
    {
      list[i]->set_ai(0);
      delete list[i];
    }
    list[i]=0;
  }
  delete g1_human;//Finally delete also the human interface.
  g1_human=0;
}

void g1_player_manager_class::unload_ai(g1_team_api_definition_class *definer)
{
  for (int i=0; i<G1_MAX_PLAYERS; i++)
    if (g1_player_man.get(i) &&
        g1_player_man.get(i)->get_ai() &&
        g1_player_man.get(i)->get_ai()->definer() == definer)
      g1_player_man.get(i)->set_ai(0);
}

void g1_player_manager_class::init_colors(const i4_pixel_format *fmt,
                                          r1_render_api_class *api)
{ 
  g1_init_color_tints(api);
  for (w32 i=0; i<G1_MAX_PLAYERS; i++)
    list[i]->init_colors(i, fmt);
}

li_object *g1_set_default_ai(li_object *o, li_environment *env)
{
  g1_player_man.set_default_ai(li_string::get(li_car(o,env),env)->value());
  return 0;
}

li_object *g1_set_ai(li_object *o,li_environment *env)
	{
	g1_player_man.set_ai(li_int::get(li_car(o,env),env)->value(),
		li_string::get(li_second(o,env),env)->value());
	return 0;
	}

li_automatic_add_function(g1_set_ai,"set_ai");
li_automatic_add_function(g1_set_default_ai, "set_default_ai");



