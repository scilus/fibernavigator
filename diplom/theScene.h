#ifndef THESCENE_H_
#define THESCENE_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GLSL/GLSLShaderProgram.h"
#include "ArcBall.h"
#include "selectionBox.h"
#include "wx/glcanvas.h"
#include "surface.h"
#include "myListCtrl.h"

enum {
	axial =1,
	coronal,
	sagittal,
	mainView
};

class TheScene {

public:
	bool m_showSagittal;
	bool m_showCoronal;
	bool m_showAxial;
	bool m_showMesh;
	bool m_showBoxes;
	bool m_blendAlpha;

	TheScene();
	~TheScene();

	void initGL(int);

	void assignTextures();
	void addTexture();
	void swapTextures(int, int);
	void releaseTextures();
	GLuint makeCallList(DatasetInfo*);

	void initShaders();

	void setMainGLContext(wxGLContext* context) {m_mainGLContext = context;};
	wxGLContext* getMainGLContext() {return m_mainGLContext;};
	void setLightPos(Vector3fT value) {m_lightPos = value;};
	void setQuadrant(int quadrant) {m_quadrant = quadrant;};
	void toggleBoxes() {m_showBoxes = !m_showBoxes;};
	bool togglePointMode() {return m_pointMode = !m_pointMode;};
	bool getPointMode() {return m_pointMode;};

	void renderScene();
	void renderNavView(int);
	void setupLights();
	void switchOffLights();
	void renderMesh();
	void renderCurves();
	void renderSurface();
	void drawSelectionBoxes();
	void drawPoints();


	void colorMap(float);

	void drawSphere(float, float, float, float);

	void updateView(float, float, float);

	bool m_texAssigned;

	float m_xSlize;
	float m_ySlize;
	float m_zSlize;

	bool m_selBoxChanged;


private:
	int m_countTextures;
	GLuint *m_texNames;
	FGLSLShaderProgram *m_textureShader;
	FGLSLShaderProgram *m_meshShader;
	FGLSLShaderProgram *m_curveShader;

	wxGLContext* m_mainGLContext;

	float m_xOffset0;
	float m_yOffset0;
	float m_xOffset1;
	float m_yOffset1;
	float m_xOffset2;
	float m_yOffset2;

	float m_ratio0;
	float m_ratio1;
	float m_ratio2;

	int m_quadrant;
	Vector3fT m_lightPos;

	bool m_pointMode;

	void bindTextures();
	void setTextureShaderVars();
	void setMeshShaderVars();
	void renderXSlize();
	void renderYSlize();
	void renderZSlize();

};

#endif /*THESCENE_H_*/
