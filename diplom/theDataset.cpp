#include "theDataset.h"

TheDataset::TheDataset()
{
	m_rows = 1;
	m_columns = 1;
	m_frames = 1;
	m_lastError = wxT("");
}

TheDataset::~TheDataset()
{
	
}

DatasetInfo* TheDataset::load(wxString filename)
{
	// check file extension
	wxString ext = filename.AfterLast('.');
	if (ext == wxT("mesh")) {
		DatasetInfo *info = new DatasetInfo();
		info->m_mesh = loadMesh(filename);
		info->setType(Mesh_);
		info->setName(filename.AfterLast('/'));
		return info;
	}
	else if (ext == wxT("curves")) {
		DatasetInfo *info = new DatasetInfo();
		info->m_curves = loadCurves(filename);
		info->setType(Curves_);
		info->setName(filename.AfterLast('/'));
		return info;
	}
	else if (ext != wxT("hea")) return false;
	
	DatasetInfo *info = new DatasetInfo();
	bool flag = info->load(filename); 
	if (!flag)
	{
		m_lastError = wxT("couldn't load header file");
		return NULL;
	}
	
	if ((m_rows + m_columns + m_frames) == 3)
	{
		if ( info->getRows() <= 0 || info->getColumns() <= 0 || info->getFrames() <= 0 )
		{
			m_lastError = wxT("couldn't parse header file");
			return NULL;
		}
	}
	else
	{
		if ( info->getRows() != m_rows || info->getColumns() != m_columns || info->getFrames() != m_frames )
		{
			m_lastError = wxT("dimensions of loaded files must be the same");
			return NULL;
		}
	}
	
	if (flag)
	{
		flag = false;
		wxFile dataFile;
		if (dataFile.Open(filename.BeforeLast('.')+ wxT(".ima")))
		{
			wxFileOffset nSize = dataFile.Length();
			if (nSize == wxInvalidOffset) return NULL;
			
			switch (info->getType())
			{
			case Head_byte: {
				info->m_byteDataset = new wxUint8[nSize];
				if (dataFile.Read(info->m_byteDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] info->m_byteDataset;
					return NULL;
				}
				flag = true;
			} break;

			case Head_short: {
				info->m_shortDataset = new wxUint16[nSize/2];
					if (dataFile.Read(info->m_shortDataset, (size_t) nSize) != nSize)
					{
						dataFile.Close();
						delete[] info->m_shortDataset;
						return NULL;
					}
					flag = true;
					wxUint16 max = 0;
					for ( uint i = 0 ; i < sizeof(info->m_shortDataset) ; ++i)
					{
						max = wxMax(max, info->m_shortDataset[i]);
					}
					printf("max: %d\n", max);
				} break;
			
			case Overlay: {
				info->m_floatDataset = new float[nSize/4];
				if (dataFile.Read(info->m_floatDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] info->m_floatDataset;
					return NULL;
				}
				flag = true;
			} break;
			
			case RGB: {
				wxUint8 *buffer = new wxUint8[nSize];
				info->m_rgbDataset = new wxUint8[nSize];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return NULL;
				}
				flag = true;

				int offset = info->getColumns() * info->getRows() ;
				int startslize = 0;
				
				for (int i = 0 ; i < info->getFrames() ; ++i)
				{
					startslize = i * offset * 3;
					for (int j = 0 ; j < offset ; ++j)
					{
						info->m_rgbDataset[startslize + 3*j] = buffer[startslize + j];
						info->m_rgbDataset[startslize + 3*j + 1] = buffer[startslize + offset + j];
						info->m_rgbDataset[startslize + 3*j + 2] = buffer[startslize + 2*offset + j];
					}
				}
			} break;
			
			case ERROR:
			default:
				m_lastError = wxT("unsupported data file format");
				return NULL;
			}
		}
		dataFile.Close();
	}
	
	if (flag)
	{
		m_rows = info->getRows();
		m_columns = info->getColumns();
		m_frames = info->getFrames();
		return info;
	}
	return NULL;
}

Mesh* TheDataset::loadMesh(wxString filename)
{
	wxFile dataFile;
	wxFileOffset nSize = 0;
	Mesh *mesh = new Mesh();
	
	if (dataFile.Open(filename))
	{
		 nSize = dataFile.Length();
		if (nSize == wxInvalidOffset) return NULL;
	}
	wxUint8* buffer = new wxUint8[nSize];
	if (dataFile.Read(buffer, (size_t) nSize) != nSize)
	{
		dataFile.Close();
		delete[] buffer;
		return NULL;
	}
	if (buffer[0] == 'a')
	{
		mesh->setFiletype(ascii);
		// ascii file, maybe later
		return NULL;
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
			mesh->setFiletype(binaryBE);
			//big endian, maybe later
			return NULL;
		}
		else if (type == wxT("binarDCBA")) {
			mesh->setFiletype(binaryLE);
		}
		else return NULL;

		mesh->setPolygonDim(buffer[17]);

		int fp = 29;
		converterByteToINT32 c;
		converterByteToFoat f;
				
		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];
		// number of vertices
		mesh->setCountVerts(c.i);
		
		mesh->m_vertexArray = new vertex[c.i];
		fp += 4;
		for (uint i = 0 ; i < c.i ; ++i)
		{
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			mesh->m_vertexArray[i].x = f.f;
			fp += 4;
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			mesh->m_vertexArray[i].y = f.f;
			fp += 4;
			f.b[0] = buffer[fp];
			f.b[1] = buffer[fp+1];
			f.b[2] = buffer[fp+2];
			f.b[3] = buffer[fp+3];
			mesh->m_vertexArray[i].z = f.f;
			fp += 4;
		}
		
		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];
		
		mesh->setCountNormals(c.i);
		fp += 4;
		if (c.i == mesh->getCountVerts())
		{
			for (uint i = 0 ; i < c.i ; ++i)
			{
				f.b[0] = buffer[fp];
				f.b[1] = buffer[fp+1];
				f.b[2] = buffer[fp+2];
				f.b[3] = buffer[fp+3];
				mesh->m_vertexArray[i].nx = f.f;
				fp += 4;
				f.b[0] = buffer[fp];
				f.b[1] = buffer[fp+1];
				f.b[2] = buffer[fp+2];
				f.b[3] = buffer[fp+3];
				mesh->m_vertexArray[i].ny = f.f;
				fp += 4;
				f.b[0] = buffer[fp];
				f.b[1] = buffer[fp+1];
				f.b[2] = buffer[fp+2];
				f.b[3] = buffer[fp+3];
				mesh->m_vertexArray[i].nz = f.f;
				fp += 4;
			}
		}
		
		fp += 4;
		
		c.b[0] = buffer[fp];
		c.b[1] = buffer[fp+1];
		c.b[2] = buffer[fp+2];
		c.b[3] = buffer[fp+3];
		mesh->setCountPolygons(c.i);
		
		mesh->m_polygonArray = new polygon[c.i];
		fp += 4;
		for (uint i = 0 ; i < mesh->getCountPolygons() ; ++i)
		{
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			mesh->m_polygonArray[i].v1 = c.i;
			fp += 4;
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			mesh->m_polygonArray[i].v2 = c.i;
			fp += 4;
			c.b[0] = buffer[fp];
			c.b[1] = buffer[fp+1];
			c.b[2] = buffer[fp+2];
			c.b[3] = buffer[fp+3];
			mesh->m_polygonArray[i].v3 = c.i;
			fp += 4;
		}		
	}
	return mesh;
}

Curves* TheDataset::loadCurves(wxString filename)
{
	wxFileInputStream input (filename);
	wxTextInputStream text (input);
	text.SetStringSeparators(wxT("(,) "));
	int countLines, countPoints;
	float x,y,z;
	
	text >> countLines; // read first byte, count lines
	Curves* curves = new Curves(countLines);

	float* lines[countLines]; 
	
	int totalPoints = 0;
	for (int i = 0 ; i < countLines ; ++i)
	{
		text >> countPoints;
		curves->setPointsPerLine(i, countPoints);
		totalPoints += countPoints;
		lines[i] = new float[countPoints*3];
		
		for (int j = 0 ; j < countPoints ; ++j)
		{
			text >> x;
			lines[i][j*3] = x;
			text >> y;
			lines[i][j*3 +1] = y;
			text >> z;
			lines[i][j*3 +2] = z;
		}
		text >> countPoints;
	}
	
	curves->m_points = new float[totalPoints*3];
	int pc = 0;
	for (int i = 0 ; i < countLines ; ++i) {
		for (int j = 0 ; j < curves->getPointsPerLine(i)*3; ++j) {
			curves->m_points[pc++] = lines[i][j];
		}
	}
	return curves;
}
