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
#include <GL/glew.h>
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

    virtual bool    buildHull       ( std::vector< Vector > &o_points ) = 0;
    virtual void    draw            (){};
    virtual bool    getHullPoints   ( std::vector< Vector > &o_points );

protected:
    //Variable
    std::vector< Vector > m_allPoints;   //The convex hull is around those point
    std::vector< Vector > m_hullPoints;  //Point forming the hull

};

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
    bool    area( double & o_surfaceArea );
    bool    buildHull( std::vector< Vector > &o_points );

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











//////////////////////////////////////////////////////////////////////////////////
// Description :
//      This class uses the Divide and conquer algorithm to calculate the convex
//      hull of a batch of points in 3D
//////////////////////////////////////////////////////////////////////////////////
class convexDivideAndConquerHull : ConvexHull
{
public:
    //Constructor/Destructor
    convexDivideAndConquerHull( std::vector< Vector > &i_pointsVector );
    ~convexDivideAndConquerHull(){};

    bool buildHull( std::vector< Vector > &o_points );
    void draw();

    std::vector< std::vector<Vector> > getTriangles(){return m_triangles;};

private:
    //Methods
    void                                    sortPoints          ();
    std::vector< std::vector<Vector> >      buildHull           (int i_firs, int i_last);
    std::vector<Vector>                     makeTriangle         (Vector pts1, Vector pts2, Vector pts3);
    void                                    deleteTriangles     (std::vector<std::vector<Vector>> triangles, int first, int last);
    bool                                    isValidTriangle     (std::vector<Vector> t, int first, int last);

    //Variable
    std::vector< std::vector<Vector> >    m_triangles; //All the triangles in the convex hull 

};


//////////////////////////////////////////////////////////////////////////////////
// Description :
//      This class represent a face
//////////////////////////////////////////////////////////////////////////////////

struct Plane{
    double a, b, c, d;

    void computePlane(Vector pt1, Vector pt2, Vector pt3)
    {
        a = pt1.y * (pt2.z - pt3.z) + pt2.y * (pt3.z - pt1.z) + pt3.y * (pt1.z - pt2.z);
		b = pt1.z * (pt2.x - pt3.x) + pt2.z * (pt3.x - pt1.x) + pt3.z * (pt1.x - pt2.x);
		c = pt1.x * (pt2.y - pt3.y) + pt2.x * (pt3.y - pt1.y) + pt3.x * (pt1.y - pt2.y);
		d = -( pt1.x * ( pt2.y * pt3.z - pt3.y * pt2.z ) + pt2.x * (pt3.y * pt1.z - pt1.y * pt3.z) + pt3.x * (pt1.y * pt2.z - pt2.y * pt1.z) );
    }

    bool isOnPlane(Vector pt)
    {
        //Plane equation is d = -aX - bY - cZ
        if ( -( pt.x * a + pt.y * b + pt.z * c ) == d )
            return true;
        return false;
    };
};


class Face3D
{
public:
    Face3D(){};
    Face3D(Vector pt1, Vector pt2, Vector pt3);
    ~Face3D(){};

    void setPoints(Vector pt1, Vector pt2, Vector pt3);
    bool isVisible(Vector point);
    Vector getCentroid();
    void FlipFace();

    Vector getPt1(){ return m_pt1; };
    Vector getPt2(){ return m_pt2; };
    Vector getPt3(){ return m_pt3; };

    bool operator==   (const Face3D& face);
    bool operator!=   (const Face3D& face);

private:
    Vector m_pt1, m_pt2, m_pt3;
    Plane m_plane;

};

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

    bool buildHull( std::vector< Vector > &o_points );
    Vector getCentroid(  Vector fourthPoint, Face3D face );

private:
    std::list< Face3D > m_validFace; 

    std::vector< Vector > getFourFirstPoint();
};

#endif // CONVEXHULL_H_