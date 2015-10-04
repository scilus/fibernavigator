/////////////////////////////////////////////////////////////////////////////
// Name:            BSpline.cpp
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   09/28/2009
//
// Description: This is the implementation file for BSplines class.
//
// Last modifications:
//      by : ----
/////////////////////////////////////////////////////////////////////////////

#include "BSpline.h"

//////////////////////////////////////////////////////////////////////////////////
// Constructor : This constructor will set the values of m_M depending on the 
//               i_BSplineType pass in argument.
//
// i_BSplineType        : This is the type of spline we want to build/use.
//////////////////////////////////////////////////////////////////////////////////
BSpline::BSpline( BSplineType i_BSplineType  ) : m_BSplineType ( i_BSplineType )
{    
    switch( m_BSplineType )
    {
        case( INTERPOLATION_ON_4_POINTS ) :
        {
            m_M.resize( 4, 4 );
            // This is the predefined M matrix for the B-Spline with 4 points.
            m_M( 0, 0 ) = -1;   m_M( 0, 1 ) =  3;  m_M( 0, 2 ) = -3;  m_M( 0, 3 ) = 1;
            m_M( 1, 0 ) =  3;   m_M( 1, 1 ) = -6;  m_M( 1, 2 ) =  3;  m_M( 1, 3 ) = 0;
            m_M( 2, 0 ) = -3;   m_M( 2, 1 ) =  0;  m_M( 2, 2 ) =  3;  m_M( 2, 3 ) = 0;
            m_M( 3, 0 ) =  1;   m_M( 3, 1 ) =  4;  m_M( 3, 2 ) =  1;  m_M( 3, 3 ) = 0;

            break;
        }
        case( INTERPOLATION_ON_5_POINTS ) :
        {
            m_M.resize( 5, 5 );
            // This is the predefined M matrix for the B-Spline with 5 points.
            m_M( 0, 0 ) =  1;   m_M( 0, 1 ) =  -4;  m_M( 0, 2 ) =   6;  m_M( 0, 3 ) = -4;   m_M( 0, 4 ) = 1;
            m_M( 1, 0 ) = -4;   m_M( 1, 1 ) =  12;  m_M( 1, 2 ) = -12;  m_M( 1, 3 ) =  4;   m_M( 1, 4 ) = 0;
            m_M( 2, 0 ) =  6;   m_M( 2, 1 ) =  -6;  m_M( 2, 2 ) =  -6;  m_M( 2, 3 ) =  6;   m_M( 2, 4 ) = 0;
            m_M( 3, 0 ) = -4;   m_M( 3, 1 ) = -12;  m_M( 3, 2 ) =  12;  m_M( 3, 3 ) =  4;   m_M( 3, 4 ) = 0;
            m_M( 4, 0 ) =  1;   m_M( 4, 1 ) =  11;  m_M( 4, 2 ) =  11;  m_M( 4, 3 ) =  1;   m_M( 4, 4 ) = 0;

            break;
        }
        default : /* This should never happen */ break;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// This function will calculate the value of a point at the progression passed in 
// argument using the B-Spline algorithm with 4 points.
//
// i_progression                : The value of the progression we want to find a value for.
// i_P0, i_P1, i_P2, i_P3       : The 4 points making the curve.
// o_result                     : The result of the calculation.
//
// Returns true if successful, false otherwise.
//////////////////////////////////////////////////////////////////////////////////
bool BSpline::getDerivativeOrder0(       double  i_progression, 
                                   const Vector &i_P0,
                                   const Vector &i_P1,
                                   const Vector &i_P2,
                                   const Vector &i_P3,
                                         Vector &o_result )
{
    validateProgression( i_progression );

    if( m_BSplineType != INTERPOLATION_ON_4_POINTS )
        return false;

    FMatrix l_U0( 1, 4 );

    l_U0( 0, 0 ) = i_progression * i_progression * i_progression;                   // u^3
    l_U0( 0, 1 ) = i_progression * i_progression;                                   // u^2
    l_U0( 0, 2 ) = i_progression;                                                   // u^1
    l_U0( 0, 3 ) = 1.0f;                                                            // u^0

    calculateResult( l_U0, i_P0, i_P1, i_P2, i_P3, o_result );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// This function will calculate the value of a point at the progression passed in 
// argument using the B-Spline algorithm with 5 points.
//
// i_progression                : The value of the progression we want to find a value for.
// i_P0, i_P1, i_P2, i_P3, i_P4 : The 5 points making the curve.
// o_result                     : The result of the calculation.
//
// Returns true if successful, false otherwise.
//////////////////////////////////////////////////////////////////////////////////
bool BSpline::getDerivativeOrder0(       double  i_progression, 
                                   const Vector &i_P0,
                                   const Vector &i_P1,
                                   const Vector &i_P2,
                                   const Vector &i_P3,
                                   const Vector &i_P4,
                                         Vector &o_result )
{
    validateProgression( i_progression );

    if( m_BSplineType != INTERPOLATION_ON_5_POINTS )
        return false;

    FMatrix l_U0( 1, 5 );

    l_U0( 0, 0 ) = i_progression * i_progression * i_progression * i_progression;   // u^4
    l_U0( 0, 1 ) = i_progression * i_progression * i_progression;                   // u^3
    l_U0( 0, 2 ) = i_progression * i_progression;                                   // u^2
    l_U0( 0, 3 ) = i_progression;                                                   // u^1
    l_U0( 0, 4 ) = 1.0f;                                                            // u^0
    
    calculateResult( l_U0, i_P0, i_P1, i_P2, i_P3, i_P4, o_result );
    
    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// This function will calculate the value of a point first derivative at the 
// progression passed in argument using the B-Spline algorithm with 4 points.
//
// i_progression                : The value of the progression we want to find a value for.
// i_P0, i_P1, i_P2, i_P3       : The 4 points making the curve.
// o_result                     : The result of the calculation.
//
// Returns true if successful, false otherwise.
//////////////////////////////////////////////////////////////////////////////////
bool BSpline::getDerivativeOrder1(       double  i_progression, 
                                   const Vector &i_P0,
                                   const Vector &i_P1,
                                   const Vector &i_P2,
                                   const Vector &i_P3,
                                         Vector &o_result )
{
    validateProgression( i_progression );

    if( m_BSplineType != INTERPOLATION_ON_4_POINTS )
        return false;

    FMatrix l_U1( 1, 4 );

    l_U1( 0, 0 ) = 3.0f * i_progression * i_progression;                    // 3*u^2
    l_U1( 0, 1 ) = 2.0f * i_progression;                                    // 2*u^1
    l_U1( 0, 2 ) = 1.0f;                                                    // 1*u^0
    l_U1( 0, 3 ) = 0.0f;                                                    // 0

    calculateResult( l_U1, i_P0, i_P1, i_P2, i_P3, o_result );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// This function will calculate the value of a point first derivative at the 
// progression passed in argument using the B-Spline algorithm with 5 points.
//
// i_progression                : The value of the progression we want to find a value for.
// i_P0, i_P1, i_P2, i_P3, i_P4 : The 5 points making the curve.
// o_result                     : The result of the calculation.
//
// Returns true if successful, false otherwise.
//////////////////////////////////////////////////////////////////////////////////
bool BSpline::getDerivativeOrder1(       double  i_progression, 
                                   const Vector &i_P0,
                                   const Vector &i_P1,
                                   const Vector &i_P2,
                                   const Vector &i_P3,
                                   const Vector &i_P4,
                                         Vector &o_result )
{
    validateProgression( i_progression );    

    if( m_BSplineType != INTERPOLATION_ON_5_POINTS )
        return false;

    FMatrix l_U1( 1, 5 );

    l_U1( 0, 0 ) = 4.0f * i_progression * i_progression * i_progression;    // 4*u^3
    l_U1( 0, 1 ) = 3.0f * i_progression * i_progression;                    // 3*u^2
    l_U1( 0, 2 ) = 2.0f * i_progression;                                    // 2*u^1
    l_U1( 0, 3 ) = 1.0f;                                                    // 1*u^0
    l_U1( 0, 4 ) = 0.0f;                                                    // 0

    calculateResult( l_U1, i_P0, i_P1, i_P2, i_P3, i_P4, o_result );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// This function will calculate the value of a point second derivative at the 
// progression passed in argument using the B-Spline algorithm with 4 points.
//
// i_progression                : The value of the progression we want to find a value for.
// i_P0, i_P1, i_P2, i_P3       : The 4 points making the curve.
// o_result                     : The result of the calculation.
//
// Returns true if successful, false otherwise.
//////////////////////////////////////////////////////////////////////////////////
bool BSpline::getDerivativeOrder2(       double  i_progression, 
                                   const Vector &i_P0,
                                   const Vector &i_P1,
                                   const Vector &i_P2,
                                   const Vector &i_P3,
                                         Vector &o_result )
{
    validateProgression( i_progression );

    if( m_BSplineType != INTERPOLATION_ON_4_POINTS )
        return false;

    FMatrix l_U2( 1, 4 );

    l_U2( 0, 0 ) = 6.0f * i_progression;                                    // 6*u^1
    l_U2( 0, 1 ) = 2.0f;                                                    // 2*u^0
    l_U2( 0, 2 ) = 0.0f;                                                    // 0
    l_U2( 0, 3 ) = 0.0f;                                                    // 0

    calculateResult( l_U2, i_P0, i_P1, i_P2, i_P3, o_result );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// This function will calculate the value of a point second derivative at the 
// progression passed in argument using the B-Spline algorithm with 5 points.
//
// i_progression                : The value of the progression we want to find a value for.
// i_P0, i_P1, i_P2, i_P3, i_P4 : The 5 points making the curve.
// o_result                     : The result of the calculation.
//
// Returns true if successful, false otherwise.
//////////////////////////////////////////////////////////////////////////////////
bool BSpline::getDerivativeOrder2(       double  i_progression, 
                                   const Vector &i_P0,
                                   const Vector &i_P1,
                                   const Vector &i_P2,
                                   const Vector &i_P3,
                                   const Vector &i_P4,
                                         Vector &o_result )
{
    validateProgression( i_progression );

    if( m_BSplineType != INTERPOLATION_ON_5_POINTS )
        return false;

    FMatrix l_U2( 1, 5 );

    l_U2( 0, 0 ) = 12.0f * i_progression * i_progression;                   // 12*u^2
    l_U2( 0, 1 ) = 6.0f * i_progression;                                    // 6*u^1
    l_U2( 0, 2 ) = 2.0f;                                                    // 2*u^0
    l_U2( 0, 3 ) = 0.0f;                                                    // 0
    l_U2( 0, 4 ) = 0.0f;                                                    // 0

    calculateResult( l_U2, i_P0, i_P1, i_P2, i_P3, i_P4, o_result );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// This function will calculate the value of a point third derivative at the 
// progression passed in argument using the B-Spline algorithm with 4 points.
//
// i_progression                : The value of the progression we want to find a value for.
// i_P0, i_P1, i_P2, i_P3       : The 4 points making the curve.
// o_result                     : The result of the calculation.
//
// Returns true if successful, false otherwise.
//////////////////////////////////////////////////////////////////////////////////
bool BSpline::getDerivativeOrder3(       double  i_progression,
                                   const Vector &i_P0,
                                   const Vector &i_P1,
                                   const Vector &i_P2,
                                   const Vector &i_P3,
                                         Vector &o_result )
{
    validateProgression( i_progression );

    if( m_BSplineType != INTERPOLATION_ON_4_POINTS )
        return false;

    FMatrix l_U3( 1, 4 );

    l_U3( 0, 0 ) = 6.0f;                                                    // 6*u^0
    l_U3( 0, 1 ) = 0.0f;                                                    // 0
    l_U3( 0, 2 ) = 0.0f;                                                    // 0
    l_U3( 0, 3 ) = 0.0f;                                                    // 0

    calculateResult( l_U3, i_P0, i_P1, i_P2, i_P3, o_result );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// This function will calculate the value of a point third derivative at the 
// progression passed in argument using the B-Spline algorithm with 5 points.
//
// i_progression                : The value of the progression we want to find a value for.
// i_P0, i_P1, i_P2, i_P3, i_P4 : The 5 points making the curve.
// o_result                     : The result of the calculation.
//
// Returns true if successful, false otherwise.
//////////////////////////////////////////////////////////////////////////////////
bool BSpline::getDerivativeOrder3(       double  i_progression,
                                   const Vector &i_P0,
                                   const Vector &i_P1,
                                   const Vector &i_P2,
                                   const Vector &i_P3,
                                   const Vector &i_P4,
                                         Vector &o_result )
{
    validateProgression( i_progression );

    if( m_BSplineType != INTERPOLATION_ON_5_POINTS )
        return false;

    FMatrix l_U3( 1, 5 );

    l_U3( 0, 0 ) = 24.0f * i_progression;                                   // 24*u^1
    l_U3( 0, 1 ) = 6.0f;                                                    // 6*u^0
    l_U3( 0, 2 ) = 0.0f;                                                    // 0
    l_U3( 0, 3 ) = 0.0f;                                                    // 0
    l_U3( 0, 4 ) = 0.0f;                                                    // 0

    calculateResult( l_U3, i_P0, i_P1, i_P2, i_P3, i_P4, o_result );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// Using the U matrix provided in arguments, this function will calculate the result
// using the B-Spline algorithm with 4 points.
//
// i_U                          : The precalculated U matrix
// i_P0, i_P1, i_P2, i_P3       : The 4 points making the curve.
// o_result                     : The result of the calculation.
//
//////////////////////////////////////////////////////////////////////////////////
void BSpline::calculateResult( const FMatrix  i_U,
                               const Vector  &i_P0,
                               const Vector  &i_P1,
                               const Vector  &i_P2,
                               const Vector  &i_P3,
                                     Vector  &o_result )
{
    FMatrix l_UM = ( ( 1.0f / 6.0f ) * i_U * m_M );

    o_result = (float)l_UM( 0 , 0 ) * i_P0 + 
               (float)l_UM( 0 , 1 ) * i_P1 + 
               (float)l_UM( 0 , 2 ) * i_P2 +
               (float)l_UM( 0 , 3 ) * i_P3;
}

//////////////////////////////////////////////////////////////////////////////////
// Using the U matrix provided in arguments, this function will calculate the result
// using the B-Spline algorithm with 5 points.
//
// i_U                          : The precalculated U matrix
// i_P0, i_P1, i_P2, i_P3, i_P4 : The 5 points making the curve.
// o_result                     : The result of the calculation.
//
//////////////////////////////////////////////////////////////////////////////////
void BSpline::calculateResult( const FMatrix  i_U,
                               const Vector  &i_P0,
                               const Vector  &i_P1,
                               const Vector  &i_P2,
                               const Vector  &i_P3,
                               const Vector  &i_P4,
                                     Vector  &o_result )
{
    FMatrix l_UM = ( ( 1.0f / 24.0f ) * i_U * m_M );

    o_result = (float)l_UM( 0 , 0 ) * i_P0 + 
               (float)l_UM( 0 , 1 ) * i_P1 + 
               (float)l_UM( 0 , 2 ) * i_P2 +
               (float)l_UM( 0 , 3 ) * i_P3 +
               (float)l_UM( 0 , 4 ) * i_P4;
}

//////////////////////////////////////////////////////////////////////////////////
// If the progression is not [0,1], then we make it this way.
//
// i_progression        : The progression that will be validated.
//////////////////////////////////////////////////////////////////////////////////
void BSpline::validateProgression( double &i_progression )
{
    if( i_progression < 0.0f )
        i_progression = 0.0f;
    else if( i_progression > 1.0f )
        i_progression = 1.0f;
}