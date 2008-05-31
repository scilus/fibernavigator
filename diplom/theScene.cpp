#include "theScene.h"



TheScene::TheScene()
{
	m_countTextures = 0;
	
	m_mainTexAssigned = false;
	m_navTexAssigned = false;
	m_mainGLContext = 0;
	m_navGLContext = 0;
	m_texNames = new GLuint[10];
	m_xSlize = 0.5;
	m_ySlize = 0.5;
	m_zSlize = 0.5;
	m_showXSlize = true;
	m_showYSlize = true;
	m_showZSlize = true;
	m_textureShader = 0;
	
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

void TheScene::initMainGL()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  printf("Error: %s\n", glewGetErrorString(err));
	}
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum( -0.52, 0.52, -0.52, 0.52, 5.0, 25.0 );
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -6.0 );
	
	glShadeModel(GL_FLAT);
	glEnable(GL_DOUBLEBUFFER);
	glEnable(GL_DEPTH_TEST);
	
	glAlphaFunc(GL_GREATER, 0.0000001); // adjust your prefered threshold here
	glEnable(GL_ALPHA_TEST);
}

void TheScene::initNavGL()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  printf("Error: %s\n", glewGetErrorString(err));
	}
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
		
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	
	glShadeModel(GL_FLAT);
	glEnable(GL_DOUBLEBUFFER);
	glEnable(GL_DEPTH_TEST);
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
	
	
	
	m_ratio0 = m_xSize/m_ySize;
	m_ratio1 = m_xSize/m_zSize;
	m_ratio2 = m_ySize/m_zSize;

	if (m_ratio0 > 1.0) {
		m_yOffset0 = (1.0 - (1.0 / m_ratio0)) / 2.0; 
	}
	else
		m_xOffset0 = (1.0 - m_ratio0) / 2.0;
	
	if (m_ratio1 > 1.0) {
		m_yOffset1 = (1.0 - (1.0 / m_ratio1)) / 2.0; 
	}
	else
		m_xOffset1 = (1.0 - m_ratio1) / 2.0;
	
	if (m_ratio2 > 1.0) {
		m_yOffset2 = (1.0 - (1.0 / m_ratio2)) / 2.0; 
	}
	else
		m_xOffset2 = (1.0 - m_ratio2) / 2.0;
	
	m_xOffset0 = wxMax(m_xOffset0, m_xOffset1);
	m_xOffset1 = wxMax(m_xOffset0, m_xOffset1);
	
	m_yOffset1 = wxMax(m_yOffset1, m_yOffset2);
	m_yOffset2 = wxMax(m_yOffset1, m_yOffset2);	
}

void TheScene::initShaders()
{
	if (m_textureShader)
	{
		delete m_textureShader;
	}
	printf("initializing shader\n");

	GLSLShader *vShader = new GLSLShader(GL_VERTEX_SHADER);
	GLSLShader *fShader = new GLSLShader(GL_FRAGMENT_SHADER);
	
	vShader->loadCode(wxT("GLSL/v1.glsl"));
	fShader->loadCode(wxT("GLSL/f1.glsl"));
	
	m_textureShader = new FGLSLShaderProgram();
	m_textureShader->link(vShader, fShader);
	m_textureShader->bind();
}

void TheScene::renderScene(int view)
{
	if (m_listctrl->GetItemCount() == 0) return;
	
	bindTextures();
	setShaderVars();

	if (m_showXSlize) renderXSlize();
	if (m_showYSlize) renderYSlize();
	if (m_showZSlize) renderZSlize();

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

void TheScene::setShaderVars()
{
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

void TheScene::renderXSlize()
{
	glBegin(GL_QUADS);
		glTexCoord3f(m_xSlize, 0.0, 0.0); glVertex3f(m_xLine - 0.5, -0.5 + m_xOffset2, -0.5 + m_yOffset2);
    	glTexCoord3f(m_xSlize, 0.0, 1.0); glVertex3f(m_xLine - 0.5, -0.5 + m_xOffset2,  0.5 - m_yOffset2);
    	glTexCoord3f(m_xSlize, 1.0, 1.0); glVertex3f(m_xLine - 0.5,  0.5 - m_xOffset2,  0.5 - m_yOffset2);
    	glTexCoord3f(m_xSlize, 1.0, 0.0); glVertex3f(m_xLine - 0.5,  0.5 - m_xOffset2, -0.5 + m_yOffset2);
    glEnd();
}

void TheScene::renderYSlize()
{
	glBegin(GL_QUADS);
    	glTexCoord3f(0.0, m_ySlize, 0.0); glVertex3f(-0.5 + m_xOffset1, m_yLine - 0.5, -0.5 + m_yOffset1);
    	glTexCoord3f(0.0, m_ySlize, 1.0); glVertex3f(-0.5 + m_xOffset1, m_yLine - 0.5,  0.5 - m_yOffset1);
    	glTexCoord3f(1.0, m_ySlize, 1.0); glVertex3f( 0.5 - m_xOffset1, m_yLine - 0.5,  0.5 - m_yOffset1);
    	glTexCoord3f(1.0, m_ySlize, 0.0); glVertex3f( 0.5 - m_xOffset1, m_yLine - 0.5, -0.5 + m_yOffset1);
    glEnd();
}

void TheScene::renderZSlize()
{
	glBegin(GL_QUADS);
    	glTexCoord3f(0.0, 0.0, m_zSlize); glVertex3f(-0.5 + m_xOffset0, -0.5 + m_yOffset0, m_zLine - 0.5);
    	glTexCoord3f(0.0, 1.0, m_zSlize); glVertex3f(-0.5 + m_xOffset0,  0.5 - m_yOffset0, m_zLine - 0.5);
    	glTexCoord3f(1.0, 1.0, m_zSlize); glVertex3f( 0.5 - m_xOffset0,  0.5 - m_yOffset0, m_zLine - 0.5);
    	glTexCoord3f(1.0, 0.0, m_zSlize); glVertex3f( 0.5 - m_xOffset0, -0.5 + m_yOffset0, m_zLine - 0.5);
    glEnd();
}

void TheScene::renderNavView(int view)
{
	if (m_listctrl->GetItemCount() == 0) return;
	
	float xline = 0.5;
	float yline = 0.5;
	
	bindTextures();
	setShaderVars();
	
	switch (view)
	{
		case axial:
			glBegin(GL_QUADS);
	        	glTexCoord3f(0.0, 1.0 , m_zSlize); glVertex3f(0.0 + m_xOffset0, 0.0 + m_yOffset0, 0.0);
	        	glTexCoord3f(0.0, 0.0 , m_zSlize); glVertex3f(0.0 + m_xOffset0, 1.0 - m_yOffset0, 0.0);
	        	glTexCoord3f(1.0, 0.0 , m_zSlize); glVertex3f(1.0 - m_xOffset0, 1.0 - m_yOffset0, 0.0);
	        	glTexCoord3f(1.0, 1.0 , m_zSlize); glVertex3f(1.0 - m_xOffset0, 0.0 + m_yOffset0, 0.0);
			glEnd();
			xline = m_xLine;
			yline = 1.0 - m_yLine;
			
			break;
		case coronal:
			glBegin(GL_QUADS);
		    	glTexCoord3f(1.0, m_ySlize, 0.0); glVertex3f(1.0 - m_xOffset1,1.0 - m_yOffset1, 0.0);
		    	glTexCoord3f(1.0, m_ySlize, 1.0); glVertex3f(1.0 - m_xOffset1,0.0 + m_yOffset1, 0.0);
		    	glTexCoord3f(0.0, m_ySlize, 1.0); glVertex3f(0.0 + m_xOffset1,0.0 + m_yOffset1, 0.0);
		    	glTexCoord3f(0.0, m_ySlize, 0.0); glVertex3f(0.0 + m_xOffset1,1.0 - m_yOffset1, 0.0);
		    glEnd();
		    xline = m_xLine;
		    yline = 1.0 - m_zLine;
		    
			break;
		case sagittal:
			glBegin(GL_QUADS);
	        	glTexCoord3f(m_xSlize, 0.0, 1.0); glVertex3f(0.0 + m_xOffset2,0.0 + m_yOffset2, 0.0);
	        	glTexCoord3f(m_xSlize, 0.0, 0.0); glVertex3f(0.0 + m_xOffset2,1.0 - m_yOffset2, 0.0);
	        	glTexCoord3f(m_xSlize, 1.0, 0.0); glVertex3f(1.0 - m_xOffset2,1.0 - m_yOffset2, 0.0);
	        	glTexCoord3f(m_xSlize, 1.0, 1.0); glVertex3f(1.0 - m_xOffset2,0.0 + m_yOffset2, 0.0);
			glEnd();
			xline = m_yLine;
			yline = 1.0 - m_zLine;
			break;
	}	
	
	glDisable(GL_TEXTURE_3D);
	
	m_textureShader->release();
	
	glColor3f(1.0, 0.0, 0.0);
	glBegin (GL_LINES);
		glVertex3f (0.0, yline, 0.1);
		glVertex3f (1.0, yline, 0.1);
		glVertex3f (xline, 0.0, 0.1);
		glVertex3f (xline, 1.0, 0.1);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
	
	m_textureShader->bind();
}


void TheScene::updateView(float x, float y, float z)
{
	m_xSlize = x/m_xSize;
	m_ySlize = y/m_ySize;
	m_zSlize = z/m_zSize;
	
	m_xLine = m_xSlize;
	m_yLine = m_ySlize;
	m_zLine = m_zSlize;
	
	if ( m_ratio0 < 1.0) m_xLine = ( m_xSlize * m_ratio0 ) + ( 1.0 - m_ratio0 ) / 2.0;
	if ( m_ratio2 > 1.0) m_zLine = ( m_zSlize / m_ratio2 ) + ( 1.0 - (1.0/m_ratio2) ) / 2.0;
	//if ( m_ratio1 < 1.0) m_zLine = ( m_zSlize * m_ratio1 ) + ( 1.0 - m_ratio1 ) / 2.0;
	if ( m_ratio1 > 1.0) m_xLine = ( m_xSlize / m_ratio1 ) + ( 1.0 - (1.0/m_ratio1) ) / 2.0;
}

void TheScene::releaseTextures()
{
	glDeleteTextures(10, m_texNames);
}
