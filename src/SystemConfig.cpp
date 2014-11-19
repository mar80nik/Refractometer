#include "stdafx.h"
#include "SystemConfig.h"
#include "resource.h"
#include <math.h>

#define SpN (Coeff1*StepPerNanometer)


int SystemConfig::RegisterDevice(AbstractDevice* t) 
{
	int ret=Devices.Add(t); t->id=(WORD)(ret+1); t->Parent=this; 
	Name=CString("SystemConfig");
	return ret;
}

SystemConfig::SystemConfig()  
{
	WAIT_TIME=1000; 
	RegisterDevice(&cal);
}

SystemConfig::~SystemConfig() {}

void SystemConfig::LoadConfig()
{
	CFile ConfigFile; ErrorsArray err; AttachErrors(&err); 

	if(ConfigFile.Open("config.dat",CFile::modeRead))
	{
		CArchive ar(&ConfigFile,CArchive::load); 
		Serialize(ar); ar.Close();		
		ConfigFile.Close();
	}
	else 
	{
		Error* t=new Error(); RegisterError(t->Create(ERR_CONFIG_NOFILE,Name));
	}
	Parent->PostThreadMessage(UM_WAVELEN_SET,0,0);
	Parent->PostThreadMessage(UM_UPDATE_CONFIG,0,0);

	DispatchErrors();
	DetachErrors();
}

void SystemConfig::SaveConfig()
{
	CFile ConfigFile;
	if(ConfigFile.Open("config.dat",CFile::modeCreate | CFile::modeWrite ))
	{
		{CArchive ar(&ConfigFile,CArchive::store); Serialize(ar);}
		ConfigFile.Close(); 
	}	
}

int SystemConfig::Serialize(CArchive &ar)
{
	int ret=0;
	CSingleLock Lock1(&SystemConfigUpd); 
	if(Lock1.Lock(WAIT_TIME))
	{
		try
		{
		for(int i=0;i<Devices.GetSize();i++) Devices[i]->Serialize(ar);
		}
		catch(CArchiveException *ex)
		{
			ex=NULL;
			Error* t=new Error(); RegisterError(t->Create(ERR_CONFIG_ARCHERR,Name));
		}
		catch(CFileException *ex)
		{
			ex=NULL;
			Error* t=new Error(); RegisterError(t->Create(ERR_CONFIG_FILEERR,Name));
		}
		catch(CMemoryException *ex)
		{			
			ex=NULL;
			Error* t=new Error(); RegisterError(t->Create(ERR_CONFIG_MEMERR,Name));
		}
	}
	return ret;
}

void SystemConfig::Get(int mode, int& val)
{
	switch(mode)
	{
	default: ASSERT(0);
	}
	return;
}

void SystemConfig::Get(int mode, BYTE& val)
{
	switch(mode)
	{
	default: ASSERT(0);
	}
	return;
}


void SystemConfig::Get(int mode, double& val)
{
	switch(mode)
	{
	default: ASSERT(0);
	}
	return;
}

void SystemConfig::DeviceUpdate( AbstractDevice *d,CfgParamTypeBase* p )
{
	Parent->PostThreadMessage(UM_UPDATE_CONFIG,(WPARAM)d,LPARAM(p));	
}

void SystemConfig::SetCalibration( CalibrationParams &_cal )
{
	cal.SetCalibration(_cal);
}

void SystemConfig::GetCalibration(CalibrationParams* tt)
{
	*tt = cal.cal;
}
