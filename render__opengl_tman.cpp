/**********************************************************************

   Golgotha Forever - A portable, free 3D strategy and FPS game.
   Copyright (C) 1999 Golgotha Forever Developers

   Sources contained in this distribution were derived from
   Crack Dot Com's public release of Golgotha which can be
   found here:  http://www.crack.com

   All changes and new works are licensed under the GPL:

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   For the full license, see COPYING.

 ***********************************************************************/
#include "pch.h"
#include <stdio.h>
#include "file/ram_file.h"
#include "image/image.h"
#include "time/profile.h"
#include "loaders/load.h"
#include "file/async.h"
#include "render/opengl/opengl_tman.h"
#include "render/r1_res.h"
#include "palette/pal.h"


//singleton elements with texture managers are no more allowed
//r1_opengl_texture_manager_class *r1_opengl_texture_manager_class_instance=0;

r1_opengl_texture_manager_class::r1_opengl_texture_manager_class(const i4_pal *pal)
	: r1_texture_manager_class(pal),
	  finished_array(16,16)
{
	//r1_opengl_texture_manager_class_instance = this;

	tex_no_heap = 0;

	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_texture_size);

	min_texture_dimention = 1;
	max_texture_dimention = max_texture_size;
	square_textures = i4_T;

	regular_format.pixel_depth = I4_16BIT;
	regular_format.red_mask   = 31 << 11;
	regular_format.green_mask = 63 << 5;
	regular_format.blue_mask  = 31;
	regular_format.alpha_mask = 0;
	regular_format.lookup     = 0;
	regular_format.calc_shift();

	chroma_format.pixel_depth = I4_16BIT;
	chroma_format.red_mask   = 31 << 10;
	chroma_format.green_mask = 31 << 5;
	chroma_format.blue_mask  = 31;
	chroma_format.alpha_mask = 1 << 15;
	chroma_format.lookup     = 0;
	chroma_format.calc_shift();

	alpha_format.pixel_depth = I4_16BIT;
	alpha_format.red_mask   = 15 << 8;
	alpha_format.green_mask = 15 << 4;
	alpha_format.blue_mask  = 15;
	alpha_format.alpha_mask = 15 << 12;
	alpha_format.lookup     = 0;
	alpha_format.calc_shift();

	init();
}

void r1_opengl_texture_manager_class::init()
{
	r1_texture_manager_class::init();

	tex_no_heap  = new r1_texture_no_heap_class(this,sizeof(used_node),(w32 *)&frame_count);

	// dx5 texture manager does this, don't know why
	//PG: It's only a test wheter the lock is working properly
	array_lock.lock();
	array_lock.unlock();

	bytes_loaded = 0;
	no_of_textures_loaded = 0;

}
void reset_decompressed_cache(void)
{
	//unimplemented for opengl
}
void r1_opengl_texture_manager_class::uninit()
{

	//reset_decompressed_cache();
	if (tex_no_heap)
	{
		delete tex_no_heap;
		tex_no_heap = 0;
	}

	r1_texture_manager_class::uninit();
}


r1_opengl_texture_manager_class::used_node *r1_opengl_texture_manager_class::make_new_used_node(r1_mip_load_info *&load_info,
																								w8 node_alloc_flags)
{
	r1_miplevel_t *mip = load_info->dest_mip;

	w8 texformatflags = 0;

	if (mip->entry->is_transparent() == i4_T)
	{
		texformatflags |= R1_OPENGL_TEXFORMAT_HOLY;
	}
	else if (mip->entry->is_alphatexture() == i4_T)
	{
		texformatflags |= R1_OPENGL_TEXFORMAT_ALPHA;
	}

	used_node *new_used = (used_node *)tex_no_heap->alloc(node_alloc_flags);

	if (!new_used)
	{
		i4_warning("could not alloc used_node");
		load_info->error = R1_MIP_LOAD_NO_ROOM;
		return 0;
	}

	new_used->texformatflags = texformatflags;

	new_used->self_tman=this; //used for async_reader callback

	return new_used;
}


i4_bool r1_opengl_texture_manager_class::immediate_mip_load(r1_mip_load_info *load_info)
{
	if (no_of_textures_loaded > 0)
	{
		if (load_info->dest_mip->last_frame_used != -1)
		{
			load_info->error = R1_MIP_LOAD_BUSY;
			return i4_F;
		}
	}

	r1_miplevel_t *mip = load_info->dest_mip;

	used_node *new_used = make_new_used_node(load_info);
	if (!new_used)
	{
		return i4_F;
	}
	new_used->async_fp=0;
	new_used->mip=0;
	new_used->data = (w8 *) new w8[(mip->width*mip->height*2) ]; //(w8 *)i4_malloc(mip->width * mip->height * 2,"failure calling i4_malloc");

	if (load_info->src_file)
	{
		//for opengl, we currently use only this case, since we support only 16 bit color depth
		//This is for loading data from the tex_cache only, where the data format
		//is always known.
		//Unfortunatelly, the above statement is just plain wrong... This
		//code is also used to load up the lowest miplevel (distant terrain)
		//texture. Therefore, we convert here

		if (load_info->flags & R1_MIPFLAGS_SRC32)
		{
			w32 co=0,r=0,g=0,b=0;
			w16 *tex=(w16 *)(new_used->data);
			const i4_pal *p=i4_pal_man.default_no_alpha_32();
			for (int i=0; i<mip->width*mip->height; i++)
			{
				//The order here is a bit strange (argb backwards).
				//I don't know why it should be like this (testing shows it's right on my system,
				//but it might as well be hardware dependent)
				b=load_info->src_file->read_8();
				g=load_info->src_file->read_8();
				r=load_info->src_file->read_8();
				load_info->src_file->read_8();
				co=p->convert(r<<16|g<<8|b,&regular_format);
				tex[i]=(w16)co;
			}
		}
		else
		{
			load_info->src_file->read(new_used->data,mip->width * mip->height * 2);
		}
	}
	else
	{
		load_texture_from_file(load_info->texture_name,
							   mip->entry->id,new_used->data,
							   mip->width, mip->height,
							   mip->width*2, 2, mip->entry->is_transparent(),
							   mip->entry->is_alphatexture());
	}

	mip->vram_handle = new_used;

	if (mip->last_frame_used != -1)
	{
		mip->last_frame_used = frame_count;
	}

	new_used->mip = mip;
	new_used->async_fp = 0;

	glGenTextures(1,&new_used->gltexname);
	glBindTexture(GL_TEXTURE_2D, new_used->gltexname);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	teximage2d(new_used);

	//i4_free(new_used->data);
	delete [] new_used->data;
	new_used->data = 0;

	//bytes_loaded += mip->width * mip->height * 2;
	//no_of_textures_loaded++;

	return i4_T;
}

void opengl_async_callback(w32 count, void *context)
{
	r1_opengl_texture_manager_class::used_node *u = (r1_opengl_texture_manager_class::used_node *)context;

	//if (u->async_fp) {
	//	delete u->async_fp;//should later delay this until texture is loaded
	//	u->async_fp = 0;
	//	}

	//r1_opengl_texture_manager_class_instance->async_load_finished(u);
	u->self_tman->async_load_finished(u);
}

void r1_opengl_texture_manager_class::async_load_finished(used_node *u)
{
	if (u->mip->flags & R1_MIPLEVEL_LOAD_JPG)
	{
		i4_file_class *asf=u->async_fp;
		i4_ram_file_class *rp=new i4_ram_file_class(
			u->data,
			asf->size()
							   );
		//i4_thread_sleep(10);
		i4_image_class *im=i4_load_image(rp,NULL);
		delete rp;
		delete u->data;
		delete u->async_fp;
		u->async_fp=0;
		w32 tex_by=2;
		if (u->mip->flags & R1_MIPLEVEL_LOAD_32BIT)
		{
			tex_by=4;
		}
		else if (u->mip->flags & R1_MIPLEVEL_LOAD_24BIT)
		{
			tex_by=3;
		}
		u->data=new w8[tex_by*u->mip->width*u->mip->height];
		size_image_to_texture(u->data,im,u->mip->width,u->mip->height,
							  tex_by,u->mip->entry->is_transparent(),
							  u->mip->entry->is_alphatexture());
		u->mip->flags &=~R1_MIPLEVEL_LOAD_JPG;

		array_lock.lock();
		r1_image_list_struct *ils=image_list.add();
		ils->init();
		ils->usage=30;
		ils->image=im;
		ils->id=u->mip->entry->id;
	}
	else
	{
		array_lock.lock();
	}

	finished_array.add(u);

	array_lock.unlock();
}

i4_image_class *r1_opengl_texture_manager_class::get_texture_image(
	r1_texture_handle handle, int frame_counter, int desired_width)
{
	sw32 act_w=0,act_h=0;
	w32 tid=registered_tnames[handle].id;
	//get the best one currently loaded
	r1_miplevel_t *best=get_texture(handle,frame_counter,desired_width,act_w,act_h);
	used_node *u=(used_node *)best->vram_handle;
	for (int i=0; i<memory_images.size(); i++)
	{
		if (memory_images[i].id==tid)
		{
			return memory_images[i].image->copy();
		}                                         //directly return the stored image
	}
	float bla_1,bla_2;
	select_texture(best->vram_handle,bla_1,bla_2);
	i4_image_class *ima=0;
	GLenum format,type;
	if (u->texformatflags==0)
	{
		ima=i4_create_image(act_w,act_h,i4_pal_man.register_pal(&regular_format));
		format=GL_RGB;
		type=GL_UNSIGNED_SHORT_5_6_5;
	}
	else if (u->texformatflags==R1_OPENGL_TEXFORMAT_HOLY)
	{
		ima=i4_create_image(act_w,act_h,i4_pal_man.register_pal(&chroma_format));
		format=GL_RGBA;
		type=GL_UNSIGNED_SHORT_1_5_5_5_REV;
	}
	else if (u->texformatflags==R1_OPENGL_TEXFORMAT_ALPHA)
	{
		ima=i4_create_image(act_w,act_h,i4_pal_man.register_pal(&alpha_format));
		format=GL_RGBA;
		type=GL_UNSIGNED_SHORT_4_4_4_4_REV;
	}
	if (!ima)
	{
		return 0;
	}

	glGetTexImage(GL_TEXTURE_2D,0,format,type,ima->data);
	return ima;

}
//this function currently assumes that the new texture has the same size as the old
int r1_opengl_texture_manager_class::set_texture_image(r1_texture_handle handle, i4_image_class *im)
{
	w32 tid=registered_tnames[handle].id;
	i4_image_class *memim;
	for (int i=0; i<memory_images.size(); i++)
	{
		if (memory_images[i].id==tid)
		{
			delete memory_images[i].image;
			memory_images[i].image=im->copy(); //replace saved memory image with new copy

			sw32 act_w=0,act_h=0;
			r1_miplevel_t *mip=get_texture(handle,0,max_texture_dimention,act_w,act_h);
			used_node *u=(used_node *) mip->vram_handle;
			float b1,b2;
			select_texture(u,b1,b2);
			u->data=im->data;
			teximage2d(u);
			u->data=0;
			return i4_T;
		}
	}
	return i4_F;
}

i4_bool r1_opengl_texture_manager_class::async_mip_load(r1_mip_load_info *load_info)
{
	if (bytes_loaded > 150000 || no_of_textures_loaded > 16)
	{
		if (load_info->dest_mip->last_frame_used != -1)
		{
			load_info->error = R1_MIP_LOAD_BUSY;
			return i4_F;
		}
	}

	r1_miplevel_t *mip = load_info->dest_mip;

	used_node *new_used = make_new_used_node(load_info, R1_TEX_NO_HEAP_DONT_LIST);

	if (!new_used)
	{
		return i4_F;
	}

	new_used->self_tman=this;
	new_used->mip = mip;
	new_used->data=0;
	new_used->async_fp = 0;

	mip->vram_handle = 0;
	mip->flags |= R1_MIPLEVEL_IS_LOADING;

	i4_bool async_worked = i4_F;

	if (load_info->src_file)
	{
		new_used->data = (w8 *) new w8[mip->width*mip->height*2];
		if (!new_used->data) //we're just run out of mem
		{
			mip->flags &= (~R1_MIPLEVEL_IS_LOADING);
			free_mip(new_used);

			load_info->error = R1_MIP_LOAD_NO_ROOM;
			return i4_F;
		}
		//i4_malloc(mip->width * mip->height * 2,"failure calling i4_malloc");
		async_worked= load_info->src_file->async_read(new_used->data,
													  mip->width * mip->height * 2,
													  opengl_async_callback,
													  new_used,255,101);
	}
	else
	{

		//i4_str *fn = r1_texture_id_to_filename(mip->entry->id,r1_get_decompressed_dir());

		//this will now always fail
		//i4_file_class *fp = i4_open(*fn,I4_READ | I4_NO_BUFFER);

		//delete fn;
		//if (!fp)
		//	{
		//This code needs huge cleanups
		//in this case, we must activate the jpg loader via the async_reader.
		//	i4_const_str *n=r1_get_texture_name(mip->entry->id);

		//	load_texture_from_file(*n,//load_info->texture_name
		//		mip->entry->id,new_used->data,mip->width,
		//		mip->height,mip->width,2,mip->entry->is_transparent(),
		//		mip->entry->is_alphatexture());
		//	opengl_async_callback(0,new_used);
		//	async_worked=i4_T;
		//	delete n;
		//return i4_T;

		//	}
		/*else
		   	{

		   	new_used->async_fp = fp;

		   	fp->seek(mip->entry->file_offsets[mip->level]+8);

		   	async_worked= new_used->async_fp->async_read(new_used->data, mip->width * mip->height * 2,opengl_async_callback, new_used,110);
		   	}*/

		async_worked=i4_F;
		int most_unused=0,really_old_guy=-1;
		array_lock.lock();
		for (int i1=0; i1<image_list.size(); i1++)
		{
			if(image_list[i1].id==mip->entry->id)
			{
				image_list[i1].usage=30; //be shure that this don't gets removed just now
				image_list[i1].lock();
				mip->flags|=R1_MIPLEVEL_JPG_ALREADY_LOADED;
				mip->flags &= (~R1_MIPLEVEL_LOAD_JPG); //if already loaded, reset this flag.
				array_lock.unlock();
				opengl_async_callback(0,new_used);
				async_worked=i4_T;
				//pf_opengl_install_vram.stop();
				return i4_T;
			}
			if (image_list[i1].usage<image_list[most_unused].usage)
			{
				most_unused=i1;
			}
		}

		//HINT1: The constant(13) bellow is resposible for the exspected uper limit
		//of the cache. Perhaps it should be user-definable or at least
		//dependent on the physical memory size of the system.

		//HINT2: This is the only place where deleting some entry from
		//the image-cache is allowed. You must not delete anything that
		//has an usage count of 28 or above (They will be accessed on next
		//next_frame() call)
		if ((image_list.size()>13) && (!image_list[most_unused].is_locked()))
		{
			delete image_list[most_unused].image;
			image_list.remove(most_unused);
			//Following Code may look quite strange
			//The only actual purpose of it is to allow
			//The image list to shrink if appropriate.
			for (int i3=13; i3<image_list.size(); i3++)
			{
				if(!image_list[i3].is_locked()) //some really old guy found
				{
					delete image_list[i3].image;
					image_list.remove(i3);
					break;
				}
			}
		}
		array_lock.unlock();
		i4_const_str *n=NULL;
		n=r1_get_texture_name(mip->entry->id);
		char buf[100],buf2[150];
		i4_os_string(*n,buf,100);
		sprintf(buf2,"textures/%s.jpg",buf);
		new_used->async_fp=i4_open(i4_const_str(buf2),I4_READ|I4_NO_BUFFER);
		if (!new_used->async_fp)
		{
			sprintf(buf2,"textures/%s.tga",buf);
			new_used->async_fp=i4_open(i4_const_str(buf2),I4_READ|I4_NO_BUFFER);
		}
		delete n;
		if (new_used->async_fp) //jpg texture exists. load directly from textures folder
		{
			int datasize=new_used->async_fp->size();
			new_used->data=new w8[datasize];
			//already allocated (need it anyway), no!!!!! the image size
			//can be less than the file size and then BOING!!
			mip->flags|=R1_MIPLEVEL_LOAD_JPG;
			async_worked=new_used->async_fp->async_read(new_used->data,
														datasize,
														opengl_async_callback,new_used,255,102);
		}
		else
		{
			mip->flags&= (~R1_MIPLEVEL_IS_LOADING);
			//delete new_used->data;
			new_used->data=0;
			free_mip(new_used);

			load_info->error = R1_MIP_LOAD_MISSING;
			//pf_opengl_install_vram.stop();
			return i4_F;
		}

	}


	if (!async_worked)
	{
		mip->flags &= (~R1_MIPLEVEL_IS_LOADING);
		free_mip(new_used);

		load_info->error = R1_MIP_LOAD_BUSY;
		return i4_F;
	}

	bytes_loaded += mip->width * mip->height * 2;
	no_of_textures_loaded++;

	return i4_T;
}


void r1_opengl_texture_manager_class::free_mip(void *vram_handle)
{
	used_node *u = (used_node *)vram_handle;

	if (u->gltexname)
	{
		glDeleteTextures(1, &u->gltexname);
		u->gltexname = 0;
	}

	if (u->async_fp)
	{
		delete u->async_fp;
		u->async_fp = 0;
	}

	if (u->data)
	{
		delete (u->data);
		u->data = 0;
	}

	tex_no_heap->free((r1_tex_no_heap_used_node *)u);
}

void r1_opengl_texture_manager_class::select_texture(r1_local_texture_handle_type handle,
													 float &smul, float &tmul)
{
	used_node *u = (used_node *)handle;
	glBindTexture(GL_TEXTURE_2D, u->gltexname);

	smul = 1.0;
	tmul = 1.0;
}


r1_miplevel_t *r1_opengl_texture_manager_class::get_texture(r1_texture_handle handle,
															w32 frame_counter,
															sw32 desired_width,
															sw32 &w, sw32 &h)
{
	r1_miplevel_t *mip = r1_texture_manager_class::get_texture(handle,frame_counter,desired_width,w,h);

	used_node *u = (used_node *)mip->vram_handle;
	tex_no_heap->update_usage((r1_tex_no_heap_used_node *)u);

	return mip;
}

// bind all textures respresented in finished_array
void r1_opengl_texture_manager_class::next_frame()
{
	r1_texture_manager_class::next_frame();

	if (!finished_array.size())
	{
		return;
	}

	array_lock.lock();
	i4_image_class *im=NULL;

	// get texture names
	GLuint *texnames = (GLuint *)I4_MALLOC(finished_array.size() * sizeof(GLuint),"failure calling i4_malloc");
	glGenTextures(finished_array.size(), texnames);

	for (sw32 i=0; i<finished_array.size(); i++)
	{
		used_node *u = finished_array[i];

		// put it in vram
		u->mip->vram_handle = u;
		u->mip->flags &= (~R1_MIPLEVEL_IS_LOADING);

		if (u->mip->last_frame_used != -1)
		{
			u->mip->last_frame_used = frame_count;
		}

		tex_no_heap->update_usage((r1_tex_no_heap_used_node *)u);

		if (u->mip->flags & R1_MIPLEVEL_JPG_ALREADY_LOADED)
		{
			//Case 2: The file was already in mem, just copy data
			for(int x=0; x<image_list.size(); x++)
			{
				if (image_list[x].id==u->mip->entry->id)
				{
					im=image_list[x].image;
					image_list[x].usage=30;
					image_list[x].unlock();
				}
				//image_list[x].usage--;
				//if (image_list[x].usage==0) image_list[x].usage=1;
			}
			I4_ASSERT(im,"Internal error in texture loader: Image deleted during access.");
			if (u->data)
			{
				delete u->data;
			}
			u->data=0;
			u->data=new w8[u->mip->width*u->mip->height*2];
			size_image_to_texture(u->data,im,
								  u->mip->width,u->mip->height,2,
								  u->mip->entry->is_transparent(),
								  u->mip->entry->is_alphatexture());
			u->mip->flags &=~R1_MIPLEVEL_JPG_ALREADY_LOADED;
		}


		// assign the texture handle, bind it
		u->gltexname = texnames[i];
		glBindTexture(GL_TEXTURE_2D, u->gltexname);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		teximage2d(u);

		delete u->data;
		u->data = 0;
		no_of_textures_loaded--;

	}

	i4_free(texnames);

	finished_array.clear();

	array_lock.unlock();

	if (tex_no_heap->needs_cleanup)
	{
		tex_no_heap->free_really_old();
	}

	bytes_loaded = 0;
	//no_of_textures_loaded = 0;
}


//
// Use this routine to call glTexImage2D on the specified
// used_node, taking into account the texformatflags data member.
// Scratch space should be zero if we must allocate it
//
void r1_opengl_texture_manager_class::teximage2d(used_node *u)
{

	GLenum intformat, format, type;

	if (u->texformatflags & R1_OPENGL_TEXFORMAT_HOLY)
	{
		// holy texture - 1555
		//intformat = GL_RGB5_A1;//GL_RGBA;
		//format = GL_BGRA_EXT;//GL_BGRA, where did this constant come from?
		//type = GL_UNSIGNED_SHORT;//GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intformat=GL_RGBA;
		format=GL_BGRA;
		type=GL_UNSIGNED_SHORT_1_5_5_5_REV;
	}
	else if (u->texformatflags & R1_OPENGL_TEXFORMAT_ALPHA)
	{
		// alpha texture - 4444
		//intformat = GL_RGBA16;//GL_RGBA;
		//format = GL_BGRA_EXT;//GL_BGRA
		//type = GL_UNSIGNED_SHORT;//GL_UNSIGNED_SHORT_4_4_4_4_REV;
		intformat=GL_RGBA;
		format=GL_BGRA;
		type=GL_UNSIGNED_SHORT_4_4_4_4_REV;
	}
	else
	{
		// RGB texture - 565
		//intformat = GL_RGB16;
		//format = GL_RGB;
		//type = GL_UNSIGNED_SHORT;//GL_UNSIGNED_SHORT_5_6_5;
		intformat=GL_RGB16;
		format=GL_RGB;
		type=GL_UNSIGNED_SHORT_5_6_5;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glTexImage2D(GL_TEXTURE_2D,0,intformat,u->mip->width,u->mip->height,0,format,type,u->data);

}
