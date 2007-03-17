#if !defined (AFX_OPTIONSOUND_H__DE83BCA2_10B8_11D5_9EDB_00E0987CDED7__INCLUDED_)
#define AFX_OPTIONSOUND_H__DE83BCA2_10B8_11D5_9EDB_00E0987CDED7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionSound.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld COptionSound

class COptionSound :
	public CPropertyPage
{
	DECLARE_DYNCREATE(COptionSound)

// Konstruktion
public:
	virtual void Apply();
	COptionSound();
	~COptionSound();

// Dialogfelddaten
	//{{AFX_DATA(COptionSound)
	enum {
		IDD = IDD_SOUND
	};
	CComboBox m_soundcard;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(COptionSound)
protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(COptionSound)
	afx_msg void OnSoundgroup();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableSound();
	afx_msg void OnSoundcapabilities();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_OPTIONSOUND_H__DE83BCA2_10B8_11D5_9EDB_00E0987CDED7__INCLUDED_
