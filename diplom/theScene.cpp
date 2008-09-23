#include "theScene.h"
#include "myListCtrl.h"
#include "splinePoint.h"
#include "curves.h"
#include "surface.h"
#include "selectionBox.h"
#include "AnatomyHelper.h"


TheScene::TheScene(DatasetHelper* dh)
{
	m_dh = dh;

	m_texAssigned = false;

	m_mainGLContext = 0;
	m_showBoxes = true;
	m_pointMode = false;
	m_blendAlpha = false;

	m_dh->anatomyHelper = new AnatomyHelper(m_dh);
	m_selBoxChanged = true;
}

TheScene::~TheScene()
{
	delete m_mainGLContext;
	printf("scene destructor done\n");
}

void TheScene::initGL(int view)
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  printf("Error: %s\n", glewGetErrorString(err));
	}
	if (view == mainView)
		printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	glEnable(GL_DEPTH_TEST);

	if (!m_texAssigned) {
		m_dh->shaderHelper = new ShaderHelper(m_dh);
		m_texAssigned = true;
	}

	float maxLength = (float)wxMax(m_dh->columns, wxMax(m_dh->rows, m_dh->frames));
	float view1 = maxLength/2.0;
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-view1, view1, -view1, view1, -(view1 + 5) , view1 + 5);

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
			c++;
		}
	}
	if (m_dh->GLError()) m_dh->printGLError(wxT("bind textures"));
}

void TheScene::renderScene()
{
	if (m_dh->mainFrame->m_listCtrl->GetItemCount() == 0) return;

	renderMesh();
	renderSurface();

	if (m_pointMode)
	{
		drawPoints();
	}

	if (m_dh->fibers_loaded)
	{
		renderFibers();
		if (m_dh->fibers_loaded)
		{
			if (m_showBoxes )
			{
				drawSelectionBoxes();
			}
		}
	}

	renderSlizes();

	if (m_dh->GLError()) m_dh->printGLError(wxT("render scene"));
}

void TheScene::renderSlizes()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (m_blendAlpha)
		glDisable(GL_BLEND);
	else
		glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	bindTextures();
	m_dh->shaderHelper->m_textureShader->bind();
	m_dh->shaderHelper->setTextureShaderVars();

	m_dh->anatomyHelper->renderMain();

	glDisable(GL_BLEND);

	m_dh->shaderHelper->m_textureShader->release();

	glPopAttrib();
}

void TheScene::renderSurface()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (m_blendAlpha)
		glDisable(GL_BLEND);
	else
		glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	lightsOn();

	bindTextures();
	m_dh->shaderHelper->m_meshShader->bind();
	m_dh->shaderHelper->setMeshShaderVars();

	for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() == Surface_ && info->getShow())
		{
			glColor3f(0.1f, 0.1f, 0.1f);
			m_dh->shaderHelper->m_meshShader->setUniInt("showFS", 1);
			m_dh->shaderHelper->m_meshShader->setUniInt("useTex", 1);

			info->draw();
		}
	}
	m_dh->shaderHelper->m_meshShader->release();

	lightsOff();

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw surface"));

	glPopAttrib();
}

void TheScene::renderMesh()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	lightsOn();

	bindTextures();
	m_dh->shaderHelper->m_meshShader->bind();
	m_dh->shaderHelper->setMeshShaderVars();

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

				glCallList(info->getGLuint());
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

		if (info->getType() == Curves_ && info->getShow())
		{
			lightsOff();

			if (m_dh->lighting)
			{
				lightsOn();
				//m_dh->shaderHelper->m_curveShader->bind();
			}

			if (m_selBoxChanged)
			{
				((Curves*)info)->updateLinesShown(m_dh->getSelectionBoxes());
				m_selBoxChanged = false;
			}
			info->draw();

			//m_dh->shaderHelper->m_curveShader->release();

			lightsOff();
		}
	}

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw fibers"));

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

	int countPoints = m_dh->mainFrame->m_treeWidget->GetChildrenCount(m_dh->mainFrame->m_tPointId, true);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = m_dh->mainFrame->m_treeWidget->GetNextChild(m_dh->mainFrame->m_tPointId, cookie);
		SplinePoint *point = (SplinePoint*)((MyTreeItemData*)m_dh->mainFrame->m_treeWidget->GetItemData(id))->getData();
		point->draw();
	}
	lightsOff();
	m_dh->shaderHelper->m_meshShader->release();
	glPopAttrib();

	if (m_dh->GLError()) m_dh->printGLError(wxT("draw points"));
}
