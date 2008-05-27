#include "theScene.h"
/**
 * axial
 * coronal
 * sagittal
 */
TheScene::TheScene()
{
	nothing_loaded = true;

	m_blendThreshold = 0.1;
	m_xSlize = 0.5;
	m_ySlize = 0.5;
	m_zSlize = 0.5;
	m_showXSlize = true;
	m_showYSlize = true;
	m_showZSlize = true;
	m_showOverlay = false;
	m_showRGB = false;
	m_textureShader = 0;
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
	
	glAlphaFunc(GL_GREATER,m_blendThreshold); // adjust your prefered threshold here
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
	
	glAlphaFunc(GL_GREATER,0.1f); // adjust your prefered threshold here
	glEnable(GL_ALPHA_TEST);
}

void TheScene::assignTextures ()
{
	if (m_dataset->headIsLoaded())
	{
		glDeleteTextures(1, &m_headTex);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1, &m_headTex);
		glBindTexture(GL_TEXTURE_3D, m_headTex);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(GL_TEXTURE_3D, 
				0, 
				GL_RGBA, 
				m_dataset->m_headInfo->getColumns(), 
				m_dataset->m_headInfo->getRows(),
				m_dataset->m_headInfo->getFrames(),
				0, 
				GL_LUMINANCE_ALPHA, 
				GL_FLOAT,
				m_dataset->getDataHead());
	}
	
	if (m_dataset->overlayIsLoaded())
	{
		glDeleteTextures(1, &m_overlayTex);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1, &m_overlayTex);
		glBindTexture(GL_TEXTURE_3D, m_overlayTex);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(GL_TEXTURE_3D, 
				0, 
				GL_RGB, 
				m_dataset->m_headInfo->getColumns(), 
				m_dataset->m_headInfo->getRows(),
				m_dataset->m_headInfo->getFrames(),
				0, 
				GL_LUMINANCE, 
				GL_FLOAT,
				m_dataset->getDataOverlay());
	}
	
	if (m_dataset->rgbIsLoaded())
	{
		glDeleteTextures(1, &m_rgbTex);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1, &m_rgbTex);
		glBindTexture(GL_TEXTURE_3D, m_rgbTex);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(GL_TEXTURE_3D, 
				0, 
				GL_RGB, 
				m_dataset->m_headInfo->getColumns(), 
				m_dataset->m_headInfo->getRows(),
				m_dataset->m_headInfo->getFrames(),
				0, 
				GL_RGB, 
				GL_FLOAT,
				m_dataset->getDataRGB());
	}
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
	
	nothing_loaded = false;
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
	bindTextures();

	if (m_showXSlize) renderXSlize();
	if (m_showYSlize) renderYSlize();
	if (m_showZSlize) renderZSlize();

	glDisable(GL_TEXTURE_3D);
}

void TheScene::bindTextures()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_headTex);
	
	GLint texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "HeadTexture");
	glUniform1i (texLoc, 0);
	
	if (m_dataset->overlayIsLoaded())
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, m_overlayTex);
		
		GLint texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "OverlayTexture");
		glUniform1i (texLoc, 1);
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "showOverlay");
		glUniform1i (texLoc, m_showOverlay);
	}
	
	if (m_dataset->rgbIsLoaded())
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, m_rgbTex);
		
		GLint texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "RGBTexture");
		glUniform1i (texLoc, 2);
		texLoc = glGetUniformLocation (m_textureShader->getProgramObject(), "showRGB");
		glUniform1i (texLoc, m_showRGB);
	}
	
		
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
	float xline = 0.5;
	float yline = 0.5;
	
	bindTextures();
	
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
		    	glTexCoord3f(1.0, m_ySlize, 0.0); glVertex3f(1.0 - m_xOffset1,1.0 - m_xOffset1, 0.0);
		    	glTexCoord3f(1.0, m_ySlize, 1.0); glVertex3f(1.0 - m_xOffset1,0.0 + m_xOffset1, 0.0);
		    	glTexCoord3f(0.0, m_ySlize, 1.0); glVertex3f(0.0 + m_xOffset1,0.0 + m_xOffset1, 0.0);
		    	glTexCoord3f(0.0, m_ySlize, 0.0); glVertex3f(0.0 + m_xOffset1,1.0 - m_xOffset1, 0.0);
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
	
	glColor3f(1.0, 0.0, 0.0);
	glBegin (GL_LINES);
		glVertex3f (0.0, yline, 0.1);
		glVertex3f (1.0, yline, 0.1);
		glVertex3f (xline, 0.0, 0.1);
		glVertex3f (xline, 1.0, 0.1);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
	
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
	if ( m_ratio1 < 1.0) m_zLine = ( m_zSlize * m_ratio1 ) + ( 1.0 - m_ratio1 ) / 2.0;
	if ( m_ratio1 > 1.0) m_xLine = ( m_xSlize / m_ratio1 ) + ( 1.0 - (1.0/m_ratio1) ) / 2.0;
}

void TheScene::updateBlendThreshold(float threshold)
{
	m_blendThreshold = threshold;
}

void TheScene::releaseTextures()
{
	if (m_dataset->headIsLoaded())
	{
		glDeleteTextures(1, &m_headTex);
	}
	
	if (m_dataset->overlayIsLoaded())
	{
		glDeleteTextures(1, &m_overlayTex);
	}
	
	if (m_dataset->rgbIsLoaded())
	{
		glDeleteTextures(1, &m_rgbTex);
	}
}
