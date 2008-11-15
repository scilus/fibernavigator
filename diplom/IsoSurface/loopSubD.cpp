// loopSubD.cpp: implementation of the loopSubD class.
//
//////////////////////////////////////////////////////////////////////

#include "loopSubD.h"
#include "triangleMesh.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

loopSubD::loopSubD()
{

}

loopSubD::~loopSubD()
{

}

loopSubD::loopSubD(TriangleMesh* nTriMesh){

	triMesh = nTriMesh;
	Vector newEdge;

	numTriVerts = triMesh->getNumVertices();
	numTriFaces = triMesh->getNumTriangles();

	printf("%d vertices , %d triangles\n", triMesh->getNumVertices(),triMesh->getNumTriangles());
	wxDateTime dt = wxDateTime::Now();
	printf("[%02d:%02d:%02d] ", dt.GetHour(), dt.GetMinute(), dt.GetSecond());

	Vector* newVertexPositions = new Vector[numTriVerts];
	printf("loop subdivision pass 1\n");
	for(int i=0; i<numTriVerts; i++){
		newVertexPositions[i] = calcNewPosition(i);
	}

	printf("loop subdivision pass 2\n");
	for(int i=0; i<numTriFaces; i++){
		insertCenterTriangle(i);
	}

	printf("loop subdivision pass 3\n");
	for(int i=0; i<numTriFaces; i++){
		insertCornerTriangles(i);

	}

	printf("loop subdivision pass 4\n");
	for(int i=0; i<numTriVerts; i++){
		triMesh->setVertex(i, newVertexPositions[i]);
	}

	delete[] newVertexPositions;

	printf("loop subdivision pass 5\n");
	triMesh->calcNeighbors();
	wxDateTime dt1 = wxDateTime::Now();
	printf("[%02d:%02d:%02d] ", dt1.GetHour(), dt1.GetMinute(), dt1.GetSecond());
	printf("loop subdivision done\n");
	printf("%d vertices , %d triangles\n", triMesh->getNumVertices(),triMesh->getNumTriangles());

}

Vector loopSubD::calcNewPosition(int vertNum)
{
	std::vector<int> starP = triMesh->getStar(vertNum);
	int starSize = starP.size();


	Vector oldPos = triMesh->getVertex(vertNum);
	double alpha = getAlpha(starSize);
	oldPos.scaleBy(1.0 - ((double) starSize * alpha));

	Vector newPos;
	int edgeV = 0;
	for(int i=0; i<starSize; i++){
		edgeV = triMesh->getNextVertex(starP[i], vertNum);
		newPos.translateBy(triMesh->getVertex(edgeV));
	}
	newPos.scaleBy(alpha);

	return oldPos + newPos;
}

void loopSubD::insertCenterTriangle(int triNum){

	Vector intP = triMesh->getTriangle(triNum);
	int edgeVerts[3];

	for(int i=0; i<3; i++){
		edgeVerts[i] = calcEdgeVert(triNum, intP[i], intP[(i + 1) % 3], intP[(i + 2) % 3]);
	}
	triMesh->addTriangle(edgeVerts[0], edgeVerts[1], edgeVerts[2]);
}

void loopSubD::insertCornerTriangles(int triNum){

	// comment:		center are twisted from the orignal vertices.
	// original:	0, 1, 2
	// center:		a, b, c
	// reAsgnOrig:	0, a, c
	// addTris:		1, b, a
	// addTris:		2, c, b
	//
	Vector originalTri = triMesh->getTriangle(triNum);
	Vector centerTri   = triMesh->getTriangle(triNum + numTriFaces);

	triMesh->addTriangle(originalTri[1], centerTri[1], centerTri[0]);
	triMesh->addTriangle(originalTri[2], centerTri[2], centerTri[1]);
	triMesh->setTriangle(triNum, originalTri[0], centerTri[0], centerTri[2]);
}

int loopSubD::calcEdgeVert(int triNum, int edgeV1, int edgeV2, int V3){

	int vertNum = -1;

	int neighborVert = -1;
	int neighborFaceNum = -1;
	Vector edgeVert;

	neighborFaceNum = triMesh->getNeighbor(edgeV1, edgeV2, triNum);

	if(neighborFaceNum > triNum){
		neighborVert = triMesh->getThirdVert(edgeV1, edgeV2, neighborFaceNum);

		Vector edgePart = triMesh->getVertex(edgeV1) + triMesh->getVertex(edgeV2);
		Vector neighborPart = triMesh->getVertex(neighborVert) + triMesh->getVertex(V3);

		edgeVert = ((edgePart * (3.0/8.0)) + (neighborPart * (1.0/8.0)));
		vertNum = triMesh->getNumVertices();
		triMesh->addVert(edgeVert);
	} else {
		Vector neighborCenterP = triMesh->getTriangle(neighborFaceNum + numTriFaces);
		Vector neighborP = triMesh->getTriangle(neighborFaceNum);

		if(neighborP[0] == edgeV2){
			vertNum = neighborCenterP[0];
		} else if(neighborP[1] == edgeV2){
			vertNum = neighborCenterP[1];
		} else {
			vertNum = neighborCenterP[2];
		}
	}

	return vertNum;
}


double loopSubD::getAlpha(int n){
	double answer;
	if(n>3){
		double center = (0.375 + (0.25 * cos((2.0 * 3.14159265358979) / (double) n)));
		answer = (0.625 - (center * center)) / (double) n;
	} else {
		answer = 3.0 / 16.0;
	}
	return answer;
}
