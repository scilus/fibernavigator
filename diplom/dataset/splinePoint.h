#ifndef POINT_H_
#define POINT_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GL/glew.h"
#include "../gui/boundingBox.h"
#include "DatasetHelper.h"

class DatasetHelper;

class SplinePoint : public wxTreeItemData
{
public:
	SplinePoint(Vector, DatasetHelper*);
	SplinePoint( double, double, double, DatasetHelper*);
	~SplinePoint();

	void setCenter(Vector c) { m_center = c;};
	Vector getCenter() {return m_center;};
	void draw();

	hitResult hitTest(Ray *ray);
	void drag(wxPoint click);
	void move(float);


	void select(bool flag);
	void unselect() {m_selected = false;};
	float X() {return m_center.x;};
	float Y() {return m_center.y;};
	float Z() {return m_center.z;};
	void setX(float x);
	void setY(float y) {m_center.y = y;};
	void setZ(float z) {m_center.z = z;};
	void setOffsetVector (Vector vec) {m_offsetVector = vec;};
	Vector getOffsetVector() {return m_offsetVector;};
	void setTreeId(wxTreeItemId treeId) {m_treeId = treeId;};
	void setIsBoundary(bool v) {m_isBoundary = v;};
	bool isBoundary() {return m_isBoundary;};

private:
	DatasetHelper* m_dh;
	Vector m_center;

	Vector m_offsetVector;
	Vector m_origin;

	bool m_selected;
	bool m_isBoundary;
	hitResult m_hr;
	void drawSphere(float, float, float, float);
	wxTreeItemId m_treeId;
};

#endif /*POINT_H_*/
