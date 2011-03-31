//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FRefArray.hh,v $
// Language:  C++
// Date:      $Date: 2003/04/22 16:13:44 $
// Author:    $Author: garth $
// Version:   $Revision: 1.1 $
//
//---------------------------------------------------------------------------

#ifndef __FRefArray_hh
#define __FRefArray_hh

#include "FArray.h"
/**
 *provides a reference on a FPosition
 */
class FRefArray:public FArray
{
public:
  /**
   *Default constructor
   */
  FRefArray();

  /**
   *Copy constructor
   *doesn't copy the components of the Tensor,
   *but only the pointer on them
   */
  FRefArray(FArray&);


  /**
   * copies the scalar components of the array x
   * \param x
   * array that is copied
   */
  FArray&operator=(const FArray&x);

  /**
   *Constructor that initializes the Fields directly
   *\param Dimension of the referenced tensor
   *\param Comp
   *pointer on the double array
   *holding the Components of the referenced Tensor
   */
  FRefArray(const int Dimension, double*Comp=0);

  /**
   *sets the pointer on a new set in FRefArray
   *\param Comp: pointer on a valid array of doubles(or NIL)
   *\pre
   *Comp has at least Dimension entries
   *\post
   *none
   *\exception
   *none
   */
  void setCompPointer( double* Comp );
  /**
   *sets dimension of Position
   */
  void setDimension( const int inDim );
  /**
   *Destructor
   */
  ~FRefArray();
};



//===========================================================================
#ifndef OUTLINE
#include "FRefArray.icc"
#endif
//===========================================================================

#endif // __FRefArray_hh
