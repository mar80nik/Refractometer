#pragma once
#include "resource.h"

// ChooseCWDDialog dialog

class ChooseCWDDialog : public CDialogEx
{
	DECLARE_DYNAMIC(ChooseCWDDialog)

public:
	ChooseCWDDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~ChooseCWDDialog();

// Dialog Data
	enum { IDD = IDD_CHOOSE_CWD };

protected:
	void SetCWD(CString&);
	void InsertDate();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CString CWD, LastDateInserted;
	BOOL AddDate;
	afx_msg void OnBnClickedCheck1();
};
