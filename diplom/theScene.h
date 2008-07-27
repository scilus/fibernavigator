#ifndef THESCENE_H_
#define THESCENE_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GLSL/GLSLShaderProgram.h"
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
	bool m_showMesh;
	bool m_showBoxes;
	bool m_blendAlpha;

	TheScene(DatasetHelper*);
	~TheScene();

	void initGL(int);

	void initShaders();
	void bindTextures();
	void setTextureShaderVars();

	void setMainGLContext(wxGLContext* context) {m_mainGLContext = context;};
	wxGLContext* getMainGLContext() {return m_mainGLContext;};
	void setLightPos(Vector3fT value) {m_lightPos = value;};
	void toggleBoxes() {m_showBoxes = !m_showBoxes;};
	bool togglePointMode() {return m_pointMode = !m_pointMode;};
	bool getPointMode() {return m_pointMode;};

	void renderScene();
	void renderNavView(int);

	void drawSphere(float, float, float, float);

	bool m_texAssigned;
	bool m_selBoxChanged;


private:
	FGLSLShaderProgram *m_textureShader;
	FGLSLShaderProgram *m_meshShader;
	FGLSLShaderProgram *m_curveShader;

	wxGLContext* m_mainGLContext;

	DatasetHelper* m_dh;

	float m_xOffset0;
	float m_yOffset0;
	float m_xOffset1;
	float m_yOffset1;
	float m_xOffset2;
	float m_yOffset2;

	float m_ratio0;
	float m_ratio1;
	float m_ratio2;

	Vector3fT m_lightPos;

	bool m_pointMode;


	void setMeshShaderVars();

	void setupLights();
	void switchOffLights();

	void renderSlizes();
	void renderMesh();
	void renderFibers();
	void renderSurface();

	void drawSelectionBoxes();
	void drawPoints();
};

#endif /*THESCENE_H_*/
