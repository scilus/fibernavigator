/*
 * AnatomyHelper.cpp
 *
 *  Created on: 23.07.2008
 *      Author: ralph
 */
#include "AnatomyHelper.h"

#include "DatasetManager.h"
#include "../gui/SceneManager.h"

#include <GL/glew.h>

#include <algorithm>
#include <vector>
using std::vector;

///////////////////////////////////////////////////////////////////////////
// Constructor
AnatomyHelper::AnatomyHelper( DatasetHelper* l_datasetHelper )
:   m_datasetHelper(l_datasetHelper)
{

}

///////////////////////////////////////////////////////////////////////////
// Destructor
AnatomyHelper::~AnatomyHelper() 
{
    // TODO Auto-generated destructor stub
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderNav( int pView, ShaderProgram *pShader )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    m_datasetHelper->m_theScene->bindTextures();
    pShader->bind();
    m_datasetHelper->m_shaderHelper->initializeArrays();
    m_datasetHelper->m_shaderHelper->setTextureShaderVars();

    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.0001f );

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

            xLine = max - SceneManager::getInstance()->getSliceY() * voxelY;
            yLine = SceneManager::getInstance()->getSliceZ() * voxelZ + zo;
            break;
        }
    }

    glDisable( GL_TEXTURE_3D );
    pShader->release();
    glPopAttrib();

    glLineWidth( 1.0f );
    glColor3f( 1.0f, 0.0f, 0.0f );
    glBegin( GL_LINES );
        glVertex3f( 0    , yLine, lineZ );
        glVertex3f( max  , yLine, lineZ );
        glVertex3f( xLine, 0    , lineZ );
        glVertex3f( xLine, max  , lineZ );
    glEnd();

    glColor3f( 1.0f, 1.0f, 1.0f );
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

    m_x = ( SceneManager::getInstance()->getSliceX() + 0.5f ) * voxelX;
    m_y = ( SceneManager::getInstance()->getSliceY() + 0.5f ) * voxelY;
    m_z = ( SceneManager::getInstance()->getSliceZ() + 0.5f ) * voxelZ;

    // m_xc, m_yc and m_zc will yield a number between 0 and 1.
    m_xc = ( SceneManager::getInstance()->getSliceX() + 0.5f ) / columns;
    m_yc = ( SceneManager::getInstance()->getSliceY() + 0.5f ) / rows;
    m_zc = ( SceneManager::getInstance()->getSliceZ() + 0.5f ) / frames;

    m_xb = columns * voxelX;
    m_yb = rows    * voxelY;
    m_zb = frames  * voxelZ;

#if 1
    renderAxial();
    renderCoronal();
    renderSagittal();
#else
    switch( m_datasetHelper->quadrant )
    {
    case 1:
        renderS1();
        renderC1();
        renderA1();
        renderC2();
        renderA2();
        renderS3();
        renderS2();
        renderS4();
        renderA3();
        renderC3();
        renderC4();
        renderA4();
        break;
    case 2:
        renderS2();
        renderC2();
        renderA1();
        renderA2();
        renderC1();
        renderS3();
        renderS1();
        renderS4();
        renderC4();
        renderA3();
        renderA4();
        renderC3();
        break;
    case 3:
        renderS4();
        renderC2();
        renderA2();
        renderS2();
        renderA1();
        renderC1();
        renderS1();
        renderS3();
        renderC4();
        renderA4();
        renderA3();
        renderC3();
        break;
    case 4:
        renderA2();
        renderC1();
        renderS3();
        renderS1();
        renderA1();
        renderC2();
        renderS2();
        renderS4();
        renderA4();
        renderC3();
        renderC4();
        renderA3();
        break;
    case 5:
        renderA4();
        renderS3();
        renderC3();
        renderS1();
        renderA3();
        renderC4();
        renderS4();
        renderS2();
        renderA2();
        renderC1();
        renderC2();
        renderA1();
        break;
    case 6:
        renderC4();
        renderS4();
        renderA4();
        renderS2();
        renderA3();
        renderC3();
        renderS1();
        renderS3();
        renderC2();
        renderA1();
        renderA2();
        renderC1();
        break;
    case 7:
        renderA3();
        renderC4();
        renderS2();
        renderA4();
        renderC3();
        renderS3();
        renderS4();
        renderS1();
        renderC2();
        renderA1();
        renderA2();
        renderC1();
        break;
    case 8:
        renderS1();
        renderC3();
        renderA3();
        renderC4();
        renderA4();
        renderS3();
        renderS2();
        renderS4();
        renderA1();
        renderC1();
        renderC2();
        renderA2();
        break;
    }
#endif
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
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderA1()
{
    if( !SceneManager::getInstance()->isAxialDisplayed() ) 
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( 0.0,  0.0,  m_zc); glVertex3f( 0,   0,   m_z + 0.5f );
        glTexCoord3f( 0.0,  m_yc, m_zc); glVertex3f( 0,   m_y, m_z + 0.5f );
        glTexCoord3f( m_xc, m_yc, m_zc); glVertex3f( m_x, m_y, m_z + 0.5f );
        glTexCoord3f( m_xc, 0.0,  m_zc); glVertex3f( m_x, 0,   m_z + 0.5f );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderA2()
{
    if( !SceneManager::getInstance()->isAxialDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( 0.0,  m_yc, m_zc); glVertex3f( 0,   m_y,  m_z + 0.5f );
        glTexCoord3f( 0.0,  1.0,  m_zc); glVertex3f( 0,   m_yb, m_z + 0.5f );
        glTexCoord3f( m_xc, 1.0,  m_zc); glVertex3f( m_x, m_yb, m_z + 0.5f );
        glTexCoord3f( m_xc, m_yc, m_zc); glVertex3f( m_x, m_y,  m_z + 0.5f );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderA3()
{
    if( !SceneManager::getInstance()->isAxialDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, 0,    m_zc ); glVertex3f( m_x,  0,   m_z + 0.5f );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x,  m_y, m_z + 0.5f );
        glTexCoord3f( 1.0,  m_yc, m_zc ); glVertex3f( m_xb, m_y, m_z + 0.5f );
        glTexCoord3f( 1.0,  0,    m_zc ); glVertex3f( m_xb, 0,   m_z + 0.5f );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderA4()
{
    if( !SceneManager::getInstance()->isAxialDisplayed() )
        return;
    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x,  m_y,  m_z + 0.5f );
        glTexCoord3f( m_xc, 1.0,  m_zc ); glVertex3f( m_x,  m_yb, m_z + 0.5f );
        glTexCoord3f( 1.0,  1.0,  m_zc ); glVertex3f( m_xb, m_yb, m_z + 0.5f );
        glTexCoord3f( 1.0,  m_yc, m_zc ); glVertex3f( m_xb, m_y,  m_z + 0.5f );
    glEnd();

}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderC1()
{
    if( !SceneManager::getInstance()->isCoronalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( 0.0,  m_yc, 0.0  ); glVertex3f( 0,   m_y + 0.5f, 0   );
        glTexCoord3f( 0.0,  m_yc, m_zc ); glVertex3f( 0,   m_y + 0.5f, m_z );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x, m_y + 0.5f, m_z );
        glTexCoord3f( m_xc, m_yc, 0.0  ); glVertex3f( m_x, m_y + 0.5f, 0   );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderC2()
{
    if( !SceneManager::getInstance()->isCoronalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( 0.0,  m_yc, m_zc ); glVertex3f( 0,   m_y + 0.5f, m_z  );
        glTexCoord3f( 0.0,  m_yc, 1.0  ); glVertex3f( 0,   m_y + 0.5f, m_zb );
        glTexCoord3f( m_xc, m_yc, 1.0  ); glVertex3f( m_x, m_y + 0.5f, m_zb );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x, m_y + 0.5f, m_z  );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderC3()
{
    if( !SceneManager::getInstance()->isCoronalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, m_yc, 0    ); glVertex3f( m_x,  m_y + 0.5f, 0   );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x,  m_y + 0.5f, m_z );
        glTexCoord3f( 1.0,  m_yc, m_zc ); glVertex3f( m_xb, m_y + 0.5f, m_z );
        glTexCoord3f( 1.0,  m_yc, 0    ); glVertex3f( m_xb, m_y + 0.5f, 0   );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderC4()
{
    if( !SceneManager::getInstance()->isCoronalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x,  m_y + 0.5f, m_z  );
        glTexCoord3f( m_xc, m_yc, 1.0  ); glVertex3f( m_x,  m_y + 0.5f, m_zb );
        glTexCoord3f( 1.0,  m_yc, 1.0  ); glVertex3f( m_xb, m_y + 0.5f, m_zb );
        glTexCoord3f( 1.0,  m_yc, m_zc ); glVertex3f( m_xb, m_y + 0.5f, m_z  );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderS1()
{
    if( !SceneManager::getInstance()->isSagittalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, 0.0,  0.0  ); glVertex3f( m_x + 0.5f, 0,   0   );
        glTexCoord3f( m_xc, 0.0,  m_zc ); glVertex3f( m_x + 0.5f, 0,   m_z );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x + 0.5f, m_y, m_z );
        glTexCoord3f( m_xc, m_yc, 0.0  ); glVertex3f( m_x + 0.5f, m_y, 0   );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderS2()
{
    if( !SceneManager::getInstance()->isSagittalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, 0.0,  m_zc ); glVertex3f( m_x + 0.5f, 0,   m_z  );
        glTexCoord3f( m_xc, 0.0,  1.0  ); glVertex3f( m_x + 0.5f, 0,   m_zb );
        glTexCoord3f( m_xc, m_yc, 1.0  ); glVertex3f( m_x + 0.5f, m_y, m_zb );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x + 0.5f, m_y, m_z  );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderS3()
{
    if( !SceneManager::getInstance()->isSagittalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, m_yc, 0    ); glVertex3f( m_x + 0.5f, m_y,  0   );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x + 0.5f, m_y,  m_z );
        glTexCoord3f( m_xc, 1.0,  m_zc ); glVertex3f( m_x + 0.5f, m_yb, m_z );
        glTexCoord3f( m_xc, 1.0,  0    ); glVertex3f( m_x + 0.5f, m_yb, 0   );
    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderS4()
{
    if( !SceneManager::getInstance()->isSagittalDisplayed() )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x + 0.5f, m_y,  m_z  );
        glTexCoord3f( m_xc, m_yc, 1.0  ); glVertex3f( m_x + 0.5f, m_y,  m_zb );
        glTexCoord3f( m_xc, 1.0,  1.0  ); glVertex3f( m_x + 0.5f, m_yb, m_zb );
        glTexCoord3f( m_xc, 1.0,  m_zc ); glVertex3f( m_x + 0.5f, m_yb, m_z  );
    glEnd();
}
