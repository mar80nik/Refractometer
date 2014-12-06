#include "stdafx.h"
#include "EventLogDialog.h"

/////////////////////////////////////////////////////////////////////////////
// EventLogDialog dialog


EventLogDialog::EventLogDialog(CWnd* pParent /*=NULL*/)
	: CDialog(EventLogDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(EventLogDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EventLogDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EventLogDialog)
	DDX_Control(pDX, IDC_LIST1, m_ListBox1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EventLogDialog, CDialog)
	//{{AFX_MSG_MAP(EventLogDialog)
	ON_MESSAGE(UM_EVENTLOG,OnEventLog)	
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EVENTLOGMENU_CLEAR, OnClearBtn)
	ON_COMMAND(ID_EVENTLOGMENU_SAVE, OnSaveButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EventLogDialog message handlers
void EventLogDialog::OnClearBtn() 
{	
	m_ListBox1.ResetContent();
}

void EventLogDialog::Add(LogMessage& Msg)
{	
	COLORREF col = 0;
	if(Msg.priority==LogMessage::high_pr) col = RGB(255,0,0);
	for(int i=0;i<Msg.Msgs.GetSize();i++) 
	{
		m_ListBox1.AddString(Msg.Msgs[i], col); 
	}
	if(Msg.priority==LogMessage::high_pr) 
		ShowWindow(SW_SHOW);	
}

LRESULT EventLogDialog::OnEventLog(WPARAM wParam, LPARAM lParam )
{
	LogMessage *Msg=(LogMessage *)lParam;
	if(Msg) 
	{
		Add(*Msg);
		delete Msg;
	}
	return 0;
}


BOOL EventLogDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	menu1.LoadMenu(IDR_MENU4);
	return TRUE;
}


void EventLogDialog::OnSaveButton()
{
	CFileDialog fd(false,"txt"); BYTE cr[3]; cr[0]=0xd;cr[1]=0xa;cr[2]=0;
	CString temp;	CFile f; temp="log.txt";
	if(f.Open(temp,CFile::modeCreate | CFile::modeWrite))
	{
		for(int i=0;i<m_ListBox1.GetCount();i++)
		{
			m_ListBox1.GetText(i,temp);  //temp+=cr;					
			f.Write(LPCSTR(temp),temp.GetLength());			
			f.Write(cr,sizeof(cr));
		}
		f.Close();
	}
}

void EventLogDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu* menu = menu1.GetSubMenu(0);
	menu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
}
