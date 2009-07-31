#include "theScene.h"
#include "../gui/myListCtrl.h"
#include "../dataset/splinePoint.h"
#include "../dataset/fibers.h"
#include "../dataset/surface.h"
#include "../gui/selectionBox.h"
#include "../dataset/AnatomyHelper.h"
#include "../dataset/Anatomy.h"
#include "../misc/IsoSurface/CIsoSurface.h"

TheScene::TheScene( DatasetHelper* dh ) :
    m_dh( dh ), m_mainGLContext( 0 )
{
    m_dh->anatomyHelper = new AnatomyHelper( m_dh );
}

TheScene::~TheScene()
{
    m_dh->printDebug( _T("execute scene destructor"), 0 );

#ifndef __WXMAC__
    // on mac, this is just a pointer to the original object that is deleted with the widgets
    if ( m_mainGLContext )
        delete m_mainGLContext;
#endif
    m_dh->printDebug( _T("scene destructor done"), 0 );

}

void TheScene::initGL( int view )
{
    try
    {
        GLenum err = glewInit();
        if ( GLEW_OK != err )
        {
            /* Problem: glewInit failed, something is seriously wrong. */
            m_dh->printDebug( _T("Error: ") + wxString::FromAscii( (char*) glewGetErrorString( err ) ), 2 );
            exit( false );
        }
        if ( view == mainView )
        {
            m_dh->printDebug( _T("Status: Using GLEW ") + wxString::FromAscii( (char*) glewGetString(
                    GLEW_VERSION ) ), 1 );

            wxString vendor;
            wxString renderer;
            vendor = wxString::FromAscii( (char*) glGetString( GL_VENDOR ) );
            renderer = wxString::FromAscii( (char*) glGetString( GL_RENDERER ) );

            if ( renderer.Contains( _T("GeForce 6") ) )
            {
                m_dh->geforceLevel = 6;
            }
            else if ( renderer.Contains( _T("GeForce 7") ) )
            {
                m_dh->geforceLevel = 7;
            }
            else if ( renderer.Contains( _T("GeForce 8") ) || renderer.Contains( _T("GeForce GTX 2") ) )
            {
                m_dh->geforceLevel = 8;
            }

            m_dh->printDebug( vendor + _T(" ") + renderer, 1 );

            if ( !glewIsSupported( "GL_ARB_shader_objects" ) )
            {
                printf( "*** ERROR no support for shader objects found.\n" );
                printf( "*** Please check your OpenGL installation...exiting.\n" );
                exit( false );
            }
        }
        glEnable( GL_DEPTH_TEST );

        if ( !m_dh->m_texAssigned )
        {
            m_dh->shaderHelper = new ShaderHelper( m_dh );
            m_dh->m_texAssigned = true;
        }

        //float maxLength = (float)wxMax(m_dh->columns, wxMax(m_dh->rows, m_dh->frames));
        //float view1 = maxLength;
        float view1 = 200;
        glClearColor( 1.0, 1.0, 1.0, 0.0 );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0, view1, 0, view1, -3000, 3000 );
    } catch ( ... )
    {
        if ( m_dh->GLError() )
            m_dh->printGLError( wxT("init") );
    }
}

void TheScene::bindTextures()
{
    //glEnable(GL_TEXTURE_3D);
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    int c = 0;

    for ( int i = 0; i < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData( i );
        if ( info->getType() < Mesh_ && info->getShow() )
        {
            glActiveTexture( GL_TEXTURE0 + c );
            glBindTexture( GL_TEXTURE_3D, info->getGLuint() );
            if ( info->getShowFS() )
            {
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            }
            else
            {
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
                glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            }
            if ( ++c == 10 )
            {
                printf( "reached 10 textures\n" );
                break;
            }
        }

    }
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("bind textures") );
}

void TheScene::renderScene()
{
    if ( m_dh->mainFrame->m_listCtrl->GetItemCount() == 0 )
        return;

    m_dh->shaderHelper->initializeArrays();

    renderSlizes();

    if ( m_dh->surface_loaded )
        renderSplineSurface();

    if ( m_dh->pointMode )
        drawPoints();

    if ( m_dh->fibers_loaded )
    {
        if ( m_dh->useFakeTubes )
            renderFakeTubes();
        else
            renderFibers();
    }

    if ( m_dh->mesh_loaded )
        renderMesh();

    if ( m_dh->vectors_loaded )
        drawVectors();

    if ( m_dh->fibers_loaded && m_dh->showBoxes )
        drawSelectionBoxes();

    if ( m_dh->showColorMapLegend )
        drawColorMapLegend();

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("render scene") );
}

void TheScene::renderSlizes()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    if ( m_dh->blendAlpha )
        glDisable( GL_ALPHA_TEST );
    //glDisable(GL_BLEND);
    else
        glEnable( GL_ALPHA_TEST );
    //glEnable(GL_BLEND);

    glAlphaFunc( GL_GREATER, 0.001f ); // adjust your prefered threshold here

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bindTextures();
    m_dh->shaderHelper->m_textureShader->bind();
    m_dh->shaderHelper->setTextureShaderVars();
    m_dh->shaderHelper->m_textureShader->setUniInt( "useColorMap", m_dh->colorMap );

    m_dh->anatomyHelper->renderMain();

    glDisable( GL_BLEND );

    m_dh->shaderHelper->m_textureShader->release();

    if ( m_dh->showCrosshair )
        m_dh->anatomyHelper->renderCrosshair();

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("render slizes") );

    glPopAttrib();
}

void TheScene::renderSplineSurface()
{
    for ( int i = 0; i < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData( i );
        if ( info->getType() == Surface_ && info->getShow() )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );

            if ( m_dh->pointMode )
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            else
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

            bindTextures();

            lightsOn();

            m_dh->shaderHelper->m_splineSurfShader->bind();
            m_dh->shaderHelper->setSplineSurfaceShaderVars();
            wxColor c = info->getColor();
            glColor3f( (float) c.Red() / 255.0, (float) c.Green() / 255.0, (float) c.Blue() / 255.0 );
            m_dh->shaderHelper->m_splineSurfShader->setUniInt( "useTex", !info->getUseTex() );
            m_dh->shaderHelper->m_splineSurfShader->setUniInt( "useLic", info->getUseLIC() );
            m_dh->shaderHelper->m_splineSurfShader->setUniInt( "useColorMap", m_dh->colorMap );

            info->draw();

            m_dh->shaderHelper->m_splineSurfShader->release();

            lightsOff();

            if ( m_dh->GLError() )
                m_dh->printGLError( wxT("draw surface") );

            glPopAttrib();
        }
    }
}

void TheScene::renderMesh()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    if ( m_dh->lighting )
    {
        lightsOn();
    }


    bindTextures();

    m_dh->shaderHelper->m_meshShader->bind();
    m_dh->shaderHelper->setMeshShaderVars();

    if ( m_dh->pointMode )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


    for ( int i = 0; i < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData( i );
        if ( info->getType() == Mesh_ || info->getType() == IsoSurface_ )
        {
            if ( info->getShow() )
            {
                wxColor c = info->getColor();
                glColor3f( (float) c.Red() / 255.0, (float) c.Green() / 255.0, (float) c.Blue() / 255.0 );
                m_dh->shaderHelper->m_meshShader->setUniInt( "showFS", info->getShowFS() );
                m_dh->shaderHelper->m_meshShader->setUniInt( "useTex", info->getUseTex() );
                m_dh->shaderHelper->m_meshShader->setUniFloat( "alpha_", info->getAlpha() );
                m_dh->shaderHelper->m_meshShader->setUniInt( "useLic", info->getUseLIC() );

                glEnable( GL_BLEND );
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

                info->draw();
            }
        }
    }
/*
    std::vector< std::vector< SelectionBox* > > boxes = m_dh->getSelectionBoxes();

    m_dh->shaderHelper->m_meshShader->bind();
    m_dh->shaderHelper->m_meshShader->setUniInt( "showFS", true );
    m_dh->shaderHelper->m_meshShader->setUniInt( "useTex", false );
    m_dh->shaderHelper->m_meshShader->setUniFloat( "alpha_", 1.0 );
    m_dh->shaderHelper->m_meshShader->setUniInt( "useColorMap", m_dh->colorMap );
    m_dh->shaderHelper->m_meshShader->setUniInt( "useLic", false );
    m_dh->shaderHelper->m_meshShader->setUniInt( "useCMAP", false );

    for ( unsigned int i = 0; i < boxes.size(); ++i )
    {
        for ( unsigned int j = 0; j < boxes[i].size(); ++j )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );
            if ( !boxes[i][j]->getIsBox() )
                boxes[i][j]->drawIsoSurface();
            glPopAttrib();
        }
    }
*/
    m_dh->shaderHelper->m_meshShader->release();

    lightsOff();

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("draw mesh") );

    glPopAttrib();
}

void TheScene::renderFibers()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    for ( int i = 0; i < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData( i );

        if ( info->getType() == Fibers_ && info->getShow() )
        {
            lightsOff();

            if ( m_dh->lighting )
            {
                lightsOn();
                GLfloat light_position0[] =
                { 1.0, 1.0, 1.0, 0.0 };
                glLightfv( GL_LIGHT0, GL_POSITION, light_position0 );

            }
            if ( !info->getUseTex() )
            {
                bindTextures();
                m_dh->shaderHelper->m_fiberShader->bind();
                m_dh->shaderHelper->setFiberShaderVars();
                m_dh->shaderHelper->m_fiberShader->setUniInt( "useTex", !info->getUseTex() );
                m_dh->shaderHelper->m_fiberShader->setUniInt( "useColorMap", m_dh->colorMap );
                m_dh->shaderHelper->m_fiberShader->setUniInt( "useOverlay", info->getShowFS() );
            }
            if ( m_dh->m_selBoxChanged )
            {
                ( (Fibers*) info )->updateLinesShown();
                m_dh->m_selBoxChanged = false;
            }
            info->draw();

            m_dh->shaderHelper->m_fiberShader->release();

            lightsOff();
        }
    }

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("draw fibers") );

    glPopAttrib();
}

void TheScene::renderFakeTubes()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    for ( int i = 0; i < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData( i );

        if ( info->getType() == Fibers_ && info->getShow() )
        {
            if ( m_dh->m_selBoxChanged )
            {
                ( (Fibers*) info )->updateLinesShown();
                m_dh->m_selBoxChanged = false;
            }

            m_dh->shaderHelper->m_fakeTubeShader->bind();
            m_dh->shaderHelper->m_fakeTubeShader->setUniInt( "globalColor", info->getShowFS() );
            m_dh->shaderHelper->m_fakeTubeShader->setUniFloat( "dimX",
                    (float) m_dh->mainFrame->m_mainGL->GetSize().x );
            m_dh->shaderHelper->m_fakeTubeShader->setUniFloat( "dimY",
                    (float) m_dh->mainFrame->m_mainGL->GetSize().y );

            info->draw();

            m_dh->shaderHelper->m_fakeTubeShader->release();
        }
    }

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("draw fake tubes") );

    glPopAttrib();
}

void TheScene::lightsOn()
{
    GLfloat light_ambient[]  = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_diffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat specref[] =  { 0.5, 0.5, 0.5, 0.5 };
    Vector3fT v1 = { { 0, 0, -1 } };
    Vector3fT l;
    Vector3fMultMat4( &l, &v1, &m_dh->m_transform );

    GLfloat light_position0[] = { l.s.X, l.s.Y, l.s.Z, 0.0 };

    glLightfv( GL_LIGHT0, GL_AMBIENT, light_ambient );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, light_diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, light_specular );
    glLightfv( GL_LIGHT0, GL_POSITION, light_position0 );

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glShadeModel( GL_SMOOTH );

    glEnable( GL_COLOR_MATERIAL );
    glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specref );
    glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 32 );

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup lights") );
}

void TheScene::lightsOff()
{
    glDisable( GL_LIGHTING );
    glDisable( GL_COLOR_MATERIAL );
}

void TheScene::drawSphere( float x, float y, float z, float r )
{
    glPushMatrix();
    glTranslatef( x, y, z );

    GLUquadricObj *quadric = gluNewQuadric();
    gluQuadricNormals( quadric, GLU_SMOOTH );
    gluSphere( quadric, r, 32, 32 );
    glPopMatrix();

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("draw sphere") );
}

void TheScene::drawSelectionBoxes()
{
    std::vector< std::vector< SelectionBox* > > boxes = m_dh->getSelectionBoxes();
    for ( unsigned int i = 0; i < boxes.size(); ++i )
    {
        for ( unsigned int j = 0; j < boxes[i].size(); ++j )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );

            boxes[i][j]->draw();

            glPopAttrib();
        }
    }

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("draw selection boxes") );
}

void TheScene::drawPoints()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    lightsOn();
    m_dh->shaderHelper->m_meshShader->bind();
    m_dh->shaderHelper->setMeshShaderVars();
    m_dh->shaderHelper->m_meshShader->setUniInt( "showFS", true );
    m_dh->shaderHelper->m_meshShader->setUniInt( "useTex", false );
    m_dh->shaderHelper->m_meshShader->setUniInt( "cutAtSurface", false );
    m_dh->shaderHelper->m_meshShader->setUniInt( "lightOn", true );

    wxTreeItemId id, childid;
    wxTreeItemIdValue cookie = 0;
    id = m_dh->mainFrame->m_treeWidget->GetFirstChild( m_dh->mainFrame->m_tPointId, cookie );
    while ( id.IsOk() )
    {
        SplinePoint *point = (SplinePoint*) ( m_dh->mainFrame->m_treeWidget->GetItemData( id ) );
        point->draw();

        id = m_dh->mainFrame->m_treeWidget->GetNextChild( m_dh->mainFrame->m_tPointId, cookie );
    }

    lightsOff();
    m_dh->shaderHelper->m_meshShader->release();
    glPopAttrib();

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("draw points") );
}

void TheScene::drawColorMapLegend()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glPushMatrix();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    int size = wxMax(wxMax(m_dh->rows, m_dh->columns), m_dh->frames);
    glOrtho( 0, size, 0, size, -3000, 3000 );

    m_dh->shaderHelper->m_legendShader->bind();
    m_dh->shaderHelper->m_legendShader->setUniInt( "useColorMap", m_dh->colorMap );

    glColor3f( 0.0, 0.0, 0.0 );
    glLineWidth( 5.0 );
    glBegin( GL_LINES );
    glTexCoord1f( 0.0 );
    glVertex3i( size - 60, 10, 2900 );
    glTexCoord1f( 1.0 );
    glVertex3i( size - 20, 10, 2900 );
    glEnd();

    m_dh->shaderHelper->m_legendShader->release();

    glLineWidth( 1.0 );
    glColor3f( 0.0, 0.0, 0.0 );
    glBegin( GL_LINES );
    glVertex3i( size - 60, 10, 2900 );
    glVertex3i( size - 60, 12, 2900 );
    glVertex3i( size - 50, 10, 2900 );
    glVertex3i( size - 50, 12, 2900 );
    glVertex3i( size - 40, 10, 2900 );
    glVertex3i( size - 40, 12, 2900 );
    glVertex3i( size - 30, 10, 2900 );
    glVertex3i( size - 30, 12, 2900 );
    glVertex3i( size - 20, 10, 2900 );
    glVertex3i( size - 20, 12, 2900 );
    glEnd();

    m_dh->shaderHelper->m_legendShader->release();

    glPopMatrix();
    glPopAttrib();
}

void TheScene::drawVectors()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    for ( int i = 0; i < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData( i );

        if ( info->getType() == Vectors_ && info->getShow() )
        {
            Anatomy* vecs = (Anatomy*) info;
            glLineWidth( 1.0 );
            glBegin( GL_LINES );

            float r, g, b, a;

            r = vecs->getColor().Red() / 255.;
            g = vecs->getColor().Green() / 255.;
            b = vecs->getColor().Blue() / 255.;
            a = 1.0;

            float bright = 1.2f;
            float dull = 0.7f;

            bool topview = m_dh->quadrant == 2 || m_dh->quadrant == 3 || m_dh->quadrant == 6
                    || m_dh->quadrant == 7;
            bool leftview = m_dh->quadrant == 5 || m_dh->quadrant == 6 || m_dh->quadrant == 7
                    || m_dh->quadrant == 8;
            bool frontview = m_dh->quadrant == 3 || m_dh->quadrant == 4 || m_dh->quadrant == 5
                    || m_dh->quadrant == 6;

            if ( m_dh->showAxial )
            {
                for ( int i = 0; i < m_dh->columns; ++i )
                {
                    for ( int j = 0; j < m_dh->rows; ++j )
                    {
                        int slize = (int) ( m_dh->zSlize * m_dh->columns * m_dh->rows * 3 );
                        int index = i * 3 + j * m_dh->columns * 3 + slize;

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

                        if ( !vecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                    (GLfloat) m_dh->zSlize + .5 + z / 2. );
                            glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                    (GLfloat) m_dh->zSlize + .5 - z / 2. );
                        }
                        else
                        {
                            if ( vecs->at( index + 2 ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if ( topview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                            (GLfloat) m_dh->zSlize + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize
                                            + .4 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                            (GLfloat) m_dh->zSlize + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize
                                            + .4 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                            (GLfloat) m_dh->zSlize + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize
                                            + .6 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                            (GLfloat) m_dh->zSlize + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize
                                            + .6 );
                                }
                            }
                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if ( topview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                            (GLfloat) m_dh->zSlize + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize
                                            + .4 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                            (GLfloat) m_dh->zSlize + .4 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize
                                            + .4 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) j + .5 - y / 2.,
                                            (GLfloat) m_dh->zSlize + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize
                                            + .6 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) j + .5 + y / 2.,
                                            (GLfloat) m_dh->zSlize + .6 );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) j + .5, (GLfloat) m_dh->zSlize
                                            + .6 );
                                }
                            }
                        }
                    }
                }
            }
            if ( m_dh->showCoronal )
            {
                for ( int i = 0; i < m_dh->columns; ++i )
                {
                    for ( int j = 0; j < m_dh->frames; ++j )
                    {
                        int slize = (int) ( m_dh->ySlize * m_dh->columns * 3 );
                        int index = i * 3 + slize + j * m_dh->columns * m_dh->rows * 3;

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
                        if ( !vecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .5 + y / 2.,
                                    (GLfloat) j + .5 + z / 2. );
                            glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .5 - y / 2.,
                                    (GLfloat) j + .5 - z / 2. );
                        }
                        else
                        {
                            if ( vecs->at( index + 1 ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if ( frontview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .4,
                                            (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_dh->ySlize + .4, (GLfloat) j
                                            + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .4,
                                            (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_dh->ySlize + .4, (GLfloat) j
                                            + .5 );
                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .6,
                                            (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_dh->ySlize + .6, (GLfloat) j
                                            + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .6,
                                            (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_dh->ySlize + .6, (GLfloat) j
                                            + .5 );
                                }
                            }

                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if ( frontview )
                                {
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .4,
                                            (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_dh->ySlize + .4, (GLfloat) j
                                            + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .4,
                                            (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_dh->ySlize + .4, (GLfloat) j
                                            + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) i + .5 - x / 2., (GLfloat) m_dh->ySlize + .6,
                                            (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_dh->ySlize + .6, (GLfloat) j
                                            + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) i + .5 + x / 2., (GLfloat) m_dh->ySlize + .6,
                                            (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) i + .5, (GLfloat) m_dh->ySlize + .6, (GLfloat) j
                                            + .5 );
                                }
                            }
                        }
                    }
                }
            }
            if ( m_dh->showSagittal )
            {
                for ( int i = 0; i < m_dh->rows; ++i )
                {
                    for ( int j = 0; j < m_dh->frames; ++j )
                    {
                        int slize = (int) ( m_dh->xSlize * 3 );
                        int index = slize + i * m_dh->columns * 3 + j * m_dh->columns * m_dh->rows * 3;

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
                        if ( !vecs->getShowFS() )
                        {
                            glColor4f( r, g, b, a );
                            glVertex3f( (GLfloat) m_dh->xSlize + .5 + x / 2., (GLfloat) i + .5 + y / 2.,
                                    (GLfloat) j + .5 + z / 2. );
                            glVertex3f( (GLfloat) m_dh->xSlize + .5 - x / 2., (GLfloat) i + .5 - y / 2.,
                                    (GLfloat) j + .5 - z / 2. );
                        }
                        else
                        {
                            if ( vecs->at( index ) <= 0.0 )
                            {
                                glColor4f( r, g, b, a );
                                if ( leftview )
                                {
                                    glVertex3f( (GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5 + y / 2.,
                                            (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5, (GLfloat) j
                                            + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5 - y / 2.,
                                            (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5, (GLfloat) j
                                            + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5 - y / 2.,
                                            (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5, (GLfloat) j
                                            + .5 );
                                    glColor4f( r * dull, g * dull, b * dull, a );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5 + y / 2.,
                                            (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5, (GLfloat) j
                                            + .5 );
                                }
                            }
                            else
                            {
                                glColor4f( r * dull, g * dull, b * dull, a );
                                if ( leftview )
                                {
                                    glVertex3f( (GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5 + y / 2.,
                                            (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5, (GLfloat) j
                                            + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5 - y / 2.,
                                            (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .4, (GLfloat) i + .5, (GLfloat) j
                                            + .5 );

                                }
                                else
                                {
                                    glVertex3f( (GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5 - y / 2.,
                                            (GLfloat) j + .5 - z / 2. );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5, (GLfloat) j
                                            + .5 );
                                    glColor4f( r, g, b, a );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5 + y / 2.,
                                            (GLfloat) j + .5 + z / 2. );
                                    glVertex3f( (GLfloat) m_dh->xSlize + .6, (GLfloat) i + .5, (GLfloat) j
                                            + .5 );
                                }
                            }
                        }
                    }
                }
            }

            for ( int j = 0; j < m_dh->mainFrame->m_listCtrl->GetItemCount(); ++j )
            {
                DatasetInfo* mesh = (DatasetInfo*) m_dh->mainFrame->m_listCtrl->GetItemData( j );

                if ( mesh->getType() == IsoSurface_ && mesh->getShow() )
                {
                    CIsoSurface* surf = (CIsoSurface*) mesh;
                    std::vector< Vector > positions = surf->getSurfaceVoxelPositions();
                    for ( size_t k = 0; k < positions.size(); ++k )
                    {
                        int index = (int) positions[k].x * 3 + (int) positions[k].y * m_dh->columns * 3
                                + (int) positions[k].z * m_dh->rows * m_dh->columns * 3;

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

                else if ( mesh->getType() == Surface_ && mesh->getShow() )
                {
                    Surface* surf = (Surface*) mesh;
                    std::vector< Vector > positions = surf->getSurfaceVoxelPositions();

                    for ( size_t k = 0; k < positions.size(); ++k )
                    {
                        int index = (int) positions[k].x * 3 + (int) positions[k].y * m_dh->columns * 3
                                + (int) positions[k].z * m_dh->rows * m_dh->columns * 3;

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
            }
            glEnd();
        }
    }

    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("draw vectors") );

    glDisable( GL_BLEND );

    glPopAttrib();
}
