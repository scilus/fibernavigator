#ifndef SELECTIONBOX_H_
#define SELECTIONBOX_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GL/glew.h"
#include "boundingBox.h"
#include <vector>
#include "../dataset/DatasetHelper.h"
#include "../misc/IsoSurface/Vector.h"

class MainCanvas;
class DatasetHelper;
class CIsoSurface;
class Anatomy;

class SelectionBox : public wxTreeItemData
{
public:
	SelectionBox(Vector center, Vector size, DatasetHelper* dh);
	SelectionBox(DatasetHelper* dh, Anatomy* a);
	~SelectionBox();

	void draw();
	void drawIsoSurface();
	hitResult hitTest(Ray *ray);
	void processDrag(wxPoint click, wxPoint lastPos);

	bool toggleShow() {return m_isVisible = !m_isVisible;};
	bool toggleNOT() {return m_isNOT = !m_isNOT;};
	bool getNOT() {return m_isNOT;};
	void setNOT(bool v) {m_isNOT = v;};
	bool toggleActive() {return m_isActive = !m_isActive;};
	bool getActive() {return m_isActive;};
	void setActive(bool v) {m_isActive = v;};

	bool getVisible() {return m_isVisible;};
	void setVisible(bool v) {m_isVisible = v;};

	bool getShow() {return m_isVisible;};
	void unselect() {m_isSelected = false;};

	void lockToCrosshair();

	void setCenter(Vector c) { m_center = c; m_dirty = true;};
	void setCenter(float x, float y, float z);

	Vector getCenter() {return m_center;};
	void setSize(Vector v) {m_size = v;m_dirty = true;};
	Vector getSize() {return m_size;};
	void setPicked(int s) {m_hr.picked = s;};

	bool isDirty() {return m_dirty;};
	void setDirty(bool v);
	void setThreshold(float v);
	float getThreshold() { return m_threshold;};

	bool colorChanged() {return m_colorChanged;};
	wxColour getColor() {return m_color;};
	wxColour getFiberColor() {return m_fiberColor;};
	void setColorChanged(bool v) {m_colorChanged = v;};

	void setTreeId(wxTreeItemId treeId) {m_treeId = treeId;};

	wxString getName() {return m_name;};
	void setName(wxString name) {m_name = name;};
	int getIcon();
	bool getIsBox() {return m_isBox;};

	int countSelectedFibers();

	void setIsMaster(bool v);
	bool getIsMaster() {return m_isMaster;};

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
	void setFiberColor(wxColour);

	void update();
	void select(bool flag);

	std::vector<bool>m_inBox;
	std::vector<bool>m_inBranch;

	DatasetHelper* m_dh;
	Anatomy* m_sourceAnatomy;

private:
	void drawSphere(float, float, float, float);
	void drag(wxPoint click, wxPoint lastPos);
	void resize(wxPoint click, wxPoint lastPos);

	float getAxisParallelMovement(int, int, int, int, Vector);
	void updateStatusBar();

	Vector m_center;
	Vector m_size;
	hitResult m_hr;
	float mx, px, my, py, mz, pz;
	wxString m_name;

	float m_handleRadius;
	bool m_dirty;
	bool m_gfxDirty;
	bool m_colorChanged;
	// used for coloring the isosurface
	wxColour m_color;
	// used for the selected fibers
	wxColour m_fiberColor;
	wxTreeItemId m_treeId;

	int m_stepSize;

	float m_threshold;
	CIsoSurface* m_isosurface;
	bool m_isBox;

	bool m_isMaster;
	bool m_isNOT;
	bool m_isActive;
	bool m_isVisible;
	bool m_isSelected;
	bool m_isLockedToCrosshair;

};

#endif /*SELECTIONBOX_H_*/
