#ifndef MESH_H_
#define MESH_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

enum MeshFileType {
	ascii,
	binaryLE,
	binaryBE,
};

struct vertex {
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
};

struct polygon {
	int v1;
	int v2;
	int v3;
};

union converterByteToINT32 {
	wxUint8 b[4];
	wxUint32 i;
};

union converterByteToFoat {
	wxUint8 b[4];
	float f;
};


class Mesh {
	
public:
	Mesh();
	~Mesh();
	
	void setFiletype(int value) {m_filetype = value;};
	void setCountVerts(int value) {m_countVerts = value;};
	void setCountNormals(int value) {m_countNormals = value;};
	void setCountPolygons(int value) {m_countPolygons = value;};
	void setPolygonDim(int value) {m_polygonDim = value;};
	
	int getFiletype() {return m_filetype;};
	int getCountVerts() {return m_countVerts;};
	int getCountNormals() {return m_countNormals;};
	int getCountPolygons() {return m_countPolygons;};
	int getPolygonDim() {return m_polygonDim;};

	vertex* m_vertexArray;
	polygon* m_polygonArray;
	
private:
	uint m_filetype;
	uint m_countVerts;
	uint m_countNormals;
	uint m_countTimeSteps;
	uint m_countPolygons;
	uint m_polygonDim;
	
};

#endif /*MESH_H_*/
