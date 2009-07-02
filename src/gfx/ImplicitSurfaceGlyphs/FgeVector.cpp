///////////////////////////////////////////////////////////////////////////////
//                                                                           
///// /////  ////       Project  :   FAnToM                    
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: FgeVector.cc,v $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2007-08-22 16:14:48 +0200 (Wed, 22 Aug 2007) $
         //             Author   :   $Author: hlawit $
//////////              Revision :   $Revision: 8272 $ 



#include "FgeVector.h"
#include <iostream>


FgeVector::FgeVector() {
}


FgeVector::FgeVector(GLdouble x, GLdouble y, GLdouble z) {
  set(x, y, z);
}

std::ostream& operator<<( std::ostream& o, const FgeVector& v )
{
  o << "[ " << v[ 0 ] << "," << v[ 1 ] << "," << v[ 2 ] << " ]";
  return o;
}

// Well, it would seem the whole rest is implemented inline...




