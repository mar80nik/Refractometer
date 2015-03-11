#ifndef GlobalHeaderH
#define GlobalHeaderH

#include <afxmt.h>
#include <afxtempl.h>
#include <math.h>
#include "externals.h"
#include "my_color.h"
#include "bmpanvas.h"
#include "MessageInspector.h"
#include "MyTime.h"

enum ThreadsID
{	
	MainTaskID =101, MainAppID,
	MVThreadID=201, MVThreadID1, MVThreadID2,MVThreadID3,
	AssistntID, PGASET_Assistnt, DRIVER_START_Assist,
	TThreadID=301, TThreadID1, TThreadID2,
	OscilThreadID=401, ChipID=501, ChipDrvID=601,
	UART_ReciverID=701, PGASET_AssistantID=801

};

enum UserDefinedMessages
{
	UM_SENSOR_PREPARE=WM_USER + 100, UM_SENSOR_OK,	UM_UPDATE_INDICATORS, UM_WAVELEN_SET, 
	UM_COMUTATOR, UM_STATUS_UPDATE,	UM_STAGE_UPDATE, UM_SENSOR_UPDATE, UM_OSCIL, UM_PGA_SET,
	UM_BACKUP_SAVE, UM_UPDATE_CONFIG, UM_ADD, UM_DATA_UPDATE, UM_ACCESS_REQUEST, 
	UM_ACCESS_GRANTED, UM_ACCESS_DEINIED, UM_SERIES_PNTR, 
	UM_LAST_MESSAGE
};

enum UserDefinedMessages1
{	
	UM_MESSAGE=UM_LAST_MESSAGE+1, UM_MSG_OUT, UM_NOTIFY, UM_LAST_MESSAGE1
};

//enum UserDefinedMessages2
//{	
//	UM_SERIES_UPDATE=UM_LAST_MESSAGE1+1, UM_LAST_MESSAGE2
//};


class Error;

struct IndicatorsText {CString device, running, paused;};

union FloatInt
{
	float flt;
	int Int;
};

int MyRound(double x);

void CreateBufferTemplate(BMPINFO& INFO);

class LoopArrayElement 
{	
public:	
	LoopArrayElement *prev,*next; 

	int PID;
	LoopArrayElement() {prev=next=NULL;};
	virtual ~LoopArrayElement() {};
};

typedef CArray<LoopArrayElement*,LoopArrayElement*> LAEArray;

template <class T>
class LoopArray: public LAEArray
{
public:
	T* GetNextBuf(T* _elmnt)
	{
		if(_elmnt==NULL) return NULL;
		return (T*)(_elmnt->next);
	}
	int Add(T* _elmnt)
	{
		int ret;
		ret=LAEArray::Add(_elmnt); 
		_elmnt->PID=ret;
		if(ret>0)
		{
			_elmnt->prev=(*this)[ret-1];
			_elmnt->next=(*this)[0];	
			_elmnt->prev->next=_elmnt->next->prev=_elmnt;
		}
		else
		{
			_elmnt->next=_elmnt->prev=_elmnt;	
		}
		return ret;    
	}
	void RemoveAll()
	{		
		for(int i=0;i<GetSize();i++)
		{
			delete (*this)[i];
		}
		LAEArray::RemoveAll();	
	}
	~LoopArray()
	{
		RemoveAll();
	}
	T* operator[](int i)
	{
		return 	(T*)(LAEArray::operator [](i));
	}
};

enum ParamsStatus {PRM_UNKNOWN, PRM_ERR, PRM_OK};

struct AbstractSetupParams
{
protected:
	ParamsStatus status;
public:
	BYTE MsgMode;
	AbstractSetupParams() { status=PRM_UNKNOWN;}
	ParamsStatus GetStatus() {return status;}	
	void SetStatus(ParamsStatus _p) {status=_p;}	
};

#endif

