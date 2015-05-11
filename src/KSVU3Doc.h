#pragma once

#include "resource.h"       // main symbols
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
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual void OnDocumentEvent(DocumentEvent deEvent);
	//}}AFX_VIRTUAL

// Implementation
public:
//	ControlsData cData;
	virtual ~CKSVU3Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CKSVU3Doc)	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	int SetPatName(CString path_name);
	virtual void SetPathName(const CString& path_name);
	CString GetPath(void) const;
};