/*
 * ShaderHelper.cpp
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#include "ShaderHelper.h"

#include "ShaderProgram.h"

#include "../dataset/DatasetHelper.h"
#include "../dataset/Surface.h"

#include "../Logger.h"

ShaderHelper::ShaderHelper( DatasetHelper* pDh, bool geometryShadersSupported ) :
    m_anatomyShader( wxT( "anatomy" )/*, geometryShadersSupported ? true : false*/ ),
    m_meshShader( wxT( "mesh" ) ),
    m_fibersShader( wxT( "fibers" ) ),
    m_fakeTubesShader( wxT( "fake-tubes") ),
    m_crossingFibersShader( wxT( "crossing_fibers" ), true, geometryShadersSupported ),
    m_splineSurfShader( wxT( "splineSurf" ) ),
    m_vectorShader( wxT( "vectors" ) ),
    m_legendShader( wxT( "legend" ) ),
    m_graphShader( wxT( "graph" ) ),
    m_tensorsShader( wxT( "tensors" ) ),
    m_odfsShader( wxT( "odfs" ) ),
    m_pDh( pDh )
{
    m_tex.resize( 10 );
    m_type.resize( 10 );
    m_threshold.resize( 10 );
    m_alpha.resize( 10 );

    Logger::getInstance()->printDebug( _T( "Initializing anatomy shader..." ), LOGLEVEL_MESSAGE );
    if( m_anatomyShader.load() && m_anatomyShader.compileAndLink() )
    {
        m_anatomyShader.bind();
        Logger::getInstance()->printDebug( _T( "Anatomy shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize anatomy shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing mesh shader..." ), LOGLEVEL_MESSAGE );
    if( m_meshShader.load() && m_meshShader.compileAndLink() )
    {
        m_meshShader.bind();
        Logger::getInstance()->printDebug( _T( "Mesh shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize mesh shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing fibers shader..." ), LOGLEVEL_MESSAGE );
    if( m_fibersShader.load() && m_fibersShader.compileAndLink() )
    {
        m_fibersShader.bind();
        Logger::getInstance()->printDebug( _T( "Fibers shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize fibers shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing fake tubes shader..." ), LOGLEVEL_MESSAGE );
    if( m_fakeTubesShader.load() && m_fakeTubesShader.compileAndLink() )
    {
        m_fakeTubesShader.bind();
        Logger::getInstance()->printDebug( _T( "Fake Tubes shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize fake Tubes shader." ), LOGLEVEL_ERROR );
    }

    if ( geometryShadersSupported )
    {
        Logger::getInstance()->printDebug( _T( "Initializing crossing fibers shader..." ), LOGLEVEL_MESSAGE );
        
		glProgramParameteriEXT( m_crossingFibersShader.getId(), GL_GEOMETRY_INPUT_TYPE_EXT, GL_LINES );
        glProgramParameteriEXT( m_crossingFibersShader.getId(), GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_LINE_STRIP );
        glProgramParameteriEXT( m_crossingFibersShader.getId(), GL_GEOMETRY_VERTICES_OUT_EXT, 50 );
        
		if( m_crossingFibersShader.load() && m_crossingFibersShader.compileAndLink() )
        {
            m_crossingFibersShader.bind();
            Logger::getInstance()->printDebug( _T( "Crossing fibers shader initialized." ), LOGLEVEL_MESSAGE );
        }
        else
        {
            Logger::getInstance()->printDebug( _T( "Could not initialize crossing fibers shader." ), LOGLEVEL_ERROR );
        }
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Geometry shaders are not supported. Cannot load crossing fibers shader." ), LOGLEVEL_WARNING );
    }

    Logger::getInstance()->printDebug( _T( "Initializing spline surf shader..." ), LOGLEVEL_MESSAGE );
    if( m_splineSurfShader.load() && m_splineSurfShader.compileAndLink() )
    {
        m_splineSurfShader.bind();
        Logger::getInstance()->printDebug( _T( "Spline surf shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize spline surf shader." ), LOGLEVEL_ERROR );
    }

    //Logger::getInstance()->printDebug( _T( "Initializing vector shader..." ), LOGLEVEL_MESSAGE );
    //if( m_vectorShader.load() && m_vectorShader.compileAndLink() )
    //{
    //    m_vectorShader.bind();
    //    Logger::getInstance()->printDebug( _T( "Vector shader initialized." ), LOGLEVEL_MESSAGE );
    //}
    //else
    //{
    //    Logger::getInstance()->printDebug( _T( "Could not initialize vector shader." ), LOGLEVEL_ERROR );
    //}

    Logger::getInstance()->printDebug( _T( "Initializing legend shader..." ), LOGLEVEL_MESSAGE );
    if( m_legendShader.load() && m_legendShader.compileAndLink() )
    {
        m_legendShader.bind();
        Logger::getInstance()->printDebug( _T( "Legend shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize legend shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing graph shader..." ), LOGLEVEL_MESSAGE );
    if( m_graphShader.load() && m_graphShader.compileAndLink() )
    {
        m_graphShader.bind();
        Logger::getInstance()->printDebug( _T( "Graph shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize graph shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing tensors shader..." ), LOGLEVEL_MESSAGE );
    if( m_tensorsShader.load() && m_tensorsShader.compileAndLink() )
    {
        m_tensorsShader.bind();
        Logger::getInstance()->printDebug( _T( "Tensors shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize tensors shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing odfs shader..." ), LOGLEVEL_MESSAGE );
    if( m_odfsShader.load() && m_odfsShader.compileAndLink() )
    {
        m_odfsShader.bind();
        Logger::getInstance()->printDebug( _T( "Odfs shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize odfs shader." ), LOGLEVEL_ERROR );
    }

}

ShaderHelper::~ShaderHelper()
{
}

void ShaderHelper::initializeArrays()
{
    m_textureCount = 0;
    m_cutTex = 0;

    m_type.resize( 10, 0 );
    m_threshold.resize( 10, 0 );
    m_alpha.resize( 10, 0 );

    for ( int i = 0; i < m_pDh->m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = (DatasetInfo*) m_pDh->m_mainFrame->m_pListCtrl->GetItemData( i );
        if ( pInfo->getType() < MESH && pInfo->getShow() )
        {
            m_threshold[m_textureCount] = pInfo->getThreshold();
            m_alpha[m_textureCount] = pInfo->getAlpha();
            m_type[m_textureCount] = pInfo->getType();
            ++m_textureCount;
        }
        if ( m_textureCount == 10 )
            break;
    }
}

void ShaderHelper::setTextureShaderVars()
{
    m_anatomyShader.setUniInt( "tex0", 0 );
    m_anatomyShader.setUniInt( "tex1", 1 );
    m_anatomyShader.setUniInt( "tex2", 2 );
    m_anatomyShader.setUniInt( "tex3", 3 );
    m_anatomyShader.setUniInt( "tex4", 4 );
    m_anatomyShader.setUniInt( "tex5", 5 );
    m_anatomyShader.setUniInt( "tex6", 6 );
    m_anatomyShader.setUniInt( "tex7", 7 );
    m_anatomyShader.setUniInt( "tex8", 8 );
    m_anatomyShader.setUniInt( "tex9", 9 );

    m_anatomyShader.setUniInt( "type0", m_type[0] );
    m_anatomyShader.setUniInt( "type1", m_type[1] );
    m_anatomyShader.setUniInt( "type2", m_type[2] );
    m_anatomyShader.setUniInt( "type3", m_type[3] );
    m_anatomyShader.setUniInt( "type4", m_type[4] );
    m_anatomyShader.setUniInt( "type5", m_type[5] );
    m_anatomyShader.setUniInt( "type6", m_type[6] );
    m_anatomyShader.setUniInt( "type7", m_type[7] );
    m_anatomyShader.setUniInt( "type8", m_type[8] );
    m_anatomyShader.setUniInt( "type9", m_type[9] );

    m_anatomyShader.setUniFloat( "threshold0", m_threshold[0] );
    m_anatomyShader.setUniFloat( "threshold1", m_threshold[1] );
    m_anatomyShader.setUniFloat( "threshold2", m_threshold[2] );
    m_anatomyShader.setUniFloat( "threshold3", m_threshold[3] );
    m_anatomyShader.setUniFloat( "threshold4", m_threshold[4] );
    m_anatomyShader.setUniFloat( "threshold5", m_threshold[5] );
    m_anatomyShader.setUniFloat( "threshold6", m_threshold[6] );
    m_anatomyShader.setUniFloat( "threshold7", m_threshold[7] );
    m_anatomyShader.setUniFloat( "threshold8", m_threshold[8] );
    m_anatomyShader.setUniFloat( "threshold9", m_threshold[9] );

    m_anatomyShader.setUniFloat( "alpha0", m_alpha[0] );
    m_anatomyShader.setUniFloat( "alpha1", m_alpha[1] );
    m_anatomyShader.setUniFloat( "alpha2", m_alpha[2] );
    m_anatomyShader.setUniFloat( "alpha3", m_alpha[3] );
    m_anatomyShader.setUniFloat( "alpha4", m_alpha[4] );
    m_anatomyShader.setUniFloat( "alpha5", m_alpha[5] );
    m_anatomyShader.setUniFloat( "alpha6", m_alpha[6] );
    m_anatomyShader.setUniFloat( "alpha7", m_alpha[7] );
    m_anatomyShader.setUniFloat( "alpha8", m_alpha[8] );
    m_anatomyShader.setUniFloat( "alpha9", m_alpha[9] );
}

void ShaderHelper::setMeshShaderVars()
{
    m_meshShader.setUniInt( "blendTex", m_pDh->m_blendTexOnMesh );

    m_meshShader.setUniInt( "cutAtSurface", m_pDh->m_surfaceLoaded );
    m_meshShader.setUniInt( "lightOn", m_pDh->m_lighting );

    m_meshShader.setUniInt( "dimX", m_pDh->m_columns );
    m_meshShader.setUniInt( "dimY", m_pDh->m_rows );
    m_meshShader.setUniInt( "dimZ", m_pDh->m_frames );
    m_meshShader.setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_meshShader.setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_meshShader.setUniFloat( "voxZ", m_pDh->m_zVoxel );

    m_meshShader.setUniInt( "sector", m_pDh->m_quadrant );
    m_meshShader.setUniFloat( "cutX", m_pDh->m_xSlize + 0.5f );
    m_meshShader.setUniFloat( "cutY", m_pDh->m_ySlize + 0.5f );
    m_meshShader.setUniFloat( "cutZ", m_pDh->m_zSlize + 0.5f );


    for ( int i = 0; i < m_pDh->m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = (DatasetInfo*) m_pDh->m_mainFrame->m_pListCtrl->GetItemData( i );

        if ( pInfo->getType() == SURFACE )
        {
            Surface* pS = (Surface*) m_pDh->m_mainFrame->m_pListCtrl->GetItemData( i );
            m_cutTex = pS->getCutTex();

            glActiveTexture( GL_TEXTURE0 + 9 );
            glBindTexture( GL_TEXTURE_2D, m_cutTex );
            m_tex[9] = 9;
            m_threshold[9] = 0;
            m_type[9] = 5;
        }
    }
    m_meshShader.setUniInt( "cutTex", 9 );

    m_meshShader.setUniInt( "tex0", 0 );
    m_meshShader.setUniInt( "tex1", 1 );
    m_meshShader.setUniInt( "tex2", 2 );
    m_meshShader.setUniInt( "tex3", 3 );
    m_meshShader.setUniInt( "tex4", 4 );
    m_meshShader.setUniInt( "tex5", 5 );

    m_meshShader.setUniInt( "type0", m_type[0] );
    m_meshShader.setUniInt( "type1", m_type[1] );
    m_meshShader.setUniInt( "type2", m_type[2] );
    m_meshShader.setUniInt( "type3", m_type[3] );
    m_meshShader.setUniInt( "type4", m_type[4] );
    m_meshShader.setUniInt( "type5", m_type[5] );

    m_meshShader.setUniFloat( "threshold0", m_threshold[0] );
    m_meshShader.setUniFloat( "threshold1", m_threshold[1] );
    m_meshShader.setUniFloat( "threshold2", m_threshold[2] );
    m_meshShader.setUniFloat( "threshold3", m_threshold[3] );
    m_meshShader.setUniFloat( "threshold4", m_threshold[4] );
    m_meshShader.setUniFloat( "threshold5", m_threshold[5] );

    m_meshShader.setUniFloat( "alpha0", m_alpha[0] );
    m_meshShader.setUniFloat( "alpha1", m_alpha[1] );
    m_meshShader.setUniFloat( "alpha2", m_alpha[2] );
    m_meshShader.setUniFloat( "alpha3", m_alpha[3] );
    m_meshShader.setUniFloat( "alpha4", m_alpha[4] );
    m_meshShader.setUniFloat( "alpha5", m_alpha[5] );
}

void ShaderHelper::setFiberShaderVars()
{
    m_fibersShader.setUniInt( "dimX", m_pDh->m_columns );
    m_fibersShader.setUniInt( "dimY", m_pDh->m_rows );
    m_fibersShader.setUniInt( "dimZ", m_pDh->m_frames );
    m_fibersShader.setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_fibersShader.setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_fibersShader.setUniFloat( "voxZ", m_pDh->m_zVoxel );

    int tex = 0;
    int show = 0;
    float threshold = 0;
    int type = 0;

    int c = 0;
    for ( int i = 0; i < m_pDh->m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = (DatasetInfo*) m_pDh->m_mainFrame->m_pListCtrl->GetItemData( i );
        if ( pInfo->getType() < MESH )
        {
            if ( ( pInfo->getType() == OVERLAY ) && pInfo->getShow() )
            {
                tex = c;
                show = pInfo->getShow();
                threshold = pInfo->getThreshold();
                type = pInfo->getType();
                break;
            }
            ++c;
        }
        if ( c == 10 )
            break;
    }

    m_fibersShader.setUniInt( "tex", tex );
    m_fibersShader.setUniInt( "type", type );
    m_fibersShader.setUniFloat( "threshold", threshold );
}

void ShaderHelper::setSplineSurfaceShaderVars()
{
    m_splineSurfShader.setUniInt( "dimX", m_pDh->m_columns );
    m_splineSurfShader.setUniInt( "dimY", m_pDh->m_rows );
    m_splineSurfShader.setUniInt( "dimZ", m_pDh->m_frames );
    m_splineSurfShader.setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_splineSurfShader.setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_splineSurfShader.setUniFloat( "voxZ", m_pDh->m_zVoxel );

    m_splineSurfShader.setUniInt( "tex0", 0 );
    m_splineSurfShader.setUniInt( "tex1", 1 );
    m_splineSurfShader.setUniInt( "tex2", 2 );
    m_splineSurfShader.setUniInt( "tex3", 3 );
    m_splineSurfShader.setUniInt( "tex4", 4 );
    m_splineSurfShader.setUniInt( "tex5", 5 );

    m_splineSurfShader.setUniInt( "type0", m_type[0] );
    m_splineSurfShader.setUniInt( "type1", m_type[1] );
    m_splineSurfShader.setUniInt( "type2", m_type[2] );
    m_splineSurfShader.setUniInt( "type3", m_type[3] );
    m_splineSurfShader.setUniInt( "type4", m_type[4] );
    m_splineSurfShader.setUniInt( "type5", m_type[5] );

    m_splineSurfShader.setUniFloat( "threshold0", m_threshold[0] );
    m_splineSurfShader.setUniFloat( "threshold1", m_threshold[1] );
    m_splineSurfShader.setUniFloat( "threshold2", m_threshold[2] );
    m_splineSurfShader.setUniFloat( "threshold3", m_threshold[3] );
    m_splineSurfShader.setUniFloat( "threshold4", m_threshold[4] );
    m_splineSurfShader.setUniFloat( "threshold5", m_threshold[5] );

    m_splineSurfShader.setUniFloat( "alpha0", m_alpha[0] );
    m_splineSurfShader.setUniFloat( "alpha1", m_alpha[1] );
    m_splineSurfShader.setUniFloat( "alpha2", m_alpha[2] );
    m_splineSurfShader.setUniFloat( "alpha3", m_alpha[3] );
    m_splineSurfShader.setUniFloat( "alpha4", m_alpha[4] );
    m_splineSurfShader.setUniFloat( "alpha5", m_alpha[5] );
}

