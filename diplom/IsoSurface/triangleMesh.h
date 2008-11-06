
/*****************************************************************
* 
* TriangleMesh.h
*
* Author:	Russ Stimpson
* Date:		2/7/03
*
*****************************************************************/

#include "Vector.h"
	
#define PI 3.14159

class TriangleMesh {

	// Attributes
	private:
		Vector* vertices;
		Vector* vertNormals;
		Vector* triNormals;
		Vector	centroid;
		/* triangles[triangleNumber][vertNumber] */
		int**	triangles;
		int**	neighbors;
		int*	starSizes;
		int*	starStarts;

		int	numVerts;
		int	numTris;
		int	numNeighbors;

		int	curVertSize;
		int	curTriSize;
		int	curNeighborSize;

		bool openMeshError;

	// Construction
	public:
		TriangleMesh ();
		~TriangleMesh ();

	// Operations
	public:
		void addVert(Vector newVert);
		void addTriangle(int vertA, int vertB, int vertC);
		void calcTriangleNormal(int triNum);
		void calcCentroid();
		void TransToCentroid();
		void clearMesh();

		Vector getVertices(int vertNum){ return vertices[vertNum]; };
		void setVertex(int vertNum, Vector nPos) { vertices[vertNum] = nPos; };
		Vector getNormal(int triNum){ return triNormals[triNum]; };
		int* getTriangles(int triNum){ return triangles[triNum]; };
		void setTriangle(int triNum, int vertA, int vertB, int vertC);
		int getNumVertices(){ return numVerts; };
		int getNumTriangles(){ return numTris; };
		/* returns true if the mesh is closed; false if the mesh is open */
		bool calcNeighbor(int triangleNum);
		int* getStar(int vertNum);
		/* */
		int getStarN(int vertNum){ return starSizes[vertNum]; };
		Vector getVertNormal(int vertNum);
		void calcStarSizes();
		bool isInTriangle(int vertNum, int triangleNum);
		int getNeighbor(int coVert1, int coVert2, int triangleNum);
		bool hasEdge(int coVert1, int coVert2, int triangleNum);
		int getThirdVert(int coVert1, int coVert2, int triangleNum);
		void clearTriangles();
		int** removeTriangles();
		/* returns true if the mesh is closed; false if the mesh is open */
		bool calcNeighbors();
		int getNextVertex(int triNum, int vertNum);
		void calcTriangleNormals();

		bool getOpenMeshError(){ return openMeshError; };

	private: 
		void resizeVerts();
		void resizeTriangles();
		void resizeNormals();
		void resizeNeighbors();

	// Overrides

	// Implementation
};
