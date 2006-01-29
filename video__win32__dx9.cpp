/*
Modifications:
Support for hardware added 2000.04.12
*/
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
//#include <d3d9.h>
//#include <d3d9types.h>
#include "video/win32/dx9.h"
#include "image/image.h"
#include "time/profile.h"
#include "main/win_main.h"
#include "image/context.h"
#include "render/dx9/render.h"

#include "video/win32/dx9_util.h"
#include "video/win32/dx9_error.h"
#include "threads/threads.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "controller.h"
#include <conio.h>

//#define DX9_NOREALPAGEFLIP /*define this if frames are copied back to the backbuffer every frame*/

static i4_profile_class pf_dx9_mouse("dx9::mouse draw"),
  pf_dx9_copy("dx9::copy dirty"),
  pf_dx9_flip("dx9::flip"),
  pf_dx9_lock("dx9::lock");

i4_dx9_display_class i4_dx9_display_class_instance;

i4_dx9_display_class *i4_dx9_display=0;

struct gr_buf_status_type9
{
  sw8            state;  // -1 = not locked, else  
                         //    I4_FRAME_BUFFER_READ,
                         // or I4_FRAME_BUFFER_WRITE
  i4_image_class *im;


} dx9_buf[2];    // I4_FRONT_FRAME_BUFFER=0, I4_BACK_FRAME_BUFFER=1



int dx9_error_function(const char *str)
{
  dx9_common.cleanup();
  i4_dx9_display->error_handler.old_error_handler(str);
  return 0;
}

i4_refresh_type i4_dx9_display_class::update_model()
{
  if (use_page_flip)
    return I4_PAGE_FLIP_REFRESH;
  else
    return I4_BLT_REFRESH;
  
}

i4_image_class *i4_dx9_display_class::lock_frame_buffer(i4_frame_buffer_type type,
                                                  i4_frame_access_type access)
{
  if (access==dx9_buf[type].state) 
    return dx9_buf[type].im;
  else if (dx9_buf[type].state!=-1)
    unlock_frame_buffer(type);

  HRESULT res=0;

  D3DLOCKED_RECT lk;
  //memset(&cur_dx9_lock_info,0,sizeof(DDSURFACEDESC));
  //cur_dx9_lock_info.dwSize = sizeof(DDSURFACEDESC);

  IDirect3DSurface9 *surf=type==I4_FRONT_FRAME_BUFFER ? 
    dx9_common.front_surface : dx9_common.back_surface;

  int flags=access==I4_FRAME_BUFFER_READ ? D3DLOCK_READONLY : 0;
  //  DDLOCK_WRITEONLY;
          
  //if (!surf)
  //  return 0;
  //Acquiring the syslock is risky, therfore we avoid it.
  res=surf->LockRect(&lk,0,flags|D3DLOCK_NOSYSLOCK);
  //res = surf->Lock(NULL,&cur_dx9_lock_info, flags | DDLOCK_WAIT,0);

  if (res == D3D_OK)
  {
    dx9_buf[type].im->data=(w8*)lk.pBits;
    dx9_buf[type].im->bpl=lk.Pitch;
    dx9_buf[type].state=access;
  }
  else 
	  return 0;

  return dx9_buf[type].im;
}


void i4_dx9_display_class::unlock_frame_buffer(i4_frame_buffer_type type)
{
  IDirect3DSurface9 *surf=type==I4_FRONT_FRAME_BUFFER ? 
    dx9_common.front_surface : dx9_common.back_surface;
         
  if (surf)
    surf->UnlockRect(); 

  dx9_buf[type].im->data=0;
  dx9_buf[type].state=-1;
}





i4_dx9_display_class::i4_dx9_display_class():input(0)
{
  mode_list=0;
  i4_dx9_display=this;
  mouse=0;
  fake_screen=0;
  context=0;  
/*
#ifdef DEBUG
  use_exclusive_mode = i4_F;
  use_page_flip      = i4_F;
#else
  use_exclusive_mode = i4_T;
  use_page_flip      = i4_F;
#endif
  */
  use_exclusive_mode=i4_T;
  use_page_flip=i4_F;//i4_T here doesn't yet really work (screen flickers, only half of the
  //frames are actually drawn)
  //the values here are ignored by default
}


i4_image_class *i4_dx9_display_class::get_screen()
{      
  return fake_screen;
}


void i4_dx9_display_class::uninit()
	{
	//dx9_common.cleanup();
	if (i4_display_list)
	  delete i4_display_list;
	i4_display_list=0;
	i4_display_class::uninit();
	}

li_object *show_gdi_surface9(li_object *o, li_environment *env)
	{
		i4_dx9_display_class_instance.FlipToGDISurface();
		return 0;
	}

// this will add all the drivers that directx find into the display list
void i4_dx9_display_class::init()
{
  int id=0;
  screen_is_gdi=i4_T;
  //actually returns the list of adaptors
  dx9_common.initialize_driver();
  CArrayList *list=dx9_common.get_driver_list();
  char *n=name_buffer;
  D3DAdapterInfo *d;
  for (UINT i=0;i<list->Count();i++)
  { 
	  d=(D3DAdapterInfo*)list->GetPtr(i);
	  sprintf(n, "DirectX9 %s: HAL device %s", d->AdapterIdentifier.Description,
		  d->AdapterIdentifier.DeviceName);
      i4_display_list_struct *s=new i4_display_list_struct;
      s->add_to_list(n, id | 0x8000, 250, this, i4_display_list);
      n+=strlen(n)+1;
	  sprintf(n, "DirectX9 %s: Reference device %s", d->AdapterIdentifier.Description,
		  d->AdapterIdentifier.DeviceName);
      s=new i4_display_list_struct;
      s->add_to_list(n, id+1 | 0x0000, 240, this, i4_display_list);
      n+=strlen(n)+1;
  }

}

// find the driver, then get it's modes and return the first one
i4_display_class::mode *i4_dx9_display_class::get_first_mode(int driver_id)
{
  int find_id = (driver_id & (~0x8001))/2;    // top bit indicates hardware accelerated
  //driver_id/2 is the adaptor id.
  dx9_common.initialize_driver();

  mode_list=0;

  mode_list=dx9_common.get_mode_list(find_id);
  
  amode.mode_id=0;
  amode.adaptor_id=driver_id;


  return get_next_mode();
}


i4_dx9_display_class::~i4_dx9_display_class()
{
  if (i4_display_list)
	  delete i4_display_list;
  i4_display_list=0;
}

void bitmask_2_format(D3DFORMAT d3dfmt,i4_pixel_format &fmt)
{
  int bitspixel=16;
  if ((d3dfmt==D3DFMT_A8R8G8B8)||(d3dfmt==D3DFMT_X8R8G8B8))
	  bitspixel=32;
  if (d3dfmt==D3DFMT_R8G8B8)
	  bitspixel=24;
  switch (d3dfmt)
  {
  case D3DFMT_A8R8G8B8:
	  fmt.alpha_mask=0xff000000;
	  fmt.red_mask  =0x00ff0000;
	  fmt.green_mask=0x0000ff00;
	  fmt.blue_mask =0x000000ff;
	  break;
  case D3DFMT_X8R8G8B8:
  case D3DFMT_R8G8B8:
	  fmt.alpha_mask=0x00000000;
	  fmt.red_mask  =0x00ff0000;
	  fmt.green_mask=0x0000ff00;
	  fmt.blue_mask =0x000000ff;
	  break;
  case D3DFMT_X1R5G5B5:
	  fmt.alpha_mask=0x00000000;
	  fmt.red_mask  =0x00007c00;
	  fmt.green_mask=0x000003e0;
	  fmt.blue_mask =0x0000001f;
	  break;
  case D3DFMT_A1R5G5B5:
	  fmt.alpha_mask=0x00008000;
	  fmt.red_mask  =0x00007c00;
	  fmt.green_mask=0x000003e0;
	  fmt.blue_mask =0x0000001f;
	  break;
  case D3DFMT_R5G6B5:
	  fmt.alpha_mask=0;
	  fmt.blue_mask =0x001F;
	  fmt.green_mask=0x07e0;
	  fmt.red_mask  =0xf800;
	  break;
  case D3DFMT_A4R4G4B4:
	  fmt.alpha_mask=0xf000;
	  fmt.blue_mask =0x0f00;
	  fmt.green_mask=0x00f0;
	  fmt.red_mask  =0x000f;
	  break;
  default:
	  fmt.alpha_mask=0;
	  fmt.blue_mask =0x001F;
	  fmt.green_mask=0x07e0;
	  fmt.red_mask  =0xf800;
	  break;
  };

  fmt.lookup=0;
  fmt.pixel_depth=(i4_pixel_depth)bitspixel;
  fmt.calc_shift();
}

i4_display_class::mode *i4_dx9_display_class::get_next_mode()
{
  if (!mode_list)
	  return 0;

  D3DDISPLAYMODE *mode=(D3DDISPLAYMODE*)mode_list->GetPtr(amode.mode_id);
  
  int bitspixel=16;
  if (mode->Format==D3DFMT_A8R8G8B8)
	  bitspixel=32;
  if (mode->Format==D3DFMT_X8R8G8B8)
	  bitspixel=32;
  if (mode->Format==D3DFMT_R8G8B8)
	  bitspixel=24;
  sprintf(amode.name, "%d X %d %dbit", mode->Width, mode->Height,
	 bitspixel);
  amode.xres=(w16)mode->Width;
  amode.yres=(w16)mode->Height;
  amode.bits_per_pixel=bitspixel;
  if (bitspixel==16)
  {
	  amode.blue_mask=0x001F;
	  amode.green_mask=0x07e0;
	  amode.red_mask=0xf100;
  }
  else
  {
	  amode.blue_mask=0xff;
	  amode.green_mask=0xff00;
	  amode.red_mask=0xff0000;
  }
  amode.flags =0;


  last_mode=amode;
  amode.mode_id++;
  if ((UINT)amode.mode_id>mode_list->Count())
  {
	  mode_list=0;
  }
  //amode.dx9=amode.dx9->next;
  return &amode;
}


i4_bool i4_dx9_display_class::initialize_mode()
{
  use_page_flip=i4_T;
  if (!i4_win32_startup_options.fullscreen)
    use_page_flip = i4_F;
  
  if (!input.create_window(0,0, 
                           last_mode.xres, 
                           last_mode.yres, this, i4_win32_startup_options.fullscreen))
    return i4_F;

  
  int x,y,w,h;

  if (!i4_win32_startup_options.fullscreen)
    input.get_window_area(x,y,w,h);
  else
  {
    w=last_mode.xres;
    h=last_mode.yres;
  }

  last_mode.xres=w;
  last_mode.yres=h;

  // find the driver and initialize it
  int find_id = (last_mode.adaptor_id & (~0x8001))/2;
  //the lowest bit will mean use ref device.
  dx9_common.initialize_driver();
  if (!dx9_common.pD3D9) 
    return i4_F;
  D3DDISPLAYMODE *mode=(D3DDISPLAYMODE*)mode_list->GetPtr(last_mode.mode_id);

  D3DFORMAT format;
  format=mode->Format;
  D3DDEVTYPE type=last_mode.adaptor_id&1?D3DDEVTYPE_REF:D3DDEVTYPE_HAL;
  //If that setting is present, we force the reference device.
  if (i4_win32_startup_options.render==R1_RENDER_DIRECTX9_REF)
      type=D3DDEVTYPE_REF;
  //D3DDISPLAYMODE disp;
  if (i4_win32_startup_options.fullscreen && use_exclusive_mode)
  {
    
	  dx9_common.present.BackBufferWidth=w;
	  dx9_common.present.BackBufferHeight=h;
	  //GetDisplayMode(0,&disp);
	  dx9_common.present.BackBufferFormat=format;
	  dx9_common.present.BackBufferCount=1;
	  dx9_common.present.MultiSampleType=D3DMULTISAMPLE_NONE;
	  dx9_common.present.SwapEffect=D3DSWAPEFFECT_FLIP;
	  dx9_common.present.hDeviceWindow=input.get_window_handle();
	  dx9_common.present.Windowed=FALSE;
	  dx9_common.present.AutoDepthStencilFormat=D3DFMT_D16;
	  dx9_common.present.EnableAutoDepthStencil=TRUE;
	  dx9_common.present.FullScreen_RefreshRateInHz=mode->RefreshRate;
	  dx9_common.present.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
      //For the following setting, the two values
      //D3DPRESENT_INTERVAL_IMMEDIATE
      //D3DPRESENT_INTERVAL_DEFAULT
      //make sense. The first disables VSYNC, therefore allows higher
      //refresh rates, but increases the risk of a very flickering 
      //display and of hidden dialogs. Should try to find a bether
      //solution for this.
      dx9_common.present.PresentationInterval=D3DPRESENT_INTERVAL_DEFAULT;

	  if (!i4_dx9_check(dx9_common.pD3D9->CreateDevice(D3DADAPTER_DEFAULT,
		  type,
		  input.get_window_handle(),
		  /*D3DCREATE_MULTITHREADED|*/D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		  &dx9_common.present,
		  &dx9_common.device
		  )))
		  return i4_F;
  }
  else 
  {
	  //HRESULT res;
	  dx9_common.present.BackBufferWidth=w;
	  dx9_common.present.BackBufferHeight=h;
	  //GetDisplayMode(0,&disp);
	  dx9_common.present.BackBufferFormat=D3DFMT_UNKNOWN;
	  dx9_common.present.BackBufferCount=1;
	  dx9_common.present.MultiSampleType=D3DMULTISAMPLE_NONE;
	  dx9_common.present.SwapEffect= D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;
	  dx9_common.present.hDeviceWindow=input.get_window_handle();
	  dx9_common.present.Windowed=TRUE;
	  dx9_common.present.AutoDepthStencilFormat=D3DFMT_D16;
	  dx9_common.present.EnableAutoDepthStencil=TRUE;
	  dx9_common.present.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER|D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	  dx9_common.present.FullScreen_RefreshRateInHz=D3DPRESENT_RATE_DEFAULT;
      
	  dx9_common.present.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
          //D3DPRESENT_INTERVAL_DEFAULT;
	  if (!i4_dx9_check(dx9_common.pD3D9->CreateDevice(D3DADAPTER_DEFAULT,
		  type,
		  input.get_window_handle(),
		  /*D3DCREATE_MULTITHREADED|*/D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		  &dx9_common.present,
		  &dx9_common.device
		  )))
		  return i4_F;
  }
  ShowWindow(input.get_window_handle(),SW_SHOW);
  UpdateWindow(input.get_window_handle());
  i4_warning("Direct3D9 Device created successfully");
  //win95 function to hide the mouse cursor. our display uses our own cursor

  //if (i4_win32_startup_options.fullscreen && 
  //    !i4_dx9_check(dx9_common.ddraw->SetDisplayMode(last_mode.xres, 
//		last_mode.yres,last_mode.bits_per_pixel,0,0)))
  //    return i4_F;


  if (use_page_flip)
  {
    if (!dx9_common.create_surface(DX9_PAGE_FLIPPED_PRIMARY_SURFACE, 
                                   last_mode.xres, last_mode.yres))
      return i4_F;
  }
  else if (!dx9_common.create_surface(DX9_BACKBUFFERED_PRIMARY_SURFACE, 
                                      last_mode.xres, last_mode.yres))
    return i4_F;
  

  // find the size of the primary surface
  D3DSURFACE_DESC ddsd;
  dx9_common.get_surface_description(dx9_common.back_surface, ddsd); 
  fake_screen=new i4_dx9_image_class(last_mode.xres, last_mode.yres);
  //fake_screen->surface->PageLock(0);
    
  


  i4_pixel_format fmt;
  //fmt.pixel_depth = (i4_pixel_depth)ddsd.Format
  bitmask_2_format(ddsd.Format,fmt);
  //fmt.red_mask    = ddsd.ddpfPixelFormat.dwRBitMask;
  //fmt.green_mask  = ddsd.ddpfPixelFormat.dwGBitMask;
  //fmt.blue_mask   = ddsd.ddpfPixelFormat.dwBBitMask;
  //don't need an alpha channel on the primary surface itself.
  //using this will give trouble when attempting to read the contents
  //of the frame buffer.
  fmt.alpha_mask  = 0; //ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
  fmt.calc_shift();

  pal = i4_pal_man.register_pal(&fmt);

  dx9_buf[0].im=i4_create_image(last_mode.xres, last_mode.yres, pal, 0, 0);
  dx9_buf[0].state=-1;

  dx9_buf[1].im=i4_create_image(last_mode.xres, last_mode.yres, pal, 0, 0);
  dx9_buf[1].state=-1;


  context             = new i4_draw_context_class(0,0, last_mode.xres-1, last_mode.yres-1);
  context->both_dirty = new i4_rect_list_class;
  context->single_dirty= new i4_rect_list_class;

  memcpy(&cur_mode, &last_mode, sizeof(last_mode));

  //if (1) //!i4_win32_startup_options.fullscreen)
//#ifndef DX5_NOREALPAGEFLIP
//    mouse = new dx9_mouse_class(use_page_flip);
//#else
	mouse =new dx9_mouse_class(i4_F);//when frontbuf copied to backbuf, normal mouse behaviour.
//#endif
//  else
//  {
//    thread_mouse = 0;//new ddraw_thread_cursor_class(pal,input.get_window_handle(),
                     //                            0,0,last_mode.xres,last_mode.yres);

//	i4_error("SEVERE: Thread mouse support is disabled. Don't try to use it right now.");
//    input.set_async_mouse(thread_mouse);
//  }
    //obviously we don't print the warning if the user forced us to 
    //use the ref device.
  if ((type==D3DDEVTYPE_REF)&&
      (i4_win32_startup_options.render!=R1_RENDER_DIRECTX9_REF))
	  {
	  if (IDNO==MessageBox(0,"WARNING: The DirectX driver was initialized using the very slow Reference device."
		  "Possible reason: Your display settings are not supported by the Hardware accelerated driver."
		  "Do you want to use this device anyway?",
		  "Reference device",MB_YESNO|MB_ICONWARNING|MB_SYSTEMMODAL|
          MB_TOPMOST|MB_SETFOREGROUND))
		  {
		  return i4_F;
		  }
	  }
  //For dx9, this function enables some special mode that 
  //enables the correct rendering of the mouse. 
  //if (use_page_flip)
  //  FlipToGDISurface();
  //this must be mapped depending on the driver, but not before
  //we are sure the driver is the one we're going to use.
  li_add_function("show_gdi_surface",show_gdi_surface9);
  screen_is_gdi=i4_T;
  return i4_T;
};

i4_bool i4_dx9_display_class::change_mode(w16 newwidth, w16 newheight, 
										  w32 newbitdepth, i4_change_type change_type)
	{
	//w32 retcode;
	if ((!dx9_common.device)||(!fake_screen)) return i4_F;//Not yet initialized, can't continue
	if (change_type==I4_CHANGE_RESOLUTION)
		{//Called after global settings change
			return i4_F; //cannot do this
		}
	int x,y,w,h;
	
	
	input.get_window_area(x,y,w,h);
	//use_page_flip = i4_T;
	if (i4_win32_startup_options.fullscreen)//Don't do anything if in FS, will only make things worse
		{
		return i4_F;
		}
    use_page_flip = i4_F;
	
	for (int i=0; i<2; i++)
		{
		if (dx9_buf[i].im)
			{
			delete dx9_buf[i].im;
			dx9_buf[i].im=0;
			}
		}
	
	if (fake_screen)
		{
		//fake_screen->surface->PageUnlock(0);
		delete fake_screen;
		fake_screen=0;
		}
	if (context)
		{
		delete context;
		context=0;
		}
	/*if (dx9_common.primary_surface) dx9_common.primary_surface->Release;
	dx9_common.primary_surface=0;
	if (dx9_common.front_surface) dx9_common.front_surface->Release();
	dx9_common.front_surface=0;
	if (dx9_common.back_surface) dx9_common.back_surface->Release();
	dx9_common.back_surface=0;
	if (use_page_flip)
		{
		if (!dx9_common.create_surface(DX5_PAGE_FLIPPED_PRIMARY_SURFACE, 
			newwidth, newheight))
			return i4_F;
		}
	else if (!dx9_common.create_surface(DX5_BACKBUFFERED_PRIMARY_SURFACE, 
		newwidth, newheight))
		return i4_F;
	*/
	D3DSURFACE_DESC ddsd;
	//This might happen if another directx app has just popped up. 
	//this is a rare case, so we don't investigate this further
	//(Golg just crashed at me once here)
	//if (dx9_common.primary_surface==NULL)
	//	return i4_F; 
	dx9_common.get_surface_description(dx9_common.back_surface, ddsd); 
	fake_screen=new i4_dx9_image_class(newwidth, newheight);
	
	i4_pixel_format fmt;
	bitmask_2_format(ddsd.Format,fmt);
	fmt.alpha_mask  = 0;//ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	fmt.calc_shift();
	
	pal = i4_pal_man.register_pal(&fmt);
	
	dx9_buf[0].im=i4_create_image(newwidth, newheight, pal, 0, 0);
	dx9_buf[0].state=-1;
	
	dx9_buf[1].im=i4_create_image(newwidth, newheight, pal, 0, 0);
	dx9_buf[1].state=-1;
	
	
	context             = new i4_draw_context_class(0,0, newwidth-1, newheight-1);
	context->both_dirty = new i4_rect_list_class;
	context->single_dirty=new i4_rect_list_class;
	
	cur_mode.xres=newwidth;//must be sure that these settings are correct as code depends on them
	cur_mode.yres=newheight;
	cur_mode.bits_per_pixel=(w8)newbitdepth;//Actually, this shan't change here.
    screen_is_gdi=i4_T;
	return i4_T;
	}


i4_bool i4_dx9_display_class::close()
{
  if (mouse)
  {
    delete mouse;
    mouse = 0;
  }

  input.set_async_mouse(0);

  if (thread_mouse)
  {
    delete thread_mouse;
    thread_mouse = 0;
  }

  /*
  if (error_handler.need_restore_old)
  {
    i4_set_error_function(error_handler.old_error_handler);
    error_handler.need_restore_old=i4_F;
  }
  */
  input.destroy_window();

  for (int i=0; i<2; i++)
  {
    if (dx9_buf[i].im)
    {
      delete dx9_buf[i].im;
      dx9_buf[i].im=0;
    }
  }

  if (fake_screen)
  {
    //fake_screen->surface->PageUnlock(0);
    delete fake_screen;
    fake_screen=0;
  }

  if (context)
  {
    delete context;
    context=0;
  }

  if (mode_list)
  {
    mode_list=0;
  }

  next_frame_copy.delete_list();

  dx9_common.cleanup();

  return i4_T;
}


i4_bool i4_dx9_display_class::set_mouse_shape(i4_cursor_class *cursor)
{
  if (mouse)
  {
    mouse->set_cursor(cursor);    
    return i4_T;
  }
  else
  if (thread_mouse)
  {
    thread_mouse->set_cursor(cursor);
    return i4_T;
  }

  return i4_F;
}

extern HWND current_window_handle;
extern w32 g1_disable_all_drawing;

void i4_dx9_display_class::flush()
	{    
	RECT src;
	if (g1_disable_all_drawing>1)
		return;
	HRESULT hres=DD_OK;
#ifdef I4_TRACE
	i4_warning("TRACE: Beginning of i4_dx9_display_class::flush()");
#endif
	if (!input.get_active())
		{
		context->both_dirty->delete_list();
		context->both_dirty->add_area(0,0,width()-1, height()-1);
		i4_thread_yield();
		return ;
		}
	
	//start of copy stuff
	pf_dx9_copy.start();
	//be sure the display exists first
	//This is actually the only location where the different surfaces
	//get correctly reallocated. If you encounter an DDERR_SURFACELOST
	//somewhere else, just abort the operation with an error and 
	//postphone it for the next frame.
	//For DX9, this is checked using TestCooperativeLevel()
	
	if (dx9_common.device->TestCooperativeLevel()!=D3D_OK)
		{
		i4_warning("Restoring D3D device state");
		Sleep(2000);//Before we come here, we must be sure that every message 
		//that has to do with the WM_ACTIVATEAPP (i.e WM_SIZE) are processed.
		//i4_dx9_check(dx9_common.ddraw->SetDisplayMode(cur_mode.xres,cur_mode.yres,cur_mode.bits_per_pixel,0,0));
		if (!r1_dx9_class_instance.reinit())
		{
			i4_warning("WARNING: Could not restore D3D device.");
			return;
		}
        screen_is_gdi=i4_T;
		context->add_both_dirty(0,0,width(),height());//Be sure everything gets redrawn.

		//due to a bug i'm gona fix soon, these are not commutative 
	    //Update: This should have been fixed
	    //if no map is loaded, then don't try to load any textures, it's of no use
		/*
		if (g1_map_is_loaded())
			{
			li_call("reload_main_textures"); 
			};
		li_call("reload_max_textures");
		*/
		i4_warning("Done restoring surfaces");
		
		}

	context->both_dirty->intersect_area(0,0,width()-1,height()-1);//Minimum requirements
	//better use the directdraw-clipper to solve this problem
	/*if (dx9_common.lpddclipper)
	{
	DWORD clipsize=0;
	RECT input;
	LPRGNDATA lprgn;
	input.bottom=height()-1;
	input.left=0;
	input.right=width()-1;
	input.top=0;
	dx9_common.lpddclipper->GetClipList(NULL,NULL,&clipsize);
	lprgn=(LPRGNDATA) malloc(clipsize);
	dx9_common.lpddclipper->GetClipList(NULL,lprgn,&clipsize);
	//etc....
	
	  free(lprgn);
	  }
	*/
	
	// Step 1 : copy middle buffer information to the back-buffer
	
	i4_rect_list_class::area_iter a,b;
	i4_rect_list_class *use_list;
	
	// if page flipped we need to make add the current dirty to the stuff left over from last frame
//#ifdef DX9_NOREALPAGEFLIP
#if 0
	use_list=context->both_dirty;
#else
	if (use_page_flip) 
		{
		a=context->both_dirty->list.begin();
		for (;a!=context->both_dirty->list.end();++a)
			next_frame_copy.add_area(a->x1, a->y1, a->x2, a->y2);
		//is adding the single - dirty list a good idea? It's of no use.
		/*a=context->single_dirty->list.begin();
		for (;a!=context->single_dirty->list.end();++a)
		next_frame_copy.add_area(a->x1,a->y1,a->x2,a->y2);*/
		use_list=&next_frame_copy;
		}
	else
		use_list=context->both_dirty;
#endif
	
#ifdef I4_TRACE
	i4_warning("TRACE: Now copying windows to backbuffer");
#endif
	//fake_screen->surface->PageLock(0);
	for (a=use_list->list.begin(); a!=use_list->list.end(); ++a)
		{        
		src.left   = a->x1;
		src.top    = a->y1;
		src.right  = a->x2+1;
		src.bottom = a->y2+1;
		POINT pt;
		pt.x=a->x1;
		pt.y=a->y1;
		
		if (!i4_dx9_check(hres=dx9_common.device->UpdateSurface(fake_screen->surface,
			&src,dx9_common.back_surface,&pt)))
			{
				i4_warning("dx9 blt failed, skipping window.");
			}
		
		}    
	pf_dx9_copy.stop();
	//end of copy stuff
	
	//start of mouse stuff
	pf_dx9_mouse.start(); 
	
	mouse_x = input.mouse_x;
	mouse_y = input.mouse_y;
#ifdef I4_TRACE
	i4_warning("TRACE: Drawing the mouse cursor");
#endif
	if (mouse)
		mouse->save_and_draw(mouse_x, mouse_y);
	
	pf_dx9_mouse.stop();  
	
	
	//start of flip stuff
	pf_dx9_flip.start();  
	hres=dx9_common.device->Present(NULL,NULL,NULL,NULL);
    screen_is_gdi=!screen_is_gdi;
	if (!i4_dx9_check(hres))
		{
			i4_error("INTERNAL: Strange error on Present()");
		}
	pf_dx9_flip.stop();
	//end of flip stuff
    
    //PG: Disabled for security reasons. 
    /*
	if (g1_current_controller.get())
		{
		if (g1_current_controller->view.stereomode==g1_view_state_class::STEREO_LEFT)
			{
			_outp(stereoport,0);
			g1_current_controller->view.stereomode=g1_view_state_class::STEREO_RIGHT;
			}
		else if (g1_current_controller->view.stereomode==g1_view_state_class::STEREO_RIGHT)
			{
			_outp(stereoport,1);
			g1_current_controller->view.stereomode=g1_view_state_class::STEREO_LEFT;
			}
		}
	*/
	
#ifdef I4_TRACE
	i4_warning("TRACE: End of flush()");	
#endif
	if (use_page_flip)
		next_frame_copy.swap(context->both_dirty);
	
	context->both_dirty->delete_list();

	pf_dx9_mouse.start();
	if (mouse)
		mouse->restore();
	pf_dx9_mouse.stop();  
	}

void i4_dx9_display_class::FlipToGDISurface()
	{
	if (use_page_flip)
		{
		//if (dx9_common.ddraw) dx9_common.ddraw->FlipToGDISurface();
            //i4_warning("Switching to GDI surface.");
            if (!screen_is_gdi)
                {
                dx9_common.device->Present(NULL,NULL,NULL,NULL);
                screen_is_gdi=i4_T;
                }
            //may yield "Invalid Call", because it only works
            //with SWAPEFFECT_DISCARD, but we must use 
            //SWAPEFFECT_FLIP to be semantically correct. 
			i4_dx9_check(dx9_common.device->SetDialogBoxMode(TRUE));
            
            context->both_dirty->add_area(0,0,width()-1, height()-1);
		}
	return;
	}

void i4_dx9_display_class::DisableDialogBoxMode()
    {
    if (use_page_flip)
        {
        i4_dx9_check(dx9_common.device->SetDialogBoxMode(FALSE));
        }
    }

li_object *dx9_disabledialogboxes(li_object *o, li_environment *env)
    {
    i4_dx9_display_class_instance.DisableDialogBoxMode();
    return 0;
    };

li_automatic_add_function(dx9_disabledialogboxes,"dx9_disabledialogbox");
