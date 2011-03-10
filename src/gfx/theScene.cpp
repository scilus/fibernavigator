/////////////////////////////////////////////////////////////////////////////
// Name:            theScene.cpp
// Author:          ---
// Creation Date:   ---
//
// Description: This is the implementation file for theScene class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#include "theScene.h"
#include "../dataset/Anatomy.h"
#include "../dataset/AnatomyHelper.h"
#include "../dataset/fibers.h"
#include "../dataset/splinePoint.h"
#include "../dataset/surface.h"
#include "../gui/myListCtrl.h"
#include "../gui/SelectionObject.h"
#include "../misc/IsoSurface/CIsoSurface.h"

//////////////////////////////////////////////////////////////////////////////////
// Constructor
//
// i_datasetHelper          :
//////////////////////////////////////////////////////////////////////////////////
TheScene::TheScene( DatasetHelper* i_datasetHelper ) :
    m_datasetHelper( i_datasetHelper ), m_mainGLContext( 0 )
{
    m_datasetHelper->m_anatomyHelper = new AnatomyHelper( m_datasetHelper );

    // Initialize those to 0.0f to make sure that by some really odd chance, they cannot
    // be initialized to the same values as the real projection and modelview matrix.
    for( int i = 0; i < 16; ++i )
    {
        m_projection[i] = 0.0f;
        m_modelview[i]  = 0.0f;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////////
TheScene::~TheScene()
{
    m_datasetHelper->printDebug( _T( "execute theScene destructor" ), 0 );

#ifndef __WXMAC__
    // On mac, this is just a pointer to the original object that is deleted with the widgets.
    if ( m_mainGLContext )
        delete m_mainGLContext;
#endif
    m_datasetHelper->printDebug( _T( "theScene destructor done" ), 0 );
}

//////////////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_view       :
//////////////////////////////////////////////////////////////////////////////////
void TheScene::initGL( int i_view )
{
    try
    {
        GLenum l_error = glewInit();

        if( GLEW_OK != l_error )
        {
            // Problem: glewInit failed, something is seriously wrong.
            m_datasetHelper->printDebug( _T( "Error: " ) + wxString::FromAscii( (char*)glewGetErrorString( l_error ) ), 2 );
            exit( false );
        }
        if( i_view == MAIN_VIEW )
        {
            m_datasetHelper->printDebug( _T( "Status: Using GLEW " ) + wxString::FromAscii( (char*)glewGetString( GLEW_VERSION ) ), 1 );

            wxString l_vendor;
            wxString l_renderer;
            l_vendor   = wxString::FromAscii( (char*)glGetString( GL_VENDOR   ) );
            l_renderer = wxString::FromAscii( (char*)glGetString( GL_RENDERER ) );

            if ( l_renderer.Contains( _T( "GeForce 6" ) ) )
                m_datasetHelper->m_geforceLevel = 6;
            else if ( l_renderer.Contains( _T( "GeForce 7" ) ) )
                m_datasetHelper->m_geforceLevel = 7;
            else if ( l_renderer.Contains( _T( "GeForce 8" ) ) || l_renderer.Contains( _T( "GeForce GTX 2" ) ) )
                m_datasetHelper->m_geforceLevel = 8;

            m_datasetHelper->printDebug( l_vendor + _T( " " ) + l_renderer, 1 );

            if( ! glewIsSupported( "GL_ARB_shader_objects" ) )
            {
                printf( "*** ERROR no support for shader objects found.\n" );
                printf( "*** Please check your OpenGL installation...exiting.\n" );
                exit( false );
            }
        }
        glEnable( GL_DEPTH_TEST );

        if( ! m_datasetHelper->m_texAssigned )
        {
            m_datasetHelper->m_shaderHelper = new ShaderHelper( m_datasetHelper );
            m_datasetHelper->m_texAssigned  = true;
        }

        float view1 = 200;
        glClearColor( 1.0, 1.0, 1.0, 0.0 );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0, view1, 0, view1, -3000, 3000 );
    } catch ( ... )
    {
        if( m_datasetHelper->GLError() )
            m_datasetHelper->printGLError( wxT( "init" ) );
    }
}

void TheScene::bindTextures()
{
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    int l_c = 0;

    for( int i = 0; i < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( i );
        if( l_info->getType() < MESH && l_info->getShow() )
        {
            glActiveTexture( GL_TEXTURE0 + l_c );
            glBindTexture( GL_TEXTURE_3D, l_info->getGLuint() );
            if( l_info->getShowFS() )
            {
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            }
            else
            {
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            }
            if ( ++l_c == 10 )
            {
                printf( "reached 10 textures\n" );
                break;
            }
        }

    }
    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "bind textures" ) );
}

///////////////////////////////////////////////////////////////////////////
// This is the main render loop for the scene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderScene()
{
    // This will put the frustum information up to date for any render that needs it. 
    extractFrustum();
    
    if( m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount() == 0 )
        return;

    m_datasetHelper->m_shaderHelper->initializeArrays();

    // Opaque objects.
    renderSlizes();

    if( m_datasetHelper->m_surfaceLoaded )
        renderSplineSurface();

    if( m_datasetHelper->m_pointMode )
        drawPoints();

    if( m_datasetHelper->m_vectorsLoaded )
        drawVectors();

    if( m_datasetHelper->m_showColorMapLegend )
        drawColorMapLegend();

    if( m_datasetHelper->m_tensorsLoaded )
        renderTensors();
    
    if( m_datasetHelper->m_ODFsLoaded )
        renderODFs();
    
    renderMesh();

    if( m_datasetHelper->m_fibersLoaded )
    {
        if ( m_datasetHelper->m_useFakeTubes )
            renderFakeTubes();
        else
            renderFibers();
    }
    
    if( m_datasetHelper->m_showObjects )
        drawSelectionObjects();

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "render theScene" ) );
}

///////////////////////////////////////////////////////////////////////////
// This function will extracts the numbers and fill the m_frustum array with 
// the location of the planes for the frustum culling.
//
// See the tutorial here to understand : http://www.crownandcutlass.com/features/technicaldetails/frustum.html
///////////////////////////////////////////////////////////////////////////
void TheScene::extractFrustum()
{
   float l_projection[16];
   float l_modelview[16];
   float l_clip[16];
   float l_t;

   // Get the current PROJECTION matrix from OpenGL.
   glGetFloatv( GL_PROJECTION_MATRIX, l_projection );

   // Get the current MODELVIEW matrix from OpenGL.
   glGetFloatv( GL_MODELVIEW_MATRIX, l_modelview );

   // If both matrix have not changed, there is no point in recalculating the frustum infos.
   // At first this was done with memcmp .. but it was not working correctly so made a small templated functions that works just fine.
   if( Helper::areEquals( l_projection, m_projection, 16 ) && Helper::areEquals( l_modelview, m_modelview, 16 ) )
       return;
   else // We copy those matrix for the same test on the next loop.
   {
        memcpy( m_projection, l_projection, 16 * sizeof( float ) );
        memcpy( m_modelview,  l_modelview,  16 * sizeof( float ) );
   }

   // Combine the two matrices (multiply projection by modelview).
   l_clip[ 0] = l_modelview[ 0] * l_projection[ 0] + l_modelview[ 1] * l_projection[ 4] + l_modelview[ 2] * l_projection[ 8] + l_modelview[ 3] * l_projection[12];
   l_clip[ 1] = l_modelview[ 0] * l_projection[ 1] + l_modelview[ 1] * l_projection[ 5] + l_modelview[ 2] * l_projection[ 9] + l_modelview[ 3] * l_projection[13];
   l_clip[ 2] = l_modelview[ 0] * l_projection[ 2] + l_modelview[ 1] * l_projection[ 6] + l_modelview[ 2] * l_projection[10] + l_modelview[ 3] * l_projection[14];
   l_clip[ 3] = l_modelview[ 0] * l_projection[ 3] + l_modelview[ 1] * l_projection[ 7] + l_modelview[ 2] * l_projection[11] + l_modelview[ 3] * l_projection[15];

   l_clip[ 4] = l_modelview[ 4] * l_projection[ 0] + l_modelview[ 5] * l_projection[ 4] + l_modelview[ 6] * l_projection[ 8] + l_modelview[ 7] * l_projection[12];
   l_clip[ 5] = l_modelview[ 4] * l_projection[ 1] + l_modelview[ 5] * l_projection[ 5] + l_modelview[ 6] * l_projection[ 9] + l_modelview[ 7] * l_projection[13];
   l_clip[ 6] = l_modelview[ 4] * l_projection[ 2] + l_modelview[ 5] * l_projection[ 6] + l_modelview[ 6] * l_projection[10] + l_modelview[ 7] * l_projection[14];
   l_clip[ 7] = l_modelview[ 4] * l_projection[ 3] + l_modelview[ 5] * l_projection[ 7] + l_modelview[ 6] * l_projection[11] + l_modelview[ 7] * l_projection[15];

   l_clip[ 8] = l_modelview[ 8] * l_projection[ 0] + l_modelview[ 9] * l_projection[ 4] + l_modelview[10] * l_projection[ 8] + l_modelview[11] * l_projection[12];
   l_clip[ 9] = l_modelview[ 8] * l_projection[ 1] + l_modelview[ 9] * l_projection[ 5] + l_modelview[10] * l_projection[ 9] + l_modelview[11] * l_projection[13];
   l_clip[10] = l_modelview[ 8] * l_projection[ 2] + l_modelview[ 9] * l_projection[ 6] + l_modelview[10] * l_projection[10] + l_modelview[11] * l_projection[14];
   l_clip[11] = l_modelview[ 8] * l_projection[ 3] + l_modelview[ 9] * l_projection[ 7] + l_modelview[10] * l_projection[11] + l_modelview[11] * l_projection[15];

   l_clip[12] = l_modelview[12] * l_projection[ 0] + l_modelview[13] * l_projection[ 4] + l_modelview[14] * l_projection[ 8] + l_modelview[15] * l_projection[12];
   l_clip[13] = l_modelview[12] * l_projection[ 1] + l_modelview[13] * l_projection[ 5] + l_modelview[14] * l_projection[ 9] + l_modelview[15] * l_projection[13];
   l_clip[14] = l_modelview[12] * l_projection[ 2] + l_modelview[13] * l_projection[ 6] + l_modelview[14] * l_projection[10] + l_modelview[15] * l_projection[14];
   l_clip[15] = l_modelview[12] * l_projection[ 3] + l_modelview[13] * l_projection[ 7] + l_modelview[14] * l_projection[11] + l_modelview[15] * l_projection[15];

   // Extract the numbers for the RIGHT plane.
   m_datasetHelper->m_frustum[0][0] = l_clip[ 3] - l_clip[ 0];
   m_datasetHelper->m_frustum[0][1] = l_clip[ 7] - l_clip[ 4];
   m_datasetHelper->m_frustum[0][2] = l_clip[11] - l_clip[ 8];
   m_datasetHelper->m_frustum[0][3] = l_clip[15] - l_clip[12];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   l_t = sqrt( m_datasetHelper->m_frustum[0][0] * m_datasetHelper->m_frustum[0][0] + 
               m_datasetHelper->m_frustum[0][1] * m_datasetHelper->m_frustum[0][1] + 
               m_datasetHelper->m_frustum[0][2] * m_datasetHelper->m_frustum[0][2] );
   m_datasetHelper->m_frustum[0][0] /= l_t;
   m_datasetHelper->m_frustum[0][1] /= l_t;
   m_datasetHelper->m_frustum[0][2] /= l_t;
   m_datasetHelper->m_frustum[0][3] /= l_t;

   // Extract the numbers for the LEFT plane.
   m_datasetHelper->m_frustum[1][0] = l_clip[ 3] + l_clip[ 0];
   m_datasetHelper->m_frustum[1][1] = l_clip[ 7] + l_clip[ 4];
   m_datasetHelper->m_frustum[1][2] = l_clip[11] + l_clip[ 8];
   m_datasetHelper->m_frustum[1][3] = l_clip[15] + l_clip[12];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   l_t = sqrt( m_datasetHelper->m_frustum[1][0] * m_datasetHelper->m_frustum[1][0] + 
               m_datasetHelper->m_frustum[1][1] * m_datasetHelper->m_frustum[1][1] + 
               m_datasetHelper->m_frustum[1][2] * m_datasetHelper->m_frustum[1][2] );
   m_datasetHelper->m_frustum[1][0] /= l_t;
   m_datasetHelper->m_frustum[1][1] /= l_t;
   m_datasetHelper->m_frustum[1][2] /= l_t;
   m_datasetHelper->m_frustum[1][3] /= l_t;

   // Extract the BOTTOM plane.
   m_datasetHelper->m_frustum[2][0] = l_clip[ 3] + l_clip[ 1];
   m_datasetHelper->m_frustum[2][1] = l_clip[ 7] + l_clip[ 5];
   m_datasetHelper->m_frustum[2][2] = l_clip[11] + l_clip[ 9];
   m_datasetHelper->m_frustum[2][3] = l_clip[15] + l_clip[13];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   l_t = sqrt( m_datasetHelper->m_frustum[2][0] * m_datasetHelper->m_frustum[2][0] + 
               m_datasetHelper->m_frustum[2][1] * m_datasetHelper->m_frustum[2][1] + 
               m_datasetHelper->m_frustum[2][2] * m_datasetHelper->m_frustum[2][2] );
   m_datasetHelper->m_frustum[2][0] /= l_t;
   m_datasetHelper->m_frustum[2][1] /= l_t;
   m_datasetHelper->m_frustum[2][2] /= l_t;
   m_datasetHelper->m_frustum[2][3] /= l_t;

   // Extract the TOP plane.
   m_datasetHelper->m_frustum[3][0] = l_clip[ 3] - l_clip[ 1];
   m_datasetHelper->m_frustum[3][1] = l_clip[ 7] - l_clip[ 5];
   m_datasetHelper->m_frustum[3][2] = l_clip[11] - l_clip[ 9];
   m_datasetHelper->m_frustum[3][3] = l_clip[15] - l_clip[13];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   l_t = sqrt( m_datasetHelper->m_frustum[3][0] * m_datasetHelper->m_frustum[3][0] + 
               m_datasetHelper->m_frustum[3][1] * m_datasetHelper->m_frustum[3][1] + 
               m_datasetHelper->m_frustum[3][2] * m_datasetHelper->m_frustum[3][2] );
   m_datasetHelper->m_frustum[3][0] /= l_t;
   m_datasetHelper->m_frustum[3][1] /= l_t;
   m_datasetHelper->m_frustum[3][2] /= l_t;
   m_datasetHelper->m_frustum[3][3] /= l_t;

   // Extract the FAR plane.
   m_datasetHelper->m_frustum[4][0] = l_clip[ 3] - l_clip[ 2];
   m_datasetHelper->m_frustum[4][1] = l_clip[ 7] - l_clip[ 6];
   m_datasetHelper->m_frustum[4][2] = l_clip[11] - l_clip[10];
   m_datasetHelper->m_frustum[4][3] = l_clip[15] - l_clip[14];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   l_t = sqrt( m_datasetHelper->m_frustum[4][0] * m_datasetHelper->m_frustum[4][0] + 
               m_datasetHelper->m_frustum[4][1] * m_datasetHelper->m_frustum[4][1] + 
               m_datasetHelper->m_frustum[4][2] * m_datasetHelper->m_frustum[4][2] );
   m_datasetHelper->m_frustum[4][0] /= l_t;
   m_datasetHelper->m_frustum[4][1] /= l_t;
   m_datasetHelper->m_frustum[4][2] /= l_t;
   m_datasetHelper->m_frustum[4][3] /= l_t;

   // Extract the NEAR plane.
   m_datasetHelper->m_frustum[5][0] = l_clip[ 3] + l_clip[ 2];
   m_datasetHelper->m_frustum[5][1] = l_clip[ 7] + l_clip[ 6];
   m_datasetHelper->m_frustum[5][2] = l_clip[11] + l_clip[10];
   m_datasetHelper->m_frustum[5][3] = l_clip[15] + l_clip[14];
}

void TheScene::renderSlizes()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    if( m_datasetHelper->m_blendAlpha )
        glDisable( GL_ALPHA_TEST );
    else
        glEnable( GL_ALPHA_TEST );

    glAlphaFunc( GL_GREATER, 0.001f ); // Adjust your prefered threshold here.

    bindTextures();
    m_datasetHelper->m_shaderHelper->m_textureShader->bind();
    m_datasetHelper->m_shaderHelper->setTextureShaderVars();
    m_datasetHelper->m_shaderHelper->m_textureShader->setUniInt( "useColorMap", m_datasetHelper->m_colorMap );

    m_datasetHelper->m_anatomyHelper->renderMain();

    glDisable( GL_BLEND );

    m_datasetHelper->m_shaderHelper->m_textureShader->release();

    if( m_datasetHelper->m_showCrosshair )
        m_datasetHelper->m_anatomyHelper->renderCrosshair();

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "render slizes" ) );

    glPopAttrib();
}

void TheScene::renderSplineSurface()
{
    for( int i = 0; i < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( i );
        if( l_info->getType() == SURFACE && l_info->getShow() )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );

            if ( m_datasetHelper->m_pointMode )
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            else
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

            bindTextures();

            lightsOn();

            m_datasetHelper->m_shaderHelper->m_splineSurfShader->bind();
            m_datasetHelper->m_shaderHelper->setSplineSurfaceShaderVars();
            wxColor l_color = l_info->getColor();
            glColor3f( (float) l_color.Red() / 255.0, (float) l_color.Green() / 255.0, (float) l_color.Blue() / 255.0 );
            m_datasetHelper->m_shaderHelper->m_splineSurfShader->setUniInt( "useTex", !l_info->getUseTex() );
            m_datasetHelper->m_shaderHelper->m_splineSurfShader->setUniInt( "useLic", l_info->getUseLIC() );
            m_datasetHelper->m_shaderHelper->m_splineSurfShader->setUniInt( "useColorMap", m_datasetHelper->m_colorMap );

            l_info->draw();

            m_datasetHelper->m_shaderHelper->m_splineSurfShader->release();

            lightsOff();

            if ( m_datasetHelper->GLError() )
                m_datasetHelper->printGLError( wxT( "draw surface" ) );

            glPopAttrib();
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will render the mesh in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderMesh()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    if( m_datasetHelper->m_lighting )
    {
        lightsOn();
    }

    bindTextures();

    m_datasetHelper->m_shaderHelper->m_meshShader->bind();
    m_datasetHelper->m_shaderHelper->setMeshShaderVars();

    if( m_datasetHelper->m_pointMode )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    //Render selection objects
    glColor3f( 1.0f, 0.0f, 0.0f );
    std::vector< std::vector< SelectionObject* > > l_selecitonObjects = m_datasetHelper->getSelectionObjects();

    m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt  ( "showFS", true );
    m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt  ( "useTex", false );
    m_datasetHelper->m_shaderHelper->m_meshShader->setUniFloat( "alpha_", 1.0 );
    m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt  ( "useLic", false );

    for( unsigned int i = 0; i < l_selecitonObjects.size(); ++i )
    {
        for( unsigned int j = 0; j < l_selecitonObjects[i].size(); ++j )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );
            if( ! l_selecitonObjects[i][j]->isSelectionObject() && m_datasetHelper->m_showObjects)
                l_selecitonObjects[i][j]->drawIsoSurface();
            glPopAttrib();
        }
    }

    //Render meshes
    for( int i = 0; i < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( i );
        if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE )
        {
            if( l_info->getShow() )
            {
                wxColor l_color = l_info->getColor();
                glColor3f( (float)l_color.Red() / 255.0f, (float)l_color.Green() / 255.0f, (float)l_color.Blue() / 255.0f );

                m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt  ( "showFS",  l_info->getShowFS() );
                m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt  ( "useTex",  l_info->getUseTex() );
                m_datasetHelper->m_shaderHelper->m_meshShader->setUniFloat( "alpha_",  l_info->getAlpha() );
                m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt  ( "useLic",  l_info->getUseLIC() );
                m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt  ( "isGlyph", l_info->getIsGlyph());

                if(l_info->getAlpha() < 0.99)
                {
                    glDepthMask(GL_FALSE);
                }

                l_info->draw();

                if(l_info->getAlpha() < 0.99)
                {
                    glDepthMask(GL_TRUE);
                }
            }
        }
    }
    
    m_datasetHelper->m_shaderHelper->m_meshShader->release();

    lightsOff();

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw mesh" ) );

    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// This function will render the fibers in normal mode (not fake tubes) in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderFibers()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    for( int i = 0; i < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( i );

        if( l_info->getType() == FIBERS && l_info->getShow() )
        {
            lightsOff();

            if( m_datasetHelper->m_lighting )
            {
                lightsOn();
                GLfloat light_position0[] = { 1.0f, 1.0f, 1.0f, 0.0f };
                glLightfv( GL_LIGHT0, GL_POSITION, light_position0 );

            }
            if( ! l_info->getUseTex() )
            {
                bindTextures();
                m_datasetHelper->m_shaderHelper->m_fiberShader->bind();
                m_datasetHelper->m_shaderHelper->setFiberShaderVars();
                m_datasetHelper->m_shaderHelper->m_fiberShader->setUniInt( "useTex", !l_info->getUseTex() );
                m_datasetHelper->m_shaderHelper->m_fiberShader->setUniInt( "useColorMap", m_datasetHelper->m_colorMap );
                m_datasetHelper->m_shaderHelper->m_fiberShader->setUniInt( "useOverlay", l_info->getShowFS() );
            }
            if( m_datasetHelper->m_selBoxChanged )
            {
                ( (Fibers*)l_info )->updateLinesShown();
                m_datasetHelper->m_selBoxChanged = false;
            }
            l_info->draw();

            m_datasetHelper->m_shaderHelper->m_fiberShader->release();

            lightsOff();
        }
    }

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw fibers" ) );

    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// This function will render the fibers as fake tubes in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderFakeTubes()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    for( int i = 0; i < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( i );

        if( l_info->getType() == FIBERS && l_info->getShow() )
        {
            if( m_datasetHelper->m_selBoxChanged )
            {
                ( (Fibers*) l_info )->updateLinesShown();
                m_datasetHelper->m_selBoxChanged = false;
            }

            m_datasetHelper->m_shaderHelper->m_fakeTubeShader->bind();
            m_datasetHelper->m_shaderHelper->m_fakeTubeShader->setUniInt  ( "globalColor", l_info->getShowFS() );
            m_datasetHelper->m_shaderHelper->m_fakeTubeShader->setUniFloat( "dimX", (float) m_datasetHelper->m_mainFrame->m_mainGL->GetSize().x );
            m_datasetHelper->m_shaderHelper->m_fakeTubeShader->setUniFloat( "dimY", (float) m_datasetHelper->m_mainFrame->m_mainGL->GetSize().y );
            m_datasetHelper->m_shaderHelper->m_fakeTubeShader->setUniFloat( "thickness", GLfloat( 3.175 ) );

            l_info->draw();

            m_datasetHelper->m_shaderHelper->m_fakeTubeShader->release();
        }
    }

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw fake tubes" ) );

    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// This function will render the tensors in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderTensors()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // This will check if we are suppose to draw the tensor using GL_LINE or GL_FILL.
    if( m_datasetHelper->m_pointMode )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    for( int i = 0; i < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( i );

        if( l_info->getType() == TENSORS && l_info->getShow() )
        {
            lightsOff();
            l_info->draw();
        }
    }

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw tensors" ) );

    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// This function will render the ODFs in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderODFs()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // This will check if we are suppose to draw the odfs usung GL_LINE or GL_FILL.
    if( m_datasetHelper->m_pointMode )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    for( int i = 0; i < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( i );

        if( l_info->getType() == ODFS && l_info->getShow() )
        {
            lightsOff();
            l_info->draw();
        }
    }

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw ODFs" ) );

    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void TheScene::lightsOn()
{
    GLfloat l_lightAmbient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat l_lightDiffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat l_lightSpecular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat l_specRef[]       = { 0.5f, 0.5f, 0.5f, 0.5f };
    Vector3fT v1 = { { 0, 0, -1 } };
    Vector3fT l;
    Vector3fMultMat4( &l, &v1, &m_datasetHelper->m_transform );

    GLfloat l_lightPosition0[] = { l.s.X, l.s.Y, l.s.Z, 0.0 };

    glLightfv( GL_LIGHT0, GL_AMBIENT,  l_lightAmbient   );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  l_lightDiffuse   );
    glLightfv( GL_LIGHT0, GL_SPECULAR, l_lightSpecular  );
    glLightfv( GL_LIGHT0, GL_POSITION, l_lightPosition0 );

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glShadeModel( GL_SMOOTH );

    glEnable( GL_COLOR_MATERIAL );
    glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, l_specRef );
    glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 32 );

    if ( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "setup lights" ) );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void TheScene::lightsOff()
{
    glDisable( GL_LIGHTING );
    glDisable( GL_COLOR_MATERIAL );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_x      :
// i_y      :
// i_z      : 
// i_r      :
///////////////////////////////////////////////////////////////////////////
void TheScene::drawSphere( float i_x, float i_y, float i_z, float i_r )
{
    glPushMatrix();
    glTranslatef( i_x, i_y, i_z );

    GLUquadricObj *quadric = gluNewQuadric();
    gluQuadricNormals( quadric, GLU_SMOOTH );
    gluSphere( quadric, i_r, 32, 32 );
    glPopMatrix();

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw sphere" ) );
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the selection objects that are present in the scene.
///////////////////////////////////////////////////////////////////////////
void TheScene::drawSelectionObjects()
{
    std::vector< std::vector< SelectionObject* > > l_selectionObjects = m_datasetHelper->getSelectionObjects();
    for ( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
    {
        for ( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );

            l_selectionObjects[i][j]->draw();

            glPopAttrib();
        }
    }

    if ( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw selection objects" ) );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void TheScene::drawPoints()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    lightsOn();
    m_datasetHelper->m_shaderHelper->m_meshShader->bind();
    m_datasetHelper->m_shaderHelper->setMeshShaderVars();
    m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt( "showFS", true );
    m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt( "useTex", false );
    m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt( "cutAtSurface", false );
    m_datasetHelper->m_shaderHelper->m_meshShader->setUniInt( "lightOn", true );

    wxTreeItemId l_treeId, l_childId;
    wxTreeItemIdValue l_cookie = 0;
    l_treeId = m_datasetHelper->m_mainFrame->m_treeWidget->GetFirstChild( m_datasetHelper->m_mainFrame->m_tPointId, l_cookie );
    while( l_treeId.IsOk() )
    {
        SplinePoint* l_point = (SplinePoint*) ( m_datasetHelper->m_mainFrame->m_treeWidget->GetItemData( l_treeId ) );
        l_point->draw();

        l_treeId = m_datasetHelper->m_mainFrame->m_treeWidget->GetNextChild( m_datasetHelper->m_mainFrame->m_tPointId, l_cookie );
    }

    lightsOff();
    m_datasetHelper->m_shaderHelper->m_meshShader->release();
    glPopAttrib();

    if ( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw points" ) );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void TheScene::drawColorMapLegend()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glPushMatrix();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    int l_size = wxMax(wxMax(m_datasetHelper->m_rows, m_datasetHelper->m_columns), m_datasetHelper->m_frames );
    glOrtho( 0, l_size, 0, l_size, -3000, 3000 );

    m_datasetHelper->m_shaderHelper->m_legendShader->bind();
    m_datasetHelper->m_shaderHelper->m_legendShader->setUniInt( "useColorMap", m_datasetHelper->m_colorMap );

    glColor3f( 0.0f, 0.0f, 0.0f );
    glLineWidth( 5.0f );
    glBegin( GL_LINES );
    glTexCoord1f( 0.0 );
    glVertex3i( l_size - 60, 10, 2900 );
    glTexCoord1f( 1.0 );
    glVertex3i( l_size - 20, 10, 2900 );
    glEnd();

    m_datasetHelper->m_shaderHelper->m_legendShader->release();

    glLineWidth( 1.0f );
    glColor3f( 0.0f, 0.0f, 0.0f );
    glBegin( GL_LINES );
    glVertex3i( l_size - 60, 10, 2900 );
    glVertex3i( l_size - 60, 12, 2900 );
    glVertex3i( l_size - 50, 10, 2900 );
    glVertex3i( l_size - 50, 12, 2900 );
    glVertex3i( l_size - 40, 10, 2900 );
    glVertex3i( l_size - 40, 12, 2900 );
    glVertex3i( l_size - 30, 10, 2900 );
    glVertex3i( l_size - 30, 12, 2900 );
    glVertex3i( l_size - 20, 10, 2900 );
    glVertex3i( l_size - 20, 12, 2900 );
    glEnd();

    m_datasetHelper->m_shaderHelper->m_legendShader->release();

    glPopMatrix();
    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void TheScene::drawVectors()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    for( int i = 0; i < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( i );

        if( info->getType() == VECTORS && info->getShow() )
        {
            Anatomy* vecs = (Anatomy*)info;
            glLineWidth( 1.0 );
            glBegin( GL_LINES );

            float r, g, b, a;

            r = vecs->getColor().Red()   / 255.;
            g = vecs->getColor().Green() / 255.;
            b = vecs->getColor().Blue()  / 255.;
            a = 1.0;

            float bright = 1.2f;
            float dull = 0.7f;

            bool topview = m_datasetHelper->m_quadrant == 2 || 
                           m_datasetHelper->m_quadrant == 3 || 
                           m_datasetHelper->m_quadrant == 6 || 
                           m_datasetHelper->m_quadrant == 7;

            bool leftview = m_datasetHelper->m_quadrant == 5 || 
                            m_datasetHelper->m_quadrant == 6 ||
                            m_datasetHelper->m_quadrant == 7 || 
                            m_datasetHelper->m_quadrant == 8;

            bool frontview = m_datasetHelper->m_quadrant == 3 || 
                             m_datasetHelper->m_quadrant == 4 || 
                             m_datasetHelper->m_quadrant == 5 || 
                             m_datasetHelper->m_quadrant == 6;

            if( m_datasetHelper->m_showAxial )
            {
                for( int i = 0; i < m_datasetHelper->m_columns; ++i )
                {
                    for( int j = 0; j < m_datasetHelper->m_rows; ++j )
                    {
                        int slize = (int) ( m_datasetHelper->m_zSlize * m_datasetHelper->m_columns * m_datasetHelper->m_rows * 3 );
                        int index = i * 3 + j * m_datasetHelper->m_columns * 3 + slize;

                        float x = vecs->at( index );
                        float y = vecs->at( index + 1 );
                        float z = vecs->at( index + 2 );

                        if ( vecs->getUseTex() )
                        {
                            r = wxMin(1.0, fabs(x) * bright);
                            g = wxMin(1.0, fabs(y) * bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt( r * r + g * g + b * b );
                            r /= a;
                            g /= a;
                            b /= a;
                        }

                        if( ! vecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) m_datasetHelper->m_zSlize + .5 + z / 2. );
                            glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) m_datasetHelper->m_zSlize + .5 - z / 2. );
                        }
                        else
                        {
                            if( vecs->at( index + 2 ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if( topview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) m_datasetHelper->m_zSlize + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_datasetHelper->m_zSlize + .4 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) m_datasetHelper->m_zSlize + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_datasetHelper->m_zSlize + .4 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) m_datasetHelper->m_zSlize + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_datasetHelper->m_zSlize + .6 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) m_datasetHelper->m_zSlize + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_datasetHelper->m_zSlize + .6 );
                                }
                            }
                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if ( topview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) m_datasetHelper->m_zSlize + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_datasetHelper->m_zSlize + .4 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) m_datasetHelper->m_zSlize + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_datasetHelper->m_zSlize + .4 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) m_datasetHelper->m_zSlize + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_datasetHelper->m_zSlize + .6 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) m_datasetHelper->m_zSlize + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_datasetHelper->m_zSlize + .6 );
                                }
                            }
                        }
                    }
                }
            }
            if( m_datasetHelper->m_showCoronal )
            {
                for( int i = 0; i < m_datasetHelper->m_columns; ++i )
                {
                    for( int j = 0; j < m_datasetHelper->m_frames; ++j )
                    {
                        int slize = (int) ( m_datasetHelper->m_ySlize * m_datasetHelper->m_columns * 3 );
                        int index = i * 3 + slize + j * m_datasetHelper->m_columns * m_datasetHelper->m_rows * 3;

                        float x = vecs->at( index );
                        float y = vecs->at( index + 1 );
                        float z = vecs->at( index + 2 );

                        if( vecs->getUseTex() )
                        {
                            r = wxMin( 1.0, fabs( x ) * bright );
                            g = wxMin( 1.0, fabs( y ) * bright );
                            b = wxMin( 1.0, fabs( z ) * bright );
                            a = sqrt( r * r + g * g + b * b );
                            r /= a;
                            g /= a;
                            b /= a;
                        }
                        if( ! vecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_datasetHelper->m_ySlize + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                            glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_datasetHelper->m_ySlize + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                        }
                        else
                        {
                            if( vecs->at( index + 1 ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if( frontview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_datasetHelper->m_ySlize + .4, (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_datasetHelper->m_ySlize + .4, (GLfloat) j + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_datasetHelper->m_ySlize + .4, (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_datasetHelper->m_ySlize + .4, (GLfloat) j + .5 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_datasetHelper->m_ySlize + .6, (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_datasetHelper->m_ySlize + .6, (GLfloat) j + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_datasetHelper->m_ySlize + .6, (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_datasetHelper->m_ySlize + .6, (GLfloat) j + .5 );
                                }
                            }

                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if ( frontview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_datasetHelper->m_ySlize + .4, (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_datasetHelper->m_ySlize + .4, (GLfloat) j + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_datasetHelper->m_ySlize + .4, (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_datasetHelper->m_ySlize + .4, (GLfloat) j + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_datasetHelper->m_ySlize + .6, (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_datasetHelper->m_ySlize + .6, (GLfloat) j + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_datasetHelper->m_ySlize + .6, (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_datasetHelper->m_ySlize + .6, (GLfloat) j + .5 );
                                }
                            }
                        }
                    }
                }
            }
            if( m_datasetHelper->m_showSagittal )
            {
                for( int i = 0; i < m_datasetHelper->m_rows; ++i )
                {
                    for( int j = 0; j < m_datasetHelper->m_frames; ++j )
                    {
                        int slize = (int) ( m_datasetHelper->m_xSlize * 3 );
                        int index = slize + i * m_datasetHelper->m_columns * 3 + j * m_datasetHelper->m_columns * m_datasetHelper->m_rows * 3;

                        float x = vecs->at( index );
                        float y = vecs->at( index + 1 );
                        float z = vecs->at( index + 2 );

                        if( vecs->getUseTex() )
                        {
                            r = wxMin(1.0, fabs(x)* bright);
                            g = wxMin(1.0, fabs(y)* bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt( r * r + g * g + b * b );
                            r /= a;
                            g /= a;
                            b /= a;
                        }
                        if( ! vecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .5 + x / 2., (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                            glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .5 - x / 2., (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                        }
                        else
                        {
                            if( vecs->at( index ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if ( leftview )
                                {
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .4, (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .4, (GLfloat) i + .5, (GLfloat) j + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .4, (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .4, (GLfloat) i + .5, (GLfloat) j + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .6, (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .6, (GLfloat) i + .5, (GLfloat) j + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .6, (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .6, (GLfloat) i + .5, (GLfloat) j + .5 );
                                }
                            }
                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if( leftview )
                                {
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .4, (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .4, (GLfloat) i + .5, (GLfloat) j + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .4, (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .4, (GLfloat) i + .5, (GLfloat) j + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .6, (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .6, (GLfloat) i + .5, (GLfloat) j + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .6, (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) m_datasetHelper->m_xSlize + .6, (GLfloat) i + .5, (GLfloat) j + .5 );
                                }
                            }
                        }
                    }
                }
            }

            for( int j = 0; j < m_datasetHelper->m_mainFrame->m_listCtrl->GetItemCount(); ++j )
            {
                DatasetInfo* mesh = (DatasetInfo*) m_datasetHelper->m_mainFrame->m_listCtrl->GetItemData( j );

                if ( mesh->getType() == ISO_SURFACE && mesh->getShow() )
                {
                    CIsoSurface* surf = (CIsoSurface*) mesh;
                    std::vector< Vector > positions = surf->getSurfaceVoxelPositions();
                    for ( size_t k = 0; k < positions.size(); ++k )
                    {
                        int index = (int) positions[k].x * 3 + (int) positions[k].y * m_datasetHelper->m_columns * 3
                                + (int) positions[k].z * m_datasetHelper->m_rows * m_datasetHelper->m_columns * 3;

                        float x = vecs->at( index );
                        float y = vecs->at( index + 1 );
                        float z = vecs->at( index + 2 );

                        if ( vecs->getUseTex() )
                        {
                            r = wxMin(1.0, fabs(x)* bright);
                            g = wxMin(1.0, fabs(y)* bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt( r * r + g * g + b * b );
                            r /= a;
                            g /= a;
                            b /= a;
                        }

                        glColor4f( r, g, b, 1.0 );
                        glVertex3f( positions[k].x - x / 2., positions[k].y - y / 2., positions[k].z - z / 2. );
                        glVertex3f( positions[k].x + x / 2., positions[k].y + y / 2., positions[k].z + z / 2. );
                    }
                }

                else if( mesh->getType() == SURFACE && mesh->getShow() )
                {
                    Surface* surf = (Surface*) mesh;
                    std::vector< Vector > positions = surf->getSurfaceVoxelPositions();

                    for( size_t k = 0; k < positions.size(); ++k )
                    {
                        int index = (int)positions[k].x * 3 + 
                                    (int) positions[k].y * m_datasetHelper->m_columns * 3 +
                                    (int) positions[k].z * m_datasetHelper->m_rows * m_datasetHelper->m_columns * 3;

                        float x = vecs->at( index );
                        float y = vecs->at( index + 1 );
                        float z = vecs->at( index + 2 );

                        if( vecs->getUseTex() )
                        {
                            r = wxMin(1.0, fabs(x)* bright);
                            g = wxMin(1.0, fabs(y)* bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt( r * r + g * g + b * b );
                            r /= a;
                            g /= a;
                            b /= a;
                        }

                        glColor4f( r, g, b, 1.0 );
                        glVertex3f( positions[k].x - x / 2., positions[k].y - y / 2., positions[k].z - z / 2. );
                        glVertex3f( positions[k].x + x / 2., positions[k].y + y / 2., positions[k].z + z / 2. );
                    }
                }
            }
            glEnd();
        }
    }

    if( m_datasetHelper->GLError() )
        m_datasetHelper->printGLError( wxT( "draw vectors" ) );

    glDisable( GL_BLEND );

    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void TheScene::drawGraph()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    std::vector<float> l_points;

    wxTreeItemId l_treeId, childid;
    wxTreeItemIdValue l_cookie = 0;
    l_treeId = m_datasetHelper->m_mainFrame->m_treeWidget->GetFirstChild( m_datasetHelper->m_mainFrame->m_tPointId, l_cookie );
    while ( l_treeId.IsOk() )
    {
        SplinePoint* l_point = (SplinePoint*)( m_datasetHelper->m_mainFrame->m_treeWidget->GetItemData( l_treeId ) );
        l_points.push_back( l_point->X() );
        l_points.push_back( l_point->Y() );
        l_points.push_back( l_point->Z() );
        l_treeId = m_datasetHelper->m_mainFrame->m_treeWidget->GetNextChild( m_datasetHelper->m_mainFrame->m_tPointId, l_cookie );
    }

    m_datasetHelper->m_shaderHelper->m_graphShader->bind();
    m_datasetHelper->m_shaderHelper->m_graphShader->setUniInt  ( "globalColor", false );
    m_datasetHelper->m_shaderHelper->m_graphShader->setUniFloat( "animation", (float)m_datasetHelper->m_animationStep );
    m_datasetHelper->m_shaderHelper->m_graphShader->setUniFloat( "dimX", (float) m_datasetHelper->m_mainFrame->m_mainGL->GetSize().x );
    m_datasetHelper->m_shaderHelper->m_graphShader->setUniFloat( "dimY", (float) m_datasetHelper->m_mainFrame->m_mainGL->GetSize().y );

    int l_countPoints = l_points.size() / 3;
    glColor3f( 1.0f, 0.0f, 0.0f );

    for( int i = 0 ; i < l_countPoints ; ++i )
    {
        for( int j = 0 ; j < l_countPoints ; ++j )
        {
            if( j > i )
            {
                float l_length = sqrt ( ( l_points[i*3] - l_points[j*3] )     * ( l_points[i*3] - l_points[j*3] ) +
                                      ( l_points[i*3+1] - l_points[j*3+1] ) * ( l_points[i*3+1] - l_points[j*3+1] ) +
                                      ( l_points[i*3+2] - l_points[j*3+2] ) * ( l_points[i*3+2] - l_points[j*3+2] ) );

                m_datasetHelper->m_shaderHelper->m_graphShader->setUniFloat( "thickness", (float)( i+1 )*2 );
                glColor3f( i/10.0f, j/10.0f, i+j/20.0f );
                glBegin( GL_QUADS );
                    glTexCoord3f( -1.0f, 0, l_length );
                    glNormal3f( l_points[i*3] - l_points[j*3], l_points[i*3+1] - l_points[j*3+1], l_points[i*3+2] - l_points[j*3+2] );
                    glVertex3f( l_points[i*3], l_points[i*3+1], l_points[i*3+2] );
                    glTexCoord3f( 1.0f, 0, l_length );
                    glNormal3f( l_points[i*3] - l_points[j*3], l_points[i*3+1] - l_points[j*3+1], l_points[i*3+2] - l_points[j*3+2] );
                    glVertex3f( l_points[i*3], l_points[i*3+1], l_points[i*3+2] );
                    glTexCoord3f( 1.0f, 1.0, l_length );
                    glNormal3f( l_points[i*3] - l_points[j*3], l_points[i*3+1] -  l_points[j*3+1], l_points[i*3+2] - l_points[j*3+2] );
                    glVertex3f( l_points[j*3], l_points[j*3+1], l_points[j*3+2] );
                    glTexCoord3f( -1.0f, 1.0, l_length );
                    glNormal3f( l_points[i*3] - l_points[j*3], l_points[i*3+1] -  l_points[j*3+1], l_points[i*3+2] - l_points[j*3+2] );
                    glVertex3f( l_points[j*3], l_points[j*3+1], l_points[j*3+2] );
                glEnd();
            }
        }
    }

    m_datasetHelper->m_shaderHelper->m_graphShader->release();

    glPopAttrib();
}
