#ifndef FACE3D_H
#define FACE3D_H

#include "../IsoSurface/Vector.h"


//////////////////////////////////////////////////////////////////////////////////
// Description :
//      Represent a plane in 3D space
//////////////////////////////////////////////////////////////////////////////////

struct Plane{
    double a, b, c, d;

    void computePlane(Vector pt1, Vector pt2, Vector pt3)
    {
        a = pt1.y * ( pt2.z - pt3.z ) + pt2.y * ( pt3.z - pt1.z ) + pt3.y * ( pt1.z - pt2.z );
		b = pt1.z * ( pt2.x - pt3.x ) + pt2.z * ( pt3.x - pt1.x ) + pt3.z * ( pt1.x - pt2.x );
		c = pt1.x * ( pt2.y - pt3.y ) + pt2.x * ( pt3.y - pt1.y ) + pt3.x * ( pt1.y - pt2.y );
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



//////////////////////////////////////////////////////////////////////////////////
// Description :
//      This class represent a 3D face
//////////////////////////////////////////////////////////////////////////////////
class Face3D
{
public:
    Face3D(){};
    Face3D( Vector pt1, Vector pt2, Vector pt3 );
    ~Face3D(){};

    void    setPoints       ( Vector pt1, Vector pt2, Vector pt3 );
    bool    isVisible       ( Vector point );
    Vector  getCentroid     ();
    void    FlipFace        ();

    Vector  getPt1          (){ return m_pt1; };
    Vector  getPt2          (){ return m_pt2; };
    Vector  getPt3          (){ return m_pt3; };

    bool    operator==      (const Face3D& face);
    bool    operator!=      (const Face3D& face);

private:
    Vector m_pt1, m_pt2, m_pt3;
    Plane m_plane;

};

#endif //FACE3D_H
