// CalcTEDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CalcTEDialog.h"
#define EMPTY_VAL _T("*****")

// CalcTEDialog dialog

IMPLEMENT_DYNAMIC(CalcTEDialog, CDialog)

CalcTEDialog::CalcTEDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CalcTEDialog::IDD, pParent)
	, nf(""), hf(""), pol(TE)
{
	int i;
	for(i = 0;i<max_modes;i++) { N[i] = 0; n[i] = 0;}
	Series=NULL; 
#if defined DEBUG

	N[3]=0833.1; N[2]=1550.8; N[1]=2159.1; N[0]=2995.4;
	for (i = 0; i < 4; i++)
	{
		Txts[i].Format("%g", N[i]);
	}
	for (; i < max_modes; i++)
	{
		Txts[i] = EMPTY_VAL;
	}
#endif
	lambda = 632.8; n_s = 1.45705; n_p = 2.15675; n_i = 1;
	QuickSearch = FALSE;
}

CalcTEDialog::~CalcTEDialog()
{
}

void CalcTEDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, SeriesCombo);
	DDX_Control(pDX, IDC_BUTTON11, CalculateBtn);
	DDX_Text(pDX, IDC_EDIT1,	Txts[0]);
	DDX_Text(pDX, IDC_EDIT10,	Txts[1]);
	DDX_Text(pDX, IDC_EDIT2,	Txts[2]);
	DDX_Text(pDX, IDC_EDIT6,	Txts[3]);
	DDX_Text(pDX, IDC_EDIT20,	Txts[4]);
	DDX_Text(pDX, IDC_EDIT22,	Txts[5]);

	DDX_Text(pDX, IDC_EDIT5,	txts[0]);
	DDX_Text(pDX, IDC_EDIT13,	txts[1]);
	DDX_Text(pDX, IDC_EDIT8,	txts[2]);
	DDX_Text(pDX, IDC_EDIT16,	txts[3]);
	DDX_Text(pDX, IDC_EDIT9,	txts[4]);
	DDX_Text(pDX, IDC_EDIT24,	txts[5]);

	DDX_Text(pDX, IDC_EDIT14, nf);
	DDX_Text(pDX, IDC_EDIT17, hf);
	DDX_Text(pDX, IDC_EDIT25, delta);
	DDX_Text(pDX, IDC_EDIT18, lambda);
	DDX_Text(pDX, IDC_EDIT19, n_s);
	DDX_Radio(pDX, IDC_RADIO1, (int&)pol);
	DDX_Text(pDX, IDC_EDIT21, n_p);
	DDX_Text(pDX, IDC_EDIT23, n_i);
	DDX_Check(pDX, IDC_CHECK1, QuickSearch);
}


BEGIN_MESSAGE_MAP(CalcTEDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON4, &CalcTEDialog::OnBnClickedConvertToAngles)
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	ON_CBN_SELCHANGE(IDC_COMBO1, &CalcTEDialog::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON11, &CalcTEDialog::OnBnClickedCalculate)
	ON_BN_CLICKED(IDC_BUTTON10, &CalcTEDialog::OnBnClickedUp)
	ON_BN_CLICKED(IDC_BUTTON18, &CalcTEDialog::OnBnClickedDn)
	ON_BN_CLICKED(IDC_RADIO1, &CalcTEDialog::OnBnClickedTE)
	ON_BN_CLICKED(IDC_RADIO2, &CalcTEDialog::OnBnClickedTE)
END_MESSAGE_MAP()


// CalcTEDialog message handlers

void CalcTEDialog::OnBnClickedConvertToAngles()
{
	CalibrationParams cal; 

	CheckValues(); MainCfg.GetCalibration(&cal); 
	if (cal.IsValidCalibration() == FALSE)
	{
		ControledLogMessage log(::lmprHIGH);
		log.T.Format("Pixels could not be converted because of wrong calibrations"); log << log.T;
		log.Dispatch();
		return;
	}
	n_exp.RemoveAll(); shift = 0; BOOL values_found = FALSE, error_found = FALSE;
	cal.val[CalibrationParams::ind_n_p] = n_p; 
	for(int i = 0, j = 0; i < max_modes; i++)
	{	
		if (error_found == FALSE && Txts[i] != EMPTY_VAL)
		{
			double N = atof(Txts[i]);
			AngleFromCalibration angle = cal.ConvertPixelToAngle(N);
			if (SUCCEEDED(angle.status))
			{
				double n = cal.ConertAngleToBeta(angle.teta);
				n_exp << n; 
				txts[i].Format("%.10f", n);
				values_found = TRUE;
			}
		}
		else 
		{
			CString T = Txts[i];
			if (error_found == FALSE)
			{
				if (values_found == FALSE) 
					shift++;
				else 
					error_found = TRUE;
			}
			if (error_found == TRUE)
				T = EMPTY_VAL;
			txts[i] = T;
		}
	}
	if (n_exp.GetSize() == 0)
	{
		ControledLogMessage log(::lmprHIGH);
		log.T.Format("Wrong number of converted pixels"); log << log.T;
		log.Dispatch();
	}
	else CalculateBtn.EnableWindow(TRUE);	
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
			N[max_modes-i-1]=t;			
		}
		for(;i<4;i++) N[max_modes-i-1]=0;
		UpdateData(0);
	}
}

void CalcTEDialog::OnBnClickedCalculate()
{
	CString T; LogMessage *log=new LogMessage(); FilmParams film;
	UpdateData();

	CalibrationParams cal; MainCfg.GetCalibration(&cal);
	if (cal.IsValidCalibration() == FALSE)
	{
		ControledLogMessage log(::lmprHIGH);
		log.T.Format("Film params could not be converted because of wrong calibrations"); log << log.T;
		log.Dispatch();
		return;
	}
	cal.val[CalibrationParams::ind_n_p] = n_p; 
	cal.val[CalibrationParams::ind_n_i] = n_i; 
	cal.val[CalibrationParams::ind_n_s] = n_s; 
	cal.val[CalibrationParams::ind_lambda] = lambda; 

	if (n_exp.GetSize() == 0)
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
	
	DoubleArray range_min, range_max, dd;
	range_min << n_exp[0] << 200; 
	range_max << 2.1;
	if (QuickSearch == TRUE) 
		range_max << 10000;
	else 
		range_max << 2000;
	
	dd << 1e-3 << 18;

	// test film parameters calculation
	ControledLogMessage log3;
	
	if(film.Calculator2(pol, cal, n_exp, shift, range_min, range_max, dd) == GSL_SUCCESS)
	{
		nf.Format("%1.4f", film.n); 
		hf.Format("%.1f", film.H); 
		delta = film.minimum_value;
		UpdateData(0);
		log->CreateEntry(CString('*'),T);
	}
	else log->CreateEntry(CString('*'),T,LogMessage::high_pr);
	
	T.Format("status = %s", gsl_strerror (film.status)); 
	T.Format("n=%.10f H=%.10f nm",film.n, film.H); log->CreateEntry("*",T);
	T.Format("errabs=%g errrel=%g fval=%.10f",film.err.abs, film.err.rel, film.minimum_value); log->CreateEntry("*",T);
	T.Format("dt=%.3f ms func_calls=%d itters=%d",film.dt.val(), film.cntr.func_call, film.cntr.iter); log->CreateEntry("*",T);

	for(int i = 0; i < film.n_teor.GetSize(); i++)
	{
		T.Format("betta_teor=%.10f betta_exp=%.10f", film.n_teor[i], film.n_exp[i]); 
		log->CreateEntry("*",T);
	}		
	log->Dispatch();
}

void CalcTEDialog::OnBnClickedUp()
{
	CheckValues();
	int i = 0; CString T = Txts[i];
	while (i < max_modes - 1)
	{
		Txts[i] = Txts[i + 1]; i++;
	}
	Txts[i] = T;
	UpdateData(0);
}

void CalcTEDialog::OnBnClickedDn()
{
	CheckValues();
	int i = max_modes - 1; CString T = Txts[i];
	while (i > 0) 
	{
		Txts[i] = Txts[i - 1]; i--;
	}
	Txts[i] = T;
	CalculateBtn.EnableWindow(FALSE);
	UpdateData(0);
}

void CalcTEDialog::CheckValues()
{
	int i; double t;
	UpdateData();
	for (i = 0; i < max_modes; i++)
	{
		if ((t = atof(Txts[i])) != 0.0) 
			Txts[i].Format("%g", t);
		else 
			Txts[i] = EMPTY_VAL;
	}
	CalculateBtn.EnableWindow(FALSE);
	UpdateData(0);
}

void CalcTEDialog::OnBnClickedTE()
{
#ifdef DEBUG
#endif // DEBUG
}


void CalcTEDialog::OnBnClickedTM()
{
#ifdef DEBUG
#endif // DEBUG
}