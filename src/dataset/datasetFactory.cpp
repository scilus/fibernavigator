#include "datasetFactory.h"
#include "DatasetHelper.h"
#include "datasetInfo.h"

DatasetFactory::DatasetFactory(DatasetHelper* datasetHelper)
{
    m_dh = datasetHelper;
}

DatasetInfo* DatasetFactory::loadNifti( wxString i_fileName )
{
 /*   char* l_hdrFile;
    l_hdrFile = (char*)malloc( i_fileName.length() + 1 );
    strcpy( l_hdrFile, (const char*)i_fileName.mb_str( wxConvUTF8 ) );

    nifti_image* l_image = nifti_image_read( l_hdrFile, 0 );

    if( ! l_image )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "nifti file corrupt, cannot create nifti image from header" );
        return false;
    }

    if (m_datasetHelper.m_columns != l_image->dim[1]; //80
    m_datasetHelper.m_rows    = l_image->dim[2]; //1
    m_datasetHelper.m_frames  = l_image->dim[3]; //72
    m_bands                   = l_image->dim[4];

    m_datasetHelper.m_xVoxel = l_image->dx;
    m_datasetHelper.m_yVoxel = l_image->dy;
    m_datasetHelper.m_zVoxel = l_image->dz;

    m_type = ODFS;

    // Order has to be between 0 and 16 (0, 2, 4, 6, 8, 10, 12, 14, 16)
    if( l_image->datatype != 16  || !( m_bands == 0   || m_bands == 15 || m_bands == 28 || 
        m_bands == 45  || m_bands == 66 || m_bands == 91 || 
        m_bands == 120 || m_bands == 153 ) )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "not a valid ODFs file format" );
        return false;
    }    

    nifti_image* l_fileData = nifti_image_read( l_hdrFile, 1 );
    if( ! l_fileData )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "nifti file corrupt" );
        return false;
    }

    int l_nSize = l_image->dim[1] * l_image->dim[2] * l_image->dim[3];

    float* l_data = (float*)l_fileData->data;

    std::vector< float > l_fileFloatData;
    l_fileFloatData.resize( l_nSize * m_bands );

    // We need to do a bit of moving around with the data in order to have it like we want.
    for( int i = 0; i < l_nSize; ++i )
        for( int j = 0; j < m_bands; ++j )
            l_fileFloatData[i * m_bands + j] = l_data[(j * l_nSize) + i];


    // Once the file has been read succesfully, we need to create the structure 
    // that will contain all the sphere points representing the ODFs.
    createStructure( l_fileFloatData );

    m_isLoaded = true;*/

    return NULL;



}