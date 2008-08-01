#include "splinePoint.h"
#include "selectionBox.h"

SplinePoint::SplinePoint(Vector3fT center, DatasetHelper* dh)
{
	m_center = center;
	m_origin = center;
	Vector3fSetZero(&m_offsetVector);
	m_dh = dh;
	m_dirty = true;
	m_selected = false;
	m_dh->surface_isDirty = true;
	m_treeId = NULL;
}

SplinePoint::SplinePoint(double x, double y, double z, DatasetHelper* dh)
{

	Vector3fT center = {{x,y,z}};
	m_center = center;
	m_origin = center;
	m_dh = dh;
	Vector3fSetZero(&m_offsetVector);
	m_dirty = true;
	m_selected = false;
	m_dh->surface_isDirty = true;
}

SplinePoint::~SplinePoint()
{
	m_dh->surface_isDirty = true;
}

void SplinePoint::draw()
{
	if (m_selected)
		glColor3f(1.0f, 0.8f, 0.0f);
	else
		glColor3f(0.0f, 0.8f, 0.8f);
	drawSphere(m_center.s.X, m_center.s.Y, m_center.s.Z, 2);
}

void SplinePoint::drawSphere(float x, float y, float z, float r)
{
	glPushMatrix();
	glTranslatef(x,y,z);
	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluSphere(quadric, r, 32, 32);
	glPopMatrix();
}

void SplinePoint::drag(wxPoint click)
{
	Vector3fT vs = m_dh->mapMouse2World(click.x, click.y);
	Vector3fT ve = m_dh->mapMouse2WorldBack(click.x, click.y);
	Vector3fT dir = {{ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z}};

	m_center.s.X = vs.s.X + dir.s.X * m_hr.tmin;
	m_center.s.Y = vs.s.Y + dir.s.Y * m_hr.tmin;
	m_center.s.Z = vs.s.Z + dir.s.Z * m_hr.tmin;
	m_dirty = true;
	m_dh->surface_isDirty = true;
}

hitResult SplinePoint::hitTest(Ray *ray)
{
	hitResult hr;

	float cx = m_center.s.X;
	float cy = m_center.s.Y;
	float cz = m_center.s.Z;

	BoundingBox *bb = new BoundingBox(cx, cy, cz, 6, 6, 6);
	hr = bb->hitTest(ray);

	if (hr.hit) {
		hr.picked = 20;
		hr.object = this;
	}
	m_hr = hr;
	return hr;
}

void SplinePoint::move(float w)
{
	m_center.s.X = m_origin.s.X + m_offsetVector.s.X * w;
	m_center.s.Y = m_origin.s.Y + m_offsetVector.s.Y * w;
	m_center.s.Z = m_origin.s.Z + m_offsetVector.s.Z * w;
}


void SplinePoint::select()
{
	m_selected = true;
	if (m_treeId)
	{
		m_dh->mainFrame->m_treeWidget->SelectItem(m_treeId);
		m_dh->mainFrame->m_treeWidget->EnsureVisible(m_treeId);
	}
}
