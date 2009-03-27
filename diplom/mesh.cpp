#include "mesh.h"

#include "wx/wfstream.h"
#include "wx/datstrm.h"

Mesh::Mesh(DatasetHelper* dh)
{
	m_dh = dh;
	m_type = not_initialized;
	m_length = 0;
	m_bands = 0;
	m_frames = 0;
	m_rows = 0;
	m_columns = 0;
	m_repn = wxT("");
	m_xVoxel = 0.0;
	m_yVoxel = 0.0;
	m_zVoxel = 0.0;
	is_loaded = false;
	m_highest_value = 1.0;
	m_threshold = 0.0f;
	m_oldMax = 1.0;
	m_newMax = 1.0;
	m_alpha = 1.0f;
	m_show = true;
	m_showFS = true;
	m_useTex = true;
	m_color = wxColour(230,230,230);
	m_hasTreeId = false;
	m_GLuint = 0;
	m_tMesh = new TriangleMesh(m_dh);
}

Mesh::~Mesh()
{
	delete m_tMesh;
}

bool Mesh::load(wxString filename)
{
	if (filename.AfterLast('.') == _T("mesh"))
		return loadMesh(filename);
	if (filename.AfterLast('.') == _T("surf"))
		return loadSurf(filename);
	return true;
}

bool Mesh::loadSurf(wxString filename)
{
	m_dh->printDebug(_T("start loading freesurfer mesh file"), 1);
	wxFile dataFile;
	wxFileOffset nSize = 0;
	int pc = 3;
	converterByteINT32 cbi;
	converterByteFloat cbf;

	if (dataFile.Open(filename))
	{
		nSize = dataFile.Length();
		if (nSize == wxInvalidOffset) return false;
	}
	
	wxUint8* buffer = new wxUint8[nSize];
	dataFile.Read(buffer, nSize);
	dataFile.Close();
	
	// find double \n (0x0a)
	while (pc < nSize)
	{
		if (buffer[pc++] == 0x0a)
		{
			if (buffer[pc++] == 0x0a)
				break;
		}
	}
	
	cbi.b[3] = buffer[pc++];
	cbi.b[2] = buffer[pc++];
	cbi.b[1] = buffer[pc++];
	cbi.b[0] = buffer[pc++];
	setCountVerts(cbi.i);
	cbi.b[3] = buffer[pc++];
	cbi.b[2] = buffer[pc++];
	cbi.b[1] = buffer[pc++];
	cbi.b[0] = buffer[pc++];
	setCountPolygons(cbi.i);
	
	printf("pc: %d   vertex count: %d   face count: %d\n", pc, m_countVerts, m_countPolygons);
	
	for (unsigned int i = 0 ; i < m_countVerts ; ++i)
	{
		cbf.b[3] = buffer[pc++];
		cbf.b[2] = buffer[pc++];
		cbf.b[1] = buffer[pc++];
		cbf.b[0] = buffer[pc++];
		float x = cbf.f + 0.5 + m_dh->columns/2;
		cbf.b[3] = buffer[pc++];
		cbf.b[2] = buffer[pc++];
		cbf.b[1] = buffer[pc++];
		cbf.b[0] = buffer[pc++];
		float y = cbf.f + 0.5 + m_dh->rows/2;
		cbf.b[3] = buffer[pc++];
		cbf.b[2] = buffer[pc++];
		cbf.b[1] = buffer[pc++];
		cbf.b[0] = buffer[pc++];
		float z = cbf.f + 0.5 + m_dh->frames/2;
		m_tMesh->addVert(x, y, z);
	}
	
	for (unsigned int i = 0 ; i < getCountPolygons() ; ++i)
	{
		cbi.b[3] = buffer[pc++];
		cbi.b[2] = buffer[pc++];
		cbi.b[1] = buffer[pc++];
		cbi.b[0] = buffer[pc++];
		int v1 = cbi.i;
		cbi.b[3] = buffer[pc++];
		cbi.b[2] = buffer[pc++];
		cbi.b[1] = buffer[pc++];
		cbi.b[0] = buffer[pc++];
		int v2 = cbi.i;
		cbi.b[3] = buffer[pc++];
		cbi.b[2] = buffer[pc++];
		cbi.b[1] = buffer[pc++];
		cbi.b[0] = buffer[pc++];
		int v3 = cbi.i;

		m_tMesh->addTriangle(v1, v2, v3);
	}
	
	m_tMesh->calcNeighbors();
	m_tMesh->calcVertNormals();

	m_fullPath = filename;
	#ifdef __WXMSW__
		m_name = filename.AfterLast('\\');
	#else
	m_name = filename.AfterLast('/');
	#endif
	m_type = Mesh_;

	
	return true;
}

bool Mesh::loadMesh(wxString filename)
{
	wxFile dataFile;
	wxFileOffset nSize = 0;

	if (dataFile.Open(filename))
	{
		 nSize = dataFile.Length();
		if (nSize == wxInvalidOffset) return false;
	}
	wxUint8* buffer = new wxUint8[nSize];
	if (dataFile.Read(buffer, (size_t) nSize) != nSize)
	{
		dataFile.Close();
		delete[] buffer;
		return false;
	}
	if (buffer[0] == 'a')
	{
		setFiletype(ascii);
		// ascii file, maybe later
		return false;
	}
	if (buffer [0] == 'b')
	{
		// maybe binary file
		char* filetype = new char[10];
		for (int i = 0; i < 9; ++i)
			filetype[i] = buffer[i];
		filetype[9] = 0;

		wxString type(filetype, wxConvUTF8);
		if (type == wxT("binarABCD")) {
			setFiletype(binaryBE);
			//big endian, maybe later
			return false;
		}
		else if (type == wxT("binarDCBA")) {
			setFiletype(binaryLE);
		}
		else return false;

		setPolygonDim(buffer[17]);

		int fp = 29;
		converterByteINT32 c;
		converterByteFloat f;

		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];
		// number of vertices
		setCountVerts(c.i);

		fp += 4;
		for (unsigned int i = 0 ; i < c.i ; ++i)
		{
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			float x = f.f;
			fp += 4;
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			float y = m_dh->rows - f.f;
			fp += 4;
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			float z = m_dh->frames - f.f;
			fp += 4;
			m_tMesh->addVert(x, y, z);
		}

		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];

		setCountNormals(c.i);

		fp += 8 + 12 * c.i;

		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];
		setCountPolygons(c.i);

		fp += 4;
		for (unsigned int i = 0 ; i < getCountPolygons() ; ++i)
		{
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			int v1 = c.i;
			fp += 4;
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			int v2 = c.i;
			fp += 4;
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			int v3 = c.i;
			fp += 4;
			m_tMesh->addTriangle(v1, v2, v3);
		}
	}

	m_tMesh->calcNeighbors();
	m_tMesh->calcVertNormals();

	m_fullPath = filename;
#ifdef __WXMSW__
	m_name = filename.AfterLast('\\');
#else
	m_name = filename.AfterLast('/');
#endif
	m_type = Mesh_;

	return true;
}

void Mesh::generateGeometry()
{
	if (m_GLuint) glDeleteLists(m_GLuint, 1);
	GLuint dl = glGenLists(1);
	glNewList (dl, GL_COMPILE);

	Triangle triangleEdges;
	Vector point;
	Vector pointNormal;

	glBegin(GL_TRIANGLES);
		for (int i = 0 ; i < m_tMesh->getNumTriangles() ; ++i)
		{
			triangleEdges = m_tMesh->getTriangle(i);
			for(int j = 0 ; j < 3 ; ++j)
			{
				pointNormal = m_tMesh->getVertNormal(triangleEdges.pointID[j]);
				glNormal3d(pointNormal.x, pointNormal.y, pointNormal.z);
				point = m_tMesh->getVertex(triangleEdges.pointID[j]);
				glVertex3d(point.x, point.y, point.z);
			}
		}
	glEnd();

	glEndList();
	m_GLuint = dl;
}

void Mesh::activateLIC()
{

}

GLuint Mesh::getGLuint()
{
	if (!m_GLuint)
		generateGeometry();
	return m_GLuint;
}

void Mesh::draw()
{
	if (!m_GLuint)
		generateGeometry();
	glCallList(m_GLuint);
}
