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
	m_threshold = 0.10f;
	m_show = true;
	m_showFS = true;
	m_useTex = true;
	m_color = wxColour(30,30,30);
}

Mesh::~Mesh()
{
	delete[] m_polygonArray;
	delete[] m_vertexArray;
}

bool Mesh::load(wxString filename)
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
		converterByteToINT32 c;
		converterByteToFoat f;

		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];
		// number of vertices
		setCountVerts(c.i);

		m_vertexArray = new vertex[c.i];
		fp += 4;
		for (unsigned int i = 0 ; i < c.i ; ++i)
		{
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			m_vertexArray[i].x = f.f;
			fp += 4;
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			m_vertexArray[i].y = f.f;
			fp += 4;
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			m_vertexArray[i].z = f.f;
			fp += 4;
		}

		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];

		setCountNormals(c.i);
		fp += 4;
		if (c.i == getCountVerts())
		{
			for (unsigned int i = 0 ; i < c.i ; ++i)
			{
				f.b[0] = buffer[fp];
				f.b[1] = buffer[fp+1];
				f.b[2] = buffer[fp+2];
				f.b[3] = buffer[fp+3];
				m_vertexArray[i].nx = f.f;
				fp += 4;
				f.b[0] = buffer[fp];
				f.b[1] = buffer[fp+1];
				f.b[2] = buffer[fp+2];
				f.b[3] = buffer[fp+3];
				m_vertexArray[i].ny = f.f;
				fp += 4;
				f.b[0] = buffer[fp];
				f.b[1] = buffer[fp+1];
				f.b[2] = buffer[fp+2];
				f.b[3] = buffer[fp+3];
				m_vertexArray[i].nz = f.f;
				fp += 4;
			}
		}

		fp += 4;

		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];
		setCountPolygons(c.i);

		m_polygonArray = new polygon[c.i];
		fp += 4;
		for (unsigned int i = 0 ; i < getCountPolygons() ; ++i)
		{
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			m_polygonArray[i].v1 = c.i;
			fp += 4;
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			m_polygonArray[i].v2 = c.i;
			fp += 4;
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			m_polygonArray[i].v3 = c.i;
			fp += 4;
		}
	}
	m_fullPath = filename;
#ifdef __WXMSW__
	m_name = filename.AfterLast('\\');
#else
	m_name = filename.AfterLast('/');
#endif
	m_type = Mesh_;

	generateGeometry();

	return true;
}

void Mesh::generateGeometry()
{
	int xOff = m_dh->columns/2;
	int yOff = m_dh->rows/2;
	int zOff = m_dh->frames/2;

	GLuint dl = glGenLists(1);
	glNewList (dl, GL_COMPILE);

	glBegin(GL_TRIANGLES);
		for (unsigned int j = 0 ; j < m_countPolygons ; ++j)
		{
			polygon p = m_polygonArray[j];

			glNormal3f( 	m_vertexArray[p.v1].nx,
							m_vertexArray[p.v1].ny,
							m_vertexArray[p.v1].nz);
			glVertex3f( 	m_vertexArray[p.v1].x - xOff,
							m_vertexArray[p.v1].y - yOff,
							m_vertexArray[p.v1].z - zOff);

			glNormal3f( 	m_vertexArray[p.v2].nx,
							m_vertexArray[p.v2].ny,
							m_vertexArray[p.v2].nz);
			glVertex3f(		m_vertexArray[p.v2].x - xOff,
							m_vertexArray[p.v2].y - yOff,
							m_vertexArray[p.v2].z - zOff);

			glNormal3f( 	m_vertexArray[p.v3].nx,
							m_vertexArray[p.v3].ny,
							m_vertexArray[p.v3].nz);
			glVertex3f(		m_vertexArray[p.v3].x - xOff,
							m_vertexArray[p.v3].y - yOff,
							m_vertexArray[p.v3].z - zOff);
		}
	glEnd();

	glEndList();
	m_GLuint = dl;
}
