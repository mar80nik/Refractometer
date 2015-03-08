// MyTime.cpp: implementation of the MyTime class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyTime.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MyTime::MyTime()
{
hour=min=Sec=msec=0;
}

MyTime::~MyTime()
{

}

void MyTime::Init(double _sec)
{
	hour=(int)(_sec/3600.); min=(int)((_sec-hour*3600)/60);
	Sec=(int)(_sec-hour*3600-min*60); msec=(int)((_sec-hour*3600-min*60-Sec)*1000);
}

CString MyTime::GetString()
{
	CString T, Out;
	if(hour)
	{
		T.Format("%d h ",hour); Out+=T;
	}
	if(min)
	{
		T.Format("%d m ",min); Out+=T;
	}
	if(Sec)
	{
		T.Format("%d s",Sec); Out+=T;
	}
	if(!hour && !min && !Sec)
	{
		T.Format("%d ms",msec); Out+=T;
	}
return Out;
}

CString MyTime::ConvertToStr(sec _time, int precision)
{
	int hour,min,Sec,msec; double t; double time=_time.val();

	modf(time/3600., &t); 
	time=fmod(time,3600.); 
	hour=(int)t;
	modf(time/60., &t); 
	time=fmod(time,60.);
	min=(int)t;
	msec=(int)(1e3*modf(time,&t)); 
	Sec=(int)t;
		
	CString T, Out;
	if(hour)
	{
		T.Format("%d h ",hour); Out+=T;
	}
	if(min)
	{
		T.Format("%d m ",min); Out+=T;
	}
	if(Sec)
	{
		T.Format("%d s ",Sec); Out+=T;
	}
	if(precision)
	{
		T.Format("%d ms",msec); Out+=T;		
	}
	else
	{		
		if(!hour && !min && !Sec) {T.Format("%d ms",msec); Out+=T;}
	}
	return Out;
}

us100 us100::operator*(double _l) {us100 ret; ret.v=MyRound(v*_l); return ret;};
us10 us10::operator*(double _l) {us10 ret; ret.v=MyRound(v*_l); return ret;};
us us::operator*(double _l) {us ret; ret.v=v*_l; return ret;};

MyTimer::MyTimer() {QueryPerformanceFrequency(&start);freq=(double)start.u.LowPart;}
void MyTimer::Start() {QueryPerformanceCounter(&start);stop=start;}
void MyTimer::Stop() {QueryPerformanceCounter(&stop);}

sec MyTimer::GetValue()
    {
    if(stop.u.HighPart!=start.u.HighPart) 
		return sec(-1.);
    return sec((stop.u.LowPart-start.u.LowPart)/freq);
    }
CString MyTimer::GetString(double dim)
    {
    CString output;
    if(stop.u.HighPart!=start.u.HighPart) output="OverFlow";
    int i=(int)(((stop.u.LowPart-start.u.LowPart)/freq)/dim);
	output.Format("%d s",i);
    return output;
    }

CString MyTimer::GetStringF(double dim)
    {
    CString output; 
    if(stop.u.HighPart!=start.u.HighPart) output="OverFlow";
    return ConvTimeToStr(GetValue());
    }

sec MyTimer::StopStart()
{
	Stop();
	sec ret=GetValue();
	Start();
	return ret;
}

CString ConvTimeToStr(sec _time)
{
	int h, m, s, ms, us; double t = _time.val(), t2;

	t = 3600*modf(t/3600., &t2);	h = (int)t2;
	t = 60*modf(t/60., &t2);		m = (int)t2;	
	t = modf(t, &t2);				s = (int)t2;
	t = modf(t*1e3, &t2);		    ms = (int)t2;
	t = modf(t*1e3, &t2);			us = (int)t2; 
	
	CString T, Out;
	if(h)	{T.Format("%d h ",h); Out+=T;}
	if(m)	{T.Format("%d m ",m); Out+=T;}
	if(s)	{T.Format("%d s ",s); Out+=T;}	
	if(ms)	{T.Format("%d ms ",ms); Out+=T;}	
	if(us)	{T.Format("%d us ",us); Out+=T;}		
	return Out;
}

Delayer::Delayer(sec _CTime) 
{	
	Name="unknown";
	CorrectionTime=_CTime;
}

void Delayer::Start(sec time, StopState* stop)
{
sec t1;NumberOfTimes=0;
time-=CorrectionTime;
T1.Start(); T2.Start();
    while(t1<time && !(*stop))
    {
		{
			for(i=0;i<200 && !(*stop);i++) {double r=2.31*5.78; r*=2;}
		}
		T1.Stop(); t1=T1.GetValue();
		NumberOfTimes++;
    }	
	T2.Stop();
}


void Delayer::Start(sec time)
{
sec t1;NumberOfTimes=0;
time-=CorrectionTime;
T1.Start(); T2.Start();
    while(t1<time)
    {
		for(i=0;i<100;i++) {double r=2.31*5.78; r*=2;}
		T1.Stop();
		t1=T1.GetValue();
		NumberOfTimes++;
    }
	
	T2.Stop();    
	t1=T2.GetValue();
}

