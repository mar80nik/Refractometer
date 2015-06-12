#include "stdafx.h"
#include "ChooseCWDDialog.h"
#include "afxdialogex.h"
#include <direct.h>
#include "MessageInspector.h"


// ChooseCWDDialog dialog

IMPLEMENT_DYNAMIC(ChooseCWDDialog, CDialogEx)

ChooseCWDDialog::ChooseCWDDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(ChooseCWDDialog::IDD, pParent)
	, CWD(_T(""))
	, AddDate(TRUE)
{

}

ChooseCWDDialog::~ChooseCWDDialog()
{
}

void ChooseCWDDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, CWD);
	DDX_Check(pDX, IDC_CHECK1, AddDate);
}


BEGIN_MESSAGE_MAP(ChooseCWDDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &ChooseCWDDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK1, &ChooseCWDDialog::OnBnClickedCheck1)
	ON_BN_CLICKED(IDOK, &ChooseCWDDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// ChooseCWDDialog message handlers


void ChooseCWDDialog::OnBnClickedButton1()
{
	CFolderPickerDialog fd(CWD); 
	if (fd.DoModal() == IDOK)
	{
		SetCWD(fd.GetPathName());
		if (AddDate IS TRUE)
		{
			InsertDate();
		}
	}
}

void ChooseCWDDialog::SetCWD( CString& cwd )
{
	BOOL valid = FALSE;
	cwd.Trim('\\');

	if (cwd.Right(1) != "\\")
	{
		cwd += '\\'; valid = TRUE;
	}
	else
	{
		valid = TRUE;
	}
	if (valid)
	{
		CWD = cwd;
		UpdateData(FALSE);
	}		
}

void ChooseCWDDialog::OnBnClickedCheck1()
{
	UpdateData();
	if (AddDate IS TRUE )	
	{
		InsertDate();
	}
	else
	{
		CString cwd(CWD);
		cwd.Replace(LastDateInserted,"");
		SetCWD(cwd);
	}
}

void ChooseCWDDialog::InsertDate()
{
	time_t t = time(0); struct tm now;
	errno_t error = localtime_s( &now, &t );

	if (error != EINVAL)
	{
		CString cwd(CWD);
		LastDateInserted.Format("%d%02d%02d%02d%02d%02d", now.tm_year + 1900, now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
		cwd += LastDateInserted;
		SetCWD(cwd);
	}	
}


void ChooseCWDDialog::OnBnClickedOk()
{	
	CDialogEx::OnOK();
	int last = 0, cur, i;
	CStringArray arr; CString t;
	while ((cur = CWD.Find('\\', last)) != -1)
	{
		t = CWD.Left(cur);
		arr.Add(t);
		last = cur + 1;		
	}
	for (i = 0; i < arr.GetSize(); i++)
	{
		t = arr[i];
		if (_mkdir(arr[i]) == -1)
		{
			switch (errno)
			{
			case EEXIST:
				cur = 0;
				break;
			case ENOENT:
				cur = 0;
				break;
			default:
				cur = 0;
				break;
			}
		}
	}
#ifdef DEBUG	
	if (i == arr.GetSize()) i--;
	ControledLogMessage msg;
	msg.T.Format("Created dir %s", arr[i]); msg << msg.T;
	msg.Dispatch();
#endif
}
