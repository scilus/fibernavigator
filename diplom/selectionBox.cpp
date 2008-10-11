#include "selectionBox.h"
#include "fibers.h"

SelectionBox::SelectionBox(Vector3fT center, Vector3fT size, DatasetHelper* dh)
{
	m_dh = dh;
	m_center = center;
	m_size = size;
	m_show = true;
	m_dirty = true;
	m_isTop = true;
	m_isNOT = false;
	m_isActive = true;
	m_isSelected = false;
	m_colorChanged = false;
	m_treeId = NULL;
	m_handleRadius = 3.0;
	m_stepSize = 9;
	m_inBox.resize(m_dh->countFibers, sizeof(bool));
	for (unsigned int i = 0; i < m_dh->countFibers ; ++i)
	{
		m_inBox[i] = 0;
	}
	m_name = wxT("box");
}

void SelectionBox::select()
{
	if (m_treeId)
	{
		m_dh->mainFrame->m_treeWidget->SelectItem(m_treeId);
		m_dh->mainFrame->m_treeWidget->EnsureVisible(m_treeId);
		m_dh->mainFrame->m_treeWidget->SetFocus();
		m_isSelected = true;
		updateStatusBar();
	}
}

void SelectionBox::draw()
{
	if (!m_isActive || !m_show) return;

	float cx = m_center.s.X;
	float cy = m_center.s.Y;
	float cz = m_center.s.Z;
	mx = cx - m_size.s.X/2;
	px = cx + m_size.s.X/2;
	my = cy - m_size.s.Y/2;
	py = cy + m_size.s.Y/2;
	mz = cz - m_size.s.Z/2;
	pz = cz + m_size.s.Z/2;

	GLfloat c[] = { 0.5, 0.5, 0.5, 0.5};

	if ( m_isTop )
	{
		c[0] = 0.0f;
		c[1] = 1.0f;
		c[2] = 1.0f;
		c[3] = 0.2f;
	}
	else {
		if ( !m_isNOT )
		{
			c[0] = 0.0f;
			c[1] = 1.0f;
			c[2] = 0.0f;
			c[3] = 0.2f;
		}
		else
		{
			c[0] = 1.0f;
			c[1] = 0.0f;
			c[2] = 0.0f;
			c[3] = 0.2f;
		}
	}
	if (m_isSelected)
	{
		c[3] = 0.4f;
	}
	glColor4f(c[0], c[1], c[2], c[3]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
		glVertex3f(mx, my, mz);
		glVertex3f(mx, py, mz);
		glVertex3f(mx, py, pz);
		glVertex3f(mx, my, pz);
		glVertex3f(px, my, mz);
		glVertex3f(px, py, mz);
		glVertex3f(px, py, pz);
		glVertex3f(px, my, pz);

		glVertex3f(mx, my, mz);
		glVertex3f(px, my, mz);
		glVertex3f(px, my, pz);
		glVertex3f(mx, my, pz);
		glVertex3f(mx, py, mz);
		glVertex3f(px, py, mz);
		glVertex3f(px, py, pz);
		glVertex3f(mx, py, pz);

		glVertex3f(mx, my, mz);
		glVertex3f(mx, py, mz);
		glVertex3f(px, py, mz);
		glVertex3f(px, my, mz);
		glVertex3f(mx, my, pz);
		glVertex3f(mx, py, pz);
		glVertex3f(px, py, pz);
		glVertex3f(px, my, pz);
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
		switch (m_dh->quadrant)
		{
		case 2:
			draw2();
			draw4();
			draw5();
			draw1();
			draw3();
			draw6();
			break;
		case 3:
			draw2();
			draw3();
			draw5();
			draw1();
			draw4();
			draw6();
			break;
		case 6:
			draw1();
			draw3();
			draw5();
			draw2();
			draw4();
			draw6();
			break;
		case 7:
			draw1();
			draw4();
			draw5();
			draw2();
			draw3();
			draw6();
			break;
		case 1:
			draw2();
			draw4();
			draw6();
			draw1();
			draw3();
			draw5();
			break;
		case 4:
			draw2();
			draw3();
			draw6();
			draw1();
			draw4();
			draw5();
			break;
		case 5:
			draw1();
			draw3();
			draw6();
			draw2();
			draw4();
			draw5();
			break;
		case 8:
			draw1();
			draw4();
			draw6();
			draw2();
			draw3();
			draw5();
			break;
		}

	glEnd();

	glDisable(GL_BLEND);




}

void SelectionBox::draw1()
{
	glVertex3f(px, my, mz);
	glVertex3f(px, py, mz);
	glVertex3f(px, py, pz);
	glVertex3f(px, my, pz);
}

void SelectionBox::draw2()
{
	glVertex3f(mx, my, mz);
	glVertex3f(mx, py, mz);
	glVertex3f(mx, py, pz);
	glVertex3f(mx, my, pz);
}

void SelectionBox::draw3()
{
	glVertex3f(mx, py, mz);
	glVertex3f(px, py, mz);
	glVertex3f(px, py, pz);
	glVertex3f(mx, py, pz);
}

void SelectionBox::draw4()
{
	glVertex3f(mx, my, mz);
	glVertex3f(px, my, mz);
	glVertex3f(px, my, pz);
	glVertex3f(mx, my, pz);
}

void SelectionBox::draw5()
{
	glVertex3f(mx, my, pz);
	glVertex3f(mx, py, pz);
	glVertex3f(px, py, pz);
	glVertex3f(px, my, pz);
}

void SelectionBox::draw6()
{
	glVertex3f(mx, my, mz);
	glVertex3f(mx, py, mz);
	glVertex3f(px, py, mz);
	glVertex3f(px, my, mz);
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
	if (m_show && m_isActive) {
		float tpicked = 0;
		int picked = 0;
		float cx = m_center.s.X;
        float cy = m_center.s.Y;
        float cz = m_center.s.Z;


		BoundingBox *bb = new BoundingBox(m_center, m_size);
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
		if (wxGetKeyState(WXK_CONTROL))
		{
			bb->setCenter(mx - 1 , cy, cz);
			bb->setSize(m_size);
			bb->setSizeX(1);
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
			bb->setCenter(px + 1, cy, cz);
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
			bb->setCenter(cx, my - 1, cz);
			bb->setSize(m_size);
			bb->setSizeY(1);
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
			bb->setCenter(cx, py + 1, cz);
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
			bb->setCenter(cx, cy, mz - 1);
			bb->setSize(m_size);
			bb->setSizeZ(1);
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
			bb->setCenter(cx, cy, pz + 1);
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
		drag(click, lastPos);
	}
	else {
		resize(click, lastPos);
	}
}

void SelectionBox::drag(wxPoint click, wxPoint lastPos)
{
	//printf("(%d,%d) (%d,%d)\n", lastPos.x, lastPos.y, click.x, click.y );
	Vector3fT vs = m_dh->mapMouse2World(click.x, click.y);
	Vector3fT ve = m_dh->mapMouse2WorldBack(click.x, click.y);
	Vector3fT dir = {{ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z}};

	Vector3fT vs2 = m_dh->mapMouse2World(lastPos.x, lastPos.y);
	Vector3fT ve2 = m_dh->mapMouse2WorldBack(lastPos.x, lastPos.y);
	Vector3fT dir2 = {{ve2.s.X - vs2.s.X, ve2.s.Y - vs2.s.Y, ve2.s.Z - vs2.s.Z}};

	Vector3fT change =
		{{(vs.s.X + dir.s.X * m_hr.tmin) - (vs2.s.X + dir2.s.X * m_hr.tmin),
		  (vs.s.Y + dir.s.Y * m_hr.tmin) - (vs2.s.Y + dir2.s.Y * m_hr.tmin),
		  (vs.s.Z + dir.s.Z * m_hr.tmin) - (vs2.s.Z + dir2.s.Z * m_hr.tmin)}};

	m_center.s.X += change.s.X;
	m_center.s.Y += change.s.Y;
	m_center.s.Z += change.s.Z;

	updateStatusBar();
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
	updateStatusBar();
	m_dirty = true;
}

void SelectionBox::moveLeft()
{
	if ( m_center.s.X < -m_dh->columns/2 ) return;
	m_center.s.X = (int)m_center.s.X - 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.s.X -= m_stepSize;
	update();
}

void SelectionBox::moveRight()
{
	if ( m_center.s.X > m_dh->columns/2 ) return;
		m_center.s.X = (int)m_center.s.X + 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.s.X += m_stepSize;
	update();
}

void SelectionBox::moveForward()
{
	if ( m_center.s.Y < -m_dh->rows/2 ) return;
	m_center.s.Y = (int)m_center.s.Y - 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.s.Y -= m_stepSize;
	update();
}

void SelectionBox::moveBack()
{
	if ( m_center.s.Y > m_dh->rows/2 ) return;
	m_center.s.Y = (int)m_center.s.Y + 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.s.Y += m_stepSize;
	update();
}

void SelectionBox::moveUp()
{
	if ( m_center.s.Z < -m_dh->frames/2 ) return;
	m_center.s.Z = (int)m_center.s.Z - 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.s.Z -= m_stepSize;
	update();
}

void SelectionBox::moveDown()
{
	if ( m_center.s.Z > m_dh->frames/2 ) return;
	m_center.s.Z = (int)m_center.s.Z + 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.s.Z += m_stepSize;
	update();
}

void SelectionBox::resizeLeft()
{
	if ( m_size.s.X < 2 ) return;
	m_size.s.X -= 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.s.X = wxMax(1 ,m_size.s.X - m_stepSize);
	update();
}

void SelectionBox::resizeRight()
{
	if ( m_size.s.X > m_dh->columns ) return;
	m_size.s.X += 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.s.X += m_stepSize;
	update();
}

void SelectionBox::resizeBack()
{
	if ( m_size.s.Y < 2 ) return;
	m_size.s.Y -= 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.s.Y = wxMax(1, m_size.s.Y - m_stepSize);
	update();
}

void SelectionBox::resizeForward()
{
	if ( m_size.s.Y > m_dh->rows ) return;
	m_size.s.Y += 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.s.Y += m_stepSize;
	update();
}

void SelectionBox::resizeDown()
{
	if ( m_size.s.Z < 2 ) return;
	m_size.s.Z -= 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.s.Z = wxMax(1, m_size.s.Z - m_stepSize);
	update();
}

void SelectionBox::resizeUp()
{
	if ( m_size.s.Z > m_dh->frames ) return;
	m_size.s.Z += 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.s.Z += m_stepSize;
	update();
}

void SelectionBox::update()
{
	updateStatusBar();
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

void SelectionBox::updateStatusBar()
{
	m_dh->mainFrame->m_statusBar->SetStatusText(wxT("Selection Box"),1);
	m_dh->mainFrame->m_statusBar->SetStatusText(wxString::Format(wxT("Position %.2f, %.2f, %.2f  Size: %.0f, %.0f, %.0f"),
			m_center.s.X + m_dh->columns/2, m_center.s.Y + m_dh->rows/2, m_center.s.Z + m_dh->frames/2, m_size.s.X, m_size.s.Y, m_size.s.Z),2);

}
