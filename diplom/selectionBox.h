#ifndef SELECTIONBOX_H_
#define SELECTIONBOX_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GL/glew.h"
#include "ArcBall.h"
#include "boundingBox.h"

class MainCanvas;

class SelectionBox {
public:
	SelectionBox(Vector3fT, Vector3fT);
	~SelectionBox() {};
	
	void draw();
	hitResult hitTest(Ray *ray);
	void processDrag(wxPoint click, wxPoint lastPos);
	
	bool toggleShow() {return m_show = !m_show;};
	bool getShow() {return m_show;};
	
	void setCenter(Vector3fT c) { m_center = c;};
	Vector3fT getCenter() {return m_center;};
	void setSize(Vector3fT v) {m_size = v;};
	Vector3fT getSize() {return m_size;};
	void setPicked(int s) {m_hr.picked = s;};
	
	
	
private:
	void drawSphere(float, float, float, float);
	void drag(wxPoint click);
	void resize(wxPoint click, wxPoint lastPos);
	Vector3fT mapMouse2World(int, int);
	Vector3fT mapMouse2WorldBack(int, int);
	float getAxisParallelMovement(int, int, int, int, Vector3fT);
	
	Vector3fT m_center;
	Vector3fT m_size;
	hitResult m_hr;
	float mx, px, my, py, mz, pz;
	
	float m_handleRadius;
	bool m_show;
};

#endif /*SELECTIONBOX_H_*/
