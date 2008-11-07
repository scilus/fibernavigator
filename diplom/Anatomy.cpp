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
	m_threshold = 0.00f;
	m_alpha = 1.0f;
	m_show = true;
	m_showFS = true;
	m_useTex = true;
	m_hasTreeId = false;
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
	case Vectors_:
		delete[] m_floatDataset;
		m_dh->vectors_loaded = false;
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
	else if (m_repn.Cmp(wxT("float")) == 0)
	{
		if (m_bands / m_frames == 3) {
			printf("vector dataset detected\n");
			m_type = Vectors_;
		}
		else
			m_type = Overlay;
	}
	else m_type = TERROR;

	if (flag)
	{
		flag = false;
		wxFile dataFile;
		if (dataFile.Open(filename.BeforeLast('.')+ wxT(".ima")))
		{
			wxFileOffset nSize = dataFile.Length();
			if (nSize == wxInvalidOffset) return false;

			switch (m_type)
			{
			case Head_byte: {
				m_byteDataset = new wxUint8[nSize];
				if (dataFile.Read(m_byteDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] m_byteDataset;
					return false;
				}
				flag = true;
			} break;

			case Head_short: {
				m_shortDataset = new wxUint16[nSize/2];
					if (dataFile.Read(m_shortDataset, (size_t) nSize) != nSize)
					{
						dataFile.Close();
						delete[] m_shortDataset;
						return false;
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
					return false;
				}
				flag = true;
			} break;

			case Vectors_: {
				m_floatDataset = new float[nSize/4];
				float* buffer = new float[nSize/4];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return false;
				}

				wxUint8 *pointbytes = (wxUint8*)buffer;
				wxUint8 temp;
				for ( int i = 0 ; i < nSize; i +=4)
				{
					temp  = pointbytes[i];
					pointbytes[i] = pointbytes[i+3];
					pointbytes[i+3] = temp;
					temp  = pointbytes[i+1];
					pointbytes[i+1] = pointbytes[i+2];
					pointbytes[i+2] = temp;
				}

				int offset = m_columns * m_rows;
				int startslize = 0;

				for (int i = 0 ; i < m_frames ; ++i)
				{
					startslize = i * offset * 3;
					for (int j = 0 ; j < offset ; ++j)
					{
						m_floatDataset[startslize + 3 * j] = wxMax( buffer[startslize + j], buffer[startslize + j] * -1.0 );
						m_floatDataset[startslize + 3 * j + 1] = wxMax ( buffer[startslize + offset + j], buffer[startslize + offset + j] * -1.0 );
						m_floatDataset[startslize + 3 * j + 2] = wxMax ( buffer[startslize + 2*offset + j], buffer[startslize + 2*offset + j]* -1.0 );
					}
				}

				flag = true;
				m_dh->vectors_loaded = true;
				m_dh->surface_isDirty = true;
			} break;


			case RGB: {
				wxUint8 *buffer = new wxUint8[nSize];
				m_rgbDataset = new wxUint8[nSize];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return false;
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
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
	case Vectors_:
		glTexImage3D(GL_TEXTURE_3D,
			0,
			GL_RGBA,
			m_columns,
			m_rows,
			m_frames,
			0,
			GL_RGB,
			GL_FLOAT,
			m_floatDataset);
		break;
	default:
		break;
	}
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

float* Anatomy::getFloatDataset()
{
	if (m_type == Vectors_)
	{
		return m_floatDataset;
	}
	else
		return NULL;
}
