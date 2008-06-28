#include "selectionBox.h"
#include "theDataset.h"
#include "curves.h"

SelectionBox::SelectionBox(Vector3fT center, Vector3fT size, int lines)
{
	m_center = center;
	m_size = size;
	m_show = true;
	m_dirty = true;
	m_isAND = true;
	m_handleRadius = 3.0;	
	m_lines = lines;
	m_inBox.resize(lines, sizeof(bool));
	for (int i = 0; i < lines ; ++i)
	{
		m_inBox[i] = 0;
	}
}

SelectionBox::SelectionBox(SelectionBox *box)
{
	m_center = box->getCenter();
	m_size = box->getSize();
	m_show = true;
	m_dirty = true;
	
	m_handleRadius = 3.0;
	m_lines = box->m_lines;
	m_inBox.resize(m_lines, sizeof(bool));
	for (int i = 0; i < m_lines ; ++i)
	{
		m_inBox[i] = 0;
	}
}

void SelectionBox::draw()
{
	float cx = m_center.s.X;
	float cy = m_center.s.Y;
	float cz = m_center.s.Z;
	mx = wxMin(cx - m_size.s.X/2, cx - m_handleRadius);
	px = wxMax(cx + m_size.s.X/2, cx + m_handleRadius);
	my = wxMin(cy - m_size.s.Y/2, cy - m_handleRadius);
	py = wxMax(cy + m_size.s.Y/2, cy + m_handleRadius);
	mz = wxMin(cz - m_size.s.Z/2, cz - m_handleRadius);
	pz = wxMax(cz + m_size.s.Z/2, cz + m_handleRadius);
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	
	glColor3f(1.0, 0.0, 0.0);
	drawSphere(cx, cy, cz, m_handleRadius);
	drawSphere(mx - m_handleRadius, cy, cz, m_handleRadius);
	drawSphere(px + m_handleRadius, cy, cz, m_handleRadius);
	drawSphere(cx, my - m_handleRadius, cz, m_handleRadius);
	drawSphere(cx, py + m_handleRadius, cz, m_handleRadius);
	drawSphere(cx, cy, mz - m_handleRadius, m_handleRadius);
	drawSphere(cx, cy, pz + m_handleRadius, m_handleRadius);
	if ( m_isTop )
		glColor3f(0.0, 1.0, 1.0);
	else {
		if ( !m_isAND )
			glColor3f(0.0, 1.0, 0.0);
		else
			glColor3f(1.0, 0.0, 0.0);
	}
		
	//glLineWidth(2.0);
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
	
	glDisable(GL_COLOR_MATERIAL);
}

void SelectionBox::drawSphere(float x, float y, float z, float r)
{
	glPushMatrix();
	glTranslatef(x,y,z);
	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluSphere(quadric, r, 32, 32);
	glPopMatrix();
}

hitResult SelectionBox::hitTest(Ray *ray)
{
	hitResult hr;
	if (m_show) {
		float cx = m_center.s.X;
		float cy = m_center.s.Y;
		float cz = m_center.s.Z;
		float tpicked = 0;
		int picked = 0;

		BoundingBox *bb = new BoundingBox(cx, cy, cz, 6, 6, 6);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = 10;
				tpicked = hr.tmin;
				
			}
			else {
				if (hr.tmin < tpicked) {
					picked = 10;
					tpicked = hr.tmin;
				}
			}
		}
		bb->setCenter(mx - m_handleRadius, cy, cz);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = 11;
				tpicked = hr.tmin;
			}
			else {
				if (hr.tmin < tpicked) {
					picked = 11;
					tpicked = hr.tmin;
				}
			}
		}
		bb->setCenter(px + m_handleRadius, cy, cz);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = 12;
				tpicked = hr.tmin;
			}
			else {
				if (hr.tmin < tpicked) {
					picked = 12;
					tpicked = hr.tmin;
				}
			}
		}
		bb->setCenter(cx, my - m_handleRadius, cz);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = 13;
				tpicked = hr.tmin;
			}
			else {
				if (hr.tmin < tpicked) {
					picked = 13;
					tpicked = hr.tmin;
				}
			}
		}
		bb->setCenter(cx, py + m_handleRadius, cz);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = 14;
				tpicked = hr.tmin;
			}
			else {
				if (hr.tmin < tpicked) {
					picked = 14;
					tpicked = hr.tmin;
				}
			}
		}
		bb->setCenter(cx, cy, mz - m_handleRadius);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = 15;
				tpicked = hr.tmin;
			}
			else {
				if (hr.tmin < tpicked) {
					picked = 15;
					tpicked = hr.tmin;
				}
			}
		}
		bb->setCenter(cx, cy, pz + m_handleRadius);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = 16;
				tpicked = hr.tmin;
			}
			else {
				if (hr.tmin < tpicked) {
					picked = 16;
					tpicked = hr.tmin;
				}
			}
		}
		if (picked != 0) {
			hr.hit = true;
			hr.tmin = tpicked;
			hr.picked = picked;
			hr.object = this;
		}
	}
	m_hr = hr;
	return hr;
}

float SelectionBox::getAxisParallelMovement(int x1, int y1, int x2, int y2, Vector3fT n)
{
	Vector3fT vs = mapMouse2World(x1, y1); 
	Vector3fT ve = mapMouse2World(x2, y2);
	Vector3fT dir = {ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z};
	float bb = ((dir.s.X * dir.s.X) + (dir.s.Y * dir.s.Y) + (dir.s.Z * dir.s.Z));
	float nb = ((dir.s.X * n.s.X) + (dir.s.Y * n.s.Y) + (dir.s.Z * n.s.Z));
	return bb/nb;
}

Vector3fT SelectionBox::mapMouse2World(int x, int y)
{
	glPushMatrix();
	glMultMatrixf(TheDataset::m_transform.M);	
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	GLdouble posX, posY, posZ;
	gluUnProject( winX, winY, 0, modelview, projection, viewport, &posX, &posY, &posZ);
	glPopMatrix();
	Vector3fT v = {posX, posY, posZ};
	return v;
}

Vector3fT SelectionBox::mapMouse2WorldBack(int x, int y)
{
	glPushMatrix();
	glMultMatrixf(TheDataset::m_transform.M);	
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	GLdouble posX, posY, posZ;
	gluUnProject( winX, winY, 1, modelview, projection, viewport, &posX, &posY, &posZ);
	glPopMatrix();
	Vector3fT v = {posX, posY, posZ};
	return v;
}


void SelectionBox::processDrag(wxPoint click, wxPoint lastPos)
{
	if (m_hr.picked == 10) {
		drag(click);
	}
	else {
		resize(click, lastPos);
	}
}

void SelectionBox::drag(wxPoint click)
{
	Vector3fT vs = mapMouse2World(click.x, click.y);
	Vector3fT ve = mapMouse2WorldBack(click.x, click.y);
	Vector3fT dir = {ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z};

	m_center.s.X = vs.s.X + dir.s.X * m_hr.tmin;
	m_center.s.Y = vs.s.Y + dir.s.Y * m_hr.tmin;
	m_center.s.Z = vs.s.Z + dir.s.Z * m_hr.tmin;
	m_dirty = true;
}

void SelectionBox::resize(wxPoint click, wxPoint lastPos)
{
	Vector3fT  n= {0,0,0};
	float delta = 0;
	switch (m_hr.picked)
	{
	case 11:
		n.s.X = -1.0;
		break;
	case 12:
		n.s.X = 1.0;
		break;
	case 13:
		n.s.Y = -1.0;
		break;
	case 14:
		n.s.Y = 1.0;
		break;
	case 15:
		n.s.Z = -1.0;
		break;
	case 16:
		n.s.Z = 1.0;
		break;
	default:;
	}
	delta =  wxMax(wxMin(getAxisParallelMovement(lastPos.x, lastPos.y, click.x, click.y, n ),1),-1);
	if (m_hr.picked == 11 || m_hr.picked == 12) {
		float newX = m_size.s.X + (delta);
		m_size.s.X = wxMin(wxMax(newX, 1),TheDataset::columns);
	}
	if (m_hr.picked == 13 || m_hr.picked == 14) {
		float newY = m_size.s.Y + (delta);
		m_size.s.Y = wxMin(wxMax(newY, 1),TheDataset::rows);
	}
	if (m_hr.picked == 15 || m_hr.picked == 16) {
		float newZ = m_size.s.Z + (delta);
		m_size.s.Z = wxMin(wxMax(newZ, 1),TheDataset::frames);
	}
	m_dirty = true;
}
