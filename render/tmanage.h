/********************************************************************** 
  \file
  This file defines the common texture manager api
  <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_TEXTURE_MANAGER_HH
#define R1_TEXTURE_MANAGER_HH


#include "arch.h"
#include "tex_id.h"
#include "math/num_type.h"

#include "file/file.h"
#include "palette/pal.h"
#include "memory/array.h"
#include "mip.h"
#include "g1_limits.h"

class i4_image_class;
//enum { R1_CHROMA_COLOR = (254<<16) | (2<<8) | (166) };
//enum {R1_CHROMA_COLOR=G1_CHROMA_COLOR};

/*!
  This class is used for the texture cache.
*/
class r1_texture_entry_struct
{
public:  
  w32 id; // checksum of filename 
  w8  flags;

  w32 average_color;

  r1_miplevel_t *mipmaps[R1_MAX_MIP_LEVELS+1]; //null terminated
  w32 file_offsets[R1_MAX_MIP_LEVELS];

  i4_bool is_transparent()
  {
    if (flags & R1_MIP_IS_TRANSPARENT)
      return i4_T;
    else
      return i4_F;
  }
  
  i4_bool is_alphatexture()
  {
    if (flags & R1_MIP_IS_ALPHATEXTURE)
      return i4_T;
    else
      return i4_F;
  }  
};


struct r1_texture_matchup_struct
{
  w32 id;
  w16 handle;
  sw16 left, right;
  char name[128];
};

struct r1_texture_animation_entry_struct
{
  w32 id;
  w16 total_frames;
  r1_texture_handle *frames;
};

struct r1_image_list_struct{
	w32 id;
	union
		{
	w32 usage;
	w32 average_color;
		};
	i4_image_class *image;
	};
//#define RENDER_VERYMUCHSLOWER

/*!
  The base class of all texture managers.
  Currently, we have two working implementations: DirectX (Windows) and
  OpenGL (Linux).
*/

class r1_texture_manager_class
{  

protected:
  i4_array<r1_image_list_struct>	  image_list;
  i4_array<r1_texture_matchup_struct> registered_tnames;
  i4_array<r1_texture_entry_struct>   *entries;
  r1_texture_animation_entry_struct   *tanims;
  i4_array<r1_image_list_struct>	  memory_images;
public:
  i4_bool textures_loaded;///<Don't assign any semantics to this flag. It's obsolete.
  i4_bool is_master_tman;///<This is the master texture manager
  i4_bool has_higher_mipmaps_loaded;///<This texture manager has higher mipmaps loaded (not only lowest levels)

  sw32 bytes_loaded;
  sw32 no_of_textures_loaded;//don't mix with the bool value textures_loaded from superclass!


  /*!Resamples the given image to the texture data size*/
  static i4_bool size_image_to_texture(
	  void *dest, //the destination buffer (raw image data)
	  i4_image_class *image,//the source image
	  w32 width,//the destination width
	  w32 height, //the destination height
	  w32 target_depth, //the target pixel size (2,3 or 4)
	  i4_bool chroma,//the source image is color-keyed
	  i4_bool alpha);//the source image has an alpha-channel

  /*!The constructor*/
  r1_texture_manager_class(const i4_pal *pal);

  /*!The destructor*/
  virtual ~r1_texture_manager_class();

  virtual void init();///<init the tman
  virtual void uninit();  ///<clear everything. Beware: All Handles become invalid

  /*!Free up memory but don't free the registered_tnames field.
    Registering textures is possible again. Need to load_texture() again!
  */
  virtual void reopen();

  /*!get the average color of a texture (Prefer getting the lowest miplevel)*/
  w32 average_texture_color(r1_texture_handle handle, w32 frame_num);

  
  void matchup_textures();///<recalculates the handles and some references

protected:
	/*!get a handle to an id (this function cannot be used to get texture
	handles from texture ids!!!)*/
  r1_texture_handle find_texture(w32 id);
public:

  i4_bool texture_resolution_change()
  {
    i4_bool ret = texture_resolution_changed;
    texture_resolution_changed  = i4_F;
    return ret;
  }

  //!Load a texture and mark it as resident.
  void keep_resident(const i4_const_str &tname, sw32 desired_width);  

  //!Get the name of a texture
  char *get_texture_name(r1_texture_handle handle);
  //!Register a texture (returns handle to it) Handle becomes invalid on next uninit()!
  r1_texture_handle register_texture(const i4_const_str &tname,
                                     const i4_const_str &error_string,
                                     i4_bool *has_been_loaded=0);

  //!returns handle to texture, but does newer add a new texture
  r1_texture_handle query_texture(const i4_const_str &tname,
	  i4_bool *has_been_loaded=0);

  /*!Loads the given image as texture and returns its handle. 
  Warning: the textures must have been loaded before! */
  r1_texture_handle register_image(i4_image_class *image);

  //!Returns the texture data for the given handle in a new image.
  virtual i4_image_class *get_texture_image(r1_texture_handle handle)=0;

  //!Changes the given texture to the new image
  virtual int set_texture_image(r1_texture_handle handle, i4_image_class *im)=0;
  
  //! loads textures previously registered with register_texture
  i4_bool load_textures();  
  
  w32 get_animation_length(r1_texture_handle handle)
  {
    if (handle>=0) return 1;
    else return tanims[-handle-1].total_frames;
  }

  //returns i4_T if something was freed
  static i4_bool release_higher_miplevels(r1_texture_manager_class *this_tman);

  //!Can be used to free up as much memory as possible without unloading.
  virtual void release_higher_miplevels();

  //!This function mainly will activate textures that were in the async_load queue.
  virtual void next_frame();
//!this is {uninit();init();}
  virtual void reset();

  virtual void toggle_texture_loading();
  
  virtual r1_miplevel_t *get_texture(r1_texture_handle handle,
                                     w32 frame_counter,
                                     sw32 desired_width,
                                     sw32 &w, sw32 &h);

  virtual i4_bool valid_handle(r1_texture_handle handle)
  {
    if ((handle<registered_tnames.size() )
		&& (handle<total_textures )
		&& (registered_tnames[handle].handle != 0)
		)//if handle>=total_textures this is no big deal: 
		//it indicates only that the textures are not loaded right now. 
      return i4_T;
#ifdef RENDER_VERYMUCHSLOWER
    else
		{
		if (handle<registered_tnames.size())
			{
			i4_warning("Trouble on texture %s(%x): Could not load.",registered_tnames[handle].name,registered_tnames[handle].id);
			}
		return i4_F;
		}
#else
	return i4_F;
#endif
  }
  const i4_pal *get_pal(){return pal;}
protected:
  //virtual i4_bool load_texture_from_file(char *name,w32 id,void *data, w32 width, w32 height, w32 pitch);
  virtual i4_bool load_texture_from_file(const i4_const_str &name,w32 id,void *data, 
	  w32 width, w32 height, w32 pitch,w32 tex_by, i4_bool chroma, i4_bool alpha);
//!functions to manage the texture cache file
  void keep_cache_current(i4_array<w32> *file_ids);

  i4_bool build_cache_file(i4_array<w32> &texture_file_ids,
                           const i4_const_str &local_dir);

  i4_bool update_cache_file(i4_array<w32> &update_ids,
                            const i4_const_str &local_dir);

  

  sw32 total_textures;
  sw32 total_tanims;
  
  //time keeper
  sw32 frame_count;

  i4_bool texture_load_toggle;

  i4_bool texture_resolution_changed;

  const i4_pal *pal;
  
  //////////////////////////// derive / specify these for each texture manager derivative
  static i4_pixel_format regular_format;
  static i4_pixel_format chroma_format;
  static i4_pixel_format alpha_format;  
  static i4_pixel_format reg24_format;
  static i4_pixel_format reg32_format;
  static i4_pixel_format alpha32_format;
  w32 default_texture_flags;
  
  sw32    min_texture_dimention;
  sw32    max_texture_dimention;
  i4_bool square_textures;
  
public:
  //!This must be overwritten
  virtual i4_bool immediate_mip_load(r1_mip_load_info *load_info) = 0;

  //!This must also be overwritten
  virtual i4_bool async_mip_load(r1_mip_load_info *load_info) = 0;
//protected:
//	int async_mip_prepare_load(w32 tex_by, async_callback call);
//public:
  //Duh... cannot virtually derive, since all renderers 
  //have different used_node types which aren't truly related.
//	virtual void async_load_finished(used_node *u)=0;

  virtual void free_mip(r1_vram_handle_type vram_handle) = 0;          
};


#endif
