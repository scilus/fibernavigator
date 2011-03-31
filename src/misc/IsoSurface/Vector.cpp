/////////////////////////////////////////////////////////////////////////////
// Name:            Vector.cpp
// Author:          --------
// Email:           --------
// Creation Date:   --------
//
// Description: implementation for the Vector class.
//
// Last modifications:
//      by : Imagicien - 10/04/2009
/////////////////////////////////////////////////////////////////////////////

#include "Vector.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Vector::Vector()
{
	x = 0;
	y = 0;
	z = 0;
}

Vector::Vector(double newX, double newY, double newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

Vector::Vector(double array[])
{
	x = array[0];
	y = array[1];
	z = array[2];
}

Vector::Vector(float array[])
{
	x = array[0];
	y = array[1];
	z = array[2];
}

Vector::Vector(int array[])
{
	x = array[0];
	y = array[1];
	z = array[2];
}

Vector::~Vector()
{

}

Vector& Vector::operator*=(double fScale)
{
	x *= fScale;
	y *= fScale;
	z *= fScale;

    return *this;
}

Vector& Vector::operator/=(double fScale)
{
	x /= fScale;
	y /= fScale;
	z /= fScale;

    return *this;
}

Vector& Vector::operator+=(const Vector& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;

    return *this;
}

Vector& Vector::operator-=(const Vector& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;

    return *this;
}

bool Vector::operator==(const Vector& rhs)
{
    if(	x == rhs.x &&
	    y == rhs.y &&
	    z == rhs.z   )
    {
        return true;
    }

	return false;
}

bool Vector::operator!=(const Vector& rhs)
{
    if(	x == rhs.x &&
	    y == rhs.y &&
	    z == rhs.z   )
    {
        return false;
    }

	return true;
}

Vector Vector::operator+(const Vector& rhs) const
{
    return Vector(x + rhs.x, y + rhs.y, z + rhs.z);
}

Vector Vector::operator-(const Vector& rhs) const
{
    return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
}

Vector operator*(const Vector& rhs, double fScale)
{
	Vector result;

	result.x = rhs.x*fScale;
	result.y = rhs.y*fScale;
	result.z = rhs.z*fScale;

	return result;
}

Vector operator*(double fScale, const Vector& rhs)
{
	Vector result;

	result.x = rhs.x*fScale;
	result.y = rhs.y*fScale;
	result.z = rhs.z*fScale;

	return result;
}

Vector Vector::operator/(const double factor) const
{
    return Vector(x / factor, y / factor, z / factor);
}

double Vector::operator[](const int index) const
{
	switch (index) {
		case VECTOR_X:
			return x;
			break;
		case VECTOR_Y:
			return y;
			break;
		default:
			return z;
			break;
	}
}

double& Vector::operator[](const int index)
{
	switch (index) {
		case VECTOR_X:
			return x;
			break;
		case VECTOR_Y:
			return y;
			break;
		default:
			return z;
			break;
	}
}

void Vector::translateBy(const Vector& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
}

void Vector::scaleBy(const double factor)
{
	x *= factor;
	y *= factor;
	z *= factor;
}

void Vector::normalize()
{
	double length = sqrt(x * x + y * y + z * z);
	if (length > 0.000001) {
		x /= length;
		y /= length;
		z /= length;
	}
}

double Vector::normalizeAndReturn()
{
	double length = sqrt(x * x + y * y + z * z);
	if (length > 0.0001) {
		x /= length;
		y /= length;
		z /= length;
	}
	return length;
}

void Vector::zero()
{
	x = y = z = 0.0;
}

double Vector::getLength() const
{
	return sqrt(x * x + y * y + z * z);
}

double Vector::getSquaredLength() const
{
	return (x * x + y * y + z * z);
}

double Vector::Dot(const Vector& rhs) const
{
	return (x * rhs.x + y * rhs.y + z * rhs.z);
}

Vector Vector::Cross(const Vector& rhs) const
{
	return Vector((y * rhs.z) - (z * rhs.y),
		          (z * rhs.x) - (x * rhs.z),
				  (x * rhs.y) - (y * rhs.x));
}

void Vector::toArray(double array[]) const
{
	array[0] = x;
	array[1] = y;
	array[2] = z;
}

void Vector::toArray(float array[]) const
{
	array[0] = (float)x;
	array[1] = (float)y;
	array[2] = (float)z;
}

void Vector::fromArray(double array[])
{
	x = array[0];
	y = array[1];
	z = array[2];
}

void Vector::fromArray(float array[])
{
	x = array[0];
	y = array[1];
	z = array[2];
}

void Vector::rotateX(const double degrees) {
	double radians = degrees * 3.14159265358979 / 180.0;
	double cosAngle = cos(radians);
	double sinAngle = sin(radians);
	double origY = y;
	y =	y * cosAngle - z * sinAngle;
	z = origY * sinAngle + z * cosAngle;
}

void Vector::rotateY(const double degrees) {
	double radians = degrees * 3.14159265358979 / 180.0;
	double cosAngle = cos(radians);
	double sinAngle = sin(radians);
	double origX = x;
	x =	x * cosAngle + z * sinAngle;
	z = z * cosAngle - origX * sinAngle;
}

void Vector::rotateZ(const double degrees) {
	double radians = degrees * 3.14159265358979 / 180.0;
	double cosAngle = cos(radians);
	double sinAngle = sin(radians);
	double origX = x;
	x =	x * cosAngle - y * sinAngle;
	y = origX * sinAngle + y * cosAngle;
}

void Vector::radianRotateX(const double radians) {
	double cosAngle = cos(radians);
	double sinAngle = sin(radians);
	double origY = y;
	y =	y * cosAngle - z * sinAngle;
	z = origY * sinAngle + z * cosAngle;
}

void Vector::radianRotateY(const double radians) {
	double cosAngle = cos(radians);
	double sinAngle = sin(radians);
	double origX = x;
	x =	x * cosAngle + z * sinAngle;
	z = z * cosAngle - origX * sinAngle;
}

void Vector::radianRotateZ(const double radians) {
	double cosAngle = cos(radians);
	double sinAngle = sin(radians);
	double origX = x;
	x =	x * cosAngle - y * sinAngle;
	y = origX * sinAngle + y * cosAngle;
}

void Vector::rotateAxis(const Vector& axis, double degrees)
{
	Vector rotator;
	Vector rx;
	Vector ry;
	Vector rz;

	// Normalize the axis and store in rz for later
	// potential use.
	rz = axis / axis.getLength();

	// If this vector is parallel to the axis (Now in rz),
	// don't bother rotating it. Check with Cauchy-Schwartz
	// u dot v == length of u * length of v iff u is linearly
	// dependant on v. Length of rz is one, because it is
	// normalized.
	if (!(fabs(this->Dot(rz)) == this->getLength())) {
	    // If we're not already rotating around the Z, transform to Z
	    if (axis.x == 0 && axis.y == 0) {
            // In this case, the axis is along Z already, so we
			// wont bother rotating our axis of rotation to Z.
			// Since we're checking directly for 0, this is going
			// to happen extremely rarely. However, if it does
			// happen, some of the below math falls apart. So,
			// we check.
			rotator = *this;
		} else {
	        // Build the rotation matrix
	        // rz was assigned already while normalizing the axis.
	        rx = this->Cross(axis);
	        rx.normalize();
	        ry = rz.Cross(rx);

	        // Move this vector such that the axis would be in Z
	        rotator = Vector(rx.Dot(*this), ry.Dot(*this), rz.Dot(*this));
		}

	    // Rotate this vector around Z
		rotator.rotateZ(degrees);

	    if (axis.x == 0 && axis.y == 0) {
		    *this = rotator;
		} else {
    	    // Move back so axis is in original location.
	        this->x = rotator.x * rx.x +
		              rotator.y * ry.x +
			          rotator.z * rz.x;
	        this->y = rotator.x * rx.y +
		              rotator.y * ry.y +
			          rotator.z * rz.y;
	        this->z = rotator.x * rx.z +
		              rotator.y * ry.z +
		    	      rotator.z * rz.z;
		}
	}
}

Vector Vector::interpolate1(const Vector& endPoint, const double t) const
{
	return Vector((1.0 - t) * x + t * endPoint.x,
		          (1.0 - t) * y + t * endPoint.y,
				  (1.0 - t) * z + t * endPoint.z);
}

Vector Vector::interpolate2(const Vector& midControl, const Vector& endControl, const double t) const
{
    Vector left = this->interpolate1(midControl, t);
	Vector right = midControl.interpolate1(endControl, t);
	return left.interpolate1(right, t);
}

Vector Vector::interpolate3(const Vector& leftControl, const Vector& rightControl, const Vector& endControl, const double t) const
{
    Vector begin = this->interpolate1(leftControl, t);
	Vector mid = leftControl.interpolate1(rightControl, t);
	Vector end = rightControl.interpolate1(endControl, t);
	return begin.interpolate2(mid, end, t);
}
