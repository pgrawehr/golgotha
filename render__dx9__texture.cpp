/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "render/dx9/texture.h"

#include "video/win32/dx9.h"
#include "video/win32/dx9_error.h"
#include "video/win32/dx9_util.h"
#include "time/profile.h"
#include "render/r1_res.h"
#include "image/image.h"
#include "loaders/load.h"
#include "render/mip_average.h"
#include "file/file.h"
#include "file/async.h"
#include "file/ram_file.h"
#include "main/win_main.h"
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>


i4_profile_class pf_dx9_install_vram("dx9 install vram texture");
i4_profile_class pf_dx9_free_vram("dx9 free vram texture");
i4_profile_class pf_dx9_select_texture("dx9 select_texture");
//i4_profile_class pf_dx9_texture_load("dx9 texture Load()");
i4_profile_class pf_dx9_make_surfaces("dx9 surface alloc");
static i4_profile_class pf_jpg_texture_decompress("decomp jpg (dx9)");


extern DDPIXELFORMAT dd_fmt_565;
extern DDPIXELFORMAT dd_fmt_1555;
extern DDPIXELFORMAT dd_fmt_4444;
extern DDPIXELFORMAT dd_fmt_8888;
extern DDPIXELFORMAT dd_fmt_888;
extern DDPIXELFORMAT dd_fmt_0888;
extern w32 allowed_texture_formats;

//no more needed (no more singleton requirements on the texture manager)
//r1_dx9_texture_class *r1_dx9_texture_class_instance=0;

//LPDDSURFACEDESC first_tex_format=0;



HRESULT WINAPI texture_enum9(LPDDSURFACEDESC lpddsd, LPVOID _regular_format)
{
  if ((lpddsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)==0 &&
       lpddsd->ddpfPixelFormat.dwRGBBitCount==16)
  {
    //find the best non-alpha surface format
    i4_pixel_format *regular_format = (i4_pixel_format *)_regular_format;

    sw32 total_new_bits = lpddsd->ddpfPixelFormat.dwRBitMask +
                          lpddsd->ddpfPixelFormat.dwGBitMask +
                          lpddsd->ddpfPixelFormat.dwBBitMask;

    sw32 total_reg_bits = regular_format->red_mask   + 
                          regular_format->green_mask +
                          regular_format->blue_mask;

    if (total_new_bits > total_reg_bits)
    {
      regular_format->red_mask   = lpddsd->ddpfPixelFormat.dwRBitMask;
      regular_format->green_mask = lpddsd->ddpfPixelFormat.dwGBitMask;
      regular_format->blue_mask  = lpddsd->ddpfPixelFormat.dwBBitMask;
      regular_format->alpha_mask = 0;
      regular_format->lookup     = 0;
      regular_format->calc_shift();
      regular_format->pixel_depth = I4_16BIT;
    }
  }
  //Bad code bellow. Prevents more than one enumeration per game-run(!)
  //Do we really have to care about the enumeration being circular?
/*
  if (first_tex_format)
  {
    if (lpddsd==first_tex_format) 
      return D3DENUMRET_CANCEL;
  }
  else
    first_tex_format = lpddsd;
*/
  if (lpddsd->ddpfPixelFormat.dwRGBBitCount==8)
	  allowed_texture_formats|=R1_MIPFLAGS_USE8;
  if (lpddsd->ddpfPixelFormat.dwRGBBitCount==16)
	  allowed_texture_formats|=R1_MIPFLAGS_USE16;
  if (lpddsd->ddpfPixelFormat.dwRGBBitCount==24)
	  allowed_texture_formats|=R1_MIPFLAGS_USE24;
  if (lpddsd->ddpfPixelFormat.dwRGBBitCount==32)
	  allowed_texture_formats|=R1_MIPFLAGS_USE32;
  return D3D_OK;
}
static i4_bool decomp_dir_loaded=i4_F;
/*typedef struct _decomp_entry{
	w32 id;
	struct _decomp_entry *next;
	}DECOMP_ENTRY;
*/
static i4_array<w32> *decomp_entries=0;

r1_dx9_texture_class::r1_dx9_texture_class(const i4_pal *pal)
: r1_texture_manager_class(pal),finished_array(16,16)
	{
	//r1_dx9_texture_class_instance = this;
	
	tex_no_heap = 0;
	
	//find out how much texture memory we have and show it to anyone debugging
	//DDSCAPS tcaps;
	//tcaps.dwCaps = DDSCAPS_TEXTURE;  
	w32 texture_free;
	//dx9_common.ddraw->GetAvailableVidMem(&tcaps, &total_free, &texture_free);  
	texture_free=dx9_common.device->GetAvailableTextureMem();
	//i4_warning("Total Display Memory Free: %d",total_free);
	i4_warning("Total Display Texture Memory Free: %d",texture_free);

	min_texture_dimention = 0;
	
	//max_texture_dimention = r1_max_texture_size;
	max_texture_dimention=i4_win32_startup_options.max_texture_quality;
	
	//if (max_texture_dimention>256)
	//  max_texture_dimention=256;     // don't allow d3d to go above 256
	
	square_textures = r1_dx9_class_instance.needs_square_textures;
	
	init();
}

//WARNING: Strange things might happen if the texture formats for
//some instances of the tmanager changes but not for all (the 
//texture formats are static variables!)
void r1_dx9_texture_class::init()
	{  
	r1_texture_manager_class::init();
	
	memset(&regular_format,0,sizeof(i4_pixel_format));
	allowed_texture_formats=R1_MIPFLAGS_USE16|R1_MIPFLAGS_USE24|R1_MIPFLAGS_USE32;
	default_texture_flags=R1_MIPFLAGS_USE16;
	regular_format.alpha_bits=0;
	regular_format.alpha_mask=0;
	regular_format.blue_mask =0x001f;
	regular_format.green_mask=0x07e0;
	regular_format.red_mask  =0xf800;
	regular_format.lookup=0;
	regular_format.calc_shift();
	regular_format.pixel_depth=I4_16BIT;
	//if (r1_dx9_class_instance.d3d_device->EnumTextureFormats(texture_enum,(LPVOID)&regular_format) != D3D_OK)
	//	i4_error("FATAL: Couldn't determine Direct3D Texture Formats");

	//TODO: Find the real value for allowed_texture_formats
	//which might depend on the current video mode.
	//Duh, where should we get the correct values from for this call?
	//dx9_common.device->CheckDeviceFormat(D3DADAPTER_DEFAULT,
	//	D3DDEVTYPE_HAL,D3DFMT_A8R8G8B8,0,D3DRTYPE_TEXTURE,
	//	D3DFMT_A8R8G8B8);
	
	
	if ((allowed_texture_formats&R1_MIPFLAGS_USE16) ==0)
		{
		i4_warning("16 Bit textures are not supported in this mode. Strange things might happen");
		}
	if (allowed_texture_formats==R1_MIPFLAGS_USE8)
		{
		i4_error("SEVERE: The current display mode / render device supports only 8 Bit textures. This is not activelly supported. "
			"Be aware that the palettes might be screwed up.");
		}
	
	chroma_format.red_mask   = 31 << 10;
	chroma_format.green_mask = 31 << 5;
	chroma_format.blue_mask  = 31;
	chroma_format.alpha_mask = 1 << 15;
	chroma_format.lookup     = 0;  
	chroma_format.calc_shift();
	chroma_format.pixel_depth = I4_16BIT;
	
	alpha_format.red_mask   = 15 << 8;
	alpha_format.green_mask = 15 << 4;
	alpha_format.blue_mask  = 15;
	alpha_format.alpha_mask = 15 << 12;
	alpha_format.lookup     = 0;  
	alpha_format.calc_shift();
	alpha_format.pixel_depth = I4_16BIT;
	
	memset(&dd_fmt_565,0,sizeof(DDPIXELFORMAT));
	dd_fmt_565.dwSize = sizeof(DDPIXELFORMAT);
	dd_fmt_565.dwFlags = DDPF_RGB;
	dd_fmt_565.dwRGBBitCount = 16;
	dd_fmt_565.dwRBitMask = regular_format.red_mask;
	dd_fmt_565.dwGBitMask = regular_format.green_mask;
	dd_fmt_565.dwBBitMask = regular_format.blue_mask;
	
	memset(&dd_fmt_1555,0,sizeof(DDPIXELFORMAT));
	dd_fmt_1555.dwSize = sizeof(DDPIXELFORMAT);
	dd_fmt_1555.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	dd_fmt_1555.dwRGBBitCount     = 16;    
	dd_fmt_1555.dwRBitMask        = 31 << 10;
	dd_fmt_1555.dwGBitMask        = 31 << 5;
	dd_fmt_1555.dwBBitMask        = 31;
	dd_fmt_1555.dwRGBAlphaBitMask = 1 << 15;
	
	memset(&dd_fmt_4444,0,sizeof(DDPIXELFORMAT));
	dd_fmt_4444.dwSize = sizeof(DDPIXELFORMAT);
	dd_fmt_4444.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	dd_fmt_4444.dwRGBBitCount     = 16;    
	dd_fmt_4444.dwRBitMask        = 15 << 8;
	dd_fmt_4444.dwGBitMask        = 15 << 4;
	dd_fmt_4444.dwBBitMask        = 15;
	dd_fmt_4444.dwRGBAlphaBitMask = 15 << 12;
	
	memset(&dd_fmt_888,0,sizeof(DDPIXELFORMAT));
	memset(&dd_fmt_8888,0,sizeof(DDPIXELFORMAT));
	
	reg24_format.red_mask=255<<16;
	reg24_format.blue_mask=255;
	reg24_format.green_mask=255<<8;
	reg24_format.alpha_mask=0;  
	reg24_format.lookup=0;
	reg24_format.calc_shift();
	reg24_format.pixel_depth=I4_24BIT;
	
	
	dd_fmt_888.dwSize = sizeof(DDPIXELFORMAT);
	dd_fmt_888.dwFlags = DDPF_RGB;
	dd_fmt_888.dwRGBBitCount     = 24;    
	dd_fmt_888.dwRBitMask        = 255 << 16;
	dd_fmt_888.dwGBitMask        = 255 << 8;
	dd_fmt_888.dwBBitMask        = 255;
	dd_fmt_888.dwRGBAlphaBitMask = 0;
	
	reg32_format.red_mask=255<<16;
	reg32_format.blue_mask=255;
	reg32_format.green_mask=255<<8;
	reg32_format.alpha_mask=0;  
	reg32_format.lookup=0;
	reg32_format.calc_shift();
	reg32_format.pixel_depth=I4_32BIT;

	alpha32_format.red_mask=255<<16;
	alpha32_format.green_mask=255<<8;
	alpha32_format.blue_mask=255;
	alpha32_format.alpha_mask=255<<24;
	alpha32_format.lookup=0;
	alpha32_format.calc_shift();
	alpha32_format.pixel_depth=I4_32BIT;


	dd_fmt_8888.dwSize = sizeof(DDPIXELFORMAT);
	dd_fmt_8888.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	dd_fmt_8888.dwRGBBitCount     = 32;    
	dd_fmt_8888.dwRBitMask        = 255 << 16;
	dd_fmt_8888.dwGBitMask        = 255 << 8;
	dd_fmt_8888.dwBBitMask        = 255;
	dd_fmt_8888.dwRGBAlphaBitMask = 255 << 24;

	dd_fmt_0888.dwSize=sizeof (DDPIXELFORMAT);
	dd_fmt_0888.dwFlags=DDPF_RGB;
	dd_fmt_0888.dwRGBBitCount=32;
	dd_fmt_0888.dwRBitMask=255<<16;
	dd_fmt_0888.dwGBitMask=255<<8;
	dd_fmt_0888.dwBBitMask=255;
	dd_fmt_0888.dwRGBAlphaBitMask=0;
	
	if ((allowed_texture_formats&R1_MIPFLAGS_USE16) !=0 )
		{
		default_texture_flags=R1_MIPFLAGS_USE16;
		
		
		if ((allowed_texture_formats&R1_MIPFLAGS_USE24) !=0)
			{
			default_texture_flags|=R1_MIPFLAGS_USE24;
			}
		
		if ((allowed_texture_formats&R1_MIPFLAGS_USE32) !=0)
			{
			default_texture_flags|=R1_MIPFLAGS_USE32;
			
			}
		if (i4_win32_startup_options.texture_bitdepth==16)
			{
			default_texture_flags=R1_MIPFLAGS_USE16;
			}
		
		if (i4_win32_startup_options.texture_bitdepth==32)
			{
			if (i4_current_app->get_display()->get_palette()->source.pixel_depth==I4_16BIT)
				{//the options should actually not allow this configuration.
				//but the error might arise if the app is running in a window
				//and the user changed the desktop bitdepth.
				i4_error("WARNING: Setting the texture bitdepth to 32 while the actual screen bitdepth is only "
					"16 Bit doesn't make sense. Reverting to 16 bit textures.");
				i4_win32_startup_options.texture_bitdepth=0;
				}
			else
				{
				default_texture_flags=R1_MIPFLAGS_USE32;
				if (allowed_texture_formats&R1_MIPFLAGS_USE24)
					default_texture_flags|=R1_MIPFLAGS_USE24;
				}
			}
		
		}//16 bit available
	else
		{//if 16 bit is not available, we set all formats to support 32 bit.
		if (i4_win32_startup_options.texture_bitdepth==16)
			{
			i4_error("WARNING: 16 Bit textures are not available in this mode. Using 32 Bit.");
			i4_win32_startup_options.texture_bitdepth=0;
			}
		default_texture_flags=R1_MIPFLAGS_USE32;
		memset(&dd_fmt_565,0,sizeof(DDPIXELFORMAT));
		
		dd_fmt_565.dwSize = sizeof(DDPIXELFORMAT);
		dd_fmt_565.dwFlags = DDPF_RGB;
		dd_fmt_565.dwRGBBitCount = 16;
		dd_fmt_565.dwRBitMask = 0x0f800;//the defaults (we expect them anyway)
		dd_fmt_565.dwGBitMask = 0x007e0;
		dd_fmt_565.dwBBitMask = 0x0001f;
		if ((allowed_texture_formats&R1_MIPFLAGS_USE24)!=0)
			memcpy(&regular_format,&reg24_format,sizeof(regular_format));//this must exist
		else
			memcpy(&regular_format,&reg32_format,sizeof(regular_format));
		memcpy(&chroma_format,&alpha32_format,sizeof(regular_format));
		memcpy(&alpha_format,&alpha32_format,sizeof(regular_format));
		}
	
	min_texture_dimention=0;
	max_texture_dimention=i4_win32_startup_options.max_texture_quality;
	tex_no_heap  = new r1_texture_no_heap_class(this,sizeof(used_node),(w32 *)&frame_count);
	
	array_lock.lock();
	array_lock.unlock();
	
	bytes_loaded = 0;
	no_of_textures_loaded = 0;
	}

void reset_decompressed_cache9(void);

void r1_dx9_texture_class::uninit()
{  
  
  while (!i4_async_reader::is_idle())
	  {
	  i4_thread_sleep(1);//wait until nothing left in queue 
	  //might bomb if a texture request is pending.
      next_frame();
	  }
  i4_thread_sleep(10);
  next_frame();
  while (finished_array.size()>0)
      {
      next_frame();
      i4_thread_sleep(20);
      }
  reset_decompressed_cache9();
  r1_texture_manager_class::uninit();
  if (tex_no_heap)
  {
    delete tex_no_heap;
    tex_no_heap = 0;
  }
  
  //DDSCAPS tcaps;
  //tcaps.dwCaps = DDSCAPS_TEXTURE;  
  //w32 total_free, texture_free;
  //if (!dx9_common.ddraw) return;
  //dx9_common.ddraw->GetAvailableVidMem(&tcaps, &total_free, &texture_free);  
  //i4_warning("Total Display Memory Free after Cleanup: %d",total_free);
  //i4_warning("Total Display Texture Memory Free after Cleanup: %d",texture_free);
}


#define R1_DX9_SURFACE_HOLY         1
#define R1_DX9_SURFACE_ALPHATEXTURE 2
#define R1_DX9_SURFACE_VRAM         4
#define R1_DX9_SURFACE_24BIT        8
#define R1_DX9_SURFACE_32BIT       16

IDirect3DTexture9 *dx9_make_texture(sw32 w, sw32 h, w8 flags)
{
	D3DFORMAT fmt;
	switch (flags)
	{
	case (R1_DX9_SURFACE_24BIT):
		fmt=D3DFMT_R8G8B8;
		break;
	case R1_DX9_SURFACE_32BIT:
		fmt=D3DFMT_X8R8G8B8;
		break;
	case (R1_DX9_SURFACE_32BIT|R1_DX9_SURFACE_HOLY):
	case (R1_DX9_SURFACE_32BIT|R1_DX9_SURFACE_ALPHATEXTURE):
		fmt=D3DFMT_A8R8G8B8;
		break;
	case R1_DX9_SURFACE_HOLY:
		fmt=D3DFMT_A1R5G5B5;
		break;
	case R1_DX9_SURFACE_ALPHATEXTURE:
		fmt=D3DFMT_A4R4G4B4;
		break;
	default:
		fmt=D3DFMT_R5G6B5;
		break;
	}
	LPDIRECT3DTEXTURE9 ptex;
	int someusage=0;
	i4_dx9_check(dx9_common.device->CreateTexture(w,h,1,someusage,fmt,D3DPOOL_MANAGED,&ptex,0));
	return ptex;
}

/*
IDirectDrawSurface3 *dx9_make_surface(sw32 w, sw32 h, w8 flags)
{
  DDSURFACEDESC ddsd;
  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);  
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.dwWidth  = w;
  ddsd.dwHeight = h;

  if (flags & R1_dx9_SURFACE_VRAM)
  {//Warning: A missing DDSCAPS_3DDEVICE seems to crash direct3d unexspectedly when 
  //the texture is in system - memory. (may be hardware-dependent)
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD|DDSCAPS_3DDEVICE;
  
    //Don't Suggest DirectX which memory to use. It will choose wisely
	//and probably won't crash on out of mem then.
	/ *if (r1_dx9_class_instance.hardware_tmapping)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    else
    ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;*  /

  }
  else
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;

  /    *
  if (flags & R1_dx9_SURFACE_HOLY)
    ddsd.ddpfPixelFormat = dd_fmt_1555;
  else
  if (flags & R1_dx9_SURFACE_ALPHATEXTURE)
    ddsd.ddpfPixelFormat = dd_fmt_4444;
  else 
  if (flags & R1_dx9_SURFACE_24BIT)
	  ddsd.ddpfPixelFormat=dd_fmt_888;
  else if (flags &R1_dx9_SURFACE_32BIT)
	  ddsd.ddpfPixelFormat=dd_fmt_8888;
  else
    ddsd.ddpfPixelFormat = dd_fmt_565;
  *   /
  if (flags & R1_dx9_SURFACE_24BIT)
	  ddsd.ddpfPixelFormat=dd_fmt_888;//cannot have chroma or alpha
  else if (flags & R1_dx9_SURFACE_32BIT)
	  {
	  if ((flags & R1_dx9_SURFACE_HOLY) || (flags & R1_dx9_SURFACE_ALPHATEXTURE))
		  ddsd.ddpfPixelFormat=dd_fmt_8888;
	  else
		  ddsd.ddpfPixelFormat=dd_fmt_0888;
	  }
  else if (flags & R1_dx9_SURFACE_HOLY)
	  ddsd.ddpfPixelFormat=dd_fmt_1555;
  else if (flags & R1_dx9_SURFACE_ALPHATEXTURE)
	  ddsd.ddpfPixelFormat=dd_fmt_4444;
  else
	  ddsd.ddpfPixelFormat=dd_fmt_565;
  
  IDirectDrawSurface  *return_surface  = 0;
  IDirectDrawSurface3 *return_surface3 = 0;
  DDSCAPS memcaps;
  memcaps.dwCaps=DDSCAPS_TEXTURE|DDSCAPS_3DDEVICE;
  DWORD memtotal,memfree;
  //BUG: DirectDraw silently fails and kills the app if it runs out
  //of memory. I have no idea how to get rid of this problem.
  //It's probably about time to change the code to use directX8.
  //i4_dx9_check(dx9_common.ddraw->GetAvailableVidMem(&memcaps,&memtotal,&memfree));
  //if (memfree<(ddsd.dwWidth*ddsd.dwHeight*2)) 
//	  {
//	  i4_warning("Video memory full, free unused textures.");
	  //return 0;
//	  }
  HRESULT res = dx9_common.ddraw->CreateSurface(&ddsd,&return_surface,0);
  
  if (res!=DD_OK)
  {
//#ifdef _DEBUG
//    i4_warning("r1_dx9_texture::dx9_make_surface failed. Possibly out of Video Memory");
//#endif
    
    i4_dx9_check(res);//Did we really get DDERR_OUTOFVIDMEM? If not, we have a real problem
	w32 capsbefore=ddsd.ddsCaps.dwCaps;
	ddsd.ddsCaps.dwCaps&=(~DDSCAPS_VIDEOMEMORY);
	if (capsbefore==ddsd.ddsCaps.dwCaps)
		{//don't try the same thing twice.

		return 0;
		}
	//ddsd.ddsCaps.dwCaps|=DDSCAPS_SYSTEMMEMORY;
	res=dx9_common.ddraw->CreateSurface(&ddsd,&return_surface,0);
    if (res)
		{
		i4_dx9_check(res);//The only error that is actually possible is DDERR_OUTOFMEMORY
		return 0;
		}
  }
        
    res = return_surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&return_surface3);    

    if (!i4_dx9_check(res))
    {
#ifdef _DEBUG
      i4_warning("r1_dx9_texture::dx9_make_surface failed.");
#endif
      return_surface3 = 0;
    }
    
    return_surface->Release();
  DDBLTFX fx;
  memset(&fx,0,sizeof(DDBLTFX));
  fx.dwSize = sizeof(DDBLTFX);
  fx.dwFillColor = 0xf800;
  return_surface3->Blt(NULL,NULL,NULL,DDBLT_WAIT | DDBLT_COLORFILL,&fx); 
	
  return return_surface3;
}
*/


r1_dx9_texture_class::used_node *r1_dx9_texture_class::make_surfaces_for_load(
                                                     IDirect3DTexture9 *&tex,
                                                     r1_mip_load_info *&load_info,
                                                     sw32 actual_w, sw32 actual_h,
                                                     w8 node_alloc_flags)
{
  pf_dx9_make_surfaces.start();
  r1_miplevel_t *mip = load_info->dest_mip;

  sw32 need_size = actual_w*actual_h*2;

  w8 create_flags = 0;
  
  if (mip->entry->is_transparent())
    create_flags = R1_DX9_SURFACE_HOLY;
  else
  if (mip->entry->is_alphatexture())
    create_flags = R1_DX9_SURFACE_ALPHATEXTURE;

  w32 tex_by=2;
  if (load_info->flags&R1_MIPFLAGS_FORCE24)
	  {
	  tex_by=3;
	  create_flags=R1_DX9_SURFACE_24BIT;
	  }
  if (load_info->flags&R1_MIPFLAGS_FORCE32)
	  {
	  tex_by=4;
	  create_flags|=R1_DX9_SURFACE_32BIT;
	  }
  tex = dx9_make_texture(actual_w, actual_h, create_flags);
    
  if (!tex)
  {    
    
	if (release_higher_miplevels(this))
		tex = dx9_make_texture(actual_w, actual_h, create_flags);
	if (!tex)
		{
		load_info->error = R1_MIP_LOAD_NO_ROOM; 
		pf_dx9_make_surfaces.stop();
		return 0;
		}
  }

  used_node *new_used=0;
  if (tex)
  {
    new_used = (used_node *)tex_no_heap->alloc(node_alloc_flags);
  }
  else  
  {
    used_node *u = (used_node *)tex_no_heap->alloc_from_used(mip->width*mip->height*2,node_alloc_flags);
    
    if (!u)
    {
      load_info->error = R1_MIP_LOAD_NO_ROOM;  
	  pf_dx9_make_surfaces.stop();
      return 0;
    }
        
    

    new_used = u;
  }
  
  if (!new_used)
  {
	  i4_warning("Internal warning: Driver problem");
    
    load_info->error = R1_MIP_LOAD_NO_ROOM;   
	pf_dx9_make_surfaces.stop();
    return 0;
  }  
  new_used->self_tman=this;
  pf_dx9_make_surfaces.stop();
  return new_used;
}


//i4_directory_struct *decompdir=NULL;
void reset_decompressed_cache9(void)
//This function is to be called whenever the decompressed dir changes
//i.e after rebuilding the texture cache.
	{
	//if (decompdir) 
	//	delete decompdir;
	//decompdir=NULL;
	if (decomp_entries) delete decomp_entries;
	decomp_entries=0;
	decomp_dir_loaded=i4_F;
	}
/*
i4_bool file_exists(i4_const_str &file)
	{
	//old version, seems to be quite slow
	struct _stat s;
	char buf[200];
	i4_os_string(file,buf,200);
	return _stat(buf,&s);
	}
*/
/*
int w32_sorter9(const w32 *a,const w32 *b)
	{
	if (*a<*b)
    return -1;
  else if (*a>*b)
    return 1;
  else return 0;
	}
*/
w32 r1_get_file_id(const i4_const_str &fname);
/*
static i4_bool file_exists(w32 id)
	{
	//return i4_F;
	
	if (!decomp_dir_loaded)
		{
		i4_directory_struct decompdir;
		decomp_entries=new i4_array<w32>(128,128);
		i4_get_directory(r1_get_decompressed_dir(), decompdir, i4_F,NULL);
		for (int i=0;i<(int)decompdir.tfiles;i++)
			{
			w32 s;
			s = r1_get_file_id(*(decompdir.files[i]));
			decomp_entries->add(s);
			}
        decomp_entries->sort(w32_sorter);
        decomp_dir_loaded=i4_T;
		}
	return (decomp_entries->binary_search(&id,w32_sorter)!=-1);
	}
	*/

i4_bool r1_dx9_texture_class::immediate_mip_load(r1_mip_load_info *load_info)
{  
  if (no_of_textures_loaded > 0)
  {
    if (load_info->dest_mip->last_frame_used!=-1)
    {
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }

  pf_dx9_install_vram.start();

  //IDirectDrawSurface3 *vram_surface   = 0;
  //IDirectDrawSurface3 *system_surface = 0;
  IDirect3DTexture9 *texture=0;

  sw32 i,/*j,*/actual_w,actual_h;
    
  r1_miplevel_t *mip = load_info->dest_mip;

  for (actual_w = 1; actual_w < mip->width;  actual_w*=2); // these need to be power of 2
  for (actual_h = 1; actual_h < mip->height; actual_h*=2);

  if ((load_info->flags&R1_MIPFLAGS_USE16)==0 && (load_info->flags&R1_MIPFLAGS_FORCEFLAGS)==0)
	  {
	  if (load_info->flags&R1_MIPFLAGS_USE24)
		  {
		  if (mip->entry->is_alphatexture()||mip->entry->is_transparent())
			  {
			  load_info->flags|=R1_MIPFLAGS_FORCE32;
			  }
		  else
			  load_info->flags|=R1_MIPFLAGS_FORCE24;
		  }
	  else
		  load_info->flags|=R1_MIPFLAGS_FORCE32;
	  }
  w32 tex_by=2;
  if (load_info->flags&R1_MIPFLAGS_FORCE24)
	  tex_by=3;
  if (load_info->flags&R1_MIPFLAGS_FORCE32)
	  tex_by=4;
    
  used_node *new_used = make_surfaces_for_load(texture,load_info,actual_w,actual_h);
  
  if (!new_used)
  {
    pf_dx9_install_vram.stop();
    return i4_F;
  }

  //DDSURFACEDESC ddsd;
  //memset(&ddsd,0,sizeof(DDSURFACEDESC));
  //ddsd.dwSize = sizeof(DDSURFACEDESC);

  // Lock the surface so we can get the memory address where it is
  //system_surface->Lock(0, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK, 0);  
  D3DLOCKED_RECT lockinfo;
  i4_dx9_check(texture->LockRect(0,&lockinfo,NULL,D3DLOCK_NOSYSLOCK));

  w8   *texture_ptr = (w8 *)lockinfo.pBits;
  sw32  bpl         = mip->width*tex_by; //?? Will this always work?

  i4_bool segmented_load = i4_F;
    
  if (load_info->src_file) 
	  //The data is being read from the tex_cache file
	  //or any other raw-data source (like an i4_ram_file that was just generated)
  {
    if ((load_info->flags&R1_MIPFLAGS_SRC24) && (load_info->flags&R1_MIPFLAGS_FORCE32))
		{//expand 24 bit to 32 bit.
		w32 co,r,b,g;
		w32 *tex=(w32*)texture_ptr;
		for (i=0;i<mip->width*mip->height;i++)
			{
			//Now THAT's gona be slow...
			//but it's the most compatible way and not really time critical here
			b=load_info->src_file->read_8();
			g=load_info->src_file->read_8();
			r=load_info->src_file->read_8();
			co=r<<16|g<<8|b;
			tex[i]=co;
			}
		}
	if ((load_info->flags&R1_MIPFLAGS_SRC24) && (load_info->flags&R1_MIPFLAGS_FORCE16))
		{
		w32 co,r,g,b;
		w16 *tex=(w16*)texture_ptr;
		const i4_pal *p=i4_current_app->get_display()->get_palette();//the 24bit pal
		for (i=0;i<mip->width*mip->height;i++)
			{
			b=load_info->src_file->read_8();
			g=load_info->src_file->read_8();
			r=load_info->src_file->read_8();
			co=p->convert(r<<16|g<<8|b,&regular_format);
			tex[i]=(w16)co;
			}
		}
	if ((load_info->flags&R1_MIPFLAGS_SRC32) && (load_info->flags&R1_MIPFLAGS_FORCE16))
		{
		w32 co,r,g,b;
		w16 *tex=(w16*)texture_ptr;
		const i4_pal *p=i4_current_app->get_display()->get_palette();//the 32bit pal
		for (i=0;i<mip->width*mip->height;i++)
			{
			load_info->src_file->read_8();//dummy
			b=load_info->src_file->read_8();
			g=load_info->src_file->read_8();
			r=load_info->src_file->read_8();
			co=p->convert(r<<16|g<<8|b,&regular_format);
			tex[i]=(w16)co;
			}
		}
	else
    if (!segmented_load)
      load_info->src_file->read(texture_ptr,mip->width*mip->height*tex_by);
    else
    {
      for (i=0; i<mip->height; i++)
      {
        load_info->src_file->read(texture_ptr,bpl);
        texture_ptr += lockinfo.Pitch;
      }
    }
  }
  else
  {
    i4_const_str *fn = r1_texture_id_to_filename(mip->entry->id,r1_get_decompressed_dir());    
    //We're going to read from an external file. See it if exists.
	//if not, we load directly from the jpg file
    
    i4_file_class *fp = NULL;
	
	//if (tex_by==2 && file_exists(mip->entry->id))//gtx files are for 16 bit only!
	//	fp=i4_open(*fn,I4_READ);
	if (!fp)
		{

		//fp=build_temp_tex_file(mip->entry->id);//MUCH too SLOWWWWWWWW (without optimization, fps drops to 1/4 (!))
		
		
		//fn=r1_get_texture_name(mip->entry->id);//Need to optimize this one
		//char buf[230];
		//sprintf(buf,"textures\\%s.jpg",load_info->texture_name);
		load_texture_from_file(load_info->texture_name,
			mip->entry->id,texture_ptr,
			mip->width,mip->height,lockinfo.Pitch,tex_by,mip->entry->is_transparent(),
			mip->entry->is_alphatexture());
		}
    delete fn;
    if (fp)
		{
		fp->seek(mip->entry->file_offsets[mip->level]+8);
    
		if (!segmented_load)
		  fp->read(texture_ptr,mip->width*mip->height*2);
        else
			{
			for (i=0; i<mip->height; i++)
			{
				fp->read(texture_ptr,bpl);
				texture_ptr += lockinfo.Pitch;
			}
		}

    delete fp;
	fp=0;
		}
  }   

  //system_surface->Unlock(0);
  texture->UnlockRect(0);

  mip->vram_handle = new_used;

  if (mip->last_frame_used != -1)
    mip->last_frame_used = frame_count;  

  new_used->tex   = texture;
  new_used->mip            = mip;
  new_used->async_fp       = 0;
  new_used->data           = 0;

  //IDirect3DTexture2 *system_texture, *vram_texture;
  
  //system_surface->QueryInterface(IID_IDirect3DTexture2,(void **)&system_texture);
  //vram_surface->QueryInterface(IID_IDirect3DTexture2,(void **)&vram_texture);
/*
  //pf_dx9_texture_load.start();
  if (!i4_dx9_check(vram_texture->Load(system_texture)))
	  {//Possible Cause: DDERR_SURFACELOST
		//i4_warning("r1_dx9_texture::Load() failed");
	  system_surface->Restore();
	  vram_surface->Restore();
	  if (!i4_dx9_check(vram_texture->Load(system_texture)))
		  return i4_F;
	  }
  //pf_dx9_texture_load.stop();
  
  vram_texture->GetHandle(r1_dx9_class_instance.d3d_device, &new_used->texture_handle);
   
  vram_texture->Release();
  system_texture->Release();

  system_surface->Release();
  
  //bytes_loaded += mip->width*mip->height*2;
  //textures_loaded++;
*/

  pf_dx9_install_vram.stop();
  return i4_T;
}



void dx9_async_callback(w32 count, void *context)
{
  r1_dx9_texture_class::used_node *u = (r1_dx9_texture_class::used_node *)context;

  /*if (u->async_fp) //need this again to get size of file after load
  {
    delete u->async_fp;
    u->async_fp = 0;
  }*/

  //This prevents using multiple instances of the texture manager
  //r1_dx9_texture_class_instance->async_load_finished(u);
  u->self_tman->async_load_finished(u);
};

void r1_dx9_texture_class::async_load_finished(used_node *u)
{
	i4_image_class *im=0;
	int x=0;
	w32 tex_by;
	r1_image_list_struct *ils=0;
    I4_ASSERT(u->mip->entry!=0,"SEVERE: Texture manager internal inconsistency");
    if (u->mip->flags & R1_MIPLEVEL_LOAD_JPG)
	  {
	  i4_ram_file_class *rp=new i4_ram_file_class(u->data,u->async_fp->size());
	  //i4_thread_sleep(10);
      im=i4_load_image(rp,NULL);
      delete rp;
      delete u->data;
      delete u->async_fp;
      u->async_fp=0;
	  u->data=0;
	  tex_by=2;
	  if (u->mip->flags & R1_MIPLEVEL_LOAD_32BIT)
		  tex_by=4;
	  else if (u->mip->flags & R1_MIPLEVEL_LOAD_24BIT)
		  tex_by=3;
      u->data=new w8[tex_by*u->mip->width*u->mip->height];
      size_image_to_texture(u->data,im,u->mip->width,u->mip->height,
		  tex_by,u->mip->entry->is_transparent(),
		  u->mip->entry->is_alphatexture());
      
      array_lock.lock();
	  u->mip->flags &=~R1_MIPLEVEL_LOAD_JPG;
      
	  //Due to delays, it might happen that we come here twice for the same texture.
	  //This is not good from a performance point of view, but can hardly be avoided.
	  //We must ensure that the image-list contains each element only once. 
	  bool found=false;
	  for (x=0;x<image_list.size();x++)
	  {
		  //The image list entry has been prepared (but with a null reference). Otherwise,
		  //the main process would not know that he already sent a load request for this image. 
		  if (image_list[x].id==u->mip->entry->id)
		  {
		      found=true;
			  if (image_list[x].image)
				delete im;
			  else
			  {
				  image_list[x].image=im;
			  }
			  break;
		  }
	  }
	  //Should actually not happen
	  if (!found)
	  {
		ils=image_list.add();
		ils->init();
		ils->id=u->mip->entry->id;
		ils->image=im;
	  }
	  
	  }
  else if (u->mip->flags&R1_MIPLEVEL_JPG_ALREADY_LOADED)
  {
	  array_lock.lock();
	  for(x=0;x<image_list.size();x++)
	  {
		  if (image_list[x].id==u->mip->entry->id)
		  {
			  im=image_list[x].image;
			  image_list[x].usage=30;
			  ils=&image_list[x];
			  I4_ASSERT(ils->is_locked()&&im,"CRITICAL: Object being accessed is not locked or is invalid");
			  break;
		  }
		  //image_list[x].usage--;
		  //if (image_list[x].usage==0) image_list[x].usage=1;
	  }
	  array_lock.unlock();
	  if (!im)
		i4_error("Internal error in texture loader: Image deleted during access.");
	  delete u->data;
	  tex_by=2;
	  if (u->mip->flags & R1_MIPLEVEL_LOAD_32BIT)
		  tex_by=4;
	  else if (u->mip->flags & R1_MIPLEVEL_LOAD_24BIT)
		  tex_by=3;
	  u->data=new w8[tex_by*u->mip->width*u->mip->height];
	  size_image_to_texture(u->data,im,
		  u->mip->width,u->mip->height,tex_by,
		  u->mip->entry->is_transparent(),
		  u->mip->entry->is_alphatexture());
	  
	  array_lock.lock();
	  u->mip->flags &=~R1_MIPLEVEL_JPG_ALREADY_LOADED;
	  //We MUST search the entry again, since it might have been moved
	  //due to removals (but the index can only be smaller than before)
	  //The problem is that we would actually have to do everything in this method 
	  //with the array locked, but this would make the asynchronous load a fake, since
	  //all the expensive calculations involved were synchronous with the main thread. 
	  if (x>=image_list.size())
	  {
		  x=image_list.size()-1;
	  }
	  for(int x2=x;x2>=0;x2--)
	  {
		  if (image_list[x2].id==u->mip->entry->id)
		  {
			  image_list[x2].unlock();			  
			  break;
		  }
	  }
  }
  else
	  {
	  array_lock.lock();
	  }
  
  finished_array.add(u);
  
  array_lock.unlock();
}


i4_bool r1_dx9_texture_class::async_mip_load(r1_mip_load_info *load_info)
{  
  if (bytes_loaded > 540000 || no_of_textures_loaded > 16)
  {
    if (load_info->dest_mip->last_frame_used!=-1)
    {
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }

  pf_dx9_install_vram.start();  

  //IDirectDrawSurface3 *vram_surface   = 0;
  //IDirectDrawSurface3 *system_surface = 0;
  IDirect3DTexture9 *texture=0;

  sw32  /*i,j,*/actual_w,actual_h;
    
  r1_miplevel_t *mip = load_info->dest_mip;

  //for (actual_w = 1; actual_w < mip->width;  actual_w*=2); // these need to be power of 2
  //for (actual_h = 1; actual_h < mip->height; actual_h*=2);
  
  //all textures are power of 2 now
  actual_w = mip->width;
  actual_h = mip->height;

  if ((load_info->flags&R1_MIPFLAGS_USE16)==0 && (load_info->flags&R1_MIPFLAGS_FORCEFLAGS)==0)
	  {
	  if (load_info->flags&R1_MIPFLAGS_USE24)
		  {
		  if (mip->entry->is_alphatexture()||mip->entry->is_transparent())
			  {
			  load_info->flags|=R1_MIPFLAGS_FORCE32;
			  }
		  else
			  load_info->flags|=R1_MIPFLAGS_FORCE24;
		  }
	  else
		  {
		  load_info->flags|=R1_MIPFLAGS_FORCE32;
		  /*if (mip->entry->is_alphatexture())
			  load_info->flags|=R1_MIPFLAGS_HASALPHA;
		  else if (mip->entry->is_transparent())
			  load_info->flags|=R1_MIPFLAGS_HASCHROMA;
			  */
		  }
	  }
  w32 tex_by=2;
  if (load_info->flags&R1_MIPFLAGS_FORCE24)
	  tex_by=3;
  if (load_info->flags&R1_MIPFLAGS_FORCE32)
	  tex_by=4;
    
  used_node *new_used = make_surfaces_for_load(texture,load_info,actual_w,actual_h,R1_TEX_NO_HEAP_DONT_LIST);
  
  if (!new_used)
  {
    load_info->error=R1_MIP_LOAD_NO_ROOM;
    pf_dx9_install_vram.stop();
    return i4_F;
  }

  new_used->mip      = mip;
  
  new_used->async_fp = 0;  
  new_used->data=0;
  new_used->tex=texture;
  new_used->self_tman=this;

  mip->vram_handle  = 0;
  mip->flags       |= R1_MIPLEVEL_IS_LOADING;
  if (tex_by==3)
	  mip->flags|=R1_MIPLEVEL_LOAD_24BIT;
  if (tex_by==4)
	  mip->flags|=R1_MIPLEVEL_LOAD_32BIT;

  i4_bool async_worked = i4_F;
  
  if (load_info->src_file)
  {//this case is mostly unused
    new_used->data     = new w8[mip->width*mip->height*tex_by];//(w8 *)I4_MALLOC(mip->width*mip->height*2,"dx9 async load alloc");
    if (!new_used->data) //we're just run out of mem
	  {
	  mip->flags &= (~R1_MIPLEVEL_IS_LOADING);
      free_mip(new_used);    
    
      load_info->error = R1_MIP_LOAD_NO_ROOM;    
      pf_dx9_install_vram.stop();
      return i4_F;
	  }
    async_worked = load_info->src_file->async_read(new_used->data,
                                                   mip->width*mip->height*tex_by,
                                                   dx9_async_callback,
                                                   new_used,255,92);
  }
  else
  {
    //Most of this is obsolete, we don't use any gtx files at all any more.
    i4_str *fn = r1_texture_id_to_filename(mip->entry->id,r1_get_decompressed_dir());    
    i4_file_class *fp = NULL;//check wheter the texture exists in g_decompressed folder
	//this is currently true for tga-files, as they where decompressed on texture cache creation
	
	//if (tex_by==2 && file_exists(mip->entry->id))//gtx only for 16 bit, otherwise directly load tga or jpg
	//	fp=i4_open(*fn,I4_READ|I4_NO_BUFFER);
    
	/*if (!fp)
		{//File does not exist, so build it
		fp=build_temp_tex_file(mip->entry->id);
		if (!fp) 
			{//must change this here
			i4_error("Fatal: Texture %x does not exist in g_decompressed. Cannot continue.",mip->entry->id);
			}*/
    delete fn;
    if (fp)
		{//gtx-files
		new_used->data     = new w8[mip->width*mip->height*2];//(w8 *)I4_MALLOC(mip->width*mip->height*2,"dx9 async load alloc");
		if (!new_used->data) //we're just run out of mem (actually, this will generate an exception and kill the process)
			{
			mip->flags &= (~R1_MIPLEVEL_IS_LOADING);
			free_mip(new_used);    
    
			load_info->error = R1_MIP_LOAD_NO_ROOM;    
			pf_dx9_install_vram.stop();
			return i4_F;
			}
		new_used->async_fp = fp;

		fp->seek(mip->entry->file_offsets[mip->level]+8);
    
		async_worked = fp->async_read(new_used->data,
                                  mip->width*mip->height*2,
                                  dx9_async_callback,
                                  new_used,110,93);
		}
	else
		{
		async_worked=i4_F;
		int most_unused=0,really_old_guy=-1;
		array_lock.lock();
		for (int i1=0;i1<image_list.size();i1++)
			{
			if(image_list[i1].id==mip->entry->id)
				{
				image_list[i1].usage=30;//be sure that this don't gets removed just now
				image_list[i1].lock();
				mip->flags|=R1_MIPLEVEL_JPG_ALREADY_LOADED;
				mip->flags &= (~R1_MIPLEVEL_LOAD_JPG); //if already loaded, reset this flag.
				array_lock.unlock();
				//dx9_async_callback(0,new_used);
				
				async_worked=i4_async_reader::request_for_callback(0,
					0,dx9_async_callback,new_used,255,91);
				
				bytes_loaded += mip->width*mip->height*2;
				no_of_textures_loaded++;
				pf_dx9_install_vram.stop();
				return async_worked;
				}
			if ((image_list[i1].usage<=image_list[most_unused].usage)
				&&(!image_list[i1].is_locked()))
				most_unused=i1;
			}
		
		//HINT1: The constant(13) bellow is resposible for the exspected uper limit
		//of the cache. Perhaps it should be user-definable or at least
		//dependent on the physical memory size of the system.

		//HINT2: This is the only place where deleting some entry from
		//the image-cache is allowed. You must not delete anything that
		//has an usage count of 28 or above (They will be accessed on next
		//next_frame() call)

		//HINT3: It is not of much use to continue till here if the entry
		//was found above, since we don't need to reduce the cache if we're
		//not going to add anything new. 
		if ((image_list.size()>13) && (image_list[most_unused].usage<28)
			&&(!image_list[most_unused].is_locked()))
			{
			delete image_list[most_unused].image;
			image_list.remove(most_unused);
			//Following Code may look quite strange
			//The only actual purpose of it is to allow
			//The image list to shrink if appropriate.
			for (int i3=13;i3<image_list.size();i3++)
				{
				if(image_list[i3].usage==1&&(!image_list[i3].is_locked()))//some really old guy found
					{
					delete image_list[i3].image;
					image_list.remove(i3);
					break;
					}
				}
			}
		
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
		if (new_used->async_fp)//jpg texture exists. load directly from textures folder
			{
			int datasize=new_used->async_fp->size();
			new_used->data=new w8[datasize];
			mip->flags|=R1_MIPLEVEL_LOAD_JPG;
			r1_image_list_struct *ils_1=image_list.add();
			ils_1->init();
			ils_1->id=mip->entry->id;
			ils_1->usage=30;
			array_lock.unlock();
			async_worked=new_used->async_fp->async_read(new_used->data,
				datasize,
				dx9_async_callback,new_used,255,94);
			}
		else
			{
			mip->flags&= (~R1_MIPLEVEL_IS_LOADING);
			array_lock.unlock();
			free_mip(new_used);    
    
			load_info->error = R1_MIP_LOAD_MISSING;    
			pf_dx9_install_vram.stop();
			return i4_F;
			}

		/*
		if (n) 
			{
			new_used->async_fp=0;
	 		load_texture_from_file(*n,mip->entry->id,new_used->data,mip->width,mip->height,mip->width);
			dx9_async_callback(0,new_used);
			delete n;
			async_worked=i4_T;
			}
			*/
		}


  }
  
  if (!async_worked)
  {
    mip->flags &= (~R1_MIPLEVEL_IS_LOADING);
    free_mip(new_used);    
    
    load_info->error = R1_MIP_LOAD_BUSY;    
    pf_dx9_install_vram.stop();
    return i4_F;
  }  
  
  bytes_loaded += mip->width*mip->height*tex_by;
  no_of_textures_loaded++;

  pf_dx9_install_vram.stop();
  return i4_T;
}

void r1_dx9_texture_class::free_mip(void *vram_handle)
{
  pf_dx9_free_vram.start();
  used_node *u = (used_node *)vram_handle;
  /*if ((u->system_surface)<(void *)0x01000) 
	  {
	  if(u->system_surface!=0) i4_warning("Decoding failed due to corrupt texture cache. Delete tex_cache.dat and restart the maxtool.");
	  pf_dx9_free_vram.stop();
	  return;
	  u->system_surface=0;
	  }
	  */
  if (u->tex)
  {
	//if (u->system_surface->IsLost()==DDERR_SURFACELOST)
	//	u->system_surface->Restore();//is releasing lost surfaces allowed?
    u->tex->Release();
    u->tex = 0;
  }
  
    
  if (u->async_fp)
  {
    delete u->async_fp;
    u->async_fp = 0;
  }
  
  if (u->data)
  {
    delete u->data;//i4_free(u->data);
    u->data = 0;
  }    

  tex_no_heap->free((r1_tex_no_heap_used_node *)u);
  
  pf_dx9_free_vram.stop();
}

void r1_dx9_texture_class::select_texture(r1_local_texture_handle_type handle,
                                          float &smul, float &tmul)
{
  pf_dx9_select_texture.start();

  used_node *u = (used_node *)handle;
  
  if (r1_dx9_class_instance.d3d_device->SetTexture(0,u->tex ) != D3D_OK)
  {
	  i4_warning("DirectX internal driver problem: select_texture failed");
  }

  smul = 1.0;//n->smul;
  tmul = 1.0;//n->tmul; 

  pf_dx9_select_texture.stop();
}

i4_pal *r1_dx9_texture_class::MatchingPal9(D3DSURFACE_DESC *ddsd)
{
	i4_pal *p=0;
	if (ddsd->Format==D3DFMT_R5G6B5)
	{
		return i4_pal_man.register_pal(&regular_format);
	}
	if (ddsd->Format==D3DFMT_A1R5G5B5)
		return i4_pal_man.register_pal(&chroma_format);
	if (ddsd->Format==D3DFMT_A4R4G4B4)
		return i4_pal_man.register_pal(&alpha_format);

	if (ddsd->Format==D3DFMT_R8G8B8)
		return i4_pal_man.register_pal(&reg24_format);
	if (ddsd->Format==D3DFMT_X8R8G8B8)
		return i4_pal_man.register_pal(&reg32_format);
	if (ddsd->Format==D3DFMT_A8R8G8B8)
		return i4_pal_man.register_pal(&alpha32_format);
	i4_warning("DirectX Problem: Unknown format encountered");
	return 0;
}

i4_pal *r1_dx9_texture_class::MatchingPal(LPDDSURFACEDESC ddsd)
	{
	i4_pal *p=0;
	//We assume here that if the source format is 16 bit, 16 was
	//allowed as texture format and the coresponding palettes contain
	//16 bit formats.
	if (ddsd->ddpfPixelFormat.dwRGBBitCount==16)
		{
		if (ddsd->ddpfPixelFormat.dwRGBAlphaBitMask==0x1000)
			return i4_pal_man.register_pal(&chroma_format);
		if (ddsd->ddpfPixelFormat.dwRGBAlphaBitMask==0xf000)
			return i4_pal_man.register_pal(&alpha_format);
		return i4_pal_man.register_pal(&regular_format);
		}
	if (ddsd->ddpfPixelFormat.dwRGBBitCount==24)
		return i4_pal_man.register_pal(&reg24_format);
	if (ddsd->ddpfPixelFormat.dwRGBAlphaBitMask==0)
		return i4_pal_man.register_pal(&reg32_format);
	return i4_pal_man.register_pal(&alpha32_format);
	}

i4_image_class *r1_dx9_texture_class::get_texture_image(r1_texture_handle handle)
	{
	sw32 act_w=0,act_h=0;
	w32 tid=registered_tnames[handle].id;
	for (int i=0;i<memory_images.size();i++)
		{
		if (memory_images[i].id==tid)
			return memory_images[i].image->copy();//directly return the stored image
		}
	r1_miplevel_t *best=get_texture(handle,0,max_texture_dimention,act_w,act_h);
	//float bla1,bla2;
	//select_texture(best->vram_handle,bla1,bla2);
	used_node *u=(used_node*) best->vram_handle;
	D3DSURFACE_DESC ddsd;
	memset(&ddsd,0,sizeof(D3DSURFACE_DESC));
	//ddsd.dwSize=sizeof(DDSURFACEDESC);
	//u->vram_surface->GetSurfaceDesc(&ddsd);
	IDirect3DSurface9 *surf;
	u->tex->GetSurfaceLevel(0,&surf);//increases the refcount
	surf->GetDesc(&ddsd);
	i4_image_class *im=0;
	const i4_pal *pal_to_use=MatchingPal9(&ddsd);
	w32 tex_by=2;
	if (pal_to_use->source.pixel_depth==I4_24BIT)
		tex_by=3;
	if (pal_to_use->source.pixel_depth==I4_32BIT)
		tex_by=4;
	im =i4_create_image(act_w,act_h,pal_to_use);
	D3DLOCKED_RECT lockinfo;
	surf->LockRect(&lockinfo,0,D3DLOCK_NOSYSLOCK);
	memcpy(im->data,lockinfo.pBits,act_w*act_h*tex_by);
	surf->UnlockRect();
	surf->Release();
	return im;
	}

//this makes only sense for memory images, others could be reloaded any time.
int r1_dx9_texture_class::set_texture_image(r1_texture_handle handle, i4_image_class *im)
	{
	w32 tid=registered_tnames[handle].id;
	//i4_image_class *memim;
	for (int i=0;i<memory_images.size();i++)
		{
		if (memory_images[i].id==tid)
			{
			delete memory_images[i].image;
			memory_images[i].image=im->copy();//replace saved memory image with new copy
			
			sw32 act_w=0,act_h=0;
			r1_miplevel_t *mip=get_texture(handle,0,max_texture_dimention,act_w,act_h);
			used_node *u=(used_node*) mip->vram_handle;
			//DDSURFACEDESC ddsd;
			//memset(&ddsd,0,sizeof(DDSURFACEDESC));
			//ddsd.dwSize=sizeof(DDSURFACEDESC);
			D3DSURFACE_DESC ddsd;
			D3DLOCKED_RECT lockinfo;
			u->tex->GetLevelDesc(0,&ddsd);
			const i4_pal *pal_to_use=MatchingPal9(&ddsd);
			w32 tex_by=2;
			if (pal_to_use->source.pixel_depth==I4_24BIT)
				tex_by=3;
			if (pal_to_use->source.pixel_depth==I4_32BIT)
				tex_by=4;
			//u->tex->Lock(0,&ddsd,DDLOCK_WAIT|DDLOCK_NOSYSLOCK|DDLOCK_WRITEONLY,0);
			u->tex->LockRect(0,&lockinfo,0,D3DLOCK_NOSYSLOCK);
			size_image_to_texture(lockinfo.pBits,im,act_w,act_h,tex_by,
				mip->entry->is_transparent(),mip->entry->is_alphatexture());
			//u->vram_surface->Unlock(0);
			u->tex->UnlockRect(0);
			return i4_T;
			}
		}
	return i4_F;
	}

r1_miplevel_t *r1_dx9_texture_class::get_texture(r1_texture_handle handle,
                                                 w32 frame_counter,
                                                 sw32 desired_width,
                                                 sw32 &w, sw32 &h)
{
  r1_miplevel_t *mip = r1_texture_manager_class::get_texture(handle,frame_counter,desired_width,w,h);
  
  used_node *u = (used_node *)mip->vram_handle;  

  tex_no_heap->update_usage((r1_tex_no_heap_used_node *)u);
  
  return mip;
}

void r1_dx9_texture_class::next_frame()
{
  r1_texture_manager_class::next_frame();
  
  sw32 i;
  array_lock.lock();
  //if (finished_array.size()>0)
//	i4_warning("Finished array size %d. Image List size %d", finished_array.size(),
//	image_list.size());
  sw32 max_work=finished_array.size()>3?3:finished_array.size();
  //sw32 max_work=finished_array.size();
  
  for (i=0; i<max_work; i++)
  {
	  //since we're deleting processed entries immediatelly, the index stays 0.
    used_node *u = finished_array[0];    
    
    //this officially puts it in vram
    u->mip->vram_handle  = u;
    u->mip->flags       &= (~R1_MIPLEVEL_IS_LOADING);
	w32 tex_by=2;
	if (u->mip->flags&R1_MIPLEVEL_LOAD_32BIT)
		tex_by=4;
	if (u->mip->flags&R1_MIPLEVEL_LOAD_24BIT)
		tex_by=3;
    
    if (u->mip->last_frame_used != -1)
      u->mip->last_frame_used  = frame_count;
    
    //this adds it into the list of used nodes
    tex_no_heap->update_usage((r1_tex_no_heap_used_node *)u);
	                                
    //DDSURFACEDESC ddsd;
    //memset(&ddsd,0,sizeof(DDSURFACEDESC));
    //ddsd.dwSize = sizeof(DDSURFACEDESC);

    //i4_dx9_check(u->vram_surface->Lock(0, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK, 0));    
	//D3DSURFACE_DESC ddsd;
	D3DLOCKED_RECT lockinfo;
	i4_dx9_check(u->tex->LockRect(0,&lockinfo,NULL,D3DLOCK_NOSYSLOCK));
    
	//if (ddsd.lpSurface==0)//might happen if a request was just pending
	//	{//when the surfaces where lost
	//	u->system_surface->Restore();
	//	u->vram_surface->Restore();
	//	i4_dx9_check(u->vram_surface->Lock(0,&ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK,0));
	//	}
		
	
    //sw32 bpl = u->mip->width*2;

    w8 *texture_ptr = (w8 *)lockinfo.pBits;
	//memset(texture_ptr,55,u->mip->width*u->mip->height*tex_by);
		

    /*if (0)//ddsd.lPitch > bpl)
    {      
      for (j=0; j<u->mip->height; j++)
      {
        memcpy(texture_ptr,u->data,bpl);
        texture_ptr += ddsd.lPitch;
      }
    }
    else */
	i4_image_class *im=NULL;
	if (u->mip->flags&R1_MIPLEVEL_LOAD_JPG)
		{
		//Case 1: Just loaded a JPG - File to u->data
		//this case is obsolete, we decompressed them in the callback already
		i4_ram_file_class *rp=new i4_ram_file_class(u->data,u->async_fp->size());
		//i4_bool must_free_mem=i4_largest_free_block()<0x0100000;
		//Gee: Deleting entries here causes internal error!
		//The problem is that we have no way of knowing which requests
		//are in the load-pipeline with a R1_MIPLEVEL_ALREADY_LOADED flag set.
			/*if (must_free_mem&&image_list.size()>5)
				{//We immediatelly need some free mem
				for (int x3=0;x3<4;x3++)
					{//just delete some of the oldest entries
					if (image_list[0].usage>18) break; //this is probably going to be used in a moment
					delete image_list[0].image;
					image_list.remove(0);
					}
				}*/
			pf_jpg_texture_decompress.start();
			im=i4_load_image(rp,NULL);
			I4_ASSERT(im,"CRITICAL: Image decompression failed.");
			pf_jpg_texture_decompress.stop();
			delete rp;
			r1_image_list_struct *ils=image_list.add();
			ils->init();
			ils->usage=30;
			ils->image=im;
			ils->id=u->mip->entry->id;;
		    size_image_to_texture(texture_ptr,im,u->mip->width,u->mip->height,tex_by,
			    u->mip->entry->is_transparent(),u->mip->entry->is_alphatexture());
		    u->mip->flags &=~R1_MIPLEVEL_LOAD_JPG;
		}
	else
		if (u->mip->flags & R1_MIPLEVEL_JPG_ALREADY_LOADED)
			{
			//Case 2: The file was already in mem, just copy data
				int x;
			for(x=0;x<image_list.size();x++)
			{
				if (image_list[x].id==u->mip->entry->id)
					{
					im=image_list[x].image;
					image_list[x].usage=30;
					}
				//image_list[x].usage--;
				//if (image_list[x].usage==0) image_list[x].usage=1;
			}
			I4_ASSERT(im,"Internal error in texture loader: Image deleted during access.");
			image_list[x].unlock();
			size_image_to_texture(texture_ptr,im,
				u->mip->width,u->mip->height,tex_by,
				u->mip->entry->is_transparent(),
				u->mip->entry->is_alphatexture());
			u->mip->flags &=~R1_MIPLEVEL_JPG_ALREADY_LOADED;
			}
		else
			//Case 3: The Data we loaded was already decompressed
			memcpy(texture_ptr,u->data,u->mip->width*u->mip->height*tex_by);    

    //u->vram_surface->Unlock(0);
	u->tex->UnlockRect(0);

    //i4_free(u->data);
	if (u->async_fp) delete u->async_fp;
	u->async_fp=0;
	if (u->data) delete u->data;
    u->data = 0;    

    //IDirect3DTexture2  *system_texture, *vram_texture;        

    //u->system_surface->QueryInterface(IID_IDirect3DTexture2,(void **)&system_texture);
    //u->vram_surface->QueryInterface(IID_IDirect3DTexture2,(void **)&vram_texture);

    //pf_dx9_texture_load.start();
    //if (!i4_dx9_check(vram_texture->Load(system_texture)))
    //  i4_error("SEVERE: Direct3D->Load() failed.");
    //pf_dx9_texture_load.stop();

    //i4_dx9_check(vram_texture->GetHandle(r1_dx9_class_instance.d3d_device, &u->texture_handle));

    //vram_texture->Release();
    //system_texture->Release();    
    
    //u->system_surface->Release();
    //u->system_surface = 0;    
	finished_array.remove(0);
	no_of_textures_loaded--;//gives number of textures being loaded
  }
  //finished_array.clear();

  

  for(int x=0;x<image_list.size();x++)
			{				
				image_list[x].usage--;
				if (image_list[x].usage==0) image_list[x].usage=1;
			}
  array_lock.unlock();
  if (tex_no_heap && tex_no_heap->needs_cleanup)
  {
    tex_no_heap->free_really_old();
  }

  bytes_loaded = 0;
  //no_of_textures_loaded = 0;
}




