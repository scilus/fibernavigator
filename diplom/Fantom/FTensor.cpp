//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FTensor.cc,v $
// Language:  C++
// Date:      $Date: 2004/07/14 06:38:16 $
// Author:    $Author: garth $
// Version:   $Revision: 1.14 $
//
//---------------------------------------------------------------------------

#include "FTensor.h"
#include <iostream>
#include "binio.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//---------------------------------------------------------------------------

FTensor::~FTensor()
{
}

//---------------------------------------------------------------------------

std::ostream& operator<< (std::ostream& os, const FTensor& t)
{
  os <<"[ ";

  if( t.comp == 0 )
      os << " empty ";
  else
  for (unsigned char i=0; i<(unsigned char)FTensor::pow (t.dimension,
							 t.order); i++)
    os << t.comp[i] << " ";

  os <<" ]";
  return os;
}

//---------------------------------------------------------------------------

std::istream& binread( std::istream& in, FTensor &t )
{
  unsigned int dimension;
  binread_raw( in, &dimension );

  unsigned int order;
  binread_raw( in, &order );

  unsigned int size;
  binread_raw( in, &size );

  std::vector<double> values;
  values.resize( size );

  for( unsigned int i = 0; i < size; ++i )
    {
      binread_raw( in, &values[i] );
    }

  FTensor tmp( dimension, order, values );
  t = tmp;

  return in;
}

std::ostream& binwrite( std::ostream& out, const FTensor& t )
{
  unsigned int dimension = t.getDimension();
  unsigned int order = t.getOrder();
  std::vector<double> values;
  t.getValues( values );
  unsigned int size = values.size();

  binwrite_raw( out, &dimension );
  binwrite_raw( out, &order );
  binwrite_raw( out, &size );

  for( unsigned int i = 0; i < size; ++i )
  {
      binwrite_raw( out, &values[i] );
  }

  return out;

}

//---------------------------------------------------------------------------

void FTensor::getEigenvalues3DS(FVector& vals) const
{
#ifndef NODEBUG
  if (order != 2)
    THROW_EXCEPTION( FInvalidDimensionException, "ERROR: invalid order (has to be 2)!");

  if (dimension != 3)
    THROW_EXCEPTION( FInvalidDimensionException, "ERROR: invalid dimension (has to be 3)!");
#endif
#define T ( *this )

  const double I1 = T( 0, 0 ) + T( 1, 1 ) + T( 2, 2 );
  const double I2 = T( 0, 0 ) * T( 1, 1 )
            + T( 1, 1 ) * T( 2, 2 )
            + T( 2, 2 ) * T( 0, 0 )
            - ( T( 0,1 )*T( 0,1 )+T( 0,2 )*T( 0,2 )+T( 1,2 )*T( 1,2 ) );
  const double I3 = T( 0,0 )*T( 1,1 )*T( 2,2 )
            + 2.*T( 0,1 ) *T( 1,2 )*T( 2,0 )
            -( T( 0,0 )*T( 1,2 )*T( 1,2 ) + T( 1,1 )*T( 2,0 )*T( 2,0 )+T( 2,2 )*T( 0,1 )*T( 0,1 ) );

  const double V = ( I1/3. )*( I1/3. ) - I2/3.;
  const double S = ( I1/3. )*( I1/3. )*( I1/3. )-I1*I2/6.+I3*0.5;
  const double sqrtV = sqrt( V );
  const double phi = acos( S/( V*sqrtV ) )/3.;

  //vals.resize( 3, false );

  const double cosphi = cos( phi );
  const double sinphi = sin( phi ); // simplify this to sqrt?
  static const double cosM_PI_3 = cos( M_PI/3. );
  static const double sinM_PI_3 = sin( M_PI/3. );

  // cphi = cos(phi)
  // cos(M_PI/3 + phi) = cos(M_PI/3)cos(phi) - sin(M_PI/3)sin(phi)
  // cos(M_PI/3 - phi) = cos(M_PI/3)cos(phi) + sin(M_PI/3)sin(phi)
  const double ls = cosM_PI_3*cosphi;
  const double rs = sinM_PI_3*sinphi;
  const double I1_3 = I1/3.;

  const double sqrtV2 = sqrtV*2.;
  //vals[ 0 ] = I1/3.+2.*sqrtV*cos( phi );
  //vals[ 1 ] = I1/3.-2.*sqrtV*cos( M_PI/3.+phi );
  //vals[ 2 ] = I1/3.-2.*sqrtV*cos( M_PI/3.-phi );
  vals[ 0 ] = I1_3 + sqrtV2*cosphi;
  vals[ 1 ] = I1_3 - sqrtV2*( ls - rs );
  vals[ 2 ] = I1_3 - sqrtV2*( ls + rs );
}

void FTensor::getEigenvector3DS(FVector &ev, const double lambda ) const
{
  const double zx = T( 0,1 )*T( 1,2 );
  const double yz = T( 0,2 )*T( 0,1 );
  const double yx = T( 0,2 )*T( 1,2 );

  for ( int i=0; i<2; ++i )
  {
    const double A = T( 0,0 ) - lambda;
    const double B = T( 1,1 ) - lambda;
    const double C = T( 2,2 ) - lambda;

    const double By = B * T( 0,2 );
    const double zxmBy = zx - By;
    const double Ax = A * T( 1,2 );
    const double yzmAx = yz - Ax;
    const double Cz = C * T( 0,1 );
    const double yxmCz = yx - Cz;

    ev[ 0 ] = (               zxmBy          ) * (                 yxmCz        ); // FIXME!!!
    ev[ 1 ] = (               yxmCz          ) * (                 yzmAx        );
    ev[ 2 ] = (               zxmBy          ) * (                 yzmAx        );
  }
  // ev is not normalized!
}

void FTensor::getEigenSystem3DS(FVector& vals, FVector v[3]) const
{
  getEigenvalues3DS( vals );

  const double zx = T( 0,1 )*T( 1,2 );
  const double yz = T( 0,2 )*T( 0,1 );
  const double yx = T( 0,2 )*T( 1,2 );

  for ( int i=0; i<2; ++i )
  {
    const double A = T( 0,0 ) - vals[ i ];
    const double B = T( 1,1 ) - vals[ i ];
    const double C = T( 2,2 ) - vals[ i ];

    const double By = B * T( 0,2 );
    const double zxmBy = zx - By;
    const double Ax = A * T( 1,2 );
    const double yzmAx = yz - Ax;
    const double Cz = C * T( 0,1 );
    const double yxmCz = yx - Cz;

    //v[ i ].resize( 3,false );
//  v[ i ][ 0 ] = ( T( 0,1 )*T( 1,2 )-B*T( 0,2 ) ) * ( T( 0,2 )*T( 1,2 )-C*T( 0,1 ) ); // FIXME!!!
//  v[ i ][ 0 ] = (         zx       -  By       ) * ( T( 0,2 )*T( 1,2 )-C*T( 0,1 ) ); // FIXME!!!
    v[ i ][ 0 ] = (               zxmBy          ) * (                 yxmCz        ); // FIXME!!!
//  v[ i ][ 1 ] = ( T( 0,2 )*T( 1,2 )-C*T( 0,1 ) ) * ( T( 0,2 )*T( 0,1 )-A*T( 1,2 ) );
//  v[ i ][ 1 ] = ( T( 0,2 )*T( 1,2 )-C*T( 0,1 ) ) * (         yz       -    Ax     );
    v[ i ][ 1 ] = (               yxmCz          ) * (                 yzmAx        );
//  v[ i ][ 2 ] = ( T( 0,1 )*T( 1,2 )-B*T( 0,2 ) ) * ( T( 0,2 )*T( 0,1 )-A*T( 1,2 ) );
//  v[ i ][ 2 ] = (         zx       -  By       ) * (         yz       -    Ax     );
    v[ i ][ 2 ] = (               zxmBy          ) * (                 yzmAx        );

    // same as in utils/GLSLShaderCode/TensorTools.glsl
//      vec.x = ( offdiag.z*offdiag.x-ABC.y*offdiag.y )*( offdiag.y*offdiag.x-ABC.z*offdiag.z ); // FIXME
              //< last component is missing in the paper! there is only a Dx?
//      vec.y = (offdiag.y*offdiag.x-ABC.z*offdiag.z)*(offdiag.y*offdiag.z-ABC.x*offdiag.x);
//      vec.z = (offdiag.z*offdiag.x-ABC.y*offdiag.y)*(offdiag.y*offdiag.z-ABC.x*offdiag.x);

    v[ i ].normalize();
  }

  //v[ 2 ] = v[ 0 ].crossProductConst( v[ 1 ] );
  v[ 0 ].crossProduct( v[ 1 ], v[ 2 ] ); //< this has an ugly interface, but is faster than above version
#undef T
}

//===========================================================================

#ifdef OUTLINE
#include "FTensor.icc"
#endif

//---------------------------------------------------------------------------
