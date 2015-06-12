#include "stdafx.h"
#include "MyStatusBar.h"

/////////////////////////////////////////////////////////////////////////////
// MyStatusBar

MyStatusBar::MyStatusBar()
{
}

MyStatusBar::~MyStatusBar()
{
}


BEGIN_MESSAGE_MAP(MyStatusBar, CStatusBar)
	//{{AFX_MSG_MAP(MyStatusBar)
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(UM_STATUS_BAR, OnStatusBarMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MyStatusBar message handlers

int MyStatusBar::AddIndicator(UINT text,_refresh r)
{
	int ret=indicators.Add(text);
	if(r==Refresh)
	{
		SetIndicators(indicators.GetData(), indicators.GetSize());	
	}
	return ret;
}

void MyStatusBar::Hide(UINT id) 
{
	int n; 
	if((n=CommandToIndex(id))>=0) 
	{
		SetPaneStyle(n,SBPS_DISABLED);
	}
}
void MyStatusBar::Show(UINT id) 
{
	int n;
	if((n=CommandToIndex(id))>=0) 
	{
		SetPaneStyle(n,SBPS_NORMAL);
	}
}
void MyStatusBar::SetText(UINT id, CString &str)
{
	int n;
	if((n=CommandToIndex(id))>0) 
	{
		int len = str.GetLength();
		CSize size = GetDC()->GetOutputTextExtent(str);
		SetWidth(id, size.cx*85/100);		
		SetPaneText(n,str);
	}		
	else if (n == 0)
	{
		SetWindowText(str);	
	}
}

void MyStatusBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
}

void MyStatusBar::SetWidth( UINT id, unsigned int width )
{
	int n;
	if((n=CommandToIndex(id))>=0) 
	{
		UINT style = GetPaneStyle(n);
		SetPaneInfo(n, id, style, width);
	}
}

LRESULT MyStatusBar::OnStatusBarMessage(WPARAM wParam, LPARAM lParam )
{
	StatusBarMessage *msg = (StatusBarMessage *)lParam;
	if (msg != NULL)
	{
		SetText(msg->Id, msg->Text);
		delete msg;
	}
	return 0;
}

StatusBarMessage::StatusBarMessage( int id, const CString& text )
{
	Msg = UM_STATUS_BAR; Reciver = StatusBarWindow;
	Id = id; Text = text;
}

StatusBarMessage::StatusBarMessage()
{
	Msg = UM_STATUS_BAR; Reciver = StatusBarWindow; 
	Id = -1; Text = "Error";
}
