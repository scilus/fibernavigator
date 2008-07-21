#ifndef MESH_H_
#define MESH_H_

#include "datasetInfo.h"

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


class Mesh : public DatasetInfo {

public:
	Mesh();
	~Mesh();

	bool load(wxString filename);
	void draw() {};
	void generateTexture() {};
	void generateGeometry() ;
	void initializeBuffer() {};

	void setFiletype(int value) 	  {m_filetype = value;};
	void setCountVerts(int value) 	  {m_countVerts = value;};
	void setCountNormals(int value)   {m_countNormals = value;};
	void setCountPolygons(int value)  {m_countPolygons = value;};
	void setPolygonDim(int value) 	  {m_polygonDim = value;};

	unsigned int getFiletype() 		{return m_filetype;};
	unsigned int getCountVerts() 	{return m_countVerts;};
	unsigned int getCountNormals() 	{return m_countNormals;};
	unsigned int getCountPolygons() {return m_countPolygons;};
	unsigned int getPolygonDim() 	{return m_polygonDim;};

	vertex* m_vertexArray;
	polygon* m_polygonArray;

private:
	unsigned int m_filetype;
	unsigned int m_countVerts;
	unsigned int m_countNormals;
	unsigned int m_countTimeSteps;
	unsigned int m_countPolygons;
	unsigned int m_polygonDim;
};

#endif /*MESH_H_*/
