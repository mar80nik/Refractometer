#pragma once
#include "BarTemplate.h"
#include "camera.h"
#include "cameraChooser.h"
#include "mythread/MyThread.h"
#include "afxwin.h"

#define SET_RGBQUAD(a,r,g,b) {a.rgbBlue=b; a.rgbGreen=g; a.rgbRed=r;}

class CaptureRequestStack
{
public:
	struct Item 
	{
		CWnd* sender; BMPanvas *buf;
		Item(CWnd* _sender=NULL, BMPanvas* _buf=NULL) {sender=_sender; buf=_buf;}
	};
    CArray<Item> stack;

	CaptureRequestStack() {};
	~CaptureRequestStack() {};
	CaptureRequestStack& operator << (Item item)
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
};

struct AccumInfo
{	
	USHORT w, h, n;
	virtual void Serialize(CArchive &ar);
	size_t GetSumsSize() const;
	size_t GetCompressorBufferSize() const;
};

struct ImagesAccumulator: public AccumInfo
{
protected:
	BYTE *sums;
public:
	BMPanvas *bmp; 
	
	ms fillTime;

	ImagesAccumulator();;
	~ImagesAccumulator() {Reset();};
	void Reset();
	void ResetSums();	
	unsigned short *GetSum();
	unsigned int *GetSums2();

	void Initialize(int _w, int _h);
	HRESULT FillAccum(BMPanvas *src);
	void ConvertToBitmap(CWnd *ref);
	HRESULT SaveTo(const CString &file);
	HRESULT LoadFrom(const CString &file);
	void ScanLine( void *buf, const int y, const int xmin, const int xmax );
};

class CaptureWnd : public CWnd, public PerfomanceStaff
{
	DECLARE_DYNAMIC(CaptureWnd)

	class CtrlsTab : public CDialog
	{		
	protected:	
		CString Name;	
	public:
		CWnd* Parent;
		enum ColorTransformModes {TrueColor, NativeGDI, HSL, HSV};
		CameraChooser Chooser;

		CtrlsTab(CWnd* pParent = NULL) : IsProgramBW(TRUE)
			, ColorTransformSelector(NativeGDI)
		{};   // standard constructor	
		// Dialog Data
		//{{AFX_DATA(DialogBarTab1)
		enum { IDD = IDD_DIALOGBARTAB4 };
		//}}AFX_DATA

		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(DialogBarTab1)
	public:
	protected:
		//}}AFX_VIRTUAL
		// Implementation
	protected:
		CButton BtnCapture, BtnStop, BtnPause, BtnResume, BtnChooseCam;
		// Generated message map functions
		//{{AFX_MSG(DialogBarTab1)
		virtual BOOL OnInitDialog();	
		virtual void OnOK() {};
		virtual void OnCancel() {};
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnBnClicked_Live();
		afx_msg void OnBnClicked_StopCapture();
		afx_msg void OnBnClicked_PauseCapture();
		afx_msg void OnBnClicked_ResumeCapture();
		afx_msg void OnBnClicked_FilterParams();
		//	afx_msg void OnBnClickedButton6();
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);
	public:
		afx_msg void OnBnClickedChooseCam();
		CComboBox PreviewSize;
		afx_msg void OnCbnSelchangeCombo1();
		eDcm800Size GetPreviewSize();
		CButton BtnFilterParams;
	public:
		BOOL IsProgramBW;
		afx_msg void OnBnClickedRadio1();
		afx_msg void OnBnClickedRadio4();
		ColorTransformModes ColorTransformSelector;
	};

protected:
	CaptureThread thrd;	
	DSCaptureSource* Src;
	CPen penRed, penBlue, penGreen, penWhite;
    
	ProtectedBMPanvas Pbuf,LevelsScanBuf;
	BMPanvas grayscaleBuf, truecolorBuf;
	RGBQUAD pal[256], palLevelsScan[256];
	CaptureRequestStack Stack;
public:
	CtrlsTab Ctrls;
	CFont font1;
	int cntr;
	CRect CameraOutWnd, LevelsScanWnd;

	CaptureWnd();
	virtual ~CaptureWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnDataUpdate(WPARAM wParam, LPARAM lParam );
	afx_msg void OnPaint();
	LRESULT OnCaptureRequest( WPARAM wParam, LPARAM lParam );
	void SelectCaptureSrc(CString name);
	afx_msg void OnDestroy();
	void ScanLevels(BMPanvas *src, BMPanvas &levels, const CtrlsTab::ColorTransformModes mode);
};

void ColorTransform(BMPanvas *color, BMPanvas *grayscale, CaptureWnd::CtrlsTab::ColorTransformModes mode);
