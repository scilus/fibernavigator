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

	m_sampleRateNormal = 0.2;
	m_sampleRateDragging = 0.5;

	m_sampleRateT = m_sampleRateU = m_sampleRateNormal;

	m_show = true;
	m_showFS = true;
	m_useTex = true;

	m_type = Surface_;
	m_hasTreeId = false;
	m_threshold = 0.5;
	m_name = wxT("spline surface");
	m_dh->surface_loaded = true;
	m_numPoints = 0;
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
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = m_dh->mainFrame->m_treeWidget->GetNextChild(m_dh->mainFrame->m_tPointId, cookie);
		SplinePoint *point = (SplinePoint*)((MyTreeItemData*)m_dh->mainFrame->m_treeWidget->GetItemData(id))->getData();

		std::vector< double > p;
		p.push_back(point->getCenter().s.X);
		p.push_back(point->getCenter().s.Y);
		p.push_back(point->getCenter().s.Z);
		givenPoints.push_back(p);
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

		FVector result = transMatrix * dmy;
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

		FVector result = transMatrix * dmy;
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

	int renderpointsPerCol = splineSurface.getNumSamplePointsU();
	int renderpointsPerRow = splineSurface.getNumSamplePointsT();
	m_numPoints = splineSurface.getNumSamplePointsU()*splineSurface.getNumSamplePointsT();

	int pi0, pi1, pi2;
	m_normals.clear();
	std::vector< FVector > triangleNormals;
	std::vector< std::vector<int> >triangleRef(m_numPoints, std::vector<int>(0,0));
	std::vector< double > p;

	for(int z = 0; z < renderpointsPerCol - 1; z++)
	{
		for(int x = 0; x < renderpointsPerRow - 1; x++)
		{
			pi0 = z * renderpointsPerCol + x;
			pi1 = z * renderpointsPerCol + x + 1;
			pi2 = (z+1) * renderpointsPerCol + x;
			m_vertices.push_back(pi0);
			m_vertices.push_back(pi1);
			m_vertices.push_back(pi2);

			p = m_splinePoints[pi0];
			FVector p1(p[0], p[1], p[2]);
			triangleRef[pi0].push_back(triangleNormals.size());
			p = m_splinePoints[pi1];
			FVector p2(p[0], p[1], p[2]);
			triangleRef[pi1].push_back(triangleNormals.size());
			p = m_splinePoints[pi2];
			FVector p3(p[0], p[1], p[2]);
			triangleRef[pi2].push_back(triangleNormals.size());
			FVector n1 = getNormalForTriangle(&p1, &p2, &p3);
			triangleNormals.push_back(n1);


			pi0 = (z+1) * renderpointsPerCol + x;
			pi1 = z * renderpointsPerCol + x + 1;
			pi2 = (z+1) * renderpointsPerCol + x + 1;
			m_vertices.push_back(pi0);
			m_vertices.push_back(pi1);
			m_vertices.push_back(pi2);

			p = m_splinePoints[pi0];
			FVector p4(p[0], p[1], p[2]);
			triangleRef[pi0].push_back(triangleNormals.size());
			p = m_splinePoints[pi1];
			FVector p5(p[0], p[1], p[2]);
			triangleRef[pi1].push_back(triangleNormals.size());
			p = m_splinePoints[pi2];
			FVector p6(p[0], p[1], p[2]);
			triangleRef[pi2].push_back(triangleNormals.size());
			FVector n2 = getNormalForTriangle(&p4, &p5, &p6);
			triangleNormals.push_back(n2);

		}
	}

	for (int i = 0 ; i < m_numPoints ; ++i )
	{
		FVector tmp(0.0,0.0,0.0);
		for ( unsigned int j = 0 ; j < triangleRef[i].size() ; ++j)
		{
			 tmp += triangleNormals[triangleRef[i][j]];
		}

		FVector n( -tmp[0] / triangleRef[i].size(), -tmp[1] / triangleRef[i].size(), -tmp[2] / triangleRef[i].size());
		m_normals.push_back(n);
	}

	createCutTexture(splineSurface.getNumSamplePointsU(), splineSurface.getNumSamplePointsT());

	m_dh->surface_isDirty = false;
}

FVector Surface::getNormalForTriangle(const FVector* p1, const FVector* p2, const FVector* p3)
{
	FVector a = *p2 - *p1;
	FVector b = *p3 - *p1;
	FVector n = a.crossProduct(b);
	return n.normalize();
}

void Surface::draw()
{
	if (!m_dh->m_isrDragging && m_sampleRateT == m_sampleRateDragging)
	{
		m_sampleRateT = m_sampleRateU = m_sampleRateNormal;
		execute();
	}

	if (m_dh->surface_isDirty)
	{
		if (m_dh->m_isrDragging) m_sampleRateT = m_sampleRateU = m_sampleRateDragging;
		execute();
	}

	if (m_dh->scene->getPointMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (m_dh->vectors_loaded && m_dh->use_lic)
	{
		float* vectorField = 0;
		Anatomy* anatomy;
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
		glBegin (GL_TRIANGLES);
		for (unsigned int i = 0 ; i < m_vertices.size() ; ++i)
		{
			std::vector< double > p = m_splinePoints[m_vertices[i]];
			FVector n = m_normals[m_vertices[i]];
			glNormal3f(n[0], n[1], n[2]);
			glVertex3f(p[0], p[1], p[2]);

			int x = wxMax(0, wxMin((int)p[0] + m_dh->xOff, m_dh->columns));
			int y = wxMax(0, wxMin((int)p[1] + m_dh->yOff, m_dh->rows));
			int z = wxMax(0, wxMin((int)p[2] + m_dh->zOff, m_dh->frames));

			int index = (x + y * m_dh->columns + z * m_dh->columns * m_dh->rows)*3;
			glColor3f(vectorField[index], vectorField[index+1], vectorField[index+2]);
		}
		glEnd();

	}

	else
	{
		glBegin (GL_TRIANGLES);
		for (unsigned int i = 0 ; i < m_vertices.size() ; ++i)
		{
			std::vector< double > p = m_splinePoints[m_vertices[i]];
			FVector n = m_normals[m_vertices[i]];
			glNormal3f(n[0], n[1], n[2]);
			glVertex3f(p[0], p[1], p[2]);
		}
		glEnd();
	}

}

void Surface::movePoints()
{
	int countPoints = m_dh->mainFrame->m_treeWidget->GetChildrenCount(m_dh->mainFrame->m_tPointId, true);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = m_dh->mainFrame->m_treeWidget->GetNextChild(m_dh->mainFrame->m_tPointId, cookie);
		SplinePoint *point = (SplinePoint*)((MyTreeItemData*)m_dh->mainFrame->m_treeWidget->GetItemData(id))->getData();
		point->move(2.0 * m_threshold);
	}
	execute();
}

void Surface::createCutTexture(int xPoints, int yPoints)
{
	int xDim = m_dh->rows;
	int yDim = m_dh->frames;
	int numPoints = xPoints*yPoints;

	m_pointArray= new float[numPoints*3];
	for (int i = 0 ; i < numPoints ; ++i )
	{
		FVector p = m_splinePoints[i];
		m_pointArray[3*i] = p[0];
		m_pointArray[3*i+1] = p[1];
		m_pointArray[3*i+2] = p[2];
	}
	m_kdTree = new KdTree(xPoints*yPoints, m_pointArray);

	float* cutTex;
	cutTex = new float[xDim*yDim];
	int xOff = m_dh->rows/2;
	int yOff = m_dh->frames/2;

	for ( int x = 0 ; x < xDim ; ++x )
	{
		for ( int y = 0 ; y < yDim ; ++y)
		{
			//cutTex[y + (xDim-1-x)*yDim] =
			cutTex[x + y*xDim] =
				(getXValue(x - xOff, y - yOff, numPoints)  + (m_dh->columns/2.0)) / (float)m_dh->columns;

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

}

float Surface::getXValue(int y, int z, int numPoints)
{
	m_boxMin = new float[3];
	m_boxMax = new float[3];
	m_boxMin[0] = - m_dh->columns / 2;
	m_boxMax[0] = m_dh->columns / 2;;
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
