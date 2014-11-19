// CalcTEDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CalcTEDialog.h"


// CalcTEDialog dialog

IMPLEMENT_DYNAMIC(CalcTEDialog, CDialog)

CalcTEDialog::CalcTEDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CalcTEDialog::IDD, pParent)
	, nf(0), hf(0), pol(TE)
{
	for(int i=0;i<modes_num;i++) { N[i] = 0; Q[i] = 0;}
	Series=NULL; 
#if defined DEBUG
	N[3]=3077; N[2]=2594; N[1]=1951; N[0]=1161;
#endif
	lambda = 632.8; n3 = 1.45705;
}

CalcTEDialog::~CalcTEDialog()
{
}

void CalcTEDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, SeriesCombo);
	DDX_Text(pDX, IDC_EDIT1, N[0]);
	DDX_Text(pDX, IDC_EDIT10, N[1]);
	DDX_Text(pDX, IDC_EDIT2, N[2]);
	DDX_Text(pDX, IDC_EDIT6, N[3]);
	DDX_Text(pDX, IDC_EDIT5, Q[0]);
	DDX_Text(pDX, IDC_EDIT13, Q[1]);
	DDX_Text(pDX, IDC_EDIT8, Q[2]);
	DDX_Text(pDX, IDC_EDIT16, Q[3]);
	DDX_Text(pDX, IDC_EDIT14, nf);
	DDX_Text(pDX, IDC_EDIT17, hf);
	DDX_Text(pDX, IDC_EDIT18, lambda);
	DDX_Text(pDX, IDC_EDIT19, n3);
	DDX_Radio(pDX, IDC_RADIO1, (int&)pol);
	DDX_Text(pDX, IDC_EDIT21, n_p);
}


BEGIN_MESSAGE_MAP(CalcTEDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON4, &CalcTEDialog::OnBnClickedConvertToAngles)
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	ON_CBN_SELCHANGE(IDC_COMBO1, &CalcTEDialog::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON11, &CalcTEDialog::OnBnClickedCalculate)
END_MESSAGE_MAP()


// CalcTEDialog message handlers

void CalcTEDialog::OnBnClickedConvertToAngles()
{
	CalibrationParams cal; UpdateData();
	MainCfg.GetCalibration(&cal); cal.val[CalibrationParams::ind_n_p] = n_p; teta_exp.RemoveAll(); 
	for(int i = 0, j = 0; i < modes_num; i++)
	{	
		AngleFromCalibration angle = cal.ConvertPixelToAngle(N[i]);
		if (SUCCEEDED(angle.status))
		{
			teta_exp << angle;
			Q[j++] = teta_exp[i].teta/DEGREE;
		}
	}
	if (teta_exp.GetSize() != modes_num)
	{
		ControledLogMessage log(::lmprHIGH);
		log.T.Format("Wrong number of converted angles because of wrong calibrations"); log << log.T;
		log.Dispatch();
	}
	UpdateData(0);
}

LRESULT CalcTEDialog::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{
	CString T; int n;
	if(lParam!=0) 
	{
		Series=(ProtectedSeriesArray*)lParam;
		SeriesCombo.ResetContent();
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
					n=SeriesCombo.AddString(T);
					SeriesCombo.SetItemData(n,(DWORD)results.Series[i]);
				}				
			}
		}
	}
	return 0;
}

void CalcTEDialog::OnCbnSelchangeCombo1()
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

void CalcTEDialog::OnBnClickedCalculate()
{
	CString T; LogMessage *log=new LogMessage(); FilmParams film;
	UpdateData();
	if (teta_exp.GetSize() != modes_num)
	{
		ControledLogMessage log(::lmprHIGH);
		log.T.Format("Film params could not be calculated because of wrong number of converted angles"); log << log.T;
		log.Dispatch();
		return;
	}
	if (pol == TM)
	{
		T.Format("--FilmParamsTM---"); 
	} 
	else
	{
		T.Format("--FilmParamsTE---");
	}
	log->CreateEntry("*",T);

	CalibrationParams cal; MainCfg.GetCalibration(&cal);
	cal.val[CalibrationParams::ind_lambda] = lambda;

	if(film.Calculator(pol, cal, teta_exp) == GSL_SUCCESS)
	{
		nf = film.n; hf = film.H;
		UpdateData(0);
		log->CreateEntry(CString('*'),T);
	}
	else log->CreateEntry(CString('*'),T,LogMessage::high_pr);
	
	T.Format("status = %s", gsl_strerror (film.status)); 
	T.Format("n=%.10f H=%.10f nm",film.n, film.H); log->CreateEntry("*",T);
	T.Format("errabs=%g errrel=%g fval=%.10f",film.err.abs, film.err.rel, film.minimum_value); log->CreateEntry("*",T);
	T.Format("dt=%.3f ms func_calls=%d",film.dt.val(), film.cntr.func_call); log->CreateEntry("*",T);

	for(int i = 0; i < film.betta_teor.GetSize(); i++)
	{
		T.Format("betta_teor[%d]=%.5f betta_exp=%.5f",
			film.betta_teor[i].n, film.betta_teor[i].val, film.betta_exp[i]); 
		log->CreateEntry("*",T);
	}		
	log->Dispatch();
}
