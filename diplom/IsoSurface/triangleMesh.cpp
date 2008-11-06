// TriangleMesh.cpp
#include "triangleMesh.h"
#include <string.h>

// Construction
TriangleMesh::TriangleMesh () {

	int defaultSize = 1000;

	numVerts	 = 0;
	numTris		 = 0;
	numNeighbors = 0;

	curVertSize		= defaultSize;
	curTriSize		= defaultSize;
	curNeighborSize = defaultSize;

	vertices   = new Vector[defaultSize];
	vertNormals = new Vector[defaultSize];
	triNormals = new Vector[defaultSize];
	triangles  = new int*[defaultSize];
	neighbors  = new int*[defaultSize];
	starSizes  = new int[defaultSize];
	starStarts = new int[defaultSize];

	for(int i=0; i<defaultSize; i++){
		triangles[i] = new int[3];
		neighbors[i] = new int[3];
	}
}

// Destructor
TriangleMesh::~TriangleMesh () {

	for(int i=0; i<curTriSize; i++){
		delete[] triangles[i];
	}
	for(int i=0; i<curNeighborSize; i++){
		delete[] neighbors[i];
	}

	delete[] vertices;
	delete[] vertNormals;
	delete[] triangles;
	delete[] neighbors;
	delete[] triNormals;
	delete[] starSizes;
	delete[] starStarts;
}

// Operations

void TriangleMesh::addVert(Vector newVert){

	if(numVerts == curVertSize){
		resizeVerts();
	}

	vertices[numVerts] = newVert;
	numVerts++;
}

void TriangleMesh::addTriangle(int vertA, int vertB, int vertC){

	if(numTris == curTriSize){
		resizeTriangles();
		resizeNeighbors();
	}

	triangles[numTris][0] = vertA;
	triangles[numTris][1] = vertB;
	triangles[numTris][2] = vertC;

	starStarts[vertA] = numTris;
	starStarts[vertB] = numTris;
	starStarts[vertC] = numTris;

//	calcTriangleNormals(numTris);
	numTris++;
}

void TriangleMesh::calcTriangleNormal(int triNum){

	Vector v1 = vertices[triangles[triNum][1]] - vertices[triangles[triNum][0]];
	Vector v2 = vertices[triangles[triNum][2]] - vertices[triangles[triNum][0]];

	Vector tempNormal = v1.Cross(v2);
	tempNormal.normalize();
	triNormals[triNum] = tempNormal;
}


void TriangleMesh::calcCentroid(){

	Vector sum(0,0,0);

	for(int i=0; i<numVerts; i++){
		sum = sum + vertices[i];
	}
	centroid = sum / numVerts;
}


void TriangleMesh::TransToCentroid(){

	for(int i=0; i<numVerts; i++){
		vertices[i] = vertices[i] - centroid;
	}
}

bool TriangleMesh::calcNeighbor(int triangleNum){
/*
	if(numNeighbors == curNeighborSize){
		resizeNeighbors();
	}
*/
	bool rVal = true;

	int index   = 0;
	int coVert0 = 0;
	int coVert1 = 0;
	int coVert2 = 0;
	int numFound = 0;

	coVert0 = triangles[triangleNum][0];
	coVert1 = triangles[triangleNum][1];
	coVert2 = triangles[triangleNum][2];

	neighbors[triangleNum][0] = -1;
	neighbors[triangleNum][1] = -1;
	neighbors[triangleNum][2] = -1;

	while((index < numTris) && (numFound < 3)){
		if(hasEdge(coVert1, coVert2, index) && (index != triangleNum)){
			neighbors[triangleNum][0] = index;
			numFound++;
		}
		if(hasEdge(coVert0, coVert2, index) && (index != triangleNum)){
			neighbors[triangleNum][1] = index;
			numFound++;
		}
		if(hasEdge(coVert0, coVert1, index) && (index != triangleNum)){
			neighbors[triangleNum][2] = index;
			numFound++;
		}
		index++;
	}
	if(numFound < 3){
		rVal = false;
	} else {
		numNeighbors++;
	}

	return rVal;
}

bool TriangleMesh::calcNeighbors()
{
	bool rVal = true;

	for(int i=0; i<numTris && rVal; i++){
		rVal = calcNeighbor(i);
	}

	return rVal;
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

void TriangleMesh::resizeVerts(){

	int size = curVertSize;
	curVertSize = curVertSize + size;

	Vector* tempVerts = new Vector[curVertSize];
	memcpy(tempVerts, vertices, size * sizeof(Vector));
	delete[] vertices;
	vertices = tempVerts;

	int* tempStarSizes = new int[curVertSize];
	memcpy(tempStarSizes, starSizes, size * sizeof(int));
	delete[] starSizes;
	starSizes = tempStarSizes;

	int* tempStarStarts = new int[curVertSize];
	memcpy(tempStarStarts, starStarts, size * sizeof(int));
	delete[] starStarts;
	starStarts = tempStarStarts;

}

// convert to a memcopy.
void TriangleMesh::resizeTriangles(){

	int size = curTriSize;
	curTriSize = curTriSize + size;

	int** tempTri = triangles;
	triangles = new int*[curTriSize];

	// copy data to new array
	for (int i=0; i < size; i++){
		triangles[i] = tempTri[i];
	}

	for(int i=size; i < curTriSize; i++){
		triangles[i] = new int[3];
	}
	delete[] tempTri;

	Vector* tempTriN = new Vector[curTriSize];
	memcpy(tempTriN, triNormals, size * sizeof(Vector));
	delete[] triNormals;
	triNormals = tempTriN;

}

// convert to a memcopy.
void TriangleMesh::resizeNeighbors(){

	int size = curNeighborSize;
	curNeighborSize = curNeighborSize + size;

	int** tempNeigh = neighbors;
	neighbors = new int*[curNeighborSize];

	// copy data to new array
	for (int i=0; i < size; i++){
		neighbors[i] = tempNeigh[i];
	}

	for(int i=size; i < curNeighborSize; i++){
		neighbors[i] = new int[3];
	}
	delete[] tempNeigh;
}

void TriangleMesh::clearMesh(){

	for(int i=0; i<curTriSize; i++){
		delete[] triangles[i];
	}
	for(int i=0; i<curNeighborSize; i++){
		delete[] neighbors[i];
	}

	delete[] vertices;
	delete[] vertNormals;
	delete[] triangles;
	delete[] neighbors;
	delete[] triNormals;
	delete[] starSizes;
	delete[] starStarts;

	int defaultSize = 250;

	numVerts	 = 0;
	numTris		 = 0;
	numNeighbors = 0;

	curVertSize		= defaultSize;
	curTriSize		= defaultSize;
	curNeighborSize = defaultSize;

	vertices   = new Vector[defaultSize];
	vertNormals = new Vector[defaultSize];
	triNormals = new Vector[defaultSize];
	triangles  = new int*[defaultSize];
	neighbors  = new int*[defaultSize];
	starSizes  = new int[defaultSize];
	starStarts = new int[defaultSize];

	for(int i=0; i<defaultSize; i++){
		triangles[i] = new int[3];
		neighbors[i] = new int[3];
	}
}

// returns the faces that the star is made of.
int* TriangleMesh::getStar(int vertNum){

	int numFaces = getStarN(vertNum);
	int* intP = new int[numFaces];
	intP[0] = starStarts[vertNum];


	int edgeV2( triangles[intP[0]][0] );
	int edgeV3( triangles[intP[0]][1] );
	if(edgeV2 == vertNum){
		edgeV2 = triangles[intP[0]][1];
		edgeV3 = triangles[intP[0]][2];
	} else if(edgeV3 == vertNum) {
		edgeV3 = triangles[intP[0]][2];
	}

	int index = 1;

	int neighbor( getNeighbor(vertNum, edgeV2, intP[0]) );

	while(neighbor != -1 && neighbor != intP[0]){
		intP[index] = neighbor;
		edgeV2 = getThirdVert(vertNum, edgeV2, intP[index]);
		neighbor = getNeighbor(vertNum, edgeV2, intP[index]);
		index++;
	}

	if(neighbor == -1){
		neighbor = getNeighbor(vertNum, edgeV3, intP[0]);
		while(neighbor != -1){
			intP[index] = neighbor;
			edgeV3 = getThirdVert(vertNum, edgeV3, intP[index]);
			neighbor = getNeighbor(vertNum, edgeV3, intP[index]);
			index++;
		}
	}

	return intP;
}


Vector TriangleMesh::getVertNormal(int vertNum){

	int* faceList = getStar(vertNum);
	int numNFaces = getStarN(vertNum);

	Vector sum(0,0,0);

	for(int i=0; i<numNFaces; i++){
		sum = sum + triNormals[faceList[i]];
	}
	sum.normalize();
	delete[] faceList;

	return sum;
}

void TriangleMesh::calcStarSizes(){

	for(int i=0; i<numVerts; i++){
		starSizes[i] = 0;
	}

	for(int i=0; i<numTris; i++){
		starSizes[triangles[i][0]]++;
		if(triangles[i][1] != triangles[i][0]){
			starSizes[triangles[i][1]]++;
		}
		if((triangles[i][2] != triangles[i][0]) && (triangles[i][2] != triangles[i][1])){
			starSizes[triangles[i][2]]++;
		}
	}
}

int TriangleMesh::getThirdVert(int coVert1, int coVert2, int triangleNum){

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

int TriangleMesh::getNeighbor(int coVert1, int coVert2, int triangleNum){

	int index = 0;
	bool found = false;

	while((index < 2) && !found){
		if((triangles[triangleNum][index] == coVert1) || (triangles[triangleNum][index] == coVert2)){
			index++;
		} else {
			found = true;
		}
	}

	return neighbors[triangleNum][index];
}

void TriangleMesh::setTriangle(int triNum, int vertA, int vertB, int vertC){

	triangles[triNum][0] = vertA;
	triangles[triNum][1] = vertB;
	triangles[triNum][2] = vertC;

//	calcTriangleNormals(triNum);
}

void TriangleMesh::clearTriangles(){

	for(int i=0; i<curTriSize; i++){
		delete[] triangles[i];
	}
	for(int i=0; i<curNeighborSize; i++){
		delete[] neighbors[i];
	}

	delete[] vertNormals;
	delete[] triangles;
	delete[] neighbors;
	delete[] triNormals;
	delete[] starSizes;
	delete[] starStarts;

	int defaultSize = 250;

	numTris		 = 0;
	numNeighbors = 0;

	curTriSize		= defaultSize;
	curNeighborSize = defaultSize;

	vertNormals = new Vector[defaultSize];
	triNormals = new Vector[defaultSize];
	triangles  = new int*[defaultSize];
	neighbors  = new int*[defaultSize];
	starSizes  = new int[defaultSize];
	starStarts = new int[defaultSize];

	for(int i=0; i<defaultSize; i++){
		triangles[i] = new int[3];
		neighbors[i] = new int[3];
	}
}

int TriangleMesh::getNextVertex(int triNum, int vertNum){
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

void TriangleMesh::calcTriangleNormals(){

	for(int i=0; i<numTris; i++){
		calcTriangleNormal(i);
	}
}
