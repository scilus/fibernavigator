#include "splinePoint.h"
#include "selectionBox.h"

SplinePoint::SplinePoint(Vector center, DatasetHelper* dh)
{
	m_center = center;
	m_origin = center;
	Vector v(0, 0, 0);
	m_offsetVector = v;
	m_dh = dh;
	m_selected = false;
	m_isBoundary = false;
	m_dh->surface_isDirty = true;
	m_treeId = NULL;
}

SplinePoint::SplinePoint(double x, double y, double z, DatasetHelper* dh)
{

	Vector center (x, y, z );
	m_center = center;
	m_origin = center;
	m_dh = dh;
	Vector v(0, 0, 0);
	m_offsetVector = v;
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
	else if (m_isBoundary)
		glColor3f(1.0f, 0.0f, 0.0f);
	else
		glColor3f(0.0f, 0.8f, 0.8f);
	drawSphere(m_center.x, m_center.y, m_center.z, 2);
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
	Vector vs = m_dh->mapMouse2World(click.x, click.y);
	Vector ve = m_dh->mapMouse2WorldBack(click.x, click.y);
	Vector dir( ve.x - vs.x, ve.y - vs.y, ve.z - vs.z );

	m_center.x = vs.x + dir.x * m_hr.tmin;
	m_center.y = vs.y + dir.y * m_hr.tmin;
	m_center.z = vs.z + dir.z * m_hr.tmin;
	m_dh->surface_isDirty = true;
}

hitResult SplinePoint::hitTest(Ray *ray)
{
	hitResult hr;

	float cx = m_center.x;
	float cy = m_center.y;
	float cz = m_center.z;

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
	m_center.x = m_origin.x + m_offsetVector.x * w;
	m_center.y = m_origin.y + m_offsetVector.y * w;
	m_center.z = m_origin.z + m_offsetVector.z * w;
}


void SplinePoint::select(bool flag)
{
	m_selected = true;
	if (m_treeId && flag)
	{
		m_dh->mainFrame->m_treeWidget->SelectItem(m_treeId);
		m_dh->mainFrame->m_treeWidget->EnsureVisible(m_treeId);
	}
}

void SplinePoint::setX(float x)
{
	m_center.x = x;
	m_origin.x = x;
}
