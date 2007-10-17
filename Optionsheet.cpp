// Optionsheet.cpp: Implementierungsdatei
//
#include "pch.h"
#include "stdafx.h"
//#include "display.h"
#include "resource.h"
#include "optionsdialog.h"
#include "optioninfo.h"
#include "optionsound.h"
#include "optionextras.h"
#include "Optionsheet.h"
#include "resource.h"
#include "main/win_main.h"

#ifdef _DEBUG
#undef new
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsheet

IMPLEMENT_DYNAMIC(COptionsheet, CPropertySheet)

COptionsheet::COptionsheet(UINT nIDCaption, CWnd * pParentWnd, UINT iSelectPage)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

COptionsheet::COptionsheet(LPCTSTR pszCaption, CWnd * pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&graphics);
	AddPage(&sound);
	AddPage(&extras);
	AddPage(&info);
}

COptionsheet::~COptionsheet()
{
}

BOOL COptionsheet::OnInitDialog()
{
	//graphics.OnInitDialog();
	BOOL bresult=CPropertySheet::OnInitDialog();

	return bresult;
}

BEGIN_MESSAGE_MAP(COptionsheet, CPropertySheet)
//{{AFX_MSG_MAP(COptionsheet)
ON_COMMAND(ID_APPLY_NOW, OnApplyNow)
ON_COMMAND(IDOK,OnOK)
ON_WM_CLOSE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten COptionsheet

void COptionsheet::OnApplyNow()
{
	graphics.Apply();
	graphics.SetModified(FALSE);
	sound.Apply();
	sound.SetModified(FALSE);
	extras.Apply();
	extras.SetModified(FALSE);
	i4_win32_startup_options.save_option();
};

void COptionsheet::OnOK()
{
	OnApplyNow();
	graphics.Cleanup();
	CPropertySheet::OnClose();
	if (!graphics.resolutionchanged)
	{
		return;
	}
	char buf[300];
	if (LoadString(i4_win32_instance,IDS_SETQUESTION,buf,300)==0)
	{
		strcpy(buf,"Set this mode right now?");
	}
	if (MessageBox(buf,NULL,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2|
				   MB_TOPMOST)==IDNO)
	{
		return;
	}


	i4_current_app->get_display()->change_mode(i4_win32_startup_options.xres,
											   i4_win32_startup_options.yres,i4_win32_startup_options.bits,
											   I4_CHANGE_RESOLUTION); //Apply resolution change immediatelly.
}

void COptionsheet::Apply()
{
	OnApplyNow();
}

void COptionsheet::OnClose()
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen

	CPropertySheet::OnClose();

}
