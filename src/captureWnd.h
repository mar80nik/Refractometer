#pragma once
#include "BarTemplate.h"
#include "camera.h"
#include "cameraChooser.h"
#include "MyThread.h"
#include "afxwin.h"
#include "GlobalHeader.h"

#define SET_RGBQUAD(a,r,g,b) {a.rgbBlue=b; a.rgbGreen=g; a.rgbRed=r;}



struct AccumInfo
{		
	USHORT w, h, n; BYTE HasErrors;
	virtual void Serialize(CArchive &ar);
	AccumInfo() {w = h = n = 0; HasErrors = FALSE; }
	size_t GetSumsSize() const;
	size_t GetCompressorBufferSize() const;
};


class CaptureWnd : public CWnd, public PerfomanceStaff, public WindowAddress
{
	DECLARE_DYNAMIC(CaptureWnd)

	class CtrlsTab : public CDialog
	{		
	protected:	
		CString Name;	
	public:
		CWnd* Parent;		
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
		afx_msg void OnBnClicked_Live();
		afx_msg void OnBnClicked_StopCapture();
		afx_msg void OnBnClicked_PauseCapture();
		afx_msg void OnBnClicked_ResumeCapture();
		afx_msg void OnBnClicked_FilterParams();
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);
	public:
		afx_msg void OnBnClickedChooseCam();
		CComboBox PreviewSize;
		eDcm800Size GetPreviewSize();
		CButton BtnFilterParams;
	public:
		BOOL IsProgramBW;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	ColorTransformModes ColorTransformSelector;
	};

protected:
	CaptureThread thrd;	
	DSCaptureSource* Src;
	CPen penRed, penBlue, penGreen, penWhite;
    
	ProtectedBMPanvas Pbuf,LevelsScanBuf;
	BMPanvas grayscaleBuf, truecolorBuf;
	RGBQUAD pal[256], palLevelsScan[256];
	ProtectedCaptureRequestStack Stack;
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
	void ScanLevels(BMPanvas *src, BMPanvas &levels, const ColorTransformModes mode);
};

void ColorTransform(BMPanvas *color, BMPanvas *grayscale, ColorTransformModes mode);
