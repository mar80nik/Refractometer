#pragma once

#include "TChartSeries.h"
#include "my_gsl.h"

enum Polarization {TE, TM};

//////////////////////////////////////////////////////////////////////////
struct AngleFromCalibration: public SolverData
{ 
	int status; double teta, Npix; DoubleArray cal; 
	AngleFromCalibration() {teta = Npix = 0; status = GSL_FAILURE;}
	AngleFromCalibration& operator=(const AngleFromCalibration& ref) 
	{
		status = ref.status; teta = ref.teta; cal = ref.cal; Npix = ref.Npix;
		*((SolverData*)this) = *((SolverData*)&ref);
		return (*this);
	}
};
//////////////////////////////////////////////////////////////////////////
struct CalibrationParams;

class PixelToAngleSolver
{
public:
	struct FuncParams: public BaseForFuncParams
	{		
		double Npix; CalibrationParams& cal;
		double (FuncParams::*funcCB)(double x);

		double func(double x); //This is callback function for GSL solver
		FuncParams(double _Npix, CalibrationParams& _cal): Npix(_Npix), cal(_cal) 
		{ 
			funcCB = &FuncParams::func;
		}
		FuncParams(FuncParams& p): Npix(p.Npix), cal(p.cal), funcCB(p.funcCB) { }
	};	
};

struct CalibrationParams: public SolverData
{
	class Calculator
	{
	public:
		struct FuncParams: public BaseForFuncParams
		{
			friend struct CalibrationParams;
		private:
			double *A, *B; double *teta, *tetapp;  
		protected:
			DoubleArray &N, &n; double n_p, n_i, n_s, alfa, lambda;
			int size;
		public:	
			double (FuncParams::*funcCB)(double x);
			double func (double x); //This is callback function for GSL solver

			FuncParams(	DoubleArray& _N, DoubleArray& _teta, 
						double _n_p, double _n_i, double _n_s, double _alfa, double _lambda):
							N(_N), n(_teta), n_p(_n_p), n_i(_n_i), n_s(_n_s), alfa(_alfa), lambda(_lambda)
			{
				A = B = NULL; teta = tetapp = NULL; size = N.GetSize(); 
				funcCB = &FuncParams::func;
			}
			virtual ~FuncParams() { DestroyBuffers(); }							
			FuncParams(FuncParams& p): N(p.N), n(p.n), n_p(p.n_p), n_s(p.n_s), alfa(p.alfa), funcCB(p.funcCB)
			{
				A = B = NULL; teta = tetapp = NULL; size = p.size; 
			}
			virtual void PrepareBuffers();
			virtual void DestroyBuffers();
			virtual void CleanUp()
			{
				BaseForFuncParams::CleanUp();
				DestroyBuffers();
			}
		protected:
			void PrepareAB(double fi);
		};
	};

	enum {ind_alfa, ind_n_p, ind_n_i, ind_n_s, ind_lambda, ind_fi0, ind_L, ind_d0, ind_N0, ind_max};
public:
	double Helper1(double teta);
public:
	DoubleArray val;
	DoubleArray Nexp, n;
	DoubleArray Ncalc; double delta;
	
	CalibrationParams(): SolverData()  {CleanUp();}
	CalibrationParams(const CalibrationParams& ref) { *this = ref;}
	~CalibrationParams() {CleanUp();}
	virtual void Serialize(CArchive& ar);
	CalibrationParams& operator=(const CalibrationParams& t);
	BOOL IsValidCalibration() {return (val.GetSize() == ind_max);}

	int CalculateFrom(	DoubleArray& _Nexp, DoubleArray& _teta, 
						double _n_p, double _n_i, double _n_s, double _alfa, double _lambda);
	AngleFromCalibration ConvertPixelToAngle(double Npix);
	double ConertAngleToBeta(double teta);
	double ConvertBettaToAngle(double betta);
	static double Get_k(const DoubleArray& cal_val) {return 2*M_PI/cal_val[ind_lambda];}
	virtual void CleanUp()
	{
		val.RemoveAll(); Nexp.RemoveAll(); n.RemoveAll();	
	}
	double ConvertBettaToPixel( double teta );
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
struct betta_info 
{
	double val; int n;
	betta_info(double _val = 0, int _n = 0): val(_val), n(_n) {}
};

class FilmParams: public SolverData
{
public:
	enum {index_n, index_H, index_max};
	
	struct FuncParams: public BaseForFuncParams
	{
	protected:
		CalibrationParams cal;
		double k, n_i, n_s, n_p;
	public:
		Polarization pol; DoubleArray n_exp, n_teor; 
		int shift; double minimum_value;
	public:
		double func(const gsl_vector * x);
		FuncParams(const Polarization _pol, const DoubleArray& _n_exp, const int _shift, const CalibrationParams &_cal): 
			pol(_pol), cal(_cal)
		{			
			n_exp = _n_exp; shift = _shift; n_teor.RemoveAll();
			k = 2*M_PI/cal.val[CalibrationParams::ind_lambda];			
			n_p = cal.val[CalibrationParams::ind_n_p];
			n_i = cal.val[CalibrationParams::ind_n_i]; 
			n_s = cal.val[CalibrationParams::ind_n_s]; 
		}
		FuncParams(FuncParams& params): 
			pol(params.pol), n_exp(params.n_exp), n_teor(params.n_teor)
		{ shift = params.shift; minimum_value = params.minimum_value; }
		virtual void CleanUp();

	};	
public:
	double n, H, m, minimum_value;
	double n_init, H_init;
	DoubleArray n_teor, n_exp;	

	FilmParams(double _n = 0, double _H = 0, double _m = 0): n(_n), H(_H), m(_m), SolverData() {minimum_value = 0; m = 0;}
	int Calculator( const Polarization pol, const CalibrationParams &cal, 
		const DoubleArray& tetaexp, const int shift,
		FilmParams initX = FilmParams(), FilmParams initdX = FilmParams(1e-4, 1e-1));
	int Calculator2( const Polarization pol, const CalibrationParams &cal, const DoubleArray& _n_exp, const int shift,
		const DoubleArray& range_min, const DoubleArray& range_max, const DoubleArray& iter);
};


struct DispEqSolver_FuncParams: public BaseForFuncParams
{
public:
	double n_i, n_f, n_s, kHf;
public:
	//This is callback function for GSL solver
	double (DispEqSolver_FuncParams::*funcCB) (double x); 
	double funcTE (double x); double funcTM (double x);	
	void SetFunc(Polarization pol)
	{
		switch (pol)
		{
		case TE: funcCB = &DispEqSolver_FuncParams::funcTE; break;
		case TM: funcCB = &DispEqSolver_FuncParams::funcTM; break;
		default: funcCB = NULL;
		}
	}
	DispEqSolver_FuncParams(Polarization pol, const CalibrationParams &cal, double _n2, double Hf);
	DispEqSolver_FuncParams(Polarization pol, double _n1, double _n2, double _n3, double _kHf):
		n_i(_n1), n_f(_n2), n_s(_n3), kHf(_kHf) { SetFunc(pol); }
	DispEqSolver_FuncParams(DispEqSolver_FuncParams& params):
		n_i(params.n_i), n_f(params.n_f), n_s(params.n_s), kHf(params.kHf), funcCB(params.funcCB) {}
};	
class DispEqSolver: public Solver1dTemplate<DispEqSolver_FuncParams>
{
protected:
	int shift, roots_num;
public:
	DispEqSolver(const SolverRegime _rgm, const int shift, const int roots_num, const int _max_iter=100);
	virtual ~DispEqSolver();
	virtual int FindSubRgns(const BoundaryConditions &X, BoundaryConditionsArray& SubRgns);
	virtual int FindSubRgns(const BoundaryConditions &X) {return FindSubRgns(X, SubRgns);}

	void * AttachParams(DispEqSolver_FuncParams *_params) { void *ret = params; params = _params; return ret; }
	virtual int Run(DispEqSolver_FuncParams *_params, const BoundaryConditions &_X, const SolverErrors &Err);	
	virtual int Run(const BoundaryConditions &_X, const SolverErrors &Err);	
	int GetSubRgnsSize() const { return SubRgns.GetSize(); } 
	int GetBettas(DoubleArray &n_teor, DispEqSolver_FuncParams *_params, const BoundaryConditions &_X, const SolverErrors &Err);
};
//////////////////////////////////////////////////////////////////////////
struct CalcR_Result
{
	double teta, Ra, ST;
	CalcR_Result(double t = 0, double ra = 0, double st = 0): teta(t), Ra(ra), ST(st) {}
};
typedef TypeArray<CalcR_Result> CalcR_ResultArray;

struct CalcRParams
{
	FilmParams i,f,s;
	double lambda, Np, teta_min, teta_max;
	int num_pnts;
	CalcRParams() { num_pnts=7000; }
};

CalcR_ResultArray CalcR(const Polarization pol, const CalcRParams& params);

int FourierFilter(FFTRealTransform::Params& in, double spec_width, FFTRealTransform::Params& out);
