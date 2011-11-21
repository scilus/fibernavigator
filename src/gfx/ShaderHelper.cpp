/*
 * ShaderHelper.cpp
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#include "ShaderHelper.h"

#include "Program.h"

#include "../dataset/DatasetHelper.h"
#include "../dataset/Surface.h"

#include "../Logger.h"

ShaderHelper::ShaderHelper( DatasetHelper* pDh ) :
    m_pDh( pDh ),
    m_anatomy( wxT( "anatomy" ) ),
    m_mesh( wxT( "mesh" ) ),
    m_fibers( wxT( "fibers" ) ),
    m_fakeTubes( wxT( "fake-tubes") ),
    m_splineSurf( wxT( "splineSurf" ) ),
    m_vector( wxT( "vectors" ) ),
    m_legend( wxT( "legend" ) ),
    m_graph( wxT( "graph" ) ),
    m_tensors( wxT( "tensors" ) ),
    m_odfs( wxT( "odfs" ) )
{
    m_tex.resize( 10 );
    m_type.resize( 10 );
    m_threshold.resize( 10 );
    m_alpha.resize( 10 );

    Logger::getInstance()->printDebug( _T( "Initializing anatomy shader..." ), LOGLEVEL_MESSAGE );
    if( m_anatomy.load() && m_anatomy.compileAndLink() )
    {
        m_anatomy.bind();
        Logger::getInstance()->printDebug( _T( "Anatomy shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize anatomy shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing mesh shader..." ), LOGLEVEL_MESSAGE );
    if( m_mesh.load() && m_mesh.compileAndLink() )
    {
        m_mesh.bind();
        Logger::getInstance()->printDebug( _T( "Mesh shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize mesh shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing fibers shader..." ), LOGLEVEL_MESSAGE );
    if( m_fibers.load() && m_fibers.compileAndLink() )
    {
        m_fibers.bind();
        Logger::getInstance()->printDebug( _T( "Fibers shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize fibers shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing fake tubes shader..." ), LOGLEVEL_MESSAGE );
    if( m_fakeTubes.load() && m_fakeTubes.compileAndLink() )
    {
        m_fakeTubes.bind();
        Logger::getInstance()->printDebug( _T( "Fake Tubes shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize fake Tubes shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing spline surf shader..." ), LOGLEVEL_MESSAGE );
    if( m_splineSurf.load() && m_splineSurf.compileAndLink() )
    {
        m_splineSurf.bind();
        Logger::getInstance()->printDebug( _T( "Spline surf shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize spline surf shader." ), LOGLEVEL_ERROR );
    }

    //Logger::getInstance()->printDebug( _T( "Initializing vector shader..." ), LOGLEVEL_MESSAGE );
    //if( m_vector.load() && m_vector.compileAndLink() )
    //{
    //    m_vector.bind();
    //    Logger::getInstance()->printDebug( _T( "Vector shader initialized." ), LOGLEVEL_MESSAGE );
    //}
    //else
    //{
    //    Logger::getInstance()->printDebug( _T( "Could not initialize vector shader." ), LOGLEVEL_ERROR );
    //}

    Logger::getInstance()->printDebug( _T( "Initializing legend shader..." ), LOGLEVEL_MESSAGE );
    if( m_legend.load() && m_legend.compileAndLink() )
    {
        m_legend.bind();
        Logger::getInstance()->printDebug( _T( "Legend shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize legend shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing graph shader..." ), LOGLEVEL_MESSAGE );
    if( m_graph.load() && m_graph.compileAndLink() )
    {
        m_graph.bind();
        Logger::getInstance()->printDebug( _T( "Graph shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize graph shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing tensors shader..." ), LOGLEVEL_MESSAGE );
    if( m_tensors.load() && m_tensors.compileAndLink() )
    {
        m_tensors.bind();
        Logger::getInstance()->printDebug( _T( "Tensors shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->printDebug( _T( "Could not initialize tensors shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->printDebug( _T( "Initializing odfs shader..." ), LOGLEVEL_MESSAGE );
    if( m_odfs.load() && m_odfs.compileAndLink() )
    {
        m_odfs.bind();
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
    m_anatomy.setUniInt( "tex0", 0 );
    m_anatomy.setUniInt( "tex1", 1 );
    m_anatomy.setUniInt( "tex2", 2 );
    m_anatomy.setUniInt( "tex3", 3 );
    m_anatomy.setUniInt( "tex4", 4 );
    m_anatomy.setUniInt( "tex5", 5 );
    m_anatomy.setUniInt( "tex6", 6 );
    m_anatomy.setUniInt( "tex7", 7 );
    m_anatomy.setUniInt( "tex8", 8 );
    m_anatomy.setUniInt( "tex9", 9 );

    m_anatomy.setUniInt( "type0", m_type[0] );
    m_anatomy.setUniInt( "type1", m_type[1] );
    m_anatomy.setUniInt( "type2", m_type[2] );
    m_anatomy.setUniInt( "type3", m_type[3] );
    m_anatomy.setUniInt( "type4", m_type[4] );
    m_anatomy.setUniInt( "type5", m_type[5] );
    m_anatomy.setUniInt( "type6", m_type[6] );
    m_anatomy.setUniInt( "type7", m_type[7] );
    m_anatomy.setUniInt( "type8", m_type[8] );
    m_anatomy.setUniInt( "type9", m_type[9] );

    m_anatomy.setUniFloat( "threshold0", m_threshold[0] );
    m_anatomy.setUniFloat( "threshold1", m_threshold[1] );
    m_anatomy.setUniFloat( "threshold2", m_threshold[2] );
    m_anatomy.setUniFloat( "threshold3", m_threshold[3] );
    m_anatomy.setUniFloat( "threshold4", m_threshold[4] );
    m_anatomy.setUniFloat( "threshold5", m_threshold[5] );
    m_anatomy.setUniFloat( "threshold6", m_threshold[6] );
    m_anatomy.setUniFloat( "threshold7", m_threshold[7] );
    m_anatomy.setUniFloat( "threshold8", m_threshold[8] );
    m_anatomy.setUniFloat( "threshold9", m_threshold[9] );

    m_anatomy.setUniFloat( "alpha0", m_alpha[0] );
    m_anatomy.setUniFloat( "alpha1", m_alpha[1] );
    m_anatomy.setUniFloat( "alpha2", m_alpha[2] );
    m_anatomy.setUniFloat( "alpha3", m_alpha[3] );
    m_anatomy.setUniFloat( "alpha4", m_alpha[4] );
    m_anatomy.setUniFloat( "alpha5", m_alpha[5] );
    m_anatomy.setUniFloat( "alpha6", m_alpha[6] );
    m_anatomy.setUniFloat( "alpha7", m_alpha[7] );
    m_anatomy.setUniFloat( "alpha8", m_alpha[8] );
    m_anatomy.setUniFloat( "alpha9", m_alpha[9] );
}

void ShaderHelper::setMeshShaderVars()
{
    m_mesh.setUniInt( "blendTex", m_pDh->m_blendTexOnMesh );

    m_mesh.setUniInt( "cutAtSurface", m_pDh->m_surfaceLoaded );
    m_mesh.setUniInt( "lightOn", m_pDh->m_lighting );

    m_mesh.setUniInt( "dimX", m_pDh->m_columns );
    m_mesh.setUniInt( "dimY", m_pDh->m_rows );
    m_mesh.setUniInt( "dimZ", m_pDh->m_frames );
    m_mesh.setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_mesh.setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_mesh.setUniFloat( "voxZ", m_pDh->m_zVoxel );

    m_mesh.setUniInt( "sector", m_pDh->m_quadrant );
    m_mesh.setUniFloat( "cutX", m_pDh->m_xSlize + 0.5f );
    m_mesh.setUniFloat( "cutY", m_pDh->m_ySlize + 0.5f );
    m_mesh.setUniFloat( "cutZ", m_pDh->m_zSlize + 0.5f );


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
    m_mesh.setUniInt( "cutTex", 9 );

    m_mesh.setUniInt( "tex0", 0 );
    m_mesh.setUniInt( "tex1", 1 );
    m_mesh.setUniInt( "tex2", 2 );
    m_mesh.setUniInt( "tex3", 3 );
    m_mesh.setUniInt( "tex4", 4 );
    m_mesh.setUniInt( "tex5", 5 );

    m_mesh.setUniInt( "type0", m_type[0] );
    m_mesh.setUniInt( "type1", m_type[1] );
    m_mesh.setUniInt( "type2", m_type[2] );
    m_mesh.setUniInt( "type3", m_type[3] );
    m_mesh.setUniInt( "type4", m_type[4] );
    m_mesh.setUniInt( "type5", m_type[5] );

    m_mesh.setUniFloat( "threshold0", m_threshold[0] );
    m_mesh.setUniFloat( "threshold1", m_threshold[1] );
    m_mesh.setUniFloat( "threshold2", m_threshold[2] );
    m_mesh.setUniFloat( "threshold3", m_threshold[3] );
    m_mesh.setUniFloat( "threshold4", m_threshold[4] );
    m_mesh.setUniFloat( "threshold5", m_threshold[5] );

    m_mesh.setUniFloat( "alpha0", m_alpha[0] );
    m_mesh.setUniFloat( "alpha1", m_alpha[1] );
    m_mesh.setUniFloat( "alpha2", m_alpha[2] );
    m_mesh.setUniFloat( "alpha3", m_alpha[3] );
    m_mesh.setUniFloat( "alpha4", m_alpha[4] );
    m_mesh.setUniFloat( "alpha5", m_alpha[5] );
}

void ShaderHelper::setFiberShaderVars()
{
    m_fibers.setUniInt( "dimX", m_pDh->m_columns );
    m_fibers.setUniInt( "dimY", m_pDh->m_rows );
    m_fibers.setUniInt( "dimZ", m_pDh->m_frames );
    m_fibers.setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_fibers.setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_fibers.setUniFloat( "voxZ", m_pDh->m_zVoxel );

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

    m_fibers.setUniInt( "tex", tex );
    m_fibers.setUniInt( "type", type );
    m_fibers.setUniFloat( "threshold", threshold );
}

void ShaderHelper::setSplineSurfaceShaderVars()
{
    m_splineSurf.setUniInt( "dimX", m_pDh->m_columns );
    m_splineSurf.setUniInt( "dimY", m_pDh->m_rows );
    m_splineSurf.setUniInt( "dimZ", m_pDh->m_frames );
    m_splineSurf.setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_splineSurf.setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_splineSurf.setUniFloat( "voxZ", m_pDh->m_zVoxel );

    m_splineSurf.setUniInt( "tex0", 0 );
    m_splineSurf.setUniInt( "tex1", 1 );
    m_splineSurf.setUniInt( "tex2", 2 );
    m_splineSurf.setUniInt( "tex3", 3 );
    m_splineSurf.setUniInt( "tex4", 4 );
    m_splineSurf.setUniInt( "tex5", 5 );

    m_splineSurf.setUniInt( "type0", m_type[0] );
    m_splineSurf.setUniInt( "type1", m_type[1] );
    m_splineSurf.setUniInt( "type2", m_type[2] );
    m_splineSurf.setUniInt( "type3", m_type[3] );
    m_splineSurf.setUniInt( "type4", m_type[4] );
    m_splineSurf.setUniInt( "type5", m_type[5] );

    m_splineSurf.setUniFloat( "threshold0", m_threshold[0] );
    m_splineSurf.setUniFloat( "threshold1", m_threshold[1] );
    m_splineSurf.setUniFloat( "threshold2", m_threshold[2] );
    m_splineSurf.setUniFloat( "threshold3", m_threshold[3] );
    m_splineSurf.setUniFloat( "threshold4", m_threshold[4] );
    m_splineSurf.setUniFloat( "threshold5", m_threshold[5] );

    m_splineSurf.setUniFloat( "alpha0", m_alpha[0] );
    m_splineSurf.setUniFloat( "alpha1", m_alpha[1] );
    m_splineSurf.setUniFloat( "alpha2", m_alpha[2] );
    m_splineSurf.setUniFloat( "alpha3", m_alpha[3] );
    m_splineSurf.setUniFloat( "alpha4", m_alpha[4] );
    m_splineSurf.setUniFloat( "alpha5", m_alpha[5] );
}

