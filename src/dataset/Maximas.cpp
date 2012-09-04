/////////////////////////////////////////////////////////////////////////////
// Name:            Maximas.cpp
// Author:          Maxime Chamberland
/////////////////////////////////////////////////////////////////////////////

#include "Maximas.h"

#include "DatasetManager.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gui/MyListCtrl.h"
#include "../gui/SceneManager.h"
#include "../misc/nifti/nifti1_io.h"

#include <GL/glew.h>
#include <wx/math.h>
#include <wx/xml/xml.h>

#include <algorithm>


#include <fstream>


#include <vector>
using std::vector;





Maximas::Maximas( const wxString &filename )
: DatasetInfo()
{

}

Maximas::~Maximas()
{

}

bool Maximas::load( nifti_image *pHeader, nifti_image *pBody )
{
    m_columns = pHeader->dim[1]; //XSlice
    m_rows    = pHeader->dim[2]; //YSlice
    m_frames  = pHeader->dim[3]; //ZSlice
    m_bands   = pHeader->dim[4]; //9

    m_voxelSizeX = pHeader->dx;
    m_voxelSizeY = pHeader->dy;
    m_voxelSizeZ = pHeader->dz;

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    if( m_voxelSizeX != voxelX || m_voxelSizeY != voxelY || m_voxelSizeZ != voxelZ )
    {
        Logger::getInstance()->print( wxT( "Voxel size different from anatomy." ), LOGLEVEL_ERROR );
        return false;
    }

    m_type = MAXIMAS;

    int datasetSize = pHeader->dim[1] * pHeader->dim[2] * pHeader->dim[3];

    float* pData = (float*)pBody->data;

    for( int i(0); i < datasetSize; ++i )
    {
        m_mainDirections[i * 3]       = pData[i];
        m_mainDirections[i * 3 + 1]   = pData[datasetSize + i];
        m_mainDirections[i * 3 + 2]   = pData[2 * datasetSize + i];
    }
    
    m_isLoaded = true;
    return true;
}


void Maximas::createPropertiesSizer( PropertiesWindow *pParent )
{
    
}

void Maximas::updatePropertiesSizer()
{

}


