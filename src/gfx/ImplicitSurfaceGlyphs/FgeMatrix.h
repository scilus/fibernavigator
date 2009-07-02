///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2009-03-21 00:20:03 +0100 (Sat, 21 Mar 2009) $
         //             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 9785 $

#ifndef FgeMatrix_hh
#define FgeMatrix_hh
#include <cassert>
#include <iostream>
#include <string.h>     // needed for memcpy

#include "FgeGL.h"
#include <math.h>
#include "FgeVector.h"

#include "FFixArray.hh"

template<typename T, int dim>
class FgeMatrix
{
  friend class FgeTransRot;
  public:
  enum { Dimension = dim };


typedef FFixArray<T, dim> Vector;

  FgeMatrix()
  {}

  FgeMatrix( const T* val )
  {
    memcpy( data, val, sizeof(T)*Dimension*Dimension);
  }

  void operator=( const T* val )
  {
    memcpy( data, val, sizeof(T)*Dimension*Dimension);
  }

  T subDeterminant(int i, int j) const
  {
    assert( dim == 4 );
    int i1=i>0?0:1;
    int i2=i>1?1:2;
    int i3=i>2?2:3;
    int j1=j>0?0:1;
    int j2=j>1?1:2;
    int j3=j>2?2:3;
    double d1=data[i1][j1]*(data[i2][j2]*data[i3][j3]-data[i2][j3]*data[i3][j2]);
    double d2=data[i2][j1]*(data[i1][j2]*data[i3][j3]-data[i1][j3]*data[i3][j2]);
    double d3=data[i3][j1]*(data[i1][j2]*data[i2][j3]-data[i1][j3]*data[i2][j2]);
    return d1-d2+d3;
  }

  T determinant() const
  {
    assert( dim == 4 );
    int j;
    T determinant=0.0;
    for(j=0;j<4;++j)
        if(data[3][j]!= 0.0)
        {
            if(j&1)
                determinant += data[3][j]*subDeterminant(3,j);
            else
                determinant += data[3][j]*subDeterminant(3,j);
        }
    return determinant;
  }

  FgeMatrix& invert()
  {
    assert( dim == 4 );
    T newA[Dimension][Dimension];
    int i,j;
    double determinant = 0.0;
    for(j=0;j<4;++j)
      if(data[3][j] != 0.0)
      {
        if(j&1)
          determinant += data[3][j]*subDeterminant(3,j);
        else
          determinant -= data[3][j]*subDeterminant(3,j);
      }
    for(i=0;i<4;++i)
      for(j=0;j<4;++j)
        if((i+j)&1)
          newA[i][j]=-subDeterminant(j,i)/determinant;
        else
          newA[i][j]= subDeterminant(j,i)/determinant;
    for(i=0;i<4;++i)
      for(j=0;j<4;++j)
        data[i][j]=newA[i][j];
    return *this;
  }

  void transform(T *result, const T* h) const
  {
    for(int i=0; i<4; ++i)
    {
      result[i]=0.0;
      for(int j=0; j<4; ++j)
      {
        result[i]+=data[i][j]*h[j];
      }
    }
  }

template<typename T2>
  FFixArray<T2,dim> transform( const FFixArray<T2,dim>&v ) const
  {
    FFixArray<T2,dim> result;
    for(int i=0; i<dim; ++i)
    {
      result[i]=0.0;
      for(int j=0; j<dim; ++j)
      {
        result[i]+=data[i][j]*v[j];
      }
    }
	return result;
  }

// transform a homogeneous vector that has one dimension less than the matrix
template<typename T2>
  FFixArray<T2, dim-1> transformH( const FFixArray<T2, dim-1> &v) const
  {
	FFixArray<T2, dim> tmp;
	for(int i=0;i<dim-1; ++i)
		tmp[i] = v[i];
	tmp[dim-1] = 1.;

	tmp = transform(tmp);
	FFixArray<T2, dim-1> res;
	for(int i=0;i<dim-1; ++i)
		res[i] = tmp[i]/tmp[dim-1];

	return res;
  }

  void inverseTransform(double *result, const double *h) const
  {
    /* Prepare 4-by-5 matrix for Gaussian elimination: */
    T m[4][5];
    int i,j,k;
    for(i=0;i<4;++i)
    {
      for(j=0;j<4;++j)
        m[i][j]=data[i][j];
      m[i][4]=h[i];
    }
    /* Do a Gaussian elimination with column pivot search: */
    for(j=0;j<3;++j)
    {
#if 1
      /* Find pivot element in column j: */
      int pivot=j;
      T max=fabs(m[pivot][j]);
      for(i=j+1;i<4;++i)
      {
        T fm=fabs(m[i][j]);
        if(fm>max)
        {
          max=fm;
          pivot=i;
        }
      }
      /* Exchange rows j and pivot: */
      for(k=0;k<5;++k)
      {
        T t=m[j][k];
        m[j][k]=m[pivot][k];
        m[pivot][k]=t;
      }
#endif
      /* Do Gaussian elimination: */
      T source=m[j][j];
      for(i=j+1;i<4;++i)
      {
        T factor=m[i][j]/source;
        m[i][j]=0.0;
        for(k=j+1;k<5;++k)
          m[i][k]-=factor*m[j][k];
      }
    }
    result[3]=m[3][4]/m[3][3];
    result[2]=(m[2][4]-m[2][3]*result[3])/m[2][2];
    result[1]=(m[1][4]-m[1][3]*result[3]-m[1][2]*result[2])/m[1][1];
    result[0]=(m[0][4]-m[0][3]*result[3]-m[0][2]*result[2]-m[0][1]*result[1])/m[0][0];
  }


  void makeIdentity()
  {
    for(int i=0; i<dim; ++i)
    {
      for(int j=0; j<dim; ++j)
      {
        if( i==j ) data[i][j] = T(1);
        else data[i][j] = T(0);
      }
    }
  }

  FgeVector operator*( const FgeVector& v )
  {
    // FIXME: do normalization, too?
    // multiplication only for a 3D Vector taking 3D Matrix at the moment
    FgeVector res;
    for ( int i=0; i<3; ++i )
      res[ i ] = v[ 0 ] * data[ i ][ 0 ] + v[ 1 ] * data[ i ][ 1 ] + v[ 2 ]*data[ i ][ 2 ];
    return res;
  }


  FgeMatrix& operator*=( const T* d )
  {
    T t[Dimension][Dimension];
    memcpy(t, data, sizeof(T)*Dimension*Dimension);
    for(int i=0; i<Dimension; ++i)
    {
      for(int j=0; j<Dimension; ++j)
      {
        data[i][j]= T(0);
        for(int k=0; k<4;++k)
        {
          data[i][j] += t[i][k]*d[k*Dimension+j];
        }
      }
    }
    return *this;
  }

  FgeMatrix& operator*=( const FgeMatrix& m2 )
  {
    T t[Dimension][Dimension];
    memcpy(t, data, sizeof(T)*Dimension*Dimension);
    for(int i=0; i<Dimension; ++i)
    {
      for(int j=0; j<Dimension; ++j)
      {
        data[i][j]= T(0);
        for(int k=0; k<4;++k)
        {
          data[i][j] += t[i][k]*m2.data[k][j];
        }
      }
    }
    return *this;
  }

  FgeMatrix& transpose()
  {
    for(unsigned int i=0; i< Dimension-1; ++i)
      for(unsigned int j=i+1; j<Dimension; ++j)
        std::swap( data[i][j], data[j][i] );
    return *this;
  }

  FgeMatrix& scale( const T& scale )
  {
    for(int i=0; i<Dimension; ++i)
      for(int j=0; j<Dimension; ++j)
      data[i][j] *= scale;
    return *this;
  }

  T& operator()(int i, int j) { return data[i][j]; }
  const T& operator()(int i, int j) const { return data[i][j]; }

  T data[Dimension][Dimension];
};

  template<typename T, int d>
std::ostream& operator<<( std::ostream&o, const FgeMatrix<T,d>& m)
{
  for(int i=0; i<d; ++i)
  {
    for(int j=0; j<d; ++j)
      o << m.data[i][j] << " ";
    o << '\n';
  }
  return o;
}

template< class T >
class FgeMatrix4 : public FgeMatrix< T, 4 >
{
  public:
    FgeMatrix4(){}

    FgeMatrix4( const FgeMatrix< T, 4>& m )
    { memcpy( this->data, m.data, 16*sizeof(T)); }

    FgeMatrix4& operator=( const FgeMatrix< T, 4> &m)
    { memcpy( this->data, m.data, 16*sizeof(T)); return *this;}


  // only available for 4x4 matrices
  FgeMatrix4& setFrustum( T left, T right, T bottom, T top, T nearp, T farp)
  {
    for(int i=0; i<4; ++i)
      for(int j=0; j<4; ++j)
        this->data[i][j] = T(0);
    const T A = (right + left) / ( right - left );
    const T B = (top  + bottom ) / ( top - bottom );
    const T C = (farp + nearp )  / ( farp - nearp );
    const T D = (T(2)*farp*nearp)/ (farp-nearp);
    this->data[0][0] = T(2)*nearp/(right-left);
    this->data[2][0] = A;
    this->data[1][1] = T(2)*nearp/(top-bottom);
    this->data[1][2] = B;
    this->data[2][2] = C;
    this->data[3][2] = D;
    this->data[2][3] = T(-1);
    return *this;
  }

  FgeMatrix4& setOrtho( T left, T right, T bottom, T top, T nearp, T farp)
  {
    for(int i=0; i<4; ++i)
      for(int j=0; j<4; ++j)
        this->data[i][j] = T(0);
    const T tx = (right+left)/(right-left);
    const T ty = (top + bottom)/(top-bottom );
    const T tz = (farp+nearp)/(farp-nearp);
    this->data[0][0] = T(2)/(right-left);
    this->data[1][1] = T(2)/(top-bottom);
    this->data[2][2] = T(-2)/(farp-nearp);
    this->data[3][3] = T(1);
    this->data[3][0] = tx;
    this->data[3][1] = ty;
    this->data[3][2] = tz;
    return *this;
  }

#define SET_ROW( row, a,b,c,d )\
  this->data[row][0] = a; \
  this->data[row][1] = b; \
  this->data[row][2] = c; \
  this->data[row][3] = d; \


  FgeMatrix4& makeLookAt( const FgeVector &eye, const FgeVector& center, FgeVector& up )
  {
    FgeVector F = center - eye;
    FgeVector Up= up;
    F.normalize();
    Up.normalize();
    FgeVector s = F ^ Up;
    FgeVector u = s ^ F;
    SET_ROW( 0,   s[0], u[0], -F[0], 0 );
    SET_ROW( 1,   s[1], u[1], -F[1], 0 );
    SET_ROW( 2,   s[2], u[2], -F[2], 0 );
    SET_ROW( 3,      0,    0,     0, 1 );
    return *this;
  }

  FgeMatrix4& makeScale(const T& x, const T&y, const T& z)
  {
    SET_ROW( 0,     x, 0, 0, 0 );
    SET_ROW( 1,     0, y, 0, 0 );
    SET_ROW( 2,     0, 0, z, 0 );
    SET_ROW( 3,     0, 0, 0, 1 );
    return *this;
  }

  static inline FgeMatrix4 scale(const T& x, const T&y, const T& z)
  {
    FgeMatrix4 m;
    return m.makeScale(x,y,z);
  }

  FgeMatrix4& makeTranslation( const T& x, const T&y, const T&z )
  {
    SET_ROW(0,      1, 0, 0, 0 );
    SET_ROW(1,      0, 1, 0, 0 );
    SET_ROW(2,      0, 0, 1, 0 );
    SET_ROW(3,      x, y, z, 1 );
    return *this;
  }

  static inline FgeMatrix4 translation(const T& x, const T&y, const T& z)
  {
    FgeMatrix4 m;
    return m.makeTranslation( x, y, z);
  }


	// OpenGL getter functions
	void getProjectionMatrix();

	void getModelViewMatrix();

	void getModelViewProjectionMatrix();
};

template<>
inline void FgeMatrix4<GLfloat>::getModelViewMatrix()
{
	  glGetFloatv( GL_MODELVIEW_MATRIX, &(this->data[0][0]) );
}

template<>
inline void FgeMatrix4<GLfloat>::getProjectionMatrix()
{
	  glGetFloatv( GL_PROJECTION_MATRIX, &this->data[0][0] );
}

template<>
inline void FgeMatrix4<GLfloat>::getModelViewProjectionMatrix()
{
	  getProjectionMatrix();
	transpose();
	  FgeMatrix4 mv; mv.getModelViewMatrix();
	mv.transpose();
	  (*this) *= mv;
}


template<>
inline void FgeMatrix4<GLdouble>::getModelViewMatrix()
{
	  glGetDoublev( GL_MODELVIEW_MATRIX, &this->data[0][0] );
}
template<>
inline void FgeMatrix4<GLdouble>::getProjectionMatrix()
{
	  glGetDoublev( GL_PROJECTION_MATRIX, &this->data[0][0] );
}

template<>
inline void FgeMatrix4<GLdouble>::getModelViewProjectionMatrix()
{
	  getProjectionMatrix();
	transpose();
	  FgeMatrix4 mv; mv.getModelViewMatrix();
	mv.transpose();
	  (*this) *= mv;
}


typedef FgeMatrix4<GLdouble  > FgeMatrix4d;
typedef FgeMatrix4<GLfloat   > FgeMatrix4f;
typedef FgeMatrix<GLdouble, 3> FgeMatrix3d;
typedef FgeMatrix<GLfloat,  3> FgeMatrix3f;

inline void applyToGl( const FgeMatrix4f &m)
{
  glLoadMatrixf( &m.data[0][0] );
}


#endif

