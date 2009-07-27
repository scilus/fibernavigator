/*
 * AnatomyHelper.cpp
 *
 *  Created on: 23.07.2008
 *      Author: ralph
 */
#include "AnatomyHelper.h"

#include "GL/glew.h"

AnatomyHelper::AnatomyHelper(DatasetHelper* dh)
: m_dh(dh)
{

}

AnatomyHelper::~AnatomyHelper() {
	// TODO Auto-generated destructor stub
}

void AnatomyHelper::renderNav(int view, Shader *shader)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	m_dh->scene->bindTextures();
	shader->bind();
	m_dh->shaderHelper->initializeArrays();
	m_dh->shaderHelper->setTextureShaderVars();

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0001f);

	float xline = 0;
	float yline = 0;

	float max = (float)wxMax(m_dh->columns * m_dh->xVoxel, wxMax(m_dh->rows * m_dh->yVoxel, m_dh->frames * m_dh->zVoxel));

	float x = m_dh->columns * m_dh->xVoxel;
	float y = m_dh->rows * m_dh->yVoxel;
	float z = m_dh->frames * m_dh->zVoxel;

	float xo = ( max - x ) / 2.0;
	float yo = ( max - y ) / 2.0;
	float zo = ( max - z ) / 2.0;

	float quadZ = -0.1f;
	float lineZ = 0.0f;

	switch (view)
	{
		case axial: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, 0.0, ((float)m_dh->zSlize + 0.5f)/(float)m_dh->frames); glVertex3f( 0 + xo, 0 + yo, quadZ);
				glTexCoord3f(1.0, 0.0, ((float)m_dh->zSlize + 0.5f)/(float)m_dh->frames); glVertex3f( x + xo, 0 + yo, quadZ);
				glTexCoord3f(1.0, 1.0, ((float)m_dh->zSlize + 0.5f)/(float)m_dh->frames); glVertex3f( x + xo, y + yo, quadZ);
				glTexCoord3f(0.0, 1.0, ((float)m_dh->zSlize + 0.5f)/(float)m_dh->frames); glVertex3f( 0 + xo, y + yo, quadZ);
		    glEnd();
			xline = m_dh->xSlize * m_dh->xVoxel + xo;
			yline = m_dh->ySlize * m_dh->yVoxel + yo;
		} break;

		case coronal: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows, 0.0); glVertex3f( 0 + xo, 0 + zo, quadZ);
		    	glTexCoord3f(0.0, ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows, 1.0); glVertex3f( 0 + xo, z + zo, quadZ);
		    	glTexCoord3f(1.0, ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows, 1.0); glVertex3f( x + xo, z + zo, quadZ);
		    	glTexCoord3f(1.0, ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows, 0.0); glVertex3f( x + xo, 0 + zo, quadZ);
		    glEnd();
		    xline = m_dh->xSlize * m_dh->xVoxel + xo;
		    yline = m_dh->zSlize * m_dh->zVoxel + zo;
		} break;

		case sagittal: {
			glBegin(GL_QUADS);
				glTexCoord3f(((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns, 1.0, 0.0); glVertex3f( 0 + yo, 0 + zo, quadZ);
		    	glTexCoord3f(((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns, 1.0, 1.0); glVertex3f( 0 + yo, z + zo, quadZ);
		    	glTexCoord3f(((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns, 0.0, 1.0); glVertex3f( y + yo, z + zo, quadZ);
		    	glTexCoord3f(((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns, 0.0, 0.0); glVertex3f( y + yo, 0 + zo, quadZ);
			glEnd();
			xline = max - m_dh->ySlize * m_dh->yVoxel;
			yline = m_dh->zSlize * m_dh->zVoxel + zo;
		} break;
	}

	glDisable(GL_TEXTURE_3D);
	shader->release();
	glPopAttrib();

	glLineWidth(1.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin (GL_LINES);
		glVertex3f (0, yline, lineZ);
		glVertex3f ( max, yline, lineZ);
		glVertex3f (xline, 0, lineZ);
		glVertex3f (xline,  max, lineZ);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
}
//////////////////////////////////////////////////////////////////////////
void AnatomyHelper::renderMain()
{
	m_x = ( m_dh->xSlize * m_dh->xVoxel ) + 0.5f * m_dh->xVoxel;
	m_y = ( m_dh->ySlize * m_dh->yVoxel ) + 0.5f * m_dh->yVoxel;
	m_z = ( m_dh->zSlize * m_dh->zVoxel ) + 0.5f * m_dh->zVoxel;

	m_xc = ((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns;
	m_yc = ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows;
	m_zc = ((float)m_dh->zSlize + 0.5f)/(float)m_dh->frames;

	m_xb = m_dh->columns * m_dh->xVoxel ;
	m_yb = m_dh->rows * m_dh->yVoxel ;
	m_zb = m_dh->frames * m_dh->zVoxel ;
#if 1
	renderAxial();
	renderCoronal();
	renderSagittal();
#else
	switch (m_dh->quadrant)
	{
	case 1:
		renderS1();
		renderC1();
		renderA1();
		renderC2();
		renderA2();
		renderS3();
		renderS2();
		renderS4();
		renderA3();
		renderC3();
		renderC4();
		renderA4();
		break;
	case 2:
		renderS2();
		renderC2();
		renderA1();
		renderA2();
		renderC1();
		renderS3();
		renderS1();
		renderS4();
		renderC4();
		renderA3();
		renderA4();
		renderC3();
		break;
	case 3:
		renderS4();
		renderC2();
		renderA2();
		renderS2();
		renderA1();
		renderC1();
		renderS1();
		renderS3();
		renderC4();
		renderA4();
		renderA3();
		renderC3();
		break;
	case 4:
		renderA2();
		renderC1();
		renderS3();
		renderS1();
		renderA1();
		renderC2();
		renderS2();
		renderS4();
		renderA4();
		renderC3();
		renderC4();
		renderA3();
		break;
	case 5:
		renderA4();
		renderS3();
		renderC3();
		renderS1();
		renderA3();
		renderC4();
		renderS4();
		renderS2();
		renderA2();
		renderC1();
		renderC2();
		renderA1();
		break;
	case 6:
		renderC4();
		renderS4();
		renderA4();
		renderS2();
		renderA3();
		renderC3();
		renderS1();
		renderS3();
		renderC2();
		renderA1();
		renderA2();
		renderC1();
		break;
	case 7:
		renderA3();
		renderC4();
		renderS2();
		renderA4();
		renderC3();
		renderS3();
		renderS4();
		renderS1();
		renderC2();
		renderA1();
		renderA2();
		renderC1();
		break;
	case 8:
		renderS1();
		renderC3();
		renderA3();
		renderC4();
		renderA4();
		renderS3();
		renderS2();
		renderS4();
		renderA1();
		renderC1();
		renderC2();
		renderA2();
		break;
	}
#endif
}

void AnatomyHelper::renderCrosshair()
{
	float offset = 0.02f;
	glLineWidth(1.0);
	glBegin(GL_LINES);
		glVertex3f(m_x - offset, m_y - offset,  0);
		glVertex3f(m_x - offset, m_y - offset,  m_zb);
		glVertex3f(m_x - offset, m_y + offset,  0);
		glVertex3f(m_x - offset, m_y + offset,  m_zb);
		glVertex3f(m_x + offset, m_y + offset,  0);
		glVertex3f(m_x + offset, m_y + offset,  m_zb);
		glVertex3f(m_x + offset, m_y - offset,  0);
		glVertex3f(m_x + offset, m_y - offset,  m_zb);

		glVertex3f(m_x - offset, 0,   	m_z - offset);
		glVertex3f(m_x - offset, m_yb,  m_z - offset);
		glVertex3f(m_x - offset, 0,   	m_z + offset);
		glVertex3f(m_x - offset, m_yb,  m_z + offset);
		glVertex3f(m_x + offset, 0,   	m_z + offset);
		glVertex3f(m_x + offset, m_yb,  m_z + offset);
		glVertex3f(m_x + offset, 0,   	m_z - offset);
		glVertex3f(m_x + offset, m_yb,  m_z - offset);

		glVertex3f(0,  		m_y - offset, m_z - offset);
		glVertex3f(m_xb, 	m_y - offset, m_z - offset);
		glVertex3f(0,  		m_y - offset, m_z + offset);
		glVertex3f(m_xb, 	m_y - offset, m_z + offset);
		glVertex3f(0,  		m_y + offset, m_z + offset);
		glVertex3f(m_xb, 	m_y + offset, m_z + offset);
		glVertex3f(0,  		m_y + offset, m_z - offset);
		glVertex3f(m_xb, 	m_y + offset, m_z - offset);

	glEnd();
}

void AnatomyHelper::renderAxial()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, 0.0, m_zc); glVertex3f(0,  0,  m_z);
		glTexCoord3f(0.0, 1.0, m_zc); glVertex3f(0,  m_yb, m_z);
		glTexCoord3f(1.0, 1.0, m_zc); glVertex3f(m_xb, m_yb, m_z);
		glTexCoord3f(1.0, 0.0, m_zc); glVertex3f(m_xb, 0,  m_z);
	glEnd();
}

void AnatomyHelper::renderCoronal()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, m_yc, 0.0); glVertex3f(0,  m_y, 0);
		glTexCoord3f(0.0, m_yc, 1.0); glVertex3f(0,  m_y, m_zb);
		glTexCoord3f(1.0, m_yc, 1.0); glVertex3f(m_xb, m_y, m_zb);
		glTexCoord3f(1.0, m_yc, 0.0); glVertex3f(m_xb, m_y, 0);
	glEnd();
}

void AnatomyHelper::renderSagittal()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, 0.0, 0.0); glVertex3f(m_x, 0,  0);
		glTexCoord3f(m_xc, 0.0, 1.0); glVertex3f(m_x, 0,  m_zb);
		glTexCoord3f(m_xc, 1.0, 1.0); glVertex3f(m_x, m_yb, m_zb);
		glTexCoord3f(m_xc, 1.0, 0.0); glVertex3f(m_x, m_yb, 0);
	glEnd();
}

void AnatomyHelper::renderA1()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, 0.0, m_zc); glVertex3f(0, 0, m_z + 0.5f);
		glTexCoord3f(0.0, m_yc, m_zc);  glVertex3f(0, m_y, m_z + 0.5f);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x, m_y, m_z + 0.5f);
		glTexCoord3f(m_xc, 0.0,m_zc);   glVertex3f(m_x, 0, m_z + 0.5f);
	glEnd();
}

void AnatomyHelper::renderA2()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, m_yc, m_zc);  glVertex3f(0, m_y,  m_z + 0.5f);
		glTexCoord3f(0.0, 1.0, m_zc); glVertex3f(0, m_yb, m_z + 0.5f);
		glTexCoord3f(m_xc, 1.0, m_zc);  glVertex3f(m_x, m_yb, m_z + 0.5f);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x, m_y,  m_z + 0.5f);
	glEnd();
}

void AnatomyHelper::renderA3()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, 0, m_zc);   glVertex3f(m_x,  0, m_z + 0.5f);
		glTexCoord3f(m_xc, m_yc, m_zc);  glVertex3f(m_x,  m_y, m_z + 0.5f);
		glTexCoord3f(1.0, m_yc, m_zc); glVertex3f(m_xb, m_y, m_z + 0.5f);
		glTexCoord3f(1.0, 0, m_zc);  glVertex3f(m_xb, 0, m_z + 0.5f);
	glEnd();
}

void AnatomyHelper::renderA4()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x,  m_y,  m_z + 0.5f);
		glTexCoord3f(m_xc, 1.0, m_zc);  glVertex3f(m_x,  m_yb, m_z + 0.5f);
		glTexCoord3f(1.0, 1.0, m_zc); glVertex3f(m_xb, m_yb, m_z + 0.5f);
		glTexCoord3f(1.0, m_yc, m_zc);  glVertex3f(m_xb, m_y,  m_z + 0.5f);
	glEnd();

}

void AnatomyHelper::renderC1()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, m_yc, 0.0); glVertex3f(0, m_y + 0.5f, 0);
		glTexCoord3f(0.0, m_yc, m_zc);  glVertex3f(0, m_y + 0.5f, m_z);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x, m_y + 0.5f, m_z);
		glTexCoord3f(m_xc, m_yc, 0.0);  glVertex3f(m_x, m_y + 0.5f, 0);
	glEnd();
}

void AnatomyHelper::renderC2()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, m_yc, m_zc);  glVertex3f(0, m_y + 0.5f, m_z);
		glTexCoord3f(0.0, m_yc, 1.0); glVertex3f(0, m_y + 0.5f, m_zb);
		glTexCoord3f(m_xc, m_yc, 1.0);  glVertex3f(m_x, m_y + 0.5f, m_zb);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x, m_y + 0.5f, m_z);
	glEnd();
}

void AnatomyHelper::renderC3()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, m_yc, 0);   glVertex3f(m_x,  m_y + 0.5f, 0);
		glTexCoord3f(m_xc, m_yc, m_zc);  glVertex3f(m_x,  m_y + 0.5f, m_z);
		glTexCoord3f(1.0, m_yc, m_zc); glVertex3f(m_xb, m_y + 0.5f, m_z);
		glTexCoord3f(1.0, m_yc, 0);  glVertex3f(m_xb, m_y + 0.5f, 0);
	glEnd();
}

void AnatomyHelper::renderC4()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x,  m_y + 0.5f, m_z);
		glTexCoord3f(m_xc, m_yc, 1.0);  glVertex3f(m_x,  m_y + 0.5f, m_zb);
		glTexCoord3f(1.0, m_yc, 1.0); glVertex3f(m_xb, m_y + 0.5f, m_zb);
		glTexCoord3f(1.0, m_yc, m_zc);  glVertex3f(m_xb, m_y + 0.5f, m_z);
	glEnd();

}

void AnatomyHelper::renderS1()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, 0.0, 0.0); glVertex3f(m_x + 0.5f, 0, 0);
		glTexCoord3f(m_xc, 0.0, m_zc);  glVertex3f(m_x + 0.5f, 0, m_z);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x + 0.5f, m_y, m_z);
		glTexCoord3f(m_xc, m_yc, 0.0);  glVertex3f(m_x + 0.5f, m_y, 0);
	glEnd();
}

void AnatomyHelper::renderS2()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, 0.0, m_zc);  glVertex3f(m_x + 0.5f, 0, m_z);
		glTexCoord3f(m_xc, 0.0, 1.0); glVertex3f(m_x + 0.5f, 0, m_zb);
		glTexCoord3f(m_xc, m_yc, 1.0);  glVertex3f(m_x + 0.5f, m_y, m_zb);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x + 0.5f, m_y, m_z);
	glEnd();
}

void AnatomyHelper::renderS3()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, m_yc, 0);   glVertex3f(m_x + 0.5f, m_y,  0);
		glTexCoord3f(m_xc, m_yc, m_zc);  glVertex3f(m_x + 0.5f, m_y,  m_z);
		glTexCoord3f(m_xc, 1.0, m_zc); glVertex3f(m_x + 0.5f, m_yb, m_z);
		glTexCoord3f(m_xc, 1.0, 0);  glVertex3f(m_x + 0.5f, m_yb, 0);
	glEnd();
}

void AnatomyHelper::renderS4()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(m_xc, m_yc, m_zc);   glVertex3f(m_x + 0.5f, m_y,  m_z);
		glTexCoord3f(m_xc, m_yc, 1.0);  glVertex3f(m_x + 0.5f, m_y,  m_zb);
		glTexCoord3f(m_xc, 1.0, 1.0); glVertex3f(m_x + 0.5f, m_yb, m_zb);
		glTexCoord3f(m_xc, 1.0, m_zc);  glVertex3f(m_x + 0.5f, m_yb, m_z);
	glEnd();
}
