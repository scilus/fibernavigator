#ifndef THESCENE_H_
#define THESCENE_H_

#include "theDataset.h"
#include "GLSL/GLSLShaderProgram.h"

class TheScene {

public:
	bool nothing_loaded;

	bool m_showXSlize;
	bool m_showYSlize;
	bool m_showZSlize;
	
	TheScene();
	~TheScene() {};
	
	void initMainGL();
	void initNavGL();
	void assignTextures();
	void initShaders();
	
	void setDataset(TheDataset*);
	
	void renderScene();
	void renderNavView(int);
	void updateBlendThreshold(float);
	
	void updateView(float, float, float);
	
		
private:
	GLuint m_tex1;
	bool m_tex1_loaded;
	
	FGLSLShaderProgram *m_textureShader; 
	
	float *m_texture_head;
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
