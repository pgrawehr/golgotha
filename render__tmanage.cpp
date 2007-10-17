/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "render/tmanage.h"
#include "memory/malloc.h"
#include "error/error.h"
#include "checksum/checksum.h"
#include "image/color.h"
#include "string/str_checksum.h"
#include "error/alert.h"
#include "time/profile.h"
#include "string/string.h"
#include "status/status.h"
#include "file/ram_file.h"
#include "render/tex_cache.h"
#include "file/file.h"
#include "file/async.h"
#include "search.h"
#include "render/r1_res.h"
#include "image/image.h"
#include "image/context.h"
#include "time/profile.h"
//#ifdef _WINDOWS
//#include "video/win32/dx5_util.h"
//#else
//#include "render/opengl/opengl_tman.h"
//#endif
#include "loaders/load.h"
#include "lisp/lisp.h"
#include "g1_render.h"
#include "g1_limits.h"
#include "render/r1_api.h"

//I don't think this need to be better in the next years, but I might be wrong...
#define MAX_USABLE_TEXTURE_DIMENTION 1024
//#define TEXTURE_LOAD_DEBUG

i4_profile_class pf_register_texture("register_texture");
i4_profile_class pf_loading_texture("load and size texture");
i4_profile_class pf_jpg_texture_load("JPG-Loader");
i4_profile_class pf_size_texture("size image to texture");

//w32 R1_CHROMA_COLOR = (254<<16) | (2<<8) | (166); //use G1_CHROMA_COLOR instead

i4_profile_class pf_get_texture("tmanage get texture");

r1_texture_manager_class * r1_texture_manager=0;

i4_pixel_format r1_texture_manager_class::regular_format;
i4_pixel_format r1_texture_manager_class::alpha_format;
i4_pixel_format r1_texture_manager_class::chroma_format;
i4_pixel_format r1_texture_manager_class::reg24_format;
i4_pixel_format r1_texture_manager_class::reg32_format;
i4_pixel_format r1_texture_manager_class::alpha32_format;

r1_texture_manager_class::r1_texture_manager_class(const i4_pal * pal)
	: pal(pal),
	  registered_tnames(0,128),
	  image_list(16,64),
	  memory_images(0,64)
{
	textures_loaded=i4_F;
	square_textures = i4_F;
	has_higher_mipmaps_loaded=i4_F;
	is_master_tman=i4_T; //Assume we are master.
	default_texture_flags=R1_MIPFLAGS_USE16;
	null_texture_handle=0;
	//install_new_tmanager() will reset this.
}

void r1_texture_manager_class::init()
{
	texture_load_toggle = i4_T;
	texture_resolution_changed = i4_F;
	textures_loaded=i4_F;
	has_higher_mipmaps_loaded=i4_F;


	frame_count       = 0;
	entries           = 0;
	tanims            = 0;

	total_tanims   = 0;
	total_textures = 0;

	registered_tnames.clear();
	image_list.clear();
	memory_images.clear();

}

char * r1_texture_manager_class::get_texture_name(r1_texture_handle handle)
{
	if (handle<registered_tnames.size())
	{
		return registered_tnames[handle].name;
	}
	else
	{
		return "invalid";
	}
}

r1_texture_manager_class::~r1_texture_manager_class()
{
	uninit();
}

void r1_texture_manager_class::reopen()
{
	sw32 i,j;

	while (!i4_async_reader::is_idle())
	{
		i4_thread_sleep(1); //wait until nothing left in queue
		//might bomb if a texture request is pending.
	}
	i4_thread_sleep(10);
	next_frame();
	for (i=1; i<total_textures; ++i)
	{
		for (j=0; j<R1_MAX_MIP_LEVELS; ++j)
		{
			r1_miplevel_t * m = (*entries)[i].mipmaps[j];
			if (m)
			{
				if(m->vram_handle)
				{
					free_mip(m->vram_handle);
					m->vram_handle = 0;
				}
				delete m;
				m = 0;
			}
		}
		//  delete (*entries)[i];
	}

	if (entries)
	{
		delete entries;
		entries = 0;
	}
	texture_load_toggle = i4_T;
	texture_resolution_changed = i4_F;
	textures_loaded=i4_F;
	total_textures=0;
	if (tanims)
	{
		delete tanims;
	}
	tanims=0;
	total_tanims=0;
	has_higher_mipmaps_loaded=i4_F;

}

void r1_texture_manager_class::uninit()
{
	sw32 i,j;

	//before freing memory, we must be sure that no requests are pending
	while (!i4_async_reader::is_idle())
	{
		i4_thread_sleep(1); //wait until nothing left in queue
		//might bomb if a texture request is pending.
		next_frame();
	}
	i4_thread_sleep(10);
	next_frame();

	for (i=1; i<total_textures; ++i)
	{
		for (j=0; j<R1_MAX_MIP_LEVELS; ++j)
		{
			r1_miplevel_t * m = (*entries)[i].mipmaps[j];
			if (m)
			{
				if(m->vram_handle)
				{
					free_mip(m->vram_handle);
					m->vram_handle = 0;
				}
				delete m;
				m = 0;
			}
		}
		//delete (*entries)[i];
	}
	int x;
	if (entries)
	{
		delete entries;
		entries = 0;
	}
	for (x=0; x<image_list.size(); ++x)
	{
		if (image_list[x].image)
		{
			delete image_list[x].image;
		}
		image_list[x].image=0;
	}
	image_list.uninit();
	registered_tnames.uninit();
	for (x=0; x<memory_images.size(); ++x)
	{
		delete memory_images[x].image;
	}
	memory_images.uninit();

	texture_load_toggle = i4_T;
	texture_resolution_changed=i4_F;

	tanims      = 0;
	frame_count =0;
	total_tanims   = 0;
	total_textures = 0;
	textures_loaded = i4_F;
	has_higher_mipmaps_loaded=i4_F;
}

void r1_texture_manager_class::reset()
{
	uninit();
	init();
}
/*
   r1_texture_manager_class *r1_texture_manager_class::set_tmanager(r1_texture_manager_class *newtman)
   	{
   	if (!newtman)
   		i4_error("r1_texture_manager_class::set_tmanager(): Must use valid tmanager.");
   	r1_texture_manager_class *oldtman=tmanager;
   	oldtman->reopen();//Free the memory used by the old tman
   	tmanager=newtman;
   	return oldtman;
   	}
 */

inline void write_to_image(w8 * dst,w32 col,w32 tex_by)
{
	if (tex_by==2)
	{
		(*((w16 *)dst))=(w16)col;
	}
	else if (tex_by==4)
	{
		(*((w32 *)dst))=(w32)col;
	}
	else
	{
		*dst=(w8)(col>>16);
		++dst;
		*dst=(w8)(col>>8);
		++dst;
		*dst=(w8)col;
	}
}
/** Convert Images from any source format to any destination format.
   This method converts an image from any source format to any
   destination format. Arbitrary shrinking and stretching is performed
   if necessary.
   Hint 1: This function must be static and thread-safe.
   Hint 2: One can assume that the static *_format members have been set up correctly
   and won't change at runtime.
   Flaw: This function is time critical, and could still be further optimized
   by avoiding the virtual calls to the i4_image_class::get_pixel() methods.
   @param dest Pointer to the raw data block where to put the target image
   @param image The source image. Its palette must be set correctly.
   @param width Desired target width
   @param height Desired target height
   @param target_depth May be 2, 3 or 4 to indicate a 16, 24 or 32 bit target image.
   @param chroma True if chroma color keying is used on this image.
   @param alpha True if the destination image uses alpha, i.e. it is an ARGB image.
   @return Always true
 */
i4_bool r1_texture_manager_class::size_image_to_texture(void * dest, i4_image_class * image, w32 width, w32 height, w32 target_depth, i4_bool chroma, i4_bool alpha)
{
	pf_size_texture.start();
	i4_bool extend_chroma=i4_F; //used if chroma needs to be converted to alpha (32 bit only)

	i4_pixel_format * convfor=&regular_format;

	const i4_pixel_depth format=image->get_pal()->source.pixel_depth;
	int convert_depth=0;

	if (!((((format==I4_16BIT) && (target_depth==2)) || ((format==I4_24BIT) && (target_depth==3))) || ((format==I4_32BIT) && (target_depth==4))))
	{
		convert_depth=1; //different texture depth -> we need to convert it


		if (target_depth==2)
		{
			if (chroma)
			{
				convfor=&chroma_format;
				if (chroma_format.alpha_bits>0)
				{
					extend_chroma=i4_T;
				}
				//chroma might also be handled in the renderer.
				//so extend_chroma is only set if chroma format is 1444
				//otherwise, we assume the renderer checks each pixel
				//before drawing (using G1_16BIT_CHROMA_565)
			}
			if (alpha)
			{
				convfor=&alpha_format;
			}
		}
		else
		if (target_depth==3)
		{
			convfor=&reg24_format;
		}
		else if (target_depth==4)
		{
			if (alpha||chroma)
			{
				convfor=&alpha32_format;
			}
			else
			{
				convfor=&reg32_format;
			}
			extend_chroma=chroma;
		}

	}
	if (target_depth==4 && chroma)
	{
		convert_depth=true;
		convfor=&reg32_format;
		extend_chroma=true;
	}
	if ((width!=image->width()) || (height!=image->height()))
	{
		//need to scale the image
		//this gets complicated, as we actually need to convert from any source
		//format and size to any destination size
		double base_width=image->width();
		double base_height=image->height();

		double width_ratio  = (double)base_width  / (double)width;
		double height_ratio = (double)base_height / (double)height;

		//now scale the old to fit the new, convert from source format to target
		w32 i,j;

		w8 * dst=(w8 *)dest;

		for (j=0; j<height; ++j)
		{
			for (i=0; i<width; ++i)
			{
				w32 c = image->get_pixel((i4_coord) i4_f_to_i((float) i * (float) width_ratio),
										 (i4_coord) i4_f_to_i((float) j * (float) height_ratio));
				w32 c2;
				if (convert_depth==1)
				{
					c2=image->pal->convert(c,convfor);
					if (target_depth==4)
					{
						if (extend_chroma)
						{
							if (c2!=G1_CHROMA_COLOR)
							{
								c2|=0xff000000;
							}
							else
							{
								c2=0;
							}         //set the chroma color in the image to black
							//this prevents the nasty pink borders
						}

					}
					else if (target_depth==2)
					{
						if (extend_chroma)
						{
							if (c2!=G1_16BIT_CHROMA)
							{
								c2|=0x8000;
							}
							else
							{
								c2=0;
							}
						}

					}
				}
				else
				{
					c2=c;
				}
				write_to_image(dst,c2,target_depth);
				//*dst=c2;
				dst+=target_depth;
			}
		}


	}
	else if (convert_depth==1)
	{

		w8 * textu=(w8 *)dest;

		for(w16 apy=0; apy<height; ++apy)
		{
			for(w16 apx=0; apx<width; ++apx)
			{

				w32 oldc=image->get_pixel(apx,apy);

				w32 c=image->pal->convert(oldc,
										  convfor);
				if (target_depth==4)
				{
					if (extend_chroma )
					{
						if (c!=G1_CHROMA_COLOR)
						{
							c|=0xff000000;
						}
						else
						{
							c=0;
						}
					}
				}
				else if (target_depth==2)
				{
					if (extend_chroma )
					{
						if (c!=G1_16BIT_CHROMA)
						{
							c|=0x8000;
						}
						else
						{
							c=0;
						}
					}
				}

				write_to_image(textu,c,target_depth);
				textu+=target_depth;
			}
		}
	}
	else
	{
		//No resizing and no re-coloring required. Just bulk-copy the data
		memcpy(dest,image->data,height*width*target_depth);
	}
	//write_to_image((w8*)dest,0x00ffff,target_depth);
	pf_size_texture.stop();
	return i4_T;
}

i4_bool r1_texture_manager_class::load_texture_from_file(const i4_const_str &n,w32 id,void * data,
														 w32 width,w32 height,
														 w32 pitch, w32 tex_by,
														 i4_bool chroma, i4_bool alpha)
{
	//ZeroMemory(data,width*height*2);
	pf_loading_texture.start();
	i4_image_class * im=NULL;
	if (!id)
	{
		char buf[100],buf2[150];
		i4_os_string(n,buf,100);
		sprintf(buf2,"textures/%s.jpg",buf);
		pf_jpg_texture_load.start();
		im=i4_load_image(i4_const_str(buf2),NULL);
		pf_jpg_texture_load.stop();
	}
	else
	{
		for(int x=0; x<image_list.size(); ++x)
		{
			if (image_list[x].id==id)
			{
				im=image_list[x].image;
				image_list[x].usage=30;
			}
			else
			{
				image_list[x].usage--;
				if (image_list[x].usage==0)
				{
					image_list[x].usage=1;
				}
			}
		}
		if (!im)
		{
			char buf[100],buf2[150];
			i4_os_string(n,buf,100);
			sprintf(buf2,"textures/%s.jpg",buf);
			i4_bool must_free_mem=i4_largest_free_block()<0x0100000;
			for (int x2=0; x2<image_list.size(); ++x2) //cleanup old entries
			{
				if (image_list[x2].usage<2&&(!image_list[x2].is_locked()))
				{
					delete image_list[x2].image;
					image_list.remove(x2);
					x2--; //Retry same index (has changed now)
				}
			}
			if (must_free_mem&&image_list.size()>5)
			{
				//We immediately need some free mem
				for (int x3=0; x3<3; ++x3)
				{
					//just delete some of the oldest entries
					if (image_list[0].is_locked())
					{
						continue;
					}
					delete image_list[0].image;
					image_list.remove(0);
				}
			}
			pf_jpg_texture_load.start();
			im=i4_load_image(i4_const_str(buf2),NULL);
			if (!im)
			{
				sprintf(buf2,"textures/%s.tga",buf);
				im=i4_load_image(i4_const_str(buf2),NULL);
				if (!im)
				{
					pf_jpg_texture_load.stop();
					pf_loading_texture.stop();
					return i4_F;
				}
			}
			pf_jpg_texture_load.stop();

			r1_image_list_struct * ils=image_list.add();
			ils->init();
			ils->image=im;
			ils->id=id;
		}

	}
	if (!im)
	{
		pf_loading_texture.stop();
		return i4_F;
	}
	size_image_to_texture(data,im,width,height,tex_by,chroma,alpha);
	pf_loading_texture.stop();
	return i4_T;
};

/*
   i4_bool r1_texture_manager_class::load_texture_from_file(char *name,w32 id,void *data, w32 width, w32 height, w32 pitch)
   	{
   	//char buf[200];
   	//sprintf(buf,"textures/%s.jpg",name);
   	i4_const_str n(name);
   	return load_texture_from_file(n,id,data,width,height,pitch);
   	}
 */
int entry_compare(const r1_texture_entry_struct * a, const r1_texture_entry_struct * b)
{
	if (a->id<b->id)
	{
		return -1;
	}
	else if (a->id>b->id)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

r1_texture_handle r1_texture_manager_class::find_texture(w32 id)
{
	sw32 lo=1,hi=total_textures-1; //,mid;

	if (!entries)
	{
		return 0;
	}

	r1_texture_entry_struct search_entry;
	search_entry.id = id;

	sw32 location = entries->binary_search(&search_entry,entry_compare);

	if (location==-1)
	{
		return 0;
	}

	return (short)location;
}


void r1_texture_manager_class::matchup_textures()
{
	sw32 i;

	for (i=0; i<registered_tnames.size(); ++i)
	{
		r1_texture_handle h = find_texture(registered_tnames[i].id);

		if (h)
		{
			registered_tnames[i].handle = h;
		}                             //this handle is NOT persistent
		//do not store this anywhere else, it might change anytime.
		else
		{
			registered_tnames[i].handle = 0;
			i4_warning("Texture matchup failed: %s. ID %x",registered_tnames[i].name, registered_tnames[i].id);
		}
	}


}

int name_compare(const r1_texture_matchup_struct * a, const r1_texture_matchup_struct * b)
{
	return strcmp(a->name, b->name);
}

r1_texture_handle r1_texture_manager_class::query_texture(const i4_const_str &tname, i4_bool * has_been_loaded)
{
	int found = -1;


	r1_texture_matchup_struct t;
	i4_filename_struct fns;

	i4_split_path(tname,fns);
	i4_os_string(fns.filename, t.name, 128);
	t.id     = r1_get_texture_id(tname);
	t.handle = 0;
	t.left   = -1;
	t.right  = -1;
	if (has_been_loaded)
	{
		*has_been_loaded=i4_F;
	}

	// search the tree for the name
	if (registered_tnames.size())
	{
		int root=0, parent=-1;
		while (found<0)
		{
			parent=root;

			int res=strcmp(registered_tnames[root].name, t.name);
			if (res==0)
			{
				if(has_been_loaded)
				{
					*has_been_loaded=i4_T;
				}
				found = root;
			}
			else if (res<0)
			{
				root=registered_tnames[root].left;
			}
			else
			{
				root=registered_tnames[root].right;
			}

			if (root==-1)
			{
				//found = -1;
				return -1;
			}
		}
	}
	else
	{
		found = -1;
	}

	return found;
}

r1_texture_handle r1_texture_manager_class::register_texture(const i4_const_str &tname,
															 const i4_const_str &error_string,
															 i4_bool * has_been_loaded)
{
	//The handle returned by this function is persistent over reloads
	pf_register_texture.start();

	//if (textures_loaded)
	//  i4_error("textures already loaded");

	int found = -1;


	r1_texture_matchup_struct t;
	i4_filename_struct fns;
	//be shure to compare only the actual name of the texture
	i4_split_path(tname,fns);
	i4_os_string(fns.filename, t.name, 128);
	//i4_os_string(tname, t.name, 128);
	t.id     = r1_get_texture_id(tname);
	t.handle = 0;
	t.left   = -1;
	t.right  = -1;

	if (has_been_loaded)
	{
		*has_been_loaded=0;
	}

	// search the tree for the name
	if (registered_tnames.size())
	{
		int root=0, parent=-1;
		while (found<0)
		{
			parent=root;

			int res=strcmp(registered_tnames[root].name, t.name);
			if (res==0)
			{
				if (has_been_loaded)
				{
					*has_been_loaded=1;
				}
				found = root;
			}
			else if (res<0)
			{
				root=registered_tnames[root].left;
			}
			else
			{
				root=registered_tnames[root].right;
			}

			if (root==-1)
			{
				if (res<0)
				{
					found = registered_tnames[parent].left=registered_tnames.add(t);
				}
				else
				{
					found = registered_tnames[parent].right=registered_tnames.add(t);
				}
			}
		}
	}
	else
	{
		found=registered_tnames.add(t);
	}

	pf_register_texture.stop();
	return found;
}

int w32_compare(const w32 * a, const w32 * b)
{
	if (*a < *b)
	{
		return -1;
	}
	else
	if (*a > *b)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

w32 r1_get_file_id(const i4_const_str &fname);


void r1_texture_manager_class::toggle_texture_loading()
{
	texture_load_toggle = (i4_bool)(!texture_load_toggle);
}

void r1_texture_manager_class::release_higher_miplevels()
{
	sw32 i,j;

	for (i=1; i<total_textures; ++i)
	{
		for (j=0; j<(R1_MAX_MIP_LEVELS-1); ++j)
		{
			r1_miplevel_t * m = (*entries)[i].mipmaps[j];
			if (m&& (*entries)[i].mipmaps[j+1]) //delete if there is one more miplevel left
			{
				if(m->vram_handle)
				{
					free_mip(m->vram_handle);
					m->vram_handle = 0;
				}
			}
		}
	}
	has_higher_mipmaps_loaded=i4_F;
}

i4_bool r1_texture_manager_class::release_higher_miplevels(r1_texture_manager_class * this_tman)
{
	r1_render_api_class * api=g1_render.r_api;
	i4_bool ret=i4_F;
	r1_texture_manager_class * m=0;

	if (!this_tman->is_master_tman)
	{
		m=api->get_tmanager();
		if (m->has_higher_mipmaps_loaded)
		{
			m->release_higher_miplevels();
			ret=i4_T;
		}
	}
	int i=0;
	//contains an actual bug, because iterating over all tmans
	//is not correct like this, but since we currently have only two, who cares?
	//todo: fix it. (problem is, that get_tmanager() may return 0,
	//although it's not the last active)
	while ((m=api->get_tmanager(i))!=0)
	{
		if (m!=this_tman&&m->has_higher_mipmaps_loaded)
		{
			m->release_higher_miplevels();
			ret=i4_T;
		}
		++i;
	}

	return ret;
}

r1_miplevel_t * r1_texture_manager_class::get_texture(r1_texture_handle _handle,
													  w32 frame_counter,
													  sw32 desired_width,
													  sw32 &ret_w, sw32 &ret_h)
{
	pf_get_texture.start();

	if (desired_width > max_texture_dimention)
	{
		//ignore desired with and just load up the highest miplevel,
		//we'll sometime need it anyway.
		//Well, not shure wheter this is a good idea (uses less memory but more time)
		desired_width = max_texture_dimention;
	}

	if (desired_width>8)
	{
		has_higher_mipmaps_loaded=i4_T;
	}

	r1_texture_entry_struct * e;

	r1_texture_handle handle = registered_tnames[_handle].handle;
	if (handle==0)
	{
		i4_error("get_texture called with invalid handle");
	}

	if (handle>=total_textures)
	{
		i4_error("asking for bad texture");
	}

	if (handle<0) // this is an animation
	{
		r1_texture_animation_entry_struct * ta=tanims+(-handle-1);
		handle=ta->frames[frame_counter%ta->total_frames];
		if (!handle)
		{
			pf_get_texture.stop();
			return 0;
		}
	}

	e = &(*entries)[handle];

	sw32 i = 0;
	sw32 highest_resident = -1;
	sw32 need_to_use = 0;

	r1_miplevel_t * t = 0;

	//find the highest resident mip. can be precalculated and updated ?
	for (i=0; i<R1_MAX_MIP_LEVELS; ++i)
	{
		t = e->mipmaps[i];

		if (t)
		{
			if (t->width >= desired_width)
			{
				need_to_use = i;
			}

			if (t->vram_handle)
			{
				if (t->width < desired_width)
				{
					//if there were no more higher than this,
					//this is the best we can do
					if (highest_resident==-1)
					{
						highest_resident = i;
					}
					break;
				}
				else
				{
					highest_resident = i;
				}
			}
		}
		else
		{
			break;
		}
	}

	if (highest_resident==-1)
	{
		i4_warning("No textures resident for handle %d", handle);
		return 0;
	}

	r1_miplevel_t * return_mip = e->mipmaps[highest_resident];

	if (!return_mip)
	{
		i4_error("FATAL: catastophic error - r1_texture_manager_class::get_texture: return_mip is 0");
	}

	//if (!return_mip->vram_handle)
	//  i4_warning("returned a mip w/no vram handle");

	//this is to prevent the load below from deleting this miplevel from vram
	sw32 old_last_frame = return_mip->last_frame_used;
	return_mip->last_frame_used = frame_count;

	//loading information
	r1_mip_load_info load_info;
	load_info.flags=default_texture_flags;

	//i4_warning("texture loading is OFF");
	if (texture_load_toggle && (highest_resident != need_to_use) && e->mipmaps[need_to_use])
	{
		load_info.src_file = 0;
		load_info.dest_mip = e->mipmaps[need_to_use];
		load_info.error=0;
		//should be asynchronous

		//prevents from trying to load this same mip level twice at the same time
		if (!(load_info.dest_mip->flags & R1_MIPLEVEL_IS_LOADING))
		{
			//if there is no room for this mip, load up as high a level as possible
			while (!async_mip_load(&load_info))
			{
				if (load_info.error==R1_MIP_LOAD_NO_ROOM)
				{
					//try to load a lower res
					++need_to_use;
					if (need_to_use==highest_resident)
					{
						break;
					}
					if (!e->mipmaps[need_to_use])
					{
						break;
					}
					if (e->mipmaps[need_to_use]->flags & R1_MIPLEVEL_IS_LOADING)
					{
						break;
					}
					load_info.dest_mip = e->mipmaps[need_to_use];
				}
				else
				{
					break;
				}
			}
		}
		//else
		//{
		//  i4_warning("texture still loading, wont queue");
		//}
	}

	pf_get_texture.stop();

	//store the old information back.
	return_mip->last_frame_used = old_last_frame;
	ret_w=return_mip->width;
	ret_h=return_mip->height;
	return return_mip;
}

w32 r1_texture_manager_class::average_texture_color(r1_texture_handle _handle, w32 frame_num)
{
	r1_texture_handle handle = registered_tnames[_handle].handle;

	r1_texture_entry_struct * e;

	if (handle>=total_textures)
	{
		i4_error("SEVERE: Asking for bad texture");
	}

	if (handle<0) // this is an animation
	{
		r1_texture_animation_entry_struct * ta=tanims+(-handle-1);
		handle=ta->frames[frame_num%ta->total_frames];
		if (!handle)
		{
			pf_get_texture.stop();
			return 0;
		}
	}

	e = &(*entries)[handle];

	return e->average_color;
}


void r1_texture_manager_class::next_frame()
{
	++frame_count;
	//eventually (after a long fucking time) this could
	//wrap around. just make sure its always > 0
	if (frame_count<0)
	{
		frame_count=0;
	}
}

int tex_entry_compare(const tex_cache_entry_t * a, const tex_cache_entry_t * b)
{
	if (a->id > b->id)
	{
		return 1;
	}
	else
	if (a->id < b->id)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

tex_cache_entry_t *find_id_in_tex_cache(tex_cache_entry_t * entries, w32 num_entries, w32 search_id)
{
	if (!entries || num_entries==0)
	{
		return 0;
	}

	w32 res;

	tex_cache_entry_t search;
	search.id = search_id;

	if (!i4_base_bsearch(&search,res,entries,
						 sizeof(tex_cache_entry_t),
						 num_entries,
						 (i4_bsearch_compare_function_type)tex_entry_compare))
	{
		return 0;
	}

	return entries+res;
}

i4_bool palettes_are_same(i4_pixel_format * a, i4_pixel_format * b)
{
	sw32 i;

	w8 * compare_a = (w8 *)a;
	w8 * compare_b = (w8 *)b;

	for (i=0; i<sizeof(i4_pixel_format); ++i)
	{
		if (*compare_a != *compare_b)
		{
			return i4_F;
		}

		++compare_a;
		++compare_b;
	}

	return i4_T;
}

//#include "tex_cache.cc"
////////////////////////////////////
////////////////////////////////////
////////////////////////////////////
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//WARNING: this code is really nauseating.
#include "render/tex_cache.h"
#include "render/mip_average.h"
#include "status/status.h"
#include "file/file.h"
#include "error/error.h"
#include "render/r1_res.h"

void do_single_file(i4_file_class * dst_file,
					w32 id,
					sw32 max_texture_dimention,
					w32 network_file_date,
					tex_cache_entry_t * t);

struct file_status_sort_struct
{
	w32 id;
	w32 file_status_index;
};

int file_status_struct_compare(const file_status_sort_struct * a, const file_status_sort_struct * b)
{
	if (a->id<b->id)
	{
		return -1;
	}
	else if (a->id>b->id)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


void tex_cache_entry_t::write(i4_file_class * f)
{
	if (!f)
	{
		i4_warning("tex_cache_entry_t::write() NULL file");
		return;
	}

	f->write_32(id);
	f->write_32(lowmipoffset);
	f->write_32(ext_lowmipoffset);
	f->write_32(average_color);
	f->write_32(last_modified);

	f->write_16(base_width);
	f->write_16(base_height);

	f->write_8(flags);
	f->write_8(num_mip_levels);
}

void tex_cache_entry_t::read(i4_file_class * f)
{
	if (!f)
	{
		i4_warning("tex_cache_entry_t::read() NULL file");
		return;
	}

	id             = f->read_32();
	lowmipoffset   = f->read_32();
	ext_lowmipoffset=f->read_32();
	average_color  = f->read_32();
	last_modified  = f->read_32();

	base_width     = f->read_16();
	base_height    = f->read_16();

	flags          = f->read_8();
	num_mip_levels = f->read_8();
}



void tex_cache_header_t::write(i4_file_class * f)
{
	if (!f)
	{
		i4_warning("tex_cache_header_t::write() NULL file");
		return;
	}

	f->write_32(TEX_CACHE_DATA_VERSION);
	regular_format.write(f);
	chroma_format.write(f);
	alpha_format.write(f);

	f->write_32(max_mip_dimention);
	f->write_32(num_entries);
	f->write_32(flags);
}

void tex_cache_header_t::read(i4_file_class * f)
{
	if (!f)
	{
		i4_warning("tex_cache_header_t::read() NULL file");
		return;
	}

	if (f->size() < (w32)tex_cache_header_disk_size())
	{
		i4_warning("WARNING: texture cache file corrupt (too small)");
		regular_format.default_format();
		chroma_format.default_format();
		alpha_format.default_format();
		max_mip_dimention = 0;
		num_entries       = 0;
		entries           = 0;
		flags=0;
		version=0;
		return;
	}

	//seek to the end and read the last dword

	f->seek(f->size()-4);
	sw32 expected_size = f->read_32();

	if ((w32)expected_size != f->size())
	{
		i4_warning("WARNING: texture cache file corrupt (size does not match exspected size)");
		entries = 0;
		num_entries = 0;
		version=0;
		return;
	}

	//seek back to the beginning and start reading the file
	f->seek(0);
	version=f->read_32();
	if (version!=TEX_CACHE_DATA_VERSION)
	{
		i4_warning("Tex_cache.dat file has invalid version, gotta rebuild.");
		entries=0;
		version=0;
		num_entries=0;
		return;
	}

	regular_format.read(f);
	chroma_format.read(f);
	alpha_format.read(f);

	max_mip_dimention = f->read_32();
	num_entries = f->read_32();
	flags=f->read_32();

	w32 entry_lump_size = tex_cache_entry_disk_size() * num_entries;

	//read in the entries

	if (entry_lump_size)
	{
		entries = (tex_cache_entry_t *)I4_MALLOC(sizeof(tex_cache_entry_t) * num_entries,"tex_cache_entries");
		f->seek(f->size() - 4 - entry_lump_size);
	}
	else
	{
		num_entries = 0;
		entries     = 0;
	}

	sw32 i;

	for (i=0; i<(int)num_entries; ++i)
	{
		entries[i].read(f);
	}
}


i4_bool r1_texture_manager_class::update_cache_file(i4_array<w32> &update_ids,
													const i4_const_str &local_dir)
{
	sw32 i,j,k;

	i4_array<w32>               texture_file_ids(128,128);
	i4_array<tex_cache_entry_t> new_cache_entries(128,128);

	//i4_status_class *stat=i4_create_status(r1_gets("checking_times"), I4_STATUS_UNKNOWN_TOTAL);
	//stat->update(0);


	i4_directory_struct ds;
	i4_get_directory("textures", ds, i4_T, 0);

	//delete stat;

	for (i=0; i<(int)ds.tfiles; ++i)
	{
		texture_file_ids.add(r1_get_texture_id(*ds.files[i]));
	}

	i4_file_class * old_cache_file = i4_open(r1_get_cache_file(), I4_READ | I4_NO_BUFFER);

	w32 csize = old_cache_file->size();

	void * old_cache_file_data = I4_MALLOC(csize,"old cache file data");
	old_cache_file->read(old_cache_file_data,csize);

	delete old_cache_file;

	old_cache_file = new i4_ram_file_class(old_cache_file_data,csize);

	tex_cache_header_t old_cache_header;

	old_cache_header.read(old_cache_file);


	w32 total_cache_entries = old_cache_header.num_entries;
	w32 old_cache_header_num_entries = old_cache_header.num_entries;

	for (i=0; i < update_ids.size();)
	{
		tex_cache_entry_t * t = find_id_in_tex_cache(old_cache_header.entries,old_cache_header.num_entries,update_ids[i]);

		if (t)
		{
			update_ids.remove(i); //pull this out of the list
			t->flags |= R1_MIP_EXTRA_FLAG;
		}
		else
		{
			//new textures going into the cache file
			++total_cache_entries;
			++i;
		}
	}

	i4_file_class * new_cache_file = i4_open(r1_get_cache_file(), I4_WRITE | I4_NO_BUFFER);

	old_cache_header.num_entries = total_cache_entries;
	old_cache_header.write(new_cache_file);

	r1_setup_decompression(&regular_format,&chroma_format,&alpha_format,G1_CHROMA_COLOR,square_textures);

	update_ids.sort(w32_compare);

	j = 0;
	i = 0;

	i4_bool i_done = i4_F;
	i4_bool j_done = i4_F;
	//delete stat;

	i4_status_class * stat = i4_create_status(i4gets("updating_texture_cache"));

	for(;;)
	{
		if (j>=update_ids.size())
		{
			j_done = i4_T;
		}

		if (i>=(int)old_cache_header_num_entries)
		{
			i_done = i4_T;
		}

		if (i_done && j_done)
		{
			break;
		}

		w32 file_offs = new_cache_file->tell();

		if ((j_done && !i_done) || (!i_done && old_cache_header.entries[i].id < update_ids[j]))
		{
			tex_cache_entry_t * t = &old_cache_header.entries[i];

			if (!(t->flags & R1_MIP_EXTRA_FLAG))
			{
				if (t->lowmipoffset != 0xFFFFFFFF)
				{
					old_cache_file->seek(t->lowmipoffset);
					sw32 base_size=t->base_width  / (1<<(t->num_mip_levels-1)) *
									t->base_height / (1<<(t->num_mip_levels-1));
					sw32 entry_size = sizeof(sw32) * 2 +
									  base_size* 6;

					t->lowmipoffset = file_offs;
					t->ext_lowmipoffset=file_offs+base_size*2;

					w8 copy_buffer[2048];
					while (entry_size)
					{
						sw32 copy_size = (entry_size < 2048) ? (entry_size) : (2048);

						old_cache_file->read(copy_buffer, copy_size);
						new_cache_file->write(copy_buffer,copy_size);

						entry_size -= copy_size;
					}
				}
			}
			else //MIP_EXTRA_FLAG set, meaning this texture needs updating
				 //(file is newer than entry of tex-cache.)
			{
				t->flags &= (~R1_MIP_EXTRA_FLAG);

				w32 file_date=0xFFFFFFFF;

				//have to update this texture
				for (k=0; k<texture_file_ids.size(); ++k)
				{
					if (t->id==texture_file_ids[k])
					{
						file_date = ds.file_status[k].last_modified;
						break;
					}
				}

				//dont want files in the cache w/different mip levels.
				do_single_file(new_cache_file,
							   t->id,
							   old_cache_header.max_mip_dimention,
							   file_date,
							   t);
			}

			new_cache_entries.add(*t);

			++i;
		}
		else
		if ((i_done && !j_done) || (!j_done && update_ids[j] < old_cache_header.entries[i].id))
		{
			//have to add this id
			w32 new_id = update_ids[j];

			w32 file_date=0xFFFFFFFF;

			//have to update this texture
			for (k=0; k<texture_file_ids.size(); ++k)
			{
				if (new_id==texture_file_ids[k])
				{
					file_date = ds.file_status[k].last_modified;
					break;
				}
			}

			tex_cache_entry_t new_entry;

			do_single_file(new_cache_file,
						   new_id,
						   old_cache_header.max_mip_dimention,
						   file_date,
						   &new_entry);

			new_cache_entries.add(new_entry);
			++j;
		}

		if (stat)
		{
			stat->update((new_cache_entries.size()+1) / (float)total_cache_entries);
		}
	}

	if (stat)
	{
		delete stat;
	}


	if (old_cache_header.entries)
	{
		i4_free(old_cache_header.entries);
	}

	i4_free(old_cache_file_data);
	delete old_cache_file;

	r1_end_decompression();

	if (total_cache_entries != (w32)new_cache_entries.size())
	{
		i4_error("SEVERE: Error updating the texture cache file. Did you modify the texture directory during the update?");
	}
	new_cache_entries.sort(tex_entry_compare);
	for (i=0; i<new_cache_entries.size(); ++i)
	{
		new_cache_entries[i].write(new_cache_file);
	}

	sw32 cache_size = new_cache_file->tell() + 4;

	new_cache_file->write_32(cache_size);

	delete new_cache_file;

	return i4_T;
}

i4_bool r1_texture_manager_class::build_cache_file(i4_array<w32> &texture_file_ids,
												   const i4_const_str &local_dir)
{
	//creates a tex_cache.dat "directory" w/lowest mip levels and mipheader_t info (tex_cache_entry_t, actually)
	//decompresses others to local .gtx files

	i4_array<w32> network_file_ids(128,128);

	sw32 i,j;


	r1_setup_decompression(&regular_format,&chroma_format,&alpha_format,G1_CHROMA_COLOR,square_textures);

	i4_file_class * dst_file = i4_open(r1_get_cache_file(),I4_WRITE | I4_NO_BUFFER);

	tex_cache_header_t tex_cache_header;

	memcpy(&tex_cache_header.regular_format,&regular_format,sizeof(i4_pixel_format));
	memcpy(&tex_cache_header.chroma_format,&chroma_format, sizeof(i4_pixel_format));
	memcpy(&tex_cache_header.alpha_format,&alpha_format,  sizeof(i4_pixel_format));

	tex_cache_header.max_mip_dimention = MAX_USABLE_TEXTURE_DIMENTION; //max_texture_dimention;
	tex_cache_header.num_entries       = texture_file_ids.size();
	tex_cache_header.version=tex_cache_header_t::TEX_CACHE_DATA_VERSION;
	tex_cache_header.flags=0;

	tex_cache_header.write(dst_file);

	tex_cache_entry_t * tex_cache_entries = 0;

	if (texture_file_ids.size())
	{
		tex_cache_entries = (tex_cache_entry_t *)
							I4_MALLOC(sizeof(tex_cache_entry_t) * texture_file_ids.size(),"tex_cache_entries");
	}

	//the entries will go at the end of the file. to get to the start of the entry list,
	//seek to:   filesize()-(num_entries*sizeof(tex_cache_entry_t))




	i4_status_class * stat=i4_create_status(i4gets("checking_times"), I4_STATUS_UNKNOWN_TOTAL);

	i4_directory_struct ds;
	i4_get_directory("textures", ds, i4_T, stat);
	delete stat;

	for (i=0; i<(int)ds.tfiles; ++i)
	{
		network_file_ids.add(r1_get_texture_id(*ds.files[i]));
	}

	stat = i4_create_status(i4gets("building_texture_cache"));

	//if (network_file_ids.size() != (int)ds.tfiles)
	//  i4_error("SEVERE: Something bad happened building the texture cache file");

	for (i=0; i<texture_file_ids.size(); ++i)
	{
		w32 id = texture_file_ids[i];

		w32 file_date = 0xFFFFFFFF;

		for (j=0; j<network_file_ids.size(); ++j)
		{
			if (id==network_file_ids[j])
			{
				file_date = ds.file_status[j].last_modified;
				break;
			}
		}

		//we need to decompress (for now, just copy)
		//the network (or cd image) texture to our hard drive
		do_single_file(dst_file,
					   id,
					   MAX_USABLE_TEXTURE_DIMENTION, //Now hardcoded upper limit instead of max_texture_dimention
					   file_date,
					   &tex_cache_entries[i]);


		if (stat)
		{
			stat->update((float)(i+1) / (float)texture_file_ids.size());
		}
	}

	if (stat)
	{
		delete stat;
	}

	typedef int (*qs_compare_type)(const void * x, const void * y);
	if (tex_cache_entries)
	{
		qsort(tex_cache_entries,tex_cache_header.num_entries,
			  sizeof(tex_cache_entry_t),(qs_compare_type) tex_entry_compare);
		for (i=0; i<(int)tex_cache_header.num_entries; ++i)
		{
			tex_cache_entries[i].write(dst_file);
		}
		i4_free(tex_cache_entries);
	}

	sw32 cache_size = dst_file->tell() + 4;

	dst_file->write_32(cache_size);

	delete dst_file;

	r1_end_decompression();

	return i4_T;
}


i4_bool r1_write_tga_mips(i4_image_class * src_texture,
						  char * dst_file,i4_file_class * dst_f,
						  char * texture_name, w32 chroma_color);

void do_single_file(i4_file_class * dst_file, //The tex_cache.dat file
					w32 id,                    //the id of the file to decompress
					sw32 max_texture_dimention, //the maximum texture dimention (1024)
					w32 network_file_date,  //A timestamp
					tex_cache_entry_t * t)   //The cache entry
{
	//Decompresses a single file from g_compressed to
	//g_decompressed_xBit.
	i4_str * local_fname   = r1_texture_id_to_filename(id, r1_get_decompressed_dir());
	//i4_str *network_fname = r1_texture_id_to_filename(id, r1_get_compressed_dir());

	char local_tex_file[256];

	i4_os_string(*local_fname,local_tex_file,256);
	delete local_fname;

	//char network_tex_file[256];
	//i4_os_string(*network_fname,network_tex_file,256);
	//delete network_fname;

	i4_bool loaded = i4_F;

	w32 file_offs = dst_file->tell();

	//open the mip file from the network to get information on the mip levels
	mipheader_t mipheader;
	i4_image_class * src_im=0;

	//doesn't exist anymore, per definition
	//i4_file_class *src_file = i4_open(i4_const_str(network_tex_file),I4_READ | I4_NO_BUFFER);
	//if (!src_file)
	//  i4_warning("Couldn't open compressed texture: %s.", network_tex_file);

	//void *file_buf = 0;
	//w32   file_size=0;
	//This would be a good place to create the src_file if it isn't there
	//Actually: What I mean is we decompress the texture to a i4_ram_file_class
	//and use that one. Due to the way r1_write_tga_mips() is organized,
	//we use the (dumb) way via disk. - Shan't give much trouble as this
	//is only used to build up the texture cache.

	//Uh, uh: Need to be sure that the system knows from which files the
	//texture cache is to be built
#if 0
	if (src_file)
	{
		file_size = src_file->size();
		file_buf  = I4_MALLOC(file_size,"file buffer");
		i4_file_status_struct fs;
		i4_get_status(i4_const_str(network_tex_file),fs);
		network_file_date=fs.last_modified;
		src_file->read(file_buf,file_size);
		delete src_file;

		src_file = new i4_ram_file_class(file_buf,file_size);
	}
	else
	{
#endif
	i4_const_str * n=r1_get_texture_name(id);
	char sbuf[256];
	if (n)
	{
		char sbuf2[256];
		i4_os_string(*n,sbuf,256);
		sprintf(sbuf2,"textures/%s.jpg",sbuf);
		delete n;
		src_im=i4_load_image(sbuf2,0);  //might still not be there

		if (!src_im)
		{
			sprintf(sbuf2,"textures/%s.tga",sbuf);
			src_im=i4_load_image(sbuf2,0);
		}
		if (src_im)
		{
			i4_file_status_struct fstat;
			i4_get_status(i4_const_str(sbuf2),fstat);
			network_file_date=fstat.last_modified;
			//The given filename is not used if dst_f is not NULL
			//src_file=new i4_temp_file_class();
			//r1_write_tga_mips(src_im,"g_compressed/tempfile.gtx",
			//  src_file,sbuf,R1_CHROMA_COLOR);
			//src_file=i4_open("g_compressed/tempfile.gtx",I4_READ|I4_NO_BUFFER);
			//file_size=src_file->size();
			//src_file->seek(0);
			//file_buf=I4_MALLOC(file_size,"file buffer");
			//src_file->read(file_buf,file_size);
			//delete src_file;
			//delete src_im;
			//i4_unlink("g_compressed/tempfile.gtx");
			//src_file= new i4_ram_file_class(file_buf,file_size);
		}

	}
#if 0
}
#endif

	if (src_im)
	{
		//mipheader.read(src_file);
		get_header_info(mipheader,src_im,sbuf,G1_CHROMA_COLOR);

		//need to write the following info to the dst_lowmip file:
		//width of lowest miplevel
		//height of lowest miplevel
		//width*height*2 - 16 bit texture data
		//width*height*4 - 32 bit texture data (alpha is padded if src is 24 bit only)
		//mipheader.flags &=~R1_MIP_IS_JPG_COMPRESSED; //don't use this any more
		w8 * texbuf;
		sw32 mipw=0,miph=0;
		get_mip_size(mipheader.num_mip_levels-1,mipheader.base_height,
					 mipheader.base_width,miph,mipw);
		texbuf=new w8[mipw*miph*4];
		//perhaps we'll introduce some anti-aliasing sizing function there?
		r1_texture_manager_class::size_image_to_texture(texbuf,src_im,
														mipw,miph,2,
														mipheader.flags& R1_MIP_IS_TRANSPARENT,
														mipheader.flags& R1_MIP_IS_ALPHATEXTURE);
		dst_file->write_32(mipw);
		dst_file->write_32(miph);
		dst_file->write(texbuf,mipw*miph*2);
		r1_texture_manager_class::size_image_to_texture(texbuf,src_im,
														mipw,miph,4,
														mipheader.flags&R1_MIP_IS_TRANSPARENT,
														mipheader.flags&R1_MIP_IS_ALPHATEXTURE);
		dst_file->write(texbuf,mipw*miph*4);
		loaded=i4_T;
		delete [] texbuf;

		/*

		   loaded = r1_decompress_to_local_mip(src_file,
		   									dst_file,
		   									network_tex_file,
		   									local_tex_file,
		   									&mipheader,
		   									max_texture_dimention);

		   if (file_buf)
		   i4_free(file_buf);
		 */
		delete src_im;
	}

	if (loaded)
	{
		t->id             = id;
		t->lowmipoffset   = file_offs;

		t->last_modified  = network_file_date;
		t->average_color  = mipheader.average_color;
		t->base_width     = mipheader.base_width;
		t->base_height    = mipheader.base_height;
		t->flags          = mipheader.flags;
		t->num_mip_levels = mipheader.num_mip_levels;
		sw32 t_base_size=t->base_width  / (1<<(t->num_mip_levels-1)) *
						  t->base_height / (1<<(t->num_mip_levels-1));
		t->ext_lowmipoffset=file_offs+t_base_size*2;
	}
	else
	{
		i4_warning("Failed to decompress %s. Try updating textures w/the maxtool.",sbuf);

		//be sure to set lowmipoffset to 0xFFFFFFFF and last_modified to 0 since the load failed
		t->id             = id;
		t->lowmipoffset   = 0xFFFFFFFF;
		t->ext_lowmipoffset=0xFFFFFFFF;
		t->last_modified  = 0;
		t->average_color  = 0;
		t->base_width     = 0;
		t->base_height    = 0;
		t->flags          = 0;
		t->num_mip_levels = 0;
	}
}

void reset_decompressed_cache(void);

void r1_texture_manager_class::keep_cache_current(i4_array<w32> * file_ids)
{
	tex_cache_header_t tex_cache_header;

	i4_file_class * cache_file = i4_open(r1_get_cache_file(),I4_READ | I4_NO_BUFFER);

	//does the cache file exist?
	if (!cache_file)
	{
		//need to build a cache file
		if (file_ids && file_ids->size())
		{
			build_cache_file(*file_ids, r1_get_decompressed_dir());
		}
	}
	else
	{
		//cache file exists, make sure its in the proper format
		//if anything doesnt match, gotta rebuild
		if (cache_file->size()<(w32)tex_cache_header_disk_size())
		{
			//This file certainly is not valid
			delete cache_file;
			i4_unlink(r1_get_cache_file());
			keep_cache_current(file_ids); //gota retry

			return;
		}
		//unfortunatelly, this doesn't work at all as the cache is written buffered and
		//and therefore tell() returns some bogus value.
		/*cache_file->seek(cache_file->tell()-4);
		   w32 cache_file_len=cache_file->read_32();
		   if (cache_file->size()!= cache_file_len)
		   	{//Same to this
		   	//Warning: These tests are not perfect! There's some possiblitiy
		   	//that these tests pass although the file is corrupt.
		   	delete cache_file;
		   	i4_unlink(r1_get_cache_file());
		   	keep_cache_current(file_ids);

		   	return;
		   	}*/
		cache_file->seek(0);
		tex_cache_header.read(cache_file);
		if (tex_cache_header.version==0)
		{
			delete cache_file;
			i4_unlink(r1_get_cache_file());
			keep_cache_current(file_ids); //gota retry
			return;
		}

		if (
			!palettes_are_same(&tex_cache_header.regular_format,&regular_format) ||
			!palettes_are_same(&tex_cache_header.chroma_format,&chroma_format)   ||
			!palettes_are_same(&tex_cache_header.alpha_format,&alpha_format)     ||
			(tex_cache_header.max_mip_dimention < (w32)MAX_USABLE_TEXTURE_DIMENTION) //hardcoded upper limit, instead of max_texture_dimention
		)
		{
			i4_warning("Could not find a matching Texture Cache for current Video Mode");
			//close the old cache file
			if (tex_cache_header.entries)
			{
				i4_free(tex_cache_header.entries);
			}

			delete cache_file;
			cache_file = 0;

			//build a new one
			if (file_ids && file_ids->size())
			{
				build_cache_file(*file_ids,
								 r1_get_decompressed_dir());
			}

		}
		else
		if (file_ids && file_ids->size())
		{
			//cache file was there (and in a compatible format) but we need to make sure it is up to date

			//yes. get directory information from the source compressed texture directory
			i4_directory_struct ds;
			//i4_get_directory(r1_get_compressed_dir(), dcomp, i4_T);

			//this directory contains everything we might want to consider being
			//a texture. to get the id's this should be ok.
			i4_get_directory("textures",ds,i4_T);

			i4_array<file_status_sort_struct> sorted_status_indices(3000,1000);
			//i4_array<file_status_sort_struct> g_compressed_indices(128,128);

			w32 i;

			for (i=0; i<(int)ds.tfiles; ++i)
			{
				file_status_sort_struct s;
				s.id = r1_get_texture_id(*(ds.files[i])); //Very important step:
				//As a sideeffect, the texture ids are registered for
				//backward-lookup.
				s.file_status_index = i;

				sorted_status_indices.add(s);
			}

			sorted_status_indices.sort(file_status_struct_compare);
			/*
			   for (i=0;i<(int)dcomp.tfiles;++i)
			   	{
			   	file_status_sort_struct g_comp;
			   	sw32 loc;
			   	g_comp.id=r1_get_file_id(*(dcomp.files[i]));
			   	g_comp.file_status_index=0;
			   	loc=sorted_status_indices.binary_search(&g_comp,file_status_struct_compare);
			   	if (loc==-1) //not found
			   		{
			   		i4_warning("Deleting file %s from g_compressed (original file no more exists).",*(dcomp.files[i]));
			   		i4_unlink(*(dcomp.files[i]));
			   		}
			   	else
			   		{//The entry actually is newer.
			   		ds.file_status[sorted_status_indices[loc].file_status_index].last_modified=dcomp.file_status[i].last_modified;
			   		}
			   	}
			 */
			i4_array<w32> ids_needing_update(128,128);

			//determine which textures need to be updated,
			//and also determine if there are any NEW ones
			//that need to be added to the cache

			for (i=0; i<ds.tfiles; ++i)
			//for (i=0;i<file_ids->size();++i)
			{
				//w32 id = r1_get_file_id(*(ds.files[i]));//when we use numbered files
				w32 id= r1_get_texture_id(*(ds.files[i])); //to use named files
				//(*file_ids)[i] (neu);
				//r1_get_file_id(*ds.files[i]);(alt)
				//Hm, ich denke, die alte Version war besser.
				tex_cache_entry_t * t = find_id_in_tex_cache(tex_cache_header.entries,
															 tex_cache_header.num_entries,id);

				if (!t || (t->lowmipoffset==0xFFFFFFFF))
				{
					if (t && t->lowmipoffset==0xFFFFFFFF)
					{
						//its not in the cache right now because the file didnt exist before
						//if it exists now, add it to the update list. if not, well dont even try
						//(this avoids creating an "update textures" wait everytime if a bunch of texture files
						// just dont exist)
						file_status_sort_struct s;
						s.id = id;

						sw32 location = sorted_status_indices.binary_search(&s,file_status_struct_compare);
						if (location != -1)
						{
							//yeah, its in the directory, add it to the update list
							ids_needing_update.add(id);
						}
					}
					else
					{
						//its not in the cache. add it
						ids_needing_update.add(id);
					}
				}
				else
				{
					//it is in the cache. make sure its current. if the file does not exist, dont
					//add it

					file_status_sort_struct s;
					s.id = id;

					sw32 location = sorted_status_indices.binary_search(&s,file_status_struct_compare);

					//it exists in the directory. if the directory file is more recent than ours, update it.
					//if it doesnt exist, well then obviously dont update it.
					if (location != -1)
					{
						if (ds.file_status[sorted_status_indices[location].file_status_index].last_modified > t->last_modified)
						{
							//must update this texture
							ids_needing_update.add(id);
						}
					}
				}
			}


			if (tex_cache_header.entries)
			{
				i4_free(tex_cache_header.entries);
			}

			delete cache_file;
			cache_file = 0;

			//does anything need updating?
			if (ids_needing_update.size())
			{
				//yes. delete this old data and re-read it after we've updated the cache file
				update_cache_file(ids_needing_update,
								  r1_get_decompressed_dir());
			}
		}
		else
		{
			if (tex_cache_header.entries)
			{
				i4_free(tex_cache_header.entries);
			}

			delete cache_file;
			cache_file = 0;
		}
	}
	reset_decompressed_cache();
	if (cache_file)
	{
		i4_warning("unhandled case: keep_cache_current");
	}
}
//////////////////////////////////
//////////////////////////////////
//////////////////////////////////
//#include "tex_heap.cc"
//////////////////////////////////
//////////////////////////////////
//////////////////////////////////
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "render/tex_heap.h"
#include "render/tex_no_heap.h"

i4_profile_class pf_tex_heap_alloc("tex_heap::alloc()");
i4_profile_class pf_tex_heap_free("tex_heap::free()");
i4_profile_class pf_tex_heap_cleanup("tex_heap::cleanup()");

i4_profile_class pf_tex_no_heap_alloc("tex_heap::alloc()");
i4_profile_class pf_tex_no_heap_free("tex_heap::free()");
i4_profile_class pf_tex_no_heap_cleanup("tex_heap::cleanup()");


r1_texture_heap_class::~r1_texture_heap_class()
{
	//kill the free list
	r1_tex_heap_free_node * next_free,* last_free;

	next_free = first_free;
	while (next_free)
	{
		last_free = next_free;
		next_free = next_free->next;

		if (last_free->mip)
		{
			last_free->mip->vram_handle = 0;
		}
		free_node_alloc->free(last_free);
	}
	first_free = 0;

	//kill the used list
	r1_tex_heap_used_node * next_used,* last_used;

	next_used = first_used;
	while (next_used)
	{
		last_used = next_used;
		next_used = next_used->next;

		if (last_used->node->mip->vram_handle)
		{
			last_used->node->mip->vram_handle = 0;
		}
		free_node_alloc->free(last_used->node);
		used_node_alloc->free(last_used);
	}
	first_used = 0;

	oldest_used = 0;
	//both lists are cleared. kill the allocaters

	delete free_node_alloc;
	free_node_alloc=0;

	delete used_node_alloc;
	used_node_alloc=0;
}

r1_texture_heap_class::r1_texture_heap_class(w32 heap_size, w32 heap_start, w32 free_node_size, w32 used_node_size, sw32 * frame_count)
{
	num_ram_misses  = 0;
	needs_cleanup   = i4_F;
	max_fail_size=0;
	free_node_alloc = 0;
	used_node_alloc = 0;

	frame_count_ptr = frame_count;

	if (!free_node_alloc)
	{
		free_node_alloc   = new i4_linear_allocator(free_node_size, 2048, 1024, "texture heap free nodes");
		first_free        = (r1_tex_heap_free_node *)free_node_alloc->alloc();
		first_free->size  = heap_size;
		first_free->start = heap_start;
		first_free->next  = 0;
		first_free->mip   = 0;
	}

	if (!used_node_alloc)
	{
		used_node_alloc  = new i4_linear_allocator(used_node_size, 2048, 1024, "texture heap used nodes");
		first_used       = 0;
		oldest_used      = 0;
	}
}


void r1_texture_heap_class::free_really_old()
{
	pf_tex_heap_cleanup.start();

	w32 total_freed = 0;

	r1_tex_heap_used_node * u = oldest_used;
	r1_tex_heap_used_node * last;

	while (u && total_freed < (w32)max_fail_size)
	{
		sw32 lfu = u->node->mip->last_frame_used;
		sw32 age = *frame_count_ptr - lfu;

		last = u->last;

		if ((lfu != -1) && (age > 10))
		{
			total_freed += u->node->size;

			free(u);
		}

		u = last;
	}

//   if (total_freed < max_fail_size)
//     i4_warning("Texture cleanup: freed %d bytes (not enough)",total_freed);
//   else
//     i4_warning("Texture cleanup: freed %d bytes (successful)",total_freed);

	max_fail_size = 0;
	needs_cleanup = i4_F;

	pf_tex_heap_cleanup.stop();
}



r1_tex_heap_used_node * r1_texture_heap_class::alloc(w32 need_size, w8 flags)
{
	pf_tex_heap_alloc.start();

	//ok. we know how much memory we need, now try to find a free area of memory
	r1_tex_heap_free_node * f    = first_free;
	r1_tex_heap_free_node * last = 0;

	while (f && f->size<need_size)
	{
		last=f;
		f=f->next;
	}

	r1_tex_heap_used_node * new_used = 0;

	if (!f)
	{
		//no memory large enough. dig through and see if there is a used chunk that we can free
		r1_tex_heap_used_node * u = oldest_used;

		while (u)
		{
			if (u->node->size >= need_size)
			{
				r1_miplevel_t * m = u->node->mip;
				if (m->last_frame_used!=-1 && m->last_frame_used < (*frame_count_ptr - 2))
				{
					if (m->flags & R1_MIPLEVEL_IS_LOADING)
					{
						i4_warning("loading miplevel is in used list. bad.");
					}
					break;
				}
			}
			u = u->last;
		}

		//couldnt find a used one that we could free
		if (!u)
		{
			++num_ram_misses;

			if (need_size > (w32)max_fail_size)
			{
				max_fail_size = need_size;
			}

			if ((num_ram_misses & 15) == 0)
			{
				//cleanup old textures every 16 misses
				needs_cleanup = i4_T;
			}

			pf_tex_heap_alloc.stop();
			return 0;
		}

		new_used = u;

		f = new_used->node;

		//kill the old mip's reference to it
		f->mip->vram_handle = 0;

		if (f->size > need_size)
		{
			//need to merge unused space back into free list

			merge_into_free_list(f->start + need_size, f->size - need_size);

			f->size = need_size;
		}

		if (flags & R1_TEX_HEAP_DONT_LIST)
		{
			if (new_used==first_used)
			{
				first_used = new_used->next;
			}
			if (new_used==oldest_used)
			{
				oldest_used = new_used->last;
			}

			//this flag is set for asynchronously loaded textures
			//we dont even want this node to be listed. it will be added
			//when the async load is finished
			if (new_used->next)
			{
				new_used->next->last = new_used->last;
			}
			if (new_used->last)
			{
				new_used->last->next = new_used->next;
			}

			new_used->next=0;
			new_used->last=0;
		}
	}
	else
	{
		// remove the this node from the free list
		if (!last)
		{
			first_free = f->next;
		}
		else
		{
			last->next = f->next;
		}

		//if we dont match the size exactly, need to create a new free node to fill in the space
		if (f->size != need_size)
		{
			r1_tex_heap_free_node * new_free = (r1_tex_heap_free_node *)free_node_alloc->alloc();

			if (!new_free)
			{
				i4_warning("alloc of tex_heap free_node failed");
				pf_tex_heap_alloc.stop();
				return 0;
			}

			new_free->size  = f->size  - need_size;
			new_free->start = f->start + need_size;
			new_free->mip=0;

			if (last)
			{
				last->next = new_free;
			}
			else
			{
				first_free = new_free;
			}


			new_free->next = f->next;
			f->size = need_size;
		}

		new_used = (r1_tex_heap_used_node *)used_node_alloc->alloc();

		//f is the node we'll use
		//add into the used_list

		if (!new_used)
		{
			i4_warning("alloc of tex_heap used_node failed");
			pf_tex_heap_alloc.stop();
			return 0;
		}

		new_used->node = f;

		if (flags & R1_TEX_HEAP_DONT_LIST)
		{
			new_used->next=0;
			new_used->last=0;
		}
		else
		{
			new_used->next = first_used;
			new_used->last = 0;

			if (first_used)
			{
				first_used->last = new_used;
			}

			first_used = new_used;
		}
	}

	pf_tex_heap_alloc.stop();
	return new_used;
}

void r1_texture_heap_class::merge_into_free_list(w32 _start, w32 _size)
{
	r1_tex_heap_free_node * next, * last;

	// find proper spot to add into free list
	next = first_free;
	last=0;
	for (; next && next->start < _start;)
	{
		last = next;
		next = next->next;
	}

	// can we combine with the last node?
	if (last && last->start+last->size == _start)
	{
		last->size += _size;

		// does this also combine with the next node?
		if (next && (last->start + last->size == next->start) )
		{
			last->size += next->size;
			last->next =  next->next;

			free_node_alloc->free(next);
		}
	}
	else
	if (next && (_start + _size == next->start) )  // does it combine with the next block?
	{
		next->start = _start;
		next->size += _size;
	}
	else
	if (last)   // put after the last proper node
	{
		r1_tex_heap_free_node * f = (r1_tex_heap_free_node *)free_node_alloc->alloc();

		f->size  = _size;
		f->start = _start;
		f->next  = last->next; //(last->next should be NULL)
		f->mip   = 0;

		last->next = f;
	}
	else   // put it at the begining of the list
	{
		r1_tex_heap_free_node * f = (r1_tex_heap_free_node *)free_node_alloc->alloc();

		f->size  = _size;
		f->start = _start;
		f->next  = first_free;
		f->mip   = 0;
		first_free = f;
	}
}

void r1_texture_heap_class::update_usage(r1_tex_heap_used_node * u)
{
	if (u->node->mip->last_frame_used==-1)
	{
		return ;
	}


	u->node->mip->last_frame_used = *frame_count_ptr;

	if (u != first_used)
	{
		//put this node at the front of the used list
		//has the effect of sorting all used nodes according to
		//their age (most recently used are at the
		//front of the list)

		if (u==oldest_used)
		{
			oldest_used = u->last;
		}

		if (u->next)
		{
			u->next->last = u->last;
		}
		if (u->last)
		{
			u->last->next = u->next;
		}

		u->last = 0;
		u->next = first_used;

		if (first_used)
		{
			first_used->last = u;
		}

		first_used = u;
	}

	//update the oldest used pointer
	if (!oldest_used)
	{
		oldest_used = u;
	}
	else
	{
		if ((u->node->mip->last_frame_used != -1) &&
			(u->node->mip->last_frame_used < oldest_used->node->mip->last_frame_used))
		{
			oldest_used = u;
		}
	}
}

void r1_texture_heap_class::free(r1_tex_heap_used_node * u)
{
	if (!u || !u->node)
	{
		i4_warning("tex_heap_class free : handle is 0");
		return;
	}

	if (u->node->mip->flags & R1_MIPLEVEL_IS_LOADING)
	{
		i4_warning("free called on a mip that was still loading");
	}

	pf_tex_heap_free.start();

	r1_tex_heap_free_node * f = u->node, * next, * last;

	//give its r1_miplevel_t an invalid vram handle
	f->mip->vram_handle = 0;

	//process these cases 1st
	if (u==first_used)
	{
		first_used = u->next;
	}

	if (u==oldest_used)
	{
		oldest_used = u->last;
	}

	//pull it from the used_list
	if (u->next)
	{
		u->next->last = u->last;
	}
	if (u->last)
	{
		u->last->next = u->next;
	}

	used_node_alloc->free(u);
	//end of pulling from the used list

	// find proper spot to add into free list
	next = first_free;
	last=0;
	for (; next && next->start<f->start;)
	{
		last=next;
		next=next->next;
	}

	// can we combine with the last node?
	if (last && last->start+last->size==f->start)
	{
		last->size += f->size;

		// does this also combine with the next node?
		if (next && (last->start + last->size == next->start) )
		{
			last->size += next->size;
			last->next =  next->next;

			free_node_alloc->free(next);
		}

		free_node_alloc->free(f);
	}
	else
	if (next && (f->start + f->size == next->start) )  // does it combine with the next block?
	{
		next->start = f->start;
		next->size += f->size;

		free_node_alloc->free(f);
	}
	else
	if (last)                // put after the last proper node
	{
		f->next = last->next;
		last->next = f;
	}
	else                         // put it at the begining of the list
	{
		f->next = first_free;
		first_free = f;
	}

	pf_tex_heap_free.stop();
}

//////////////////////////////////
//////////////////////////////////
//////////////////////////////////

void r1_texture_manager_class::keep_resident(const i4_const_str &tname, sw32 desired_width)
{
	w32 id = r1_get_texture_id(tname);
	sw16 handle = find_texture(id);

	if (!handle)
	{
		return;
	}

	r1_texture_entry_struct * t = &(*entries)[handle];

	r1_mip_load_info load_info;
	load_info.flags=default_texture_flags;

	//load all levels <= desired_width
	load_info.src_file = 0;

	sw32 j;
	for (j=0; j<R1_MAX_MIP_LEVELS; ++j)
	{
		r1_miplevel_t * mip = t->mipmaps[j];

		if (mip && (mip->width <= desired_width))
		{
			mip->last_frame_used = -1;

			if ((mip->vram_handle==0) && (!(mip->flags & R1_MIPLEVEL_IS_LOADING)))
			{
				load_info.dest_mip = mip;
				i4_os_string(tname,load_info.texture_name,sizeof(load_info.texture_name));
				if (!immediate_mip_load(&load_info))
				{
					//these absolutely must work
					i4_error("FATAL: Keep_resident: could not load previously existing texture, cannot continue.");
				}
			}
		}
	}
}

//int r1_texture_manager_class::async_mip_prepare_load(w32 tex_by, async_callback call)
//	{

//Would be great if some parts of async_mip_load could be handled
//at one place, but data types of the different nodes differ.
//	}

i4_bool r1_texture_manager_class::load_textures()
{
	sw32 i,j;
	static w8 active=i4_F; //used as prevention for recursion

	if (textures_loaded||entries||active)
	{
		//The textures flag has no assured semantics at all
		return i4_F;
	}
	active=i4_T;

	//Register the NULL texture.
	null_texture_handle=register_texture("null", "NULL texture");
	//these GLOBAL textures are only registered with the main texture manager
	//Otherwise, every instance of the tman has different handles for these
	//and in global context, the textures get screwed up.
	if (this->is_master_tman)
	{
		r1_texture_ref * p=r1_texture_ref::first;
		for (; p; p=p->next)
		{
			p->texture_handle=register_texture(p->name, "code reference");
		}
	}

	//li_call("update_all_textures");//Be shure compressed dir is up to date
	//Warning: This can call ourselves recursivelly! (Through stat->update())

	//create an array of texture id's from names[] and sort it
	i4_array<w32> texture_file_ids(6000,6000); //We currently start out with 2000!
	i4_directory_struct texts;
	i4_status_class * stat = 0;

	//Usually the following line is really helpfull for the user.
	//But we must implement a way to suppress the stat window if we are
	//already inside the draw-recursion, or we crash the window-manager

	if (!g1_render.main_draw)
	{
		stat=i4_create_status(i4gets("checking_times"));
	}
	i4_get_directory("textures",texts,i4_F,stat);
	//for (i=0; i<registered_tnames.size(); ++i)
	//  texture_file_ids.add(registered_tnames[i].id);
	for (int kk=0; kk<(int)texts.tfiles; ++kk)
	{
		// this line is dumb. Want's to get ids from id-filenames.
		// we need id's from textural names.
		//texture_file_ids.add(r1_get_file_id(*(texts.files[kk])));

		//Very important step:
		//As a sideeffect, the texture ids are registered for
		//backward-lookup.
		i4_const_str tstr(* (texts.files[kk]));
		w32 thisid=r1_get_texture_id(tstr);
		texture_file_ids.add(thisid);
	}
	delete stat;
	stat=0;
	//sort the list
	texture_file_ids.sort(w32_compare);

	reset_decompressed_cache();
	//update / build / rebuild the cache file
	//for this, we need the ids of ALL textures.
	keep_cache_current(&texture_file_ids);
	//but for loading, we need only the actually used ones.
	texture_file_ids.clear();
	for (i=0; i<registered_tnames.size(); ++i)
	{
		texture_file_ids.add(registered_tnames[i].id);
	}


	texture_file_ids.sort(w32_compare);

	w32 tocompare=0;

	//remove duplicate entries (don't wanna load a texture twice)
	//doesn't work this way, many textures will be missing.
	/*
	   for (i=0; i<texture_file_ids.size();)
	   	{
	   	if (texture_file_ids[i]==tocompare)
	   		{
	   		texture_file_ids.remove(i);
	   		}
	   	else
	   		{
	   		tocompare=texture_file_ids[i];
	 ++i;
	   		}
	   	}
	 */
	i4_file_class * cache_file = i4_open(r1_get_cache_file(), I4_READ | I4_NO_BUFFER);

	tex_cache_header_t tex_cache_header;

	if (cache_file)
	{
		//read in the header for further processing
		tex_cache_header.read(cache_file);
	}
	else
	{
		i4_warning("Couldn't locate texture cache file.");
		tex_cache_header.num_entries = 0;
		tex_cache_header.entries     = 0;
	}


	//OK FINALLY. process this crap. load information for all
	//requested textures (their ids are currently in texture_file_ids)

	//dynamic array of loaded texture entries
	i4_array<r1_texture_entry_struct> new_texture_entries(128,128);

	//i4_status_class *stat = 0;

	//Usually the following line is really helpfull for the user.
	//But we must implement a way to suppress the stat window if we are
	//already inside the draw-recursion, or we crash the window-manager

	if (!g1_render.main_draw)
	{
		stat=i4_create_status(i4gets("loading_textures"));
	}


	for (i=0; i<texture_file_ids.size(); ++i)
	{
		w32 id = texture_file_ids[i];

		tex_cache_entry_t * t = find_id_in_tex_cache(tex_cache_header.entries,
													 tex_cache_header.num_entries,id);

//#ifdef __linux
//	i4_warning("Attempting to load low mip level of texture %i of %i.",i,texture_file_ids.size());
//#endif
		if (!t || (t->lowmipoffset==0xFFFFFFFF))
		{
			int found=0,en=0;
			for (; en<memory_images.size(); ++en)
			{
				if (memory_images[en].id==id)
				{
					break;
				}
			}
			//why so complicated, you ask? Because we want to use continue afterwards.
			if (en<memory_images.size())
			{
				//we need to restore a memory_image
				//fortunatelly, we saved them ;-)
				r1_image_list_struct * pres=&memory_images[en];
				i4_image_class * texture_image=pres->image;
				w32 textby=2;
				if (texture_image->pal->source.pixel_depth==I4_24BIT)
				{
					textby=3;
				}
				if (texture_image->pal->source.pixel_depth==I4_32BIT)
				{
					textby=4;
				}
				i4_ram_file_class * fake_file = new i4_ram_file_class(texture_image->data,
																	  texture_image->width() *
																	  texture_image->height() * textby);

				r1_texture_entry_struct * new_entry=new_texture_entries.add();
				memset(new_entry,0,sizeof(r1_texture_entry_struct));
				new_entry->id            =pres->id;

				new_entry->average_color = pres->average_color;

				new_entry->mipmaps[0]    = new r1_miplevel_t;
				r1_miplevel_t * mip = new_entry->mipmaps[0];

				mip->level  = 0;
				mip->width  = texture_image->width();
				mip->height = texture_image->height();

				mip->last_frame_used = -1;
				mip->vram_handle     = 0;
				mip->entry=new_entry; //install circular reference
				mip->flags = 0;

				r1_mip_load_info load_info;
				load_info.flags=r1_pal_to_flags(texture_image->get_pal(),default_texture_flags);
				load_info.dest_mip = mip;
				load_info.src_file = fake_file;
				load_info.texture_name[0]=0;
				if (!immediate_mip_load(&load_info))
				{
					i4_error("INTERNAL: Couldn't reload memory texture.");
					continue;
				}

				delete fake_file;
				if (stat)
				{
					stat->update((float)(i+1) / (float)texture_file_ids.size());
				}
				continue;
			}
			for (int k=0; k<registered_tnames.size(); ++k)
			{
				if (registered_tnames[k].id==id)
				{
					i4_warning("Texture: %s not found in texture cache, run maxtool 'Update textures'. Encoded name %x", registered_tnames[k].name, registered_tnames[k].id);
					found=1;
				}
			}
			if (!found)
			{
				i4_const_str * n=r1_get_texture_name(id);
				if (n)
				{
					char buf[300];
					i4_os_string(*n,buf,300);
					delete n;
					i4_warning("Texture loader: Texture %s (%x) is not requested but needed.",buf,id);
				}
				else
				{
					i4_warning("Texture loader: Request to load unknown texture %x ignored.",id);
				}
			}
		}
		else
		{
			//texture is in the cache. need to load up some info, load the lowest mip, etc
			//add it to the list of valid textures
			r1_texture_entry_struct * new_entry = new_texture_entries.add();

			//the last one should always be null, hence the R1_MAX_MIP_LEVELS+1
			memset(new_entry->mipmaps,0,sizeof(r1_miplevel_t *) * (R1_MAX_MIP_LEVELS+1));

			new_entry->flags         = t->flags;
			new_entry->id            = t->id;
			new_entry->average_color = t->average_color;

			generate_mip_offsets(t->base_width,t->base_height,t->num_mip_levels,
								 (sw32 *)new_entry->file_offsets,
								 new_entry->flags & R1_MIP_IS_ALPHATEXTURE ? 4 : 3);

			//fill in this structure. information on mip levels
			for (j=0; j<t->num_mip_levels; ++j)
			{
				new_entry->mipmaps[j] = new r1_miplevel_t;

				r1_miplevel_t * mip = new_entry->mipmaps[j];

				mip->level  = (w8)j;
				mip->width  = t->base_width /(1<<j);
				mip->height = t->base_height/(1<<j);
				mip->entry  = new_entry;

				mip->flags = 0;
			}

			//seek to the low mip offset (stored IN the cache file)
			if (default_texture_flags & R1_MIPFLAGS_USE16)
			{
				cache_file->seek(t->lowmipoffset+2*sizeof(sw32));
			}
			else
			{
				cache_file->seek(t->ext_lowmipoffset+2*sizeof(sw32));
			}

			r1_mip_load_info load_info;
			load_info.flags=default_texture_flags;

			//the dst_mip is the very last one
			load_info.src_file = cache_file;
			load_info.texture_name[0]=0; //We don't need this one for lowmips
			load_info.dest_mip = new_entry->mipmaps[t->num_mip_levels-1];

			//dont want these to ever be thrown out of texture memory
			load_info.dest_mip->last_frame_used   = -1;
			/*
			   if ((default_texture_flags & R1_MIPFLAGS_USE16)==0)
			   	{
			   	//no 16 bit available, don't try to do anything with the cache.
			   	load_info.src_file=0;
			   	for (int k2=0; k2<registered_tnames.size(); ++k2)
			   	  if (registered_tnames[k2].id==id)
			   		  {
			   		  i4_filename_struct fns;
			   		  i4_split_path(registered_tnames[k2].name,fns);
			   		  strcpy(load_info.texture_name,fns.filename);
			   		  }
			   	  //SUGGESTION: Do not load the lowest miplevel, but something
			   	  //better. Since this might be VERY slow.
			   	}

			 */
#ifdef TEXTURE_LOAD_DEBUG
			for (int k3=0; k3<registered_tnames.size(); ++k3)
			{
				if (registered_tnames[k3].id==id)
				{
					i4_warning("Attempting to load texture %s. Code 0x%x.",registered_tnames[k3].name,id);
				}
			}
#endif
			//load that low mip level
			if (!immediate_mip_load(&load_info))
			{
				//check the error field in load_info
				i4_error("FATAL: Could not load lowest miplevel of a texture, cannot continue");
			}
			/* //Code bellow would preload all textures to main memory
			   //but is actually unusable as takes way to much time
			   //and overloads the virtual memory manager.
			   //We should have a way to know WHICH textures should be preloaded for best performance
			   //I suggest preloading all world-textures
			   load_info.src_file=NULL;
			   load_info.dest_mip=new_entry->mipmaps[0];
			   load_info.dest_mip->last_frame_used=-1;
			   i4_const_str *s=r1_get_texture_name(new_entry->id);
			   i4_os_string(*s,load_info.texture_name,
			   	sizeof(load_info.texture_name));
			   delete s;
			   load_info.error=0;
			   if (!immediate_mip_load(&load_info))//Ignore any failures, will be corrected run-time
			   	i4_warning("Could not preload texture %s",load_info.texture_name);
			 */

		}

		if (stat)
		{
			stat->update((float)(i+1) / (float)texture_file_ids.size());
		}
	}

	if (stat)
	{
		delete stat;
	}

	if (cache_file)
	{
		delete cache_file;
	}

	if (tex_cache_header.entries)
	{
		i4_free(tex_cache_header.entries);
	}
	else
	{
		//there were no texture cache entries? just get rid of the file.
		i4_unlink(r1_get_cache_file());
	}


	//theres 1 additional texture, which will be entries[0], the default texture
	total_textures = new_texture_entries.size() + 1;
	//total_textures=texture_file_ids.size()+1;
	if (texture_file_ids.size()!=new_texture_entries.size())
	{
		i4_warning("WARNING: Some textures didn't load. Scheme and Level files inconsistent?");
	}
#ifdef TEXTURE_LOAD_DEBUG
	else
	{
		i4_warning("Texture loader: All textures have been loaded.");
	}
#endif

	entries = new i4_array<r1_texture_entry_struct>(total_textures+1,8);

	r1_texture_entry_struct blank_entry;
	memset(&blank_entry,0,sizeof(r1_texture_entry_struct));
	blank_entry.average_color = 0x00FFFFFF;

	entries->add(blank_entry);

	//should be sorted already but go ahead, sort again just in case
	if (new_texture_entries.size())
	{
		new_texture_entries.sort(entry_compare);
	}

	for (i=0; i<total_textures-1; ++i)
	{
		entries->add();
		memset(&(*entries)[i+1],0,sizeof(r1_texture_entry_struct));

		(*entries)[i+1] = new_texture_entries[i];

		//crap. have to update the entry pointers since the mip's ->entry
		//references are in new_texture_entries (instead of entries[])
		for (j=0; j<R1_MAX_MIP_LEVELS; ++j)
		{
			if ((*entries)[i+1].mipmaps[j])
			{
				(*entries)[i+1].mipmaps[j]->entry = &(*entries)[i+1];
			}
		}
	}

	matchup_textures();

	/*
	   i4_array<r1_texture_animation_entry_struct> anim_a(128,128);
	   for (i=0; i<names.size(); ++i)
	   {
	   w32 id=r1_get_texture_id(*names[i]);
	   i4_str *fn=r1_animation_id_to_filename(id);
	   i4_file_class *fp=opener(*fn);
	   if (fp)
	   {
	   	r1_texture_animation_entry_struct *a=anim_a.add();
	   	a->id=fp->read_32();
	   	a->total_frames=fp->read_16();
	   	a->frames=(r1_texture_handle *)i4_malloc(sizeof(r1_texture_handle)*a->total_frames,
	   											  "animation frames");
	   	for (j=0; j<a->total_frames; ++j)
	   	{
	   	  w32 id=fp->read_32();
	   	  a->frames[j]=find_texture(id);
	   	}
	   }

	   delete fn;
	   }

	   total_tanims=anim_a.size();
	   if (total_tanims)
	   {
	   tanims=(r1_texture_animation_entry_struct *)i4_malloc(sizeof(r1_texture_animation_entry_struct)
	 * total_tanims, "animations");
	   for (i=0; i<anim_a.size(); ++i)
	   	tanims[i]=anim_a[i];
	   }
	   else
	   tanims=0;

	 */
	/*for (int v=0;v<=new_texture_entries.size();++v)
	   	{
	   	for (int mips=0;mips<=R1_MAX_MIP_LEVELS;++mips)
	   		{
	   		if (new_texture_entries[v]->mipmaps[mips])
	   			delete new_texture_entries[v]->mipmaps[mips];
	   		}
	   	}*/
	//textures_loaded=i4_T;
	active=i4_F;
	return i4_T;
}

w32 r1_pal_to_flags(const i4_pal * pal,w32 allowed_formats)
{
	w32 retval=0,retval2;

	allowed_formats &= R1_MIPFLAGS_USEFLAGS;
	switch (pal->source.pixel_depth)
	{
		case I4_8BIT:
			retval= R1_MIPFLAGS_FORCE8; //Cannot do anything with this...
			break;
		case I4_16BIT:
			retval= R1_MIPFLAGS_FORCE16;
			break;
		case I4_24BIT:
			retval= R1_MIPFLAGS_FORCE24;
			break;
		case I4_32BIT:
			retval= R1_MIPFLAGS_FORCE32;
			break;
		default:
			retval= R1_MIPFLAGS_FORCE16;
	}
	if (((retval>>4) & allowed_formats)==0) //uh oh. We are forced to use a format
	//that is not supported...
	//This can only happen if copying from a generated image to a texture
	//I.e the lod textures if the desktop bit depth is 24 and 24 is not available
	//as texture format.
	{
		retval2=retval<<4; //now this is a SRCxx constant
		if (allowed_formats&R1_MIPFLAGS_USE32)
		{
			retval2|=R1_MIPFLAGS_FORCE32;
		}
		else
		if (allowed_formats&R1_MIPFLAGS_USE24)
		{
			retval2|=R1_MIPFLAGS_FORCE24;
		}
		else
		if (allowed_formats&R1_MIPFLAGS_USE16)
		{
			retval2|=R1_MIPFLAGS_FORCE16;
		}
		else
		{
			retval2|=R1_MIPFLAGS_FORCE8;
		}
		return retval2;

	}
	return retval;
}

r1_texture_handle r1_texture_manager_class::register_image(i4_image_class * image)
{
	if (!entries || !entries->size())
	{
		load_textures();
	}

	//check to make sure image is power of 2
	sw32 i,j,new_width,new_height;

	for (i=1; i < image->width();  i = i<<1)
	{
		;
	}
	new_width  = i;

	for (i=1; i < image->height(); i = i<<1)
	{
		;
	}
	new_height = i;


	r1_texture_handle return_handle = 0;

	i4_draw_context_class context(0,0,image->width()-1,image->height()-1);


	r1_texture_entry_struct new_entry;
	memset(&new_entry,0,sizeof(r1_texture_entry_struct));


	/* const i4_pal *put_pal=0;
	   put_pal=i4_pal_man.register_pal(&regular_format);
	   if (image->pal->source.alpha_bits)
	   {
	   put_pal=i4_pal_man.register_pal(&alpha_format);
	   new_entry.flags|=R1_MIP_IS_ALPHATEXTURE;
	   }
	   if (this->default_texture_flags&R1_MIPFLAGS_USE24)
	   {
	   	 put_pal=i4_pal_man.register_pal(&reg24_format);
	   	 if (image->pal->source.alpha_bits)
	   	 {
	   		 put_pal=i4_pal_man.register_pal(&alpha32_format);
	   		 new_entry.flags|=R1_MIP_IS_ALPHATEXTURE;
	   	 }
	   }
	   if (this->default_texture_flags&R1_MIPFLAGS_USE32)
	   {
	   	 put_pal=i4_pal_man.register_pal(&reg32_format);
	   	 if (image->pal->source.alpha_bits)
	   	 {
	   		 put_pal=i4_pal_man.register_pal(&alpha32_format);
	   		 new_entry.flags|=R1_MIP_IS_ALPHATEXTURE;
	   	 }
	   }*/


	i4_image_class * temp_image    = i4_create_image(image->width(),image->height(), image->pal);
	i4_image_class * texture_image = temp_image;

	image->put_image(temp_image,0,0,context);

	if (new_width != temp_image->width() || new_height != temp_image->height())
	{
		texture_image = i4_create_image(new_width,new_height, image->pal);

		sw32 old_width  = temp_image->width();
		sw32 old_height = temp_image->height();

		w16 * old_tex = (w16 *)temp_image->data;
		w16 * new_tex = (w16 *)texture_image->data;
		w16 * dst     = new_tex;

		float width_ratio  = (float)old_width  / (float)new_width;
		float height_ratio = (float)old_height / (float)new_height;

		//now scale the old to fit the new

		for (j=0; j<new_height; ++j)
		{
			for (i=0; i<new_width;  ++i, ++dst)
			{
				*dst = old_tex[(sw32)((double)j * height_ratio)*old_width + (sw32)((double)i * width_ratio)];
			}
		}

		delete temp_image;
	}
	temp_image=0; //do not use beyond this point.
	r1_image_list_struct * pres=memory_images.add(); //preserve texture image data
	//we need it for reloading textures
	pres->image=texture_image;
	w32 tex_by=2;
	if (texture_image->pal->source.pixel_depth==I4_24BIT)
	{
		tex_by=3;
	}
	else
	{
		if (texture_image->pal->source.pixel_depth==I4_32BIT)
		{
			tex_by=4;
		}
	}

	i4_ram_file_class * fake_file = new i4_ram_file_class(texture_image->data,
														  texture_image->width() *
														  texture_image->height() * tex_by);


	new_entry.id            = (*entries)[entries->size()-1].id + 1; //maintains the sorted order of the array
	pres->id=new_entry.id;

	w32 rt=0,gt=0,bt=0, t=0;
	sw32 width  = texture_image->width();
	sw32 height = texture_image->height();
	//i4_bool has_chroma=i4_F,has_alpha=i4_F;//although alpha will be used anyways
	//well, unusable: We don't need to declare alpha AND chroma since
	//using alpha can also handle chroma effects.

	if (texture_image->get_pal()->source.pixel_depth==I4_32BIT)
	{
		w32 * base_mip = (w32 *)texture_image->data;

		sw32 im_size = width * height;

		for (i=0; i<im_size; ++i, ++base_mip)
		{
			w32 c = *base_mip;

			if (c & 0xFF000000)
			{
				continue;
			}

			if (c==G1_CHROMA_COLOR)
			{
				continue;
			}
			else
			{
				rt += ((c>>16) & 0xFF);
				gt += ((c>>8)  & 0xFF);
				bt += ((c>>0)  & 0xFF);
				++t;
			}
		}
	}
	else
	{
		i4_draw_context_class context(0,0,width-1,height-1);

		for (int y=0; y<height; ++y)
		{
			for (int x=0; x<width; ++x)
			{
				i4_color c=texture_image->get_pixel(x,y, context);

				if (c & 0xFF000000)
				{
					continue;
				}

				if (c==G1_CHROMA_COLOR)
				{
					continue;
				}
				else
				{
					rt += ((c>>16) & 0xFF);
					gt += ((c>>8)  & 0xFF);
					bt += ((c>>0)  & 0xFF);
					++t;
				}
			}
		}
	}
	if (t>0)
	{
		new_entry.average_color = ((rt/t)<<16) | ((gt/t)<<8) | (bt/t);
	}

	pres->average_color=new_entry.average_color;
	new_entry.mipmaps[0]    = new r1_miplevel_t;

	r1_miplevel_t * mip = new_entry.mipmaps[0];

	mip->level  = 0;
	mip->width  = texture_image->width();
	mip->height = texture_image->height();

	mip->last_frame_used = -1;
	mip->vram_handle     = 0;
	mip->flags = 0;

	entries->add(new_entry);
	++total_textures;

	//update entry pointers for all miplevels
	for (i=0; i<total_textures-1; ++i)
	{
		//references are in new_texture_entries (instead of entries[])
		for (j=0; j<R1_MAX_MIP_LEVELS; ++j)
		{
			if ((*entries)[i+1].mipmaps[j])
			{
				(*entries)[i+1].mipmaps[j]->entry = &(*entries)[i+1];
			}
		}
	}

	r1_mip_load_info load_info;
	load_info.flags=r1_pal_to_flags(texture_image->get_pal(),default_texture_flags);
	load_info.dest_mip = mip;
	load_info.src_file = fake_file;
	load_info.texture_name[0]=0;
	if (!immediate_mip_load(&load_info))
	{
		i4_warning("tmanager:: register_image failed.");
	}
	else
	{
		return_handle = total_textures-1;
	}

	delete fake_file;
	//delete texture_image;  //will be preserved until uninit()

	char name[256];
	sprintf(name, "memory_image_%d", return_handle);
	r1_texture_handle han=register_texture(name, name);
	registered_tnames[han].handle=return_handle; //for normal textures, this
	//remaping is done on matchup_textures(), here we need to do it manually
	//as the textures are already loaded.
	registered_tnames[han].id=new_entry.id; //hack to get the id correct.
	//I would be very happy if somebody could find an inverse for
	//i4_str_checksum(). ;-)
	//For Beginners: It seems that this would require
	//calculation of discrete logarithms. Being able to do this in linear time
	//would prove P=NP - you would have the Touring award for shure.

	return han;
}
