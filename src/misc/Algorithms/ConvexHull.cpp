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
#include <assert.h>

//////////////////////////////////////////////////////////////////////////////////
// Constructor
//
// std::vector< Vector > & i_pointsVector : vector containing all points
//////////////////////////////////////////////////////////////////////////////////
ConvexHull::ConvexHull( std::vector< Vector > & i_pointsVector )
:   m_allPoints( i_pointsVector )

{
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
// Constructor
//
// std::vector< Vector > & i_pointsVector : vector containing all points
//////////////////////////////////////////////////////////////////////////////////
ConvexGrahamHull::ConvexGrahamHull( std::vector< Vector > & i_pointsVector )
:   ConvexHull( i_pointsVector )

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
bool ConvexGrahamHull::buildHull( std::vector< Vector > &o_points )
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
bool ConvexGrahamHull::area( double & o_surfaceArea )
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
// Compute the surface area of a given triangle
//
// Vector i_p0, i_p1, i_p2 : Points forming the triangle
//
// Returns true if successful, false otherwise
//////////////////////////////////////////////////////////////////////////////////
double ConvexGrahamHull::triangleArea( const Vector &i_p0, const Vector &i_p1, const Vector &i_p2 )
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
void ConvexGrahamHull::partitionPoints()
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
void ConvexGrahamHull::buildHalfHull( std::vector< Vector > &i_points, std::vector< Vector > &o_points, const int i_factor )
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
double ConvexGrahamHull::direction( const Vector &i_p0, const Vector &i_p1, const Vector &i_p2 )
{
    return ( (i_p0.x - i_p1.x ) * (i_p2.y - i_p1.y ) ) - ( (i_p2.x - i_p1.x ) * (i_p0.y - i_p1.y ) );
}





/*******************************************************************************************************************************************************/
/*********************************************Begin implementation of ConvexDivideAndConquerHull********************************************************/
/*******************************************************************************************************************************************************/

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
    m_triangles = buildHull(0, m_allPoints.size() - 1);
    return true;
}


std::vector< std::vector<Vector> > convexDivideAndConquerHull::buildHull( int i_first, int i_last ){
    if (i_last - i_first < 2)
        return std::vector< std::vector< Vector > >();

    std::vector< std::vector< Vector > > triangles;
    int mid = (i_last + i_first)/2;

    std::vector< std::vector< Vector > > leftHull = buildHull(i_first, mid);
    std::vector< std::vector< Vector > > rightHull = buildHull(mid + 1, i_last);

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
        for( int i(mid+1); i <= i_last; i++)
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


    if (leftHull.size() > 0)
        triangles.insert(triangles.end(), leftHull.begin(), leftHull.end());
    if (rightHull.size() > 0)
        triangles.insert(triangles.end(), rightHull.begin(), rightHull.end());
    

    //Building the cylinder between the two hull.
    int leftPoint = leftLowerTangeantCandidate;
    int rightPoint = rightLowerTangeantCandidate;
    do {
        int candidate = (leftPoint==i_first)?i_first+1:i_first;
        HalfSpace candh = HalfSpace(m_allPoints[leftPoint], m_allPoints[rightPoint],m_allPoints[candidate]);
        for(int i=candidate+1; i <= i_last; i++)
        {
            if ( ( i != leftPoint ) && ( i != rightPoint ) && ( candh.inside(m_allPoints[i] ) ) )
            {
                candidate = i;
                candh = HalfSpace( m_allPoints[leftPoint],m_allPoints[rightPoint], m_allPoints[candidate] );
            }
        }
        
        triangles.push_back( makeTriangle( m_allPoints[leftPoint], m_allPoints[rightPoint], m_allPoints[candidate] ) );
        if (candidate <= mid) 
        {
          leftPoint = candidate;
        } 
        else 
        {
          rightPoint = candidate;
        }
    } while( ( leftPoint != leftLowerTangeantCandidate ) || ( rightPoint != rightLowerTangeantCandidate ) );
      
    //Delete triangles that are not part of the convex hull anymore
    //deleteTriangles(leftHull, mid+1,i_last);
    //deleteTriangles(rightHull, i_first,mid);

    return triangles;
    
}


void convexDivideAndConquerHull::deleteTriangles(std::vector<std::vector<Vector>> triangles, int first, int last)
{
    if (triangles.size() >= 0)
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
  
}


std::vector<Vector> convexDivideAndConquerHull::makeTriangle( Vector pt1, Vector pt2, Vector pt3 )
{
    std::vector<Vector> v;
    v.push_back(pt1);
    v.push_back(pt2);
    v.push_back(pt3);

    return v;
}


bool convexDivideAndConquerHull::isValidTriangle(std::vector<Vector> t, int first, int last)
{
    assert(t.size() == 3);
    //Compute the triangle halfspace
    HalfSpace h(t[0], t[1], t[2]);

    for (int i (first); i<=last; i++)
    {
        if (!h.inside(m_allPoints[i]))
            return false;
    }
    
    return true;
}


/*******************************************************************************************************************************************************/
/*********************************************Begin implementation of ConvexHullIncremental********************************************************/
/*******************************************************************************************************************************************************/


ConvexHullIncremental::ConvexHullIncremental( std::vector< Vector > &i_pointsVector ) :
ConvexHull(i_pointsVector)
{}

std::vector<Vector> ConvexHullIncremental::getFourFirstPoint()
{
    std::vector<Vector> points;
    //The 2 first points
    points.push_back(m_allPoints[0]);
    points.push_back(m_allPoints[1]);
    
    Vector tmp;
    //Searching for the third point. It must not be on the line between the first and second point
    for (int(i) = 2; i<m_allPoints.size(); i++)
    {
        tmp = (m_allPoints[1] - m_allPoints[0]).Cross(m_allPoints[i] - m_allPoints[0]);
        if ((tmp.x!=0) || (tmp.y != 0) || (tmp.z != 0))
        {
            points.push_back(m_allPoints[i]);
            tmp =  m_allPoints[i];
            m_allPoints[i] = m_allPoints[2];
            m_allPoints[2] = tmp;
            break;
        }
    }

    if (points.size() == 3){
        Plane p;
        p.computePlane(points[0], points[1], points[2]);
        //Seaching for the fourth point. It must not be on the plane between the three point
        for (int(i) = 3; i<m_allPoints.size(); i++)
        {
            if (!p.isOnPlane(m_allPoints[i]) )
            {
                points.push_back( m_allPoints[i] );
                tmp =  m_allPoints[i];
                m_allPoints[i] = m_allPoints[3];
                m_allPoints[3] = tmp;
                break;
            }
        }
    }

    return points;
}

bool ConvexHullIncremental::buildHull( std::vector< Vector > &o_points )
{
    if ( m_allPoints.size() < 3 )
        return false;
    
    //Compute the fisrt convex tetrahedron
    //Get 4 point that do not lie on a common plane
    std::vector<Vector> fourPoint = getFourFirstPoint();
    if (fourPoint.size() < 4)
        return false; //All points are in the same plane. The incremental method cannot be use.

    //Create a first face with the three first point we have
    Face3D face(fourPoint[0], fourPoint[1], fourPoint[2]);

    //Compute the centroid of the thethraedron. All face normal should point in the opposite direction
    //The thethrahedron is made with the fist four points of the array
    Vector thethaCenter = getCentroid( fourPoint[3], face );

    //Create all three remaining face and make sure all four face are in the good direction
    if ( face.isVisible( thethaCenter ) )
        face.FlipFace();
			
    Face3D face0( fourPoint[3], face.getPt1(), face.getPt2() );
	if ( face0.isVisible( thethaCenter ) )
        face0.FlipFace();
			
    Face3D face1( fourPoint[3], face.getPt2(), face.getPt3() );
	if ( face1.isVisible( thethaCenter ) ) 
        face1.FlipFace();
			
    Face3D face2( fourPoint[3], face.getPt3(), face.getPt1() );
	if ( face2.isVisible( thethaCenter ) ) 
        face2.FlipFace();

    //Add the thethrahedron to the valid face
    m_validFace.push_back(face);
    m_validFace.push_back(face0);
    m_validFace.push_back(face1);
    m_validFace.push_back(face2);

    //Main loop - Add all the remaining points to the hull
    std::vector< Face3D > visiblesFaces; //Faces that are visibles from the current point
    std::vector< Face3D > tempFaces;

    for ( unsigned int i(4); i<m_allPoints.size(); i++ )
    {
        visiblesFaces.clear();
        //Check if the vertice i is visible from all valid face
        std::list<Face3D>::iterator it = m_validFace.begin();
        for (it; it!=m_validFace.end(); it++){
            if ( it->isVisible( m_allPoints[i] ) )
                visiblesFaces.push_back(*it);
        }

        if (visiblesFaces.size() == 0)
            continue; //The current point is inside the convex hull
        
        //The vertex is outside the convex hull. We must delete all faces that can
        //see it.
        for (unsigned int j(0); j<visiblesFaces.size(); j++)
        {
            m_validFace.remove(visiblesFaces[j]);
        }

        //Create new faces to include the new point in the hull
        
        //Only one face see the points. We can directly create the 3 faces as they won't enclose any other point
		if ( visiblesFaces.size() == 1 )
		{
			face = visiblesFaces[ 0 ];
            m_validFace.push_back( Face3D( m_allPoints[i], face.getPt1(), face.getPt2() ) );
            m_validFace.push_back( Face3D( m_allPoints[i], face.getPt2(), face.getPt3() ) );
            m_validFace.push_back( Face3D( m_allPoints[i], face.getPt3(), face.getPt1() ) );
			continue;
		}

        //creates all possible new faces from the visibleFaces
		tempFaces.clear();
		for ( unsigned int j(0); j<visiblesFaces.size(); j++ )
		{
            tempFaces.push_back( Face3D( m_allPoints[i], visiblesFaces[j].getPt1(), visiblesFaces[j].getPt2() ) );
            tempFaces.push_back( Face3D( m_allPoints[i], visiblesFaces[j].getPt2(), visiblesFaces[j].getPt3() ) );
            tempFaces.push_back( Face3D( m_allPoints[i], visiblesFaces[j].getPt3(), visiblesFaces[j].getPt1() ) );
		}
				
		Face3D ot, cur;
        bool isValid = true;
        for ( unsigned int j(0); j<tempFaces.size(); j++)
		{
            cur = tempFaces[j];
			//search if there is a point in front of the face : 
			//this means the face doesn't belong to the convex hull
			for ( unsigned int k(0); k < tempFaces.size(); k++ )
			{
                if (k != j){
                    ot = tempFaces[k];
					if ( cur.isVisible( ot.getCentroid() ) )
					{
						isValid = false;
						break;
					}
                }
			}
			//the face has no point in front of it
			if ( isValid ) 
                m_validFace.push_back( cur );
            isValid = true;
		}
    }

    std::list<Face3D>::iterator it = m_validFace.begin();
    for (it = m_validFace.begin(); it!=m_validFace.end(); it++) 
    {
        o_points.push_back( it->getPt1() );
        o_points.push_back( it->getPt2() );
        o_points.push_back( it->getPt3() );
	}

    return true;
}


/*bool ConvexHullIncremental::buildHull(std::vector< Vector > &o_points)
{
    if (m_allPoints.size() < 3)
        return false;

    //Get four points to form a polyhedra
    std::vector<Vector> fourPoints = getFourFirstPoint();
    if (fourPoints.size() < 4)
        return false;
    
    //Make a random permutation
    //TODO



    return true;
}*/


Vector ConvexHullIncremental::getCentroid( Vector fourthPoint, Face3D face )
{
    return Vector( 	( fourthPoint.x + face.getPt1().x +  face.getPt2().x +  face.getPt3().x  ) / 4,	( fourthPoint.y + face.getPt1().y +  face.getPt2().y +  face.getPt3().y  ) / 4,	( fourthPoint.z + face.getPt1().z +  face.getPt2().z +  face.getPt3().z  ) / 4		);			
}



/*******************************************************************************************************************************************************/
/*********************************************Begin implementation of Face3D********************************************************/
/*******************************************************************************************************************************************************/

//////////////////////////////////////////////////////////////////////////////////
// Create a new Face3D
//
// int pt1, pt2, pt3 : index of the points forming the face
//
//////////////////////////////////////////////////////////////////////////////////
Face3D::Face3D( Vector pt1, Vector pt2, Vector pt3 )
{
    m_pt1 = pt1;
    m_pt2 = pt2;
    m_pt3 = pt3;

    m_plane.computePlane(pt1, pt2, pt3);
}

void Face3D::setPoints(Vector pt1, Vector pt2, Vector pt3)
{
    m_pt1 = pt1;
    m_pt2 = pt2;
    m_pt3 = pt3;

    m_plane.computePlane(pt1, pt2, pt3);
}

bool Face3D::isVisible( Vector point )
{
    return ( m_plane.a * point.x + m_plane.b * point.y + m_plane.c * point.z + m_plane.d ) >  0;
}


Vector Face3D::getCentroid()
{
    return Vector( ( m_pt1.x + m_pt2.x + m_pt3.x ) / 3, ( m_pt1.y + m_pt2.y + m_pt3.y ) / 3, ( m_pt1.z + m_pt2.z + m_pt3.z ) / 3 );
}

void Face3D::FlipFace()
{
    Vector temp = m_pt1;
    m_pt1 = m_pt2;
    m_pt2 = temp;
    m_plane.computePlane(m_pt1, m_pt2, m_pt3);
}


bool Face3D::operator==(const Face3D& face)
{
    if( m_pt1 == face.m_pt1 &&
        m_pt2 == face.m_pt2 &&
        m_pt3 == face.m_pt3    )
    {
        return true;
    }

    return false;
}

bool Face3D::operator!=(const Face3D& face)
{
    return !(*this==face);
}








