/*
 * ShaderHelper.cpp
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#include "ShaderHelper.h"

ShaderHelper::ShaderHelper(DatasetHelper* dh) {
	m_dh = dh;
	/*
	wxString vShaderModules;
	m_dh->loadTextFile(&vShaderModules, wxT("GLSL/lighting.vs"));
	wxString fShaderModules;
	m_dh->loadTextFile(&fShaderModules, wxT("GLSL/lighting.fs"));
*/
	printf("initializing  texture shader\n");

	m_textureShader = new Shader(wxT("GLSL/anatomy"));
	m_textureShader->bind();

	if (m_dh->GLError()) m_dh->printGLError(wxT("setup shader 1"));


	printf("initializing mesh shader\n");

	m_meshShader = new Shader(wxT("GLSL/mesh"));
	m_meshShader->bind();

	if (m_dh->GLError()) m_dh->printGLError(wxT("setup shader 2"));


	printf("initializing curves shader\n");


	m_curveShader = new Shader(wxT("GLSL/fibers"));
	m_curveShader->bind();

	if (m_dh->GLError()) m_dh->printGLError(wxT("setup shader 3"));

}

ShaderHelper::~ShaderHelper() {
	delete m_textureShader;
	delete m_meshShader;
	delete m_curveShader;
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

	m_meshShader->setUniArrayInt("texes", tex, c);
	m_meshShader->setUniArrayInt("show", show, c);
	m_meshShader->setUniArrayInt("type", type, c);
	m_meshShader->setUniArrayFloat("threshold", threshold, c);
	m_meshShader->setUniInt("countTextures", c);
}
