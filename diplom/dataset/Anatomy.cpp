/*
 * Anatomy.cpp
 *
 *  Created on: 07.07.2008
 *      Author: ralph
 */

#include "Anatomy.h"
#include "fibers.h"

#include "wx/textfile.h"
#include <GL/glew.h>

#include "../gui/selectionBox.h"
#include "../misc/nifti/nifti1_io.h"

#define MIN_HEADER_SIZE 348
#define NII_HEADER_SIZE 352

Anatomy::Anatomy(DatasetHelper* dh)
{
    m_dh = dh;
    m_type = not_initialized;
    m_length = 0;
    m_bands = 0;
    m_frames = 0;
    m_rows = 0;
    m_columns = 0;
    m_repn = wxT("");
    is_loaded = false;
    m_highest_value = 1.0;
    m_threshold = 0.00f;
    m_alpha = 1.0f;
    m_show = true;
    m_showFS = true;
    m_useTex = true;
    m_GLuint = 0;
    m_roi = 0;
    m_oldMax = 1.0;
    m_newMax = 1.0;
    m_isGlyph = false;
}

Anatomy::Anatomy(DatasetHelper* dh, float* dataset)
{
    m_dh = dh;
    m_type = Head_byte;
    m_length = 0;
    m_bands = 0;
    m_frames = m_dh->frames;
    m_rows = m_dh->rows;
    m_columns = m_dh->columns;
    m_repn = wxT("");
    is_loaded = true;
    m_highest_value = 1.0;
    m_threshold = 0.00f;
    m_alpha = 1.0f;
    m_show = true;
    m_showFS = true;
    m_useTex = true;
    m_GLuint = 0;
    m_roi = 0;
    m_oldMax = 1.0;
    m_newMax = 1.0;

    createOffset(dataset);
}

Anatomy::~Anatomy()
{
    delete[] m_floatDataset;
    const GLuint* tex = &m_GLuint;
    glDeleteTextures(1, tex);
    if (m_roi)
        m_roi->m_sourceAnatomy = NULL;
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

    if (m_dh->anatomy_loaded)
    {
        if (m_rows != m_dh->rows || m_columns != m_dh->columns || m_frames
                != m_dh->frames)
        {
            m_dh->lastError = wxT("dimensions of loaded files must be the same");
            return false;
        }
    }
    else
    {
        m_dh->rows = m_rows;
        m_dh->columns = m_columns;
        m_dh->frames = m_frames;

        m_dh->anatomy_loaded = true;
    }

    m_dh->xVoxel = ima->dx;
    m_dh->yVoxel = ima->dy;
    m_dh->zVoxel = ima->dz;

    if (ima->datatype == 2)
    {
        if (ima->dim[4] == 1)
        {
            m_type = Head_byte;
        }
        else if (ima->dim[4] == 3)
        {
            m_type = RGB;
        }
        else
            m_type = TERROR;
    }
    else if (ima->datatype == 4)
        m_type = Head_short;

    else if (ima->datatype == 16)
    {
        if (ima->dim[4] == 3)
        {
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
    else
        m_type = TERROR;

    nifti_image* filedata = nifti_image_read(hdr_file, 1);
    int nSize = ima->dim[1] * ima->dim[2] * ima->dim[3];

    bool flag = false;

    switch (m_type)
    {
        case Head_byte:
        {
            unsigned char *data = (unsigned char*)filedata->data;

            m_floatDataset = new float[nSize];
            for (int i = 0; i < nSize; ++i)
            {
                m_floatDataset[i] = (float)data[i] / 255.0;
            }
            flag = true;
            delete[] data;
        }
            break;

        case Head_short:
        {
            short int *data = (short int*)filedata->data;
            int max = 0;
            std::vector<int> histo(65536, 0);
            for (int i = 0; i < nSize; ++i)
            {
                max = wxMax(max, data[i]);
                ++histo[data[i]];
            }
            int fivepercent = (int)(nSize * 0.001);
            int newMax = 65535;
            int adder = 0;
            for (int i = 65535; i > 0; --i)
            {
                adder += histo[i];
                newMax = i;
                if (adder > fivepercent)
                    break;
            }
            for (int i = 0; i < nSize; ++i)
            {
                if (data[i] > newMax)
                    data[i] = newMax;
            }

            m_floatDataset = new float[nSize];
            for (int i = 0; i < nSize; ++i)
            {
                m_floatDataset[i] = (float)data[i] / (float)newMax;
            }
            m_oldMax = max;
            m_newMax = newMax;
            flag = true;
            delete[] data;
        }
            break;

        case Overlay:
        {
            m_floatDataset = (float*) filedata->data;
            float max = 0.0;
            for (int i = 0; i < nSize; ++i)
            {
                if (m_floatDataset[i] > max)
                    max = m_floatDataset[i];
            }
            for (int i = 0; i < nSize; ++i)
            {
                m_floatDataset[i] = m_floatDataset[i]/max;
            }
            m_oldMax = max;
            m_newMax = 1.0;

            flag = true;
        }
            break;

        case RGB:
        {
            unsigned char *data=(unsigned char*)filedata->data;

            m_floatDataset = new float[nSize*3];

            for (int i = 0; i < nSize; ++i)
            {

                m_floatDataset[i * 3 ] = (float)data[i] / 255.0;
                m_floatDataset[i * 3 + 1] = (float)data[nSize + i ] / 255.0;
                m_floatDataset[i * 3 + 2] = (float)data[(2 * nSize) + i] / 255.0;

            }
            flag = true;
            delete[] data;
        }
            break;

        case Vectors_:
        {
            float *data=(float*)filedata->data;

            m_floatDataset = new float[nSize*3];

            for (int i = 0; i < nSize; ++i)
            {

                m_floatDataset[i * 3 ] = data[i];
                m_floatDataset[i * 3 + 1] = data[nSize + i ];
                m_floatDataset[i * 3 + 2] = data[(2 * nSize) + i];

            }
            m_tensorField = new TensorField(m_dh, m_floatDataset, true);
            m_dh->tensors_loaded = true;
            m_dh->vectors_loaded = true;
            m_dh->surface_isDirty = true;
            delete[] data;
            flag = true;
        }
            break;

        default:
            break;
    }
    delete [] hdr_file;
    delete ima;
    delete filedata;

    is_loaded = flag;

    return flag;

}

void Anatomy::saveNifti(wxString filename)
{
    int dims [] =
    { 3, m_columns, m_rows, m_frames, 1, 0, 0, 0 };
    nifti_image* ima = nifti_make_new_nim(dims, DT_FLOAT32, 1);
    char fn[1024];
    strcpy(fn, (const char*)filename.mb_str(wxConvUTF8));
    ima->fname = fn;
    ima->data = m_floatDataset;
    nifti_image_write(ima);
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
        case Vectors_:
        {
            int size = m_rows*m_columns*m_frames*3;
            float *tempData = new float[size];
            for (int i = 0; i < size; ++i)
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

void Anatomy::createOffset(float* source)
{
    int b, r, c, bb, rr, r0, b0, c0;
    int i, istart, iend;
    int nbands, nrows, ncols, npixels;
    int d, d1, d2, cc1, cc2;
    float u, dmin, dmax;
    bool *srcpix;
    double g, *array;

    nbands = m_frames;
    nrows = m_rows;
    ncols = m_columns;

    npixels = wxMax(nbands,nrows);
    array = new double [npixels];

    npixels = nbands * nrows * ncols;

    m_floatDataset = new float[npixels];
    for (int i = 0; i < npixels; ++i)
    {
        m_floatDataset[i] = 0.0;
    }

    bool* bitmask = new bool[npixels];
    for (int i = 0; i < npixels; ++i)
    {
        if (source[i] < 0.01)
            bitmask[i] = true;
        else
            bitmask[i] = false;
    }

    dmax = 999999999.0;

    // first pass
    for (b=0; b<nbands; ++b)
    {
        for (r=0; r<nrows; ++r)
        {
            for (c=0; c<ncols; ++c)
            {
                //if (VPixel(src,b,r,c,VBit) == 1)
                if (bitmask[b*nrows*ncols + r*ncols + c])
                {
                    m_floatDataset[b*nrows*ncols + r*ncols + c] = 0;
                    continue;
                }

                srcpix = bitmask + b*nrows*ncols + r*ncols + c;
                cc1 = c;
                while (cc1 < ncols && *srcpix++ == 0)
                    cc1++;
                d1 = (cc1 >= ncols ? ncols : (cc1 - c));

                srcpix = bitmask + b*nrows*ncols + r*ncols + c;
                cc2 = c;
                while (cc2 >= 0 && *srcpix-- == 0)
                    cc2--;
                d2 = (cc2 <= 0 ? ncols : (c - cc2));

                if (d1 <= d2)
                {
                    d = d1;
                    c0 = cc1;
                }
                else
                {
                    d = d2;
                    c0 = cc2;
                }
                m_floatDataset[b*nrows*ncols + r*ncols + c] = (float) (d * d);
            }
        }
    }

    // second pass
    for (b=0; b<nbands; b++)
    {
        for (c=0; c<ncols; c++)
        {
            for (r=0; r<nrows; r++)
                array[r] = (double) m_floatDataset[b*nrows*ncols + r*ncols + c];

            for (r=0; r<nrows; r++)
            {
                if (bitmask[b*nrows*ncols + r*ncols + c] == 1)
                    continue;

                dmin = dmax;
                r0 = r;
                g = sqrt(array[r]);
                istart = r - (int) g;
                if (istart < 0)
                    istart = 0;
                iend = r + (int) g + 1;
                if (iend >= nrows)
                    iend = nrows;

                for (rr=istart; rr<iend; rr++)
                {
                    u = array[rr] + (r - rr) * (r - rr);
                    if (u < dmin)
                    {
                        dmin = u;
                        r0 = rr;
                    }
                }
                m_floatDataset[b*nrows*ncols + r*ncols + c] = dmin;
            }
        }
    }

    // third pass

    for (r=0; r<nrows; r++)
    {
        for (c=0; c<ncols; c++)
        {
            for (b=0; b<nbands; b++)
                array[b] = (double) m_floatDataset[b*nrows*ncols + r*ncols + c];

            for (b=0; b<nbands; b++)
            {
                if (bitmask[b*nrows*ncols + r*ncols + c] == 1)
                    continue;

                dmin = dmax;
                b0 = b;

                g = sqrt(array[b]);
                istart = b - (int) g - 1;
                if (istart < 0)
                    istart = 0;
                iend = b + (int) g + 1;
                if (iend >= nbands)
                    iend = nbands;

                for (bb=istart; bb<iend; bb++)
                {
                    u = array[bb] + (b - bb) * (b - bb);
                    if (u < dmin)
                    {
                        dmin = u;
                        b0 = bb;
                    }
                }
                m_floatDataset[b*nrows*ncols + r*ncols + c] = dmin;
            }
        }
    }

    delete[] array;

    float max = 0;
    for (i = 0; i < npixels; ++i)
    {
        m_floatDataset[i] = sqrt((double) m_floatDataset[i]);
        if (m_floatDataset[i] > max)
            max = m_floatDataset[i];
    }
    for (i = 0; i < npixels; ++i)
    {
        m_floatDataset[i] = m_floatDataset[i]/max;

    }

    // filter with gauss
    // create the filter kernel
    double sigma = 4;

    int dim = (int)(3.0 * sigma + 1);
    int n = 2*dim+1;
    double step = 1;

    float* kernel = new float[n];

    double sum = 0;
    double x = -(float)dim;

    double uu;
    for (int i = 0; i < n; ++i)
    {
        uu = xxgauss(x, sigma);
        sum += uu;
        kernel[i] = uu;
        x += step;
    }

    /* normalize */
    for (int i=0; i<n; ++i)
    {
        uu = kernel[i];
        uu /= sum;
        kernel[i] = uu;
    }

    d = n / 2;
    float* float_pp;
    float* tmp = new float[npixels];
    int c1, cc;

    for (int i = 0; i < npixels; ++i)
    {
        tmp[i] = 0.0;
    }

    for (b=0; b<nbands; ++b)
    {
        for (r=0; r<nrows; ++r)
        {
            for (c=d; c<ncols-d; ++c)
            {

                float_pp = kernel;
                sum = 0;
                c0 = c-d;
                c1 = c+d;
                for (cc=c0; cc<=c1; cc++)
                {
                    x = m_floatDataset[b*nrows*ncols + r*ncols + cc];
                    sum += x * (*float_pp++);
                }
                tmp[b*nrows*ncols + r*ncols + c] = sum;
            }
        }
    }
    int r1;
    for (b=0; b<nbands; ++b)
    {
        for (r=d; r<nrows-d; ++r)
        {
            for (c=0; c<ncols; ++c)
            {
                float_pp = kernel;
                sum = 0;
                r0 = r-d;
                r1 = r+d;
                for (rr=r0; rr<=r1; rr++)
                {
                    x = tmp[b*nrows*ncols + rr*ncols + c];
                    sum += x * (*float_pp++);
                }
                m_floatDataset[b*nrows*ncols + r*ncols + c] = sum;
            }
        }
    }
    int b1;
    for (b=d; b<nbands-d; ++b)
    {
        for (r=0; r<nrows; ++r)
        {
            for (c=0; c<ncols; ++c)
            {

                float_pp = kernel;
                sum = 0;
                b0 = b-d;
                b1 = b+d;
                for (bb=b0; bb<=b1; bb++)
                {
                    x = m_floatDataset[bb*nrows*ncols + r*ncols + c];
                    sum += x * (*float_pp++);
                }
                tmp[b*nrows*ncols + r*ncols + c] = sum;
            }
        }
    }

    m_floatDataset = tmp;
}

double Anatomy::xxgauss(double x, double sigma)
{
    double y, z, a=2.506628273;
    z = x / sigma;
    y = exp((double)-z*z*0.5)/(sigma * a);
    return y;
}

void Anatomy::setZero(int x, int y, int z)
{
    m_columns = x;
    m_rows = y;
    m_frames = z;

    int nSize = m_rows * m_columns * m_frames;
    if (m_type == not_initialized)
        m_floatDataset = new float[nSize];
    else
        delete[] m_floatDataset;
    m_floatDataset = new float[nSize];

    for (int i = 0; i < nSize; ++i)
    {
        m_floatDataset[i] = 0.0;
    }
}

void Anatomy::minimize()
{
    if ( !m_dh->fibers_loaded)
        return;
    std::vector<bool> tmp(m_columns * m_rows * m_frames, false);
    Fibers* fib= NULL;
    m_dh->getFiberDataset(fib);

    int x, y, z;

    for (int i = 0; i < fib->getLineCount() ; ++i)
    {
        if (fib->m_inBox[i] == 1)
        {
            for (int j = fib->getStartIndexForLine(i) ; j
                    < (fib->getStartIndexForLine(i) + (fib->getPointsPerLine(i)
                            *3)); ++j)
            {
                x = wxMin(m_dh->columns-1, wxMax(0, (int)fib->m_pointArray[j++]));
                y = wxMin(m_dh->rows -1, wxMax(0, (int)fib->m_pointArray[j++]));
                z = wxMin(m_dh->frames -1, wxMax(0, (int)fib->m_pointArray[j]));
                int index = x + y * m_dh->columns + z * m_dh->rows
                        * m_dh->columns;
                tmp[index] = true;
            }
        }
    }

    Anatomy* newAnatomy = new Anatomy(m_dh);
    newAnatomy->setZero(m_columns, m_rows, m_frames);

    float* dst = newAnatomy->getFloatDataset();

    for (int i = 0; i < m_columns * m_rows * m_frames; ++i)
    {
        if (tmp[i] && m_floatDataset[i] > 0)
            dst[i] = 1.0;
    }

    newAnatomy->setName(getName() + _T("(minimal)"));
    newAnatomy->setType(getType());

    m_dh->mainFrame->m_listCtrl->InsertItem(0, wxT(""), 0);
    m_dh->mainFrame->m_listCtrl->SetItem(0, 1, newAnatomy->getName());
    m_dh->mainFrame->m_listCtrl->SetItem(0, 2, wxT("0.00"));
    m_dh->mainFrame->m_listCtrl->SetItem(0, 3, wxT(""), 1);
    m_dh->mainFrame->m_listCtrl->SetItemData(0, (long) newAnatomy);
    m_dh->mainFrame->m_listCtrl->SetItemState(0, wxLIST_STATE_SELECTED,
            wxLIST_STATE_SELECTED);

}

void Anatomy::dilate()
{
    int nsize = m_columns*m_rows*m_frames;
    std::vector<bool> tmp(nsize, false);
    for (int c = 1; c < m_columns - 1; ++c)
    {
        for (int r = 1; r < m_rows - 1; ++r)
        {
            for (int f = 1; f < m_frames - 1; ++f)
            {
                int index = c + r * m_columns + f * m_columns * m_rows;
                if (m_floatDataset[index] == 1.0)
                    dilate1(&tmp, index);
            }
        }
    }
    for (int i = 0; i < nsize; ++i)
    {
        if (tmp[i])
            m_floatDataset[i] = 1.0;
    }
    const GLuint* tex = &m_GLuint;
    glDeleteTextures(1, tex);
    generateTexture();
    m_dh->mainFrame->m_mainGL->render();
}

void Anatomy::dilate1(std::vector<bool>* input, int index)
{
    input->at(index - 1) = true;
    input->at(index) = true;
    input->at(index + 1) = true;
    input->at(index - m_columns - 1) = true;
    input->at(index - m_columns) = true;
    input->at(index - m_columns + 1) = true;
    input->at(index + m_columns - 1) = true;
    input->at(index + m_columns) = true;
    input->at(index + m_columns + 1) = true;
    input->at(index - m_columns * m_rows - 1) = true;
    input->at(index - m_columns * m_rows) = true;
    input->at(index - m_columns * m_rows + 1) = true;
    input->at(index + m_columns * m_rows - 1) = true;
    input->at(index + m_columns * m_rows) = true;
    input->at(index + m_columns * m_rows + 1) = true;
    input->at(index - m_columns * m_rows - m_columns) = true;
    input->at(index - m_columns * m_rows + m_columns) = true;
    input->at(index + m_columns * m_rows - m_columns) = true;
    input->at(index + m_columns * m_rows + m_columns) = true;

}

void Anatomy::erode()
{
    int nsize = m_columns*m_rows*m_frames;
    std::vector<bool> tmp(nsize, false);
    for (int c = 1; c < m_columns - 1; ++c)
    {
        for (int r = 1; r < m_rows - 1; ++r)
        {
            for (int f = 1; f < m_frames - 1; ++f)
            {
                int index = c + r * m_columns + f * m_columns * m_rows;
                if (m_floatDataset[index] == 1.0)
                    erode1(&tmp, index);
            }
        }
    }
    for (int i = 0; i < nsize; ++i)
    {
        if (!tmp[i])
            m_floatDataset[i] = 0.0;
    }
    const GLuint* tex = &m_GLuint;
    glDeleteTextures(1, tex);
    generateTexture();
    m_dh->mainFrame->m_mainGL->render();
}

void Anatomy::erode1(std::vector<bool>* tmp, int index)
{
    float test = m_floatDataset[index - 1] + m_floatDataset[index]
            + m_floatDataset[index + 1] + m_floatDataset[index - m_columns - 1]
            + m_floatDataset[index - m_columns]
            + m_floatDataset[index - m_columns + 1]
            + m_floatDataset[index + m_columns - 1]
            + m_floatDataset[index + m_columns]
            + m_floatDataset[index + m_columns + 1]
            + m_floatDataset[index - m_columns * m_rows - 1]
            + m_floatDataset[index - m_columns * m_rows]
            + m_floatDataset[index - m_columns * m_rows + 1]
            + m_floatDataset[index + m_columns * m_rows - 1]
            + m_floatDataset[index + m_columns * m_rows]
            + m_floatDataset[index + m_columns * m_rows + 1]
            + m_floatDataset[index - m_columns * m_rows - m_columns]
            + m_floatDataset[index - m_columns * m_rows + m_columns]
            + m_floatDataset[index + m_columns * m_rows - m_columns]
            + m_floatDataset[index + m_columns * m_rows + m_columns];
    if (test == 19.0)
        tmp->at(index) = 1.0;
}
