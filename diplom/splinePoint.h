#ifndef POINT_H_
#define POINT_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GL/glew.h"
#include "ArcBall.h"
#include "boundingBox.h"
#include "DatasetHelper.h"

class DatasetHelper;

class SplinePoint {
public:
	SplinePoint(Vector3fT, DatasetHelper*);
	SplinePoint( double, double, double, DatasetHelper*);
	~SplinePoint();

	void setCenter(Vector3fT c) { m_center = c;};
	Vector3fT getCenter() {return m_center;};
	void draw();

	hitResult hitTest(Ray *ray);
	void drag(wxPoint click);
	void move(float);


	void select();
	void unselect() {m_selected = false;};
	float X() {return m_center.s.X;};
	float Y() {return m_center.s.Y;};
	float Z() {return m_center.s.Z;};
	void setX(float x);
	void setY(float y) {m_center.s.Y = y;};
	void setZ(float z) {m_center.s.Z = z;};
	void setOffsetVector (Vector3fT vec) {m_offsetVector = vec;};
	Vector3fT getOffsetVector() {return m_offsetVector;};
	void setTreeId(wxTreeItemId treeId) {m_treeId = treeId;};

private:
	DatasetHelper* m_dh;
	Vector3fT m_center;

	Vector3fT m_offsetVector;
	Vector3fT m_origin;

	bool m_selected;
	hitResult m_hr;
	void drawSphere(float, float, float, float);
	wxTreeItemId m_treeId;
};

#endif /*POINT_H_*/
