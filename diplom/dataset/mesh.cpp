#include "mesh.h"

#include "wx/wfstream.h"
#include "wx/datstrm.h"

Mesh::Mesh(DatasetHelper* dh) : DatasetInfo(dh)
{
	m_color = wxColour(230,230,230);
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
	else if (filename.AfterLast('.') == _T("surf"))
		return loadSurf(filename);
	else if (filename.AfterLast('.') == _T("dip"))
			return loadDip(filename);
	return true;
}

bool Mesh::loadDip(wxString filename)
{
	m_dh->printDebug(_T("start loading DIP mesh file"), 1);
	wxTextFile file;
	wxString line;
	wxString numberString, xString, yString, zString;
	double x,y,z;
	long tmpVal, v1, v2, v3;
	float minMagnitude = 100000;
	float maxMagnitude = 0;


	if ( file.Open(filename) )
	{
		line = file.GetFirstLine();
		while ( !file.Eof() )
		{
			line = file.GetNextLine();
			if ( line.BeforeFirst('=') == _T("NumberPositions") )
			{
				 numberString = line.AfterLast('=');
				 numberString.ToLong(&tmpVal, 10);
				 setCountVerts((int)tmpVal);
			}
			if ( line == _T("PositionsFixed"))
			{
				for ( size_t i = 0 ; i < m_countVerts ; ++i)
				{
					line = file.GetNextLine();
					xString = line.BeforeFirst(' ');
					yString = line.AfterFirst(' ').BeforeLast(' ');
					zString = line.AfterLast(' ');
					xString.ToDouble(&x);
					yString.ToDouble(&y);
					zString.ToDouble(&z);
					m_tMesh->addVert(x, m_dh->rows * m_dh->yVoxel - y, m_dh->frames * m_dh->zVoxel - z);
				}
			}
			if ( line == _T("Magnitudes"))
			{
				std::vector<float>tmpMagnitudes(m_countVerts, 0);
				for ( size_t i = 0 ; i < m_countVerts ; ++i)
				{
					line = file.GetNextLine();
					line.ToDouble(&x);
					if ( x < minMagnitude) minMagnitude = x;
					if ( x > maxMagnitude) maxMagnitude = x;

					tmpMagnitudes[i] = x;
				}
				float diff = maxMagnitude - minMagnitude;
				for ( size_t i = 0 ; i < m_countVerts ; ++i)
				{
					float c = ( tmpMagnitudes[i] - minMagnitude ) / diff;
					m_tMesh->setVertexColor(i, c, c, c);
				}
			}
			if ( line.BeforeFirst('=') == _T("NumberPolygons") )
			{
				 numberString = line.AfterLast('=');
				 numberString.ToLong(&tmpVal, 10);
				 setCountPolygons((int)tmpVal);
			}
			if ( line == _T("Polygons"))
			{
				for ( size_t i = 0 ; i < m_countPolygons ; ++i)
				{
					line = file.GetNextLine();
					xString = line.BeforeFirst(' ');
					yString = line.AfterFirst(' ').BeforeLast(' ');
					zString = line.AfterLast(' ');
					xString.ToLong(&v1, 10);
					yString.ToLong(&v2, 10);
					zString.ToLong(&v3, 10);
					m_tMesh->addTriangle(v1, v2, v3);
				}
			}
		}
	}

	m_tMesh->finalize();

	m_fullPath = filename;
	#ifdef __WXMSW__
	m_name = filename.AfterLast('\\');
	#else
	m_name = filename.AfterLast('/');
	#endif
	m_type = Mesh_;
	m_isGlyph = true;

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

	for (unsigned int i = 0 ; i < m_countVerts ; ++i)
	{
		cbf.b[3] = buffer[pc++];
		cbf.b[2] = buffer[pc++];
		cbf.b[1] = buffer[pc++];
		cbf.b[0] = buffer[pc++];
		float x = cbf.f + 0.5  * m_dh->xVoxel + m_dh->columns/2 * m_dh->xVoxel;
		cbf.b[3] = buffer[pc++];
		cbf.b[2] = buffer[pc++];
		cbf.b[1] = buffer[pc++];
		cbf.b[0] = buffer[pc++];
		float y = cbf.f + 0.5 * m_dh->yVoxel + m_dh->rows/2  * m_dh->yVoxel;
		cbf.b[3] = buffer[pc++];
		cbf.b[2] = buffer[pc++];
		cbf.b[1] = buffer[pc++];
		cbf.b[0] = buffer[pc++];
		float z = cbf.f + 0.5 * m_dh->zVoxel + m_dh->frames/2  * m_dh->zVoxel;
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

	m_tMesh->finalize();

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
			float y = m_dh->rows * m_dh->yVoxel - f.f;
			fp += 4;
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			float z = m_dh->frames * m_dh->zVoxel - f.f;
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

	m_tMesh->finalize();

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
    if (m_GLuint)
        glDeleteLists(m_GLuint, 1);
    GLuint dl = glGenLists(1);
    glNewList(dl, GL_COMPILE);

    Triangle triangleEdges;
    Vector point;
    Vector pointNormal;

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_tMesh->getNumTriangles(); ++i)
    {
        triangleEdges = m_tMesh->getTriangle(i);
        for (int j = 0; j < 3; ++j)
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
