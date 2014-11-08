#pragma once
#include "BarTemplate.h"
#include "camera.h"
#include "captureWnd.h"
#include "calibratorDialog.h"
#include "CalcTEDialog.h"
#include "afxwin.h"

enum CaptureWndMSGS {UM_CAPTURE_REQUEST=4000, UM_CAPTURE_READY};
enum CEditInterceptorMessages {UM_BUTTON_ITERCEPTED = 3000};

enum HelperEvent {
	EvntOnCaptureButton, EvntOnCaptureReady,
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

class ImageWnd : public CWnd
{
public:
	struct PicRgn: protected CRect
	{
		PicRgn() {}
		PicRgn(const CRect &rgn): CRect(rgn) {}
		CRect Get() const
		{
			return *this;
		}
		void Set(const CRect &r)
		{
			*this = r;
		}

	};
	struct AvaPicRgn: public CRect 
	{ 
		CSize org, ava;
		CRect storedOrgRslt;

		AvaPicRgn(): CRect() {}
		AvaPicRgn(const CRect& rgn): CRect(rgn) {}
		BOOL Check(const CSize _org, const CSize _ava) const
		{
			BOOL ret = FALSE;
			if (org == _org && ava == _ava)
			{
				ret = TRUE;
			}
			return ret;
		}
	};
	struct OrgPicRgn: public CRect 
	{
		CSize org, ava;
		CRect storedAvaRslt;

		OrgPicRgn(): CRect() {}
		OrgPicRgn(const CRect& rgn): CRect(rgn) {}	
		BOOL Check(const CSize _org, const CSize _ava) const
		{
			BOOL ret = FALSE;
			if (org == _org && ava == _ava)
			{
				ret = TRUE;
			}
			return ret;
		}
		//OrgPicRgn& operator=(const OrgPicRgn& r)
		//{
		//	*(CRect*)this = r;
		//	org = r.org; ava = r.ava; storedAvaRslt = r.storedAvaRslt;
		//	return *this;
		//}
	};
	
	class CtrlsTab : public BarTemplate
	{		
	protected:	
		CString Name;	
	public:
		enum { IDD = IDD_DIALOGBARTAB1 };

		int stroka, Xmin, Xmax, AvrRange;
		CEditInterceptor XminCtrl, XmaxCtrl, strokaCtrl, AvrRangeCtrl;
		CComboBox NofScans;

		CtrlsTab(CWnd* pParent = NULL);  
		OrgPicRgn GetScanRgnFromCtrls();
		void InitScanRgnCtrlsFields(const OrgPicRgn&);
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
	};

	class PicWnd: public CWnd
	{
		friend struct AccumHelper;

		enum ScanRgnDrawModes { DRAW, ERASE };
		class c_ScanRgn: public AvaPicRgn
		{
		protected:
			BOOL ToErase; 
			AvaPicRgn last;
			CPoint curL, curR;

			void Draw(BMPanvas* bmp, const AvaPicRgn& rgn, ScanRgnDrawModes mode );
		public:
			c_ScanRgn() { ToErase=FALSE; }
			virtual void Draw(BMPanvas* Parent);
			virtual void Erase(BMPanvas * canvas);		
			c_ScanRgn& operator= (const AvaPicRgn& rgn) { *((AvaPicRgn*)this) = rgn; return *this; }
		};

	protected:
		CButton CaptureButton;
		CMenu menu1; c_ScanRgn ScanRgn;		
		CList<BaseForHelper*> helpers; 

		AvaPicRgn Convert( OrgPicRgn&);
		OrgPicRgn Convert( AvaPicRgn&);
		BOOL IsRgnInAva( const AvaPicRgn& );	
		HRESULT ValidatePicRgn( CRect& rgn, BMPanvas& ref );
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
		void SetScanRgn( OrgPicRgn& rgn);
		HRESULT ValidateOrgPicRgn(OrgPicRgn&);
		HRESULT ValidateAvaPicRgn(AvaPicRgn&);		

		DECLARE_MESSAGE_MAP()
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnPaint();
		afx_msg void OnDropFiles(HDROP hDropInfo);	
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnCaptureButton();
		LRESULT OnCaptureReady( WPARAM wParam, LPARAM lParam );
		afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
		afx_msg void OnMove(int x, int y);	
		void ConvertOrgToGrayscale();
	};
	
	class c_ScanRgn: public OrgPicRgn
	{
	public:
		c_ScanRgn() {}
		void Draw();
		c_ScanRgn& operator= (const OrgPicRgn& rgn) { *((OrgPicRgn*)this) = rgn; return *this;}
	};
	
	DECLARE_DYNAMIC(ImageWnd)
protected:
	PicWnd dark, cupol, strips;
	c_ScanRgn ScanRgn;
public:
	CtrlsTab Ctrls;
	CaptureWnd	CameraWnd;

	ImageWnd();
	virtual ~ImageWnd();
protected:
	DECLARE_MESSAGE_MAP()
public:
	void OnChildMove();
	void SetScanRgn(const OrgPicRgn&);
	void * GetChartFromParent();
	c_ScanRgn GetScanRgn() const {return ScanRgn;}

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


