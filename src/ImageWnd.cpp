// ImageWnd.cpp : implementation file
//

#include "stdafx.h"
#include "KSVU3.h"
#include "ImageWnd.h"
#include "TchartSeries.h"
#include "MainFrm.h"
#include "dcm800.h"
#include "metricon.h"
#include "my_color.h"
#include "captureWnd.h"
#include "BMPanvas.h"
#include "compressor.h"
// ImageWnd

IMPLEMENT_DYNAMIC(ImageWnd, CWnd)
ImageWnd::ImageWnd()
{
}

ImageWnd::~ImageWnd() {}


BEGIN_MESSAGE_MAP(ImageWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// ImageWnd message handlers

int ImageWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect tr; GetClientRect(&tr); CSize wndSize(800,600+200), PicWndSize(326,265);
	CRect CameraOutWnd; CameraOutWnd.UnionRect(CameraWnd.CameraOutWnd, CameraWnd.LevelsScanWnd);
	dark.Create(0,"DARK",WS_CHILD | WS_VISIBLE | WS_CAPTION,CRect(CPoint(0,0),PicWndSize),this,111,0);	
	cupol.Create(0,"CUPOL",WS_CHILD | WS_VISIBLE | WS_CAPTION,CRect(CPoint(0,PicWndSize.cy),PicWndSize),this,222,0);
	strips.Create(0,"STRIPS",WS_CHILD | WS_VISIBLE | WS_CAPTION,CRect(CPoint(0,2*PicWndSize.cy),PicWndSize),this,333,0);
	CameraWnd.Create(0, "CameraWnd", WS_CHILD | WS_BORDER | WS_VISIBLE, 
		CRect(CPoint(tr.Width() - CameraOutWnd.Width(),0), CameraOutWnd.Size()), 
		this, ID_MV_WND, 0);	
	OnChildMove();
	CameraWnd.Ctrls.Create(IDD_DIALOGBARTAB4,&Ctrls); 
	CameraWnd.Ctrls.SetWindowPos(NULL,500,0,0,0,SWP_NOSIZE | SWP_NOZORDER);
	CameraWnd.Ctrls.ShowWindow(SW_SHOW);
#define TEST1
#ifdef DEBUG
	#if defined TEST1
//	 	dark.LoadPic(CString("..\\exe\\ttt.png"));
	 	//cupol.LoadPic(CString("..\\exe\\cupol.png"));
	 	//strips.LoadPic(CString("..\\exe\\strips.png"));
	#elif defined TEST2
		dark.LoadPic(CString("exe\\test1.png"));
		cupol.LoadPic(CString("exe\\test2.png"));
		strips.LoadPic(CString("exe\\test3.png"));
	#endif	
#endif
	Ctrls.Parent=this;
	SetScanRgn(Ctrls.GetScanRgnFromCtrls());	

	return 0;
}

void ImageWnd::OnDestroy()
{
	CameraWnd.DestroyWindow();
	CameraWnd.Ctrls.DestroyWindow();
	Ctrls.DestroyWindow();
	CWnd::OnDestroy();

}

void ImageWnd::SetScanRgn( const OrgPicRgn& rgn)
{
	OrgPicRgn tmp(rgn); HRESULT ret = RSLT_ERR;
	if ((ret = dark.ValidateOrgPicRgn(tmp)) != RSLT_OK)
	{
		if ((ret = cupol.ValidateOrgPicRgn(tmp)) != RSLT_OK)
		{
			if ((ret = strips.ValidateOrgPicRgn(tmp)) != RSLT_OK)
			{
				ScanRgn = rgn;
				return;
			}
		}
	}
	ScanRgn = tmp; Ctrls.InitScanRgnCtrlsFields(ScanRgn);
	dark.SetScanRgn(ScanRgn); cupol.SetScanRgn(ScanRgn); strips.SetScanRgn(ScanRgn);
}

typedef CArray<HWND> HWNDArray;

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	HWNDArray &childs=*((HWNDArray*)lParam); 
	HWND parent=childs[1], hwndParent=GetParent(hwnd);
	int n=(int)(childs[0]);
	CWnd* wnd=CWnd::FromHandle(hwnd);
	CString T; wnd->GetWindowText(T);
	if( hwndParent==parent ) childs.Add(hwnd); 
	if((childs.GetCount()-2)==n) return 0;
	else return 1;
}

void ImageWnd::OnChildMove()
{
	int n=GetWindowedChildCount();
	HWNDArray childs; childs.Add((HWND)n); childs.Add(m_hWnd);
	CRect result,tr,cr; GetClientRect(&cr);

	while(EnumChildWindows(m_hWnd, EnumChildProc,(LPARAM)&childs));
	int i=2; CWnd* twnd=NULL;
	if( (childs.GetCount()-2)>0 )
	{
		twnd=CWnd::FromHandle(childs[i++]);
		twnd->GetWindowRect(&tr); result.UnionRect(tr,tr);
		for(;i<childs.GetCount();i++)
		{
			
			twnd=CWnd::FromHandle(childs[i]);
			CString T; twnd->GetWindowText(T);
			twnd->GetWindowRect(&tr); 
			result.UnionRect(result,tr);
		}
	}
	CPoint tl=result.TopLeft(), br=result.BottomRight();
	ScreenToClient(&tl); ScreenToClient(&br);
	result=CRect(tl,br);
	if(cr.top!=result.top || cr.bottom!=result.bottom || cr.left!=result.left || cr.right!=result.right)
	{
		SetWindowPos(NULL,result.left,result.top,result.Width(),result.Height(), SWP_NOZORDER );
	}
}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(ImageWnd::CtrlsTab, BarTemplate)
	//{{AFX_MSG_MAP(DialogBarTab1)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedScan)
	ON_EN_KILLFOCUS(IDC_EDIT1, &ImageWnd::CtrlsTab::OnEnKillfocusEdit1)	
	ON_MESSAGE(UM_BUTTON_ITERCEPTED,&ImageWnd::CtrlsTab::OnButtonIntercepted)
	ON_BN_CLICKED(IDC_BUTTON5, &ImageWnd::CtrlsTab::OnBnClickedButton5)	
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

BOOL ImageWnd::CtrlsTab::OnInitDialog() 
{
	CDialog::OnInitDialog();
	NofScans.SetCurSel(1);
	return TRUE; 	
}

ImageWnd::CtrlsTab::CtrlsTab( CWnd* pParent /*= NULL*/ ): BarTemplate(pParent),
#if defined DEBUG
	stroka(220), Xmin(100), Xmax(6000)
#else
	stroka(1224), Xmin(2), Xmax(3263)
#endif

{
}

void ImageWnd::CtrlsTab::DoDataExchange(CDataExchange* pDX)
{
	BarTemplate::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
	DDX_Text(pDX, IDC_EDIT1, stroka);
	DDX_Text(pDX, IDC_EDIT3, Xmin);
	DDX_Text(pDX, IDC_EDIT4, Xmax);
	DDX_Control(pDX, IDC_EDIT3, XminCtrl);
	DDX_Control(pDX, IDC_EDIT4, XmaxCtrl);
	DDX_Control(pDX, IDC_EDIT1, strokaCtrl);
	DDX_Control(pDX, IDC_COMBO1, NofScans);
	//}}AFX_DATA_MAP

}

void ImageWnd::CtrlsTab::OnBnClickedScan()
{
	UpdateData(); ImageWnd* parent=(ImageWnd*)Parent;
	MyTimer Timer1,Timer2; sec time; 
	void* x; CString T; BOOL exit = FALSE;		
	ConrtoledLogMessage log; log << _T("Speed tests results");
	ImagesAccumulator &dark = parent->dark.accum, &cupol = parent->cupol.accum, &strips = parent->strips.accum;

	if (dark.bmp == NULL) 	{log << _T("There is no DARK image"); exit = TRUE;}
	if (cupol.bmp == NULL)	{log << _T("There is no CUPOL image"); exit = TRUE;}
	if (strips.bmp == NULL)	{log << _T("There is no STRIPS image"); exit = TRUE;}
	if (exit == FALSE)
	{
		if (dark.w != cupol.w || dark.h != cupol.h)	{log << _T("DARK != CUPOL"); exit = TRUE;}
		if (dark.w != strips.w || dark.h != strips.h)	{log << _T("DARK != STRIPS"); exit = TRUE;}
		if (cupol.w != strips.w || cupol.h != strips.h)	{log << _T("CUPOL != STRIPS"); exit = TRUE;}
		if (exit == FALSE)
		{			
			OrgPicRgn scan_rgn = parent->ScanRgn; CPoint cntr = scan_rgn.CenterPoint();
			T.Format("Scan line y = %d N = %d", cntr.y, strips.n);

			PointVsErrorArray dark_points, cupol_points, strips_points, result; Timer1.Start();

			dark.ScanLine(&(dark_points), cntr.y, scan_rgn.left, scan_rgn.right);
			cupol.ScanLine(&(cupol_points), cntr.y, scan_rgn.left, scan_rgn.right);
			strips.ScanLine(&(strips_points), cntr.y, scan_rgn.left, scan_rgn.right);

			PointVsError pnte; pnte.type.Set(GenericPnt); 
			for (int i = 0; i < dark_points.GetSize(); i++)
			{
				pnte.type.Set(GenericPnt); 
				pnte = (strips_points[i] - dark_points[i])/(cupol_points[i] - dark_points[i]);
				if (pnte.type.Get() != DivisionError) result.Add(pnte);
			}
			int diff;
			if ((diff = dark_points.GetSize() - result.GetSize()) != 0)
			{
				log.T.Format("%d points were excluded because of division error", diff); log << log.T;
				log.SetPriority(lmprHIGH);
			}
			if (result.GetSize() != 0)
			{
				CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 			
				if((x = chrt.Series.GainAcsess(WRITE))!=0)
				{
					SeriesProtector Protector(x); TSeriesArray& Series(Protector);
					TPointVsErrorSeries *t2 = NULL;
					if((t2 = new TPointVsErrorSeries(T)) != NULL)	
					{
						t2->_SymbolStyle::Set(NO_SYMBOL); t2->_ErrorBarStyle::Set(POINTvsERROR_BAR);				
						t2->PointType.Set(GenericPnt); 										
						for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
						Series.Add(t2); 
						t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
						t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
						TPointVsErrorSeries::DataImportMsg *ChartMsg = t2->CreateDataImportMsg(); 
						for (int i = 0; i < result.GetSize(); i++) 
						{
							ChartMsg->Points.Add(result[i]);
						}
						ChartMsg->Dispatch(); 
						mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	
					}		
				}				
			}
			Timer1.Stop(); 
			log.T.Format("Scan took %s", ConvTimeToStr( Timer1.GetValue()));
			log << log.T;
		}
	}
	if (exit == TRUE) log.SetPriority(lmprHIGH);
	log.Dispatch();			
}

ImageWnd::PicWnd::PicWnd()
{
	Parent=0;
}

ImageWnd::PicWnd::~PicWnd()
{
	POSITION pos = helpers.GetHeadPosition();
	while ( pos != NULL)
	{
		delete helpers.GetNext(pos);
	}
	helpers.RemoveAll();
}
BEGIN_MESSAGE_MAP(ImageWnd::PicWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DROPFILES()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_COMMAND(CaptureBtnID, OnCaptureButton)
	ON_MESSAGE(UM_CAPTURE_READY,OnCaptureReady)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PICWNDMENU_ERASE, OnPicWndErase)
	ON_COMMAND(ID_PICWNDMENU_SAVE, OnPicWndSave)
	ON_COMMAND(ID_PICWNDMENU_SCANLINE, OnPicWndScanLine)
	ON_WM_MOVE()
END_MESSAGE_MAP()

int ImageWnd::PicWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
    Parent=(ImageWnd*)GetParent();
	CRect r; GetClientRect(&r);
	ava.Create(this,r.Width(),r.Height(),24); 
	EraseAva();
	DragAcceptFiles(true);
	font1.CreatePointFont(80,"Arial"); 
	CRect t(CPoint(0,0),CSize(150,20)); t.OffsetRect(r.CenterPoint()-t.CenterPoint());
	CString T; GetWindowText(T); T="Capture "+T+" image";
	CaptureButton.Create(T,BS_CENTER | BS_TEXT | WS_VISIBLE | BS_PUSHBUTTON, t,this,CaptureBtnID);
	menu1.LoadMenu(IDR_MENU3);
	return 0;
}

void ImageWnd::PicWnd::EraseAva()
{
	CString T,T1;  GetWindowText(T1); T.Format("Drag or Capture %s image",T1);
	CRect r; GetClientRect(&r); ScanRgn.Erase(NULL);
	ava.PatBlt(WHITENESS); ava.Rectangle(r);
	ava.TextOut(10,10,T);
}

void ImageWnd::PicWnd::OnPaint()
{
	CPaintDC dc(this); 	
	HDC hdc=dc.GetSafeHdc();
	if (accum.bmp != NULL)
	{
		if(accum.bmp->HasImage())
		{
			ScanRgn.Draw(&ava);
		}
	}
	ava.CopyTo(hdc,TOP_LEFT);
}

void ImageWnd::PicWnd::UpdateNow(void)
{
	RedrawWindow(0,0,RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);					
}

HRESULT ImageWnd::PicWnd::LoadPic(CString T)
{	
	HRESULT ret;
	if(SUCCEEDED(ret = accum.LoadFrom(T)))
	{
		accum.ConvertToBitmap(this); BMPanvas &org = *(accum.bmp);

		//Parent->CameraWnd.Ctrls.UpdateData(); 
		//if (Parent->CameraWnd.Ctrls.ColorTransformSelector == CaptureWnd::CtrlsTab::TrueColor)
		//{
		//	ConrtoledLogMessage log(MessagePriorities::lmprHIGH); 
		//	log << _T("ERR: Image you are trying to load is no GRAYSCALE.");
		//	log << _T("ERR: In order to use bult-in convertor please select");			
		//	log << _T("ERR: convert method: NativeGDI, HSL or HSV.");			
		//	log.Dispatch();
		//	return E_FAIL;
		//}

		//if (accum.bmp->ColorType != BMPanvas::GRAY_PAL) ConvertOrgToGrayscale();
		FileName=T;
		EraseAva(); MakeAva();
        HGDIOBJ tfont=ava.SelectObject(font1); ava.SetBkMode(TRANSPARENT); ava.SetTextColor(clRED);
		ava.TextOut(0,0,T);
		T.Format("%dx%d", org.w, org.h); ava.TextOut(0,10,T);
		ava.SelectObject(tfont); 
		CaptureButton.ShowWindow(SW_HIDE); DragAcceptFiles(FALSE);
		UpdateNow();
		Parent->Ctrls.Xmax=org.w; Parent->Ctrls.UpdateData();
	}
	else FileName="";		 
	return ret;
}

void ImageWnd::PicWnd::OnDropFiles(HDROP hDropInfo)
{
	char buf[1000]; CString T,T2;  GetWindowText(T2);
	DragQueryFile(hDropInfo,0xFFFFFFFF,buf,1000);
	DragQueryFile(hDropInfo,0,buf,1000); T=CString(buf); 
	MyTimer Timer1; sec time; 	
	
	Timer1.Start(); 
	if (SUCCEEDED(LoadPic(T)))
	{
		BMPanvas &org = *(accum.bmp); ConrtoledLogMessage log;
		Parent->SetScanRgn(Parent->GetScanRgn());
		Timer1.Stop(); 
		time=Timer1.GetValue(); 
		log.T.Format("%s org (%.2f Mpix) load time=%s", T2, org.w*org.h/1e6, ConvTimeToStr(time)); log << log.T;		
		log.Dispatch();
	}	
	CWnd::OnDropFiles(hDropInfo);
}

void ImageWnd::PicWnd::c_ScanRgn::Draw( BMPanvas* canvas )
{
	Draw( canvas, *this, DRAW ); 
}

void ImageWnd::PicWnd::c_ScanRgn::Erase( BMPanvas * canvas )
{
	if ( ToErase == TRUE && canvas != NULL) 
		Draw( canvas, last, ERASE );
	ToErase = FALSE;
}

void ImageWnd::PicWnd::c_ScanRgn::Draw( BMPanvas* canvas, const AvaPicRgn& rgn, ScanRgnDrawModes mode )
{
	CPoint rgnCenter = rgn.CenterPoint(); int lastMode;
	if ( canvas == NULL) return;
	switch ( mode)
	{
	case DRAW: 
		Erase( canvas ); 
		last = *this; 
		ToErase = TRUE; 
		break;
	}	
	lastMode = canvas->SetROP2(R2_NOT);
	canvas->MoveTo(rgn.left, rgn.bottom); canvas->LineTo(rgn.right, rgn.top); 
	canvas->SetROP2(lastMode);
}

void ImageWnd::PicWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	BOOL update = FALSE; AvaPicRgn tmpRgn = ScanRgn; CPoint tmpRgnCntr = tmpRgn.CenterPoint();
	if (accum.bmp == NULL) return;
	if (accum.bmp->HasImage() == FALSE ) return;
	switch( nFlags )
	{
	case MK_SHIFT: tmpRgn.left = point.x; update=TRUE; break;
	case MK_CONTROL: tmpRgn.OffsetRect( 0, point.y - tmpRgnCntr.y ); update=TRUE; break;
	case 0: tmpRgn.OffsetRect( point - tmpRgnCntr ); update=TRUE; break;	
	}
	if ( update )
	{
		if (ValidateAvaPicRgn(tmpRgn) == RSLT_OK)
		{
			Parent->SetScanRgn( Convert(tmpRgn) );
		}		
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void ImageWnd::PicWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	BOOL update = FALSE; AvaPicRgn tmpRgn = (AvaPicRgn&)ScanRgn;
	if (accum.bmp == NULL) return;
	if (accum.bmp->HasImage() == FALSE ) return;
	switch( nFlags )
	{
	case MK_SHIFT: tmpRgn.right = point.x; update=TRUE; break;
	default: ;
	}
	if (update)
	{
		Parent->SetScanRgn( Convert(tmpRgn) );	
	}
	else
	{
		CWnd::OnRButtonUp(nFlags, point);
	}	
}

BOOL ImageWnd::PicWnd::IsRgnInAva( const AvaPicRgn& rgn)
{
	AvaPicRgn tmpRgn = rgn; tmpRgn.IntersectRect(ava.Rgn, rgn);
	return tmpRgn == rgn;
}


void ImageWnd::PicWnd::OnCaptureButton()
{
	UpdateHelpers(EvntOnCaptureButton);
}

LRESULT ImageWnd::PicWnd::OnCaptureReady( WPARAM wParam, LPARAM lParam )
{
	UpdateHelpers(EvntOnCaptureReady);
	return 0;
}
void ImageWnd::PicWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (accum.bmp != NULL)
	{
		if (accum.bmp->HasImage())
		{
			CMenu* menu = menu1.GetSubMenu(0);
			menu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
		}
	}
}

void ImageWnd::PicWnd::OnPicWndErase()
{
	accum.Reset(); EraseAva();
	CaptureButton.EnableWindow(TRUE); CaptureButton.ShowWindow(SW_SHOW); DragAcceptFiles(true);
	UpdateNow();	
}

void ImageWnd::PicWnd::OnPicWndSave()
{
	if (accum.bmp != NULL)
	{
		if(accum.bmp->HasImage())
		{
			CFileDialog dlg1(FALSE,"pack");
			if(dlg1.DoModal()==IDOK)
			{
				accum.SaveTo(dlg1.GetPathName());
			}
		}
	}
}

int ImageWnd::CtrlsTab::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (BarTemplate::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void ImageWnd::OnSize(UINT nType, int cx, int cy)
{
	CRect r,r1;
	CWnd::OnSize(nType, cx, cy);
	
	if (CameraWnd.m_hWnd!=NULL)
	{
		GetClientRect(&r); CameraWnd.GetWindowRect(&r1);
		CameraWnd.SetWindowPos(NULL,cx-r1.Width(),0,0,0,SWP_NOZORDER | SWP_NOSIZE);

		CScrollView* pFirstView=(CScrollView*)GetParent();
		pFirstView->SetScrollSizes(MM_TEXT, CSize(cx, cy));
	}
	
}

BOOL ImageWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect r; GetClientRect(&r);
	pDC->PatBlt(0,0,r.Width(),r.Height(),WHITENESS);

	return CWnd::OnEraseBkgnd(pDC);
}

void ImageWnd::PicWnd::OnMove(int x, int y)
{	
	CRect r,r1,r2; GetWindowRect(&r);
	CPoint tl=r.TopLeft(), br=r.BottomRight(); 
	Parent->ScreenToClient(&tl); Parent->ScreenToClient(&br);	
	r2=CRect(tl,br);
	CPoint pnt=r2.TopLeft(),shift(0,0);

	if(pnt.x<0 || pnt.y<0) 
	{
		if(pnt.x<0) shift.x=-pnt.x;
		if(pnt.y<0) shift.y=-pnt.y;
		r2.OffsetRect(shift);
		SetWindowPos(NULL,r2.left,r2.top,0,0,SWP_NOZORDER | SWP_NOSIZE);
		return;
	}		
	CWnd::OnMove(x, y);	
	RedrawWindow(0,0,RDW_FRAME | RDW_INVALIDATE);
	Parent->OnChildMove();
}

void ImageWnd::PicWnd::ConvertOrgToGrayscale()
{
	if (accum.bmp != NULL)
	{
		BMPanvas temp_replica; BMPanvas &org = *(accum.bmp);
		temp_replica.Create(&org, org.Rgn); org.CopyTo(&temp_replica, TOP_LEFT);
		org.Destroy(); org.Create(this,temp_replica.w,temp_replica.h,8);
		org.CreateGrayPallete(); Parent->CameraWnd.Ctrls.UpdateData();			
		ColorTransform(&temp_replica, &org, Parent->CameraWnd.Ctrls.ColorTransformSelector);
	}
}

HRESULT ImageWnd::PicWnd::ValidateOrgPicRgn( OrgPicRgn& rgn )
{
	CRect ret;
	if (accum.bmp == NULL) return RSLT_BMP_ERR;
	return ValidatePicRgn(rgn, *accum.bmp);	
}

HRESULT ImageWnd::PicWnd::ValidateAvaPicRgn( AvaPicRgn& rgn )
{
	return ValidatePicRgn(rgn, ava);
}

HRESULT ImageWnd::PicWnd::ValidatePicRgn( CRect& rgn, BMPanvas& ref )
{
	CRect ret(rgn); ret.NormalizeRect(); CSize offset; int diff; BOOL update = false;
	if (ref.HasImage() == FALSE) return RSLT_BMP_ERR;
	if (rgn.Width() > ref.Rgn.Width())
	{
		ret.left = ref.Rgn.left; ret.right = ref.Rgn.right; update = true;
	}
	else
	{
		if ((diff = ref.Rgn.left - ret.left) > 0 || (diff = ref.Rgn.right - ret.right) < 0)
		{
			offset.cx = diff; update = true;
		}			
	}
	if (rgn.Height() > ref.Rgn.Height())
	{
		ret.top = ref.Rgn.top; ret.bottom = ref.Rgn.bottom; update = true;
	} 
	else
	{
		//ref.Rgn.bottom - 1	in order not to cross the bottom border on scanline
		//ref.Rgn.top + 1		in order not to reduce ScanRgn height when it is moved above top border 
		if ((diff = ref.Rgn.top - ret.top) > 0 || (diff = ref.Rgn.bottom -1 - ret.bottom) < 0)
		{
			offset.cy = diff; update = true;
		}			
	}
	if (update)
	{
		ret.OffsetRect(offset);
		rgn = ret;
	}
	return RSLT_OK;	
}

void ImageWnd::PicWnd::SetScanRgn( const OrgPicRgn& rgn )
{
	ScanRgn = Convert(rgn);
	UpdateNow(); 
}

ImageWnd::OrgPicRgn ImageWnd::PicWnd::Convert( const AvaPicRgn& rgn )
{
	OrgPicRgn ret; 
	if (accum.bmp != NULL)
	{
		BMPanvas &org = *accum.bmp;
		if ( ava.HasImage() )
		{
			struct {double cx, cy;} scale = {(double)org.Rgn.Width() / ava.Rgn.Width(), (double)org.Rgn.Height() / ava.Rgn.Height()}; 
			ret.left = (LONG)(rgn.left * scale.cx); ret.right = (LONG)(rgn.right * scale.cx); 
			ret.top = (LONG)(rgn.top * scale.cy); ret.bottom = (LONG)(rgn.bottom * scale.cy);
		}
	}	
	return ret;
}

ImageWnd::AvaPicRgn ImageWnd::PicWnd::Convert( const OrgPicRgn& rgn )
{
	AvaPicRgn ret; 
	if (accum.bmp != NULL)
	{
		BMPanvas &org = *accum.bmp;
		if ( org.HasImage() )
		{
			struct {double cx, cy;} scale = {(double)ava.Rgn.Width() / org.Rgn.Width(), (double)ava.Rgn.Height() / org.Rgn.Height()}; 
			ret.left = (LONG)(rgn.left * scale.cx); ret.right = (LONG)(rgn.right * scale.cx); 
			ret.top = (LONG)(rgn.top * scale.cy); ret.bottom = (LONG)(rgn.bottom * scale.cy);
		}
	}
	return ret;
}

void ImageWnd::PicWnd::UpdateHelpers( const HelperEvent &event )
{
	BaseForHelper * accumCapture = NULL;
	ImageWnd::CtrlsTab &ctrls = Parent->Ctrls; ctrls.UpdateData();
	switch (event)
	{
	case EvntOnCaptureButton:
		this->accum.Reset();
		accumCapture = new AccumHelper(this, ctrls.GetNofScans());
		helpers.AddTail(accumCapture);
		break;
	default:
		POSITION pos = helpers.GetHeadPosition();
		while ( pos != NULL)
		{
			POSITION prev = pos;
			BaseForHelper* helper = helpers.GetNext(pos); 
			if (helper->Update(event) == RSLT_HELPER_COMPLETE)
			{
				delete helper; helpers.RemoveAt(prev);
			}			
		}
	}
}

HRESULT ImageWnd::PicWnd::MakeAva()
{
	if (accum.bmp != NULL)
	{
		BMPanvas &org = *(accum.bmp);
		SetStretchBltMode(ava.GetDC(),COLORONCOLOR);		
		org.StretchTo(&ava, ava.Rgn, org.Rgn,SRCCOPY);
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

void ImageWnd::PicWnd::OnPicWndScanLine()
{
	void* x; CString T; 	
	TPointVsErrorSeries::DataImportMsg *ChartMsg = NULL; 
	ConrtoledLogMessage log; log << _T("Speed tests results");

	if (accum.bmp != NULL)
	{
		CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 
		mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	

		OrgPicRgn scan_rgn = Parent->ScanRgn; CPoint cntr = scan_rgn.CenterPoint();
		T.Format("Scan line y = %d N = %d", cntr.y, accum.n);

		if((x=chrt.Series.GainAcsess(WRITE))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			TPointVsErrorSeries *t2;
			if((t2 = new TPointVsErrorSeries(T))!=0)	
			{
				for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
				Series.Add(t2); 
				t2->_SymbolStyle::Set(NO_SYMBOL); t2->_ErrorBarStyle::Set(POINTvsERROR_BAR);
				ChartMsg = t2->CreateDataImportMsg(); 
				t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
				t2->PointType.Set(GenericPnt); 
				t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
			}		
		}
		
		accum.ScanLine(&(ChartMsg->Points), cntr.y, scan_rgn.left, scan_rgn.right);
		log.T.Format("Scan of %d points took %g ms", ChartMsg->Points.GetSize(), accum.fillTime.val()); 
		log << log.T;
		ChartMsg->Dispatch();
		log.Dispatch();
	}
}


HelperEvent AccumHelper::Update( const HelperEvent &event )
{
	ImagesAccumulator &accum = parent->accum; CString T;
	switch (event)
	{
	case EvntOnCaptureReady:
		if (accum.n < n_max)
		{
			if (SUCCEEDED(accum.FillAccum(tmp_bmp)))
			{
				accum.ConvertToBitmap(parent);
				parent->Parent->SetScanRgn(parent->Parent->GetScanRgn());
				
				parent->MakeAva();
				parent->ScanRgn.Erase(NULL);

				HGDIOBJ tfont = parent->ava.SelectObject(parent->font1);
				parent->ava.SetBkMode(TRANSPARENT); COLORREF old_color = parent->ava.SetTextColor(clRED);
				T.Format("Camera capture %d of %d", accum.n, n_max); parent->ava.TextOut(0,0,T);
				T.Format("%dx%d", accum.w, accum.h); parent->ava.TextOut(0,10,T);
				parent->ava.SelectObject(tfont); parent->ava.SetTextColor(old_color);
				parent->UpdateNow();				

				if (accum.n == n_max)
				{
					return RSLT_HELPER_COMPLETE;					
				}
				else
				{
					parent->Parent->CameraWnd.PostMessage(UM_CAPTURE_REQUEST,(WPARAM)parent, (LPARAM)tmp_bmp);			
				}
			}		
		}
		else ASSERT(0);
		break;
	}
	return RSLT_OK;
}

AccumHelper::AccumHelper( ImageWnd::PicWnd *_parent, const int _n_max ) : parent(_parent), n_max(_n_max)
{
	tmp_bmp = NULL; tmp_bmp = new BMPanvas();
	parent->CaptureButton.EnableWindow(FALSE);
	parent->Parent->CameraWnd.PostMessage(UM_CAPTURE_REQUEST,(WPARAM)parent, (LPARAM)tmp_bmp);
	parent->CaptureButton.ShowWindow(SW_HIDE); parent->DragAcceptFiles(FALSE);
}

AccumHelper::~AccumHelper()
{	
	if (tmp_bmp != NULL)
	{
		delete tmp_bmp; tmp_bmp = NULL;
	}
	BaseForHelper::~BaseForHelper();
}


BOOL ImageWnd::CtrlsTab::DestroyWindow()
{
	return BarTemplate::DestroyWindow();
}

void ImageWnd::CtrlsTab::OnBnClickedButton5()
{
	ImageWnd *parent=(ImageWnd*)Parent;
	parent->dark.PostMessage(WM_COMMAND,ID_PICWNDMENU_ERASE,0);
	parent->cupol.PostMessage(WM_COMMAND,ID_PICWNDMENU_ERASE,0);
	parent->strips.PostMessage(WM_COMMAND,ID_PICWNDMENU_ERASE,0);
}

void ImageWnd::CtrlsTab::OnEnKillfocusEdit1() {}

ImageWnd::OrgPicRgn ImageWnd::CtrlsTab::GetScanRgnFromCtrls()
{
	UpdateData();
	OrgPicRgn ret; *((CRect*)&ret) = CRect( Xmin, stroka, Xmax, stroka); 
	return ret;
}

void ImageWnd::CtrlsTab::InitScanRgnCtrlsFields( const OrgPicRgn& rgn)
{
	Xmin = rgn.left; Xmax = rgn.right; stroka = rgn.CenterPoint().y;
	UpdateData(FALSE);
}

LRESULT ImageWnd::CtrlsTab::OnButtonIntercepted( WPARAM wParam, LPARAM lParam )
{
	UpdateData(); ImageWnd* parent=(ImageWnd*)Parent;
	parent->SetScanRgn(GetScanRgnFromCtrls());
	return NULL;
}

int ImageWnd::CtrlsTab::GetNofScans()
{
	CString text;
	NofScans.GetLBText(NofScans.GetCurSel(),text);
	return atoi(text);
}

IMPLEMENT_DYNAMIC(CEditInterceptor, CEdit)

BEGIN_MESSAGE_MAP(CEditInterceptor, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()


void CEditInterceptor::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 13)
	{
		CWnd * parent = GetParent();
		parent->PostMessage(UM_BUTTON_ITERCEPTED, nChar, 0);		
	}
	else
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
}

