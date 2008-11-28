#include "boundingBox.h"

Ray::Ray(Vector origin, Vector end) : m_origin(origin), m_end(end)
{
	Vector v1 (end[0] - origin[0], end[1] - origin[1], end[2] - origin[2]);
	m_dir = v1;
}

Ray::Ray(float x1, float y1, float z1, float x2, float y2, float z2)
{
	Vector v1(x1, y1, z1);
	m_origin = v1;
	Vector v2(x2, y2, z2);
	m_end = v2;
	Vector v3( m_end[0] - m_origin[0], m_end[1] - m_origin[1], m_end[2] - m_origin[2]);
	m_dir = v3;
}

BoundingBox::BoundingBox(Vector center, Vector size)
{
	xmin = center[0] -  size[0]/2.0;
	xmax = center[0] +  size[0]/2.0;
	ymin = center[1] -  size[1]/2.0;
	ymax = center[1] +  size[1]/2.0;
	zmin = center[2] -  size[2]/2.0;
	zmax = center[2] +  size[2]/2.0;
}

BoundingBox::BoundingBox(float x, float y, float z, float sizex, float sizey, float sizez)
{
	xmin = x - sizex/2.0;
	xmax = x + sizex/2.0;
	ymin = y - sizey/2.0;
	ymax = y + sizey/2.0;
	zmin = z - sizez/2.0;
	zmax = z + sizez/2.0;
}

void BoundingBox::setCenter(float x, float y, float z)
{
	float xs = xmax - xmin;
	xmin = x - xs/2.0;
	xmax = x + xs/2.0;
	float ys = ymax - ymin;
	ymin = y - ys/2.0;
	ymax = y + ys/2.0;
	float zs = zmax - zmin;
	zmin = z - zs/2.0;
	zmax = z + zs/2.0;
}

void BoundingBox::setCenter(Vector c)
{
	setCenter(c[0], c[1], c[2]);
}

void BoundingBox::setSize(float x, float y, float z)
{
	setSizeX(x);
	setSizeY(y);
	setSizeZ(z);
}

void BoundingBox::setSize(Vector c)
{
	setSize(c[0], c[1], c[2]);
}

hitResult BoundingBox::hitTest(Ray *ray)
{
	hitResult hr = {false, 0, 0, NULL};
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	float dirx = ray->m_dir.x;
	if (dirx >= 0) {
		tmin = ( xmin - ray->m_origin.x)/dirx;
		tmax = ( xmax - ray->m_origin.x)/dirx;
	}
	else {
		tmin = ( xmax - ray->m_origin.x)/dirx;
		tmax = ( xmin - ray->m_origin.x)/dirx;
	}
	float diry = ray->m_dir.y;
	if (diry >= 0) {
		tymin = ( ymin - ray->m_origin.y)/diry;
		tymax = ( ymax - ray->m_origin.y)/diry;
	}
	else {
		tymin = ( ymax - ray->m_origin.y)/diry;
		tymax = ( ymin - ray->m_origin.y)/diry;
	}
	if ( (tmin > tymax) || (tymin > tmax)) return hr;
	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;
	float dirz = ray->m_dir.z;
	if (dirz >= 0) {
		tzmin = ( zmin - ray->m_origin.z)/dirz;
		tzmax = ( zmax - ray->m_origin.z)/dirz;
	}
	else {
		tzmin = ( zmax - ray->m_origin.z)/dirz;
		tzmax = ( zmin - ray->m_origin.z)/dirz;
	}
	if ( (tmin > tzmax) || (tzmin > tmax)) return hr;
	if (tzmin > tmin) tmin = tzmin;
	if (tzmax < tmax) tmax = tzmax;

	if (tmin > tmax) return hr;
	hr.hit = true;
	hr.tmin = tmin;
	return hr;
}
