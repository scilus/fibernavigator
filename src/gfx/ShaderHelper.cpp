/*
 * ShaderHelper.cpp
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#include "ShaderHelper.h"
#include "../dataset/DatasetHelper.h"
#include "../dataset/surface.h"

ShaderHelper::ShaderHelper(DatasetHelper* dh)
:   m_dh(dh)
{
    tex.resize(10);
    show.resize(10);
    type.resize(10);
    threshold.resize(10);
    alpha.resize(10);

	m_dh->printDebug(_T("initializing texture shader"), 1);
	m_textureShader = new Shader(wxT("anatomy"));
	m_textureShader->bind();
	if (m_dh->GLError()) m_dh->printGLError(wxT("setup anatomy shader"));

	m_dh->printDebug(_T("initializing mesh shader"), 1);
	m_meshShader = new Shader(wxT("mesh"));
	m_meshShader->bind();
	if (m_dh->GLError()) m_dh->printGLError(wxT("setup mesh shader"));

	m_dh->printDebug(_T("initializing fiber shader"), 1);
	m_fiberShader = new Shader(wxT("fibers"));
	m_fiberShader->bind();
	if (m_dh->GLError()) m_dh->printGLError(wxT("setup fiber shader"));

	m_dh->printDebug(_T("initializing fake tube shader"), 1);
	m_fakeTubeShader = new Shader(wxT("fake-tubes"));
	m_fakeTubeShader->bind();
	if (m_dh->GLError()) m_dh->printGLError(wxT("setup fake tube shader"));

	m_dh->printDebug(_T("initializing spline surface shader"), 1);
	m_splineSurfShader = new Shader(wxT("splineSurf"));
	m_splineSurfShader->bind();
	if (m_dh->GLError()) m_dh->printGLError(wxT("setup spline surface shader"));

	m_dh->printDebug(_T("initializing vector shader"), 1);
	m_vectorShader = new Shader(wxT("vectors"));
	m_vectorShader->bind();
	if (m_dh->GLError()) m_dh->printGLError(wxT("setup vectors shader"));

	m_dh->printDebug(_T("initializing legend shader"), 1);
	m_legendShader = new Shader(wxT("legend"));
	m_legendShader->bind();
	if (m_dh->GLError()) m_dh->printGLError(wxT("setup legend shader"));
}

ShaderHelper::~ShaderHelper() {
	m_dh->printDebug(_T("execute shader helper destructor\n"), 0);
	delete m_textureShader;
	delete m_meshShader;
	delete m_fiberShader;
	delete m_splineSurfShader;
	delete m_fakeTubeShader;

	m_dh->printDebug(_T("shader helper destructor done\n"), 0);
}

void ShaderHelper::initializeArrays()
{
    m_textureCount = 0;
    m_cutTex = 0;

    for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
    {
        DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
        if(info->getType() < Mesh_)
        {
            tex[m_textureCount]         = m_textureCount;
            show[m_textureCount]        = info->getShow();
            threshold[m_textureCount]   = info->getThreshold();
            alpha[m_textureCount]       = info->getAlpha();
            type[m_textureCount]        = info->getType();
            ++m_textureCount;
        }
        if ( m_textureCount == 10 ) break;
    }
}

void ShaderHelper::setTextureShaderVars()
{
	m_textureShader->setUniArrayInt("texes", &tex[0], m_textureCount);
	m_textureShader->setUniArrayInt("show", &show[0], m_textureCount);
	m_textureShader->setUniArrayInt("type", &type[0], m_textureCount);
	m_textureShader->setUniArrayFloat("threshold", &threshold[0], m_textureCount);
	m_textureShader->setUniArrayFloat("alpha", &alpha[0], m_textureCount);

}

void ShaderHelper::setMeshShaderVars()
{
	m_meshShader->setUniInt("dimX", m_dh->columns);
	m_meshShader->setUniInt("dimY", m_dh->rows);
	m_meshShader->setUniInt("dimZ", m_dh->frames);
	m_meshShader->setUniFloat("voxX", m_dh->xVoxel);
	m_meshShader->setUniFloat("voxY", m_dh->yVoxel);
	m_meshShader->setUniFloat("voxZ", m_dh->zVoxel);
	m_meshShader->setUniInt("blendTex", m_dh->blendTexOnMesh);
	m_meshShader->setUniFloat("cutX", m_dh->xSlize + 0.5f);
	m_meshShader->setUniFloat("cutY", m_dh->ySlize + 0.5f);
	m_meshShader->setUniFloat("cutZ", m_dh->zSlize + 0.5f);
	m_meshShader->setUniInt("sector", m_dh->quadrant);

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	    {
	        DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);

	        if (info->getType() == Surface_)
            {
                Surface* s = (Surface*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
                m_cutTex = s->getCutTex();

                glActiveTexture(GL_TEXTURE0 + 9);
                glBindTexture(GL_TEXTURE_2D, m_cutTex);
                tex[9] = 9;
                show[9] = false;
                threshold[9] = 0;
                type[9] = 5;
                m_meshShader->setUniInt("cutTex", 9);
            }
	    }

	m_meshShader->setUniArrayInt("texes", &tex[0], 10);
	m_meshShader->setUniArrayInt("show", &show[0], 10);
	m_meshShader->setUniArrayInt("type", &type[0], 10);
	m_meshShader->setUniArrayFloat("threshold", &threshold[0], 10);
	m_meshShader->setUniArrayFloat("alpha", &alpha[0], 10);

}

void ShaderHelper::setFiberShaderVars()
{
	m_fiberShader->setUniInt("dimX", m_dh->columns);
	m_fiberShader->setUniInt("dimY", m_dh->rows);
	m_fiberShader->setUniInt("dimZ", m_dh->frames);
	m_fiberShader->setUniFloat("voxX", m_dh->xVoxel);
    m_fiberShader->setUniFloat("voxY", m_dh->yVoxel);
    m_fiberShader->setUniFloat("voxZ", m_dh->zVoxel);

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
			if( (info->getType() == Overlay) && info->getShow() ) {
				tex = c;
				show = info->getShow();
				threshold = info->getThreshold();
				type = info->getType();
				break;
			}
			++c;
		}
		if ( c == 10 ) break;
	}

	m_fiberShader->setUniInt("tex", tex);
	m_fiberShader->setUniInt("type", type);
	m_fiberShader->setUniFloat("threshold", threshold);
}

void ShaderHelper::setSplineSurfaceShaderVars()
{
	m_splineSurfShader->setUniInt("dimX", m_dh->columns);
	m_splineSurfShader->setUniInt("dimY", m_dh->rows);
	m_splineSurfShader->setUniInt("dimZ", m_dh->frames);
	m_splineSurfShader->setUniFloat("voxX", m_dh->xVoxel);
	m_splineSurfShader->setUniFloat("voxY", m_dh->yVoxel);
	m_splineSurfShader->setUniFloat("voxZ", m_dh->zVoxel);

	m_splineSurfShader->setUniArrayInt("texes", &tex[0], m_textureCount);
	m_splineSurfShader->setUniArrayInt("show", &show[0], m_textureCount);
	m_splineSurfShader->setUniArrayInt("type", &type[0], m_textureCount);
	m_splineSurfShader->setUniArrayFloat("threshold", &threshold[0], m_textureCount);
	m_splineSurfShader->setUniArrayFloat("alpha", &alpha[0], m_textureCount);
}

