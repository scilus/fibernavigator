#ifndef CONVEXHULLINCREMENTAL_H_
#define CONVEXHULLINCREMENTAL_H_

#include "ConvexHull.h"
#include "Face3D.h"

//////////////////////////////////////////////////////////////////////////////////
// Description :
//      This class uses the incremental algorithm to calculate the convex
//      hull of a batch of points in 3D
//////////////////////////////////////////////////////////////////////////////////
class ConvexHullIncremental : ConvexHull
{
public:

    ConvexHullIncremental( std::vector< Vector > &i_pointsVector );
    ~ConvexHullIncremental(){};

    bool   buildHull     ();
    Vector getCentroid   ( Vector fourthPoint, Face3D face );
    bool   getHullPoints ( std::vector< Vector > &o_points );
    bool   getHullTriangles ( std::list< Face3D > &o_faces );

private:
    std::list< Face3D > m_hullTriangles; 

    std::vector< Vector > getFourFirstPoint();
};

#endif //CONVEXHULLINCREMENTAL_H_
