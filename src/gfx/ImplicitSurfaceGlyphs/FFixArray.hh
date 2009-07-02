//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FFixArray.hh,v $
// Language:  C++
// Date:      $Date: 2003/06/25 07:54:46 $
// Author:    $Author: garth $
// Version:   $Revision: 1.4 $
//
//--------------------------------------------------------------------------- 
#ifndef __FFixArray_hh
#define __FFixArray_hh

#include <vector>
#include <iosfwd>
#include <string>
#include <sstream>
#include <cmath>
#include <cassert>

/**
 * Class implementing an array of doubles.
 * Please note: FObject is not a base class of FArray to reduce
 * memory consumption.
 */
template< typename T, int I >
struct FFixArray
{
public:
  
  // ============= constructors ======================
  /** constructs empty array.
   */  
  FFixArray();
  
  /** 
   * Copy constructor.
   * \param a
   * Array to copy.
   */
  FFixArray(const FFixArray< T, I >& a);

  /** 
   * Copy constructor.
   * \param a
   * Vector to copy into this array.
   */
  FFixArray(const std::vector< T >& a);

  /** 
   * Constructor, construct array from double array.
   * \param s
   * Size of double array.
   * \param ref
   * Standard array of doubles.
   */
  explicit FFixArray( const T ref[] );

  /**
   * Constructors with a given amount of values
   */ 
  explicit FFixArray( const T &ref );
  explicit FFixArray( const T &ref0, const T &ref1 );
  explicit FFixArray( const T &ref0, const T &ref1, const T &ref2 );
  explicit FFixArray( const T &ref0, const T &ref1, const T &ref2, const T &ref3 );
  explicit FFixArray( const T &ref0, const T &ref1, const T &ref2, const T &ref3, const T &ref4 );
 
  /**
   * clear containts of FVector
   */
  void reset( const T &def );
  
  // ============= modifiers ======================
  
  /** Size reporter.
   * \return number of array-entries.
   */
  static unsigned int size(void);

  // ============= operators ======================
  
  /** Access operator ()
   * \param index
   * Index of element to access.
   * \return 
   * Reference to entry.
   * \exception FIndexOutOfBoundsException 
   * Thrown if NODEBUG is not defined and an invalid element is to be accessed.
   */
  T& operator() (unsigned int i);
  
  /** Access operator () const.
   * \param index
   * Index of element to access.
   * \return 
   * Const reference to entry.
   * \exception FIndexOutOfBoundsException 
   * Thrown if NODEBUG is not defined and an invalid element is to be accessed.
   */
  const T& operator() (unsigned int i) const;

  /** Access operator []
   * \param index
   * Index of element to access.
   * \return 
   * Reference to entry.
   * \exception FIndexOutOfBoundsException 
   * Thrown if NODEBUG is not defined and an invalid element is to be accessed.
   */
  T& operator[] (unsigned int i);  

  /** Access operator [] const.
   * \param index
   * Index of element to access.
   * \return 
   * Const reference to entry.
   * \exception FIndexOutOfBoundsException 
   * Thrown if NODEBUG is not defined and an invalid element is to be accessed.
   */
  const T& operator[] (unsigned int i) const;
  
  /** assignment operator
   * \param a
   * Rightside
   * \return 
   * Reference to self
   */
  FFixArray< T, I > &operator=( const std::vector< T >& a ); 

  /** assignment operator
   * \param a
   * Rightside
   * \return 
   * Reference to self
   */
  FFixArray< T, I >& operator=(const FFixArray< T, I >& a);
  
  /** comparison operator
   * \param a
   * Rightside
   * \return 
   * True if condition matches, false if not.
   */
  bool operator==(const FFixArray< T, I >& a) const;

  /** comparison operator
   * \param a
   * Rightside
   * \return 
   * true if this is smaller than a (the priority of the components
   * is equivalent to their index, index 0 has highest priority)
   */
  bool operator<(const FFixArray< T, I >& a) const;

  /** comparison operator
   * \param a
   * Rightside
   * \return 
   * true if this is greater than a (the priority of the components
   * is equivalent to their index, index 0 has highest priority)
   */
  bool operator>(const FFixArray< T, I >& a) const;

  /** comparison operator
   * \param a
   * Rightside
   * \return true if every component of *this is greater than the
   * corresponding component in a
   */
  bool isGreaterThan(const FFixArray< T, I >& a) const;
  bool isGreaterEqThan(const FFixArray< T, I >& a) const;
  bool isLessThan(const FFixArray< T, I >& a) const;
  bool isLessEqThan(const FFixArray< T, I >& a) const;
  
  
  /** comparison operator
   * \param a
   * Rightside
   * \return 
   * True if condition matches, false if not.
   */
  bool operator!=(const FFixArray< T, I >& a) const;

  /** simple lowlevel (hopefully fast) array addition operator
   * \param a
   * Rightside
   * \exception FInvalidDimensionException 
   * If NODEBUG is not defined and arguments have nonmatching dimensions.
   */
  FFixArray< T, I > &operator+=(const FFixArray< T, I > &a); 
  
  /** simple lowlevel (hopefully fast) array subtraction operator
   * \param a
   * Rightside
   * \exception FInvalidDimensionException 
   * If NODEBUG is not defined and arguments have nonmatching dimensions.
   */
  FFixArray< T, I > &operator-=(const FFixArray< T, I > &a); 
  
  /** simple lowlevel (hopefully fast) array multiplication operator
   * \param a
   * Rightside
   * \exception FInvalidDimensionException 
   * If NODEBUG is not defined and arguments have nonmatching dimensions.
   */
  FFixArray< T, I > &operator*=( const T &d );

  /** simple lowlevel (hopefully fast) array division operator
   * \param a
   * Rightside
   * \exception FInvalidDimensionException 
   * If NODEBUG is not defined and arguments have nonmatching dimensions.
   */
  FFixArray< T, I > &operator/=( const T &d );

  /** Array addition operator.
   * Should be used carefully since this operator implies
   * one complete copy operation. A more flexible operator is plus.
   * \param a
   * Rightside
   * \return 
   * Reference to self / copy of result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  FFixArray< T, I > operator+(const FFixArray< T, I > &a) const;

  /** Array subtraction operator.
   * Should be used carefully since this operator implies
   * one complete copy operation. A more flexible operator is minus.
   * \param a
   * Rightside
   * \return 
   * Reference to self / copy of result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  FFixArray< T, I > operator-(const FFixArray< T, I > &a) const;

  /**
   * unariy minus operator
   * \return
   * Negated vector (see negate)
   */
  FFixArray< T, I > operator-(void) const;

  /**
   * negates every component
   */
  FFixArray< T, I >& negate();

  /**
   * component wise comp[i] = fabs(comp[i])
   */
  FFixArray< T, I >& positivate();
  
  /**
   * clips every component to range [min, max]
   */
  FFixArray< T, I >& clipValues(double min, double max);

  /** Array multiplication operator.
   * Should be used carefully since this operator implies
   * one complete copy operation. A more flexible operator is multiply.
   * \param a
   * Rightside
   * \return 
   * Reference to self / copy of result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  FFixArray< T, I > operator*(const T &d) const;

  /**
   * division operator
   */
  FFixArray< T, I > operator/(const T &d) const;
  
  /**
   * Store array values in a vector.
   * \param coords
   * Destination vector.
   */
  void getCoordinates( std::vector< T >& coords ) const;

  /**
   * Calculate cross product of this vector and another one.
   * \param b
   * Rightside argument.
   * \param dest
   * Destination array.
   * \exception FInvalidDimensionException
   * Thrown, if any of the arrays has dimension other than 3.
   */
  void crossProduct(const FFixArray< T, 3 > &b,
		    FFixArray< T, 3 > &dest) const;

  /**
   * Calculate cross product of this vector and another one.
   * \param b
   * Rightside argument.
   * \return
   * Destination array.
   * \post
   * object has been modified: Its new value is the cross product 
   * \exception FInvalidDimensionException
   * Thrown, if any of the arrays has dimension other than 3.
   */
  FFixArray< T, 3 >& crossProduct( const FFixArray< T, 3 > &b );

  /**
   * Normalize this array. (Make it a unit vector.)
   */
  FFixArray< T, I>&  normalize();

  /**
   * return normalized array. (Make it a unit vector.)
   * Keep current vector untouched
   */ 
  FFixArray< T, I>  normalized() const;

  /** Euclidian scalar product.
   * This operator computes the Euclidian scalar product:
   *   \[ \langle \vec{a}, \vec{b} \rangle = \sum_{i=0}^{dim-1} a_i b_i \]
   * \pre
   * Operands must have the same dimension.
   * \exception
   * FIllegalDimensionException if NODEBUG isn't defined
   * \param vector
   * Rightside.
   * \return 
   * Euclidian scalar product.
   */
  T operator*(const FFixArray< T, I > &vector) const;

  /** Euclidian vector norm.
   * \return 
   * Euclidian norm: $ \| \langle \vec{a}, \vec{a} \rangle \| $.
   */
  T norm() const;

  /** Euclidian vector norm.
   * \return 
   * Square of Euclidian norm: $ \| \langle \vec{a}, \vec{a} \rangle \| $.
   */
  T normSquare() const;
  
  /** Only reasonable, if you interpret this array as a vector.
   * Calculate distance between this vector and another one.
   * \param pos2
   * The other vector.
   * \return
   * Distance.
   */
  T distance(const FFixArray< T, I > &pos2) const;

  /** Only reasonable, if you interpret this array as a vector.
   * Calculate square distance between this vector and another one.
   * \param pos2
   * The other vector.
   * \return
   * Square distance.
   */
  T distanceSquare(const FFixArray< T, I >& pos2) const;

  T dot( const FFixArray<T,I>&rhs) const
  { return *this * rhs; }

  /**
   * \return
   * a const-pointer to the data
   */
  const T* data() const
  {
	  return &comp[0];
  }
  
  /*
   * \return 
   * a pointer to the data
   */
  T* data()
  {
	  return &comp[0];
  }
  
  /**
   * Store array values in a vector.
   * \param coords pre allocated target data structure
   * Destination vector.
   */
  template<class RHS>
  void getCoordinates(RHS* coords) const
  {
    for ( int i=0; i< I; ++i )
      coords[ i ] = comp[ i ];
  }


protected:
  
  /// pointer to memory holding the entries
  T comp[ I ];

};


template< typename T, int I >
inline FFixArray< T, I >::FFixArray()
{
}

template< typename T, int I >
inline FFixArray< T, I >::FFixArray(const FFixArray< T, I >& a)
{
  memcpy( comp, a.comp, I * sizeof ( T ) );
}

template< typename T, int I >
inline FFixArray< T, I >::FFixArray(const std::vector< T >& a)
{
  assert( a.size() <= I );
  memcpy( comp, &(a[0]), I * sizeof ( T ) );
}

template< typename T, int I >
inline FFixArray< T, I >::FFixArray( const T ref[] )
{
  memcpy( comp, ref, I * sizeof ( T ) );
}

template< typename T, int I >
inline FFixArray< T, I >::FFixArray( const T &ref )
{
  for ( int i=0; i<I; i++ )
    comp[ i ] = ref;
}

template< typename T, int I >
inline void FFixArray< T, I >::reset( const T &def )
{
  for ( int i=0; i<I; i++ )
    comp[ i ] = def;
}

template< typename T, int I >
inline unsigned int FFixArray< T, I >::size(void)
{
  return I;
}

template< typename T, int I >
inline T&
FFixArray< T, I >::operator() (unsigned int i)
{
  assert( i < I );
  return comp[i];
}


template< typename T, int I >
inline const T&
FFixArray< T, I >::operator() (unsigned int i) const
{
  assert( i < I );
  return comp[i];
}


template< typename T, int I >
inline T&
FFixArray< T, I >::operator[] (unsigned int i)
{
  assert( i < I );
  return comp[i];
}

template< typename T, int I >
inline const T&
FFixArray< T, I >::operator[] (unsigned int i) const
{
  assert( i < I );
  return comp[i];
}

  
template< typename T, int I >
inline FFixArray< T, I > &
FFixArray< T, I >::operator=( const std::vector< T >& a )
{
  assert( a.size() <= I );
  memcpy( comp, &(a[0]), I * sizeof( T ) );
  return *this;
}

template< typename T, int I >
inline FFixArray< T, I >&
FFixArray< T, I >::operator=(const FFixArray< T, I >& a)
{
  memcpy( comp, a.comp, I * sizeof( T ) );
  return *this;
}
  
template< typename T, int I >
inline bool
FFixArray< T, I >::operator==(const FFixArray< T, I >& a) const
{
  return memcmp( comp, a.comp, I * sizeof( T ) ) == 0;
}

template< typename T, int I >
inline bool
FFixArray< T, I >::operator!=(const FFixArray< T, I >& a) const
{
  return !operator==( a );
}

template< typename T, int I >
inline bool
FFixArray< T, I >::operator<(const FFixArray< T, I >& a) const
{
  for ( unsigned int i = 0; i < I; i++ )
    if ( comp[ i ] < a.comp[ i ] )
      return true;
    else if ( comp[ i ] > a.comp[ i ] )
      return false;
      
  return false;
}

template< typename T, int I >
inline bool
FFixArray< T, I >::operator>(const FFixArray< T, I >& a) const
{
  for ( unsigned int i = 0; i < I; i++ )
    if ( comp[ i ] > a.comp[ i ] )
      return true;
    else if ( comp[ i ] < a.comp[ i ] )
      return false;
  
  return false;
}

template< typename T, int I >
inline bool 
FFixArray< T, I >::isGreaterThan(const FFixArray< T, I >& a) const
{
    for( unsigned int i = 0; i < I; ++i )
        if(comp[i] <= a.comp[i])
            return false;
    return true;
}

template< typename T, int I >
inline bool 
FFixArray< T, I >::isGreaterEqThan(const FFixArray< T, I >& a) const
{
    for( unsigned int i = 0; i < I; ++i )
        if(comp[i] < a.comp[i])
            return false;
    return true;
}

template< typename T, int I>
inline bool 
FFixArray< T, I >::isLessThan(const FFixArray< T, I >& a) const
{
    for( unsigned int i = 0; i < I; ++i )
        if(comp[i] >= a.comp[i])
            return false;
    return true;
}

template< typename T, int I>
inline bool 
FFixArray< T, I>::isLessEqThan(const FFixArray< T, I >& a) const
{
    for( unsigned int i = 0; i < I; ++i )
        if(comp[i] > a.comp[i])
            return false;
    return true;
}

template< typename T, int I >
inline FFixArray< T, I > &
FFixArray< T, I >::operator+=(const FFixArray< T, I > &a)
{
  for ( int i=0; i<I; i++ )
    comp[ i ] += a.comp[ i ];
  return *this;
}

template< typename T, int I >
inline FFixArray< T, I > &
FFixArray< T, I >::operator-=(const FFixArray< T, I > &a)
{
  for ( int i=0; i<I; i++ )
    comp[ i ] -= a.comp[ i ];
  return *this;
}
  
template< typename T, int I >
inline FFixArray< T, I > &
FFixArray< T, I >::operator*=( const T &d )
{
  for ( int i=0; i<I; i++ )
    comp[ i ] *= d;
  return *this;
}
  
template< typename T, int I >
inline FFixArray< T, I > &
FFixArray< T, I >::operator/=( const T &d )
{
  for ( int i=0; i<I; i++ )
    comp[ i ] /= d;
  return *this;
}
  
template< typename T, int I >
inline FFixArray< T, I >
FFixArray< T, I >::operator+(const FFixArray< T, I > &a) const
{
  FFixArray< T, I > tmp( *this );
  tmp += a;
  return tmp;
}
  

template< typename T, int I >
inline FFixArray< T, I >
FFixArray< T, I >::operator-(const FFixArray< T, I > &a) const
{
  FFixArray< T, I > tmp( *this );
  tmp -= a;
  return tmp;
}

// unary -
template< typename T, int I >
inline FFixArray< T, I >
FFixArray< T, I >::operator-(void) const
{
  FFixArray< T, I > tmp( *this );
  tmp *= -1;
  return tmp;
}



template< typename T, int I >
inline FFixArray< T, I >
FFixArray< T, I >::operator*(const T &d) const
{
  FFixArray< T, I > tmp( *this );
  tmp *= d;
  return tmp;
}
template< typename T, int I >
inline FFixArray< T, I >
FFixArray< T, I >::operator/(const T &d) const
{
  FFixArray< T, I > tmp( *this );
  tmp /= d;
  return tmp;
}


template< typename T, int I >
inline void
FFixArray< T, I >::getCoordinates( std::vector< T >& coords ) const
{
  coords.resize( I );
  memcpy( &coords[0], comp, I * sizeof( T ) );
}

template< typename T >
inline FFixArray< T, 3 > crossProduct( const FFixArray< T, 3 > &a,
				       const FFixArray< T, 3 > &b)
{
  FFixArray< T, 3 > dest;
  dest[0] = a[1]*b[2] - a[2]*b[1];
  dest[1] = a[2]*b[0] - a[0]*b[2];
  dest[2] = a[0]*b[1] - a[1]*b[0];
  return( dest );
}

template< typename T, int I >
inline void
FFixArray< T, I >::crossProduct( const FFixArray< T, 3 > &src2,
				 FFixArray< T, 3 > &dest ) const
{
  assert( I == 3 );
  dest[0] = comp[1]*src2[2] - comp[2]*src2[1];
  dest[1] = comp[2]*src2[0] - comp[0]*src2[2];
  dest[2] = comp[0]*src2[1] - comp[1]*src2[0];
}

template< typename T, int I >
inline FFixArray< T, 3 >&
FFixArray< T, I >::crossProduct( const FFixArray< T, 3 > &src )
{
  assert( I == 3 );
  FFixArray< T, 3 > dest;
  crossProduct( src, dest );
  *this = dest;
  return *this;
}

template< typename T, int I >
inline T
FFixArray< T, I >::operator*(const FFixArray< T, I > &vec) const
{
  T d = 0.0;
  for ( int i=0; i<I; i++ )
    d += comp[i] * vec.comp[i];
  return d;
}

template< typename T, int I >
inline T
FFixArray< T, I >::normSquare() const
{
  T d = 0.0;
  for ( int i=0; i<I; i++ )
    d += comp[i]*comp[i];
  return d;
}

template< typename T, int I >
inline T
FFixArray< T, I >::norm() const
{
  return sqrt( normSquare() );
}

template< typename T, int I >
inline FFixArray<T,I>&
FFixArray< T, I >::normalize()
{
  T invNorm = 1. / norm();
  for ( int i=0; i<I; i++ )
    comp[ i ] *= invNorm;
  return *this;
}

template< typename T, int I >
inline FFixArray<T,I>
FFixArray< T, I >::normalized() const
{
  FFixArray< T, I> ret;
  T invNorm = 1. / norm();
  for ( int i=0; i<I; i++ )
    ret[ i ] = comp[ i ] * invNorm;
  return *this;
}

template< typename T, int I >
inline T
FFixArray< T, I >::distanceSquare(const FFixArray< T, I >& pos2) const
{
  T d = 0.0, x;
  for ( int i=0; i<I; i++ ) {
    x = comp[ i ] - pos2.comp[ i ]; d += x*x;
  }
  return d;
}

template< typename T, int I >
inline T
FFixArray< T, I >::distance(const FFixArray< T, I > &pos2) const
{
  return sqrt( distanceSquare( pos2 ) );
}

template< typename T, int I >
inline std::ostream& operator<< ( std::ostream& os, const FFixArray< T, I > &a )
{
 
  os << "[ " << a[ 0 ];
  for ( int i=1; i<I; i++ ) {
    os << " " << a[ i ];
  }
  os << " ]";
  return os;
}

template< typename T, int I >
inline std::istream& operator>> ( std::istream &is ,FFixArray< T, I > &a )
{
  std::string tmp;
  is >> tmp >> a[ 0 ];
  for ( int i=1; i<I; i++ ) {
    is >> tmp >> a[ i ];
  }
  is >> tmp;
  return is;
}
  
template< typename T, int I >
inline FFixArray< T, I > operator*( const T &d, const FFixArray< T, I >& a)
{
  FFixArray< T, I > tmp ( a );
  tmp *= d;
  return tmp;
}

typedef FFixArray< double, 2 > FArray2;
typedef FFixArray< double, 3 > FArray3;

template< typename T, int I >
inline FFixArray< T, I >::FFixArray( const T &ref0, const T &ref1 )
{
    comp[ 0 ] = ref0;
    comp[ 1 ] = ref1;
}
template< typename T, int I >
inline FFixArray< T, I >::FFixArray( const T &ref, const T &ref1, const T &ref2)
{
    comp[ 0 ] = ref;
    comp[ 1 ] = ref1;
    comp[ 2 ] = ref2;
}
template< typename T, int I >
inline FFixArray< T, I >::FFixArray( const T &ref, const T &ref1, const T &ref2, const T &ref3)
{
    comp[ 0 ] = ref;
    comp[ 1 ] = ref1;
    comp[ 2 ] = ref2;
    comp[ 3 ] = ref3;
}
template< typename T, int I >
inline FFixArray< T, I >::FFixArray( const T &ref, const T &ref1, const T &ref2, const T &ref3, const T &ref4)
{
    comp[ 0 ] = ref;
    comp[ 1 ] = ref1;
    comp[ 2 ] = ref2;
    comp[ 3 ] = ref3;
    comp[ 4 ] = ref4;
}


template< typename T, int I >
inline FFixArray< T, I >& FFixArray< T, I >::negate()
{
	for(int i=0; i<I; ++i)
		comp[i] = -comp[i];
	return *this;
}

//---------------------------------------------------------------------------

template< typename T, int I >
inline FFixArray< T, I>& FFixArray<T,I >::positivate()
{
  unsigned int i;
  for(i=0; i<I; ++i)
  {
    if(comp[i]<0) comp[i] = -comp[i];
  }
  return *this;
}

//---------------------------------------------------------------------------

template< typename T, int I>
inline FFixArray< T, I >& FFixArray< T, I >::clipValues(double min, double max)
{
  unsigned int i;
  for(i=0; i<I; ++i)
  {
    if(comp[i] < min) comp[i] = min;
    if(comp[i] > max) comp[i] = max;
  }
  return *this;
}


#endif
