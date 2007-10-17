#if !defined (AFX_OPTIONSDIALOG_H__C69E5243_C3D2_11D4_9EDB_00E0987CDED7__INCLUDED_)
#define AFX_OPTIONSDIALOG_H__C69E5243_C3D2_11D4_9EDB_00E0987CDED7__INCLUDED_

#include "pch.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Optionsdialog.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld OptionsDialog

class OptionsDialog :
	public CPropertyPage
{
// Konstruktion
public:
	OptionsDialog(CWnd * pParent = NULL);   // Standardkonstruktor
	virtual ~OptionsDialog();
	void Apply();
	void Cleanup();
	LPGUID lpDevice;
	bool resolutionchanged;
// Dialogfelddaten
	//{{AFX_DATA(OptionsDialog)
	enum {
		IDD = IDD_OPTIONS
	};
	CButton m_TexQualCombo;
	CStatic m_view_range_text;
	CSliderCtrl m_view_range;
	CSliderCtrl m_texture_quality;
	CStatic m_texture_qual_text;
	CComboBox m_render_device;
	CStatic m_currentmode;
	CButton m_create;
	CComboBox m_devices;
	CListBox m_resolutions;
	BOOL m_windowed;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(OptionsDialog)
public:
	virtual BOOL DestroyWindow();
	virtual void OnCancel();
protected:
	virtual void DoDataExchange(CDataExchange * pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(OptionsDialog)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnCreate();
	afx_msg void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	afx_msg void OnWindowed();
	afx_msg void OnSelchangeResolutions();
	afx_msg void OnSelchangeRenderDevice();
	afx_msg void OnReleasedcaptureTextureQuality(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnReleasedcaptureView(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar);
	afx_msg void OnTex16();
	afx_msg void OnTex32();
	afx_msg void OnTexdef();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_OPTIONSDIALOG_H__C69E5243_C3D2_11D4_9EDB_00E0987CDED7__INCLUDED_
