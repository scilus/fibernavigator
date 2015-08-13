/////////////////////////////////////////////////////////////////////////////
// Name:            selecitonEllipsoid.cpp
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/26/2009
//
// Description: This is the implementation file for SelectionEllipsoid class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#include "SelectionEllipsoid.h"

#include "../dataset/DatasetManager.h"

///////////////////////////////////////////////////////////////////////////
// Constructor
// i_center             : The center of the ellipsoid.
// i_size               : The size of the ellipsoid.
// i_dataHelper         : The datasetHelper associated with this ellipsoid.
///////////////////////////////////////////////////////////////////////////
SelectionEllipsoid::SelectionEllipsoid( Vector i_center, Vector i_size )
:   SelectionObject( i_center, i_size )
{
    m_name       = wxT( "ellipsoid" );
    m_objectType = ELLIPSOID_TYPE;

    update();
}

SelectionEllipsoid::SelectionEllipsoid( Vector i_center, Vector i_size, Vector magnet )
:   SelectionObject( i_center, i_size )
{
    m_name       = wxT( "magnet" );
    m_objectType = ELLIPSOID_TYPE;
    
    
    m_size = Vector(5,5,5);
    m_isActive = false;
    m_isVisible = true;
    m_isMagnet = true;
    m_name =  wxT( "Magnet" );
    m_magnetField = magnet;

    update();
}

SelectionEllipsoid::SelectionEllipsoid( const wxXmlNode selObjNode )
: SelectionObject( selObjNode )
{
    m_objectType = ELLIPSOID_TYPE;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
SelectionEllipsoid::~SelectionEllipsoid()
{
}

wxString SelectionEllipsoid::getTypeTag() const
{
    return wxT( "ellipsoid" );
}

///////////////////////////////////////////////////////////////////////////
// This is the specific implementation to draw a selectionEllipsoid object.
//
// i_color      : The color of the ellipsoid to draw.
///////////////////////////////////////////////////////////////////////////
void SelectionEllipsoid::drawObject( GLfloat* i_color )
{
    glColor4f( i_color[0], i_color[1], i_color[2], i_color[3] );
    
    glDepthMask(GL_FALSE);
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    
    
    glPushMatrix();
        glTranslatef( m_center.x, m_center.y, m_center.z );
        GLUquadricObj* l_quadric = gluNewQuadric();
        gluQuadricNormals( l_quadric, GLU_SMOOTH );
        glScalef( m_xRadius, m_yRadius, m_zRadius );
        gluSphere( l_quadric, 1.0f, 32, 32 );
    glPopMatrix();

    glDisable( GL_BLEND );
    glDepthMask(GL_TRUE);
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_ray        :
///////////////////////////////////////////////////////////////////////////
hitResult SelectionEllipsoid::hitTest( Ray* i_ray )
{
    hitResult hr = { false, 0.0f, 0, NULL };

    // TODO selection remove objectType
    if( m_isVisible && m_objectType == ELLIPSOID_TYPE ) 
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

///////////////////////////////////////////////////////////////////////////
// This function set the proper radius of the ellipsoid.
///////////////////////////////////////////////////////////////////////////
void SelectionEllipsoid::objectUpdate()
{
    m_xRadius = ( m_maxX - m_minX ) / 2.0f;
    m_yRadius = ( m_maxY - m_minY ) / 2.0f;
    m_zRadius = ( m_maxZ - m_minZ ) / 2.0f;
}