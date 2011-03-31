/////////////////////////////////////////////////////////////////////////////
// Name:            BSpline.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   09/28/2009
//
// Description: BSpline class.
//
// Last modifications:
//      by : ----
/////////////////////////////////////////////////////////////////////////////

#ifndef BSPLINE_H_
#define BSPLINE_H_

#include <vector>

#include "../Fantom/FMatrix.h"
#include "../IsoSurface/Vector.h"

/****************************************************************************/
// Description : This class is a B-Spline implementation.
// The Equation used to calculate everything is this P(u) = U^T*M*B 
// This equation is to find the value of the BSpline at a certain progression "u" on the function
// M : represent a specific function depending of the type of interpolation we wat to use ( 4 points or 5 points ) 
// B : is a matrix made with the points used to calculate the curve
// For more information see this page http://en.wikipedia.org/wiki/B-spline  ( Uniform cubic B-splines ) 
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////////
// This is only to enumerate the different type of interpolation that are currently implemented for this B-spline
//////////////////////////////////////////////////////////////////////////////////
enum BSplineType 
{
	INTERPOLATION_ON_4_POINTS = 0,
	INTERPOLATION_ON_5_POINTS = 1,
};

class BSpline
{
public :

    // Constructor / Destructor
    BSpline  ( BSplineType i_BSplineType );
    ~BSpline ( void ) {};

    // Functions
    bool getDerivativeOrder0( double i_u, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, Vector &o_result );
    bool getDerivativeOrder0( double i_u, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, const Vector &i_P4, Vector &o_result );

    bool getDerivativeOrder1( double i_u, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, Vector &o_result );
    bool getDerivativeOrder1( double i_u, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, const Vector &i_P4, Vector &o_result );

    bool getDerivativeOrder2( double i_u, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, Vector &o_result );
    bool getDerivativeOrder2( double i_u, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, const Vector &i_P4, Vector &o_result );

    bool getDerivativeOrder3( double i_u, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, Vector &o_result );
    bool getDerivativeOrder3( double i_u, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, const Vector &i_P4, Vector &o_result );

private :

    // Variables
    FMatrix         m_M;
    BSplineType     m_BSplineType;

    // Functions
    void calculateResult( const FMatrix i_U, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, Vector &o_result );
    void calculateResult( const FMatrix i_U, const Vector &i_P0, const Vector &i_P1, const Vector &i_P2, const Vector &i_P3, const Vector &i_P4, Vector &o_result );
    void validateProgression( double &i_progression );
};

#endif // BSPLINE_H_