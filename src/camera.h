#pragma once
#include <dshow.h>
#include <Qedit.h>
#include "BMPanvas.h"
#include "dcm800.h"
#include "MessageInspector.h"
#include "MyThread.h"
#include "GlobalHeader.h"
//#include "captureWnd.h"

#define SAFE_RELEASE_(a) if(a!=NULL) { a->Release(); a=NULL; }
class MyDSFilter;
class DSCaptureSource;

struct SAM_MEDIA_TYPE: public AM_MEDIA_TYPE
{
	SAM_MEDIA_TYPE() {};
	~SAM_MEDIA_TYPE()
	{
		if (cbFormat != 0) { CoTaskMemFree((PVOID)pbFormat); cbFormat = 0; pbFormat = NULL; }
		if (pUnk != NULL) { pUnk->Release(); pUnk = NULL; }
	}
};

enum ColorTransformModes {TrueColor, NativeGDI, HSL, HSV};

class CaptureRequestStack
{
public:
	struct Item 
	{
		CWnd* sender; BMPanvas *buf;
		Item(CWnd* _sender=NULL, BMPanvas* _buf=NULL) {sender=_sender; buf=_buf;}
	};
protected:
	CArray<Item> stack;
public:
	CaptureRequestStack() {};
	~CaptureRequestStack() {RemoveAll();};
	CaptureRequestStack& operator << (const Item &item)
	{
		stack.Add(item);
		return *this;
	}
	BOOL operator >> (Item& item)
	{
		int size; BOOL ret=FALSE;
		if((size=stack.GetSize())!=0)
		{
			item=stack[size-1]; ret=TRUE;
			stack.RemoveAt(size-1);
		}		
		return ret;
	}
	void RemoveAll()
	{
		stack.RemoveAll();
	}
};

typedef ProtectedObjectX<CaptureRequestStack> ProtectedCaptureRequestStack;
typedef ProtectorX<CaptureRequestStack> CaptureRequestStackGuard;

struct CaptureParams 
{
	ColorTransformModes *ColorTransformSelector;
	ProtectedCaptureRequestStack *Stack;
	ProtectedBMPanvas *Pbuf;
	CStringW SourceName;
	DSCaptureSource* Src;
	WindowAddress Parent;
	CEvent StopCapture, PauseCapture,ResumeCapture;
	CEvent ShowFilterParams;
	int Width,Height; 
	eDcm800Size size;
	void* thrd;

	CaptureParams(): StopCapture(true,true),PauseCapture(true,true),ResumeCapture(true,true),
        ShowFilterParams(true,true)
	{
			thrd=NULL; Src=NULL;
	}
	UINT workthread_main();
};

typedef WorkThread<CaptureParams> CaptureThread;


struct BMPanvasTAGSmk1
{
    double d1,d2,d3;
	ms timel;
	int i1,i2,i3;
	BMPanvasTAGSmk1() {d1=d2=d3=0.; i1=i2=i3=0;}
};

class FrameGrabCallback : public ISampleGrabberCB
{
public :
	STDMETHODIMP_(ULONG) AddRef() { return 2; }
	STDMETHODIMP_(ULONG) Release() { return 1; }
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	FrameGrabCallback() {framenum=0; font.CreatePointFont(120,"Arial"); }
	~FrameGrabCallback() {}
public:
	VIDEOINFOHEADER header;
	long framenum;
	CString strPath;
	void *capture_params;
	MyTimer t1;
	CFont font;

	STDMETHODIMP SampleCB(double n,IMediaSample *pms);
	STDMETHODIMP BufferCB( double SampleTime, BYTE * pBuffer, long BufferSize ) {return 0;};
};

struct PinInfo 
{
	PIN_INFO i;  
	struct SAFE_AM_MEDIA_TYPE  
	{
		GUID majortype;
		GUID subtype;
		BOOL bFixedSizeSamples;
		BOOL bTemporalCompression;
		ULONG lSampleSize;
		GUID formattype;

		SAFE_AM_MEDIA_TYPE() {}
		SAFE_AM_MEDIA_TYPE& operator=(SAM_MEDIA_TYPE& type)
		{
			majortype=type.majortype;
			subtype=type.subtype;
			bFixedSizeSamples=type.bFixedSizeSamples;
			bTemporalCompression=type.bTemporalCompression;
			lSampleSize=type.lSampleSize;
			formattype=type.formattype;
			return *this;
		}
	} MediaType;
	VIDEOINFOHEADER header;

	PinInfo() {i.pFilter=NULL;}
	PinInfo& operator=(SAM_MEDIA_TYPE& type)
	{
		MediaType=type;
		if (type.formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)type.pbFormat;
			header=*pVIH;
		}
		return *this;
	}
};
//////////////////////////////////////////////////////////////////////////
struct MyMoniker
{
	IMoniker* pMoniker;
	MyMoniker(MyMoniker& m);
	MyMoniker(IMoniker* m=NULL);
	~MyMoniker();
	MyMoniker& operator=(IMoniker* m);
	MyMoniker& operator=(MyMoniker& m);
	operator IMoniker*();
	CString GetName();
};
struct MonikersArray: public CArray<MyMoniker,MyMoniker&>
{
	MonikersArray() {}
	virtual ~MonikersArray() {}
};

#define ANY_NAME "AnyName"

class MyEnumerator
{
protected:
	HRESULT FindMonikers(MonikersArray& ret);
public:
	HRESULT status;
	MyEnumerator() {status=S_FALSE;}
	virtual ~MyEnumerator() {}
	HRESULT FindSource(CString name,MonikersArray& ret);
};
//////////////////////////////////////////////////////////////////////////
class MyDSFilter
{
	friend class DSGraphBuilder;
public:
	CArray<PinInfo,PinInfo&> Pins;
	IBaseFilter* pFilter; 
	HRESULT status;
	CStringW Name; 
//-------------------------------------------
	MyDSFilter() { status=S_FALSE; pFilter=NULL; }
	virtual ~MyDSFilter() { Destroy(); }
	operator IBaseFilter*() { return pFilter; }
	virtual void Destroy() { SAFE_RELEASE_(pFilter); }
	HRESULT GetPinsInfo();
	virtual HRESULT Create(CStringW name) { Name=name; return (status=S_FALSE);}
	virtual HRESULT Create(IMoniker* pMoniker) 
	{
		return (status=pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter));
	}
	HRESULT ShowFilterProperties();
};

class DSCaptureSource: public MyDSFilter
{
public:
	DSCaptureSource() {}	
	virtual ~DSCaptureSource() {}
	virtual HRESULT Setup(void* p) {return (status=S_OK);};	
	virtual HRESULT Create(CStringW name);
	virtual HRESULT Create(IMoniker* pMoniker) {return MyDSFilter::Create(pMoniker);};
};

class ScopeTek_DCM800: public DSCaptureSource
{
protected:
	IDcm800 *pCntrl;
public:
	ScopeTek_DCM800() {pCntrl=NULL; Name=L"ScopeTek DCM800";}
	virtual ~ScopeTek_DCM800() {}
	virtual HRESULT Setup(void* p);	
	virtual HRESULT Create(CStringW name=CStringW(""));
	virtual void Destroy() { SAFE_RELEASE_(pFilter); SAFE_RELEASE_(pCntrl);}
	virtual HRESULT Create(IMoniker* pMoniker);
};

class DSNullRender: public MyDSFilter
{
public: 
	DSNullRender() {Name=L"NULLRender"; }	
	HRESULT Create() { return (status=CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)(&pFilter))); }	
};

class DSSampleGrabber: public MyDSFilter
{
public:
	FrameGrabCallback   m_FrameGrabCallback; 
	ISampleGrabber *pGrabber;
public:
	DSSampleGrabber() { Name=L"SampleGraber"; pGrabber = NULL; }	
	virtual ~DSSampleGrabber() { Destroy(); }
	virtual void Destroy() { SAFE_RELEASE_(pGrabber); MyDSFilter::Destroy(); }
	HRESULT Create();
};

class DSGraphBuilder
{
protected:
	
public:
	IGraphBuilder *pGraph;
	ICaptureGraphBuilder2 *pBuilder;
	IMediaControl *pControl;
	IMediaEventEx *pEvent; 
	CArray<HANDLE> Handles;
	HRESULT status;

	DSGraphBuilder() { pGraph=NULL; status=S_OK; pBuilder=NULL; pControl = NULL; pEvent = NULL; }
	~DSGraphBuilder() { Destroy(); }
	virtual void Destroy();
	HRESULT Create();		
	DSGraphBuilder& operator << (MyDSFilter& filter);
	HRESULT SaveGraph(CStringW filename);
	virtual HRESULT Run();
	HRESULT Stop();
	HRESULT Pause();
	HRESULT StreamIsCOMPLETE();	
	DWORD WaitForHandles(DWORD ms) {return WaitForMultipleObjects(Handles.GetSize(),Handles.GetData(),false, 1000);}
};

class GrabberStream: public DSGraphBuilder
{
protected:
    MyDSFilter* Src;
public:
	DSNullRender NullRenderer;
	DSSampleGrabber Grabber;
public:
	GrabberStream() {Src=NULL;}
	virtual ~GrabberStream() { Destroy(); }
	HRESULT Render(MyDSFilter* Src);
	HRESULT Setup(void* Params);
	virtual void Destroy() { NullRenderer.Destroy(); Grabber.Destroy();	DSGraphBuilder::Destroy(); }	
	HRESULT Create(MyDSFilter* Src);		
};


