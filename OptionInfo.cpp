// OptionInfo.cpp: Implementierungsdatei
//
#include "pch.h"
#include "stdafx.h"
#include "resource.h"
#include "OptionInfo.h"
#include "x86_proc.h"
#include ".\optioninfo.h"

#ifdef _DEBUG
#undef new
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite COptionInfo

IMPLEMENT_DYNCREATE(COptionInfo, CPropertyPage)

COptionInfo::COptionInfo() :
	CPropertyPage(COptionInfo::IDD)
{
	//{{AFX_DATA_INIT(COptionInfo)
	//}}AFX_DATA_INIT
	cpu=0;
}

COptionInfo::~COptionInfo()
{
	if (cpu)
	{
		delete cpu;
	}
	cpu=0;
}

void COptionInfo::DoDataExchange(CDataExchange *pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionInfo)
	DDX_Control(pDX, IDC_INFORMATIONTEXT, m_informationtext);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionInfo, CPropertyPage)
//{{AFX_MSG_MAP(COptionInfo)
// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten COptionInfo
extern void GetDXVersion( DWORD *pdwDXVersion, DWORD *pdwDXPlatform );
extern int GetOsVersion( LPSTR s);

BOOL COptionInfo::OnInitDialog()
{
	CHAR strBuff[32000];
	CHAR strBuff2[1024];
	DWORD dwDXVersion;
	DWORD dwDXPlatform;
	DWORD dwlen;
	if (!cpu)
	{
		cpu=new CProcessor();
	}
	CPropertyPage::OnInitDialog();
	GetDXVersion( &dwDXVersion, &dwDXPlatform );
	GetOsVersion(strBuff);
	strcat(strBuff,"\r\n");
	switch( dwDXVersion )
	{
		case 0x000:
			strcat( strBuff, "DirectX-Version:\tDirectX ist nicht installiert." );
			break;
		case 0x100:
			strcat( strBuff, "DirectX-Version:\tDirectX 1" );
			break;
		case 0x200:
			strcat( strBuff, "DirectX-Version:\tDirectX 2" );
			break;
		case 0x300:
			strcat( strBuff, "DirectX-Version:\tDirectX 3" );
			break;
		case 0x500:
			strcat( strBuff, "DirectX-Version:\tDirectX 5" );
			break;
		case 0x600:
			strcat( strBuff, "DirectX-Version:\tDirectX 6" );
			break;
		case 0x601:
			strcat( strBuff, "DirectX-Version:\tDirectX 6.1" );
			break;
		case 0x700:
			strcat( strBuff, "DirectX-Version:\tDirectX 7");
			break;
		case 0x800:
			strcat( strBuff, "DirectX-Version:\tDirectX 8");
			break;
		case 0x801:
			strcat( strBuff, "DirectX-Version:\tDirectX 8.1");
			break;
		case 0x900:
			strcat( strBuff, "DirectX-Version:\tDirectX 9 oder neuer");
			break;
		default:
			strcat( strBuff, "Unbekannte DirectX Version." );
			break;
	}
	strcat(strBuff,"\r\n");
	if (i4_current_app!=NULL)
	{
		strcat(strBuff,i4_current_app->display_mode_name);
	}
	strcat(strBuff,"\r\n");
	MEMORYSTATUS memstat;
	ZeroMemory(&memstat,sizeof(memstat));
	memstat.dwLength=sizeof(memstat);
	GlobalMemoryStatus(&memstat);
	wsprintf(strBuff2,
			 "Physischer Speicher insgesamt:\t%i\r\n"
			 "Physischer Speicher frei:\t\t%i\r\n"
			 "Virtueller Speicher insgesamt:\t%i\r\n"
			 "Virtueller Speicher frei:\t\t%i\r\n"
			 "Aktuelle Speicherauslastung:\t%i%c\r\n",
			 memstat.dwTotalPhys,memstat.dwAvailPhys,
			 memstat.dwTotalVirtual,memstat.dwAvailVirtual,
			 memstat.dwMemoryLoad,'%'
	);
	strcat(strBuff,strBuff2);

	char szFullPath[256];
	DWORD dwVerHnd;
	DWORD dwVerInfoSize;
	// Get version information from the application
	GetModuleFileName(0, szFullPath, sizeof(szFullPath));
	dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if (dwVerInfoSize)
	{
		// If we were able to get the information, process it:
		HANDLE hMem;
		LPVOID lpvMem;
		char szGetName[256];
		int cchRoot;
		int i;
		const int VERFIRST=0;
		const int VERLAST=12;
		char *entry_names[VERLAST+1]=
		{
			"Comments",
			"CompanyName",
			"FileDescription",
			"FileVersion",
			"InternalName",
			"LegalCopyright",
			"LegalTrademarks",
			"OriginalFilename",
			"PrivateBuild",
			"ProductName",
			"ProductVersion",
			"SpecialBuild"
		};

		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpvMem = GlobalLock(hMem);
		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem);
		//the number in the next line must match the BLOCK statement
		//in the .rc file.
		lstrcpy(szGetName, "\\StringFileInfo\\080704b0\\");
		cchRoot = lstrlen(szGetName);

		// Walk through the dialog items that we want to replace:
		for (i = VERFIRST; i < VERLAST; i++)
		{
			BOOL fRet;
			UINT cchVer = 0;
			LPSTR lszVer = NULL;
			char szResult[256];

			//GetDlgItemText(hdlg, i, szResult, sizeof(szResult));
			lstrcpy(szResult,entry_names[i]);
			lstrcpy(&szGetName[cchRoot], szResult);

			fRet = VerQueryValue(lpvMem, szGetName, (VOID **) &lszVer, &cchVer);

			if (fRet && cchVer && lszVer)
			{
				// Replace dialog item text with version info
				//lstrcpy(szResult, lszVer);
				//SetDlgItemText(hdlg, i, szResult);
				strcat(strBuff,szResult);
				strcat(strBuff,":\t");
				strcat(strBuff,lszVer);
				strcat(strBuff,"\r\n");
			}
		}
		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
	else
	{
		strcat(strBuff,"Could not find versioninfo Resource in GOLGOTHA.EXE. Possible cause: File corrupt.");
		i4_error("FATAL: Golgotha.exe seems to be corrupt (resources missing). Cannot continue."); //No hackerz please.
	}

	dwlen=lstrlen(strBuff);
	cpu->CPUInfoToText(&strBuff[dwlen],32000-dwlen-1);
	m_informationtext.SetWindowText(strBuff);
	m_informationtext.ShowScrollBar(SB_BOTH,TRUE);
	/*
	   CFont font;
	   font.CreateFont(
	   	12,                        // nHeight
	   	0,                         // nWidth
	   	0,                         // nEscapement
	   	0,                         // nOrientation
	   	FW_NORMAL,                 // nWeight
	   	FALSE,                     // bItalic
	   	FALSE,                     // bUnderline
	   	0,                         // cStrikeOut
	   	ANSI_CHARSET,              // nCharSet
	   	OUT_DEFAULT_PRECIS,        // nOutPrecision
	   	CLIP_DEFAULT_PRECIS,       // nClipPrecision
	   	DEFAULT_QUALITY,           // nQuality
	   	DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
	   	_T("Courier"));                 // lpszFacename


	   m_informationtext.SetFont(&font,TRUE);
	 */
	m_informationtext.SetSel(-1,-1,TRUE);
	return TRUE;
};



BOOL COptionInfo::OnSetActive()
{
	m_informationtext.SetSel(-1,-1,TRUE);

	return CPropertyPage::OnSetActive();
}
