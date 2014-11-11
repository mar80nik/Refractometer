// BMPanvas.cpp: implementation of the BMPanvas class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "testWMF.h"
#include "BMPanvas.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BMPanvas::BMPanvas():
	w(INFO.bmiHeader.biWidth),h(INFO.bmiHeader.biHeight),wbyte(bitmap.bmWidthBytes), bpp(INFO.bmiHeader.biBitCount)
{
	w=-1; h=-1; hdc=NULL; hbmp=last_hbmp=NULL;
	INFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	INFO.bmiHeader.biCompression=BI_RGB;
	INFO.bmiHeader.biSizeImage=0; 
	INFO.bmiHeader.biClrImportant=INFO.bmiHeader.biClrUsed=0;
	lpbi=(LPBITMAPINFO)(&INFO); arr=0;
	Rgn=CRect(0,0,0,0);
	FileName="NoName"; arr_size=0;
	ColorType=COL_ERROR; 
	tags_size=100; tags=NULL; tags=new BYTE[tags_size];
}

BMPanvas::~BMPanvas()
{
	Destroy();
	if(hdc!=NULL) DeleteDC(hdc);
	if(arr) delete[] arr;
	if(tags!=NULL) {delete[] tags; tags=NULL;}
}

int BMPanvas::Destroy()
{
	int ret=0; 
	if(last_hbmp!=NULL) SelectObject(last_hbmp);	
	if( (ret=DeleteObject(hbmp)) ) hbmp=NULL;
	ColorType=COL_ERROR;
	w=h=0; 
	UnloadBitmapArray(); 	
	return ret;
}

//******************************************
int BMPanvas::Create(BMPanvas *src,CRect rgn)
{	
    Destroy(); 
	if((hdc=CreateCompatibleDC(src->hdc))!=NULL)
	{
		if( (hbmp=CreateCompatibleBitmap( src->hdc,rgn.Width(),rgn.Height() ) )!=NULL)
		{
			last_hbmp=(HBITMAP)SelectObject(hbmp); 
			GetBitmap();	
			Rgn=rgn;
		}
		else ASSERT(0);
	}
	else ASSERT(0);
	return (int)hbmp;	
}

int BMPanvas::Create(HDC src, BMPINFO*  info)
{		
    Destroy();
	if((hdc=CreateCompatibleDC(src))!=NULL)
	{
		hbmp=CreateDIBSection(hdc,(LPBITMAPINFO)(info),DIB_RGB_COLORS,0,0,0); 	
		last_hbmp=(HBITMAP)SelectObject(hbmp); GetBitmap();		
		Rgn=CRect(0,0,w,h);
	}
	else ASSERT(0);
	return (int)hbmp;
}

int BMPanvas::Create(CWnd *wnd, int w,int h,int bpp)
{		
	Destroy();
	CDC *src=wnd->GetDC();
	if((hdc=CreateCompatibleDC(src->m_hDC))!=NULL)
	{
		BMPINFO info; info.bmiHeader.biWidth=w; info.bmiHeader.biHeight=h; info.bmiHeader.biBitCount=(WORD)bpp;
		hbmp=CreateDIBSection(hdc,(LPBITMAPINFO)(&info),DIB_RGB_COLORS,0,0,0); 	
		last_hbmp=(HBITMAP)SelectObject(hbmp); GetBitmap();		
		Rgn=CRect(0,0,w,h);
	}
	else ASSERT(0);
	wnd->ReleaseDC(src);
	return (int)hbmp;
}

int BMPanvas::Create(HDC src, int w,int h,int bpp)
{		
	Destroy();
	if((hdc=CreateCompatibleDC(src))!=NULL)
	{
		BMPINFO info; info.bmiHeader.biWidth=w; info.bmiHeader.biHeight=h; info.bmiHeader.biBitCount=(WORD)bpp;
		hbmp=CreateDIBSection(hdc,(LPBITMAPINFO)(&info),DIB_RGB_COLORS,0,0,0); 	
		last_hbmp=(HBITMAP)SelectObject(hbmp); GetBitmap();		
		Rgn=CRect(0,0,w,h);
	}
	else ASSERT(0);
	return (int)hbmp;
}
//******************************************
HRESULT BMPanvas::LoadImage( CString name )
{	
	HRESULT ret; CImage TI; 	
	if((ret=TI.Load(name))==S_OK)
	{		
		HDC thdc=TI.GetDC(); hdc=CreateCompatibleDC(thdc); TI.ReleaseDC();		
		if(hdc!=NULL)
		{	
			Attach(TI.Detach());
			FileName=name;
		}
	}
	return ret;
}

HRESULT BMPanvas::SaveImage(CString name)
{	
	HRESULT ret = E_FAIL; CImage TI; 	
	if(HasImage())
	{		
		TI.Attach(GetHBMP(BMP_DETACH));
		ret=TI.Save(name,ImageFormatPNG);
		Attach(TI.Detach());
	}
	return ret;
}

void BMPanvas::UnloadBitmapArray() {if(arr) {delete[] arr;arr=0;}}

int BMPanvas::CopyTo(BMPanvas* dest, CRect destRect, CPoint srcXY)
{
	int ret; 
	ret=BitBlt(dest->hdc,destRect.left,destRect.top,destRect.Width(),destRect.Height(),hdc,srcXY.x,srcXY.y,SRCCOPY);					
	return ret;
}

int BMPanvas::CopyTo(HDC dest, CRect destRect, CPoint srcXY)
{
	int ret; 
	ret=BitBlt(dest,destRect.left,destRect.top,destRect.Width(),destRect.Height(),hdc,srcXY.x,srcXY.y,SRCCOPY);					
	return ret;
}

int BMPanvas::CopyTo(BMPanvas* dest, CPoint destXY, CRect srcXY)
{
	int ret;
	ret=BitBlt(dest->hdc,destXY.x,destXY.y,srcXY.Width(),srcXY.Height(),hdc,srcXY.left,srcXY.top,SRCCOPY);					
	return ret;
}

int BMPanvas::CopyTo(HDC dest, CPoint destXY, CRect srcXY)
{
	int ret;
	ret=BitBlt(dest,destXY.x,destXY.y,srcXY.Width(),srcXY.Height(),hdc,srcXY.left,srcXY.top,SRCCOPY);					
	return ret;
}

int BMPanvas::CopyTo(BMPanvas* dest, CPoint destXY)
{
	int ret;
	ret=BitBlt(dest->hdc,destXY.x,destXY.y,w,h,hdc,0,0,SRCCOPY);					
	return ret;
}

int BMPanvas::CopyTo(HDC dest, CPoint destXY)
{
	int ret;
	ret=BitBlt(dest,destXY.x,destXY.y,w,h,hdc,0,0,SRCCOPY);					
	return ret;
}

int BMPanvas::GetBitmap()
{	
	int ret=0;
	GetObject(hbmp,sizeof(bitmap),&bitmap);
    
	INFO.bmiHeader.biWidth=bitmap.bmWidth;
	INFO.bmiHeader.biHeight=bitmap.bmHeight;
	INFO.bmiHeader.biBitCount=bitmap.bmBitsPixel;
	INFO.bmiHeader.biPlanes=bitmap.bmPlanes;	
	CheckColorType();
	Rgn=CRect(Rgn.TopLeft(),CSize(w,h));

	return ret;
}


int BMPanvas::LoadBitmapArray(int hmin,int hmax)
{
	int ret=0;
	if(hmin==MIN_SCANLINE) hmin=0;
	if(hmax==MAX_SCANLINE) hmax=h-1;
	if(hmax<hmin) {int t=hmax; hmax=hmin; hmin=t;}
	UnloadBitmapArray();	

	arr_size=(wbyte)*(hmax-hmin+1)+1;
	arr=new BYTE[arr_size];	

	ret=GetDIBits(hdc,hbmp,h-1-hmax,hmax-hmin+1,arr,lpbi,DIB_RGB_COLORS);
	GetDIBColorTable(hdc,0,256,INFO.bmiColors);					
	return ret;
}

int BMPanvas::SetBitmapArray(int hmin,int hmax,void *buf)
{
	if(buf==NULL) buf=arr;
	if(buf==NULL) return 0;
	int ret=0;	
	
	if(hmin==MIN_SCANLINE) hmin=0;
	if(hmax==MAX_SCANLINE) hmax=h-1;

	ret=SetDIBits(hdc,hbmp,h-1-hmax,hmax-hmin+1,buf,lpbi,DIB_RGB_COLORS);
	return ret;
}


int BMPanvas::CreateGrayPallete()
{
	int ret=0;	
	RGBQUAD col; 
	for(int i=0;i<256;i++) {col.rgbBlue=col.rgbGreen=col.rgbRed=(BYTE)i; INFO.bmiColors[i]=col;}
	ret=SetDIBColorTable(hdc,0,256,INFO.bmiColors);
	CheckColorType();
	return ret;
}

int BMPanvas::GetPallete(RGBQUAD **pColors, int first,int total)
{
	if(*pColors==NULL) *pColors=new RGBQUAD[total];
	return GetDIBColorTable(hdc,first,total,*pColors);
}

int BMPanvas::SetPallete(RGBQUAD *pColors, int first,int total)
{	
	int ret=SetDIBColorTable(hdc,first,total,pColors);
	CheckColorType();
	return ret;
}


int BMPanvas::CopyPallete(BMPanvas *src)
{
	int ret=0;	
	ret+=GetDIBColorTable(src->hdc,0,256,INFO.bmiColors);
	ret+=SetDIBColorTable(hdc,0,256,INFO.bmiColors);
	CheckColorType();
	return ret;
}

int BMPanvas::CheckColorType()
{
	int ret=0;	

    	RGBQUAD pal[256]; int i,k,j;
		switch(INFO.bmiHeader.biBitCount)
		{
		case 32: ColorType=RGBA; break;
		case 24: ColorType=TRUE_COLOR; break;
		case 8:		
			GetDIBColorTable(hdc,0,256,pal);		
			for(i=0,k=0,j=0;i<256;i++) 
			{				
				if(pal[i].rgbBlue==i && pal[i].rgbGreen==i && pal[i].rgbRed==i) k++;			
				if(pal[i].rgbBlue==0 && pal[i].rgbGreen==0 && pal[i].rgbRed==0) j++;			
			}
			ColorType=COLOR_PAL; 
			if(k==256) ColorType=GRAY_PAL;
			if(j==255) ColorType=BW;
			break;
		case 1:
			ColorType=BW; break;
		default:
			ColorType=COL_ERROR;	
		}
	
    return ret;
}

HDC BMPanvas::GetDC(void) {return hdc;}

HBITMAP BMPanvas::GetHBMP( BMPanvasDetachMode detach )
{
	HBITMAP ret = NULL;
	if(detach==BMP_DETACH)
	{
		ret=(HBITMAP)SelectObject(last_hbmp); last_hbmp=NULL;
		ASSERT(ret==hbmp);
		hbmp = NULL;
	}
	return ret;
}

int BMPanvas::PatBlt( DWORD mode ) {return ::PatBlt(GetDC(),0,0,w,h,mode);}

int BMPanvas::LoadResImg( UINT nIDRes )
{
	Destroy();
	hdc=CreateCompatibleDC(NULL); 
	CString name; name.Format("#%d",nIDRes);
	HINSTANCE hinst=AfxFindResourceHandle(name, RT_BITMAP);
	hbmp=LoadBitmap(hinst, name);
	if(hbmp!=NULL)
	{
		last_hbmp=(HBITMAP)SelectObject(hbmp);   
		GetBitmap();	
	}	
	return (int)hbmp;
}

HGDIOBJ BMPanvas::SelectObject(HGDIOBJ obj)
{
	return ::SelectObject(hdc,obj);
}

BOOL BMPanvas::MoveTo( CPoint pnt ) {return MoveToEx(hdc,pnt.x,pnt.y,NULL);}
BOOL BMPanvas::LineTo( CPoint pnt ) {return ::LineTo(hdc,pnt.x,pnt.y);}
BOOL BMPanvas::MoveTo( int x,int y ) {return MoveToEx(hdc,x,y,NULL);}
BOOL BMPanvas::LineTo( int x,int y ) {return ::LineTo(hdc,x,y);}

BOOL BMPanvas::Ellipse( int t,int l,int r,int b ) {return ::Ellipse(hdc,t,l,r,b);}

int BMPanvas::SetBkMode( DWORD mode ) {return ::SetBkMode(hdc,mode);}

BOOL BMPanvas::TextOut(int x, int y, CString str ) {return ::TextOut(hdc,x,y,str,str.GetLength());}

COLORREF BMPanvas::SetTextColor( COLORREF col) {return ::SetTextColor(hdc,col);}

CSize BMPanvas::GetTextExtent( CString str )
{
	CSize ret; GetTextExtentPoint32(hdc,str,str.GetLength(),&ret);
	return ret;
}

BOOL BMPanvas::Rectangle( int l, int t, int r, int b ) {return ::Rectangle(hdc,l,t,r,b);}
BOOL BMPanvas::Rectangle( const CRect r ){return ::Rectangle(hdc,r.left,r.top,r.right,r.bottom);}
BOOL BMPanvas::Rectangle( CPoint& tl, CPoint& br ) {return ::Rectangle(hdc,tl.x,tl.y,br.x,br.y);}

BOOL BMPanvas::StretchTo( BMPanvas* Dest, CRect dest, CRect src, DWORD dwRop )
{
    return StretchBlt(	Dest->hdc,
						dest.left,dest.top,dest.Width(),dest.Height(),
						hdc,
						src.left,src.top,src.Width(),src.Height(),
						dwRop);
}
BOOL BMPanvas::StretchTo( HDC hdc, CRect dest, CRect src, DWORD dwRop )
{
	return StretchBlt(	hdc,
		dest.left,dest.top,dest.Width(),dest.Height(),
		hdc,
		src.left,src.top,src.Width(),src.Height(),
		dwRop);
}

HBITMAP BMPanvas::Attach( HBITMAP _hbmp )
{	
	if(hbmp!=NULL)
	{
		HBITMAP t=(HBITMAP)SelectObject(_hbmp);
		DeleteObject(t);
	}
	else
	{
		last_hbmp=(HBITMAP)SelectObject(_hbmp);   
	}
	hbmp=_hbmp; 
	GetBitmap();
	return hbmp;
}

BOOL BMPanvas::HasImage() const
{
	return ( (hbmp!=NULL && hdc!=NULL) ? 1:0);
}

BOOL BMPanvas::SetWorldTransform( XFORM *lpXform ) {return ::SetWorldTransform(hdc,lpXform);}
int BMPanvas::SetGraphicsMode( int iMode ) {return ::SetGraphicsMode(hdc,iMode);}

CPoint BMPanvas::GetCurrentPosition()
{
	CPoint ret;
	GetCurrentPositionEx(hdc,&ret);
	return ret;
}

int BMPanvas::SetROP2( int fnDrawMode )
{
	return ::SetROP2(hdc,fnDrawMode);
}

BOOL BMPanvas::operator!=(BMPanvas& bmp)
{
	return !(w==bmp.w && h==bmp.h && bpp==bmp.bpp);
}