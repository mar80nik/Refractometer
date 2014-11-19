#if !defined(AFX_MYTHREAD_H__192EEA8B_2C40_4C22_9F15_F8F602DC6038__INCLUDED_)
#define AFX_MYTHREAD_H__192EEA8B_2C40_4C22_9F15_F8F602DC6038__INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyThread.h : header file
/////////////////////////////////////////////////////////////////////////////
//CRuntimeClass* prt = RUNTIME_CLASS(NewThread);
//NewThreadPointer=(NewThread*)AfxBeginThread(prt,THREAD_PRIORITY_NORMAL,0,0);	
//NewThreadPointer->Create(ParentThreadPointer,NewThreadID, ExternTerminateEventPointer) 
//...
//...
//NewThreadPointer->TerminateThread();
//Header DECLARE_DYNCREATE(NewThread)		
//Source IMPLEMENT_DYNCREATE(NewThread, MyThread)

UINT StartThreadFunction( LPVOID pParam );
UINT StopThreadFunction( LPVOID pParam );

#include "mytime.h"
#include "MessageInspector.h"
#include <afxmt.h>
#include <afxtempl.h>
#include <cstringt.h>
#include <afxcoll.h> 


#define AssistantID 123456
enum {ERR_MIN=50000, ERR_MAX=59999, WRN_MIN, WRN_MAX=70000};

enum {ResourceMSG_TYPE=MSG_ARRAY_TYPE+1, ErrorMSG_TYPE, ErrorResMSG_TYPE, WarningResMSG_TYPE, 
		LOG_MSG_TYPE, START_PARAMS_MSG_TYPE, STOP_PARAMS_MSG_TYPE, ERR_ARR_MSG_TYPE, 
		PARENT_REPORT_MSG, MYTHREAD_MSG_LAST_TYPE};


class ResourceMSG: public AbstractMessage
{
protected:
	DWORD Body_ID;
public:
	ResourceMSG() {Body_ID=0; ClassID=ResourceMSG_TYPE; Body=CString("look for ID");}
	ResourceMSG* Create(DWORD _body_ID, CString& _sender, HWND _reciver=NULL, CString& _subj=CString("no subject"))
	{
		Body_ID=_body_ID;
		return this;
	};
	operator DWORD() {return Body_ID;}
	virtual ResourceMSG& operator=(const ResourceMSG& T);
};

class ErrorMessage: public AbstractMessage
{
//protected:
//	static int classID;
public:
	ErrorMessage():AbstractMessage() {ClassID=ErrorMSG_TYPE; }
	ErrorMessage* Create(CString& _sender, CString& _body, CString& _subj=CString("Error"))
	{
		Sender=_sender; Body=_body; Subject=_subj;
//		AbstractMessage::Create(_sender, _body, _subj);
		return this;
	}		
};

class Error: public ResourceMSG
{
//protected:
//	static int classID;
public:
	Error():ResourceMSG() {ClassID=ErrorResMSG_TYPE; }
	virtual CString Format();
};

class Warning: public Error
{
//protected:
//	static int classID;
public:
	Warning():Error() {ClassID=WarningResMSG_TYPE; }
};

//**************************************************
//**************************************************
/*
class ErrorsArray: public MessagesArray
{
	friend class ErrorInspector;
protected:
//	static int classID;
	int err_num, warn_num;
	virtual int RemoveAll() {err_num=warn_num=0; return MessagesArray::RemoveAll();}
public:	
	ErrorsArray() {err_num=warn_num=0; Name="ErrorsArray"; ClassID=ERR_ARR_MSG_TYPE;};
	virtual int Add(MessagesInspectorSubject* t);
	virtual ~ErrorsArray() {}
};
*/
class ErrorsArray: public MessageForWindow, public AbstractMessagesArray
{
	friend class ErrorInspector;
protected:
	int err_num, warn_num;
	virtual void RemoveAll() {err_num=warn_num=0; return AbstractMessagesArray::RemoveAll();}
public:	
	ErrorsArray() 
	{
		err_num=warn_num=0; Name="ErrorsArray"; ClassID=ERR_ARR_MSG_TYPE;
	};
	virtual int Add(AbstractMessage* t);
	virtual ~ErrorsArray() 
	{
		int a=5;
		a++;
	}
//	virtual void Dispatch();
};
#define NO_ERRORS 0

class ErrorInspector
{
protected:
	CList<ErrorsArray*> ErrArr;
public:
	ErrorInspector() {}
	~ErrorInspector();
	void RegisterError(AbstractMessage* _err);
	int Errors();
	int Warnings();
	ErrorsArray* GetErrorsArray();
	void ResetErrors();
	void AttachErrors(ErrorsArray* src) ;
	void AttachErrorsOF(ErrorInspector* src) ;
	ErrorsArray* DetachErrors();
	int DispatchErrors();
};


enum MyThreadMessages {UM_STOP=WM_USER + 76, UM_PAUSE, UM_CONTINUE, UM_START, UM_EXIT_THREAD,UM_EVENTLOG};
enum StopState {None=0, ToDo, Done};

enum MessagePriorities {lmprLOW = 0, lmprHIGH = 78};

class LogMessage: public MessageForWindow 
{	
//protected:
//	static int classID;
public:	
	enum MessagePriorities {low_pr=0, high_pr=78};

	int priority;
	CStringArray Msgs;

	LogMessage* CreateEntry(const CString& obj, const CString Message=CString(""),int _priority=low_pr);
	LogMessage* CreateEntry(ErrorsArray* err);
	LogMessage& operator <<(const CString &Message);
	void SetPriority(const LogMessage::MessagePriorities &pr) {priority = pr;}
	void SetPriority(const ::MessagePriorities &pr) {priority = pr;}
	LogMessage(::MessagePriorities prio = ::lmprLOW);
	int GetSize() {return (int)(Msgs.GetSize());}
	BOOL HasMessages() {return (GetSize() != 0);}
};

class ControledLogMessage
{
protected:
	LogMessage* msg; 
public:
	CString T;

	ControledLogMessage(::MessagePriorities prio = ::lmprLOW)	
		{msg = NULL;	msg = new LogMessage(prio);}
	~ControledLogMessage()	
		{if (msg != NULL) delete msg;}	
	virtual void Dispatch()	
	{
		if (msg->HasMessages())
		{
			msg->Dispatch(); msg = NULL;
		}
		else
		{
			delete msg; msg = NULL;
		}
	}
	LogMessage& operator <<(const CString &Message)
		{return (*msg) << Message;}
	void SetPriority(const ::MessagePriorities &pr) 
		{msg->SetPriority(pr);}
	int GetSize() const
		{msg->GetSize();}
	BOOL HasMessages() const
		{msg->HasMessages();}
};

class MyThread;
struct StartParams: public AbstractMessage
{
public:	
	WPARAM wParam; LPARAM lParam; CWinThread *src, *dest;

	StartParams() {wParam=lParam=0; src=dest=NULL; ClassID=START_PARAMS_MSG_TYPE; };
	StartParams* Create(WPARAM wP, LPARAM lP, CWinThread* _dest)
	{
		wParam=wP; lParam=lP; dest=_dest; 
		src=AfxGetThread(); Name="StartParams";
		return this;
	};
	virtual ~StartParams() {}
};

struct StopParams: public AbstractMessage
{
public:
	MyThread* T;	

	StopParams(MyThread* _T=0) {T=_T; Name="StopParams"; ClassID=STOP_PARAMS_MSG_TYPE; };
	virtual ~StopParams() {}

};

/////////////////////////////////////////////////////////////////////////////

class MyThread : public CWinThread, public ErrorInspector
{
	friend UINT StartThreadFunction( LPVOID pParam);
	friend UINT StopThreadFunction( LPVOID pParam);

	DECLARE_DYNCREATE(MyThread)		

	enum _ThreadStatus {STOPED=0, PAUSED, RUNNING, THRD_ERROR};
	enum _ThreadState {Suspended, Resumed};	

	struct ThreadConfig
	{
		friend class MyThread;
	protected:
		_ThreadStatus ThreadStatus;
		_ThreadState ThreadState;	
		StopState Stop, Terminate;
	public:
		int PersonalID;
		CString Name;
		bool UpdateParent;
		bool NotifyUser;
		CString ActionDescription;
		CString PauseComment;
		WindowAddress ParentWindow;

		ThreadConfig();
		_ThreadStatus GetStatus() {return ThreadStatus;};
		StopState GetStop() {return Stop;}
		StopState GetTerminate() {return Terminate;}
		void SetTerminate(StopState t) {Terminate=t;}
		ThreadConfig& operator=(ThreadConfig&);
	};
	struct ParentStatusReport: public AbstractMessage
	{
	public:
		ThreadConfig ConfigCopy;
		ParentStatusReport(ThreadConfig&);
	};

private:
	CEvent *TerminateEvent;	
	CEvent RunEvent;

	void MyResume();
	void MySuspend();	
protected:
	StartParams startParam1;
	CWinThread* Parent;
	int lastID;		
	ThreadConfig LastConfig;

	virtual void BeforeExecute(WPARAM wParam, LPARAM lParam);
	virtual void Main(WPARAM wParam, LPARAM lParam) {};	
	virtual void AfterExecute();	
public:			
	MyTimer Timer1;  int	WAIT_TIME;
	int pass_In, pass_Out;
//	MessagesInspector Inspector1; 
	ThreadConfig Config;
	
	MyThread();           // protected constructor used by dynamic creation	
	virtual void Create(CWinThread* parent,int ID, CEvent* evnt);	
	virtual void ContinueThread();	
	virtual void PauseThread();	
	virtual void StartThread(WPARAM,LPARAM);
	virtual void StopThread(bool Wait);	
	virtual int TerminateThread();
	virtual ~MyThread();
	virtual bool PostParentMessage(UINT msg, MessagesInspectorSubject* lParam=NULL);
	virtual int RegisterMsg(MessagesInspectorSubject *);
	CEvent* GetTerminateEvent() {return TerminateEvent;}
	ParentStatusReport* GetParentReport();
	void UpdateParent();
	ThreadConfig ChangeConfig(ThreadConfig cfg);
	void RestoreLastConfig(MyThread::ThreadConfig* cfg=NULL);

	friend CString GetString(enum MyThread::_ThreadState a);
	friend CString GetString(enum MyThread::_ThreadStatus a);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Implementation
	//{{AFX_MSG(MyThread)	
	afx_msg void OnExitThread(WPARAM wParam, LPARAM lParam );	
	afx_msg void OnStart(WPARAM wParam, LPARAM lParam );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


template<typename T>
class WorkThread: public MyThread
{
public:
	T params;
	WorkThread(int id)
	{
		MyThread::Create(AfxGetApp(),id,NULL);
	};
	~WorkThread(){};

	static UINT proc(void* param);
	virtual HRESULT Start()
	{
		CWinThread *ret;
		ret=AfxBeginThread(proc,(void*)this);		
		return S_OK;
	}
};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYTHREAD_H__192EEA8B_2C40_4C22_9F15_F8F602DC6038__INCLUDED_)
