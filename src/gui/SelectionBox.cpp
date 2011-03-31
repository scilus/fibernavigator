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

#include "../dataset/Anatomy.h"
#include "../misc/IsoSurface/CIsoSurface.h"

///////////////////////////////////////////////////////////////////////////
// Constructor
// i_center             : The center of the box.
// i_size               : The size of the box.
// i_dataHelper         : The datasetHekper associated with this box.
///////////////////////////////////////////////////////////////////////////
SelectionBox::SelectionBox( Vector i_center, Vector i_size, DatasetHelper* i_datasetHelper ) :
    SelectionObject( i_center, i_size, i_datasetHelper )
{
    m_gfxDirty   = true;
    m_name       = wxT( "box" );
    m_objectType = BOX_TYPE;

    update();
}

///////////////////////////////////////////////////////////////////////////
// Constructor
// 
// i_datasetHelper          : The dataset Helper associated with this box.
// i_anatomy                : The anatomy associated with this box.
///////////////////////////////////////////////////////////////////////////
SelectionBox::SelectionBox( DatasetHelper* i_datasetHelper, Anatomy* i_anatomy ) :
   SelectionObject( Vector( 0.0f, 0.0f, 0.0f ), Vector( 0.0f, 0.0f, 0.0f ), i_datasetHelper )
{
	
   m_isMaster      = true;
   m_isosurface    = new CIsoSurface( m_datasetHelper, i_anatomy );
   
   wxString mystring(wxT("[ROI] - ") + i_anatomy->getName());
   m_name          =  mystring;
   
   m_objectType    = CISO_SURFACE_TYPE;
	m_sourceAnatomy = i_anatomy;
}

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
SelectionBox::~SelectionBox()
{
	if( m_isLockedToCrosshair )
	{
		m_datasetHelper->m_boxLockIsOn = false;
	}
	if( m_objectType == CISO_SURFACE_TYPE )
	{
		delete m_isosurface;
		if( m_sourceAnatomy && m_sourceAnatomy->m_roi == this)
		{
			m_sourceAnatomy->m_roi = NULL;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// This is the specific implementation to draw a selectionBox object.
//
// i_color      : The color of the box to draw.
///////////////////////////////////////////////////////////////////////////
void SelectionBox::drawObject( GLfloat* i_color )
{
    glDepthMask(GL_FALSE);
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
		switch( m_datasetHelper->m_quadrant )
		{
		    case 1: draw2(); draw4(); draw6(); draw1(); draw3(); draw5(); break;    
            case 2: draw2(); draw4(); draw5(); draw1();	draw3(); draw6(); break;
		    case 3: draw2(); draw3(); draw5(); draw1(); draw4(); draw6(); break;
            case 4: draw2(); draw3(); draw6(); draw1(); draw4(); draw5(); break;
            case 5: draw1(); draw3(); draw6(); draw2(); draw4(); draw5(); break;
		    case 6:	draw1(); draw3(); draw5(); draw2(); draw4(); draw6(); break;
		    case 7:	draw1(); draw4(); draw5(); draw2(); draw3(); draw6(); break;
		    case 8: draw1(); draw4(); draw6(); draw2(); draw3(); draw5(); break;
		}

	glEnd();

	glDisable( GL_BLEND );
    glDepthMask(GL_TRUE);
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void SelectionBox::draw1()
{
	glVertex3f( m_maxX, m_minY, m_minZ );
	glVertex3f( m_maxX, m_maxY, m_minZ );
	glVertex3f( m_maxX, m_maxY, m_maxZ );
	glVertex3f( m_maxX, m_minY, m_maxZ );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void SelectionBox::draw2()
{
	glVertex3f( m_minX, m_minY, m_minZ );
	glVertex3f( m_minX, m_maxY, m_minZ );
	glVertex3f( m_minX, m_maxY, m_maxZ );
	glVertex3f( m_minX, m_minY, m_maxZ );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void SelectionBox::draw3()
{
	glVertex3f( m_minX, m_maxY, m_minZ );
	glVertex3f( m_maxX, m_maxY, m_minZ );
	glVertex3f( m_maxX, m_maxY, m_maxZ );
	glVertex3f( m_minX, m_maxY, m_maxZ );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void SelectionBox::draw4()
{
	glVertex3f( m_minX, m_minY, m_minZ );
	glVertex3f( m_maxX, m_minY, m_minZ );
	glVertex3f( m_maxX, m_minY, m_maxZ );
	glVertex3f( m_minX, m_minY, m_maxZ );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void SelectionBox::draw5()
{
	glVertex3f( m_minX, m_minY, m_maxZ );
	glVertex3f( m_minX, m_maxY, m_maxZ );
	glVertex3f( m_maxX, m_maxY, m_maxZ );
	glVertex3f( m_maxX, m_minY, m_maxZ );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void SelectionBox::draw6()
{
	glVertex3f( m_minX, m_minY, m_minZ );
	glVertex3f( m_minX, m_maxY, m_minZ );
	glVertex3f( m_maxX, m_maxY, m_minZ );
	glVertex3f( m_maxX, m_minY, m_minZ );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_ray        :
///////////////////////////////////////////////////////////////////////////
hitResult SelectionBox::hitTest( Ray* i_ray )
{
	hitResult hr = { false, 0.0f, 0, NULL };

	if( m_isVisible && m_isActive && m_objectType == BOX_TYPE ) 
	{
		int   picked  = 0;
        float tpicked = 0;
		float cx = m_center.x;
        float cy = m_center.y;
        float cz = m_center.z;
        float sx = m_size.x * m_datasetHelper->m_xVoxel;
        float sy = m_size.y * m_datasetHelper->m_yVoxel;
        float sz = m_size.z * m_datasetHelper->m_zVoxel;

       	if( wxGetKeyState( WXK_CONTROL ) )
		{
       		BoundingBox *bb = new BoundingBox( cx, cy, cz, sx, sy, sz );

			bb->setCenter( m_minX , cy, cz );
			bb->setSize( sx, sy, sz );
			bb->setSizeX( m_datasetHelper->m_xVoxel );
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
			bb->setSizeY( m_datasetHelper->m_yVoxel );
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
			bb->setSizeZ( m_datasetHelper->m_zVoxel );
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
