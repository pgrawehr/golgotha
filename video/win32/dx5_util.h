/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DX5_UTIL_HH
#define DX5_UTIL_HH



#include <ddraw.h>
#include <d3d.h>
#include "image/image.h"
#include "palette/pal.h"
#include "threads/threads.h"

enum dx5_surface_type
{ 
  DX5_SURFACE,                          // used only for Blt's
  DX5_TEXTURE,
  DX5_PAGE_FLIPPED_PRIMARY_SURFACE,    // will grab backbuffer surface
  DX5_BACKBUFFERED_PRIMARY_SURFACE     // will grab backbuffer surface
} ;


enum
{
  DX5_VRAM              = 1,
  DX5_SYSTEM_RAM        = 2,
  DX5_RENDERING_SURFACE = 4,
  DX5_CLEAR				= 8
};

struct dx5_mode
{
  DDSURFACEDESC desc;
  dx5_mode     *next;
  dx5_mode(dx5_mode *next) : next(next) {}
};

struct dx5_driver
{
  LPGUID lpGuid;
  char   DriverName[128];
  char   DeviceDesc[128];
  dx5_driver *next;
  dx5_driver(dx5_driver *next) : next(next) {}
};

struct dx5_d3d_info
{
  LPGUID lpGuid;
  LPGUID lpSoftGuid;
  LPSTR  lpDeviceName;
  LPSTR  lpSoftDeviceName;
  D3DDEVICEDESC hw_desc,sw_desc;
};


enum {
	DRIVER_MODE_ENUMERATE=1,
	DRIVER_MODE_INIT=2
	};

class dx5_common_class
{

public:
  static IDirectDraw2          *ddraw;
  static IDirectDrawSurface3   *primary_surface, *back_surface, *front_surface;
  static DDPIXELFORMAT          dd_fmt_565, dd_fmt_1555;
  static i4_pixel_format        i4_fmt_565, i4_fmt_1555;
  static LPDIRECTDRAWCLIPPER    lpddclipper;

  dx5_common_class();

  IDirectDrawSurface3 *create_surface(dx5_surface_type type,
                                      int width=0, int height=0, // not need for primary
                                      int flags=0,
                                      DDPIXELFORMAT *format=0); // format not need for primary


  i4_bool get_surface_description(IDirectDrawSurface3 *surface, DDSURFACEDESC &surface_desc)
  {
    memset(&surface_desc,0,sizeof(DDSURFACEDESC));
    surface_desc.dwSize = sizeof(DDSURFACEDESC);
    return (i4_bool)(surface->GetSurfaceDesc(&surface_desc)==DD_OK);
	  }

  i4_bool get_desc(IDirectDrawSurface3 *surface, DDSURFACEDESC &surface_desc)
  {
    memset(&surface_desc,0,sizeof(DDSURFACEDESC));
    surface_desc.dwSize = sizeof(DDSURFACEDESC);
    return (i4_bool)(surface->GetSurfaceDesc(&surface_desc)==DD_OK);
  }


  dx5_d3d_info *get_driver_hardware_info(IDirectDraw2 *dd, w32 mode);

  dx5_mode *get_mode_list(IDirectDraw2 *dd);
  void free_mode_list(dx5_mode *list);

  dx5_driver *get_driver_list();
  void free_driver_list(dx5_driver *list);
  IDirectDraw2 *initialize_driver(dx5_driver *driver);

  IDirectDrawSurface3 *get_surface(i4_image_class *im);
  i4_image_class *create_image(int w, int h, w32 surface_flags);

  void cleanup();
};


class i4_dx5_image_class : public i4_image_class
{
public:
  IDirectDrawSurface3 *surface;
  i4_dx5_image_class(w16 w, w16 h, w32 surface_flags=DX5_SYSTEM_RAM);
  ~i4_dx5_image_class();
  

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
			x1=x*2;//??? Funktionierte ursrünglich mit nur x
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


extern dx5_common_class dx5_common;


#endif
