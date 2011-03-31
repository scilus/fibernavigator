/////////////////////////////////////////////////////////////////////////////
// Name:            Vector.h
// Author:          --------
// Email:           --------
// Creation Date:   --------
//
// Description: interface for the Vector class.
//
// Last modifications:
//      by : Imagicien - 10/04/2009
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VECTOR_H__0AFDDCFD_6A7A_4943_8068_CBA5CEC834C5__INCLUDED_)
#define AFX_VECTOR_H__0AFDDCFD_6A7A_4943_8068_CBA5CEC834C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>

#define VECTOR_X 0
#define VECTOR_Y 1
#define VECTOR_Z 2

class Vector  
{
public:
	Vector();
	Vector(double x, double y, double z);
	Vector(double[]);
	Vector(float[]);
	Vector(int[]);
	virtual ~Vector();

	double x;
	double y;
	double z;

	/* Operators for vector addition. Note that these
	   create a new Vector.
	*/
	Vector operator+(const Vector& rhs) const;
	Vector operator-(const Vector& rhs) const;

	/* Operators for scalar multiplication. Note that
	   these create a new Vector
	*/
	friend Vector  operator*    (const Vector& rhs, double fScale);
	friend Vector  operator*    (double fScale, const Vector& rhs);
	Vector operator/(const double factor) const;

    /* 
        Operators for vector arithmetic updates.
	*/
    Vector&  operator*=   (double fScale);
    Vector&  operator/=   (double fScale);
	Vector&  operator+=   (const Vector& rhs);
	Vector&  operator-=   (const Vector& rhs);

    /* 
        Operators for vector comparisons.
	*/
    bool operator==   (const Vector& rhs);
    bool operator!=   (const Vector& rhs);

	/* Array operator allows access to x, y and z
	   members through indices VECTOR_X, VECTOR_Y,
	   and VECTOR_Z, respectively (0, 1, and 2).
	   Only these three values should be used. If
	   another index is used, the Z-value is returned.
	   (RHS version)
	*/
	double operator[](const int index) const;

	/* Array operator allows access to x, y and z
	   members through indices VECTOR_X, VECTOR_Y,
	   and VECTOR_Z, respectively (0, 1, and 2).
	   Only these three values should be used. If
	   another index is used, the Z-value is returned.
	   (LHS version)
	*/
	double& operator[](const int index);

	/* Operation for vector addition. This operation
	   changes the original Vector. If the original is
	   not needed after the operation, then this is faster
	   than creating a new Vector object.
	*/
    void translateBy(const Vector& rhs);

	/* Operation for scalar multiplication. This operation
	   changes the original Vector. If the original is
	   not needed after the operation, then this is faster
	   than creating a new Vector object.
	*/
	void scaleBy(const double factor);

	/* Normalizes the vector, if the vector is not the
	   zero vector. If it is the zero vector, then it is
	   unchanged.
	*/
	void normalize();

	/* Normalizes the vector, if the vector is not the
	   zero vector. If it is the zero vector, then it is
	   unchanged. Returns the length the vector has before
	   normalization.
	*/
	double normalizeAndReturn();

	/* Zeros out the vector.
	*/
	void zero();

	/* Return the length of the vector
	*/
	double getLength() const;
	/* Return the squared length of the vector. Slightly
	   faster, and we may only need to compare relative lengths.
	*/
	double getSquaredLength() const;

	/* Return the dot product of this vector with the
	   specified right-hand side vector.
	*/
	double Dot(const Vector& rhs) const;

	/* Return the cross product of this vector with the
	   specified right-hand side vector.
    */
	Vector Cross(const Vector& rhs) const;

	/* Put this vector in the specified float array. The
	   array must be at least 3 elements long.
	*/
	void toArray(double array[]) const;
	/* Put this vector in the specified float array. The
	   array must be at least 3 elements long. Each component
	   is cast to a float, so loss of precision is likely.
	*/
	void toArray(float array[]) const;

	/* Set this vector to the contents of the first three
	   elements of the specified double array. The
	   array must be at least 3 elements long.
	*/
	void fromArray(double array[]);
	/* Set this vector to the contents of the first three
	   elements of the specified float array. The
	   array must be at least 3 elements long.
	*/
	void fromArray(float array[]);

	/* Rotate this vector about an axis. The rotation is specified
	   in degrees.
	*/
	void rotateX(const double degrees);
	void rotateY(const double degrees);
	void rotateZ(const double degrees);

	/* Rotate this vector about an axis. The rotation is specified
	   in radians. This is faster than the rotations specified in degrees, if
	   you already know the radians.
	*/
	void radianRotateX(const double radians);
	void radianRotateY(const double radians);
	void radianRotateZ(const double radians);

	/* Rotate this vector about an arbitrary axis. The rotation
	   is specified in degrees.
	*/
	void rotateAxis(const Vector& axis, const double degrees);

	/* Return a linearly interpolated vector between this
	   vector and an endpoint. t should vary between 0 and 1.
	*/
	Vector interpolate1(const Vector& endPoint, const double t) const;

	/* Return a qudratic Bezier interpolated vector with the
	   three controls points this vector, midControl, and endControl.
	   t should vary between 0 and 1.
	*/
	Vector interpolate2(const Vector& midControl, const Vector& endControl, const double t) const;

	/* Return a cubic Bezier interpolated vector with the four
	   controls points this vector, leftControl, rightControl,
	   and endControl. t should vary between 0 and 1.
	*/
	Vector interpolate3(const Vector& leftControl, const Vector& rightControl, const Vector& endControl, const double t) const;

};

#endif // !defined(AFX_VECTOR_H__0AFDDCFD_6A7A_4943_8068_CBA5CEC834C5__INCLUDED_)
