//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FArray.hh,v $
// Language:  C++
// Date:      $Date: 2004/06/29 15:10:19 $
// Author:    $Author: wiebel $
// Version:   $Revision: 1.27 $
//
//---------------------------------------------------------------------------
#ifndef __FArray_hh
#define __FArray_hh

#include <iosfwd>
#include <vector>
#include <cstring>

#include "stdAliases.hh"

class FMatrix;

/**
 * Class implementing an array of doubles.
 * Please note: FObject is not a base class of FArray to reduce
 * memory consumption.
 */
class FArray
{
public:

// ============= constructors ======================
  /** constructs empty array.
   */
  FArray();

  /** constructs array of size s.
   * \param s size.
   */
  explicit FArray(unsigned int s);

  /**
   * Copy constructor.
   * \param a
   * Array to copy.
   */
  FArray(const FArray& a);

  /**
   * Copy constructor.
   * \param a
   * Vector to copy into this array.
   */
  FArray(const std::vector<double>& a);

  /**
   * Copy constructor.
   * \param m
   * Matrix to copy into this array.
   */
  FArray(const FMatrix& m);

  /**
   * Constructor for a 2-dimensional array.
   * \param x
   * Value of first element.
   * \param y
   * Value of second element.
   */
  FArray(double x, double y);

  /**
   * Constructor for a 3-dimensional array.
   * \param x
   * Value of first element.
   * \param y
   * Value of second element.
   * \param z
   * Value of third element.
   */
  FArray(double x, double y, double z);

  /**
   * Constructor for a 4-dimensional array.
   * \param x
   * Value of first element.
   * \param y
   * Value of second element.
   * \param z
   * Value of third element.
   */
  FArray(double x, double y, double z, double w);

  /**
   * Constructor for a 5-dimensional array.
   * \param x
   * Value of first element.
   * \param y
   * Value of second element.
   * \param z
   * Value of third element.
   */
  FArray(double x, double y, double z, double w, double w2);

  /**
   * Constructor for a 6-dimensional array.
   * \param x
   * Value of first element.
   * \param y
   * Value of second element.
   * \param z
   * Value of third element.
   */
  FArray(double x, double y, double z, double w, double w2, double w3);

  /**
   * Constructor, construct array from double array.
   * \param s
   * Size of double array.
   * \param ref
   * Standard array of doubles.
   */
  explicit FArray(unsigned int s, const double ref[]);

  /**
   * Dummy constructor. This constructor is a left-over of the old
   * FVector class. Since this constructor is ambigous with
   * FArray::FArray(double x, double y) it can't be used any longer.
   * Please use the other constructors.
   */
  FArray(unsigned int c, double v);

  /**
   * clear containts of FVector
   */
  void clear();

  /** Destructor.
   */
  virtual ~FArray();

  // ============= modifiers ======================

  /** Size reporter.
   * \return number of array-entries.
   */
  unsigned int size(void) const ;

  /** resizer.
   * \param s
   * New size.
   * \param keepContent
   * Preserve entries.
   */
  void resize(unsigned int s, bool keepContent=false);

  // ============= operators ======================

  /** Access operator ()
   * \param index
   * Index of element to access.
   * \return
   * Reference to entry.
   * \exception FIndexOutOfBoundsException
   * Thrown if NODEBUG is not defined and an invalid element is to be accessed.
   */
  double& operator() (unsigned int i);

  /** Access operator () const.
   * \param index
   * Index of element to access.
   * \return
   * Const reference to entry.
   * \exception FIndexOutOfBoundsException
   * Thrown if NODEBUG is not defined and an invalid element is to be accessed.
   */
  const double& operator() (unsigned int i) const;

  /** Access operator []
   * \param index
   * Index of element to access.
   * \return
   * Reference to entry.
   * \exception FIndexOutOfBoundsException
   * Thrown if NODEBUG is not defined and an invalid element is to be accessed.
   */
  double& operator[] (unsigned int i);

  /** Access operator [] const.
   * \param index
   * Index of element to access.
   * \return
   * Const reference to entry.
   * \exception FIndexOutOfBoundsException
   * Thrown if NODEBUG is not defined and an invalid element is to be accessed.
   */
  const double& operator[] (unsigned int i) const;

  /** assignment operator
   * \param a
   * Rightside
   * \return
   * Reference to self
   */
  FArray& operator=(const std::vector<double>& a);

  /** assignment operator
   * \param a
   * Rightside
   * \return
   * Reference to self
   */
  FArray& operator=(const FArray& a);

  /** comparison operator
   * \param a
   * Rightside
   * \return
   * True if condition matches, false if not.
   */
  bool operator==(const FArray& a) const;

  /** comparison operator
   * \param a
   * Rightside
   * \return
   * true if this is smaller than a (the priority of the components
   * is equivalent to their index, index 0 has highest priority)
   */
  bool operator<(const FArray& a) const;

  /** comparison operator
   * \param a
   * Rightside
   * \return
   * true if every element of a is smaller than every element of b
   */
  bool isLessThan(const FArray& b) const;

  /** comparison operator
   * \param a
   * Rightside
   * \return
   * true if this is greater than a (the priority of the components
   * is equivalent to their index, index 0 has highest priority)
   */
  bool operator>(const FArray& a) const;

  /** comparison operator
   * \param a
   * Rightside
   * \return
   * True if condition matches, false if not.
   */
  bool operator!=(const FArray& a) const;

  /** simple lowlevel (hopefully fast) array addition operator
   * \param a
   * Rightside
   * \exception FInvalidDimensionException
   * If NODEBUG is not defined and arguments have nonmatching dimensions.
   */
  void operator+=(const FArray& a);

  /** simple lowlevel (hopefully fast) array subtraction operator
   * \param a
   * Rightside
   * \exception FInvalidDimensionException
   * If NODEBUG is not defined and arguments have nonmatching dimensions.
   */
  void operator-=(const FArray& a);

  /** simple lowlevel (hopefully fast) array multiplication operator
   * \param a
   * Rightside
   * \exception FInvalidDimensionException
   * If NODEBUG is not defined and arguments have nonmatching dimensions.
   */
  void operator*=(double d);

  /** Array addition operator.
   * Should be used carefully since this operator implies
   * one complete copy operation. A more flexible operator is plus.
   * \param a
   * Rightside
   * \return
   * Reference to self / copy of result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  FArray operator+(const FArray& a) const;

  /** Array subtraction operator.
   * Should be used carefully since this operator implies
   * one complete copy operation. A more flexible operator is minus.
   * \param a
   * Rightside
   * \return
   * Reference to self / copy of result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  FArray operator-(const FArray& a) const;

  /** Unary Array minus operator.
   * Should be used carefully since this operator implies
   * one complete copy operation.
   * \return
   * Result.
   */
  FArray operator-() const;

  /** Array multiplication operator.
   * Should be used carefully since this operator implies
   * one complete copy operation. A more flexible operator is multiply.
   * \param a
   * Rightside
   * \return
   * Reference to self / copy of result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  FArray operator*(double d) const;


  /** Array addition operator.
   * Add an array to this one and store the result in a destination array.
   * \param arg
   * Array to add.
   * \param dst
   * Destination array for result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  void plus(const FArray& arg, FArray& dst) const;

  /** Array subtraction operator.
   * Subtract an array from this one and store the result in a
   * destination array.
   * \param arg
   * Array to subtract.
   * \param dst
   * Destination array for result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  void minus(const FArray& arg, FArray& dst) const;

  /** Array multiplication operator.
   * Multiply a double with this array and store the result in a
   * destination array.
   * \param arg
   * Scalar multiplicator.
   * \param dst
   * Destination array for result.
   * \exception FInvalidDimensionException if NODEBUG is not defined
   */
  void mult(double arg, FArray& dst) const;

  /**
   * Store array values in a vector.
   * \param coords
   * Destination vector.
   */
  void getCoordinates(std::vector<double>& coords) const;

  /**
   * Store array values in a vector.
   * \param coords pre allocated target data structure
   * Destination vector.
   */
  void getCoordinates(double* coords) const;

  /**
   * Calculate cross product of this vector and another one.
   * \param b
   * Rightside argument.
   * \param dest
   * Destination array.
   * \exception FInvalidDimensionException
   * Thrown, if any of the arrays has dimension other than 3.
   */
  void crossProduct(const FArray &b, FArray &dest) const;

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
  FArray& crossProduct(const FArray &b);

  /**
   * Calculate cross product of this vector and another one and
   * DON'T modify this vector.
   * \param b
   * Rightside argument.
   * \return
   * Destination array.
   * \post
   * Object has NOT been modified
   * \exception FInvalidDimensionException
   * Thrown, if any of the arrays has dimension other than 3.
   */
  FArray crossProductConst(const FArray &b) const;

  /**
   * Calculate cross product of two 3D vectors
   * \param a, b
   * vector arguments
   * \exception FInvalidDimensionException
   * Thrown, if any of the arrays has dimension other than 3.
   * \return
   * cross product of both vectors
   */
  friend FArray crossProduct(const FArray &a, const FArray &b);

  /**
   * Calculate cross product of two 2D vectors and returns scalar value
   * \param a, b
   * vector arguments
   * \exception FInvalidDimensionException
   * Thrown, if any of the arrays has dimension other than 2.
   * \return
   * cross product of both vectors
   */
  friend double crossProduct2D(const FArray &a, const FArray &b);

  /**
   * (*this) = -(*this) but faster
   * \return *this
   */
  FArray& negate();

  /**
   * (*this)[i] = fabs((*this)[i]) for every i
   */
  FArray& positivate();

  /**
   * clip component values to range of min and max
   */
  FArray& clipValues(double min, double max);

  /**
   * Normalize this array. (Make it a unit vector.)
   */

  FArray& normalize();

  /**
   * return a normalized version of this array.
   * (Same direction but unit vector)
   */
  FArray normalized() const;

  /**
   *Gives the dimension of the array.
   * \return
   * Dimension of this array.
   */
  unsigned int getDimension() const;

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
  double operator*(const FArray& vector) const;

  /** Euclidian vector norm.
   * \return
   * Euclidian norm: $ \| \langle \vec{a}, \vec{a} \rangle \| $.
   */
  double norm() const;

  /** Euclidian vector norm.
   * \return
   * Square of Euclidian norm: $ \| \langle \vec{a}, \vec{a} \rangle \| $.
   */
  double normSquare() const;

  /**  Calculate distance between this position and another one.
   * Only reasonable, if you interpret this array as a position.
   * \param pos2
   * The other position.
   * \return
   * Distance.
   */
  double distance(const FArray& pos2) const;

  /** Calculate square distance between this position and another one.
   * Only reasonable, if you interpret this array as a position.
   * \param pos2
   * The other position.
   * \return
   * Square distance.
   */
  double distanceSquare(const FArray& pos2) const;

   /** Conversion operator
   * \return
   * the FArray as a vector of doubles
   */
  operator std::vector<double>() const;

  friend class FMatrix;
  friend class FTensor;
  friend class FRefArray;

  friend std::ostream& operator<< ( std::ostream& os, const FArray& a );
  friend std::istream& operator>> ( std::istream& is ,FArray& a );
  friend FArray operator*(double d, const FArray& a);
  friend FArray& operator*=(FArray& v, const FMatrix& matrix);
  friend FArray operator*(const FArray& v, const FMatrix& matrix);

  // this is required in theDataSet to make it fast.
  // Please don't use it anywhere else
  double* getRawData(){return comp;}
  const double* getRawData() const {return comp;}

protected:

  /// pointer to memory holding the entries
  double* comp;

  /// the amount of currently allocated doubles
  unsigned int sizeOfArray;

  friend std::ostream & binwrite( std::ostream & os, const FArray & xr );
  friend std::istream & binread( std::istream & is, FArray& xpr );
};

/**
 *! Define FPosition as FArray
 */
typedef FArray FPosition;
/**
 *! Define FVector as FArray
 */
typedef FArray FVector;

#ifndef OUTLINE
#include "FArray.icc"
#endif

#endif
