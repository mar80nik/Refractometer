#include "stdafx.h"
#include "DialogBarTab1.h"
#include "MainFrm.h"
#include "mytime.h"
#include "metricon.h"
#include "fittings.h"



IMPLEMENT_DYNAMIC(MainChartWnd, TChart)
MainChartWnd::MainChartWnd(): TChart("Chart1")
{
}
/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 dialog
BEGIN_MESSAGE_MAP(MainChartWnd, TChart)
	//{{AFX_MSG_MAP(TChart)
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	//}}AFX_MSG_MAP	
	ON_WM_CREATE()
END_MESSAGE_MAP()

int MainChartWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (TChart::OnCreate(lpCreateStruct) == -1)
		return -1;
	Panel.Parent=this;
	return 0;
}

void MainChartWnd::Serialize(CArchive& ar)
{		
	Panel.Serialize(ar);
}

LRESULT MainChartWnd::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{	
	TChart::OnSeriesUpdate(wParam, lParam);
	Panel.PostMessage(UM_SERIES_UPDATE,wParam,lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 dialog
BEGIN_MESSAGE_MAP(DialogBarTab1, BarTemplate)
	//{{AFX_MSG_MAP(DialogBarTab1)
	ON_EN_KILLFOCUS(IDC_LMAX_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_STEP_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_SCANSPEED_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_QUANTITY_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_LMIN_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_INTERVAL_EDIT, OnKillfocus)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClicked_Fit)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClicked_Locate)
	ON_BN_CLICKED(IDC_BUTTON1, &DialogBarTab1::OnBnClickedLoadCalibration)
	ON_BN_CLICKED(IDC_BUTTON4, &DialogBarTab1::OnBnClickedKneeTest)

	ON_BN_CLICKED(IDC_BUTTON9, &DialogBarTab1::OnBnClickedCalibrate)
	ON_BN_CLICKED(IDC_BUTTON17, &DialogBarTab1::OnBnClickedCalcFilm)


END_MESSAGE_MAP()

DialogBarTab1::DialogBarTab1(CWnd* pParent /*=NULL*/)
	: BarTemplate(pParent)
	, spec_wdth(5)
	, X0(1200)
	, dX(20)
	, PolinomOrder(2)
	, minimum_widht_2(20)
	, Xmin(860)
	, Xmax(3000)
	, level(0.95)
{
	//Message1.body.Object="MainControl";
	//{{AFX_DATA_INIT(DialogBarTab1)
		Name="MainControlTab";
	//}}AFX_DATA_INIT
	CheckButtonsStatus=0;
}

void DialogBarTab1::DoDataExchange(CDataExchange* pDX)
{
	BarTemplate::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
	DDX_Text(pDX, IDC_EDIT1, spec_wdth);
	DDV_MinMaxInt(pDX, spec_wdth, 1, 100);
	DDX_Text(pDX, IDC_EDIT4, X0);
	DDX_Text(pDX, IDC_EDIT5, dX);
	DDX_Text(pDX, IDC_EDIT2, PolinomOrder);
	DDV_MinMaxInt(pDX, PolinomOrder, 0, 100);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT3, minimum_widht_2);
	DDV_MinMaxInt(pDX, minimum_widht_2, 1, 3000);
	DDX_Control(pDX, IDC_COMBO1, SeriesCombo);
	DDX_Text(pDX, IDC_EDIT7, Xmin);
	DDX_Text(pDX, IDC_EDIT8, Xmax);
	DDX_Text(pDX, IDC_EDIT9, level);
}

/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 message handlers
int DialogBarTab1::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (BarTemplate::OnCreate(lpCreateStruct) == -1) return -1;

	CalibratorDlg.Create(IDD_DIALOG_CAL,this);
	CalibratorDlg.SetWindowPos(NULL,300,300,0,0,SWP_NOSIZE | SWP_NOZORDER);

	CalcTEDlg.Create(IDD_DIALOG_CALCTE,this);
	CalcTEDlg.SetWindowPos(NULL,600,300,0,0,SWP_NOSIZE | SWP_NOZORDER);

	return 0;
}

BOOL DialogBarTab1::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DialogBarTab1::OnKillfocus() {UpdateData();}

void DialogBarTab1::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	// storing code
	}
	else
	{	// loading code
		UpdateData(0);
	}
}

int DialogBarTab1::GetActiveSeriesData(ControledLogMessage& log, PointVsErrorArray& buf, const double x_min, const double x_max)
{
	buf.RemoveAll(); void *x; TChart *chart=(TChart *)Parent;

	if((x=chart->Series.GainAcsess(READ))!=NULL)
	{
		TPointVsErrorSeries *graph;
		SeriesProtector guard(x); TSeriesArray& series(guard);
		if(	(graph=GetSeries(series))!=NULL) 
		{
			graph->GetValues(buf);
			int N=buf.GetSize(), n1=GetArrayIndex(buf.x, x_min), n2=GetArrayIndex(buf.x, x_max);
			if(n1<0 || n2>=N)
			{
				log.T.Format("ERR: No valid points found %d+/-%d",X0,dX); log << log.T;	
				log.SetPriority(lmprHIGH); log.Dispatch(); 
				buf.RemoveAll(); return buf.GetSize();
			}				
			buf.RemoveAll(); graph->GetValues(buf,n1,n2);					
		}			
		else
		{
			log.T.Format("ERR: No series matching criteria (ACTIVE) found"); log << log.T;	
			log.SetPriority(lmprHIGH); log.Dispatch(); 
			buf.RemoveAll(); return buf.GetSize();
		}
	}
	return buf.GetSize();
}

void DialogBarTab1::OnBnClicked_Fit()
{		
	UpdateData(); ControledLogMessage log; PointVsErrorArray buf;

	if (GetActiveSeriesData(log, buf, X0 - dX, X0 + dX) IS 0) return;

	DoubleArray init; ParabolaFitFunc fiting; 	SimplePoint pnt;	
	init << 1 << 1e-1 << 1e-2; fiting.CalculateFrom(buf.x, buf.y, buf.dy, init);
	log.T.Format("************ ParabolaFit ******************"); log << log.T;
	fiting.GetReport(log); log.Dispatch();		

	DoubleArray X, Y;
	if(fiting.PrepareGraph(X, Y, buf.x[0], buf.x[buf.x.GetUpperBound()], buf.x.GetSize()*3 ) IS GSL_SUCCESS)
	{
		CString name; name.Format("PolyFit%d",PolinomOrder);
		CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 
		mf->Chart1.Visualize(name, X, Y);
	}
}

void DialogBarTab1::OnBnClickedKneeTest()
{
	UpdateData(); ControledLogMessage log; PointVsErrorArray buf;

	if (GetActiveSeriesData(log, buf, Xmin, Xmax) IS 0) return;

	DoubleArray init;  KneeFitFunc fiting;  
	double x_min = buf.x[0], x_max = buf.x[buf.x.GetUpperBound()];
	init << 1 << (x_min + x_max)/2 << 0.1 << 0.1; 
	fiting.CalculateFrom(buf.x, buf.y, buf.dy, init);
	log.T.Format("************ KneeFit **********************"); log << log.T;
	fiting.GetReport(log, level); log.Dispatch();

	DoubleArray X, Y;
	if(fiting.PrepareGraph(X, Y, x_min, x_max, buf.x.GetSize()*3 ) IS GSL_SUCCESS)
	{
		CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 

		mf->Chart1.Visualize("KneeFit", X, Y);

		X.RemoveAll(); Y.RemoveAll();		
		SimplePoint pnt; pnt.y =  fiting.GetInflection(pnt.x, level); X << pnt.x; Y << pnt.y;
		TChartSeriesStyleHelper style; 
		style << NO_LINE << VERT_LINE << ColorsStyle(clWHITE, clWHITE) << SYMBOL_DY(10);
		mf->Chart1.Visualize("KneeInflection", X, Y, style);
	}
}

//////////////////////////////////////////////////////////////////////////

int FourierSmoothFunc(ControledLogMessage &log, SimplePointArray &data, const int spec_wdth)
{
	ms dt; int status; 

	FFTRealTransform::Params in(data.y), out;
	status = FourierFilter(in,spec_wdth,out);
	dt = out.dt;

	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 
	TChartSeriesStyleHelper style; style << ColorsStyle(clRED,clRED);
	mf->Chart1.Visualize("FourierSmooth", data.x, data.y, style);

	log.T.Format("********Fourier smooth*************"); log << log.T;
	log.T.Format("time=%g ms", dt.val()); log << log.T;
	
	return status;
}

//////////////////////////////////////////////////////////////////////////

int LocateMinimumsFunc(ControledLogMessage &log, SimplePointArray &data)
{
	ms dt; int minimumN; int status; 
	double drv_l, drv_r; int i, j; MyTimer timer1;
    
	timer1.Start();

	double *y=data.GetY(), *x=data.GetX(); 
	drv_l=y[1]-y[0];
	for(i=1,j=0;i<data.GetSize()-1;i++)
	{		
		drv_r=y[i]-y[i+1]; 
		if( drv_l<0 && drv_r<0)
		{
			x[j]=x[i]; y[j++]=y[i]; 
		}
		drv_l=-drv_r;
	}
	data.SetSize(j); 

	minimumN = data.GetSize();
	dt = timer1.StopStart();
	status = S_OK;

	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 
	TChartSeriesStyleHelper style; style << NO_LINE << CIRCLE << ColorsStyle(clRED, RANDOM_COLOR);
	mf->Chart1.Visualize("Minimums", data.x, data.y, style);	

	log.T.Format("********Minimums 1stage*************"); log << log.T;
	log.T.Format("minimums=%d time=%g ms", minimumN, dt.val()); log << log.T;

    return status;
}
//////////////////////////////////////////////////////////////////////////

enum FitingFailReason {OUT_OF_RANGE = 0,FIT_GSL_FAIL_DMAX, FIT_SIGN_FAIL_DMAX,FIT_GSL_FAIL, FIT_SIGN_FAIL};

struct FailedFiting
{
	SimplePoint min;
	FitingFailReason Reason;

	FailedFiting() {}
	FailedFiting(const SimplePoint _min, const FitingFailReason _Reason)		
	{
		min = _min; Reason = _Reason;
	}
	FailedFiting& operator=(const FailedFiting &f)
	{
		min = f.min;
		Reason = f.Reason;
		return *this;
	}
};

#define INDEX_OUT_OF_RANGE -1;

int GetArrayIndex(DoubleArray& arr, double x )
{
	int ret=INDEX_OUT_OF_RANGE;
	double l,r,m; int N=arr.GetSize(),l_n=0,r_n=N-1,m_n;
	if(N==0) return INDEX_OUT_OF_RANGE;
	if(N==1) return 0;
	l=arr[l_n]-x; r=arr[r_n]-x;
	if( (l<0 && r<0) || (l>0 && r>0) ) return ( (fabs(l)<fabs(r)) ? l_n: r_n );
	
	m_n=l_n+(r_n-l_n)/2; m=arr[m_n];
	while(m_n!=l_n)
	{
		m=arr[m_n]-x;
		if( (m<0 && r<0) || (m>0 && r>0) ) { r_n=m_n; r=m; }
		else { l_n=m_n; l=m; }
		m_n=l_n+(r_n-l_n)/2; 
	}
	ret=( (fabs(l)<fabs(r)) ? l_n: r_n );
	return ret;
}

int MinimumsFitFilterFunc(ControledLogMessage &log, PointVsErrorArray &data,SimplePointArray &mins, int dn)
{
	ms dt; int status; 
	TypeArray<ParabolaFitFunc> fitings;
	TypeArray<FailedFiting> failed_fitings;

	MyTimer timer1; int i, j, dn_max=80, index; DoubleArray init; init << 1 << 1e-1 << 1e-2;
	ParabolaFitFunc fiting;  
	PointVsErrorArray buft; 

	timer1.Start();
	SimplePoint data0=data[0]; 	
	for(i = 0, j = 0; i < mins.GetSize(); i++)
	{
		SimplePoint tt = mins[i];
		index = GetArrayIndex(data.x, tt.x);
		if( index < dn_max ) 
		{
			FailedFiting ff(tt, OUT_OF_RANGE); 
			failed_fitings << ff;
			continue;
		}

		buft.CopyFrom(data, 2*dn_max + 1, index - dn_max); 
		fiting.CalculateFrom(buft.x, buft.y, buft.dy, init);
	    if(fiting.status != GSL_SUCCESS) 
		{
			FailedFiting ff(tt, FIT_GSL_FAIL_DMAX); 
			failed_fitings << ff;
			continue;
		}
		if(fiting.a[2] < 0) 
		{
			FailedFiting ff(tt, FIT_SIGN_FAIL_DMAX); 
			failed_fitings << ff;
			continue;
		}

		buft.CopyFrom(data,2*dn+1,index-dn); 
		fiting.CalculateFrom(buft.x, buft.y, buft.dy, init);
		double a0 = fiting.a[ParabolaFuncParams::ind_c];
		double a1 = fiting.a[ParabolaFuncParams::ind_b];
		double a2 = fiting.a[ParabolaFuncParams::ind_a];
		if(fiting.status != GSL_SUCCESS) 
		{
			FailedFiting ff(tt, FIT_GSL_FAIL); 
			failed_fitings << ff;
			continue;
		}
		if(fiting.a[2] < 0) 
		{
			FailedFiting ff(tt, FIT_SIGN_FAIL); 
			failed_fitings << ff;
			continue;
		}

		tt.y = fiting.GetTop(tt.x);
		if( tt.x < buft.x[0] || tt.x > buft.x[buft.GetUpperBound()] ) continue;

		fitings << fiting; 
		mins.Set(j++, tt);
	}
	mins.SetSize(j);
	dt = timer1.StopStart();
	status = S_OK;	

	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 
	TChartSeriesStyleHelper style; style << NO_LINE << VERT_LINE << ColorsStyle(clWHITE,clWHITE) << SYMBOL_DY(10);
	mf->Chart1.Visualize("FinalMins", mins.x, mins.y, style);

	log.T.Format("********Minimums 2stage*************"); log << log.T;
	log.T.Format("minimums=%d time=%g ms", fitings.GetSize(), dt.val()); log << log.T;
	if (failed_fitings.HasValues())
	{
		log.T.Format("--=== Failed Mins (%d) ===--", failed_fitings.GetSize()); log << log.T;
		for (int i = 0; i < failed_fitings.GetSize(); i++)
		{
			FailedFiting &tt = failed_fitings[i];
			log.T.Format("%d: at %g with Reason = %d", i + 1, tt.min.x, (int)tt.Reason); log << log.T;
		}
	}

	return status;
}

TPointVsErrorSeries* DialogBarTab1::GetSeries(TSeriesArray& series)
{
	TPointVsErrorSeries* ret=NULL;	TChart *chart=(TChart *)Parent; int n;
	
	SeriesSearchPattern pattern; pattern.mode=SERIES_PID | SERIES_TYPE; 
	pattern.SeriesType=POINTvsERROR;
	if( (n=SeriesCombo.GetCurSel())!=CB_ERR)
	{
		pattern.PID=SeriesCombo.GetItemData(n);
		TSeriesArray results(DO_NOT_DELETE_SERIES);
		if(	series.FindSeries(pattern,results)!=0 ) 
		{
			ret=(TPointVsErrorSeries*)results.Series[0];
		}	
	}	
	return ret;
}
//////////////////////////////////////////////////////////////////////////
afx_msg void DialogBarTab1::OnBnClicked_Locate()
{
	TChart *chart=(TChart *)Parent; void *x; SimplePoint val; TChartSeries *t1=NULL;
	int n=0; MyTimer timer1; ms dt1,dt2,dt3,dt4,dt5; UpdateData();
	TPointVsErrorSeries *graph; CString str;
	PointVsErrorArray BUF; SimplePointArray buf_smooth; 
	ControledLogMessage log; 
	
	timer1.Start();
	if((x=chart->Series.GainAcsess(READ))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);
		if(	(graph=GetSeries(series))!=NULL) 
		{
			graph->GetValues(BUF);
		}			
		else
		{
			log.SetPriority(lmprHIGH);
			log.T.Format("No series matching criteria (ACTIVE) found"); log << log.T;
			log.Dispatch(); return;
		}
	}
	else return;

	buf_smooth.CopyFrom(BUF);

	FourierSmoothFunc(log, buf_smooth, spec_wdth); 
	LocateMinimumsFunc(log, buf_smooth); 
	MinimumsFitFilterFunc(log, BUF, buf_smooth, minimum_widht_2); 

	dt1=timer1.StopStart();

	log.T.Format("*********Total********************"); log << log.T;
	log.T.Format("time=%g ms",dt1.val()); log << log.T;

	log.Dispatch();
	//delete MinimumsFitFilter;
}

#define MAX_SYMBOLS 1000
#define MAX_VALUES 7
#define END_OF_FILE -1
#define BUF_OVERLOAD -2

int ReadString(FILE* file, unsigned char *buf, int buf_max )
{
	int n=0;
	while(n<buf_max)
	{
		if( fread(buf,1,1,file) ==0) { *buf=0; return END_OF_FILE; }		
		if(*buf==0x0a) { *buf=0; return n+1; }
		buf++; n++;
	}
	return BUF_OVERLOAD;
}

int AnalyzeString(CString &str, double *arr, int max_arr)
{
	int first=0,last=0,n=0; CString val_str;
	while( (last=str.Find(',',first)) >=0 && n<MAX_VALUES) 
	{
		val_str=str.Mid(first,last-first); arr[n++]=atof(val_str);
		first=last+1;		
	}
	val_str=str.Mid(first,str.GetLength()-first); if(n<MAX_VALUES) arr[n++]=atof(val_str);
	return n;
}

void DialogBarTab1::OnBnClickedLoadCalibration()
{
	CString filter="Metricon output (*.txt)|*.txt||", logT, FullFileName, FileName; double vals[MAX_VALUES]; 
	MyTimer Timer1; sec time; int n,strs=0;
	ControledLogMessage log; log.T.Format("Log: Metricon load Speed tests"); log << log.T;
	
	CFileDialog dlg1(true,NULL,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);
	if(dlg1.DoModal()==IDOK)
	{
		Timer1.Start(); FullFileName=dlg1.GetFileName();

		int pos=-1; n=-1;
		while( (n=FullFileName.Find("\\",n+1))!=-1 ) pos=n; 
		FileName=FullFileName.Right(FullFileName.GetLength()-(pos+1));
		while( (n=FullFileName.Find(".",n+1))!=-1 ) pos=n; 
		FileName=FileName.Left(pos);

		FILE *file; fopen_s(&file,FullFileName,"r");
		unsigned char buf[MAX_SYMBOLS]; 

		TPointVsErrorSeries *t1 = NULL; PointVsError pnte;	PointVsErrorArray pnts;	
		CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 
		if((t1 = new TPointVsErrorSeries(FileName))!=0)	
		{			
			t1->SetParentUpdateStatus(UPD_OFF);
			t1->_SymbolStyle::Set(NO_SYMBOL); t1->AssignColors(ColorsStyle(clRED, RANDOM_COLOR));

			while( (n=ReadString(file,buf,MAX_SYMBOLS)) != END_OF_FILE && n!=BUF_OVERLOAD && t1 != NULL )
			{
				if(strs++<3) continue;
				CString str(buf);
				if( AnalyzeString(str, vals, MAX_VALUES)==MAX_VALUES )
				{
					pnte.x=vals[2]; pnte.y=vals[5]; pnte.dy=1e-3;
					if(pnte.x<0) break;
					t1->AddXY(pnte); pnts.Add(pnte);
				}			
			}	
			t1->DispatchDataImportMsg(mf->Chart1); 

			int N = t1->GetSize(); double tt;
			double* X=pnts.GetX(),*lX,*rX; lX=X; rX=lX+N-1;
			double* Y=pnts.GetY(),*lY,*rY; lY=Y; rY=lY+N-1;
			double* DY=pnts.GetdY(),*lDY,*rDY; lDY=DY; rDY=lDY+N-1;
			for(int i=0;i<N/2;i++)
			{
				tt=*lX; *lX=*rX; *rX=tt; lX++; rX--;
				tt=*lY; *lY=*rY; *rY=tt; lY++; rY--;
				tt=*lDY; *lDY=*rDY; *rDY=tt; lDY++; rDY--;
			}
			Timer1.Stop(); time=Timer1.GetValue();
			log.T.Format("* Loaded %d points time=%s", N,ConvTimeToStr(time)); log << log.T;
			log.Dispatch();
		}		
	}
}

LRESULT DialogBarTab1::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{
	CString T; int n,pid;
	if(lParam!=0) 
	{
		Series=(ProtectedSeriesArray*)lParam;
		SeriesCombo.ResetContent();
		void *x; 
		if((x=Series->GainAcsess(READ_EX))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);			

			SeriesSearchPattern pattern; 
			pattern.mode=SERIES_TYPE; pattern.SeriesType=POINTvsERROR;
			TSeriesArray results(DO_NOT_DELETE_SERIES); 
			if( Series.FindSeries(pattern,results)!=0 )
			{				
				for(int i=0;i<results.Series.GetSize();i++)
				{
					T.Format("%s (%d)",results.Series[i]->Name,results.Series[i]->GetSize());
					n=SeriesCombo.AddString(T); pid=results.Series[i]->GetPID();
					SeriesCombo.SetItemData(n,pid);
				}				
			}
		}
		CalibratorDlg.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
		CalcTEDlg.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
	}
	return 0;
}

void MyGSL_Tester_Helper(Polarization pol, DoubleArray &Nexp, DoubleArray &_n_exp, int shift)
{
	MyTimer Timer1; sec dt;

	FilmParams film; CalibrationParams cal; ControledLogMessage log;
	if (pol == TE)
	{
		log.T.Format("Log: ---=== TE testing ===---"); log << log.T;
	}
	if (pol == TM)
	{
		log.T.Format("Log: ---=== TM testing ===---"); log << log.T;
	}
	log.Dispatch();

	// tetsing calibration creation
	ControledLogMessage log1;
	cal.CalculateFrom(Nexp, _n_exp, 2.15675, 1., 1.45705, 51*DEGREE, 632.8);
	log1.T.Format("---Calibration (status = %s)---", gsl_strerror (cal.status)); log1 << log1.T;
	log1.T.Format("N0=%.10f L=%.10f", 
		cal.val[CalibrationParams::ind_N0], cal.val[CalibrationParams::ind_L]); log1 << log1.T;
	log1.T.Format("d0=%.10f fi0=%.10f", 
		cal.val[CalibrationParams::ind_d0],	cal.val[CalibrationParams::ind_fi0]); log1 << log1.T;
	log1.T.Format("errabs=%g errrel=%g dt=%s func_calls=%d",
		cal.err.abs, cal.err.rel, ConvTimeToStr(cal.dt), cal.cntr.func_call); log1 << log1.T;
	log1.T.Format("delta = %g", cal.delta); log1 << log1.T;
	for(int i = 0; i < Nexp.GetSize(); i++)
	{		
		log1.T.Format("m = %d -> exp = %g calc = %g", i, cal.Nexp[i], cal.Ncalc[i]); log1 << log1.T;
	}
	if (cal.status != GSL_SUCCESS) log1.SetPriority(lmprHIGH);
	log1.Dispatch();

	// testing pixel to angle conversion with calibration
	ControledLogMessage log2;
	log2.T.Format("---Calibrator----"); log2 << log2.T;
	DoubleArray n_exp; 
	for(int i = 0; i < Nexp.GetSize(); i++)
	{
		AngleFromCalibration angle; double n_calc;
		angle = cal.ConvertPixelToAngle(Nexp[i]); 
		n_calc = cal.ConertAngleToBeta(angle.teta);
		log2.T.Format("status = %s", gsl_strerror (angle.status)); log2 << log2.T;
		log2.T.Format("teta_calc=%.5f degree n_calc = %.4f n_orig=%.4f diff=%g%%", 
			angle.teta/DEGREE, n_calc, _n_exp[i], 100.*(_n_exp[i] - n_calc)/n_calc); log2 << log2.T;
		log2.T.Format("dt=%s func_calls=%d", ConvTimeToStr(angle.dt), angle.cntr.func_call); log2 << log2.T;
		n_exp << n_calc;
		if (angle.status != GSL_SUCCESS) log1.SetPriority(lmprHIGH);		
	}
	log2.Dispatch();


	DoubleArray range_min, range_max, dd;
	range_min << _n_exp[0] << 200; 
	range_max << 2.1 << 2000;
	dd << 1e-3 << 18;

	// test film parameters calculation
	ControledLogMessage log3;
	film.Calculator2(pol, cal, n_exp, shift, range_min, range_max, dd); 	
	log3.T.Format("--FilmParams (status = %s)---", gsl_strerror (film.status)); log3 << log3.T;	
	log3.T.Format("---- frst ----n=%.10f H=%.10f nm", film.n_init, film.H_init); log3 << log3.T;
	log3.T.Format("---- last ----n=%.10f H=%.10f nm", film.n, film.H); log3 << log3.T;
	log3.T.Format("errabs=%g errrel=%g fval=%.10f", film.err.abs, film.err.rel, film.minimum_value); log3 << log3.T;
	log3.T.Format("dt=%s iters=%d func_calls=%d", ConvTimeToStr(film.dt), film.cntr.iter, film.cntr.func_call); log3 << log3.T;
	for( int i = 0; i < film.n_teor.GetSize(); i++)
	{
		log3.T.Format("betta_teor=%.10f betta_exp=%.10f", film.n_teor[i], film.n_exp[i]); log3 << log3.T;
	}	
	if (film.status != GSL_SUCCESS) log3.SetPriority(lmprHIGH);	

	double N_fval = 0.;
	for( int i = 0; i < film.n_teor.GetSize(); i++)
	{
		double Nteor = cal.ConvertBettaToPixel(film.n_teor[i]);
		log3.T.Format("Nexp=%.10f Nteor=%.10f", Nexp[i], Nteor); log3 << log3.T;
		N_fval += fabs(Nexp[i] - Nteor);
	}	
	log3.T.Format("N_fval=%.10f", N_fval); log3 << log3.T;

	log3.Dispatch();
}

void DialogBarTab1::OnBnClickedCalibrate()
{
#ifdef DEBUG
	DoubleArray Nexp_TE, Nexp_TM, n_exp_TE, n_exp_TM; 
	CalcRParams params;
	params.i=FilmParams(1,			150,	0+1e-100); 
	params.f=FilmParams(1.84,		1082,	5e-3+1e-100);  
	params.s=FilmParams(1.45705,	1e6,	0+1e-100); 
	params.lambda=632.8; params.Np=2.15675; params.teta_min=15; params.teta_max=85;

	Nexp_TE << 2995.4 << 2159.1 << 1550.8 << 0833.1; 
	n_exp_TE << 1.94514 << 1.85266 << 1.72802 << 1.55678;
	Nexp_TM << 2995.4 << 2159.1 << 1550.8 << 0833.1; ; 
	n_exp_TM << 1.94906 << 1.84743 << 1.70400 << 1.51424;
	
	MyGSL_Tester_Helper(TE, Nexp_TE, n_exp_TE, 1);
	MyGSL_Tester_Helper(TM, Nexp_TM, n_exp_TM, 1);	
#endif
	CalibratorDlg.ShowWindow(SW_SHOW);
}

void DialogBarTab1::OnBnClickedCalcFilm()
{
	CalcTEDlg.ShowWindow(SW_SHOW);
}

BOOL DialogBarTab1::DestroyWindow()
{
	CalibratorDlg.DestroyWindow();
	CalcTEDlg.DestroyWindow();
	return BarTemplate::DestroyWindow();
}
 
//TE
//	nf=    2.012999999999971      Hf=    1083.000000000000      DETabs= 4.2799999999971750E-003
//	neffexp(0)=    0.000000000000000      neffthr(0)=    0.000000000000000
//	neffexp(1)=    1.945140000000000      neffthr(1)=    1.944499999999973
//	neffexp(2)=    1.852660000000000      neffthr(2)=    1.855999999999983
//	neffexp(3)=    1.728020000000000      neffthr(3)=    1.727999999999997
//	neffexp(4)=    1.556780000000000      neffthr(4)=    1.556500000000016
//	neffexp(5)=    0.000000000000000      neffthr(5)=    0.000000000000000
//	neffexp(6)=    0.000000000000000      neffthr(6)=    0.000000000000000
//	neffexp(7)=    0.000000000000000      neffthr(7)=    0.000000000000000
//TM
//	nf=    2.028499999999976      Hf=    1074.000000000000      DETabs= 4.3699999999908812E-003
//	neffexp(0)=    0.000000000000000      neffthr(0)=    0.000000000000000
//	neffexp(1)=    1.949060000000000      neffthr(1)=    1.948999999999973
//	neffexp(2)=    1.847430000000000      neffthr(2)=    1.847499999999984
//	neffexp(3)=    1.704000000000000      neffthr(3)=    1.700000000000000
//	neffexp(4)=    1.514240000000000      neffthr(4)=    1.514000000000020
//	neffexp(5)=    0.000000000000000      neffthr(5)=    0.000000000000000
//	neffexp(6)=    0.000000000000000      neffthr(6)=    0.000000000000000
//	neffexp(7)=    0.000000000000000      neffthr(7)=    0.000000000000000