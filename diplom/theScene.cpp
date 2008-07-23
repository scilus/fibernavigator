#include "theScene.h"
#include "myListCtrl.h"
#include "point.h"
#include "curves.h"
#include "theDataset.h"
#include "surface.h"
#include "selectionBox.h"


TheScene::TheScene()
{
	m_texAssigned = false;

	m_mainGLContext = 0;
	m_xSlize = 0.5;
	m_ySlize = 0.5;
	m_zSlize = 0.5;
	m_showSagittal = true;
	m_showCoronal = true;
	m_showAxial = true;
	m_showMesh = true;
	m_showBoxes = true;
	m_pointMode = false;
	m_blendAlpha = false;
	m_textureShader = 0;
	m_meshShader = 0;
	m_curveShader = 0;

	m_xOffset0 = 0.0;
	m_yOffset0 = 0.0;
	m_xOffset1 = 0.0;
	m_yOffset1 = 0.0;
	m_xOffset2 = 0.0;
	m_yOffset2 = 0.0;

	m_quadrant = 1;
	Vector3fT v1 = {{1.0,1.0,1.0}};
	m_lightPos = v1;

	m_selBoxChanged = true;
}

TheScene::~TheScene()
{

}

void TheScene::initGL(int view)
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  printf("Error: %s\n", glewGetErrorString(err));
	}
	(view == mainView) ? printf("Main View: ") : printf("Nav View: %d ", view);
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	glEnable(GL_DEPTH_TEST);

	if (!m_texAssigned) {
		initShaders();
		m_texAssigned = true;
	}

	float maxLength = (float)wxMax(TheDataset::columns, wxMax(TheDataset::rows, TheDataset::frames));
	float view1 = maxLength/2.0;
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-view1, view1, -view1, view1, -(view1 + 5) , view1 + 5);

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("init"));
}

void TheScene::bindTextures()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	int c = 0;

	for (int i = 0 ; i < TheDataset::mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)TheDataset::mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() < Mesh_)
		{
			glActiveTexture(GL_TEXTURE0 + c);
			glBindTexture(GL_TEXTURE_3D, info->getGLuint());
			c++;
		}
	}
	if (TheDataset::GLError()) TheDataset::printGLError(wxT("bind textures"));
}



void TheScene::initShaders()
{
	if (m_textureShader)
	{
		delete m_textureShader;
	}
	printf("initializing  texture shader\n");

	GLSLShader *vShader = new GLSLShader(GL_VERTEX_SHADER);
	GLSLShader *fShader = new GLSLShader(GL_FRAGMENT_SHADER);

	vShader->loadCode(wxT("GLSL/anatomy.vs"));
	fShader->loadCode(wxT("GLSL/anatomy.fs"));

	m_textureShader = new FGLSLShaderProgram();
	m_textureShader->link(vShader, fShader);
	m_textureShader->bind();

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("setup shader 1"));


	if (m_meshShader)
	{
		delete m_meshShader;
	}
	printf("initializing mesh shader\n");

	GLSLShader *vShader1 = new GLSLShader(GL_VERTEX_SHADER);
	GLSLShader *fShader1 = new GLSLShader(GL_FRAGMENT_SHADER);

	vShader1->loadCode(wxT("GLSL/mesh.vs"));
	fShader1->loadCode(wxT("GLSL/mesh.fs"));

	m_meshShader = new FGLSLShaderProgram();
	m_meshShader->link(vShader1, fShader1);
	m_meshShader->bind();

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("setup shader 2"));

	if (m_curveShader)
	{
		delete m_curveShader;
	}
	printf("initializing curves shader\n");

	GLSLShader *vShader2 = new GLSLShader(GL_VERTEX_SHADER);
	GLSLShader *fShader2 = new GLSLShader(GL_FRAGMENT_SHADER);

	vShader2->loadCode(wxT("GLSL/fibers.vs"));
	fShader2->loadCode(wxT("GLSL/fibers.fs"));

	m_curveShader = new FGLSLShaderProgram();
	m_curveShader->link(vShader2, fShader2);
	m_curveShader->bind();

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("setup shader 3"));
}

void TheScene::setTextureShaderVars()
{
	int* tex = new int[10];
	int* show = new int[10];
	float* threshold = new float[10];
	int* type = new int[10];
	int c = 0;
	for (int i = 0 ; i < TheDataset::mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)TheDataset::mainFrame->m_listCtrl->GetItemData(i);
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

void TheScene::setMeshShaderVars()
{
	m_meshShader->setUniInt("dimX", TheDataset::columns);
	m_meshShader->setUniInt("dimY", TheDataset::rows);
	m_meshShader->setUniInt("dimZ", TheDataset::frames);
	m_meshShader->setUniInt("cutX", (int)(m_xSlize - TheDataset::columns/2.0));
	m_meshShader->setUniInt("cutY", (int)(m_ySlize - TheDataset::rows/2.0));
	m_meshShader->setUniInt("cutZ", (int)(m_zSlize - TheDataset::frames/2.0));
	m_meshShader->setUniInt("sector", m_quadrant);

	int* tex = new int[10];
	int* show = new int[10];
	float* threshold = new float[10];
	int* type = new int[10];
	int c = 0;
	for (int i = 0 ; i < TheDataset::mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)TheDataset::mainFrame->m_listCtrl->GetItemData(i);
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

void TheScene::renderScene()
{
	if (TheDataset::mainFrame->m_listCtrl->GetItemCount() == 0) return;

	renderSlizes();

	renderMesh();
	renderSurface();

	if (m_showBoxes && TheDataset::fibers_loaded)
	{
		renderFibers();
		drawSelectionBoxes();
	}
	if (m_pointMode)
	{
		drawPoints();
	}

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("render"));
}

void TheScene::renderSlizes()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (m_blendAlpha)
		glDisable(GL_ALPHA_TEST);
	else
		glEnable(GL_ALPHA_TEST);

	glAlphaFunc(GL_GREATER, 0.0000001);

	bindTextures();
	m_textureShader->bind();
	setTextureShaderVars();

	if (m_showSagittal) renderXSlize();
	if (m_showCoronal) renderYSlize();
	if (m_showAxial) renderZSlize();

	m_textureShader->release();

	glPopAttrib();
}


void TheScene::renderXSlize()
{
	int x = m_xSlize - TheDataset::columns/2;
	int y = TheDataset::rows/2;
	int z = TheDataset::frames/2;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xSlize/(float)TheDataset::columns, 0.0, 0.0); glVertex3i(x, -y, -z);
    	glTexCoord3f(m_xSlize/(float)TheDataset::columns, 0.0, 1.0); glVertex3i(x, -y,  z);
    	glTexCoord3f(m_xSlize/(float)TheDataset::columns, 1.0, 1.0); glVertex3i(x,  y,  z);
    	glTexCoord3f(m_xSlize/(float)TheDataset::columns, 1.0, 0.0); glVertex3i(x,  y, -z);
    glEnd();

    if (TheDataset::GLError()) TheDataset::printGLError(wxT("render X slize"));
}

void TheScene::renderYSlize()
{
	int x = TheDataset::columns/2;
	int y = m_ySlize - TheDataset::rows/2;
	int z = TheDataset::frames/2;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, m_ySlize/(float)TheDataset::rows, 0.0); glVertex3i(-x, y, -z);
    	glTexCoord3f(0.0, m_ySlize/(float)TheDataset::rows, 1.0); glVertex3i(-x, y,  z);
    	glTexCoord3f(1.0, m_ySlize/(float)TheDataset::rows, 1.0); glVertex3i( x, y,  z);
    	glTexCoord3f(1.0, m_ySlize/(float)TheDataset::rows, 0.0); glVertex3i( x, y, -z);
    glEnd();

    if (TheDataset::GLError()) TheDataset::printGLError(wxT("render Y slize"));
}

void TheScene::renderZSlize()
{
	int x = TheDataset::columns/2;
	int y = TheDataset::rows/2;
	int z = m_zSlize - TheDataset::frames/2;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, 0.0, m_zSlize/(float)TheDataset::frames); glVertex3i(-x, -y, z);
    	glTexCoord3f(0.0, 1.0, m_zSlize/(float)TheDataset::frames); glVertex3i(-x,  y, z);
    	glTexCoord3f(1.0, 1.0, m_zSlize/(float)TheDataset::frames); glVertex3i( x,  y, z);
    	glTexCoord3f(1.0, 0.0, m_zSlize/(float)TheDataset::frames); glVertex3i( x, -y, z);
    glEnd();

    if (TheDataset::GLError()) TheDataset::printGLError(wxT("render Z slize"));
}

void TheScene::setupLights()
{
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat light_specular[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat specref[] = { 0.5, 0.5, 0.5, 0.5};

	GLfloat light_position0[] = { -m_lightPos.s.X, -m_lightPos.s.Y, -m_lightPos.s.Z, 0.0};

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

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("setup lights"));
}

void TheScene::switchOffLights()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
}

void TheScene::renderMesh()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	setupLights();

	bindTextures();
	m_meshShader->bind();
	setMeshShaderVars();

	for (int i = 0 ; i < TheDataset::mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)TheDataset::mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() == Mesh_)
		{
			if (info->getShow()) {
				float c = (float)info->getThreshold();
				glColor3f(c,c,c);
				m_meshShader->setUniInt("showFS", info->getShowFS());
				m_meshShader->setUniInt("useTex", info->getUseTex());

				glCallList(info->getGLuint());
			}
		}
	}
	m_meshShader->release();

	switchOffLights();

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("draw mesh"));

	glPopAttrib();
}

void TheScene::renderFibers()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glEnable(GL_BLEND);
	m_curveShader->bind();
	for (int i = 0 ; i < TheDataset::mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)TheDataset::mainFrame->m_listCtrl->GetItemData(i);

		m_curveShader->setUniInt("useNormals", !info->getShowFS());
		if (info->getType() == Curves_ && info->getShow())
		{
			if (m_selBoxChanged)
			{
				((Curves*)info)->updateLinesShown(TheDataset::getSelectionBoxes());
				m_selBoxChanged = false;
			}
			info->draw();
		}
	}
	m_curveShader->release();

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("draw fibers"));

	glPopAttrib();
}

void TheScene::renderSurface()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	setupLights();

	m_meshShader->bind();
	setMeshShaderVars();

	for (int i = 0 ; i < TheDataset::mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)TheDataset::mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() == Surface_ && info->getShow())
		{
			float c = (float)info->getThreshold();
			glColor3f(c,c,c);
			m_meshShader->setUniInt("showFS", info->getShowFS());
			m_meshShader->setUniInt("useTex", info->getUseTex());

			info->draw();
		}
	}
	m_meshShader->release();

	switchOffLights();

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("draw surface"));

	glPopAttrib();
}

void TheScene::renderNavView(int view)
{
	if (TheDataset::mainFrame->m_listCtrl->GetItemCount() == 0) return;

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0000001);

	bindTextures();
	m_textureShader->bind();
	setTextureShaderVars();

	float xline = 0;
	float yline = 0;

	float border = (float)wxMax(TheDataset::columns, wxMax(TheDataset::rows, TheDataset::frames))/2.0;

	int x = TheDataset::columns/2;
	int y = TheDataset::rows/2;
	int z = TheDataset::frames/2;

	int xs = m_xSlize - TheDataset::columns/2;
	int ys = m_ySlize - TheDataset::rows/2;
	int zs = m_zSlize - TheDataset::frames/2;

	switch (view)
	{
		case axial: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, 1.0, m_zSlize/(float)TheDataset::frames); glVertex3f(-x, -y, zs);
		    	glTexCoord3f(0.0, 0.0, m_zSlize/(float)TheDataset::frames); glVertex3f(-x,  y, zs);
		    	glTexCoord3f(1.0, 0.0, m_zSlize/(float)TheDataset::frames); glVertex3f( x,  y, zs);
		    	glTexCoord3f(1.0, 1.0, m_zSlize/(float)TheDataset::frames); glVertex3f( x, -y, zs);
			glEnd();
			xline = m_xSlize - (float)TheDataset::columns/2.0;
			yline = (float)TheDataset::rows/2.0 - m_ySlize;
		} break;

		case coronal: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, m_ySlize/(float)TheDataset::rows, 1.0); glVertex3f( -x, -z, ys);
		    	glTexCoord3f(0.0, m_ySlize/(float)TheDataset::rows, 0.0); glVertex3f( -x,  z, ys);
		    	glTexCoord3f(1.0, m_ySlize/(float)TheDataset::rows, 0.0); glVertex3f(  x,  z, ys);
		    	glTexCoord3f(1.0, m_ySlize/(float)TheDataset::rows, 1.0); glVertex3f(  x, -z, ys);
		    glEnd();
		    xline = m_xSlize - (float)TheDataset::columns/2.0;
		    yline = (float)TheDataset::frames/2.0 - m_zSlize;
		} break;

		case sagittal: {
			glBegin(GL_QUADS);
				glTexCoord3f(m_xSlize/(float)TheDataset::columns, 0.0, 1.0); glVertex3f(-y, -z, xs);
		    	glTexCoord3f(m_xSlize/(float)TheDataset::columns, 0.0, 0.0); glVertex3f(-y,  z, xs);
		    	glTexCoord3f(m_xSlize/(float)TheDataset::columns, 1.0, 0.0); glVertex3f( y,  z, xs);
		    	glTexCoord3f(m_xSlize/(float)TheDataset::columns, 1.0, 1.0); glVertex3f( y, -z, xs);
			glEnd();
			xline = m_ySlize - (float)TheDataset::rows/2.0;
			yline = (float)TheDataset::frames/2.0 - m_zSlize;
		} break;
	}

	glDisable(GL_TEXTURE_3D);

	m_textureShader->release();

	glPopAttrib();

	glColor3f(1.0, 0.0, 0.0);
	glBegin (GL_LINES);
		glVertex3f (-border, yline, border);
		glVertex3f ( border, yline, border);
		glVertex3f (xline, -border, border);
		glVertex3f (xline,  border, border);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("render nav view"));
}

void TheScene::drawSphere(float x, float y, float z, float r)
{
	glPushMatrix();
	glTranslatef(x,y,z);
	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluSphere(quadric, r, 32, 32);
	glPopMatrix();

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("draw sphere"));
}



void TheScene::drawSelectionBoxes()
{
	std::vector<std::vector<SelectionBox*> > boxes = TheDataset::getSelectionBoxes();
	for (uint i = 0 ; i < boxes.size() ; ++i)
	{
		for (uint j = 0 ; j < boxes[i].size() ; ++j)
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);

			setupLights();
			m_meshShader->bind();
			setMeshShaderVars();
			m_meshShader->setUniInt("showFS", true);
			m_meshShader->setUniInt("useTex", false);

			boxes[i][j]->drawHandles();
			switchOffLights();

			m_meshShader->release();
			boxes[i][j]->drawFrame();
			glPopAttrib();
		}
	}

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("draw selection boxes"));
}

void TheScene::drawPoints()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	setupLights();
	m_meshShader->bind();
	setMeshShaderVars();
	m_meshShader->setUniInt("showFS", true);
	m_meshShader->setUniInt("useTex", false);

	std::vector< std::vector< double > > givenPoints;
	int countPoints = TheDataset::mainFrame->m_treeWidget->GetChildrenCount(TheDataset::mainFrame->m_tPointId, true);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = TheDataset::mainFrame->m_treeWidget->GetNextChild(TheDataset::mainFrame->m_tPointId, cookie);
		Point *point = (Point*)((MyTreeItemData*)TheDataset::mainFrame->m_treeWidget->GetItemData(id))->getData();
		point->draw();
		std::vector< double > p;
		p.push_back(point->getCenter().s.X);
		p.push_back(point->getCenter().s.Y);
		p.push_back(point->getCenter().s.Z);
		givenPoints.push_back(p);
	}
	switchOffLights();
	m_meshShader->release();
	glPopAttrib();

	if (TheDataset::GLError()) TheDataset::printGLError(wxT("draw points"));
}
