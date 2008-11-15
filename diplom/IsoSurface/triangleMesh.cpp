// TriangleMesh.cpp
#include "triangleMesh.h"
#include <string.h>

// Construction
TriangleMesh::TriangleMesh ()
{
	numVerts	 = 0;
	numTris		 = 0;
}

// Destructor
TriangleMesh::~TriangleMesh ()
{

}

// Operations

void TriangleMesh::addVert(Vector newVert)
{
	vertices.push_back( newVert );
	std::vector<int> v;
	vIsInTriangle.push_back( v );
	numVerts = vertices.size();
}

void TriangleMesh::addTriangle(int vertA, int vertB, int vertC)
{
	Vector t(vertA, vertB, vertC);
	triangles.push_back(t);
	triNormals.push_back(calcTriangleNormal(t));
	vIsInTriangle[vertA].push_back(numTris);
	vIsInTriangle[vertB].push_back(numTris);
	vIsInTriangle[vertC].push_back(numTris);
	std::vector<int> v(3,-1);
	neighbors.push_back( v );

	numTris = triangles.size();
}

Vector TriangleMesh::calcTriangleNormal(Vector t){

	Vector v1 = vertices[t[1]] - vertices[t[0]];
	Vector v2 = vertices[t[2]] - vertices[t[0]];

	Vector tempNormal = v1.Cross(v2);
	tempNormal.normalize();
	return tempNormal;
}

Vector TriangleMesh::calcTriangleNormal(int triNum){

	Vector v1 = vertices[triangles[triNum][1]] - vertices[triangles[triNum][0]];
	Vector v2 = vertices[triangles[triNum][2]] - vertices[triangles[triNum][0]];

	Vector tempNormal = v1.Cross(v2);
	tempNormal.normalize();
	return tempNormal;
}

Vector TriangleMesh::getVertNormal(int vertNum)
{
	Vector sum(0,0,0);

	for(size_t i = 0 ; i < vIsInTriangle[vertNum].size() ; ++i)
	{
		sum = sum + triNormals[vIsInTriangle[vertNum][i]];
	}
	sum.normalize();
	return sum;
}

void TriangleMesh::calcVertNormals()
{
	vertNormals.clear();
	for ( int i = 0 ; i <numVerts ; ++i)
		vertNormals.push_back(getVertNormal(i));
}

int TriangleMesh::getNeighbor(int coVert1, int coVert2, int triangleNum)
{
	std::vector<int>candidates = vIsInTriangle[coVert1];
	std::vector<int>compares   = vIsInTriangle[coVert2];

	for (size_t i = 0 ; i < candidates.size() ; ++i)
		for (size_t k = 0 ; k < compares.size() ; ++k)
		{
			if ( (candidates[i] != triangleNum) && (candidates[i] == compares[k]))
				return candidates[i];
		}
	return -1;
}

void TriangleMesh::calcNeighbors()
{
	for( int i = 0 ; i < numTris ; ++i)
	{
		int coVert0 = triangles[i][0];
		int coVert1 = triangles[i][1];
		int coVert2 = triangles[i][2];

		neighbors[i][0] = getNeighbor(coVert0, coVert1, i);
		neighbors[i][1] = getNeighbor(coVert1, coVert2, i);
		neighbors[i][2] = getNeighbor(coVert2, coVert0, i);
	}
}

void TriangleMesh::calcNeighbor(int triangleNum)
{
	int coVert0 = triangles[triangleNum][0];
	int coVert1 = triangles[triangleNum][1];
	int coVert2 = triangles[triangleNum][2];

	neighbors[triangleNum][0] = getNeighbor(coVert0, coVert1, triangleNum);
	neighbors[triangleNum][1] = getNeighbor(coVert1, coVert2, triangleNum);
	neighbors[triangleNum][2] = getNeighbor(coVert2, coVert0, triangleNum);
}





bool TriangleMesh::hasEdge(int coVert1, int coVert2, int triangleNum){

	return(isInTriangle(coVert1,triangleNum) && isInTriangle(coVert2,triangleNum));

}

bool TriangleMesh::isInTriangle(int vertNum, int triangleNum){

	bool found = false;
	int index = 0;

	while(!found && (index < 3)){
		if(vertNum == triangles[triangleNum][index]){
			found = true;
		} else {
			index++;
		}
	}

	return found;
}

void TriangleMesh::clearMesh()
{
	vertices.clear();
	vertNormals.clear();
	triangles.clear();
	neighbors.clear();
	triNormals.clear();
	vIsInTriangle.clear();

	numVerts	 = 0;
	numTris		 = 0;

}





int TriangleMesh::getThirdVert(int coVert1, int coVert2, int triangleNum)
{
	int index = 0;
	bool found = false;

	while((index < 2) && !found){
		if((triangles[triangleNum][index] == coVert1) || (triangles[triangleNum][index] == coVert2)){
			index++;
		} else {
			found = true;
		}
	}

	return triangles[triangleNum][index];
}


void TriangleMesh::setTriangle(int triNum, int vertA, int vertB, int vertC)
{
	// TODO
	eraseTriFromVert(triNum, triangles[triNum][1]);
	eraseTriFromVert(triNum, triangles[triNum][2]);

	triangles[triNum][0] = vertA;
	triangles[triNum][1] = vertB;
	triangles[triNum][2] = vertC;

	vIsInTriangle[vertB].push_back(triNum);
	vIsInTriangle[vertC].push_back(triNum);

	triNormals[triNum] = calcTriangleNormal(triNum);
}

void TriangleMesh::eraseTriFromVert(int triNum, int vertNum)
{
	std::vector<int>temp;
	for ( size_t i = 0 ; i < vIsInTriangle[vertNum].size() ; ++i)
	{
		if ( triNum != vIsInTriangle[vertNum][i])
			temp.push_back(vIsInTriangle[vertNum][i]);
	}
	vIsInTriangle[vertNum] = temp;
}

int TriangleMesh::getNextVertex(int triNum, int vertNum)
{
	int answer = -1;

	if(triangles[triNum][0] == vertNum){
		answer = triangles[triNum][1];
	} else if(triangles[triNum][1] == vertNum){
		answer = triangles[triNum][2];
	} else {
		answer = triangles[triNum][0];
	}

	return answer;
}

void TriangleMesh::cleanUp()
{
	std::vector<int> queue;
	std::vector<bool> visited(numTris, false);
	std::vector< std::vector<int> >objects;
	queue.push_back(0);
	std::vector<int>n;

	while (!queue.empty())
	{
		std::vector<int>newObject;
		newObject.clear();
		while (!queue.empty())
		{
			int index = queue.back();
			visited[index] = true;
			queue.pop_back();
			newObject.push_back(index);
			n = neighbors[index];
			for ( int i = 0 ; i < 3 ; ++i)
			{
				if ( (n[i] != -1) && !visited[n[i]])
					queue.push_back(n[i]);
			}
		}
		int counter = 0;
		for (int i = 0 ; i < numTris ; ++i)
			if (!visited[i]) ++counter;
		//printf("%d triangles not visited\n", counter);
		for (int i = 0 ; i < numTris ; ++i)
		{
			if (!visited[i])
			{
				queue.push_back(i);
				break;
			}
		}
		objects.push_back(newObject);
	}
	size_t biggest = 0;
	size_t sizeBiggest = objects[0].size();
	for ( size_t i = 0 ; i < objects.size() ; ++i)
	{
		if (sizeBiggest < objects[i].size() )
		{
			biggest = i;
			sizeBiggest = objects[i].size();
		}
	}

	std::vector<int>obj = objects[biggest];
	std::vector<Vector>tempTriangles;
	for (size_t i = 0 ; i < obj.size() ; ++i)
	{
		tempTriangles.push_back(triangles[obj[i]]);
	}

	vertNormals.clear();
	triangles.clear();
	triNormals.clear();
	vIsInTriangle.clear();
	neighbors.clear();
	numTris = 0;

	std::vector<int> v;
	for (int i = 0 ; i < numVerts ; ++i)
	{
		vIsInTriangle.push_back( v );
	}

	for (size_t i = 0 ; i < tempTriangles.size() ; ++i)
	{
		Vector t = tempTriangles[i];
		addTriangle((int)t[0], (int)t[1], (int)t[2]);
	}
	calcNeighbors();
	calcVertNormals();
}
