#include "stdafx.h"
#include "camera.h"
#include "captureWnd.h"
#include "ImageWnd.h"

static const GUID IID_IDcm800 = { 0x7a3cfafd, 0x3168, 0x4fa1, { 0x99, 0x3a, 0x11, 0xc0, 0x24, 0xf2, 0xb0, 0xb } };

void GetWH(eDcm800Size& size, int& w, int &h)
{
	switch(size)
	{
	case eDcm800Size_3264_2448: w=3264; h=2448; break;
	case eDcm800Size_1600_1200: w=1600; h=1200; break;
	case eDcm800Size_1280_960:  w=1280; h=960; break;
	case eDcm800Size_1024_768:  w=1024; h=768; break;
	case eDcm800Size_640_480:   w=640; h=480; break;
	}
}
// This is callback for the thread which runs Grabber. 
// It makes setup then runs it and wait for control signals from user via CEvents. 
// All the grabbing makes in callback
UINT CaptureParams::workthread_main()
{
	UINT ret=444; HRESULT hr; DWORD n=0; BOOL bDone=FALSE;
	GrabberStream Grabber; ControledLogMessage log;

	CoInitialize(NULL);
	hr = Grabber.Create(Src);
	if (SUCCEEDED(hr)) 
	{
		hr = Src->Setup(&Src); 		
		hr = Grabber.Render(Src);	
		hr = Grabber.Setup(this);		
		if (SUCCEEDED(Grabber.status))
		{
			StopCapture.ResetEvent();	
			hr=Grabber.Run();
			while(!bDone) 
			{		
				if ( (n=Grabber.WaitForHandles(1000))!=WAIT_TIMEOUT )
				{ 
					switch (n - WAIT_OBJECT_0)
					{
					case 0: if (SUCCEEDED(Grabber.StreamIsCOMPLETE())) 
							{
								bDone=true; 
							}
							break;
					case 1: bDone = true; StopCapture.ResetEvent(); break;
					case 2: Grabber.Pause(); PauseCapture.ResetEvent(); break;
					case 3: Grabber.Run(); ResumeCapture.ResetEvent(); break;
					case 4: hr = Src->ShowFilterProperties(); ShowFilterParams.ResetEvent(); break;
					}	
				}
				if (FAILED(hr)) bDone=TRUE;
			}
			hr=Grabber.Stop();
			if (FAILED(hr)) { log.T = "GRAPH RUN error"; log << log.T; log.SetPriority(lmprHIGH); }
		}
		else { log.T = "GRAPH SETUP error"; log << log.T; log.SetPriority(lmprHIGH); }
	}
	else { log.T = "GRAPH INIT error"; log << log.T; log.SetPriority(lmprHIGH); }
	
	CoUninitialize();
	log.Dispatch();
	return ret;
}

struct CaptureRequestStackCBparams 
{
	BMPanvas *buf; 
	ColorTransformModes *ColorTransformSelector;
	void GainAcsessCB(CaptureRequestStack& Stack)
	{
		CaptureRequestStack::Item request;
		if (*ColorTransformSelector != TrueColor)
		{
			while(Stack >> request)
			{
				if((*request.buf) != (*buf))
				{
					request.buf->Create(buf->GetDC(), buf->w, buf->h, 8);
					request.buf->CreateGrayPallete();
				}
				ColorTransform(buf, request.buf, *ColorTransformSelector);
				request.sender->PostMessage(UM_CAPTURE_EVENT,EvntOnCaptureReady,0);
			}			
		}
	}
};

struct BMPanvasCBparams
{
	BYTE *pbuf; double n; sec dt1;
	VIDEOINFOHEADER *header;	
	CaptureParams *capture_params;
	void GainAcsessCB(BMPanvas& buf)
	{
		MyTimer t2;	t2.Start();		
		int ret=buf.SetBitmapArray(BMPanvas::MIN_SCANLINE,BMPanvas::MAX_SCANLINE, pbuf);
		BMPanvasTAGSmk1* tags=(BMPanvasTAGSmk1*)buf.tags;
		tags->d1 = n; 
		tags->i1 = header->bmiHeader.biWidth; 
		tags->i2 = header->bmiHeader.biHeight; 
		tags->i3 = header->bmiHeader.biBitCount;
		tags->d2=(1./dt1.val()); 
		tags->timel=t2.StopStart();

		CaptureRequestStackCBparams paramsCB = {&buf, capture_params->ColorTransformSelector};
		capture_params->Stack->ModifyWith(paramsCB);

		MessageForWindow* msg=new MessageForWindow(UM_DATA_UPDATE, capture_params->Parent);
		msg->Dispatch();
	}
};

STDMETHODIMP FrameGrabCallback::SampleCB( double n, IMediaSample *pms )
{
	BMPanvasCBparams paramsCB;

	paramsCB.header = &header; paramsCB.capture_params = (CaptureParams*)capture_params;
	paramsCB.dt1 = t1.StopStart(); paramsCB.n = n;
	HRESULT hr = pms->GetPointer(&paramsCB.pbuf);
	long size = pms->GetSize();

	paramsCB.capture_params->Pbuf->ModifyWith(paramsCB);	
	return 0;	
}

STDMETHODIMP FrameGrabCallback::QueryInterface( REFIID iid, void** ppv )
{
	if (NULL == ppv) return E_POINTER;
	*ppv = NULL;
	if (IID_IUnknown == iid) {*ppv = (IUnknown*)this; AddRef(); return S_OK;}
	else 
		if (IID_ISampleGrabberCB == iid) {*ppv = (ISampleGrabberCB*)this; AddRef(); return S_OK;}
		return E_NOINTERFACE;
}

//////////////////////////////////////////////////////////////////////////

HRESULT DSSampleGrabber::Create()
{
	ASSERT(pFilter==NULL); 
	SAFE_RELEASE_(pFilter);
	SAM_MEDIA_TYPE mt; ZeroMemory(&mt, sizeof(SAM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video; mt.subtype = MEDIASUBTYPE_RGB24;

	status = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)(&pFilter)); 
	if (FAILED(status)) return status;
	status = pFilter->QueryInterface(IID_ISampleGrabber, (void**)(&pGrabber));
	if (FAILED(status)) { SAFE_RELEASE_(pFilter); return status; }
	status = pGrabber->SetMediaType(&mt);		
	if (FAILED(status)) { SAFE_RELEASE_(pFilter); SAFE_RELEASE_(pGrabber); return status; }
	status = pGrabber->SetCallback( &m_FrameGrabCallback, 0 );
	if (FAILED(status)) { SAFE_RELEASE_(pFilter); SAFE_RELEASE_(pGrabber); return status; }
	return status;
}
//////////////////////////////////////////////////////////////////////////

HRESULT DSGraphBuilder::Create()
{
	HANDLE  hEvent; 
	status = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&pGraph); 
	if (FAILED(status)) { return status; }
	status = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&pBuilder);
		if (FAILED(status)) { Destroy(); return status; }
	status = pBuilder->SetFiltergraph(pGraph);  
		if (FAILED(status)) { Destroy(); return status; }
	status = pGraph->QueryInterface(IID_IMediaControl,(void**)&pControl);
		if (FAILED(status)) { Destroy(); return status; }
	status = pGraph->QueryInterface(IID_IMediaEventEx,(void**)&pEvent);
		if (FAILED(status)) { Destroy(); return status; }
	status = pEvent->GetEventHandle((OAEVENT*)&hEvent);
		if (FAILED(status)){ Destroy(); return status; }
	Handles.Add(hEvent);
	return status;
}

DSGraphBuilder& DSGraphBuilder::operator<<( MyDSFilter& filter )
{
	if (SUCCEEDED(status))
	{
		status = filter.status;
		if (SUCCEEDED(status))
		{
			status = pGraph->AddFilter(filter.pFilter, filter.Name);	
		}			
	}		
	return *this;
}

HRESULT DSGraphBuilder::SaveGraph( CStringW filename )
{
	const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
	HRESULT hr;

	IStorage *pStorage = NULL;
	hr = StgCreateDocfile(
		filename,
		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, &pStorage);
	if (FAILED(hr)) {return hr;}

	IStream *pStream;
	hr = pStorage->CreateStream(
		wszStreamName,
		STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStream);
	if (FAILED(hr)) {pStorage->Release(); return hr;}

	IPersistStream *pPersist = NULL;
	pGraph->QueryInterface(IID_IPersistStream, (void**)&pPersist);
	if (pPersist != NULL)
	{
		hr = pPersist->Save(pStream, TRUE);	
		pPersist->Release();
	}
	pStream->Release();
	if (SUCCEEDED(hr)) { hr = pStorage->Commit(STGC_DEFAULT); }
	pStorage->Release();
	return hr;
}

HRESULT DSGraphBuilder::StreamIsCOMPLETE()
{
	long evCode, param1, param2;
	while (SUCCEEDED(pEvent->GetEvent(&evCode, &param1, &param2, 0))) 
	{		
		if (evCode == EC_COMPLETE) return S_OK;
		pEvent->FreeEventParams(evCode, param1, param2);
	}
	return E_FAIL;
}

HRESULT DSGraphBuilder::Pause() {if(status==S_OK) pControl->Pause(); return status;}
HRESULT DSGraphBuilder::Stop() {if(status==S_OK) pControl->Stop(); return status;}
HRESULT DSGraphBuilder::Run() {if(status==S_OK) pControl->Run(); return status;}
void DSGraphBuilder::Destroy()
{
	SAFE_RELEASE_(pGraph); SAFE_RELEASE_(pBuilder); SAFE_RELEASE_(pControl); SAFE_RELEASE_(pEvent);
	Handles.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////

HRESULT GrabberStream::Render( MyDSFilter* Src )
{
	if (FAILED(status)) return status;
	status = pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, *Src, Grabber, NullRenderer);
	if (FAILED(status)) { Destroy(); return status; }
	SaveGraph(CStringW("graph.grf"));
    return status;
}

HRESULT GrabberStream::Setup( void* _params )
{
	CaptureParams &params = *((CaptureParams*)_params); void *x;
	if(SUCCEEDED(Grabber.status))
	{
		if (SUCCEEDED(Grabber.GetPinsInfo()))
		{
			VIDEOINFOHEADER hdr=Grabber.Pins[0].header;
			Grabber.m_FrameGrabCallback.header=hdr;
			Grabber.m_FrameGrabCallback.capture_params = &params;
			Grabber.m_FrameGrabCallback.framenum=1;		
			//		Grabber.m_FrameGrabCallback.CreateTempBuf();
			Grabber.m_FrameGrabCallback.t1.Start();

			Handles.Add(params.StopCapture);
			Handles.Add(params.PauseCapture);
			Handles.Add(params.ResumeCapture);
			Handles.Add(params.ShowFilterParams);	

			if((x=params.Pbuf->GainAcsess(WRITE))!=NULL)
			{
				BMPanvasGuard guard1(x); BMPanvas &buf(guard1);
				buf.Create((HDC)NULL,hdr.bmiHeader.biWidth, hdr.bmiHeader.biHeight, hdr.bmiHeader.biBitCount);
			}
		}
	}	
    return Grabber.status;
}

HRESULT GrabberStream::Create(MyDSFilter* Src)
{
	Destroy();
	if (FAILED(Src->status)) return (status = Src->status);
	status = DSGraphBuilder::Create(); if (FAILED(status)) {Destroy(); return status;}
	status = Grabber.Create();	if (FAILED(status)) { Destroy(); return status; }
	status = NullRenderer.Create();	if (FAILED(status)) { Destroy(); return status; }
	(*this) << *Src << Grabber << NullRenderer; if (FAILED(status)) { Destroy(); return status; }
    return status;
}

//////////////////////////////////////////////////////////////////////////

HRESULT MyDSFilter::GetPinsInfo()
{
	HRESULT hr;	Pins.RemoveAll();
	if(FAILED(status)) return status;

	IEnumPins *EPins; IPin *pin; PinInfo PIN;
	if (SUCCEEDED(status = pFilter->EnumPins(&EPins)))
	{
		while(EPins->Next(1, &pin,NULL) == S_OK)
		{
			SAM_MEDIA_TYPE amt;
			hr=pin->QueryPinInfo(&PIN.i);
			hr=pin->ConnectionMediaType(&amt);	
			PIN=amt; Pins.Add(PIN); 
			SAFE_RELEASE_(pin);
		}
	}
	SAFE_RELEASE_(EPins);
    return status;    
}

HRESULT MyDSFilter::ShowFilterProperties()
{
	HRESULT hr;
	ISpecifyPropertyPages *pProp = NULL; IUnknown *pFilterUnk = NULL; CAUUID caGUID;
	hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	hr = pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);	
	if (pProp != NULL)
	{		
		hr = pProp->GetPages(&caGUID);
		if (SUCCEEDED(hr))
		{
			OleCreatePropertyFrame(NULL,0,0,NULL,1,&pFilterUnk,caGUID.cElems,caGUID.pElems,0,0, NULL);
			if (caGUID.pElems != NULL) 
			{
				CoTaskMemFree(caGUID.pElems);
			}				
		}		
	}	
	SAFE_RELEASE_(pFilterUnk); SAFE_RELEASE_(pProp);	
	return hr;
}

HRESULT ScopeTek_DCM800::Setup( void* params )
{
	CaptureParams& Params=*((CaptureParams*)params); 
	if (FAILED(status)) return status;	
	if(params==NULL) return (status=S_FALSE);
	
	status=pCntrl->put_PreviewSize(Params.size);
	status=pCntrl->put_StillSize(Params.size);
	return status;
}

HRESULT ScopeTek_DCM800::Create( CStringW name )
{
	DSCaptureSource::Create(Name);
		if(status!=S_OK) {Destroy(); return status;}
	status=pFilter->QueryInterface(IID_IDcm800,(void**)&pCntrl); 	
		if(status!=S_OK) {Destroy(); return status;}
	return status;
}

HRESULT ScopeTek_DCM800::Create( IMoniker* pMoniker )
{
	DSCaptureSource::Create(pMoniker);
		if(status!=S_OK) {Destroy(); return status;}
	status=pFilter->QueryInterface(IID_IDcm800,(void**)&pCntrl); 
		if(status!=S_OK) {Destroy(); return status;}
	return status;
}

HRESULT DSCaptureSource::Create( CStringW name )
{
	VARIANT varName; 
	ICreateDevEnum *pSysDevEnum = NULL;
	IPropertyBag *pPropBag=NULL;
	IEnumMoniker *pEnumCat = NULL; IMoniker *pMoniker=NULL;
	ULONG cFetched; CStringW T; 
	ASSERT(pFilter==NULL); SAFE_RELEASE_(pFilter);

	MyDSFilter::Create(name);

	status = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,	CLSCTX_INPROC_SERVER,IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if(status!=S_OK) return status;	
	status = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
	if(status!=S_OK) {SAFE_RELEASE_(pSysDevEnum); return status;}
	
	while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
	{			
		VariantInit(&varName); 
		status=pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);					
		if(status==S_OK)
		{
			status = pPropBag->Read(L"FriendlyName", &varName, 0);
			if(status==S_OK) 
				if(varName.vt==VT_BSTR) 
					if(varName==Name) status=MyDSFilter::Create(pMoniker);						
		}
		VariantClear(&varName);
		SAFE_RELEASE_(pPropBag);
		SAFE_RELEASE_(pMoniker);
		if(status!=S_OK) break;
	}
		
	SAFE_RELEASE_(pEnumCat);
	SAFE_RELEASE_(pSysDevEnum);
	return status;
}

//////////////////////////////////////////////////////////////////////////
HRESULT MyEnumerator::FindMonikers( MonikersArray& ret )
{
	ULONG cFetched;
	ICreateDevEnum *pSysDevEnum = NULL; 
	IPropertyBag *pPropBag=NULL;
	IEnumMoniker *pEnumCat = NULL; IMoniker *pMoniker=NULL;

	status = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,	CLSCTX_INPROC_SERVER,IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (SUCCEEDED(status))
	{
		status = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
		if (SUCCEEDED(status))
		{
			while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				MyMoniker m(pMoniker);
				ret.Add(m);
			}
			SAFE_RELEASE_(pEnumCat);
		}			
		SAFE_RELEASE_(pSysDevEnum);
	}
	return status;
}

HRESULT MyEnumerator::FindSource( CString name,MonikersArray& ret )
{
	MonikersArray arr; CString T; 
	FindMonikers(arr); 
	for(int i=0;i<arr.GetSize();i++)
	{
		T=arr[i].GetName();
		if(name=="AnyName" || name==T) 
		{
			ret.Add(arr[i]); arr[i]=0; 
		}
	}
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////
CString MyMoniker::GetName()
{
	VARIANT varName; HRESULT hr;
	IPropertyBag *pPropBag=NULL; CString ret("Error"); 
	if(pMoniker==NULL) return ret;

	VariantInit(&varName); 
	hr=pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);					
	if(hr==S_OK)
	{
		hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		if(hr==S_OK) 
		{
			if(varName.vt==VT_BSTR) ret=varName;
			SAFE_RELEASE_(pPropBag);		
		}
	}
	VariantClear(&varName);	
	return ret;
}

MyMoniker::MyMoniker( MyMoniker& m ) {pMoniker=m.pMoniker; m.pMoniker=NULL;}
MyMoniker::MyMoniker( IMoniker* m/*=NULL*/ ) {pMoniker=m;}
MyMoniker::~MyMoniker() {SAFE_RELEASE_(pMoniker);}
MyMoniker& MyMoniker::operator=( IMoniker* m ) {pMoniker=m; return *(this);}
MyMoniker& MyMoniker::operator=(MyMoniker& m ) {pMoniker=m.pMoniker;  m.pMoniker=NULL; return *(this);}
MyMoniker::operator IMoniker*() {return pMoniker;}