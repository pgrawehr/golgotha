/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "video/win32/dx9_util.h"
#include "video/win32/dx9_error.h"
#include "threads/threads.h"
#include "error/error.h"
#include "main/win_main.h"

//-----------------------------------------------------------------------------
// File: D3DEnumeration.cpp
//
// Desc: Enumerates D3D adapters, devices, modes, etc.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
//#define STRICT
#include <windows.h>
#include "D3D9.h"
//#include "DXUtil.h"
#include "video/win32/d3denumeration.h"


//-----------------------------------------------------------------------------
// Name: ColorChannelBits
// Desc: Returns the number of color channel bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
static UINT ColorChannelBits( D3DFORMAT fmt )
{
	switch( fmt )
	{
		case D3DFMT_R8G8B8:
			return 8;

		case D3DFMT_A8R8G8B8:
			return 8;

		case D3DFMT_X8R8G8B8:
			return 8;

		case D3DFMT_R5G6B5:
			return 5;

		case D3DFMT_X1R5G5B5:
			return 5;

		case D3DFMT_A1R5G5B5:
			return 5;

		case D3DFMT_A4R4G4B4:
			return 4;

		case D3DFMT_R3G3B2:
			return 2;

		case D3DFMT_A8R3G3B2:
			return 2;

		case D3DFMT_X4R4G4B4:
			return 4;

		case D3DFMT_A2B10G10R10:
			return 10;

		case D3DFMT_A2R10G10B10:
			return 10;

		default:
			return 0;
	}
}




//-----------------------------------------------------------------------------
// Name: AlphaChannelBits
// Desc: Returns the number of alpha channel bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
static UINT AlphaChannelBits( D3DFORMAT fmt )
{
	switch( fmt )
	{
		case D3DFMT_R8G8B8:
			return 0;

		case D3DFMT_A8R8G8B8:
			return 8;

		case D3DFMT_X8R8G8B8:
			return 0;

		case D3DFMT_R5G6B5:
			return 0;

		case D3DFMT_X1R5G5B5:
			return 0;

		case D3DFMT_A1R5G5B5:
			return 1;

		case D3DFMT_A4R4G4B4:
			return 4;

		case D3DFMT_R3G3B2:
			return 0;

		case D3DFMT_A8R3G3B2:
			return 8;

		case D3DFMT_X4R4G4B4:
			return 0;

		case D3DFMT_A2B10G10R10:
			return 2;

		case D3DFMT_A2R10G10B10:
			return 2;

		default:
			return 0;
	}
}




//-----------------------------------------------------------------------------
// Name: DepthBits
// Desc: Returns the number of depth bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
static UINT DepthBits( D3DFORMAT fmt )
{
	switch( fmt )
	{
		case D3DFMT_D16:
			return 16;

		case D3DFMT_D15S1:
			return 15;

		case D3DFMT_D24X8:
			return 24;

		case D3DFMT_D24S8:
			return 24;

		case D3DFMT_D24X4S4:
			return 24;

		case D3DFMT_D32:
			return 32;

		default:
			return 0;
	}
}




//-----------------------------------------------------------------------------
// Name: StencilBits
// Desc: Returns the number of stencil bits in the specified D3DFORMAT
//-----------------------------------------------------------------------------
static UINT StencilBits( D3DFORMAT fmt )
{
	switch( fmt )
	{
		case D3DFMT_D16:
			return 0;

		case D3DFMT_D15S1:
			return 1;

		case D3DFMT_D24X8:
			return 0;

		case D3DFMT_D24S8:
			return 8;

		case D3DFMT_D24X4S4:
			return 4;

		case D3DFMT_D32:
			return 0;

		default:
			return 0;
	}
}




//-----------------------------------------------------------------------------
// Name: D3DAdapterInfo destructor
// Desc:
//-----------------------------------------------------------------------------
D3DAdapterInfo::~D3DAdapterInfo( void )
{
	if( pDisplayModeList != NULL )
	{
		delete pDisplayModeList;
	}
	if( pDeviceInfoList != NULL )
	{
		for( UINT idi = 0; idi < pDeviceInfoList->Count(); idi++ )
		{
			delete (D3DDeviceInfo *)pDeviceInfoList->GetPtr(idi);
		}
		delete pDeviceInfoList;
	}
}




//-----------------------------------------------------------------------------
// Name: D3DDeviceInfo destructor
// Desc:
//-----------------------------------------------------------------------------
D3DDeviceInfo::~D3DDeviceInfo( void )
{
	if( pDeviceComboList != NULL )
	{
		for( UINT idc = 0; idc < pDeviceComboList->Count(); idc++ )
		{
			delete (D3DDeviceCombo *)pDeviceComboList->GetPtr(idc);
		}
		delete pDeviceComboList;
	}
}




//-----------------------------------------------------------------------------
// Name: D3DDeviceCombo destructor
// Desc:
//-----------------------------------------------------------------------------
D3DDeviceCombo::~D3DDeviceCombo( void )
{
	if( pDepthStencilFormatList != NULL )
	{
		delete pDepthStencilFormatList;
	}
	if( pMultiSampleTypeList != NULL )
	{
		delete pMultiSampleTypeList;
	}
	if( pMultiSampleQualityList != NULL )
	{
		delete pMultiSampleQualityList;
	}
	if( pDSMSConflictList != NULL )
	{
		delete pDSMSConflictList;
	}
	if( pVertexProcessingTypeList != NULL )
	{
		delete pVertexProcessingTypeList;
	}
	if( pPresentIntervalList != NULL )
	{
		delete pPresentIntervalList;
	}
}



//-----------------------------------------------------------------------------
// Name: CD3DEnumeration constructor
// Desc:
//-----------------------------------------------------------------------------
CD3DEnumeration::CD3DEnumeration()
{
	m_pAdapterInfoList = NULL;
	m_pAllowedAdapterFormatList = NULL;
	AppMinFullscreenWidth = 640;
	AppMinFullscreenHeight = 480;
	AppMinColorChannelBits = 5;
	AppMinAlphaChannelBits = 0;
	AppMinDepthBits = 15;
	AppMinStencilBits = 0;
	AppUsesDepthBuffer = true;
	AppUsesMixedVP = false;
	AppRequiresWindowed = false;
	AppRequiresFullscreen = false;
	m_pD3D=0;
	ConfirmDeviceCallback=NULL;
}




//-----------------------------------------------------------------------------
// Name: CD3DEnumeration destructor
// Desc:
//-----------------------------------------------------------------------------
CD3DEnumeration::~CD3DEnumeration()
{
	if( m_pAdapterInfoList != NULL )
	{
		for( UINT iai = 0; iai < m_pAdapterInfoList->Count(); iai++ )
		{
			delete (D3DAdapterInfo *)m_pAdapterInfoList->GetPtr(iai);
		}
		delete m_pAdapterInfoList;
	}
	SAFE_DELETE( m_pAllowedAdapterFormatList );
}




//-----------------------------------------------------------------------------
// Name: SortModesCallback
// Desc: Used to sort D3DDISPLAYMODEs
//-----------------------------------------------------------------------------
static int __cdecl SortModesCallback( const void *arg1, const void *arg2 )
{
	D3DDISPLAYMODE *pdm1 = (D3DDISPLAYMODE *)arg1;
	D3DDISPLAYMODE *pdm2 = (D3DDISPLAYMODE *)arg2;

	if (pdm1->Width > pdm2->Width)
	{
		return 1;
	}
	if (pdm1->Width < pdm2->Width)
	{
		return -1;
	}
	if (pdm1->Height > pdm2->Height)
	{
		return 1;
	}
	if (pdm1->Height < pdm2->Height)
	{
		return -1;
	}
	if (pdm1->Format > pdm2->Format)
	{
		return 1;
	}
	if (pdm1->Format < pdm2->Format)
	{
		return -1;
	}
	if (pdm1->RefreshRate > pdm2->RefreshRate)
	{
		return 1;
	}
	if (pdm1->RefreshRate < pdm2->RefreshRate)
	{
		return -1;
	}
	return 0;
}




//-----------------------------------------------------------------------------
// Name: Enumerate
// Desc: Enumerates available D3D adapters, devices, modes, etc.
//-----------------------------------------------------------------------------
HRESULT CD3DEnumeration::Enumerate()
{
	HRESULT hr;
	CArrayList adapterFormatList( AL_VALUE, sizeof(D3DFORMAT) );

	if( m_pD3D == NULL )
	{
		return E_FAIL;
	}

	m_pAdapterInfoList = new CArrayList( AL_REFERENCE );
	if( m_pAdapterInfoList == NULL )
	{
		return E_OUTOFMEMORY;
	}

	m_pAllowedAdapterFormatList = new CArrayList( AL_VALUE, sizeof(D3DFORMAT) );
	if( m_pAllowedAdapterFormatList == NULL )
	{
		return E_OUTOFMEMORY;
	}
	D3DFORMAT fmt;
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_X8R8G8B8 ) ) ) )
	{
		return hr;
	}
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_X1R5G5B5 ) ) ) )
	{
		return hr;
	}
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_R5G6B5 ) ) ) )
	{
		return hr;
	}
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_A2R10G10B10 ) ) ) )
	{
		return hr;
	}
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_R8G8B8))))
	{
		return hr;
	}

	D3DAdapterInfo *pAdapterInfo = NULL;
	UINT numAdapters = m_pD3D->GetAdapterCount();

	for (UINT adapterOrdinal = 0; adapterOrdinal < numAdapters; adapterOrdinal++)
	{
		pAdapterInfo = new D3DAdapterInfo;
		if( pAdapterInfo == NULL )
		{
			return E_OUTOFMEMORY;
		}
		pAdapterInfo->pDisplayModeList = new CArrayList( AL_VALUE, sizeof(D3DDISPLAYMODE));
		pAdapterInfo->pDeviceInfoList = new CArrayList( AL_REFERENCE );
		if( pAdapterInfo->pDisplayModeList == NULL ||
		   pAdapterInfo->pDeviceInfoList == NULL )
		{
			delete pAdapterInfo;
			return E_OUTOFMEMORY;
		}
		pAdapterInfo->AdapterOrdinal = adapterOrdinal;
		m_pD3D->GetAdapterIdentifier(adapterOrdinal, 0, &pAdapterInfo->AdapterIdentifier);

		// Get list of all display modes on this adapter.
		// Also build a temporary list of all display adapter formats.
		adapterFormatList.Clear();
		for( UINT iaaf = 0; iaaf < m_pAllowedAdapterFormatList->Count(); iaaf++ )
		{
			D3DFORMAT allowedAdapterFormat = *(D3DFORMAT *)m_pAllowedAdapterFormatList->GetPtr( iaaf );
			UINT numAdapterModes = m_pD3D->GetAdapterModeCount( adapterOrdinal, allowedAdapterFormat );
			for (UINT mode = 0; mode < numAdapterModes; mode++)
			{
				D3DDISPLAYMODE displayMode;
				m_pD3D->EnumAdapterModes( adapterOrdinal, allowedAdapterFormat, mode, &displayMode );
				if( displayMode.Width < AppMinFullscreenWidth ||
				   displayMode.Height < AppMinFullscreenHeight ||
				   ColorChannelBits(displayMode.Format) < AppMinColorChannelBits )
				{
					continue;
				}
				pAdapterInfo->pDisplayModeList->Add(&displayMode);
				if( !adapterFormatList.Contains( &displayMode.Format ) )
				{
					adapterFormatList.Add( &displayMode.Format );
				}
			}
		}

		// Sort displaymode list
		qsort( pAdapterInfo->pDisplayModeList->GetPtr(0),
			  pAdapterInfo->pDisplayModeList->Count(), sizeof( D3DDISPLAYMODE ),
			  SortModesCallback );

		// Get info for each device on this adapter
		if( FAILED( hr = EnumerateDevices( pAdapterInfo, &adapterFormatList ) ) )
		{
			delete pAdapterInfo;
			return hr;
		}

		// If at least one device on this adapter is available and compatible
		// with the app, add the adapterInfo to the list
		if (pAdapterInfo->pDeviceInfoList->Count() == 0)
		{
			delete pAdapterInfo;
		}
		else
		{
			m_pAdapterInfoList->Add(pAdapterInfo);
		}
	}
	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumerateDevices
// Desc: Enumerates D3D devices for a particular adapter.
//-----------------------------------------------------------------------------
HRESULT CD3DEnumeration::EnumerateDevices( D3DAdapterInfo *pAdapterInfo,
										  CArrayList *pAdapterFormatList )
{
	const D3DDEVTYPE devTypeArray[] = {
		D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF
	};
	const UINT devTypeArrayCount = sizeof(devTypeArray) / sizeof(devTypeArray[0]);
	HRESULT hr;

	D3DDeviceInfo *pDeviceInfo = NULL;
	for( UINT idt = 0; idt < devTypeArrayCount; idt++ )
	{
		pDeviceInfo = new D3DDeviceInfo;
		if( pDeviceInfo == NULL )
		{
			return E_OUTOFMEMORY;
		}
		pDeviceInfo->pDeviceComboList = new CArrayList( AL_REFERENCE );
		if( pDeviceInfo->pDeviceComboList == NULL )
		{
			delete pDeviceInfo;
			return E_OUTOFMEMORY;
		}
		pDeviceInfo->AdapterOrdinal = pAdapterInfo->AdapterOrdinal;
		pDeviceInfo->DevType = devTypeArray[idt];
		if( FAILED( m_pD3D->GetDeviceCaps( pAdapterInfo->AdapterOrdinal,
										  pDeviceInfo->DevType, &pDeviceInfo->Caps ) ) )
		{
			delete pDeviceInfo;
			continue;
		}

		// Get info for each devicecombo on this device
		if( FAILED( hr = EnumerateDeviceCombos(pDeviceInfo, pAdapterFormatList) ) )
		{
			delete pDeviceInfo;
			return hr;
		}

		// If at least one devicecombo for this device is found,
		// add the deviceInfo to the list
		if (pDeviceInfo->pDeviceComboList->Count() == 0)
		{
			delete pDeviceInfo;
			continue;
		}
		pAdapterInfo->pDeviceInfoList->Add(pDeviceInfo);
	}
	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumerateDeviceCombos
// Desc: Enumerates DeviceCombos for a particular device.
//-----------------------------------------------------------------------------
HRESULT CD3DEnumeration::EnumerateDeviceCombos( D3DDeviceInfo *pDeviceInfo,
											   CArrayList *pAdapterFormatList )
{
	const D3DFORMAT backBufferFormatArray[] =
	{
		D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A2R10G10B10,
		D3DFMT_R5G6B5, D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5
	};
	const UINT backBufferFormatArrayCount = sizeof(backBufferFormatArray) / sizeof(backBufferFormatArray[0]);
	bool isWindowedArray[] = {
		false, true
	};

	// See which adapter formats are supported by this device
	D3DFORMAT adapterFormat;
	for( UINT iaf = 0; iaf < pAdapterFormatList->Count(); iaf++ )
	{
		adapterFormat = *(D3DFORMAT *)pAdapterFormatList->GetPtr(iaf);
		D3DFORMAT backBufferFormat;
		for( UINT ibbf = 0; ibbf < backBufferFormatArrayCount; ibbf++ )
		{
			backBufferFormat = backBufferFormatArray[ibbf];
			if (AlphaChannelBits(backBufferFormat) < AppMinAlphaChannelBits)
			{
				continue;
			}
			bool isWindowed;
			for( UINT iiw = 0; iiw < 2; iiw++)
			{
				isWindowed = isWindowedArray[iiw];
				if (!isWindowed && AppRequiresWindowed)
				{
					continue;
				}
				if (isWindowed && AppRequiresFullscreen)
				{
					continue;
				}
				if (FAILED(m_pD3D->CheckDeviceType(pDeviceInfo->AdapterOrdinal, pDeviceInfo->DevType,
												   adapterFormat, backBufferFormat, isWindowed)))
				{
					continue;
				}
				// At this point, we have an adapter/device/adapterformat/backbufferformat/iswindowed
				// DeviceCombo that is supported by the system.  We still need to confirm that it's
				// compatible with the app, and find one or more suitable depth/stencil buffer format,
				// multisample type, vertex processing type, and present interval.
				D3DDeviceCombo *pDeviceCombo = NULL;
				pDeviceCombo = new D3DDeviceCombo;
				if( pDeviceCombo == NULL )
				{
					return E_OUTOFMEMORY;
				}
				pDeviceCombo->pDepthStencilFormatList = new CArrayList( AL_VALUE, sizeof( D3DFORMAT ) );
				pDeviceCombo->pMultiSampleTypeList = new CArrayList( AL_VALUE, sizeof( D3DMULTISAMPLE_TYPE ) );
				pDeviceCombo->pMultiSampleQualityList = new CArrayList( AL_VALUE, sizeof( DWORD ) );
				pDeviceCombo->pDSMSConflictList = new CArrayList( AL_VALUE, sizeof( D3DDSMSConflict ) );
				pDeviceCombo->pVertexProcessingTypeList = new CArrayList( AL_VALUE, sizeof( VertexProcessingType ) );
				pDeviceCombo->pPresentIntervalList = new CArrayList( AL_VALUE, sizeof( UINT ) );
				if( pDeviceCombo->pDepthStencilFormatList == NULL ||
				   pDeviceCombo->pMultiSampleTypeList == NULL ||
				   pDeviceCombo->pMultiSampleQualityList == NULL ||
				   pDeviceCombo->pDSMSConflictList == NULL ||
				   pDeviceCombo->pVertexProcessingTypeList == NULL ||
				   pDeviceCombo->pPresentIntervalList == NULL )
				{
					delete pDeviceCombo;
					return E_OUTOFMEMORY;
				}
				pDeviceCombo->AdapterOrdinal = pDeviceInfo->AdapterOrdinal;
				pDeviceCombo->DevType = pDeviceInfo->DevType;
				pDeviceCombo->AdapterFormat = adapterFormat;
				pDeviceCombo->BackBufferFormat = backBufferFormat;
				pDeviceCombo->IsWindowed = isWindowed;
				if (AppUsesDepthBuffer)
				{
					BuildDepthStencilFormatList(pDeviceCombo);
					if (pDeviceCombo->pDepthStencilFormatList->Count() == 0)
					{
						delete pDeviceCombo;
						continue;
					}
				}
				BuildMultiSampleTypeList(pDeviceCombo);
				if (pDeviceCombo->pMultiSampleTypeList->Count() == 0)
				{
					delete pDeviceCombo;
					continue;
				}
				BuildDSMSConflictList(pDeviceCombo);
				BuildVertexProcessingTypeList(pDeviceInfo, pDeviceCombo);
				if (pDeviceCombo->pVertexProcessingTypeList->Count() == 0)
				{
					delete pDeviceCombo;
					continue;
				}
				BuildPresentIntervalList(pDeviceInfo, pDeviceCombo);

				pDeviceInfo->pDeviceComboList->Add(pDeviceCombo);
			}
		}
	}

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: BuildDepthStencilFormatList
// Desc: Adds all depth/stencil formats that are compatible with the device
//       and app to the given D3DDeviceCombo.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildDepthStencilFormatList( D3DDeviceCombo *pDeviceCombo )
{
	const D3DFORMAT depthStencilFormatArray[] =
	{
		D3DFMT_D16,
		D3DFMT_D15S1,
		D3DFMT_D24X8,
		D3DFMT_D24S8,
		D3DFMT_D24X4S4,
		D3DFMT_D32,
	};
	const UINT depthStencilFormatArrayCount = sizeof(depthStencilFormatArray) /
											  sizeof(depthStencilFormatArray[0]);

	D3DFORMAT depthStencilFmt;
	for( UINT idsf = 0; idsf < depthStencilFormatArrayCount; idsf++ )
	{
		depthStencilFmt = depthStencilFormatArray[idsf];
		if (DepthBits(depthStencilFmt) < AppMinDepthBits)
		{
			continue;
		}
		if (StencilBits(depthStencilFmt) < AppMinStencilBits)
		{
			continue;
		}
		if (SUCCEEDED(m_pD3D->CheckDeviceFormat(pDeviceCombo->AdapterOrdinal,
												pDeviceCombo->DevType, pDeviceCombo->AdapterFormat,
												D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depthStencilFmt)))
		{
			if (SUCCEEDED(m_pD3D->CheckDepthStencilMatch(pDeviceCombo->AdapterOrdinal,
														 pDeviceCombo->DevType, pDeviceCombo->AdapterFormat,
														 pDeviceCombo->BackBufferFormat, depthStencilFmt)))
			{
				pDeviceCombo->pDepthStencilFormatList->Add(&depthStencilFmt);
			}
		}
	}
}




//-----------------------------------------------------------------------------
// Name: BuildMultiSampleTypeList
// Desc: Adds all multisample types that are compatible with the device and app to
//       the given D3DDeviceCombo.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildMultiSampleTypeList( D3DDeviceCombo *pDeviceCombo )
{
	const D3DMULTISAMPLE_TYPE msTypeArray[] = {
		D3DMULTISAMPLE_NONE,
		D3DMULTISAMPLE_NONMASKABLE,
		D3DMULTISAMPLE_2_SAMPLES,
		D3DMULTISAMPLE_3_SAMPLES,
		D3DMULTISAMPLE_4_SAMPLES,
		D3DMULTISAMPLE_5_SAMPLES,
		D3DMULTISAMPLE_6_SAMPLES,
		D3DMULTISAMPLE_7_SAMPLES,
		D3DMULTISAMPLE_8_SAMPLES,
		D3DMULTISAMPLE_9_SAMPLES,
		D3DMULTISAMPLE_10_SAMPLES,
		D3DMULTISAMPLE_11_SAMPLES,
		D3DMULTISAMPLE_12_SAMPLES,
		D3DMULTISAMPLE_13_SAMPLES,
		D3DMULTISAMPLE_14_SAMPLES,
		D3DMULTISAMPLE_15_SAMPLES,
		D3DMULTISAMPLE_16_SAMPLES,
	};
	const UINT msTypeArrayCount = sizeof(msTypeArray) / sizeof(msTypeArray[0]);

	D3DMULTISAMPLE_TYPE msType;
	DWORD msQuality;
	for( UINT imst = 0; imst < msTypeArrayCount; imst++ )
	{
		msType = msTypeArray[imst];
		if (SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(pDeviceCombo->AdapterOrdinal, pDeviceCombo->DevType,
														 pDeviceCombo->BackBufferFormat, pDeviceCombo->IsWindowed, msType, &msQuality)))
		{
			pDeviceCombo->pMultiSampleTypeList->Add(&msType);
			pDeviceCombo->pMultiSampleQualityList->Add( &msQuality );
		}
	}
}




//-----------------------------------------------------------------------------
// Name: BuildDSMSConflictList
// Desc: Find any conflicts between the available depth/stencil formats and
//       multisample types.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildDSMSConflictList( D3DDeviceCombo *pDeviceCombo )
{
	D3DDSMSConflict DSMSConflict;

	for( UINT ids = 0; ids < pDeviceCombo->pDepthStencilFormatList->Count(); ids++ )
	{
		D3DFORMAT dsFmt = *(D3DFORMAT *)pDeviceCombo->pDepthStencilFormatList->GetPtr(ids);
		for( UINT ims = 0; ims < pDeviceCombo->pMultiSampleTypeList->Count(); ims++ )
		{
			D3DMULTISAMPLE_TYPE msType = *(D3DMULTISAMPLE_TYPE *)pDeviceCombo->pMultiSampleTypeList->GetPtr(ims);
			if( FAILED( m_pD3D->CheckDeviceMultiSampleType( pDeviceCombo->AdapterOrdinal, pDeviceCombo->DevType,
														   dsFmt, pDeviceCombo->IsWindowed, msType, NULL ) ) )
			{
				DSMSConflict.DSFormat = dsFmt;
				DSMSConflict.MSType = msType;
				pDeviceCombo->pDSMSConflictList->Add( &DSMSConflict );
			}
		}
	}
}




//-----------------------------------------------------------------------------
// Name: BuildVertexProcessingTypeList
// Desc: Adds all vertex processing types that are compatible with the device
//       and app to the given D3DDeviceCombo.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildVertexProcessingTypeList( D3DDeviceInfo *pDeviceInfo,
													D3DDeviceCombo *pDeviceCombo )
{
	VertexProcessingType vpt;
	if ((pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
	{
		if ((pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0)
		{
			if (ConfirmDeviceCallback == NULL ||
				ConfirmDeviceCallback(&pDeviceInfo->Caps, PURE_HARDWARE_VP,
									  pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
			{
				vpt = PURE_HARDWARE_VP;
				pDeviceCombo->pVertexProcessingTypeList->Add(&vpt);
			}
		}
		if (ConfirmDeviceCallback == NULL ||
			ConfirmDeviceCallback(&pDeviceInfo->Caps, HARDWARE_VP,
								  pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
		{
			vpt = HARDWARE_VP;
			pDeviceCombo->pVertexProcessingTypeList->Add(&vpt);
		}
		if (AppUsesMixedVP && (ConfirmDeviceCallback == NULL ||
							   ConfirmDeviceCallback(&pDeviceInfo->Caps, MIXED_VP,
													 pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat)))
		{
			vpt = MIXED_VP;
			pDeviceCombo->pVertexProcessingTypeList->Add(&vpt);
		}
	}
	if (ConfirmDeviceCallback == NULL ||
		ConfirmDeviceCallback(&pDeviceInfo->Caps, SOFTWARE_VP,
							  pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
	{
		vpt = SOFTWARE_VP;
		pDeviceCombo->pVertexProcessingTypeList->Add(&vpt);
	}
}




//-----------------------------------------------------------------------------
// Name: BuildPresentIntervalList
// Desc: Adds all present intervals that are compatible with the device and app
//       to the given D3DDeviceCombo.
//-----------------------------------------------------------------------------
void CD3DEnumeration::BuildPresentIntervalList( D3DDeviceInfo *pDeviceInfo,
											   D3DDeviceCombo *pDeviceCombo )
{
	const UINT piArray[] = {
		D3DPRESENT_INTERVAL_IMMEDIATE,
		D3DPRESENT_INTERVAL_DEFAULT,
		D3DPRESENT_INTERVAL_ONE,
		D3DPRESENT_INTERVAL_TWO,
		D3DPRESENT_INTERVAL_THREE,
		D3DPRESENT_INTERVAL_FOUR,
	};
	const UINT piArrayCount = sizeof(piArray) / sizeof(piArray[0]);

	UINT pi;
	for( UINT ipi = 0; ipi < piArrayCount; ipi++ )
	{
		pi = piArray[ipi];
		if( pDeviceCombo->IsWindowed )
		{
			if( pi == D3DPRESENT_INTERVAL_TWO ||
			   pi == D3DPRESENT_INTERVAL_THREE ||
			   pi == D3DPRESENT_INTERVAL_FOUR )
			{
				// These intervals are not supported in windowed mode.
				continue;
			}
		}
		// Note that D3DPRESENT_INTERVAL_DEFAULT is zero, so you
		// can't do a caps check for it -- it is always available.
		if( pi == D3DPRESENT_INTERVAL_DEFAULT ||
		   (pDeviceInfo->Caps.PresentationIntervals & pi) )
		{
			pDeviceCombo->pPresentIntervalList->Add( &pi );
		}
	}
}

//#define DEPTH 16


dx9_common_class dx9_common;
LPDIRECT3D9 dx9_common_class::pD3D9;
IDirect3DSurface9 *dx9_common_class::back_surface,
*dx9_common_class::front_surface;
IDirect3DDevice9 *dx9_common_class::device;
D3DPRESENT_PARAMETERS dx9_common_class::present;
DDPIXELFORMAT dx9_common_class::dd_fmt_565, dx9_common_class::dd_fmt_1555;
i4_pixel_format dx9_common_class::i4_fmt_565, dx9_common_class::i4_fmt_1555;
//LPDIRECTDRAWCLIPPER    dx9_common_class::lpddclipper;



//-----------------------------------------------------------------------------
// Name: CArrayList constructor
// Desc:
//-----------------------------------------------------------------------------
CArrayList::CArrayList( ArrayListType Type, UINT BytesPerEntry )
{
	if( Type == AL_REFERENCE )
	{
		BytesPerEntry = sizeof(void *);
	}
	m_ArrayListType = Type;
	m_pData = NULL;
	m_BytesPerEntry = BytesPerEntry;
	m_NumEntries = 0;
	m_NumEntriesAllocated = 0;
}



//-----------------------------------------------------------------------------
// Name: CArrayList destructor
// Desc:
//-----------------------------------------------------------------------------
CArrayList::~CArrayList( void )
{
	if( m_pData != NULL )
	{
		delete[] m_pData;
	}
}




//-----------------------------------------------------------------------------
// Name: CArrayList::Add
// Desc: Adds pEntry to the list.
//-----------------------------------------------------------------------------
HRESULT CArrayList::Add( void *pEntry )
{
	if( m_BytesPerEntry == 0 )
	{
		return E_FAIL;
	}
	if( m_pData == NULL || m_NumEntries + 1 > m_NumEntriesAllocated )
	{
		void *pDataNew;
		UINT NumEntriesAllocatedNew;
		if( m_NumEntriesAllocated == 0 )
		{
			NumEntriesAllocatedNew = 16;
		}
		else
		{
			NumEntriesAllocatedNew = m_NumEntriesAllocated * 2;
		}
		pDataNew = new BYTE[NumEntriesAllocatedNew * m_BytesPerEntry];
		if( pDataNew == NULL )
		{
			return E_OUTOFMEMORY;
		}
		if( m_pData != NULL )
		{
			CopyMemory( pDataNew, m_pData, m_NumEntries * m_BytesPerEntry );
			delete[] m_pData;
		}
		m_pData = pDataNew;
		m_NumEntriesAllocated = NumEntriesAllocatedNew;
	}

	if( m_ArrayListType == AL_VALUE )
	{
		CopyMemory( (BYTE *)m_pData + (m_NumEntries * m_BytesPerEntry), pEntry, m_BytesPerEntry );
	}
	else
	{
		*(((void **)m_pData) + m_NumEntries) = pEntry;
	}
	m_NumEntries++;

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CArrayList::Remove
// Desc: Remove the item at Entry in the list, and collapse the array.
//-----------------------------------------------------------------------------
void CArrayList::Remove( UINT Entry )
{
	// Decrement count
	m_NumEntries--;

	// Find the entry address
	BYTE *pData = (BYTE *)m_pData + (Entry * m_BytesPerEntry);

	// Collapse the array
	MoveMemory( pData, pData + m_BytesPerEntry, ( m_NumEntries - Entry ) * m_BytesPerEntry );
}




//-----------------------------------------------------------------------------
// Name: CArrayList::GetPtr
// Desc: Returns a pointer to the Entry'th entry in the list.
//-----------------------------------------------------------------------------
void *CArrayList::GetPtr( UINT Entry )
{
	if( m_ArrayListType == AL_VALUE )
	{
		return (BYTE *)m_pData + (Entry * m_BytesPerEntry);
	}
	else
	{
		return *(((void **)m_pData) + Entry);
	}
}




//-----------------------------------------------------------------------------
// Name: CArrayList::Contains
// Desc: Returns whether the list contains an entry identical to the
//       specified entry data.
//-----------------------------------------------------------------------------
bool CArrayList::Contains( void *pEntryData )
{
	for( UINT iEntry = 0; iEntry < m_NumEntries; iEntry++ )
	{
		if( m_ArrayListType == AL_VALUE )
		{
			if( memcmp( GetPtr(iEntry), pEntryData, m_BytesPerEntry ) == 0 )
			{
				return true;
			}
		}
		else
		{
			if( GetPtr(iEntry) == pEntryData )
			{
				return true;
			}
		}
	}
	return false;
}



IDirect3DSurface9 *dx9_common_class::create_surface(dx9_surface_type type,
													int width, int height,
													int flags,
													D3DFORMAT format)
{
	HRESULT result;
	D3DSURFACE_DESC ddsd;
	IDirect3DSurface9 *surface=0;
	//if (width<30||height<30) flags=((flags&(~DX5_VRAM))|DX5_SYSTEM_RAM);
	//Creates new texcache, fails to load textures
	memset(&ddsd,0,sizeof(D3DSURFACE_DESC));
	// Hint: This method should NOT be used to create textures.

	if ((type==DX9_PAGE_FLIPPED_PRIMARY_SURFACE)||
		(type==DX9_BACKBUFFERED_PRIMARY_SURFACE))
	{
		//We wan't to get the backbuffer and the render target buffer
		if (!back_surface)
		{
			device->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back_surface);
		}
		//Hm, is there still a way to get the primary surface? (not its contents)
		return back_surface;
	}
	if (type==DX9_SYSMEM)
	{
		i4_dx9_check(device->CreateOffscreenPlainSurface(width,height,format,
														 D3DPOOL_SYSTEMMEM,&surface,0));
		return surface;
	}
	result=device->CreateOffscreenPlainSurface(width,height,format,D3DPOOL_SYSTEMMEM,&surface,0);
	if (!i4_dx9_check(result))
	{
		return 0;
	}

	return surface;
}


void dx9_common_class::cleanup()
{
	delete d3denum;
	d3denum=0;
	i4_warning("dx9_common_class::cleanup() freeing directdraw interfaces.");
//  if (lpddclipper)
//	  {
//	  lpddclipper->Release();
//	  lpddclipper=0;
//	  }
	if (back_surface)
	{
		back_surface->Release();
		back_surface=0;
	}

	if (front_surface)
	{
		front_surface->Release();
		front_surface=0;
	}

	/*if (primary_surface)
	   {
	   primary_surface->Release();
	   primary_surface=0;
	   }  */

	//DDSCAPS tcaps;
	//tcaps.dwCaps = DDSCAPS_TEXTURE;
	w32 total_free;
	if (!device)
	{
		return;
	}
	total_free=dx9_common.device->GetAvailableTextureMem();
	i4_warning("Total Texture Memory Free just before Releasing the ddraw interface: %d",total_free);


	if (device)
	{
		if (device->Release()>0)
		{
			i4_warning("There are still references to the IDirectDrawDevice9 object. You should try to fix memory leaks.");
		}
		device=0;
	}
	if (pD3D9)
	{
		if (pD3D9->Release()>0)
		{
			i4_warning("The IDirect3D9 Interface was not properly released.");
		}
		pD3D9=0;
	}
}


dx9_common_class::dx9_common_class()
{
	device=0;
	pD3D9=0;
	back_surface=front_surface=0;
	//lpddclipper=0;

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
//static dx9_mode *dx_mode_list;
/*
   HRESULT WINAPI dx9_vidmode_callback(LPDDSURFACEDESC lpDDSurfaceDesc,LPVOID lpContext)
   {
   //Wir werden hier alle Modi bekommen. Filtere alle Modi raus,
   //Die weniger als 16 Bit Farbtiefe liefern.
   //Zur Zeit noch alle Modi dalassen, mal sehen wie das wird.
   dx9_mode *m=new dx9_mode(dx_mode_list);
   dx_mode_list=m;
   m->desc=*lpDDSurfaceDesc;

   return DDENUMRET_OK;
   }
 */
CArrayList *dx9_common_class::get_mode_list(UINT adaptor)
{
//  dx_mode_list=0;
	if (!d3denum)
	{
		d3denum=new CD3DEnumeration();
		d3denum->SetD3D(pD3D9);
		d3denum->Enumerate();
	}
	D3DAdapterInfo *adapt=(D3DAdapterInfo *)d3denum->m_pAdapterInfoList->GetPtr(adaptor);
	return adapt->pDisplayModeList;
}
/*
   void dx9_common_class::free_mode_list(dx9_mode *list)
   {
   while (list)
   {
   	dx9_mode *p=list;
   	list=list->next;
   	delete p;
   }
   }
 */


//************************** DRIVER ENUMERATION AND CREATION ************************
/*
   static dx9_driver *dx_driver_list=0;

   BOOL WINAPI dd_device_callback(LPGUID lpGuid, LPSTR lpDeviceDesc,
   							   LPSTR lpDriverName, LPVOID lpUserArg, HMONITOR hm)
   {
   dx9_driver *d=new dx9_driver(dx_driver_list);
   dx_driver_list=d;
   d->lpGuid = lpGuid;
   strcpy(d->DriverName, lpDriverName);
   strcpy(d->DeviceDesc, lpDeviceDesc);
   return DDENUMRET_OK;
   }
 */
CArrayList *dx9_common_class::get_driver_list()
{
	//if (dx_driver_list) free_driver_list(dx_driver_list);//This function can be called more than once.
//but its the callers responsability to delete the list after use.
	if (!d3denum)
	{
		d3denum=new CD3DEnumeration();
		d3denum->SetD3D(pD3D9);
		d3denum->Enumerate();
	}
	return d3denum->m_pAdapterInfoList;
}
/*
   void dx9_common_class::free_driver_list(dx9_driver *list)
   {
   while (list)
   {
   	dx9_driver *n=list;
   	list=list->next;
   	delete n;
   }
   }
 */
//static IDirectDraw         *dx9_ddraw;

/*
   BOOL WINAPI dd_create_callback(LPGUID lpGuid,	LPSTR lpDeviceDescription,
   							   LPSTR lpDriverName, LPVOID lpUserArg, HMONITOR hm)
   {
   char *desired_driver = (char *)lpUserArg;
   //PG: Changed this as I'm not shure wheter driver name is guaranteed to
   //be unique even when using multiple displays.
   //Using lpGuid instead, which certainly IS unique.
   if (!stricmp(desired_driver,lpDriverName))
   {
   	HRESULT res = DirectDrawCreate(lpGuid, &dx9_ddraw, 0);
   	return DDENUMRET_CANCEL;
   }
   if (!lpGuid) //is the primary display driver
   	  {
   	  for(int i=0;i<=sizeof(GUID);i++)
   		  {
   		  if (((w8 *)&i4_win32_startup_options.guid_screen)[i]!=0)
   			  //is the saved guid a null-guid?
   			  return DDENUMRET_OK;
   		  }
   	  i4_dx9_check(DirectDrawCreate(lpGuid,&dx9_ddraw,0));
   	  return DDENUMRET_CANCEL;
   	  }
   if (memcmp(lpGuid,&i4_win32_startup_options.guid_screen,sizeof(GUID))==0)
   	  {
   	  HRESULT res=DirectDrawCreate(lpGuid,&dx9_ddraw,0);
   	  if (!i4_dx9_check(res))
   		  {
   		  MessageBox(NULL,"The currently choosen display device failed to initialize.\n"
   			  "Reverting to default (primary display driver)","DirectX initialisation failed",MB_OK);
   		  ZeroMemory(&i4_win32_startup_options.guid_screen,sizeof(GUID));
   		  i4_dx9_check(DirectDrawCreate(NULL,&dx9_ddraw,0));
   		  }
   	  return DDENUMRET_CANCEL;
   	  }
   return DDENUMRET_OK;
   }
 */
LPDIRECT3D9 dx9_common_class::initialize_driver()
{
	if (pD3D9)
	{
		return pD3D9;
	}
	pD3D9=Direct3DCreate9(D3D_SDK_VERSION);
	return pD3D9;
}

D3DDeviceInfo *dx9_common_class::get_driver_hardware_info(LPDIRECT3D9 dd, w32 adapter)
{
	int a=0;
	a=(adapter& (~0x8001))/2;
	return (D3DDeviceInfo *)d3denum->m_pAdapterInfoList->GetPtr(a);
}

void i4_dx9_image_class::put_pixel(i4_coord x, i4_coord y, w32 color)
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

i4_dx9_image_class::i4_dx9_image_class(w16 _w, w16 _h, w32 flags)
{
	w=_w;
	h=_h;

	D3DSURFACE_DESC ddsd;

	dx9_common.get_surface_description(dx9_common.back_surface,ddsd);

	i4_pixel_format fmt;
	bitmask_2_format(ddsd.Format,fmt);
	fmt.alpha_mask  = 0;
	fmt.calc_shift();

	pal = i4_pal_man.register_pal(&fmt);

	//we want to create the surface w/the same pixel format as the primary surface
	surface = dx9_common.create_surface(DX9_SYSMEM, w,h, flags, ddsd.Format);

	D3DSURFACE_DESC desc;
	surface->GetDesc(&desc);
	if (desc.Format!=ddsd.Format)
	{
		i4_warning("WARNING: Created offscreen surface might have wrong format");
	}
	if (!surface)
	{
		i4_warning("i4_dx9_image_class::create_surface failed");
	}
	else
	{
		lock();
		unlock();
	}
}
/*
   w32* i4_dx9_image_class::paddr(int x, int y)
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

void i4_dx9_image_class::put_part(i4_image_class *to, i4_coord x, i4_coord y, i4_coord x1,
								  i4_coord y1, i4_coord x2, i4_coord y2, i4_draw_context_class &context)
{
	i4_image_class::put_part(to,x,y,x1,y1,x2,y2,context);
};


void i4_dx9_image_class::lock()
{

	D3DLOCKED_RECT lkinfo;
	if (!i4_dx9_check(surface->LockRect(&lkinfo,0,D3DLOCK_NOSYSLOCK)))
	{
		i4_error("SEVERE: Could not lock auxilary backbuffer surface");
		return;
	}


	data = (w8 *)lkinfo.pBits;
	bpl  = lkinfo.Pitch;
}

void i4_dx9_image_class::unlock()
{
	surface->UnlockRect();
}


i4_dx9_image_class::~i4_dx9_image_class()
{
	if (surface)
	{
		//surface->PageUnlock(0);
		surface->Release();
	}
}

IDirect3DSurface9 *dx9_common_class::get_surface(i4_image_class *im)
{
	return ((i4_dx9_image_class *)im)->surface;

}



i4_image_class *dx9_common_class::create_image(int w, int h, w32 surface_flags)
{
	return new i4_dx9_image_class(w,h,surface_flags);
}
