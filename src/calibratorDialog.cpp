#include "stdafx.h"
#include "calibratorDialog.h"
#include "ImageWnd.h"

// CalibratorDialog dialog

IMPLEMENT_DYNAMIC(CalibratorDialog, CDialog)

CalibratorDialog::CalibratorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CalibratorDialog::IDD, pParent)
{
	for(int i=0;i<calibrator_modes_num;i++) { N[i]=0; n[i]=0; }
	N0 = L = d0 = fi0 = alfa = n_p = 0.;
	Series=NULL; 
#if defined DEBUG
	N[0] = 2995.4; N[1] = 2159.1; N[2] = 1550.8; N[3] = 0833.1;
	n[0] = 1.94514; n[1] = 1.85266; n[2] = 1.72802; n[3] = 1.55678;
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
	DDX_Text(pDX, IDC_EDIT5, n[0]);
	DDX_Text(pDX, IDC_EDIT13, n[1]);
	DDX_Text(pDX, IDC_EDIT8, n[2]);
	DDX_Text(pDX, IDC_EDIT16, n[3]);
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
			N[calibrator_modes_num-i-1]=t;			
		}
		for(;i<4;i++) N[calibrator_modes_num-i-1]=0;
		UpdateData(0);
	}
}

void CalibratorDialog::OnCbnSelchangeCombo3()
{
	int j, i; double t;
	if( (j = SeriesCombo1.GetCurSel()) != CB_ERR)
	{
		TSimplePointSeries *seriea=(TSimplePointSeries *)SeriesCombo1.GetItemData(j);
		for(i = 0; i < seriea->GetSize() && i < 4; i++)
		{
			t=(*seriea)[i].x;
			t*=100; t=(int)t; t/=100;
			n[calibrator_modes_num-i-1]=t;			
		}
		for(;i<4;i++) n[calibrator_modes_num-i-1]=0;
		UpdateData(0);
	}
}


void CalibratorDialog::OnBnClickedCalculateCal()
{
	MyTimer Timer1; ms dt1, dt2;
	DoubleArray Nexp, nn; CString T;
	UpdateData();
	for(int i = 0; i < calibrator_modes_num; i++)
	{
		Nexp << N[i]; nn << n[i];
	}
	UpdateData();
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
	
	ControledLogMessage log;
	log.T.Format("Speed tests results"); log << log.T;

	cal.CalculateFrom(Nexp, nn, n_p, 1, 1.45705, alfa*DEGREE, 632.8);

	fi0 = cal.val[CalibrationParams::ind_fi0];
	N0 = cal.val[CalibrationParams::ind_N0]; L = cal.val[CalibrationParams::ind_L]; 
	d0 = cal.val[CalibrationParams::ind_d0]; 
	UpdateData(0);

	log.T.Format("****Statistics***"); log << log.T;
	if (cal.status != GSL_SUCCESS) log.SetPriority(lmprHIGH);
	log.T.Format("---Calibration---"); log << log.T;
	log.T.Format("Nexp=[%g %g %g %g]",cal.Nexp[0],cal.Nexp[1],cal.Nexp[2],cal.Nexp[3]); log << log.T;
	log.T.Format("n=[%g %g %g %g]",cal.n[0],cal.n[1],cal.n[2],cal.n[3]); log << log.T;
	log.T.Format("N0=%.10f", N0); log << log.T;
	log.T.Format("L=%.10f", L); log << log.T;
	log.T.Format("d0=%.10f", d0); log << log.T;
	log.T.Format("fi0=%.10f", fi0); log << log.T;
	log.T.Format("dt=%.3f ms func_calls=%d", cal.dt.val(), cal.cntr.func_call); log << log.T;
	log.Dispatch();		
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


