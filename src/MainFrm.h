#pragma once

#include "MyToolBar.h"	// Added by ClassView
#include "DialogBarTab1.h"	// Added by ClassView
#include "MyTabCtrl.h"	// Added by ClassView
#include "mystatusbar.h"

#include "EventLogDialog.h"	// Added by ClassView
#include "tconfigdialog.h"
#include "ImageWnd.h"
#include "captureWnd.h"


#define PAUSE 1
#define START 0


class CMainFrame : public CFrameWnd
{
    friend class CKSVU3App;	
	friend class CKSVU3Doc;
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	EventLogDialog EventLog1; 
	TConfigDialog Config;
// Implementation
public:
	MyToolBar Toolbar1;
	MyTabCtrl TabCtrl1;
	CDialog * CurTabDialog;
	ImageWnd	Img;
	MainChartWnd Chart1;

	CDialogBar MainBar;
	CToolTipCtrl ToolTip1;
	MyStatusBar  m_wndStatusBar;	
	
	int ShowWarning();	
	void InitChart();
	virtual ~CMainFrame();
	virtual void Serialize(CArchive& ar);
	CString GetCWD();
protected: 
	bool SearchForMsgRequest(DWORD msg,WPARAM wParam, LPARAM lParam );	

protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEventlog();
	afx_msg void OnMainTabAccel();
	afx_msg void OnImageTabAccel();
	afx_msg void OnConfig();
	afx_msg void OnChooseCWD();
	afx_msg void OnPGAControl();
	afx_msg void OnTerminalButton();
	afx_msg void OnTabChange( NMHDR * pNotifyStruct, LRESULT * result );
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnUpdateConfig(WPARAM,LPARAM);
	afx_msg LRESULT OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

