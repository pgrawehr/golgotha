// Optionsdialog.cpp: Implementierungsdatei
//
#include "pch.h"
#include "stdafx.h"
#include "resource.h"
#include "Optionsdialog.h"
#include "main/win_main.h"
#include "video/win32/dx9_error.h"
//#include "render/dx5/r1_dx5.h"
//#include "render/dx9/render.h"
#include "app/registry.h"

//#ifdef _DEBUG
//#undef new
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

#define ENTRY_AUTO_SELECT 1
#define ENTRY_PLAIN_SOFTWARE 2
#define ENTRY_DX9_HAL 3
#define ENTRY_DX9_REF 4
/////////////////////////////////////////////////////////////////////////////
// Dialogfeld OptionsDialog


OptionsDialog::OptionsDialog(CWnd * pParent /*=NULL*/)
	: CPropertyPage(OptionsDialog::IDD)
{
	//{{AFX_DATA_INIT(OptionsDialog)
	m_windowed = FALSE;
	//}}AFX_DATA_INIT
	lpDevice=NULL;
	resolutionchanged=false;
}

OptionsDialog::~OptionsDialog()
{
	//Cleanup();
}

void OptionsDialog::DoDataExchange(CDataExchange * pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OptionsDialog)
	DDX_Control(pDX, IDC_TexQualCombo, m_TexQualCombo);
	DDX_Control(pDX, IDC_VIEW_QUAL_TEXT, m_view_range_text);
	DDX_Control(pDX, IDC_VIEW_RANGE, m_view_range);
	DDX_Control(pDX, IDC_TEXTURE_QUALITY, m_texture_quality);
	DDX_Control(pDX, IDC_TEXTURE_QUAL_TEXT, m_texture_qual_text);
	DDX_Control(pDX, IDC_RENDER_DEVICE, m_render_device);
	DDX_Control(pDX, IDC_CURRENTMODE, m_currentmode);
	DDX_Control(pDX, IDC_CREATE, m_create);
	DDX_Control(pDX, IDC_DEVICES, m_devices);
	DDX_Control(pDX, IDC_RESOLUTIONS, m_resolutions);
	DDX_Check(pDX, IDC_WINDOWED, m_windowed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptionsDialog, CPropertyPage)
//{{AFX_MSG_MAP(OptionsDialog)
ON_WM_CREATE()
ON_BN_CLICKED(IDC_CREATE, OnBtnCreate)
ON_WM_DELETEITEM()
ON_BN_CLICKED(IDC_WINDOWED, OnWindowed)
ON_LBN_SELCHANGE(IDC_RESOLUTIONS, OnSelchangeResolutions)
ON_CBN_SELCHANGE(IDC_RENDER_DEVICE, OnSelchangeRenderDevice)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_TEXTURE_QUALITY, OnReleasedcaptureTextureQuality)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_VIEW_RANGE, OnReleasedcaptureView)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_Tex16, OnTex16)
ON_BN_CLICKED(IDC_Tex32, OnTex32)
ON_BN_CLICKED(IDC_TEXDEF, OnTexdef)
ON_WM_CLOSE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten OptionsDialog

int OptionsDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// TODO: Speziellen Erstellungscode hier einfügen


	return 0;
}
BOOL WINAPI EnumDDrawDevice( GUID FAR * lpGUID,
							LPSTR lpDriverDescription,
							LPSTR lpDriverName,
							LPVOID lpContext,
							HMONITOR hm)
{
	LONG iIndex;
	//HWND    hWnd = ( HWND )lpContext;
	CComboBox * m=(CComboBox *)lpContext;
	LPVOID lpDevice = NULL;
	//lpDriverName=lpDriverName;//Disable Warning
	char buf[500];
	char tbuf[200];

	strcpy(tbuf,"%s auf Monitor ID %i");
	LoadString(i4_win32_instance,IDS_MONITORDESC,tbuf,200);
	wsprintf(buf,tbuf,lpDriverDescription,hm);
	iIndex=m->AddString(buf);
	//iIndex = SendMessage( hWnd, CB_ADDSTRING, 0,( LPARAM )lpDriverDescription );
	//SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)lpDriverName);
	//Teste das hier

	// Wenn das Einsetzen in das Listenfeld geklappt hat:
	// GUID kopieren und einen Zeiger darauf in dem gleichen Element
	// des Listenfelds speichern.

	if ( iIndex != LB_ERR )
	{
		// NULL entspricht dem primären Gerät, das *keinen* GUID
		// mitgeliefert bekommt. Muß immer geprüft werden!
		if ( lpGUID == NULL )
		{
			lpDevice = NULL;
		}
		else
		{
			lpDevice = ( LPGUID )malloc( sizeof( GUID ) );
			if ( !lpDevice )
			{
				return FALSE;
			}
			memcpy( lpDevice, lpGUID, sizeof( GUID ) );
		}
		m->SetItemData(iIndex,(DWORD)lpDevice);
		//SendMessage( hWnd, CB_SETITEMDATA, iIndex, ( LPARAM )lpDevice );
	}
	else
	{
		return DDENUMRET_CANCEL;
	}

	return DDENUMRET_OK;
}

BOOL WINAPI EnumDisplayModes( LPDDSURFACEDESC lpDDSurfaceDesc,
							 LPVOID lpContext )
{
	LONG iIndex;
	char buff[256];
	//HWND    hWnd = ( HWND )lpContext;
	CListBox * m=(CListBox *)lpContext;
	LPVOID lpDesc = NULL;
	char templateb[200];

	strcpy(templateb,"%dx%dx%d");
	LoadString(i4_win32_instance,IDS_RESOLUTIONDESC,templateb,200);
	wsprintf( buff, templateb, //"%dx%d, Farbtiefe %d Bit",
			 lpDDSurfaceDesc->dwWidth,
			 lpDDSurfaceDesc->dwHeight,
			 lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);

	iIndex = m->AddString(buff);
	//SendMessage( hWnd, LB_ADDSTRING, 0, ( LONG )( LPSTR )buff );

	// Wenn das Einsetzen in die Listbox geklappt hat, eine Kopie der
	// Oberflächenbeschreibung anlegen und den Zeiger darauf im selben
	// Listenelement speichern. Diese Daten werden später zum Setzen
	// des gewünschten Videomodus gebraucht

	if ( iIndex != LB_ERR )
	{
		lpDesc = ( LPDDSURFACEDESC )malloc( sizeof( DDSURFACEDESC ) );
		if ( !lpDesc )
		{
			return FALSE;
		}

		memcpy( lpDesc, lpDDSurfaceDesc, sizeof( DDSURFACEDESC ) );

		//SendMessage( hWnd, LB_SETITEMDATA, iIndex, ( LPARAM )lpDesc );
		m->SetItemData(iIndex,(ULONG)lpDesc);
	}
	else
	{
		return DDENUMRET_CANCEL;
	}

	return DDENUMRET_OK;
}

//
//HRESULT WINAPI d3denumdevicescallback(LPGUID lpGuid,
//									  LPSTR lpDeviceDescription,
//									  LPSTR lpDeviceName,
//									  LPD3DDEVICEDESC lpD3DHWDeviceDesc,
//									  LPD3DDEVICEDESC lpD3DHELDeviceDesc,
//									  LPVOID lpUserArg
//)
//{
//	CComboBox * list=(CComboBox *)lpUserArg;
//	char buf[500];
//	int index=0;
//
//	sprintf(buf,"%s (%s)",lpDeviceDescription,lpDeviceName);
//	index=list->AddString(buf);
//	list->SetItemDataPtr(index,lpDeviceName);
//	return DDENUMRET_OK;
//}

void OptionsDialog::Apply()
{
	if (!m_hWnd)
	{
		return;
	}
	int checked=0;
	i4_win32_startup_options.fullscreen = !IsDlgButtonChecked(IDC_WINDOWED);
	m_windowed= !i4_win32_startup_options.fullscreen;
	if (lpDevice)
	{
		memcpy(&i4_win32_startup_options.guid_screen,lpDevice,sizeof(GUID));
	}
	else
	{
		ZeroMemory(&i4_win32_startup_options.guid_screen,sizeof(GUID));
	}

	LPDDSURFACEDESC lpdesc=NULL;
	if (m_devices.GetItemData(m_devices.GetCurSel())==1)
	{
		i4_set_registry(I4_REGISTRY_USER,0,"display","Windowed GDI");
	}
	else
	{
		i4_set_registry(I4_REGISTRY_USER,0,"display","");
	}
	int isel=m_resolutions.GetCurSel();
	if (isel>0)
	{
		resolutionchanged=true;
		lpdesc=(LPDDSURFACEDESC) m_resolutions.GetItemData(isel);
		if(lpdesc)
		{
			i4_win32_startup_options.xres=(short)lpdesc->dwWidth;
			i4_win32_startup_options.yres=(short)lpdesc->dwHeight;
			i4_win32_startup_options.bits=(short)lpdesc->ddpfPixelFormat.dwRGBBitCount;
		}
	}
	char buf[256];
	ZeroMemory(buf,256);
	char tbuf[256];
	strcpy(tbuf,"Auflösung beim nächsten Programmstart:\n%dx%d mit Farbtiefe %d Bit. %s\n");
	LoadString(i4_win32_instance,IDS_RESOLUTIONSTRING,tbuf,256);
	wsprintf(buf,tbuf,
			 i4_win32_startup_options.xres,i4_win32_startup_options.yres,
			 i4_win32_startup_options.bits,
			 i4_win32_startup_options.fullscreen ? "" : "(Windowed)");
	m_currentmode.SetWindowText(buf);
	double d=m_texture_quality.GetPos();
	d=pow(2.0,d);
	i4_win32_startup_options.max_texture_quality=(int)d;
	i4_win32_startup_options.max_view_distance=m_view_range.GetPos();
	m_texture_qual_text.SetWindowText(_itoa((int) d,buf,10));
	w32 tb=0;
	if (IsDlgButtonChecked(IDC_Tex16))
	{
		tb=16;
	}
	if (IsDlgButtonChecked(IDC_Tex32))
	{
		tb=32;
	}
	i4_win32_startup_options.texture_bitdepth=tb;
	int index=m_render_device.GetCurSel();
	DWORD indexdata=m_render_device.GetItemData(index);
	if (indexdata<0x1000)
	{
		if (indexdata==ENTRY_AUTO_SELECT)
		{
			i4_win32_startup_options.render=R1_RENDER_USEDEFAULT;
			if (i4_win32_startup_options.render_data)
			{
				free(i4_win32_startup_options.render_data);
			}
			i4_win32_startup_options.render_data=0;
			i4_win32_startup_options.render_data_size=0;
		}
		if (indexdata==ENTRY_PLAIN_SOFTWARE)
		{
			i4_win32_startup_options.render=R1_RENDER_DIRECTX5_SOFTWARE;
			if (i4_win32_startup_options.render_data)
			{
				free(i4_win32_startup_options.render_data);
			}
			i4_win32_startup_options.render_data=0;
			i4_win32_startup_options.render_data_size=0;
		}
		if (indexdata==ENTRY_DX9_HAL)
		{
			i4_win32_startup_options.render=R1_RENDER_DIRECTX9_HAL;
			if (i4_win32_startup_options.render_data)
			{
				free(i4_win32_startup_options.render_data);
			}
			i4_win32_startup_options.render_data=0;
			i4_win32_startup_options.render_data_size=0;
		}
		if (indexdata==ENTRY_DX9_REF)
		{
			i4_win32_startup_options.render=R1_RENDER_DIRECTX9_REF;
			if (i4_win32_startup_options.render_data)
			{
				free(i4_win32_startup_options.render_data);
			}
			i4_win32_startup_options.render_data=0;
			i4_win32_startup_options.render_data_size=0;
		}
	}
	else
	{
		i4_win32_startup_options.render=R1_RENDER_DIRECTX5_USER_SETTING;
		char * dt=(char *)m_render_device.GetItemDataPtr(index);
		int len=strlen(dt);
		if (i4_win32_startup_options.render_data)
		{
			free(i4_win32_startup_options.render_data);
		}
		i4_win32_startup_options.render_data=(char *)malloc(len+1);
		strcpy(i4_win32_startup_options.render_data,dt);
		i4_win32_startup_options.render_data_size=len+1;
	}
}



BOOL OptionsDialog::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_windowed= (!i4_win32_startup_options.fullscreen);
	CheckDlgButton(IDC_WINDOWED,m_windowed ? BST_CHECKED : BST_UNCHECKED);

	// Get List of devices
	int gdi_id=m_devices.AddString("Windows GDI (compatible but slow)");
	m_devices.SetItemData(gdi_id,1);
#if I4_64BITCPU
	// ddraw.lib doesn't exist for 64 bit (although the functionality is there - Need to use LoadLibrary etc. to get this to work
#else 
	if ( FAILED( DirectDrawEnumerateEx( ( LPDDENUMCALLBACKEX )EnumDDrawDevice,
									   ( LPVOID )&m_devices,
									   DDENUM_ATTACHEDSECONDARYDEVICES|
									   DDENUM_DETACHEDSECONDARYDEVICES|
									   DDENUM_NONDISPLAYDEVICES) ) )
	{
		i4_warning("SEVERE: Cannot enumerate Direct Draw Devices.\n" );
		//Just ignore this warning...
		//return FALSE;
	}
#endif
	m_devices.SetCurSel(0);
	char buf[256];
	ZeroMemory(buf,256);
	char tbuf[256];
	strcpy(tbuf,"Auflösung beim nächsten Programmstart:\n%dx%d mit Farbtiefe %d Bit. %s\n");
	LoadString(i4_win32_instance,IDS_RESOLUTIONSTRING,tbuf,256);
	wsprintf(buf,tbuf,
			 i4_win32_startup_options.xres,i4_win32_startup_options.yres,
			 i4_win32_startup_options.bits,
			 i4_win32_startup_options.fullscreen ? "" : "(Windowed)");
	m_currentmode.SetWindowText(buf);
	m_render_device.AddString("default"); //need to change this
	m_texture_quality.SetRange(3,10,TRUE);
	m_texture_quality.SetLineSize(1);
	//Set the slider to the base-2 log of the texture size
	double lg2=log((double) i4_win32_startup_options.max_texture_quality)/log(2.0);
	m_texture_quality.SetPos((int)lg2);
	m_view_range.SetRange(50,300,TRUE);
	m_view_range.SetPos(i4_win32_startup_options.max_view_distance);
	m_view_range.SetPageSize(100);
	m_view_range.SetLineSize(1);
	//m_view_range.SetWindowText(i4_win32_startup_options.max_view_distance);
	char s[20];
	int index;
	m_texture_qual_text.SetWindowText(_itoa(i4_win32_startup_options.max_texture_quality,s,10));
	m_view_range_text.SetWindowText(_itoa(i4_win32_startup_options.max_view_distance,s,10));
	m_render_device.ResetContent();
	index=m_render_device.AddString("Auto-Select best");
	m_render_device.SetItemData(index,ENTRY_AUTO_SELECT);
	index=m_render_device.AddString("Golgotha plain Software");
	m_render_device.SetItemData(index,ENTRY_PLAIN_SOFTWARE);
	
	//We can't really test this here, since including both
	//dx5 and dx9 headers doesn't work.
	index=m_render_device.AddString("DX9 HAL (default)");
	m_render_device.SetItemData(index,ENTRY_DX9_HAL);
	index=m_render_device.AddString("DX9 Reference (slow)");
	m_render_device.SetItemData(index,ENTRY_DX9_REF);
	//index=m_render_device.AddString("DX9 Software (medium)");
	//m_render_device.SetItemData(index,ENTRY_DX9_SW);

	m_render_device.SetCurSel(0);
	if (i4_win32_startup_options.render==R1_RENDER_DIRECTX5_SOFTWARE)
	{
		m_render_device.SetCurSel(1);
	}
	w32 tb=i4_win32_startup_options.texture_bitdepth;

	if (i4_win32_startup_options.bits==16)
	{
		if (tb==32)
		{
			tb=0;
			i4_win32_startup_options.texture_bitdepth=0;
		}
		CWnd * hWndTex32=GetDlgItem(IDC_Tex32);
		//::EnableWindow(hWndTex32,FALSE);
		hWndTex32->EnableWindow(FALSE);
	}

	if (tb==16)
	{
		CheckRadioButton(IDC_Tex16,IDC_TEXDEF,IDC_Tex16);
	}
	else if (tb==32)
	{
		CheckRadioButton(IDC_Tex16,IDC_TEXDEF,IDC_Tex32);
	}
	else
	{
		CheckRadioButton(IDC_Tex16,IDC_TEXDEF,IDC_TEXDEF);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void OptionsDialog::OnBtnCreate()
{
	//Modi dieses Geräts abzählen.
	UINT iIndex;
	LPDIRECTDRAW lpDD=NULL;
	LPDIRECTDRAW2 lpDD2=NULL;

	for (int j=0; j<m_resolutions.GetCount(); j++)
	{
		void * p=m_resolutions.GetItemDataPtr(j);
		if ((int)p>0x1000)
		{
			free(p);
		}
		m_resolutions.SetItemDataPtr(j,0);
	}
	m_resolutions.ResetContent();
	resolutionchanged=true;
	// GUID des ausgewählten Gerätes aus der Listbox (NULL für das primäre Gerät)
	iIndex =m_devices.GetCurSel();
	//SendDlgItemMessage( IDC_DEVICES,
	//                         CB_GETCURSEL, 0, 0L );
	int indexdata=m_devices.GetItemData(iIndex);
	if (indexdata==1)
	{
		//is the gdi entry. We have to emulate the mode enumeration
		LPDDSURFACEDESC lpWinDesc=0; //Use the same data format as
		//for the normal dx cases
		lpWinDesc=(LPDDSURFACEDESC)malloc(sizeof(DDSURFACEDESC));
		ZeroMemory(lpWinDesc,sizeof(DDSURFACEDESC));
		lpWinDesc->dwSize=sizeof(DDSURFACEDESC);
		lpWinDesc->ddpfPixelFormat.dwRGBBitCount=16;
		lpWinDesc->dwWidth=640;
		lpWinDesc->dwHeight=480;
		iIndex=m_resolutions.AddString("640x480");
		m_resolutions.SetItemDataPtr(iIndex,lpWinDesc);
		lpWinDesc=(LPDDSURFACEDESC)malloc(sizeof(DDSURFACEDESC));
		ZeroMemory(lpWinDesc,sizeof(DDSURFACEDESC));
		lpWinDesc->dwSize=sizeof(DDSURFACEDESC);
		lpWinDesc->ddpfPixelFormat.dwRGBBitCount=16;
		lpWinDesc->dwWidth=800;
		lpWinDesc->dwHeight=600;
		iIndex=m_resolutions.AddString("800x600");
		m_resolutions.SetItemDataPtr(iIndex,lpWinDesc);

		lpWinDesc=(LPDDSURFACEDESC)malloc(sizeof(DDSURFACEDESC));
		ZeroMemory(lpWinDesc,sizeof(DDSURFACEDESC));
		lpWinDesc->dwSize=sizeof(DDSURFACEDESC);
		lpWinDesc->ddpfPixelFormat.dwRGBBitCount=16;
		lpWinDesc->dwWidth=1024;
		lpWinDesc->dwHeight=768;
		iIndex=m_resolutions.AddString("1024x768");
		m_resolutions.SetItemDataPtr(iIndex,lpWinDesc);
		lpWinDesc=(LPDDSURFACEDESC)malloc(sizeof(DDSURFACEDESC));
		ZeroMemory(lpWinDesc,sizeof(DDSURFACEDESC));
		lpWinDesc->dwSize=sizeof(DDSURFACEDESC);
		lpWinDesc->ddpfPixelFormat.dwRGBBitCount=16;
		lpWinDesc->dwWidth=1280;
		lpWinDesc->dwHeight=1024;
		iIndex=m_resolutions.AddString("1280x1024");
		m_resolutions.SetItemDataPtr(iIndex,lpWinDesc);
		m_resolutions.SetCurSel(0);

		SetModified(TRUE);
		return;
	}
	lpDevice = (LPGUID)m_devices.GetItemData(iIndex);
	//( LPGUID )SendDlgItemMessage(
	//                     IDC_DEVICES,
	//                     CB_GETITEMDATA, iIndex, 0 );
	/*free(lpourdevice);
	   if (lpDevice!=NULL)
	   {
	   lpourdevice = ( LPGUID )malloc( sizeof( GUID ) );
	   if ( !lpourdevice ) return FALSE;
	   memcpy( lpourdevice, lpDevice, sizeof( GUID ) );
	   }else lpourdevice=NULL;
	 */
	// DirectDraw-Objekt anlegen, lpDevice kann wie gesagt NULL sein
#if I4_64BITCPU
	i4_warning("Enumerating modes is not implemented for 64 bit yet");
	return;
#else
	if ( !i4_dx9_check(DirectDrawCreate( lpDevice, &lpDD, NULL ) ) )
	{
		int a=MessageBox("Sorry, there's currently a problem creating a DirectDraw object for this device.\n"
						 "Would you like to use it anyway? If you choose yes, it will be used upon next restart with the current resolution.",
						 "DirectDraw problem",MB_YESNO+MB_ICONINFORMATION);
		if (a==IDYES)
		{
			memcpy(&i4_win32_startup_options,lpDevice,sizeof(GUID));
			//How to close the Property-Sheet-Window from here?
		}
		//DestroyWindow();
		i4_warning("Fehler beim Anlegen des DirectDraw-Objekts.\n" );
		return;
	}

	// Abfrage der IDirectDraw-Schnittstelle nach IDirectDraw2
	if ( FAILED( lpDD->QueryInterface( IID_IDirectDraw2,
									  ( LPVOID * )&lpDD2 ) ) )
	{
		i4_warning("DirectDraw2-Schnittstelle nicht gefunden.\n" );
		return;
	}

	// Freigabe der IDirectDraw-Schnittstelle - wird nicht mehr gebraucht
	lpDD->Release();



	// Verfügbare Modi abzählen
	if ( FAILED( lpDD2->EnumDisplayModes(DDEDM_STANDARDVGAMODES, NULL,
										 ( LPVOID )&m_resolutions,
										 ( LPDDENUMMODESCALLBACK )EnumDisplayModes ) ) )
	{
		i4_warning("Error enumerating display modes.\n" );
		return;
	}

	m_resolutions.SetCurSel(0);
	lpDD2->Release();
	SetModified(TRUE);
	/*EnableWindow( GetDlgItem( hWnd,IDC_CREATE ), FALSE );
	   SetFocus(GetDlgItem(hWnd,IDC_MODES));
	   EnableWindow( GetDlgItem( hWnd,IDC_SET ), TRUE );
	   EnableWindow( GetDlgItem(hWnd,1021),TRUE);
	   CheckDlgButton(hWnd,1021,BST_CHECKED);
	 */
#endif
}

void OptionsDialog::OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	if (nIDCtl==IDC_DEVICES||nIDCtl==IDC_RESOLUTIONS)
	{
		//Delete the pointers in the device and resolution list-boxes,
		//but don't delete those in the renderers list box
		//They point to some data in the directx-dll!! =>BSOD
		//And don't delete the special-purpose markers.
		if (((DWORD)lpDeleteItemStruct->itemData)>0x1000)
		{
			free((LPVOID)lpDeleteItemStruct->itemData );
		}
	}
	lpDeleteItemStruct->itemData=NULL;
	CPropertyPage::OnDeleteItem(nIDCtl, lpDeleteItemStruct);

}

void OptionsDialog::OnWindowed()
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SetModified(TRUE);
}

void OptionsDialog::Cleanup()
{
	for (int i=0; i<m_devices.GetCount(); i++)
	{
		void * p=m_devices.GetItemDataPtr(i);
		if ((int)p>0x1000)
		{
			free(p);
		}
		m_devices.SetItemDataPtr(i,0);
	}
	m_devices.ResetContent();
	for (int j=0; j<m_resolutions.GetCount(); j++)
	{
		void * p=m_resolutions.GetItemDataPtr(j);
		if ((int)p>0x1000)
		{
			free(p);
		}
		m_resolutions.SetItemDataPtr(j,0);
	}
	m_resolutions.ResetContent();
};

BOOL OptionsDialog::DestroyWindow()
{
	Cleanup();
	return CPropertyPage::DestroyWindow();
}

void OptionsDialog::OnSelchangeResolutions()
{
	LPDDSURFACEDESC lpdesc;
	int isel=m_resolutions.GetCurSel();

	if (isel<0)
	{
		return;
	}
	lpdesc=(LPDDSURFACEDESC) m_resolutions.GetItemData(isel);
	w32 btd=lpdesc->ddpfPixelFormat.dwRGBBitCount;
	CheckRadioButton(IDC_Tex16,IDC_TEXDEF,IDC_TEXDEF);
	CWnd * hWndTex32=GetDlgItem(IDC_Tex32);
	CWnd * hWndTex16=GetDlgItem(IDC_Tex16);
	if (btd==16)
	{
		//::EnableWindow(hWndTex32,FALSE);
		hWndTex32->EnableWindow(FALSE);
	}
	else
	{
		//::EnableWindow(hWndTex32,TRUE);
		hWndTex32->EnableWindow(TRUE);
	}
	SetModified(TRUE);
}

void OptionsDialog::OnSelchangeRenderDevice()
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SetModified(TRUE);
}

void OptionsDialog::OnReleasedcaptureTextureQuality(NMHDR * pNMHDR, LRESULT * pResult)
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	char s[20];
	double d=m_texture_quality.GetPos();

	d=pow(2.0,d);
	m_texture_qual_text.SetWindowText(_itoa((int) d,s,10));
	*pResult = 0;
}

void OptionsDialog::OnReleasedcaptureView(NMHDR * pNMHDR, LRESULT * pResult)
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	char s[20];
	double d=m_view_range.GetPos();

	m_view_range_text.SetWindowText(_itoa((int) d,s,10));
	*pResult = 0;
}

void OptionsDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	char s[20];
	double d;

	if ((CSliderCtrl *)pScrollBar==&m_texture_quality)
	{
		d=pow(2.0,m_texture_quality.GetPos());
		m_texture_qual_text.SetWindowText(_itoa((int) d,s,10));
	}
	if ((CSliderCtrl *)pScrollBar==&m_view_range)
	{
		d=m_view_range.GetPos();
		m_view_range_text.SetWindowText(_itoa((int) d,s,10));
	}
	SetModified();
}

void OptionsDialog::OnTex16()
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SetModified(TRUE);
}

void OptionsDialog::OnTex32()
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SetModified(TRUE);
}

void OptionsDialog::OnTexdef()
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SetModified();
}

void OptionsDialog::OnClose()
{
	Cleanup();

	CPropertyPage::OnClose();
}

void OptionsDialog::OnCancel()
{
	Cleanup();
	CPropertyPage::OnCancel();
}
