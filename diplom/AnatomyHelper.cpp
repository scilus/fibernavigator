/*
 * AnatomyHelper.cpp
 *
 *  Created on: 23.07.2008
 *      Author: ralph
 */
#include "AnatomyHelper.h"

#include "GL/glew.h"

AnatomyHelper::AnatomyHelper(DatasetHelper* dh) {
	m_dh = dh;
}

AnatomyHelper::~AnatomyHelper() {
	// TODO Auto-generated destructor stub
}

void AnatomyHelper::renderNav(int view, Shader *shader)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	m_dh->scene->bindTextures();
	shader->bind();
	m_dh->shaderHelper->setTextureShaderVars();

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0001f);

	float xline = 0;
	float yline = 0;

	float max = (float)wxMax(m_dh->columns, wxMax(m_dh->rows, m_dh->frames));

	int x = m_dh->columns;
	int y = m_dh->rows;
	int z = m_dh->frames;

	float xo = -(m_dh->columns - max)/2.0;
	float yo = -(m_dh->rows    - max)/2.0;
	float zo = -(m_dh->frames  - max)/2.0;

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
			xline = m_dh->xSlize + xo;
			yline = m_dh->ySlize;
		} break;

		case coronal: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows, 0.0); glVertex3f( 0 + xo, 0 + zo, quadZ);
		    	glTexCoord3f(0.0, ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows, 1.0); glVertex3f( 0 + xo, z + zo, quadZ);
		    	glTexCoord3f(1.0, ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows, 1.0); glVertex3f( x + xo, z + zo, quadZ);
		    	glTexCoord3f(1.0, ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows, 0.0); glVertex3f( x + xo, 0 + zo, quadZ);
		    glEnd();
		    xline = m_dh->xSlize + xo;
		    yline = m_dh->zSlize + zo;
		} break;

		case sagittal: {
			glBegin(GL_QUADS);
				glTexCoord3f(((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns, 1.0, 0.0); glVertex3f( 0 + yo, 0 + zo, quadZ);
		    	glTexCoord3f(((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns, 1.0, 1.0); glVertex3f( 0 + yo, z + zo, quadZ);
		    	glTexCoord3f(((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns, 0.0, 1.0); glVertex3f( y + yo, z + zo, quadZ);
		    	glTexCoord3f(((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns, 0.0, 0.0); glVertex3f( y + yo, 0 + zo, quadZ);
			glEnd();
			xline = max - m_dh->ySlize;
			yline = m_dh->zSlize + zo;
		} break;
	}

	glDisable(GL_TEXTURE_3D);
	shader->release();
	glPopAttrib();

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
	x = m_dh->xSlize + 0.5f;
	y = m_dh->ySlize + 0.5f;
	z = m_dh->zSlize + 0.5f;

	xc = ((float)m_dh->xSlize + 0.5f)/(float)m_dh->columns;
	yc = ((float)m_dh->ySlize + 0.5f)/(float)m_dh->rows;
	zc = ((float)m_dh->zSlize + 0.5f)/(float)m_dh->frames;

	xb = m_dh->columns;
	yb = m_dh->rows;
	zb = m_dh->frames;
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

void AnatomyHelper::renderAxial()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, 0.0, zc); glVertex3f(0,  0,  z);
		glTexCoord3f(0.0, 1.0, zc); glVertex3f(0,  yb, z);
		glTexCoord3f(1.0, 1.0, zc); glVertex3f(xb, yb, z);
		glTexCoord3f(1.0, 0.0, zc); glVertex3f(xb, 0,  z);
	glEnd();
}

void AnatomyHelper::renderCoronal()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, yc, 0.0); glVertex3f(0,  y, 0);
		glTexCoord3f(0.0, yc, 1.0); glVertex3f(0,  y, zb);
		glTexCoord3f(1.0, yc, 1.0); glVertex3f(xb, y, zb);
		glTexCoord3f(1.0, yc, 0.0); glVertex3f(xb, y, 0);
	glEnd();
}

void AnatomyHelper::renderSagittal()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, 0.0, 0.0); glVertex3f(x, 0,  0);
		glTexCoord3f(xc, 0.0, 1.0); glVertex3f(x, 0,  zb);
		glTexCoord3f(xc, 1.0, 1.0); glVertex3f(x, yb, zb);
		glTexCoord3f(xc, 1.0, 0.0); glVertex3f(x, yb, 0);
	glEnd();
}

void AnatomyHelper::renderA1()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, 0.0, zc); glVertex3f(0, 0, z + 0.5f);
		glTexCoord3f(0.0, yc, zc);  glVertex3f(0, y, z + 0.5f);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x, y, z + 0.5f);
		glTexCoord3f(xc, 0.0,zc);   glVertex3f(x, 0, z + 0.5f);
	glEnd();
}

void AnatomyHelper::renderA2()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, yc, zc);  glVertex3f(0, y,  z + 0.5f);
		glTexCoord3f(0.0, 1.0, zc); glVertex3f(0, yb, z + 0.5f);
		glTexCoord3f(xc, 1.0, zc);  glVertex3f(x, yb, z + 0.5f);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x, y,  z + 0.5f);
	glEnd();
}

void AnatomyHelper::renderA3()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, 0, zc);   glVertex3f(x,  0, z + 0.5f);
		glTexCoord3f(xc, yc, zc);  glVertex3f(x,  y, z + 0.5f);
		glTexCoord3f(1.0, yc, zc); glVertex3f(xb, y, z + 0.5f);
		glTexCoord3f(1.0, 0, zc);  glVertex3f(xb, 0, z + 0.5f);
	glEnd();
}

void AnatomyHelper::renderA4()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x,  y,  z + 0.5f);
		glTexCoord3f(xc, 1.0, zc);  glVertex3f(x,  yb, z + 0.5f);
		glTexCoord3f(1.0, 1.0, zc); glVertex3f(xb, yb, z + 0.5f);
		glTexCoord3f(1.0, yc, zc);  glVertex3f(xb, y,  z + 0.5f);
	glEnd();

}

void AnatomyHelper::renderC1()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, yc, 0.0); glVertex3f(0, y + 0.5f, 0);
		glTexCoord3f(0.0, yc, zc);  glVertex3f(0, y + 0.5f, z);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x, y + 0.5f, z);
		glTexCoord3f(xc, yc, 0.0);  glVertex3f(x, y + 0.5f, 0);
	glEnd();
}

void AnatomyHelper::renderC2()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, yc, zc);  glVertex3f(0, y + 0.5f, z);
		glTexCoord3f(0.0, yc, 1.0); glVertex3f(0, y + 0.5f, zb);
		glTexCoord3f(xc, yc, 1.0);  glVertex3f(x, y + 0.5f, zb);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x, y + 0.5f, z);
	glEnd();
}

void AnatomyHelper::renderC3()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, 0);   glVertex3f(x,  y + 0.5f, 0);
		glTexCoord3f(xc, yc, zc);  glVertex3f(x,  y + 0.5f, z);
		glTexCoord3f(1.0, yc, zc); glVertex3f(xb, y + 0.5f, z);
		glTexCoord3f(1.0, yc, 0);  glVertex3f(xb, y + 0.5f, 0);
	glEnd();
}

void AnatomyHelper::renderC4()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x,  y + 0.5f, z);
		glTexCoord3f(xc, yc, 1.0);  glVertex3f(x,  y + 0.5f, zb);
		glTexCoord3f(1.0, yc, 1.0); glVertex3f(xb, y + 0.5f, zb);
		glTexCoord3f(1.0, yc, zc);  glVertex3f(xb, y + 0.5f, z);
	glEnd();

}

void AnatomyHelper::renderS1()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, 0.0, 0.0); glVertex3f(x + 0.5f, 0, 0);
		glTexCoord3f(xc, 0.0, zc);  glVertex3f(x + 0.5f, 0, z);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x + 0.5f, y, z);
		glTexCoord3f(xc, yc, 0.0);  glVertex3f(x + 0.5f, y, 0);
	glEnd();
}

void AnatomyHelper::renderS2()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, 0.0, zc);  glVertex3f(x + 0.5f, 0, z);
		glTexCoord3f(xc, 0.0, 1.0); glVertex3f(x + 0.5f, 0, zb);
		glTexCoord3f(xc, yc, 1.0);  glVertex3f(x + 0.5f, y, zb);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x + 0.5f, y, z);
	glEnd();
}

void AnatomyHelper::renderS3()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, 0);   glVertex3f(x + 0.5f, y,  0);
		glTexCoord3f(xc, yc, zc);  glVertex3f(x + 0.5f, y,  z);
		glTexCoord3f(xc, 1.0, zc); glVertex3f(x + 0.5f, yb, z);
		glTexCoord3f(xc, 1.0, 0);  glVertex3f(x + 0.5f, yb, 0);
	glEnd();
}

void AnatomyHelper::renderS4()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, zc);   glVertex3f(x + 0.5f, y,  z);
		glTexCoord3f(xc, yc, 1.0);  glVertex3f(x + 0.5f, y,  zb);
		glTexCoord3f(xc, 1.0, 1.0); glVertex3f(x + 0.5f, yb, zb);
		glTexCoord3f(xc, 1.0, zc);  glVertex3f(x + 0.5f, yb, z);
	glEnd();
}
