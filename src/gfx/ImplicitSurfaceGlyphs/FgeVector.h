///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: FgeVector.hh,v $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2007-08-22 16:14:48 +0200 (Wed, 22 Aug 2007) $
         //             Author   :   $Author: hlawit $
//////////              Revision :   $Revision: 8272 $



#ifndef __FGE_VECTOR_HH__
#define __FGE_VECTOR_HH__



#include "FgeGL.h"
#include <iosfwd>


/** This class implements a 3-dimensional light-weight vector (yup, yet another
 vector class) using GLdouble for its elements.

    \ingroup Fge

    It was introduced quite late intended to (someday) replace certain
    ugly-looking and redundant code for element-wise vector operations (for
    example in the mouse mapper and zoom mode code).
*/
class FgeVector {
 private:
  GLdouble data[3];

 public:
  //! This constructor does not do any initialization.
  FgeVector();
  //! Use this constructor to also set the vector.
  FgeVector(GLdouble x, GLdouble y, GLdouble z);

  //! I can't convince myself to lose words on this...
  inline void set(GLdouble elem_x, GLdouble elem_y, GLdouble elem_z);
  //! ... same here...
  inline GLdouble getX(void) const;
  //! ... and here ...
  inline GLdouble getY(void) const;
  //! ... and even here.
  inline GLdouble getZ(void) const;

  // these are easier to use in loops than the one above
  inline GLdouble& operator[](unsigned int i){ return data[i]; }
  inline GLdouble  operator[](unsigned int i)const{ return data[i]; }

  GLdouble* values()  {
    return &data[0];
  }

  const GLdouble*const values() const {
    return &data[0];
  }

  inline FgeVector operator-(void) const;
  inline void operator+=(const FgeVector& vector);
  inline FgeVector operator+(const FgeVector& vector) const;
  inline void operator-=(const FgeVector& vector);
  inline FgeVector operator-(const FgeVector& vector) const;
  inline void operator*=(GLdouble s);
  inline FgeVector operator*(GLdouble s) const;

  //! This operator implements the scalar product
  inline GLdouble operator*(const FgeVector& vector) const;
  //! This operator implements the cross-product.
  inline FgeVector operator^(const FgeVector& vector) const;

  //! This returns the vector's length.
  inline GLdouble len(void) const;
  //! This returns the vector's squared length.
  inline GLdouble lenSquare(void) const;
  //! Call this to normalize your vector.
      /*!
          \exception FgeDivisionByZeroException
          Thrown if the vector is 0. The vector remains unchanged.
      */
  inline void normalize(void);

  //! This method returns an arbitrary normal to the vector. It will be 
  //! normalized.
      /*!
          \exception FgeDivisionByZeroException
          Thrown if a division by zero is encountered along the way.
          This should not happen if the vector is of "reasonable" length.
      */
  inline FgeVector findNormal(void) const;

  friend std::ostream& operator<<( std::ostream&, const FgeVector& );
};



#include "FgeVector.icc"



#endif   // __FGE_VECTOR_HH__
