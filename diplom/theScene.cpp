#include "theScene.h"

#include "wx/wx.h"

//static GLuint tex1;

TheScene::TheScene()
{
	nothing_loaded = true;
	m_xSlize = 0.5;
	m_ySlize = 0.5;
	m_zSlize = 0.5;
	m_xTexture = 0.5;
	m_yTexture = 0.5;
	m_zTexture = 0.5;
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
	
	glAlphaFunc(GL_GREATER,0.1f); // adjust your prefered threshold here
	glEnable(GL_ALPHA_TEST);

	if (tex1_loaded)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1, &tex1);
		glBindTexture(GL_TEXTURE_3D, tex1);
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
	
	if (tex1_loaded)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			glGenTextures(1, &tex2);
			glBindTexture(GL_TEXTURE_3D, tex2);
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
		m_texture_head[(2*i)+1] =  temp[i] < 0.05 ? 0 : 255;
	}
		
	m_xSize = (float)dataset->getColumns();
	m_ySize = (float)dataset->getRows();
	m_zSize = (float)dataset->getFrames();
	
	float ratio0 = m_xSize/m_ySize;
	float ratio1 = m_xSize/m_zSize;
	float ratio2 = m_ySize/m_zSize;

	m_xOffset0 = (wxMax (0, 1.0 - ratio0))/2.0;
	m_yOffset0 = (wxMax (0, ratio0 - 1.0))/2.0;
	m_xOffset1 = (wxMax (0, 1.0 - ratio1))/2.0;
	m_yOffset1 = (wxMax (0, ratio1 - 1.0))/2.0;
	m_xOffset2 = (wxMax (0, 1.0 - ratio2))/2.0;
	m_yOffset2 = (wxMax (0, ratio2 - 1.0))/2.0;
	
	m_xOffset0 = wxMax(m_xOffset0, m_xOffset1);
	m_xOffset1 = wxMax(m_xOffset0, m_xOffset1);
	
	m_yOffset1 = wxMax(m_yOffset1, m_yOffset2);
	m_yOffset2 = wxMax(m_yOffset1, m_yOffset2);
	
	tex1_loaded = true;
	nothing_loaded = false;
}

void TheScene::renderScene()
{
	renderXSlize();
	renderYSlize();
	renderZSlize();
}


void TheScene::renderXSlize()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, tex1);
	
	glBegin(GL_QUADS);
		glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 0.0 - m_yOffset2); glVertex3f(m_xTexture -0.5, -0.5, -0.5);
    	glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 1.0 + m_yOffset2); glVertex3f(m_xTexture -0.5, -0.5, 0.5);
    	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 1.0 + m_yOffset2); glVertex3f(m_xTexture -0.5, 0.5, 0.5);
    	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 0.0 - m_yOffset2); glVertex3f(m_xTexture -0.5, 0.5, -0.5);
    glEnd();
    
    glDisable(GL_TEXTURE_3D);
}

void TheScene::renderYSlize()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, tex1);
	
	glBegin(GL_QUADS);
    	glTexCoord3f(0.0 - m_xOffset1, m_yTexture, 0.0 - m_yOffset1); glVertex3f(-0.5, m_yTexture -0.5, -0.5);
    	glTexCoord3f(0.0 - m_xOffset1, m_yTexture, 1.0 + m_yOffset1); glVertex3f(-0.5, m_yTexture -0.5, 0.5);
    	glTexCoord3f(1.0 + m_xOffset1, m_yTexture, 1.0 + m_yOffset1); glVertex3f(0.5, m_yTexture -0.5, 0.5);
    	glTexCoord3f(1.0 + m_xOffset1, m_yTexture, 0.0 - m_yOffset1); glVertex3f(0.5, m_yTexture -0.5, -0.5);
    glEnd();
    
    glDisable(GL_TEXTURE_3D);
}

void TheScene::renderZSlize()
{
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, tex1);
	
	glBegin(GL_QUADS);
    	glTexCoord3f(0.0 - m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(-0.5, -0.5, m_zTexture -0.5);
    	glTexCoord3f(0.0 - m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(-0.5, 0.5, m_zTexture -0.5);
    	glTexCoord3f(1.0 + m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(0.5, 0.5, m_zTexture -0.5);
    	glTexCoord3f(1.0 + m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(0.5, -0.5, m_zTexture -0.5);
    glEnd();
    
    glDisable(GL_TEXTURE_3D);
}

void TheScene::renderNavView(int view)
{
	float xline = 0.5;
	float yline = 0.5;
	
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, tex2);
	
	switch (view)
	{
		case 0:
			glBegin(GL_QUADS);
	        	glTexCoord3f(0.0 - m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(0.0,0.0,0.0);
	        	glTexCoord3f(0.0 - m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(0.0,1.0,0.0);
	        	glTexCoord3f(1.0 + m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(1.0,1.0,0.0);
	        	glTexCoord3f(1.0 + m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(1.0,0.0,0.0);
			glEnd();
			xline = m_xTexture;
			yline = 1.0 - m_yTexture;
			break;
		case 1:
			glBegin(GL_QUADS);
		    	glTexCoord3f(1.0 + m_xOffset1, m_yTexture, 0.0 - m_yOffset1); glVertex3f(1.0,1.0,0.0);
		    	glTexCoord3f(1.0 + m_xOffset1, m_yTexture, 1.0 + m_yOffset1); glVertex3f(1.0,0.0,0.0);
		    	glTexCoord3f(0.0 - m_xOffset1, m_yTexture, 1.0 + m_yOffset1); glVertex3f(0.0,0.0,0.0);
		    	glTexCoord3f(0.0 - m_xOffset1, m_yTexture, 0.0 - m_yOffset1); glVertex3f(0.0,1.0,0.0);
		    glEnd();
		    xline = m_xTexture;
		    yline = 1.0 - m_zTexture;
			break;
		case 2:
			glBegin(GL_QUADS);
	        	glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 1.0 + m_yOffset2); glVertex3f(0.0,0.0,0.0);
	        	glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 0.0 - m_yOffset2); glVertex3f(0.0,1.0,0.0);
	        	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 0.0 - m_yOffset2); glVertex3f(1.0,1.0,0.0);
	        	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 1.0 + m_yOffset2); glVertex3f(1.0,0.0,0.0);
			glEnd();
			xline = m_yTexture;
			yline = 1.0 - m_zTexture;
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
	m_xSlize = x;
	m_ySlize = y;
	m_zSlize = z;
	//wxMessageBox(wxString::Format(wxT("%f : %f"), m_xSlize, m_xSize),  wxT(""), wxOK|wxICON_INFORMATION, NULL);
	m_xTexture = m_xSlize/m_xSize;
	m_yTexture = m_ySlize/m_ySize;
	m_zTexture = m_zSlize/m_zSize;
}
