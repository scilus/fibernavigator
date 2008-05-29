#ifndef THESCENE_H_
#define THESCENE_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "theDataset.h"

#include "GLSL/GLSLShaderProgram.h"

enum {
	axial,
	coronal,
	sagittal,
	mainView
};

class TheScene {

public:
	bool m_showXSlize;
	bool m_showYSlize;
	bool m_showZSlize;
	bool m_showOverlay;
	bool m_showRGB;
	
	TheScene();
	~TheScene();
	
	void initMainGL();
	void initNavGL();
	void assignTextures();
	void initShaders();
	
	void setDataset(TheDataset*);
	
	void renderScene(int);
	void renderNavView(int);
	void updateBlendThreshold(float);
	TheDataset* getDataset() {return m_dataset;};
	
	void updateView(float, float, float);
	void releaseTextures();
	
		
private:
	GLuint *m_texNames;
	int m_countTextures;
	
	FGLSLShaderProgram *m_textureShader; 
	
	TheDataset* m_dataset;
	float m_blendThreshold;
	
	float m_xSize;
	float m_ySize;
	float m_zSize;
	
	float m_xOffset0;
	float m_yOffset0;
	float m_xOffset1;
	float m_yOffset1;
	float m_xOffset2;
	float m_yOffset2;
	float m_xSlize;
	float m_ySlize;
	float m_zSlize;

	float m_ratio0;
	float m_ratio1;
	float m_ratio2;

	float m_xLine;
	float m_yLine;
	float m_zLine;
	
	void bindTextures();
	void renderXSlize();
	void renderYSlize();
	void renderZSlize();
};

#endif /*THESCENE_H_*/
