#ifndef BOUNDINGBOX_H_
#define BOUNDINGBOX_H_

#include "GL/glew.h"
#include "../misc/IsoSurface/Vector.h"


struct hitResult {
	bool    hit;
	float   tmin;
	int     picked;
	void*   object;
};

class Ray {
public:
	Ray (Vector, Vector);
	Ray (float, float, float, float, float, float);
	Vector m_origin;
	Vector m_end;
	Vector m_dir;
};


class BoundingBox {
public:
	BoundingBox(Vector, Vector);
	BoundingBox(float, float, float, float, float, float);

	hitResult hitTest(Ray*);

	void setCenterX(float x) {float xs = xmax - xmin;xmin = x - xs/2; xmax = x + xs/2;};
	void setCenterY(float y) {float ys = ymax - ymin;ymin = y - ys/2; ymax = y + ys/2;};
	void setCenterZ(float z) {float zs = zmax - zmin;zmin = z - zs/2; zmax = z + zs/2;};
	void setCenter( float, float, float);
	void setCenter( Vector );
	void setSize( float, float, float);
	void setSize( Vector );
	void setSizeX(float x) {float cx = xmin +(xmax - xmin)/2; xmin = cx - x/2; xmax = cx + x/2 ;};
	void setSizeY(float y) {float cy = ymin +(ymax - ymin)/2; ymin = cy - y/2; ymax = cy + y/2 ;};
	void setSizeZ(float z) {float cz = zmin +(zmax - zmin)/2; zmin = cz - z/2; zmax = cz + z/2 ;};

private:
	float xmin;
	float ymin;
	float zmin;
	float xmax;
	float ymax;
	float zmax;
};


#endif /*BOUNDINGBOX_H_*/
