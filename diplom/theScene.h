#ifndef THESCENE_H_
#define THESCENE_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ArcBall.h"
#include "datasetInfo.h"
#include "DatasetHelper.h"

#include "wx/glcanvas.h"

enum {
	axial =1,
	coronal,
	sagittal,
	mainView
};

class DatasetHelper;

class TheScene {

public:
	bool m_showBoxes;
	bool m_blendAlpha;

	TheScene(DatasetHelper*);
	~TheScene();

	void initGL(int);

	void bindTextures();

	void setMainGLContext(wxGLContext* context) {m_mainGLContext = context;};
	wxGLContext* getMainGLContext() {return m_mainGLContext;};

	void toggleBoxes() {m_showBoxes = !m_showBoxes;};
	bool togglePointMode() {return m_pointMode = !m_pointMode;};
	bool getPointMode() {return m_pointMode;};

	void renderScene();
	void renderNavView(int);

	void drawSphere(float, float, float, float);

	bool m_texAssigned;
	bool m_selBoxChanged;


private:
	wxGLContext* m_mainGLContext;

	DatasetHelper* m_dh;

	bool m_pointMode;

	void lightsOn();
	void lightsOff();

	void renderSlizes();
	void renderMesh();
	void renderFibers();
	void renderSurface();

	void drawSelectionBoxes();
	void drawPoints();
};

#endif /*THESCENE_H_*/
