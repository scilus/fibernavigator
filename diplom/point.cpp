#include "point.h"
#include "selectionBox.h"

Point::Point(Vector3fT center)
{
	m_center = center;
	m_dirty = true;
}


void Point::draw()
{
	glColor3f(0.0, 0.8, 0.8);
	drawSphere(m_center.s.X, m_center.s.Y, m_center.s.Z, 2);
}

void Point::drawSphere(float x, float y, float z, float r)
{
	glPushMatrix();
	glTranslatef(x,y,z);
	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluSphere(quadric, r, 32, 32);
	glPopMatrix();
}

void Point::drag(wxPoint click)
{
	Vector3fT vs = SelectionBox::mapMouse2World(click.x, click.y);
	Vector3fT ve = SelectionBox::mapMouse2WorldBack(click.x, click.y);
	Vector3fT dir = {{ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z}};

	m_center.s.X = vs.s.X + dir.s.X * m_hr.tmin;
	m_center.s.Y = vs.s.Y + dir.s.Y * m_hr.tmin;
	m_center.s.Z = vs.s.Z + dir.s.Z * m_hr.tmin;
	m_dirty = true;
}

hitResult Point::hitTest(Ray *ray)
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
