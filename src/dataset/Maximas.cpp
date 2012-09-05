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

///////////////////////////////////////////
Maximas::Maximas( const wxString &filename )
: Glyph()
{
    m_fullPath = filename;


#ifdef __WXMSW__
    m_name = filename.AfterLast( '\\' );
#else
    m_name = filename.AfterLast( '/' );
#endif

}

//////////////////////////////////////////////////////////////////////////
Maximas::~Maximas()
{
    Logger::getInstance()->print( wxT( "Maximas destructor called but nothing to do." ), LOGLEVEL_DEBUG );
}

//////////////////////////////////////////////////////////////////////////
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
    std::vector< float > l_fileFloatData( datasetSize * m_bands );

    float* pData = (float*)pBody->data;

    for( int i( 0 ); i < datasetSize; ++i )
    {
        for( int j( 0 ); j < m_bands; ++j )
        {
            l_fileFloatData[i * m_bands + j] = pData[j * datasetSize + i];
        }
    }
    
    createStructure( l_fileFloatData );

    m_isLoaded = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////
bool Maximas::createStructure  ( std::vector< float > &i_fileFloatData )
{
    m_nbGlyphs         = DatasetManager::getInstance()->getColumns() * DatasetManager::getInstance()->getRows() * DatasetManager::getInstance()->getFrames();
    m_mainDirections.resize( m_nbGlyphs );

    vector< float >::iterator it;
    int i = 0;

    //Fetching the directions
    for( it = i_fileFloatData.begin(), i = 0; it != i_fileFloatData.end(); it += m_bands, ++i )
    { 
        m_mainDirections[i].insert( m_mainDirections[i].end(), it, it + m_bands );
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
void Maximas::draw()
{
    Glyph::draw();
}

//////////////////////////////////////////////////////////////////////////
void Maximas::drawGlyph( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis )
{
    if( ! boxInFrustum( Vector( ( i_xVoxel + 0.5f ) * m_voxelSizeX,
                                ( i_yVoxel + 0.5f ) * m_voxelSizeY,
                                ( i_zVoxel + 0.5f ) * m_voxelSizeZ ),
                        Vector( m_voxelSizeX * 0.5f,
                                m_voxelSizeY * 0.5f,
                                m_voxelSizeZ * 0.5f ) ) )
        return;

    // Get the current tensors index in the coeffs's buffer
    int  currentIdx = getGlyphIndex( i_zVoxel, i_yVoxel, i_xVoxel );   
 
    // Odf offset.
    GLfloat l_offset[3];
    getVoxelOffset( i_zVoxel, i_yVoxel, i_xVoxel, l_offset );

    for(unsigned int i =0; i < m_mainDirections[currentIdx].size(); i+=3)
    {
        if(m_mainDirections[currentIdx].size() != 0)
        {  
            GLfloat l_coloring[3];
            l_coloring[0] = m_mainDirections[currentIdx][i*3];
            l_coloring[1] = m_mainDirections[currentIdx][i*3+1];
            l_coloring[2] = m_mainDirections[currentIdx][i*3+2];
            glColor3f(l_coloring[0],l_coloring[1],l_coloring[2]);
            
            float halfScale = 3.0f / 5.0f;
            GLfloat stickPos[3];
            stickPos[0] = -halfScale*m_mainDirections[currentIdx][i*3] + l_offset[0];
            stickPos[1] = -halfScale*m_mainDirections[currentIdx][i*3+1] + l_offset[1];
            stickPos[2] = -halfScale*m_mainDirections[currentIdx][i*3+2] + l_offset[2];

            glBegin(GL_LINES);  
                glVertex3f(-stickPos[0],-stickPos[1],-stickPos[2]);
                glVertex3f(stickPos[0],stickPos[1],stickPos[2]);       
            glEnd();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
void Maximas::createPropertiesSizer( PropertiesWindow *pParent )
{
    DatasetInfo::createPropertiesSizer( pParent );
}

//////////////////////////////////////////////////////////////////////////
void Maximas::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();
}
