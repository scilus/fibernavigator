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

void AnatomyHelper::renderNav(int view, FGLSLShaderProgram *shader)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	m_dh->scene->bindTextures();
	shader->bind();
	m_dh->shaderHelper->setTextureShaderVars();

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0000001);

	float xline = 0;
	float yline = 0;

	float border = (float)wxMax(m_dh->columns, wxMax(m_dh->rows, m_dh->frames))/2.0;

	int x = m_dh->columns/2;
	int y = m_dh->rows/2;
	int z = m_dh->frames/2;

	int xs = m_dh->xSlize - m_dh->columns/2;
	int ys = m_dh->ySlize - m_dh->rows/2;
	int zs = m_dh->zSlize - m_dh->frames/2;

	switch (view)
	{
		case axial: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, 1.0, m_dh->zSlize/(float)m_dh->frames); glVertex3f(-x, -y, zs);
		    	glTexCoord3f(0.0, 0.0, m_dh->zSlize/(float)m_dh->frames); glVertex3f(-x,  y, zs);
		    	glTexCoord3f(1.0, 0.0, m_dh->zSlize/(float)m_dh->frames); glVertex3f( x,  y, zs);
		    	glTexCoord3f(1.0, 1.0, m_dh->zSlize/(float)m_dh->frames); glVertex3f( x, -y, zs);
			glEnd();
			xline = m_dh->xSlize - (float)m_dh->columns/2.0;
			yline = (float)m_dh->rows/2.0 - m_dh->ySlize;
		} break;

		case coronal: {
			glBegin(GL_QUADS);
				glTexCoord3f(0.0, m_dh->ySlize/(float)m_dh->rows, 1.0); glVertex3f( -x, -z, ys);
		    	glTexCoord3f(0.0, m_dh->ySlize/(float)m_dh->rows, 0.0); glVertex3f( -x,  z, ys);
		    	glTexCoord3f(1.0, m_dh->ySlize/(float)m_dh->rows, 0.0); glVertex3f(  x,  z, ys);
		    	glTexCoord3f(1.0, m_dh->ySlize/(float)m_dh->rows, 1.0); glVertex3f(  x, -z, ys);
		    glEnd();
		    xline = m_dh->xSlize - (float)m_dh->columns/2.0;
		    yline = (float)m_dh->frames/2.0 - m_dh->zSlize;
		} break;

		case sagittal: {
			glBegin(GL_QUADS);
				glTexCoord3f(m_dh->xSlize/(float)m_dh->columns, 0.0, 1.0); glVertex3f(-y, -z, xs);
		    	glTexCoord3f(m_dh->xSlize/(float)m_dh->columns, 0.0, 0.0); glVertex3f(-y,  z, xs);
		    	glTexCoord3f(m_dh->xSlize/(float)m_dh->columns, 1.0, 0.0); glVertex3f( y,  z, xs);
		    	glTexCoord3f(m_dh->xSlize/(float)m_dh->columns, 1.0, 1.0); glVertex3f( y, -z, xs);
			glEnd();
			xline = m_dh->ySlize - (float)m_dh->rows/2.0;
			yline = (float)m_dh->frames/2.0 - m_dh->zSlize;
		} break;
	}

	glDisable(GL_TEXTURE_3D);
	shader->release();
	glPopAttrib();

	glColor3f(1.0, 0.0, 0.0);
	glBegin (GL_LINES);
		glVertex3f (-border, yline, border);
		glVertex3f ( border, yline, border);
		glVertex3f (xline, -border, border);
		glVertex3f (xline,  border, border);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
}

void AnatomyHelper::renderMain()
{
	x = m_dh->xSlize - m_dh->columns/2;
	y = m_dh->ySlize - m_dh->rows/2;
	z = m_dh->zSlize - m_dh->frames/2;

	xc = m_dh->xSlize/(float)m_dh->columns;
	yc = m_dh->ySlize/(float)m_dh->rows;
	zc = m_dh->zSlize/(float)m_dh->frames;

	xb = m_dh->columns/2;
	yb = m_dh->rows/2;
	zb = m_dh->frames/2;

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
}

void AnatomyHelper::renderA1()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, 0.0, zc); glVertex3i(-xb, -yb, z);
		glTexCoord3f(0.0, yc, zc); glVertex3i(-xb,  y, z);
		glTexCoord3f(xc, yc, zc); glVertex3i(x,  y, z);
		glTexCoord3f(xc, 0.0,zc); glVertex3i(x, -yb, z);
	glEnd();
}

void AnatomyHelper::renderA2()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, yc, zc); glVertex3i(-xb, y, z);
		glTexCoord3f(0.0, 1.0, zc); glVertex3i(-xb,  yb, z);
		glTexCoord3f(xc, 1.0, zc); glVertex3i(x,  yb, z);
		glTexCoord3f(xc, yc, zc); glVertex3i(x, y, z);
	glEnd();
}

void AnatomyHelper::renderA3()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, 0, zc); glVertex3i(x, -yb, z);
		glTexCoord3f(xc, yc, zc); glVertex3i(x, y, z);
		glTexCoord3f(1.0, yc, zc); glVertex3i(xb,  y, z);
		glTexCoord3f(1.0, 0, zc); glVertex3i(xb, -yb, z);
	glEnd();
}

void AnatomyHelper::renderA4()
{
	if (!m_dh->showAxial) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, zc); glVertex3i(x, y, z);
		glTexCoord3f(xc, 1.0, zc); glVertex3i(x, yb, z);
		glTexCoord3f(1.0, 1.0, zc); glVertex3i(xb, yb, z);
		glTexCoord3f(1.0, yc, zc); glVertex3i(xb, y, z);
	glEnd();

}

void AnatomyHelper::renderC1()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, yc, 0.0); glVertex3i(-xb, y, -zb);
		glTexCoord3f(0.0, yc, zc); glVertex3i(-xb, y,  z);
		glTexCoord3f(xc, yc, zc); glVertex3i(x,  y,  z);
		glTexCoord3f(xc, yc, 0.0); glVertex3i(x,  y, -zb);
	glEnd();
}

void AnatomyHelper::renderC2()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(0.0, yc, zc); glVertex3i(-xb, y, z);
		glTexCoord3f(0.0, yc, 1.0); glVertex3i(-xb, y,  zb);
		glTexCoord3f(xc, yc, 1.0); glVertex3i(x,  y,  zb);
		glTexCoord3f(xc, yc, zc); glVertex3i(x,  y, z);
	glEnd();
}

void AnatomyHelper::renderC3()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, 0); glVertex3i(x, y, -zb);
		glTexCoord3f(xc, yc, zc); glVertex3i(x, y, z);
		glTexCoord3f(1.0, yc, zc); glVertex3i(xb, y,  z);
		glTexCoord3f(1.0, yc, 0); glVertex3i(xb, y, -zb);
	glEnd();
}

void AnatomyHelper::renderC4()
{
	if (!m_dh->showCoronal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, zc); glVertex3i(x, y, z);
		glTexCoord3f(xc, yc, 1.0); glVertex3i(x, y, zb);
		glTexCoord3f(1.0, yc, 1.0); glVertex3i(xb, y, zb);
		glTexCoord3f(1.0, yc, zc); glVertex3i(xb, y, z);
	glEnd();

}

void AnatomyHelper::renderS1()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, 0.0, 0.0); glVertex3i(x, -yb, -zb);
		glTexCoord3f(xc, 0.0, zc); glVertex3i(x, -yb,  z);
		glTexCoord3f(xc, yc, zc); glVertex3i(x,  y,  z);
		glTexCoord3f(xc, yc, 0.0); glVertex3i(x,  y, -zb);
	glEnd();
}

void AnatomyHelper::renderS2()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, 0.0, zc); glVertex3i(x, -yb, z);
		glTexCoord3f(xc, 0.0, 1.0); glVertex3i(x, -yb,  zb);
		glTexCoord3f(xc, yc, 1.0); glVertex3i(x,  y,  zb);
		glTexCoord3f(xc, yc, zc); glVertex3i(x,  y, z);
	glEnd();
}

void AnatomyHelper::renderS3()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, 0); glVertex3i(x, y, -zb);
		glTexCoord3f(xc, yc, zc); glVertex3i(x, y, z);
		glTexCoord3f(xc, 1.0, zc); glVertex3i(x, yb,  z);
		glTexCoord3f(xc, 1.0, 0); glVertex3i(x, yb, -zb);
	glEnd();
}

void AnatomyHelper::renderS4()
{
	if (!m_dh->showSagittal) return;
	glBegin(GL_QUADS);
		glTexCoord3f(xc, yc, zc); glVertex3i(x, y, z);
		glTexCoord3f(xc, yc, 1.0); glVertex3i(x, y, zb);
		glTexCoord3f(xc, 1.0, 1.0); glVertex3i(x, yb, zb);
		glTexCoord3f(xc, 1.0, zc); glVertex3i(x, yb, z);
	glEnd();
}
