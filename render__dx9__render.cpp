/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "render/dx9/texture.h"
#include "video/win32/dx9_error.h"
#include "video/win32/dx9.h"
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
void init_d3d_vert_buffer();

r1_dx9_class r1_dx9_class_instance;
    
i4_profile_class pf_dx9_use_texture("dx9::use_texture");
//i4_profile_class pf_dx9_vertex_setup("dx9::vertex_setup");
i4_profile_class pf_dx9_drawprimitive("dx9::drawprimitive");

CR1_dx9_render_window_class::~CR1_dx9_render_window_class()
{
}

CR1_dx9_render_window_class::CR1_dx9_render_window_class(w16 w, w16 h, r1_expand_type expand_type, r1_render_api_class *api)
    : r1_render_window_class(w,h, expand_type, api) {}



void r1_dx9_class::copy_part(i4_image_class *im,                                          
                            int x, int y,             // position on screen
                            int x1, int y1,           // area of image to copy 
                            int x2, int y2)
{
  
  //DDSURFACEDESC ddsd;
  //memset(&ddsd,0,sizeof(DDSURFACEDESC));
  //ddsd.dwSize = sizeof(DDSURFACEDESC);
  //dx9_common.back_surface->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_WRITEONLY,0);  
  //dx9_common.back_surface->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_NOSYSLOCK,0);
	D3DLOCKED_RECT lockinfo;
	dx9_common.back_surface->LockRect(&lockinfo,NULL,D3DLOCK_NOSYSLOCK);
  
  //get frame buffer pointer
  w8 *fb = (w8 *)lockinfo.pBits;
  w8 *im_src = (w8 *)im->data + x + y*im->bpl;
  
  sw32 w_pitch = lockinfo.Pitch;
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
  dx9_common.back_surface->UnlockRect();
}


void CR1_dx9_render_window_class::draw(i4_draw_context_class &context)
{
  static i4_bool recursion=i4_F;
  r1_dx9_class_instance.x_off = context.xoff;
  r1_dx9_class_instance.y_off = context.yoff;
  
  if (!recursion)//if this is called recursively, ignore second call.
	  {//must not call BeginScene twice
	  recursion=i4_T;
	  //i4_warning("TRACE: At beginning of new frame");
	  if (dx9_common.device->TestCooperativeLevel()!=D3D_OK) 
		  {
		  
		  recursion=i4_F;
		  request_redraw();
		  return;//skip current frame (might be other problems pending)
		  }
	  dx9_common.device->BeginScene();
      //api->set_z_range(0.01f, 1000.0f);
	  clip_with_z(context,RENDER_DEFAULT_NEAR_DISTANCE, RENDER_DEFAULT_FAR_DISTANCE);
	  
      //r1_dx9_class_instance.d3d_device->SetRenderState(0,0);
      r1_render_window_class::draw(context);  

      r1_dx9_class_instance.flush_vert_buffer();

      //r1_dx9_class_instance.d3d_device->EndScene();  
	  dx9_common.device->EndScene();
      recursion=i4_F;
	  //i4_warning("TRACE: Frame rendered successfully");
	  }
  else
	  {
	  r1_render_window_class::draw(context);//the render context is already active.
	  r1_dx9_class_instance.flush_vert_buffer();
	  }
  //request_redraw(i4_T);//request redrawing of our childs for next frame
};

r1_render_window_class *r1_dx9_class::create_render_window(int visable_w, int visable_h,
                                                           r1_expand_type type)
{
  return new CR1_dx9_render_window_class(visable_w, visable_h, type, this);
}

void r1_dx9_class::set_write_mode(r1_write_mask_type mask)
{
  if (mask==get_write_mask()) return;

  states_have_changed = i4_T;
  flush_vert_buffer();  
  
  
  if (mask & R1_WRITE_W)
    d3d_device->SetRenderState(D3DRS_ZWRITEENABLE,1);
  else
    d3d_device->SetRenderState(D3DRS_ZWRITEENABLE,0);

  if (mask & R1_COMPARE_W)
  {
	  UINT cmpfn=D3DCMP_GREATER;
    d3d_device->SetRenderState(D3DRS_ZFUNC,cmpfn);
  }
  else
    d3d_device->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);

  r1_render_api_class::set_write_mode(mask);
}

void r1_dx9_class::set_alpha_mode(r1_alpha_type type)
{
  if (type==get_alpha_mode()) return;

  states_have_changed = i4_T;
  flush_vert_buffer();  

  switch (type)
  {
    case R1_ALPHA_DISABLED :
      d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	  d3d_device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
      break;

    case R1_ALPHA_CONSTANT :    // enable alpha
    case R1_ALPHA_LINEAR   :    // w/constant alpha, the constant alpha value is copied into the vertices
      d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
      d3d_device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
      d3d_device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	  d3d_device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	  d3d_device->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	  d3d_device->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_CURRENT);
      break;    
  }

  r1_render_api_class::set_alpha_mode(type);
}

r1_dx9_class::r1_dx9_class()
{    
  //zbuffer_surface     = 0;
  //d3d                 = 0;
  d3d_device          = 0;
  //d3d_viewport        = 0;

  render_device_flags = 0;

  strcpy(dd_driver_name,"display");
  strcpy(d3d_driver_name,"Direct3D HAL");  
  
  last_node = 0;
  
  texture_mode = i4_F;
  holy_mode    = i4_F;
  
  states_have_changed = i4_F;
}

r1_dx9_class::~r1_dx9_class()
{
	uninit();
}

i4_bool r1_dx9_class::reinit()
{
	HRESULT rr=d3d_device->TestCooperativeLevel();
	if (rr==D3D_OK)
		return i4_T;
	if (rr==D3DERR_DEVICELOST)
	{
		i4_warning("WARNING: D3D Device lost, cannot restore now.");
		return i4_F;
	}
	if (rr!=D3DERR_DEVICENOTRESET)
	{
		i4_error("ERROR: TestCooperativeLevel() returned a weird status.");
		return i4_F;
	}
	if (dx9_common.back_surface)
	{
		dx9_common.back_surface->Release();
		dx9_common.back_surface=0;
	};
	if (!i4_dx9_check(d3d_device->Reset(&dx9_common.present)))
	{
		i4_warning("ERROR: Could not reset the device. Some objects might still depend on it");
		return i4_F;
	}
	dx9_common.create_surface(DX9_BACKBUFFERED_PRIMARY_SURFACE);
	d3d_device->SetRenderState(D3DRS_DITHERENABLE,TRUE);
	dx9_common.device->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_GOURAUD);
	dx9_common.device->SetRenderState(D3DRS_ZENABLE,TRUE);
	dx9_common.device->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	dx9_common.device->SetFVF(FVF_CUSTOMVERTEX);
	
    dx9_common.device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
    dx9_common.device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	dx9_common.device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	dx9_common.device->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    dx9_common.device->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 
	              D3DTSS_TCI_CAMERASPACEPOSITION );
    dx9_common.device->SetRenderState(D3DRS_LIGHTING,FALSE);
    dx9_common.device->SetRenderState(D3DRS_SPECULARENABLE,FALSE);
	default_state();
    return i4_T;		
}
i4_bool r1_dx9_class::init(i4_display_class *display)
{    
  HRESULT hResult = DD_OK;
  // JJ FOR TEST
  if (display!=i4_dx9_display)// || !i4_dx9_display->using_accelerated_driver())
    return i4_F;     
  d3d_device=dx9_common.device;
  set_color_tint(0);
  init_d3d_vert_buffer();
  D3DDeviceInfo *info;
  info=dx9_common.get_driver_hardware_info(dx9_common.pD3D9,i4_dx9_display->cur_mode.adaptor_id);
  if (!info) return i4_F;

  
  
  if (info->Caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
    needs_square_textures = i4_T;
  else
    needs_square_textures = i4_F;

  
  //turn zbuffering on
  i4_dx9_check(d3d_device->SetRenderState(D3DRS_ZENABLE,TRUE));

  //turn gouraud shading on the entire time
  i4_dx9_check(d3d_device->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_GOURAUD));

  //dithering.. eh..
  i4_dx9_check(d3d_device->SetRenderState(D3DRS_DITHERENABLE,TRUE));
  
  //no monochomatic shading?
  //d3d_device->SetRenderState(D3DRS_MONOENABLE,FALSE);
  
  //texturemapping mode setup
  //d3d_device->SetRenderState(D3DRS_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
  //d3d_device->SetRenderState(D3DRS_TEXTUREHANDLE,0);  
  
  //This is the default anyway
  //d3d_device->SetRenderState(D3DRS_TEXTUREADDRESS,D3DTADDRESS_CLAMP);
  //d3d_device->SetRenderState(D3DRS_WRAPU,0);
  //d3d_device->SetRenderState(D3DRS_WRAPV,0);

  //d3d_device->SetRenderState(D3DRS_TEXTUREPERSPECTIVE,1);
    
  //no automatic face culling
  i4_dx9_check(d3d_device->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE));

  i4_dx9_check(d3d_device->SetFVF(FVF_CUSTOMVERTEX));
  //setup other basic modes
  set_write_mode(R1_WRITE_W | R1_COMPARE_W | R1_WRITE_COLOR);

  //Duh, I would need this, but it's not supported on my hw... 
  //What else can we do?
  //Hm... the hw supports it, but the driver says it doesn't, so
  //we have a lot of false errors. 
  i4_dx9_check(d3d_device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE));
  //d3d_device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);
  //i4_dx9_check(d3d_device->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
  d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
  d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
  d3d_device->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
  d3d_device->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 
	              D3DTSS_TCI_CAMERASPACEPOSITION );
  d3d_device->SetRenderState(D3DRS_LIGHTING,FALSE);
  d3d_device->SetRenderState(D3DRS_SPECULARENABLE,FALSE);
  
  set_shading_mode(R1_COLORED_SHADING);

  set_alpha_mode(R1_ALPHA_DISABLED);  
      
  set_filter_mode(R1_BILINEAR_FILTERING);

  set_z_range(0.01f,1.0f);

  m_dwFogColor           = 0x00b5b5ff;
  m_dwFogMode            = D3DFOG_LINEAR;
  m_fFogStartValue       = 0.2f;
  m_fFogEndValue         = 1.0f;
  m_fFogDensity          = 0.5f;
  m_bRangeBasedFog       = FALSE;


  //clear the viewport, initialize type thing
  //clear_area(0,0,639,479,0x00000000,1.0);
  int w=i4_dx9_display->cur_mode.xres;
  int h=i4_dx9_display->cur_mode.yres;
  clear_area(0,0,w-1,h-1,0x0,RENDER_DEFAULT_FAR_DISTANCE);

  //if we're page flipping, flip so we can clear the other page initially as well
  i4_dx9_display->flush();

  //clear_area(0,0,639,479,0x00000000,1.0);  
  clear_area(0,0,w-1,h-1,0x0,RENDER_DEFAULT_FAR_DISTANCE);

  tmanager = new r1_dx9_texture_class(display->get_palette());
  tmanagers.reallocate(10,10);
  r1_name_cache_file("dx9");
  default_state();
  return i4_T;
}

void r1_dx9_class::set_fogging_mode(w32 fogcolor, i4_float startvalue, i4_float endvalue)
	{//doesn't work yet. Perhaps only sensefull on dx8.
	//currently just shows the entire scene in fogcolor.
	m_dwFogColor=fogcolor;
	m_fFogStartValue=startvalue;
	m_fFogEndValue=endvalue;
	if (fogcolor==0x0) 
		{
		d3d_device->SetRenderState(D3DRS_FOGENABLE,FALSE);
		return;
		}
	D3DMATRIX matrix1;//let's keep that one just in case.
	ZeroMemory(&matrix1,sizeof(matrix1));
	matrix1._11=1.0f;//ok... widht is..what is the near/far plane?
	matrix1._22=1.0f;//the near plane is 0.0(0?)1 and the far plane is 100
	matrix1._33=1.0f;//the width is 2*g1_render.center_x, the height 2*g1_render.center_y
	matrix1._43=4.0f; //should be a + number....from my tests
	matrix1._34=1.0f;//but msdn says this is positive, 
	//43 might be negative... in the mcfog i send you negative doesn't work
	//ok, what do you suggest? +1? I've tried this, too. 
	//i used a 4*_33 value so in this case 4 but firts try witthout a table...
	//compile now
    //the 11 and 22 are aspect ratio modifiers...so 1 1 is ok....
	//but i sugest we try on the smaller exampole()fog test(  it takes less time to see the modifications
	//ok, if it behaves as golg does...
	d3d_device->SetTransform(D3DTS_PROJECTION,&matrix1);
	d3d_device->SetRenderState( D3DRS_FOGENABLE, TRUE );
    d3d_device->SetRenderState( D3DRS_FOGCOLOR,  m_dwFogColor );
      
    d3d_device->SetRenderState( D3DRS_FOGVERTEXMODE,  m_dwFogMode );
	d3d_device->SetRenderState( D3DRS_RANGEFOGENABLE, FALSE);
    //d3d_device->SetRenderState( D3DRS_FOGTABLEMODE,   m_dwFogMode );
	d3d_device->SetRenderState( D3DRS_FOGSTART,   FtoDW(m_fFogStartValue) );
    d3d_device->SetRenderState( D3DRS_FOGEND,     FtoDW(m_fFogEndValue) );
    d3d_device->SetRenderState( D3DRS_FOGDENSITY, FtoDW(m_fFogDensity) );
	}

i4_bool r1_dx9_class::resize(w16 newx, w16 newy)
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

  ddsd.dwWidth  = i4_dx9_display->current_mode()->xres;
  ddsd.dwHeight = i4_dx9_display->current_mode()->yres;
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
    CHECK_DX(dx9_common.ddraw->CreateSurface(&ddsd, &z_surf,0));
    if(FAILED(hResult) )
        if (hResult==DDERR_OUTOFVIDEOMEMORY)//Must check wheter this
			//can happen because the primary surface is now larger
			//if yes, we need to reload the tman (expensive)
			{
			ddsd.ddsCaps.dwCaps=DDSCAPS_SYSTEMMEMORY|DDSCAPS_ZBUFFER;
			
			CHECK_DX(dx9_common.ddraw->CreateSurface(&ddsd,&z_surf,0));
			if(FAILED(hResult))
				return i4_F;
			}
			      
    // >> JJ NEW CODE 
  if (!i4_dx9_check(z_surf->QueryInterface(IID_IDirectDrawSurface3,(void **)&zbuffer_surface)))
  {
    z_surf->Release();
    return i4_F;
  }

  z_surf->Release();

  if (!i4_dx9_check(dx9_common.back_surface->AddAttachedSurface(zbuffer_surface)))
	  {//Try again, using Systemmemory (Attaching fails if Surface in Systemmemory, Zbuffer not)
	  zbuffer_surface->Release();
	  ddsd.ddsCaps.dwCaps=DDSCAPS_SYSTEMMEMORY|DDSCAPS_ZBUFFER;
	  if(!i4_dx9_check(dx9_common.ddraw->CreateSurface(&ddsd,&z_surf,0)))
				return i4_F;
	  if (!i4_dx9_check(z_surf->QueryInterface(IID_IDirectDrawSurface3,(void **)&zbuffer_surface)))
		  {
		  z_surf->Release();
		  return i4_F;
		  }
	  if (!i4_dx9_check(dx9_common.back_surface->AddAttachedSurface(zbuffer_surface)))
		  return i4_F;
	  z_surf->Release();
	  }

  IDirectDrawSurface *back_surf;
  dx9_common.back_surface->QueryInterface(IID_IDirectDrawSurface,(void **)&back_surf);  

  if (!i4_dx9_check(d3d->CreateDevice(*info->lpGuid, back_surf, &d3d_device)))
	  {
	  if(!i4_dx9_check(d3d->CreateDevice(IID_IDirect3DMMXDevice,back_surf,
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
  d3d_device->SetRenderState(D3DRS_ZENABLE,TRUE);   

  //turn gouraud shading on the entire time
  d3d_device->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_GOURAUD);

  //dithering.. eh..
  d3d_device->SetRenderState(D3DRS_DITHERENABLE,1);
  
  //no monochomatic shading?
  d3d_device->SetRenderState(D3DRS_MONOENABLE,FALSE);
  
  //texturemapping mode setup
  d3d_device->SetRenderState(D3DRS_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
  d3d_device->SetRenderState(D3DRS_TEXTUREHANDLE,0);  
  
  d3d_device->SetRenderState(D3DRS_TEXTUREADDRESS,D3DTADDRESS_CLAMP);
  d3d_device->SetRenderState(D3DRS_WRAPU,0);
  d3d_device->SetRenderState(D3DRS_WRAPV,0);

  d3d_device->SetRenderState(D3DRS_TEXTUREPERSPECTIVE,1);
    
  //no automatic face culling
  d3d_device->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

  //setup other basic modes
  set_write_mode(R1_WRITE_W | R1_COMPARE_W | R1_WRITE_COLOR);
  
  set_shading_mode(R1_COLORED_SHADING);

  set_alpha_mode(R1_ALPHA_DISABLED);  
      
  set_filter_mode(R1_NO_FILTERING);

  set_z_range(0.01f,1.0f);

  //clear the viewport, initialize type thing
  clear_area(0,0,639,479,0x00000000,1.0);

  //if we're page flipping, flip so we can clear the other page initially as well
  i4_dx9_display->flush();

  clear_area(0,0,639,479,0x00000000,1.0);  
  if (!tmanager->load_textures())
	  {
	i4_error("Texture reloading failed.");
	return i4_F;
	  }
*/
  return i4_T;	
	}


i4_bool r1_dx9_class::redepth(w16 new_bitdepth)
	{
	return i4_T;
	}

r1_texture_manager_class *r1_dx9_class::install_new_tmanager(w32& index)
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
	new_tman= new r1_dx9_texture_class(tmanager->get_pal());//use same parameters as default instance (must never be deleted)
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

void r1_dx9_class::set_filter_mode(r1_filter_type type)
{
  if (type==R1_NO_FILTERING)
  {
	  // The Point filter is a NEAREST filter
    d3d_device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
    d3d_device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
  }
  else
  if (type==R1_BILINEAR_FILTERING)
  {
	  //This is a bilinear filter
    d3d_device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);    
    d3d_device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
  }

  r1_render_api_class::set_filter_mode(type);
}

void r1_dx9_class::uninit()
{
  if (tmanager)
  {
    delete tmanager;
    tmanager = 0;  
  }

  //if (d3d_device)
  //{
  //  d3d_device->Release();
    d3d_device = 0;  //the device is owned by the common interface
  //}

  //if (d3d)
  //{
  //  d3d->Release();  
  //  d3d = 0;
  //}
  r1_render_api_class::uninit();
}

void r1_dx9_class::enable_holy()
{
  if (!holy_mode)
  {
    states_have_changed = i4_T;
    flush_vert_buffer();    

    pre_holy_alpha_mode = get_alpha_mode();
    pre_holy_write_mask = get_write_mask();

    //d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	//D3DTBLEND_MODULATEALPHA
	//d3d_device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	//d3d_device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

    set_alpha_mode(R1_ALPHA_LINEAR);
    set_write_mode(R1_COMPARE_W);
    
    holy_mode = i4_T;    
  }
}

void r1_dx9_class::disable_holy()
{
  if (holy_mode)
  {    
    states_have_changed = i4_T;
    flush_vert_buffer();    
    
    //d3d_device->SetRenderState(D3DRS_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
	//d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
    
    set_alpha_mode(pre_holy_alpha_mode);
    set_write_mode(pre_holy_write_mask);    
    
    holy_mode = i4_F;    
  }
}

void r1_dx9_class::use_texture(w32 index,r1_texture_handle material_ref,sw32 desired_width, w32 frame)
{  
  pf_dx9_use_texture.start();

  if (!tmanagers[index]->valid_handle(material_ref))
  {
	//i4_warning("Tried to use a texture before tmanager::load_textures. Case 1. Material %i",material_ref);
    pf_dx9_use_texture.stop();
    disable_texture();
    return;
  }

  if (!texture_mode)
  {
	  states_have_changed = i4_T;
	  flush_vert_buffer();
	  d3d_device->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	  //d3d_device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
      d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
      d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

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
      ((r1_dx9_texture_class *)tmanagers[index])->select_texture(mip->vram_handle, blahfloat_a, blahfloat_b);            
	  //d3d_device->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
      //d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
      //d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	  //d3d_device->SetRenderState(D3DRS_DITHERENABLE,FALSE);
      
    }
  }
  
  pf_dx9_use_texture.stop();
}

void r1_dx9_class::use_texture(r1_texture_handle material_ref, sw32 desired_width, w32 frame)
{  
  pf_dx9_use_texture.start();

  if (!tmanager->valid_handle(material_ref))
  {
	//i4_warning("Tried to use a texture before tmanager::load_textures. Case 2.");
    pf_dx9_use_texture.stop();
    disable_texture();
    return;
  }

  if (!texture_mode)
  {
	  d3d_device->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	  //d3d_device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
      d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
      d3d_device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

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
      ((r1_dx9_texture_class *)tmanager)->select_texture(mip->vram_handle, blahfloat_a, blahfloat_b);            
      //d3d_device->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
      //i4_dx9_check(d3d_device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE));
	  //i4_dx9_check(d3d_device->SetRenderState(D3DRS_DITHERENABLE,FALSE));
    }
  }
  pf_dx9_use_texture.stop();
}

// drawing will the constant color to render with if textures are disabled
void r1_dx9_class::disable_texture()
{
  if (texture_mode)
  {
    states_have_changed = i4_T;
    flush_vert_buffer();

    //d3d_device->SetRenderState(D3DRS_TEXTUREHANDLE,0);
	//d3d_device->SetTexture(0,0);
	d3d_device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);
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
 
static float dx9_z_scale, dx9_z_add, dx9_w_scale, dx9_w_add;


void r1_dx9_class::set_z_range(i4_float near_z, i4_float far_z)
{
  //dx9_near_z = near_z;
  //dx9_far_z  = far_z;

  dx9_z_scale = 0.999f / far_z;
  dx9_w_scale = near_z * 0.999f;
  dx9_z_add   = 0;//.01;
  dx9_w_add   = 0;//.01;

  r1_near_clip_z = near_z;
  r1_far_clip_z  = far_z;
}

inline void make_d3d_verts(CUSTOMVERTEX *dxverts,r1_vert *r1verts,r1_dx9_class *c,int total)
{                                                                        
  int i;  
    
  CUSTOMVERTEX *dx_v = dxverts;
  r1_vert     *r1_v = r1verts;

  //add vertex information
  for (i=0;i<total;i++)
  {
    dx_v->sx       = r1_v->px + c->x_off;
    dx_v->sy       = r1_v->py + c->y_off;
                                    
    dx_v->sz       = r1_v->w * dx9_w_scale;

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
	//so far, our customvertex structure doesn't need this
#ifdef USE_SPECULAR
    dx_v->specular=0xffffffff;  
#endif
    //add color information
    dx_v->diffuse=0xff000000;
    
    dx_v->tu=0;
    dx_v->tv=0;
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
  //so far, our customvertex structure doesn't need this
  //dx_v->specular=0xffffffff;  
  
  //add color information
  //dx_v->diffuse=0xff808080;
  switch (c->shade_mode)                                                    
  {                                                                      
    /* routines will draw fullbright */
    case R1_SHADE_DISABLED:
      dx_v = dxverts;
      r1_v = r1verts;
      ccolor = 0x00FFFFFF;            
      for (i=0;i<total;i++)
      {
        dx_v->diffuse    |= ccolor;
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
        dx_v->diffuse |= ccolor;
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
        
        dx_v->diffuse |= (red<<16) | (red<<8) | (red);
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
        dx_v->diffuse |= make_d3d_color(r1_v->r,r1_v->g,r1_v->b);
        
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
	  //dx_v->diffuse &= 0x00ffffff;
      dx_v->diffuse = alpha;
        
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
     //FIXED
	  dx_v->diffuse &=0x00FFFFFF;
      dx_v->diffuse |= (f_0_1_to_i_0_255(r1_v->a) << 24);
        
      dx_v++;
      r1_v++;
    }
  }  
}                                                                        

sw32 used_verts9   = 0;
sw32 used_indices9 = 0;

#define DX9_VERT_BUF_SIZE  128
#define DX9_INDEX_BUF_SIZE 128

CUSTOMVERTEX r1_dx9_tmp_verts[DX9_VERT_BUF_SIZE];
w16        r1_dx9_tmp_indices[DX9_INDEX_BUF_SIZE];

void init_d3d_vert_buffer()
{
  //sw32 i;
  //for (i=0; i<DX9_VERT_BUF_SIZE; i++)
  //{
  //  r1_dx9_tmp_verts[i].color = 0;
  //}
  
  used_verts9   = 0;
  used_indices9 = 0;
}

void r1_dx9_class::flush_vert_buffer()
{
#ifdef USE_BUFFER
  if (states_have_changed && (used_verts9!=0))
  //if (used_verts>0)
  {
    pf_dx9_drawprimitive.start();
    //d3d_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
    //                                 D3DVT_TLVERTEX,
    //                                 (void *)r1_dx9_tmp_verts,
    //                                 used_verts,
    //                                 r1_dx9_tmp_indices,
    //                                 used_indices,
    //                                 D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);  
	d3d_device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,
		0,
		used_verts9,
		used_indices9/3,
		r1_dx9_tmp_indices,
		D3DFMT_INDEX16,
		r1_dx9_tmp_verts,
		sizeof(CUSTOMVERTEX)
		);
    pf_dx9_drawprimitive.stop();
    
    used_verts9   = 0;
    used_indices9 = 0;

    states_have_changed = i4_F;
  }
#endif
}

void r1_dx9_class::render_poly(int t_verts, r1_vert *verts)
{   
  if (t_verts > DX9_VERT_BUF_SIZE)  return;

#ifdef USE_BUFFER
  sw32 t_index = (t_verts-2)*3;
    
  if (t_index > DX9_INDEX_BUF_SIZE) return;
  
  if (t_verts + used_verts9   > DX9_VERT_BUF_SIZE ||
      t_index + used_indices9 > DX9_INDEX_BUF_SIZE)
  {
    states_have_changed = i4_T;
    flush_vert_buffer();
  }
  
#endif

  sw32 i;

  //pf_dx9_vertex_setup.start();
  if (color_tint_on)
  {
    for (i=0; i<t_verts; i++)
    {
      verts[i].r *= r_tint_mul;
      verts[i].g *= g_tint_mul;
      verts[i].b *= b_tint_mul;
    }
  }
  
  make_d3d_verts(r1_dx9_tmp_verts+used_verts9,verts,this,t_verts);
  //pf_dx9_vertex_setup.stop();

#ifdef USE_BUFFER  
  
  //setup indices
  for (i=1; i<t_verts-1; i++)
  {
    r1_dx9_tmp_indices[used_indices9]   = (w16)(used_verts9);

    r1_dx9_tmp_indices[used_indices9+1] = (w16)(used_verts9+i);

    r1_dx9_tmp_indices[used_indices9+2] = (w16)(used_verts9+i+1);
    used_indices9 += 3;
  }

  used_verts9 += t_verts;

#else
  
  pf_dx9_drawprimitive.start();

  i4_dx9_check(d3d_device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,
							t_verts,
                            (void *)r1_dx9_tmp_verts,
                            sizeof(CUSTOMVERTEX)));

  pf_dx9_drawprimitive.stop();

#endif
}

void r1_dx9_class::render_pixel(int t_points, r1_vert *pixel)
{
  if (t_points>256) return;
  CUSTOMVERTEX dx_verts[256];
  make_d3d_verts(dx_verts,pixel,this,t_points);
  i4_dx9_check(d3d_device->DrawPrimitiveUP(D3DPT_POINTLIST,
	  t_points,
	  dx_verts,
	  sizeof(CUSTOMVERTEX)));
}

void r1_dx9_class::render_lines(int t_lines, r1_vert *verts)
    {
    if (t_lines+1>256) return;
    
    for (int line=0;line<t_lines;line++)
        {
        CUSTOMVERTEX dx_v[4];
        memset(dx_v,0,4*sizeof(CUSTOMVERTEX));
        float ooz=r1_ooz(verts[line].w);
        float oozw=ooz*dx9_w_scale;
        float x1=verts[line].px;
        float y1=verts[line].py;
        float x2=verts[line+1].px+1;
        float y2=verts[line+1].py+1;
        
        w32 color=make_d3d_color(verts[line].r,verts[line].g,verts[line].b);
        w32 color2=make_d3d_color(verts[line+1].r,
            verts[line+1].g,
            verts[line+1].b);
        if (i4_fabs(x1-x2)>=i4_fabs(y1-y2))
            {
            //dx_v[0].sx=  x2+x_off;
            //dx_v[0].sy=  y1+y_off;
            dx_v[0].sx=  x2+x_off;
            dx_v[0].sy=  y2+y_off-1;
            dx_v[0].sz=  oozw;
            dx_v[0].rhw= ooz;
            dx_v[0].diffuse=color2;
            
            //dx_v[1].sx=  x1+x_off;
            //dx_v[1].sy=  y1+y_off;
            dx_v[1].sx=  x1+x_off;
            dx_v[1].sy=  y1+y_off;
            dx_v[1].sz=  oozw;
            dx_v[1].rhw=ooz;
            dx_v[1].diffuse=color;
            
            //dx_v[2].sx=  x1+x_off;
            //dx_v[2].sy=  y2+y_off;
            dx_v[2].sx=  x1+x_off;
            dx_v[2].sy=  y1+y_off+1;
            dx_v[2].sz=  oozw;
            dx_v[2].rhw=ooz;
            dx_v[2].diffuse=color;
            
            //dx_v[3].sx=  x2+x_off;
            //dx_v[3].sy=  y2+y_off;
            dx_v[3].sx=  x2+x_off;
            dx_v[3].sy=  y2+y_off;
            dx_v[3].sz=  oozw;
            dx_v[3].rhw=ooz;
            dx_v[3].diffuse=color2;
            d3d_device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,
                2,
                dx_v,
                sizeof(CUSTOMVERTEX));
            
            }
        else
            {
            dx_v[0].sx=  x2+x_off;
            dx_v[0].sy=  y2+y_off;
            dx_v[0].sz=  oozw;
            dx_v[0].rhw= ooz;
            dx_v[0].diffuse=color2;
            
            dx_v[1].sx=  x1+x_off;
            dx_v[1].sy=  y1+y_off;
            dx_v[1].sz=  oozw;
            dx_v[1].rhw=ooz;
            dx_v[1].diffuse=color;
            
            dx_v[2].sx=  x1+x_off-1;
            dx_v[2].sy=  y1+y_off;
            dx_v[2].sz=  oozw;
            dx_v[2].rhw=ooz;
            dx_v[2].diffuse=color;
            
            dx_v[3].sx=  x2+x_off-1;
            dx_v[3].sy=  y2+y_off;
            dx_v[3].sz=  oozw;
            dx_v[3].rhw=ooz;
            dx_v[3].diffuse=color2;
            d3d_device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,
                2,
                dx_v,
                sizeof(CUSTOMVERTEX));
            }
        }
    
    }

void r1_dx9_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
	{
	float ooz=r1_ooz(z);
	float oozw=ooz*dx9_w_scale;
	if ((abs(x2-x1)<10) && (abs(y2-y1)<10))
		{
		//disable_texture();
		//w32 color=i4_pal_man.convert_32_to(lcolor,&(i4_current_app->get_display()->get_palette()->source));
		
		CUSTOMVERTEX dx_v[4];
		memset(dx_v,0,4*sizeof(CUSTOMVERTEX));
		
		dx_v[0].sx=  x2+x_off;
		dx_v[0].sy=  y1+y_off;
		dx_v[0].sz=  oozw;
		dx_v[0].rhw= ooz;
		dx_v[0].diffuse=color;
		
		dx_v[1].sx=  x1+x_off;
		dx_v[1].sy=  y1+y_off;
		dx_v[1].sz=  oozw;
		dx_v[1].rhw=ooz;
		dx_v[1].diffuse=color;
		
		dx_v[2].sx=  x1+x_off;
		dx_v[2].sy=  y2+y_off;
		dx_v[2].sz=  oozw;
		dx_v[2].rhw=ooz;
		dx_v[2].diffuse=color;
		
		dx_v[3].sx=  x2+x_off;
		dx_v[3].sy=  y2+y_off;
		dx_v[3].sz=  oozw;
		dx_v[3].rhw=ooz;
		dx_v[3].diffuse=color;
		d3d_device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,
			2,
			dx_v,
			sizeof(CUSTOMVERTEX));
		}
	else
		{
		D3DRECT    rect;
		HRESULT res;
		
		rect.x1   = (long)(x_off + x1);
		rect.y1    = (long)(y_off + y1);  
		rect.x2  = (long)(x_off + x2+1);
		rect.y2 = (long)(y_off + y2+1);
		
		//DDBLTFX fx;
		//memset(&fx,0,sizeof(DDBLTFX));
		//fx.dwSize = sizeof(DDBLTFX);
		
		if (write_mask & R1_WRITE_COLOR)
			{
			
			//fx.dwFillColor = color;
			//fx.dwFillColor=i4_pal_man.convert_32_to(color,&(i4_current_app->get_display()->get_palette()->source));
			//need a method here that converts 32bit ARGB values to 
			//what the current video mode desires.
			//DDSURFACEDESC ddsd;
			//dx9_common.get_surface_description(dx9_common.back_surface,&ddsd);
			//ddsd.ddpfPixelFormat.dwGBitMask
			//UINT tcolor=i4_pal_man.convert_32_to(color,&(i4_current_app->get_display()->get_palette()->source));
			UINT tcolor=color;
			//dx9_common.back_surface->Blt(&rect,NULL,NULL,DDBLT_COLORFILL,&fx);
			dx9_common.device->Clear(1,&rect,D3DCLEAR_TARGET,tcolor,1.0,0);
			}
		
		if (write_mask & R1_WRITE_W)
			{
				float filldepth;
			if (z == r1_near_clip_z)
				{  
				filldepth = 1.0f;
				}
			else
				if (z == r1_far_clip_z)
					{
					filldepth = 0.0f;
					}
				else
				{
					
					filldepth = oozw;
				}
				
				//res = zbuffer_surface->Blt(&rect,NULL,NULL,DDBLT_DEPTHFILL,&fx);
			res=dx9_common.device->Clear(1,&rect,D3DCLEAR_ZBUFFER,0,filldepth,0);
			}
		}
	};

/*
//Let's see wheter we can do this a little bit faster
//Blt() is actually a very slow function if used on small rectagles
void r1_dx9_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
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
	//dx9_common.get_surface_description(dx9_common.back_surface,&ddsd);
	//ddsd.ddpfPixelFormat.dwGBitMask

    dx9_common.back_surface->Blt(&rect,NULL,NULL,DDBLT_COLORFILL,&fx);
  }

  if (write_mask & R1_WRITE_W)
  {
    if (z == dx9_near_z)
    {  
      fx.dwFillDepth = 0xFFFF;
    }
    else
    if (z == dx9_far_z)
    {
      fx.dwFillDepth = 0;
    }
    else
      fx.dwFillDepth = i4_f_to_i((1.f/(float)z) * dx9_w_scale * (float)0xFFFF);

    res = zbuffer_surface->Blt(&rect,NULL,NULL,DDBLT_DEPTHFILL,&fx);
  }
}*/

i4_image_class *r1_dx9_class::create_compatible_image(w16 w, w16 h)
{  
  return i4_create_image(w,h,i4_dx9_display->get_palette());
}


// void r1_dx9_class::copy_part(i4_image_class *im,                                          
//                             int x, int y,             // position on screen
//                             int x1, int y1,           // area of image to copy 
//                             int x2, int y2)
// {
  
//   DDSURFACEDESC ddsd;
//   memset(&ddsd,0,sizeof(DDSURFACEDESC));
//   ddsd.dwSize = sizeof(DDSURFACEDESC);
//   dx9_common.back_surface->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_WRITEONLY,0);  
  
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
//   dx9_common.back_surface->Unlock(NULL);
// }
