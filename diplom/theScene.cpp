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
	m_textureShader = 0;
	m_meshShader = 0;
	
	m_xOffset0 = 0.0;
	m_yOffset0 = 0.0;
	m_xOffset1 = 0.0;
	m_yOffset1 = 0.0;
	m_xOffset2 = 0.0;
	m_yOffset2 = 0.0;
	
	m_quadrant = 1;
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
	
	float maxLength = (float)wxMax(m_dataset->m_columns, wxMax(m_dataset->m_rows, m_dataset->m_frames));
	float view1 = maxLength/2.0;
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-view1, view1, -view1, view1, -(view1 + 5) , view1 + 5);	
}

void TheScene::assignTextures ()
{
	printf("assign textures\n");
	m_countTextures = m_listctrl->GetItemCount();
	if (m_countTextures == 0) return;
	
	glDeleteTextures(10, m_texNames);
	
	for (int i = 0 ; i < m_countTextures ; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_3D, m_texNames[i]);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		DatasetInfo* info = (DatasetInfo*)m_listctrl->GetItemData(i);
		info->generateTexture();
	}
}

void TheScene::bindTextures()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	
	for (int i = 0 ; i < m_countTextures ; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_3D, m_texNames[i]);
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
}

void TheScene::setTextureShaderVars()
{
	m_textureShader->bind();

	DatasetInfo* info;
	switch (m_countTextures)
	{
	case 10:
		m_textureShader->setUniInt("tex9", 9);
	case 9:
		m_textureShader->setUniInt("tex8", 8);
	case 8:
		m_textureShader->setUniInt("tex7", 7);
	case 7:
		m_textureShader->setUniInt("tex6", 6);
	case 6:
		m_textureShader->setUniInt("tex5", 5);
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
	
	m_meshShader->setUniInt("dimX", m_dataset->m_columns);
	m_meshShader->setUniInt("dimY", m_dataset->m_rows);
	m_meshShader->setUniInt("dimZ", m_dataset->m_frames);
	m_meshShader->setUniInt("cutX", (int)(m_xSlize - m_xSize/2.0));
	m_meshShader->setUniInt("cutY", (int)(m_ySlize - m_ySize/2.0));
	m_meshShader->setUniInt("cutZ", (int)(m_zSlize - m_zSize/2.0));
	m_meshShader->setUniInt("sector", m_quadrant);
	
	DatasetInfo* info;
	
	switch (m_countTextures)
	{
	case 10:
	case 9:
	case 8:
	case 7:
	case 6:
	case 5:
	case 4:
	case 3:
	case 2:
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

void TheScene::renderScene(int view, int quadrant)
{
	if (m_listctrl->GetItemCount() == 0) return;
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	m_quadrant = quadrant;
	bindTextures();
	setTextureShaderVars();

	if (m_showXSlize) renderXSlize();
	if (m_showYSlize) renderYSlize();
	if (m_showZSlize) renderZSlize();

	glPopAttrib();
	
	//glDisable(GL_TEXTURE_3D);
	m_textureShader->release();
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	renderMesh();
	
	glPopAttrib();
}

void TheScene::renderXSlize()
{
	int x = m_xSlize - m_dataset->m_columns/2;
	int y = m_dataset->m_rows/2;
	int z = m_dataset->m_frames/2;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xSlize/m_xSize, 0.0, 0.0); glVertex3i(x, -y, -z);
    	glTexCoord3f(m_xSlize/m_xSize, 0.0, 1.0); glVertex3i(x, -y,  z);
    	glTexCoord3f(m_xSlize/m_xSize, 1.0, 1.0); glVertex3i(x,  y,  z);
    	glTexCoord3f(m_xSlize/m_xSize, 1.0, 0.0); glVertex3i(x,  y, -z);
    glEnd();
}

void TheScene::renderYSlize()
{
	int x = m_dataset->m_columns/2;
	int y = m_ySlize - m_dataset->m_rows/2;
	int z = m_dataset->m_frames/2;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, m_ySlize/m_ySize, 0.0); glVertex3i(-x, y, -z);
    	glTexCoord3f(0.0, m_ySlize/m_ySize, 1.0); glVertex3i(-x, y,  z);
    	glTexCoord3f(1.0, m_ySlize/m_ySize, 1.0); glVertex3i( x, y,  z);
    	glTexCoord3f(1.0, m_ySlize/m_ySize, 0.0); glVertex3i( x, y, -z);
    glEnd();
}

void TheScene::renderZSlize()
{
	int x = m_dataset->m_columns/2;
	int y = m_dataset->m_rows/2;
	int z = m_zSlize - m_dataset->m_frames/2;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, 0.0, m_zSlize/m_zSize); glVertex3i(-x, -y, z);
    	glTexCoord3f(0.0, 1.0, m_zSlize/m_zSize); glVertex3i(-x,  y, z);
    	glTexCoord3f(1.0, 1.0, m_zSlize/m_zSize); glVertex3i( x,  y, z);
    	glTexCoord3f(1.0, 0.0, m_zSlize/m_zSize); glVertex3i( x, -y, z);
    glEnd();
}

void TheScene::makeLights()
{
	GLfloat  light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat  light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	
	GLfloat light_position0[] = { 1.0, 0.0, 0.0, 0.0 };
	GLfloat light_position1[] = { -1.0, 0.0, 0.0, 0.0 };
	
	glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv (GL_LIGHT0, GL_POSITION, light_position0);
	
	glLightfv (GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv (GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv (GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
}

void TheScene::renderMesh()
{
	makeLights();

	setMeshShaderVars();
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	
	int x = m_dataset->m_columns/2;
	int y = m_dataset->m_rows/2;
	int z = m_dataset->m_frames/2;
	
	for (int i = 0 ; i < m_listctrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_listctrl->GetItemData(i);
		colorMap(info->getThreshold());
		m_meshShader->setUniInt("showFS", info->getShowFS());
		
		if (info->getType() == Mesh_ && info->getShow())
		{
			//m_meshShader->setUniFloat("mainColor", info->getThreshold());
			glBegin(GL_TRIANGLES);
			
			for ( int i = 0 ; i < info->m_mesh->getCountPolygons() ; ++i)
			{
				polygon p = info->m_mesh->m_polygonArray[i];
				
				glNormal3f( 	info->m_mesh->m_vertexArray[p.v1].nx, 
								info->m_mesh->m_vertexArray[p.v1].ny,
								info->m_mesh->m_vertexArray[p.v1].nz);
				glVertex3f( 	info->m_mesh->m_vertexArray[p.v1].x - x, 
								info->m_mesh->m_vertexArray[p.v1].y - y, 
								info->m_mesh->m_vertexArray[p.v1].z - z);
				
				glNormal3f( 	info->m_mesh->m_vertexArray[p.v2].nx, 
								info->m_mesh->m_vertexArray[p.v2].ny,
								info->m_mesh->m_vertexArray[p.v2].nz);
				glVertex3f(	info->m_mesh->m_vertexArray[p.v2].x - x, 
								info->m_mesh->m_vertexArray[p.v2].y - y, 
								info->m_mesh->m_vertexArray[p.v2].z - z);
		
				glNormal3f( 	info->m_mesh->m_vertexArray[p.v3].nx, 
								info->m_mesh->m_vertexArray[p.v3].ny,
								info->m_mesh->m_vertexArray[p.v3].nz);
				glVertex3f(	info->m_mesh->m_vertexArray[p.v3].x - x, 
								info->m_mesh->m_vertexArray[p.v3].y - y, 
								info->m_mesh->m_vertexArray[p.v3].z - z);
			}
			glEnd();
		}
	}
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	
}

void TheScene::renderNavView(int view)
{
	if (m_listctrl->GetItemCount() == 0) return;
	
	bindTextures();
	setTextureShaderVars();
	
	float xline = 0;
	float yline = 0;
	
	float border = (float)wxMax(m_dataset->m_columns, wxMax(m_dataset->m_rows, m_dataset->m_frames))/2.0;
	
	int x = m_dataset->m_columns/2;
	int y = m_dataset->m_rows/2;
	int z = m_dataset->m_frames/2;
	
	int xs = m_xSlize - m_dataset->m_columns/2;
	int ys = m_ySlize - m_dataset->m_rows/2;
	int zs = m_zSlize - m_dataset->m_frames/2;
	
	switch (view)
	{
		case axial: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, 1.0, m_zSlize/m_zSize); glVertex3f(-x, -y, zs);
		    	glTexCoord3f(0.0, 0.0, m_zSlize/m_zSize); glVertex3f(-x,  y, zs);
		    	glTexCoord3f(1.0, 0.0, m_zSlize/m_zSize); glVertex3f( x,  y, zs);
		    	glTexCoord3f(1.0, 1.0, m_zSlize/m_zSize); glVertex3f( x, -y, zs);
			glEnd();
			xline = m_xSlize - m_xSize/2.0;
			yline = m_ySize/2.0 - m_ySlize;
		} break;
		
		case coronal: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, m_ySlize/m_ySize, 1.0); glVertex3f( -x, -z, ys);
		    	glTexCoord3f(0.0, m_ySlize/m_ySize, 0.0); glVertex3f( -x,  z, ys);
		    	glTexCoord3f(1.0, m_ySlize/m_ySize, 0.0); glVertex3f(  x,  z, ys);
		    	glTexCoord3f(1.0, m_ySlize/m_ySize, 1.0); glVertex3f(  x, -z, ys);
		    glEnd();
		    xline = m_xSlize - m_xSize/2.0;
		    yline = m_zSize/2.0 - m_zSlize;
		} break;
		
		case sagittal: {
			glBegin(GL_QUADS);
				glTexCoord3f(m_xSlize/m_xSize, 0.0, 1.0); glVertex3f(-y, -z, xs);
		    	glTexCoord3f(m_xSlize/m_xSize, 0.0, 0.0); glVertex3f(-y,  z, xs);
		    	glTexCoord3f(m_xSlize/m_xSize, 1.0, 0.0); glVertex3f( y,  z, xs);
		    	glTexCoord3f(m_xSlize/m_xSize, 1.0, 1.0); glVertex3f( y, -z, xs);
			glEnd();
			xline = m_ySlize - m_ySize/2.0;
			yline = m_zSize/2.0 - m_zSlize;
		} break;
	}	
	
	glDisable(GL_TEXTURE_3D);
	
	m_textureShader->release();
	//m_meshShader->bind();

	glBegin (GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
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

void TheScene::setDataset(TheDataset *dataset)
{
	m_dataset = dataset;
	
	m_xSize = (float)dataset->m_columns;
	m_ySize = (float)dataset->m_rows;
	m_zSize = (float)dataset->m_frames;
}
