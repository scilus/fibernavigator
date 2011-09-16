// loopSubD.cpp: implementation of the loopSubD class.
//
//////////////////////////////////////////////////////////////////////

#include "LoopSubD.h"
#include "TriangleMesh.h"

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

    triMesh->reserveVerts(numTriVerts*4);
    triMesh->reserveTriangles(numTriFaces*4);

    //wxDateTime dt = wxDateTime::Now();
    //printf("[%02d:%02d:%02d] ", dt.GetHour(), dt.GetMinute(), dt.GetSecond());

    Vector* newVertexPositions = new Vector[numTriVerts];
    //printf("start loop subdivision on %d vertices , %d triangles\n", numTriVerts, numTriFaces);
    for(int i=0; i<numTriVerts; i++){
        newVertexPositions[i] = calcNewPosition(i);
    }

    //printf("loop subdivision pass 2\n");
    for(int i=0; i<numTriFaces; i++){
        insertCenterTriangle(i);
    }

    //printf("loop subdivision pass 3\n");
    for(int i=0; i<numTriFaces; i++){
        insertCornerTriangles(i);
    }

    //printf("loop subdivision pass 4\n");
    for(int i=0; i<numTriVerts; i++){
        triMesh->setVertex(i, newVertexPositions[i]);
    }

    delete[] newVertexPositions;

    //wxDateTime dt1 = wxDateTime::Now();
    //printf("[%02d:%02d:%02d] ", dt1.GetHour(), dt1.GetMinute(), dt1.GetSecond());
    //printf("loop subdivision done, mesh now has %d vertices , %d triangles\n",triMesh->getNumVertices(),triMesh->getNumTriangles());
    //triMesh->printInfo();

}

Vector loopSubD::calcNewPosition(unsigned int vertNum)
{
    std::vector<unsigned int> starP = triMesh->getStar(vertNum);
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

void loopSubD::insertCenterTriangle(unsigned int triNum){

    Triangle intP = triMesh->getTriangle(triNum);
    int edgeVerts[3];

    for(int i=0; i<3; i++){
        edgeVerts[i] = calcEdgeVert(triNum, intP.pointID[i], intP.pointID[(i + 1) % 3], intP.pointID[(i + 2) % 3]);
    }
    triMesh->addTriangle(edgeVerts[0], edgeVerts[1], edgeVerts[2], triMesh->getTriangleTensor(triNum));
}

void loopSubD::insertCornerTriangles(unsigned int triNum){

    // comment:        center are twisted from the orignal vertices.
    // original:    0, 1, 2
    // center:        a, b, c
    // reAsgnOrig:    0, a, c
    // addTris:        1, b, a
    // addTris:        2, c, b
    //
    Triangle originalTri = triMesh->getTriangle(triNum);
    Triangle centerTri   = triMesh->getTriangle(triNum + numTriFaces);

    triMesh->addTriangle(originalTri.pointID[1], centerTri.pointID[1], centerTri.pointID[0], triMesh->getTriangleTensor(triNum));
    triMesh->addTriangle(originalTri.pointID[2], centerTri.pointID[2], centerTri.pointID[1], triMesh->getTriangleTensor(triNum));
    triMesh->setTriangle(triNum, originalTri.pointID[0], centerTri.pointID[0], centerTri.pointID[2]);
}

int loopSubD::calcEdgeVert(int triNum, unsigned int edgeV1, unsigned int edgeV2, unsigned int V3){

    int vertNum = -1;

    int neighborVert = -1;
    int neighborFaceNum = -1;
    Vector edgeVert;

    neighborFaceNum = triMesh->getNeighbor(edgeV1, edgeV2, triNum);

    if(neighborFaceNum == triNum)
    {
        Vector edgeVert = (triMesh->getVertex(edgeV1) + triMesh->getVertex(edgeV2))/2.0;
        vertNum = triMesh->getNumVertices();
        triMesh->addVert(edgeVert);
    }

    else if(neighborFaceNum > triNum)
    {
        neighborVert = triMesh->getThirdVert(edgeV1, edgeV2, neighborFaceNum);

        Vector edgePart = triMesh->getVertex(edgeV1) + triMesh->getVertex(edgeV2);
        Vector neighborPart = triMesh->getVertex(neighborVert) + triMesh->getVertex(V3);

        edgeVert = ((edgePart * (3.0/8.0)) + (neighborPart * (1.0/8.0)));
        vertNum = triMesh->getNumVertices();
        triMesh->addVert(edgeVert);
    }
    else
    {
        Triangle neighborCenterP = triMesh->getTriangle(neighborFaceNum + numTriFaces);
        Triangle neighborP = triMesh->getTriangle(neighborFaceNum);

        if(neighborP.pointID[0] == edgeV2){
            vertNum = neighborCenterP.pointID[0];
        } else if(neighborP.pointID[1] == edgeV2){
            vertNum = neighborCenterP.pointID[1];
        } else {
            vertNum = neighborCenterP.pointID[2];
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
