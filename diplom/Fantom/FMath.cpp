//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FMath.cc,v $
// Language:  C++
// Date:      $Date: 2003/06/25 07:54:46 $
// Author:    $Author: garth $
// Version:   $Revision: 1.17 $
//
//---------------------------------------------------------------------------

#include "FMath.h"
#include "FException.h"
#include "FVector.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <complex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TINY 1.0e-30
#undef SWAP
#define SWAP(a,b,c) {c=a;a=b;b=c;}

void FMath::FGaussJ( FMatrix &a, FMatrix &b )
// PAR: a[1..n][1..n] contains the matrix of the system to solve
//	n   is the number of rows and columns in the system
//	b[1..n][1..m] contains m right sides, which are solved simultan
// PRE: no singular matrix. TESTED
// POST: **a contains the inverse of the original matrix
//       **b conatins the solutions for the m right sides
// REMARK: the program was taken from "Numerical Recipes in C", p.39/40
{
    int *indxc, *indxr, *ipiv;
    int
      icol=0, //initialized to avoid warning
      irow=0, //initialized to avoid warning
      i, j, k, l, ll, m, n;
    double big;
    double dum, pivinv, c;

    if (a.getDimensionX() != a.getDimensionY()) {
      FException e("ERROR: first argument is not a square matrix!");
      e.addTraceMessage("void FMath::FGaussJ( FMatrix &a, FMatrix &b )");
      throw e;
    }

    if (a.getDimensionX() != b.getDimensionY()) {
      FException e("ERROR: incompatible dimensions!");
      e.addTraceMessage("void FMath::FGaussJ( FMatrix &a, FMatrix &b )");
      throw e;
    }

    m = a.getDimensionX();
    n = b.getDimensionX();

    indxc=new int[m];
    indxr=new int[m];
    ipiv=new int[m];

    // IPIV preloaden
    for (j=0;j<m;j++) ipiv[j]=0;

  //
  for (i=0; i<m; i++)
  {
    big=0.0;
    for (j=0;j<m;j++)
      if (ipiv[j] != 1)
        for (k=0;k<m;k++)
          {
            if (ipiv[k] == 0)
              {
                if (fabs(a(j,k))>=big)
                  {
                    big=fabs(a(j,k));
                    irow=j;
                    icol=k;
                  }

              }
#ifndef NODEBUG
            else
              {
                FException e("Singular Matrix-1");
                e.addTraceMessage("void FMath::FGaussJ( FMatrix &a, FMatrix &b )");
              }
#endif
          }
    ++(ipiv[icol]);
    // pivot found, interchange rows if necessary
    if (irow != icol)
      {
        for (l=0;l<m;l++) SWAP(a(irow,l), a(icol,l), c);
        for (l=0;l<n;l++) SWAP(b(irow,l), b(icol,l), c);
      }

    indxr[i]=irow;
    indxc[i]=icol;
    // ErrorCheck(a(icol,icol) == 0.0, "gaussj: Singular Matrix-2");

    pivinv=1.0/a(icol,icol);
    a(icol,icol)=1.0;
    for (l=0;l<m;l++) a(icol,l) *= pivinv;
    for (l=0;l<n;l++) b(icol,l) *= pivinv;
    for (ll=0;ll<m;ll++)
      if (ll!=icol)
        {
          dum=a(ll,icol);
          a(ll,icol)=0.0;
          for (l=0;l<m;l++) a(ll,l) -= a(icol,l)*dum;
          for (l=0;l<n;l++) b(ll,l) -= b(icol,l)*dum;
        }
  }
  // system solved, interchange pairs of columns back in original order :
  for (l=m-1;l>=0;l--)
    {
      if (indxr[l] != indxc[l])
        for (k=0;k<m;k++)
          SWAP(a(k,indxr[l]), a(k,indxc[l]), c);
    }
  delete[] ipiv;
  delete[] indxr;
  delete[] indxc;
}

//---------------------------------------------------------------------------

//===========================================================================

int FMath::QuadraticEquation(double a, double b, double c,
			     complex<double> x[2])
// PAR: a*x^2+b*x+c=0.0, x[0..1] gets the complex solutions
// POST: x[0], x[1] contain the solution
// RETURN: number of different solutions
//         (-1,0,1,2) -1 stands for infinite many!
{
  // mario
    if(fabs(a) < 1e-9)
    {
      // linear equation
      if( fabs(b) <1e-9) return 0; // FIXME: there may be infinite, how to deal with that?
      x[0] = complex<double>(-c/b,0.);
      return 1;
    }
  // end mario
    double d=b*b-4*a*c;
    if (d>0.0)
    {
        double q= b>=0.0 ? -0.5*(b+sqrt(d)) : -0.5*(b-sqrt(d));
        x[0]=(q/a);     // x[0]=q/a+i*0
        x[1]=(c/q);     // x[1]=c/q+i*0
        return 2;
    }
    if (d==0.0)
    {
      if (a != 0.0) {
        x[0]=complex<double>(-0.5*b/a);
	//although there is only one solution
	//set x[1] for convenience to the same value as x[0]
        x[1]=x[0];
        return 1;
      }
      else if (c == 0.0)
	return -1;
      else
	return 0;
    }
    // ASSERT: d<0.0
    complex<double> dd=d;
    dd=sqrt(dd);
    dd=(real(b*dd)>=0) ? dd : -dd;
    complex<double> q=-0.5*(b+dd);
    x[0]=(q/a);
    x[1]=(c/q);
    return 2;
}

// --------------------------------------------------------------------------

int FMath::CubicEquation(double a3, double a2, double a1, double a0, complex<double> x[3])
// PAR: a3*x^3+a2*x^2+a1*x+a0=0.0, x[0..2] gets the solution
// POST: x[0], x[1], x[2] contain the solutions
// RETURN: number of different solutions
//         (-1,0,1,2,3) -1 stands for infinite many!
// REMARK:  ideas taken from "Numerical Recipes in C", p.184/185
{
  if (a3==0.0)
    {
      int n=QuadraticEquation(a2, a1, a0, x);
      x[2]=x[1];
      return n;
    }
  a2 /= a3;
  a1 /= a3;
  a0 /= a3;
  double Q = (a2*a2-3.0*a1)/9.0;
  double R = (2.0*a2*a2*a2-9.0*a2*a1+27.0*a0)/54.0;
  double comp=R*R-Q*Q*Q;
  if (comp>=0.0)
    {
    double sgn_R = (R>=0.0) ? 1.0 : -1.0;
    double A = fabs(R) + sqrt(comp);
      A = pow(A, 1.0/3.0);
      A *= (-sgn_R);
    double B = (A!=0.0) ? Q/A : 0.0;
      x[0]=complex<double>((A+B)-a2/3.0);
      x[1]=complex<double>(-0.5*(A+B)-a2/3.0,0.5*sqrt(3.0)*(A-B));
      x[2]=complex<double>(-0.5*(A+B)-a2/3.0,-0.5*sqrt(3.0)*(A-B));
    return 3;
    }
  double theta=acos(R/sqrt(Q*Q*Q));
  x[0]=complex<double>(-2.0*sqrt(Q)*cos(theta/3.0)-a2/3);
  x[1]=complex<double>(-2.0*sqrt(Q)*cos((theta+2*M_PI)/3.0)-a2/3);
  x[2]=complex<double>(-2.0*sqrt(Q)*cos((theta-2*M_PI)/3.0)-a2/3);
  return 3;
}

// --------------------------------------------------------------------------

void FMath::Eigensystem(const FMatrix& A, std::complex<double> e[2], double* EV0,
			double* EV1)
// PAR: e[2] gets the eigen values, EV0, EV1 gets eigen vectors
// POST: e[2] contains the eigen values,
//       if e[0..1] real , EV0, EV1 contain eigen vectors
//       A remains unchanged
{
    // solve characteristic polynomial
    double sp=A(0,0)+A(1,1) ;
    double det=A(0,0)*A(1,1) - A(1,0)*A(0,1) ;
    FMath::QuadraticEquation(1, -sp, det, e);
    if ( e[0].imag()==0.0 )
    {
      double tmp = e[0].real()-A(0,0);
      if (tmp)
        {
	  EV0[0] = A(0,1) / tmp;
	  EV0[1] = 1.0 ;
        }
      else
	{
	  EV0[0] = 1.0;
	  EV0[1] = 0.0;
	}

      tmp = e[1].real()-A(0,0);
      if (tmp)
        {
	  EV1[0] = A(0,1) / tmp;
	  EV1[1] = 1.0 ;
        }
      else
	{
	  EV1[0] = 1.0;
	  EV1[1] = 0.0;
	}
    }
}

// --------------------------------------------------------------------------

void FMath::mnewt(int ntrial, FVector& x, double tolx, double tolf,
		  void (*usrfun)(FVector, FVector, FMatrix)){
  // cf. Numerical Recipes pp.379-383

  positive N = x.getDimension();

  positive *indx;
  int k, i;

  double errx, errf, d;
  FVector fvec(N);
  FMatrix fjac(N,N);
  FVector p(N);

  indx = new positive [3] ;
  for (k=0 ; k<ntrial ; k++) {
    usrfun(x, fvec, fjac) ;
    errf = 0.0 ;
    for (i=0 ; i<2 ; i++)
      errf += fabs(fvec[i]) ;
    if (errf <= tolf) {
      delete indx ;
      return ;
    }
    for (i=0 ; i<2 ; i++)
      p[i] = -fvec[i] ;
    Ludecomp (fjac, indx, d);
    Lubacksb (fjac, p, indx);
    errx = 0.0 ;
    for (i=0 ; i<2 ; i++) {
      errx += fabs(p[i]) ;
      x[i] += p[i] ;
    }
    if (errx <= tolx) {
      delete indx ;
      return ;
    }
  }
  delete indx ;
  return ;
}


void FMath::Ludecomp(FMatrix& a, positive* indx, double& d)
// PAR: **a contains a matrix a[1..n][1..n]
//	indx[1..n] gives the row permutations back (partial pivoting)
//	d becomes +1, if equal number row interchanges and -1 if odd
// POST: a contains LU decomposition, *indx the permutation and d the sign
// REMARK: the program was taken from "Numerical Recipes in C", p.46/47
{
#ifndef NODEBUG
  if( a.getDimensionX() != a.getDimensionY() )
    {
      FException e ("matrix is not square matrix!");
      throw e;
    }
#endif

  int N = a.getDimensionX();
  int
    i, j, k,
    imax=0;//initialized to avoid warning
  double big, dum, sum, temp;
  FVector vv(N);

  d = 1.0;
  for (i=0;i<N;i++) // nachsehen ob eine zeile leer (0) ist
    {
      big=(double) 0;
      for (j=0;j<N;j++)
        if ((temp=fabs(a(i,j))) > big) big=temp;
      if(big==0) {
        throw new FMatrixSingularException;
      }

      // in vv den kehrwert des groessten wertes speichern
      vv[i]=(1.0)/big;
    }

  for (j=0;j<N;j++) // alle Zeilen abarbeiten
    {
      for (i=0;i<j;i++) // bis 1 unter j laufen
        {
          sum=a(i,j);
          for (k=0;k<i;k++) sum -=a(i,k)*a(k,j);
          a(i,j)=sum;
        }
      big=0.0;
      for (i=j;i<N;i++)
        {
          sum=a(i,j);
          for (k=0;k<j;k++) sum -=a(i,k)*a(k,j);
          a(i,j)=sum;
          if ( (dum=vv[i]*fabs(sum)) >= big)
            {
              big=dum;
              imax=i;
            }
        }
      if (j!=imax)
        {
          for (k=0;k<N;k++)
            {
              dum=a(imax,k);
              a(imax,k)=a(j,k);
              a(j,k)=dum;
            }
          d= -(d);
          vv[imax]=vv[j];
        }
      indx[j]=imax;
      if (a(j,j) == 0) a(j,j)=TINY; // pivot==0 => singular matrix!
      // try with TINY, sometimes desirable
      if (j!=N-1)
        {
          dum=(1.0)/(a(j,j));
          for (i=j+1;i<N;i++) a(i,j) *= dum;
        }
    }
}

// --------------------------------------------------------------------------

void FMath::Lubacksb(const FMatrix& a, FVector& b, positive * indx)
  // PAR: a[1..n][1..n] is matrix in LU-decomposition from ludcmp,
  //	indx[1..n] is permutation vector from ludcmp,
  //	n is the dimension of a,
  //	b[1..n] is a right side.
  // POST: a, n, indx are unchanged
  //	 b contains the solution of the equation
  // REMARK: the routine was taken from "Numerical Recipes in C", p.47
{
#ifndef NODEBUG
  if( a.getDimensionX() != a.getDimensionY() )
    {
      FException e ("matrix is not square matrix!");
      throw e;
    }
#endif

  int N = a.getDimensionX();

  int i, ii=-1, ip, j;
  double sum;
  for (i=0;i<N;i++)
    {
      ip=indx[i];
      sum=b[ip];
      b[ip]=b[i];

      if (ii!=-1)
        for (j=ii;j<i;j++) sum -= a(i,j) * b[j];
      else
        if (sum) ii=i;
      b[i]=sum;
    }
  for (i=N-1;i>=0;i--)
    {
      sum=b[i];
      for (j=i+1;j<N;j++) sum -=a(i,j)*b[j];
      b[i]=sum/a(i,i);
    }
}

// --------------------------------------------------------------------------

void FMath::jordanCanonicalForm( const FMatrix& m, FMatrix& d, vector< complex <double> > & eValues, double zeroTolerance)
{
#ifndef NODEBUG
  if( m.getDimensionX() != m.getDimensionY() ) {
    FInvalidDimensionException e ("Matrix given for jordan canonival form is not square !");
    e.addTraceMessage("void FMath::jordanCanonicalForm( const FMatrix& m, FMatrix& d,  vector< complex <double> > & eValues, double zeroTolerance )");
    throw e;
  }
#endif

  positive i,j,k;


  // make very small values zero
  for (i=0; i<eValues.size(); i++) {
    if (fabs(eValues[i].real()) < zeroTolerance)
      eValues[i] = complex<double> (0.0, eValues[i].imag());
    if (fabs(eValues[i].imag()) < zeroTolerance)
      eValues[i] = complex<double> (eValues[i].real(), 0.0);
  }

  vector< double >          evSorted;
  vector< complex<double> > evSortedComplex;

  // sort out EV's that occur more than once
  bool flag;
  for (i=0; i<eValues.size(); i++) {

    // first only the real eigenvalues
    if (eValues[i].imag() != 0.0)
      continue;

    flag = true; // if true the ev is not in our list yet

    for (j=0; j<evSorted.size(); j++){
      if (fabs(eValues[i].real()-evSorted[j]) < zeroTolerance)
        flag=false;
    }

    if (flag)
      evSorted.push_back(eValues[i].real());
  }
  //  evSorted.sort();

  for (i=0; i<eValues.size(); i++) {

    // now only the complex eigenvalues
    if (eValues[i].imag()==0.0)
      continue;

    flag = true; // if true the ev is not in our list yet
    for (j=0; j<evSortedComplex.size(); j++)
      if ((fabs(eValues[i].real()-evSortedComplex[j].real()) < zeroTolerance) &&
          (fabs(fabs(eValues[i].imag())-fabs(evSortedComplex[j].imag())) < zeroTolerance))
        flag=false;
    if (flag)
      evSortedComplex.push_back(eValues[i]);
  }

  // we have the Eigenvalues all together in evSorted, lets determine the
  // size of the jordan blocks...

  positive n = m.getDimensionX();
  positive s = evSorted.size();


  FMatrix mExp(n,n);
  FMatrix mMinusLambdaI(n,n);
  FMatrix I(n,n);
  I.makeDiagonal(1.0);
  FMatrix r(s,n+1);
  FMatrix b(s,n);

  vector< FVector > nullSpace;
  vector< FVector > range;

  FVector dmy1(n), dmy2(n);


  for (i=0; i<s; i++) {
    mExp=I;
    mMinusLambdaI=m-(evSorted[i]*I);
    for (j=0; j<n+1; j++) {
      mExp *= mMinusLambdaI;
      if (j>1)
        if (r(i,j-2) == r(i,j-1))
          r(i,j)=r(i,j-1);
        else {
          mExp.solveSingularSystem(dmy1, dmy2, nullSpace, range, zeroTolerance);
          r(i,j) = range.size();
          range.clear();
        }
      else {
        mExp.solveSingularSystem(dmy1, dmy2, nullSpace, range, zeroTolerance);
        r(i,j) = range.size();
        range.clear();
      }
    }

    b(i,0) = n-2*r(i,0) + r(i,1);

    for (j=1;j<n;j++) {
      b(i,j) = r(i,j+1) - 2*r(i,j) + r(i,j-1);
    }
  }

  FMatrix J(n,n); // our final jordan matrix...

  positive x,y;

  k=0;
  for (i=0; i<s; i++) {
    for (j=0; j<n; j++) {
      if (b(i,j) != 0)
        for ( x=0; x<b(i,j); x++) {
          for (y=0; y<=j;y++) {
            J(k+y, k+y)=evSorted[i];
            if (y<j) J(k+y, k+y+1) = 1.0;
          }
          k+=j+1;
        }
    }
  }

  for (i=0; i<evSortedComplex.size(); i++) {
    J(k, k)=J(k+1, k+1)=evSortedComplex[i].real();
    J(k+1, k)=evSortedComplex[i].imag();
    J(k, k+1)=-1.0*evSortedComplex[i].imag();
    k+=2;
  }

  d=J;
}

#undef SWAP

//---------------------------------------------------------------------------

// This method is only used by the Tet-2-Plane intersection calcs.
static void mix( FArray& result,
                 const FArray& v0,
                 const FArray& v1,
                 double a, double b)
{
  if (fabs(b-a) < 1.0e-9)
    cout << "warning : VERY SMALL DIVISOR" << endl;

  a=b/(b-a);

  result=v0;
  result*=(a);
  result+=( v1*(1.0-a) );
}

//---------------------------------------------------------------------------

void FMath::intersectTetrahedronToPlane( const vector<FPosition>& tet,
                                         const FVector& n,
                                         const FPosition& p,
                                         vector<FPosition>& result )
{
  intersectTetrahedronToPlane(tet[0],tet[1],tet[2],tet[3],
                              n, p, result);
}


//---------------------------------------------------------------------------

void FMath::intersectTetrahedronToPlane( const FPosition& tet0,
                                         const FPosition& tet1,
                                         const FPosition& tet2,
                                         const FPosition& tet3,
                                         const FVector& n,
                                         const FPosition& p,
                                         vector<FPosition>& result )
{

  try {

    double lamda[4];
    const FArray* vecs[4];
    vecs[0]=&tet0; vecs[1]=&tet1; vecs[2]=&tet2; vecs[3]=&tet3;

    positive set[2][4];
    int setAsize=0;
    int setBsize=0;

    if ((n*n) < 1.0e-9)
      cout << "warning : VERY SMALL DIVISOR AAA" << endl;
    double nn_inv=1.0 / (n*n);
    double np=n*p;

    lamda[0]=(n*tet0 - np)*nn_inv;
    lamda[1]=(n*tet1 - np)*nn_inv;
    lamda[2]=(n*tet2 - np)*nn_inv;
    lamda[3]=(n*tet3 - np)*nn_inv;

    for (positive i=0; i<4; i++)
      if (lamda[i]>0)
	set[0][setAsize++]=i;
      else
	set[1][setBsize++]=i;

    if (setAsize-setBsize == 0){
      result.resize(4);
      mix( result[0], (*vecs)[set[0][0]], (*vecs)[set[1][0]],
	   lamda[set[0][0]], lamda[set[1][0]]);

      mix( result[1], (*vecs)[set[0][1]], (*vecs)[set[1][0]],
	   lamda[set[0][1]], lamda[set[1][0]]);

      mix( result[2], (*vecs)[set[0][1]], (*vecs)[set[1][1]],
	   lamda[set[0][1]], lamda[set[1][1]]);

      mix( result[3], (*vecs)[set[0][0]], (*vecs)[set[1][1]],
	   lamda[set[0][0]], lamda[set[1][1]]);
    }
    else if (abs(setAsize-setBsize) == 2){
      // mix 0 - 1, 0 - 2, 0 - 3 if set 1 is {0} and set 2 is {1 2 3}

      // when setAsize > setBsize (setAsize == 3, setBsize == 1)
      // setAsize = 1, setBsize = 0
      // else setAsize = 0, setBsize = 1
      // ---> setBsize iss the index of the smaller set.
      setAsize=(setAsize-1) >> 1;
      setBsize=(setBsize-1) >> 1;
      result.resize(3);
      mix( result[0],
	   (*vecs)[set[setAsize][0]],
	   (*vecs)[set[setBsize][0]],
	   lamda[set[setAsize][0]],
	   lamda[set[setBsize][0]]);
      mix( result[1],
	   (*vecs)[set[setAsize][0]],
	   (*vecs)[set[setBsize][1]],
	   lamda[set[setAsize][0]],
	   lamda[set[setBsize][1]]);
      mix( result[2],
	   (*vecs)[set[setAsize][0]],
	   (*vecs)[set[setBsize][2]],
	   lamda[set[setAsize][0]],
	   lamda[set[setBsize][2]]);
    }
    else
      result.resize(0);
  }
  catch (FException& e) {
    cout << "EXCEPTION IN  void FMath::intersectTetrahedronToPlane( const FArray& tet0, " << endl;
    cout << e;
  }

}

/*
void FMath::FFT(vector<double> &x, vector<double> &y, unsigned int offset, unsigned int stepsize, unsigned int size)
{
    if(x.size() != y.size()) {
        throw FInvalidDimensionException("FMath::FFT dimensions of x and y do not match");
    }
#define cc(I) ((offset+I*stepsize) )

    int dir = inverseTransform ? -1 : 1;
    long n, i, i1, j, k, i2, l, l1, l2;
    double c1, c2, tx, ty, t1, t2, u1, u2, z;

    int m=0;
    while(pow(2.,m)<size)++m;       // ugly way of getting log_2(size) but good for small values
    if(size != (unsigned int)pow(2.,m)){
        std::cout << "invalid size of " << size << std::endl;
    }


    // Calculate the number of points
    //      n = 1;
    //          for (i = 0; i < m; i++)
    //                  n *= 2;
    n = size;

    // Do the bit reversal
    i2 = n >> 1;
    j = 0;
    for (i = 0; i < n - 1; i++) {
        if (i < j) {
            tx = x[cc(i)];
            ty = y[cc(i)];
            x[cc(i)] = x[cc(j)];
            y[cc(i)] = y[cc(j)];
            x[cc(j)] = tx;
            y[cc(j)] = ty;
        }
        k = i2;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    // Compute the FFT
    c1 = -1.0;
    c2 = 0.0;
    l2 = 1;
    for (l = 0; l < m; l++) {
        l1 = l2;
        l2 <<= 1;
        u1 = 1.0;
        u2 = 0.0;
        for (j = 0; j < l1; j++) {
            for (i = j; i < n; i += l2) {
                i1 = i + l1;
                t1 = u1 * x[cc(i1)] - u2 * y[cc(i1)];
                t2 = u1 * y[cc(i1)] + u2 * x[cc(i1)];
                x[cc(i1)] = x[cc(i)] - t1;
                y[cc(i1)] = y[cc(i)] - t2;
                x[cc(i)] += t1;
                y[cc(i)] += t2;
            }
            z = u1 * c1 - u2 * c2;
            u2 = u1 * c2 + u2 * c1;
            u1 = z;
        }
        c2 = sqrt((1.0 - c1) / 2.0);
        if (dir == 1)
            c2 = -c2;
        c1 = sqrt((1.0 + c1) / 2.0);
    }

    // Scaling for forward transform

    //    if (dir == 1) {
    for (i = 0; i < n; i++) {
        x[cc(i)] /= sqrt((double)n);
        y[cc(i)] /= sqrt((double)n);
    }
    //    }
}
*/

/****************************************************************/
/* fft.c */
/* Douglas L. Jones */
/* University of Illinois at Urbana-Champaign */
/* January 19, 1992 */
/* */
/*   fft: in-place radix-2 DIT DFT of a complex input */
/* */
/*   input: */
/* n: length of FFT: must be a power of two */
/* m: n = 2**m */
/*   input/output */
/* x: double array of length n with real part of data */
/* y: double array of length n with imag part of data */
/* */
/*   Permission to copy and use this program is granted */
/*   as long as this header is included. */
/****************************************************************/
void FMath::inPlaceRadix2DitDft(unsigned int n, unsigned int m, vector<double>& x,vector<double>& y)
{
    unsigned int i,j,k,n1,n2;
    double c,s,e,a,t1,t2;

    j = 0; /* bit-reverse */
    n2 = n/2;
    for (i=1; i < n - 1; i++)
    {
        n1 = n2;
        while ( j >= n1 )
        {
            j = j - n1;
            n1 = n1/2;
        }
        j = j + n1;

        if (i < j)
        {
            t1 = x[i];
            x[i] = x[j];
            x[j] = t1;
            t1 = y[i];
            y[i] = y[j];
            y[j] = t1;
        }
    }


    n1 = 0; /* FFT */
    n2 = 1;

    for (i=0; i < m; i++)
    {
        n1 = n2;
        n2 = n2 + n2;
        e = -2.*M_PI/n2;
        a = 0.0;

        for (j=0; j < n1; j++)
        {
            c = cos(a);
            s = sin(a);
            a = a + e;

            for (k=j; k < n; k=k+n2)
            {
                t1 = c*x[k+n1] - s*y[k+n1];
                t2 = s*x[k+n1] + c*y[k+n1];
                x[k+n1] = x[k] - t1;
                y[k+n1] = y[k] - t2;
                x[k] = x[k] + t1;
                y[k] = y[k] + t2;
            }
        }
    }

    return;
}

