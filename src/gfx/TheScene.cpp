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

#include "TheScene.h"

#include "../Logger.h"
#include "../main.h"
#include "../dataset/Anatomy.h"
#include "../dataset/AnatomyHelper.h"
#include "../dataset/DatasetInfo.h"
#include "../dataset/DatasetManager.h"
#include "../dataset/Fibers.h"
#include "../dataset/Mesh.h"
#include "../dataset/ODFs.h"
#include "../gfx/ShaderHelper.h"
#include "../gui/ArcBall.h"
#include "../gui/MainFrame.h"
#include "../gui/MyListCtrl.h"
#include "../gui/SceneManager.h"
#include "../gui/SelectionObject.h"
#include "../misc/IsoSurface/CIsoSurface.h"

#include <algorithm>
#include <vector>
using std::vector;

namespace
{
    void drawVectorsHelper( const vector< Vector > &positions, const Anatomy * const pVecs, const float bright )
    {
        float columns = DatasetManager::getInstance()->getColumns();
        float rows    = DatasetManager::getInstance()->getRows();

        for( vector< Vector >::const_iterator it = positions.begin(); it != positions.end(); ++it )
        {
            float r, g, b, a;
            int index = 3 * ((int)it->x + (int)( it->y * columns ) + (int)( it->z * rows * columns ) );

            float x = pVecs->at( index );
            float y = pVecs->at( index + 1 );
            float z = pVecs->at( index + 2 );

            if( pVecs->getUseTex() )
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
            glVertex3f( it->x - x / 2., it->y - y / 2., it->z - z / 2. );
            glVertex3f( it->x + x / 2., it->y + y / 2., it->z + z / 2. );
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////
// Constructor
//
// i_datasetHelper          :
//////////////////////////////////////////////////////////////////////////////////
TheScene::TheScene()
:   m_isRotateZ( false ),
    m_isRotateY( false ),
    m_isRotateX( false ),
    m_isNavSagital( false ),
    m_isNavAxial( false ),
    m_isNavCoronal( false ),
    m_rotAngleX( 0.0f ),
    m_rotAngleY( 0.0f ),
    m_rotAngleZ( 0.0f ),
    m_posSagital( 0.0f ),
    m_posCoronal( 0.0f ),
    m_posAxial( 0.0f ),
    m_pMainGLContext( NULL )
{
    // Initialize those to 0.0f to make sure that by some really odd chance, they cannot
    // be initialized to the same values as the real projection and modelview matrix.
    std::fill( m_projection, m_projection + 16, 0.0f );
    std::fill( m_modelview, m_modelview + 16, 0.0f );
}

//////////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////////
TheScene::~TheScene()
{
    Logger::getInstance()->print( wxT( "Execute TheScene destructor" ), LOGLEVEL_DEBUG );

#ifndef __WXMAC__
    // On mac, this is just a pointer to the original object that is deleted with the widgets.
    delete m_pMainGLContext;
    m_pMainGLContext = NULL;
#endif
    Logger::getInstance()->print( wxT( "TheScene destructor done" ), LOGLEVEL_DEBUG );
}

//////////////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_view       :
//////////////////////////////////////////////////////////////////////////////////
void TheScene::initGL( int whichView )
{
    try
    {
        GLenum errorCode = glewInit();
        bool useGeometry( true );

        if( GLEW_OK != errorCode )
        {
            // Problem: glewInit failed, something is seriously wrong.
            Logger::getInstance()->print( wxT( "Error: " ) + wxString::FromAscii( (char*)glewGetErrorString( errorCode ) ), LOGLEVEL_ERROR );
            exit( false );
        }
        if( whichView == MAIN_VIEW )
        {
            Logger::getInstance()->print( wxT( "Status: Using GLEW " ) + wxString::FromAscii( (char*)glewGetString( GLEW_VERSION ) ), LOGLEVEL_MESSAGE );

            wxString vendorId;
            wxString rendererId;
            vendorId   = wxString::FromAscii( (char*)glGetString( GL_VENDOR   ) );
            rendererId = wxString::FromAscii( (char*)glGetString( GL_RENDERER ) );

            Logger::getInstance()->print( vendorId + _T( " " ) + rendererId, LOGLEVEL_MESSAGE );

            if( !glewIsSupported( "GL_ARB_shader_objects" ) )
            {
                printf( "*** ERROR no support for shader objects found.\n" );
                printf( "*** Please check your OpenGL installation...exiting.\n" );
                exit( false );
            }
            else if ( !glewIsSupported( "GL_VERSION_3_2" ) && !glewIsSupported( "GL_ARB_geometry_shader4" ) && !glewIsExtensionSupported( "GL_EXT_geometry_shader4" ) )
            {
                Logger::getInstance()->print( wxT( "Geometry shaders not supported. Some operations may run slower and use more CPU." ), LOGLEVEL_WARNING );
                SceneManager::getInstance()->setGeometryShaderSupported( false );
                useGeometry = false;
            }
            ShaderHelper::getInstance()->loadShaders( useGeometry );
        }
        glEnable( GL_DEPTH_TEST );

        float view1 = 200;
        glClearColor( 1.0, 1.0, 1.0, 0.0 );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0, view1, 0, view1, -3000, 3000 );
    }
    catch ( ... )
    {
        Logger::getInstance()->printIfGLError( wxT( "Init" ) );
    }
}

void TheScene::bindTextures()
{
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    int allocatedTextureCount = 0;

    for( int i = 0; i < MyApp::frame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pDsInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( i ) );
        if( pDsInfo->getType() < MESH && pDsInfo->getShow() )
        {
            glActiveTexture( GL_TEXTURE0 + allocatedTextureCount );
            glBindTexture( GL_TEXTURE_3D, pDsInfo->getGLuint() );
            if( pDsInfo->getShowFS() )
            {
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            }
            else
            {
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            }
            if ( ++allocatedTextureCount == 10 )
            {
                Logger::getInstance()->print( wxT( "Reached 10 textures!" ), LOGLEVEL_WARNING );
                break;
            }
        }
    }

    Logger::getInstance()->printIfGLError( wxT( "Bind textures") );
}

///////////////////////////////////////////////////////////////////////////
// This is the main render loop for the scene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderScene()
{
    Logger::getInstance()->printIfGLError( wxT( "Error before renderScene" ) );
    // This will put the frustum information up to date for any render that needs it. 
    extractFrustum();
    
    if( MyApp::frame->m_pListCtrl->GetItemCount() == 0 )
        return;

    ShaderHelper::getInstance()->initializeArrays();

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    //Animate
    if(m_isRotateZ)
    {
        if (m_rotAngleZ>360) 
            m_rotAngleZ=0;

        glTranslatef( columns * voxelX * 0.5f, rows * voxelY * 0.5f, frames * voxelZ * 0.5f );
        glRotatef( m_rotAngleZ, 0, 0, 1 );
        glTranslatef( -columns * voxelX * 0.5f, -rows * voxelY * 0.5f, -frames * voxelZ * 0.5f );
    }

    if(m_isRotateY)
    {
        if (m_rotAngleY>360) 
            m_rotAngleY=0;

        glTranslatef( columns * voxelX * 0.5f, rows * voxelY * 0.5f, frames * voxelZ * 0.5f );
        glRotatef( m_rotAngleY, 0, 1, 0 );
        glTranslatef( -columns * voxelX * 0.5f, -rows * voxelY * 0.5f, -frames * voxelZ * 0.5f );
    }

    if(m_isRotateX)
    {
        if (m_rotAngleX>360) 
            m_rotAngleX=0;

        glTranslatef( columns * voxelX * 0.5f, rows * voxelY * 0.5f, frames * voxelZ * 0.5f );
        glRotatef( m_rotAngleX, 1, 0, 0 );
        glTranslatef( -columns * voxelX * 0.5f, -rows * voxelY * 0.5f, -frames * voxelZ * 0.5f );
    }

    //Navigate through slices
    if(m_isNavSagital) 
    {
        if (m_posSagital > columns) 
            m_posSagital = 0;

        SceneManager::getInstance()->updateView( m_posSagital, 
                                                 SceneManager::getInstance()->getSliceY(), 
                                                 SceneManager::getInstance()->getSliceZ() );
        MyApp::frame->m_pXSlider->SetValue( m_posSagital );
    }

    if(m_isNavCoronal)
    {
        if (m_posCoronal > rows) 
            m_posCoronal = 0;

        SceneManager::getInstance()->updateView( SceneManager::getInstance()->getSliceX(),
                                                 m_posCoronal,
                                                 SceneManager::getInstance()->getSliceZ() );
        MyApp::frame->m_pYSlider->SetValue( m_posCoronal );
    }

    if(m_isNavAxial)
    {
        if (m_posAxial > frames) 
            m_posAxial = 0;

        SceneManager::getInstance()->updateView( SceneManager::getInstance()->getSliceX(),
                                                 SceneManager::getInstance()->getSliceY(),
                                                 m_posAxial );
        MyApp::frame->m_pZSlider->SetValue( m_posAxial );
    }

    // Opaque objects.
    renderSlices();


    if( DatasetManager::getInstance()->isVectorsLoaded() )
        drawVectors();

    if( DatasetManager::getInstance()->isTensorsLoaded() )
        renderTensors();
    
    if( DatasetManager::getInstance()->isOdfsLoaded() )
        renderODFs();
    
    renderMesh();
	renderFibers();
    
    if( SceneManager::getInstance()->getShowAllSelObj() )
    {
        drawSelectionObjects();
    }

    Logger::getInstance()->printIfGLError( wxT( "Rendering Scene" ) );
}

///////////////////////////////////////////////////////////////////////////
// This function will extracts the numbers and fill the m_frustum array with 
// the location of the planes for the frustum culling.
//
// See the tutorial here to understand : http://www.crownandcutlass.com/features/technicaldetails/frustum.html
///////////////////////////////////////////////////////////////////////////
void TheScene::extractFrustum()
{
   float curProjection[16];
   float curModelview[16];
   float clipMat[16];

   // Get the current PROJECTION matrix from OpenGL.
   glGetFloatv( GL_PROJECTION_MATRIX, curProjection );

   // Get the current MODELVIEW matrix from OpenGL.
   glGetFloatv( GL_MODELVIEW_MATRIX, curModelview );

   // If both matrix have not changed, there is no point in recalculating the frustum infos.
   // At first this was done with memcmp .. but it was not working correctly so made a small templated functions that works just fine.
   if( Helper::areEquals( curProjection, m_projection, 16 ) && Helper::areEquals( curModelview, m_modelview, 16 ) )
       return;
   else // We copy those matrix for the same test on the next loop.
   {
        memcpy( m_projection, curProjection, 16 * sizeof( float ) );
        memcpy( m_modelview,  curModelview,  16 * sizeof( float ) );
   }

   // Combine the two matrices (multiply projection by modelview).
   clipMat[ 0] = curModelview[ 0] * curProjection[ 0] + curModelview[ 1] * curProjection[ 4] + curModelview[ 2] * curProjection[ 8] + curModelview[ 3] * curProjection[12];
   clipMat[ 1] = curModelview[ 0] * curProjection[ 1] + curModelview[ 1] * curProjection[ 5] + curModelview[ 2] * curProjection[ 9] + curModelview[ 3] * curProjection[13];
   clipMat[ 2] = curModelview[ 0] * curProjection[ 2] + curModelview[ 1] * curProjection[ 6] + curModelview[ 2] * curProjection[10] + curModelview[ 3] * curProjection[14];
   clipMat[ 3] = curModelview[ 0] * curProjection[ 3] + curModelview[ 1] * curProjection[ 7] + curModelview[ 2] * curProjection[11] + curModelview[ 3] * curProjection[15];

   clipMat[ 4] = curModelview[ 4] * curProjection[ 0] + curModelview[ 5] * curProjection[ 4] + curModelview[ 6] * curProjection[ 8] + curModelview[ 7] * curProjection[12];
   clipMat[ 5] = curModelview[ 4] * curProjection[ 1] + curModelview[ 5] * curProjection[ 5] + curModelview[ 6] * curProjection[ 9] + curModelview[ 7] * curProjection[13];
   clipMat[ 6] = curModelview[ 4] * curProjection[ 2] + curModelview[ 5] * curProjection[ 6] + curModelview[ 6] * curProjection[10] + curModelview[ 7] * curProjection[14];
   clipMat[ 7] = curModelview[ 4] * curProjection[ 3] + curModelview[ 5] * curProjection[ 7] + curModelview[ 6] * curProjection[11] + curModelview[ 7] * curProjection[15];

   clipMat[ 8] = curModelview[ 8] * curProjection[ 0] + curModelview[ 9] * curProjection[ 4] + curModelview[10] * curProjection[ 8] + curModelview[11] * curProjection[12];
   clipMat[ 9] = curModelview[ 8] * curProjection[ 1] + curModelview[ 9] * curProjection[ 5] + curModelview[10] * curProjection[ 9] + curModelview[11] * curProjection[13];
   clipMat[10] = curModelview[ 8] * curProjection[ 2] + curModelview[ 9] * curProjection[ 6] + curModelview[10] * curProjection[10] + curModelview[11] * curProjection[14];
   clipMat[11] = curModelview[ 8] * curProjection[ 3] + curModelview[ 9] * curProjection[ 7] + curModelview[10] * curProjection[11] + curModelview[11] * curProjection[15];

   clipMat[12] = curModelview[12] * curProjection[ 0] + curModelview[13] * curProjection[ 4] + curModelview[14] * curProjection[ 8] + curModelview[15] * curProjection[12];
   clipMat[13] = curModelview[12] * curProjection[ 1] + curModelview[13] * curProjection[ 5] + curModelview[14] * curProjection[ 9] + curModelview[15] * curProjection[13];
   clipMat[14] = curModelview[12] * curProjection[ 2] + curModelview[13] * curProjection[ 6] + curModelview[14] * curProjection[10] + curModelview[15] * curProjection[14];
   clipMat[15] = curModelview[12] * curProjection[ 3] + curModelview[13] * curProjection[ 7] + curModelview[14] * curProjection[11] + curModelview[15] * curProjection[15];

   // Extract the numbers for the RIGHT plane.
   SceneManager::getInstance()->m_frustum[0][0] = clipMat[ 3] - clipMat[ 0];
   SceneManager::getInstance()->m_frustum[0][1] = clipMat[ 7] - clipMat[ 4];
   SceneManager::getInstance()->m_frustum[0][2] = clipMat[11] - clipMat[ 8];
   SceneManager::getInstance()->m_frustum[0][3] = clipMat[15] - clipMat[12];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   float normalizationTerm = sqrt( SceneManager::getInstance()->m_frustum[0][0] * SceneManager::getInstance()->m_frustum[0][0] + 
                                   SceneManager::getInstance()->m_frustum[0][1] * SceneManager::getInstance()->m_frustum[0][1] + 
                                   SceneManager::getInstance()->m_frustum[0][2] * SceneManager::getInstance()->m_frustum[0][2] );
   SceneManager::getInstance()->m_frustum[0][0] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[0][1] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[0][2] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[0][3] /= normalizationTerm;

   // Extract the numbers for the LEFT plane.
   SceneManager::getInstance()->m_frustum[1][0] = clipMat[ 3] + clipMat[ 0];
   SceneManager::getInstance()->m_frustum[1][1] = clipMat[ 7] + clipMat[ 4];
   SceneManager::getInstance()->m_frustum[1][2] = clipMat[11] + clipMat[ 8];
   SceneManager::getInstance()->m_frustum[1][3] = clipMat[15] + clipMat[12];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   normalizationTerm = sqrt( SceneManager::getInstance()->m_frustum[1][0] * SceneManager::getInstance()->m_frustum[1][0] + 
                             SceneManager::getInstance()->m_frustum[1][1] * SceneManager::getInstance()->m_frustum[1][1] + 
                             SceneManager::getInstance()->m_frustum[1][2] * SceneManager::getInstance()->m_frustum[1][2] );
   SceneManager::getInstance()->m_frustum[1][0] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[1][1] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[1][2] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[1][3] /= normalizationTerm;

   // Extract the BOTTOM plane.
   SceneManager::getInstance()->m_frustum[2][0] = clipMat[ 3] + clipMat[ 1];
   SceneManager::getInstance()->m_frustum[2][1] = clipMat[ 7] + clipMat[ 5];
   SceneManager::getInstance()->m_frustum[2][2] = clipMat[11] + clipMat[ 9];
   SceneManager::getInstance()->m_frustum[2][3] = clipMat[15] + clipMat[13];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   normalizationTerm = sqrt( SceneManager::getInstance()->m_frustum[2][0] * SceneManager::getInstance()->m_frustum[2][0] + 
                             SceneManager::getInstance()->m_frustum[2][1] * SceneManager::getInstance()->m_frustum[2][1] + 
                             SceneManager::getInstance()->m_frustum[2][2] * SceneManager::getInstance()->m_frustum[2][2] );
   SceneManager::getInstance()->m_frustum[2][0] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[2][1] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[2][2] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[2][3] /= normalizationTerm;

   // Extract the TOP plane.
   SceneManager::getInstance()->m_frustum[3][0] = clipMat[ 3] - clipMat[ 1];
   SceneManager::getInstance()->m_frustum[3][1] = clipMat[ 7] - clipMat[ 5];
   SceneManager::getInstance()->m_frustum[3][2] = clipMat[11] - clipMat[ 9];
   SceneManager::getInstance()->m_frustum[3][3] = clipMat[15] - clipMat[13];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   normalizationTerm = sqrt( SceneManager::getInstance()->m_frustum[3][0] * SceneManager::getInstance()->m_frustum[3][0] + 
                             SceneManager::getInstance()->m_frustum[3][1] * SceneManager::getInstance()->m_frustum[3][1] + 
                             SceneManager::getInstance()->m_frustum[3][2] * SceneManager::getInstance()->m_frustum[3][2] );
   SceneManager::getInstance()->m_frustum[3][0] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[3][1] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[3][2] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[3][3] /= normalizationTerm;

   // Extract the FAR plane.
   SceneManager::getInstance()->m_frustum[4][0] = clipMat[ 3] - clipMat[ 2];
   SceneManager::getInstance()->m_frustum[4][1] = clipMat[ 7] - clipMat[ 6];
   SceneManager::getInstance()->m_frustum[4][2] = clipMat[11] - clipMat[10];
   SceneManager::getInstance()->m_frustum[4][3] = clipMat[15] - clipMat[14];

   // Normalize the result.
   // Since we only use the frustum information for box bounding object, we do not need to normalize the values.
   normalizationTerm = sqrt( SceneManager::getInstance()->m_frustum[4][0] * SceneManager::getInstance()->m_frustum[4][0] + 
                             SceneManager::getInstance()->m_frustum[4][1] * SceneManager::getInstance()->m_frustum[4][1] + 
                             SceneManager::getInstance()->m_frustum[4][2] * SceneManager::getInstance()->m_frustum[4][2] );
   SceneManager::getInstance()->m_frustum[4][0] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[4][1] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[4][2] /= normalizationTerm;
   SceneManager::getInstance()->m_frustum[4][3] /= normalizationTerm;

   // Extract the NEAR plane.
   SceneManager::getInstance()->m_frustum[5][0] = clipMat[ 3] + clipMat[ 2];
   SceneManager::getInstance()->m_frustum[5][1] = clipMat[ 7] + clipMat[ 6];
   SceneManager::getInstance()->m_frustum[5][2] = clipMat[11] + clipMat[10];
   SceneManager::getInstance()->m_frustum[5][3] = clipMat[15] + clipMat[14];
}

void TheScene::renderSlices()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    if( SceneManager::getInstance()->isAlphaBlend() )
    {
        glDisable( GL_ALPHA_TEST );
    }
    else
    {
        glEnable( GL_ALPHA_TEST );
    }

    glAlphaFunc( GL_GREATER, 0.001f ); // Adjust your preferred threshold here.

    bindTextures();
    ShaderHelper::getInstance()->getAnatomyShader()->bind();
    ShaderHelper::getInstance()->setTextureShaderVars();
    ShaderHelper::getInstance()->getAnatomyShader()->setUniInt( "useColorMap", SceneManager::getInstance()->getColorMap() );

    SceneManager::getInstance()->getAnatomyHelper()->renderMain();

    glDisable( GL_BLEND );

    ShaderHelper::getInstance()->getAnatomyShader()->release();

    if( SceneManager::getInstance()->isCrosshairDisplayed() )
    {
        SceneManager::getInstance()->getAnatomyHelper()->renderCrosshair();
    }

    Logger::getInstance()->printIfGLError( wxT( "Render slices" ) );

    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// This function will render the mesh in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderMesh()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    if( SceneManager::getInstance()->isLightingActive() )
    {
        lightsOn();
    }

    bindTextures();

    ShaderHelper::getInstance()->getMeshShader()->bind();
    ShaderHelper::getInstance()->setMeshShaderVars();

    if( SceneManager::getInstance()->isPointMode() )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    //Render selection objects
    glColor3f( 1.0f, 0.0f, 0.0f );
    SelectionObjList selectionObjects = SceneManager::getInstance()->getSelectionObjects();

    ShaderHelper::getInstance()->getMeshShader()->setUniInt  ( "showFS", true );
    ShaderHelper::getInstance()->getMeshShader()->setUniInt  ( "useTex", false );
    ShaderHelper::getInstance()->getMeshShader()->setUniFloat( "alpha_", 1.0 );

    for( unsigned int i = 0; i < selectionObjects.size(); ++i )
    {
        for( unsigned int j = 0; j < selectionObjects[i].size(); ++j )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );
            if( SceneManager::getInstance()->getShowAllSelObj() && !selectionObjects[i][j]->isSelectionObject() )
                selectionObjects[i][j]->drawIsoSurface();
            glPopAttrib();
        }
    }

    //Render meshes
    vector< Mesh * > v = DatasetManager::getInstance()->getMeshes();
    for( vector<Mesh *>::iterator mesh = v.begin(); mesh != v.end(); ++mesh )
    {
        if( (*mesh)->getShow() )
        {
            renderMeshInternal( *mesh );
        }
    }

    for( int i = 0; i < MyApp::frame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pDsInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( i ) );

        if( ISO_SURFACE == pDsInfo->getType() && pDsInfo->getShow() )
        {
            renderMeshInternal( pDsInfo );
        }
    }

    ShaderHelper::getInstance()->getMeshShader()->release();

    lightsOff();

    Logger::getInstance()->printIfGLError( wxT( "Draw mesh " ) );

    glPopAttrib();
}

//////////////////////////////////////////////////////////////////////////

void TheScene::renderMeshInternal(  DatasetInfo *pDsInfo )
{
    wxColor color = pDsInfo->getColor();
    glColor3f( (float)color.Red() / 255.0f, (float)color.Green() / 255.0f, (float)color.Blue() / 255.0f );

    ShaderHelper::getInstance()->getMeshShader()->setUniInt  ( "showFS",  pDsInfo->getShowFS() );
    ShaderHelper::getInstance()->getMeshShader()->setUniInt  ( "useTex",  pDsInfo->getUseTex() );
    ShaderHelper::getInstance()->getMeshShader()->setUniFloat( "alpha_",  pDsInfo->getAlpha() );
    ShaderHelper::getInstance()->getMeshShader()->setUniInt  ( "isGlyph", pDsInfo->getIsGlyph());

    if( pDsInfo->getAlpha() < 0.99 )
    {
        glDepthMask( GL_FALSE );
    }

    pDsInfo->draw();

    if( pDsInfo->getAlpha() < 0.99 )
    {
        glDepthMask( GL_TRUE );
    }
}

//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// This function will render the fibers in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderFibers()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    for( int i = 0; i < MyApp::frame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pDsInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( i ) );

        if( pDsInfo->getType() == FIBERS && pDsInfo->getShow())
        {
			Fibers* pFibers = (Fibers*)pDsInfo;
			if( pFibers != NULL )
			{
                if( SceneManager::getInstance()->isSelBoxChanged() )
                {
                    pFibers->updateLinesShown();
                }

				if( pFibers->isUsingFakeTubes() )
				{
					pFibers->draw();

                    Logger::getInstance()->printIfGLError( wxT( "Draw fake tubes" ) );
				}
				else // render normally
				{
					if( SceneManager::getInstance()->isLightingActive() )
					{
						lightsOn();
						GLfloat light_position0[] = { 1.0f, 1.0f, 1.0f, 0.0f };
						glLightfv( GL_LIGHT0, GL_POSITION, light_position0 );
					}
					if( ! pFibers->getUseTex() )
					{
						bindTextures();
					}
					
					pFibers->draw();
					lightsOff();

                    Logger::getInstance()->printIfGLError( wxT( "Draw fibers" ) );
				}
			}
		}
	}

    SceneManager::getInstance()->setSelBoxChanged( false );

    SelectionObjList selectionObjects = SceneManager::getInstance()->getSelectionObjects();
    for( SelectionObjList::iterator itMaster = selectionObjects.begin(); itMaster != selectionObjects.end(); ++itMaster )
    {
        for( vector< SelectionObject *>::iterator itChild = itMaster->begin(); itChild != itMaster->end(); ++itChild )
        {
            (*itChild)->setIsDirty( false );
        }
    }

	glPopAttrib();
}


///////////////////////////////////////////////////////////////////////////
// This function will render the tensors in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderTensors()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // This will check if we are suppose to draw the tensor using GL_LINE or GL_FILL.
    if( SceneManager::getInstance()->isPointMode() )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    for( int i = 0; i < MyApp::frame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pDsInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( i ) );

        if( pDsInfo->getType() == TENSORS && pDsInfo->getShow() )
        {
            lightsOff();
            pDsInfo->draw();
        }
    }

    Logger::getInstance()->printIfGLError( wxT( "Draw tensors" ) );
    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// This function will render the ODFs in theScene.
///////////////////////////////////////////////////////////////////////////
void TheScene::renderODFs()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // This will check if we are suppose to draw the odfs using GL_LINE or GL_FILL.
    if( SceneManager::getInstance()->isPointMode() )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    vector<ODFs *> v = DatasetManager::getInstance()->getOdfs();
    for(vector<ODFs *>::iterator it = v.begin(); it != v.end(); ++it )
    {
        ODFs *pOdfs = *it;
        if( pOdfs->getShow() )
        {
            lightsOff();
            pOdfs->draw();
        }
    }

    Logger::getInstance()->printIfGLError( wxT( "Draw ODFs" ) );
    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void TheScene::lightsOn()
{
    GLfloat ambientLight[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat diffuseLight[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat specularLight[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat specRef[]       = { 0.5f, 0.5f, 0.5f, 0.5f };
    Vector3fT v1 = { { 0, 0, -1 } };
    Vector3fT l;
    Matrix4fT transform = SceneManager::getInstance()->getTransform();
    Vector3fMultMat4( &l, &v1, &transform );

    GLfloat lightPosition0[] = { l.s.X, l.s.Y, l.s.Z, 0.0 };

    glLightfv( GL_LIGHT0, GL_AMBIENT,  ambientLight   );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  diffuseLight   );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specularLight  );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPosition0 );

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glShadeModel( GL_SMOOTH );

    glEnable( GL_COLOR_MATERIAL );
    glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specRef );
    glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 32 );

    Logger::getInstance()->printIfGLError( wxT( "Setup lights" ) );
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
void TheScene::drawSphere( float xPos, float yPos, float zPos, float ray )
{
    glPushMatrix();
    glTranslatef( xPos, yPos, zPos );

    GLUquadricObj *pQuadric = gluNewQuadric();
    gluQuadricNormals( pQuadric, GLU_SMOOTH );
    gluSphere( pQuadric, ray, 32, 32 );
    glPopMatrix();

    Logger::getInstance()->printIfGLError( wxT( "Draw sphere" ) );
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the selection objects that are present in the scene.
///////////////////////////////////////////////////////////////////////////
void TheScene::drawSelectionObjects()
{
    SelectionObjList selectionObjects = SceneManager::getInstance()->getSelectionObjects();
    for ( unsigned int i = 0; i < selectionObjects.size(); ++i )
    {
        for ( unsigned int j = 0; j < selectionObjects[i].size(); ++j )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );
            selectionObjects[i][j]->draw();
            glPopAttrib();
        }
    }

    Logger::getInstance()->printIfGLError( wxT( "Draw selection objects" ) );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void TheScene::drawColorMapLegend()
{
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();

    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glPushMatrix();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    int maxSize = std::max( std::max( rows, columns ), frames );
    glOrtho( 0, maxSize, 0, maxSize, -3000, 3000 );

    ShaderHelper::getInstance()->getLegendShader()->bind();
    ShaderHelper::getInstance()->getLegendShader()->setUniInt( "useColorMap", SceneManager::getInstance()->getColorMap() );

    glColor3f( 0.0f, 0.0f, 0.0f );
    glLineWidth( 5.0f );
    glBegin( GL_LINES );
    glTexCoord1f( 0.0 );
    glVertex3i( maxSize - 60, 10, 2900 );
    glTexCoord1f( 1.0 );
    glVertex3i( maxSize - 20, 10, 2900 );
    glEnd();

    ShaderHelper::getInstance()->getLegendShader()->release();

    glLineWidth( 1.0f );
    glColor3f( 0.0f, 0.0f, 0.0f );
    glBegin( GL_LINES );
    glVertex3i( maxSize - 60, 10, 2900 );
    glVertex3i( maxSize - 60, 12, 2900 );
    glVertex3i( maxSize - 50, 10, 2900 );
    glVertex3i( maxSize - 50, 12, 2900 );
    glVertex3i( maxSize - 40, 10, 2900 );
    glVertex3i( maxSize - 40, 12, 2900 );
    glVertex3i( maxSize - 30, 10, 2900 );
    glVertex3i( maxSize - 30, 12, 2900 );
    glVertex3i( maxSize - 20, 10, 2900 );
    glVertex3i( maxSize - 20, 12, 2900 );
    glEnd();

    ShaderHelper::getInstance()->getLegendShader()->release();

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

    for( int i = 0; i < MyApp::frame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( i ) );

        if( pInfo->getType() == VECTORS && pInfo->getShow() )
        {
            Anatomy* pVecs = (Anatomy*)pInfo;
            glLineWidth( 1.0 );
            glBegin( GL_LINES );

            float r, g, b, a;

            r = pVecs->getColor().Red()   / 255.;
            g = pVecs->getColor().Green() / 255.;
            b = pVecs->getColor().Blue()  / 255.;
            a = 1.0;

            float bright( 1.2f );
            float dull( 0.7f );

            int quadrant = SceneManager::getInstance()->getQuadrant();

            bool topview = quadrant == 2 || 
                           quadrant == 3 || 
                           quadrant == 6 || 
                           quadrant == 7;

            bool leftview = quadrant == 5 || 
                            quadrant == 6 ||
                            quadrant == 7 || 
                            quadrant == 8;

            bool frontview = quadrant == 3 || 
                             quadrant == 4 || 
                             quadrant == 5 || 
                             quadrant == 6;

            float columns = DatasetManager::getInstance()->getColumns();
            float rows    = DatasetManager::getInstance()->getRows();
            float frames  = DatasetManager::getInstance()->getFrames();

            if( SceneManager::getInstance()->isAxialDisplayed() )
            {
                float sliceZ = SceneManager::getInstance()->getSliceZ();

                for( int i = 0; i < columns; ++i )
                {
                    for( int j = 0; j < rows; ++j )
                    {
                        int slice = (int) ( sliceZ * columns * rows * 3 );
                        int index = i * 3 + j * columns * 3 + slice;

                        float x = pVecs->at( index );
                        float y = pVecs->at( index + 1 );
                        float z = pVecs->at( index + 2 );

                        if ( pVecs->getUseTex() )
                        {
                            r = wxMin(1.0, fabs(x) * bright);
                            g = wxMin(1.0, fabs(y) * bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt( r * r + g * g + b * b );
                            r /= a;
                            g /= a;
                            b /= a;
                        }

                        if( ! pVecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) i + .5f + x / 2.f, (GLfloat) j + .5f + y / 2.f, (GLfloat) sliceZ + .5f + z / 2.f );
                            glVertex3f( (GLfloat) i + .5f - x / 2.f, (GLfloat) j + .5f - y / 2.f, (GLfloat) sliceZ + .5f - z / 2.f );
                        }
                        else
                        {
                            if( pVecs->at( index + 2 ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if( topview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) sliceZ + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) sliceZ + .4 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) sliceZ + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) sliceZ + .4 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) sliceZ + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) sliceZ + .6 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) sliceZ + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) sliceZ + .6 );
                                }
                            }
                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if ( topview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) sliceZ + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) sliceZ + .4 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) sliceZ + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) sliceZ + .4 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2., (GLfloat) sliceZ + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) sliceZ + .6 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2., (GLfloat) sliceZ + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) sliceZ + .6 );
                                }
                            }
                        }
                    }
                }
            }

            if( SceneManager::getInstance()->isCoronalDisplayed() )
            {
                float sliceY = SceneManager::getInstance()->getSliceY();

                for( int i = 0; i < columns; ++i )
                {
                    for( int j = 0; j < frames; ++j )
                    {
                        int slice = (int) ( sliceY * columns * 3 );
                        int index = i * 3 + slice + j * columns * rows * 3;

                        float x = pVecs->at( index );
                        float y = pVecs->at( index + 1 );
                        float z = pVecs->at( index + 2 );

                        if( pVecs->getUseTex() )
                        {
                            r = wxMin( 1.0, fabs( x ) * bright );
                            g = wxMin( 1.0, fabs( y ) * bright );
                            b = wxMin( 1.0, fabs( z ) * bright );
                            a = sqrt( r * r + g * g + b * b );
                            r /= a;
                            g /= a;
                            b /= a;
                        }
                        if( ! pVecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) sliceY + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                            glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) sliceY + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                        }
                        else
                        {
                            if( pVecs->at( index + 1 ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if( frontview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) sliceY + .4, (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) sliceY + .4, (GLfloat) j + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) sliceY + .4, (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) sliceY + .4, (GLfloat) j + .5 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) sliceY + .6, (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) sliceY + .6, (GLfloat) j + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) sliceY + .6, (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) sliceY + .6, (GLfloat) j + .5 );
                                }
                            }

                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if ( frontview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) sliceY + .4, (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) sliceY + .4, (GLfloat) j + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) sliceY + .4, (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) sliceY + .4, (GLfloat) j + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) sliceY + .6, (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) sliceY + .6, (GLfloat) j + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) sliceY + .6, (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) sliceY + .6, (GLfloat) j + .5 );
                                }
                            }
                        }
                    }
                }
            }

            if( SceneManager::getInstance()->isSagittalDisplayed() )
            {
                float sliceX = SceneManager::getInstance()->getSliceX();

                for( int i = 0; i < rows; ++i )
                {
                    for( int j = 0; j < frames; ++j )
                    {
                        int slice = (int) ( sliceX * 3 );
                        int index = slice + i * columns * 3 + j * columns * rows * 3;

                        float x = pVecs->at( index );
                        float y = pVecs->at( index + 1 );
                        float z = pVecs->at( index + 2 );

                        if( pVecs->getUseTex() )
                        {
                            r = wxMin(1.0, fabs(x)* bright);
                            g = wxMin(1.0, fabs(y)* bright);
                            b = wxMin(1.0, fabs(z)* bright);
                            a = sqrt( r * r + g * g + b * b );
                            r /= a;
                            g /= a;
                            b /= a;
                        }
                        if( ! pVecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) sliceX + .5 + x / 2., (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                            glVertex3f( (GLfloat) sliceX + .5 - x / 2., (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                        }
                        else
                        {
                            if( pVecs->at( index ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if ( leftview )
                                {
                                    glVertex3f( (GLfloat) sliceX + .4, (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) sliceX + .4, (GLfloat) i + .5, (GLfloat) j + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) sliceX + .4, (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) sliceX + .4, (GLfloat) i + .5, (GLfloat) j + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) sliceX + .6, (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) sliceX + .6, (GLfloat) i + .5, (GLfloat) j + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) sliceX + .6, (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) sliceX + .6, (GLfloat) i + .5, (GLfloat) j + .5 );
                                }
                            }
                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if( leftview )
                                {
                                    glVertex3f( (GLfloat) sliceX + .4, (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) sliceX + .4, (GLfloat) i + .5, (GLfloat) j + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) sliceX + .4, (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) sliceX + .4, (GLfloat) i + .5, (GLfloat) j + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) sliceX + .6, (GLfloat) i + .5 - y / 2., (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) sliceX + .6, (GLfloat) i + .5, (GLfloat) j + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) sliceX + .6, (GLfloat) i + .5 + y / 2., (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) sliceX + .6, (GLfloat) i + .5, (GLfloat) j + .5 );
                                }
                            }
                        }
                    }
                }
            }

            for( int j = 0; j < MyApp::frame->m_pListCtrl->GetItemCount(); ++j )
            {
                DatasetInfo* pMesh = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( j ) );

                if ( pMesh->getType() == ISO_SURFACE && pMesh->getShow() )
                {
                    CIsoSurface* pSurf = (CIsoSurface*) pMesh;
                    drawVectorsHelper( pSurf->getSurfaceVoxelPositions(), pVecs, bright );
                }
            }
            glEnd();
        }
    }

    Logger::getInstance()->printIfGLError( wxT( "Draw vectors" ) );

    glDisable( GL_BLEND );

    glPopAttrib();
}
