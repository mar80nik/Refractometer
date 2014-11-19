#include "stdafx.h"
#include "calibratorDialog.h"
#include "ImageWnd.h"

// CalibratorDialog dialog

IMPLEMENT_DYNAMIC(CalibratorDialog, CDialog)

CalibratorDialog::CalibratorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CalibratorDialog::IDD, pParent)
{
	for(int i=0;i<modes_num;i++) { N[i]=0; Q[i]=0; }
	N0 = L = d0 = fi0 = alfa = n_p = 0.;
	Series=NULL; 
#if defined DEBUG
	N[0]=3077; N[1]=2594; N[2]=1951; N[3]=1161;
	Q[0]=64.02; Q[1]=60.43; Q[2]=55.24; Q[3]=48.62;
	alfa = 51; n_p = 2.15675;
#endif
}

CalibratorDialog::~CalibratorDialog()
{
}

void CalibratorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, N[0]);
	DDX_Text(pDX, IDC_EDIT10, N[1]);
	DDX_Text(pDX, IDC_EDIT2, N[2]);
	DDX_Text(pDX, IDC_EDIT6, N[3]);
	DDX_Text(pDX, IDC_EDIT5, Q[0]);
	DDX_Text(pDX, IDC_EDIT13, Q[1]);
	DDX_Text(pDX, IDC_EDIT8, Q[2]);
	DDX_Text(pDX, IDC_EDIT16, Q[3]);
	DDX_Text(pDX, IDC_EDIT14, N0);
	DDX_Text(pDX, IDC_EDIT17, L);
	DDX_Text(pDX, IDC_EDIT18, d0);
	DDX_Text(pDX, IDC_EDIT19, fi0);
	DDX_Text(pDX, IDC_EDIT15, alfa);
	DDX_Text(pDX, IDC_EDIT20, n_p);
	DDX_Control(pDX, IDC_COMBO1, SeriesCombo);
	DDX_Control(pDX, IDC_COMBO3, SeriesCombo1);
}


BEGIN_MESSAGE_MAP(CalibratorDialog, CDialog)
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	ON_CBN_SELCHANGE(IDC_COMBO1, &CalibratorDialog::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON4, &CalibratorDialog::OnBnClickedCalculateCal)
	ON_BN_CLICKED(IDC_BUTTON10, &CalibratorDialog::OnBnClickedSaveToConfig)
	ON_BN_CLICKED(IDC_BUTTON15, &CalibratorDialog::OnBnClickedLoadFromConfig)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CalibratorDialog::OnCbnSelchangeCombo3)
END_MESSAGE_MAP()


// CalibratorDialog message handlers

LRESULT CalibratorDialog::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{
	CString T; int n;
	if(lParam!=0) 
	{
		Series=(ProtectedSeriesArray*)lParam;
		SeriesCombo.ResetContent(); SeriesCombo1.ResetContent();
		void *x; 
		if((x=Series->GainAcsess(READ))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);			

			SeriesSearchPattern pattern; 
			pattern.mode=SERIES_NAME | SERIES_TYPE; 
			pattern.name=CString("FinalMins"); pattern.SeriesType=SIMPLE_POINT;
			TSeriesArray results(DO_NOT_DELETE_SERIES); 
			if( Series.FindSeries(pattern,results)!=0 )
			{				
				for(int i=0;i<results.Series.GetSize();i++)
				{
					T.Format("%s (%d)",results.Series[i]->Name,results.Series[i]->GetSize());
					n=SeriesCombo.AddString(T); SeriesCombo.SetItemData(n,(DWORD)results.Series[i]);
					n=SeriesCombo1.AddString(T); SeriesCombo1.SetItemData(n,(DWORD)results.Series[i]);
				}				
			}
		}
	}
	return 0;
}
void CalibratorDialog::OnCbnSelchangeCombo1()
{
	int n, i; double t;
	if( (n=SeriesCombo.GetCurSel())!=CB_ERR)
	{
		TSimplePointSeries *seriea=(TSimplePointSeries *)SeriesCombo.GetItemData(n);
		for(i=0;i<seriea->GetSize() && i<4;i++)
		{
			t=(*seriea)[i].x;
			t*=100; t=(int)t; t/=100;
			N[modes_num-i-1]=t;			
		}
		for(;i<4;i++) N[modes_num-i-1]=0;
		UpdateData(0);
	}
}

void CalibratorDialog::OnCbnSelchangeCombo3()
{
	int n, i; double t;
	if( (n=SeriesCombo1.GetCurSel())!=CB_ERR)
	{
		TSimplePointSeries *seriea=(TSimplePointSeries *)SeriesCombo1.GetItemData(n);
		for(i=0;i<seriea->GetSize() && i<4;i++)
		{
			t=(*seriea)[i].x;
			t*=100; t=(int)t; t/=100;
			Q[modes_num-i-1]=t;			
		}
		for(;i<4;i++) Q[modes_num-i-1]=0;
		UpdateData(0);
	}
}


void CalibratorDialog::OnBnClickedCalculateCal()
{
	MyTimer Timer1; ms dt1,dt2; double t;
	DoubleArray Nexp,teta; CString T;
	UpdateData();
	for(int i = 0; i < modes_num; i++)
	{
		t=N[i]; Nexp.Add(N[i]); 
		t=Q[i]*DEGREE; teta.Add(t);
	}
	UpdateData();
	for(int i = 0; i < modes_num; i++)
	{
		t=N[i]; Nexp.Add(N[i]); 
		t=Q[i]*DEGREE; teta.Add(t);
	}
	//TChart *chart=(TChart*)&GlobalChart; 
	//ASSERT(chart != NULL);
	//if (chart == NULL) return;
	//
	//void *xxx; 	CString str; TSimplePointSeries* t1=NULL; SimplePoint pnt;
	//
	//if((xxx=chart->Series.GainAcsess(WRITE))!=NULL)
	//{
	//	SeriesProtector guard(xxx); TSeriesArray& series(guard); str.Format("PolyFit%d",1234);
	//	if((t1=new TSimplePointSeries(str))!=0)	
	//	{
	//		series.Add(t1); 
	//		t1->_SymbolStyle::Set(NO_SYMBOL);
	//		t1->AssignColors(ColorsStyle(clRED,series.GetRandomColor()));
	//		t1->SetVisible(true); 

	//		//CalculateCalibrationParams::FuncParams in_params(Nexp.GetSize(), Nexp, teta, cal.n_p, cal.n_s, cal.alfa );

	//		t1->ParentUpdate(UPD_OFF);
	//		for(double x=0; x<45.0; x+=0.1)
	//		{
	//			pnt.x=x;
	//			//pnt.y=Solver1dTemplate<CalculateCalibrationParams::FuncParams>::func(x*DEGREE,&in_params);
	//			t1->AddXY(pnt);
	//		}
	//		t1->ParentUpdate(UPD_ON);
	//	}	
	//}
	//chart->PostMessage(UM_CHART_SHOWALL);	
	
	LogMessage *log=new LogMessage(); log->CreateEntry("Log","Speed tests results",LogMessage::low_pr);

	cal.CalculateFrom(Nexp, teta, n_p, 1, 1.45705, alfa*DEGREE, 632.8);

	fi0 = cal.val[CalibrationParams::ind_fi0]/DEGREE;
	N0 = cal.val[CalibrationParams::ind_N0]; L = cal.val[CalibrationParams::ind_L]; d0 = cal.val[CalibrationParams::ind_d0]; 
	UpdateData(0);

	T.Format("****Statistics***"); *log << T;
	if (cal.status != GSL_SUCCESS) log->SetPriority(lmprHIGH);
	T.Format("---Calibration---"); *log << T;
	T.Format("Nexp=[%g %g %g %g]",cal.Nexp[0],cal.Nexp[1],cal.Nexp[2],cal.Nexp[3]); *log << T;
	T.Format("teta=[%g %g %g %g]",cal.teta[0],cal.teta[1],cal.teta[2],cal.teta[3]); *log << T;
	T.Format("N0=%.10f L=%.10f d0=%.10f fi0=%.10f errabs=%g errrel=%g", N0, L, d0, fi0, cal.err.abs, cal.err.rel); 
	*log << T;
	T.Format("dt=%.3f ms func_calls=%d", cal.dt.val(), cal.cntr.func_call); *log << T;
	log->Dispatch();		
}

void CalibratorDialog::OnBnClickedSaveToConfig()
{
	UpdateData(); 
	MainCfg.SetCalibration(cal);	
	MainFrame.pWND->PostMessage(UM_UPDATE_CONFIG,0,0);
}

void CalibratorDialog::OnBnClickedLoadFromConfig()
{
	MainCfg.GetCalibration(&cal); 
	UpdateData(FALSE);
}


