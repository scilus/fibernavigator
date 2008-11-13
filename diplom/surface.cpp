#include "surface.h"
#include "math.h"
#include "Fantom/FMatrix.h"
#include "surface.h"
#include "splinePoint.h"
#include "myListCtrl.h"
#include "Anatomy.h"

#include "GL/glew.h"

Surface::Surface(DatasetHelper* dh)
{
	m_dh = dh;
	m_radius = 30.0;
	m_my = 8.0;
	m_numDeBoorRows = 12;
	m_numDeBoorCols = 12;
	m_order = 4;

	m_sampleRateT = m_sampleRateU = 0.5;

	m_show = true;
	m_showFS = true;
	m_useTex = true;

	m_type = Surface_;
	m_hasTreeId = false;
	m_threshold = 0.5;
	m_name = wxT("spline surface");
	m_dh->surface_loaded = true;
	m_numPoints = 0;
	m_hasTreeId = false;
	m_alpha = 0.2;
	execute();
}

Surface::~Surface()
{
	m_dh->surface_loaded = false;
}

FTensor Surface::getCovarianceMatrix(std::vector< std::vector< double > > points)
{
	FTensor result(3,2,true);
	m_xAverage = m_yAverage = m_zAverage = 0;

	std::vector< std::vector< double > >::iterator pointsIt;
	for( pointsIt = points.begin(); pointsIt != points.end(); pointsIt++)
	{
		std::vector< double > dmy = *pointsIt;
		m_xAverage += dmy[0];
		m_yAverage += dmy[1];
		m_zAverage += dmy[2];
	}

	m_xAverage /= points.size();
	m_yAverage /= points.size();
	m_zAverage /= points.size();

	/*
    	 /        \
   		| XX XY XZ |
   		| YX YY YZ |
   		| ZX ZY ZZ |
    	 \        /
	 */
	for( pointsIt = points.begin(); pointsIt != points.end(); pointsIt++)
	{
		std::vector< double > dmy = *pointsIt;

		result(0,0) += (dmy[0] - m_xAverage) * (dmy[0] - m_xAverage); //XX
		result(0,1) += (dmy[0] - m_xAverage) * (dmy[1] - m_yAverage); //XY
		result(0,2) += (dmy[0] - m_xAverage) * (dmy[2] - m_zAverage); //XZ

		result(1,1) += (dmy[1] - m_yAverage) * (dmy[1] - m_yAverage); //YY
		result(1,2) += (dmy[1] - m_yAverage) * (dmy[2] - m_zAverage); //YZ

		result(2,2) += (dmy[2] - m_zAverage) * (dmy[2] - m_zAverage); //ZZ
	}

	result(1,0) = result(0,1);
	result(2,0) = result(0,2);
	result(2,1) = result(1,2);

	return result;
}

void Surface::getSplineSurfaceDeBoorPoints(std::vector< std::vector< double > > &givenPoints, std::vector< std::vector< double > > &deBoorPoints, int numRows, int numCols)
{
	double xMin = givenPoints[0][0];
	double xMax = givenPoints[0][0];
	double zMin = givenPoints[0][2];
	double zMax = givenPoints[0][2];

	std::vector< std::vector< double > >::iterator givenPointsIt;

	for( givenPointsIt = givenPoints.begin(); givenPointsIt != givenPoints.end(); givenPointsIt++)
	{
		std::vector< double > dmy = *givenPointsIt;
		if(dmy[0] < xMin)
			xMin = dmy[0];
		if(dmy[0] > xMax)
			xMax = dmy[0];
		if(dmy[2] < zMin)
			zMin = dmy[2];
		if(dmy[2] > zMax)
			zMax = dmy[2];
	}

	double dX = (xMax - xMin) / (numCols - 1);
	double dZ = (zMax - zMin) / (numRows - 1);


	for( int row = 0; row < numRows; ++row)
		for( int col = 0; col < numCols; ++col)
		{
			std::vector< double > dmy;
			double x = xMin + dX * col;
			double z = zMin + dZ * row;
			dmy.push_back(x);

			double y = 0;
			double numerator = 0, denominator = 0;

			//<local shepard with franke-little-weights>
			for( givenPointsIt = givenPoints.begin(); givenPointsIt != givenPoints.end(); givenPointsIt++)
			{
				std::vector< double > dmy1 = *givenPointsIt;
				FArray dmyArray(dmy1);
				dmyArray[1] = 0;
				FArray thisPoint(x,0,z);

				double xi; //greek alphabet

				if( thisPoint.distance(dmyArray) < m_radius)
					xi = 1 - thisPoint.distance(dmyArray) / m_radius;
				else
					xi = 0;

				numerator += (pow(xi, m_my) * dmy1[1]);
				denominator += (pow(xi, m_my));
			}
			if( denominator == 0)
				y = 0;
			else
				y = numerator / denominator;
			//</local shepard with franke-little-weights>
			dmy.push_back(y);
			dmy.push_back(z);

			deBoorPoints.push_back(dmy);
		}
  return;
}

void Surface::execute ()
{
	std::vector< std::vector< double > > givenPoints;
	int countPoints = m_dh->mainFrame->m_treeWidget->GetChildrenCount(m_dh->mainFrame->m_tPointId, true);
	if (countPoints == 0) return;

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	id = m_dh->mainFrame->m_treeWidget->GetFirstChild(m_dh->mainFrame->m_tPointId, cookie);
	while ( id.IsOk() )
	{
		SplinePoint *point = (SplinePoint*)(m_dh->mainFrame->m_treeWidget->GetItemData(id));
		std::vector< double > p;
		p.push_back(point->getCenter().s.X);
		p.push_back(point->getCenter().s.Y);
		p.push_back(point->getCenter().s.Z);
		givenPoints.push_back(p);

		id = m_dh->mainFrame->m_treeWidget->GetNextChild(m_dh->mainFrame->m_tPointId, cookie);
	}

	std::vector< std::vector< double > > deBoorPoints;
	m_splinePoints.clear();
	m_vertices.clear();

	FTensor myTensor = getCovarianceMatrix(givenPoints);

	FArray eigenValues(3);
	FArray eigenVectors[3];
	eigenVectors[0] = FArray(3);
	eigenVectors[1] = FArray(3);
	eigenVectors[2] = FArray(3);

	myTensor.getEigenSystem(eigenValues, eigenVectors);

	eigenVectors[0].normalize();
	eigenVectors[1].normalize();
	eigenVectors[2].normalize();

	FTensor::sortEigenvectors(eigenValues, eigenVectors);

	FMatrix transMatrix = FMatrix(3,3);
	transMatrix(0,0) = eigenVectors[1][0];
	transMatrix(0,1) = eigenVectors[1][1];
	transMatrix(0,2) = eigenVectors[1][2];

	transMatrix(1,0) = eigenVectors[2][0];
	transMatrix(1,1) = eigenVectors[2][1];
	transMatrix(1,2) = eigenVectors[2][2];

	transMatrix(2,0) = eigenVectors[0][0];
	transMatrix(2,1) = eigenVectors[0][1];
	transMatrix(2,2) = eigenVectors[0][2];

	std::vector< std::vector< double > >::iterator pointsIt;

	//translate and orientate given points to origin
	for( pointsIt = givenPoints.begin(); pointsIt != givenPoints.end(); pointsIt++)
	{
		(*pointsIt)[0] -= m_xAverage;
		(*pointsIt)[1] -= m_yAverage;
		(*pointsIt)[2] -= m_zAverage;

		FArray dmy(*pointsIt);

		F::FVector result = transMatrix * dmy;
		(*pointsIt)[0] = result[0];
		(*pointsIt)[1] = result[1];
		(*pointsIt)[2] = result[2];
	}

	//get de Boor points using shepard's method
	getSplineSurfaceDeBoorPoints(givenPoints, deBoorPoints, m_numDeBoorRows, m_numDeBoorCols);

	//translate and orientate de Boor points back
	transMatrix.invert();
	for( pointsIt = deBoorPoints.begin(); pointsIt != deBoorPoints.end(); pointsIt++)
	{
		FArray dmy(*pointsIt);

		F::FVector result = transMatrix * dmy;
		(*pointsIt)[0] = result[0];
		(*pointsIt)[1] = result[1];
		(*pointsIt)[2] = result[2];

		(*pointsIt)[0] += m_xAverage;
		(*pointsIt)[1] += m_yAverage;
		(*pointsIt)[2] += m_zAverage;
	}


	FBSplineSurface splineSurface(m_order, m_order, deBoorPoints, m_numDeBoorCols, m_numDeBoorRows);

	splineSurface.samplePoints(m_splinePoints, m_sampleRateT, m_sampleRateU);

	std::vector< double > positions;

	for( std::vector< std::vector< double > >::iterator posIt = m_splinePoints.begin(); posIt != m_splinePoints.end(); posIt++)
	{
		positions.push_back((*posIt)[0]);
		positions.push_back((*posIt)[1]);
		positions.push_back((*posIt)[2]);
	}

	m_renderpointsPerCol = splineSurface.getNumSamplePointsU();
	m_renderpointsPerRow = splineSurface.getNumSamplePointsT();


	int pi0, pi1, pi2, pi3;
	m_normals.clear();
	std::vector< F::FVector > quadNormals;

	// TODO
	if (m_dh->m_isrDragging)
	{
		for (int i = 0 ; i < 2 ; ++i)
			overSamplePoints();
	}
	else
	{
		for (int i = 0 ; i < 4 ; ++i)
			overSamplePoints();
	}

	m_numPoints = m_renderpointsPerRow * m_renderpointsPerCol;
	std::vector< std::vector<int> >quadRef(m_numPoints, std::vector<int>(0,0));
	std::vector< double > p, n;

	float* vectorField = 0;
	Anatomy* anatomy;

	if (m_dh->vectors_loaded)
	{
		//get pointer to vector field
		for (int i = 0 ; i < m_dh->mainFrame->m_listCtrl->GetItemCount() ; ++i)
		{
			DatasetInfo* info = (DatasetInfo*)m_dh->mainFrame->m_listCtrl->GetItemData(i);

			if (info->getType() == Vectors_ )
			{
				anatomy = (Anatomy*)info;
				vectorField = anatomy->getFloatDataset();
			}
		}
		m_colorArray = new float[m_splinePoints.size()*3];
	}

	for(int z = 0; z < m_renderpointsPerCol - 1; z++)
	{
		for(int x = 0; x < m_renderpointsPerRow - 1; x++)
		{
			pi0 = z     * m_renderpointsPerRow + x;
			pi1 = (z+1) * m_renderpointsPerRow + x;
			pi2 = (z+1) * m_renderpointsPerRow + x + 1;
			pi3 = z     * m_renderpointsPerRow + x + 1;

			m_vertices.push_back(pi0);
			m_vertices.push_back(pi1);
			m_vertices.push_back(pi2);
			m_vertices.push_back(pi3);

			p = m_splinePoints[pi0];
			F::FVector p0(p[0], p[1], p[2]);
			quadRef[pi0].push_back(quadNormals.size());

			p = m_splinePoints[pi1];
			F::FVector p1(p[0], p[1], p[2]);
			quadRef[pi1].push_back(quadNormals.size());

			p = m_splinePoints[pi2];
			F::FVector p2(p[0], p[1], p[2]);
			quadRef[pi2].push_back(quadNormals.size());

			p = m_splinePoints[pi3];
			F::FVector p3(p[0], p[1], p[2]);
			quadRef[pi3].push_back(quadNormals.size());

			F::FVector n1 = getNormalForQuad(&p0, &p1, &p2);
			quadNormals.push_back(n1);
		}
	}

	for (int i = 0 ; i < m_numPoints ; ++i )
	{
		F::FVector tmp(0.0,0.0,0.0);
		for ( unsigned int j = 0 ; j < quadRef[i].size() ; ++j)
		{
			 tmp += quadNormals[quadRef[i][j]];
		}
		F::FVector n( tmp[0] / quadRef[i].size() * m_dh->normalDirection,
				   tmp[1] / quadRef[i].size() * m_dh->normalDirection,
				   tmp[2] / quadRef[i].size() * m_dh->normalDirection);
		m_normals.push_back(n);
	}

	m_vertexArray = new float[m_splinePoints.size()*3];
	m_normalArray = new float[m_splinePoints.size()*3];

	int x,y,z;
	for (unsigned int i = 0 ; i < m_splinePoints.size() ; ++i)
	{
		p = m_splinePoints[i];
		m_vertexArray[ 3 * i    ] = p[0];
		m_vertexArray[ 3 * i + 1] = p[1];
		m_vertexArray[ 3 * i + 2] = p[2];
		n = m_normals[i];
		m_normalArray[ 3 * i    ] = n[0];
		m_normalArray[ 3 * i + 1] = n[1];
		m_normalArray[ 3 * i + 2] = n[2];

		x = wxMin(m_dh->columns-1, wxMax(0,(int)p[0]));
		y = wxMin(m_dh->rows   -1, wxMax(0,(int)p[1]));
		z = wxMin(m_dh->frames -1, wxMax(0,(int)p[2]));

		if (m_dh->vectors_loaded)
		{
			int index = (x + y * m_dh->columns + z * m_dh->columns * m_dh->rows)*3;
			m_colorArray[ 3 * i    ] = vectorField[ index];
			m_colorArray[ 3 * i + 1] = vectorField[ index + 1 ];
			m_colorArray[ 3 * i + 2] = vectorField[ index + 2 ];
		}
	}

	m_indexArray = new unsigned int [m_vertices.size()];
	for (unsigned int i = 0 ; i < m_vertices.size() ; ++i)
	{
		m_indexArray[i] = m_vertices[i];
	}

	m_dh->surface_isDirty = false;

	createCutTexture();
}

F::FVector Surface::getNormalForQuad(const F::FVector* p1, const F::FVector* p2, const F::FVector* p3)
{
	F::FVector a = *p2 - *p1;
	F::FVector b = *p3 - *p1;
	F::FVector n = a.crossProduct(b);
	return n.normalize();
}

void Surface::draw()
{
	if (m_dh->surface_isDirty)
	{
		execute();
	}

	//m_dh->mainFrame->m_gl0->testRender(m_GLuint);

	if (m_dh->scene->getPointMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// TODO i have no clue why the draw arrays don't work
	// apparently everything is ok, as this rendering pass shows
#if 1
	int numQuads = (m_renderpointsPerCol - 1) * (m_renderpointsPerRow - 1);
	int i0, i1, i2;
	glBegin(GL_QUADS);
	if (m_dh->vectors_loaded)
	{
		for (int i = 0 ; i < numQuads*4 ; ++i)
		{
			i0 = m_indexArray[i]*3;
			i1 = m_indexArray[i]*3+1;
			i2 = m_indexArray[i]*3+2;
			glNormal3f(m_normalArray[i0], m_normalArray[i1], m_normalArray[i2]);
			glColor3f(m_colorArray[i0], m_colorArray[i1], m_colorArray[i2]);
			glVertex3f(m_vertexArray[i0], m_vertexArray[i1], m_vertexArray[i2]);
		}
	}
	else
	{
		for (int i = 0 ; i < numQuads*4 ; ++i)
		{
			i0 = m_indexArray[i]*3;
			i1 = m_indexArray[i]*3+1;
			i2 = m_indexArray[i]*3+2;
			glNormal3f(m_normalArray[i0], m_normalArray[i1], m_normalArray[i2]);
			glVertex3f(m_vertexArray[i0], m_vertexArray[i1], m_vertexArray[i2]);
		}
	}
	glEnd();
#else

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, m_vertexArray);
	glNormalPointer (GL_FLOAT, 0, m_normalArray);

	if (m_dh->vectors_loaded)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer (3, GL_FLOAT, 0, m_colorArray);
	}

	glColor3f(0.1, 0.1, 0.1);

	int numQuads = (m_renderpointsPerCol - 1) * (m_renderpointsPerRow - 1);
	glDrawElements(GL_QUADS, numQuads*4, GL_UNSIGNED_INT, m_indexArray);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

#endif
}

void Surface::movePoints()
{
	int countPoints = m_dh->mainFrame->m_treeWidget->GetChildrenCount(m_dh->mainFrame->m_tPointId, true);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = m_dh->mainFrame->m_treeWidget->GetNextChild(m_dh->mainFrame->m_tPointId, cookie);
		SplinePoint *point = (SplinePoint*)m_dh->mainFrame->m_treeWidget->GetItemData(id);
		point->move(2.0 * m_threshold);
	}
	execute();
}

void Surface::createCutTexture()
{
#if 0
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport); // get the current viewport

	// initialize Framebuffer for offscreen rendering ( calculates proper texture sizes for us )
	FgeOffscreen fbo(viewport[2], viewport[3], true);

	fbo.setClearColor(0.0, 0.0, 0.0);
	fbo.activate();
	fbo.addDepthTexture(true);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
/*
	float maxLength = (float)wxMax(m_dh->columns, wxMax(m_dh->rows, m_dh->frames));
	float view1 = maxLength;
	glLoadIdentity();
	glOrtho( 0, view1, 0, view1, 0, view1);
*/
	// render
	draw();

	fbo.deactivate();

	// restore matrix stuff
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	// and restore all attributes to previous status
	glPopAttrib();

	m_GLuint = fbo.getDepthTexID();

#else
	int xDim = m_dh->rows;
	int yDim = m_dh->frames;
	int numPoints = m_renderpointsPerCol * m_renderpointsPerRow;

	m_pointArray= new float[numPoints*3];
	for (int i = 0 ; i < numPoints ; ++i )
	{
		F::FVector p = m_splinePoints[i];
		m_pointArray[3*i] = p[0];
		m_pointArray[3*i+1] = p[1];
		m_pointArray[3*i+2] = p[2];
	}
	m_kdTree = new KdTree(numPoints, m_pointArray);

	float* cutTex;
	cutTex = new float[xDim*yDim];

	for ( int x = 0 ; x < xDim ; ++x )
	{
		for ( int y = 0 ; y < yDim ; ++y)
		{
			cutTex[x + y*xDim] = getXValue(x, y, numPoints) / (float) m_dh->columns;

		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &m_GLuint);
	glBindTexture(GL_TEXTURE_2D, m_GLuint);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D,	0, GL_RGB, xDim, yDim, 0, GL_LUMINANCE, GL_FLOAT, cutTex);
#endif
}

float Surface::getXValue(int y, int z, int numPoints)
{
	m_boxMin = new float[3];
	m_boxMax = new float[3];
	m_boxMin[0] = 0;
	m_boxMax[0] = m_dh->columns;
	m_boxMin[1] = y - 3;
	m_boxMax[1] = y + 3;
	m_boxMin[2] = z - 3;
	m_boxMax[2] = z + 3;

	m_xValue = 0.0;
	m_count = 0;
	boxTest(0, numPoints-1, 0);
	if (m_count > 0)
		return m_xValue / m_count;
	else {
		for (int i = 0 ; i < 3 ; ++i)
		{
			m_boxMin[1] -= 5;
			m_boxMax[1] += 5;
			m_boxMin[2] -= 5;
			m_boxMax[2] += 5;
			boxTest(0, numPoints-1, 0);
			if (m_count > 0)
				return m_xValue / m_count;
		}
		return m_dh->columns / 2;
	}
}

void Surface::boxTest(int left, int right, int axis)
{
	// abort condition
	if (left > right) return;

	int root = left + ((right-left)/2);
	int axis1 = (axis+1) % 3;
	int pointIndex = m_kdTree->m_tree[root]*3;

	if (m_pointArray[pointIndex + axis] < m_boxMin[axis]) {
		boxTest(root +1, right, axis1);
	}
	else if (m_pointArray[pointIndex + axis] > m_boxMax[axis]) {
		boxTest(left, root-1, axis1);
	}
	else {
		int axis2 = (axis+2) % 3;
		if (	m_pointArray[pointIndex + axis1] <= m_boxMax[axis1] &&
				m_pointArray[pointIndex + axis1] >= m_boxMin[axis1] &&
				m_pointArray[pointIndex + axis2] <= m_boxMax[axis2] &&
				m_pointArray[pointIndex + axis2] >= m_boxMin[axis2] )
		{
			m_xValue += m_pointArray[pointIndex];
			++m_count;
		}
		boxTest(left, root -1, axis1);
		boxTest(root+1, right, axis1);
	}
}

void Surface::overSamplePoints()
{
	std::vector< std::vector< double > > tempPoints = m_splinePoints;
	m_splinePoints.clear();

	int pi0, pi1, pi2, pi3;

	m_numPoints = m_renderpointsPerRow * m_renderpointsPerCol;
	std::vector< std::vector<int> >quadRef(m_numPoints, std::vector<int>(0,0));
	std::vector< double > p;
	F::FVector p1, p2, p3, p4;

	for(int z = 0 ; z < m_renderpointsPerCol - 1; z++)
	{
		for(int x = 0 ; x < m_renderpointsPerRow - 1 ; x++)
		{
			pi0 = z * m_renderpointsPerRow + x;
			pi1 = z * m_renderpointsPerRow + x + 1;

			p1 = tempPoints[pi0];
			p2 = tempPoints[pi1];

			m_splinePoints.push_back(p1);

			F::FVector p5( (p1[0] + p2[0]) / 2.0 , (p1[1] + p2[1]) / 2.0 , (p1[2] + p2[2]) / 2.0);

			m_splinePoints.push_back(p5);
		}
		m_splinePoints.push_back(p2);

		for(int x = 0 ; x < m_renderpointsPerRow - 1 ; x++)
		{
			pi0 = z * m_renderpointsPerRow + x;
			pi1 = z * m_renderpointsPerRow + x + 1;
			pi2 = (z+1) * m_renderpointsPerRow + x;
			pi3 = (z+1) * m_renderpointsPerRow + x + 1;

			p1 = tempPoints[pi0];
			p2 = tempPoints[pi1];
			p3 = tempPoints[pi2];
			p4 = tempPoints[pi3];

			F::FVector p5( (p1[0] + p3[0]) / 2.0 , (p1[1] + p3[1]) / 2.0 , (p1[2] + p3[2]) / 2.0);
			m_splinePoints.push_back(p5);

			F::FVector p6( (p1[0] + p2[0] + p3[0] + p4[0]) / 4.0 , (p1[1] + p2[1] + p3[1] + p4[1]) / 4.0 , (p1[2] + p2[2] + p3[2] + p4[2]) / 4.0);
			m_splinePoints.push_back(p6);

		}
		F::FVector p7( (p2[0] + p4[0]) / 2.0 , (p2[1] + p4[1]) / 2.0 , (p2[2] + p4[2]) / 2.0);
		m_splinePoints.push_back(p7);

	}

	for(int x = 0 ; x < m_renderpointsPerRow - 1 ; x++)
	{
		pi0 = (m_renderpointsPerCol - 1) * m_renderpointsPerRow + x;
		pi1 = (m_renderpointsPerCol - 1) * m_renderpointsPerRow + x + 1;

		p1 = tempPoints[pi0];
		p2 = tempPoints[pi1];

		m_splinePoints.push_back(p1);

		F::FVector p5( (p1[0] + p2[0]) / 2.0 , (p1[1] + p2[1]) / 2.0 , (p1[2] + p2[2]) / 2.0);

		m_splinePoints.push_back(p5);
	}
	m_splinePoints.push_back(p2);

	m_renderpointsPerRow = m_renderpointsPerRow * 2 - 1;
	m_renderpointsPerCol = m_renderpointsPerCol * 2 - 1;
}

void Surface::drawVectors()
{
	int numQuads = (m_renderpointsPerCol - 1) * (m_renderpointsPerRow - 1);
	int i0, i1, i2;
	glBegin(GL_LINES);

	for (int i = 0 ; i < numQuads*4 ; ++i)
	{
		i0 = m_indexArray[i]*3;
		i1 = m_indexArray[i]*3+1;
		i2 = m_indexArray[i]*3+2;
		glNormal3f(m_normalArray[i0], m_normalArray[i1], m_normalArray[i2]);
		glColor3f(m_colorArray[i0], m_colorArray[i1], m_colorArray[i2]);
		glMultiTexCoord2f(GL_TEXTURE0, -1.0f, 0.0f);
		glVertex3f(m_vertexArray[i0], m_vertexArray[i1], m_vertexArray[i2]);
		glNormal3f(m_normalArray[i0], m_normalArray[i1], m_normalArray[i2]);
		glColor3f(m_colorArray[i0], m_colorArray[i1], m_colorArray[i2]);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
		glVertex3f(m_vertexArray[i0], m_vertexArray[i1], m_vertexArray[i2]);
	}

	glEnd();
}
