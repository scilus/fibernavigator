#include "surface.h"
#include "math.h"
#include "Fantom/FMatrix.hh"
#include "Fantom/FBSplineSurface.hh"
#include "surface.h"
#include "point.h"
#include "myListCtrl.h"
#include "theDataset.h"

#include "GL/glew.h"

Surface::Surface()
{
	m_radius = 30.0;
	m_my = 8.0;
	m_numDeBoorRows = 12;
	m_numDeBoorCols = 12;
	m_order = 4;
	m_sampleRateT = m_sampleRateU = 0.2;

	m_show = true;
	m_showFS = true;
	m_useTex = true;

	m_type = Surface_;
	m_hasTreeId = false;
	m_threshold = 0.5;
	m_name = wxT("spline surface");
	TheDataset::surface_loaded = true;
	execute();
}

Surface::~Surface()
{
	TheDataset::surface_loaded = false;
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
	int countPoints = TheDataset::mainFrame->m_treeWidget->GetChildrenCount(TheDataset::mainFrame->m_tPointId, true);
	if (countPoints == 0) return;

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = TheDataset::mainFrame->m_treeWidget->GetNextChild(TheDataset::mainFrame->m_tPointId, cookie);
		Point *point = (Point*)((MyTreeItemData*)TheDataset::mainFrame->m_treeWidget->GetItemData(id))->getData();

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

	for(int z = 0; z < renderpointsPerCol - 1; z++)
	{
		for(int x = 0; x < renderpointsPerRow - 1; x++)
		{
			m_vertices.push_back(z * renderpointsPerCol + x);
			m_vertices.push_back(z * renderpointsPerCol + x + 1);
			m_vertices.push_back((z+1) * renderpointsPerCol + x);

			m_vertices.push_back((z+1) * renderpointsPerCol + x);
			m_vertices.push_back(z * renderpointsPerCol + x + 1);
			m_vertices.push_back((z+1) * renderpointsPerCol + x + 1);
		}
	}

	getNormalsForVertices();

	TheDataset::surface_isDirty = false;
}

FVector Surface::getNormalForTriangle(const FVector* p1, const FVector* p2, const FVector* p3)
{
	FVector a = *p2 - *p1;
	FVector b = *p3 - *p1;
	FVector n = a.crossProduct(b);
	return n.normalize();
}

void Surface::getNormalsForVertices()
{
	m_normals.clear();
	int numPoints = 46*46;
	std::vector< FVector > triangleNormals;
	std::vector< std::vector<int> >triangleRef(numPoints, std::vector<int>(0,0));

	for (unsigned int i = 0 ; i < m_vertices.size() ; ++i)
	{
		std::vector< double > p = m_splinePoints[m_vertices[i]];
		FVector p1(p[0], p[1], p[2]);
		triangleRef[m_vertices[i]].push_back(triangleNormals.size());
		++i;
		p = m_splinePoints[m_vertices[i]];
		FVector p2(p[0], p[1], p[2]);
		triangleRef[m_vertices[i]].push_back(triangleNormals.size());
		++i;
		p = m_splinePoints[m_vertices[i]];
		FVector p3(p[0], p[1], p[2]);
		triangleRef[m_vertices[i]].push_back(triangleNormals.size());
		FVector n = getNormalForTriangle(&p1, &p2, &p3);
		triangleNormals.push_back(n);
	}

	for (int i = 0 ; i < numPoints ; ++i )
	{
		FVector tmp(0.0,0.0,0.0);
		for ( unsigned int j = 0 ; j < triangleRef[i].size() ; ++j)
		{
			 tmp += triangleNormals[triangleRef[i][j]];
		}

		FVector n( tmp[0] / triangleRef[i].size(), tmp[1] / triangleRef[i].size(), tmp[2] / triangleRef[i].size());
		m_normals.push_back(n);
	}
}

void Surface::draw()
{
	if (TheDataset::surface_isDirty)
	{
		execute();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin (GL_TRIANGLES);
	for (uint i = 0 ; i < m_vertices.size() ; ++i)
	{
		std::vector< double > p = m_splinePoints[m_vertices[i]];
		FVector n = m_normals[m_vertices[i]];
		glNormal3f(n[0], n[1], n[2]);
		glVertex3f(p[0], p[1], p[2]);
	}
	glEnd();
}
