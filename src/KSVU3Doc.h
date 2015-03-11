// KSVU3Doc.h : interface of the CKSVU3Doc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_KSVU3DOC_H__F7D3E6C3_6C7F_4FB6_A009_227D43638530__INCLUDED_)
#define AFX_KSVU3DOC_H__F7D3E6C3_6C7F_4FB6_A009_227D43638530__INCLUDED_

//#include "SaveHeader.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "tchartseries.h"


class CKSVU3Doc : public CDocument
{
protected: // create from serialization only
	CKSVU3Doc();
	DECLARE_DYNCREATE(CKSVU3Doc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKSVU3Doc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
//	ControlsData cData;
	virtual ~CKSVU3Doc();
	ProtectedSeriesArray* GetSeries();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CKSVU3Doc)	
//	afx_msg void OnSeriesUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KSVU3DOC_H__F7D3E6C3_6C7F_4FB6_A009_227D43638530__INCLUDED_)
