#pragma once
#include "afxwin.h"
#include "resource.h"
#include "TChartSeries.h"
#include "metricon.h"
#include "SystemConfig.h"
#include <math.h>

#define max_modes 6

// CalcTEDialog dialog

//enum PolarizationModes {TE, TM};

class CalcTEDialog : public CDialog
{
	DECLARE_DYNAMIC(CalcTEDialog)

public:
	double N[max_modes]; DoubleArray n_exp;
	double n[max_modes];

	CString Txts[max_modes];
	CString txts[max_modes];
	
	CalcTEDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CalcTEDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_CALCTE };

protected:
	ProtectedSeriesArray* Series;
	Polarization pol;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void CheckValues();

	DECLARE_MESSAGE_MAP()
	LRESULT OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
public:
	afx_msg void OnBnClickedConvertToAngles();
	afx_msg void OnCbnSelchangeCombo1();
public:
	CComboBox SeriesCombo;
	CButton CalculateBtn;
	CString nf, hf; double delta, lambda, n_s, n_p, n_i;
	int shift; int QuickSearch;
public:
	afx_msg void OnBnClickedCalculate();
	afx_msg void OnBnClickedUp();
	afx_msg void OnBnClickedDn();
	afx_msg void OnBnClickedTE();
	afx_msg void OnBnClickedTM();
};
