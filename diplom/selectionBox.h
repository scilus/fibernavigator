#ifndef SELECTIONBOX_H_
#define SELECTIONBOX_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GL/glew.h"
#include "ArcBall.h"
#include "boundingBox.h"
#include <vector>
#include "DatasetHelper.h"

class MainCanvas;
class DatasetHelper;

class SelectionBox : public wxTreeItemData
{
public:
	SelectionBox(Vector3fT, Vector3fT, DatasetHelper*);
	~SelectionBox();

	void draw();
	hitResult hitTest(Ray *ray);
	void processDrag(wxPoint click, wxPoint lastPos);

	bool toggleShow() {return m_isVisible = !m_isVisible;};
	bool toggleNOT() {return m_isNOT = !m_isNOT;};
	bool getShow() {return m_isVisible;};
	void unselect() {m_isSelected = false;};

	void lockToCrosshair();

	void setCenter(Vector3fT c) { m_center = c; m_dirty = true;};
	void setCenter(float x, float y, float z);

	Vector3fT getCenter() {return m_center;};
	void setSize(Vector3fT v) {m_size = v;m_dirty = true;};
	Vector3fT getSize() {return m_size;};
	void setPicked(int s) {m_hr.picked = s;};
	bool isDirty() {return m_dirty;};
	void setDirty() {m_dirty = true;};
	void notDirty() {m_dirty = false;};
	bool colorChanged() {return m_colorChanged;};
	wxColour getColor() {return m_color;};
	void setColorChanged(bool v) {m_colorChanged = v;};
	void setTreeId(wxTreeItemId treeId) {m_treeId = treeId;};
	wxString getName() {return m_name;};
	void setName(wxString name) {m_name = name;};

	void draw1();
	void draw2();
	void draw3();
	void draw4();
	void draw5();
	void draw6();

	void moveLeft();
	void moveRight();
	void moveForward();
	void moveBack();
	void moveUp();
	void moveDown();

	void resizeLeft();
	void resizeRight();
	void resizeForward();
	void resizeBack();
	void resizeUp();
	void resizeDown();

	void setColor(wxColour);

	void update();
	void select();

	std::vector<bool>m_inBox;

	bool m_isTop;
	bool m_isNOT;
	bool m_isActive;
	bool m_isVisible;
	bool m_isSelected;
	bool m_isLockedToCrosshair;

	DatasetHelper* m_dh;

private:
	void drawSphere(float, float, float, float);
	void drag(wxPoint click, wxPoint lastPos);
	void resize(wxPoint click, wxPoint lastPos);

	float getAxisParallelMovement(int, int, int, int, Vector3fT);
	void updateStatusBar();

	Vector3fT m_center;
	Vector3fT m_size;
	hitResult m_hr;
	float mx, px, my, py, mz, pz;
	wxString m_name;

	float m_handleRadius;
	bool m_dirty;
	bool m_colorChanged;
	wxColour m_color;
	wxTreeItemId m_treeId;

	int m_stepSize;

};

#endif /*SELECTIONBOX_H_*/
