/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "video/win32/dx5_util.h"
#include "video/win32/dx5_error.h"
#include "threads/threads.h"
#include "error/error.h"
#include "main/win_main.h"

#define DEPTH 16


dx5_common_class dx5_common;
IDirectDraw2 *dx5_common_class::ddraw;
IDirectDrawSurface3 *dx5_common_class::primary_surface, *dx5_common_class::back_surface,
*dx5_common_class::front_surface;
DDPIXELFORMAT dx5_common_class::dd_fmt_565, dx5_common_class::dd_fmt_1555;
i4_pixel_format dx5_common_class::i4_fmt_565, dx5_common_class::i4_fmt_1555;
LPDIRECTDRAWCLIPPER dx5_common_class::lpddclipper;



IDirectDrawSurface3 *dx5_common_class::create_surface(dx5_surface_type type,
													  int width, int height,
													  int flags,
													  DDPIXELFORMAT *format)
{
	HRESULT result;
	DDSURFACEDESC ddsd;
	IDirectDrawSurface *surface=0;
	IDirectDrawSurface3 *surface3=0;
	//if (width<30||height<30) flags=((flags&(~DX5_VRAM))|DX5_SYSTEM_RAM);
	//Creates new texcache, fails to load textures
	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);

	if (type==DX5_PAGE_FLIPPED_PRIMARY_SURFACE)
	{
		if (front_surface==primary_surface)
		{
			front_surface=0;
		}
		if (primary_surface)
		{
			primary_surface->Release();
		}
		primary_surface=0;
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
							  DDSCAPS_FLIP |
							  DDSCAPS_COMPLEX |
							  DDSCAPS_3DDEVICE |
							  DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = 1;
	}
	else if (type==DX5_BACKBUFFERED_PRIMARY_SURFACE)
	{
		if (primary_surface)
		{
			primary_surface->Release();
		}
		primary_surface=0;
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
	}
	else
	{
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = 0;

		if (format)
		{
			ddsd.dwFlags |= DDSD_PIXELFORMAT;
			ddsd.ddpfPixelFormat = *format;
		}

		ddsd.dwWidth = width;
		ddsd.dwHeight = height;

		if (flags & DX5_SYSTEM_RAM)
		{
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}

		if (flags & DX5_VRAM)
		{
			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}

		if (flags & DX5_RENDERING_SURFACE)
		{
			ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
		}

		if (type==DX5_TEXTURE)
		{
			ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;
		}
	}


	result = ddraw->CreateSurface(&ddsd, &surface, NULL);
	if (!i4_dx5_check(result))
	{
		surface=0;
	}

	if (surface)
	{
		surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&surface3);
		surface->Release();

		if (flags & DX5_CLEAR)
		{
			DDBLTFX fx;
			memset(&fx,0,sizeof(DDBLTFX));
			fx.dwSize = sizeof(DDBLTFX);

			RECT r;
			r.left   = 0;
			r.top    = 0;
			r.right  = width;
			r.bottom = height;

			surface3->Blt(&r, 0, 0, DDBLT_COLORFILL,&fx);
		}

		if (type==DX5_PAGE_FLIPPED_PRIMARY_SURFACE)
		{
			primary_surface = surface3;

			// get the back buffer surface
			DDSCAPS caps;
			caps.dwCaps = DDSCAPS_BACKBUFFER;
			if (back_surface)
			{
				back_surface->Release();
			}
			result = primary_surface->GetAttachedSurface(&caps, &back_surface);
			if (!i4_dx5_check(result))
			{
				primary_surface->Release();
				primary_surface=0;
				surface3=0;
			}

			caps.dwCaps = DDSCAPS_FRONTBUFFER;
			if (front_surface)
			{
				front_surface->Release();
			}
			result = primary_surface->GetAttachedSurface(&caps, &front_surface);
			if (!i4_dx5_check(result))
			{
				//back_surface->Release();      back_surface=0;
				//primary_surface->Release();   primary_surface=0;
				front_surface=primary_surface;
				//surface3=0;
			}

		}
		else if (type==DX5_BACKBUFFERED_PRIMARY_SURFACE)
		{
			DDSURFACEDESC desc;

			get_desc(surface3, desc);

			primary_surface = surface3;
			//We have a problem if primary+backbuffer+zbuffer don't fit in vidmem
			//for the set gdi resolution in windowed mode. (like on my machine...)
			//I suggest a compromise: The resolution entered in settings becomes the
			//upper limit such that the system knows what will work.

			if(back_surface)
			{
				back_surface->Release();
			}
			//Backbuffer must have same size as primary surface, as we cannot change its size later
			back_surface = create_surface(DX5_SURFACE, width, height,
										  DX5_RENDERING_SURFACE | DX5_VRAM | DX5_CLEAR);

			if (!back_surface)
			{
				primary_surface->Release();
				surface3=0;
			}
		}

	}
	else if (flags & DX5_VRAM) // try to create it in system ram
	{
		i4_warning("failed to create surface in video memory");
		surface3=create_surface(DX5_SURFACE, width, height,
								(flags & (~DX5_VRAM)) | DX5_SYSTEM_RAM, format);
	}
//  memset(&ddsd,0,sizeof(ddsd));
//  ddsd.dwSize=sizeof(ddsd);
	if (surface3&&(surface3->IsLost()==DDERR_SURFACELOST))
	{
		surface3->Restore();
	}
/*  if (surface3)
   	  {
   surface3->GetSurfaceDesc(&ddsd);
   i4_warning("Surface allocated %dx%d Bitdepth %d",ddsd.dwWidth,
   	  ddsd.dwHeight,ddsd.ddpfPixelFormat.dwRGBBitCount);
   	  }
   else
   	  i4_warning("Will be crashing now due to unknown reason in DirectX...");
 */
	return surface3;
}


void dx5_common_class::cleanup()
{
	i4_warning("dx5_common_class::cleanup() freeing directdraw interfaces.");
	if (lpddclipper)
	{
		lpddclipper->Release();
		lpddclipper=0;
	}
	if (back_surface)
	{
		back_surface->Release();
		back_surface=0;
	}

	if (front_surface)
	{
		if (front_surface==primary_surface)
		{
			primary_surface=0;
		}
		front_surface->Release();
		front_surface=0;
	}

	if (primary_surface)
	{
		primary_surface->Release();
		primary_surface=0;
	}

	DDSCAPS tcaps;
	tcaps.dwCaps = DDSCAPS_TEXTURE;
	w32 total_free, texture_free;
	if (!dx5_common.ddraw)
	{
		return;
	}
	dx5_common.ddraw->GetAvailableVidMem(&tcaps, &total_free, &texture_free);
	i4_warning("Total Display Memory Free just before Releasing the ddraw interface: %d",total_free);
	i4_warning("Total Display Texture Memory Free before Releasing the ddraw interface: %d",texture_free);

	if (ddraw)
	{
		ddraw->Release();
		ddraw=0;
	}
}


dx5_common_class::dx5_common_class()
{
	ddraw=0;
	primary_surface=back_surface=front_surface=0;
	lpddclipper=0;

	i4_fmt_565.pixel_depth = I4_16BIT;
	i4_fmt_565.red_mask    = 31 << 11;
	i4_fmt_565.green_mask  = 63 << 5;
	i4_fmt_565.blue_mask   = 31;
	i4_fmt_565.alpha_mask  = 0;
	i4_fmt_565.lookup = 0;
	i4_fmt_565.calc_shift();

	i4_fmt_1555.pixel_depth = I4_16BIT;
	i4_fmt_1555.red_mask    = 31 << 10;
	i4_fmt_1555.green_mask  = 31 << 5;
	i4_fmt_1555.blue_mask   = 31;
	i4_fmt_1555.alpha_mask  = 0;
	i4_fmt_1555.lookup = 0;
	i4_fmt_1555.calc_shift();


	memset(&dd_fmt_565,0,sizeof(DDPIXELFORMAT));
	dd_fmt_565.dwSize = sizeof(DDPIXELFORMAT);
	dd_fmt_565.dwFlags = DDPF_RGB;
	dd_fmt_565.dwRGBBitCount = 16;
	dd_fmt_565.dwRBitMask = 31 << 11;
	dd_fmt_565.dwGBitMask = 63 << 5;
	dd_fmt_565.dwBBitMask = 31;

	memset(&dd_fmt_1555,0,sizeof(DDPIXELFORMAT));
	dd_fmt_1555.dwSize = sizeof(DDPIXELFORMAT);
	dd_fmt_1555.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	dd_fmt_1555.dwRGBBitCount     = 16;
	dd_fmt_1555.dwRBitMask        = 31 << 10;
	dd_fmt_1555.dwGBitMask        = 31 << 5;
	dd_fmt_1555.dwBBitMask        = 31;
	dd_fmt_1555.dwRGBAlphaBitMask = 1 << 15;

}


//******************************* VIDEO MODE ENUMERATION ***************************
static dx5_mode *dx_mode_list;

HRESULT WINAPI dx5_vidmode_callback(LPDDSURFACEDESC lpDDSurfaceDesc,LPVOID lpContext)
{
	//Wir werden hier alle Modi bekommen. Filtere alle Modi raus,
	//Die weniger als 16 Bit Farbtiefe liefern.
	//Zur Zeit noch alle Modi dalassen, mal sehen wie das wird.
	dx5_mode *m=new dx5_mode(dx_mode_list);
	dx_mode_list=m;
	m->desc=*lpDDSurfaceDesc;

	return DDENUMRET_OK;
}

dx5_mode *dx5_common_class::get_mode_list(IDirectDraw2 *dd)
{
	dx_mode_list=0;

	DDSURFACEDESC ddsd;
	DDPIXELFORMAT p_format;

	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize   = sizeof(DDSURFACEDESC);
	ddsd.dwFlags  = DDSD_PIXELFORMAT;

	memset(&p_format,0,sizeof(DDPIXELFORMAT));
	p_format.dwSize = sizeof(DDPIXELFORMAT);
	p_format.dwFlags = DDPF_RGB;
	p_format.dwRGBBitCount = DEPTH; //DEPTH zur Zeit 16
	ddsd.ddpfPixelFormat = p_format; //Wir können kein 16-Bit Modus suchen,
	//wenn wir gerade im 24-Bit Fenstermodus arbeiten!
	//Zählen heute ALLE Modi ab (2. Parameter sonst &ddsd)
	if (!i4_dx5_check(dd->EnumDisplayModes(0,NULL,0,(LPDDENUMMODESCALLBACK)dx5_vidmode_callback)))
	{
		free_mode_list(dx_mode_list);
		dx_mode_list=0;
	}

	return dx_mode_list;
}

void dx5_common_class::free_mode_list(dx5_mode *list)
{
	while (list)
	{
		dx5_mode *p=list;
		list=list->next;
		delete p;
	}
}



//************************** DRIVER ENUMERATION AND CREATION ************************
static dx5_driver *dx_driver_list=0;

BOOL WINAPI dd_device_callback(LPGUID lpGuid, LPSTR lpDeviceDesc,
							   LPSTR lpDriverName, LPVOID lpUserArg, HMONITOR hm)
{
	dx5_driver *d=new dx5_driver(dx_driver_list);
	dx_driver_list=d;
	d->lpGuid = lpGuid;
	strcpy(d->DriverName, lpDriverName);
	strcpy(d->DeviceDesc, lpDeviceDesc);
	return DDENUMRET_OK;
}

dx5_driver *dx5_common_class::get_driver_list()
{
	//if (dx_driver_list) free_driver_list(dx_driver_list);//This function can be called more than once.
//but its the callers responsability to delete the list after use.
	dx_driver_list=0;
	if (!i4_dx5_check(DirectDrawEnumerateEx(dd_device_callback,0,
											DDENUM_ATTACHEDSECONDARYDEVICES|
											DDENUM_DETACHEDSECONDARYDEVICES|
											DDENUM_NONDISPLAYDEVICES)))
	{
		free_driver_list(dx_driver_list);
		dx_driver_list=0;
	}
	return dx_driver_list;
}

void dx5_common_class::free_driver_list(dx5_driver *list)
{
	while (list)
	{
		dx5_driver *n=list;
		list=list->next;
		delete n;
	}
}

static IDirectDraw *dx5_ddraw;

BOOL WINAPI dd_create_callback(LPGUID lpGuid,   LPSTR lpDeviceDescription,
							   LPSTR lpDriverName, LPVOID lpUserArg, HMONITOR hm)
{
	char *desired_driver = (char *)lpUserArg;
	//PG: Changed this as I'm not shure wheter driver name is guaranteed to
	//be unique even when using multiple displays.
	//Using lpGuid instead, which certainly IS unique.
	/*if (!stricmp(desired_driver,lpDriverName))
	   {
	   HRESULT res = DirectDrawCreate(lpGuid, &dx5_ddraw, 0);
	   return DDENUMRET_CANCEL;
	   }*/
	if (!lpGuid) //is the primary display driver
	{
		for(int i=0; i<=sizeof(GUID); i++)
		{
			if (((w8 *)&i4_win32_startup_options.guid_screen)[i]!=0)
			{
				//is the saved guid a null-guid?
				return DDENUMRET_OK;
			}
		}
		i4_dx5_check(DirectDrawCreate(lpGuid,&dx5_ddraw,0));
		return DDENUMRET_CANCEL;
	}
	if (memcmp(lpGuid,&i4_win32_startup_options.guid_screen,sizeof(GUID))==0)
	{
		HRESULT res=DirectDrawCreate(lpGuid,&dx5_ddraw,0);
		if (!i4_dx5_check(res))
		{
			MessageBox(NULL,"The currently choosen display device failed to initialize.\n"
							"Reverting to default (primary display driver)","DirectX initialisation failed",MB_OK);
			ZeroMemory(&i4_win32_startup_options.guid_screen,sizeof(GUID));
			i4_dx5_check(DirectDrawCreate(NULL,&dx5_ddraw,0));
		}
		return DDENUMRET_CANCEL;
	}
	return DDENUMRET_OK;
}

IDirectDraw2 *dx5_common_class::initialize_driver(dx5_driver *driver)
{
	if (!driver)
	{
		return 0;
	}

	dx5_ddraw=0;
	if (!i4_dx5_check(DirectDrawEnumerateEx(dd_create_callback,(void *)driver->DriverName,
											DDENUM_ATTACHEDSECONDARYDEVICES|DDENUM_DETACHEDSECONDARYDEVICES|DDENUM_NONDISPLAYDEVICES)))
	{
		return 0;
	}

	if (!dx5_ddraw)
	{
		MessageBox(NULL,"The DirectDraw Driver stored in the registry has failed or does not exist.\n"
						"Reverting to Default","Invalid Settings",MB_OK);
		ZeroMemory(&i4_win32_startup_options.guid_screen,sizeof(GUID));
		if (!i4_dx5_check(DirectDrawCreate(NULL,&dx5_ddraw,0)))
		{
			return 0;
		}
		;
	}
	;

	IDirectDraw2 *dd=0;
	if (!i4_dx5_check(dx5_ddraw->QueryInterface(IID_IDirectDraw2,(void **)&dd)))
	{
		dx5_ddraw->Release();
		return 0;
	}

	dx5_ddraw->Release(); // we are only using DirectDraw2 interface, free this one

	return dd;
}

static LPGUID dx5_hardware_guid;
static dx5_d3d_info dx5_d3d;

HRESULT __stdcall d3d_manual_callback(LPGUID lpGuid, LPSTR lpDeviceDescription,
									  LPSTR lpDeviceName, LPD3DDEVICEDESC lpD3DHWDeviceDesc,
									  LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID lpUserArg)
{
	if (strcmp(lpDeviceName, i4_win32_startup_options.render_data)==0)
	{
		dx5_d3d.lpGuid=lpGuid;
		dx5_d3d.lpDeviceName=lpDeviceName;
		dx5_d3d.hw_desc=*lpD3DHWDeviceDesc;
		dx5_d3d.sw_desc=*lpD3DHELDeviceDesc;
		return DDENUMRET_CANCEL; //we've found it.
	}
	return DDENUMRET_OK;
}

HRESULT __stdcall d3d_device_callback(LPGUID lpGuid,    LPSTR lpDeviceDescription,
									  LPSTR lpDeviceName, LPD3DDEVICEDESC lpD3DHWDeviceDesc,
									  LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID lpUserArg)
{
	//The device should be user-selectable (Hardware dependent)
//Statements further down in this list have priority over statements at the beginning.
	if (strcmp(lpDeviceName, "Direct3D HAL")==0 && lpD3DHWDeviceDesc->dwDeviceZBufferBitDepth!=0)
	{
		//Ist das sinnvoll? Wir nehmen nur ein hardwarebeschleunigtes Gerät?
		dx5_d3d.lpGuid = lpGuid;
		dx5_d3d.lpDeviceName = lpDeviceName;
		dx5_d3d.hw_desc = *lpD3DHWDeviceDesc;
		dx5_d3d.sw_desc = *lpD3DHELDeviceDesc;
	}
	if (dx5_d3d.lpGuid==0 && lpD3DHELDeviceDesc->dwDeviceZBufferBitDepth!=0)
	{
		//If we find no matching HAL, we use any HEL that supports ZBuffer
		dx5_d3d.lpGuid=lpGuid;
		dx5_d3d.lpDeviceName=lpDeviceName;
		dx5_d3d.hw_desc=*lpD3DHWDeviceDesc;
		dx5_d3d.sw_desc=*lpD3DHELDeviceDesc;
	}
	if (dx5_d3d.lpSoftGuid==0 && lpD3DHWDeviceDesc->dwDeviceZBufferBitDepth==0)
	{
		dx5_d3d.lpSoftGuid=lpGuid;
		dx5_d3d.lpSoftDeviceName=lpDeviceName;
	}
	if (strcmp(lpDeviceName, "RGB Emulation")==0)
	{
		dx5_d3d.lpSoftGuid=lpGuid;
		dx5_d3d.lpSoftDeviceName=lpDeviceName;
	}
	/*if (strcmp(lpDeviceName, "MMX Emulation")==0)
	   	{
	   	dx5_d3d.lpSoftGuid=lpGuid;
	   	dx5_d3d.lpSoftDeviceName=lpDeviceName;
	   	}*/
	return D3DENUMRET_OK;
}


dx5_d3d_info *dx5_common_class::get_driver_hardware_info(IDirectDraw2 *dd, w32 mode)
{
	dx5_d3d.lpGuid=0;
	dx5_d3d.lpSoftDeviceName=0;
	dx5_d3d.lpSoftGuid=0;


	IDirect3D2 *d3d;
	if (!i4_dx5_check(dd->QueryInterface(IID_IDirect3D2,(void **)&d3d)))
	{
		return 0;
	}
	if (mode==DRIVER_MODE_INIT) //Otherwise, we wan't all devices enumerated
	{

		if (i4_win32_startup_options.render==R1_RENDER_DIRECTX5_SOFTWARE)
		{
			d3d->Release();
			d3d=0;
			return 0; //Don't use this rendering device at all
		}
		if (i4_win32_startup_options.render==R1_RENDER_DIRECTX5_USER_SETTING
			&&i4_win32_startup_options.render_data!=0)
		{
			//The user wants to manually choose which render device to use.
			d3d->EnumDevices(d3d_manual_callback,0);
			if (dx5_d3d.lpGuid)
			{
				d3d->Release();
				return &dx5_d3d;
			}
			i4_error("WARNING: The coosen rendering device for DirectX is not available. "
					 "Reverting to default.");

		}
	}
	d3d->EnumDevices(d3d_device_callback,0);
	d3d->Release();

	if (dx5_d3d.lpGuid)
	{
		return &dx5_d3d;
	}

	return 0;
}

void i4_dx5_image_class::put_pixel(i4_coord x, i4_coord y, w32 color)
{
	//char* a;
	w16 *a16;
	w8 *a8;
	//w32 b;
	//char *d;
	switch(pal->source.pixel_depth)
	{
		case I4_32BIT: //Target is same as source, so copy only.
			*paddr(x,y)=color;
			break;
		case I4_24BIT:

			//w32 *ad24=paddr(x,y);
			a8=(w8 *)paddr(x,y); //Wir müssen (leider) byteweise zugreifen
			//dafür ersparen wir uns den Aufruf der Konvertierungsfunktion
			*a8=(w8)(color); //r
			a8++;
			*a8=(w8)(color>>8); //g
			a8++;
			*a8=(w8)(color>>16); //b
			/*
			   b=bpl;
			   d=(char*)data;
			   __asm
			   	{
			   push ecx;
			   movzx eax,y;
			   mul b;
			   mov ecx,3
			   mov ebx,eax;
			   movzx eax,x;
			   mul ecx;//Uhh, teuer...
			   add ebx,eax;
			   add ebx,d;
			   mov ecx,dword ptr [ebx];
			   and ecx,0xff;
			   mov edx,color;
			   shl edx,8;
			   or ecx,edx;
			   mov dword ptr[ebx],ecx;
			   pop ecx;
			   	}*/
			break;
		case I4_16BIT:
			a16=(w16 *)paddr(x,y);
			*a16=(w16)i4_pal_man.convert_32_to(color, &pal->source);
			break;
		case I4_8BIT:
			a8=(w8 *)paddr(x,y);
			*a8=(w8)i4_pal_man.convert_32_to(color, &pal->source);
			break;
		default:
			i4_warning("Unsupported Bitdepth choosen.");
			a8=(w8 *)paddr(x,y);
			*a8=(w8)i4_pal_man.convert_32_to(color, &pal->source);
			break;
	}


	// WHY SHORT ???? JJ
	//->because short is correct for 16bit color depth
	//Well, we correct this. PG
}

i4_dx5_image_class::i4_dx5_image_class(w16 _w, w16 _h, w32 flags)
{
	w=_w;
	h=_h;

	DDSURFACEDESC ddsd;

	dx5_common.get_surface_description(dx5_common.primary_surface,ddsd);

	i4_pixel_format fmt;
	fmt.pixel_depth = (i4_pixel_depth)ddsd.ddpfPixelFormat.dwRGBBitCount;
	fmt.red_mask    = ddsd.ddpfPixelFormat.dwRBitMask;
	fmt.green_mask  = ddsd.ddpfPixelFormat.dwGBitMask;
	fmt.blue_mask   = ddsd.ddpfPixelFormat.dwBBitMask;
	fmt.alpha_mask  = 0;
	fmt.calc_shift();

	pal = i4_pal_man.register_pal(&fmt);

	//we want to create the surface w/the same pixel format as the primary surface
	surface = dx5_common.create_surface(DX5_SURFACE, w,h, flags, &ddsd.ddpfPixelFormat);

	if (!surface)
	{
		i4_warning("i4_dx5_image_class::create_surface failed");
	}
	else
	{
		lock();
		unlock();
	}
}
/*
   w32* i4_dx5_image_class::paddr(int x, int y)
   	{
   	w32 x1=x;
   	w32 target=0;
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
   	target=((w32)data)+target;
   	target=target+(y*bpl);
   	return (w32 *)target;
   	}
 */

void i4_dx5_image_class::put_part(i4_image_class *to, i4_coord x, i4_coord y, i4_coord x1,
								  i4_coord y1, i4_coord x2, i4_coord y2, i4_draw_context_class &context)
{
	i4_image_class::put_part(to,x,y,x1,y1,x2,y2,context);
};


void i4_dx5_image_class::lock()
{
	DDSURFACEDESC ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize=sizeof(ddsd);

	if (!i4_dx5_check(surface->Lock(0, &ddsd,DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_NOSYSLOCK,0)))

	{
		i4_warning("couldn't lock surface");
		Sleep(5000);
		if (surface->Restore()!=DD_OK)
		{
			i4_error("Restoring DirectDraw not yet supported");
		}
		surface->Lock(0,&ddsd,DDLOCK_WRITEONLY|DDLOCK_WAIT|DDLOCK_NOSYSLOCK,0);
	}

	data = (w8 *)ddsd.lpSurface;
	bpl  = ddsd.lPitch;
}

void i4_dx5_image_class::unlock()
{
	surface->Unlock(0);
}


i4_dx5_image_class::~i4_dx5_image_class()
{
	if (surface)
	{
		//surface->PageUnlock(0);
		surface->Release();
	}
}

IDirectDrawSurface3 *dx5_common_class::get_surface(i4_image_class *im)
{
	return ((i4_dx5_image_class *)im)->surface;

}



i4_image_class *dx5_common_class::create_image(int w, int h, w32 surface_flags)
{
	return new i4_dx5_image_class(w,h,surface_flags);
}
