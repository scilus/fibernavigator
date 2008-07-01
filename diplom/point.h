#ifndef POINT_H_
#define POINT_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GL/glew.h"
#include "ArcBall.h"
#include "boundingBox.h"

class Point {
public:
	Point(Vector3fT);
	~Point() {};
	
	void setCenter(Vector3fT c) { m_center = c; m_dirty = true;};
	Vector3fT getCenter() {return m_center;};
	void draw();
	
	hitResult hitTest(Ray *ray);
	void drag(wxPoint click);
	
private:
	Vector3fT m_center;
	bool m_dirty;
	hitResult m_hr;
	void drawSphere(float, float, float, float);
};

#endif /*POINT_H_*/
