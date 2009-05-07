#include "selectionBox.h"
#include "../dataset/fibers.h"
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../dataset/Anatomy.h"

SelectionBox::SelectionBox(Vector center, Vector size, DatasetHelper* dh)
{
	m_dh = dh;
	m_isBox = true;
	m_center = center;
	m_size = size;
	m_isVisible = true;
	m_dirty = true;
	m_gfxDirty = true;
	m_isMaster = false;
	m_isNOT = false;
	m_isActive = true;
	m_isSelected = false;
	m_colorChanged = false;
	m_isLockedToCrosshair = false;
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

SelectionBox::SelectionBox(DatasetHelper* dh, Anatomy* a)
{
	m_dh = dh;
	m_isBox = false;
	m_isVisible = true;
	m_dirty = true;
	m_isMaster = true;
	m_isNOT = false;
	m_isActive = true;
	m_isSelected = false;
	m_colorChanged = false;
	wxColour col(240, 30, 30);
	m_color = col;
	m_isLockedToCrosshair = false;
	m_treeId = NULL;
	m_handleRadius = 3.0;
	m_stepSize = 9;
	m_threshold = 0;
	m_inBox.resize(m_dh->countFibers, sizeof(bool));
	for (unsigned int i = 0; i < m_dh->countFibers ; ++i)
	{
		m_inBox[i] = 0;
	}
	m_name = wxT("ROI");
	m_sourceAnatomy = a;
	m_isosurface = new CIsoSurface(m_dh, a);
}

SelectionBox::~SelectionBox()
{
	if (m_isLockedToCrosshair)
	{
		m_dh->boxLockIsOn = false;
	}
	if ( !m_isBox)
	{
		delete m_isosurface;
		if ( m_sourceAnatomy && m_sourceAnatomy->m_roi == this)
		{
			m_sourceAnatomy->m_roi = NULL;
		}
	}
}

void SelectionBox::select(bool flag)
{
	if (m_treeId)
	{
		if (flag)
		{
			m_dh->mainFrame->m_treeWidget->SelectItem(m_treeId);
			m_dh->mainFrame->m_treeWidget->EnsureVisible(m_treeId);
			m_dh->mainFrame->m_treeWidget->SetFocus();
		}
		m_isSelected = true;
		updateStatusBar();
	}
}

void SelectionBox::drawIsoSurface()
{
	if ( !m_isActive || !m_isVisible ) return;

	if ( m_gfxDirty )
	{
		m_isosurface->setThreshold( m_threshold );
		m_isosurface->GenerateWithThreshold();
		m_gfxDirty = false;
	}
	glColor3ub(m_color.Red(), m_color.Green(), m_color.Blue());
	m_isosurface->draw();
}

void SelectionBox::draw()
{
	if ( !m_isActive || !m_isVisible ) return;

	if ( !m_isBox )
	{
		return;
	}

	float cx = m_center.x;
	float cy = m_center.y;
	float cz = m_center.z;
	mx = cx - ( m_size.x/2 * m_dh->xVoxel );
	px = cx + ( m_size.x/2 * m_dh->xVoxel );
	my = cy - ( m_size.y/2 * m_dh->yVoxel );
	py = cy + ( m_size.y/2 * m_dh->yVoxel );
	mz = cz - ( m_size.z/2 * m_dh->zVoxel );
	pz = cz + ( m_size.z/2 * m_dh->zVoxel );

	GLfloat c[] = { 0.5, 0.5, 0.5, 0.5};

	if ( m_isMaster )
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
	hitResult hr = {false, 0.0f, 0, NULL};
	if (m_isVisible && m_isActive && m_isBox) {
		float tpicked = 0;
		int picked = 0;
		float cx = m_center.x;
        float cy = m_center.y;
        float cz = m_center.z;
        float sx = m_size.x * m_dh->xVoxel;
        float sy = m_size.y * m_dh->yVoxel;
        float sz = m_size.z * m_dh->zVoxel;

       	if (wxGetKeyState(WXK_CONTROL))
		{
       		BoundingBox *bb = new BoundingBox(cx, cy, cz, sx, sy, sz);

			bb->setCenter(mx - 1 , cy, cz);
			bb->setSize(sx, sy, sz);
			bb->setSizeX(m_dh->xVoxel);
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
			bb->setSize(sx, sy, sz);
			bb->setSizeY(m_dh->yVoxel);
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
			bb->setSize(sx, sy, sz);
			bb->setSizeZ(m_dh->zVoxel);
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

		else  // if (wxGetKeyState(WXK_CONTROL))
		{
			BoundingBox *bb = new BoundingBox(cx, cy, cz, sx, sy, sz);
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
		}
		if (picked != 0)
		{
			hr.hit = true;
			hr.tmin = tpicked;
			hr.picked = picked;
			hr.object = this;
		}

	}
	m_hr = hr;
	return hr;
}

float SelectionBox::getAxisParallelMovement(int x1, int y1, int x2, int y2, Vector n)
{
	Vector vs = m_dh->mapMouse2World(x1, y1);
	Vector ve = m_dh->mapMouse2World(x2, y2);
	Vector dir ( ve.x - vs.x, ve.y - vs.y, ve.z - vs.z );
	float bb = ((dir.x * dir.x) + (dir.y * dir.y) + (dir.z * dir.z));
	float nb = ((dir.x * n.x) + (dir.y * n.y) + (dir.z * n.z));
	return bb/nb;
}

void SelectionBox::setCenter(float x, float y, float z)
{
	m_center.x = x;
	m_center.y = y;
	m_center.z = z;
	update();
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
	Vector vs = m_dh->mapMouse2World(click.x, click.y);
	Vector ve = m_dh->mapMouse2WorldBack(click.x, click.y);
	Vector dir ( ve.x - vs.x, ve.y - vs.y, ve.z - vs.z );

	Vector vs2 = m_dh->mapMouse2World(lastPos.x, lastPos.y);
	Vector ve2 = m_dh->mapMouse2WorldBack(lastPos.x, lastPos.y);
	Vector dir2 ( ve2.x - vs2.x, ve2.y - vs2.y, ve2.z - vs2.z );

	Vector change (
		(vs.x + dir.x * m_hr.tmin) - (vs2.x + dir2.x * m_hr.tmin),
		(vs.y + dir.y * m_hr.tmin) - (vs2.y + dir2.y * m_hr.tmin),
		(vs.z + dir.z * m_hr.tmin) - (vs2.z + dir2.z * m_hr.tmin) );

	m_center.x += change.x;
	m_center.y += change.y;
	m_center.z += change.z;

	update();
}

void SelectionBox::resize(wxPoint click, wxPoint lastPos)
{
	Vector  n ( 0, 0, 0 );
	float delta = 0;
	switch (m_hr.picked)
	{
	case 11:
		n.x = -1.0;
		break;
	case 12:
		n.x = 1.0;
		break;
	case 13:
		n.y = -1.0;
		break;
	case 14:
		n.y = 1.0;
		break;
	case 15:
		n.z = -1.0;
		break;
	case 16:
		n.z = 1.0;
		break;
	default:;
	}
	delta =  wxMax(wxMin(getAxisParallelMovement(lastPos.x, lastPos.y, click.x, click.y, n ),1),-1);
	if (m_hr.picked == 11 || m_hr.picked == 12) {
		float newX = m_size.x + (delta);
		m_size.x = wxMin(wxMax(newX, 1),m_dh->columns);
	}
	if (m_hr.picked == 13 || m_hr.picked == 14) {
		float newY = m_size.y + (delta);
		m_size.y = wxMin(wxMax(newY, 1),m_dh->rows);
	}
	if (m_hr.picked == 15 || m_hr.picked == 16) {
		float newZ = m_size.z + (delta);
		m_size.z = wxMin(wxMax(newZ, 1),m_dh->frames);
	}
	update();
}

void SelectionBox::moveLeft()
{
	if ( m_center.x < 0 ) return;
	m_center.x = (int)m_center.x - 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.x -= m_stepSize;
	update();
}

void SelectionBox::moveRight()
{
	if ( m_center.x > m_dh->columns ) return;
		m_center.x = (int)m_center.x + 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.x += m_stepSize;
	update();
}

void SelectionBox::moveForward()
{
	if ( m_center.y < 0 ) return;
	m_center.y = (int)m_center.y - 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.y -= m_stepSize;
	update();
}

void SelectionBox::moveBack()
{
	if ( m_center.y > m_dh->rows ) return;
	m_center.y = (int)m_center.y + 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.y += m_stepSize;
	update();
}

void SelectionBox::moveUp()
{
	if ( m_center.z < 0 ) return;
	m_center.z = (int)m_center.z - 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.z -= m_stepSize;
	update();
}

void SelectionBox::moveDown()
{
	if ( m_center.z > m_dh->frames ) return;
	m_center.z = (int)m_center.z + 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_center.z += m_stepSize;
	update();
}

void SelectionBox::resizeLeft()
{
	if ( m_size.x < 2 ) return;
	m_size.x -= 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.x = wxMax(1 ,m_size.x - m_stepSize);
	update();
}

void SelectionBox::resizeRight()
{
	if ( m_size.x > m_dh->columns ) return;
	m_size.x += 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.x += m_stepSize;
	update();
}

void SelectionBox::resizeBack()
{
	if ( m_size.y < 2 ) return;
	m_size.y -= 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.y = wxMax(1, m_size.y - m_stepSize);
	update();
}

void SelectionBox::resizeForward()
{
	if ( m_size.y > m_dh->rows ) return;
	m_size.y += 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.y += m_stepSize;
	update();
}

void SelectionBox::resizeDown()
{
	if ( m_size.z < 2 ) return;
	m_size.z -= 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.z = wxMax(1, m_size.z - m_stepSize);
	update();
}

void SelectionBox::resizeUp()
{
	if ( m_size.z > m_dh->frames ) return;
	m_size.z += 1.0;
	if (wxGetKeyState(WXK_SHIFT)) m_size.z += m_stepSize;
	update();
}

void SelectionBox::update()
{
	if (m_isLockedToCrosshair)
	{
		m_dh->semaphore = true;
		m_dh->updateView((int)m_center.x , (int)m_center.y , (int)m_center.z);
		m_dh->mainFrame->m_xSlider->SetValue((int)m_center.x);
		m_dh->mainFrame->m_ySlider->SetValue((int)m_center.y);
		m_dh->mainFrame->m_zSlider->SetValue((int)m_center.z);
		m_dh->semaphore = false;
	}

	updateStatusBar();
	m_dirty = true;
	m_dh->m_selBoxChanged = true;
	m_dh->mainFrame->refreshAllGLWidgets();
}

void SelectionBox::setColor(wxColour color)
{
	m_color = color;
	update();
}

void SelectionBox::setFiberColor(wxColour color)
{
    m_fiberColor = color;
    if (!m_isMaster) return;
    m_colorChanged = true;
    update();
}



void SelectionBox::updateStatusBar()
{
	m_dh->mainFrame->GetStatusBar()->SetStatusText(wxT("Selection Box"),1);
	m_dh->mainFrame->GetStatusBar()->SetStatusText(wxString::Format(wxT("Position %.2f, %.2f, %.2f  Size: %.2f, %.2f, %.2f"),
			m_center.x , m_center.y , m_center.z , m_size.x * m_dh->xVoxel, m_size.y * m_dh->yVoxel, m_size.z * m_dh->zVoxel),2);

}

void SelectionBox::lockToCrosshair()
{
	if (m_isLockedToCrosshair)
	{
		m_isLockedToCrosshair = false;
		m_dh->boxLockIsOn = false;
	}
	else
	{
		if (m_dh->boxLockIsOn)
		{
			m_dh->boxAtCrosshair->m_isLockedToCrosshair = false;
		}
		m_isLockedToCrosshair = true;
		m_dh->boxLockIsOn = true;
		m_dh->boxAtCrosshair = this;
		m_dh->semaphore = true;
		m_dh->updateView((int)m_center.x , (int)m_center.y , (int)m_center.z);
		m_dh->mainFrame->m_xSlider->SetValue((int)m_center.x);
		m_dh->mainFrame->m_ySlider->SetValue((int)m_center.y);
		m_dh->mainFrame->m_zSlider->SetValue((int)m_center.z);
		m_dh->semaphore = false;
		m_dh->mainFrame->refreshAllGLWidgets();
	}
}

void SelectionBox::setDirty(bool v)
{
	m_dirty = v;
	m_dh->m_selBoxChanged = true;
}

int SelectionBox::getIcon()
{
	if (m_isActive && m_isVisible)
		return 1;
	else if (m_isActive && !m_isVisible)
		return -1;
	else
		return 0;
}

void SelectionBox::setIsMaster(bool v)
{
	m_isMaster = v;
	if (m_isMaster)
	{
		m_inBranch.resize(m_dh->countFibers, sizeof(bool));
		for (unsigned int i = 0; i < m_dh->countFibers ; ++i)
		{
			m_inBranch[i] = 0;
		}
	}
}

void SelectionBox::setThreshold(float v)
{
	m_threshold = v;
	m_dirty = true;
	m_gfxDirty = true;
	m_dh->m_selBoxChanged = true;
}

int SelectionBox::countSelectedFibers()
{
    int count = 0;
    for (unsigned int i = 0; i < m_dh->countFibers ; ++i)
    {
        if ( m_inBranch[i] ) ++count;
    }
    return count;
}
