// KSVU3.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "KSVU3.h"
#include "MainFrm.h"
#include "KSVU3Doc.h"
#include "KSVU3View.h"
#include "MyStatusBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////
//MainChartWnd GlobalChart;
SystemConfig MainCfg;
WindowAddress EventsLog, MainFrame, StatusBarWindow;
MessagesInspector GlobalInspector;
MessagesInspector* MessagesInspectorSubject::GlobalInspector=&::GlobalInspector;
WindowAddress LogMessage::LogWindow;
WindowAddress MyThread::ConfigParentWindow;
WindowAddress StatusBarMessage::StatusBarWindow;

CString SeriesListCtrl::GetSaveAsPath()
{
	CMainFrame* MW=(CMainFrame*)AfxGetMainWnd();
	return MW->GetActiveDocument()->GetPathName();
}

BEGIN_MESSAGE_MAP(CKSVU3App, CWinApp)
	//{{AFX_MSG_MAP(CKSVU3App)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_THREAD_MESSAGE(UM_BACKUP_SAVE,OnBackupSave)	
	ON_THREAD_MESSAGE(UM_UPDATE_CONFIG,OnUpdateConfig)	
	ON_THREAD_MESSAGE(UM_GENERIC_MESSAGE,OnGenericMessage)	
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKSVU3App construction

CKSVU3App::CKSVU3App()
{	
	myThread.Create(this,0,0);
	myThread.Config.PersonalID=MainTaskID;
}

CKSVU3App::~CKSVU3App()
{	
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CKSVU3App object

CKSVU3App theApp;

/////////////////////////////////////////////////////////////////////////////
// CKSVU3App initialization

static void my_handler (const char * reason, const char * file, int line, int gsl_errno)
{
	CString T; LogMessage *log=new LogMessage();	
	T.Format("%d:%s %s(%d)", gsl_errno, reason, file, line); 
	log->CreateEntry("ERR", T, LogMessage::high_pr);
	log->Dispatch();
}

BOOL CKSVU3App::InitInstance()
{
	AfxEnableControlContainer(); //myThread.Config.Terminate=None;
	CoInitialize(NULL);

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register document templates
	MainCfg.Parent=this;

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CKSVU3Doc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CKSVU3View));
	AddDocTemplate(pDocTemplate);
	
//	icon=theApp.LoadIcon(IDI_START_ICON);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	CMainFrame* MainWnd=(CMainFrame*)m_pMainWnd;
	EventsLog.pThrd = AfxGetThread(); EventsLog.pWND=&MainWnd->EventLog1;
	MainFrame.pThrd = AfxGetThread(); MainFrame.pWND=MainWnd;
	StatusBarWindow.pThrd = AfxGetThread(); StatusBarWindow.pWND = &MainWnd->m_wndStatusBar;

	LogMessage::LogWindow = EventsLog;
	MyThread::ConfigParentWindow = MainFrame;
	StatusBarMessage::StatusBarWindow = StatusBarWindow;

	MainCfg.LoadConfig(); 
	MainWnd->InitChart(); 

	if(cmdInfo.m_strFileName=="") MainWnd->TabCtrl1.ChangeTab(1);	
	else MainWnd->TabCtrl1.ChangeTab(0);	
	CString ProgPortName;

	StatusBarMessage *msg = new StatusBarMessage(IDS_CWD_SEPARATOR, MainWnd->GetCWD()); msg->Dispatch();

	gsl_error_handler_t * old_handler = gsl_set_error_handler (&my_handler);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CKSVU3App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CKSVU3App message handlers

int CKSVU3App::ExitInstance() 
{
	MainCfg.SaveConfig();
	myThread.Config.SetTerminate(Done);

	CoUninitialize();
	
	return CWinApp::ExitInstance();
}

void CKSVU3App::OnBackupSave(WPARAM wParam, LPARAM lParam )
{	
	CFile f;
	if(f.Open("backup.dat",CFile::modeCreate | CFile::modeWrite))
	{
		{
			CArchive ar(&f,CArchive::store);
			((CKSVU3Doc*)(((CMainFrame*)MainFrame.pWND)->GetActiveDocument()))->Serialize(ar);
		}
		f.Close();
	}
	return;
}

void CKSVU3App::OnUpdateConfig(WPARAM wParam, LPARAM lParam )
	{if(myThread.Config.GetTerminate()==None) 
	MainFrame.pWND->PostMessage(UM_UPDATE_CONFIG,wParam,lParam);
	}

void CKSVU3App::OnGenericMessage( WPARAM wParam, LPARAM lParam )
{
	MessageForWindow* msg=(MessageForWindow*)lParam;

	if(myThread.Config.GetTerminate()==None)
	{
		msg->wParam=wParam;
		msg->Dispatch();
	}
}