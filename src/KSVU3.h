// KSVU3.h : main header file for the KSVU3 application
//

#if !defined(AFX_KSVU3_H__EA4AAC18_D078_498A_A9F1_FABDECBB313B__INCLUDED_)
#define AFX_KSVU3_H__EA4AAC18_D078_498A_A9F1_FABDECBB313B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "systemconfig.h"
#include "externals.h"
#include "tchartglobal.h"
#include "GlobalHeader.h"	// Added by ClassView
//#include "chipdriver.h"

/////////////////////////////////////////////////////////////////////////////
// CKSVU3App:
// See KSVU3.cpp for the implementation of this class
//
class CMainFrame;

class CKSVU3App : public CWinApp
{
protected:	
public:	
	CArray<MyThread*,MyThread*> Threads;
	MyThread myThread;
	HICON icon;

	void TerminateThreads();
//	int TerminateThread(MyThread* T);
	void LogErrors(int err_code);
	CKSVU3App();
	~CKSVU3App();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKSVU3App)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();	
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CKSVU3App)
	afx_msg void OnAppAbout();
//	afx_msg void OnEventLog(WPARAM wParam, LPARAM lParam );		
	afx_msg void OnBackupSave(WPARAM wParam, LPARAM lParam );	
	afx_msg void OnWavelenSet(WPARAM wParam, LPARAM lParam );
	afx_msg void OnStop(WPARAM wParam, LPARAM lParam );
	afx_msg void OnStart(WPARAM wParam, LPARAM lParam );
	afx_msg void OnPause(WPARAM wParam, LPARAM lParam );
	afx_msg void OnContinue(WPARAM wParam, LPARAM lParam );
	afx_msg void OnUpdateIndicators(WPARAM wParam, LPARAM lParam );
	afx_msg void OnUpdateConfig(WPARAM wParam, LPARAM lParam );
	afx_msg void OnDataUpdate(WPARAM wParam, LPARAM lParam );
//	afx_msg void OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
	afx_msg void OnGenericMessage(WPARAM wParam, LPARAM lParam );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KSVU3_H__EA4AAC18_D078_498A_A9F1_FABDECBB313B__INCLUDED_)
