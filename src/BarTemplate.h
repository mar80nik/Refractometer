#pragma once

#include "mythread.h"
#include "resource.h"

enum RequestType {DONT_CARE=0xFFFFFF};

// BarTemplate dialog
struct RequestForAcsess
{
	CWnd *wnd; DWORD msg; DWORD WorkThreadID;
	RequestForAcsess(CWnd *_wnd=0, DWORD _msg=0, DWORD _id=0)  {wnd=_wnd; msg=_msg; WorkThreadID=_id;}
	RequestForAcsess& operator=(const RequestForAcsess& t) 
	{
		wnd=t.wnd; msg=t.msg; WorkThreadID=t.WorkThreadID;
		return (*this);
	}
};

struct RequestForAcsessMsg: public MessagesInspectorSubject
{
	RequestForAcsess data;
	RequestForAcsessMsg(CWnd *_wnd=0, DWORD _msg=0, DWORD _id=0)
	{
		data.wnd=_wnd; data.msg=_msg; data.WorkThreadID=_id;
	}
};

class BarTemplate : public CDialog, public ErrorInspector
{
	DECLARE_DYNAMIC(BarTemplate)
public:
	CWnd* Parent;	

	BarTemplate(CWnd* pParent = NULL);   // standard constructor
	virtual ~BarTemplate();

protected:		
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
public:
	// Dialog Data
	//{{AFX_DATA(DialogBarTab1)
	enum { IDD = IDD_BARTEMPLATE };
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);	
};
