#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
//#include "maxtool/sdk_inc/max.h"
#include "render/r1_api.h"
#include "lisp/li_init.h"
#include "status/status.h"
#include "maxtool/m1_info.h"
#include "memory/array.h"
#include "render/r1_res.h"
#include "maxtool/tupdate.h"
#include "error/alert.h"
#include "file/file.h"

w32 m1_get_file_id(const i4_const_str &fname)
{
  int x;
  char st[30], *s;
  s=st;
  
  i4_const_str::iterator l=fname.begin();
  while (l!=fname.end() && l.get().ascii_value()!='.')
  {
    *(s++)=l.get().ascii_value();
    ++l;
  }

  *s=0;

  if (sscanf(st,"%x",&x))
    return x;
  else return 0;   
}


struct m1_texture
{
  w32 id;
  w32 last_modified;
};

int m1_find_texture(i4_array<m1_texture> &t, int id)
{
  for (int i=0; i<t.size(); i++)
    if (t[i].id==(w32)id)
      return i;
  return -1;
}

int m1_texture_compare(const m1_texture *a, const m1_texture *b)
{
  if (a->id < b->id)
    return -1;
  else
  if (a->id > b->id)
    return 1;
  else
    return 0;
}

inline char *remove_paths(char *src)
{
  char *ret = src;

  while (*src)
  {
    if (*src=='/' || *src=='\\')
      ret=src+1;

    src++;    
  }

  return ret;
}
w32 r1_get_file_id(const i4_const_str &fname);
void m1_copy_update(i4_bool all)
{
  w32 i;
  int t=0;
  i4_status_class *stat=i4_create_status(i4gets("updating_textures"));  

  i4_array<i4_str *> tlist(64,64);
  
  i4_directory_struct ds;
  i4_get_directory(i4gets("default_tga_dir"), ds, i4_T);
  
  i4_array <m1_texture> network_tga_list(128,128);  
  //if (!all)
//	  {
  m1_info.get_texture_list(tlist,all);
  t=tlist.size();  
//	  }
//  else 
//	  {
//	  t=ds.tfiles;
//	  }
  for (i=0; i<ds.tfiles; i++)
  {
    if (stat)
      stat->update(i/(float)ds.tfiles);

    m1_texture new_entry;    
    
    new_entry.id            = r1_get_texture_id(*ds.files[i]);
    new_entry.last_modified = ds.file_status[i].last_modified;

    network_tga_list.add(new_entry);
  }

  if (stat)
    delete stat;

  stat = i4_create_status(i4gets("updating_textures"), i4_T);  

  i4_directory_struct ds2;
  i4_get_directory(r1_get_compressed_dir(), ds2, i4_T);
  //i4_get_directory("textures",ds2,i4_T); //ds is "textures"
//  if (ds2.tfiles>t) t=ds.tfiles;//egal
  i4_array <m1_texture> local_gtx_list(128,128);
  
  m1_texture temp_tex;


  for (i=0; i<ds2.tfiles; i++)
  {    
    if (stat && !stat->update(i/(float)ds2.tfiles))
    {
      if (stat)
        delete stat;
      return;
    }

    temp_tex.id            = r1_get_file_id(*ds2.files[i]);//get the number
    temp_tex.last_modified = ds2.file_status[i].last_modified;
    local_gtx_list.add(temp_tex);
  }

  if (stat)
    delete stat;
  stat = i4_create_status(i4gets("updating_textures"), i4_T);

  network_tga_list.sort(m1_texture_compare);
  local_gtx_list.sort(m1_texture_compare);    
  
  for (i=0; i<t; i++)
  {    
    if (stat && !stat->update(i/(float)t))
    {
      if (stat)
        delete stat;
	  for (int ii=i;ii<t;ii++)
		  delete tlist[ii];
      return;
    }

    if (tlist[i]->null()) continue;
	i4_const_str tlistname("Texture_0000");
	if (*(tlist[i])==tlistname) 
		{
		i4_warning("Skipped updating of anonymous texture %s", tlist[i]);
		delete tlist[i];
		continue; //continue if texture is not set (default texture name after conversion)
		}

    temp_tex.id = r1_get_texture_id(*tlist[i]);    

    w32 network_index = network_tga_list.binary_search(&temp_tex,m1_texture_compare);
    if (network_index != -1)
    {     
      w32 local_index = local_gtx_list.binary_search(&temp_tex,m1_texture_compare);

      if (local_index==-1 || 
          (local_gtx_list[local_index].last_modified<network_tga_list[network_index].last_modified)
         )
      {
        //m1_info.texture_list_changed();
        
        m1_update_texture(*tlist[i], i4_T,
                          stat, 
                          i/(float)t, (i+1)/(float)t);
      }
    }
    else
    {
      char s1[256];
      char s2[256];
      char tga_dir[256];

      i4_os_string(*tlist[i],s1,256);
      i4_os_string(i4gets("default_tga_dir"),tga_dir,256);

      char *filename = remove_paths(s1);

      sprintf(s2,"Texture Missing: %s\\%s",tga_dir,filename);

      i4_str *n = i4_from_ascii(s2);
      i4_alert(*n,256);
      delete n;
    }
       
    delete tlist[i];        
  }
  m1_info.texture_list_changed();    
  if (stat)
    delete stat;
}

li_object *m1_update_all_textures(li_object *o, li_environment *env)
{
  m1_copy_update(i4_T);
  return 0;
}
li_automatic_add_function(m1_update_all_textures, "update_all_textures");

li_object *m1_update_textures(li_object *o, li_environment *env)
{
  m1_info.texture_list_changed();
  return 0;
}
li_automatic_add_function(m1_update_textures, "update_textures");
