//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FMatrix.hh,v $
// Language:  C++
// Date:      $Date: 2004/03/08 11:52:33 $
// Author:    $Author: hlawit $
// Version:   $Revision: 1.23 $
//
//---------------------------------------------------------------------------

#ifndef __FMatrix_hh
#define __FMatrix_hh

#include <vector>
#include <complex>
#include <iosfwd>

#include "FArray.h"

//#include "FRefArray.hh"
class FRefArray;
class FTensor;

//===========================================================================

/**
 * The FMatrix class provides an implementation of an NxM-Matrix.
 * To put it in other words, the matrix consists of N columns and M rows.
 */
class FMatrix
{
public:

  /** Default constructor.
   */
  FMatrix();

  /** Copy constructor.
   * \param
   * matrix Matrix to copy.
   */
  FMatrix(const FMatrix& m);

  /** Copy constructor.
   * \param
   * tensor Second order tensor to copy matrix from.
   */
  FMatrix(const FTensor& tensor);

  /** Constructor of square matrix, elements defaulted to zero.
   * \param dim
   * Dimension.
   */
  FMatrix(positive dim);

  /** Constructor, elements defaulted to zero.
   * \param dimy
   * Number of rows.
   * \param dimx
   * Number of columns.
   * \param dimn
   * Number of rows.
   */
  FMatrix(positive dimy, positive dimx);

  /** Constructor.
   * \param dimy
   * Number of rows.
   * \param dimx
   * Number of columns.
   * \param ref
   * Double array containing the matrix entries.
   */
  FMatrix(positive dimy, positive dimx, const double ref[]);

  /** Destructor. */
  ~FMatrix();

  /**
   * compute the squared 2-norm
   */
  double normSquare( void ) const;

  /**
   * compute the 2-norm
   *
   * use normSquare if you want it faster
   */
  double norm( void ) const;

  /** Inversion of a matrix
   *   Operation on the Object Itself only works with MxM matrices.
   * \return
   *   denominator if matrix has dimension<=3,
   *   0 else
   * \exception
   *   FInvalidDimensionException
   *   FMatrixSingularException if matrix has dimension <=3
   */
  double invert (void );

   /** Inversion of a matrix
   *   Returns inverted Matrix
   *   Operation on the Object only works with MxM matrices.
   * \return
   *   denominator if matrix has dimension<=3,
   *   0 else
   * \exception
   *   FInvalidDimensionException
   *   FMatrixSingularException
   */
  friend FMatrix invert(const FMatrix& A);

  /** Transposition of a matrix
   *   Returns the transposed self
   * \return
   *   FMatrix holding a NxM matrix (switched dimensions..)
   */
  FMatrix transposed(void) const;

  /** Transposition of a matrix
   *   transposes the object itself
   * \return
   *   FMatrix holding a NxM matrix (switched dimensions..)
   */
  FMatrix& transpose(void);

  /** Resizing of matrix
   *   depending on the keepvalues flag fills new space with zero or
   *   undefined values
   * \param
   *   m number or rows
   * \param
   *   n number of columns
   * \exception
   *   FInvalidDimensionException
   */
  FMatrix& resize(unsigned int m, unsigned int n, bool keepvalues=false);

  /**  Calculate the discriminant of a matrix (3,3).
   *
   * \pre
   *   Matrix is (3,3) or (2,2).
   * \exception
   *   FInvalidDimensionException
   */
  double discriminant(void) const;

  /**  Calculate the trace of a quadratic matrix.
   *
   *\pre
   * Matrix is quadratic.
   * \exception
   *   FInvalidDimensionException
   */
  double trace(void) const;

  /**  Calculate the determinant \f[ det|M| \f] of a quadratic (square) matrix M.
   *
   *\pre
   * Matrix is quadratic (2,2) or (3,3).
   * \exception
   *   FInvalidDimensionException
   */
  double detOf(void) const;


  /**
   * Create a diagonal matrix containing v as its diagonal elements.
   * \pre
   * v is a valid F::FVector
   * \post
   * Matrix is square and M(i,i) = v(i)
   */
  void makeDiagonal(const F::FVector& v);

  /**
   * Set the matrix to be a idendity matrix, i.e., a diagonal matrix
   * containing only 1 on the diagonal
   */
  void makeIdendity();

  /**
   * Set the matrix to a scaling matrix, i.e., a diagonal matrix
   * containing value d on the diagonal
   */
  void makeScaling(double d);

   /**
   * Create a diagonal matrix containing v as its diagonal elements.
   * \pre if dim == 0, matrix must be square
   * if dim > 0, size of d must match dim
   * \param d
   * values
   * \param dim
   * dimension of d and of matrix to construct
   * if dim==0 current dimensions are assumend
   * \post Matrix is square diagonal matrix with m(i,i) = d[i] and dimension
   * dim or current dimensions if dim == 0
   */
  void makeDiagonal(const double& d, positive dim = 0);

  /**
   * Efficient method to convert an FArray into a matrix.
   * \pre
   * v is a valid F::FVector and has same number of components as *this
   * \post
   * components of *this are set to those of v
   */
  void set(const F::FVector & v);

  /**
   * Householder reduction of a real symmetric matrix to a tridiagonal one.
   * \pre
   * Matrix must be real and symmetric (implies square)
   *\post
   * Matrix is now tridiagonal.
   *\exception
   * FInvalidDimensionException
   *\param
   * d Returns diagonal elements of the tridiagonal matrix.
   *\param
   * e Returns the off-diagonal elements with e[0]=0.
   */
  void tridiagonalize(F::FVector& d, F::FVector& e);

  /**
   * Tridiagonalized QL algorithm with implicit shifts.
   * This algorithm determines the eigenvectors and -values
   * of a tridiagonalized FMatrix.
   * \pre
   * ::tridiagonalize() has been called. (and its preconditions
   * are satisfied)
   * \post
   * Matrix now contains eigenvectors (as columns).
   *\exception
   * FInvalidDimensionException
   *\param
   * d Diagonal elements of the tridiagonal matrix.
   *\param
   * e Off-diagonal elements with e[0] arbitrary..
   */
  void tQLiAlgorithm(F::FVector& d, F::FVector& e);

  /**
   * Calculate Eigenvalues and Eigenvectors of matrix.
   *\pre
   * Matrix is real and symmetric (square with arbitrary size).
   *\post
   * Matrix now contains eigenvectors (as columns).
   * vals and vectors have been resized to match the
   * size of the matrix
   *\param
   * vals F::FVector to store Eigenvalues in.
   *\param
   * vectors STL vector containing eigenvectors.
   */
  void getEigenSystem(F::FVector& vals, std::vector<F::FVector>& vectors);

   /**
   *\par Description:
   * Calculate Eigenvalues and eigenvectors of the Matrix.
   *\pre
   * By now the matrix must be 3x3 (because it uses getEigenValues)
   *\post
   * evals contains the eigenvalues of the matrix and is resized to 3
   * evecs contains the eigenvectors of the matrix and is resized to 3
   */
    void getEigenSystem( std::vector< std::complex< double > >& evals,
             std::vector< F::FVector >& evecs );

  /**
   *\par Description:
   * Calculate Eigenvalues of the Matrix.
   *\pre
   * By now the matrix must be 2x2 or 3x3. future inventions welcome.
   *\post
   * values contains the eigenvalues of the matrix and is resized to 2 or 3 resp.
   *\param
   * values   vector<> of complex doubles.
   */
  void getEigenValues( std::vector< std::complex<double> >& values) const;


  /** Gauss-Jordan Solver
   *\pre
   * Matrix is quadratic.
   *\post
   * Martrix is inverted and rightside contains the solution(s)
   *\param rightside containing the results to solve the system for
   */
  FMatrix& gaussJ( FMatrix& rightside );

  /** Gauss-Jordan Solver
   *\pre
   * Matrix is quadratic.
   *\post
   * Martrix is inverted and rightside contains the solution(s)
   *\param rightside containing the result to solve the system for
   */
  F::FVector& gaussJ( F::FVector& rightside );

  /**
   * Cyclic Jacobi Algorithm
   *
   * Compute eigenvectors and eigenvalues of a symmetric Matrix A
   * - quadratic convergence
   * - 2* sizeof(A) memory consumtion ( need to store A and V )
   *
   * \pre
   * A has to be symmetric
   * S(A^(k)) < \delta, S(A) = sum_{i,j=0, i!=j}^n a_{i,j}^2
   *
   * \post
   * Diagonal elements of A^(k) are the approximated eigenvalues
   * Column vectors of V_k = V_{k-1}U_k are approximated corresponding eigenvectors
   *
   * taken from: http://www.uni-kl.de/AG-Heinrich/uni/numerik/Numerik-fol-kap7.pdf
   * pp 26ff
   */
  void cyclicJacobi(FMatrix &v, const double epsSquare, unsigned int nbIter);

  /** Matrix multiplication (binary).
   */
  FMatrix operator*(const FMatrix &b) const;

  ///  Undocumented.
  FMatrix& operator*=(const FMatrix &b);

  ///  Undocumented.
  FMatrix& mult(const FMatrix &m, FMatrix &d) const;

  /** Matrix by Vector multiplication (binary).
   */
  F::FVector operator*(const F::FVector& v) const;

  ///  Undocumented.
  F::FVector& mult(const F::FVector &m, F::FVector &d) const;

  F::FVector& mult(const FTensor &m, F::FVector &d) const;

  FTensor& mult(const FTensor &m, FTensor &d) const;

  /** Matrix by double multiplication (binary).
   *   Multiplication of a Matrix by a Matrix.
   */
  FMatrix operator *(const double& lamda) const;

  ///  Undocumented.
  FMatrix& operator *=(const double& lamda);

  ///  Undocumented.
  FMatrix& mult(const double &, FMatrix &d) const;

  /** Matrix by Matrix addition
   */
  FMatrix operator+(const FMatrix &m) const;

  ///  Undocumented.
  FMatrix& operator+=(const FMatrix &m);
  ///  Undocumented.
  FMatrix& plus(const FMatrix &m, FMatrix& d) const;
  ///  Undocumented.
  FMatrix operator-(void) const;
  ///  Undocumented.
  FMatrix operator-(const FMatrix &m) const;
  ///  Undocumented.
  FMatrix& operator-=(const FMatrix &m);
  ///  Undocumented.
  FMatrix& minus(const FMatrix &m, FMatrix& d) const;

  ///  Undocumented.
  double& operator()(positive i, positive j);
  ///  Undocumented.
  const double& operator ()(positive i, positive j) const;

  ///  Undocumented.
  double& operator[](positive i);
  ///  Undocumented.
  const double& operator[](positive i) const;

  /** gives the ith row of the matrix as refarray
   * \return FRefArray with data pointer set
   * to ith row in matrix
   */
  FRefArray row(positive i);

  /** const version of row(positive i)
   */
  const FRefArray row(positive i) const;

  void setRow( positive i, const FArray &values );
  void setColumn( positive i, const FArray &values );

  FArray getColumn( positive i ) const; //< slow!

  ///  Undocumented.
  FMatrix& operator=(const FMatrix &src);

  /** Row/Column Modifiers
   * Rows [Columns] a and b are interchanged
   *\param a,b  the indices of the 2 rows.
   */
  void swapCols (positive a, positive b);

  ///  Undocumented.
  void swapRows (positive a, positive b);

  ///  Undocumented.
  positive getDimensionX() const;
  ///  Undocumented.
  positive getDimensionY() const;

  friend std::ostream& operator<< (std::ostream& os, const FMatrix& m);
  friend std::istream& operator>> (std::istream& os, FMatrix& m);

  friend F::FVector operator*(const F::FVector &v, const FMatrix& m);
  friend FMatrix operator*(const double &lamda, const FMatrix& m);
  friend F::FVector& operator*=(F::FVector& v, const FMatrix& matrix);

  ///  Undocumented.
  bool solveSingularSystem(F::FVector& b, F::FVector& x,
          std::vector<F::FVector>& nullspace,
          std::vector<F::FVector>& range, double zeroTresholdEpsilon = 1.0e-6) const;

  /** Size reporter.
   * \return number of array-entries.
   */
  unsigned int size(void) const;

  /**
   *  used by solveSingularSystem()
   * \post
   * *this replaced by the matrix U of decomposition
   */
  void svdcmp(F::FVector& w, FMatrix& v);

  /**
   *  used by solveSingularSystem()
   */
  static void svDecomp(const FMatrix& u, const F::FVector& w, const FMatrix& v, const F::FVector& b, F::FVector& x);

  /**
   * copy a matrix sub in a bigger matrix
   * \post
   *   sub is set at position j,i
   * \param i,j row and column of left-top element
   * \param sub
   *    matrix data to be copied
   * \returns
   *    reference to restult
   */
  FMatrix& setSubMatrix( unsigned int j, unsigned int i, const FMatrix &sub);

  void getSubMatrix( FMatrix &sub, unsigned int j, unsigned int i);



  /**
   * Compute the n-th power of this matrix
   * \param n power to comput, n>= 0
   * \result matrix now contains the n-th power of itself
   * \returns reference to result
   */
  FMatrix& pow(int n);

  /**
   * set the matrix to its symmetric part only
   * \pre matrix must be square
   * \post matrix contains symmetric part only
   * \returns reference to self
   */
  FMatrix& makeSymmetric();

  /**
   * set the matrix to its antisymmetric part only
   * \pre matrix must be square
   * \post matrix contains antisymmetric part only
   * \returns reference to self
   */
  FMatrix& makeAntisymmetric();

  //
  // Functions to create 3x3 and 4x4 rotation matrices
  //

  /**
   * Create a right handed 3x3 rotation matrix rotating theta degrees around F::FVector
   */
  static FMatrix rot3RH( const F::FVector& axis, double angle );
  /**
   * Create a right handed 4x4 rotation matrix rotating theta degrees around F::FVector
   */
  static FMatrix rot4RH( const F::FVector& axis, double angle );
  /**
   * Create a left handed 3x3 rotation matrix rotating theta degrees around F::FVector
   */
  static FMatrix rot3LH( const F::FVector& axis, double angle );
  /**
   * Create a left handed 4x4 rotation matrix rotating theta degrees around F::FVector
   */
  static FMatrix rot4LH( const F::FVector& axis, double angle );



private:
  double* comp;  // pointer to memory holding the entries
  unsigned int sizeOfArray; // the amount of currently allocated doubles
  unsigned int dimx, dimy;  // the dimensions of our matrix
};


#ifndef OUTLINE
#include "FMatrix.icc"
#endif


#endif // __FMatrix_hh
//===========================================================================
