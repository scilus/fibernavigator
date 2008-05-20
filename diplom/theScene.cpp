#include "theScene.h"

#include "wx/wx.h"

//static GLuint tex1;

TheScene::TheScene()
{
	nothing_loaded = true;
	m_blendThreshold = 0.1;
	m_xSlize = 0.5;
	m_ySlize = 0.5;
	m_zSlize = 0.5;
	m_xTexture = 0.5;
	m_yTexture = 0.5;
	m_zTexture = 0.5;
	m_showXSlize = true;
	m_showYSlize = true;
	m_showZSlize = true;
}

void TheScene::initMainGL()
{
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
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glAlphaFunc(GL_GREATER,m_blendThreshold); // adjust your prefered threshold here
	glEnable(GL_ALPHA_TEST);

	if (m_tex1_loaded)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1, &m_tex1);
		glBindTexture(GL_TEXTURE_3D, m_tex1);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexImage3D(GL_TEXTURE_3D, 
				0, 
				GL_LUMINANCE_ALPHA, 
				m_dataset->getColumns(), 
				m_dataset->getRows(),
				m_dataset->getFrames(),
				0, 
				GL_LUMINANCE_ALPHA, 
				GL_FLOAT,
				m_texture_head);
	}
}

void TheScene::initNavGL()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	
	glShadeModel(GL_FLAT);
	glEnable(GL_DOUBLEBUFFER);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glAlphaFunc(GL_GREATER,0.1f); // adjust your prefered threshold here
	glEnable(GL_ALPHA_TEST);
	
	if (m_tex1_loaded)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			glGenTextures(1, &m_tex2);
			glBindTexture(GL_TEXTURE_3D, m_tex2);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
			glTexImage3D(GL_TEXTURE_3D, 
					0, 
					GL_LUMINANCE_ALPHA, 
					m_dataset->getColumns(), 
					m_dataset->getRows(),
					m_dataset->getFrames(),
					0, 
					GL_LUMINANCE_ALPHA, 
					GL_FLOAT,
					m_texture_head);
		}
}

void TheScene::setDataset(TheDataset *dataset)
{
	m_dataset = dataset;
	
	float *temp = m_dataset->getData();
	int size = m_dataset->getLength();
	m_texture_head = new float[size *2];
	
	float div = m_dataset->getHighestValue();
	
	for (int i = 0 ; i < size; ++i)
	{
		m_texture_head[2*i] = temp[i]/div;
		m_texture_head[(2*i)+1] =  temp[i] /div;
	}
		
	m_xSize = (float)dataset->getColumns();
	m_ySize = (float)dataset->getRows();
	m_zSize = (float)dataset->getFrames();
	
	m_ratio0 = m_xSize/m_ySize;
	m_ratio1 = m_xSize/m_zSize;
	m_ratio2 = m_ySize/m_zSize;

	m_xOffset0 = (wxMax (0, 1.0 - m_ratio0))/2.0;
	m_yOffset0 = (wxMax (0, m_ratio0 - 1.0))/2.0;
	m_xOffset1 = (wxMax (0, 1.0 - m_ratio1))/2.0;
	m_yOffset1 = (wxMax (0, m_ratio1 - 1.0))/2.0;
	m_xOffset2 = (wxMax (0, 1.0 - m_ratio2))/2.0;
	m_yOffset2 = (wxMax (0, m_ratio2 - 1.0))/2.0;
	
	m_xOffset0 = wxMax(m_xOffset0, m_xOffset1);
	m_xOffset1 = wxMax(m_xOffset0, m_xOffset1);
	
	m_yOffset1 = wxMax(m_yOffset1, m_yOffset2);
	m_yOffset2 = wxMax(m_yOffset1, m_yOffset2);
	
	m_tex1_loaded = true;
	nothing_loaded = false;
}

void TheScene::renderScene()
{
	if (m_showXSlize) renderXSlize();
	if (m_showYSlize) renderYSlize();
	if (m_showZSlize) renderZSlize();
}


void TheScene::renderXSlize()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, m_tex1);
	
	glBegin(GL_QUADS);
		glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 0.0 - m_yOffset2); glVertex3f(m_xSlize -0.5, -0.5, -0.5);
    	glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 1.0 + m_yOffset2); glVertex3f(m_xSlize -0.5, -0.5, 0.5);
    	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 1.0 + m_yOffset2); glVertex3f(m_xSlize -0.5, 0.5, 0.5);
    	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 0.0 - m_yOffset2); glVertex3f(m_xSlize -0.5, 0.5, -0.5);
    glEnd();
    
    glDisable(GL_TEXTURE_3D);
}

void TheScene::renderYSlize()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, m_tex1);
	
	glBegin(GL_QUADS);
    	glTexCoord3f(0.0 - m_xOffset1, m_yTexture, 0.0 - m_yOffset1); glVertex3f(-0.5, m_ySlize -0.5, -0.5);
    	glTexCoord3f(0.0 - m_xOffset1, m_yTexture, 1.0 + m_yOffset1); glVertex3f(-0.5, m_ySlize -0.5, 0.5);
    	glTexCoord3f(1.0 + m_xOffset1, m_yTexture, 1.0 + m_yOffset1); glVertex3f(0.5, m_ySlize -0.5, 0.5);
    	glTexCoord3f(1.0 + m_xOffset1, m_yTexture, 0.0 - m_yOffset1); glVertex3f(0.5, m_ySlize -0.5, -0.5);
    glEnd();
    
    glDisable(GL_TEXTURE_3D);
}

void TheScene::renderZSlize()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, m_tex1);
	
	glBegin(GL_QUADS);
    	glTexCoord3f(0.0 - m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(-0.5, -0.5, m_zSlize -0.5);
    	glTexCoord3f(0.0 - m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(-0.5, 0.5, m_zSlize -0.5);
    	glTexCoord3f(1.0 + m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(0.5, 0.5, m_zSlize -0.5);
    	glTexCoord3f(1.0 + m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(0.5, -0.5, m_zSlize -0.5);
    glEnd();
    
    glDisable(GL_TEXTURE_3D);
}

void TheScene::renderNavView(int view)
{
	float xline = 0.5;
	float yline = 0.5;
	
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, m_tex2);
	
	switch (view)
	{
		case 0:
			glBegin(GL_QUADS);
	        	glTexCoord3f(0.0 - m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(0.0,0.0,0.0);
	        	glTexCoord3f(0.0 - m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(0.0,1.0,0.0);
	        	glTexCoord3f(1.0 + m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(1.0,1.0,0.0);
	        	glTexCoord3f(1.0 + m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(1.0,0.0,0.0);
			glEnd();
			xline = m_xSlize;
			yline = 1.0 - m_ySlize;
			break;
		case 1:
			glBegin(GL_QUADS);
		    	glTexCoord3f(1.0 + m_xOffset1, m_yTexture, 0.0 - m_yOffset1); glVertex3f(1.0,1.0,0.0);
		    	glTexCoord3f(1.0 + m_xOffset1, m_yTexture, 1.0 + m_yOffset1); glVertex3f(1.0,0.0,0.0);
		    	glTexCoord3f(0.0 - m_xOffset1, m_yTexture, 1.0 + m_yOffset1); glVertex3f(0.0,0.0,0.0);
		    	glTexCoord3f(0.0 - m_xOffset1, m_yTexture, 0.0 - m_yOffset1); glVertex3f(0.0,1.0,0.0);
		    glEnd();
		    xline = m_xSlize;
		    yline = 1.0 - m_zSlize;
			break;
		case 2:
			glBegin(GL_QUADS);
	        	glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 1.0 + m_yOffset2); glVertex3f(0.0,0.0,0.0);
	        	glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 0.0 - m_yOffset2); glVertex3f(0.0,1.0,0.0);
	        	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 0.0 - m_yOffset2); glVertex3f(1.0,1.0,0.0);
	        	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 1.0 + m_yOffset2); glVertex3f(1.0,0.0,0.0);
			glEnd();
			xline = m_ySlize;
			yline = 1.0 - m_zSlize;
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
	//wxMessageBox(wxString::Format(wxT("%f : %f"), m_ratio0, m_xSize),  wxT(""), wxOK|wxICON_INFORMATION, NULL);
	m_xTexture = m_xSlize;
	m_yTexture = m_ySlize;
	m_zTexture = m_zSlize;
	
	if ( m_ratio0 < 1.0) m_zTexture = (m_zTexture / m_ratio0) + (1.0 - (1.0/m_ratio0))/2.0;
	if ( m_ratio2 > 1.0) m_xTexture = (m_xTexture *m_ratio2) + (1.0 - (m_ratio2))/2.0;
}

void TheScene::updateBlendThreshold(float threshold)
{
	m_blendThreshold = threshold;
}
