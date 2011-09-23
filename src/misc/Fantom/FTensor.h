//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FTensor.hh,v $
// Language:  C++
// Date:      $Date: 2004/06/21 12:41:32 $
// Author:    $Author: hlawit $
// Version:   $Revision: 1.33 $
//
//---------------------------------------------------------------------------

#ifndef __FTensor_hh
#define __FTensor_hh

#include "StdAliases.h"
#include "FArray.h"
#include "FMultiIndex.h"

#include <vector>

class FRefTensor;

//===========================================================================

#include "FVector.h"
// class FMatrix;

/**
 *The FTensor class provides a representation of the mathematical
 *tensor. Its order may be 0, 1 or 2, 3 corresponding to scalar,
 *vector, matrix or 3d-order tensor values.
 */
class FTensor : public FArray
{
public:

  /**
   *\par Description:
   *Constructor: provides an empty tensor.
   */
  FTensor();

  /**
   *\par Description:
   *Constructor: provides a tensor of dimension \b dimension and order
   *\b order.
   *\pre
   *order <= 2, dimension = {0,2,3}
   *0-dimensional tensors are non meaningful.
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\param
   *dimension: tensor dimension
   *\param
   *order: tensor order
   *\param
   *clear: boolean to indicate that the constructor must return a zero tensor
   */
  FTensor(unsigned char dimension, unsigned char order, bool clear=false);

  /**
   *\par Description:
   *Constructor: provides a tensor of dimension \b dimension and order
   *\b order and with double coordinates \b comp.
   *\pre
   *order <= 2, dimension = {0,2,3}\\
   *0-dimensional tensors are not meaningful\\
   *The size of comp must fit the one defined by order and dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\param
   *dimension: value of the dimension.
   *\param
   *order: value of the order.
   *\param
   *comp: array of double coordinates.
   */
  FTensor(unsigned char dimension, unsigned char order,
      const std::vector<double>& comp);

  /**
   *\par Description:
   *Copy constructor: provides a tensor, copy of its argument.
   *\pre
   *none
   *\post
   *none
   *\exception
   *none
   *\param
   *T: tensor to copy.
   */
  FTensor(const FTensor& T);

  /**
   * Copy constructor. Generate tensor from matrix.
   * \param m
   * Matrix to copy.
   */
  explicit FTensor(const FMatrix& m);

  /**
   * Copy constructor. Generate tensor from vector.
   * \param v
   * Vector to copy.
   */
  explicit FTensor(const F::FVector& v);

  /**
   *\par Description:
   *Destructor.
   *\pre
   *none
   *\post
   *none
   *\exception
   *none
   */
  ~FTensor();

  /**
   *\par Description:
   *Gets the class name.
   *\pre
   *none
   *\post
   *none
   *\exception
   *none
   */
  //  virtual const FString& getClassName() const;

  /**
   *\par Description:
   *Gives the dimension of the tensor.
   *\pre
   *none
   *\post
   *none
   *\exception
   *none
   */
  unsigned int getDimension() const;

  /**
   *\par Description:
   *Gives the order of the tensor.
   *\pre
   *none
   *\post
   *none
   *\exception
   *none
   */
  unsigned int getOrder() const;

  /**
   * sets dimension of the tensor (and invalidates the actual value !)
   * \exception
   * FInvalidDimensionException, FException...
   */
  void setDimension(unsigned char );

  /**
   * Sets order of the tensor (and invalidates the actual value !)
   * \exception
   * FInvalidDimensionException, FException...
   */
  void setOrder(unsigned char );

  /** resizer(version of resize ( , ) for compatibility.
   * \param dim
   * New dimension.
   * \param ord
   * New order.
   */
  void resizeTensor (unsigned char dim, unsigned char ord);

  /** resizer.
   * \param dim
   * New dimension.
   * \param ord
   * New order.
   */
  void resize(unsigned char dim, unsigned char ord);

  /**
   *\par Description:
   *Assignment of a FTensor.
   *\pre
   *none
   *\post
   *Every components have been copied.
   *\exception
   *none
   *\param
   *T: tensor to copy.
   */
  FTensor& operator=(const FTensor& T);

  /**
   *\par Description:
   *Set all components of a Tensor to a given double value
   *\pre
   *Tensor's dimension and order have been set
   *\post
   *Every components have been set
   *\exception
   *FEmptyObjectException
   *\param
   *val: double value.
   */
  FTensor& operator=(double val);

  /**
   *\par Description:
   *Binary operator+.
   *\pre
   *Both tensors have the same dimension and order.
   *\post
   *Every components have been summed.
   *\exception
   *FException: incompatible dimensions and/or orders.
   *\param
   *T: tensor to be summed.
   */
  FTensor operator+(const FTensor& T) const;

  /**
   *\par Description:
   *Binary operator-.
   *\pre
   *Both tensors have the same dimension and order.
   *\post
   *Every components have been subtracted.
   *\exception
   *FException: incompatible dimensions and/or orders.
   *\param
   *T: tensor to be subtracted.
   */
  FTensor operator-(const FTensor& T) const;

  /**
   *\par Description:
   *Binary operator+ followed by an affectation.
   *\pre
   *Both tensors have the same dimension and order.
   *\post
   *Every components have been summed.
   *\exception
   *FException: incompatible dimensions and/or orders.
   *\param
   *T: tensor to be summed.
   */
  FTensor& operator+=(const FTensor& T);

  /**
   *\par Description:
   *Multiplication of a FTensor by a scalar.
   *\pre
   *The dimension and order of the tensor have been defined (ie != 0).
   *\post
   *Every coordinates have been multiplied by {\it lambda}.
   *\exception
   *FEmptyObjectException
   *\param
   *lambda: double value to multiply.
   */
  FTensor operator*(double lambda) const;

  /**
   *\par Description:
   *Multiplication of a FTensor by a scalar.
   *\pre
   *The dimension and order of the tensor have been defined (ie != 0).
   *\post
   *Every coordinates have been multiplied by {\it lambda}.
   *\exception
   *FEmptyObjectException
   *\param
   *lambda: double value to multiply.
   *\param
   *T: tensor to be multiplied.
   */
  friend FTensor operator*(double lambda, const FTensor& T);

  /**
   *\par Description:
   *Multiplication of a FTensor by a scalar.
   *\pre
   *The dimension and order of the tensor have been defined (ie != 0).
   *\post
   *Every coordinates have been multiplied by {\it lambda}.
   *\exception
   *FEmptyObjectException
   *\param
   *lambda: double value to multiply.
   */
  FTensor& operator*=(double lambda);

  /**
   *\par Description:
   *Comparison of two FTensors.
   *\pre
   *none
   *\post
   *returns true iff both positions are the same.
   *\exception
   *none
   *\param
   *T: tensor to compare.
   */
  bool operator==(const FTensor& T) const;

  /**
   *\par Description:
   *Comparison of two FTensors.
   *\pre
   *none
   *\post
   *returns true iff both positions are different.
   *\exception
   *none
   *\param
   *T: tensor to compare.
   */
  bool operator!=(const FTensor& T) const;

  /**
   *\par Description:
   *Gets the scalar component of a scalar FTensor.
   *\pre
   *the FTensor is of order 0.
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FEmptyObjectException
   */
  const double& operator()(void) const;

  /**
   *\par Description:
   *Gets the scalar component of a scalar FTensor.
   *\pre
   *the FTensor is of order 0.
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FEmptyObjectException
   */
  double& operator()(void);


  const double& operator()(const FMultiIndex& i) const;
  double& operator()(const FMultiIndex& i);

  /**
   *\par Description:
   *Gets a scalar component of a vector FTensor.
   *\pre
   *the FTensor is of order 1.\\
   *i < dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i: index of the component to get.
   */
  const double& operator()(unsigned char i) const;

  /**
   *\par Description:
   *Gets a scalar component of a vector FTensor.
   *\pre
   *the FTensor is of order 1.\\
   *i < dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i: index of the component to get.
   */
  double& operator()(unsigned char i);

  /**
   *\par Description:
   *Gets a scalar component of a matrix FTensor.
   *\pre
   *the FTensor is of order 2.\\
   *i,j < dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i,j: indices of the component to get.
   */
  const double& operator()(unsigned char i, unsigned char j) const;

  /**
   *\par Description:
   *Gets a scalar component of a matrix FTensor.
   *\pre
   *the FTensor is of order 2.\\
   *i,j < dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i,j: indices of the component to get.
   */
  double& operator()(unsigned char i, unsigned char j);


  /**
   *\par Description:
   *Gets a scalar component of a 3rd-order FTensor.
   *\pre
   *the FTensor is of order 3.\\
   *i,j,k < dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i,j,k: indices of the component to get.
   */
  const double& operator()(unsigned char i, unsigned char j, unsigned char k) const;

  /**
   *\par Description:
   *Gets a scalar component of a 3rd-order FTensor.
   *\pre
   *the FTensor is of order 3.\\
   *i,j,k < dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i,j,k: indices of the component to get.
   */
  double& operator()(unsigned char i, unsigned char j, unsigned char k);

  /**
   *\par Description:
   *Gets a scalar component of a 4th-order FTensor.
   *\pre
   *the FTensor is of order 4.\\
   *i,j,k,l < dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i,j,k,l: indices of the component to get.
   */
  const double& operator()(unsigned char i, unsigned char j, unsigned char k, unsigned char l) const;

  /**
   *\par Description:
   *Gets a scalar component of a 4th-order FTensor.
   *\pre
   *the FTensor is of order 4.\\
   *i,j,k,l < dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i,j,k,l: indices of the component to get.
   */
  double& operator()(unsigned char i, unsigned char j, unsigned char k, unsigned char l);

  /**
   *\par Description:
   *Gets a reference of a subtensor T(*,*,k) with an order one smaller
   *\pre
   *the FTensor is of order >0.\\
   *i<dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *k: index of the component to get.
   */
  const FRefTensor operator [] (unsigned char i) const;

  /**
   * access the i-th component of this tensor
   */
  double& getComp(unsigned int i);
  const double& getComp(unsigned int i) const;

  /**
   *\par Description:
   *Gets a subtensor of order order-1
   *\pre
   *the FTensor is of order >0.\\
   *i<dimension
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *k: index of the component to get.
   */
  FRefTensor operator [] (unsigned char i);



/**
   *\par Description:
   *prints the contents of the position to os
   *\pre
   *none
   *\post
   *none
   *\exception
   *none
   *\param tensor tensor to print.
   */
  friend std::ostream& operator<< (std::ostream &os, const FTensor &tensor);

  /**
   *\par Description:
   *Gets all the scalar components of the FTensor.
   *\pre
   *none
   *\post
   *comp[k*dimension^2+j*dimension+i] = Tensor(i,j,k), for all i, j, k
   *\exception
   *none
   *\param
   *comp: double array to get all the components of the tensor.
   */
  void getValues(std::vector<double>& comp) const;

  /**
   *\par Description:
   *Sets the scalar component of a scalar FTensor.
   *\pre
   *the FTensor is of order 0, dimension 1.
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FEmptyObjectException
   *\param
   *val: scalar value to set.
   */
  void setValue(double val);

  /**
   *\par Description:
   *Sets a scalar component of a vector FTensor.
   *\pre
   *the FTensor is of order 1
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i: index of the component to get.
   *\param
   *val: scalar value to set.
   */
  void setValue(unsigned char i, double val);

  /**
   *\par Description:
   *Sets a scalar component of a matrix FTensor.
   *\pre
   *the FTensor is of order 2
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i,j: indices of the component to get.
   *\param
   *val: scalar value to set.
   */
  void setValue(unsigned char i, unsigned char j, double val);

  /**
   *\par Description:
   *Sets a scalar component of a 3d-order FTensor.
   *\pre
   *the FTensor is of order 3
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FInvalidIteratorException
   *\exception
   *FEmptyObjectException
   *\param
   *i,j,k: indices of the component to get.
   *\param
   *val: scalar value to set.
   */
  void setValue(unsigned char i, unsigned char j, unsigned char k, double val);

  /**
   *\par Description:
   *Sets all the scalar components of the FTensor.
   *\pre
   *\b comp corresponds to the order and dimension of the tensor.
   *\post
   *none
   *\exception
   *FInvalidDimensionException
   *\exception
   *FEmptyObjectException
   *\param
   *comp: double vector to set all the components of the tensor.
   */
  void setValues(const std::vector<double>& comp);

  /**
   *\par Description:
   *Sets all the scalar components of the FTensor. Only use this function
   *if it helps removing useless copy operations and you are sure of the
   *number of components present in comp array
   *\pre
   *\b comp corresponds to the order and dimension of the tensor.
   *\post
   *none
   *\exception
   *FEmptyObjectException
   *\param
   *comp: double vector to set all the components of the tensor.
   */
  void setValues(const double* comp);

 /**
   *\par Description:
   * Householder reduction of a real symmetric matrix to a tridiagonal one.
   *\pre
   * FTensor is of order 2.
   *\post
   * Matrix tensor is now tridiagonal matrix.
   *\exception
   * FInvalidDimensionException
   *\param
   * d Returns diagonal elements of the tridiagonal matrix.
   *\param
   * e Returns the off-diagonal elements with e[0]=0.
   */
  void tridiagonalize(F::FVector& d, F::FVector& e);

  /**
   *\par Description:
   * Tridiagonalized QL algorithm with implicit shifts.
   * This algorithm determines the eigenvectors and -values
   * of a tridiagonalized FTensor.
   *\pre
   * FTensor is of order 2 and ::tridiagonalize() has been called.
   *\post
   * Matrix tensor now contains eigenvectors (as columns).
   *\exception
   * FInvalidDimensionException
   *\param
   * d Diagonal elements of the tridiagonal matrix.
   *\param
   * e Off-diagonal elements with e[0] arbitrary..
   */
  void tQLiAlgorithm(F::FVector& d, F::FVector& e);

  /**
   *\par Description:
   * Calculate Eigenvalues and Eigenvectors of tensor.
   *\pre
   * FTensor is real, symmetric and of order 2.
   *\post
   * Tensor now contains eigenvectors (as columns).
   *\param
   * vals F::FVector to store Eigenvalues in.
   *\param
   * v1 F::FVector[3] array to store Eigenvectors in.
   */
  void getEigenSystem(F::FVector& vals, F::FVector v[3]);

  /**
   *\par Description:
   * Calculate Eigenvalues and Eigenvectors of tensor.
   *\pre
   * FTensor has dimension 3, is real, symmetric, positive definite (?) and of order 2.
   *\param
   * vals F::FVector of dimension 3 to store Eigenvalues in.
   *\param
   * v1 F::FVector[3] array to store Eigenvectors in.
   */
  void getEigenSystem3DS(F::FVector& vals, F::FVector v[3]) const;

  /**
   * get the non-normalized eigenvector fitting to eigenvalue lambda
   */
  void getEigenvector3DS(F::FVector &ev, const double lambda ) const;

  /**
   * get the sorted eigenvalues of a 3D symmetric tensor
   * \pre evals has size 3
   * \pre tensor is symmetric
   * \param sorted eigenvalues
   */
  void getEigenvalues3DS(F::FVector &evals ) const;

  /// Undocumented
  static int pow(int a, int b);

  /**
   * Computes the frobenius nurm || this || = sqrt(<A,A>)
   */
  //double norm(void) const; // implemented in FArray

  /// Undocumented.
  friend class FRefTensor;
  friend double operator*(const FTensor& A, const FTensor& B);

  /**
   *\par Description:
   * Sort Eigenvectors and values returned from getEigenSystem(...)
   *\pre
   * vals.size() == 2 || vals.size() == 3
   * vecs is an array of the same amount of F::FVectors
   *\post
   * vals and vecs are sorted by the size of vals. vals[0] is the major eigenvalue, vecs[0] the major eigenvector.
   */
  static void sortEigenvectors(F::FVector &vals, F::FVector *vecs);

  static void getEigenvalueIndex(unsigned int *index, const F::FVector &vals);

  /**
   *\par Description:
   * Return the deviator part of the input tensor
   *\pre
   * FTensor is real and symmetric.
   *\post
   * deviator has been returned
   *\param
   *tensor input tensor
   */
  friend FTensor deviator(const FTensor& tensor);
  FTensor& makeDeviator();

  double trace() const;

  friend FTensor contract(const FTensor& tensor, int index1, int index2);
  friend FTensor tensorProduct(const FTensor& tensor,const FTensor &rhs);
  friend FTensor dyadicProduct( const F::FVector& v1, const F::FVector& v2);
  FTensor& multAndContract( const F::FVector& v1, unsigned int index );

  static FTensor tensorFromEigensystem( const FArray& vals, const F::FVector vecs[3] );

  /**
   * Transforms the tensor from the current coordinate system into a new
   * one where the mapping is defined by the matrix A.
   *
   * \pre A has to be square and same dimension as the tensor
   * \post tensor is a rotatied version of the original tensor
   */
  FTensor& transform( const FMatrix& A );

  /**
   * computes the inner product <t1,t2> of two tensors
   */
  friend double innerProduct(const FTensor& t1, const FTensor& t2);

protected:
  // Dimension of the FTensor.
  unsigned char dimension;

  // Order of the FTensor.
  unsigned char order;

private:
  //to prevent invocation of resize of superclass
  void resize(unsigned int s, bool keepContent=false);


  friend std::ostream & binwrite( std::ostream & out, const FTensor& t );
  friend std::istream & binread( std::istream & in, FTensor& t );
};



#ifndef OUTLINE
#include "FTensor.icc"
#endif

#endif // __FTensor_hh

//===========================================================================

