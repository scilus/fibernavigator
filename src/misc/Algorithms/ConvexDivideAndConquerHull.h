#ifndef CONVEXDIVIDEANDCONQUERHULL_H
#define CONVEXDIVIDEANDCONQUERHULL_H

#include "ConvexHull.h"

//////////////////////////////////////////////////////////////////////////////////
// Description :
//      This class uses the Divide and conquer algorithm to calculate the convex
//      hull of a batch of points in 3D
// FIXME - Right now, this class is not working. The hull that is compute is not
//         good.
//////////////////////////////////////////////////////////////////////////////////
class convexDivideAndConquerHull : ConvexHull
{
public:
    //Constructor/Destructor
    convexDivideAndConquerHull( std::vector< Vector > &i_pointsVector );
    ~convexDivideAndConquerHull(){};

    bool    buildHull( std::vector< Vector > &o_points );
    void    draw();

    std::vector< std::vector<Vector> > getTriangles(){return m_triangles;};

private:
    //Methods
    void                                    sortPoints          ();
    std::vector< std::vector<Vector> >      buildHull           ( int i_firs, int i_last );
    std::vector<Vector>                     makeTriangle        ( Vector pts1, Vector pts2, Vector pts3 );
    void                                    deleteTriangles     ( std::vector<std::vector<Vector>> triangles, int first, int last );
    bool                                    isValidTriangle     ( std::vector<Vector> t, int first, int last );

    //Variable
    std::vector< std::vector< Vector > >    m_triangles; //All the triangles in the convex hull 

};

#endif