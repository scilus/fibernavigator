#include "theScene.h"
#include "myListCtrl.h"
#include "splinePoint.h"
#include "fibers.h"
#include "surface.h"
#include "selectionBox.h"
#include "AnatomyHelper.h"

TheScene::TheScene(DatasetHelper* dh)
{
	m_dh = dh;
	m_lic = new FgeImageSpaceLIC(m_dh);

	m_texAssigned = false;

	m_mainGLContext = 0;

	m_dh->anatomyHelper = new AnatomyHelper(m_dh);
	m_selBoxChanged = true;
}

TheScene::~TheScene()
{
	m_dh->printDebug(_T("execute scene destructor"), 0);

#ifndef __WXMAC__
    // on mac, this is just a pointer to the original object that is deleted with the widgets
	if (m_mainGLContext) delete m_mainGLContext;
#endif
	m_dh->printDebug(_T("scene destructor done"), 0);

}

void TheScene::initGL(int view)
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		m_dh->printDebug(_T("Error: ") + wxString::FromAscii((char*)glewGetErrorString(err)), 2);
	}
	if (view == mainView) {
		m_dh->printDebug(_T("Status: Using GLEW ") +  wxString::FromAscii((char*)glewGetString(GLEW_VERSION)), 1);
	}
	glEnable(GL_DEPTH_TEST);

	if (!m_texAssigned) {
		m_dh->shaderHelper = new ShaderHelper(m_dh);
		m_texAssigned = true;
	}

	//float maxLength = (float)wxMax(m_dh->columns, wxMax(m_dh->rows, m_dh->frames));
	//float view1 = maxLength;
	float view1 = 200;
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho( 0, view1, 0, view1, -3000, 3000);

	if (m_dh->GLError()) m_dh->printGLError(wxT("init"));
}

void TheScene::bindTextures()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	int c = 0;

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() < Mesh_)
		{
			glActiveTexture(GL_TEXTURE0 + c);
			glBindTexture(GL_TEXTURE_3D, info->getGLuint());
			if(info->getShowFS())
			{
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
			c++;
		}
	}
	if (m_dh->GLError()) m_dh->printGLError(wxT("bind textures"));
}

void TheScene::renderScene()
{
	if (m_dh->mainFrame->m_listCtrl->GetItemCount() == 0) return;

	renderSplineSurface();

	if (m_dh->pointMode)
	{
		drawPoints();
	}

	renderSlizes();

	if (m_dh->fibers_loaded)
	{
		if (m_dh->useFakeTubes)
			renderFakeTubes();
		else
			renderFibers();

		if (m_dh->fibers_loaded)
		{
			if (m_dh->showBoxes )
			{
				drawSelectionBoxes();
			}
		}
	}

	renderMesh();

	if (m_dh->showColorMapLegend)
		drawColorMapLegend();

	if (m_dh->GLError()) m_dh->printGLError(wxT("render scene"));
}

void TheScene::renderSlizes()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (m_dh->blendAlpha)
		glDisable(GL_ALPHA_TEST);
		//glDisable(GL_BLEND);
	else
		glEnable(GL_ALPHA_TEST);
		//glEnable(GL_BLEND);

	glAlphaFunc(GL_GREATER, 0.001f); // adjust your prefered threshold here

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	bindTextures();
	m_dh->shaderHelper->m_textureShader->bind();
	m_dh->shaderHelper->setTextureShaderVars();
	m_dh->shaderHelper->m_textureShader->setUniInt("useColorMap", m_dh->colorMap);

	m_dh->anatomyHelper->renderMain();

	glDisable(GL_BLEND);

	m_dh->shaderHelper->m_textureShader->release();

	glPopAttrib();
}

void TheScene::renderSplineSurface()
{
	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() == Surface_ && info->getShow())
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);

			if (m_dh->pointMode)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			bindTextures();

			lightsOn();

			m_dh->shaderHelper->m_splineSurfShader->bind();
			m_dh->shaderHelper->setSplineSurfaceShaderVars();

			wxColor c = info->getColor();
			glColor3f((float)c.Red()/255.0, (float)c.Green()/255.0, (float)c.Blue()/255.0);
			m_dh->shaderHelper->m_splineSurfShader->setUniInt("useTex", !info->getUseTex());
			m_dh->shaderHelper->m_splineSurfShader->setUniInt("useLic", info->getUseLIC());
			m_dh->shaderHelper->m_splineSurfShader->setUniInt("useColorMap", m_dh->colorMap);

			info->draw();

			m_dh->shaderHelper->m_splineSurfShader->release();

			lightsOff();

			if (m_dh->GLError()) m_dh->printGLError(wxT("draw surface"));

/*
			if ( m_dh->vectors_loaded && m_dh->drawVectors)
			{
				m_dh->shaderHelper->m_vectorShader->bind();
				info->drawVectors();
				m_dh->shaderHelper->m_vectorShader->release();
			}
*/
			glPopAttrib();
		}
	}
}

void TheScene::renderMesh()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	lightsOn();

	bindTextures();

	m_dh->shaderHelper->m_meshShader->bind();
	m_dh->shaderHelper->setMeshShaderVars();

	if (m_dh->pointMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() == Mesh_ || info->getType() == IsoSurface_)
		{
			if (info->getShow()) {
				wxColor c = info->getColor();
				glColor3f((float)c.Red()/255.0, (float)c.Green()/255.0, (float)c.Blue()/255.0);
				m_dh->shaderHelper->m_meshShader->setUniInt("showFS", info->getShowFS());
				m_dh->shaderHelper->m_meshShader->setUniInt("useTex", info->getUseTex());
				m_dh->shaderHelper->m_meshShader->setUniFloat("alpha_", info->getAlpha());
				m_dh->shaderHelper->m_meshShader->setUniInt("useColorMap", m_dh->colorMap);
				m_dh->shaderHelper->m_meshShader->setUniInt("useLic", info->getUseLIC());

				if (m_dh->surface_loaded)
					m_dh->shaderHelper->m_meshShader->setUniInt("cutAtSurface", true);
				else
					m_dh->shaderHelper->m_meshShader->setUniInt("cutAtSurface", false);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				info->draw();
			}
		}
	}
	m_dh->shaderHelper->m_meshShader->release();

	lightsOff();

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw mesh"));

	glPopAttrib();
}

void TheScene::renderFibers()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);

		if (info->getType() == Fibers_ && info->getShow())
		{
			lightsOff();

			if (m_dh->lighting)
			{
				lightsOn();
				GLfloat light_position0[] = { 1.0, 1.0, 1.0, 0.0};
				glLightfv (GL_LIGHT0, GL_POSITION, light_position0);

			}
			if (!info->getUseTex())
			{
				bindTextures();
				m_dh->shaderHelper->m_fiberShader->bind();
				m_dh->shaderHelper->setFiberShaderVars();
				m_dh->shaderHelper->m_fiberShader->setUniInt("useTex", !info->getUseTex());
				m_dh->shaderHelper->m_fiberShader->setUniInt("lightOn", m_dh->lighting);
				m_dh->shaderHelper->m_fiberShader->setUniInt("useColorMap", m_dh->colorMap);
			}
			if (m_selBoxChanged)
			{
				((Fibers*)info)->updateLinesShown(m_dh->getSelectionBoxes());
				m_selBoxChanged = false;
			}
			info->draw();

			m_dh->shaderHelper->m_fiberShader->release();

			lightsOff();
		}
	}

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw fibers"));

	glPopAttrib();
}

void TheScene::renderFakeTubes()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);

		if (info->getType() == Fibers_ && info->getShow())
		{
			if (m_selBoxChanged)
			{
				((Fibers*)info)->updateLinesShown(m_dh->getSelectionBoxes());
				m_selBoxChanged = false;
			}

			m_dh->shaderHelper->m_fakeTubeShader->bind();
			m_dh->shaderHelper->m_fakeTubeShader->setUniInt("globalColor", info->getShowFS());

			info->draw();

			m_dh->shaderHelper->m_fakeTubeShader->release();
		}
	}

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw fake tubes"));

	glPopAttrib();
}

void TheScene::lightsOn()
{
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat light_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat specref[] = { 0.5, 0.5, 0.5, 0.5};

	Vector3fT v1 = {{0,0,-1}};
	Vector3fT l;
	Vector3fMultMat4(&l, &v1, &m_dh->m_transform);

	GLfloat light_position0[] = { l.s.X, l.s.Y, l.s.Z, 0.0};

	glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv (GL_LIGHT0, GL_POSITION, light_position0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specref);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 32);

	if (m_dh->GLError()) m_dh->printGLError(wxT("setup lights"));
}

void TheScene::lightsOff()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
}


void TheScene::renderNavView(int view)
{
	if (m_dh->mainFrame->m_listCtrl->GetItemCount() == 0) return;

	m_dh->anatomyHelper->renderNav(view, m_dh->shaderHelper->m_textureShader);

	if (m_dh->GLError()) m_dh->printGLError(wxT("render nav view"));
}

void TheScene::drawSphere(float x, float y, float z, float r)
{
	glPushMatrix();
	glTranslatef(x,y,z);
	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluSphere(quadric, r, 32, 32);
	glPopMatrix();

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw sphere"));
}



void TheScene::drawSelectionBoxes()
{
	std::vector<std::vector<SelectionBox*> > boxes = m_dh->getSelectionBoxes();
	for (unsigned int i = 0 ; i < boxes.size() ; ++i)
	{
		for (unsigned int j = 0 ; j < boxes[i].size() ; ++j)
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);

			boxes[i][j]->draw();

			glPopAttrib();
		}
	}

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw selection boxes"));
}

void TheScene::drawPoints()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	lightsOn();
	m_dh->shaderHelper->m_meshShader->bind();
	m_dh->shaderHelper->setMeshShaderVars();
	m_dh->shaderHelper->m_meshShader->setUniInt("showFS", true);
	m_dh->shaderHelper->m_meshShader->setUniInt("useTex", false);
	m_dh->shaderHelper->m_meshShader->setUniInt("cutAtSurface", false);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	id = m_dh->mainFrame->m_treeWidget->GetFirstChild(m_dh->mainFrame->m_tPointId, cookie);
	while ( id.IsOk() )
	{
		SplinePoint *point = (SplinePoint*)(m_dh->mainFrame->m_treeWidget->GetItemData(id));
		point->draw();

		id = m_dh->mainFrame->m_treeWidget->GetNextChild(m_dh->mainFrame->m_tPointId, cookie);
	}

	lightsOff();
	m_dh->shaderHelper->m_meshShader->release();
	glPopAttrib();

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw points"));
}

void TheScene::drawColorMapLegend()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	int size = wxMax(wxMax(m_dh->rows, m_dh->columns), m_dh->frames);
	glOrtho( 0, size, 0, size, -3000, 3000);

	m_dh->shaderHelper->m_legendShader->bind();
	m_dh->shaderHelper->m_legendShader->setUniInt("useColorMap", m_dh->colorMap);

	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(5.0);
	glBegin(GL_LINES);
		glTexCoord1f(0.0);
		glVertex3i(size - 60, 10, 2900);
		glTexCoord1f(1.0);
		glVertex3i(size - 20, 10, 2900);
	glEnd();

	m_dh->shaderHelper->m_legendShader->release();

	glLineWidth(1.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
		glVertex3i(size - 60, 10, 2900);
		glVertex3i(size - 60, 12, 2900);
		glVertex3i(size - 50, 10, 2900);
		glVertex3i(size - 50, 12, 2900);
		glVertex3i(size - 40, 10, 2900);
		glVertex3i(size - 40, 12, 2900);
		glVertex3i(size - 30, 10, 2900);
		glVertex3i(size - 30, 12, 2900);
		glVertex3i(size - 20, 10, 2900);
		glVertex3i(size - 20, 12, 2900);
	glEnd();

	m_dh->shaderHelper->m_legendShader->release();

	glPopMatrix();
	glPopAttrib();
}
