#include "boundingBox.h"

Ray::Ray(Vector3fT origin, Vector3fT end)
{
	m_origin = origin;
	m_end = end;
	Vector3fT v1 = {{end.s.X - origin.s.X, end.s.Y - origin.s.Y, end.s.Z - origin.s.Z}};
	m_dir = v1;
}

Ray::Ray(float x1, float y1, float z1, float x2, float y2, float z2)
{
	Vector3fT v1 = {{x1, y1, z1}};
	m_origin = v1;
	Vector3fT v2 = {{x2, y2, z2}};
	m_end = v2;
	Vector3fT v3 = {{m_end.s.X - m_origin.s.X, m_end.s.Y - m_origin.s.Y, m_end.s.Z - m_origin.s.Z}};
	m_dir = v3;
}

BoundingBox::BoundingBox(Vector3fT center, Vector3fT size)
{
	xmin = center.s.X -  size.s.X/2;
	xmax = center.s.X +  size.s.X/2;
	ymin = center.s.Y -  size.s.Y/2;
	ymax = center.s.Y +  size.s.Y/2;
	zmin = center.s.Z -  size.s.Z/2;
	zmax = center.s.Z +  size.s.Z/2;
}

BoundingBox::BoundingBox(float x, float y, float z, float sizex, float sizey, float sizez)
{
	xmin = x - sizex/2;
	xmax = x + sizex/2;
	ymin = y - sizey/2;
	ymax = y + sizey/2;
	zmin = z - sizez/2;
	zmax = z + sizez/2;
}

void BoundingBox::setCenter(float x, float y, float z)
{
	float xs = xmax - xmin;
	xmin = x - xs/2;
	xmax = x + xs/2;
	float ys = ymax - ymin;
	ymin = y - ys/2;
	ymax = y + ys/2;
	float zs = zmax - zmin;
	zmin = z - zs/2;
	zmax = z + zs/2;
}

void BoundingBox::setCenter(Vector3fT c)
{
	setCenter(c.s.X, c.s.Y, c.s.Z);
}

void BoundingBox::setSize(float x, float y, float z)
{
	setSizeX(x);
	setSizeY(y);
	setSizeZ(z);
}

void BoundingBox::setSize(Vector3fT c)
{
	setSize(c.s.X, c.s.Y, c.s.Z);
}

hitResult BoundingBox::hitTest(Ray *ray)
{
	hitResult hr = {false, 0};
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	float dirx = ray->m_dir.s.X;
	if (dirx >= 0) {
		tmin = ( xmin - ray->m_origin.s.X)/dirx;
		tmax = ( xmax - ray->m_origin.s.X)/dirx;
	}
	else {
		tmin = ( xmax - ray->m_origin.s.X)/dirx;
		tmax = ( xmin - ray->m_origin.s.X)/dirx;
	}
	float diry = ray->m_dir.s.Y;
	if (diry >= 0) {
		tymin = ( ymin - ray->m_origin.s.Y)/diry;
		tymax = ( ymax - ray->m_origin.s.Y)/diry;
	}
	else {
		tymin = ( ymax - ray->m_origin.s.Y)/diry;
		tymax = ( ymin - ray->m_origin.s.Y)/diry;
	}
	if ( (tmin > tymax) || (tymin > tmax)) return hr;
	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;
	float dirz = ray->m_dir.s.Z;
	if (dirz >= 0) {
		tzmin = ( zmin - ray->m_origin.s.Z)/dirz;
		tzmax = ( zmax - ray->m_origin.s.Z)/dirz;
	}
	else {
		tzmin = ( zmax - ray->m_origin.s.Z)/dirz;
		tzmax = ( zmin - ray->m_origin.s.Z)/dirz;
	}
	if ( (tmin > tzmax) || (tzmin > tmax)) return hr;
	if (tzmin > tmin) tmin = tzmin;
	if (tzmax < tmax) tmax = tzmax;

	if (tmin > tmax) return hr;
	hr.hit = true;
	hr.tmin = tmin;
	return hr;
}
