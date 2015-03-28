#pragma once

#include "resource.h"       // main symbols
#include "systemconfig.h"
#include "externals.h"
#include "tchartglobal.h"
#include "GlobalHeader.h"	// Added by ClassView

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
	afx_msg void OnBackupSave(WPARAM wParam, LPARAM lParam );	
	afx_msg void OnWavelenSet(WPARAM wParam, LPARAM lParam );
	afx_msg void OnStop(WPARAM wParam, LPARAM lParam );
	afx_msg void OnStart(WPARAM wParam, LPARAM lParam );
	afx_msg void OnPause(WPARAM wParam, LPARAM lParam );
	afx_msg void OnContinue(WPARAM wParam, LPARAM lParam );
	afx_msg void OnUpdateIndicators(WPARAM wParam, LPARAM lParam );
	afx_msg void OnUpdateConfig(WPARAM wParam, LPARAM lParam );
	afx_msg void OnDataUpdate(WPARAM wParam, LPARAM lParam );
	afx_msg void OnGenericMessage(WPARAM wParam, LPARAM lParam );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
