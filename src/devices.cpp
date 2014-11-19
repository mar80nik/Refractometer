#include "stdafx.h"
#include "devices.h"
#include "systemconfig.h"
#include "resource.h"

void CfgParamTypeBase::UpdateParent() {if(Parent!=NULL) Parent->ParamUpdate(this);}

void CfgParamTypeBase::FillCfgTree( CTreeCtrl* t,HTREEITEM l0 )
{
	CString T;
	FormatParam(T);
	ASSERT(Parent);
	TreeItem=t->InsertItem(T,l0); TPD r(Parent->id,(WORD)pid);
	t->SetItemData(TreeItem,r);
}

void YesNoCfgParam::GetParamValue(CString& t)	{t=(val ? "YES":"NO");}
void DblCfgParam::GetParamValue(CString& t)		{t.Format("%g",val);}
void AngleCfgParam::GetParamValue( CString& t )	
{
	t.Format("%g", val/DEGREE);
}
void IntCfgParam::GetParamValue(CString& t)		{t.Format("%d",val);}
void ByteCfgParam::GetParamValue(CString& t)	{t.Format("%d",val);}
void StrCfgParam::GetParamValue(CString& t)		{t=val;}

void YesNoCfgParam::SetParamValue(CString& t)	{if(IsEditable) {val=(t=="YES" ? true:false);UpdateParent();}}
void DblCfgParam::SetParamValue(CString& t)		{if(IsEditable) {val=atof(t);UpdateParent();}}
void AngleCfgParam::SetParamValue( CString& t )	{if(IsEditable) {val=atof(t)*DEGREE; UpdateParent();}}
void IntCfgParam::SetParamValue(CString& t)		{if(IsEditable) {val=atoi(t);UpdateParent();}}
void ByteCfgParam::SetParamValue(CString& t)	{if(IsEditable) {val=(BYTE)atoi(t);UpdateParent();}}
void StrCfgParam::SetParamValue(CString& t)		{if(IsEditable) {val=t;UpdateParent();}}

void AngleCfgParam::SetParamValue(double t) {val = t; UpdateParent();}	

void AbstractDevice::FillCfgTree(CTreeCtrl* t,HTREEITEM l0)
{
	for(int i=0;i<Params.GetSize(); i++)
	{
		Params[i]->FillCfgTree(t,l0);
	}
}

void AbstractDevice::Serialize(CArchive& ar)
{
		if (ar.IsStoring()) ar << Name;
		else ar >> Name;
		for(int i=0;i<Params.GetSize();i++) Params[i]->Serialize(ar);
}	


int AbstractDevice::FormatParam(int id,CString& T) {return 0;}

int AbstractDevice::RegisterParam(CfgParamTypeBase* t) {int ret=Params.Add(t); t->pid=ret+1; t->Parent=this; return ret;}

void AbstractDevice::ParamUpdate( CfgParamTypeBase* t )
{
	 Parent->DeviceUpdate(this,t);
}


CalibrationData::CalibrationData( void ):
	fi("fi","град",0), d("d","отн.ед",0), L("L","отн.ед",0), N("N","отн.ед",0), Q("Q", "град", 0), n_p("n_p", "", 0)//,
{
	Name="Calibration";
	RegisterParam(&Q); RegisterParam(&n_p);
	RegisterParam(&fi); RegisterParam(&d); RegisterParam(&L); RegisterParam(&N);   

	CString T;
	for(int i=0;i<CALIBRATION_MODES_NUM; i++) 
	{ 
		T.Format("Nexp[%d]",i); N_exp[i] = new DblCfgParam(T,"пикс",0); RegisterParam(N_exp[i]);
		T.Format("teta[%d]",i); teta[i] = new AngleCfgParam(T,"град",0); RegisterParam(teta[i]);
	}
}

CalibrationData::~CalibrationData()
{
	for(int i=0;i<CALIBRATION_MODES_NUM; i++) 
	{
		delete N_exp[i]; delete teta[i];
	}
}

void CalibrationData::Serialize( CArchive& ar )
{	
	if(ar.IsStoring())
	{
		cal.Serialize(ar);
	}
	else
	{
		CalibrationParams _cal; 
		_cal.Serialize(ar);
		SetCalibration(_cal);
	}
}

void CalibrationData::FillCfgTree( CTreeCtrl* t,HTREEITEM l0 )
{
	int i, j = 0; HTREEITEM l1,l2;
	l1 = t->InsertItem("Prism",l0); 
	for(i = 0; i < Params.GetSize() && i < 2; i++)
	{
		Params[j++]->FillCfgTree(t, l1);
	}
	for(i = 0; i < Params.GetSize() && i < 4; i++)
	{
		Params[j++]->FillCfgTree(t, l0);
	}
	l1 = t->InsertItem("Nexp",l0); l2 = t->InsertItem("teta",l0); 
	for(;j < Params.GetSize();)
	{
		Params[j++]->FillCfgTree(t,l1);
		Params[j++]->FillCfgTree(t,l2);
	}
}

void CalibrationData::SetCalibration( CalibrationParams& _cal )
{
	int i = 0;
	cal=_cal;
	fi.SetParamValue(cal.val.get(CalibrationParams::ind_fi0)); 
	d.SetParamValue(cal.val.get(CalibrationParams::ind_d0)); 
	N.SetParamValue(cal.val.get(CalibrationParams::ind_N0)); 
	L.SetParamValue(cal.val.get(CalibrationParams::ind_L));
	Q.SetParamValue(cal.val.get(CalibrationParams::ind_alfa));
	n_p.SetParamValue(cal.val.get(CalibrationParams::ind_n_p));
	for(i = 0; i < cal.Nexp.GetSize(); i++)
	{
		N_exp[i]->SetParamValue(cal.Nexp[i]);
	}
	for(;i < CALIBRATION_MODES_NUM;i++)
	{
		N_exp[i]->SetParamValue(0);
	}
	
	for(i = 0; i < cal.teta.GetSize(); i++)
	{
		teta[i]->SetParamValue(cal.teta[i]);
	}
	for(; i < CALIBRATION_MODES_NUM; i++)
	{
		teta[i]->SetParamValue(0);
	}
}



