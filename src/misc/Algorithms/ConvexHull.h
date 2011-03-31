/////////////////////////////////////////////////////////////////////////////
// Name:            ConvexHull.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   09/26/2009
//
// Description: ConvexHull class.
// References: Dr. Dobb's (http://www.ddj.com/cpp/201806315)
//
// Last modifications:
//      by : ----
/////////////////////////////////////////////////////////////////////////////

#ifndef CONVEXHULL_H_
#define CONVEXHULL_H_

#include <vector>

#include "Helper.h"
#include "../IsoSurface/Vector.h"

//////////////////////////////////////////////////////////////////////////////////
// Description :
//      Function object used to compare Vector object in the convex hull 
//      algorith while sorting points using std::sort
//////////////////////////////////////////////////////////////////////////////////
struct CompareVector
{   
    // Constructor / Destructor
    CompareVector( AxisType i_comparisonAxis )
        : m_comparisonAxis( i_comparisonAxis )
    {};

    //Functions
    inline bool operator()( const Vector& i_p0, const Vector& i_p1 )
	{
        if( m_comparisonAxis == X_AXIS )
        {
            if( i_p0.x < i_p1.x )
                return true;

        }
        else if( m_comparisonAxis == Y_AXIS )
        {
            if( i_p0.y < i_p1.y )
                return true;
        }
        else
        {
            if( i_p0.z < i_p1.z )
                return true;
        }

        return false;
	}

private:
    
    // Variables
    AxisType m_comparisonAxis;
};

//////////////////////////////////////////////////////////////////////////////////
// Description :
//      This class uses the Graham scan algorithm to calculate the convex
//      hull of a batch of points in 3D, on a specific plane (x, y or z)
//////////////////////////////////////////////////////////////////////////////////

class ConvexHull
{

public :

    // Constructor / Destructor
    ConvexHull  ( std::vector< Vector > &i_pointsVector );
    ~ConvexHull (){};

    // Functions
    bool    area( double & o_surfaceArea );
    bool    buildHull( std::vector< Vector > &o_points );
    bool    getHullPoints( std::vector< Vector > &o_points );

private:

    // Functions    
    void    buildHalfHull( std::vector< Vector > &i_points, std::vector< Vector > &o_points, const int i_factor );    
    double  direction( const Vector &i_p0, const Vector &i_p1, const Vector &i_p2 );
    void    partitionPoints();
    double  triangleArea( const Vector &i_p0, const Vector &i_p1, const Vector &i_p2 );
    
private :

    // Variables
    Vector m_farLeftPoint;                      // Leftmost point
    Vector m_farRightPoint;                     // Rightmost point
    std::vector< Vector > m_allPoints;          // All the inputted points
    std::vector< Vector > m_hullPoints;         // Points forming the hull
    std::vector< Vector > m_upperPartitionPts;  // Upper partition points
    std::vector< Vector > m_lowerPartitionPts;  // Lower partition points
    std::vector< Vector > m_lowerHullPts;       // Lower hull points
    std::vector< Vector > m_upperHullPts;       // Upper hull points
};

#endif // CONVEXHULL_H_