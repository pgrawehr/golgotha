/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "render/software/r1_software_texture.h"
//#include "video/win32/dx5.h"
#include "time/profile.h"
#include "render/r1_res.h"
#include "main/win_main.h"
#include "file/file.h"
#include "file/ram_file.h"
#include "image/image.h"
#include "loaders/load.h"

i4_profile_class pf_software_install_vram("software install vram texture");
i4_profile_class pf_software_free_vram("software free vram texture");

//multiple instances at once are allowed, so this is of no use.
//r1_software_texture_class *r1_software_texture_class_instance=0;

r1_software_texture_class::r1_software_texture_class(const i4_pal *pal)
: r1_texture_manager_class(pal),finished_array(16,16)
{  
  //r1_software_texture_class_instance = this;

  tex_heap_man = 0;
  texture_heap = 0;

  init();  
  
  min_texture_dimention = 1;
  max_texture_dimention = i4_win32_startup_options.max_texture_quality;
	  //r1_max_texture_size;

  //setup the texture formats
  //the regular format for the software texture manager is always 565
  //regular_format = pal->source;
  regular_format.red_mask  =0xf800;
  regular_format.green_mask=0x07e0;
  regular_format.blue_mask =0x001f;
  regular_format.alpha_mask=0;
  regular_format.lookup=0;
  regular_format.calc_shift();
  regular_format.pixel_depth=I4_16BIT;

  /*
  chroma_format.red_mask   =0x7C00;
  chroma_format.green_mask =0x03e0;
  chroma_format.blue_mask  =0x001f;
  chroma_format.alpha_mask =0x8000;
  chroma_format.lookup     =0;
  chroma_format.calc_shift();
  chroma_format.pixel_depth=I4_16BIT;
  */
  chroma_format  = regular_format;   //perhaps this works, since 
  //for chroma, we always have to check for the key-color in
  //the draw loop anyways.
  
  alpha_format.red_mask   = 15 << 8;
  alpha_format.green_mask = 15 << 4;
  alpha_format.blue_mask  = 15;
  alpha_format.alpha_mask = 15 << 12;
  alpha_format.lookup     = 0;  
  alpha_format.calc_shift();
  alpha_format.pixel_depth = I4_16BIT;//Another Hardcoded 16
  //for software rendering, 32 bit would be overkill anyway, won't it?
  //alpha_format=regular_format;
}


void r1_software_texture_class::init()
{
  r1_texture_manager_class::init();
  
  //PG: Perhaps this will again be used if memory is limited. 
  //(For Windows and unix we assume we have infinite memory)
  //int tex_mem_size = 1024*1024*8;
  //if (i4_available()/4 < tex_mem_size)
  //  tex_mem_size=i4_available()/4;
  int tex_mem_size=1024*1024*8;
  if (i4_win32_startup_options.max_texture_quality>256)
	  {
	  tex_mem_size=tex_mem_size*(i4_win32_startup_options.max_texture_quality/256);
	  }
  i4_warning("Software texture memory heap allocated: %d bytes", tex_mem_size);
  
  //texture_heap = (w8 *)I4_MALLOC(tex_mem_size,"software texture memory heap");
  texture_heap = new w8[tex_mem_size];

  tex_heap_man = new r1_texture_heap_class(tex_mem_size,
                                           (w32)texture_heap,
                                           sizeof(free_node),
                                           sizeof(used_node),
                                           &frame_count);
  
  array_lock.lock();
  array_lock.unlock();
  
  bytes_loaded = 0;
  no_of_textures_loaded = 0;
}

void r1_software_texture_class::uninit()
{  
  
  //r1_software_texture_class_instance=0;
  r1_texture_manager_class::uninit();
  //uninitialisation of the base class might require that
  //the texture manager is still operational (i.e for flushing 
  //textures currently in the load queue)
  if (tex_heap_man)
  {
    delete tex_heap_man;  
    tex_heap_man = 0;
  }

  if (texture_heap)
  {
    i4_warning("Software texture memory heap deallocated.");

    //i4_free(texture_heap);
	delete[] texture_heap;
    texture_heap = 0;
  }
}

i4_bool r1_software_texture_class::immediate_mip_load(r1_mip_load_info *load_info)
{  
  if (no_of_textures_loaded > 0)
  {
    if (load_info->dest_mip->last_frame_used != -1)
    {
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }  
  pf_software_install_vram.start();  
  r1_miplevel_t *mip = load_info->dest_mip;  
  sw32 need_size;  
  need_size = mip->width * mip->height * 2;
  used_node *new_used = (used_node *)tex_heap_man->alloc(need_size);
  if (!new_used)
  {
    pf_software_install_vram.stop();
    load_info->error = R1_MIP_LOAD_NO_ROOM;
    return i4_F;
  }
    
  free_node *f = (free_node *)new_used->node;
  
  //f is the node we'll use  
  
  if (load_info->src_file)
  {
    load_info->src_file->read((w16 *)f->start,mip->width*mip->height*2);
  }
  else
  {
  /*
    i4_str *fn = r1_texture_id_to_filename(mip->entry->id, r1_get_decompressed_dir());    
    
    i4_file_class *fp = i4_open(*fn);

    delete fn;
    
    fp->seek(mip->entry->file_offsets[mip->level]+8);    
    fp->read((w16 *)f->start,mip->width*mip->height*2);        

    delete fp;
	*/
      load_texture_from_file(load_info->texture_name,
		  mip->entry->id,(void*)f->start,
		  mip->width,mip->height,mip->width*2,2,
		  mip->entry->is_transparent(),mip->entry->is_alphatexture());
  }
  
  bytes_loaded += mip->width*mip->height*2;
  no_of_textures_loaded++;

  //load the texture
  f->mip = mip;
  f->async_fp=0;
  f->data=0;
  f->self_tman=this;
  mip->vram_handle = new_used;

  if (mip->last_frame_used != -1)
    mip->last_frame_used = frame_count;

  pf_software_install_vram.stop();

  return i4_T;
}

int r1_software_texture_class::set_texture_image(r1_texture_handle handle, i4_image_class *im)
	{
	w32 tid=registered_tnames[handle].id;
	//i4_image_class *memim;
	sw32 act_w=0,act_h=0;
	for (int i=0;i<memory_images.size();i++)
		{
		if (memory_images[i].id==tid)
			{
			delete memory_images[i].image;
			memory_images[i].image=im->copy();//replace saved memory image with new copy
			
			sw32 act_w=0,act_h=0;
			r1_miplevel_t *mip=get_texture(handle,0,max_texture_dimention,act_w,act_h);
			used_node *u=(used_node*) mip->vram_handle;
			free_node *f=(free_node*) u->node;
			float b1,b2;
			select_texture(u,b1,b2);
			memcpy((void*)f->start,im->data,act_w*act_h*2);
			return i4_T;
			}
		}
	return i4_F;
	}



i4_image_class *r1_software_texture_class::get_texture_image(r1_texture_handle handle)
	{
	sw32 act_w=0,act_h=0;
	w32 tid=registered_tnames[handle].id;
	//get the best one currently loaded
	r1_miplevel_t *best=get_texture(handle,0,max_texture_dimention,act_w,act_h);
	used_node *u=(used_node*)best->vram_handle;
	for (int i=0;i<memory_images.size();i++)
		{
		if (memory_images[i].id==tid)
			return memory_images[i].image->copy();//directly return the stored image
		}
	float bla_1,bla_2;
	select_texture(best->vram_handle,bla_1,bla_2);
	i4_image_class *ima=0;
	free_node *f=(free_node*)u->node;
	if (f->mip->entry->is_transparent())
		{
		ima=i4_create_image(act_w,act_h,i4_pal_man.register_pal(&chroma_format));
		}
	else if (f->mip->entry->is_alphatexture())
		{
		ima=i4_create_image(act_w,act_h,i4_pal_man.register_pal(&alpha_format));
		}
	else 
		{
		ima=i4_create_image(act_w,act_h,i4_pal_man.register_pal(&regular_format));
		}
	if (!ima)
		return 0;
	
	memcpy(ima->data,f->data,act_w*act_h*2);
	return ima;
	}

static i4_critical_section_class num_async_pending_lock;
static int num_async_pending=0;

r1_software_texture_class::~r1_software_texture_class()
{
  while (num_async_pending!=0)
    i4_sleep(0);


  uninit();
}
//typedef void (*async_callback)(w32 count, void *context);

void software_async_callback(w32 count, void *context)
{  
  r1_software_texture_class::used_node *u = (r1_software_texture_class::used_node *)context;
  r1_software_texture_class::free_node *f = (r1_software_texture_class::free_node *)u->node;
  
  //if (f->async_fp)
  //  delete f->async_fp;

  //f->async_fp = 0;  

  //if (count != (w32)(f->mip->width*f->mip->height*2) )
  //  i4_warning("async texture read failure");

  f->self_tman->async_load_finished(u);  

  num_async_pending_lock.lock();
  num_async_pending--;
  num_async_pending_lock.unlock();
}

i4_bool r1_software_texture_class::async_mip_load(r1_mip_load_info *load_info)
{
  if (bytes_loaded > 100000 || no_of_textures_loaded > 16)
  {
    if (load_info->dest_mip->last_frame_used != -1)
    {    
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }

  pf_software_install_vram.start();

//  sw32 i;

  r1_miplevel_t *mip = load_info->dest_mip;  
  
  w32 need_size = mip->width * mip->height * 2;

  used_node *new_used = (used_node *)tex_heap_man->alloc(need_size,R1_TEX_HEAP_DONT_LIST);
  
  if (!new_used)
  {
    pf_software_install_vram.stop();
    load_info->error = R1_MIP_LOAD_NO_ROOM;
    return i4_F;
  }
  
  //f is the node we'll use  
  free_node *f = (free_node *)new_used->node;    
  
  f->mip         = mip;
  f->async_fp    = 0;
  f->self_tman=this;
  f->data=0;

  mip->flags |= R1_MIPLEVEL_IS_LOADING;

  i4_bool async_worked;

  num_async_pending_lock.lock();
  num_async_pending++;
  num_async_pending_lock.unlock();

  if (load_info->src_file)
  {
    //new_used->data=new w8[need_size];
    //directly load to new_used->node->start
    async_worked = load_info->src_file->async_read((w16 *)f->start,mip->width*mip->height*2,software_async_callback,new_used);    
  }
  else
  {

  /*
    i4_str *fn = r1_texture_id_to_filename(mip->entry->id,r1_get_decompressed_dir());    
    
    i4_file_class *fp = i4_open(*fn,I4_READ | I4_NO_BUFFER | I4_SUPPORT_ASYNC);

    delete fn;
        
    if (!fp)
      async_worked = i4_F;
    else
    {
      f->async_fp = fp;

      fp->seek(mip->entry->file_offsets[mip->level]+8);

      async_worked = fp->async_read((w16 *)f->start,mip->width*mip->height*2,software_async_callback,new_used);
    }
	*/

  //See the dx5 renderer on explanations how this works
    i4_file_class *fp = NULL;
    
		async_worked=i4_F;
		int most_unused=0,really_old_guy=-1;
		array_lock.lock();
		for (int i1=0;i1<image_list.size();i1++)
			{
			if(image_list[i1].id==mip->entry->id)
				{
				image_list[i1].usage=30;//be shure that this don't gets removed just now
				mip->flags|=R1_MIPLEVEL_JPG_ALREADY_LOADED;
				mip->flags &= (~R1_MIPLEVEL_LOAD_JPG); //if already loaded, reset this flag.
				array_lock.unlock();
				software_async_callback(0,new_used);
				async_worked=i4_T;
				pf_software_install_vram.stop();
				return i4_T;
				}
			if (image_list[i1].usage<image_list[most_unused].usage)
				most_unused=i1;
			}
		
		//HINT1: The constant(13) bellow is resposible for the exspected uper limit
		//of the cache. Perhaps it should be user-definable or at least
		//dependent on the physical memory size of the system.

		//HINT2: This is the only place where deleting some entry from
		//the image-cache is allowed. You must not delete anything that
		//has an usage count of 28 or above (They will be accessed on next
		//next_frame() call)
		if ((image_list.size()>13) && (image_list[most_unused].usage<28))
			{
			delete image_list[most_unused].image;
			image_list.remove(most_unused);
			for (int i3=13;i3<image_list.size();i3++)
				{
				if(image_list[i3].usage==1)//some really old guy found
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
		f->async_fp=i4_open(i4_const_str(buf2),I4_READ|I4_NO_BUFFER);
		if (!f->async_fp)
			{
			sprintf(buf2,"textures/%s.tga",buf);
			f->async_fp=i4_open(i4_const_str(buf2),I4_READ|I4_NO_BUFFER);
			}
		delete n;
		if (f->async_fp)//jpg (or tga) texture exists. load directly from textures folder
			{
			int datasize=f->async_fp->size();
			f->data=new w8[datasize];
			mip->flags|=R1_MIPLEVEL_LOAD_JPG;
			async_worked=f->async_fp->async_read(f->data,
				datasize,
				software_async_callback,new_used);
			}
		else
			{
			mip->flags&= (~R1_MIPLEVEL_IS_LOADING);
			free_mip(new_used);    
    
			load_info->error = R1_MIP_LOAD_MISSING;    
			pf_software_install_vram.stop();
			return i4_F;
			}
		
  }
  
  if (!async_worked)
  {  
    if (f->async_fp)
      delete f->async_fp;
    
    mip->flags &= (~R1_MIPLEVEL_IS_LOADING);    
    tex_heap_man->free((r1_tex_heap_used_node *)new_used);
    
    load_info->error = R1_MIP_LOAD_BUSY;    
    pf_software_install_vram.stop();

    num_async_pending_lock.lock();
    num_async_pending--;
    num_async_pending_lock.unlock();

    return i4_F;
  }

  bytes_loaded += mip->width*mip->height*2;
  no_of_textures_loaded++;

  pf_software_install_vram.stop();

  return i4_T;
}

void r1_software_texture_class::async_load_finished(used_node *u)
{
  free_node *f = (free_node *)u->node;
  if (f->mip->flags & R1_MIPLEVEL_LOAD_JPG)
	  {
	  i4_ram_file_class *rp=new i4_ram_file_class(f->data,f->async_fp->size());
	  //i4_thread_sleep(10);
      i4_image_class *im=i4_load_image(rp,NULL);
      delete rp;
      delete f->data;
	  f->data=0;
      delete f->async_fp;
      f->async_fp=0;
	  //w32 tex_by=2;
	  //if (u->mip->flags & R1_MIPLEVEL_LOAD_32BIT)
	  //	  tex_by=4;
	  //else if (u->mip->flags & R1_MIPLEVEL_LOAD_24BIT)
	  //	  tex_by=3;
      //u->data=new w8[tex_by*u->mip->width*u->mip->height];
	  //in this case, we don't copy back to u->data (aka f->data)
	  //but to f->start, which should already have been allocated
      //size_image_to_texture(u->data,im,u->mip->width,u->mip->height,
	  //	  tex_by,u->mip->entry->is_transparent(),
	  //  u->mip->entry->is_alphatexture());
	  size_image_to_texture((void*)f->start,im,f->mip->width,f->mip->height,
		  2,f->mip->entry->is_transparent(),
		  f->mip->entry->is_alphatexture());
      f->mip->flags &=~R1_MIPLEVEL_LOAD_JPG;

      array_lock.lock();
      r1_image_list_struct *ils=image_list.add();
      ils->usage=30;
      ils->image=im;
      ils->id=f->mip->entry->id;
	  }
  else
	  {
	  array_lock.lock();
	  }
  
  finished_array.add(u);
  
  array_lock.unlock();
}

void r1_software_texture_class::next_frame()
{
  r1_texture_manager_class::next_frame(); 

  sw32 i;
  
  array_lock.lock();
  i4_image_class *im=NULL;
  for (i=0; i<finished_array.size(); i++)
  {
    used_node *u = finished_array[i];
    free_node *f = (free_node *)u->node;    
    
    //this officially puts it in vram

	
	if (f->mip->flags & R1_MIPLEVEL_JPG_ALREADY_LOADED)
		{
		//Single case that still needs special handling:
		//Image is in cache
		for(int x=0;x<image_list.size();x++)
		{
			if (image_list[x].id==f->mip->entry->id)
				{
				im=image_list[x].image;
				image_list[x].usage=30;
				}
			//image_list[x].usage--;
			//if (image_list[x].usage==0) image_list[x].usage=1;
		}
		I4_ASSERT(im,"Internal error in software texture loader: Image deleted during access.");
		//size image to destination f->start
		size_image_to_texture((void*)f->start,im,
			f->mip->width,f->mip->height,2,
			f->mip->entry->is_transparent(),
			f->mip->entry->is_alphatexture());
		f->mip->flags &=~R1_MIPLEVEL_JPG_ALREADY_LOADED;
		}
		
    f->mip->vram_handle      = u;
    f->mip->flags           &= (~R1_MIPLEVEL_IS_LOADING);
    
    if (f->mip->last_frame_used != -1)
      f->mip->last_frame_used  = frame_count;    

    //this adds it into the list of used nodes
    tex_heap_man->update_usage((r1_tex_heap_used_node *)u);
	no_of_textures_loaded--;
  }  

  finished_array.clear();

  array_lock.unlock();

  if (tex_heap_man && tex_heap_man->needs_cleanup) //may not yet be initialized
  {
    tex_heap_man->free_really_old();
  }

  bytes_loaded = 0;
  no_of_textures_loaded = 0;
}


void r1_software_texture_class::free_mip(r1_vram_handle_type vram_handle)
{
  pf_software_free_vram.start();

  tex_heap_man->free((r1_tex_heap_used_node *)vram_handle);
  
  pf_software_free_vram.stop();
}

void r1_software_texture_class::select_texture(
										 r1_local_texture_handle_type handle,
                                         float &smul, float &tmul)
{
  r1_tex_heap_used_node *u = (r1_tex_heap_used_node *)handle;

  free_node *f = (free_node *)u->node;

  smul = f->mip->width;
  tmul = f->mip->height;  
}

r1_miplevel_t *r1_software_texture_class::get_texture(r1_texture_handle handle,
                                                      w32 frame_counter,
                                                      sw32 desired_width,
                                                      sw32 &w, sw32 &h)
{
  r1_miplevel_t *mip = r1_texture_manager_class::get_texture(handle,frame_counter,desired_width,w,h);
  if (!mip)
    return 0;

  r1_tex_heap_used_node *u = (r1_tex_heap_used_node *)mip->vram_handle;  
  
  if (u)
    tex_heap_man->update_usage(u);

  return mip;
}
