/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "lisp/lisp.h"
#include "file/file.h"
#include "memory/array.h"
#include "tile.h"
#include "load3d.h"
#include "g1_render.h"
#include "render/r1_api.h"
#include "render/tmanage.h"
#include "map_man.h"
#include "map.h"
#include "cwin_man.h"
#include "make_tlist.h"
#include "saver.h"
#include "border_frame.h"
#include "file/sub_section.h"
#include <stdlib.h>

li_object *g1_add_textures(li_object *o, li_environment *env)
{
  for (o=li_cdr(o,0); o; o=li_cdr(o,0))
    g1_tile_man.add(li_car(o,0), env);

  return 0;
}


static li_object *g1_ignore(li_object *o, li_environment *env) {  return 0;}

i4_str *g1_get_res_filnename(const i4_const_str &map_filename)
{
  i4_filename_struct fn;
  i4_split_path(map_filename, fn);

  char res_name[256];

  int len=strlen(fn.filename);
  while (len && (fn.filename[len-1]>='0' && fn.filename[len-1]<='9'))
    len--;
  res_name[len]=0;

  if (fn.path[0])
    sprintf(res_name, "%s/%s.scm", fn.path, fn.filename);
  else
    sprintf(res_name, "%s.scm", fn.filename);

  return new i4_str(res_name);
}

w32 g1_load_res_info(i4_file_class *fp, w32 exclude_flags)
{
  li_environment *local_env=new li_environment(0,i4_T);
  //li_environment *local_env=0;//I don't think loading these not globaly is a good idea 
  //is ok, applies only to functions
  w32 include_flags=~exclude_flags;
  i4_array<i4_str *> tlist(512,512), mlist(512,512);

//  li_list *scheme_list;
  int t_tiles;
  r1_texture_manager_class *tman=g1_render.r_api->get_tmanager();
  
  if (include_flags & G1_MAP_TEXTURES)
	  {
	  tman->reset();
	  };

  i4_file_class *fp_list[1];
  fp_list[0]=fp;

  // get a list of models and textures we need to load for this level
  // with the last parameter set to 0, won't actually touch any files.
  // Will postphone that for later.
  g1_get_load_info(fp_list,1,  tlist,  mlist, t_tiles, 0);

  if (g1_strategy_screen.get())
    g1_strategy_screen->create_build_buttons();

  if (include_flags & G1_MAP_TEXTURES)
	  {
	  g1_tile_man.reset(t_tiles);
	  //for (int k=0;k<t_tiles;k++)
	//	  {
	//	  g1_tile_man.add(new li_string(*tlist[k]),0);
	//	  }
	  }

  if (include_flags & G1_MAP_MODELS)
	  {
	  g1_model_list_man.reset( mlist, tman);
	  }

  int i;
  for (i=0; i<tlist.size(); i++)
    delete   tlist[i];

  for (i=0; i<mlist.size(); i++)
    delete mlist[i];

  li_add_function("models", g1_ignore, local_env);
  li_add_function("textures", g1_add_textures, local_env);

  li_load("scheme/models.scm", local_env);
  fp->seek(0);
  li_load(fp, local_env);


  if (include_flags & G1_MAP_TEXTURES)
	  {
	  g1_tile_man.finished_load();
	  tman->load_textures();
      g1_render.install_font();
	  }

  tman->keep_resident("lod_vehicles1", 512);
  tman->keep_resident("target_cursor",64);
  tman->keep_resident("compass",64);
  return 0;
}

i4_bool g1_load_level(const i4_const_str &filename, int reload_textures_and_models,
                      w32 exclude_flags)
{
//  int i;
  i4_file_class *fp=i4_open(filename);

  if (!fp) 
	  {
	  if (g1_map_is_loaded())
		g1_destroy_map();
	  return 0;
	  }

  g1_loader_class *load=g1_open_save_file(fp);
  if (!load)
	  {
	  if (g1_map_is_loaded())
		g1_destroy_map();
      return 0;
	  }

  if (reload_textures_and_models)
  {
    if (g1_map_is_loaded())
      g1_destroy_map();

    w32 off,size;
    i4_file_class *res_file=0;
    if (load->get_section_info("resources", off, size))
      res_file=new i4_sub_section_file(i4_open(filename),off,size);

    if (!res_file)
    {
      i4_str *res_name=g1_get_res_filnename(filename);
      res_file=i4_open(*res_name);
      delete res_name;
    }

    if (res_file)
    {
	  //li_call("map_height_restore");//restore the height scalling factors 
	  //li_set_value("world_scaling",new li_float(1.0f),0);
	  li_load("scheme/map_init.scm");
      //of the map to the default
      g1_load_res_info(res_file, exclude_flags);
      delete res_file;
    }
	else
		{
		i4_error("WARNING: Cannot find scm file for current level. Strange things may happen.");
		}
  }
   
  g1_initialize_loaded_objects();

  g1_map_class *map;
  if (g1_map_is_loaded())
    map=g1_get_map();
  else
    map=new g1_map_class(filename);


  w32 ret=map->load(load, G1_MAP_ALL & (~exclude_flags));
  delete load;

  if ((ret & (G1_MAP_CELLS | G1_MAP_VERTS))==0)
  {
    delete map;
    return i4_F;
  }

  g1_set_map(map);

  //(OLI) hack for jc to only calculate LOD textures once
  if (reload_textures_and_models)
    map->init_lod();

  g1_cwin_man->map_changed();
  //delete fp;
  li_call("ForcePause");
  if (!g1_cwin_man->is_edit_mode())
	  {
	  li_call("Pause");//unpauses the game if not in editor.
	  }
  return i4_T;
}




