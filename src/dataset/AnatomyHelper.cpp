/*
 * AnatomyHelper.cpp
 *
 *  Created on: 23.07.2008
 *      Author: ralph
 */
#include "AnatomyHelper.h"

#include <GL/glew.h>


///////////////////////////////////////////////////////////////////////////
// Constructor
AnatomyHelper::AnatomyHelper( DatasetHelper* l_datasetHelper )
    : m_datasetHelper(l_datasetHelper)
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
void AnatomyHelper::renderNav( int i_view, Shader* i_shader )
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    m_datasetHelper->m_theScene->bindTextures();
    i_shader->bind();
    m_datasetHelper->m_shaderHelper->initializeArrays();
    m_datasetHelper->m_shaderHelper->setTextureShaderVars();

    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.0001f );

    float l_xLine = 0;
    float l_yLine = 0;

    float l_max = (float)wxMax( m_datasetHelper->m_columns * m_datasetHelper->m_xVoxel, 
                         wxMax( m_datasetHelper->m_rows    * m_datasetHelper->m_yVoxel, m_datasetHelper->m_frames * m_datasetHelper->m_zVoxel ) );

    float l_x = m_datasetHelper->m_columns * m_datasetHelper->m_xVoxel;
    float l_y = m_datasetHelper->m_rows    * m_datasetHelper->m_yVoxel;
    float l_z = m_datasetHelper->m_frames  * m_datasetHelper->m_zVoxel;

    float l_xo = ( l_max - l_x ) / 2.0;
    float l_yo = ( l_max - l_y ) / 2.0;
    float l_zo = ( l_max - l_z ) / 2.0;

    float l_quadZ = -0.1f;
    float l_lineZ = 0.0f;

    switch(i_view )
    {
        case AXIAL: 
         {
            glBegin( GL_QUADS );
                glTexCoord3f( 0.0, 0.0, ( (float)m_datasetHelper->m_zSlize + 0.5f) / (float)m_datasetHelper->m_frames ); glVertex3f( 0   + l_xo, 0   + l_yo, l_quadZ );
                glTexCoord3f( 1.0, 0.0, ( (float)m_datasetHelper->m_zSlize + 0.5f) / (float)m_datasetHelper->m_frames ); glVertex3f( l_x + l_xo, 0   + l_yo, l_quadZ );
                glTexCoord3f( 1.0, 1.0, ( (float)m_datasetHelper->m_zSlize + 0.5f) / (float)m_datasetHelper->m_frames ); glVertex3f( l_x + l_xo, l_y + l_yo, l_quadZ );
                glTexCoord3f( 0.0, 1.0, ( (float)m_datasetHelper->m_zSlize + 0.5f) / (float)m_datasetHelper->m_frames ); glVertex3f( 0   + l_xo, l_y + l_yo, l_quadZ );
            glEnd();

            l_xLine = m_datasetHelper->m_xSlize * m_datasetHelper->m_xVoxel + l_xo;
            l_yLine = m_datasetHelper->m_ySlize * m_datasetHelper->m_yVoxel + l_yo;
        } break;

        case CORONAL: 
        {
            glBegin( GL_QUADS );
                glTexCoord3f( 0.0, ( (float)m_datasetHelper->m_ySlize + 0.5f ) / (float)m_datasetHelper->m_rows, 0.0 ); glVertex3f( 0   + l_xo, 0   + l_zo, l_quadZ );
                glTexCoord3f( 0.0, ( (float)m_datasetHelper->m_ySlize + 0.5f ) / (float)m_datasetHelper->m_rows, 1.0 ); glVertex3f( 0   + l_xo, l_z + l_zo, l_quadZ );
                glTexCoord3f( 1.0, ( (float)m_datasetHelper->m_ySlize + 0.5f ) / (float)m_datasetHelper->m_rows, 1.0 ); glVertex3f( l_x + l_xo, l_z + l_zo, l_quadZ );
                glTexCoord3f( 1.0, ( (float)m_datasetHelper->m_ySlize + 0.5f ) / (float)m_datasetHelper->m_rows, 0.0 ); glVertex3f( l_x + l_xo, 0   + l_zo, l_quadZ );
            glEnd();

            l_xLine = m_datasetHelper->m_xSlize * m_datasetHelper->m_xVoxel + l_xo;
            l_yLine = m_datasetHelper->m_zSlize * m_datasetHelper->m_zVoxel + l_zo;
        } break;

        case SAGITTAL: 
        {
            glBegin( GL_QUADS );
                glTexCoord3f( ( (float)m_datasetHelper->m_xSlize + 0.5f ) / (float)m_datasetHelper->m_columns, 1.0, 0.0 ); glVertex3f( 0   + l_yo, 0   + l_zo, l_quadZ );
                glTexCoord3f( ( (float)m_datasetHelper->m_xSlize + 0.5f ) / (float)m_datasetHelper->m_columns, 1.0, 1.0 ); glVertex3f( 0   + l_yo, l_z + l_zo, l_quadZ );
                glTexCoord3f( ( (float)m_datasetHelper->m_xSlize + 0.5f ) / (float)m_datasetHelper->m_columns, 0.0, 1.0 ); glVertex3f( l_y + l_yo, l_z + l_zo, l_quadZ );
                glTexCoord3f( ( (float)m_datasetHelper->m_xSlize + 0.5f ) / (float)m_datasetHelper->m_columns, 0.0, 0.0 ); glVertex3f( l_y + l_yo, 0 +   l_zo, l_quadZ );
            glEnd();

            l_xLine = l_max - m_datasetHelper->m_ySlize * m_datasetHelper->m_yVoxel;
            l_yLine = m_datasetHelper->m_zSlize * m_datasetHelper->m_zVoxel + l_zo;
        } break;
    }

    glDisable( GL_TEXTURE_3D );
    i_shader->release();
    glPopAttrib();

    glLineWidth( 1.0 );
    glColor3f( 1.0, 0.0, 0.0 );
    glBegin( GL_LINES );
        glVertex3f( 0      , l_yLine, l_lineZ );
        glVertex3f( l_max  , l_yLine, l_lineZ );
        glVertex3f( l_xLine, 0      , l_lineZ );
        glVertex3f( l_xLine, l_max  , l_lineZ );
    glEnd();

    glColor3f( 1.0, 1.0, 1.0 );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
void AnatomyHelper::renderMain()
{
    m_x = ( m_datasetHelper->m_xSlize * m_datasetHelper->m_xVoxel ) + 0.5f * m_datasetHelper->m_xVoxel;
    m_y = ( m_datasetHelper->m_ySlize * m_datasetHelper->m_yVoxel ) + 0.5f * m_datasetHelper->m_yVoxel;
    m_z = ( m_datasetHelper->m_zSlize * m_datasetHelper->m_zVoxel ) + 0.5f * m_datasetHelper->m_zVoxel;

    // m_xc, m_yc and m_zc will yield a number between 0 and 1.
    m_xc = ( (float)m_datasetHelper->m_xSlize + 0.5f ) / (float)m_datasetHelper->m_columns;
    m_yc = ( (float)m_datasetHelper->m_ySlize + 0.5f ) / (float)m_datasetHelper->m_rows;
    m_zc = ( (float)m_datasetHelper->m_zSlize + 0.5f ) / (float)m_datasetHelper->m_frames;

    m_xb = m_datasetHelper->m_columns * m_datasetHelper->m_xVoxel ;
    m_yb = m_datasetHelper->m_rows    * m_datasetHelper->m_yVoxel ;
    m_zb = m_datasetHelper->m_frames  * m_datasetHelper->m_zVoxel ;
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
    if( ! m_datasetHelper->m_showAxial )
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
    if( ! m_datasetHelper->m_showCoronal )
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
    if( ! m_datasetHelper->m_showSagittal )
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
    if( ! m_datasetHelper->m_showAxial ) 
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
    if( ! m_datasetHelper->m_showAxial )
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
    if( ! m_datasetHelper->m_showAxial)
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
    if( ! m_datasetHelper->m_showAxial)
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
    if( ! m_datasetHelper->m_showCoronal )
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
    if( ! m_datasetHelper->m_showCoronal )
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
    if( ! m_datasetHelper->m_showCoronal)
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
    if( ! m_datasetHelper->m_showCoronal)
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
    if( ! m_datasetHelper->m_showSagittal )
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
    if( ! m_datasetHelper->m_showSagittal )
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
    if( ! m_datasetHelper->m_showSagittal )
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
    if( ! m_datasetHelper->m_showSagittal )
        return;

    glBegin( GL_QUADS );
        glTexCoord3f( m_xc, m_yc, m_zc ); glVertex3f( m_x + 0.5f, m_y,  m_z  );
        glTexCoord3f( m_xc, m_yc, 1.0  ); glVertex3f( m_x + 0.5f, m_y,  m_zb );
        glTexCoord3f( m_xc, 1.0,  1.0  ); glVertex3f( m_x + 0.5f, m_yb, m_zb );
        glTexCoord3f( m_xc, 1.0,  m_zc ); glVertex3f( m_x + 0.5f, m_yb, m_z  );
    glEnd();
}
