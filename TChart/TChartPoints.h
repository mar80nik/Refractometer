// ScaterErrorPoint.h: interface for the ScaterErrorPoint class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "TChartGlobal.h"


//CHNL1=...Y01X,CHNL2=...Y10X, BCKG=...YXX1, NON-BCKG=...YXX0
//T type=...YYYXXX
enum PointChnlType {mCHNL=6, shCHNL=1, KEEP_CHNL=0, CHNL0=1,CHNL1=2};
enum PointBckgType {mBCKG=1, shBCKG=0, NON_BCKG=0, BCKG=1, KEEP_BCKG=2 };
enum PointTypeID {mType=0x38, shType=3, KEEP_TYPE=0, 
	ErrorPntType=0, ChanelPntType, TransmitPnt, GenericPnt, AveragePnt, DivisionError};

class PointTypes
{
	friend class AbstractPoint2D;
protected: 	int val;
public: 
	PointTypes() {val=0;}
	bool operator==(PointTypes& p) {return (val==p.val);}
	PointTypes& operator=(const PointTypes& p) {val = p.val; return *this;}
	int GetChannel() {return ((val&mCHNL)>>shCHNL);};
	void Set(PointTypeID id, PointChnlType chnl=KEEP_CHNL, PointBckgType bckg=KEEP_BCKG) 
	{
		if(id!=KEEP_TYPE) {val&=(~mType); val|=(id<<shType);}
		if(chnl!=KEEP_CHNL) {val&=(~mCHNL); val|=(chnl<<shCHNL);}
		if(bckg!=KEEP_BCKG) {val&=(~mBCKG); val|=(bckg<<shBCKG);}
		return;
	}
	bool IsBckg() {return (bool)(val&mBCKG);}
	BOOL IsT() {return (BOOL)(val&mType);}
	int Get() {return ((val&mType)>>shType);}
	operator int() {return val;}
	virtual void Serialize(CArchive& ar);	
};

class AbstractPoint2D
{	
public:
	PointTypes type;
	AbstractPoint2D(int tag=0) {type.val=tag;}
	virtual void Serialize(CArchive& ar);
	BOOL IsVisible() 
	{
		return (type.IsBckg() ? 0:1);
	}
};

class Point1D: public AbstractPoint2D
{
public:
	double x;

	Point1D(double _x=0, AbstractPoint2D& type=AbstractPoint2D()): x(_x), AbstractPoint2D(type) {}
	~Point1D() {}
	virtual void Serialize(CArchive& ar);	
};

class SimplePoint: public Point1D
{
public:
	double y;

	virtual void Serialize(CArchive& ar);	
	explicit SimplePoint()  {x=y=0;};
	SimplePoint(CPoint& pnt) {x=pnt.x; y=pnt.y;}
	SimplePoint(double _x, double _y=0): Point1D(_x), y(_y)  {};
	SimplePoint(Point1D& _x, double _y=0): Point1D(_x), y(_y)  {};
	virtual void Clear(){x=y=0;};
	CString Format();
	SimplePoint operator-(SimplePoint& pnt) {SimplePoint ret; ret.x=x-pnt.x; ret.y=y-pnt.y; return ret;}

};

class PointVsError : public SimplePoint
{
public:
	double dy;

	PointVsError operator - (PointVsError pnt);
	PointVsError operator + (PointVsError pnt);	
	PointVsError operator / (PointVsError pnt);	
	PointVsError operator * (double a);	
	void operator *= (double a);
	void operator -=(PointVsError pnt);

	virtual void Serialize(CArchive& ar);	
	explicit PointVsError()  {x=y=dy=0;};
	PointVsError(double _x, double _y=0, double _dy=0): SimplePoint(_x,_y), dy(_dy)  {};
	PointVsError(SimplePoint& pnt, double _dy): SimplePoint(pnt), dy(_dy) {};	
	PointVsError(Point1D& pnt, double _y, double _dy): SimplePoint(Point1D(pnt), _y), dy(_dy) {};
	virtual void Clear(){x=y=dy=0;};
	CString Format();
};

class CPointVsError: public CPoint
{
public:
	int dy;
	CPointVsError() {x=y=dy=0;};
	CPointVsError& operator=(CPoint& pnt) {x=pnt.x; y=pnt.y; return (*this);}
};


class ValuesAccumulator: public AbstractPoint2D
{
public:	
	unsigned short n;
protected:
	double sum, sum2;
	double last_mean, last_sd;	
	int FLAGS;
public:
	ValuesAccumulator() {sum=sum2=last_mean=last_sd=0;n=0;}
	virtual double GetSD();
	virtual double GetMean();	
	virtual ValuesAccumulator& operator << (double a);	
	virtual void operator *=(double a);
	virtual void Serialize(CArchive& ar);
	virtual void Clear();
	virtual operator PointVsError() { return PointVsError(SimplePoint(Point1D(0,*this)),GetMean(),GetSD()); }
};


class AbstractPoint2DArray: public CArray<AbstractPoint2D>
{
public:
	AbstractPoint2DArray() {}
	~AbstractPoint2DArray() {}
	virtual void Serialize(CArchive& ar);
};

class Point1DArray: public AbstractPoint2DArray
{
public:
	DoubleArray x;

	Point1DArray() {}
	~Point1DArray() {}
	Point1D operator[](int n);
	int Add(Point1D& pnt);
	double* GetX();
	virtual void SetSize(int n);
	virtual int CopyFrom( const Point1DArray& src, UINT total=0, UINT src_shift=0 );
	virtual void RemoveAll() {x.RemoveAll(); AbstractPoint2DArray::RemoveAll(); }
	virtual void RemoveAt(int n) {x.RemoveAt(n); AbstractPoint2DArray::RemoveAt(n); }
	virtual void Serialize(CArchive& ar);
};

class SimplePointArray: public Point1DArray
{
public:
	DoubleArray y;

	SimplePointArray() {};
	virtual ~SimplePointArray() {};
	SimplePoint operator[](int n);
	int Add(SimplePoint& pnt);
	double* GetY();
	virtual void SetSize(int n);
	virtual int CopyFrom( const SimplePointArray& src, UINT total=0, UINT src_shift=0 );
	virtual void RemoveAll() {y.RemoveAll(); Point1DArray::RemoveAll(); }
	virtual void RemoveAt(int n) {y.RemoveAt(n); Point1DArray::RemoveAt(n); }
	virtual void Serialize(CArchive& ar);
};
class PointVsErrorArray: public SimplePointArray
{
public:
	DoubleArray dy;

	PointVsErrorArray() {};
	virtual ~PointVsErrorArray() {};
	PointVsError operator[](int n);
	int Add(PointVsError& pnt);
	double* GetdY();
	virtual void SetSize(int n);
	virtual int CopyFrom( const PointVsErrorArray& src, UINT total=0, UINT src_shift=0 );
	virtual void RemoveAll() {dy.RemoveAll(); SimplePointArray::RemoveAll(); }
	virtual void RemoveAt(int n) {dy.RemoveAt(n); SimplePointArray::RemoveAt(n); }
	virtual void Serialize(CArchive& ar);
};

//////////////////////////////////////////////////////////////////////////
struct LineStyleStyleParams 
{
	ChartLineStyles style;
	LineStyleStyleParams(ChartLineStyles _style=STRAIGHT) 
	{
		style=_style;
	}
	void Serialize(CArchive& ar);
};
class _LineStyle: public LineStyleStyleParams 
{
protected:	
	void UpdateStyle();
public:
	void (_LineStyle::*Draw)(BMPanvas* img,CPoint& t);
	void (_LineStyle::*IconDraw)(BMPanvas& icon);
	//-------------------------------------------
	_LineStyle(LineStyleStyleParams prms) 
	{
		Set(prms); 
	}	
	void Set(LineStyleStyleParams style);

	void Serialize(CArchive& ar) {LineStyleStyleParams::Serialize(ar); UpdateStyle(); };
protected:
	void DrawStraight(BMPanvas* img,CPoint& t);
	void IconDrawStraight(BMPanvas& img);	
};
//////////////////////////////////////////////////////////////////////////
struct SymbolStyleStyleParams 
{
	int dx,dy;
	ChartSymbolStyles style;
	SymbolStyleStyleParams(ChartSymbolStyles _style=CIRCLE, int _dx=4, int _dy=4) 
	{
		dx=_dx; dy=_dy; style=_style;
	}
	void Serialize(CArchive& ar);
};
class _SymbolStyle: public SymbolStyleStyleParams
{
protected:	
	void UpdateStyle();	
public:

	void (_SymbolStyle::*Draw)(BMPanvas* img,CPoint& t);
	void (_SymbolStyle::*IconDraw)(BMPanvas& icon);
//-------------------------------------------
	_SymbolStyle(SymbolStyleStyleParams& prms) 
		{ Set(prms); }	
	void Set(SymbolStyleStyleParams style);
	void Serialize(CArchive& ar) {SymbolStyleStyleParams::Serialize(ar); UpdateStyle(); };
protected:
	void DrawCircle(BMPanvas* img,CPoint& t);
	void IconDrawCircle(BMPanvas& icon);	
	void DrawCross45(BMPanvas* img,CPoint& t);
	void IconDrawCross45(BMPanvas& icon);	
	void DrawVertLine(BMPanvas* img,CPoint& t);
	void IconDrawVertLine(BMPanvas& icon);	
};
//////////////////////////////////////////////////////////////////////////
struct ErrorBarStyleParams 
{
	int CapWidth;
	ChartErrorBarStyles style;
	ErrorBarStyleParams(ChartErrorBarStyles _style=NO_BARS, int cap=2) { CapWidth=cap; style=_style; }
	void Serialize(CArchive& ar);
};
template<class C1>
class _ErrorBarStyle: public ErrorBarStyleParams
{
protected:
	void UpdateStyle()
	{
		switch (style)
		{
		case POINTvsERROR_BAR: Draw=&_ErrorBarStyle<C1>::DrawPointVsErrorBar; IconDraw=&_ErrorBarStyle<C1>::IconDrawPointVsErrorBar; break;
		default: Draw=NULL; IconDraw=NULL;
		}
	}
public:
	void (_ErrorBarStyle::*Draw)(BMPanvas* img,C1& t);
	void (_ErrorBarStyle::*IconDraw)(BMPanvas& icon);
//-------------------------------------------
public:
	_ErrorBarStyle(ErrorBarStyleParams prms) { Set(prms); }
	void Set(ErrorBarStyleParams style)
	{
		*((ErrorBarStyleParams*)this)=style; UpdateStyle();
	}
	void Serialize(CArchive& ar) {ErrorBarStyleParams::Serialize(ar); UpdateStyle(); };
protected:
	void DrawPointVsErrorBar(BMPanvas* img,C1& t);
	void IconDrawPointVsErrorBar(BMPanvas& icon)
	{
		CPointVsError t; t.x=icon.w/2; t.y=icon.h/2; t.dy=(int)(0.8*icon.h/2);
		int temp_capwidth=CapWidth; CapWidth=icon.w/4;
		DrawPointVsErrorBar(&icon,t);
		CapWidth=temp_capwidth;
	}
}; 

typedef _ErrorBarStyle<CPoint> NoErrorBarsStyle;
typedef _ErrorBarStyle<CPointVsError> VertErrorBarsStyle;
//////////////////////////////////////////////////////////////////////////
struct ChartSeriesStyleParams: public LineStyleStyleParams, public SymbolStyleStyleParams, public ErrorBarStyleParams
{};
//////////////////////////////////////////////////////////////////////////

struct LogicRect
{
	double l,r,b,t;
	LogicRect() {l=r=b=t=0;}
	double Width() {return fabs(r-l);}
	double Height() {return fabs(t-b);}
};

class ChartRender
{
protected:
	XFORM trsfm, ident;
	BOOL init;	
	TChartInterval X,Y; CRect area;
	CSize size; CPoint Origin;
	ChartRendersID UID;
	int modifID;
public:
	ChartRender();
	void Set(TChartInterval X, TChartInterval Y,CRect area);
	operator XFORM();

	int LogicToScreenX(double& x);
	int LogicToScreenY(double& y);
	CPoint LogicToScreen(SimplePoint& pnt);
	SimplePoint ScreenToLogic(CPoint& pnt);
	CPointVsError LogicToScreen(PointVsError& pnt);

	LogicRect ZoomIN(SimplePoint cntr, double factor);
	LogicRect ZoomOUT(SimplePoint cntr, double factor);
	ChartRendersID GetUID() {return UID;}
	CSize GetScreenSize() {return size;};
	BOOL IsInit() {return init;}
	int GetModificationID() {return modifID;}
};