#include "surface.h"
#include "math.h"
#include "../misc/Fantom/FMatrix.h"
#include "surface.h"
#include "splinePoint.h"
#include "../gui/myListCtrl.h"
#include "Anatomy.h"

#include "GL/glew.h"
#include "../misc/lic/SurfaceLIC.h"

#include <fstream>

Surface::Surface(DatasetHelper* dh) : DatasetInfo(dh)
{
    m_radius = 30.0;
	m_my = 8.0;
	m_numDeBoorRows = 12;
	m_numDeBoorCols = 12;
	m_order = 4;

	m_sampleRateT = m_sampleRateU = 0.5;

	m_type = SURFACE;
	m_threshold = 0.5;
	m_name = wxT("spline surface");

	m_numPoints = 0;
	m_alpha = 0.2f;
	m_tMesh = NULL;
	m_CutTex = 0;
	m_normalDirection = 1.0;

	subDCount = 0;
	m_positionsCalculated = false;
}

Surface::~Surface()
{
	m_dh->m_mainFrame->m_treeWidget->DeleteChildren(m_dh->m_mainFrame->m_tPointId);
	m_dh->m_surfaceLoaded = false;
	m_tMesh->clearMesh();
	delete m_tMesh;
	if (m_kdTree)
	    delete m_kdTree;
	if (m_pointArray)
	    delete m_pointArray;
	if (m_GLuint)
	    glDeleteLists(m_GLuint, 1);
	if (m_CutTex)
	    glDeleteTextures(1, &m_CutTex);
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

	deBoorPoints.reserve(numRows * numCols);

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
	int countPoints = m_dh->m_mainFrame->m_treeWidget->GetChildrenCount(m_dh->m_mainFrame->m_tPointId, true);
	if (countPoints == 0) return;
	if (m_tMesh) delete m_tMesh;
	m_tMesh = new TriangleMesh(m_dh);
	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	id = m_dh->m_mainFrame->m_treeWidget->GetFirstChild(m_dh->m_mainFrame->m_tPointId, cookie);
	givenPoints.reserve(m_dh->m_mainFrame->m_treeWidget->GetChildrenCount(m_dh->m_mainFrame->m_tPointId));
	while ( id.IsOk() )
	{
		SplinePoint *point = (SplinePoint*)(m_dh->m_mainFrame->m_treeWidget->GetItemData(id));
		std::vector< double > p(3);
		p[0] = point->getCenter().x;
		p[1] = point->getCenter().y;
		p[2] = point->getCenter().z;
		givenPoints.push_back(p);

		id = m_dh->m_mainFrame->m_treeWidget->GetNextChild(m_dh->m_mainFrame->m_tPointId, cookie);
	}

	std::vector< std::vector< double > > deBoorPoints;
	m_splinePoints.clear();

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

	m_tMesh->reserveVerts(m_splinePoints.size());
	for( std::vector< std::vector< double > >::iterator posIt = m_splinePoints.begin(); posIt != m_splinePoints.end(); posIt++)
	{
		m_tMesh->addVert((*posIt)[0], (*posIt)[1], (*posIt)[2]);
	}

	m_renderpointsPerCol = splineSurface.getNumSamplePointsU();
	m_renderpointsPerRow = splineSurface.getNumSamplePointsT();

	m_tMesh->reserveTriangles(2 * m_renderpointsPerCol * m_renderpointsPerRow);

	for(int z = 0; z < m_renderpointsPerCol - 1; z++)
	{
		for(int x = 0; x < m_renderpointsPerRow - 1; x++)
			{
				int p0 = z * m_renderpointsPerCol + x;
				int p1 = z * m_renderpointsPerCol + x + 1;
				int p2 = (z+1) * m_renderpointsPerCol + x;
				int p3 = (z+1) * m_renderpointsPerCol + x + 1;

				m_tMesh->addTriangle( p0, p1, p2 );
				m_tMesh->addTriangle( p2, p1, p3 );
			}
	}

	licCalculated = false;
//TODO
	for (int i = 0 ; i < 3 ; ++i)
		m_tMesh->doLoopSubD();
	subDCount = 3;

	if (m_dh->m_vectorsLoaded && m_useLIC)
	{
		m_tMesh->doLoopSubD();
		m_tMesh->doLoopSubD();
		subDCount = 5;
		m_dh->printDebug(_T("initiating lic"), 1);
		SurfaceLIC lic(m_dh, m_tMesh);
		lic.execute();

		licCalculated = true;
	}
	m_dh->m_surfaceIsDirty = false;

#ifndef __WXMAC__
	// FIXME MAC !!!
	if (m_GLuint)
	    glDeleteLists(m_GLuint, 1);
	m_GLuint = 0;
	m_positionsCalculated = false;

	createCutTexture();
#endif
}

void Surface::draw()
{
	if (m_dh->m_surfaceIsDirty)
	{
		execute();
	}
	if (!m_GLuint)
		generateGeometry();

	glCallList(m_GLuint);
}

void Surface::movePoints()
{
	int countPoints = m_dh->m_mainFrame->m_treeWidget->GetChildrenCount(m_dh->m_mainFrame->m_tPointId, true);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = m_dh->m_mainFrame->m_treeWidget->GetNextChild(m_dh->m_mainFrame->m_tPointId, cookie);
		SplinePoint *point = (SplinePoint*)m_dh->m_mainFrame->m_treeWidget->GetItemData(id);
		point->move(2.0 * m_threshold);
	}
	execute();
}

void Surface::createCutTexture()
{

	int xDim = m_dh->m_rows;
	int yDim = m_dh->m_frames;
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
			cutTex[x + y*xDim] = getXValue(x, y, numPoints) / (float) m_dh->m_columns;

		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &m_CutTex);
	glBindTexture(GL_TEXTURE_2D, m_CutTex);
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
	m_boxMin[0] = 0;
	m_boxMax[0] = m_dh->m_columns;
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
		return m_dh->m_columns / 2;
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

void Surface::activateLIC()
{
	m_useLIC = !m_useLIC;
	if (!m_useLIC || licCalculated) return;

	for (int i = subDCount ; i < 5 ; ++i)
		m_tMesh->doLoopSubD();
	subDCount = 5;

	SurfaceLIC lic(m_dh, m_tMesh);
	lic.execute();

	if (m_GLuint)
	    glDeleteLists(m_GLuint, 1);
	m_GLuint = 0;
	m_positionsCalculated = false;

	licCalculated = true;
}

void Surface::generateGeometry()
{
	if (m_useLIC)
	{
		generateLICGeometry();
		return;
	}

	if (m_GLuint) glDeleteLists(m_GLuint, 1);
	GLuint dl = glGenLists(1);
	glNewList (dl, GL_COMPILE);

	Triangle triangleEdges;
	Vector point;
	Vector pointNormal;

	glBegin(GL_TRIANGLES);
		for (int i = 0 ; i < m_tMesh->getNumTriangles() ; ++i)
		{
			triangleEdges = m_tMesh->getTriangle(i);
			for(int j = 0 ; j < 3 ; ++j)
			{
				pointNormal = m_tMesh->getVertNormal(triangleEdges.pointID[j]);
				glNormal3d(pointNormal.x * m_normalDirection,
						pointNormal.y  * m_normalDirection,
						pointNormal.z  * m_normalDirection);
				point = m_tMesh->getVertex(triangleEdges.pointID[j]);
				glVertex3d(point.x, point.y, point.z);
			}
		}
	glEnd();

	glEndList();
	m_GLuint = dl;
}

bool Surface::save(wxString filename ) const
{
#if 0
	m_dh->printDebug(_T("start saving vtk file"), 1);
	wxFile dataFile;
	wxFileOffset nSize = 0;

	if (dataFile.Open(filename))
	{
//		nSize = dataFile.Length();
//		if (nSize == wxInvalidOffset) return false;
	}
	else
	{
		return false;
	}

	m_dh->printDebug(_T("start writing file)"));
	dataFile.write("# vtk DataFile Version 2.0\n");
	dataFile.write("generated using FiberNavigator\n");
	dataFile.write("ASCII\n");

	dataFile.write("POINT_DATA %d float\n", m_tMesh->getNumVertices());
	for(int i=0; i< m_tMesh->getNumVertices(); ++i)
	{
		point = m_tMesh->getVertex(i);
		dataFile.write("%d %d %d\n", point.x, point.y, point.z);
	}

	dataFile.write("CELLS %d %d\n", m_tMesh->getNumTriangles(), m_tMesh->getNumTriangles()*4);
	for(int i=0; i< m_tMesh->getNumTriangles() ; ++i)
	{
		triangleEdges = m_tMesh->getTriangle(i);
		dataFile.write("3 %d %d %d\n", triangleEdges.pointID[0],
			triangleEdges.pointID[1], triangleEdges.pointID[2]);
	}
	dataFile.write("CELL_TYPES");
	for(int i=0; i< m_tMesh->getNumTriangles() ; ++i)
	{
		dataFile.write("3\n");
	}
	return true;
#else
	char* c_file;
	c_file = (char*) malloc(filename.length()+1);
	strcpy(c_file, (const char*) filename.mb_str(wxConvUTF8));

	//m_dh->printDebug(_T("start saving vtk file"), 1);
	std::ofstream dataFile(c_file);

	if (dataFile)
	{
		std::cout << "opening file" << std::endl;
//		nSize = dataFile.Length();
//		if (nSize == wxInvalidOffset) return false;
	}
	else
	{
		std::cout << "open file failed: " << filename.c_str() << std::endl;
		return false;
	}

	m_dh->printDebug(_T("start writing file)"), 1);
	dataFile << ("# vtk DataFile Version 2.0\n");
	dataFile << ("generated using FiberNavigator\n");
	dataFile << ("ASCII\n");

	Triangle triangleEdges;
	Vector point;
	dataFile << "POINT_DATA " << m_tMesh->getNumVertices() << " float\n";
	for(int i=0; i< m_tMesh->getNumVertices(); ++i)
	{
		point = m_tMesh->getVertex(i);
		dataFile << point.x << " " << point.y << " " << point.z <<"\n";
	}

	dataFile << "CELLS " << m_tMesh->getNumTriangles() << " " << m_tMesh->getNumTriangles()*4;
	for(int i=0; i< m_tMesh->getNumTriangles() ; ++i)
	{
		triangleEdges = m_tMesh->getTriangle(i);
		dataFile << "3 " << triangleEdges.pointID[0] << " " <<
			triangleEdges.pointID[1] << " " << triangleEdges.pointID[2] << "\n";
	}
	dataFile << "CELL_TYPES\n";
	for(int i=0; i< m_tMesh->getNumTriangles() ; ++i)
	{
		dataFile << "3\n";
	}
	std::cout << " saving  done" << std::endl;
	return true;

#endif
}

void Surface::generateLICGeometry()
{
	if (m_GLuint) glDeleteLists(m_GLuint, 1);
	GLuint dl = glGenLists(1);
	glNewList (dl, GL_COMPILE);

	Triangle triangleEdges;
	Vector point;
	Vector pointNormal;
	wxColour color;

	glBegin(GL_TRIANGLES);
		for (int i = 0 ; i < m_tMesh->getNumTriangles() ; ++i)
		{
			triangleEdges = m_tMesh->getTriangle(i);
			color = m_tMesh->getTriangleColor(i);
			glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
			for(int j = 0 ; j < 3 ; ++j)
			{
				pointNormal = m_tMesh->getVertNormal(triangleEdges.pointID[j]);
				glNormal3d(pointNormal.x * m_normalDirection,
						pointNormal.y  * m_normalDirection,
						pointNormal.z  * m_normalDirection);
				point = m_tMesh->getVertex(triangleEdges.pointID[j]);
				glVertex3d(point.x, point.y, point.z);
			}
		}
	glEnd();

	glEndList();
	m_GLuint = dl;
}

void Surface::flipNormals()
{
	m_normalDirection = m_dh->m_normalDirection;
	if (m_GLuint) glDeleteLists(m_GLuint, 1);
	m_GLuint = 0;
	m_positionsCalculated = false;
}

std::vector<Vector> Surface::getSurfaceVoxelPositions()
{
    if (!m_positionsCalculated)
    {
        Vector v(0, 0, 0);
        size_t nSize = m_dh->m_columns * m_dh->m_rows * m_dh->m_frames;
        std::vector<Vector> accu(nSize, v);
        std::vector<int> hits(nSize, 0);
        std::vector<Vector> vertices = m_tMesh->getVerts();
        m_svPositions.clear();

        for (size_t i = 0; i < vertices.size(); ++i)
        {
            v = vertices[i];
            int index = (int) v.x + (int) v.y * m_dh->m_columns + (int) v.z * m_dh->m_columns * m_dh->m_rows;
            if ( !(index < 0 || index > m_dh->m_columns * m_dh->m_rows * m_dh->m_frames || v.x <  0 ||
                    v.x > m_dh->m_columns || v.y < 0 || v.z > m_dh->m_rows || v.z < 0 || v.z > m_dh->m_frames) )
            {
                accu[index].x += v.x;
                accu[index].y += v.y;
                accu[index].z += v.z;
                hits[index] += 1;
            }
        }

        int pointsInVoxels = 0;
        int voxelsHit = 0;
        for (size_t i = 0; i < nSize; ++i)
        {
            if (hits[i] > 0)
            {
                ++voxelsHit;
                pointsInVoxels += hits[i];
            }
        }
        pointsInVoxels /= voxelsHit;
        int threshold = pointsInVoxels / 2;

        for (size_t i = 0; i < nSize; ++i)
        {
            if (hits[i] > threshold)
            {
                accu[i].x /= hits[i];
                accu[i].y /= hits[i];
                accu[i].z /= hits[i];
                if ((int) accu[i].x)
                {
                    Vector v(accu[i].x, accu[i].y, accu[i].z);
                    m_svPositions.push_back(v);
                }
            }
        }
        m_positionsCalculated = true;
    }
    return m_svPositions;
}

void Surface::smooth()
{
    if (m_GLuint)
        glDeleteLists(m_GLuint, 1);
    m_GLuint = 0;
    m_positionsCalculated = false;
    m_tMesh->doLoopSubD();
    ++subDCount;
    licCalculated = false;
}

