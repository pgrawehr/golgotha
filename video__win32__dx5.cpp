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
#include "video/win32/dx5.h"
#include "image/image.h"
#include "time/profile.h"
#include "main/win_main.h"
#include "image/context.h"

#include "video/win32/dx5_util.h"
#include "video/win32/dx5_error.h"
#include "threads/threads.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "controller.h"
#include <conio.h>

#define DX5_NOREALPAGEFLIP /*define this if frames are copied back to the backbuffer every frame*/

static i4_profile_class pf_dx5_mouse("dx5::mouse draw"),
  pf_dx5_copy("dx5::copy dirty"),
  pf_dx5_flip("dx5::flip"),
  pf_dx5_lock("dx5::lock");

i4_dx5_display_class i4_dx5_display_class_instance;

i4_dx5_display_class *i4_dx5_display=0;

struct gr_buf_status_type
{
  sw8            state;  // -1 = not locked, else  
                         //    I4_FRAME_BUFFER_READ,
                         // or I4_FRAME_BUFFER_WRITE
  i4_image_class *im;


} dx_buf[2];    // I4_FRONT_FRAME_BUFFER=0, I4_BACK_FRAME_BUFFER=1



int dx5_error_function(const char *str)
{
  dx5_common.cleanup();
  i4_dx5_display->error_handler.old_error_handler(str);
  return 0;
}

i4_refresh_type i4_dx5_display_class::update_model()
{
  if (use_page_flip)
    return I4_PAGE_FLIP_REFRESH;
  else
    return I4_BLT_REFRESH;
  
}

i4_image_class *i4_dx5_display_class::lock_frame_buffer(i4_frame_buffer_type type,
                                                  i4_frame_access_type access)
{
  if (access==dx_buf[type].state) 
    return dx_buf[type].im;
  else if (dx_buf[type].state!=-1)
    unlock_frame_buffer(type);

  HRESULT res=0;

  DDSURFACEDESC cur_dx5_lock_info;
  memset(&cur_dx5_lock_info,0,sizeof(DDSURFACEDESC));
  cur_dx5_lock_info.dwSize = sizeof(DDSURFACEDESC);

  IDirectDrawSurface3 *surf=type==I4_FRONT_FRAME_BUFFER ? 
    dx5_common.front_surface : dx5_common.back_surface;

  int flags=access==I4_FRAME_BUFFER_READ ? DDLOCK_READONLY :
    DDLOCK_WRITEONLY;
          
  if (!surf)
    return 0;

  res = surf->Lock(NULL,&cur_dx5_lock_info, flags | DDLOCK_WAIT,0);

  if (res == DD_OK)
  {
//#ifdef DEBUG
//    surf->Unlock(NULL); 
//#endif

    dx_buf[type].im->data=(w8*)cur_dx5_lock_info.lpSurface;
    dx_buf[type].im->bpl=cur_dx5_lock_info.lPitch;
    dx_buf[type].state=access;
  }

  return dx_buf[type].im;
}


void i4_dx5_display_class::unlock_frame_buffer(i4_frame_buffer_type type)
{
  IDirectDrawSurface3 *surf=type==I4_FRONT_FRAME_BUFFER ? 
    dx5_common.front_surface : dx5_common.back_surface;
          
//#ifndef DEBUG
  if (surf)
    surf->Unlock(NULL); 
//#endif

  dx_buf[type].im->data=0;
  dx_buf[type].state=-1;
}



// i4_image_class *i4_dx5_display_class::grab_framebuffer()
// { 
//   if (!fake_screen) return 0;

//   i4_image_class *im=i4_create_image(width(), height(), fake_screen->get_pal());

//   void *mem=dx5_lock(DX5_LOCK_READ);


//   for (int y=0; y<height(); y++)
//   {
//     w16 *p=(w16 *)((w8 *)im->data + y*im->bpl);

//     for (int x=0; x<width(); x++)
//     {      
//       p[x]=*(((w16 *)mem)+x);
//     }

//     mem=(w8 *)mem + cur_dx5_lock_info.lPitch;
//   }

//   dx5_unlock();

//   return im;
// }

i4_dx5_display_class::i4_dx5_display_class():input(0)
{
  mode_list=0;
  i4_dx5_display=this;
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


i4_image_class *i4_dx5_display_class::get_screen()
{      
  return fake_screen;
}


void i4_dx5_display_class::uninit()
	{
	//dx5_common.cleanup();
	if (i4_display_list)
	  delete i4_display_list;
	i4_display_list=0;
	i4_display_class::uninit();
	}

// this will add all the drivers that directx find into the display list
void i4_dx5_display_class::init()
{
  int id=0;
  dx5_driver *list=dx5_common.get_driver_list();
  char *n=name_buffer;

  for (dx5_driver *d=list; d; d=d->next, id++)
  {    
    IDirectDraw2 *ddraw=dx5_common.initialize_driver(d);
    if (ddraw)
    {
      if (dx5_common.get_driver_hardware_info(ddraw,DRIVER_MODE_ENUMERATE))
      {
        sprintf(n, "%s : 3d Accelerated", d->DriverName);
        i4_display_list_struct *s=new i4_display_list_struct;
        s->add_to_list(n, id | 0x8000, 200, this, i4_display_list);
        //s->add_to_list(n, id, this, i4_display_list);     // << JJ 2000.04.12
        n+=strlen(n)+1;
      }
       
     if (!stricmp(d->DriverName,"display"))
      {
        sprintf(n, "%s : Software Rendered", d->DriverName);
        i4_display_list_struct *s=new i4_display_list_struct;
        s->add_to_list(n, id, 190, this, i4_display_list);
        n+=strlen(n)+1;
      }
      ddraw->Release();
    }
  }

  dx5_common.free_driver_list(list);
}

// find the driver, then get it's modes and return the first one
i4_display_class::mode *i4_dx5_display_class::get_first_mode(int driver_id)
{
  int find_id = driver_id & (~0x8000);    // top bit indicates hardware accelerated

  if (mode_list)
  {
    dx5_common.free_mode_list(mode_list);
    mode_list=0;
  }

  dx5_driver *driver_list=dx5_common.get_driver_list(), *d;//?
  for (d=driver_list; find_id && d; d=d->next, find_id--);//Wie wird das Gerät gewählt?
  IDirectDraw2 *ddraw=dx5_common.initialize_driver(d); 
  dx5_common.free_driver_list(driver_list);
  driver_list=0;
  if (!ddraw) return 0;

  mode_list=dx5_common.get_mode_list(ddraw);
  ddraw->Release();
  
  amode.dx5=mode_list;
  amode.driver_id=driver_id;


  return get_next_mode();
}


i4_dx5_display_class::~i4_dx5_display_class()
{
  if (i4_display_list)
	  delete i4_display_list;
  i4_display_list=0;
}

i4_display_class::mode *i4_dx5_display_class::get_next_mode()
{
  if (!amode.dx5) 
    return 0;


  DDSURFACEDESC *desc=&amode.dx5->desc;

  sprintf(amode.name, "%d X %d %dbit", desc->dwWidth, desc->dwHeight,
	  desc->ddpfPixelFormat.dwRGBBitCount);
  amode.xres=(w16)desc->dwWidth;
  amode.yres=(w16)desc->dwHeight;
  amode.bits_per_pixel=(w8)desc->ddpfPixelFormat.dwRGBBitCount;
  //16;//Noch einmal Hardcoded 16 Bit!
  //lies das von desc->ddpfPixelFormat.dwRGBBitCount
  //Bedingt jedoch, dass alle Modi auch wirklich abgezählt wurden
  //und dass wir am Schluss irgendwie unterscheiden können
  //was wir letztendlich machen wollen.
  amode.red_mask    = desc->ddpfPixelFormat.dwRBitMask;
  amode.green_mask  = desc->ddpfPixelFormat.dwGBitMask;
  amode.blue_mask   = desc->ddpfPixelFormat.dwBBitMask;  
  amode.flags =0;


  last_mode=amode;
  amode.dx5=amode.dx5->next;
  return &amode;
}

li_object *show_gdi_surface5(li_object *o, li_environment *env)
	{
		i4_dx5_display_class_instance.FlipToGDISurface();
		return 0;
	}

i4_bool i4_dx5_display_class::initialize_mode()
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
  int find_id = last_mode.driver_id & (~0x8000);

  dx5_driver *driver_list=dx5_common.get_driver_list(), *d;
  for (d=driver_list; find_id && d; d=d->next, find_id--);
  dx5_common.ddraw=dx5_common.initialize_driver(d); 
  dx5_common.free_driver_list(driver_list);
  driver_list=0;  
  if (!dx5_common.ddraw) 
    return i4_F;


  if (i4_win32_startup_options.fullscreen && use_exclusive_mode)
  {
    // change the error handling function so that the screen is restored before
    // a dialog box is displayed, or they won't be able to read the message!
    if (i4_dx5_check(dx5_common.ddraw->SetCooperativeLevel(input.get_window_handle(),
                                                           DDSCL_EXCLUSIVE | 
														   DDSCL_FULLSCREEN |
														   DDSCL_ALLOWMODEX | 
														   DDSCL_ALLOWREBOOT)))
    {
      error_handler.old_error_handler=i4_get_error_function();
      error_handler.need_restore_old=i4_T;
      i4_set_error_function(dx5_error_function);
    }
    else return i4_F;
  }
  else if (!i4_dx5_check(dx5_common.ddraw->SetCooperativeLevel(input.get_window_handle(),
                                                               DDSCL_NORMAL)))
    return i4_F;

  //win95 function to hide the mouse cursor. our display uses our own cursor

  if (i4_win32_startup_options.fullscreen && 
      !i4_dx5_check(dx5_common.ddraw->SetDisplayMode(last_mode.xres, 
		last_mode.yres,last_mode.bits_per_pixel,0,0)))
      return i4_F;


  if (use_page_flip)
  {
    if (!dx5_common.create_surface(DX5_PAGE_FLIPPED_PRIMARY_SURFACE, 
                                   last_mode.xres, last_mode.yres))
      return i4_F;
  }
  else if (!dx5_common.create_surface(DX5_BACKBUFFERED_PRIMARY_SURFACE, 
                                      last_mode.xres, last_mode.yres))
    return i4_F;
  

  // find the size of the primary surface
  DDSURFACEDESC ddsd;
  dx5_common.get_surface_description(dx5_common.primary_surface, ddsd); 
  fake_screen=new i4_dx5_image_class(last_mode.xres, last_mode.yres);
  //fake_screen->surface->PageLock(0);
    
  


  i4_pixel_format fmt;
  fmt.pixel_depth = (i4_pixel_depth)ddsd.ddpfPixelFormat.dwRGBBitCount;//I4_16BIT;  
  fmt.red_mask    = ddsd.ddpfPixelFormat.dwRBitMask;
  fmt.green_mask  = ddsd.ddpfPixelFormat.dwGBitMask;
  fmt.blue_mask   = ddsd.ddpfPixelFormat.dwBBitMask;
  //don't need an alpha channel on the primary surface itself.
  //using this will give trouble when attempting to read the contents
  //of the frame buffer.
  fmt.alpha_mask  = 0; //ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
  fmt.calc_shift();

  pal = i4_pal_man.register_pal(&fmt);

  dx_buf[0].im=i4_create_image(last_mode.xres, last_mode.yres, pal, 0, 0);
  dx_buf[0].state=-1;

  dx_buf[1].im=i4_create_image(last_mode.xres, last_mode.yres, pal, 0, 0);
  dx_buf[1].state=-1;


  context             = new i4_draw_context_class(0,0, last_mode.xres-1, last_mode.yres-1);
  context->both_dirty = new i4_rect_list_class;
  context->single_dirty= new i4_rect_list_class;

  memcpy(&cur_mode, &last_mode, sizeof(last_mode));

//  if (1) //!i4_win32_startup_options.fullscreen)
#ifndef DX5_NOREALPAGEFLIP
    mouse = new dx5_mouse_class(use_page_flip);
#else
	mouse =new dx5_mouse_class(i4_F);//when frontbuf copied to backbuf, normal mouse behaviour.
#endif
//  else
//  {
//    thread_mouse = 0;//new ddraw_thread_cursor_class(pal,input.get_window_handle(),
                     //                            0,0,last_mode.xres,last_mode.yres);
//
//	i4_error("SEVERE: Thread mouse support is disabled. Don't try to use it right now.");
//    input.set_async_mouse(thread_mouse);
//  }

    li_add_function("show_gdi_surface",show_gdi_surface5);
  return i4_T;
}

i4_bool i4_dx5_display_class::change_mode(w16 newwidth, w16 newheight, 
										  w32 newbitdepth, i4_change_type change_type)
	{
	w32 retcode;
	if ((!dx5_common.ddraw)||(!fake_screen)) return i4_F;//Not yet initialized, can't continue
	if (change_type==I4_CHANGE_RESOLUTION)
		{//Called after global settings change
		
		if (i4_win32_startup_options.fullscreen)
			{//Assume the fields of i4_win32_startup_options are set according to the new resolution
			
			i4_dx5_check(dx5_common.ddraw->SetCooperativeLevel(input.get_window_handle(),
				DDSCL_FULLSCREEN|DDSCL_ALLOWMODEX|DDSCL_EXCLUSIVE|DDSCL_ALLOWREBOOT));
			retcode=i4_dx5_check(dx5_common.ddraw->SetDisplayMode(newwidth, 
				newheight,newbitdepth,0,0));
			if (retcode) return i4_F;
			cur_mode.xres=newwidth;
			cur_mode.yres=newheight;
			cur_mode.bits_per_pixel=(w8)newbitdepth;
			MoveWindow(input.get_window_handle(),0,0,newwidth,newheight,TRUE);
			return i4_T;
			}
		RECT rt1, rt2;
        int x2,y2;
		HWND whandle=input.get_window_handle();
        GetWindowRect(whandle,&rt1);
        GetClientRect(whandle,&rt2);        
        x2 = (rt1.right - rt1.left) - rt2.right  + newwidth;
        y2 = (rt1.bottom- rt1.top ) - rt2.bottom + newheight;
        MoveWindow(whandle,rt1.top,rt1.left,x2,y2,TRUE);
		
		return i4_T;
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
		if (dx_buf[i].im)
			{
			delete dx_buf[i].im;
			dx_buf[i].im=0;
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
	/*if (dx5_common.primary_surface) dx5_common.primary_surface->Release;
	dx5_common.primary_surface=0;
	if (dx5_common.front_surface) dx5_common.front_surface->Release();
	dx5_common.front_surface=0;
	if (dx5_common.back_surface) dx5_common.back_surface->Release();
	dx5_common.back_surface=0;
	if (use_page_flip)
		{
		if (!dx5_common.create_surface(DX5_PAGE_FLIPPED_PRIMARY_SURFACE, 
			newwidth, newheight))
			return i4_F;
		}
	else if (!dx5_common.create_surface(DX5_BACKBUFFERED_PRIMARY_SURFACE, 
		newwidth, newheight))
		return i4_F;
	*/
	DDSURFACEDESC ddsd;
	dx5_common.get_surface_description(dx5_common.primary_surface, ddsd); 
	fake_screen=new i4_dx5_image_class(newwidth, newheight);
	//fake_screen->surface->PageLock(0);
    
	
	
	
	i4_pixel_format fmt;
	fmt.pixel_depth = (i4_pixel_depth)ddsd.ddpfPixelFormat.dwRGBBitCount;//I4_16BIT;  
	fmt.red_mask    = ddsd.ddpfPixelFormat.dwRBitMask;
	fmt.green_mask  = ddsd.ddpfPixelFormat.dwGBitMask;
	fmt.blue_mask   = ddsd.ddpfPixelFormat.dwBBitMask;
	fmt.alpha_mask  = 0;//ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	fmt.calc_shift();
	
	pal = i4_pal_man.register_pal(&fmt);
	
	dx_buf[0].im=i4_create_image(newwidth, newheight, pal, 0, 0);
	dx_buf[0].state=-1;
	
	dx_buf[1].im=i4_create_image(newwidth, newheight, pal, 0, 0);
	dx_buf[1].state=-1;
	
	
	context             = new i4_draw_context_class(0,0, newwidth-1, newheight-1);
	context->both_dirty = new i4_rect_list_class;
	context->single_dirty=new i4_rect_list_class;
	
	cur_mode.xres=newwidth;//must be shure that these settings are correct as code depends on them
	cur_mode.yres=newheight;
	cur_mode.bits_per_pixel=(w8)newbitdepth;//Actually, this shan't change here.
	return i4_T;
	}


i4_bool i4_dx5_display_class::close()
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

  if (error_handler.need_restore_old)
  {
    i4_set_error_function(error_handler.old_error_handler);
    error_handler.need_restore_old=i4_F;
  }

  input.destroy_window();

  for (int i=0; i<2; i++)
  {
    if (dx_buf[i].im)
    {
      delete dx_buf[i].im;
      dx_buf[i].im=0;
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
    dx5_common.free_mode_list(mode_list);
    mode_list=0;
  }

  next_frame_copy.delete_list();

  dx5_common.cleanup();

  return i4_T;
}


i4_bool i4_dx5_display_class::set_mouse_shape(i4_cursor_class *cursor)
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
void i4_dx5_display_class::flush()
	{    
	RECT src;
	if (g1_disable_all_drawing>1)
		return;
	HRESULT hres=DD_OK;
#ifdef I4_TRACE
	i4_warning("TRACE: Beginning of i4_dx5_display_class::flush()");
#endif
	if (!input.get_active())
		{
		context->both_dirty->delete_list();
		context->both_dirty->add_area(0,0,width()-1, height()-1);
		i4_thread_yield();
		return ;
		}
	
	//start of copy stuff
	pf_dx5_copy.start();
	//be shure the display exists first
	//This is actually the only location where the different surfaces
	//get correctly reallocated. If you encounter an DDERR_SURFACELOST
	//somewhere else, just abort the operation with an error and 
	//postphone it for the next frame.
	if (DDERR_SURFACELOST==dx5_common.primary_surface->IsLost())
		{
		i4_warning("Restoring primary surface");
		Sleep(2000);//Before we come here, we must be sure that every message 
		//that has to do with the WM_ACTIVATEAPP (i.e WM_SIZE) are processed.
		//i4_dx5_check(dx5_common.ddraw->SetDisplayMode(cur_mode.xres,cur_mode.yres,cur_mode.bits_per_pixel,0,0));
		
		i4_dx5_check(dx5_common.primary_surface->Restore());
		if (dx5_common.back_surface) 
			{
			i4_dx5_check(dx5_common.back_surface->Restore());
			}
		context->add_both_dirty(0,0,width(),height());//Be shure everything gets redrawn.

		//due to a bug i'm gona fix soon, these are not commutative 
	    //Update: This should have been fixed
	    //if no map is loaded, then don't try to load any textures, it's of no use
		if (g1_map_is_loaded())
			{
			li_call("reload_main_textures"); 
			};
		li_call("reload_max_textures");
			
		i4_warning("Done restoring surfaces");
		//li_call("reload_main_textures");
		}
	
	
	context->both_dirty->intersect_area(0,0,width()-1,height()-1);//Minimum requirements
	//better use the directdraw-clipper to solve this problem
	/*if (dx5_common.lpddclipper)
	{
	DWORD clipsize=0;
	RECT input;
	LPRGNDATA lprgn;
	input.bottom=height()-1;
	input.left=0;
	input.right=width()-1;
	input.top=0;
	dx5_common.lpddclipper->GetClipList(NULL,NULL,&clipsize);
	lprgn=(LPRGNDATA) malloc(clipsize);
	dx5_common.lpddclipper->GetClipList(NULL,lprgn,&clipsize);
	//etc....
	
	  free(lprgn);
	  }
	*/
	
	// Step 1 : copy middle buffer information to the back-buffer
	
	i4_rect_list_class::area_iter a,b;
	i4_rect_list_class *use_list;
	
	// if page flipped we need to make add the current dirty to the stuff left over from last frame
#ifdef DX5_NOREALPAGEFLIP
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
		
		
		if (!i4_dx5_check(hres=dx5_common.back_surface->BltFast(a->x1, a->y1, fake_screen->surface,
			&src,DDBLTFAST_NOCOLORKEY | 
			DDBLTFAST_WAIT)))
			{
			if (hres==DDERR_SURFACEBUSY)
				{
				int loop=0;
				while(hres==DDERR_SURFACEBUSY && loop<200)
					{
					hres=dx5_common.back_surface->BltFast(a->x1, a->y1, fake_screen->surface,
						&src,DDBLTFAST_NOCOLORKEY | 
						DDBLTFAST_WAIT);
					loop++;
					i4_dx5_check(hres);
					Sleep(20);
					}
				}
			else
			if (hres==DDERR_SURFACELOST)
				{
				i4_warning("BltFast: Surfaces still not restored, trying again");
				Sleep(2000);
				
				if (use_page_flip)
					hres=dx5_common.primary_surface->Restore();			
				else
					{
					dx5_common.primary_surface->Restore();
					hres=dx5_common.back_surface->Restore();
					}
				//li_call("reload_main_textures");
				//li_call("reload_max_textures");
				
				fake_screen->surface->Restore();//This one cannot get lost (always in sysmem)
				if(hres==DDERR_WRONGMODE)
					{
					i4_error("dx5 blt failed on area %d %d %d %d (Mode Restoring not yet supported)", a->x1, a->y1,a->x2, a->y2);
					}
				}
			else 
				{
				i4_warning("dx5 blt failed, skipping window.");
				continue;
				//Sleep(2000);
				//fake_screen->surface->Restore();
				//dx5_common.primary_surface->Restore();
				}
			}
		
		}    
	//fake_screen->surface->PageUnlock(0);
	pf_dx5_copy.stop();
	//end of copy stuff
	
	//start of mouse stuff
	pf_dx5_mouse.start(); 
	
	mouse_x = input.mouse_x;
	mouse_y = input.mouse_y;
#ifdef I4_TRACE
	i4_warning("TRACE: Drawing the mouse cursor");
#endif
	if (mouse)
		mouse->save_and_draw(mouse_x, mouse_y);
	
	pf_dx5_mouse.stop();  
	
	
	//start of flip stuff
	pf_dx5_flip.start();  
#ifdef I4_TRACE
	i4_warning("TRACE: Emulating the Flip()");
#endif
	// now flip or copy the backbuffer to video memory
	if (use_page_flip)
		{
		//     if (thread_mouse)
		//     {
		//       thread_mouse->draw_lock.lock();
		//       thread_mouse->use_backbuffer(i4_F);
		//       thread_mouse->remove(); //remove from front buffer
		//       thread_mouse->use_backbuffer(i4_T);      
		//       thread_mouse->display();//display on back buffer
		//     }
		
		hres=dx5_common.primary_surface->Flip(NULL,DDFLIP_WAIT);
		if (!i4_dx5_check(hres))
			{
			if (hres==DDERR_SURFACELOST)
				{
				i4_warning("Flip: DDERR_SURFACELOST at strange location");
				Sleep(1000);//Be sure the mode is restored by now.
				
				dx5_common.back_surface->Restore();
				hres=dx5_common.primary_surface->Restore();//This may fail if the debugger did somestring strange in between
				if (hres==DDERR_WRONGMODE)
					{
					i4_warning("Needs Mode Restore");
					hres=dx5_common.ddraw->SetDisplayMode(cur_mode.xres,cur_mode.yres,cur_mode.bits_per_pixel,0,0);
					i4_dx5_check(hres);
					hres=dx5_common.primary_surface->Restore();
					dx5_common.back_surface->Restore();
					//li_call("reload_max_textures");
					li_call("reload_main_textures");
					
					if (!i4_dx5_check(hres)) 
						{
						i4_error("Restoring the old video-mode failed. This usually only happens if a debugger is running.");
						}
					}
				}
			else 
				i4_error("Strange...");
			}
		//the easiest way to resolve flickering
#ifdef DX5_NOREALPAGEFLIP
		//do
		//	{
			hres=dx5_common.back_surface->BltFast(0,0,dx5_common.front_surface,0,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);
			//this call will rather surelly return with DDERR_WASSTILLDRAWING if flip returned asynchronously
		//	Sleep(1);//give time to other threads;
		//	}
		//	while (hres==DDERR_WASSTILLDRAWING);
			i4_dx5_check(hres);
#endif
			
			
			//     if (thread_mouse)
			//     {      
			//       thread_mouse->use_backbuffer(i4_F);
			//       thread_mouse->draw_lock.unlock();
			//     }
		}
	else
		{
		//     if (thread_mouse)//JJ
		//     {
		//       thread_mouse->draw_lock.lock();      
		//       thread_mouse->use_backbuffer(i4_T);
		//       thread_mouse->display();      
		//     }
        
		int wx,wy,ww,wh;
		input.get_window_area(wx,wy,ww,wh);
        POINT pt= {0,0};
        ClientToScreen(current_window_handle,&pt);
		RECT r={0,0,0,0};
		RECT to={0,0,0,0};
		DDSURFACEDESC pdesc;
		dx5_common.get_desc(dx5_common.primary_surface,pdesc);
		r.top=0;
		r.left=0;
		r.bottom=wh;
		//do a signed compare
		if ((r.bottom+pt.y)>(int)pdesc.dwHeight) r.bottom=pdesc.dwHeight-pt.y;//Bltfast doesn't like to copy things outside screen.
		r.right=ww;
		if ((r.right+pt.x)>(int)pdesc.dwWidth) r.right=pdesc.dwWidth-pt.x;
        // JJ
		// 0  // 0
		HRESULT prob=0;
		//prob=dx5_common.primary_surface->BltFast( pt.x, pt.y,dx5_common.back_surface, &r,
		//	DDBLTFAST_NOCOLORKEY |
		//	DDBLTFAST_WAIT);
		DDBLTFX ddfx;
		ZeroMemory(&ddfx,sizeof(ddfx));
		ddfx.dwSize=sizeof(ddfx);
		to.top=pt.y;
		to.left=pt.x;
		to.bottom=to.top+wh;
		to.right=to.left+ww;
		prob=dx5_common.primary_surface->Blt(&to, dx5_common.back_surface, &r,
			DDBLT_WAIT,&ddfx);
		if (!i4_dx5_check(prob))
			{
			int loop=0;
			if (prob==DDERR_SURFACEBUSY)
				{//i.e. the reference rasterizer may have this problem
				while(prob==DDERR_SURFACEBUSY && loop<100)
					{
					Sleep(10);
					loop++;
					prob=dx5_common.primary_surface->BltFast( pt.x, pt.y,dx5_common.back_surface, &r,
						DDBLTFAST_NOCOLORKEY |
						DDBLTFAST_WAIT);
					i4_dx5_check(prob);
					}
				i4_warning("SEVER: Blitter won't get ready for a very long time");
				}
			else
				{
				if (i4_dx5_check(dx5_common.primary_surface->Restore()))
					{
					dx5_common.back_surface->Restore();
					i4_dx5_check(dx5_common.primary_surface->BltFast(pt.x,pt.y,dx5_common.back_surface,
						NULL,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT));
					}
				else
					{
					i4_error("FATAL: IDIRECTDRAW3::BLTFAST() failed (Mode Restore required, not available for windowed mode)");
					}
				}
			}
		
		//     if (thread_mouse)
		//     {      
		//       thread_mouse->remove();
		//       thread_mouse->set_visible(i4_T);        
		//       thread_mouse->use_backbuffer(i4_F);
		//       thread_mouse->draw_lock.unlock();
		//     }
		}
	
	pf_dx5_flip.stop();
	//end of flip stuff
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
	
	
#ifdef I4_TRACE
	i4_warning("TRACE: End of flush()");	
#endif
	if (use_page_flip)
		next_frame_copy.swap(context->both_dirty);
	
	context->both_dirty->delete_list();

	pf_dx5_mouse.start();
	if (mouse)
		mouse->restore();
	pf_dx5_mouse.stop();  
	}

void i4_dx5_display_class::FlipToGDISurface()
	{
	if (use_page_flip)
		{

		if (dx5_common.ddraw) dx5_common.ddraw->FlipToGDISurface();
		}
	return;
	}

li_object *show_gdi_surface_null(li_object *o, li_environment *env)
    {
    return 0;
    }
class li_add_null_gdi_surface_class : public i4_init_class             
{                                                                  
  int init_type() { return I4_INIT_TYPE_LISP_FUNCTIONS; }          
  void init()  
      { 
      li_add_function("show_gdi_surface", show_gdi_surface_null);
      li_add_function("hide_gdi_surface", show_gdi_surface_null);
      }
} li_add_function_gdi_null;

//li_automatic_add_function(show_gdi_surface_null,"show_gdi_surface");
//li_automatic_add_function(show_gdi_surface_null,"hide_gdi_surface");



