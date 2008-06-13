#include "mesh.h"

Mesh::Mesh()
{
	
}

Mesh::~Mesh()
{
	delete[] m_polygonArray;
	delete[] m_vertexArray;
}
