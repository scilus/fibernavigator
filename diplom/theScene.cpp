#include "theScene.h"

#include "wx/wx.h"
#include <GL/gl.h>
#include <GL/glu.h>


TheScene::TheScene()
{
	tex1_loaded = false;
	m_xSlize = 0.5;
	m_ySlize = 0.5;
	m_zSlize = 0.5;
	m_xTexture = 0.5;
	m_yTexture = 0.5;
	m_zTexture = 0.5;
}

void TheScene::setDataset(TheDataset *dataset)
{
	m_dataset = dataset;
	
	float *temp = m_dataset->getData();
	int size = m_dataset->getLength();
	m_texture = new float[size *2];
	
	for (int i = 0 ; i < size; ++i)
	{
		m_texture[2*i] = temp[i];
		m_texture[(2*i)+1] =  temp[i] < 0.05 ? 0 : 255;
	}
	
		
	float xSize = (float)dataset->getColumns();
	float ySize = (float)dataset->getRows();
	float zSize = (float)dataset->getFrames();
	
	float ratio0 = xSize/ySize;
	float ratio1 = xSize/zSize;
	float ratio2 = ySize/zSize;

	m_xOffset0 = (wxMax (0, 1.0 - ratio0))/2.0;
	m_yOffset0 = (wxMax (0, ratio0 - 1.0))/2.0;
	m_xOffset1 = (wxMax (0, 1.0 - ratio1))/2.0;
	m_yOffset1 = (wxMax (0, ratio1 - 1.0))/2.0;
	m_xOffset2 = (wxMax (0, 1.0 - ratio2))/2.0;
	m_yOffset2 = (wxMax (0, ratio2 - 1.0))/2.0;
	
	m_xOffset0 = wxMax(m_xOffset0, m_xOffset1);
	m_xOffset1 = wxMax(m_xOffset0, m_xOffset1);
	
	m_yOffset1 = wxMax(m_yOffset1, m_yOffset2);
	m_yOffset2 = wxMax(m_yOffset1, m_yOffset2);
	
	tex1_loaded = true;
}

void TheScene::renderScene()
{
	renderXSlize();
	renderYSlize();
	renderZSlize();
}


void TheScene::renderXSlize()
{
	glBegin(GL_QUADS);
		glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 0.0 - m_yOffset2); glVertex3f(m_xSlize -0.5, -0.5, -0.5);
    	glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 1.0 + m_yOffset2); glVertex3f(m_xSlize -0.5, -0.5, 0.5);
    	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 1.0 + m_yOffset2); glVertex3f(m_xSlize -0.5, 0.5, 0.5);
    	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 0.0 - m_yOffset2); glVertex3f(m_xSlize -0.5, 0.5, -0.5);
    glEnd();
}

void TheScene::renderYSlize()
{
	glBegin(GL_QUADS);
    	glTexCoord3f(0.0 - m_xOffset1, m_ySlize, 0.0 - m_yOffset1); glVertex3f(-0.5, m_ySlize -0.5, -0.5);
    	glTexCoord3f(0.0 - m_xOffset1, m_ySlize, 1.0 + m_yOffset1); glVertex3f(-0.5, m_ySlize -0.5, 0.5);
    	glTexCoord3f(1.0 + m_xOffset1, m_ySlize, 1.0 + m_yOffset1); glVertex3f(0.5, m_ySlize -0.5, 0.5);
    	glTexCoord3f(1.0 + m_xOffset1, m_ySlize, 0.0 - m_yOffset1); glVertex3f(0.5, m_ySlize -0.5, -0.5);
    glEnd();
}

void TheScene::renderZSlize()
{
	glBegin(GL_QUADS);
    	glTexCoord3f(0.0 - m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(-0.5, -0.5, m_zSlize -0.5);
    	glTexCoord3f(0.0 - m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(-0.5, 0.5, m_zSlize -0.5);
    	glTexCoord3f(1.0 + m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(0.5, 0.5, m_zSlize -0.5);
    	glTexCoord3f(1.0 + m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(0.5, -0.5, m_zSlize -0.5);
    glEnd();
}

void TheScene::updateView(int dir, float slize)
{
	float ratio;
	switch (dir)
	{
	case 0:
		m_xSlize = slize;
		ratio = (float)m_dataset->getColumns()/(float)m_dataset->getRows();
		if ( ratio > 0.0) 
			m_xTexture = (m_xSlize / ratio) + (1.0 - (1.0/ratio))/2.0;
		break;
	case 1:
		m_ySlize = slize;
		break;
	case 2:
		m_zSlize = slize;
		ratio = (float)m_dataset->getFrames()/(float)m_dataset->getRows();
		if ( ratio > 0.0) 
			m_zTexture = (m_zSlize / ratio) + (1.0 - (1.0/ratio))/2.0;
		break;
	}
}

