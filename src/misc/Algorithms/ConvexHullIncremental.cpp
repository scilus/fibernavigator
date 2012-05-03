#include "ConvexHullIncremental.h"

ConvexHullIncremental::ConvexHullIncremental( std::vector< Vector > &i_pointsVector ) :
    ConvexHull( i_pointsVector )
{}


//////////////////////////////////////////////////////////////////////////////////
// Search for points that can be use to build a thethraedron. 
// Point are valid if the are not coplanar. This method will failed (return less
// than four points) if all the points in m_allPoints are complanar (or if there is
// less than four points in the vector)
//
// Return the four firts point that can be use to build a tethrahedron.
//////////////////////////////////////////////////////////////////////////////////
std::vector<Vector> ConvexHullIncremental::getFourFirstPoint()
{
    std::vector< Vector > points;

    if ( m_allPoints.size() < 4 )
        return points;

    //The 2 first points
    points.push_back( m_allPoints[0] );
    points.push_back( m_allPoints[1] );
    
    Vector tmp;
    //Searching for the third point. It must not be on the line between the first and second point
    for ( unsigned int(i) = 2; i < m_allPoints.size(); i++ )
    {
        tmp = ( m_allPoints[1] - m_allPoints[0] ).Cross( m_allPoints[i] - m_allPoints[0] );
        if ( ( tmp.x!=0 ) || ( tmp.y != 0 ) || ( tmp.z != 0) )
        {
            points.push_back( m_allPoints[i] );
            tmp =  m_allPoints[i];
            m_allPoints[i] = m_allPoints[2];
            m_allPoints[2] = tmp;
            break;
        }
    }

    if ( points.size() == 3 ){
        Plane p;
        p.computePlane( points[0], points[1], points[2] );
        //Seaching for the fourth point. It must not be on the plane between the three point
        for ( unsigned int(i) = 3; i<m_allPoints.size(); i++ )
        {
            if ( !p.isOnPlane(m_allPoints[i]) )
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


//////////////////////////////////////////////////////////////////////////////////
// Build the actual convex hull. The hull is build using the incremental convex hull
// algorithm. It start by building a thethaedron and then add all other points one by
// one. If the points is in the hull, nothing is done, else, the points is include to
// the hull.
//
// Return the four firts point that can be use to build a tethrahedron.
//////////////////////////////////////////////////////////////////////////////////
bool ConvexHullIncremental::buildHull()
{
    if ( m_allPoints.size() < 3 )
        return false;
    
    //Compute the fisrt convex tetrahedron
    //Get 4 point that do not lie on a common plane
    std::vector< Vector > fourPoint = getFourFirstPoint();
    if ( fourPoint.size() < 4 )
        return false; //All points are in the same plane. The incremental method cannot be use.

    //Create a first face with the three first point we have
    Face3D face( fourPoint[0], fourPoint[1], fourPoint[2] );

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

    m_hullTriangles.clear();
    m_hullPoints.clear();
    //Add the thethrahedron to the valid face
    m_hullTriangles.push_back(face);
    m_hullTriangles.push_back(face0);
    m_hullTriangles.push_back(face1);
    m_hullTriangles.push_back(face2);

    //Main loop - Add all the remaining points to the hull
    std::vector< Face3D > visiblesFaces; //Faces that are visibles from the current point
    std::vector< Face3D > tempFaces;

    for ( unsigned int i(4); i < m_allPoints.size(); i++ )
    {
        visiblesFaces.clear();
        //Check if the vertice i is visible from all valid face
        std::list< Face3D >::iterator it = m_hullTriangles.begin();
        for ( ; it != m_hullTriangles.end(); it++ )
        {
            if ( it->isVisible( m_allPoints[i] ) )
                visiblesFaces.push_back( *it );
        }

        if ( visiblesFaces.size() == 0 )
            continue; //The current point is inside the convex hull
        
        //The vertex is outside the convex hull. We must delete all faces that can
        //see it.
        for ( unsigned int j(0); j<visiblesFaces.size(); j++ )
        {
            m_hullTriangles.remove( visiblesFaces[j] );
        }

        //Create new faces to include the new point in the hull
        //Only one face see the points. We can directly create the 3 faces as they won't enclose any other point
        if ( visiblesFaces.size() == 1 )
        {
            face = visiblesFaces[ 0 ];
            m_hullTriangles.push_back( Face3D( m_allPoints[i], face.getPt1(), face.getPt2() ) );
            m_hullTriangles.push_back( Face3D( m_allPoints[i], face.getPt2(), face.getPt3() ) );
            m_hullTriangles.push_back( Face3D( m_allPoints[i], face.getPt3(), face.getPt1() ) );
            continue;
        }

        //creates all possible new faces from the visibleFaces
        tempFaces.clear();
        for ( unsigned int j(0); j < visiblesFaces.size(); j++ )
        {
            tempFaces.push_back( Face3D( m_allPoints[i], visiblesFaces[j].getPt1(), visiblesFaces[j].getPt2() ) );
            tempFaces.push_back( Face3D( m_allPoints[i], visiblesFaces[j].getPt2(), visiblesFaces[j].getPt3() ) );
            tempFaces.push_back( Face3D( m_allPoints[i], visiblesFaces[j].getPt3(), visiblesFaces[j].getPt1() ) );
        }

        Face3D ot, cur;
        bool isValid = true;
        for ( unsigned int j(0); j < tempFaces.size(); j++ )
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
                m_hullTriangles.push_back( cur );
            isValid = true;
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////
// Compute the centroid of a face and a point. The centroid is simply the average
// of the three planes points and the fourth point
//
// Return the the centroid
//////////////////////////////////////////////////////////////////////////////////
Vector ConvexHullIncremental::getCentroid( Vector fourthPoint, Face3D face )
{
    return Vector( ( fourthPoint.x + face.getPt1().x + face.getPt2().x + face.getPt3().x ) / 4, ( fourthPoint.y + face.getPt1().y + face.getPt2().y + face.getPt3().y ) / 4, ( fourthPoint.z + face.getPt1().z +  face.getPt2().z + face.getPt3().z ) / 4 );
}


bool ConvexHullIncremental::getHullPoints ( std::vector< Vector > &o_points )
{
    if ( m_hullTriangles.size() == 0 )
        return false;

    if ( m_hullPoints.size() == 0 )
    {
        std::list< Face3D >::iterator it;
        for ( it = m_hullTriangles.begin(); it != m_hullTriangles.end(); it++ )
        {
            m_hullPoints.push_back( it->getPt1() );
            m_hullPoints.push_back( it->getPt2() );
            m_hullPoints.push_back( it->getPt3() );
        }
    }

    o_points = m_hullPoints;
    return true;
}

bool ConvexHullIncremental::getHullTriangles ( std::list< Face3D > &o_faces )
{
    if (m_hullTriangles.size() == 0)
        return false;
    o_faces = m_hullTriangles;
    return true;
}