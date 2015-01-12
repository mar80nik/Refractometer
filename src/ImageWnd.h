#pragma once
#include "BarTemplate.h"
#include "camera.h"
#include "captureWnd.h"
#include "calibratorDialog.h"
#include "CalcTEDialog.h"
#include "afxwin.h"

enum CaptureWndMSGS {UM_CAPTURE_REQUEST=4000, UM_CAPTURE_EVENT};
enum CEditInterceptorMessages {UM_BUTTON_ITERCEPTED = 3000};

enum HelperEvent {
	EvntOnCaptureButton, EvntOnCaptureReady, EvntOnCaptureStop,
	RSLT_HELPER_COMPLETE, RSLT_OK, RSLT_OK_RGN_MODIFIED, RSLT_BMP_ERR, RSLT_ERR
};
//================================================
struct BaseForHelper
{
	virtual HelperEvent Update(const HelperEvent &event) = 0;
	virtual ~BaseForHelper() {}
};

//================================================


class CEditInterceptor : public CEdit
{
	DECLARE_DYNAMIC(CEditInterceptor)
public:
	CEditInterceptor() {};
	virtual ~CEditInterceptor() {};
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class CComboBoxInterceptor : public CComboBox
{
	DECLARE_DYNAMIC(CComboBoxInterceptor)
public:
	CComboBoxInterceptor() {};
	virtual ~CComboBoxInterceptor() {};
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnCbnSelchangeCombo2();
};

struct ScanRgnData
{ int stroka, Xmin, Xmax, AvrRange;};

struct ImagesAccumulator: public AccumInfo
{
protected:
	BYTE *sums; size_t OldSumsSize;
public:
	BMPanvas *bmp; 

	ms fillTime;

	ImagesAccumulator();
	~ImagesAccumulator() {Reset();};
	void Reset();
	void ResetSums();	
	unsigned short *GetSum() const;
	unsigned int *GetSums2() const;
	HRESULT GetPicRgn(CRect&) const;

	HRESULT Initialize(int _w, int _h, BOOL hasErrors = TRUE);
	HRESULT FillAccum(BMPanvas *src);
	void ConvertToBitmap(CWnd *ref);
	HRESULT SaveTo(const CString &file);
	HRESULT LoadFrom(const CString &file);
	void ScanLine( void *buf, const ScanRgnData &data);
};

class ImageWnd : public CWnd
{
public:
	class CtrlsTab : public BarTemplate, public ScanRgnData
	{		
	protected:	
		CString Name;	
	public:
		enum { IDD = IDD_DIALOGBARTAB1 };

		CEditInterceptor XminCtrl, XmaxCtrl, strokaCtrl;
		CComboBox NofScans;
		CComboBoxInterceptor AvrRgnCombo;

		CtrlsTab(CWnd* pParent = NULL);  
		CRect GetScanRgnFromCtrls();
		void InitCtrlsFromScanRgn( const CRect& rgn);
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);   
	protected:
		virtual BOOL OnInitDialog();	
		virtual void OnOK() {};
		virtual void OnCancel() {};
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnBnClickedScan();
		virtual BOOL DestroyWindow();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnBnClickedButton5();
		afx_msg void OnEnKillfocusEdit1();
		LRESULT OnButtonIntercepted(WPARAM wParam, LPARAM lParam );
		int GetNofScans();
		int GetAvrRgn();	
	};

	class PicWnd: public CWnd
	{
		friend struct AccumHelper;

		enum ScanRgnDrawModes { DRAW, ERASE };
		class c_ScanRgn
		{
		protected:
			BOOL ToErase; 
			CRect last;
			CPoint curL, curR;

			void Draw(BMPanvas* bmp, const CRect& rgn, ScanRgnDrawModes mode );
		public:
			c_ScanRgn() { ToErase=FALSE; }
			virtual void Draw(BMPanvas* Parent, const CRect& rgn);
			virtual void Erase(BMPanvas * canvas);		
		};

	protected:
		CButton CaptureButton;
		CMenu menu1; c_ScanRgn ScanRgn;		
		CList<BaseForHelper*> helpers; 

		void UpdateHelpers(const HelperEvent &event);
	public:
		BMPanvas ava;
		ImageWnd* Parent;
		CFont font1;
		CString FileName;
		ImagesAccumulator accum;
		enum {CaptureBtnID=234234};

		PicWnd();
		virtual ~PicWnd();
		HRESULT LoadPic(CString T);
		void UpdateNow(void);
		void OnPicWndErase();
		void OnPicWndSave();
		void OnPicWndScanLine();
		void EraseAva();
		HRESULT MakeAva();
		void UpdateScanRgn();
		HRESULT GetRgnOfScan(CRect&);
		HRESULT ValidateScanRgn( CRect& rgn ) const;
		void ConvertOrgToGrayscale();
		HRESULT TryLoadBitmap(CString T, BMPanvas &bmp);

		DECLARE_MESSAGE_MAP()
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnPaint();
		afx_msg void OnDropFiles(HDROP hDropInfo);	
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnCaptureButton();
		afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
		afx_msg void OnMove(int x, int y);	
		HRESULT ConvertAvaToOrg( CPoint& pnt ) const;
		HRESULT ConvertOrgToAva( CRect& rgn ) const;
		LRESULT OnCaptureEvent( WPARAM wParam, LPARAM lParam );
	};
	
	DECLARE_DYNAMIC(ImageWnd)
protected:
	PicWnd dark, cupol, strips;
	CRect RgnOfScan;
	
public:
	CtrlsTab Ctrls;
	CaptureWnd	CameraWnd;

	ImageWnd();
	virtual ~ImageWnd();
	CRect GetRgnOfScan() const {return RgnOfScan;};
	void OnChildMove();
	void SetScanRgn(const CRect&);
	void * GetChartFromParent();
	ScanRgnData GetScanRgnData()
	{
		ScanRgnData ret;
		ret.Xmin = RgnOfScan.left; ret.Xmax = RgnOfScan.right;
		ret.stroka = RgnOfScan.CenterPoint().y;
		ret.AvrRange = (RgnOfScan.bottom - RgnOfScan.top)/2;
		return ret;
	}
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnEnUpdateEdit3();
	afx_msg void OnNMThemeChangedEdit3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);	
};

struct AccumHelper: public BaseForHelper
{	
	int n_max;
	ImageWnd::PicWnd *parent;
	BMPanvas *tmp_bmp;

	AccumHelper(ImageWnd::PicWnd *_parent, const int _n_max);
	virtual ~AccumHelper();
	virtual HelperEvent Update(const HelperEvent &event);
};


