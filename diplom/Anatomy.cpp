/*
 * Anatomy.cpp
 *
 *  Created on: 07.07.2008
 *      Author: ralph
 */

#include "Anatomy.h"

#include "wx/textfile.h"
#include <GL/glew.h>

#include "nifti/nifti1_io.h"

#define MIN_HEADER_SIZE 348
#define NII_HEADER_SIZE 352

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
	m_GLuint = 0;
	m_roi = 0;
}

Anatomy::~Anatomy()
{
	delete[] m_floatDataset;
	m_dh->tensors_loaded = false;

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
	// test for nifti
	if (m_name.AfterLast('.') == _T("nii"))
	{
		//printf("detected nifti file\n");
		return loadNifti(filename);
	}

	else if (m_name.AfterLast('.') == _T("gz"))
	{
		//printf("checking for compressed nifti file\n");
		wxString tmpName = m_name.BeforeLast('.');
		if (tmpName.AfterLast('.') == _T("nii"))
		{
			//printf("found compressed nifti file\n");
			return loadNifti(filename);
		}
	}
	return false;


}
// TODO
bool Anatomy::loadNifti(wxString filename)
{
	char* hdr_file;
	hdr_file = (char*) malloc(filename.length()+1);
	strcpy(hdr_file, (const char*) filename.mb_str(wxConvUTF8));

	nifti_image* ima = nifti_image_read(hdr_file, 0);

	m_columns = ima->dim[1]; // 160
	m_rows = ima->dim[2]; // 200
	m_frames = ima->dim[3]; // 160

	/*
	printf("XYZT dimensions: %d %d %d %d\n", ima->dim[1], ima->dim[2], ima->dim[3], ima->dim[4]);
	printf("datatype: %d\n", ima->datatype);
	printf("byte order: %d\n", ima->byteorder);
	*/

	if (ima->datatype == 2)
	{
		if (ima->dim[4] == 1) {
			m_type = Head_byte;
		}
		else if (ima->dim[4] == 3) {
			m_type = RGB;
		}
		else m_type = TERROR;
	}
	else if (ima->datatype == 4) m_type = Head_short;

	else if (ima->datatype == 16)
	{
		if (ima->dim[4] == 3) {
			m_type = Vectors_;
		}
		/*
		else if (m_bands / m_frames == 6) {
			m_type = Tensors_;
		}
		*/
		else
			m_type = Overlay;
	}
	else m_type = TERROR;

	nifti_image* filedata = nifti_image_read(hdr_file, 1);
	int nSize = ima->dim[1] * ima->dim[2] * ima->dim[3];

	bool flag = false;

	switch (m_type)
	{
		case Head_byte: {
			unsigned char *data = (unsigned char*)filedata->data;

			m_floatDataset = new float[nSize];
			for ( int i = 0 ; i < nSize ; ++i)
			{
				m_floatDataset[i] = (float)data[i] / 255.0;
			}

			flag = true;
		} break;

		case Head_short: {
			short int *data = (short int*)filedata->data;

			int max = 0;
			for ( int i = 0 ; i < nSize ; ++i)
			{
				max = wxMax(max, data[i]);
			}
			printf("max: %d\n", max);

			m_floatDataset = new float[nSize];
			for ( int i = 0 ; i < nSize ; ++i)
			{
				m_floatDataset[i] = (float)data[i] / (float)max;
			}
			flag = true;

		} break;

		case Overlay: {
			m_floatDataset = (float*) filedata->data;
			flag = true;
		} break;

		case RGB: {
			unsigned char *data=(unsigned char*)filedata->data;

			m_floatDataset = new float[nSize*3];

			for (int i = 0 ; i < nSize ; ++i)
			{

				m_floatDataset[i * 3    ] = (float)data[i] / 255.0;
				m_floatDataset[i * 3 + 1] = (float)data[nSize + i  ] / 255.0;
				m_floatDataset[i * 3 + 2] = (float)data[(2 * nSize) + i] / 255.0;

			}

			flag = true;
		} break;

		case Vectors_: {
			float *data=(float*)filedata->data;

			m_floatDataset = new float[nSize*3];

			for (int i = 0 ; i < nSize ; ++i)
			{

				m_floatDataset[i * 3    ] = data[i];
				m_floatDataset[i * 3 + 1] = data[nSize + i  ];
				m_floatDataset[i * 3 + 2] = data[(2 * nSize) + i];

			}
			m_tensorField = new TensorField(m_dh, m_floatDataset, true);
            m_dh->tensors_loaded = true;
            m_dh->vectors_loaded = true;
            m_dh->surface_isDirty = true;

			flag = true;
		} break;

		default:
			break;
	}

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
	case Head_short:
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
			GL_FLOAT,
			m_floatDataset);
		break;
	case Vectors_: {
		int size = m_rows*m_columns*m_frames*3;
		float *tempData = new float[size];
		for ( int i = 0 ; i < size ; ++i )
			tempData[i] = wxMax(m_floatDataset[i], -m_floatDataset[i]);


		glTexImage3D(GL_TEXTURE_3D,
			0,
			GL_RGBA,
			m_columns,
			m_rows,
			m_frames,
			0,
			GL_RGB,
			GL_FLOAT,
			tempData);
		delete[] tempData;
		break;
	}
	default:
		break;
	}
}

GLuint Anatomy::getGLuint()
{
	if (!m_GLuint)
		generateTexture();
	return m_GLuint;
}

float* Anatomy::getFloatDataset()
{
	return m_floatDataset;
}
