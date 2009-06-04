#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H
/*****************************************************************
*
* TriangleMesh.h
*
* Author:	Russ Stimpson
* Date:		2/7/03
*
*****************************************************************/

#include "Vector.h"
#include <vector>
#include "../../dataset/DatasetHelper.h"

#include "../Fantom/FIndex.h"
#include "../Fantom/FArray.h"

#define PI 3.14159

class DatasetHelper;

struct Triangle {
	unsigned int pointID[3];
};

class TriangleMesh {

	// Attributes
	private:
		DatasetHelper* m_dh;

		std::vector<Vector> vertices;
		std::vector<Vector> vertNormals;
		std::vector<Vector> vertColors;
		std::vector < std::vector<unsigned int> >vIsInTriangle;

		std::vector< Triangle > triangles;
		std::vector<Vector> triNormals;
		std::vector< int >triangleTensor;
		std::vector<wxColour>triangleColor;
		std::vector< std::vector<int> > neighbors;

		int	numVerts;
		int	numTris;

		bool openMeshError;

		// we don't delete vertices yet, so can do a cleanup only once
		bool isCleaned;
		// flag to indicate vertNormals and neighbors aren't calculated yet
		bool m_isFinished;

		wxColour defaultColor;

	// Construction
	public:
		TriangleMesh (DatasetHelper* dh);
		~TriangleMesh ();

	// Operations
	public:
		void addVert(const Vector newVert);
		void addVert(const float x, const float y, const float z);
		void addTriangle(const int vertA, const int vertB, const int vertC);
		void addTriangle(const int vertA, const int vertB, const int vertC, const int tensorIndex);


		void clearMesh();
		void finalize() { calcNeighbors(); calcVertNormals(); m_isFinished = true;};

		int getNumVertices()									{ return numVerts; };
		int getNumTriangles()									{ return numTris; };
		Vector getVertex (const int vertNum) 					{ return vertices[vertNum]; };
		Vector getVertex (const int triNum, int pos);
		Vector getNormal(const int triNum)						{ return triNormals[triNum]; };
		Vector getVertNormal(const int vertNum) 				{ return vertNormals[vertNum];};
		Vector getVertColor( const int vertNum)					{ return vertColors[vertNum]; };
		Triangle getTriangle(const int triNum)					{ return triangles[triNum];  };
		wxColour getTriangleColor(const int triNum)				{ return triangleColor[triNum]; };
		std::vector<unsigned int> getStar(const int vertNum) 	{ return vIsInTriangle[vertNum]; };
		int getTriangleTensor(const int triNum)					{ return triangleTensor[triNum]; };
		bool isFinished()										{ return m_isFinished; }

		std::vector<Vector> getVerts()                          { return vertices; };

		Vector getTriangleCenter(int triNum) ;

		void setVertex(const unsigned int vertNum, const Vector nPos)	{ vertices[vertNum] = nPos; };
		void eraseTriFromVert( const unsigned int triNum, const unsigned int vertNum);
		void setTriangle(const unsigned int triNum, const unsigned int vertA, const unsigned int vertB, const unsigned int vertC);
		void setTriangleColor(const unsigned int triNum, const float r, const float g, const float b, const float a);
		void setTriangleColor(const unsigned int triNum, const float r, const float g, const float b);
		void setTriangleAlpha(const unsigned int triNum, const float a);
		void setTriangleRed(const unsigned int triNum, const float r);
		void setTriangleGreen(const unsigned int triNum, const float g);
		void setTriangleBlue(const unsigned int triNum, const float b);

		void setVertexColor(const unsigned int vertNum, const float r, const float g, const float b);

		bool isInTriangle(const unsigned int vertNum, const unsigned int triangleNum);

		bool hasEdge(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum);
		int getThirdVert(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum);

		int getNextVertex(const unsigned int triNum, const unsigned int vertNum);

		bool getOpenMeshError() { return openMeshError; };

		void cleanUp();
		void doLoopSubD();

		void getCellVerticesIndices( const FIndex& cellId, std::vector< FIndex >& vertices ) const;
		void getPosition( FPosition& resultPos, const FIndex& pIndex ) const;
		void getEdgeNeighbor( const FIndex& cellId, int pos, std::vector< FIndex >& neigh ) const;
		void getNeighbors( const FIndex& vertId, std::vector< FIndex >& neighs ) const;
		int getNeighbor(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum);

		void reserveVerts(const int size);
		void reserveTriangles(const int size);

		void printInfo();

		void calcTriangleTensors();
		void calcNeighbors();
		void calcVertNormals();


	private:
		Vector calcTriangleNormal(const Triangle);
		Vector calcTriangleNormal(const int triNum);
		Vector calcVertNormal(const int vertNum);

		void flipNormals();


		int calcTriangleTensor(const int triNum);
		void calcNeighbor(const int triangleNum);



};
#endif
