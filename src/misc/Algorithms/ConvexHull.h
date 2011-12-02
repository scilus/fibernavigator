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
#include <list>

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
//      Represent a halfspace in 3D space
//////////////////////////////////////////////////////////////////////////////////
struct HalfSpace{
    Vector normal ; // normal to boundary plane
    double d; // eqn of half space is normal.x - d > 0
  
    // Create a half space
    HalfSpace( Vector a, Vector b, Vector c )
    {
        normal = ((b - a).Cross(c - a));
        normal.normalize();
        d = normal.Dot(a);
    }

    //Create a half space parallel to z axis
    HalfSpace(Vector a, Vector b)
    {
        normal = (b - a).Cross(Vector(0,0,1));
        normal.normalize();
        d = normal.Dot(a);
    }

    
    bool inside (Vector x)
    {
        return normal.Dot(x) > d;
    }
};


//////////////////////////////////////////////////////////////////////////////////
// Description :
//      This is the base class for all convex hull algorithm
//////////////////////////////////////////////////////////////////////////////////
class ConvexHull
{
public:
    //Constructor/Destructor
    ConvexHull(std::vector< Vector > &i_pointsVector);
    ~ConvexHull(){};

    virtual bool    buildHull       () = 0;
    virtual bool    getHullPoints   ( std::vector< Vector > &o_points );

protected:
    //Variable
    std::vector< Vector > m_allPoints;   //The convex hull is around those point
    std::vector< Vector > m_hullPoints;  //Point forming the hull

};

#endif // CONVEXHULL_H_