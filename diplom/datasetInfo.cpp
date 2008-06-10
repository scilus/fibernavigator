#include "datasetInfo.h"
#include <GL/glew.h>

DatasetInfo::DatasetInfo()
{
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
	m_threshold = 0.10;
	m_show = true;
	m_showFS = true;
	m_useTex = true;
}

DatasetInfo::~DatasetInfo()
{
	switch (m_type)
		{
		case Head_byte:
			delete m_byteDataset;
			break;
		case Head_short:
			delete m_shortDataset;
			break;
		case Overlay:
			delete m_floatDataset;
			break;
		case RGB:
			delete m_rgbDataset;
			break;
		default:
			break;
		}
}

bool DatasetInfo::load(wxString filename)
{
	m_name = filename.AfterLast('/');
	// read header file
	wxTextFile headerFile;
	bool flag = false;
	if (headerFile.Open(filename))
	{
		size_t i;
		wxString sLine;
		wxString sValue;
		wxString sLabel;
		long lTmpValue;
		for (i = 3 ; i < headerFile.GetLineCount() ; ++i)
		{
			sLine = headerFile.GetLine(i);
			sLabel = sLine.BeforeLast(' ');
			sValue = sLine.AfterLast(' ');
			sLabel.Trim(false);
			sLabel.Trim();
			if (sLabel.Contains(wxT("length:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_length = (int)lTmpValue;
			}
			if (sLabel == wxT("nbands:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_bands = (int)lTmpValue;
			}
			if (sLabel == wxT("nframes:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_frames = (int)lTmpValue;
			}
			if (sLabel == wxT("nrows:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_rows = (int)lTmpValue;
			}
			if (sLabel == wxT("ncolumns:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_columns = (int)lTmpValue;
			}
			if (sLabel == wxT("repn:"))
			//if (sLabel.Contains(wxT("repn:"))) 
			{
				m_repn = sValue;
			}
			if (sLabel.Contains(wxT("voxel:"))) 
			{
				wxString sNumber;
				sValue = sLine.AfterLast(':');
				sValue = sValue.BeforeLast('\"');
				sNumber = sValue.AfterLast(' ');
				flag = sNumber.ToDouble(&m_zVoxel); 
				sValue = sValue.BeforeLast(' ');
				sNumber = sValue.AfterLast(' ');
				flag = sNumber.ToDouble(&m_yVoxel);
				sValue = sValue.BeforeLast(' ');
				sNumber = sValue.AfterLast('\"');
				flag = sNumber.ToDouble(&m_xVoxel);
			}
		}
	}
	headerFile.Close();
	
	if (m_repn.Cmp(wxT("ubyte")) == 0)
	{
		if (m_bands / m_frames == 1) {
			m_type = Head_byte;
		}
		else if (m_bands / m_frames == 3) {
			m_type = RGB;
		}
		else m_type = ERROR;
	}
	else if (m_repn.Cmp(wxT("short")) == 0) m_type = Head_short;
	else if (m_repn.Cmp(wxT("float")) == 0) m_type = Overlay;
	else m_type = ERROR;
	
	is_loaded = flag;
	return flag;
}

void DatasetInfo::generateTexture()
{
	switch (m_type)
	{
	case Head_byte:
		glTexImage3D(GL_TEXTURE_3D, 
			0, 
			GL_RGBA, 
			m_columns, 
			m_rows,
			m_frames,
			0, 
			GL_LUMINANCE, 
			GL_UNSIGNED_BYTE,
			m_byteDataset);
		break;
	case Head_short:
		glTexImage3D(GL_TEXTURE_3D, 
			0, 
			GL_RGBA, 
			m_columns, 
			m_rows,
			m_frames,
			0, 
			GL_LUMINANCE, 
			GL_UNSIGNED_SHORT,
			m_shortDataset);
		break;
	case Overlay:
		glTexImage3D(GL_TEXTURE_3D, 
			0, 
			GL_RGBA, 
			m_columns, 
			m_rows,
			m_frames,			
			0, 
			GL_LUMINANCE, 
			GL_FLOAT,
			m_floatDataset);
		break;
	case RGB:
		glTexImage3D(GL_TEXTURE_3D, 
			0, 
			GL_RGBA, 
			m_columns, 
			m_rows,
			m_frames,
			0, 
			GL_RGB, 
			GL_UNSIGNED_BYTE,
			m_rgbDataset);
		break;
	default:
		break;
	}
}

void DatasetInfo::generateGeometry(int x, int y, int z)
{
	switch (m_type)
	{
	case Mesh_: {
		glBegin(GL_TRIANGLES);
			for ( int j = 0 ; j < m_mesh->getCountPolygons() ; ++j)
			{
				polygon p = m_mesh->m_polygonArray[j];
				
				glNormal3f( 	m_mesh->m_vertexArray[p.v1].nx, 
								m_mesh->m_vertexArray[p.v1].ny,
								m_mesh->m_vertexArray[p.v1].nz);
				glVertex3f( 	m_mesh->m_vertexArray[p.v1].x - x, 
								m_mesh->m_vertexArray[p.v1].y - y, 
								m_mesh->m_vertexArray[p.v1].z - z);
				
				glNormal3f( 	m_mesh->m_vertexArray[p.v2].nx, 
								m_mesh->m_vertexArray[p.v2].ny,
								m_mesh->m_vertexArray[p.v2].nz);
				glVertex3f(		m_mesh->m_vertexArray[p.v2].x - x, 
								m_mesh->m_vertexArray[p.v2].y - y, 
								m_mesh->m_vertexArray[p.v2].z - z);

				glNormal3f( 	m_mesh->m_vertexArray[p.v3].nx, 
								m_mesh->m_vertexArray[p.v3].ny,
								m_mesh->m_vertexArray[p.v3].nz);
				glVertex3f(		m_mesh->m_vertexArray[p.v3].x - x, 
								m_mesh->m_vertexArray[p.v3].y - y, 
								m_mesh->m_vertexArray[p.v3].z - z);
			}
			glEnd();
	} break;
	case Curves_: {
		int pc = 0;
		for ( int i = 0 ; i < m_curves->getCountLines() ; ++i )
		{
		float x1 = m_curves->getPoints()[pc];
		float y1 = m_curves->getPoints()[pc+1];
		float z1 = m_curves->getPoints()[pc+2];
		float x2 = m_curves->getPoints()[pc + m_curves->getPointsPerLine(i)*3 - 3];
		float y2 = m_curves->getPoints()[pc + m_curves->getPointsPerLine(i)*3 - 2];
		float z2 = m_curves->getPoints()[pc + m_curves->getPointsPerLine(i)*3 - 1];
		
		float r = (x1/x) - (x2/x);
		float g = (y1/y) - (y2/y);
		float b = (z1/z) - (z2/z);
		if (r < 0.0) r *= -1.0 ;
		if (g < 0.0) g *= -1.0 ;
		if (b < 0.0) b *= -1.0 ;
		printf("%.4f : %.4f : %.4f\n", r,g,b);
		
		glBegin(GL_LINE_STRIP);
		glColor3f(r,g,b);
		for (int j = 0; j < m_curves->getPointsPerLine(i) ; ++j )
		{
			glVertex3f ( m_curves->getPoints()[pc++]-x, m_curves->getPoints()[pc++]-y, m_curves->getPoints()[pc++]-z);
		}
		
		glEnd();
		}
	} break;
	default:;
	}
	
}

wxString DatasetInfo::getInfoString()
{
	if (m_type == Mesh_) return wxT("Mesh File");
	if (!is_loaded) return wxT("not loaded");
	wxString infoString1, infoString2, infoString3;
	infoString1.Empty();
	infoString2.Empty();
	infoString3.Empty();
	infoString1 = wxString::Format(wxT("Length: %d\nBands: %d\nFrames: %d\nRows: %d\nColumns: %d\nTyp: %d\nRepn: "), 
			m_length, m_bands, m_frames, m_rows, m_columns, m_type) + m_repn;
	infoString2 = wxString::Format(wxT("\nx Voxel: %.2f\ny Voxel: %.2f\nz Voxel: %.2f"), m_xVoxel, m_yVoxel, m_zVoxel);
	return m_name + wxT(":\n") + infoString1;
}
