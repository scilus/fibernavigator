/*
 * ShaderHelper.cpp
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#include "ShaderHelper.h"

ShaderHelper::ShaderHelper(DatasetHelper* dh) {
	m_dh = dh;
#ifdef DEBUG
	m_dh->printTime();
	printf("initializing texture shader\n");
#endif
	m_textureShader = new Shader(wxT("GLSL/anatomy"));
	m_textureShader->bind();

	if (m_dh->GLError()) m_dh->printGLError(wxT("setup anatomy shader"));
#ifdef DEBUG
	m_dh->printTime();
	printf("initializing mesh shader\n");
#endif
	m_meshShader = new Shader(wxT("GLSL/mesh"));
	m_meshShader->bind();

	if (m_dh->GLError()) m_dh->printGLError(wxT("setup mesh shader"));
#ifdef DEBUG
	m_dh->printTime();
	printf("initializing fiber shader\n");
#endif

	m_fiberShader = new Shader(wxT("GLSL/fibers"));
	m_fiberShader->bind();

	if (m_dh->GLError()) m_dh->printGLError(wxT("setup fiber shader"));

#ifdef DEBUG
	m_dh->printTime();
	printf("initializing spline surface shader\n");
#endif
	m_splineSurfShader = new Shader(wxT("GLSL/splineSurf"));
	m_splineSurfShader->bind();

	if (m_dh->GLError()) m_dh->printGLError(wxT("setup spline surface shader"));

}

ShaderHelper::~ShaderHelper() {
	printf("execute shader helper destructor\n");
	delete m_textureShader;
	delete m_meshShader;
	delete m_fiberShader;
	printf("shader helper destructor done\n");
}


void ShaderHelper::setTextureShaderVars()
{
	int* tex = new int[10];
	int* show = new int[10];
	float* threshold = new float[10];
	int* type = new int[10];
	int c = 0;
	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if(info->getType() < Mesh_) {
			tex[c] = c;
			show[c] = info->getShow();
			threshold[c] = info->getThreshold();
			type[c] = info->getType();
			++c;
		}
	}

	m_textureShader->setUniArrayInt("texes", tex, c);
	m_textureShader->setUniArrayInt("show", show, c);
	m_textureShader->setUniArrayInt("type", type, c);
	m_textureShader->setUniArrayFloat("threshold", threshold, c);
	m_textureShader->setUniInt("countTextures", c);
}

void ShaderHelper::setMeshShaderVars()
{
	m_meshShader->setUniInt("dimX", m_dh->columns);
	m_meshShader->setUniInt("dimY", m_dh->rows);
	m_meshShader->setUniInt("dimZ", m_dh->frames);
	m_meshShader->setUniInt("cutX", (int)(m_dh->xSlize - m_dh->columns/2.0));
	m_meshShader->setUniInt("cutY", (int)(m_dh->ySlize - m_dh->rows/2.0));
	m_meshShader->setUniInt("cutZ", (int)(m_dh->zSlize - m_dh->frames/2.0));
	m_meshShader->setUniInt("sector", m_dh->quadrant);

	int* tex = new int[10];
	int* show = new int[10];
	float* threshold = new float[10];
	int* type = new int[10];
	int c = 0;
	GLuint cutTex = 0;
	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if(info->getType() < Mesh_) {
			tex[c] = c;
			show[c] = info->getShow();
			threshold[c] = info->getThreshold();
			type[c] = info->getType();
			++c;
		}
		else if (info->getType() == Surface_)
		{
			cutTex = info->getGLuint();
		}
	}

	if (cutTex != 0 && m_dh->surface_loaded)
	{
		glActiveTexture(GL_TEXTURE0 + c);
		glBindTexture(GL_TEXTURE_2D, cutTex);
		tex[c] = c;
		show[c] = false;
		threshold[c] = 0;
		type[c] = 5;
		m_meshShader->setUniInt("cutTex", c);
		++c;
	}

	m_meshShader->setUniArrayInt("texes", tex, c);
	m_meshShader->setUniArrayInt("show", show, c);
	m_meshShader->setUniArrayInt("type", type, c);
	m_meshShader->setUniArrayFloat("threshold", threshold, c);
	m_meshShader->setUniInt("countTextures", c);
}

void ShaderHelper::setFiberShaderVars()
{
	m_fiberShader->setUniInt("dimX", m_dh->columns);
	m_fiberShader->setUniInt("dimY", m_dh->rows);
	m_fiberShader->setUniInt("dimZ", m_dh->frames);

	int tex = 0;
	int show = 0;
	float threshold = 0;
	int type = 0;

	int c = 0;
	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if(info->getType() < Mesh_)
		{
			if(info->getType() == Overlay) {
				tex = c;
				show = info->getShow();
				threshold = info->getThreshold();
				type = info->getType();
			}
			++c;
		}
	}

	m_fiberShader->setUniInt("tex", tex);
	m_fiberShader->setUniInt("show", show);
	m_fiberShader->setUniInt("type", type);
	m_fiberShader->setUniFloat("threshold", threshold);
}

void ShaderHelper::setSplineSurfaceShaderVars()
{
	m_splineSurfShader->setUniInt("dimX", m_dh->columns);
	m_splineSurfShader->setUniInt("dimY", m_dh->rows);
	m_splineSurfShader->setUniInt("dimZ", m_dh->frames);

	int* tex = new int[10];
	int* show = new int[10];
	float* threshold = new float[10];
	int* type = new int[10];
	int c = 0;
	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if(info->getType() < Mesh_) {
			tex[c] = c;
			show[c] = info->getShow();
			threshold[c] = info->getThreshold();
			type[c] = info->getType();
			++c;
		}
	}

	m_splineSurfShader->setUniArrayInt("texes", tex, c);
	m_splineSurfShader->setUniArrayInt("show", show, c);
	m_splineSurfShader->setUniArrayInt("type", type, c);
	m_splineSurfShader->setUniArrayFloat("threshold", threshold, c);
	m_splineSurfShader->setUniInt("countTextures", c);
}

