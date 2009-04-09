// loopSubD.h: interface for the loopSubD class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOOPSUBD_H__50785B50_B91B_4AA7_9A1A_E10178EA5309__INCLUDED_)
#define AFX_LOOPSUBD_H__50785B50_B91B_4AA7_9A1A_E10178EA5309__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TriangleMesh;
#include "Vector.h"

class loopSubD  
{
public:
	
	TriangleMesh* triMesh;

	int numTriVerts;
	int numTriFaces;

	int numEdgeVerts;

	Vector vec;

	loopSubD();
	loopSubD(TriangleMesh* nTriMesh);
	virtual ~loopSubD();

	int calcEdgeVert(int triNum, unsigned int edgeV1, unsigned int edgeV2, unsigned int V3);
	Vector calcNewPosition(unsigned int vertNum);
	void insertCenterTriangle(unsigned int triNum);
	void insertCornerTriangles(unsigned int triNum);
	double getAlpha(int n);

};

#endif // !defined(AFX_LOOPSUBD_H__50785B50_B91B_4AA7_9A1A_E10178EA5309__INCLUDED_)
