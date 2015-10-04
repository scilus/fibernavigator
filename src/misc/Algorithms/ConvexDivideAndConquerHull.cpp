#include "ConvexDivideAndConquerHull.h"
#include <algorithm>
#include <assert.h>

convexDivideAndConquerHull::convexDivideAndConquerHull( std::vector< Vector > & i_pointsVector )
:   ConvexHull( i_pointsVector )
{
}

//////////////////////////////////////////////////////////////////////////////////
//Sort all point base on their x position
//////////////////////////////////////////////////////////////////////////////////
void convexDivideAndConquerHull::sortPoints()
{
    std::sort( m_allPoints.rbegin(), m_allPoints.rend(), CompareVector( X_AXIS ) );
}

void convexDivideAndConquerHull::draw(){
    /*glBegin( GL_TRIANGLES );
        for (int i(0); i<m_triangles.size(); i++){
            glVertex3f( m_triangles[i][0].x, m_triangles[i][0].x, m_triangles[i][0].z );
            glVertex3f( m_triangles[i][1].x, m_triangles[i][1].x, m_triangles[i][1].z );
            glVertex3f( m_triangles[i][2].x, m_triangles[i][2].x, m_triangles[i][2].z );
        }
    glEnd();*/
}

bool convexDivideAndConquerHull::buildHull( std::vector< Vector > &o_points )
{
    sortPoints();
    m_triangles = buildHull( 0, m_allPoints.size() - 1 );
    return true;
}


std::vector< std::vector< Vector > > convexDivideAndConquerHull::buildHull( int i_first, int i_last ){
    if ( i_last - i_first < 2 )
        return std::vector< std::vector< Vector > >();

    std::vector< std::vector< Vector > > triangles;
    int mid = ( i_last + i_first )/2;

    std::vector< std::vector< Vector > > leftHull = buildHull( i_first, mid );
    std::vector< std::vector< Vector > > rightHull = buildHull( mid + 1, i_last );

    //Find the lower common tangeant for the left and right side
    //The tangeant connect the left hull to the right hull
    //We start with two candidate point for the beginning and the end of the tangeant
    int leftLowerTangeantCandidate = i_first;
    int rightLowerTangeantCandidate = i_last;

    bool find; //True if the tangeant stop changing in the loop
    HalfSpace leftCommonTangeant( m_allPoints[leftLowerTangeantCandidate], m_allPoints[rightLowerTangeantCandidate] );
   
    do
    {
        find = true;
        
        //Check on left side
        for( int i( i_first ); i <= mid; i++ ) 
        {
            if ( ( i != leftLowerTangeantCandidate ) && ( leftCommonTangeant.inside( m_allPoints[i] ) ) ) {
                leftLowerTangeantCandidate = i;
                leftCommonTangeant = HalfSpace( m_allPoints[rightLowerTangeantCandidate], m_allPoints[leftLowerTangeantCandidate]);
                find = false;
            }
        }

        //Check on right side
        for( int i(mid+1); i <= i_last; i++ )
        {
            if ( ( i != rightLowerTangeantCandidate ) && ( leftCommonTangeant.inside( m_allPoints[i] ) ) )
            {
                rightLowerTangeantCandidate = i;
                leftCommonTangeant = HalfSpace( m_allPoints[rightLowerTangeantCandidate], m_allPoints[leftLowerTangeantCandidate] );
                find = false;
            }
        }

    }
    while(!find);


    if ( leftHull.size() > 0 )
        triangles.insert( triangles.end(), leftHull.begin(), leftHull.end() );
    if ( rightHull.size() > 0 )
        triangles.insert( triangles.end(), rightHull.begin(), rightHull.end() );
    

    //Building the cylinder between the two hull.
    int leftPoint = leftLowerTangeantCandidate;
    int rightPoint = rightLowerTangeantCandidate;
    do {
        int candidate = ( leftPoint==i_first ) ? i_first + 1 : i_first;
        HalfSpace candh = HalfSpace( m_allPoints[leftPoint], m_allPoints[rightPoint], m_allPoints[candidate] );
        for(int i=candidate+1; i <= i_last; i++)
        {
            if ( ( i != leftPoint ) && ( i != rightPoint ) && ( candh.inside(m_allPoints[i] ) ) )
            {
                candidate = i;
                candh = HalfSpace( m_allPoints[leftPoint],m_allPoints[rightPoint], m_allPoints[candidate] );
            }
        }
        
        triangles.push_back( makeTriangle( m_allPoints[leftPoint], m_allPoints[rightPoint], m_allPoints[candidate] ) );
        if ( candidate <= mid ) 
        {
          leftPoint = candidate;
        } 
        else 
        {
          rightPoint = candidate;
        }
    } while( ( leftPoint != leftLowerTangeantCandidate ) || ( rightPoint != rightLowerTangeantCandidate ) );
      
    //Delete triangles that are not part of the convex hull anymore
    deleteTriangles(leftHull, mid+1,i_last);
    deleteTriangles(rightHull, i_first,mid);

    return triangles;
    
}


void convexDivideAndConquerHull::deleteTriangles( std::vector< std::vector< Vector > > triangles, int first, int last )
{
    for ( unsigned int i(0); i < triangles.size(); i++ )
    {
        if (isValidTriangle(triangles[i], first, last)) 
        {
            //Remove the triangle
            triangles.erase(triangles.begin() + i);
        }
    } 
}


std::vector<Vector> convexDivideAndConquerHull::makeTriangle( Vector pt1, Vector pt2, Vector pt3 )
{
    std::vector< Vector > v;
    v.push_back( pt1 );
    v.push_back( pt2 );
    v.push_back( pt3 );

    return v;
}


bool convexDivideAndConquerHull::isValidTriangle( std::vector< Vector > t, int first, int last)
{
    assert( t.size() == 3 );
    //Compute the triangle halfspace
    HalfSpace h( t[0], t[1], t[2] );

    for ( int i ( first); i<=last; i++ )
    {
        if ( !h.inside( m_allPoints[i] ) )
            return false;
    }
    
    return true;
}