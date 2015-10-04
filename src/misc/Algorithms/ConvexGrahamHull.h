#ifndef CONVEXGRAHAMHULL_H_
#define CONVEXGRAHAMHULL_H_

#include "ConvexHull.h"

//////////////////////////////////////////////////////////////////////////////////
// Description :
//      This class uses the Graham scan algorithm to calculate the convex
//      hull of a batch of points in 3D, on a specific plane (x, y or z)
//////////////////////////////////////////////////////////////////////////////////

class ConvexGrahamHull : public ConvexHull
{

public :

    // Constructor / Destructor
    ConvexGrahamHull  ( std::vector< Vector > &i_pointsVector );
    ~ConvexGrahamHull (){};

    // Functions
    bool                    area( double & o_surfaceArea );
    bool                    buildHull();

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
    std::vector< Vector > m_upperPartitionPts;  // Upper partition points
    std::vector< Vector > m_lowerPartitionPts;  // Lower partition points
    std::vector< Vector > m_lowerHullPts;       // Lower hull points
    std::vector< Vector > m_upperHullPts;       // Upper hull points
};

#endif //CONVEXGRAHAMHULL_H_
