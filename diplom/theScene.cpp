#include "theScene.h"

TheScene::TheScene()
{
	m_countTextures = 0;
	
	m_texAssigned = false;
	
	m_mainGLContext = 0;
	m_texNames = new GLuint[10];
	m_xSlize = 0.5;
	m_ySlize = 0.5;
	m_zSlize = 0.5;
	m_showXSlize = true;
	m_showYSlize = true;
	m_showZSlize = true;
	m_showMesh = true;
	m_showSelBox = true;
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
	Vector3fT v1 = {0,0,1};
	m_lightPos = v1;
	Vector3fT v2 = {0,0,0};
	m_selBoxCenter = v2;
	Vector3fT v3 = {TheDataset::columns,TheDataset::rows, TheDataset::frames};
	m_selBoxSize = v3;
}

TheScene::~TheScene()
{
	glDeleteTextures(10, m_texNames);
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

	glShadeModel(GL_FLAT);
	glEnable(GL_DOUBLEBUFFER);
	glEnable(GL_DEPTH_TEST);
	
	GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	
	glAlphaFunc(GL_GREATER, 0.0000001); // adjust your prefered threshold here
	glEnable(GL_ALPHA_TEST);
	
	if (!m_texAssigned) {
		assignTextures();
		initShaders();
		m_texAssigned = true;
	}
	
	float maxLength = (float)wxMax(TheDataset::columns, wxMax(TheDataset::rows, TheDataset::frames));
	float view1 = maxLength/2.0;
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-view1, view1, -view1, view1, -(view1 + 5) , view1 + 5);	
}

void TheScene::assignTextures ()
{
	printf("assign textures\n");
	glDeleteTextures(10, m_texNames);
	m_countTextures = m_listctrl->GetItemCount();
	
	if (m_countTextures == 0) return;
	
	
	for (int i = 0 ; i < m_countTextures ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_listctrl->GetItemData(i);
		if(info->getType() >= Mesh_)
		{
			m_texNames[i] = makeCallList(info);
			return;
		}
		glActiveTexture(GL_TEXTURE0 + i);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1, &m_texNames[i]);
		glBindTexture(GL_TEXTURE_3D, m_texNames[i]);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		info->generateTexture();
	}
	
	Vector3fT v3 = {TheDataset::columns/2,TheDataset::rows/2, TheDataset::frames/2};
	m_selBoxSize = v3;
}

void TheScene::addTexture()
{
	m_countTextures = m_listctrl->GetItemCount();
	if (m_countTextures == 0) return;
	DatasetInfo* info = (DatasetInfo*)m_listctrl->GetItemData(m_countTextures - 1);
	if(info->getType() >= Mesh_)
	{
		m_texNames[m_countTextures -1] = makeCallList(info);
		return;
	}
	glActiveTexture(GL_TEXTURE0 + m_countTextures -1);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &m_texNames[m_countTextures -1]);
	glBindTexture(GL_TEXTURE_3D, m_texNames[m_countTextures -1]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	info->generateTexture();
}

void TheScene::bindTextures()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	
	for (int i = 0 ; i < m_countTextures ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_listctrl->GetItemData(i);
		if (info->getType() < Mesh_) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_3D, m_texNames[i]);
		}
	}
}

void TheScene::swapTextures(int a, int b)
{
	GLuint temp = m_texNames[a];
	m_texNames[a] = m_texNames[b];
	m_texNames[b] = temp;
}

void TheScene::releaseTextures()
{
	glDeleteTextures(10, m_texNames);
}

GLuint TheScene::makeCallList(DatasetInfo *info)
{
	int x = TheDataset::columns/2;
	int y = TheDataset::rows/2;
	int z = TheDataset::frames/2;
	
	GLuint mesh = glGenLists(1);
	glNewList (mesh, GL_COMPILE);
	
	info->generateGeometry(x,y,z);
	
	glEndList();
	
	return mesh;
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
	
	vShader->loadCode(wxT("GLSL/v1.glsl"));
	fShader->loadCode(wxT("GLSL/f1.glsl"));
	
	m_textureShader = new FGLSLShaderProgram();
	m_textureShader->link(vShader, fShader);
	m_textureShader->bind();
	

	if (m_meshShader)
	{
		delete m_meshShader;
	}
	printf("initializing mesh shader\n");

	GLSLShader *vShader1 = new GLSLShader(GL_VERTEX_SHADER);
	GLSLShader *fShader1 = new GLSLShader(GL_FRAGMENT_SHADER);
	
	vShader1->loadCode(wxT("GLSL/v2.glsl"));
	fShader1->loadCode(wxT("GLSL/f2.glsl"));
	
	m_meshShader = new FGLSLShaderProgram();
	m_meshShader->link(vShader1, fShader1);
	m_meshShader->bind();
	
	if (m_curveShader)
	{
		delete m_curveShader;
	}
	printf("initializing curves shader\n");

	GLSLShader *vShader2 = new GLSLShader(GL_VERTEX_SHADER);
	GLSLShader *fShader2 = new GLSLShader(GL_FRAGMENT_SHADER);
	
	vShader2->loadCode(wxT("GLSL/v3.glsl"));
	fShader2->loadCode(wxT("GLSL/f3.glsl"));
	
	m_curveShader = new FGLSLShaderProgram();
	m_curveShader->link(vShader2, fShader2);
	m_curveShader->bind();
}

void TheScene::setTextureShaderVars()
{
	m_textureShader->bind();

	DatasetInfo* info;
	switch (m_countTextures)
	{
	case 10:
		info = (DatasetInfo*)m_listctrl->GetItemData(9);
		m_textureShader->setUniInt("tex9", 9);
		m_textureShader->setUniInt("show9", info->getShow());
		m_textureShader->setUniFloat("threshold9",  info->getThreshold());
		m_textureShader->setUniInt("type9", info->getType());
	case 9:
		info = (DatasetInfo*)m_listctrl->GetItemData(8);
		m_textureShader->setUniInt("tex8", 8);
		m_textureShader->setUniInt("show8", info->getShow());
		m_textureShader->setUniFloat("threshold8",  info->getThreshold());
		m_textureShader->setUniInt("type8", info->getType());
	case 8:
		info = (DatasetInfo*)m_listctrl->GetItemData(7);
		m_textureShader->setUniInt("tex7", 7);
		m_textureShader->setUniInt("show7", info->getShow());
		m_textureShader->setUniFloat("threshold7",  info->getThreshold());
		m_textureShader->setUniInt("type7", info->getType());
	case 7:
		info = (DatasetInfo*)m_listctrl->GetItemData(6);
		m_textureShader->setUniInt("tex6", 6);
		m_textureShader->setUniInt("show6", info->getShow());
		m_textureShader->setUniFloat("threshold6",  info->getThreshold());
		m_textureShader->setUniInt("type6", info->getType());
	case 6:
		info = (DatasetInfo*)m_listctrl->GetItemData(5);
		m_textureShader->setUniInt("tex5", 5);
		m_textureShader->setUniInt("show5", info->getShow());
		m_textureShader->setUniFloat("threshold5",  info->getThreshold());
		m_textureShader->setUniInt("type5", info->getType());
	case 5:
		info = (DatasetInfo*)m_listctrl->GetItemData(4);
		m_textureShader->setUniInt("tex4", 4);
		m_textureShader->setUniInt("show4", info->getShow());
		m_textureShader->setUniFloat("threshold4",  info->getThreshold());
		m_textureShader->setUniInt("type4", info->getType());
	case 4:
		info = (DatasetInfo*)m_listctrl->GetItemData(3);
		m_textureShader->setUniInt("tex3", 3);
		m_textureShader->setUniInt("show3", info->getShow());
		m_textureShader->setUniFloat("threshold3",  info->getThreshold());
		m_textureShader->setUniInt("type3", info->getType());
	case 3:
		info = (DatasetInfo*)m_listctrl->GetItemData(2);
		m_textureShader->setUniInt("tex2", 2);
		m_textureShader->setUniInt("show2", info->getShow());
		m_textureShader->setUniFloat("threshold2",  info->getThreshold());
		m_textureShader->setUniInt("type2", info->getType());
	case 2:
		info = (DatasetInfo*)m_listctrl->GetItemData(1);
		m_textureShader->setUniInt("tex1", 1);
		m_textureShader->setUniInt("show1", info->getShow());
		m_textureShader->setUniFloat("threshold1",  info->getThreshold());
		m_textureShader->setUniInt("type1", info->getType());
	case 1:
		info = (DatasetInfo*)m_listctrl->GetItemData(0);
		m_textureShader->setUniInt("tex0", 0);
		m_textureShader->setUniInt("show0", info->getShow());
		m_textureShader->setUniFloat("threshold0",  info->getThreshold());
		m_textureShader->setUniInt("type0", info->getType());
		
	case 0:
	default:
	;}	
}

void TheScene::setMeshShaderVars()
{
	m_meshShader->bind();
	
	m_meshShader->setUniInt("dimX", TheDataset::columns);
	m_meshShader->setUniInt("dimY", TheDataset::rows);
	m_meshShader->setUniInt("dimZ", TheDataset::frames);
	m_meshShader->setUniInt("cutX", (int)(m_xSlize - TheDataset::columns/2.0));
	m_meshShader->setUniInt("cutY", (int)(m_ySlize - TheDataset::rows/2.0));
	m_meshShader->setUniInt("cutZ", (int)(m_zSlize - TheDataset::frames/2.0));
	m_meshShader->setUniInt("sector", m_quadrant);
	
	DatasetInfo* info;
	
	switch (m_countTextures)
	{
	case 10:
		info = (DatasetInfo*)m_listctrl->GetItemData(9);
		m_meshShader->setUniInt("tex9", 9);
		m_meshShader->setUniInt("show9", info->getShow());
		m_meshShader->setUniFloat("threshold9",  info->getThreshold());
		m_meshShader->setUniInt("type9", info->getType());
	case 9:
		info = (DatasetInfo*)m_listctrl->GetItemData(8);
		m_meshShader->setUniInt("tex8", 8);
		m_meshShader->setUniInt("show8", info->getShow());
		m_meshShader->setUniFloat("threshold8",  info->getThreshold());
		m_meshShader->setUniInt("type8", info->getType());
	case 8:
		info = (DatasetInfo*)m_listctrl->GetItemData(7);
		m_meshShader->setUniInt("tex7", 7);
		m_meshShader->setUniInt("show7", info->getShow());
		m_meshShader->setUniFloat("threshold7",  info->getThreshold());
		m_meshShader->setUniInt("type7", info->getType());
	case 7:
		info = (DatasetInfo*)m_listctrl->GetItemData(6);
		m_meshShader->setUniInt("tex6", 6);
		m_meshShader->setUniInt("show6", info->getShow());
		m_meshShader->setUniFloat("threshold6",  info->getThreshold());
		m_meshShader->setUniInt("type6", info->getType());
	case 6:
		info = (DatasetInfo*)m_listctrl->GetItemData(5);
		m_meshShader->setUniInt("tex5", 5);
		m_meshShader->setUniInt("show5", info->getShow());
		m_meshShader->setUniFloat("threshold5",  info->getThreshold());
		m_meshShader->setUniInt("type5", info->getType());
	case 5:
		info = (DatasetInfo*)m_listctrl->GetItemData(4);
		m_meshShader->setUniInt("tex4", 4);
		m_meshShader->setUniInt("show4", info->getShow());
		m_meshShader->setUniFloat("threshold4",  info->getThreshold());
		m_meshShader->setUniInt("type4", info->getType());
	case 4:
		info = (DatasetInfo*)m_listctrl->GetItemData(3);
		m_meshShader->setUniInt("tex3", 3);
		m_meshShader->setUniInt("show3", info->getShow());
		m_meshShader->setUniFloat("threshold3",  info->getThreshold());
		m_meshShader->setUniInt("type3", info->getType());
	case 3:
		info = (DatasetInfo*)m_listctrl->GetItemData(2);
		m_meshShader->setUniInt("tex2", 2);
		m_meshShader->setUniInt("show2", info->getShow());
		m_meshShader->setUniFloat("threshold2",  info->getThreshold());
		m_meshShader->setUniInt("type2", info->getType());
	case 2:
		info = (DatasetInfo*)m_listctrl->GetItemData(1);
		m_meshShader->setUniInt("tex1", 1);
		m_meshShader->setUniInt("show1", info->getShow());
		m_meshShader->setUniFloat("threshold1",  info->getThreshold());
		m_meshShader->setUniInt("type1", info->getType());
	case 1:
		info = (DatasetInfo*)m_listctrl->GetItemData(0);
		m_meshShader->setUniInt("tex0", 0);
		m_meshShader->setUniInt("show0", info->getShow());
		m_meshShader->setUniFloat("threshold0",  info->getThreshold());
		m_meshShader->setUniInt("type0", info->getType());
	case 0:
	default:
	;}
}

void TheScene::renderScene()
{
	if (m_listctrl->GetItemCount() == 0) return;
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	bindTextures();
	setTextureShaderVars();
	
	if (m_showXSlize) renderXSlize();
	if (m_showYSlize) renderYSlize();
	if (m_showZSlize) renderZSlize();

	glPopAttrib();
	
	m_textureShader->release();
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	renderMesh();
	
	glPopAttrib();
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
		
	renderCurves();
		
	glPopAttrib();
	
	if (m_showSelBox) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		drawSelectionBox();
		glPopAttrib();
	}
	
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
}

void TheScene::makeLights()
{
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	
	GLfloat light_position0[] = { -m_lightPos.s.X, -m_lightPos.s.Y, -m_lightPos.s.Z, 0.0};
	
	glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv (GL_LIGHT0, GL_POSITION, light_position0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
}

void TheScene::renderMesh()
{
	makeLights();

	setMeshShaderVars();
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	
	for (int i = 0 ; i < m_listctrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_listctrl->GetItemData(i);
		float c = (float)info->getThreshold();
		glColor3f(c,c,c);
		m_meshShader->setUniInt("showFS", info->getShowFS());
		m_meshShader->setUniInt("useTex", info->getUseTex());
		
		if (info->getType() == Mesh_ && info->getShow())
		{
			glCallList(m_texNames[i]);
		}
	}
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	
	m_meshShader->release();
}

void TheScene::renderCurves()
{
	m_curveShader->bind();
	for (int i = 0 ; i < m_listctrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_listctrl->GetItemData(i);
		
		m_curveShader->setUniInt("useNormals", !info->getShowFS());
		//m_curveShader->setUniInt("useNormals", info->getUseTex());
		
		if (info->getType() == Curves_ && info->getShow())
		{
			glCallList(m_texNames[i]);
		}
	}
	m_curveShader->release();
}

void TheScene::renderNavView(int view)
{
	if (m_listctrl->GetItemCount() == 0) return;
	
	bindTextures();
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
	
	glColor3f(1.0, 0.0, 0.0);
	glBegin (GL_LINES);
		glVertex3f (-border, yline, border);
		glVertex3f ( border, yline, border);
		glVertex3f (xline, -border, border);
		glVertex3f (xline,  border, border);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);

	m_textureShader->bind();
}


void TheScene::updateView(float x, float y, float z)
{
	m_xSlize = x;
	m_ySlize = y;
	m_zSlize = z;
}

void TheScene::colorMap(float value)
{
    value *= 5.0;
	
	if( value < 0.0 )
		glColor3f( 0.0, 0.0, 0.0 );
    else if( value < 1.0 )
    	glColor3f( 0.0, value, 1.0 );
	else if( value < 2.0 )
		glColor3f( 0.0, 1.0, 2.0-value );
    else if( value < 3.0 )
    	glColor3f( value-2.0, 1.0, 0.0 );
    else if( value < 4.0 )
    	glColor3f( 1.0, 4.0-value, 0.0 );
    else if( value <= 5.0 )
    	glColor3f( 1.0, 0.0, value-4.0 );
    else 
    	glColor3f( 1.0, 0.0, 1.0 );
}

void TheScene::drawSphere(float x, float y, float z, float r)
{
	glPushMatrix();
	glTranslatef(x,y,z);
	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluSphere(quadric, r, 32, 32);
	glPopMatrix();
}

void TheScene::drawSelectionBox()
{
	float cx = m_selBoxCenter.s.X;
	float cy = m_selBoxCenter.s.Y;
	float cz = m_selBoxCenter.s.Z;
	float mx = cx - m_selBoxSize.s.X/2;
	float px = cx + m_selBoxSize.s.X/2;
	float my = cy - m_selBoxSize.s.Y/2;
	float py = cy + m_selBoxSize.s.Y/2;
	float mz = cz - m_selBoxSize.s.Z/2;
	float pz = cz + m_selBoxSize.s.Z/2;
	
	glColor3f(1.0, 0.0, 0.0);
	drawSphere(cx, cy, cz, 3.0);
	drawSphere(mx, cy, cz, 3.0);
	drawSphere(px, cy, cz, 3.0);
	drawSphere(cx, my, cz, 3.0);
	drawSphere(cx, py, cz, 3.0);
	drawSphere(cx, cy, mz, 3.0);
	drawSphere(cx, cy, pz, 3.0);
	glColor3f(0.0, 1.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
		glVertex3f(mx, cy, cz);
		glVertex3f(px, cy, cz);
		glVertex3f(cx, my, cz);
		glVertex3f(cx, py, cz);
		glVertex3f(cx, cy, mz);
		glVertex3f(cx, cy, pz);
		glVertex3f(px, py, pz);
		glVertex3f(px, py, mz);
		glVertex3f(mx, py, pz);
		glVertex3f(mx, py, mz);
		glVertex3f(mx, my, pz);
		glVertex3f(mx, my, mz);
		glVertex3f(px, my, pz);
		glVertex3f(px, my, mz);
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex3f( px, py, pz );
		glVertex3f( mx, py, pz );
		glVertex3f( mx, my, pz );
		glVertex3f( px, my, pz );
		glVertex3f( px, py, pz );
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex3f( px, py, mz );
		glVertex3f( mx, py, mz );
		glVertex3f( mx, my, mz );
		glVertex3f( px, my, mz );
		glVertex3f( px, py, mz );
	glEnd();
}
