/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include <stdio.h>
#include "render/r1_res.h"
#include "string/string.h"
#include "file/file.h"
#include "app/registry.h"
#include "main/win_main.h"

int r1_max_texture_size=256;

static char compressed[60], decompressed[60], cache_file[MAX_PATH];
static i4_const_str /*comp(""), */decomp(""), cache("");


static void get(char *var, char *buffer, int max_buffer, char *def)
{
  if (i4_get_registry(I4_REGISTRY_USER,
                      0,
                      var, buffer, max_buffer))
	{
	if (strlen(buffer)>0)
	//otherwise, the unix ini-getter just returned an empty string->bad
	    	return ;
	}
  
  char *c=getenv(var);
  if (c)
  {
    strncpy(buffer, c, max_buffer);
    return ;
  }
  
  strcpy(buffer, def);

}

class r1_resource_manager_class : public i4_init_class
{
public:
  //i4_string_manager_class r1_strings;



  void init() 
  {
	char decomp_prefix[60];
	get("G_DECOMPRESSED", decomp_prefix, 60, "g_decompressed");
	sprintf(decompressed,"%s",decomp_prefix);
    
//todo: Need a replacement for this code
	//golgotha/render.res has been removed, it contains no useful data
	/*
	i4_file_class *dirtest=i4_open("render.res",I4_READ);
	//check wheter we started from a valid directory
	if (!dirtest)
		{
		i4_chdir("..");//used for visual studio if started from release\ or debug\ dir.
		i4_file_class *dirtest=i4_open("render.res",I4_READ);
		if (!dirtest)
			{
			i4_error("FATAL: Cannot find \"render.res\". Most probably Golgotha wasn't started from the correct directory. "
				"Check that your link doesn't contain some strange path.");
			}
		}
	delete dirtest;
	*/
//    i4_mkdir(compressed);
	i4_file_class *f=i4_open("resource.res");
	if (f)
		{
		delete f;
		}
	else 
		{
		i4_warning("Changing directory to parent, might be release build.");
		i4_chdir("..");
		}

    i4_mkdir(decompressed);

	//See arch.h for an explanation of I4_FILE_PREFIX
    sprintf(cache_file, "%s/%s_tex_cache.dat", decompressed, I4_FILE_PREFIX);

//    comp=compressed; 
    decomp=decompressed; 
    cache=cache_file;
    
    //r1_strings.load("render.res"); 

    //i4_const_str::iterator tmax=r1_gets("max_texture_size").begin();
    //r1_max_texture_size = tmax.read_number();
	//this should only define the ultimate default
	//in any case, entries in the registry or in golgotha.ini
	//should overwrite any setting here.
	r1_max_texture_size=256;
    
    char max_size[40];
    get("MAX_TEXTURE_SIZE", max_size, 40, "256");

    sscanf(max_size, "%d", &r1_max_texture_size);
    if (r1_max_texture_size<16) r1_max_texture_size=16;

//    i4_warning("Setting max texture size to %d", r1_max_texture_size);  
  }

} r1_resource_man;

/*
const i4_const_str &r1_gets(char *str, i4_bool barf_on_error)
{
  const i4_const_str *s=&r1_resource_man.r1_strings.get(str);

  if (barf_on_error && s->null())
    i4_error("FATAL: Render (render.res) resource missing %s", str);

  return *s; 
}
*/

const i4_const_str &r1_get_decompressed_dir()
{
  return decomp;
}

/*
const i4_const_str &r1_get_compressed_dir()
{
  return comp;
}
*/

//This function should be called once after it is known which 
//renderer is used.
void r1_name_cache_file(const char *rendername)
	{

	sprintf(cache_file,"%s/%s_%s_tex_cache.dat",decompressed, I4_FILE_PREFIX, rendername);
	cache=cache_file;
	}

const i4_const_str &r1_get_cache_file()
{
  return cache;
}

