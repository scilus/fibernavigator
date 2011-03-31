/////////////////////////////////////////////////////////////////////////////
// Name:            ConvexHull.cpp
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   09/26/2009
//
// Description: This is the implementation file for ConvexHull class.
//
// Last modifications:
//      by : ----
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <math.h>

#include "ConvexHull.h"

//////////////////////////////////////////////////////////////////////////////////
// Constructor
//
// std::vector< Vector > & i_pointsVector : vector containing all points
//////////////////////////////////////////////////////////////////////////////////
ConvexHull::ConvexHull( std::vector< Vector > & i_pointsVector )
:   m_allPoints( i_pointsVector )

{
    if( i_pointsVector.size() > 0 )
        partitionPoints();
}

//////////////////////////////////////////////////////////////////////////////////
// Building the hull consists of two procedures: building the lower and
// then the upper hull.When building the upper hull, the middle point 
// must always be ABOVE the line formed by its two closest neighbors. 
// When building the lower hull, the middle point must be BELOW its two 
// closest neighbors. We pass this information to the building routine as 
// the last parameter, which is either -1 or 1.
//
// std::vector< Vector > o_points : Hull points
//
// Returns true if successful, false otherwise
//////////////////////////////////////////////////////////////////////////////////
bool ConvexHull::buildHull( std::vector< Vector > &o_points )
{
    if( m_allPoints.size() == 0 )
        return false;
    
    if( m_allPoints.size() < 3 )
    {
        o_points = m_hullPoints = m_allPoints;
        return true;
    }

    buildHalfHull( m_lowerPartitionPts, m_lowerHullPts,  1 );
    buildHalfHull( m_upperPartitionPts, m_upperHullPts, -1 );

    std::vector< Vector >::iterator it;

    // The points are stored in a counter clock-wise order
    for( it = m_lowerHullPts.begin(); it != m_lowerHullPts.end(); it++ )
    {
        m_hullPoints.push_back(*it);
    }

    std::vector< Vector >::reverse_iterator rit;
    for( rit = m_upperHullPts.rbegin(); rit != m_upperHullPts.rend(); rit++ )
    {
        // Avoiding duplicates
        if( *rit != m_farLeftPoint && *rit != m_farRightPoint )
            m_hullPoints.push_back(*rit);
    }

    o_points = m_hullPoints;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// Compute the surface area of the convex hull, by splitting the convex hull in 
// N - 2 triangles
//
// double o_surfaceArea : surface area of the convex hull
//
// Returns true if successful, false otherwise
//////////////////////////////////////////////////////////////////////////////////
bool ConvexHull::area( double & o_surfaceArea )
{
    // No area
    if( m_hullPoints.size() < 3 )
    {
        o_surfaceArea = 0.0f;
        return false;
    }

    for( unsigned int i = 2; i < m_hullPoints.size(); ++i )
    {
        o_surfaceArea += triangleArea( m_hullPoints[0], m_hullPoints[i - 1], m_hullPoints[i] );
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// Gives the convex hull points
//
// std::vector< Vector > o_points : Hull points
//
// Returns true if successful, false otherwise
//////////////////////////////////////////////////////////////////////////////////
bool ConvexHull::getHullPoints( std::vector< Vector > &o_points )
{
     // No hull
    if( m_hullPoints.size() == 0 )
        return false;

    o_points = m_hullPoints;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////
// Compute the surface area of a given triangle
//
// Vector i_p0, i_p1, i_p2 : Points forming the triangle
//
// Returns true if successful, false otherwise
//////////////////////////////////////////////////////////////////////////////////
double ConvexHull::triangleArea( const Vector &i_p0, const Vector &i_p1, const Vector &i_p2 )
{
    double l_area = ( i_p1.x - i_p0.x ) * ( i_p2.y - i_p0.y ) - ( i_p2.x - i_p0.x ) * ( i_p1.y - i_p0.y );

    l_area /= 2.0f;

    // In case we have a negative area, should not happen since our value are stored 
    // in a counterclockwise order, but just in case.
    if( l_area < 0)
        l_area *= -1.0;

    return l_area;
}

//////////////////////////////////////////////////////////////////////////////////
// Divides the points in 2 arrays depending on their location; above
// or below the line formed by the leftmost and rightmost points
//////////////////////////////////////////////////////////////////////////////////
void ConvexHull::partitionPoints()
{
    // No point in partionning.
    if( m_allPoints.size() < 3 )
        return;

    // We want to keep the inputted points intact.
    std::vector< Vector > points( m_allPoints );

    // Sorting the points using only one axis
    std::sort( points.begin(), points.end(), CompareVector( X_AXIS ) );

    // The the leftmost and rightmost points need to be removed and stored elsewhere.
    m_farLeftPoint = points.front();
    points.erase( points.begin() );

    m_farRightPoint = points.back();
    points.pop_back();

    double dir = 0.0f;

    // Separating points depending if they are above or below the line formed by the leftmost and rightmost points.
    for ( size_t i = 0 ; i < points.size() ; i++ )
    {
        dir = direction( m_farLeftPoint, m_farRightPoint, points[i] );

        if ( dir < 0.0f )
            m_upperPartitionPts.push_back( points[i] );
        else
            m_lowerPartitionPts.push_back( points[i] );
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Builds either the upper or the lower half convex hull, depending on the factor.
//
// int factor : 1 for the lower hull or -1 for the upper hull.
// std::vector< Vector > i_points : Input points
// std::vector< Vector > o_points : The points of the half hull
//////////////////////////////////////////////////////////////////////////////////
void ConvexHull::buildHalfHull( std::vector< Vector > &i_points, std::vector< Vector > &o_points, const int i_factor )
{
    // The hull will always start with the leftmost point and end with the rightmost point
    i_points.push_back( m_farRightPoint );
    o_points.push_back( m_farLeftPoint );

    unsigned int end = 0;

    while ( i_points.size() != 0 ) 
    {
        // 1. Add the leftmost point to the hull
        // 2. If a convexity violation occurs, remove the next-to-last  
        //    point in o_points until convexity is restored.
        o_points.push_back( i_points.front() );
        i_points.erase( i_points.begin() );

        while ( o_points.size() >= 3 ) 
        {
            end = o_points.size() - 1;

            if ( i_factor * direction( o_points[ end - 2 ], o_points[ end ], o_points[ end - 1 ] ) <= 0 ) 
            {
                o_points.erase( o_points.begin() + end - 1 );
            }
            else
                break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
// We look at three consecutive points, p0, p1, and p2, and determine whether p2
// has taken a left or right turn We translate the points so that p1 is at the 
// origin, then take the cross product of p0 and p2. 
//
// The result is:
//      positive if p2 has turned left
//      negative if p2 has turned right
//      equals 0 if p2 is on a straight line
//
// Vector i_p0, i_p1, i_p2 : Points with which we determine the direction
//////////////////////////////////////////////////////////////////////////////////
double ConvexHull::direction( const Vector &i_p0, const Vector &i_p1, const Vector &i_p2 )
{
    return ( (i_p0.x - i_p1.x ) * (i_p2.y - i_p1.y ) ) - ( (i_p2.x - i_p1.x ) * (i_p0.y - i_p1.y ) );
}
