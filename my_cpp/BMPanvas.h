// BMPanvas.h: interface for the BMPanvas class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BMPANVAS_H__1D024EEE_E5D6_4B1B_A5A4_3AE8B8FABC20__INCLUDED_)
#define AFX_BMPANVAS_H__1D024EEE_E5D6_4B1B_A5A4_3AE8B8FABC20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gdiplus.h>
using namespace Gdiplus;

#include "atlimage.h"
#include "mythread\mytime.h"
#include "afxmt.h"
#include "mythread\\ProtectedObject.h"

#define TOP_LEFT CPoint(0,0)

enum BMPanvasDetachMode {BMP_KEEP, BMP_DETACH};

struct BMPINFO
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];

		void operator=(BMPINFO& src)
		{
			bmiHeader=src.bmiHeader;		
		};
		BMPINFO() 
		{
			bmiHeader.biSize=40;
			bmiHeader.biPlanes=1;	
			bmiHeader.biClrUsed=0;
			bmiHeader.biClrImportant=0;
			bmiHeader.biCompression=0;
			bmiHeader.biSizeImage=0;
			memset(bmiColors,0,256*sizeof(RGBQUAD));
		};
		BMPINFO(BITMAP src)
		{			
			bmiHeader.biSize=40;
			bmiHeader.biPlanes=1;	
			bmiHeader.biClrUsed=0;
			bmiHeader.biClrImportant=0;
			bmiHeader.biCompression=0;
			bmiHeader.biSizeImage=0;
			memset(bmiColors,0,256*sizeof(RGBQUAD));

			bmiHeader.biWidth=src.bmWidth;
			bmiHeader.biHeight=src.bmHeight;
			bmiHeader.biBitCount=src.bmBitsPixel;			
		};
	};

class BMPanvas
{
protected:
    HDC hdc;
	HBITMAP hbmp,last_hbmp;

	int GetBitmap();
	int CheckColorType();
public:
	enum {COL_ERROR=0,BW=1,COLOR_PAL=256,GRAY_PAL=8,TRUE_COLOR=24, RGBA=32} ColorType;
	enum {READ,WRITE};
	enum {MIN_SCANLINE=-1,MAX_SCANLINE=-1};
	BMPINFO INFO;
	LPBITMAPINFO lpbi;
	BITMAP bitmap;
	BYTE* tags; int tags_size;

	MyTimer Timer1; 	
	long &w,&h,&wbyte; WORD& bpp; int arr_size;
	CRect Rgn; BYTE *arr; CString FileName;
	
	BMPanvas();	
	virtual ~BMPanvas();

	int Create(BMPanvas *src,CRect rgn);
	int Create(HDC src, BMPINFO*  info);
	int Create(HDC src, int w,int h,int bpp);
	int Create(CWnd *wnd, int w,int h,int bpp);
	int Destroy();	

	HRESULT LoadImage(CString name);		
	int LoadBitmapArray(int hmin=MIN_SCANLINE,int hmax=MAX_SCANLINE);
	void UnloadBitmapArray();
	int SetBitmapArray(int hmin=MIN_SCANLINE,int hmax=MAX_SCANLINE,void *buf=NULL);

//	enum BMPStatus {BMP_ERROR, BMP_EMPTY, BMP_LOAD} status;	
	
	int CreateGrayPallete();
	int PatBlt(DWORD mode);
	int SetBkMode(DWORD mode);
	int CopyPallete(BMPanvas *src);
	int CopyTo(BMPanvas* dest, CRect destRect, CPoint srcXY=TOP_LEFT);
	int CopyTo(HDC dest, CRect destRect, CPoint srcXY=TOP_LEFT);
	int CopyTo(BMPanvas* dest, CPoint destXY, CRect srcXY);
	int CopyTo(HDC dest, CPoint destXY, CRect srcXY);
	int CopyTo(BMPanvas* dest, CPoint destXY);	
	int CopyTo(HDC dest, CPoint destXY);
	BOOL StretchTo(BMPanvas* hdcDest, CRect dest, CRect src,	DWORD dwRop);
	BOOL StretchTo( HDC hdc, CRect dest, CRect src, DWORD dwRop );
	HDC GetDC(void);
	HBITMAP GetHBMP(BMPanvasDetachMode detach);
	int LoadResImg(UINT nIDRes);
	HBITMAP Attach(HBITMAP _hbmp);
	HGDIOBJ SelectObject(HGDIOBJ hgdiobj);
	CPoint GetCurrentPosition();
	BOOL MoveTo(CPoint pnt);
	BOOL LineTo(CPoint pnt);
	BOOL MoveTo( int x,int y );
	BOOL LineTo( int x,int y );
	BOOL Ellipse(int t,int l,int r,int b);
	BOOL Rectangle(int l, int t, int r, int b);
	BOOL Rectangle(CPoint& tl, CPoint& br);
	BOOL Rectangle(const CRect r);
	int SetROP2(int fnDrawMode);

	BOOL TextOut(int nXStart, int nYStart, CString str);
	CSize GetTextExtent(CString str);
	BOOL HasImage() const;
	COLORREF SetTextColor( COLORREF col);
	int GetPallete(RGBQUAD **pColors, int first=0,int total=256);
	int SetPallete(RGBQUAD *pColors, int first=0,int total=256);
	BOOL SetWorldTransform(XFORM *lpXform);
	int SetGraphicsMode(int iMode);
	HRESULT SaveImage(CString name);
	BOOL BMPanvas::operator!=(BMPanvas& bmp);
};

BITMAPINFOHEADER GetBMPInfo(CString name);

typedef ProtectedObjectX<BMPanvas> ProtectedBMPanvas;
typedef ProtectorX<BMPanvas> BMPanvasGuard;



#endif // !defined(AFX_BMPANVAS_H__1D024EEE_E5D6_4B1B_A5A4_3AE8B8FABC20__INCLUDED_)
