#if !defined(AFX_OPTIONINFO_H__DE83BCA3_10B8_11D5_9EDB_00E0987CDED7__INCLUDED_)
#define AFX_OPTIONINFO_H__DE83BCA3_10B8_11D5_9EDB_00E0987CDED7__INCLUDED_

#include "x86_proc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionInfo.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld COptionInfo 

class COptionInfo : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionInfo)

// Konstruktion
public:
	COptionInfo();
	~COptionInfo();
	CProcessor *cpu;
// Dialogfelddaten
	//{{AFX_DATA(COptionInfo)
	enum { IDD = IDD_INFORMATION };
	CEdit	m_informationtext;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(COptionInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(COptionInfo)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnSetActive();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_OPTIONINFO_H__DE83BCA3_10B8_11D5_9EDB_00E0987CDED7__INCLUDED_
