#pragma once

#include "mythread.h"
#include "metricon.h"

class SystemConfig; class AbstractDevice;
enum {DEV_ERR=0, CFG_PARAM_ERR=0};

union TreeParamData
{
	struct {WORD dev,param;};
	DWORD data;
	explicit TreeParamData(WORD _dev, WORD _param) {dev=_dev; param=_param;}
	explicit TreeParamData(DWORD t) {data=t;}
	operator DWORD() {return data;}
};

struct CfgParamTypeBase
{
	int pid; bool IsEditable;
	CString Name, Units;
	AbstractDevice* Parent;
	HTREEITEM TreeItem;

	CfgParamTypeBase() {pid=-1; IsEditable=false; Name="NoName"; Units="NoUnits";TreeItem=0;}
	virtual void GetParamValue(CString& t)=0;
	virtual void FormatParam(CString& t)=0;
	virtual void SetParamValue(CString& t)=0;
	virtual void GetParamTitle(CString& t) {t=(Units=="" ? Name: Name+" ["+Units+"]");};
	virtual void Serialize(CArchive& ar)=0;
	virtual void UpdateParent();
	virtual void FillCfgTree(CTreeCtrl* t,HTREEITEM l0);
};

template<typename T>
class CfgParamType: public CfgParamTypeBase
{
protected:
	T val;
public:
	CfgParamType(CString n, CString u, T InitValue) {Name=n; Units=u; val=InitValue;} 
	operator T() {return val;};
	CfgParamType& operator=(T t) {val=t; return *this;};
	virtual void GetParamValue(CString& t);
	virtual void FormatParam(CString& t)
	{
		CString T1; t=""; GetParamTitle(T1); t+=T1+" = "; GetParamValue(T1); t+=T1;
	}
	virtual void SetParamValue(CString& t);	
	virtual void SetParamValue(T t) {val=t;UpdateParent();}	
	virtual void Serialize(CArchive& ar) {if (ar.IsStoring()) ar << val; else ar >> val;};
};

class CfgCalibrationParams: public CfgParamTypeBase
{
protected:
	CalibrationParams val;
public:
	CfgCalibrationParams() {} 
	virtual void GetParamValue(CString& t) {};
	virtual void FormatParam(CString& t) {};
	virtual void SetParamValue(CString& t) {};
	virtual void GetParamTitle(CString& t) {};
	virtual void Serialize(CArchive& ar);
	virtual void FillCfgTree(CTreeCtrl* t,HTREEITEM l0);
};

typedef CArray<CfgParamTypeBase*> ParamsArray;

typedef CfgParamType<bool>		YesNoCfgParam;
typedef CfgParamType<BYTE>		ByteCfgParam;
typedef CfgParamType<int>		IntCfgParam;
typedef CfgParamType<double>	DblCfgParam;
typedef CfgParamType<CString>	StrCfgParam;

struct AngleCfgParam: public DblCfgParam
{
	AngleCfgParam(CString n, CString u, double InitValue): DblCfgParam(n, u, InitValue) {}
	virtual void GetParamValue(CString& t);
	virtual void SetParamValue(CString& t);
	virtual void SetParamValue(double t);
};


typedef TreeParamData TPD;

class SystemConfig;

class AbstractDevice: public ErrorInspector
{
	friend class TConfigDialog;
	friend class SystemConfig;
	friend struct CfgParamTypeBase;
protected:
	WORD id;
    CString Name;
	SystemConfig* Parent;
	ParamsArray Params;

	virtual void ParamUpdate(CfgParamTypeBase* t);
	int RegisterParam(CfgParamTypeBase* t);
public:
	AbstractDevice(void) {Name="NoName"; id=DEV_ERR;}
	virtual void Serialize(CArchive& ar);
	virtual int FormatParam(int id,CString& T);
	virtual void FillCfgTree(CTreeCtrl* t,HTREEITEM l0);
	void FillCfgTreeSingleParam(CTreeCtrl* t,HTREEITEM l0, CfgParamTypeBase* t2);
};

typedef CArray<AbstractDevice*> DevicesArray;

#define CALIBRATION_MODES_NUM 4
class CalibrationData: public AbstractDevice
{
	friend class SystemConfig;
protected:	
	DblCfgParam d,N,L,n_p; AngleCfgParam fi, Q;
	DblCfgParam n_i,n_s; 
	DblCfgParam* N_exp[CALIBRATION_MODES_NUM]; DblCfgParam *n[CALIBRATION_MODES_NUM];
	
	CalibrationParams cal;
public:
	CalibrationData(void);
	virtual ~CalibrationData();
	virtual void FillCfgTree(CTreeCtrl* t,HTREEITEM l0);
	virtual void Serialize(CArchive& ar);
	void SetCalibration(CalibrationParams& cal);
};

class ByteArray: public CByteArray
{
public:
	ByteArray& operator << (BYTE t) {Add(t); return *this;};
	ByteArray() {};
};
