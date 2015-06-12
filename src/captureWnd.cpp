#include "stdafx.h"
#include "captureWnd.h"
#include "my_color.h"
#include "KSVU3.h"
#include "ImageWnd.h"
#include "compressor.h"
#include "MyStatusBar.h"

IMPLEMENT_DYNAMIC(CaptureWnd, CWnd)
CaptureWnd::CaptureWnd(): thrd(444)
{
	Src=NULL;
	CameraOutWnd=CRect(CPoint(0,0),CSize(800,600));
	LevelsScanWnd=CRect(CPoint(0,600+10),CSize(800,200));
	penRed.CreatePen(PS_SOLID,1,clRED);
	penBlue.CreatePen(PS_SOLID,1,clBLUE);
	penGreen.CreatePen(PS_SOLID,1,clGREEN);
	penWhite.CreatePen(PS_SOLID,1,clWHITE);
}

CaptureWnd::~CaptureWnd()
{
	if (Src != NULL)	{ delete Src; Src=NULL; }
}

BEGIN_MESSAGE_MAP(CaptureWnd, CWnd)
	ON_WM_CREATE()
	ON_MESSAGE(UM_DATA_UPDATE,OnDataUpdate)	
	ON_WM_PAINT()
	ON_MESSAGE(UM_CAPTURE_REQUEST,OnCaptureRequest)
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CaptureWnd message handlers


BEGIN_MESSAGE_MAP(CaptureWnd::CtrlsTab, CDialog)
	//{{AFX_MSG_MAP(DialogBarTab1)
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClicked_Live)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClicked_StopCapture)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClicked_PauseCapture)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClicked_ResumeCapture)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClicked_FilterParams)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedChooseCam)
END_MESSAGE_MAP()

int CaptureWnd::CtrlsTab::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	int ret=0;
	if (CWnd::OnCreate(lpCreateStruct) == -1) ret=-1;
	return ret;
}

BOOL CaptureWnd::CtrlsTab::OnInitDialog()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent; 
	CDialog::OnInitDialog();

	BtnCapture.EnableWindow(TRUE);
	BtnStop.EnableWindow(FALSE);
	BtnPause.EnableWindow(FALSE);
	BtnResume.EnableWindow(FALSE);
	BtnFilterParams.EnableWindow(FALSE);
	PreviewSize.AddString("3264x2448");
	PreviewSize.AddString("1600x1200");
	PreviewSize.AddString("1280x960");
	PreviewSize.AddString("1024x768");
	PreviewSize.AddString("640x480");
	PreviewSize.SetCurSel(3);
	return TRUE; 
}

eDcm800Size CaptureWnd::CtrlsTab::GetPreviewSize()
{
	eDcm800Size ret;
	switch (PreviewSize.GetCurSel())
	{
	case 0: ret=eDcm800Size_3264_2448;	break;
	case 1: ret=eDcm800Size_1600_1200;	break;
	case 2: ret=eDcm800Size_1280_960;	break;
	case 3: ret=eDcm800Size_1024_768;	break;
	case 4: 
	default:
			ret=eDcm800Size_640_480;	break;
	}
	return ret;
}

void CaptureWnd::CtrlsTab::OnBnClicked_Live()
{
	UpdateData();

	CaptureWnd *pParent=(CaptureWnd*)Parent; CString T;	LogMessage *log=NULL; 
	if(pParent->Src==NULL) pParent->SelectCaptureSrc(pParent->Ctrls.Chooser.CaptureName);
	if(pParent->Src==NULL) 
	{
		log=new LogMessage(); 
		T.Format(" No cameras found at all"); log->CreateEntry("ERR",T,LogMessage::high_pr);
		log->Dispatch(); return;
	}
	if(SUCCEEDED(pParent->Src->status))
	{
		pParent->Timer1.Start(); 
		pParent->cntr=0;

		CaptureParams &thread_params = pParent->thrd;
		thread_params.Parent = *pParent;
		thread_params.Pbuf=&pParent->Pbuf;
		thread_params.ColorTransformSelector = &ColorTransformSelector;
		thread_params.Stack = &pParent->Stack;
		//thread_params.LevelScanBuf=&pParent->LevelsScanBuf;
		thread_params.Src=pParent->Src;
		thread_params.StopCapture.ResetEvent();
		thread_params.PauseCapture.ResetEvent();
		thread_params.ResumeCapture.ResetEvent();
		thread_params.ShowFilterParams.ResetEvent();
		thread_params.size=GetPreviewSize();
		thread_params.thrd=&pParent->thrd;

		CRect r=((CaptureWnd*)Parent)->CameraOutWnd;
		pParent->grayscaleBuf.Create(this,r.Width(),r.Height(),8);
		pParent->grayscaleBuf.CreateGrayPallete();		
		pParent->truecolorBuf.Create(this,r.Width(),r.Height(),24);		

		//pParent->accum.Initialize(r.Width(), r.Height());

		pParent->thrd.Start();	
		BtnCapture.EnableWindow(FALSE);
		BtnStop.EnableWindow(TRUE);
		BtnPause.EnableWindow(TRUE);
		BtnResume.EnableWindow(FALSE);
		BtnChooseCam.EnableWindow(FALSE);
		BtnFilterParams.EnableWindow(TRUE);
	}		
	else 
	{
		log=new LogMessage(); 
		T.Format(" Please CHOOSE valid CAMera"); log->CreateEntry("ERR",T,LogMessage::high_pr); 
		log->Dispatch(); return;
	}	
	return;
}

struct CaptureRequestStackCBparams_StopCapture
{	
	void GainAcsessCB(CaptureRequestStack& Stack);
};

void CaptureWnd::CtrlsTab::OnBnClicked_StopCapture()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	pParent->thrd.StopCapture.SetEvent();  
	BtnCapture.EnableWindow(TRUE);
	BtnStop.EnableWindow(FALSE);
	BtnPause.EnableWindow(FALSE);
	BtnResume.EnableWindow(FALSE);
	BtnChooseCam.EnableWindow(TRUE);
	BtnFilterParams.EnableWindow(FALSE);

	CaptureRequestStackCBparams_StopCapture paramsCB;
	pParent->Stack.ModifyWith(paramsCB);

}
void CaptureRequestStackCBparams_StopCapture::GainAcsessCB( CaptureRequestStack& Stack)
{
	CaptureRequestStack::Item request;
	while(Stack >> request)
	{
		request.sender->PostMessage(UM_CAPTURE_EVENT,EvntOnCaptureStop,0);
	}
}


void CaptureWnd::CtrlsTab::OnBnClicked_PauseCapture()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	pParent->thrd.PauseCapture.SetEvent();  
	BtnCapture.EnableWindow(FALSE);
	BtnStop.EnableWindow(TRUE);
	BtnPause.EnableWindow(FALSE);
	BtnResume.EnableWindow(TRUE);
}

void CaptureWnd::CtrlsTab::OnBnClicked_ResumeCapture()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	pParent->thrd.ResumeCapture.SetEvent();    
	BtnCapture.EnableWindow(FALSE);
	BtnStop.EnableWindow(TRUE);
	BtnPause.EnableWindow(TRUE);
	BtnResume.EnableWindow(FALSE);
}

void CaptureWnd::CtrlsTab::OnBnClicked_FilterParams()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	pParent->thrd.ShowFilterParams.SetEvent();    
}

void CaptureWnd::SelectCaptureSrc(CString name)
{	
	MyEnumerator CaptureSrc; MonikersArray arr;
	CaptureSrc.FindSource(name,arr); CString T;
//	ASSERT(arr.GetSize()==1);

	for (int i=0;i<arr.GetSize();i++)
	{
		if(Src!=NULL) {delete Src; Src=NULL;}
		if(name=="ScopeTek DCM800") 
		{
			Src=new ScopeTek_DCM800();
			Ctrls.PreviewSize.EnableWindow(TRUE);
		}
		else 
		{
			Src=new DSCaptureSource(); 
			Ctrls.PreviewSize.EnableWindow(FALSE);
		}
		Src->Create(arr[i]);
		if(FAILED(Src->status))
		{			
			LogMessage *log=new LogMessage(); log->SetPriority(lmprHIGH);
			T.Format(" %s - INIT ERROR",name); *log << T;
			log->Dispatch();
		}
		else 
		{
			Src->Name=name;
			StatusBarMessage *msg = new StatusBarMessage(IDS_CAMERA_SEPARATOR, name);
			msg->Dispatch();
		}
	}	
}

int CaptureWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	Ctrls.Parent=this; 	WindowAddress::pWND = this;

	font1.CreatePointFont(90,"Arial"); 

	//for(int i=0;i<256;i++) pal[i]=HSLColor((int)(i*360./256.));
	int range_min=250;
	for(int i=0;i<range_min;i++) pal[i].rgbBlue=pal[i].rgbGreen=pal[i].rgbRed=i;
	for(int i=range_min;i<256;i++) 
		pal[i]=HSLColor(0,127,(i-range_min)/(256.-range_min));

	void *x;
	if((x=LevelsScanBuf.GainAcsess(WRITE))!=NULL)
	{
		BMPanvasGuard guard1(x); BMPanvas &buf(guard1);
		buf.Create(this, LevelsScanWnd.Width(),LevelsScanWnd.Height(), 8);
		memset(palLevelsScan, 0, sizeof(palLevelsScan));
		palLevelsScan[1].rgbBlue = palLevelsScan[1].rgbGreen = palLevelsScan[1].rgbRed = 255;
		palLevelsScan[2].rgbRed = 255; palLevelsScan[3].rgbGreen = 255; palLevelsScan[4].rgbBlue = 255;
		buf.SetPallete(palLevelsScan); 
	}
	Timer1.Start(); cntr=0;

	return 0;
}

LRESULT CaptureWnd::OnDataUpdate(WPARAM wParam, LPARAM lParam )
{
	if(lParam!=0)
	{
		MessageForWindow* msg=(MessageForWindow*)lParam;
		delete msg;
	}
	if(wParam!=0)
	{
		AbstractMessage* msg=(AbstractMessage*)wParam;
		delete msg;
	}
	RedrawWindow(0,0,RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME | RDW_ALLCHILDREN);
	return 0;
}

void ColorTransform(BMPanvas *color, BMPanvas *grayscale, ColorTransformModes mode)
{	
	RGBTRIPLE *colorCurrentPoint; int i,j; BYTE *colorLineBegin=NULL, *bwCurrentPoint=NULL, *bwLineBegin=NULL;
	int (*LuminosityFunc)(RGBTRIPLE&)=NULL;
	
	switch(mode)
	{
	case HSL: LuminosityFunc=getL_HSL; break;
	case HSV: LuminosityFunc=getL_HSV; break;
	case NativeGDI: color->CopyTo(grayscale,TOP_LEFT); return;
	}
	if(LuminosityFunc==NULL) return;

	color->LoadBitmapArray(); colorLineBegin=bwLineBegin=color->arr;
	for(i=0;i<color->h;i++)
	{
		colorCurrentPoint=(RGBTRIPLE *)colorLineBegin; bwCurrentPoint=bwLineBegin;
		for(j=0;j<color->w;j++)
		{
			(*bwCurrentPoint) = LuminosityFunc(*colorCurrentPoint);
			colorCurrentPoint++; bwCurrentPoint++;
		}
		colorLineBegin += color->wbyte; bwLineBegin += grayscale->wbyte;
	}
	grayscale->SetBitmapArray(BMPanvas::MIN_SCANLINE,BMPanvas::MAX_SCANLINE,color->arr);
	color->UnloadBitmapArray();

}

void CaptureWnd::ScanLevels(BMPanvas *src, BMPanvas &levels, const ColorTransformModes mode)
{
	int i;  HGDIOBJ t; double l; MyTimer time1; time1.Start(); ms dt;CString T;
	src->LoadBitmapArray(src->h/2,src->h/2); 
	levels.PatBlt(BLACKNESS);

	if (mode == TrueColor)
	{
		RGBTRIPLE *col = (RGBTRIPLE*)src->arr;
		for (i = 0; i < src->w; i++)
		{
			t=levels.SelectObject(penBlue); l=(levels.h-1)*(255-(*col).rgbtBlue)/255.; 
			levels.MoveTo(i,levels.h-1); levels.LineTo(i,(int)l);
			levels.SelectObject(penRed); l=(levels.h-1)*(255-(*col).rgbtRed)/255.; 
			levels.MoveTo(i,levels.h-1); levels.LineTo(i,(int)l);
			levels.SelectObject(penGreen); l=(levels.h-1)*(255-(*col).rgbtGreen)/255.; 
			levels.MoveTo(i,levels.h-1); levels.LineTo(i,(int)l);
			levels.SelectObject(t); 
			col++;
		}
	}
	else
	{
		BYTE *col = (BYTE *)src->arr; t = levels.SelectObject(penWhite);
		for (i = 0; i < src->w; i++)
		{
			l=(levels.h - 1)*(255 - (*col))/255.; 
			levels.MoveTo(i,levels.h-1); levels.LineTo(i,(int)l);
			col++;
		}
		levels.SelectObject(t); 
	}

	src->UnloadBitmapArray();
	HGDIOBJ tf=levels.SelectObject(font1); levels.SetBkMode(TRANSPARENT); levels.SetTextColor(clWHITE);
	dt=time1.StopStart(); T.Format("%.2f ms",dt.val()); levels.TextOut(0,0,T);
	levels.SelectObject(tf);
}

void CaptureWnd::OnPaint()
{	
	CPaintDC dc(this); Ctrls.UpdateData(); CString accumText;
	CRect tr; GetClientRect(&tr); HDC hdc=dc.GetSafeHdc();
	
    if(grayscaleBuf.GetDC()==NULL) return;
	void* x,* xx; CString T; sec dt1; MyTimer Timer2;
	if((x=Pbuf.GainAcsess(READ))!=NULL)
	{
		Timer2.Start();
		BMPanvasGuard guard1(x); BMPanvas &buf(guard1);
		BMPanvas* tbuf=&buf;
		dt1=Timer1.StopStart();		

		CRect rgn1=buf.Rgn, rgn2=grayscaleBuf.Rgn, trgn;
		double r1=(double)rgn1.Width()/rgn2.Width(), r2=(double)rgn1.Height()/rgn2.Height(), r;
		r=(r1>r2 ? r1:r2);
		if(r>1) rgn1=CRect(0,0,(int)(rgn1.Width()/r),(int)(rgn1.Height()/r));
		//else rgn1.OffsetRect(rgn2.CenterPoint()-rgn1.CenterPoint());
		rgn1.OffsetRect(rgn2.CenterPoint()-rgn1.CenterPoint());

		tbuf=&truecolorBuf;
		if(r>1) { SetStretchBltMode(tbuf->GetDC(),COLORONCOLOR); buf.StretchTo(tbuf,rgn1,buf.Rgn,SRCCOPY); }
		else buf.CopyTo(tbuf,rgn1);
		
		if (Ctrls.ColorTransformSelector != TrueColor)
		{
			ColorTransform(tbuf, &grayscaleBuf, Ctrls.ColorTransformSelector); tbuf=&grayscaleBuf;

			//ms time;
			//accum.FillAccum(&grayscaleBuf); time = accum.fillTime;
			//accum.ConvertToBitmap(&grayscaleBuf);
			//accumText.Format("Fill = %.2f ms Init = %.2f ms n = %d", time.val(), accum.fillTime.val(), accum.n);
		}

		if ((xx=LevelsScanBuf.GainAcsess(WRITE)) != NULL)
		{
			BMPanvasGuard guard1(xx); BMPanvas &levels(guard1);
			ScanLevels(tbuf, levels, Ctrls.ColorTransformSelector);
			levels.CopyTo(hdc,LevelsScanWnd.TopLeft());
		}

		BMPanvasTAGSmk1* tags=(BMPanvasTAGSmk1*)buf.tags;
		CPoint TextOutput; TextOutput=rgn1.TopLeft();

		HGDIOBJ tf=tbuf->SelectObject(font1); tbuf->SetBkMode(TRANSPARENT); tbuf->SetTextColor(RGB(255,255,0));
		T.Format("%dx%dx%d %.2f sec",tags->i1,tags->i2,tags->i3,tags->d1); tbuf->TextOut(TextOutput.x,TextOutput.y+0,T);
		T.Format("DShow fps=%g",tags->d2); tbuf->TextOut(TextOutput.x,TextOutput.y+10,T);
		T.Format("Buffer process = %.2f+%.2f ms", tags->timel.val(), Timer2.StopStart().val()); tbuf->TextOut(TextOutput.x,TextOutput.y+30,T);		
		T.Format("%d eqv fps = %g",cntr++,(1./dt1.val()));  tbuf->TextOut(TextOutput.x,TextOutput.y+40,T);

		tbuf->TextOut(rgn1.left, rgn1.bottom - 40, accumText);

		tbuf->SelectObject(tf);
		
		if(Ctrls.ColorTransformSelector!=TrueColor) grayscaleBuf.SetPallete(pal); 
		//pal - special pallete to diagnose overlight pixels with red color
		tbuf->CopyTo(hdc,TOP_LEFT); 
		if(Ctrls.ColorTransformSelector!=TrueColor) grayscaleBuf.CreateGrayPallete();
	}
	//else ASSERT(0);
}

struct CaptureRequestStackCBparams_OnRequest 
{
	CaptureRequestStack::Item item; 
	void GainAcsessCB(CaptureRequestStack& Stack) { Stack << item; }
};

LRESULT CaptureWnd::OnCaptureRequest( WPARAM wParam, LPARAM lParam )
{
	CaptureRequestStackCBparams_OnRequest paramsCB;
	paramsCB.item = CaptureRequestStack::Item((CWnd*)wParam,(BMPanvas*)lParam);
	Stack.ModifyWith(paramsCB);
	return 0;
}

void CaptureWnd::CtrlsTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
	DDX_Control(pDX, IDC_BUTTON1, BtnCapture);
	DDX_Control(pDX, IDC_BUTTON2, BtnStop);
	DDX_Control(pDX, IDC_BUTTON3, BtnPause);
	DDX_Control(pDX, IDC_BUTTON4, BtnResume);
	DDX_Control(pDX, IDC_BUTTON6, BtnChooseCam);
	DDX_Control(pDX, IDC_COMBO1, PreviewSize);
	DDX_Control(pDX, IDC_BUTTON5, BtnFilterParams);
	DDX_Radio(pDX, IDC_RADIO1, (int&)ColorTransformSelector);
	//}}AFX_DATA_MAP
}


void CaptureWnd::CtrlsTab::OnBnClickedChooseCam()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	if(Chooser.DoModal()==IDOK)
	{
		pParent->SelectCaptureSrc(Chooser.CaptureName);
	}	
}

void CaptureWnd::OnDestroy()
{
	__super::OnDestroy();

	font1.DeleteObject();
}

