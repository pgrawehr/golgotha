/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DX9_UTIL_HH
#define DX9_UTIL_HH

//#include <ddraw.h>
//#undef DIRECT3D_VERSION
#include "d3d9.h"
#include "d3d9types.h"
#include "image/image.h"
#include "palette/pal.h"
#include "threads/threads.h"
#include "video/win32/d3denumeration.h"

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
enum dx9_surface_type
{ 
  DX9_SURFACE,                          // used only for Blt's
  DX9_SYSMEM,						   // Some operations (like updatesurface)
									   // work only on system memory.
  DX9_TEXTURE,                        
  DX9_PAGE_FLIPPED_PRIMARY_SURFACE,    // will grab backbuffer surface
  DX9_BACKBUFFERED_PRIMARY_SURFACE     // will grab backbuffer surface
} ;


enum
{
  DX9_VRAM              = 1,
  DX9_SYSTEM_RAM        = 2,
  DX9_RENDERING_SURFACE = 4,
  DX9_CLEAR				= 8
};
/*
class dx9_mode
{
public:
  D3DSURFACE_DESC desc;
  D3DFORMAT format;
  dx9_mode     *next;
  dx9_mode(dx9_mode *next) : next(next) {}
};

struct dx9_driver
{
  LPGUID lpGuid;
  char   DriverName[128];
  char   DeviceDesc[128];
  dx9_driver *next;
  dx9_driver(dx9_driver *next) : next(next) {}
};

struct dx9_d3d_info
{
  LPGUID lpGuid;
  LPGUID lpSoftGuid;
  LPSTR  lpDeviceName;
  LPSTR  lpSoftDeviceName;
  D3DCAPS9 hw_desc,sw_desc;
};
*/

void bitmask_2_format(D3DFORMAT d3dfmt,i4_pixel_format &fmt);

enum {
	DRIVER_MODE_ENUMERATE=1,
	DRIVER_MODE_INIT=2
	};

class dx9_common_class
{

public:
  //static IDirectDraw2          *ddraw;
  //static IDirectDrawSurface3   *primary_surface, *back_surface, *front_surface;
  static LPDIRECT3D9            pD3D9;
  static D3DPRESENT_PARAMETERS  present;
  static DDPIXELFORMAT          dd_fmt_565, dd_fmt_1555;
  static IDirect3DSurface9      *primary_surface, *back_surface, *front_surface;
  static i4_pixel_format        i4_fmt_565, i4_fmt_1555;
  static LPDIRECT3DDEVICE9      device;
  //static LPDIRECTDRAWCLIPPER    lpddclipper;
  CD3DEnumeration			    *d3denum;

  dx9_common_class();

  IDirect3DSurface9 *create_surface(dx9_surface_type type,
                                      int width=0, int height=0, 
                                      int flags=0,
                                      D3DFORMAT format=D3DFMT_UNKNOWN); 


  i4_bool get_surface_description(IDirect3DSurface9 *surface, D3DSURFACE_DESC &surface_desc)
  {
    memset(&surface_desc,0,sizeof(D3DSURFACE_DESC));
    //surface_desc.dwSize = sizeof(D3DSURFACE_DESC);
    return (i4_bool)(surface->GetDesc(&surface_desc)==D3D_OK);
	  }
/*
  i4_bool get_desc(IDirectDrawSurface3 *surface, DDSURFACEDESC &surface_desc)
  {
    memset(&surface_desc,0,sizeof(DDSURFACEDESC));
    surface_desc.dwSize = sizeof(DDSURFACEDESC);
    return (i4_bool)(surface->GetSurfaceDesc(&surface_desc)==DD_OK);
  }
*/

  D3DDeviceInfo *get_driver_hardware_info(LPDIRECT3D9 dd, w32 adapter);

  CArrayList *get_mode_list(UINT adaptor);
  //void free_mode_list(dx9_mode *list);

  CArrayList *get_driver_list();
  //void free_driver_list(dx9_driver *list);
  LPDIRECT3D9 initialize_driver();

  LPDIRECT3DSURFACE9 get_surface(i4_image_class *im);
  i4_image_class *create_image(int w, int h, w32 surface_flags);

  void cleanup();
};


class i4_dx9_image_class : public i4_image_class
{
public:
  IDirect3DSurface9 *surface;
  i4_dx9_image_class(w16 w, w16 h, w32 surface_flags=DX9_SYSTEM_RAM);
  ~i4_dx9_image_class();
  

  w32 *paddr(int x, int y)
	  {
	  w32 x1=x;
	wptr target=0;
	switch (pal->source.pixel_depth)
		{
		case I4_32BIT:
			x1=x*4;
		    break;
		case I4_24BIT:
			x1=x*3;
			break;
		case I4_16BIT:
			x1=x*2;
			break;
		case I4_8BIT:
			x1=x;
			break;
		}
	target=x1;
	target=((wptr)data)+target;
	target=target+(y*bpl);
	return (w32 *)target; 
	  };
  
  i4_color get_pixel(i4_coord x, i4_coord y)
  {
    return i4_pal_man.convert_to_32(*paddr(x,y), pal);
  }
  
  void put_pixel(i4_coord x, i4_coord y, w32 color);//Put Pixel to surface

  void put_part(i4_image_class *to, i4_coord x, i4_coord y, i4_coord x1,
	  i4_coord y1, i4_coord x2, i4_coord y2, i4_draw_context_class &context);


  void lock();
  void unlock();
};


extern dx9_common_class dx9_common;


#endif
