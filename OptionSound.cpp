// OptionSound.cpp: Implementierungsdatei
//
#include "pch.h"
#include "stdafx.h"
#include "resource.h"
#include "mmsystem.h"
#include "dsound.h"
#include "OptionSound.h"
#include "main/win_main.h"
#include "lisp/lisp.h"

#ifdef _DEBUG
#undef new
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite COptionSound 

IMPLEMENT_DYNCREATE(COptionSound, CPropertyPage)

COptionSound::COptionSound() : CPropertyPage(COptionSound::IDD)
{
	//{{AFX_DATA_INIT(COptionSound)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

COptionSound::~COptionSound()
{
}

void COptionSound::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionSound)
	DDX_Control(pDX, IDC_SOUNDCARD, m_soundcard);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionSound, CPropertyPage)
	//{{AFX_MSG_MAP(COptionSound)
	ON_BN_CLICKED(IDC_SOUNDGROUP, OnSoundgroup)
	ON_BN_CLICKED(IDC_ENABLE_SOUND, OnEnableSound)
	ON_BN_CLICKED(IDC_SOUNDCAPABILITIES, OnSoundcapabilities)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten COptionSound 

void COptionSound::OnSoundgroup() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SetModified(TRUE);
}


BOOL CALLBACK DSEnumCallback(
  LPGUID lpGuid,            
  LPCSTR lpcstrDescription,  
  LPCSTR lpcstrModule,       
  LPVOID lpContext          
)
	{
	//Currently, this is only displaying the devices, it has no active
	//feature
	CComboBox *cb=(CComboBox*) lpContext;
	int index=cb->AddString(lpcstrDescription);
	cb->SetItemData(index,(ULONG) lpGuid);
	return TRUE;
	}



BOOL COptionSound::OnInitDialog()
	{
	CPropertyPage::OnInitDialog();
	if (i4_win32_startup_options.use2dsound
		&&i4_win32_startup_options.use3dsound)
		{
		CheckRadioButton( IDC_3DSOUND,IDC_NOSOUND,IDC_3DSOUND);
		}
		else if(i4_win32_startup_options.use2dsound)
			{
			CheckRadioButton(IDC_3DSOUND,IDC_NOSOUND,IDC_2DSOUND);
			}
		else
			CheckRadioButton(IDC_3DSOUND,IDC_NOSOUND,IDC_NOSOUND);
	
	DirectSoundEnumerate((LPDSENUMCALLBACK) DSEnumCallback,(LPVOID) &m_soundcard);
	m_soundcard.SetCurSel(0);
	return TRUE;
	}

void COptionSound::Apply()
{
	if (!m_hWnd) return;
	int checked=GetCheckedRadioButton(IDC_3DSOUND,IDC_NOSOUND);
	if (checked==IDC_3DSOUND)
		{
		i4_win32_startup_options.use3dsound =TRUE;
		i4_win32_startup_options.use2dsound =TRUE;
		}
	else if (checked==IDC_2DSOUND)
		{
		i4_win32_startup_options.use3dsound =FALSE;
		i4_win32_startup_options.use2dsound =TRUE;
		}
	else 
		{
		i4_win32_startup_options.use3dsound =FALSE;
		i4_win32_startup_options.use2dsound =FALSE;
		}
	li_call("enable_sound");

}

void COptionSound::OnEnableSound() 
{
	
	//i4_sound_man->uninit();//Restart the sound - manager
	//i4_sound_man->init();

//Er... HELL, how does this work? We need to signal the clients
//that we want to kill their buffers, otherwise they pocess an
//old pointer to a directsound-buffer object but the directsound
//object itself is gone -> HUGE crash.
	li_call("enable_sound");
}

BOOL CALLBACK DSCapsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static LPDIRECTSOUND lpds=NULL;
    static LPGUID lpguid;
    static HWND hlist;
    DSCAPS DSCaps;
    UINT dwEntry=0;
    HRESULT hr;
    switch (message)
    {
        case WM_INITDIALOG:
                lpguid=(LPGUID) lParam;
                hlist=GetDlgItem(hDlg,IDC_DSCAPSLIST);
                hr=DirectSoundCreate(lpguid,&lpds,NULL);
                if (FAILED(hr))
                {
                    MessageBox(GetParent(hDlg),"Die Eigenschaften dieses Geräts sind zur Zeit nicht verfügbar. ",
                      "DirectSound",MB_OK+MB_ICONSTOP);
                    return FALSE;
                }
                ZeroMemory(&DSCaps,sizeof(DSCAPS));
                DSCaps.dwSize=sizeof(DSCAPS);
                lpds->GetCaps(&DSCaps);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Zertifiziert"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_CERTIFIED?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Jede Samplerate möglich"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_CONTINUOUSRATE?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("DirectSound ist emuliert"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_EMULDRIVER?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("16Bit Wiedergabe möglich"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_PRIMARY16BIT?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("8Bit Wiedergabe möglich"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_PRIMARY8BIT?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Stereowiedergabe möglich"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_PRIMARYSTEREO?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("16Bit Wiedergabe mit Hardwarepuffer möglich"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_SECONDARY16BIT?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("8Bit Wiedergabe mit Hardwarepuffer möglich"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_SECONDARY8BIT?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Hardwarepuffer für Stereowiedergabe verwendbar"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFlags & DSCAPS_SECONDARYSTEREO?TRUE:FALSE);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Minimale Samplerate für Hardwarepuffer"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwMinSecondarySampleRate);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Maximale Samplerate für Hardwarepuffer"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwMaxSecondarySampleRate);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Kanäle des Hardwaremischers"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwMaxHwMixingAllBuffers);
				dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("3D-Kanäle auf Soundkarte"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwMaxHw3DAllBuffers);
				dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Maximaler Durchsatz des Hardwaremischers"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwUnlockTransferRateHwBuffers);
				dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Geschätzter CPU-Overhead für Softwaremischung"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwPlayCpuOverheadSwBuffers);
				dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Speicherplatz auf der Soundkarte"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwTotalHwMemBytes);
                dwEntry=SendMessage(hlist,LB_ADDSTRING,0,(LPARAM) TEXT("Freier Speicher auf der Soundkarte"));
                SendMessage(hlist,LB_SETITEMDATA,dwEntry,(LPARAM) DSCaps.dwFreeHwMemBytes);
                
                
                
                lpds->Release();
          return TRUE;
        case WM_CLOSE:
          EndDialog(hDlg,TRUE);
          return TRUE;
        case WM_COMMAND:
          DWORD wNotifycode=HIWORD(wParam);
          DWORD wID=LOWORD(wParam);
          char buf[256];
          //HWND hwndCntl=lParam;
          switch (wID)
          {
              case IDC_DSCAPS_CLOSE:
                EndDialog(hDlg,TRUE);
                return TRUE;
              case IDC_DSCAPSLIST:
                switch (wNotifycode)
                {
                    case LBN_SELCHANGE:
                      dwEntry=SendMessage(hlist,LB_GETCURSEL,0,0);
                      dwEntry=SendMessage(hlist,LB_GETITEMDATA,dwEntry,0);
                      if (dwEntry==0)
                      {
                          strcpy(buf,TEXT("Nein/nicht vorhanden"));
                      }
                      else if (dwEntry==TRUE)
                      {
                          strcpy(buf,TEXT("Ja"));
                      }
                      else
                          wsprintf(buf,TEXT("%i"),dwEntry);
                      SendDlgItemMessage(hDlg,IDC_DSCAPS_VALUE,WM_SETTEXT,0,(LPARAM)buf);
                      break;
                }
                break;
          }
          break;
          
          
          
    }
    return FALSE;
}



void COptionSound::OnSoundcapabilities() 
	{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	//BOOL DefaultDevice ;
	GUID guPreferredDevice;
	LPGUID  lpguTemp;
	lpguTemp = (LPGUID)m_soundcard.GetItemData( m_soundcard.GetCurSel());
	if( NULL != lpguTemp )
			guPreferredDevice = *lpguTemp;
	else
			guPreferredDevice = GUID_NULL;
	
	DialogBoxParam(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDD_SOUNDINFO),m_hWnd,(DLGPROC) DSCapsProc,
		(LPARAM) (LPGUID)&guPreferredDevice);
	}
