// KSVU3Doc.cpp : implementation of the CKSVU3Doc class
//

#include "stdafx.h"
#include "KSVU3.h"
#include "mainfrm.h"
#include "KSVU3Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKSVU3Doc

IMPLEMENT_DYNCREATE(CKSVU3Doc, CDocument)

BEGIN_MESSAGE_MAP(CKSVU3Doc, CDocument)
	//{{AFX_MSG_MAP(CKSVU3Doc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKSVU3Doc construction/destruction

CKSVU3Doc::CKSVU3Doc() {}

CKSVU3Doc::~CKSVU3Doc() {}

BOOL CKSVU3Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

    CMainFrame* parent=(CMainFrame*)AfxGetMainWnd();
	if(parent!=NULL)
	{
		void *x;	
		if((x=parent->Chart1.Series.GainAcsess(WRITE))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& series(Protector);
			series.ClearAll();
		}

	}
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// CKSVU3Doc serialization

void CKSVU3Doc::Serialize(CArchive& ar)
{
	void *x;
	CMainFrame* MW=(CMainFrame*)AfxGetMainWnd();
	if (MW == NULL) return;	
	if (ar.IsStoring())
	{
		if((x = MW->Chart1.Series.GainAcsess(READ))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			Series.Serialize(ar);
		}
	}
	else
	{
		if((x = MW->Chart1.Series.GainAcsess(WRITE))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			Series.Serialize(ar);
		}	
	}
}

/////////////////////////////////////////////////////////////////////////////
// CKSVU3Doc diagnostics

#ifdef _DEBUG
void CKSVU3Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CKSVU3Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CKSVU3Doc commands

BOOL CKSVU3Doc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	
	return TRUE;
}

BOOL CKSVU3Doc::OnSaveDocument(LPCTSTR lpszPathName) 
{		
	return CDocument::OnSaveDocument(lpszPathName);
}

void CKSVU3Doc::OnCloseDocument() 
{
	CDocument::OnCloseDocument();
}

void CKSVU3Doc::OnDocumentEvent( DocumentEvent deEvent )
{
	ControledStatusBarMessage msg;
	CFolderPickerDialog fd; 

	if (GetPathName() == "")
	{		
		CDocument::SetPathName(m_strTitle, false);
		SetPathName(GetPath());
	}

	CString path = GetPath();

	switch (deEvent)
	{
	case onAfterNewDocument:		
	case onAfterOpenDocument:
	case onAfterSaveDocument:
		msg << StatusBarMessage(IDS_CWD_SEPARATOR, path);
		msg.Dispatch();	
		break;
	case onAfterCloseDocument:
		break;
	}
}


void CKSVU3Doc::SetPathName(const CString& path)
{
	CDocTemplate* pTemplate = GetDocTemplate();
	if (pTemplate IS_NOT NULL)
	{
		CString name = m_strTitle, ext, path_name;
		if (pTemplate->GetDocString(ext, CDocTemplate::filterExt) && !ext.IsEmpty())
		{
			CString t; int iStart = 0;
			t = ext.Tokenize(_T(";"), iStart); 
			ext = t;
		}
		path_name = path; 
		path_name += name; 
		CDocument::SetPathName(path_name, false);
		OnDocumentEvent(onAfterSaveDocument);		
	}	
}


CString CKSVU3Doc::GetPath(void) const
{
	CString path, path_name = GetPathName();
	int iStart = 0, last_pos = -1, pos;
	while ((pos = path_name.Find('\\', iStart)) IS_NOT -1)
	{
		last_pos = pos; iStart = pos + 1;
	}
	path = path_name.Left(last_pos + 1);
	return path;
}
