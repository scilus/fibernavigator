#ifndef THESCENE_H_
#define THESCENE_H_

#include "theDataset.h"



class TheScene {

public:
	float *m_texture;
	TheDataset* m_dataset;
	bool tex1_loaded;
	
	TheScene();
	~TheScene() {};
	
	void setDataset(TheDataset*);
	
	void renderScene();
	
	void updateView(int, float);
	
		
private:
	float m_xOffset0;
	float m_yOffset0;
	float m_xOffset1;
	float m_yOffset1;
	float m_xOffset2;
	float m_yOffset2;
	float m_xSlize;
	float m_ySlize;
	float m_zSlize;
	float m_xTexture;
	float m_yTexture;
	float m_zTexture;
	
	void renderXSlize();
	void renderYSlize();
	void renderZSlize();
};

#endif /*THESCENE_H_*/
