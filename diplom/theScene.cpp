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

void TheScene::swapTextures(int a, int b)
{
	GLuint temp = m_texNames[a];
	m_texNames[a] = m_texNames[b];
	m_texNames[b] = temp;
}

void TheScene::setDataset(TheDataset *dataset)
{
	m_dataset = dataset;
	
	m_xSize = (float)dataset->m_columns;
	m_ySize = (float)dataset->m_rows;
	m_zSize = (float)dataset->m_frames;
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
	m_meshShader->link(vShader, fShader);
	//m_meshShader->bind();
}

void TheScene::setTextureShaderVars()
{
	m_textureShader->bind();
	
	GLint texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "countTextures");
	glUniform1i (texLoc, m_countTextures);
	DatasetInfo* info;
	switch (m_countTextures)
	{
	case 10:
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex9");
		glUniform1i (texLoc, 9);
	case 9:
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex8");
		glUniform1i (texLoc, 8);
	case 8:
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex7");
		glUniform1i (texLoc, 7);
	case 7:
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex6");
		glUniform1i (texLoc, 6);
	case 6:
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex5");
		glUniform1i (texLoc, 5);
	case 5:
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex4");
		glUniform1i (texLoc, 4);
	case 4:
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex3");
		glUniform1i (texLoc, 3);
	case 3:
		info = (DatasetInfo*)m_listctrl->GetItemData(2);
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex2");
		glUniform1i (texLoc, 2);
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "showTex2");
		glUniform1i (texLoc, info->getShow());
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "thresholdTex2");
		glUniform1f (texLoc, info->getThreshold());
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "typeTex2");
		glUniform1i (texLoc, info->getType());
	case 2:
		info = (DatasetInfo*)m_listctrl->GetItemData(1);
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex1");
		glUniform1i (texLoc, 1);
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "showTex1");
		glUniform1i (texLoc, info->getShow());
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "thresholdTex1");
		glUniform1f (texLoc, info->getThreshold());
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "typeTex1");
		glUniform1i (texLoc, info->getType());
	case 1:
		info = (DatasetInfo*)m_listctrl->GetItemData(0);
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex0");
		glUniform1i (texLoc, 0);
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "showTex0");
		glUniform1i (texLoc, info->getShow());
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "thresholdTex0");
		glUniform1f (texLoc, info->getThreshold());
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "typeTex0");
		glUniform1i (texLoc, info->getType());
	case 0:
	default:
	;}	
}

void TheScene::setMeshShaderVars()
{
	m_meshShader->bind();
	
	GLint texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "dimX");
	glUniform1i (texLoc, m_dataset->m_columns);
	texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "dimY");
	glUniform1i (texLoc, m_dataset->m_rows);
	texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "dimZ");
	glUniform1i (texLoc, m_dataset->m_frames);
	texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "tex0");
	glUniform1i (texLoc, 0);
	texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "showMesh");
	glUniform1i (texLoc, m_showMesh);
}

void TheScene::renderScene(int view)
{
	if (m_listctrl->GetItemCount() == 0) return;
	
	bindTextures();
	setTextureShaderVars();

	if (m_showXSlize) renderXSlize();
	if (m_showYSlize) renderYSlize();
	if (m_showZSlize) renderZSlize();

	setMeshShaderVars();
	if (m_dataset->meshLoaded)
	{
		renderMesh();
	}
	
	glDisable(GL_TEXTURE_3D);
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

void TheScene::renderMesh()
{
	m_textureShader->release();
	//m_meshShader->bind();

	glColor3f(1.0, 1.0, 1.0);
	
	int x = m_dataset->m_columns/2;
	int y = m_dataset->m_rows/2;
	int z = m_dataset->m_frames/2;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	
	for ( int i = 0 ; i < m_dataset->m_mesh->getCountPolygons() ; ++i)
	{
		polygon p = m_dataset->m_mesh->m_polygonArray[i];
		glNormal3f( 	m_dataset->m_mesh->m_vertexArray[p.v1].nx, 
						m_dataset->m_mesh->m_vertexArray[p.v1].ny,
						m_dataset->m_mesh->m_vertexArray[p.v1].nz);
		glVertex3f( 	m_dataset->m_mesh->m_vertexArray[p.v1].x - x, 
				     	m_dataset->m_mesh->m_vertexArray[p.v1].y - y, 
				     	m_dataset->m_mesh->m_vertexArray[p.v1].z - z);
		glNormal3f( 	m_dataset->m_mesh->m_vertexArray[p.v2].nx, 
						m_dataset->m_mesh->m_vertexArray[p.v2].ny,
						m_dataset->m_mesh->m_vertexArray[p.v2].nz);
		glVertex3f(	m_dataset->m_mesh->m_vertexArray[p.v2].x - x, 
						m_dataset->m_mesh->m_vertexArray[p.v2].y - y, 
						m_dataset->m_mesh->m_vertexArray[p.v2].z - z);
		glNormal3f( 	m_dataset->m_mesh->m_vertexArray[p.v3].nx, 
						m_dataset->m_mesh->m_vertexArray[p.v3].ny,
						m_dataset->m_mesh->m_vertexArray[p.v3].nz);
		glVertex3f(	m_dataset->m_mesh->m_vertexArray[p.v3].x - x, 
						m_dataset->m_mesh->m_vertexArray[p.v3].y - y, 
						m_dataset->m_mesh->m_vertexArray[p.v3].z - z);
	}
	
	glEnd();
	
	//m_meshShader->release();
	//m_textureShader->bind();
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

void TheScene::releaseTextures()
{
	glDeleteTextures(10, m_texNames);
}
