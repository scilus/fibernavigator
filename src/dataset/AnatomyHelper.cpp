/*
 * AnatomyHelper.cpp
 *
 *  Created on: 23.07.2008
 *      Author: ralph
 */
#include "AnatomyHelper.h"

#include "DatasetManager.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gui/MainFrame.h"
#include "../gui/SceneManager.h"

#include <GL/glew.h>

#include <algorithm>
#include <vector>
using std::vector;

///////////////////////////////////////////////////////////////////////////
// Constructor
AnatomyHelper::AnatomyHelper()
{
}

///////////////////////////////////////////////////////////////////////////
// Destructor
AnatomyHelper::~AnatomyHelper() 
{
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderNav( int pView )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - glPushAttrib") );

    SceneManager::getInstance()->getScene()->bindTextures();
    ShaderHelper::getInstance()->getAnatomyShader()->bind();
    ShaderHelper::getInstance()->initializeArrays();
    ShaderHelper::getInstance()->setTextureShaderVars();

    glEnable( GL_ALPHA_TEST );
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - glEnable") );
    glAlphaFunc( GL_GREATER, 0.0001f );
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - glAlphaFunc") );

    float xLine( 0.0f );
    float yLine( 0.0f );

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    float x = columns * voxelX;
    float y = rows    * voxelY;
    float z = frames  * voxelZ;

    float max = std::max( x, std::max( y, z ) );

    float xo = ( max - x ) / 2.0f;
    float yo = ( max - y ) / 2.0f;
    float zo = ( max - z ) / 2.0f;

    float quadZ = -0.1f;
    float lineZ = 0.0f;

    switch( pView )
    {
        case AXIAL: 
        {
            float sliceZ = SceneManager::getInstance()->getSliceZ();
            glBegin( GL_QUADS );
                glTexCoord3f( 0.0f, 0.0f, ( sliceZ + 0.5f ) / frames ); glVertex3f( 0 + xo, 0 + yo, quadZ );
                glTexCoord3f( 1.0f, 0.0f, ( sliceZ + 0.5f ) / frames ); glVertex3f( x + xo, 0 + yo, quadZ );
                glTexCoord3f( 1.0f, 1.0f, ( sliceZ + 0.5f ) / frames ); glVertex3f( x + xo, y + yo, quadZ );
                glTexCoord3f( 0.0f, 1.0f, ( sliceZ + 0.5f ) / frames ); glVertex3f( 0 + xo, y + yo, quadZ );
            glEnd();
            Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - AXIAL: glBegin") );

            xLine = SceneManager::getInstance()->getSliceX() * voxelX + xo;
            yLine = SceneManager::getInstance()->getSliceY() * voxelY + yo;
            break;
        }

        case CORONAL: 
        {
            float sliceY = SceneManager::getInstance()->getSliceY();
            glBegin( GL_QUADS );
                glTexCoord3f( 0.0f, ( sliceY + 0.5f ) / rows, 0.0f ); glVertex3f( 0 + xo, 0 + zo, quadZ );
                glTexCoord3f( 0.0f, ( sliceY + 0.5f ) / rows, 1.0f ); glVertex3f( 0 + xo, z + zo, quadZ );
                glTexCoord3f( 1.0f, ( sliceY + 0.5f ) / rows, 1.0f ); glVertex3f( x + xo, z + zo, quadZ );
                glTexCoord3f( 1.0f, ( sliceY + 0.5f ) / rows, 0.0f ); glVertex3f( x + xo, 0 + zo, quadZ );
            glEnd();
            Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - CORONAL: glBegin") );

            xLine = SceneManager::getInstance()->getSliceX() * voxelX + xo;
            yLine = SceneManager::getInstance()->getSliceZ() * voxelZ + zo;
            break;
        }

        case SAGITTAL: 
        {
            float sliceX = SceneManager::getInstance()->getSliceX();
            glBegin( GL_QUADS );
                glTexCoord3f( ( sliceX + 0.5f ) / columns, 1.0f, 0.0f ); glVertex3f( 0 + yo, 0 + zo, quadZ );
                glTexCoord3f( ( sliceX + 0.5f ) / columns, 1.0f, 1.0f ); glVertex3f( 0 + yo, z + zo, quadZ );
                glTexCoord3f( ( sliceX + 0.5f ) / columns, 0.0f, 1.0f ); glVertex3f( y + yo, z + zo, quadZ );
                glTexCoord3f( ( sliceX + 0.5f ) / columns, 0.0f, 0.0f ); glVertex3f( y + yo, 0 + zo, quadZ );
            glEnd();
            Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - SAGITTAL: glBegin") );

            xLine = max - SceneManager::getInstance()->getSliceY() * voxelY;
            yLine = SceneManager::getInstance()->getSliceZ() * voxelZ + zo;
            break;
        }
    }

    glDisable( GL_TEXTURE_3D );
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - glDisable") );
    ShaderHelper::getInstance()->getAnatomyShader()->release();
    glPopAttrib();
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - glPopAttrib") );

    glLineWidth( 1.0f );
    glColor3f( 1.0f, 0.0f, 0.0f );
    glBegin( GL_LINES );
        glVertex3f( 0    , yLine, lineZ );
        glVertex3f( max  , yLine, lineZ );
        glVertex3f( xLine, 0    , lineZ );
        glVertex3f( xLine, max  , lineZ );
    glEnd();

    glColor3f( 1.0f, 1.0f, 1.0f );
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderNav - glBegin") );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderMain()
{
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();
    float sliceX  = SceneManager::getInstance()->getSliceX();
    float sliceY  = SceneManager::getInstance()->getSliceY();
    float sliceZ  = SceneManager::getInstance()->getSliceZ();

    m_x = ( sliceX + 0.5f ) * voxelX;
    m_y = ( sliceY + 0.5f ) * voxelY;
    m_z = ( sliceZ + 0.5f ) * voxelZ;

    // m_xc, m_yc and m_zc will yield a number between 0 and 1.
    m_xc = ( sliceX + 0.5f ) / columns;
    m_yc = ( sliceY + 0.5f ) / rows;
    m_zc = ( sliceZ + 0.5f ) / frames;

    m_xb = columns * voxelX;
    m_yb = rows    * voxelY;
    m_zb = frames  * voxelZ;

    renderAxial();
    renderCoronal();
    renderSagittal();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderCrosshair()
{
    float offset = 0.02f;
    glLineWidth( 1.0 );

    glBegin( GL_LINES );
        glVertex3f( m_x - offset, m_y - offset,  0    );
        glVertex3f( m_x - offset, m_y - offset,  m_zb );
        glVertex3f( m_x - offset, m_y + offset,  0    );
        glVertex3f( m_x - offset, m_y + offset,  m_zb );
        glVertex3f( m_x + offset, m_y + offset,  0    );
        glVertex3f( m_x + offset, m_y + offset,  m_zb );
        glVertex3f( m_x + offset, m_y - offset,  0    );
        glVertex3f( m_x + offset, m_y - offset,  m_zb );

        glVertex3f( m_x - offset, 0,        m_z - offset );
        glVertex3f( m_x - offset, m_yb,  m_z - offset );
        glVertex3f( m_x - offset, 0,        m_z + offset );
        glVertex3f( m_x - offset, m_yb,  m_z + offset );
        glVertex3f( m_x + offset, 0,        m_z + offset );
        glVertex3f( m_x + offset, m_yb,  m_z + offset );
        glVertex3f( m_x + offset, 0,        m_z - offset );
        glVertex3f( m_x + offset, m_yb,  m_z - offset );

        glVertex3f( 0,    m_y - offset, m_z - offset );
        glVertex3f( m_xb, m_y - offset, m_z - offset );
        glVertex3f( 0,    m_y - offset, m_z + offset );
        glVertex3f( m_xb, m_y - offset, m_z + offset );
        glVertex3f( 0,    m_y + offset, m_z + offset );
        glVertex3f( m_xb, m_y + offset, m_z + offset );
        glVertex3f( 0,    m_y + offset, m_z - offset );
        glVertex3f( m_xb, m_y + offset, m_z - offset );

    glEnd();
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderCrosshair") );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderAxial()
{
    if( !SceneManager::getInstance()->isAxialDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( 0.0, 0.0, m_zc ); glVertex3f( 0,    0,    m_z );
        glTexCoord3f( 0.0, 1.0, m_zc ); glVertex3f( 0,    m_yb, m_z );
        glTexCoord3f( 1.0, 1.0, m_zc ); glVertex3f( m_xb, m_yb, m_z );
        glTexCoord3f( 1.0, 0.0, m_zc ); glVertex3f( m_xb, 0,    m_z );
    glEnd();
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderAxial") );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderCoronal()
{
    if( !SceneManager::getInstance()->isCoronalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( 0.0, m_yc, 0.0 ); glVertex3f( 0,    m_y, 0    );
        glTexCoord3f( 0.0, m_yc, 1.0 ); glVertex3f( 0,    m_y, m_zb );
        glTexCoord3f( 1.0, m_yc, 1.0 ); glVertex3f( m_xb, m_y, m_zb );
        glTexCoord3f( 1.0, m_yc, 0.0 ); glVertex3f( m_xb, m_y, 0    );
    glEnd();
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderCoronal") );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderSagittal()
{
    if( !SceneManager::getInstance()->isSagittalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, 0.0, 0.0); glVertex3f( m_x, 0,    0    );
        glTexCoord3f( m_xc, 0.0, 1.0); glVertex3f( m_x, 0,    m_zb );
        glTexCoord3f( m_xc, 1.0, 1.0); glVertex3f( m_x, m_yb, m_zb );
        glTexCoord3f( m_xc, 1.0, 0.0); glVertex3f( m_x, m_yb, 0    );
    glEnd();
    Logger::getInstance()->printIfGLError( wxT( "AnatomyHelper::renderSagittal") );
}
