#pragma once
#include "afxwin.h"
#include "MyListBox.h"
#include "resource.h"
#include "camera.h"
#include "mythread.h"


// CameraChooser dialog

class CameraChooser : public CDialog
{
	DECLARE_DYNAMIC(CameraChooser)

public:
	DSCaptureSource* Src;
	CString CaptureName;

	CameraChooser(CWnd* pParent = NULL);   // standard constructor
	virtual ~CameraChooser();

// Dialog Data
	enum { IDD = IDD_DIALOG17 };

protected:
    MyListBox CamerasList;
//	MessagesInspector MsgInsp;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();	
	afx_msg void OnLbnDblclkList3();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
};
