#pragma once

#include "MyToolBar.h"	// Added by ClassView
#include "DialogBarTab1.h"	// Added by ClassView
#include "DialogBarTab2.h"	// Added by ClassView
#include "MyTabCtrl.h"	// Added by ClassView
//#include "mystatusbar.h"

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
//	CArray<RequestForAcsess> ReqArr;
// Implementation
public:
	MyToolBar Toolbar1;
	MyTabCtrl TabCtrl1;
	CDialog * CurTabDialog;
	DialogBarTab2 SeriesList;
//	TerminalDialog Tab4;
    
//	MessagesInspector Inspector1;
	ImageWnd	Img;
	MainChartWnd& Chart1;
//	Oscilloscope Scope1;
//	ControllerWnd CntrlerWnd;

	CDialogBar MainBar;
	CToolTipCtrl ToolTip1;
	CStatusBar  m_wndStatusBar;	
//	MVThread* WorkThread;
	
	
	int ShowWarning();	
	void InitChart();
	virtual ~CMainFrame();
	virtual void Serialize(CArchive& ar);
protected: 
	bool SearchForMsgRequest(DWORD msg,WPARAM wParam, LPARAM lParam );	

protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEventlog();
	afx_msg void OnMainTabAccel();
	afx_msg void OnImageTabAccel();
	afx_msg void OnConfig();
	afx_msg void OnPGAControl();
	afx_msg void OnTerminalButton();
	afx_msg void OnTabChange( NMHDR * pNotifyStruct, LRESULT * result );
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
//	afx_msg LRESULT OnUpdateIndicators(WPARAM,LPARAM);
//	afx_msg LRESULT OnUpdateConfig(WPARAM,LPARAM);
//	afx_msg LRESULT OnWavelenSet(WPARAM wParam, LPARAM lParam );	
//	afx_msg LRESULT OnDataUpdate(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
//	afx_msg LRESULT OnStop(WPARAM wParam, LPARAM lParam );
//	afx_msg LRESULT OnStart(WPARAM wParam, LPARAM lParam );
//	afx_msg LRESULT OnPause(WPARAM wParam, LPARAM lParam );
//	afx_msg LRESULT OnContinue(WPARAM wParam, LPARAM lParam );
	afx_msg void OnClose();
//	afx_msg LRESULT OnAccessRequest(WPARAM,LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void * GetChartFromParent();

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

