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
	 	dark.LoadPic(CString("d:\\REPO\\pics\\dark.png"));
	 	cupol.LoadPic(CString("d:\\REPO\\pics\\cupol.png"));
	 	strips.LoadPic(CString("d:\\REPO\\pics\\strips.png"));
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

void ImageWnd::SetScanRgn( const CRect& rgn)
{
	CRect tmpScanRgn(rgn); HRESULT ret = RSLT_ERR;

	if (FAILED(ret = dark.ValidateScanRgn(tmpScanRgn)))
	{
		if (FAILED(ret = cupol.ValidateScanRgn(tmpScanRgn)))
		{
			if (FAILED(ret = strips.ValidateScanRgn(tmpScanRgn)))
			{
				RgnOfScan = tmpScanRgn;
				return;
			}
		}
	}
	RgnOfScan = tmpScanRgn; 
	Ctrls.InitCtrlsFromScanRgn(RgnOfScan);
	dark.UpdateScanRgn(); cupol.UpdateScanRgn(); strips.UpdateScanRgn();
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
	AvrRgnCombo.SetCurSel(1);
	return TRUE; 	
}

ImageWnd::CtrlsTab::CtrlsTab( CWnd* pParent /*= NULL*/ ): BarTemplate(pParent)
{
#if defined DEBUG
	stroka = 1200; AvrRange = 1; Xmin = 600; Xmax = 3100;
#else
	stroka = 1224; Xmin = 2; Xmax = 3263;
#endif
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
	DDX_Control(pDX, IDC_COMBO2, AvrRgnCombo);
	//}}AFX_DATA_MAP

}

void ImageWnd::CtrlsTab::OnBnClickedScan()
{
	UpdateData(); ImageWnd* parent=(ImageWnd*)Parent;
	MyTimer Timer1,Timer2; sec time; 
	CString T; BOOL exit = FALSE;		
	ControledLogMessage log; log << _T("Speed tests results");
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
			ScanRgnData data = parent->GetScanRgnData();
			T.Format("Scan line y = %d N = %d", data.stroka, strips.n);

			PointVsErrorArray dark_points, cupol_points, strips_points, result; Timer1.Start();

			dark.ScanLine(&(dark_points), data);
			cupol.ScanLine(&(cupol_points), data);
			strips.ScanLine(&(strips_points), data);

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
				CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 
				TPointVsErrorSeries *t1 = NULL;
				
				if((t1 = new TPointVsErrorSeries(T)) != NULL)	
				{
					t1->SetParentUpdateStatus(UPD_OFF);
					t1->_SymbolStyle::Set(NO_SYMBOL); t1->_ErrorBarStyle::Set(POINTvsERROR_BAR);				
					t1->AssignColors(ColorsStyle(clRED,RANDOM_COLOR));

					for (int i = 0; i < result.GetSize(); i++) 
					{
						t1->AddXY(result[i]);
					}
					t1->DispatchDataImportMsg(mf->Chart1);
					mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	
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
	ON_MESSAGE(UM_CAPTURE_EVENT,OnCaptureEvent)
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
			CRect RgnOfScan;
			if (SUCCEEDED(GetRgnOfScan(RgnOfScan)))
			{
				if (SUCCEEDED(ConvertOrgToAva(RgnOfScan)))
				{
					ScanRgn.Draw(&ava, RgnOfScan);
				}				
			}			
		}
	}
	ava.CopyTo(hdc,TOP_LEFT);
}

void ImageWnd::PicWnd::UpdateNow(void)
{
	RedrawWindow(0,0,RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);					
}

HRESULT ImageWnd::PicWnd::TryLoadBitmap(CString T, BMPanvas &bmp)
{
	HRESULT ret;
	if (SUCCEEDED(ret = bmp.LoadImage(T)))
	{
		Parent->CameraWnd.Ctrls.UpdateData();	
		const ColorTransformModes &colorTransformModes = Parent->CameraWnd.Ctrls.ColorTransformSelector;
		if (bmp.ColorType != BMPanvas::GRAY_PAL)
		{
			if (colorTransformModes == TrueColor)
			{
				ControledLogMessage log(lmprHIGH);
				log.T.Format("Error: Image you are trying to load which is no GRAYSCALE."); log << log.T;
				log.T.Format("*****: In order to use bult-in convertor please select"); log << log.T;
				log.T.Format("*****: convert method: NativeGDI, HSL or HSV."); log << log.T;
				log.Dispatch(); 
				return E_FAIL;
			}
			BMPanvas temp_replica; 
			temp_replica.Create(&bmp, bmp.Rgn); bmp.CopyTo(&temp_replica, TOP_LEFT);
			bmp.Destroy(); bmp.Create(this,temp_replica.w,temp_replica.h,8);
			bmp.CreateGrayPallete(); 
			ColorTransform(&temp_replica, &bmp, colorTransformModes);
		}
	}
	return ret;
}

HRESULT ImageWnd::PicWnd::LoadPic(CString T)
{	
	HRESULT ret; ControledLogMessage log;
	if(FAILED(ret = accum.LoadFrom(T)))
	{
		BMPanvas org;
		if (FAILED(ret = TryLoadBitmap(T, org)))
		{
			log.T.Format("Failed to Load as BITMAP %s", T); 
			log << log.T; log.SetPriority(lmprHIGH);	
			return E_FAIL;
		}
		else
		{
			if (FAILED(ret = accum.FillAccum(&org)))
			{
				log.T.Format("Failed to INIT accumulator from %s", T); 
				log << log.T; log.SetPriority(lmprHIGH);	
				return E_FAIL;				
			}			
		}
	}
	
	accum.ConvertToBitmap(this); BMPanvas &org = *(accum.bmp);

	FileName=T;
	EraseAva(); MakeAva();
	HGDIOBJ tfont=ava.SelectObject(font1); ava.SetBkMode(TRANSPARENT); ava.SetTextColor(clRED);
	ava.TextOut(0,0,T);
	T.Format("%dx%d", org.w, org.h); ava.TextOut(0,10,T);
	ava.SelectObject(tfont); 
	CaptureButton.ShowWindow(SW_HIDE); DragAcceptFiles(FALSE);
	UpdateNow();
	Parent->Ctrls.Xmax=org.w; Parent->Ctrls.UpdateData();

	log.Dispatch();
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
		BMPanvas &org = *(accum.bmp); ControledLogMessage log;
		Parent->SetScanRgn(Parent->GetRgnOfScan());
		Timer1.Stop(); 
		time=Timer1.GetValue(); 
		log.T.Format("%s org (%.2f Mpix) load time=%s", T2, org.w*org.h/1e6, ConvTimeToStr(time)); log << log.T;		
		log.Dispatch();
	}	
	CWnd::OnDropFiles(hDropInfo);
}

void ImageWnd::PicWnd::c_ScanRgn::Draw( BMPanvas* canvas, const CRect& rgn )
{
	Draw( canvas, rgn, DRAW ); 
}

void ImageWnd::PicWnd::c_ScanRgn::Erase( BMPanvas * canvas )
{
	if ( ToErase == TRUE && canvas != NULL) 
		Draw( canvas, last, ERASE );
	ToErase = FALSE;
}

void ImageWnd::PicWnd::c_ScanRgn::Draw( BMPanvas* canvas, const CRect& rgn, ScanRgnDrawModes mode )
{
	CPoint rgnCenter = rgn.CenterPoint(); int lastMode;
	if ( canvas == NULL) return;
	switch ( mode)
	{
	case DRAW: 
		Erase( canvas ); 
		last = rgn; 
		ToErase = TRUE; 
		break;
	}	
	lastMode = canvas->SetROP2(R2_NOT);
	canvas->MoveTo(rgn.left, rgn.top); canvas->LineTo(rgn.right, rgn.top); 
	if (rgn.bottom != rgn.top)
	{
		canvas->MoveTo(rgn.left, rgn.bottom); canvas->LineTo(rgn.right, rgn.bottom); 	
	}	
	canvas->SetROP2(lastMode);
}

void ImageWnd::PicWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPoint OrgPnt(point); 
	if (SUCCEEDED(ConvertAvaToOrg(OrgPnt)))
	{
		CRect ScanRgn;
		if (SUCCEEDED(GetRgnOfScan(ScanRgn)))
		{
			BOOL update = FALSE; CPoint ScanRgnCntr = ScanRgn.CenterPoint();		
			switch( nFlags )
			{
			case MK_SHIFT: ScanRgn.left = OrgPnt.x; update=TRUE; break;
			case MK_CONTROL: ScanRgn.OffsetRect( 0, OrgPnt.y - ScanRgnCntr.y ); update=TRUE; break;
			case 0: ScanRgn.OffsetRect( OrgPnt - ScanRgnCntr ); update=TRUE; break;	
			}
			if ( update )
			{
				Parent->SetScanRgn( ScanRgn );
			}
		}		
	}
	CWnd::OnLButtonUp(nFlags, point);	
}

void ImageWnd::PicWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	CPoint OrgPnt(point); 
	if (SUCCEEDED(ConvertAvaToOrg(OrgPnt)))
	{
		CRect ScanRgn;
		if (SUCCEEDED(GetRgnOfScan(ScanRgn)))
		{
			BOOL update = FALSE; CPoint ScanRgnCntr = ScanRgn.CenterPoint();		
			switch( nFlags )
			{
			case MK_SHIFT: ScanRgn.right = OrgPnt.x; update=TRUE; break;
			}
			if ( update )
			{
				Parent->SetScanRgn( ScanRgn );
			}
			else
			{
				CWnd::OnRButtonUp(nFlags, point);
			}
		}		
	}
}

void ImageWnd::PicWnd::OnCaptureButton()
{
	UpdateHelpers(EvntOnCaptureButton);	
}

LRESULT ImageWnd::PicWnd::OnCaptureEvent( WPARAM wParam, LPARAM lParam )
{
	HelperEvent event;
	switch (wParam)
	{
	case EvntOnCaptureReady:	event = EvntOnCaptureReady; break;
	case EvntOnCaptureStop:		event = EvntOnCaptureStop; break;
	default: return 0;
	}
	UpdateHelpers(event);
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

HRESULT ImageWnd::PicWnd::ValidateScanRgn( CRect& RgnOfScan ) const
{
	CRect ret(RgnOfScan); CSize offset; int diff; BOOL update = false;
	CRect PicRgn; 
	if (SUCCEEDED(accum.GetPicRgn(PicRgn)))
	{
		ret.NormalizeRect(); 
		if (ret.Width() > PicRgn.Width())
		{
			ret.left = PicRgn.left; ret.right = PicRgn.right; update = true;
		}
		else
		{
			if ((diff = PicRgn.left - ret.left) > 0 || (diff = PicRgn.right - ret.right) < 0)
			{
				offset.cx = diff; update = true;
			}			
		}
		if (ret.Height() > PicRgn.Height())
		{
			ret.bottom = PicRgn.bottom; ret.top = PicRgn.top; update = true;
		} 
		else
		{
			if ((diff = PicRgn.top - ret.top) > 0 || (diff = PicRgn.bottom - ret.bottom) < 0)
			{
				offset.cy = diff; update = true;
			}			
		}
		if (update)
		{
			ret.OffsetRect(offset);
			RgnOfScan = ret; 
		}
		return S_OK;
	}
	else
	{
		return E_FAIL;	
	}
}

void ImageWnd::PicWnd::UpdateScanRgn()
{
	UpdateNow(); 
}

HRESULT ImageWnd::PicWnd::ConvertAvaToOrg( CPoint& pnt ) const
{
	CRect AvaRgn, OrgRgn; GetClientRect(&AvaRgn);
	if (SUCCEEDED(accum.GetPicRgn(OrgRgn)))
	{
		CSize Ava = AvaRgn.Size(), Org = OrgRgn.Size();
		pnt.x = pnt.x*Org.cx/Ava.cx; pnt.y = pnt.y*Org.cy/Ava.cy;
		return S_OK;
	}
	return E_FAIL;
}

HRESULT ImageWnd::PicWnd::ConvertOrgToAva( CRect& rgn ) const
{
	CRect AvaRgn, OrgRgn; GetClientRect(&AvaRgn);
	if (SUCCEEDED(accum.GetPicRgn(OrgRgn)))
	{
		CSize Ava = AvaRgn.Size(), Org = OrgRgn.Size();
		rgn.left = rgn.left*Ava.cx/Org.cx; rgn.right = rgn.right*Ava.cx/Org.cx; 
		rgn.bottom = rgn.bottom*Ava.cy/Org.cy; rgn.top = rgn.top*Ava.cy/Org.cy;
		return S_OK;
	}
	return E_FAIL;
}

void ImageWnd::PicWnd::UpdateHelpers( const HelperEvent &event )
{
	BaseForHelper * accumCapture = NULL; POSITION pos;
	ImageWnd::CtrlsTab &ctrls = Parent->Ctrls; ctrls.UpdateData();
	switch (event)
	{
	case EvntOnCaptureButton:
		this->accum.Reset();
		accumCapture = new AccumHelper(this, ctrls.GetNofScans());
		helpers.AddTail(accumCapture);
		break;
	case EvntOnCaptureReady:
		pos = helpers.GetHeadPosition();
		while ( pos != NULL)
		{
			POSITION prev = pos;
			BaseForHelper* helper = helpers.GetNext(pos); 
			if (helper->Update(event) == RSLT_HELPER_COMPLETE)
			{
				delete helper; helpers.RemoveAt(prev);
			}			
		}
		break;
	case EvntOnCaptureStop:
		pos = helpers.GetHeadPosition();
		while ( pos != NULL)
		{
			POSITION prev = pos;
			BaseForHelper* helper = helpers.GetNext(pos); 
			delete helper; helpers.RemoveAt(prev);
		}
		UpdateNow();
		break;
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
	if (accum.bmp != NULL)
	{
		CString T; TPointVsErrorSeries *t1 = NULL;
		ScanRgnData data = Parent->GetScanRgnData();
		T.Format("Scan line y = %d N = %d", data.stroka, accum.n);
		if((t1 = new TPointVsErrorSeries(T)) != 0)	
		{
			ControledLogMessage log; log << _T("Speed tests results");
			CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 

			t1->SetParentUpdateStatus(UPD_OFF);
			t1->_SymbolStyle::Set(NO_SYMBOL); t1->_ErrorBarStyle::Set(POINTvsERROR_BAR);
			t1->AssignColors(ColorsStyle(clRED, RANDOM_COLOR));

			PointVsErrorArray pnts; accum.ScanLine(&pnts, data);
			for (int i = 0; i < pnts.GetSize(); i++) t1->AddXY(pnts[i]);	
			log.T.Format("Scan of %d points took %g ms", pnts.GetSize(), accum.fillTime.val()); log << log.T;
			t1->DispatchDataImportMsg(mf->Chart1);		
			log.Dispatch();

			mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	
		}				
	}
}

HRESULT ImageWnd::PicWnd::GetRgnOfScan( CRect& ScanRgn )
{
	if (Parent != NULL)
	{
		ScanRgn = Parent->GetRgnOfScan();
		return S_OK;
	}
	return E_FAIL;
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
				parent->Parent->SetScanRgn(parent->Parent->GetRgnOfScan());
				
				parent->MakeAva();
				parent->ScanRgn.Erase(NULL);

				HGDIOBJ tfont = parent->ava.SelectObject(parent->font1);
				parent->ava.SetBkMode(TRANSPARENT); COLORREF old_color = parent->ava.SetTextColor(clRED);
				T.Format("Camera capture %d of %d", accum.n, n_max); parent->ava.TextOut(0,0,T);

				CString T1, T2; parent->GetWindowText(T1); T2.Format("%s: %s", T1, T);
				StatusBarMessage *msg = new StatusBarMessage(0, T2);
				msg->Dispatch();

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

CRect ImageWnd::CtrlsTab::GetScanRgnFromCtrls()
{
	UpdateData(); AvrRange = GetAvrRgn();
	return CRect( Xmin, stroka - AvrRange, Xmax, stroka + AvrRange + 1 ); 
}

void ImageWnd::CtrlsTab::InitCtrlsFromScanRgn( const CRect& rgn)
{
	Xmin = rgn.left; Xmax = rgn.right; AvrRange = rgn.Height()/2; stroka = rgn.CenterPoint().y;	
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

int ImageWnd::CtrlsTab::GetAvrRgn()
{
	CString text;
	AvrRgnCombo.GetLBText(AvrRgnCombo.GetCurSel(),text);
	return atoi(text);
}

//======================================
void ImagesAccumulator::ResetSums()
{
	if (sums != NULL)	{ free(sums); sums = NULL; OldSumsSize = 0;}
}

void ImagesAccumulator::Reset()
{
	ResetSums(); 
	if (bmp != NULL)	{ delete bmp; bmp = NULL; }
	n = 0; w = h = 0; HasErrors = FALSE;
}

HRESULT ImagesAccumulator::Initialize(int _w, int _h, BOOL _HasErrors/* = TRUE*/)
{
	if (w != _w || h != _h)
	{
		ResetSums();
		w = _w; h = _h; HasErrors = _HasErrors;
		sums = (BYTE*)malloc((OldSumsSize = GetSumsSize())); 
		if (sums == 0) return E_FAIL;
		memset(sums, 0, GetSumsSize());		
	}
	else
	{
		if (HasErrors != _HasErrors)
		{
			w = _w; h = _h; HasErrors = _HasErrors;
			size_t NewSumsSize = GetSumsSize();
			BYTE *new_sums = (BYTE*)realloc(sums, NewSumsSize); 
			if (new_sums == NULL) return E_FAIL;
			sums = new_sums; 
			if(NewSumsSize > OldSumsSize)
			{
				BYTE *tmp_sums = sums; tmp_sums += OldSumsSize;
				memset(tmp_sums, 0, NewSumsSize - OldSumsSize);				
			}
			for (int i = OldSumsSize - 1; i > 0; i--)
			{
				new_sums[2*i] = new_sums[i];
				new_sums[i] = 0;
			}
			USHORT *tmp_sums = GetSum(); UINT *tmp_sums2 = GetSums2();
			for (size_t i = 0; i < OldSumsSize; i++, tmp_sums++, tmp_sums2++)
			{
				*tmp_sums2 = (*tmp_sums)*(*tmp_sums);				
			}
			OldSumsSize = NewSumsSize;
		}
	}
	return S_OK;
}

HRESULT ImagesAccumulator::FillAccum(BMPanvas *src)
{
	HRESULT ret; MyTimer Timer1;
	if (src == NULL) return E_FAIL;
	if (src->HasImage() == FALSE) return E_FAIL;	
		
	if (w != src->w || h != src->h)
	{
		if (n > 0)
		{
			ret = E_FAIL;			
		}
		else
		{
			ret = Initialize(src->w, src->h, FALSE);
		}
		if (FAILED(ret))
		{
			Reset();
			ControledLogMessage log(lmprHIGH);
			log.T.Format("ImagesAccumualtor error: %d != %d or %d != %d", w, src->w, h, src->h); log << log.T;
			log.Dispatch(); 
			return ret;
		}
	}
	if (n == 1)
	{		
		ret = Initialize(src->w, src->h);
	}
	if (sums == NULL) return E_FAIL;
	if (FAILED(ret)) 
	{
		Reset();
		return E_FAIL;
	}		
	Timer1.Start(); 
	src->LoadBitmapArray(); 
	if (HasErrors)
	{		
		BYTE *src_pxl; USHORT *accum_pxl; UINT* accum_pxl2;
		accum_pxl = GetSum(); accum_pxl2 = GetSums2();
		
		for (int y = 0; y < h; y++)
		{			
			src_pxl = src->arr + src->wbyte*y;
			for (int x = 0; x < w; x++)
			{
				*accum_pxl += *src_pxl; *accum_pxl2 += (*src_pxl)*(*src_pxl);
				src_pxl++; accum_pxl++; accum_pxl2++;
			}
		}		
	}
	else
	{
		BYTE *accum_pxl = (BYTE*)GetSum(), *bmp_pxl = src->arr;
		size_t line_size =  w*sizeof(BYTE);
		for (int y = 0; y < h; y++)
		{
			memcpy(accum_pxl, bmp_pxl, line_size);
			accum_pxl += line_size; bmp_pxl += src->wbyte;
		}
	}
	src->UnloadBitmapArray();		
	n++; 
	Timer1.Stop(); fillTime = Timer1.GetValue();
	return S_OK;
}

void ImagesAccumulator::ConvertToBitmap(CWnd *ref)
{
	MyTimer Timer1; 
	if (sums != NULL)
	{
		BYTE *dst_pxl; 
		Timer1.Start();
		if (bmp == NULL)
		{
			bmp = new BMPanvas(); 
		}
		bmp->Create(ref, w, h, 8); bmp->CreateGrayPallete(); bmp->LoadBitmapArray(); 
		if (HasErrors)
		{
			USHORT *accum_pxl = GetSum(); 
			for (int y = 0; y < h; y++)
			{			
				dst_pxl = bmp->arr + bmp->wbyte*y;
				for (int x = 0; x < w; x++)
				{
					*dst_pxl = (*accum_pxl)/n; 				
					dst_pxl++; accum_pxl++; 
				}
			}
		}
		else
		{
			BYTE *accum_pxl = (BYTE*)GetSum(); 
			for (int y = 0; y < h; y++)
			{			
				dst_pxl = bmp->arr + bmp->wbyte*y;
				for (int x = 0; x < w; x++)
				{
					*dst_pxl = *accum_pxl; 				
					dst_pxl++; accum_pxl++; 
				}
			}
		}
		bmp->SetBitmapArray(); 
		Timer1.Stop(); fillTime = Timer1.GetValue();
	}
}

HRESULT ImagesAccumulator::SaveTo( const CString &file )
{
	HRESULT ret; ControledLogMessage log; MyTimer Timer1;

	if (sums != NULL)
	{
		Compressor cmpr(Compressor::ZIP, 9); CFile dst0;
		TRY
		{
			CFileException Ex;
			if (dst0.Open(file, CFile::modeCreate | CFile::modeWrite| CFile::typeBinary) == FALSE)
			{
				Ex.ReportError(); ret = E_FAIL; return ret;
			}			
			CArchive ar(&dst0, CArchive::store); Serialize(ar); ar.Close();
		}
		AND_CATCH(CArchiveException, pEx)
		{
			pEx->ReportError(); ret = E_FAIL; return ret;
		}
		END_CATCH

		CMemFile src0; 
		src0.Attach(sums, GetSumsSize(), GetCompressorBufferSize()); src0.SetLength(GetSumsSize());				
		if ((ret = cmpr.Process(&src0, &dst0)) != Z_OK) 
		{
			dst0.Abort();
			log.T.Format("Failed to save %s", file); 
			log << log.T; log.SetPriority(lmprHIGH);	
		}		
		else 
		{			
			dst0.Close(); 
			log.T.Format("Time to save %s - %s Ratio = %.2f", 
				file, ConvTimeToStr(cmpr.LastSession.dt), cmpr.LastSession.ratio); 			
			log << log.T;
		}
		src0.Detach(); src0.Close();
	}
	log.Dispatch();	
	return ret;
}

HRESULT ImagesAccumulator::LoadFrom( const CString &file )
{	
	HRESULT ret = E_FAIL; ControledLogMessage log;
	if (bmp == NULL) bmp = new BMPanvas();		
	bmp->Destroy();
	
	ResetSums();
	ULONGLONG buf_size = 0; CFile src0;
	Compressor cmpr(Compressor::UNZIP);			
	TRY
	{
		CFileException Ex;
		if (src0.Open(file, CFile::modeRead| CFile::typeBinary, &Ex) == FALSE)
		{
			Ex.ReportError(); 
			return E_FAIL;
		}			
		CArchive ar(&src0, CArchive::load); 
		Serialize(ar); 
		ar.Close();
	}
	AND_CATCH(CArchiveException, pEx)
	{
		pEx->ReportError(); 
		return E_FAIL;
	}
	END_CATCH
	CMemFile dst0(GetCompressorBufferSize());	

	if ((ret = cmpr.Process(&src0, &dst0)) == Z_OK)
	{
		src0.Close(); sums = dst0.Detach();
		log.T.Format("Time to load %s - %s Ratio = %.2f", 
			file, ConvTimeToStr(cmpr.LastSession.dt), cmpr.LastSession.ratio); 			
		log << log.T;
	}	
	else
	{
		Reset();
		log.T.Format("Failed to UNZIP %s", file); 
		log << log.T; log.SetPriority(lmprHIGH);	
	}
	log.Dispatch();		
	return ret;
}

void ImagesAccumulator::ScanLine( void *_buf, const ScanRgnData &data)
{	
	PointVsErrorArray *buf = (PointVsErrorArray*)_buf;
	MyTimer Timer1; 
	if (sums != NULL)
	{
		PointVsError pnte; pnte.type.Set(GenericPnt);
		Timer1.Start(); 
		CSize size(data.Xmax - data.Xmin, 2*data.AvrRange + 1); int N = n*size.cy;	
		UINT *avr_accum = new UINT[size.cx];
		ULONG *avr_accum2 = new ULONG[size.cx];
		memset(avr_accum, 0, size.cx*sizeof(UINT));
		memset(avr_accum2, 0, size.cx*sizeof(ULONG));

		if (HasErrors)
		{			
			for (int y = data.stroka - data.AvrRange; y <= data.stroka + data.AvrRange; y++)
			{
				USHORT *accum_pxl = GetSum(); UINT *accum_pxl2 = GetSums2(); 
				accum_pxl += y*w + data.Xmin; accum_pxl2 += y*w + data.Xmin;

				for (int x = 0; x < size.cx; x++, accum_pxl++, accum_pxl2++)
				{
					avr_accum[x] += *accum_pxl;
					avr_accum2[x] += *accum_pxl2;
				}
			}
		}
		else
		{
			for (int y = data.stroka - data.AvrRange; y <= data.stroka + data.AvrRange; y++)
			{
				BYTE *accum_pxl = (BYTE*)GetSum();
				accum_pxl += y*w + data.Xmin;

				for (int x = 0; x < size.cx; x++, accum_pxl++)
				{
					avr_accum[x] += *accum_pxl;
					avr_accum2[x] += (*accum_pxl)*(*accum_pxl);
				}
			}
		}

		for (int x = 0; x < size.cx; x++)
		{
			pnte.x = data.Xmin + x; pnte.y = (float)(avr_accum[x])/N; 
			if (N > 1)
			{				
				ULONGLONG val = N*avr_accum2[x] - avr_accum[x]*avr_accum[x];
				pnte.dy = sqrt(((float)val)/((N - 1)*N));
			}
			else
			{
				pnte.dy = 0;
			}				
			buf->Add(pnte);
		}
		Timer1.Stop(); fillTime = Timer1.GetValue();
		delete[] avr_accum; delete[] avr_accum2;
	}
}

USHORT* ImagesAccumulator::GetSum() const
{
	return (USHORT*)(sums != NULL ? sums:NULL);
}

UINT* ImagesAccumulator::GetSums2() const
{
	return (UINT*)((sums != NULL && HasErrors == TRUE) ? sums + w*h*sizeof(USHORT):NULL);
}

ImagesAccumulator::ImagesAccumulator() : sums(NULL), bmp(NULL)
{
	Reset();	
}

HRESULT ImagesAccumulator::GetPicRgn( CRect& PicRgn ) const
{
	if (GetSum() != NULL)
	{
		PicRgn = CRect(CPoint(0, 0),CSize(w, h));
		return S_OK;
	}
	return E_FAIL;
}

size_t AccumInfo::GetSumsSize() const
{
	size_t ret;
	if (HasErrors)
	{
		ret = w*h*(sizeof(USHORT) + sizeof(UINT));
	}
	else
	{
		ret = w*h*sizeof(BYTE);
	}
	return ret;
}

//======================================

void AccumInfo::Serialize( CArchive &ar )
{
	if(ar.IsStoring())
	{
		ar << n << w << h << HasErrors;
	}
	else
	{
		ar >> n >> w >> h >> HasErrors;
	}

}

size_t AccumInfo::GetCompressorBufferSize() const
{
	size_t ret;
	if (HasErrors)
	{
		ret = (w*10*(sizeof(UINT) + sizeof(USHORT)));
	}
	else
	{
		ret = w*10*sizeof(BYTE);				
	}
	return ret;	
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


IMPLEMENT_DYNAMIC(CComboBoxInterceptor, CComboBox)

BEGIN_MESSAGE_MAP(CComboBoxInterceptor, CComboBox)
	ON_CONTROL_REFLECT_EX(CBN_SELCHANGE, OnCbnSelchangeCombo2)
END_MESSAGE_MAP()


BOOL CComboBoxInterceptor::OnCbnSelchangeCombo2()
{
	CWnd * parent = GetParent();
	if (parent != NULL)
	{
		parent->PostMessage(UM_BUTTON_ITERCEPTED, 0, 0);		
	}	
	return 0;
}


