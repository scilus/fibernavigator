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

///////////////////////////////////////////////////////////////////////////
// Constructor
// i_center             : The center of the ellipsoid.
// i_size               : The size of the ellipsoid.
// i_dataHelper         : The datasetHelper associated with this ellipsoid.
///////////////////////////////////////////////////////////////////////////
SelectionEllipsoid::SelectionEllipsoid( Vector i_center, Vector i_size, DatasetHelper* i_datasetHelper ) :
    SelectionObject( i_center, i_size, i_datasetHelper )
{
    m_gfxDirty   = true;
    m_name       = wxT( "ellipsoid" );
    m_objectType = ELLIPSOID_TYPE;

    update();
}

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
SelectionEllipsoid::~SelectionEllipsoid()
{

}

///////////////////////////////////////////////////////////////////////////
// This is the specific implementation to draw a selectionEllipsoid object.
//
// i_color      : The color of the ellipsoid to draw.
///////////////////////////////////////////////////////////////////////////
void SelectionEllipsoid::drawObject( GLfloat* i_color )
{
	glColor4f( i_color[0], i_color[1], i_color[2], i_color[3] );
    
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
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_ray        :
///////////////////////////////////////////////////////////////////////////
hitResult SelectionEllipsoid::hitTest( Ray* i_ray )
{    
    hitResult l_hitResult;    
    l_hitResult.hit    = false;
    l_hitResult.object = NULL;
    l_hitResult.picked = 0;
    l_hitResult.tmin   = 0.0f;

    return l_hitResult;
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