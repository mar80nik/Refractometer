#pragma once

#include "MessageInspector.h"

enum {UM_STATUS_BAR = WM_USER + 200};

class StatusBarMessage: public MessageForWindow 
{	
public:
	static WindowAddress StatusBarWindow;
public:	
	int Id;
	CString Text;

	StatusBarMessage();;
	StatusBarMessage(int id, const CString& text);
	StatusBarMessage& operator= (const StatusBarMessage& msg)
	{
		Id = msg.Id; Text = msg.Text; return *this;
	}
};

typedef ControledMessage<StatusBarMessage> ControledStatusBarMessage;

class MyStatusBar : public CStatusBar
{
// Construction
public:
	MyStatusBar();

// Attributes
public:
	CUIntArray indicators;
	enum _refresh {Refresh, NoRefresh};
	enum _mode {SHOW,HIDE,SET};

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyStatusBar)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetText(UINT id, CString &);
	void Show(UINT id);
	void Hide(UINT id);
	void SetWidth(UINT id, unsigned int width);
	int AddIndicator(UINT text, _refresh r=NoRefresh);
	virtual ~MyStatusBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(MyStatusBar)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	LRESULT OnStatusBarMessage(WPARAM wParam, LPARAM lParam );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
