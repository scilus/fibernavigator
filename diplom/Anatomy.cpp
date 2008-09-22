/*
 * Anatomy.cpp
 *
 *  Created on: 07.07.2008
 *      Author: ralph
 */

#include "Anatomy.h"

#include "wx/textfile.h"
#include <GL/glew.h>

Anatomy::Anatomy(DatasetHelper* dh) {
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
}

Anatomy::~Anatomy() {
	switch (m_type) {
	case Head_byte:
		delete[] m_byteDataset;
		break;
	case Head_short:
		delete[] m_shortDataset;
		break;
	case Overlay:
		delete[] m_floatDataset;
		break;
	case RGB:
		delete[] m_rgbDataset;
		break;
	}
	const GLuint* tex = &m_GLuint;
	glDeleteTextures(1, tex);
}

bool Anatomy::load(wxString filename)
{
	m_fullPath = filename;
#ifdef __WXMSW__
	m_name = filename.AfterLast('\\');
#else
	m_name = filename.AfterLast('/');
#endif
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
		else m_type = TERROR;
	}
	else if (m_repn.Cmp(wxT("short")) == 0) m_type = Head_short;
	else if (m_repn.Cmp(wxT("float")) == 0) m_type = Overlay;
	else m_type = TERROR;

	if (flag)
	{
		flag = false;
		wxFile dataFile;
		if (dataFile.Open(filename.BeforeLast('.')+ wxT(".ima")))
		{
			wxFileOffset nSize = dataFile.Length();
			if (nSize == wxInvalidOffset) return NULL;

			switch (m_type)
			{
			case Head_byte: {
				m_byteDataset = new wxUint8[nSize];
				if (dataFile.Read(m_byteDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] m_byteDataset;
					return NULL;
				}
				flag = true;
			} break;

			case Head_short: {
				m_shortDataset = new wxUint16[nSize/2];
					if (dataFile.Read(m_shortDataset, (size_t) nSize) != nSize)
					{
						dataFile.Close();
						delete[] m_shortDataset;
						return NULL;
					}
					flag = true;
					wxUint16 max = 0;
					for ( unsigned int i = 0 ; i < sizeof(m_shortDataset) ; ++i)
					{
						max = wxMax(max, m_shortDataset[i]);
					}
					printf("max: %d\n", max);
				} break;

			case Overlay: {
				m_floatDataset = new float[nSize/4];
				if (dataFile.Read(m_floatDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] m_floatDataset;
					return NULL;
				}
				flag = true;
			} break;

			case RGB: {
				wxUint8 *buffer = new wxUint8[nSize];
				m_rgbDataset = new wxUint8[nSize];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return NULL;
				}
				flag = true;

				int offset = m_columns * m_rows;
				int startslize = 0;

				for (int i = 0 ; i < m_frames ; ++i)
				{
					startslize = i * offset * 3;
					for (int j = 0 ; j < offset ; ++j)
					{
						m_rgbDataset[startslize + 3*j] = buffer[startslize + j];
						m_rgbDataset[startslize + 3*j + 1] = buffer[startslize + offset + j];
						m_rgbDataset[startslize + 3*j + 2] = buffer[startslize + 2*offset + j];
					}
				}
			} break;
			}
		}
		dataFile.Close();
	}

	if (flag) generateTexture();

	is_loaded = flag;

	return flag;
}

void Anatomy::generateTexture()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &m_GLuint);
	glBindTexture(GL_TEXTURE_3D, m_GLuint);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

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

void Anatomy::cutSurface()
{
	Surface* s = new Surface(m_dh);
	s->setSetSampleRate(0.025);

	int xOff = m_columns/2;
	int yOff = m_rows/2;
	int zOff = m_frames/2;
	std::vector< std::vector< double > > splinePoints = s->getSplinePoints();
	for ( unsigned int i = 0 ; i < splinePoints.size() ; ++i)
	{
		std::vector< double > p = splinePoints[i];
		p[0] += xOff;
		p[1] += yOff;
		p[2] += zOff;
		splinePoints[i] = p;
	}


	std::vector<int>mask(m_rows*m_frames, m_columns);
	switch (m_type)
	{
		case Head_byte: {
			for ( unsigned int i = 0 ; i < splinePoints.size() ; ++i)
			{
				std::vector< double > p = splinePoints[i];
				if (p[0] < 0 || p[0] > m_columns - 1 || p[1] < 0 || p[1] > m_rows -1 || p[2] < 0 || p[2] > m_frames -1) continue;

				if (mask[(int)p[1] + m_rows*(int)p[2]] > (int)p[0])
					mask[(int)p[1] + m_rows*(int)p[2]] = (int)p[0];
			}
			for ( int y = 0; y < m_rows ; ++y)
				for ( int z = 0 ; z < m_frames ; ++z)
				{
					// distance = start of the cut
					int distance = -2;
					for (int x = mask[y + z*m_rows] - distance  ; x > 0 ; --x)
					{
						m_byteDataset[x + y * m_columns + z * m_columns * m_rows] = 0;
					}
				}
			break;
		}
		case Head_short: {
			break;
		}
		case Overlay: {
			break;
		}
		case RGB: {
			break;
		}
		default:
			break;
	}
	generateTexture();
}

wxUint8* Anatomy::getByteDataset()
{
	if (m_type == Head_byte)
	{
		return m_byteDataset;
	}
	else
		return NULL;
}
/*
bool Anatomy::isInsideTriangle(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3)
{
	FVector A(x1, y1);
	FVector B(x2, y2);
	FVector C(x3, y3);
	FVector P(x,y);
	// Compute vectors
	FVector v0 = C - A;
	FVector v1 = B - A;
	FVector v2 = P - A;

	// Compute dot products
	double dot00 = v0 *v0;
	double dot01 = v0 * v1;
	double dot02 = v0 * v2;
	double dot11 = v1 * v1;
	double dot12 = v1 * v2;

	// Compute barycentric coordinates
	double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in triangle
	return (u > 0) && (v > 0) && (u + v < 1);
}

void Anatomy::cutRestOfLine(int x1, int y, int z)
{
	switch (m_type)
	{
		case Head_byte: {
			for (int x = x1 ; x < m_columns ; ++x)
				m_byteDataset[x + y*m_columns + z*m_columns*m_rows ] = 0;
			break;
		}
		case Head_short: {
			for (int x = x1; x < m_columns ; ++x)
				m_shortDataset[x + y*m_columns + z*m_columns*m_rows ] = 0;
			break;
		}
		case Overlay: {
			for (int x = x1 ; x < m_columns ; ++x)
				m_floatDataset[x + y*m_columns + z*m_columns*m_rows ] = 0;
			break;
		}
		case RGB: {
			for (int x = x1 ; x < m_columns ; ++x)
				m_rgbDataset[x + y*m_columns + z*m_columns*m_rows ] = 0;
			break;
		}
		default:
			break;
	}
}
*/
