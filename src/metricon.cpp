#include "stdafx.h"
#include "metricon.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CalibrationParams::Serialize( CArchive& ar )
{
	if(ar.IsStoring())
	{
		val.Serialize(ar);
		Nexp.Serialize(ar);  n.Serialize(ar);
	}
	else
	{
		val.Serialize(ar);
		Nexp.Serialize(ar);  n.Serialize(ar);	
	}
}
CalibrationParams& CalibrationParams::operator=(const CalibrationParams& t )
{
	val = t.val; Nexp = t.Nexp; n = t.n;
	return *this;
}
double CalibrationParams::Calculator::FuncParams::func( double fi)
{
	PrepareAB(fi);
	double ret = ((N[1] - N[3])*(A[0] - A[3]) - (N[0] - N[3])*(A[1] - A[3]))*
		  ((B[2] - B[3])*(A[0] - A[3]) - (B[0] - B[3])*(A[2] - A[3])) -
		  ((N[2] - N[3])*(A[0] - A[3]) - (N[0] - N[3])*(A[2] - A[3]))*
		  ((B[1] - B[3])*(A[0] - A[3]) - (B[0] - B[3])*(A[1] - A[3]));
	return ret;
}

void CalibrationParams::Calculator::FuncParams::PrepareBuffers()
{
	DestroyBuffers(); 
	A = new double[size]; B = new double[size];
	teta = new double[size]; tetapp = new double[size];
	for(int i = 0; i < size; i++)
	{
		teta[i] = asin(n[i]/n_p);
		tetapp[i] = asin(n_p*sin(teta[i] - alfa));
	}
}

void CalibrationParams::Calculator::FuncParams::DestroyBuffers()
{
	if(A != NULL) {delete[] A; A = NULL;}
	if(B != NULL) {delete[] B; B = NULL;}
	if(teta != NULL) {delete[] teta; teta = NULL;}
	if(tetapp != NULL) {delete[] tetapp; tetapp = NULL;}
}

void CalibrationParams::Calculator::FuncParams::PrepareAB( double fi )
{
	for(int i = 0; i < size; i++)
	{		
		A[i] = sin(tetapp[i])/cos(fi - tetapp[i]);
		B[i] = sin(alfa)*tan(alfa - teta[i])*(tan(fi)*A[i] - (1/cos(fi)));
	}
}

int CalibrationParams::CalculateFrom(	DoubleArray& _Nexp, DoubleArray& _teta, 
										double n_p, double n_i, double n_s, double alfa, double lambda)
{
	Calculator::FuncParams params(_Nexp, _teta, n_p, n_i, n_s, alfa, lambda);
	Solver1dTemplate<CalibrationParams::Calculator::FuncParams> FindFI(SINGLE_ROOT);
	CleanUp();
	if( FindFI.Run(&params, BoundaryConditions(-45*DEGREE, 45*DEGREE), SolverErrors(1e-12)) == GSL_SUCCESS) 
	{
		double *N = params.N, *A = params.A, *B = params.B;
		
		double f0 = FindFI.Roots[0]; params.PrepareAB(f0);
		double L0 = ((N[1] - N[3])*(A[0] - A[3]) - (N[0] - N[3])*(A[1] - A[3])) /
					((B[1] - B[3])*(A[0] - A[3]) - (B[0] - B[3])*(A[1] - A[3]));
		double d0 = ((N[0] - N[3]) - L0*(B[0] - B[3]))/(A[0] - A[3]);
		double N0 = N[0] - d0*A[0] - L0*B[0];
		val << params.alfa <<  params.n_p << params.n_i << params.n_s << params.lambda << f0 << L0 << d0 << N0;
		Nexp = params.N; n = params.n;
		Ncalc.RemoveAll(); delta = 0;
		for (int i = 0; i < params.size; i++)
		{
			Ncalc << (N0 + d0*A[i] + L0*B[i]);			
			delta += fabs(N[i] - Ncalc[i]);
		}
		*((SolverData*)(this)) = *((SolverData*)&FindFI);
	}
	return FindFI.status;
}
double PixelToAngleSolver::FuncParams::func( double teta ) { return Npix - cal.Helper1(teta); }

AngleFromCalibration CalibrationParams::ConvertPixelToAngle(double Npix)
{
	AngleFromCalibration ret;
	if (this->IsValidCalibration() == FALSE)
	{
		ret.status = E_FAIL;
		return ret;
	}
	PixelToAngleSolver::FuncParams params(Npix, *this);
	Solver1dTemplate<PixelToAngleSolver::FuncParams> FindTETA(SINGLE_ROOT);	

	if ((ret.status = FindTETA.Run(&params, BoundaryConditions(35*DEGREE, 68.*DEGREE), 
		SolverErrors(1e-12))) == GSL_SUCCESS) 
	{
		ret.teta = FindTETA.Roots[0];
		ret.cal = params.cal.val; ret.Npix = Npix;
		*((SolverData*)(&ret)) = *((SolverData*)&FindTETA);
	}
	return ret;
}

double CalibrationParams::ConertAngleToBeta( double teta ) { return val[ind_n_p]*sin(teta); }

double CalibrationParams::ConvertBettaToPixel( double teta )
{
	double betta = ConvertBettaToAngle(teta);
	return Helper1(betta);
}

double CalibrationParams::Helper1( double teta )
{
	double tetapp, A, B;	
	tetapp = asin(val[ind_n_p]*sin(teta - val[ind_alfa]));
	A = sin(tetapp)/cos(val[ind_fi0] - tetapp);
	B = sin(val[ind_alfa])*tan(val[ind_alfa] - teta)*(tan(val[ind_fi0])*A - (1/cos(val[ind_fi0])));
	return val[ind_N0] + val[ind_d0]*A + val[ind_L]*B;
}

double CalibrationParams::ConvertBettaToAngle( double betta ) { return asin(betta/val[ind_n_p]); }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
DispEqSolver::DispEqSolver( const SolverRegime _rgm, const int _shift, const int _roots_num, const int _max_iter/*=100*/ ) :
	Solver1dTemplate<DispEqSolver_FuncParams>(_rgm, _max_iter), shift(_shift), roots_num(_roots_num) {}

DispEqSolver::~DispEqSolver() { CleanUp(); }

int DispEqSolver::FindSubRgns( const BoundaryConditions &X, BoundaryConditionsArray& SubRgns )
{
	if (rgm == SINGLE_ROOT)
	{
		SubRgns.Add(X);
	}
	else
	{
		double x, dx = (X.max - X.min)/(subrgns_max - 1); 
		double y_l, y_r = func(X.max, F.params);
		int shift_left = shift, roots_left = roots_num;

		for(int i = 1; i < subrgns_max && roots_left != 0; i++)
		{
			x = X.max - i*dx; y_l = func(x, F.params);
			if ((y_l < 0 && y_r > 0) || (y_l > 0 && y_r < 0))
			{
				if (shift_left == 0)
				{
					SubRgns.Add(BoundaryConditions(x, x + dx));
					roots_left--;
				}
				else 
					shift_left--;
			}
			y_r = y_l;
		}
	}
	return GSL_SUCCESS;
}

int DispEqSolver::Run(DispEqSolver_FuncParams *_params, const BoundaryConditions &range, const SolverErrors &Err)
{
	MyTimer Timer1; int ret;
	
	Timer1.Start(); 
	CleanUp(); AttachParams(_params); 
	FindSubRgns(range); 
	ret = Run(range, Err);
	dt += Timer1.StopStart();
	return ret;
}

int DispEqSolver::Run( const BoundaryConditions &_X, const SolverErrors &Err )
{
	MyTimer Timer1; Timer1.Start(); 
	err = Err; size_t iter;
	params->PrepareBuffers(); 
	s = gsl_root_fsolver_alloc (fsolver_type);

	for(int i = 0; i < SubRgns.GetSize(); i++)
	{
		BoundaryConditions& X = SubRgns[i]; iter = 0;
		gsl_root_fsolver_set (s, &F, X.min, X.max);
		do
		{
			iter++;
			status = gsl_root_fsolver_iterate (s);
			X.min = gsl_root_fsolver_x_lower (s); X.max = gsl_root_fsolver_x_upper (s);
			status = gsl_root_test_interval (X.min, X.max, err.abs, err.rel);
		}
		while (status == GSL_CONTINUE && iter < max_iter);	

		if( status == GSL_SUCCESS) 
		{		
			Roots << gsl_root_fsolver_root(s);
		}
		cntr.iter += iter;
	}
	dt=Timer1.StopStart(); 
	return status;
}

int DispEqSolver::GetBettas( DoubleArray &n_teor, DispEqSolver_FuncParams *_params, 
							const BoundaryConditions &_range, const SolverErrors &Err )
{
	int ret = Run(_params, _range, SolverErrors(1e-6));
	n_teor = Roots;
	return ret;
}

double DispEqSolver_FuncParams::funcTE( double betta )
{
	double gam1, gam2, gam3, ret;
	gam1 = sqrt(betta*betta - n_i*n_i); gam2 = sqrt(n_f*n_f - betta*betta); gam3 = sqrt(betta*betta - n_s*n_s);
	ret = (gam2*gam2 - gam1*gam3)*sin(gam2*kHf) - gam2*(gam1 + gam3)*cos(gam2*kHf);
	return ret;
}

double DispEqSolver_FuncParams::funcTM( double betta )
{
	double gam1, gam2, gam3, ni2, nf2, ns2, ret; 
	ni2 = n_i*n_i; nf2 = n_f*n_f; ns2=n_s*n_s;
	gam1 = sqrt(betta*betta - ni2); gam2 = sqrt(nf2 - betta*betta); gam3 = sqrt(betta*betta - ns2);    
	ret = ( gam2*gam2*ni2*ns2 - gam1*gam3*nf2*nf2 )*sin(gam2*kHf) - gam2*nf2*(gam1*ns2 + gam3*ni2)*cos(gam2*kHf);
	return ret;
}

DispEqSolver_FuncParams::DispEqSolver_FuncParams( Polarization pol, const CalibrationParams &cal, double _n2, double Hf )
{
	kHf = Hf*2*M_PI/cal.val[CalibrationParams::ind_lambda];			
	n_i = cal.val[CalibrationParams::ind_n_i]; 
	n_f = _n2;
	n_s = cal.val[CalibrationParams::ind_n_s]; 
	SetFunc(pol);
}

double FilmParams::FuncParams::func(const gsl_vector * x)
{
    double ret = MAX_DELTA; int status;	DoubleArray film; 
	Convert_gsl_vector_to_DoubleArray(x, film);

	int roots_n = n_exp.GetSize();
	DispEqSolver FindBettas(MULTI_ROOT, shift, roots_n);
	DispEqSolver_FuncParams params(pol, n_i, film[index_n], n_s, k*film[index_H]);
	BoundaryConditions range(n_s, film[index_n] + 1e-3);
	FindBettas.CleanUp(); FindBettas.AttachParams(&params); 
	FindBettas.FindSubRgns(range); 

	if (FindBettas.GetSubRgnsSize() == roots_n)
	{
		if ((status = FindBettas.Run(range, SolverErrors(1e-6))) == GSL_SUCCESS) 
		{
			ret = 0; 
			for (int j = 0; j < roots_n; j++)
			{
				double root = FindBettas.Roots[j];
				//n_teor << betta_info(root, j + shift);
				ret += abs(root - n_exp[j]);
			}
		}
	}
	return ret;
}
void FilmParams::FuncParams::CleanUp()
{	
	n_exp.RemoveAll(); n_teor.RemoveAll(); minimum_value = MAX_DELTA;
}
int FilmParams::Calculator(	const Polarization pol, const CalibrationParams &cal, 
						   const DoubleArray& _n_exp, const int shift,
						   FilmParams initX, FilmParams initdX)
{
	CleanUp();
	MultiDimMinimizerTemplate<FuncParams> FindFilmParams(400);
	FuncParams params(pol, _n_exp, shift, cal);
	DoubleArray X0, dX0; 
	
	X0 << initX.n << initX.H; dX0 << initdX.n << initdX.H;

	FindFilmParams.Run(&params, X0, dX0, SolverErrors(1e-6));

	n = FindFilmParams.Roots[index_n]; H = FindFilmParams.Roots[index_H]; 
	minimum_value = FindFilmParams.minimum_value;
	n_teor = params.n_teor;
	n_exp = params.n_exp;
	*((SolverData*)(this)) = *((SolverData*)&FindFilmParams);
	return FindFilmParams.status;
}

int FilmParams::Calculator2( 
	const Polarization pol, const CalibrationParams &cal, const DoubleArray& _n_exp, const int shift, 
	const DoubleArray& range_min, const DoubleArray& range_max, const DoubleArray& dd)
{
	CleanUp();
	n_exp = _n_exp;

	Simple2DMinimizerTemplate<FuncParams> FindFilmParams0(400);
	FuncParams params0(pol, _n_exp, shift, cal);
	FindFilmParams0.Run(&params0, range_min, range_max, dd);
	n_init = FindFilmParams0.Roots[index_n]; H_init = FindFilmParams0.Roots[index_H]; minimum_value = FindFilmParams0.minimum_value;
	*((SolverData*)(this)) = *((SolverData*)&FindFilmParams0);

	MultiDimMinimizerTemplate<FuncParams> FindFilmParams1(400);
	FuncParams params1(pol, _n_exp, shift, cal);
	DoubleArray X0, dX0; 
	X0 << n_init << H_init; dX0 << 1e-4 << 1e-1;
	FindFilmParams1.Run(&params1, X0, dX0, SolverErrors(1e-6));
	n = FindFilmParams1.Roots[index_n]; H = FindFilmParams1.Roots[index_H]; minimum_value = FindFilmParams1.minimum_value;	
	err = FindFilmParams1.err;
	
	DispEqSolver FindBettas(MULTI_ROOT, shift, _n_exp.GetSize());
	DispEqSolver_FuncParams params(pol, cal, n, H);
	BoundaryConditions range(params.n_s, n + 1e-3);
	FindBettas.GetBettas(n_teor, &params, range, SolverErrors(1e-6));

	return FindFilmParams1.status;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef void (*CalcRHelper)(const ComplexGSL &gam2, const ComplexGSL &gam3, const ComplexGSL &gam4,
							const ComplexGSL &epsi, const ComplexGSL &epsf, const ComplexGSL &epss,
							const double &CT, const double &Np,
							ComplexGSL &A, ComplexGSL &B, ComplexGSL &C);

void CalcRTEhelper(const ComplexGSL &gam2, const ComplexGSL &gam3, const ComplexGSL &gam4,
				   const ComplexGSL &epsi, const ComplexGSL &epsf, const ComplexGSL &epss,
				   const double &CT, const double &Np,
				   ComplexGSL &A, ComplexGSL &B, ComplexGSL &C)
{
	ComplexGSL t, t1;
	t = gam2;			t1 = gam3;	A = (t - t1)/(t + t1); 
	t = gam3;			t1 = gam4;	B = (t - t1)/(t + t1); 			
	t = gam2; t *= cJ;	t1 = CT;	C = (t - t1)/(t + t1); 
	C*=-1;
}

void CalcRTMhelper(const ComplexGSL &gam2, const ComplexGSL &gam3, const ComplexGSL &gam4,
				   const ComplexGSL &epsi, const ComplexGSL &epsf, const ComplexGSL &epss,
				   const double &CT, const double &Np,
				   ComplexGSL &A, ComplexGSL &B, ComplexGSL &C)
{
	ComplexGSL t, t1;
	t = epsf;	t *= gam2;	t1 = epsi;	t1 *= gam3;	
	A = (t - t1)/(t + t1);
	t = epss;	t *= gam3;	t1 = epsf;	t1 *= gam4;	
	B = (t - t1)/(t + t1);
	t = epsi;	t *= CT;	t1 = gam2; t1 *= ComplexImGSL(Np*Np);	
	C = (t - t1)/(t + t1);	
}

CalcR_ResultArray CalcR(const Polarization pol, const CalcRParams& params)
{
	CalcR_ResultArray ret; CalcRHelper func;
	double k, teta, ST, ST2, CT, dteta;
	ComplexGSL gam2, gam3, gam4, A, B, C, t, t1, Ax, Ra;
	const double &Np = params.Np; const FilmParams &i=params.i, &f=params.f, &s=params.s;

	switch (pol)
	{
	case TE: func = CalcRTEhelper; break;
	case TM: func = CalcRTMhelper; break;
	}

	k = 2*M_PI/params.lambda; dteta=(params.teta_max-params.teta_min)/(params.num_pnts-1);

	ComplexGSL epsi(i.n, i.m), epsf(f.n, f.m), epss(s.n, s.m);
	epsi=pow2(epsi); epsf=pow2(epsf); epss=pow2(epss);

	for(int j=0; j<params.num_pnts; j++)
	{
		teta = params.teta_min + j*dteta;
		ST = Np*sin(teta*DEGREE); ST2 = ST*ST; CT = Np*cos(teta*DEGREE);

		gam2 = sqrt((epsi-ST2)*-1.);	gam3 = sqrt((epsf-ST2)*-1.);	gam4 = sqrt((epss-ST2)*-1.); 
		func(gam2, gam3, gam4, epsi, epsf, epss, CT, Np, A, B, C);
		t = B*exp(gam3*(-2*k*f.H));		Ax = ( A + t )/( A*t + 1. );
		t = Ax*exp(gam2*(-2*k*i.H));	Ra = ( C + t )/( C*t + 1 );

		ret << CalcR_Result(teta, Ra.abs2(), ST);
	}
	return ret;	
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int FourierFilter( FFTRealTransform::Params& in, double spec_width, FFTRealTransform::Params& out) // spec_width=0...100.
{
	int ret, i, j,  n; ms dt;
	FFTRealTransform Trasformer;

	ret=Trasformer.Run(in,FFTRealTransform::FORWARD);
    if(ret!=GSL_SUCCESS) return ret;
	
	j=(int)((n=in.y->GetSize())*spec_width/100.);
	for(i=j;i<n;i++) (*in.y)[i]=0.;

	dt+=in.dt;
	ret=Trasformer.Run(in,FFTRealTransform::BACKWARD);
	
	out=in; out.dt+=dt;
	return ret;

}

