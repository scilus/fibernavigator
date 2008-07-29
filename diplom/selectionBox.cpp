#include "selectionBox.h"
#include "curves.h"

SelectionBox::SelectionBox(Vector3fT center, Vector3fT size, DatasetHelper* dh)
{
	m_dh = dh;
	m_center = center;
	m_size = size;
	m_show = true;
	m_dirty = true;
	m_isNOT = true;
	m_isActive = true;
	m_colorChanged = false;
	m_handleRadius = 3.0;
	m_inBox.resize(m_dh->countFibers, sizeof(bool));
	for (int i = 0; i < m_dh->countFibers ; ++i)
	{
		m_inBox[i] = 0;
	}
}

SelectionBox::SelectionBox(SelectionBox *box)
{
	m_dh = box->m_dh;
	m_center = box->getCenter();
	m_size = box->getSize();
	m_isNOT = false;
	m_isActive = true;
	m_show = true;
	m_dirty = true;
	m_colorChanged = false;

	m_handleRadius = 3.0;
	m_inBox.resize(m_dh->countFibers, sizeof(bool));
	for (int i = 0; i < m_dh->countFibers ; ++i)
	{
		m_inBox[i] = 0;
	}
}

void SelectionBox::drawHandles()
{
	m_handleRadius = 2.0 + Vector3fLength(&m_size)/50.0;
	float cx = m_center.s.X;
	float cy = m_center.s.Y;
	float cz = m_center.s.Z;
	mx = cx - m_size.s.X/2;
	px = cx + m_size.s.X/2;
	my = cy - m_size.s.Y/2;
	py = cy + m_size.s.Y/2;
	mz = cz - m_size.s.Z/2;
	pz = cz + m_size.s.Z/2;

	glColor3f(1.0, 0.0, 0.0);
	drawSphere(cx, cy, cz, m_handleRadius);
	drawSphere(mx - m_handleRadius, cy, cz, m_handleRadius);
	drawSphere(px + m_handleRadius, cy, cz, m_handleRadius);
	drawSphere(cx, my - m_handleRadius, cz, m_handleRadius);
	drawSphere(cx, py + m_handleRadius, cz, m_handleRadius);
	drawSphere(cx, cy, mz - m_handleRadius, m_handleRadius);
	drawSphere(cx, cy, pz + m_handleRadius, m_handleRadius);

}

void SelectionBox::drawFrame()
{
	float cx = m_center.s.X;
	float cy = m_center.s.Y;
	float cz = m_center.s.Z;
	if ( m_isTop )
		glColor3f(0.0, 1.0, 1.0);
	else {
		if ( !m_isNOT )
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
	Vector3fT vs = m_dh->mapMouse2World(x1, y1);
	Vector3fT ve = m_dh->mapMouse2World(x2, y2);
	Vector3fT dir = {{ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z}};
	float bb = ((dir.s.X * dir.s.X) + (dir.s.Y * dir.s.Y) + (dir.s.Z * dir.s.Z));
	float nb = ((dir.s.X * n.s.X) + (dir.s.Y * n.s.Y) + (dir.s.Z * n.s.Z));
	return bb/nb;
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
	Vector3fT vs = m_dh->mapMouse2World(click.x, click.y);
	Vector3fT ve = m_dh->mapMouse2WorldBack(click.x, click.y);
	Vector3fT dir = {{ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z}};

	m_center.s.X = vs.s.X + dir.s.X * m_hr.tmin;
	m_center.s.Y = vs.s.Y + dir.s.Y * m_hr.tmin;
	m_center.s.Z = vs.s.Z + dir.s.Z * m_hr.tmin;

	m_dirty = true;
}

void SelectionBox::resize(wxPoint click, wxPoint lastPos)
{
	Vector3fT  n= {{0,0,0}};
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
		m_size.s.X = wxMin(wxMax(newX, 1),m_dh->columns);
	}
	if (m_hr.picked == 13 || m_hr.picked == 14) {
		float newY = m_size.s.Y + (delta);
		m_size.s.Y = wxMin(wxMax(newY, 1),m_dh->rows);
	}
	if (m_hr.picked == 15 || m_hr.picked == 16) {
		float newZ = m_size.s.Z + (delta);
		m_size.s.Z = wxMin(wxMax(newZ, 1),m_dh->frames);
	}
	m_dirty = true;
}

void SelectionBox::moveLeft()
{
	if ( m_center.s.X < -m_dh->columns/2 ) return;
	m_center.s.X -= 1.0;
	update();
}

void SelectionBox::moveRight()
{
	if ( m_center.s.X > m_dh->columns/2 ) return;
	m_center.s.X += 1.0;
	update();
}

void SelectionBox::moveForward()
{
	if ( m_center.s.Y < -m_dh->rows/2 ) return;
	m_center.s.Y -= 1.0;
	update();
}

void SelectionBox::moveBack()
{
	if ( m_center.s.Y > m_dh->rows/2 ) return;
	m_center.s.Y += 1.0;
	update();
}

void SelectionBox::moveUp()
{
	if ( m_center.s.Z < -m_dh->frames/2 ) return;
	m_center.s.Z -= 1.0;
	update();
}

void SelectionBox::moveDown()
{
	if ( m_center.s.Z > m_dh->frames/2 ) return;
	m_center.s.Z += 1.0;
	update();
}

void SelectionBox::resizeLeft()
{
	if ( m_size.s.X < 2 ) return;
	m_size.s.X -= 1.0;
	update();
}

void SelectionBox::resizeRight()
{
	if ( m_size.s.X > m_dh->columns ) return;
	m_size.s.X += 1.0;
	update();
}

void SelectionBox::resizeBack()
{
	if ( m_size.s.Y < 2 ) return;
	m_size.s.Y -= 1.0;
	update();
}

void SelectionBox::resizeForward()
{
	if ( m_size.s.Y > m_dh->rows ) return;
	m_size.s.Y += 1.0;
	update();
}

void SelectionBox::resizeDown()
{
	if ( m_size.s.Z < 2 ) return;
	m_size.s.Z -= 1.0;
	update();
}

void SelectionBox::resizeUp()
{
	if ( m_size.s.Z > m_dh->frames ) return;
	m_size.s.Z += 1.0;
	update();
}

void SelectionBox::update()
{
	m_dirty = true;
	m_dh->scene->m_selBoxChanged = true;
	m_dh->mainFrame->refreshAllGLWidgets();
}

void SelectionBox::setColor(wxColour color)
{
	if (!m_isTop) return;
	m_color = color;
	m_colorChanged = true;
	update();
}
