//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FMath.hh,v $
// Language:  C++
// Date:      $Date: 2003/06/25 07:54:46 $
// Author:    $Author: garth $
// Version:   $Revision: 1.11 $
//
//---------------------------------------------------------------------------

#ifndef __FMath_hh
#define __FMath_hh

#include <complex>
#include <cstdlib>

#include "FMatrix.h"

using namespace std;
//===========================================================================

/**
 * FMath provides numerical routines such as
 * Gauss-Jordan, lower/upper decomposition, ...  under construction
 */
namespace FMath
{

  /**
   *\par Description:
   *   Solvation of a system of equations given by A*X=B,
   *   where B contains a set of right sides that are solved
   *   simultaneously.
   * \pre
   *   first  argument has to be MxM
   *   second argument has to be MxN
   * \post
   *   A contains the inverted initial A, B[0] to B[N-1] contain the
   *   solutions to the equation for the initial B[0] to B[N-1].
   * \param a
   *   Matrix containing left side of equat-system.
   * \param b
   *   Matrix containing various right sides.
   * \return
   *   void.
   */
  void FGaussJ( FMatrix &a, FMatrix &b);

  /**
   *\par Description:
   *Solves the quadratic equation a*x^2 + b*x + c = 0 and returns the
   *number of found roots.
   *\pre
   *none
   *\post
   *The (possibly complex) roots have been set
   *\param a, b, c
   *  double parameters of the quadratic equation
   *\param x
   *  found roots
   *\return
   *  number of roots
   */
  int QuadraticEquation(double a, double b, double c, std::complex<double> x[2]);

  /**
   *\par Description:
   *Solves the cubic equation a*x^3 + b*x^2 + c*x + d = 0 and returns the
   *number of found roots..
   *\pre
   *  none
   *\post
   *  The (possibly complex) roots have been set
   *\param a, b, c, d:
   *  double parameters of the cubic equation
   *\param x
   *  found roots
   *\return
   *  number of roots
   */
  int CubicEquation(double a3, double a2, double a1, double a0, std::complex<double> x[3]);

  /**
   *\par Description:
   *  Solves the eigensystem with 2x2-matrix A.
   *\pre
   *  The matrix is 2x2.
   *\post
   *  The eigenvectors and corresponding eigenvalues have been set.
   *\param A
   *  matrix of the eigensystem.
   *\param E
   *  returned eigenvalues.
   *\param EV0, EV1
   *  alulated eigenvectors
   */
  void Eigensystem(const FMatrix& A, std::complex<double> e[2], double* EV0,
		   double* EV1);

  /**
   *\par Description:
   *Newton Raphson's method
   *\param ntrial
   *  number of trials
   *\param x
   *  first guess
   *\param tolx
   *  position change tolerance
   *\param tolf
   *  zero approximation tolerance
   *\param usrfunc
   *  given function that evaluates derivatives and Jacobian
   *  matrix of the considered scalar function.
   */
  void mnewt(int ntrial, F::FVector& x, double tolx, double tolf,
             void (*usrfun)(F::FVector, F::FVector, FMatrix));

  /**
   *\par Description:
   * Solves an equation system using the second step of the
   * lower/upper decomposition method.
   *\pre
   * The matrix a is in l/u form, indx holds the permutation vector
   * from Ludecomp.
   *\post
   * b contains the solution of the equation problem, a and indx are
   * untouched.
   *\param a
   * l/u-decomposition of the equation system.
   *\param b
   * vector containing the rightside of the equation system.
   *\param indx
   * permutation vector that was produced by the l/u-decomposition.
   */
  void Lubacksb(const FMatrix& a, F::FVector& b, positive* indx);

  /**
   *\par Description:
   * Performs the l/u-decomposition for the given matrix.
   *\pre
   * The matrix a is in l/u form, indx holds the permutation vector
   * from Ludecomp.
   *\param a
   * l/u-decomposition of the equation system.
   *\param b
   * vector containing the rightside of the equation system.
   *\param indx
   * permutation vector that was produced by the l/u-decomposition.
   *\param d
   * +1 for an even number of row interchanges, -1 for odd.
   */
  void Ludecomp(FMatrix& a, positive* indx, double& d);

  /**
   * Produce Jordan canonical form.
   * \par Description:
   * produces the jordan canonical form of the given matrix.
   * \pre

   * \param m
   *  matrix to put in jordan form.
   * \param d
   *  matrix to store the result in.
   */
  void jordanCanonicalForm( const FMatrix& m, FMatrix& d, vector< complex <double> > & eValues, double zeroTolerance= 1.0e-6);


  /**
   * \par Description:
   * This procedure takes a tetrahedron defined by 4 points and a
   * plane defined by a point and a normal vector ( n*x-p = 0 )
   * and calculates the intersection points of edges and plane, if any.
   * \param tet
   *  array of four 3D points.
   * \param n, p
   *  the plane's normal and a point in the plane.
   * \param result
   * vector containing the found intersection points (either 0, 3 or 4).
   */
  void intersectTetrahedronToPlane(const vector<FPosition>& tet,
                                   const F::FVector& n,
                                   const FPosition& p,
                                   vector<FPosition>& result);

  /**
   * \par Description:
   * This procedure takes a tetrahedron defined by 4 points and a
   * plane defined by a point and a normal vector ( n*x-p = 0 )
   * and calculates the intersection points of edges and plane, if any.
   * \param tet0, tet1, tet2, tet3
   *  the four 3D points defining the tetrahedron.
   * \param n, p
   *  the plane's normal and a point in the plane.
   * \param result
   * vector containing the found intersection points (either 0, 3 or 4).
   */
  void intersectTetrahedronToPlane(const FPosition& tet0,
                                   const FPosition& tet1,
                                   const FPosition& tet2,
                                   const FPosition& tet3,
                                   const F::FVector& n,
                                   const FPosition& p,
                                   vector<FPosition>& result);

  /**
   * \par Description
   * Calcualte an in place radix 2 decimation in time discrete fourier transform.
   * \param n
   * number of points, must be a power of two and n=2**m
   * \param m
   * log_2 of n
   * \param x, y
   * real and complex part of the data to be transformed. Must both be of size n
   */
  void inPlaceRadix2DitDft(unsigned int n, unsigned int m, vector<double>& x, vector<double>& y);
}


#endif // __FMath_hh
