/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "render/dx5/r1_dx5_texture.h"
#include "video/win32/dx5_error.h"
#include "video/win32/dx5.h"
#include "time/profile.h"
#include "image/image.h"
#include "render/r1_clip.h"
#include "render/r1_win.h"
#include "app/app.h"//for application object (contains reference to display)
#include "palette/pal.h"//for color conversion
#include "render/r1_res.h"

#include "JJ_Error.h"
#define USE_BUFFER  /* Use Buffer for Calls to DrawPrimitive*/
//But beware: This increases Speed but Requires Flushing the 
//Buffer when finished drawing.
void init_d3d5_vert_buffer();

r1_dx5_class r1_dx5_class_instance;
    
i4_profile_class pf_dx5_use_texture("dx5::use_texture");
i4_profile_class pf_dx5_vertex_setup("dx5::vertex_setup");
i4_profile_class pf_dx5_drawprimitive("dx5::drawprimitive");

CR1_dx5_render_window_class::~CR1_dx5_render_window_class()
{
}

CR1_dx5_render_window_class::CR1_dx5_render_window_class(w16 w, w16 h, r1_expand_type expand_type, r1_render_api_class *api)
    : r1_render_window_class(w,h, expand_type, api) {}



void r1_dx5_class::copy_part(i4_image_class *im,                                          
                            int x, int y,             // position on screen
                            int x1, int y1,           // area of image to copy 
                            int x2, int y2)
{
  
  DDSURFACEDESC ddsd;
  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);
  //dx5_common.back_surface->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_WRITEONLY,0);  
  dx5_common.back_surface->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_NOSYSLOCK,0);
  
  //get frame buffer pointer
  w8 *fb = (w8 *)ddsd.lpSurface;
  w8 *im_src = (w8 *)im->data + x + y*im->bpl;
  
  sw32 w_pitch = ddsd.lPitch; 
  sw32 im_width  = x2-x1+1;
  sw32 im_height = y2-y1+1;
  sw32 i;//,j;
  
  sw32 ix_off = i4_f_to_i(x_off);
  sw32 iy_off = i4_f_to_i(y_off);
  if (fb)
  {
    fb += ((x+ix_off)*2 + (y+iy_off)*w_pitch);
    for (i=0;i<im_height;i++)
    {
      memcpy(fb,im_src,im_width*2);
      im_src += im->width()*2;
      fb += w_pitch;
    }
  }
  dx5_common.back_surface->Unlock(NULL);
}


void CR1_dx5_render_window_class::draw(i4_draw_context_class &context)
{
  static i4_bool recursion=i4_F;
  r1_dx5_class_instance.x_off = context.xoff;
  r1_dx5_class_instance.y_off = context.yoff;
  
  if (!recursion)//if this is called recursively, ignore second call.
	  {//must not call BeginScene twice
	  recursion=i4_T;
	  //i4_warning("TRACE: At beginning of new frame");
	  if (r1_dx5_class_instance.zbuffer_surface->IsLost()==DDERR_SURFACELOST) 
		  {
		  i4_dx5_check(r1_dx5_class_instance.zbuffer_surface->Restore());
		  //i4_dx5_check(dx5_common.back_surface->Restore());
		  //i4_dx5_check(dx5_common.primary_surface->Restore());
		  recursion=i4_F;
		  request_redraw();
		  return;//skip current frame (might be other problems pending)
		  }
	  r1_dx5_class_instance.d3d_device->BeginScene();
      //api->set_z_range(0.01f, 1000.0f);
	  clip_with_z(context,RENDER_DEFAULT_NEAR_DISTANCE, RENDER_DEFAULT_FAR_DISTANCE);
	  
      //r1_dx5_class_instance.d3d_device->SetRenderState(0,0);
      r1_render_window_class::draw(context);  

      r1_dx5_class_instance.flush_vert_buffer();

      r1_dx5_class_instance.d3d_device->EndScene();  
      recursion=i4_F;
	  //i4_warning("TRACE: Frame rendered successfully");
	  }
  else
	  {
	  r1_render_window_class::draw(context);//the render context is already active.
	  r1_dx5_class_instance.flush_vert_buffer();
	  }
  //request_redraw(i4_T);//request redrawing of our childs for next frame
};

r1_render_window_class *r1_dx5_class::create_render_window(int visable_w, int visable_h,
                                                           r1_expand_type type)
{
  return new CR1_dx5_render_window_class(visable_w, visable_h, type, this);
}

void r1_dx5_class::set_write_mode(r1_write_mask_type mask)
{
  if (mask==get_write_mask()) return;

  states_have_changed = i4_T;
  flush_vert_buffer();  
  
  

  if (mask & R1_WRITE_W)
    d3d_device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,1);
  else
    d3d_device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,0);

  if (mask & R1_COMPARE_W)
    d3d_device->SetRenderState(D3DRENDERSTATE_ZFUNC,D3DCMP_GREATER);
  else
    d3d_device->SetRenderState(D3DRENDERSTATE_ZFUNC,D3DCMP_ALWAYS);

  r1_render_api_class::set_write_mode(mask);
}

void r1_dx5_class::set_alpha_mode(r1_alpha_type type)
{
  if (type==get_alpha_mode()) return;

  states_have_changed = i4_T;
  flush_vert_buffer();  

  switch (type)
  {
    case R1_ALPHA_DISABLED :
      d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
      break;

    case R1_ALPHA_CONSTANT :    // enable alpha
    case R1_ALPHA_LINEAR   :    // w/constant alpha, the constant alpha value is copied into the vertices
      d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
      d3d_device->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA);
      d3d_device->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
      break;    
  }

  r1_render_api_class::set_alpha_mode(type);
}

r1_dx5_class::r1_dx5_class()
{    
  zbuffer_surface     = 0;
  d3d                 = 0;
  d3d_device          = 0;
  d3d_viewport        = 0;

  render_device_flags = 0;

  strcpy(dd_driver_name,"display");
  strcpy(d3d_driver_name,"Direct3D HAL");  
  
  last_node = 0;
  
  texture_mode = i4_F;
  holy_mode    = i4_F;
  
  states_have_changed = i4_F;
}

r1_dx5_class::~r1_dx5_class()
{
	uninit();
}

i4_bool r1_dx5_class::init(i4_display_class *display)
{    
  HRESULT hResult = DD_OK;
  // JJ FOR TEST
  if (display!=i4_dx5_display)// || !i4_dx5_display->using_accelerated_driver())
    return i4_F;     
  
  set_color_tint(0);
  init_d3d5_vert_buffer();
  //dx5_d3d_info
  info=dx5_common.get_driver_hardware_info(dx5_common.ddraw,DRIVER_MODE_INIT);
  if (!info) return i4_F;

  if (!i4_dx5_check(dx5_common.ddraw->QueryInterface(IID_IDirect3D2,(void **)&d3d)))
    return i4_F;

  D3DDEVICEDESC hw_desc = info->hw_desc; 
  D3DDEVICEDESC sw_desc = info->sw_desc;
  
  if (hw_desc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
    needs_square_textures = i4_T;
  else
    needs_square_textures = i4_F;

  DDSURFACEDESC ddsd,pdesc;
  
  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  memset(&pdesc,0,sizeof(DDSURFACEDESC));
  pdesc.dwSize=sizeof(DDSURFACEDESC);
  ddsd.dwSize  = sizeof(DDSURFACEDESC);  
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_ZBUFFERBITDEPTH;

  dx5_common.back_surface->GetSurfaceDesc(&pdesc);
  //force the zbuffer to have the same size as the backbuffer surface (cannot enlarge later)
  //We have a problem if primary+backbuffer+zbuffer don't fit in vidmem
  //for the set gdi resolution in windowed mode. (like on my machine...)
  //I suggest a compromise: The resolution entered in settings becomes the
  //upper limit such that the system knows what will work.


  ddsd.dwWidth  = pdesc.dwWidth;//i4_dx5_display->current_mode()->xres;
  ddsd.dwHeight = pdesc.dwHeight;//i4_dx5_display->current_mode()->yres;
  //ddsd.dwWidth  = i4_dx5_display->current_mode()->xres;
  //ddsd.dwHeight = i4_dx5_display->current_mode()->yres;

  ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
  
  w32 bd = hw_desc.dwDeviceZBufferBitDepth;
  if (bd==0)
  {
    //zbuffer must be in system memory
    bd = sw_desc.dwDeviceZBufferBitDepth;
    ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    OutputDebugString("putting zbuffer in system memory\n");
  }
  else
  {
    //zbuffer must be in video memory
    ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    OutputDebugString("putting zbuffer in video memory\n");
  }
  // default !  
  ddsd.dwZBufferBitDepth = 16;   
  switch (bd)
  {
   case DDBD_8:  ddsd.dwZBufferBitDepth = 8; break;
   case DDBD_16: ddsd.dwZBufferBitDepth = 16; break;
   case DDBD_24: ddsd.dwZBufferBitDepth = 24; break;
   case DDBD_32: ddsd.dwZBufferBitDepth = 32; break;
  }

  // JJ >>  2000.04.12
  // Force 16-bit depth , because i can be 16 or 24 0200|0400 = 0600 !!!!!, so above switch wil do nothing
  if(DDBD_16&bd)
    ddsd.dwZBufferBitDepth = 16; 
  // JJ  

    IDirectDrawSurface *z_surf;  
    // <<JJ NEW CODE
    CHECK_DX(dx5_common.ddraw->CreateSurface(&ddsd, &z_surf,0));
    if(FAILED(hResult) )
        if (hResult==DDERR_OUTOFVIDEOMEMORY)
			{
			ddsd.ddsCaps.dwCaps=DDSCAPS_SYSTEMMEMORY|DDSCAPS_ZBUFFER;
			
			CHECK_DX(dx5_common.ddraw->CreateSurface(&ddsd,&z_surf,0));
			if(FAILED(hResult))
				return i4_F;
			}
			      
    // >> JJ NEW CODE 
  if (!i4_dx5_check(z_surf->QueryInterface(IID_IDirectDrawSurface3,(void **)&zbuffer_surface)))
  {
    z_surf->Release();
    return i4_F;
  }

  z_surf->Release();

  if (!i4_dx5_check(dx5_common.back_surface->AddAttachedSurface(zbuffer_surface)))
	  {//Try again, using Systemmemory (Attaching fails if Surface in Systemmemory, Zbuffer not)
	  zbuffer_surface->Release();
	  ddsd.ddsCaps.dwCaps=DDSCAPS_SYSTEMMEMORY|DDSCAPS_ZBUFFER;
	  if(!i4_dx5_check(dx5_common.ddraw->CreateSurface(&ddsd,&z_surf,0)))
				return i4_F;
	  if (!i4_dx5_check(z_surf->QueryInterface(IID_IDirectDrawSurface3,(void **)&zbuffer_surface)))
		  {
		  z_surf->Release();
		  return i4_F;
		  }
	  if (!i4_dx5_check(dx5_common.back_surface->AddAttachedSurface(zbuffer_surface)))
		  {
		  z_surf->Release();//Most probably, we don't have enough 
		  //video memory to create the zbuffer in vmem. But it must be there, if backbuffer is.
		  return i4_F;
		  }
	  z_surf->Release();
	  }

  IDirectDrawSurface *back_surf;
  dx5_common.back_surface->QueryInterface(IID_IDirectDrawSurface,(void **)&back_surf);  

  if (!i4_dx5_check(d3d->CreateDevice(*info->lpGuid, back_surf, &d3d_device)))
	  {
	  i4_dx5_check(dx5_common.back_surface->DeleteAttachedSurface(0,zbuffer_surface));
	  zbuffer_surface->Release();
	  ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY; 
	  ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	  CHECK_DX(dx5_common.ddraw->CreateSurface(&ddsd,&z_surf,0));
	  if (!i4_dx5_check(z_surf->QueryInterface(IID_IDirectDrawSurface3,(void **)&zbuffer_surface)))
		  {
		  z_surf->Release();
          return i4_F;
		  }
	  z_surf->Release();
	  i4_dx5_check(dx5_common.back_surface->AddAttachedSurface(zbuffer_surface));

	  if(!i4_dx5_check(d3d->CreateDevice(*info->lpSoftGuid,back_surf,
		  &d3d_device))) //IID_IDirect3DMMXDevice
		  {

		  return i4_F; 
		  }
	  }
  
  back_surf->Release();
     
  if (d3d->CreateViewport(&d3d_viewport,0) != D3D_OK)
    return i4_F;

  if (d3d_device->AddViewport(d3d_viewport) != D3D_OK)
    return i4_F;

  hardware_tmapping = i4_T;
  
  /*
  use_stipled_alpha = i4_F;
  if (hw_desc.dwShadeCaps & D3DSHADECAPS_ALPHAFLATSTIPPLED)
  {
    if (!(hw_desc.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND))
    {
      //this device doesnt support gouraud alpha blending, but it does
      //support flat stipled alpha, so lets at least use the flat
      //stipled alpha where possible
      use_stipled_alpha = i4_T;
    }
  }
  */

  D3DVIEWPORT2 viewport_desc;
  
  //float  aspect = (float)640/(float)480;
  float aspect=(float) pdesc.dwWidth/(float) pdesc.dwHeight;

  memset(&viewport_desc, 0, sizeof(D3DVIEWPORT2));  
  viewport_desc.dwSize       = sizeof(D3DVIEWPORT2);
  viewport_desc.dwX          = 0;
  viewport_desc.dwY          = 0;
  viewport_desc.dwWidth      = pdesc.dwWidth;
  viewport_desc.dwHeight     = pdesc.dwHeight;
  viewport_desc.dvClipX      = -1.0f;
  viewport_desc.dvClipY      = aspect;
  viewport_desc.dvClipWidth  = 2.0f;
  viewport_desc.dvClipHeight = 2.0f * aspect;
  viewport_desc.dvMinZ       = 0.f;
  viewport_desc.dvMaxZ       = 1.f;
  
  if (d3d_viewport->SetViewport2(&viewport_desc) != D3D_OK)
    return i4_F;

  if (d3d_device->SetCurrentViewport(d3d_viewport) != D3D_OK)
    return i4_F;

  //turn zbuffering on
  d3d_device->SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);   

  //turn gouraud shading on the entire time
  d3d_device->SetRenderState(D3DRENDERSTATE_SHADEMODE,D3DSHADE_GOURAUD);

  //dithering.. eh..
  d3d_device->SetRenderState(D3DRENDERSTATE_DITHERENABLE,1);
  
  //no monochomatic shading?
  d3d_device->SetRenderState(D3DRENDERSTATE_MONOENABLE,FALSE);
  
  //texturemapping mode setup
  d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
  d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,0);  
  
  d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS,D3DTADDRESS_CLAMP);
  d3d_device->SetRenderState(D3DRENDERSTATE_WRAPU,0);
  d3d_device->SetRenderState(D3DRENDERSTATE_WRAPV,0);

  d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE,1);
    
  //no automatic face culling
  d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);

  //setup other basic modes
  set_write_mode(R1_WRITE_W | R1_COMPARE_W | R1_WRITE_COLOR);
  
  set_shading_mode(R1_COLORED_SHADING);

  set_alpha_mode(R1_ALPHA_DISABLED);  
      
  set_filter_mode(R1_NO_FILTERING);

  set_z_range(0.01f,1.0f);

  m_dwFogColor           = 0x00b5b5ff;
  m_dwFogMode            = D3DFOG_LINEAR;
  m_fFogStartValue       = 0.2f;
  m_fFogEndValue         = 1.0f;
  m_fFogDensity          = 0.5f;
  m_bRangeBasedFog       = FALSE;


  //clear the viewport, initialize type thing
  //clear_area(0,0,639,479,0x00000000,1.0);
  clear_area(0,0,pdesc.dwWidth-1,pdesc.dwHeight-1,
	  0x0,RENDER_DEFAULT_FAR_DISTANCE);

  //if we're page flipping, flip so we can clear the other page initially as well
  i4_dx5_display->flush();

  //clear_area(0,0,639,479,0x00000000,1.0);  
  clear_area(0,0,pdesc.dwWidth-1,pdesc.dwHeight-1,0x0,RENDER_DEFAULT_FAR_DISTANCE);

  tmanager = new r1_dx5_texture_class(display->get_palette());
  tmanagers.reallocate(10,10);
  r1_name_cache_file("dx5");

  return i4_T;
}

void r1_dx5_class::set_fogging_mode(w32 fogcolor, i4_float startvalue, i4_float endvalue)
	{//doesn't work yet. Perhaps only sensefull on dx8.
	//currently just shows the entire scene in fogcolor.
	m_dwFogColor=fogcolor;
	m_fFogStartValue=startvalue;
	m_fFogEndValue=endvalue;
	if (fogcolor==0x0) 
		{
		d3d_device->SetRenderState(D3DRENDERSTATE_FOGENABLE,FALSE);
		return;
		}
	D3DMATRIX matrix1;//let's keep that one just in case.
	ZeroMemory(&matrix1,sizeof(matrix1));
	matrix1._11=1.0f;//ok... widht is..what is the near/far plane?
	matrix1._22=1.0f;//the near plane is 0.0(0?)1 and the far plane is 100
	matrix1._33=1.0f;//the width is 2*g1_render.center_x, the height 2*g1_render.center_y
	matrix1._43=4.0f; //should be a + number....from my tests
	matrix1._34=1.0f;//but msdn says this is positive, 43 might be negative... in the mcfog i send you negative doesn't work
	//ok, what do you suggest? +1? I've tried this, too. i used a 4*_33 value so in this case 4 but firts try witthout a table...compile now
    //the 11 and 22 are aspect ratio modifiers...so 1 1 is ok....\but i sugest we try on the smaller exampole()fog test(  it takes less time to see the modifications
	//ok, if it behaves as golg does...
	d3d_device->SetTransform(D3DTRANSFORMSTATE_PROJECTION,&matrix1);
	d3d_device->SetRenderState( D3DRENDERSTATE_FOGENABLE, TRUE );
    d3d_device->SetRenderState( D3DRENDERSTATE_FOGCOLOR,  m_dwFogColor );
      
    d3d_device->SetRenderState( D3DRENDERSTATE_FOGVERTEXMODE,  m_dwFogMode );
	d3d_device->SetRenderState( D3DRENDERSTATE_RANGEFOGENABLE, FALSE);
    //d3d_device->SetRenderState( D3DRENDERSTATE_FOGTABLEMODE,   m_dwFogMode );
	d3d_device->SetRenderState( D3DRENDERSTATE_FOGSTART,   FtoDW(m_fFogStartValue) );
    d3d_device->SetRenderState( D3DRENDERSTATE_FOGEND,     FtoDW(m_fFogEndValue) );
    d3d_device->SetRenderState( D3DRENDERSTATE_FOGDENSITY, FtoDW(m_fFogDensity) );
	}

i4_bool r1_dx5_class::resize(w16 newx, w16 newy)
	{//Seems to work without anything. (Be shure not to recreate
	//the directdraw interface and the primary and backbuffer surfaces.)
	HRESULT hResult = DD_OK;
  
	
  /*tmanager->next_frame();
  tmanager->reopen();
	if (d3d_viewport)
  {
    d3d_viewport->Release();
    d3d_viewport = 0;
  }

  if (d3d_device)
  {
    d3d_device->Release();
    d3d_device = 0;
  }

  
  //if (d3d)
  //{
  //  d3d->Release();  
  //  d3d = 0;
  //}

      
  
  //info is declared on class-level
  D3DDEVICEDESC hw_desc = info->hw_desc; 
  D3DDEVICEDESC sw_desc = info->sw_desc;
  
  if (hw_desc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
    needs_square_textures = i4_T;
  else
    needs_square_textures = i4_F;

  DDSURFACEDESC ddsd;
  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize  = sizeof(DDSURFACEDESC);  
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_ZBUFFERBITDEPTH;

  ddsd.dwWidth  = i4_dx5_display->current_mode()->xres;
  ddsd.dwHeight = i4_dx5_display->current_mode()->yres;
  ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
  
  w32 bd = hw_desc.dwDeviceZBufferBitDepth;
  if (bd==0)
  {
    //zbuffer must be in system memory
    bd = sw_desc.dwDeviceZBufferBitDepth;
    ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    OutputDebugString("putting zbuffer in system memory\n");
  }
  else
  {
    //zbuffer must be in video memory
    ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    OutputDebugString("putting zbuffer in video memory\n");
  }
  // default !  
  ddsd.dwZBufferBitDepth = 16;   
  switch (bd)
  {
   case DDBD_8:  ddsd.dwZBufferBitDepth = 8; break;
   case DDBD_16: ddsd.dwZBufferBitDepth = 16; break;
   case DDBD_24: ddsd.dwZBufferBitDepth = 24; break;
   case DDBD_32: ddsd.dwZBufferBitDepth = 32; break;
  }

  // JJ >>  2000.04.12
  // Force 16-bit depth , because it can be 16 or 24 0200|0400 = 0600 !!!!!, so above switch wil do nothing
  if(DDBD_16&bd)
    ddsd.dwZBufferBitDepth = 16; 
  // JJ  

    IDirectDrawSurface *z_surf;  
    // <<JJ NEW CODE
    CHECK_DX(dx5_common.ddraw->CreateSurface(&ddsd, &z_surf,0));
    if(FAILED(hResult) )
        if (hResult==DDERR_OUTOFVIDEOMEMORY)//Must check wheter this
			//can happen because the primary surface is now larger
			//if yes, we need to reload the tman (expensive)
			{
			ddsd.ddsCaps.dwCaps=DDSCAPS_SYSTEMMEMORY|DDSCAPS_ZBUFFER;
			
			CHECK_DX(dx5_common.ddraw->CreateSurface(&ddsd,&z_surf,0));
			if(FAILED(hResult))
				return i4_F;
			}
			      
    // >> JJ NEW CODE 
  if (!i4_dx5_check(z_surf->QueryInterface(IID_IDirectDrawSurface3,(void **)&zbuffer_surface)))
  {
    z_surf->Release();
    return i4_F;
  }

  z_surf->Release();

  if (!i4_dx5_check(dx5_common.back_surface->AddAttachedSurface(zbuffer_surface)))
	  {//Try again, using Systemmemory (Attaching fails if Surface in Systemmemory, Zbuffer not)
	  zbuffer_surface->Release();
	  ddsd.ddsCaps.dwCaps=DDSCAPS_SYSTEMMEMORY|DDSCAPS_ZBUFFER;
	  if(!i4_dx5_check(dx5_common.ddraw->CreateSurface(&ddsd,&z_surf,0)))
				return i4_F;
	  if (!i4_dx5_check(z_surf->QueryInterface(IID_IDirectDrawSurface3,(void **)&zbuffer_surface)))
		  {
		  z_surf->Release();
		  return i4_F;
		  }
	  if (!i4_dx5_check(dx5_common.back_surface->AddAttachedSurface(zbuffer_surface)))
		  return i4_F;
	  z_surf->Release();
	  }

  IDirectDrawSurface *back_surf;
  dx5_common.back_surface->QueryInterface(IID_IDirectDrawSurface,(void **)&back_surf);  

  if (!i4_dx5_check(d3d->CreateDevice(*info->lpGuid, back_surf, &d3d_device)))
	  {
	  if(!i4_dx5_check(d3d->CreateDevice(IID_IDirect3DMMXDevice,back_surf,
		  &d3d_device)))
		  {

		  return i4_F; 
		  }
	  }
  
  back_surf->Release();
     
  if (d3d->CreateViewport(&d3d_viewport,0) != D3D_OK)
    return i4_F;

  if (d3d_device->AddViewport(d3d_viewport) != D3D_OK)
    return i4_F;

  hardware_tmapping = i4_T;
  
  
  use_stipled_alpha = i4_F;
  if (hw_desc.dwShadeCaps & D3DSHADECAPS_ALPHAFLATSTIPPLED)
  {
    if (!(hw_desc.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND))
    {
      //this device doesnt support gouraud alpha blending, but it does
      //support flat stipled alpha, so lets at least use the flat
      //stipled alpha where possible
      use_stipled_alpha = i4_T;
    }
  }
  

  D3DVIEWPORT2 viewport_desc;
  
  float  aspect = (float)640/(float)480;

  memset(&viewport_desc, 0, sizeof(D3DVIEWPORT2));  
  viewport_desc.dwSize       = sizeof(D3DVIEWPORT2);
  viewport_desc.dwX          = 0;
  viewport_desc.dwY          = 0;
  viewport_desc.dwWidth      = 640;
  viewport_desc.dwHeight     = 480;
  viewport_desc.dvClipX      = -1.0f;
  viewport_desc.dvClipY      = aspect;
  viewport_desc.dvClipWidth  = 2.0f;
  viewport_desc.dvClipHeight = 2.0f * aspect;
  viewport_desc.dvMinZ       = 0.f;
  viewport_desc.dvMaxZ       = 1.f;
  
  if (d3d_viewport->SetViewport2(&viewport_desc) != D3D_OK)
    return i4_F;

  if (d3d_device->SetCurrentViewport(d3d_viewport) != D3D_OK)
    return i4_F;

  //turn zbuffering on
  d3d_device->SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);   

  //turn gouraud shading on the entire time
  d3d_device->SetRenderState(D3DRENDERSTATE_SHADEMODE,D3DSHADE_GOURAUD);

  //dithering.. eh..
  d3d_device->SetRenderState(D3DRENDERSTATE_DITHERENABLE,1);
  
  //no monochomatic shading?
  d3d_device->SetRenderState(D3DRENDERSTATE_MONOENABLE,FALSE);
  
  //texturemapping mode setup
  d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
  d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,0);  
  
  d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS,D3DTADDRESS_CLAMP);
  d3d_device->SetRenderState(D3DRENDERSTATE_WRAPU,0);
  d3d_device->SetRenderState(D3DRENDERSTATE_WRAPV,0);

  d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE,1);
    
  //no automatic face culling
  d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);

  //setup other basic modes
  set_write_mode(R1_WRITE_W | R1_COMPARE_W | R1_WRITE_COLOR);
  
  set_shading_mode(R1_COLORED_SHADING);

  set_alpha_mode(R1_ALPHA_DISABLED);  
      
  set_filter_mode(R1_NO_FILTERING);

  set_z_range(0.01f,1.0f);

  //clear the viewport, initialize type thing
  clear_area(0,0,639,479,0x00000000,1.0);

  //if we're page flipping, flip so we can clear the other page initially as well
  i4_dx5_display->flush();

  clear_area(0,0,639,479,0x00000000,1.0);  
  if (!tmanager->load_textures())
	  {
	i4_error("Texture reloading failed.");
	return i4_F;
	  }
*/
  return i4_T;	
	}


i4_bool r1_dx5_class::redepth(w16 new_bitdepth)
	{
	return i4_T;
	}

r1_texture_manager_class *r1_dx5_class::install_new_tmanager(w32& index)
	{
	/*if (!index)
		{
		return 0;
		//The caller must know it himself wheter the old one can be reopened
		//I'm not shure, but there shan't be a problem having both active at the same time.
		//(At least as long as the caller knows to which one he refers)
		//if (tmanager->textures_loaded) {tmanager->reopen();};
		}*/
	r1_texture_manager_class *new_tman=0;
	new_tman= new r1_dx5_texture_class(tmanager->get_pal());//use same parameters as default instance (must never be deleted)
	new_tman->is_master_tman=i4_F;
	//Reuse entries that might have been freed.
	for (int k=0;k<tmanagers.size();k++)
		{
		if (tmanagers[k]==0) 
			{
			tmanagers[k]=new_tman;
			index=k;
			return new_tman;
			}
		}
	index=tmanagers.add(new_tman);
	return new_tman;
	}

void r1_dx5_class::set_filter_mode(r1_filter_type type)
{
  if (type==R1_NO_FILTERING)
  {
    d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREMAG,D3DFILTER_LINEAR);//NEAREST);//D3DFILTER_LINEAR);    
    d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREMIN,D3DFILTER_LINEAR);//NEAREST);//D3DFILTER_LINEAR);
  }
  else
  if (type==R1_BILINEAR_FILTERING)
  {
    d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREMAG,D3DFILTER_LINEAR);    
    d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREMIN,D3DFILTER_LINEAR);
  }

  r1_render_api_class::set_filter_mode(type);
}

void r1_dx5_class::uninit()
{
  if (tmanager)
  {
    delete tmanager;
    tmanager = 0;  
  }

  if (d3d_viewport)
  {
    d3d_viewport->Release();
    d3d_viewport = 0;
  }

  if (d3d_device)
  {
    d3d_device->Release();
    d3d_device = 0;
  }

  if (d3d)
  {
    d3d->Release();  
    d3d = 0;
  }
  r1_render_api_class::uninit();
}

void r1_dx5_class::enable_holy()
{
  if (!holy_mode)
  {
    states_have_changed = i4_T;
    flush_vert_buffer();    

    pre_holy_alpha_mode = get_alpha_mode();
    pre_holy_write_mask = get_write_mask();

    d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATEALPHA);

    set_alpha_mode(R1_ALPHA_LINEAR);
    set_write_mode(R1_COMPARE_W);
    
    holy_mode = i4_T;    
  }
}

void r1_dx5_class::disable_holy()
{
  if (holy_mode)
  {    
    states_have_changed = i4_T;
    flush_vert_buffer();    
    
    d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
    
    set_alpha_mode(pre_holy_alpha_mode);
    set_write_mode(pre_holy_write_mask);    
    
    holy_mode = i4_F;    
  }
}

void r1_dx5_class::use_texture(w32 index,r1_texture_handle material_ref,sw32 desired_width, w32 frame)
{  
  pf_dx5_use_texture.start();

  if (!tmanagers[index]->valid_handle(material_ref))
  {
	//i4_warning("Tried to use a texture before tmanager::load_textures. Case 1. Material %i",material_ref);
    pf_dx5_use_texture.stop();
    disable_texture();
    return;
  }

  texture_mode = i4_T;

  sw32 width=0,height=0;    

  r1_miplevel_t *mip = tmanagers[index]->get_texture(material_ref, frame, desired_width, width, height);

  // don't select the texture again if it's the same one we used last time
  if (mip)
  {        
    i4_bool is_alpha = mip->entry->is_alphatexture();
    i4_bool is_holy  = mip->entry->is_transparent();

    if (is_alpha || is_holy)      
      enable_holy();      
    else
      disable_holy();

    if (mip != last_node)
    {
      states_have_changed = i4_T;
      flush_vert_buffer();

      last_node = mip;

      i4_float blahfloat_a, blahfloat_b;
      ((r1_dx5_texture_class *)tmanagers[index])->select_texture(mip->vram_handle, blahfloat_a, blahfloat_b);            
    }
  }
  
  pf_dx5_use_texture.stop();
}

void r1_dx5_class::use_texture(r1_texture_handle material_ref, sw32 desired_width, w32 frame)
{  
  pf_dx5_use_texture.start();

  if (!tmanager->valid_handle(material_ref))
  {
	//i4_warning("Tried to use a texture before tmanager::load_textures. Case 2.");
    pf_dx5_use_texture.stop();
    disable_texture();
    return;
  }

  texture_mode = i4_T;

  sw32 width=0,height=0;    

  r1_miplevel_t *mip = tmanager->get_texture(material_ref, frame, desired_width, width, height);

  // don't select the texture again if it's the same one we used last time
  if (mip)
  {        
    i4_bool is_alpha = mip->entry->is_alphatexture();
    i4_bool is_holy  = mip->entry->is_transparent();

    if (is_alpha || is_holy)      
      enable_holy();      
    else
      disable_holy();

    if (mip != last_node)
    {
      states_have_changed = i4_T;
      flush_vert_buffer();

      last_node = mip;

      i4_float blahfloat_a, blahfloat_b;
      ((r1_dx5_texture_class *)tmanager)->select_texture(mip->vram_handle, blahfloat_a, blahfloat_b);            
    }
  }
  
  pf_dx5_use_texture.stop();
}

// drawing will the constant color to render with if textures are disabled
void r1_dx5_class::disable_texture()
{
  if (texture_mode)
  {
    states_have_changed = i4_T;
    flush_vert_buffer();

    d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,0);
    texture_mode = i4_F;
    last_node    = 0;
    
    disable_holy();
  }  
}

static i4_float f_0_1_to_i_0_255_255 = 255.f;

sw32 inline f_0_1_to_i_0_255(float f)
{
  sw32 res;
  __asm
  {
    fld f
    fld f_0_1_to_i_0_255_255
    fmul
    fistp res
  }
  return res;
}

i4_float inline i_to_f(sw32 i)
{
  i4_float res;
  __asm
  {
    fild i
    fstp res
  }
  return res;
}

w32 inline make_d3d_color(i4_float r, i4_float g, i4_float b)
{
  return (f_0_1_to_i_0_255(r) << 16) |
         (f_0_1_to_i_0_255(g) <<  8) |
         (f_0_1_to_i_0_255(b) <<  0);
}
 
static float dx5_z_scale, dx5_z_add, dx5_w_scale, dx5_w_add, dx5_near_z, dx5_far_z;


void r1_dx5_class::set_z_range(i4_float near_z, i4_float far_z)
{
  dx5_near_z = near_z;
  dx5_far_z  = far_z;

  dx5_z_scale = 0.999f / far_z;
  dx5_w_scale = near_z * 0.999f;
  dx5_z_add   = 0;//.01;
  dx5_w_add   = 0;//.01;

  r1_near_clip_z = near_z;
  r1_far_clip_z  = far_z;
}

inline void make_d3d5_verts(D3DTLVERTEX *dxverts,r1_vert *r1verts,r1_dx5_class *c,int total)
{                                                                        
  int i;  
    
  D3DTLVERTEX *dx_v = dxverts;
  r1_vert     *r1_v = r1verts;

  //add vertex information
  for (i=0;i<total;i++)
  {
    dx_v->sx       = r1_v->px + c->x_off;
    dx_v->sy       = r1_v->py + c->y_off;
                                    
    dx_v->sz       = r1_v->w * dx5_w_scale;

    if (dx_v->sz<0)
    {
      //i4_warning("dx vert z too small");
      dx_v->sz=0;
    }
    else
    if (dx_v->sz>1)
    {
      //i4_warning("dx vert z too large");
      dx_v->sz=1;
    }

    dx_v->rhw = r1_v->w;
    
    dx_v++;
    r1_v++;
  }

  //add texture information
  if (c->texture_mode)
  {
    dx_v = dxverts;
    r1_v = r1verts;
    
    for (i=0;i<total;i++)
    {
      dx_v->tu = r1_v->s;
      dx_v->tv = r1_v->t;

      if (dx_v->tu > 1) dx_v->tu = 1;
      else
      if (dx_v->tu < 0) dx_v->tu = 0;

      if (dx_v->tv > 1) dx_v->tv = 1;
      else
      if (dx_v->tv < 0) dx_v->tv = 0;

      dx_v++;
      r1_v++;
    }
  }
    
  w32 ccolor;
  w32 red;
  w32 alpha;

  dx_v->specular=0;
  //add color information
  switch (c->shade_mode)                                                    
  {                                                                      
    /* routines will draw fullbright */
    case R1_SHADE_DISABLED:
      dx_v = dxverts;
      r1_v = r1verts;
      ccolor = 0x00FFFFFF;            
      for (i=0;i<total;i++)
      {
        dx_v->color    = ccolor;
        dx_v++;
        r1_v++;
      }
      break;
    /* routines will use the constant color*/
    case R1_CONSTANT_SHADING:
      dx_v = dxverts;
      r1_v = r1verts;
      ccolor = c->const_color;
            
      for (i=0;i<total;i++)
      {
        dx_v->color = ccolor;
        dx_v++;
        r1_v++;
      }
      break;
                                                                         
    /* routines will use only the red component (as white) */            
    case R1_WHITE_SHADING:                                               
      dx_v = dxverts;
      r1_v = r1verts;      
      
      for (i=0;i<total;i++)
      {        
        red = f_0_1_to_i_0_255(r1_v->r);
        
        dx_v->color = (red<<16) | (red<<8) | (red);
        dx_v++;
        r1_v++;
      }      
      break;                                                             
                                                                         
    /* routines will use r,g, and b*/
    case R1_COLORED_SHADING:
      dx_v = dxverts;
      r1_v = r1verts;      
      
      for (i=0;i<total;i++)
      {        
        dx_v->color = make_d3d_color(r1_v->r,r1_v->g,r1_v->b);
        
        dx_v++;
        r1_v++;
      }      
      break;
  }    
                                                                         
  //add alpha - check constant alpha 1st
  if ((c->alpha_mode & R1_ALPHA_CONSTANT) && !(c->shade_mode==R1_SHADE_DISABLED))
  {
    dx_v = dxverts;
    r1_v = r1verts;          

    alpha = c->const_color & 0xFF000000;

    for (i=0;i<total;i++)
    {        
      dx_v->color |= alpha;
        
      dx_v++;
      r1_v++;
    }    
  }
  else
  if (c->alpha_mode & R1_ALPHA_LINEAR)
  {
    dx_v = dxverts;
    r1_v = r1verts;    

    for (i=0;i<total;i++)
    {        
	  //The following line may have a bug:
	  //Explosions are not drawn with correct transparency.
      dx_v->color |= (f_0_1_to_i_0_255(r1_v->a) << 24);
        
      dx_v++;
      r1_v++;
    }
  }  
}                                                                        

sw32 used_verts   = 0;
sw32 used_indices = 0;

#define DX5_VERT_BUF_SIZE  64
#define DX5_INDEX_BUF_SIZE 64

D3DTLVERTEX r1_dx5_tmp_verts[DX5_VERT_BUF_SIZE];
WORD        r1_dx5_tmp_indices[DX5_INDEX_BUF_SIZE];

void init_d3d5_vert_buffer()
{
  sw32 i;
  for (i=0; i<DX5_VERT_BUF_SIZE; i++)
  {
    r1_dx5_tmp_verts[i].specular = 0;
  }
  
  used_verts   = 0;
  used_indices = 0;
}

void r1_dx5_class::flush_vert_buffer()
{
#ifdef USE_BUFFER
  if (states_have_changed && (used_verts!=0))
  //if (used_verts>0)
  {
    pf_dx5_drawprimitive.start();
    d3d_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
                                     D3DVT_TLVERTEX,
                                     (void *)r1_dx5_tmp_verts,
                                     used_verts,
                                     r1_dx5_tmp_indices,
                                     used_indices,
                                     D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);  
    pf_dx5_drawprimitive.stop();
    
    used_verts   = 0;
    used_indices = 0;

    states_have_changed = i4_F;
  }
#endif
}

void r1_dx5_class::render_poly(int t_verts, r1_vert *verts)
{   
  if (t_verts > DX5_VERT_BUF_SIZE)  return;

#ifdef USE_BUFFER
  sw32 t_index = (t_verts-2)*3;
    
  if (t_index > DX5_INDEX_BUF_SIZE) return;
  
  if (t_verts + used_verts   > DX5_VERT_BUF_SIZE ||
      t_index + used_indices > DX5_INDEX_BUF_SIZE)
  {
    states_have_changed = i4_T;
    flush_vert_buffer();
  }
  
#endif

  sw32 i;

  pf_dx5_vertex_setup.start();
  if (color_tint_on)
  {
    for (i=0; i<t_verts; i++)
    {
      verts[i].r *= r_tint_mul;
      verts[i].g *= g_tint_mul;
      verts[i].b *= b_tint_mul;
    }
  }
  
  make_d3d5_verts(r1_dx5_tmp_verts+used_verts,verts,this,t_verts);
  pf_dx5_vertex_setup.stop();

#ifdef USE_BUFFER  
  
  //setup indices
  for (i=1; i<t_verts-1; i++)
  {
    r1_dx5_tmp_indices[used_indices]   = (w16)(used_verts);

    r1_dx5_tmp_indices[used_indices+1] = (w16)(used_verts+i);

    r1_dx5_tmp_indices[used_indices+2] = (w16)(used_verts+i+1);
    used_indices += 3;
  }

  used_verts += t_verts;

#else
  
  pf_dx5_drawprimitive.start();

  d3d_device->DrawPrimitive(D3DPT_TRIANGLEFAN,
                            D3DVT_TLVERTEX,
                            (void *)r1_dx5_tmp_verts,
                            t_verts,
                            D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);

  pf_dx5_drawprimitive.stop();

#endif
}

void r1_dx5_class::render_pixel(int t_points, r1_vert *pixel)
{
  if (t_points>256) return;
  D3DTLVERTEX dx_verts[256];
  make_d3d5_verts(dx_verts,pixel,this,t_points);
  d3d_device->DrawPrimitive(D3DPT_POINTLIST,
	  D3DVT_TLVERTEX,
	  (LPVOID) dx_verts,
	  t_points,
	  0);
}

void r1_dx5_class::render_lines(int t_lines, r1_vert *verts)
{
  if (t_lines+1>256) return;
  D3DTLVERTEX dx_verts[256];

  sw32 i;

  //for (i=0;i<t_lines+1;i++)
  //{
    make_d3d5_verts(dx_verts,verts,this,t_lines+1);
	//either code the loop or use t_lines verts, but not both
  //}

  for (i=0;i<t_lines;i++)
  {
    d3d_device->DrawPrimitive(D3DPT_LINELIST,
                              D3DVT_TLVERTEX,
                              (void *)&dx_verts[i],
                              2,
                              D3DDP_DONOTCLIP);  
  }
}

void r1_dx5_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
	{
	float ooz=r1_ooz(z);
	
	if ((abs(x2-x1)<10) && (abs(y2-y1)<10))
		{
		//disable_texture();
		//w32 color=i4_pal_man.convert_32_to(lcolor,&(i4_current_app->get_display()->get_palette()->source));
		
		D3DTLVERTEX dx_v[4];
		memset(dx_v,0,4*sizeof(D3DTLVERTEX));
		float oozw=ooz*dx5_w_scale;
		dx_v[0].sx=  x2+x_off;
		dx_v[0].sy=  y1+y_off;
		dx_v[0].sz=  oozw;
		dx_v[0].rhw= ooz;
		dx_v[0].color=color;
		
		dx_v[1].sx=  x1+x_off;
		dx_v[1].sy=  y1+y_off;
		dx_v[1].sz=  oozw;
		dx_v[1].rhw=ooz;
		dx_v[1].color=color;
		
		dx_v[2].sx=  x1+x_off;
		dx_v[2].sy=  y2+y_off;
		dx_v[2].sz=  oozw;
		dx_v[2].rhw=ooz;
		dx_v[2].color=color;
		
		dx_v[3].sx=  x2+x_off;
		dx_v[3].sy=  y2+y_off;
		dx_v[3].sz=  oozw;
		dx_v[3].rhw=ooz;
		dx_v[3].color=color;
		d3d_device->DrawPrimitive(D3DPT_TRIANGLEFAN,
			D3DVT_TLVERTEX,
			(LPVOID) dx_v,
			4,
			D3DDP_DONOTCLIP);
		}
	else
		{
		RECT    rect;
		HRESULT res;
		
		rect.left   = (long)(x_off + x1);
		rect.top    = (long)(y_off + y1);  
		rect.right  = (long)(x_off + x2+1);
		rect.bottom = (long)(y_off + y2+1);
		
		DDBLTFX fx;
		memset(&fx,0,sizeof(DDBLTFX));
		fx.dwSize = sizeof(DDBLTFX);
		
		if (write_mask & R1_WRITE_COLOR)
			{
			
			//fx.dwFillColor = color;
			fx.dwFillColor=i4_pal_man.convert_32_to(color,&(i4_current_app->get_display()->get_palette()->source));
			//need a method here that converts 32bit ARGB values to 
			//what the current video mode desires.
			//DDSURFACEDESC ddsd;
			//dx5_common.get_surface_description(dx5_common.back_surface,&ddsd);
			//ddsd.ddpfPixelFormat.dwGBitMask
			
			dx5_common.back_surface->Blt(&rect,NULL,NULL,DDBLT_COLORFILL,&fx);
			}
		
		if (write_mask & R1_WRITE_W)
			{
			if (z == dx5_near_z)
				{  
				fx.dwFillDepth = 0xFFFF;
				}
			else
				if (z == dx5_far_z)
					{
					fx.dwFillDepth = 0;
					}
				else
					fx.dwFillDepth = i4_f_to_i((1.f/(float)z) * dx5_w_scale * (float)0xFFFF);
				
				res = zbuffer_surface->Blt(&rect,NULL,NULL,DDBLT_DEPTHFILL,&fx);
			}
		}
	};

/*
//Let's see wheter we can do this a little bit faster
//Blt() is actually a very slow function if used on small rectagles
void r1_dx5_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
{  
  RECT    rect;
  HRESULT res;
  
  rect.left   = (long)(x_off + x1);
  rect.top    = (long)(y_off + y1);  
  rect.right  = (long)(x_off + x2+1);
  rect.bottom = (long)(y_off + y2+1);

  DDBLTFX fx;
  memset(&fx,0,sizeof(DDBLTFX));
  fx.dwSize = sizeof(DDBLTFX);
    
  if (write_mask & R1_WRITE_COLOR)
  {
    
    //fx.dwFillColor = color;
	fx.dwFillColor=i4_pal_man.convert_32_to(color,&(i4_current_app->get_display()->get_palette()->source));
	//need a method here that converts 32bit ARGB values to 
	//what the current video mode desires.
	//DDSURFACEDESC ddsd;
	//dx5_common.get_surface_description(dx5_common.back_surface,&ddsd);
	//ddsd.ddpfPixelFormat.dwGBitMask

    dx5_common.back_surface->Blt(&rect,NULL,NULL,DDBLT_COLORFILL,&fx);
  }

  if (write_mask & R1_WRITE_W)
  {
    if (z == dx5_near_z)
    {  
      fx.dwFillDepth = 0xFFFF;
    }
    else
    if (z == dx5_far_z)
    {
      fx.dwFillDepth = 0;
    }
    else
      fx.dwFillDepth = i4_f_to_i((1.f/(float)z) * dx5_w_scale * (float)0xFFFF);

    res = zbuffer_surface->Blt(&rect,NULL,NULL,DDBLT_DEPTHFILL,&fx);
  }
}*/

i4_image_class *r1_dx5_class::create_compatible_image(w16 w, w16 h)
{  
  return i4_create_image(w,h,i4_dx5_display->get_palette());
}


// void r1_dx5_class::copy_part(i4_image_class *im,                                          
//                             int x, int y,             // position on screen
//                             int x1, int y1,           // area of image to copy 
//                             int x2, int y2)
// {
  
//   DDSURFACEDESC ddsd;
//   memset(&ddsd,0,sizeof(DDSURFACEDESC));
//   ddsd.dwSize = sizeof(DDSURFACEDESC);
//   dx5_common.back_surface->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_WRITEONLY,0);  
  
//   //get frame buffer pointer
//   w8 *fb = (w8 *)ddsd.lpSurface;
//   w8 *im_src = (w8 *)im->data + x + y*im->bpl;
  
//   sw32 w_pitch = ddsd.lPitch; 
//   sw32 im_width  = x2-x1+1;
//   sw32 im_height = y2-y1+1;
//   sw32 i,j;
  
//   sw32 ix_off = i4_f_to_i(x_off);
//   sw32 iy_off = i4_f_to_i(y_off);
//   if (fb)
//   {
//     fb += ((x+ix_off)*2 + (y+iy_off)*w_pitch);
//     for (i=0;i<im_height;i++)
//     {
//       memcpy(fb,im_src,im_width*2);
//       im_src += im->width()*2;
//       fb += w_pitch;
//     }
//   }
//   dx5_common.back_surface->Unlock(NULL);
// }
