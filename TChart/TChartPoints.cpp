// ScaterErrorPoint.cpp: implementation of the ScaterErrorPoint class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TChartPoints.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

double ValuesAccumulator::GetMean()
{
	if(GET(FLAGS,MEAN_CALC)) {last_mean=sum/n;RST(FLAGS,MEAN_CALC);}
	return last_mean;
}

double ValuesAccumulator::GetSD()
{
	if(GET(FLAGS,SD_CALC))
	{
		last_sd=sqrt(fabs(sum2*n-sum*sum)/(n*(n-1)));
		RST(FLAGS,SD_CALC);
	}
	return last_sd;
}
ValuesAccumulator& ValuesAccumulator::operator << (double a)
{
	sum+=a; sum2+=a*a; n++; SET(FLAGS,MEAN_CALC | SD_CALC);
	return (*this);
}

void ValuesAccumulator::operator *=(double a)
{
	sum*=a;sum2*=a*a; SET(FLAGS,MEAN_CALC | SD_CALC);
}
void ValuesAccumulator::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << sum << sum2 << n;
	}
	else
	{
		ar >> sum >> sum2 >> n; FLAGS=0; SET(FLAGS,MEAN_CALC | SD_CALC);
	}			
}
void ValuesAccumulator::Clear()
{
	sum=sum2=last_mean=last_sd=0; n=0; FLAGS=0;
}

//////////////////////////////////////////////////////////////////////
PointVsError PointVsError::operator +(PointVsError pnt)
{
	ASSERT(type==pnt.type);
	PointVsError temp;
	temp.x=x;
	temp.y=y+pnt.y;
	temp.dy=sqrt(dy*dy+pnt.dy*pnt.dy);
	return temp;
}

PointVsError PointVsError::operator -(PointVsError pnt)
{
	ASSERT(type==pnt.type);
	PointVsError temp;
	temp.x=x;
	temp.y=y-pnt.y;
	temp.dy=sqrt(dy*dy+pnt.dy*pnt.dy);
	return temp;
}

PointVsError PointVsError::operator /(PointVsError pnt)
{
	ASSERT(type==pnt.type);
	PointVsError temp;
	temp.x=x; double t1,t2;
	if(pnt.y!=0)
	{
		temp.y=y/pnt.y; t1=dy/pnt.y; t2=temp.y*pnt.dy/pnt.y;
		temp.dy=sqrt(t1*t1+t2*t2);
	}
	else
	{
		temp.y = temp.dy = 0; 
		temp.type.Set(DivisionError);
	}
	return temp;
}

PointVsError PointVsError::operator * (double a)
{
	PointVsError temp;
	temp.x=x;
	temp.y*=a;
	temp.dy*=a;
	return temp;
}

void PointVsError::Serialize(CArchive& ar)
{
	SimplePoint::Serialize(ar);
	if (ar.IsStoring()) ar << dy; else ar >> dy; 			
}

void PointVsError::operator*=( double a ) {y*=a;dy*=a;}

void PointVsError::operator-=( PointVsError pnt )
{
	ASSERT(type==pnt.type);
	y-=pnt.y;
	dy=sqrt(dy*dy+pnt.dy*pnt.dy);
}

CString PointVsError::Format()
{
	CString ret;
	ret.Format("%4.3f%c%1.4f%c%1.6f%c",x,9,y,9,dy,9);
	return ret;
}

//--------------------------------

void PointTypes::Serialize( CArchive& ar )
{
	if (ar.IsStoring()) ar << val; else ar >> val; 			
}

void AbstractPoint2D::Serialize( CArchive& ar ) { type.Serialize(ar); }


void Point1D::Serialize( CArchive& ar )
{
	AbstractPoint2D::Serialize(ar);
	if (ar.IsStoring()) ar << x; else ar >> x; 	
}

void SimplePoint::Serialize( CArchive& ar )
{
	Point1D::Serialize(ar);
	if (ar.IsStoring()) ar << y; else ar >> y; 	
}

CString SimplePoint::Format()
{
	CString ret;
	ret.Format("%4.3f%c%1.4f%c",x,9,y,9);
	return ret;
}
//////////////////////////////////////////////////////////////////////////
ChartRender::operator XFORM()
{
	XFORM ret; double W=X.Length(),H=Y.Length();
	ret.eM12=ret.eM21=0; 
	ret.eM11=(float)(W<1e-15 ? 0:size.cx/W); 	
	ret.eM22=(float)(H<1e-15 ? 0:-size.cy/H); 
	ret.eDx=(float)(-X.min*ret.eM11); 
	ret.eDy=(float)(size.cy-Y.min*ret.eM22); 
	return ret;

}

void ChartRender::Set(TChartInterval _X, TChartInterval _Y,CRect _area)
{
	if(X==_X && Y==_Y && area==_area) return;
	X=_X; Y=_Y; area=_area; modifID++;
	size.cx=area.Width(); size.cy=area.Height(); Origin.x=area.left; Origin.y=area.top;
	trsfm=*this;
	if(trsfm.eM11==0 || trsfm.eM22==0) init=FALSE; else init=TRUE;
}

ChartRender::ChartRender()
{
	init=FALSE; UID=CHARTRENDER_DEFAULT;
	ident.eM11=ident.eM22=1; ident.eM12=ident.eM21=ident.eDx=ident.eDy=0;
	modifID=0;
}

LogicRect ChartRender::ZoomIN( SimplePoint cntr, double factor ) //factor=1...10.....
{
	LogicRect ret;
	double W=X.Length(),H=Y.Length();
	W/=factor; H/=factor;
	ret.l=cntr.x-W/2; ret.r=ret.l+W;
	ret.b=cntr.y-H/2; ret.t=ret.b+H;
	return ret;
}

CPoint ChartRender::LogicToScreen( SimplePoint& pnt )
{
	CPoint ret;
	if(init)
	{
		ret.x=LogicToScreenX(pnt.x);
		ret.y=LogicToScreenY(pnt.y);
	}
//	else ASSERT(0);
	return ret;
}

CPointVsError ChartRender::LogicToScreen( PointVsError& pnt )
{
	CPointVsError ret;
	if(init)
	{
		ret=LogicToScreen((SimplePoint&)pnt);
		ret.dy=(int)(pnt.dy*trsfm.eM22);
	}
//	else ASSERT(0);
	return ret;
}

int ChartRender::LogicToScreenX( double& x )
{
	int ret=0;
	if(init) ret=(int)(x*trsfm.eM11+trsfm.eDx+Origin.x);
//	else ASSERT(0);
	return ret;
}

int ChartRender::LogicToScreenY( double& y )
{
	int ret=0;
	if(init) ret=(int)((y*trsfm.eM22+trsfm.eDy)+Origin.y);
//	else ASSERT(0);
	return ret;
}

SimplePoint ChartRender::ScreenToLogic( CPoint& pnt )
{
	SimplePoint ret;
	if(init)
	{
		ret.x=(pnt.x-trsfm.eDx-Origin.x)/trsfm.eM11;
		ret.y=(pnt.y-trsfm.eDy-Origin.y)/trsfm.eM22;	
	}
//	else ASSERT(0);
	return ret;
}

LogicRect ChartRender::ZoomOUT( SimplePoint cntr, double factor )
{
	return ZoomIN(cntr,1/factor);
}
//////////////////////////////////////////////////////////////////////////

void _SymbolStyle::DrawCircle( BMPanvas* img,CPoint& t )
{
	img->MoveTo(t.x, t.y); img->Ellipse(t.x-dx,t.y-dy, t.x+dx,t.y+dy);
}
void _SymbolStyle::DrawCross45( BMPanvas* img,CPoint& t )
{
	img->MoveTo(t.x-dx, t.y-dy); img->LineTo(t.x+dx+1,t.y+dy+1);
	img->MoveTo(t.x-dx, t.y+dy); img->LineTo(t.x+dx+1,t.y-dy-1); img->MoveTo(t.x,t.y);	
}
void _SymbolStyle::DrawVertLine( BMPanvas* img,CPoint& t )
{
	img->MoveTo(t.x, t.y-dy); img->LineTo(t.x,t.y+dy);
}
void _SymbolStyle::Set( SymbolStyleStyleParams style ) 
{ 
	*((SymbolStyleStyleParams*)this)=style; 
	UpdateStyle(); 
}

void _SymbolStyle::UpdateStyle()
{
	switch (style)
	{
	case CIRCLE: Draw=&_SymbolStyle::DrawCircle; IconDraw=&_SymbolStyle::IconDrawCircle; break;
	case CROSS45: Draw=&_SymbolStyle::DrawCross45; IconDraw=&_SymbolStyle::IconDrawCross45; break;
	case VERT_LINE: Draw=&_SymbolStyle::DrawVertLine; IconDraw=&_SymbolStyle::IconDrawVertLine; break;
	default: Draw=NULL;
	}
}

void SymbolStyleStyleParams::Serialize( CArchive& ar )
{
	int t;
	if (ar.IsStoring()) {ar << dx <<dy <<style;}
	else 
	{ 
		ar >> dx >> dy;
		ar >> t;		
		switch (t)
		{
		case CIRCLE: style=CIRCLE; break;
		case CROSS45: style=CROSS45; break;
		case VERT_LINE: style=VERT_LINE; break;
		case NO_SYMBOL:
		default: style=NO_SYMBOL;
		}
	}
}

void _SymbolStyle::IconDrawCircle( BMPanvas& icon )
{
	CPoint t, d; t.x=icon.w/2; t.y=icon.h/2; 
	int max=(dx>dy ? dx:dy); 
	int temp_dx=dx, temp_dy=dy; dx=(int)(dx*(icon.w/4.)/max); dy=(int)(dy*(icon.h/4.)/max);
	DrawCircle(&icon, t);
	dx=temp_dx; dy=temp_dy;
}

void _SymbolStyle::IconDrawCross45( BMPanvas& icon )
{
	CPoint t, d; t.x=icon.w/2; t.y=icon.h/2; 
	int max=(dx>dy ? dx:dy); 
	int temp_dx=dx, temp_dy=dy; dx=(int)(dx*(icon.w/4.)/max); dy=(int)(dy*(icon.h/4.)/max);
	DrawCross45(&icon, t);
	dx=temp_dx; dy=temp_dy;
}
void _SymbolStyle::IconDrawVertLine( BMPanvas& icon )
{
	CPoint t, d; t.x=icon.w/2; t.y=icon.h/2; 
	int max=(dx>dy ? dx:dy); 
	int temp_dx=dx, temp_dy=dy; dx=(int)(dx*(icon.w/4.)/max); dy=(int)(dy*(icon.h/4.)/max);
	DrawVertLine(&icon, t);
	dx=temp_dx; dy=temp_dy;
}

//////////////////////////////////////////////////////////////////////////
void _LineStyle::Set( LineStyleStyleParams style ) 
{ 
	*((LineStyleStyleParams*)this)=style; 
	UpdateStyle();
}

void _LineStyle::UpdateStyle()
{
	switch (style)
	{
	case STRAIGHT: Draw=&_LineStyle::DrawStraight; IconDraw=&_LineStyle::IconDrawStraight; break;
	default: Draw=NULL; IconDraw=NULL;
	}
}

void LineStyleStyleParams::Serialize( CArchive& ar )
{
	int t;
	if (ar.IsStoring()) 
	{
		ar << style;
	}
	else 
	{ 
		ar >> t; 
		switch(t)
		{
		case STRAIGHT: style=STRAIGHT; break;
		case NO_LINE:
		default: style=NO_LINE;
		}
	}
}

void _LineStyle::DrawStraight( BMPanvas* img,CPoint& t )
{
	img->LineTo(t.x,t.y);    
}

void _LineStyle::IconDrawStraight( BMPanvas& icon )
{
	icon.MoveTo(0,icon.h/2); icon.LineTo(icon.w,icon.h/2);
}

void _ErrorBarStyle<CPoint>::DrawPointVsErrorBar( BMPanvas* img, CPoint& t ) {}
void _ErrorBarStyle<CPointVsError>::DrawPointVsErrorBar( BMPanvas* img, CPointVsError& t )
{
	img->MoveTo(t.x-CapWidth,t.y-t.dy); img->LineTo(t.x+CapWidth,t.y-t.dy);
	img->MoveTo(t.x-CapWidth,t.y+t.dy); img->LineTo(t.x+CapWidth,t.y+t.dy);
	img->MoveTo(t.x,t.y-t.dy); img->LineTo(t.x,t.y+t.dy); img->MoveTo(t.x,t.y);
}

void ErrorBarStyleParams::Serialize( CArchive& ar )
{
	int t;
	if (ar.IsStoring()) { ar << CapWidth << style;}
	else 
	{
		ar >> CapWidth >> t;
		switch (t)
		{
		case POINTvsERROR_BAR: style=POINTvsERROR_BAR; break;
		case NO_BARS:
		default: style=NO_BARS;
		}
	}
}

/////////////////////////////////////////////////////////////////////////
Point1D Point1DArray::operator[]( int n ) { return Point1D(x[n],AbstractPoint2DArray::operator[](n)); }
int Point1DArray::Add( Point1D& pnt ) { AbstractPoint2DArray::Add(pnt); return x.Add(pnt.x); }
double* Point1DArray::GetX() { return x.GetData(); }
void Point1DArray::SetSize( int n ) { AbstractPoint2DArray::SetSize(n); x.SetSize(n);}


void AbstractPoint2DArray::Serialize( CArchive& ar )
{
	int n;
	if (ar.IsStoring()) 	
	{		
		ar << (n=GetSize());
		ar.Write(GetData(),n*sizeof(AbstractPoint2D));
	}
	else 
	{
		RemoveAll(); ar >> n; SetSize(n);
		ar.Read(GetData(),n*sizeof(AbstractPoint2D));
	}
}

void Point1DArray::Serialize( CArchive& ar )
{
	int n; AbstractPoint2DArray::Serialize(ar);
	if (ar.IsStoring()) 	
	{		
		ar << (n=GetSize());
		ar.Write(x.GetData(),n*sizeof(double));
	}
	else 
	{
		RemoveAll(); ar >> n; x.SetSize(n);
		ar.Read(x.GetData(),n*sizeof(double));
	}	
}

void SimplePointArray::Serialize( CArchive& ar )
{
	int n; AbstractPoint2DArray::Serialize(ar);
	if (ar.IsStoring()) 	
	{		
		ar << (n=GetSize());
		ar.Write(y.GetData(),n*sizeof(double));
	}
	else 
	{
		RemoveAll(); ar >> n; y.SetSize(n);
		ar.Read(y.GetData(),n*sizeof(double));
	}	
}

void PointVsErrorArray::Serialize( CArchive& ar )
{
	int n; AbstractPoint2DArray::Serialize(ar);
	if (ar.IsStoring()) 	
	{		
		ar << (n=GetSize());
		ar.Write(dy.GetData(),n*sizeof(double));
	}
	else 
	{
		RemoveAll(); ar >> n; dy.SetSize(n);
		ar.Read(dy.GetData(),n*sizeof(double));
	}
}

//////////////////////////////////////////////////////////////////////////
SimplePoint SimplePointArray::operator[]( int n ) { return SimplePoint(Point1DArray::operator[](n),y[n]); }
int SimplePointArray::Add( SimplePoint& pnt ) { Point1DArray::Add(pnt); return y.Add(pnt.y); }
double* SimplePointArray::GetY() { return y.GetData(); }
void SimplePointArray::SetSize( int n ) { Point1DArray::SetSize(n); y.SetSize(n); }
//////////////////////////////////////////////////////////////////////////
int Point1DArray::CopyFrom( const Point1DArray& src, UINT total/*=0*/, UINT src_shift/*=0*/ )
{
	UINT n=src.x.GetSize();
	if( (total + src_shift) > n ) return -1;
	RemoveAll(); if(total==0) total=n;	 SetSize(total); 
	const double *xSrc = src.x.GetData(); xSrc += src_shift; 
	memcpy(x.GetData(),xSrc,total*sizeof(double));
	const AbstractPoint2D *abstrSrc=src.GetData(); abstrSrc += src_shift; 
	memcpy(GetData(),abstrSrc,total*sizeof(AbstractPoint2D));
	return total;
}

int SimplePointArray::CopyFrom( const SimplePointArray& src, UINT total, UINT src_shift )
{	
	UINT n=src.x.GetSize();
	if( (total + src_shift) > n ) return -1;
	RemoveAll(); 
	Point1DArray::CopyFrom(src,total,src_shift);
	if(total==0) total=n;	 SetSize(total); 
	const double *ySrc = src.y.GetData(); ySrc += src_shift; 
	memcpy(y.GetData(),ySrc,total*sizeof(double));	
	return total;
}

int PointVsErrorArray::CopyFrom( const PointVsErrorArray& src, UINT total, UINT src_shift )
{
	UINT n=src.x.GetSize();
	if( (total + src_shift) > n ) return -1;
	RemoveAll(); 
	SimplePointArray::CopyFrom(src,total,src_shift);
	if(total==0) total=n;	 SetSize(total); 
	const double *dySrc = src.dy.GetData(); dySrc += src_shift; 
	memcpy(dy.GetData(),dySrc,total*sizeof(double));	
	return total;
}

PointVsError PointVsErrorArray::operator[]( int n ) { return PointVsError (SimplePointArray::operator [](n),dy[n]); }
int PointVsErrorArray::Add( PointVsError& pnt ) { SimplePointArray::Add(pnt); return dy.Add(pnt.dy); }
double* PointVsErrorArray::GetdY() { return dy.GetData(); }
void PointVsErrorArray::SetSize( int n ) { SimplePointArray::SetSize(n); dy.SetSize(n);}
//////////////////////////////////////////////////////////////////////////
