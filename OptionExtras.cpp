// OptionExtras.cpp: Implementierungsdatei
//
#include "pch.h"
#include "stdafx.h"
#include "resource.h"
//#include "golgotha.h"
#include "OptionExtras.h"
#include "main/win_main.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite OptionExtras 

IMPLEMENT_DYNCREATE(OptionExtras, CPropertyPage)

OptionExtras::OptionExtras() : CPropertyPage(OptionExtras::IDD)
{
	//{{AFX_DATA_INIT(OptionExtras)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

OptionExtras::~OptionExtras()
{
}

void OptionExtras::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OptionExtras)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptionExtras, CPropertyPage)
	//{{AFX_MSG_MAP(OptionExtras)
	ON_BN_CLICKED(IDC_STEREOMODE, OnStereomode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten OptionExtras 

void OptionExtras::OnStereomode() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SetModified(TRUE);
}

void OptionExtras::Apply()
	{
	if (!m_hWnd)
		return;
	
	i4_win32_startup_options.stereo=IsDlgButtonChecked(IDC_STEREOMODE);

	//SetModified(FALSE);
	}

BOOL OptionExtras::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	CheckDlgButton(IDC_STEREOMODE,i4_win32_startup_options.stereo);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}
