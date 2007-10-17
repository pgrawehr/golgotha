// OptionExtras.cpp: Implementierungsdatei
//
#include "pch.h"
#include "stdafx.h"
#include "resource.h"
//#include "golgotha.h"
#include "OptionExtras.h"
#include "main/win_main.h"
#include "string/string.h"

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

OptionExtras::OptionExtras() :
	CPropertyPage(OptionExtras::IDD)
{
	//{{AFX_DATA_INIT(OptionExtras)
	// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

OptionExtras::~OptionExtras()
{
}

void OptionExtras::DoDataExchange(CDataExchange * pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OptionExtras)
	DDX_Control(pDX, IDC_LANGSELECT, m_langselect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptionExtras, CPropertyPage)
//{{AFX_MSG_MAP(OptionExtras)
ON_BN_CLICKED(IDC_STEREOMODE, OnStereomode)
ON_CBN_EDITCHANGE(IDC_LANGSELECT, OnEditchangeLangselect)
ON_CBN_SELENDOK(IDC_LANGSELECT, OnSelendokLangselect)
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
	{
		return;
	}

	i4_win32_startup_options.stereo=IsDlgButtonChecked(IDC_STEREOMODE);

	//CEdit *ed=m_langselect.GetEditCtrl();

	char mybuf[255];
	m_langselect.GetWindowText(mybuf,255);

	if (strlen(mybuf)==0)
	{
		ZeroMemory(i4_win32_startup_options.langcode,10);
	}
	else
	{
		i4_str langstring(mybuf);
		int st=langstring.find_first_of("[")+1;
		int en=langstring.find_last_of("]");
		i4_str langsubstr;
		if ((st==-1)||(en==-1))
		{
			langsubstr=langstring;
		}
		else
		{
			en=en-st; //need the length
			langsubstr=langstring.substr(st,en);
		}
		strncpy(i4_win32_startup_options.langcode,langsubstr.c_str(),9);
	}
}

BOOL OptionExtras::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CheckDlgButton(IDC_STEREOMODE,i4_win32_startup_options.stereo);

	m_langselect.SetWindowText("");
	//CEdit *ed=m_langselect.GetEditCtrl();
	//ed->SetWindowText(i4_win32_startup_options.langcode);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

BOOL OptionExtras::OnApply()
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen

	return CPropertyPage::OnApply();
}

void OptionExtras::OnEditchangeLangselect()
{
	SetModified(TRUE);

}

void OptionExtras::OnSelendokLangselect()
{
	SetModified(TRUE);
}
