/////////////////////////////////////////////////////////////////////////////
// Name:            selecitonBox.cpp
// Author:          ---
// Creation Date:   ---
//
// Description: This is the implementation file for SelectionBox class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#include "SelectionBox.h"

#include "SceneManager.h"
#include "../dataset/Anatomy.h"
#include "../dataset/DatasetManager.h"
#include "../misc/IsoSurface/CIsoSurface.h"

#include "Logger.h"

///////////////////////////////////////////////////////////////////////////
// Constructor
// i_center             : The center of the box.
// i_size               : The size of the box.
// i_dataHelper         : The datasetHekper associated with this box.
///////////////////////////////////////////////////////////////////////////
SelectionBox::SelectionBox( Vector i_center, Vector i_size )
:   SelectionObject( i_center, i_size )
{
    m_name       = wxT( "box" );
    m_objectType = BOX_TYPE;

    update();
}

SelectionBox::SelectionBox( const wxXmlNode selObjNode )
: SelectionObject( selObjNode )
{
    m_objectType = BOX_TYPE;
    
    update();
}

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
SelectionBox::~SelectionBox()
{
}

wxString SelectionBox::getTypeTag() const
{
    return wxT( "box" );
}

///////////////////////////////////////////////////////////////////////////
// This is the specific implementation to draw a selectionBox object.
//
// i_color      : The color of the box to draw.
///////////////////////////////////////////////////////////////////////////
void SelectionBox::drawObject( GLfloat* i_color )
{
    glDepthMask( GL_FALSE );
    glColor4f( i_color[0], i_color[1], i_color[2], i_color[3] );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glBegin( GL_QUADS );
        glVertex3f( m_minX, m_minY, m_minZ );
        glVertex3f( m_minX, m_maxY, m_minZ );
        glVertex3f( m_minX, m_maxY, m_maxZ );
        glVertex3f( m_minX, m_minY, m_maxZ );
        glVertex3f( m_maxX, m_minY, m_minZ );
        glVertex3f( m_maxX, m_maxY, m_minZ );
        glVertex3f( m_maxX, m_maxY, m_maxZ );
        glVertex3f( m_maxX, m_minY, m_maxZ );

        glVertex3f( m_minX, m_minY, m_minZ );
        glVertex3f( m_maxX, m_minY, m_minZ );
        glVertex3f( m_maxX, m_minY, m_maxZ );
        glVertex3f( m_minX, m_minY, m_maxZ );
        glVertex3f( m_minX, m_maxY, m_minZ );
        glVertex3f( m_maxX, m_maxY, m_minZ );
        glVertex3f( m_maxX, m_maxY, m_maxZ );
        glVertex3f( m_minX, m_maxY, m_maxZ );

        glVertex3f( m_minX, m_minY, m_minZ );
        glVertex3f( m_minX, m_maxY, m_minZ );
        glVertex3f( m_maxX, m_maxY, m_minZ );
        glVertex3f( m_maxX, m_minY, m_minZ );
        glVertex3f( m_minX, m_minY, m_maxZ );
        glVertex3f( m_minX, m_maxY, m_maxZ );
        glVertex3f( m_maxX, m_maxY, m_maxZ );
        glVertex3f( m_maxX, m_minY, m_maxZ );
    glEnd();

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glBegin( GL_QUADS );
    switch( SceneManager::getInstance()->getQuadrant() )
    {
        case 1: draw2(); draw4(); draw6(); draw1(); draw3(); draw5(); break;
        case 2: draw2(); draw4(); draw5(); draw1(); draw3(); draw6(); break;
        case 3: draw2(); draw3(); draw5(); draw1(); draw4(); draw6(); break;
        case 4: draw2(); draw3(); draw6(); draw1(); draw4(); draw5(); break;
        case 5: draw1(); draw3(); draw6(); draw2(); draw4(); draw5(); break;
        case 6: draw1(); draw3(); draw5(); draw2(); draw4(); draw6(); break;
        case 7: draw1(); draw4(); draw5(); draw2(); draw3(); draw6(); break;
        case 8: draw1(); draw4(); draw6(); draw2(); draw3(); draw5(); break;
    }

    glEnd();

    glDisable( GL_BLEND );
    glDepthMask( GL_TRUE );
}

void SelectionBox::draw1()
{
    glVertex3f( m_maxX, m_minY, m_minZ );
    glVertex3f( m_maxX, m_maxY, m_minZ );
    glVertex3f( m_maxX, m_maxY, m_maxZ );
    glVertex3f( m_maxX, m_minY, m_maxZ );
}

void SelectionBox::draw2()
{
    glVertex3f( m_minX, m_minY, m_minZ );
    glVertex3f( m_minX, m_maxY, m_minZ );
    glVertex3f( m_minX, m_maxY, m_maxZ );
    glVertex3f( m_minX, m_minY, m_maxZ );
}

void SelectionBox::draw3()
{
    glVertex3f( m_minX, m_maxY, m_minZ );
    glVertex3f( m_maxX, m_maxY, m_minZ );
    glVertex3f( m_maxX, m_maxY, m_maxZ );
    glVertex3f( m_minX, m_maxY, m_maxZ );
}

void SelectionBox::draw4()
{
    glVertex3f( m_minX, m_minY, m_minZ );
    glVertex3f( m_maxX, m_minY, m_minZ );
    glVertex3f( m_maxX, m_minY, m_maxZ );
    glVertex3f( m_minX, m_minY, m_maxZ );
}

void SelectionBox::draw5()
{
    glVertex3f( m_minX, m_minY, m_maxZ );
    glVertex3f( m_minX, m_maxY, m_maxZ );
    glVertex3f( m_maxX, m_maxY, m_maxZ );
    glVertex3f( m_maxX, m_minY, m_maxZ );
}

void SelectionBox::draw6()
{
    glVertex3f( m_minX, m_minY, m_minZ );
    glVertex3f( m_minX, m_maxY, m_minZ );
    glVertex3f( m_maxX, m_maxY, m_minZ );
    glVertex3f( m_maxX, m_minY, m_minZ );
}

hitResult SelectionBox::hitTest( Ray* i_ray )
{
    hitResult hr = { false, 0.0f, 0, NULL };

    // TODO selection remove objectType
    if( m_isVisible && m_isActive && m_objectType == BOX_TYPE ) 
    {
        float voxelX = DatasetManager::getInstance()->getVoxelX();
        float voxelY = DatasetManager::getInstance()->getVoxelY();
        float voxelZ = DatasetManager::getInstance()->getVoxelZ();

        int   picked  = 0;
        float tpicked = 0;
        float cx = m_center.x;
        float cy = m_center.y;
        float cz = m_center.z;
        float sx = m_size.x * voxelX;
        float sy = m_size.y * voxelY;
        float sz = m_size.z * voxelZ;

           if( wxGetKeyState( WXK_CONTROL ) )
        {
               BoundingBox *bb = new BoundingBox( cx, cy, cz, sx, sy, sz );

            bb->setCenter( m_minX , cy, cz );
            bb->setSize( sx, sy, sz );
            bb->setSizeX( voxelX );
            hr = bb->hitTest( i_ray );
            if( hr.hit )
            {

                if( picked == 0 )
                {
                    picked = 11;
                    tpicked = hr.tmin;
                }
                else 
                {
                    if( hr.tmin < tpicked )
                    {
                        picked = 11;
                        tpicked = hr.tmin;
                    }
                }
            }
            bb->setCenter( m_maxX, cy, cz );
            hr = bb->hitTest( i_ray );
            if( hr.hit ) 
            {
                if( picked == 0 ) 
                {
                    picked = 12;
                    tpicked = hr.tmin;
                }
                else 
                {
                    if( hr.tmin < tpicked ) 
                    {
                        picked  = 12;
                        tpicked = hr.tmin;
                    }
                }
            }
            bb->setCenter( cx, m_minY, cz );
            bb->setSize( sx, sy, sz);
            bb->setSizeY( voxelY );
            hr = bb->hitTest( i_ray );
            if( hr.hit )
            {
                if( picked == 0 )
                {
                    picked = 13;
                    tpicked = hr.tmin;
                }
                else 
                {
                    if( hr.tmin < tpicked ) 
                    {
                        picked = 13;
                        tpicked = hr.tmin;
                    }
                }
            }
            bb->setCenter( cx, m_maxY, cz );
            hr = bb->hitTest( i_ray );
            if( hr.hit)
            {
                if( picked == 0 )
                {
                    picked = 14;
                    tpicked = hr.tmin;
                }
                else 
                {
                    if( hr.tmin < tpicked ) 
                    {
                        picked = 14;
                        tpicked = hr.tmin;
                    }
                }
            }
            bb->setCenter( cx, cy, m_minZ );
            bb->setSize( sx, sy, sz );
            bb->setSizeZ( voxelZ );
            hr = bb->hitTest( i_ray );
            if( hr.hit ) 
            {
                if( picked == 0 )
                {
                    picked  = 15;
                    tpicked = hr.tmin;
                }
                else 
                {
                    if( hr.tmin < tpicked )
                    {
                        picked = 15;
                        tpicked = hr.tmin;
                    }
                }
            }
            bb->setCenter( cx, cy, m_maxZ );
            hr = bb->hitTest( i_ray );
            if( hr.hit )
            {
                if( picked == 0 ) 
                {
                    picked = 16;
                    tpicked = hr.tmin;
                }
                else 
                {
                    if( hr.tmin < tpicked ) 
                    {
                        picked = 16;
                        tpicked = hr.tmin;
                    }
                }
            }

        }
        else  // if (wxGetKeyState(WXK_CONTROL))
        {
            BoundingBox *bb = new BoundingBox( cx, cy, cz, sx, sy, sz );
            hr = bb->hitTest( i_ray );
            if( hr.hit )
            {
                if( picked == 0 )
                {
                    picked = 10;
                    tpicked = hr.tmin;

                }
                else 
                {
                    if( hr.tmin < tpicked )
                    {
                        picked = 10;
                        tpicked = hr.tmin;
                    }
                }
            }
        }
        if( picked != 0 )
        {
            hr.hit = true;
            hr.tmin = tpicked;
            hr.picked = picked;
            hr.object = this;
        }

    }

    m_hitResult = hr;

    return hr;
}
