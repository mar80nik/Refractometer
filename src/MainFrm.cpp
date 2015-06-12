// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "KSVU3.h"
#include "MainFrm.h"
#include "ksvu3doc.h"
#include "resource.h"
#include "ChooseCWDDialog.h"
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_MAIN_TAB, OnMainTabAccel)
	ON_COMMAND(ID_IMAGE_TAB, OnImageTabAccel)
	ON_COMMAND(IDB_LOG_BUTTON, OnEventlog)
	ON_COMMAND(ID_EVENT_LOG, OnEventlog)	
	ON_COMMAND(ID_VIEW_CONFIG, OnConfig)	
	ON_COMMAND(ID_FILE_CHOOSECWD, OnChooseCWD)	
	ON_NOTIFY(TCN_SELCHANGE,IDC_TAB1,OnTabChange)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	ON_MESSAGE(UM_UPDATE_CONFIG,OnUpdateConfig)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame(): TabCtrl1(&MainBar,IDC_TAB1)
{
}

CMainFrame::~CMainFrame()
{
	Chart1.DestroyWindow();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{	
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	if (!m_wndStatusBar.Create(this,WS_CHILD | WS_VISIBLE | CBRS_BOTTOM | CBRS_NOALIGN))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.AddIndicator(ID_SEPARATOR);	
	m_wndStatusBar.AddIndicator(IDS_CWD_SEPARATOR); 
	m_wndStatusBar.AddIndicator(IDS_CAMERA_SEPARATOR, MyStatusBar::Refresh);	
	m_wndStatusBar.SetText(IDS_CWD_SEPARATOR, GetCWD());
	
	MainBar.Create(this,IDD_DIALOGBAR,CBRS_ALIGN_BOTTOM,IDD_DIALOGBAR);

	Toolbar1.Create(this,WS_CHILD | WS_VISIBLE | CBRS_RIGHT); Toolbar1.LoadToolBar(IDR_TOOLBAR2);
	Toolbar1.AddButton(IDB_LOG_BUTTON,MyToolBar::Refresh);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnClose() 
{
	EventLog1.DestroyWindow();
	Config.DestroyWindow();
	Img.DestroyWindow();
	Chart1.DestroyWindow();
	CFrameWnd::OnClose();
}

void CMainFrame::OnConfig() {Config.ShowWindow(SW_SHOW);}

void CMainFrame::OnEventlog() 
{
	EventLog1.ShowWindow(SW_SHOW);
}

void CMainFrame::OnMainTabAccel() 
{
	TabCtrl1.ChangeTab(0);
}

void CMainFrame::OnImageTabAccel()
{
	TabCtrl1.ChangeTab(1);
}

void CMainFrame::OnTabChange( NMHDR * pNotifyStruct, LRESULT * result )
{	
	TabCtrl1.ChangeTab();
	if(pNotifyStruct) *result=0;
}

void CMainFrame::InitChart() 
{
	CScrollView* pFirstView = (CScrollView*)GetActiveView();
	CRect r; pFirstView->GetClientRect(r);		

	TabCtrl1.AddTab("Main control",&Chart1.Panel,IDD_DIALOGBARTAB11);	
	TabCtrl1.AddTab("Image control",&Img.Ctrls,IDD_DIALOGBARTAB1);		

	Img.Create(0, "ImageWnd", WS_CHILD, r, pFirstView, ID_MV_WND+1, 0);
	Chart1.Create(pFirstView,r); Chart1.SetVisible(true); 

#ifdef DEBUG
	Img.CameraWnd.SelectCaptureSrc(CString("Logitech HD Webcam C270"));
#else
	Img.CameraWnd.SelectCaptureSrc(CString("ScopeTek DCM800"));
#endif	
}

void CMainFrame::Serialize(CArchive& ar)
{		
	Chart1.Serialize(ar);
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{	
	ShowWindow(SW_SHOWMAXIMIZED);
	CFrameWnd::OnShowWindow(bShow, nStatus);

	if(bShow)
	{		
		EventLog1.Create(IDD_DIALOG5,this);
		Config.Create(IDD_CONFIG,this);
	}
}

LRESULT CMainFrame::OnUpdateConfig(WPARAM wParam, LPARAM lParam )
{
	Config.SendMessage(UM_UPDATE_CONFIG,wParam,lParam);
	return 0;
}

LRESULT CMainFrame::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{
	return 0;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
}

CString CMainFrame::GetCWD()
{
	CKSVU3Doc *doc = (CKSVU3Doc*)GetActiveDocument();
	if (doc != NULL)
	{
		return doc->GetPath();
	}
	return _T("No CWD");	
}

void CMainFrame::OnChooseCWD()
{
	ChooseCWDDialog dlg;
	dlg.CWD = GetCWD();
	if (dlg.DoModal() == IDOK)
	{
		CKSVU3Doc *doc = (CKSVU3Doc*)GetActiveDocument();
		doc->SetPathName(dlg.CWD);
	}
	else
	{

	}
		
}

