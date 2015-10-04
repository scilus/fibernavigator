#include "Face3D.h"

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

    m_plane.computePlane( pt1, pt2, pt3 );
}

void Face3D::setPoints( Vector pt1, Vector pt2, Vector pt3 )
{
    m_pt1 = pt1;
    m_pt2 = pt2;
    m_pt3 = pt3;

    m_plane.computePlane( pt1, pt2, pt3 );
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
    m_plane.computePlane( m_pt1, m_pt2, m_pt3 );
}


bool Face3D::operator==( const Face3D& face )
{
    if( m_pt1 == face.m_pt1 &&
        m_pt2 == face.m_pt2 &&
        m_pt3 == face.m_pt3    )
    {
        return true;
    }

    return false;
}

bool Face3D::operator!=( const Face3D& face )
{
    return !( *this==face );
}