/////////////////////////////////////////////////////////////////////////////
// Name:            Helper.cpp
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   11/14/2009
//
// Description: This is the implementation file for the Helper class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#include "Helper.h"

#include "misc/Algorithms/BSpline.h"

#include <math.h>
#include <cassert>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

wxString Helper::getColorationModeString( const FibersColorationMode &colMode )
{
    wxString modeStr;
    
    switch(colMode)
    {
        case NORMAL_COLOR:
            modeStr = "normal";
            break;
        case CURVATURE_COLOR:
            modeStr = "curvature";
            break;
        case TORSION_COLOR:
            modeStr = "torsion";
            break;
        case DISTANCE_COLOR:
            modeStr = "distance";
            break;
        case MINDISTANCE_COLOR:
            modeStr = "mindistance";
            break;
        case CUSTOM_COLOR:
            modeStr = "custom";
            break;
        case CONSTANT_COLOR:
            modeStr = "constant";
            break;
    }
    
    return modeStr;
}

FibersColorationMode Helper::getColorationModeFromString( const wxString &modeStr )
{
    FibersColorationMode curMode;
    
    if( modeStr == wxT("normal") )
        curMode = NORMAL_COLOR;
    else if( modeStr == wxT("curvature") )
        curMode = CURVATURE_COLOR;
    else if( modeStr == wxT("torsion") )
        curMode = TORSION_COLOR;
    else if( modeStr == wxT("distance") )
        curMode = DISTANCE_COLOR;
    else if( modeStr == wxT("mindistance") )
        curMode = MINDISTANCE_COLOR;
    else if( modeStr == wxT("custom") )
        curMode = CUSTOM_COLOR;
    else if( modeStr == wxT("constant") )
        curMode = CONSTANT_COLOR;
    
    return curMode;
}

///////////////////////////////////////////////////////////////////////////
// Simple function to convert a HSL color system to a RGB color system.
//
// i_hue            : The provided hue value must be between [0,1[.
// i_saturation     : The provided saturation value must be between [0,1].
// i_luminance      : The provided luminance value must be between [0,1].
// o_r              : The resulting red value that will between [0,1].
// o_g              : The resulting green value that will between [0,1].
// o_b              : The resulting blue value that will between [0,1].
//
// The formulation for this conversation was taken here : http://en.wikipedia.org/wiki/HSL_and_HSV
///////////////////////////////////////////////////////////////////////////
void Helper::HSLtoRGB( float i_hue, float i_saturation, float i_luminance, float &o_r, float &o_g, float &o_b )
{
    float q, p, tr, tg, tb;

    // Remapping the values if they are not in their good range.
    if(      i_hue        <  0.0f ) i_hue        = 0.0f;
    else if( i_hue        >= 1.0f ) i_hue        = 0.9999f;
    if(      i_saturation <  0.0f ) i_saturation = 0.0f;
    else if( i_saturation >  1.0f ) i_saturation = 1.0f;
    if(      i_luminance  <  0.0f ) i_luminance  = 0.0f;
    else if( i_luminance  >  1.0f ) i_luminance  = 1.0f;

    if( i_luminance < 0.5f )
        q = i_luminance * ( 1.0f + i_saturation );
    else 
        q = i_luminance + i_saturation - ( i_luminance * i_saturation );

    p = 2.0f * i_luminance - q;
    
    tr = i_hue + ( 1.0f / 3.0f );
    tg = i_hue;
    tb = i_hue - ( 1.0f / 3.0f );

    if(      tr < 0.0f ) tr += 1.0f;
    else if( tr > 1.0f ) tr -= 1.0f;
    if(      tg < 0.0f ) tg += 1.0f;
    else if( tg > 1.0f ) tg -= 1.0f;
    if(      tb < 0.0f ) tb += 1.0f;
    else if( tb > 1.0f ) tb -= 1.0f;

    if(      tr < ( 1.0f / 6.0f ) ) o_r = p + ( ( q - p ) * 6.0f * tr );
    else if( tr < ( 1.0f / 2.0f ) ) o_r = q;
    else if( tr < ( 2.0f / 3.0f ) ) o_r = p + ( ( q - p ) * 6 * ( ( 2.0f / 3.0f ) - tr ) );
    else                            o_r = p;

    if(      tg < ( 1.0f / 6.0f ) ) o_g = p + ( ( q - p ) * 6.0f * tg );
    else if( tg < ( 1.0f / 2.0f ) ) o_g = q;
    else if( tg < ( 2.0f / 3.0f ) ) o_g = p + ( ( q - p ) * 6 * ( ( 2.0f / 3.0f ) - tg ) );
    else                            o_g = p;

    if(      tb < ( 1.0f / 6.0f ) ) o_b = p + ( ( q - p ) * 6.0f * tb );
    else if( tb < ( 1.0f / 2.0f ) ) o_b = q;
    else if( tb < ( 2.0f / 3.0f ) ) o_b = p + ( ( q - p ) * 6 * ( ( 2.0f / 3.0f ) - tb ) );
    else                            o_b = p;
}

/////////////////////////////////////////////////////////////////////////// 
// Converts cartesian coords to spherical coords
//
// i_catesianDir  : cartesian coords
// o_sphericalDir : spherical coords
///////////////////////////////////////////////////////////////////////////
void Helper::cartesianToSpherical( float i_catesianDir[3], float o_sphericalDir[3] ) 
{
    float l_radius, l_phi, l_theta;
    l_radius = l_phi = l_theta = 0.0f;

    l_radius = sqrt( i_catesianDir[0] * i_catesianDir[0] + 
                     i_catesianDir[1] * i_catesianDir[1] + 
                     i_catesianDir[2] * i_catesianDir[2] );
    
      
    if( l_radius == 0 ) 
        return;    

    // Theta computation  
    l_theta = acos( i_catesianDir[2] / l_radius );  /*this value is between 0 and PI*/

    // Phi computation :
    // atan2 is the same as atan(y/x) but looks at the sign of x and y
    // to determine the quadrant of the answer. It returns a value between
    // -PI and PI.
    l_phi = atan2( i_catesianDir[1], i_catesianDir[0] );  /*this value is between 0 and PI*/

    o_sphericalDir[0] = l_radius;
    o_sphericalDir[1] = l_phi;
    o_sphericalDir[2] = l_theta;
}

/////////////////////////////////////////////////////////////////////////// 
// Computes and returns the factorial value of n
//
// i_n : n
///////////////////////////////////////////////////////////////////////////
float Helper::getFactorial( int i_n ) 
{
  float l_result = 1.0f;

  for(int i = 2; i <= i_n; ++i )
    l_result *= i;

  return l_result;
}

/////////////////////////////////////////////////////////////////////////// 
// Computes a Legendre polynomial of the m order for x
//
// i_x : Value with wich the polynomial will be evaluated
// i_m : Polynomial's order
//
// Returns the polynomial's value for x
///////////////////////////////////////////////////////////////////////////
double Helper::getLegendrePlm( int i_m, double i_x )
{
    // Calculate P_m^i_m(i_x) from the analytic result:
    //  P_m^i_m(i_x) = (-1)^i_m (2m-1)!! (1-i_x^2)^(i_m/2) , i_m > 0
    //           = 1 , i_m = 0
    if( i_m == 0 )
    {
        return 1.0;
    }
    else
    {
        double p_mm = 1.0;
        double root_factor = sqrt(1.0-i_x)*sqrt(1.0+i_x);
        double fact_coeff = 1.0;
        int i = 0;

        for(i=1; i<=i_m; i++)
        {
            p_mm *= -fact_coeff * root_factor;
            fact_coeff += 2.0;
        }

        return p_mm;
    }
}

/////////////////////////////////////////////////////////////////////////// 
// Computes the Associated Legendre Polynomial
//
// i_l      : Polynomial's degree
// i_m      : Polynomial's order
// i_x      : Value with wich the polynomial will be evaluated
// o_value  : evaluated polynomial
///////////////////////////////////////////////////////////////////////////
void Helper::getAssociatedLegendrePlm( const int i_l, const int i_m, const double i_x, double &o_value )
{
    //double dbl_epsilon =  2.2204460492503131e-16;
    double log_dbl_min = -7.0839641853226408e+02;

    //If i_l is large and i_m is large, then we have to worry
    //about overflow. Calculate an approximate exponent which
    //measures the normalization of this thing.
    
    const double dif = i_l-i_m;
    const double sum = i_l+i_m;
    const double t_d = ( dif == 0.0 ? 0.0 : 0.5 * dif * ( log( dif ) - 1.0 ) );
    const double t_s = ( dif == 0.0 ? 0.0 : 0.5 * sum * ( log( sum ) - 1.0 ) );
    const double exp_check = 0.5 * log( 2.0 * i_l + 1.0 ) + t_d - t_s;

    if( i_m < 0 || i_l < i_m || i_x < -1.0 || i_x > 1.0 )
        return; // The domain is wrong!

    else if(exp_check < log_dbl_min + 10.0)
        return; // Overflow error!

    else 
    {
        //Account for the error due to the
        //representation of 1-i_x.
        
        //const double err_amp = 1.0 / (dbl_epsilon + fabs( 1.0 - fabs( i_x ) ) );

        // P_m^i_m(i_x) and P_{i_m+1}^i_m(i_x) 
        double p_mm   = getLegendrePlm( i_m, i_x );
        double p_mmp1 = i_x * ( 2 * i_m + 1 ) * p_mm;

        if(i_l == i_m)
        {
            o_value = p_mm;
            return;
        }
        else if(i_l == i_m + 1) 
        {
            o_value = p_mmp1;
            return;
        }
        else
        {
            //upward recurrence: (i_l-i_m) P(i_l,i_m) = (2l-1) z P(i_l-1,i_m) - (i_l+i_m-1) P(i_l-2,i_m)
            //start at P(i_m,i_m), P(i_m+1,i_m)            

            double p_ellm2 = p_mm;
            double p_ellm1 = p_mmp1;
            double p_ell   = 0.0;
            int ell = 0;

            for( ell = i_m + 2; ell <= i_l; ell++ )
            {
                p_ell   = ( i_x * ( 2 * ell - 1 ) * p_ellm1 - ( ell + i_m - 1 ) * p_ellm2 ) / ( ell - i_m );
                p_ellm2 = p_ellm1;
                p_ellm1 = p_ell;
            }

            o_value = p_ell;
        }
    }
}

/////////////////////////////////////////////////////////////////////////// 
// Computes and returns the fractional anisotropie from a set of 3 eigen values.
//
// i_eigenValue1      : The first eigen value used to calculate the FA.
// i_eigenValue2      : The second eigen value used to calculate the FA.
// i_eigenValue3      : The third eigen value used to calculate the FA.
//
// The formula for the fractional anisotropie was taken here : 
// http://en.wikipedia.org/wiki/Fractional_anisotropy
///////////////////////////////////////////////////////////////////////////
double Helper::getFAFromEigenValues( double i_eigenValue1, double i_eigenValue2, double i_eigenValue3 )
{
    double l_trace = ( i_eigenValue1 + i_eigenValue2 + i_eigenValue3 ) / (double)3.0f;

    double l_numerator = pow( pow( i_eigenValue1 - l_trace, (double)2.0f ) +
                              pow( i_eigenValue2 - l_trace, (double)2.0f ) +
                              pow( i_eigenValue3 - l_trace, (double)2.0f ), (double)0.5f ); 

    double l_denominator = pow( pow( i_eigenValue1, (double)2.0f ) +
                                pow( i_eigenValue2, (double)2.0f ) +
                                pow( i_eigenValue3, (double)2.0f ), (double)0.5f );
    if (l_denominator == 0)
        return -1.0f;
    else
        return pow( 3.0f / 2.0f, 0.5f ) * l_numerator / l_denominator;
}

///////////////////////////////////////////////////////////////////////////
// This function will fill up o_spherePoints with the calculated points of 
// a sphere with i_latsFullSphere laterals and i_longsFullSphere longitudinal.
// When i_latsThisSphere or i_longsThisSphere are not equal to i_latsFullSphere 
// and i_longsFullSphere, this means that we will only generate the points for 
// a portion of a sphere.
//
// i_latsFullSphere         : The amount of laterals the full sphere would have.
// i_longsFullSphere        : The amount of longitudinal the full sphere would have.
// i_latsThisSphere         : The amount of laterals this sphere will have.
// i_longsThisSphere        : The amount of longitudinal this sphere will have.
// o_spherePoints           : The vector that will countain the calculated points.
///////////////////////////////////////////////////////////////////////////
void Helper::createSphere( int                   i_latsFullSphere, 
                           int                   i_longsFullSphere,
                           int                   i_latsThisSphere,
                           int                   i_longsThisSphere,
                           std::vector< float > &o_spherePoints )
{   
    for( int i = 0; i <= i_latsThisSphere; ++i ) 
    {
        double lat0 = M_PI * ( -0.5f + (double)( i - 1 ) / i_latsFullSphere );
        double z0   = sin( lat0 );
        double zr0  = cos( lat0 );

        double lat1 = M_PI * ( -0.5f + (double)i / i_latsFullSphere );
        double z1   = sin( lat1 );
        double zr1  = cos( lat1 );
        
        for( int j = 0; j <= i_longsThisSphere; ++j ) 
        {
            double lng = 2 * M_PI * (double)( j - 1 ) / i_longsFullSphere;
            double x = cos( lng );
            double y = sin( lng );

            // The first point.
            o_spherePoints.push_back( (float)(x * zr0) );
            o_spherePoints.push_back( (float)(y * zr0) );
            o_spherePoints.push_back( (float)z0        );

            // The second point.
            o_spherePoints.push_back( (float)(x * zr1) );
            o_spherePoints.push_back( (float)(y * zr1) );
            o_spherePoints.push_back( (float)z1        );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Computes the torsion for a given set of 5 points.
//
// The formula for the torsion calculation was taken here : http://en.wikipedia.org/wiki/Torsion_of_a_curve
//
// i_point0                 : The first point of the set.
// i_point1                 : The second point of the set.
// i_point2                 : The third point of the set.
// i_point3                 : The fourth point of the set.
// i_point4                 : The fifth point of the set.
// i_progression            : The progression on the spline we want to have the curvature and torsion calculated for.
// o_torsion                : The calculated torsion.
///////////////////////////////////////////////////////////////////////////
void Helper::getProgressionTorsion( const Vector &i_point0, 
                                    const Vector &i_point1, 
                                    const Vector &i_point2, 
                                    const Vector &i_point3, 
                                    const Vector &i_point4,
                                    double i_progression,
                                    double &o_torsion )
{
    // We have to use 5 points for the BSpline because the torsion required derivative or the third order.
    BSpline l_BSpline( INTERPOLATION_ON_5_POINTS );
    Vector deriv1, deriv2, deriv3;
    
    l_BSpline.getDerivativeOrder1( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, deriv1 );
    l_BSpline.getDerivativeOrder2( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, deriv2 );
    l_BSpline.getDerivativeOrder3( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, deriv3  );
        
    double denominator = ( deriv1.x * deriv1.x + deriv1.y * deriv1.y + deriv1.z * deriv1.z ) * ( deriv2.x * deriv2.x + deriv2.y * deriv2.y + deriv2.z * deriv2.z );
    
    if( fabs( denominator ) < EPSILON )
    {
        o_torsion = 0.0f;
        return;
    }
    
    double numerator = ( deriv3.z * ( deriv1.x * deriv2.y - deriv1.y * deriv2.x ) ) +
    ( deriv2.z * ( deriv3.x * deriv1.y - deriv1.x * deriv3.y ) ) +
    ( deriv1.z * ( deriv2.x * deriv3.y - deriv3.x * deriv2.y ) );
    
    o_torsion = numerator / denominator;
}

///////////////////////////////////////////////////////////////////////////
// Computes the curvature for a given set of 5 points. 
//
// The formula for the curvature calculation was taken here : http://en.wikipedia.org/wiki/Curvature
//
// i_point0                 : The first point of the set.
// i_point1                 : The second point of the set.
// i_point2                 : The third point of the set.
// i_point3                 : The fourth point of the set.
// i_point4                 : The fifth point of the set.
// i_progression            : The progression on the spline we want to have the curvature calculated for.
// o_curvature              : The calculated curvature.
///////////////////////////////////////////////////////////////////////////
void Helper::getProgressionCurvature( const Vector &i_point0, 
                                      const Vector &i_point1, 
                                      const Vector &i_point2, 
                                      const Vector &i_point3, 
                                      const Vector &i_point4,
                                      double  i_progression,
                                      double &o_curvature )
{
    // The calculation of the curvature could be done with INTERPOLATION_ON_4_POINTS since
    // we do not need the derivative or the third order, but its easier to use 5 points.
    BSpline l_BSpline( INTERPOLATION_ON_5_POINTS );
    Vector deriv1, deriv2;
    
    l_BSpline.getDerivativeOrder1( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, deriv1  );
    l_BSpline.getDerivativeOrder2( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, deriv2 );
    
    double numerator = sqrt( pow( deriv2.z * deriv1.y - deriv2.y * deriv1.z, 2 ) +
                             pow( deriv2.x * deriv1.z - deriv2.z * deriv1.x, 2 ) +
                             pow( deriv2.y * deriv1.x - deriv2.x * deriv1.y, 2 ) );
    
    double denominator = pow( float(deriv1.x * deriv1.x + deriv1.y * deriv1.y + deriv1.z * deriv1.z), 3.0f / 2.0f );
    
    if( fabs( denominator ) < EPSILON )
        o_curvature = 0.0f;
    
    o_curvature = numerator / denominator;
}

///////////////////////////////////////////////////////////////////////////
// This function will calculate the points of a circle formed by a center, 
// a normal, a radius and a number of points.
//
// i_center             : The center of the circle we wat to generate points for.
// i_normal             : The normal of the circle. ( has to be normalized )
// i_radius             : The radius of the circle.
// i_nmTubeEdge         : This param will determine the number of points the circle will have.
// o_circlePoints       : The output vector countaning the points of the calculated circle.
///////////////////////////////////////////////////////////////////////////
void Helper::getCirclePoints( const Vector                &i_center, 
                              const Vector                &i_normal, 
                                    float                  i_radius, 
                                    int                    i_nmPoints, 
                                    std::vector< Vector > &o_circlePoints )
{
    Vector l_u, l_v;

    // Find the ortho-normal base for the current point.
    double l_factor =  1.0f / sqrt( i_normal.y*i_normal.y + i_normal.z*i_normal.z );
    l_u.x           =  0.0f;
    l_u.y           =  i_normal.z * l_factor;
    l_u.z           = -i_normal.y * l_factor;

    l_v      = i_normal.Cross( l_u );
    
    double l_t         = 0.0f;
    double l_increment = 2.0f*M_PI / i_nmPoints;

    for( int j = 0; j < i_nmPoints; ++j )
    {
        Vector l_result = i_center + l_u*( i_radius*cos( l_t ) ) + l_v*( i_radius*sin( l_t ) );
        o_circlePoints.push_back( l_result );
        l_t += l_increment;
    }
}

///////////////////////////////////////////////////////////////////////////
// Will calculate the intersection point between a line an a plane.
//
// i_la and i_lb       : 2 points of the line.
// i_p0                : 1 point on the plane.
// i_n                 : Normal of the plane.
// o_intersectionPoint : The intersection point between the line and the plane.
//
// Return true if the intersection point is on the line between i_la and i_lb, false otherwise. 
//
// SEE http://en.wikipedia.org/wiki/Line-plane_intersection Line-Plane Intersection (Algebraic form)
///////////////////////////////////////////////////////////////////////////
bool Helper::getIntersectionPoint( const Vector &i_la, 
                                   const Vector &i_lb,
                                   const Vector &i_p0,
                                   const Vector &i_n,
                                         Vector &o_intersectionPoint )
{
    double l_t = 0.0f;
    
    // Dot prodcut between the point on the plane and the normal of the plane.
    double l_d = - ( i_p0.x * i_n[0] +
                     i_p0.y * i_n[1] +
                     i_p0.z * i_n[2] );

    double l_numerator = -l_d - ( i_n[0] * i_la.x ) -
                                ( i_n[1] * i_la.y ) -
                                ( i_n[2] * i_la.z );

    double l_denominator = i_n[0] * ( i_lb.x - i_la.x ) + 
                           i_n[1] * ( i_lb.y - i_la.y ) +
                           i_n[2] * ( i_lb.z - i_la.z );

    // This mean that the direction of the line is perpendicular to the normal of the plane.
    if( fabs( l_denominator ) < EPSILON ) // Then l_denominator == 0.0f.
    {
        // This mean that the line is in the plane. We will return the intersection point to be in the middle of the line.
        if( fabs( l_numerator ) < EPSILON ) // Then l_numerator == 0.0f.
            l_t = 0.5f;
        // This mean that the line is not in the plane.
        else
            return false;
    }
    else
    {
        l_t = l_numerator / l_denominator;
    }

    // If l_t is between [0,1] then the intersection point is on the line between i_la and i_lb.
    if( l_t <= 1.0f && l_t >= 0.0f )
        o_intersectionPoint = i_la + ( ( i_lb - i_la ) * l_t );
    else
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Maps in 2d a set of 3d points that are on the same plane.
//
// i_planeNormal            : The plane's normal.
// i_pointsOnThePlane       : The vector containing the initial 3d points that are on the plane,                           
//                            the values in this vector will be replace by there 2d mapping value.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool Helper::convert3DPlanePointsTo2D( const Vector &i_planeNormal, std::vector< Vector > &io_pointsOnThePlane )
{
    // If we do not have at least 2 points, we will not be able to map them in 2d.
    if( io_pointsOnThePlane.size() < 2 )
        return false;

    // Create a vector with the first 2 points in the plane.
    Vector l_i( io_pointsOnThePlane[1].x - io_pointsOnThePlane[0].x, 
                io_pointsOnThePlane[1].y - io_pointsOnThePlane[0].y,
                io_pointsOnThePlane[1].z - io_pointsOnThePlane[0].z );

    l_i.normalize();

    // By doing a cross product on the normal vector and the vector on the plane, 
    // we get a vector that is perpendicular to both of them and its on the plane.
    Vector l_j = i_planeNormal.Cross( l_i );
    l_j.normalize();

    // The origin of the points in 2d will be the first point in io_pointsOnThePlane.
    Vector l_origin = io_pointsOnThePlane[0];

    io_pointsOnThePlane[0].x = 0.0f;
    io_pointsOnThePlane[0].y = 0.0f;
    io_pointsOnThePlane[0].z = 0.0f;

    for( unsigned i = 1; i < io_pointsOnThePlane.size(); ++i )
    {
        Vector l_vect( io_pointsOnThePlane[i].x - l_origin.x,
                       io_pointsOnThePlane[i].y - l_origin.y,
                       io_pointsOnThePlane[i].z - l_origin.z );
        
        io_pointsOnThePlane[i].x = l_vect.Dot( l_i );
        io_pointsOnThePlane[i].y = l_vect.Dot( l_j );

        // We set the z coord to the point index in the 
        // 3D point array, this way we'll be able to retrieve the
        // 3D coord from the 2D coord, with minimal computations.
        io_pointsOnThePlane[i].z = i;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////
// WARNING : ONLY WORKS WITH POINTS MAPPED FROM 3D TO 2D USING convert3DPlanePointsTo2D
// Re-Maps a set of 2d points to there 3d equivalent. The 3D points will be on the same plane.
// 
// i_original3Dpoints   : 3D points used to find the cross sections
// i_2Dpoints           : 2D cross sections points that were mapped into 2D
// o_3Dpoints           : Will be filled with the 3D points equivalent of the 2D points
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool Helper::convert2DPlanePointsTo3D( const std::vector< Vector > &i_original3Dpoints, 
                                       const std::vector< Vector > &i_2Dpoints, 
                                             std::vector< Vector > &o_3Dpoints )
{
    // Oh God, what a terrible hack, but it saves us a lot a calculations.
    if( i_original3Dpoints.size() == 0 || i_2Dpoints.size() == 0 )
        return false;

    for( unsigned i = 0; i < i_2Dpoints.size(); ++i )
    {
        if( i_2Dpoints[i].z >= i_original3Dpoints.size() )
            return false;

        o_3Dpoints.push_back( i_original3Dpoints[ (int)i_2Dpoints[i].z ] );
    }

    return true;
}
