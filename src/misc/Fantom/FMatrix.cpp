//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FMatrix.cc,v $
// Language:  C++
// Date:      $Date: 2004/03/17 10:23:24 $
// Author:    $Author: hlawit $
// Version:   $Revision: 1.22 $
//
//---------------------------------------------------------------------------

#ifndef NODEBUG
#include <iostream>
#endif

#include <ctype.h>
#include <cmath>
#include <complex>

#include "FMatrix.h"
#include "FMath.h"
#include "FTensor.h"
#include "FString.h"
#include "FException.h"

#include "FtQLiAlgorithm.h"

static double maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))
static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ? (iminarg1) : (iminarg2))

#undef SWAP
#define SWAP(a,b,c) {c=a;a=b;b=c;}

//===========================================================================

//---------------------------------------------------------------------------

FMatrix& FMatrix::resize(unsigned int m, unsigned int n, bool keepvalues)
{
#ifndef NODEBUG
  if (!m || !n)
    {
      FInvalidDimensionException e;
      e.addTraceMessage("FMatrix& FMatrix::resize(unsigned int m, unsigned int n, bool keepvalues)");
    }
#endif

//   positive tmpsize = m*n;

//   keepvalues=true;

//   if (!keepvalues)
//     {
//       if (sizeOfArray != tmpsize)
//         {
//           delete[]comp;
//           comp=new double[tmpsize];
//         }
//     }
//   else
//     if ((dimx != n) || (dimy!=m))
//     {
//       positive copysize = (n<dimx)?n:dimx;
//       // should be greater or equal to zero
//       positive clearsize = ((n>dimx)?n:dimx) - copysize;
//       copysize *= sizeof(double);
//       clearsize *= sizeof(double);

//       double *comp2=new double[tmpsize];

//       for ( positive i=0;i<m; i++)
//         {
//           memcpy(&comp2[i*n], &comp[i*dimx], copysize);
//           if (clearsize)
//             memset(&comp2[i*n+copysize], 0, clearsize);
//         }
//       delete[] comp;
//       comp = comp2;
//     }

//   sizeOfArray = tmpsize;
//   dimy = m;
//   dimx = n;

  FMatrix tmp(*this);

  sizeOfArray=m*n;
  if (comp) delete[] comp;
  comp=new double[sizeOfArray];
  memset(comp, 0, sizeOfArray*sizeof(double));

  if(keepvalues)
  {
    for (positive j = 0; j<m; j++)
      for (positive i = 0; i<n; i++)
        if ( (i<dimx) && (j<dimy) )
          comp[j*n+i]=tmp(j,i);
  }
  dimy=m; dimx=n;
  return (*this);
}

//---------------------------------------------------------------------------

FMatrix invert(const FMatrix& A) {

  FMatrix result(A.dimx, A.dimx);

  try {
    if (A.dimx != A.dimy)
      THROW_EXCEPTION( FInvalidDimensionException, "Matrix must be square !!!" ) ;

    switch (A.dimx)
      {
      case 1:
	{
	  if (A.comp[0] == 0.)
	    THROW_DEFAULT_EXCEPTION(FMatrixSingularException);

	  result.comp[0] = 1. / A.comp[0];
	  break;
	}
      case 2:
	{
	  double denom, a11, a21, a12, a22;
	  a11 = A.comp[0];
	  a21 = A.comp[1];
	  a12 = A.comp[2];
	  a22 = A.comp[3];
	  denom = (a11*a22-a12*a21);
	  if (denom == 0.)
	    THROW_DEFAULT_EXCEPTION( FMatrixSingularException );

	  double invdenom=1.0/denom;
	  result.comp[0] = a22 *invdenom;
	  result.comp[1] = -a12 *invdenom;
	  result.comp[2] = -a21 *invdenom;
	  result.comp[3] = a11 *invdenom;
	  break;
	}
      case 3:
	{
	  double denom, a11, a12, a13, a21, a22, a23, a31, a32, a33;
	  a11 = A.comp[0];
	  a21 = A.comp[1];
	  a31 = A.comp[2];
	  a12 = A.comp[3];
	  a22 = A.comp[4];
	  a32 = A.comp[5];
	  a13 = A.comp[6];
	  a23 = A.comp[7];
	  a33 = A.comp[8];

	  //  old:
	  //   denom = a11*a22*a33-a11*a23*a32-a21*a12*a33+a21*a13*a32+
	  //     a31*a12*a23-a31*a13*a22;
	  // I optimized determinant computation
	  // so that the scalarproduct of
	  // the crossproduct of the 2nd and the 3rd column of the matrix
	  // (= the 1st row of the inverted matrix before division)
	  // and the 1st column of the initial matrix is used (c) Max


	  result.comp[0] =  (a22*a33-a23*a32) ;
	  result.comp[1] = -(a21*a33-a23*a31) ;
	  result.comp[2] =  (a21*a32-a22*a31) ;

	  denom= result.comp[0]*a11+result.comp[1]*a12+result.comp[2]*a13;

	  if (denom == 0.) {
	    THROW_DEFAULT_EXCEPTION( FMatrixSingularException );
	  }

	  double invdenom = 1.0 / denom;

	  result.comp[0] *= invdenom;
	  result.comp[1] *= invdenom;
	  result.comp[2] *= invdenom;
	  result.comp[3] = -(a12*a33-a13*a32) * invdenom;
	  result.comp[4] =  (a11*a33-a13*a31) * invdenom;
	  result.comp[5] = -(a11*a32-a12*a31) * invdenom;
	  result.comp[6] =  (a12*a23-a13*a22) * invdenom;
	  result.comp[7] = -(a11*a23-a13*a21) * invdenom;
	  result.comp[8] =  (a11*a22-a12*a21) * invdenom;
	  break;
	}
      default:
	{
	  // a la Tom
      //
      // Explanation:
      // We invert by solving a system without right side,
      // thus we need an invalid Matrix B which is empty
      // but its dimy must match A.dimy
      //
	  FMatrix B(0, 0);
	  B.dimx=0;
	  B.dimy= A.dimy;
	  result = A;
	  result.gaussJ(B);
	}
      }
  }
  CATCH_N_RETHROW(FException);

  return result;
}

//---------------------------------------------------------------------------

double FMatrix::invert (void )
{
#ifndef NODEBUG
  if (dimy!=dimx)
    THROW_EXCEPTION( FException, "Matrix must be square !!!" );
#endif


  switch (dimx)
    {
    case 1:
      {
        if (comp[0] == 0.)
          THROW_DEFAULT_EXCEPTION( FMatrixSingularException );
        double denom=comp[0];
        comp[0] = 1. / denom;
        return denom;
      }
    case 2:
      {
        double denom, a11, a21, a12, a22;
        a11 = comp[0];
        a21 = comp[1];
        a12 = comp[2];
        a22 = comp[3];
        denom = (a11*a22-a12*a21);
        if (denom == 0.)
          THROW_DEFAULT_EXCEPTION( FMatrixSingularException );

	double invdenom=1.0/denom;

        comp[0] = a22 *invdenom;
        comp[1] = -a12 *invdenom;
        comp[2] = -a21 *invdenom;
        comp[3] = a11 *invdenom;
        return denom;
      }
    case 3:
      {
        double denom, a11, a12, a13, a21, a22, a23, a31, a32, a33;
        a11 = comp[0];
        a21 = comp[1];
        a31 = comp[2];
        a12 = comp[3];
        a22 = comp[4];
        a32 = comp[5];
        a13 = comp[6];
        a23 = comp[7];
        a33 = comp[8];
	//  old:
	//   denom = a11*a22*a33-a11*a23*a32-a21*a12*a33+a21*a13*a32+
        //     a31*a12*a23-a31*a13*a22;
	// I optimized determinant computation
	// so that the scalarproduct of
	// the crossproduct of the 2nd and the 3rd column of the matrix
	// (= the 1st row of the inverted matrix before division)
	// and the 1st column of the initial matrix is used (c) Max


        comp[0] =  (a22*a33-a23*a32) ;
        comp[1] = -(a21*a33-a23*a31) ;
        comp[2] =  (a21*a32-a22*a31) ;

	denom= comp[0]*a11+comp[1]*a12+comp[2]*a13;

        if (denom == 0.) {
          THROW_DEFAULT_EXCEPTION( FMatrixSingularException );
	}

	double invdenom=1.0/denom;

	comp[0] *= invdenom;
	comp[1] *= invdenom;
	comp[2] *= invdenom;
        comp[3] = -(a12*a33-a13*a32) * invdenom;
        comp[4] =  (a11*a33-a13*a31) * invdenom;
        comp[5] = -(a11*a32-a12*a31) * invdenom;
        comp[6] =  (a12*a23-a13*a22) * invdenom;
        comp[7] = -(a11*a23-a13*a21) * invdenom;
        comp[8] =  (a11*a22-a12*a21) * invdenom;
        return denom;
      }
    default:
      {
        // a la Tom
        FMatrix B(0,0);
        B.dimx=0;
        B.dimy=dimy;
        // this call should invert this Matrix without needing too many overhead
        gaussJ(B);

	//no return of denominator:
	return 0;
      }
    }
}

//---------------------------------------------------------------------------

void FMatrix::tridiagonalize(F::FVector& d, F::FVector& e)
{
  positive n=dimy;

//#ifndef NODEBUG
  if(n!=dimx)
      THROW_EXCEPTION( FException, "matrix must be square" );
//#endif

  d.resize(n);
  e.resize(n);

  FMath::tridiagonalize( FMath::RowMajor( comp, n ), d, e );
}

//---------------------------------------------------------------------------

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

void FMatrix::tQLiAlgorithm(F::FVector& d, F::FVector& e)
{
  positive n=d.getDimension();

  FMath::tQLiAlgorithm( FMath::RowMajor( comp, n ), d, e );
}

//---------------------------------------------------------------------------

void FMatrix::getEigenSystem( std::vector< std::complex< double > >& evals,
			      std::vector< F::FVector >& evecs )
{
  if( dimx == 3 )
  {
    static double epsilon = 1.0e-6;

    static positive indices[3][2]={ {1,2}, {0,2}, {0,1} };

    getEigenValues( evals );
    evecs.resize( evals.size() );
    FMatrix matrix(3,3);
    double det[3];
    positive *col, *row;
    positive maxi(0);
    double maxdet;
    for ( positive i=0 ; i<evals.size() ; ++i )
    {
	if ( !evals[i].imag() )
	{
	    // compute eigenvector associated with real eigenvalue

	    // A-\lambda I_3
	    matrix = *this;
	    for ( positive j=0 ; j<3 ; ++j )
		matrix(j,j) -= evals[i].real();

	    // loop over scalar components of the eigenvector to
	    // find one that is not zero
	    for ( positive j=0 ; j<3 ; ++j )
	    {
		col = indices[j];

		// assume evec[j]=1 i.e. is non zero
		// compute determinant of associated subsystems
		for ( positive k=0 ; k<3 ; ++k )
		{
		    row = indices[k];
		    det[k] = matrix(row[0],col[0])*matrix(row[1],col[1])-
			matrix(row[1],col[0])*matrix(row[0],col[1]);
		}

		// get maximum
		maxdet = 0;
		for ( positive k=0 ; k<3 ; ++k )
		    if ( fabs(det[k]) > maxdet )
		    {
			maxi = k;
			maxdet = fabs(det[k]);
		    }

		if ( maxdet < epsilon )
		    continue;

                // can solve the system by setting
		// maxi-th coordinate equal to 1
		row = indices[maxi];
		det[maxi] = 1./det[maxi];
		evecs[i].resize(3);
		evecs[i]( col[0] ) = det[maxi]*
		    ( -matrix( row[0], j )*matrix( row[1], col[1] )+
		      matrix( row[1], j )*matrix( row[0], col[1] ) );
		evecs[i]( col[1] ) = det[maxi]*
		    ( -matrix( row[0], col[0] )*matrix( row[1], j )+
		      matrix( row[1], col[0] )*matrix( row[0], j ) );
		evecs[i]( j ) = 1.;
	    }
	    if ( !evecs[i].size() )
	    {
#ifndef NODEBUG
	      std::cout << "degenerate eigensystem: can't solve it!" << std::endl;
	      std::cout << "corresponding matrix is" << std::endl << matrix << std::endl;
#endif
		return;
	    }
	}
    }
  }
  else
  { // dimx == 2
    FMatrix matrix = *this;
    evals.resize(2);
    FMath::QuadraticEquation(1., -(matrix(0,0)+matrix(1,1)),
			     matrix(0,0)*matrix(1,1)-matrix(0,1)*matrix(1,0),
			     &(evals[0]) );

    if ( evals.size() != 2 || evals[0].imag() || evals[1].imag() )
      THROW_DEFAULT_EXCEPTION( FNotImplementedException );

    if ( evals[0].real() < evals[1].real() ) {
      complex<double> q = evals[0];
      evals[0] = evals[1];
      evals[1] = q;
    }

    static double epsilon = 1.0e-6;
    evecs.resize( evals.size() );

    evecs[0].resize(2);
    evecs[0](0) = -matrix(0,1)/(matrix(0,0)-evals[0].real());
    evecs[0](1) = 1;

    if (fabs(fabs(evecs[0](0)) -
	     fabs((matrix(1,1) - evals[0].real())/matrix(1,0))) > epsilon)
      throw FException("Cannot solve eigensystem.");

    evecs[0].normalize();

    evecs[1].resize(2);
    evecs[1](0) = -matrix(0,1)/(matrix(0,0)-evals[1].real());
    evecs[1](1) = 1;

    if (fabs(fabs(evecs[1](0)) -
	     fabs((matrix(1,1)-evals[1].real())/matrix(1,0))) > epsilon)
      throw FException("Cannot solve eigensystem.");

    evecs[1].normalize();
  }
}

//---------------------------------------------------------------------------

void FMatrix::getEigenSystem(F::FVector& vals, std::vector<F::FVector>& vectors)
{
  positive dim = dimy;

  vals.resize(dim);
  vectors.clear();

  positive x;

  F::FVector di, ed, v(dim);

  //  std::cerr <<"Matrix: "<<std::endl<<(*this);

  tridiagonalize(di, ed);  // First of all transform tensor into tridiagonal form

  tQLiAlgorithm(di, ed);   // Now calculate Eigenvectors and -values by QLi alg.

  for(x=0; x<dim; x++) vals[x]=di[x];  // Copy Eigenvalues

  // Copy Eigenvectors...

  for(unsigned int y=0; y<dim; y++)
    {
      for(x=0; x<dim; x++) v[x] = (*this)(x,y);
      vectors.push_back(v);
    }
}

//---------------------------------------------------------------------------

FMatrix& FMatrix::gaussJ( FMatrix &b )
// PAR: a[1..n][1..n] contains the matrix of the system to solve
//	n   is the number of rows and columns in the system
//	b[1..n][1..m] contains m right sides, which are solved simultan
// PRE: no singular matrix. TESTED
// POST: **a contains the inverse of the original matrix
//       **b conatins the solutions for the m right sides
// REMARK: the program was taken from "Numerical Recipes in C", p.39/40
{
    int *indxc, *indxr, *ipiv;
    int i, icol(0), irow(0), j, k, l, ll, n;
    double big;
    double dum, pivinv, c;

#ifndef NODEBUG
    if (dimx != dimy)
      THROW_EXCEPTION( FException, "ERROR: not a square matrix!");

    if (dimx != b.dimy)
      THROW_EXCEPTION( FInvalidDimensionException, "ERROR: incompatible dimensions!");
#endif

    n = b.getDimensionX();

    indxc=new int[dimx];
    indxr=new int[dimx];
    ipiv=new int[dimx];

    // IPIV preloaden
    for (j=0;j<(int)dimx;j++) ipiv[j]=0;

  //
  for (i=0; i<(int)dimx; i++)
  {
    big=0.0;
    for (j=0;j<(int)dimx;j++)
      if (ipiv[j] != 1)
        for (k=0;k<(int)dimx;k++)
          {
            if (ipiv[k] == 0)
              {
                if (fabs((*this)(j,k))>=big)
                  {
                    big=fabs((*this)(j,k));
                    irow=j;
                    icol=k;
                  }
              }
            // NO IDEA WHY THIS INDICATES SINGULARITY. SINCE IT DOESNT WORK
            // I KICKED IT (tom)
#ifndef NODEBUG
//             else
//               {
//                 FException e;
//                 e.setErrorMessage("FMatrix.invert: Singular Matrix-1");
//                 e.addTraceMessage("FMatrix& FMatrix::gaussJ( FMatrix &m ) ");
//                 throw e;
//               }
#endif
          }
    ++(ipiv[icol]);
    // pivot found, interchange rows if necessary
    if (irow != icol)
      {
        for (l=0;l<(int)dimx;l++) SWAP((*this)(irow,l), (*this)(icol,l), c);
        for (l=0;l<n;l++) SWAP(b(irow,l), b(icol,l), c);
      }

    indxr[i]=irow;
    indxc[i]=icol;
    // ErrorCheck((*this)(icol,icol) == 0.0, "gaussj: Singular Matrix-2");

    pivinv=1.0/(*this)(icol,icol);
    (*this)(icol,icol)=1.0;
    for (l=0;l<(int)dimx;l++) (*this)(icol,l) *= pivinv;
    for (l=0;l<n;l++) b(icol,l) *= pivinv;
    for (ll=0;ll<(int)dimx;ll++)
      if (ll!=icol)
        {
          dum=(*this)(ll,icol);
          (*this)(ll,icol)=0.0;
          for (l=0;l<(int)dimx;l++) (*this)(ll,l) -= (*this)(icol,l)*dum;
          for (l=0;l<n;l++) b(ll,l) -= b(icol,l)*dum;
        }
  }
  // system solved, interchange pairs of columns back in original order :
  for (l=(int)dimx-1;l>=0;l--)
    {
      if (indxr[l] != indxc[l])
        for (k=0;k<(int)dimx;k++)
          SWAP((*this)(k,indxr[l]), (*this)(k,indxc[l]), c);
    }
  delete[] ipiv;
  delete[] indxr;
  delete[] indxc;
  return b;
}

//---------------------------------------------------------------------------

F::FVector& FMatrix::gaussJ( F::FVector &v )
// PAR: a[1..n][1..n] contains the matrix of the system to solve
//	n   is the number of rows and columns in the system
//	b[1..n][1..m] contains m right sides, which are solved simultan
// PRE: no singular matrix. TESTED
// POST: **a contains the inverse of the original matrix
//       **b conatins the solutions for the m right sides
// REMARK: the program was taken from "Numerical Recipes in C", p.39/40
{
    int *indxc, *indxr, *ipiv;
    int i, icol(0), irow(0), j, k, l, ll;
    double big;
    double dum, pivinv, c;
#ifndef NODEBUG
    if (dimx != dimy)
      THROW_EXCEPTION( FException, "ERROR: not a square matrix!");

    if (dimx != v.sizeOfArray)
      THROW_EXCEPTION( FException, "ERROR: incompatible dimensions!");
#endif

    indxc=new int[dimx];
    indxr=new int[dimx];
    ipiv=new int[dimx];

    // IPIV preloaden
    for (j=0;j<(int)dimx;j++) ipiv[j]=0;

  //
  for (i=0; i<(int)dimx; i++)
  {
    big=0.0;
    for (j=0;j<(int)dimx;j++)
      if (ipiv[j] != 1)
        for (k=0;k<(int)dimx;k++)
          {
            if (ipiv[k] == 0)
              {
                if (fabs((*this)(j,k))>=big)
                  {
                    big=fabs((*this)(j,k));
                    irow=j;
                    icol=k;
                  }

              }
            // NO IDEA WHY THIS INDICATES SINGULARITY. SINCE IT DOESNT WORK
            // I KICKED IT (tom)
#ifndef NODEBUG
//             else
//               {
//                 FException e;
//                 e.setErrorMessage("FMatrix.invert: Singular Matrix-1");
//                 e.addTraceMessage("F::FVector& FMatrix::gaussJ( FMatrix &v ) ");
//                 throw e;
//               }
#endif
          }
    ++(ipiv[icol]);
    // pivot found, interchange rows if necessary
    if (irow != icol)
      {
        for (l=0;l<(int)dimx;l++) SWAP((*this)(irow,l), (*this)(icol,l), c);
        SWAP(v(irow), v(icol), c);
      }

    indxr[i]=irow;
    indxc[i]=icol;
    // ErrorCheck((*this)(icol,icol) == 0.0, "gaussj: Singular Matrix-2");

    pivinv=1.0/(*this)(icol,icol);
    (*this)(icol,icol)=1.0;
    for (l=0;l<(int)dimx;l++) (*this)(icol,l) *= pivinv;
    v(icol) *= pivinv;
    for (ll=0;ll<(int)dimx;ll++)
      if (ll!=icol)
        {
          dum=(*this)(ll,icol);
          (*this)(ll,icol)=0.0;
          for (l=0;l<(int)dimx;l++) (*this)(ll,l) -= (*this)(icol,l)*dum;
          v(ll) -= v(icol)*dum;
        }
  }
  // system solved, interchange pairs of columns back in original order :
  for (l=(int)dimx-1;l>=0;l--)
    {
      if (indxr[l] != indxc[l])
        for (k=0;k<(int)dimx;k++)
          SWAP((*this)(k,indxr[l]), (*this)(k,indxc[l]), c);
    }
  delete[] ipiv;
  delete[] indxr;
  delete[] indxc;
  return v;
}

#undef SWAP

//---------------------------------------------------------------------------
void FMatrix::swapCols (positive a, positive b)
{
#ifndef NODEBUG
  if ((a>= dimx)||(b>=dimx))
    THROW_DEFAULT_EXCEPTION( FIndexOutOfBoundsException );
#endif

  if (a==b) return;
  double *j, *k, tmp;
  j = &comp[a];
  k = &comp[b];
  for ( positive i=0;i<dimy;i++ )
    { tmp=*j; *j=*k; *k=tmp;
      j+=dimx*sizeof(double);
      k+=dimx*sizeof(double);
    }
}

//---------------------------------------------------------------------------
void FMatrix::swapRows (positive a, positive b)
{
#ifndef NODEBUG
  if ((a>= dimy)||(b>=dimy))
    THROW_DEFAULT_EXCEPTION( FIndexOutOfBoundsException );
#endif

  if (a==b) return;

  double *j = new double[dimx];
  memcpy( j, &comp[a*dimx], dimx*sizeof(double) );
  memcpy( &comp[a*dimx], &comp[b*dimx], dimx*sizeof(double) );
  memcpy( &comp[b*dimx], j, dimx*sizeof(double) );
}

//---------------------------------------------------------------------------

ostream& operator<< ( ostream& os, const FMatrix& matrix )
{
  positive i,j;

  for( i=0; i<matrix.dimy; i++ )
  {
      os << (i?" [":"[[");
      for (j=0; j< matrix.dimx; j++)
      {
	  if (j<matrix.dimx-1)
	      os << matrix[i*matrix.dimx+j] << ",\t";
	  else
	      os << matrix[i*matrix.dimx+j] << ((i<matrix.dimy-1) ? "]":"]]") << "\n";
      }
  }

  return os;
}

//---------------------------------------------------------------------------

istream& operator>> ( istream& is , FMatrix& a)
{
  char c=' ';
  positive olddimx, dimx=0, dimy=0, i;
  // jump the first whitespaces

  vector<double> tmp;
  double tmpdouble;

  while (isspace(c) && (is >> c));
  if (c != '[')
    { is.putback(c);
    THROW_EXCEPTION( FSyntaxErrorException, "no matrix definition found");
    }

  c=' ';

  while (1)
    {

      // If the Matrix definition has ended there should appear
      // a character different from '['
      while (isspace(c) && (is >> c));
      if (c == ']')
        { break;}

      if ((c != '[')|| (!is))
        THROW_EXCEPTION( FSyntaxErrorException, "Matrix input did not finish correctly");

      olddimx = dimx;
      dimx=0;
      while ( is >> tmpdouble >> c )
        {
          dimx++;
          tmp.push_back ( tmpdouble );

          // finished !
          if (c==']') break;

          // if operation did not succeed
          if (c!=',')
            {
              is.clear(ios::badbit);
              break;
            }
        }
      if (olddimx)
        if (olddimx!=dimx)
          THROW_EXCEPTION( FSyntaxErrorException, "Matrix width is not constant");

      dimy++;
      c = ' ';
    }

  if ((dimx == 0) || (dimy == 0))
    THROW_EXCEPTION( FSyntaxErrorException, "No Matrix Definition Found");

  if (!is)
    THROW_EXCEPTION( FSyntaxErrorException, "Matrix input did not finish correctly");

  a.resize(dimy, dimx);
  for (i=0; i<a.sizeOfArray; i++)
    a.comp[i]=tmp[i];

  return is;
}

//---------------------------------------------------------------------------

void FMatrix::makeDiagonal(const F::FVector& v)
{
  dimx = dimy = v.sizeOfArray;
  if (sizeOfArray != dimx*dimy)
    {
      sizeOfArray=dimx*dimy;
      delete[] comp;
      comp=new double[sizeOfArray];
    }

  memset(comp, 0, sizeOfArray*sizeof(double));
  for (positive j=0, i=0;i<sizeOfArray;i+=dimx+1, j++)
    {
      comp[i]=v[j];
    }
}

//---------------------------------------------------------------------------

void FMatrix::makeDiagonal(const double& d, positive dim)
{
  // Matrix taken as is if no dimension is given explicitly
  if (!dim)
    {
      if (dimx != dimy)
        THROW_EXCEPTION( FException, "Matrix is not square");
    }
  else
    dimx = dimy = dim;

  if (sizeOfArray != dimx*dimy)
    {
      sizeOfArray=dimx*dimy;
      delete[] comp;
      comp=new double[sizeOfArray];
    }

  memset(comp, 0, sizeOfArray*sizeof(double));
  for (positive i=0;i<sizeOfArray;i+=(dimx+1))
    {
      comp[i]=d;
    }
}

//---------------------------------------------------------------------------

void FMatrix::makeIdendity( )
{
  makeScaling( 1. );
}

//---------------------------------------------------------------------------

void FMatrix::makeScaling(double d)
{
  // Matrix taken as is if no dimension is given explicitly
  if (dimx != dimy)
    THROW_EXCEPTION( FException, "Matrix is not square");

  if (sizeOfArray != dimx*dimy)
    {
      sizeOfArray=dimx*dimy;
      delete[] comp;
      comp=new double[sizeOfArray];
    }

  memset(comp, 0, sizeOfArray*sizeof(double));
  for (positive i=0;i<sizeOfArray;i+=(dimx+1))
    {
      comp[i]=d;
    }
}


//---------------------------------------------------------------------------

void FMatrix::set(const F::FVector & v)
{
#ifndef NODEBUG
    if( this->dimx * this->dimy != v.sizeOfArray )
        THROW_DEFAULT_EXCEPTION( FInvalidDimensionException );
#endif
    memcpy( comp, v.comp, v.sizeOfArray*sizeof(double));
}

//---------------------------------------------------------------------------

FMatrix FMatrix::operator *(const FMatrix &m) const
{
  positive i,j,k;
  double *src1, *src2, *dst;

#ifndef NODEBUG
  if (m.dimy != dimx)
      THROW_EXCEPTION( FInvalidDimensionException, "MatrixMultiplication only permittet for (NxM) * (MxK)");

  if ((dimx==0)||(dimy==0)||(m.dimx==0))
      THROW_EXCEPTION( FInvalidDimensionException, "Sorry Matrices must have Dimension != Zero !");

#endif

  FMatrix result(dimy ,m.dimx); // new Matrix with combined dimensions initialized to zero

#ifndef NODEBUG
  try
    {
#endif
      for( i=0; i<dimy; i++ )
        for( k=0; k<m.dimx; k++ )
          {
            dst = &result.comp[i*m.dimx+k];
            src1 = &comp[i*dimx];
            src2 = &m.comp[k];
            for( j=0; j<dimx; j++ )
              {
                (*dst) += (*src1) * (*src2);
                src1++;
                src2+=m.dimx;
              }
          }
#ifndef NODEBUG
    }
  CATCH_N_RETHROW(FIndexOutOfBoundsException)
#endif

  return result;
}

//---------------------------------------------------------------------------

FMatrix& FMatrix::operator *=(const FMatrix &m)
{
  positive i,j,k;
  double * src1, *src2, *dst, *comp2;

#ifndef NODEBUG
  if (m.dimy != dimx)
      THROW_EXCEPTION( FInvalidDimensionException, "MatrixMultiplication only permittet for (NxM) * (MxK)");
  if ((dimx==0)||(dimy==0)||(m.dimx==0))
      THROW_DEFAULT_EXCEPTION( FEmptyObjectException );
#endif


  // make the new array...
  sizeOfArray = m.dimx*dimy;
  comp2 = new double[sizeOfArray];
  memset(comp2, 0, sizeOfArray*sizeof(double));

#ifndef NODEBUG
  try
    {
#endif
      for( i=0; i<dimy; i++ )
        for( k=0; k<m.dimx; k++ )
          {
            dst = &comp2[i*m.dimx+k];
            src1 = &comp[i*dimx];
            src2 = &m.comp[k];

            for( j=0; j<dimx; j++ )
              {
                (*dst) += (*src1) * (*src2);
                src1++;
                src2+=m.dimx;
              }
          }
#ifndef NODEBUG
    }
  CATCH_N_RETHROW( FIndexOutOfBoundsException );
#endif

  // und die neue matrix �bernehmen
  delete[] comp;
  comp = comp2;
  // only x-dimension should have changed
  dimx=m.dimx;

  return (*this);
}

//---------------------------------------------------------------------------

FMatrix& FMatrix::mult(const FMatrix &m, FMatrix &d) const
{
  positive i,j,k;
  double *src1, *src2, *dst;

#ifndef NODEBUG
  if (m.dimy != dimx)
      THROW_EXCEPTION( FInvalidDimensionException, "MatrixMultiplication only permittet for (NxM) * (MxK)");
  if ((dimx==0)||(dimy==0)||(m.dimx==0))
      THROW_EXCEPTION( FInvalidDimensionException, "Sorry Matrices must have Dimension != Zero !");
#endif


  d.dimy = dimy;
  d.dimx = m.dimx;

  if ( d.sizeOfArray != m.dimx*dimy )
    {
      // make the new array...
      d.sizeOfArray = dimy*m.dimx;
      d.comp = new double[d.sizeOfArray];
    }
  memset(d.comp, 0, d.sizeOfArray*sizeof(double));

#ifndef NODEBUG
  try
    {
#endif
      for( i=0; i<dimy; i++ )
        for( k=0; k<m.dimx; k++ )
          {
            dst = &d.comp[i*m.dimx+k];
            src1 = &comp[i*dimx];
            src2 = &m.comp[k];

            for( j=0; j<dimx; j++ )
              {
                (*dst) += (*src1) * (*src2);
                src1++;
                src2+=m.dimx;
              }
          }
#ifndef NODEBUG
    }
  CATCH_N_RETHROW( FIndexOutOfBoundsException );
#endif

  return d;
}


//---------------------------------------------------------------------------

F::FVector FMatrix::operator*(const F::FVector &v) const
{
  positive i, j;
  F::FVector result(dimy); // should be initialized to zero
  double *src, *dst;

#ifndef NODEBUG
  if (dimx != v.sizeOfArray)
    {
		cout << dimx << " " << v.sizeOfArray << endl;
		THROW_EXCEPTION(FInvalidDimensionException, "Vector has to be same size as width of Matrix");
    }
#endif

  dst=result.comp;
  for(i=0; i<dimy; i++)
    {
      src = &comp[i*dimx]; // start in row i
      for(j=0; j<dimx; j++)
        {
          (*dst) += v[j]* (*src);
          src++;
        }
      dst++;
    }
  return result;
}

//---------------------------------------------------------------------------

F::FVector& FMatrix::mult(const F::FVector &v, F::FVector &d) const
{
  positive i, j;
  double *src, *dst;

  if (d.sizeOfArray != dimy)
    d.resize(dimy);

#ifndef NODEBUG
  if (dimx != v.sizeOfArray)
      THROW_EXCEPTION( FInvalidDimensionException, "Vector has to be same size as width of Matrix");
#endif

  dst= &d.comp[0];
  memset(dst, 0, dimy*sizeof(double));
  for(i=0; i<dimy; i++)
    {
      src = &comp[i*dimx]; // start in row i
      for(j=0; j<dimx; j++)
        {
          (*dst) += v[j]* (*src);
          src++;
        }
      dst++;
    }
  return d;
}

//---------------------------------------------------------------------------

F::FVector& FMatrix::mult(const FTensor &v, F::FVector &d) const
{
  unsigned char i, j;
  double *src, *dst;

  if (d.getDimension() != dimy)
    d.resize(dimy);

#ifndef NODEBUG
  if (dimx != v.sizeOfArray)
      THROW_EXCEPTION( FInvalidDimensionException, "Vector has to be same size as width of Matrix");
#endif

  dst= &d.comp[0];
  memset(dst, 0, dimy*sizeof(double));
  for(i=0; i<dimy; i++)
    {
      src = &comp[i*dimx]; // start in row i
      for(j=0; j<dimx; j++)
        {
          (*dst) += v(j)* (*src);
          src++;
        }
      dst++;
    }
  return d;
}

//---------------------------------------------------------------------------

FTensor& FMatrix::mult(const FTensor &v, FTensor &d) const
{
  unsigned char i, j;
  double *src, *dst;

#ifndef NODEBUG
  if (dimx != v.sizeOfArray)
    THROW_EXCEPTION( FInvalidDimensionException, "Tensor has to be same size as width of Matrix");
  if (v.getOrder() != 1)
    THROW_EXCEPTION( FInvalidDimensionException, "Tensor order has to be 1, i.e. a vector");
#endif

  if (d.getDimension() != dimy || d.getOrder() != 1)
    d.resizeTensor((unsigned char)dimy,1);

  dst= &d.comp[0];
  memset(dst, 0, dimy*sizeof(double));
  for(i=0; i<dimy; i++)
    {
      src = &comp[i*dimx]; // start in row i
      for(j=0; j<dimx; j++)
        {
          (*dst) += v(j)* (*src);
          src++;
        }
      dst++;
    }
  return d;
}

//---------------------------------------------------------------------------

void FMatrix::svdcmp(F::FVector& w, FMatrix& v)
{
  // Given a matrix a[1..m][1..n], this routine computes its singular value
  // decomposition, A =U  W  V T . The matrix U replaces a on output.
  // The diagonal matrix of singular values W is output as a vector w[1..n].
  // The matrix V (not the transpose V T ) is output as v[1..n][1..n].
  unsigned int m = getDimensionY(), n = getDimensionX();

  unsigned int flag,i,its,j,jj,k,l(0),nm(0);
  double anorm,c,f,g,h,s,scale,x,y,z;

  w.resize(n);
  v.resize(n,n);

  F::FVector rv1(n);

  g=scale=anorm=0.0; // Householder reduction to bidiagonal form.
  for (i=1;i<=n;i++)
    {
      l=i+1;
      rv1[i-1]=scale*g;
      g=s=scale=0.0;
      if (i <= m)
	{
	  for (k=i;k<=m;k++) scale += fabs((*this)(k-1,i-1));
	  if (scale)
	    {
	      for (k=i;k<=m;k++)
		{
		  (*this)(k-1,i-1) /= scale;
		  s += (*this)(k-1,i-1)*(*this)(k-1,i-1);
		}
	      f=(*this)(i-1,i-1);
	      g = -SIGN(sqrt(s),f);
	      h=f*g-s;
	      (*this)(i-1,i-1)=f-g;
	      for (j=l;j<=n;j++)
		{
		  for (s=0.0,k=i;k<=m;k++) s += (*this)(k-1,i-1)*(*this)(k-1,j-1);
		  f=s/h;
		  for (k=i;k<=m;k++) (*this)(k-1,j-1) += f*(*this)(k-1,i-1);
		}
	      for (k=i;k<=m;k++) (*this)(k-1,i-1) *= scale;
	    }
	}
      w[i-1]=scale *g;
      g=s=scale=0.0;
      if (i <= m && i != n)
	{
	  for (k=l;k<=n;k++) scale += fabs((*this)(i-1,k-1));
	  if (scale)
	    {
	      for (k=l;k<=n;k++)
		{
		  (*this)(i-1,k-1) /= scale;
		  s += (*this)(i-1,k-1)*(*this)(i-1,k-1);
		}
	      f=(*this)(i-1,l-1);
	      g = -SIGN(sqrt(s),f);
	      h=f*g-s;
	      (*this)(i-1,l-1)=f-g;
	      for (k=l;k<=n;k++) rv1[k-1]=(*this)(i-1,k-1)/h;
	      for (j=l;j<=m;j++)
		{
		  for (s=0.0,k=l;k<=n;k++) s += (*this)(j-1,k-1)*(*this)(i-1,k-1);
		  for (k=l;k<=n;k++) (*this)(j-1,k-1) += s*rv1[k-1];
		}
	      for (k=l;k<=n;k++) (*this)(i-1,k-1) *= scale;
	    }
	}
      anorm=FMAX(anorm,(fabs(w[i-1])+fabs(rv1[i-1])));
    }
  for (i=n;i>=1;i--) //  Accumulation of right-hand transformations.
    {
      if (i < n)
	{
	  if (g)
	    {
	      for (j=l;j<=n;j++) // Double division to avoid possible under ow.
		v(j-1,i-1)=((*this)(i-1,j-1)/(*this)(i-1,l-1))/g;
	      for (j=l;j<=n;j++)
		{
		  for (s=0.0,k=l;k<=n;k++) s += (*this)(i-1,k-1)*v(k-1,j-1);
		  for (k=l;k<=n;k++) v(k-1,j-1) += s*v(k-1,i-1);
		}
	    }
	  for (j=l;j<=n;j++) v(i-1,j-1)=v(j-1,i-1)=0.0;
	}
      v(i-1,i-1)=1.0;
      g=rv1[i-1];
      l=i;
    }
  for (i=IMIN(m,n);i>=1;i--) // Accumulation of left-hand transformations.
    {
      l=i+1;
      g=w[i-1];
      for (j=l;j<=n;j++) (*this)(i-1,j-1)=0.0;
      if (g)
	{
	  g=1.0/g;
	  for (j=l;j<=n;j++)
	    {
	      for (s=0.0,k=l;k<=m;k++) s += (*this)(k-1,i-1)*(*this)(k-1,j-1);
	      f=(s/(*this)(i-1,i-1))*g;
	      for (k=i;k<=m;k++) (*this)(k-1,j-1) += f*(*this)(k-1,i-1);
	    }
	  for (j=i;j<=m;j++) (*this)(j-1,i-1) *= g;
	} else for (j=i;j<=m;j++) (*this)(j-1,i-1)=0.0;
      ++(*this)(i-1,i-1);
    }
  for (k=n;k>=1;k--) // Diagonalization of the bidiagonal form: Loop over
    {                // singular values, and over allowed iterations.
      for (its=1;its<=30;its++)
	{
	  flag=1;
	  for (l=k;l>=1;l--) // Test for splitting.
	    {		     // Note that rv1[0] is always zero.
	      nm=l-1;
	      if ((double)(fabs(rv1[l-1])+anorm) == anorm)
		{
		  flag=0;
		  break;
		}
	      if ((double)(fabs(w[nm-1])+anorm) == anorm) break;
	    }
	  if (flag)
	    {
	      c=0.0; // Cancellation of rv1[0], if l > 1.
	      s=1.0;
	      for (i=l;i<=k;i++)
		{
		  f=s*rv1[i-1];
		  rv1[i-1]=c*rv1[i-1];
		  if ((double)(fabs(f)+anorm) == anorm) break;
		  g=w[i-1];
		  h=FMath::pythag(f,g);
		  w[i-1]=h;
		  h=1.0/h;
		  c=g*h;
		  s = -f*h;
		  for (j=1;j<=m;j++)
		    {
		      y=(*this)(j-1,nm-1);
		      z=(*this)(j-1,i-1);
		      (*this)(j-1,nm-1)=y*c+z*s;
		      (*this)(j-1,i-1)=z*c-y*s;
		    }
		}
	    }
	  z=w[k-1];
	  if (l == k) // Convergence.
	    {
	      if (z < 0.0) // Singular value is made nonnegative.
		{
		  w[k-1] = -z;
		  for (j=1;j<=n;j++) v(j-1,k-1) = -v(j-1,k-1);
		}
	      break;
	    }
	  if (its == 30)
	      THROW_EXCEPTION(FTooManyIterationsException, "No convergence in 30 svdcmp iterations");

	  x=w[l-1]; // Shift from bottom 2-by-2 minor.
	  nm=k-1;
	  y=w[nm-1];
	  g=rv1[nm-1];
	  h=rv1[k-1];
	  f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
	  g=FMath::pythag(f,1.0);
	  f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
	  c=s=1.0; // Next QR transformation:
	  for (j=l;j<=nm;j++)
	    {
	      i=j+1;
	      g=rv1[i-1];
	      y=w[i-1];
	      h=s*g;
	      g=c*g;
	      z=FMath::pythag(f,h);
	      rv1[j-1]=z;
	      c=f/z;
	      s=h/z;
	      f=x*c+g*s;
	      g = g*c-x*s;
	      h=y*s;
	      y *= c;
	      for (jj=1;jj<=n;jj++)
		{
		  x=v(jj-1,j-1);
		  z=v(jj-1,i-1);
		  v(jj-1,j-1)=x*c+z*s;
		  v(jj-1,i-1)=z*c-x*s;
		}
	      z=FMath::pythag(f,h);
	      w[j-1]=z;  // Rotation can be arbitrary if z = 0.
	      if (z)
		{
		  z=1.0/z;
		  c=f*z;
		  s=h*z;
		}
	      f=c*g+s*y;
	      x=c*y-s*g;

	      for (jj=1;jj<=m;jj++)
		{
		  y=(*this)(jj-1,j-1);
		  z=(*this)(jj-1,i-1);
		  (*this)(jj-1,j-1)=y*c+z*s;
		  (*this)(jj-1,i-1)=z*c-y*s;
		}
	    }
	  rv1[l-1]=0.0;
	  rv1[k-1]=f;
	  w[k-1]=x;
	}
    }
}

//---------------------------------------------------------------------------

void FMatrix::svDecomp(const FMatrix& u, const F::FVector& w, const FMatrix& v, const F::FVector& b, F::FVector& x)
{
  // Solves A  X = B for a vector X, where A is specied by the arrays u[1..m][1..n],
  // w[1..n], v[1..n][1..n] as returned by svdcmp. m and n are the dimensions of a, and
  // will be equal for square matrices. b[1..m] is the input right-hand side. x[1..n]
  // is the output solution vector. No input quantities are destroyed, so the routine
  // may be called sequentially with different b's.
#ifndef NODEBUG
  if(v.getDimensionX() != v.getDimensionY())
	  THROW_EXCEPTION( FInvalidDimensionException, "v is not square matrix" );
  if(u.getDimensionX() != v.getDimensionX())
	  THROW_EXCEPTION( FInvalidDimensionException, "dimension of u must match dimension of v" );
  if(u.getDimensionY() != b.size())
	  THROW_EXCEPTION( FInvalidDimensionException, "size of b must match dimension of u" );
  if(u.getDimensionX() != x.size())
	  THROW_EXCEPTION( FInvalidDimensionException, "size of x must match dimension of u" );
#endif
  int m = u.getDimensionY(), n = u.getDimensionX();

  int jj,j,i;
  double s;
  F::FVector tmp(n);

  for (j=1;j<=n;j++) // Calculate U T B.
    {
      s=0.0;
      if (w[j-1]) // Nonzero result only if wj is nonzero.
	{
	  for (i=1;i<=m;i++) s += u(i-1,j-1)*b[i-1];
	  s /= w[j-1];  // This is the divide by wj .
	}
      tmp[j-1]=s;
    }
  for (j=1;j<=n;j++) // Matrix multiply by V to get answer.
    {
      s=0.0;
      for (jj=1;jj<=n;jj++) s += v(j-1,jj-1)*tmp[jj-1];
      x[j-1]=s;
    }
}

//---------------------------------------------------------------------------

#define FSVD_EPSILON 0.01

bool FMatrix::solveSingularSystem(F::FVector& b, F::FVector& x,
				  std::vector<F::FVector>& nullspace,
				  std::vector<F::FVector>& range, double zeroTresholdEpsilon) const
{
  FMatrix v(x.getDimension(),x.getDimension()),u(*this);
  F::FVector w(x.getDimension());
  double wmax,wmin;

  u.svdcmp(w, v);  // a is replaced by u

  // ---- following sequence taken from NR example:

  wmax=0.0; // Will be the maximum singular value obtained.
  for(unsigned int j=0;j<w.getDimension();j++) if (fabs(w[j]) > wmax) wmax=w[j];

  // This is where we set the threshold for singular values allowed
  // to be nonzero. The constant is typical, but not universal.
  // You have to experiment with your own application.
  wmin=wmax*zeroTresholdEpsilon;
  for(unsigned int j=0;j<w.getDimension();j++) if (fabs(w[j]) < wmin) w[j]=0.0;

  // ----

  for(unsigned int l=0; l<w.getDimension(); l++)
    {
      if(w[l]==0.0) // this column of V belongs to nullspace
	{
	  unsigned int rows=v.getDimensionY();
	  F::FVector n(rows);
	  for(unsigned int c=0; c<rows; c++) n[c] = v(c,l);
	  nullspace.push_back(n);
	}
      else // this column of U belongs to range
	{
	  unsigned int rows=u.getDimensionY();
	  F::FVector n(rows);
	  for(unsigned int c=0; c<rows; c++) n[c] = u(c,l);
	  range.push_back(n);
	}
    }

  // Now check system's solvability

  unsigned int rank = range.size();

  F::FVector check(b.getDimension());

  for(unsigned int c=0; c<rank; c++)
    check += (range[c]*b)*range[c];

  if((check-b).normSquare()< FSVD_EPSILON) // there ain't any solution
    {
      return false;
    }

  FMatrix d(w.getDimension());
  for(unsigned int c=0; c<w.getDimension();c++)
    {
      if(w[c]==0.0) d(c,c) = 0.0;
      else d(c,c) = 1.0 / w[c];
    }

  // compute solution with smallest norm

  x = v*d*u.transpose()*b;

  return true;
}

//---------------------------------------------------------------------------

void FMatrix::getEigenValues( std::vector< std::complex<double> >& values ) const
{

#ifndef NODEBUG
  if (((dimx != 3) || (dimy != 3)) && ((dimx != 2) || (dimy != 2))){
    THROW_EXCEPTION( FInvalidDimensionException, "Matrix has to be 2x2 or 3x3");
  }
#endif

  if( dimx == 3 )
  {
  values.resize(3);
  //  complex<double> lambda[3];

//   double a = comp[0],
//     b = comp[1],
//     c = r(0,2),
//     d = r(1,0),
//     e = r(1,1),
//     f = r(1,2),
//     g = r(2,0),
//     h = r(2,1),
//     i = r(2,2);
  FMath::CubicEquation( -1.0,
                        comp[0]+comp[4]+comp[8],
                        comp[6]*comp[2]+
                        comp[7]*comp[5]+
                        comp[3]*comp[1]-
                        comp[0]*comp[4]-
                        comp[0]*comp[8]-
                        comp[4]*comp[8],
                        comp[0]*comp[4]*comp[8]+
                        comp[1]*comp[5]*comp[6]+
                        comp[2]*comp[3]*comp[7]-
                        comp[6]*comp[4]*comp[2]-
                        comp[0]*comp[5]*comp[7]-
                        comp[8]*comp[3]*comp[1],
                        &(values[0]) );
  }
  else
  { // assume dimx == 2
    values.resize(2);
    FMath::QuadraticEquation(1,
        -(comp[0]+comp[3]),
        comp[0]*comp[3]-comp[1]*comp[2],
        &(values[0]));
  }
}

//---------------------------------------------------------------------------

double FMatrix::discriminant(void) const
{
 if( dimx == 3 && dimy == 3 )
 {
  // see Roth (Page 143) !
  double result,P,Q,R,q,r;

  P = -(*this).trace();
  Q = (*this)(0,0)*(*this)(1,1)-(*this)(0,1)*(*this)(1,0) +
      (*this)(0,0)*(*this)(2,2)-(*this)(0,2)*(*this)(2,0) +
      (*this)(1,1)*(*this)(2,2)-(*this)(1,2)*(*this)(2,1) ;
  R = - (*this).detOf() ;

  q = Q/3 - (P*P)/9;
  r = P*Q/6 - R/2 - (P*P*P)/27;

  result = q*q*q + r*r;

  return result;
 }
 else if (dimx == 2 && dimy == 2 )
 {
   double b = (*this)(0,0)+(*this)(1,1);
   double c = (*this)(0,0)*(*this)(1,1)-(*this)(0,1)*(*this)(1,0);
   double result = b*b-4*1*c;
   return result;
 }
 else
   THROW_EXCEPTION( FInvalidDimensionException, "The Matrix must be 2x2 or 3x3" );
 return 0;
}

//---------------------------------------------------------------------------

double FMatrix::trace(void) const
{
#ifndef NODEBUG
  if ( dimx != dimy )
    THROW_EXCEPTION( FInvalidDimensionException, "Matrix is not square.");
#endif

  double result = 0;

  for( unsigned int i=0 ; i < dimx; i++ )
    result += (*this)(i,i);

  return result;
}


//------------------------------------------------------------------------------

double FMatrix::detOf() const
{
  double result=0;

  if((dimx == 3)&&(dimy == 3))
    {
      result = (*this)(0,0)*(*this)(1,1)*(*this)(2,2) -
	       (*this)(0,0)*(*this)(1,2)*(*this)(2,1) -
               (*this)(1,0)*(*this)(0,1)*(*this)(2,2) +
	       (*this)(1,0)*(*this)(0,2)*(*this)(2,1) +
               (*this)(2,0)*(*this)(0,1)*(*this)(1,2) -
               (*this)(2,0)*(*this)(0,2)*(*this)(1,1);
    }
  else if((dimx == 2)&&(dimx == 2))
    {
      result = (*this)(0,0)*(*this)(1,1)-(*this)(0,1)*(*this)(1,0);
    }
  else
    {
#ifndef NODEBUG
      THROW_EXCEPTION( FInvalidDimensionException, "ERROR: wrong dim(detOfMatrix)" );
#endif
    }
  return result;
}

//------------------------------------------------------------------------------

// Calculation taken from http://www.cprogramming.com/tutorial/3d/rotation.html
FMatrix FMatrix::rot3RH( const F::FVector& axis, double angle )
{
  FMatrix m(3,3);
  double c = cos(angle);
  double s = sin(angle);
  double t= 1.-c;

  double tXY=t*axis[0]*axis[1];
  double tXZ=t*axis[0]*axis[2];
  double tYZ=t*axis[1]*axis[2];
  double sX =s*axis[0];
  double sY =s*axis[1];
  double sZ =s*axis[2];
  m(0,0) = t*axis[0]*axis[0]+c;
  m(0,1) = tXY+sZ;
  m(0,2) = tXZ-sY;
  m(1,0) = tXY-sZ;
  m(1,1) = t*axis[1]*axis[1]+c;
  m(1,2) = tYZ+sX;
  m(2,0) = tXZ+sY; // typo in homepage
  m(2,1) = tYZ-sX;
  m(2,2) = t*axis[2]*axis[2]+c;
  return m;
}

FMatrix FMatrix::rot4RH( const F::FVector& axis, double angle )
{
  FMatrix m(4,4);
  double c = cos(angle);
  double s = sin(angle);
  double t= 1.-c;

  double tXY=t*axis[0]*axis[1];
  double tXZ=t*axis[0]*axis[2];
  double tYZ=t*axis[1]*axis[2];
  double sX =s*axis[0];
  double sY =s*axis[1];
  double sZ =s*axis[2];
  m(0,0) = t*axis[0]*axis[0]+c;
  m(0,1) = tXY+sZ;
  m(0,2) = tXZ-sY;
  m(1,0) = tXY-sZ;
  m(1,1) = t*axis[1]*axis[1]+c;
  m(1,2) = tYZ+sX;
  m(2,0) = tXZ+sY; // typo in homepage
  m(2,1) = tYZ-sX;
  m(2,2) = t*axis[2]*axis[2]+c;
  m(3,3) = 1.0;
  return m;
}

FMatrix FMatrix::rot3LH( const F::FVector& axis, double angle )
{
  FMatrix m(3,3);
  double c = cos(angle);
  double s = sin(angle);
  double t= 1.-c;

  double tXY=t*axis[0]*axis[1];
  double tXZ=t*axis[0]*axis[2];
  double tYZ=t*axis[1]*axis[2];
  double sX =s*axis[0];
  double sY =s*axis[1];
  double sZ =s*axis[2];
  m(0,0) = t*axis[0]*axis[0]+c;
  m(0,1) = tXY-sZ;
  m(0,2) = tXZ+sY;
  m(1,0) = tXY+sZ;
  m(1,1) = t*axis[1]*axis[1]+c;
  m(1,2) = tYZ-sX;
  m(2,0) = tXZ-sY;
  m(2,1) = tYZ+sX;
  m(2,2) = t*axis[2]*axis[2]+c;
  return m;
}

FMatrix FMatrix::rot4LH( const F::FVector& axis, double angle )
{
  FMatrix m(4,4);
  double c = cos(angle);
  double s = sin(angle);
  double t= 1.-c;

  double tXY=t*axis[0]*axis[1];
  double tXZ=t*axis[0]*axis[2];
  double tYZ=t*axis[1]*axis[2];
  double sX =s*axis[0];
  double sY =s*axis[1];
  double sZ =s*axis[2];
  m(0,0) = t*axis[0]*axis[0]+c;
  m(0,1) = tXY-sZ;
  m(0,2) = tXZ+sY;
  m(1,0) = tXY+sZ;
  m(1,1) = t*axis[1]*axis[1]+c;
  m(1,2) = tYZ-sX;
  m(2,0) = tXZ-sY;
  m(2,1) = tYZ+sX;
  m(2,2) = t*axis[2]*axis[2]+c;
  m(3,3) = 1.0;
  return m;
}

//===========================================================================

FMatrix& FMatrix::pow(int n)
{
#ifndef NODEBUG
  if ( (dimx != dimy))
    THROW_EXCEPTION( FInvalidDimensionException, "The Matrix is not square !!");
  if ( n < 0 )
    THROW_EXCEPTION( FException, "pow(n), n must not be less than zero" );
#endif

  FMatrix result;
  result.makeDiagonal(1.0, dimx );
  while(n>0)
  {
    if(n%2) result *= *this;
    n/=2;
    *this *= *this;
  }
  *this = result;
  return *this;
}




//===========================================================================

FMatrix& FMatrix::makeSymmetric()
{
#ifndef NODEBUG
  if ( (dimx != dimy))
    THROW_EXCEPTION( FInvalidDimensionException, "The Matrix is not square !!");
#endif

  for(unsigned int i=0; i<dimx; ++i)
  {
    for(unsigned int j=i; j< dimx; ++j)
    {
      double val = 0.5*((*this)(i,j)+(*this)(j,i));
      (*this)(i,j) = (*this)(j,i) = val;
    }
  }
  return *this;
}

//===========================================================================

FMatrix& FMatrix::makeAntisymmetric()
{
#ifndef NODEBUG
  if ( (dimx != dimy))
    THROW_EXCEPTION( FInvalidDimensionException, "The Matrix is not square !!");
#endif

  for(unsigned int i=0; i<dimx; ++i)
  {
    for(unsigned int j=i; j< dimx; ++j)
    {
      double val = 0.5*((*this)(i,j)-(*this)(j,i));
      (*this)(i,j) = (*this)(j,i) = val;
    }
  }
  return *this;
}

//===========================================================================

#ifdef OUTLINE
#include "FMatrix.icc"
#endif

//===========================================================================
