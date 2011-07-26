#include "splinePoint.h"
#include "../gui/SelectionObject.h"
#include "../main.h"

SplinePoint::SplinePoint( Vector center, DatasetHelper* dh )
:   m_dh( dh),
    m_center( center ),
    m_origin( center ),
    m_offsetVector( Vector( 0.0, 0.0, 0.0 ) ),
    m_selected( false ),
    m_isBoundary( false ),
    m_treeId( 0 ),
    m_name(wxT(""))
{
    m_dh->m_surfaceIsDirty = true;
}

SplinePoint::SplinePoint( double x, double y, double z, DatasetHelper* dh )
:   m_dh( dh ),
    m_center( Vector( x, y, z ) ),
    m_origin( Vector( x, y, z ) ),
    m_offsetVector( Vector( 0.0, 0.0, 0.0 ) ),
    m_selected( false ),
    m_isBoundary( false ),
    m_treeId( 0 ),
    m_name(wxT(""))
{
    m_dh->m_surfaceIsDirty = true;
}

SplinePoint::~SplinePoint()
{
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_lastSelectedPoint = 0;
}

void SplinePoint::draw()
{
    if( m_selected )
        glColor3f( 1.0f, 0.8f, 0.0f );
    else if( m_isBoundary )
        glColor3f( 1.0f, 0.0f, 0.0f );
    else
        glColor3f( 0.0f, 0.8f, 0.8f );

    drawSphere( m_center.x, m_center.y, m_center.z, 2 );
}

void SplinePoint::drawSphere( float x, float y, float z, float r )
{
    glPushMatrix();
    glTranslatef( x,y,z );
    GLUquadricObj* quadric = gluNewQuadric();
    gluQuadricNormals( quadric, GLU_SMOOTH );
    gluSphere( quadric, r, 32, 32 );
    glPopMatrix();
}

void SplinePoint::drag( wxPoint click, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] )
{
    Vector vs = m_dh->mapMouse2World( click.x, click.y, i_projection, i_viewport, i_modelview );
    Vector ve = m_dh->mapMouse2WorldBack( click.x, click.y, i_projection, i_viewport, i_modelview );
    Vector dir( ve.x - vs.x, ve.y - vs.y, ve.z - vs.z );

    m_center.x = vs.x + dir.x * m_hr.tmin;
    m_center.y = vs.y + dir.y * m_hr.tmin;
    m_center.z = vs.z + dir.z * m_hr.tmin;
    m_dh->m_surfaceIsDirty = true;
}

hitResult SplinePoint::hitTest( Ray* ray )
{
    hitResult hr;

    float cx = m_center.x;
    float cy = m_center.y;
    float cz = m_center.z;

    BoundingBox* bb = new BoundingBox( cx, cy, cz, 6, 6, 6 );
    hr = bb->hitTest( ray );

    if( hr.hit ) 
    {
        hr.picked = 20;
        hr.object = this;
    }

    m_hr = hr;
    return hr;
}

void SplinePoint::move( float w )
{
    m_center.x = m_origin.x + m_offsetVector.x * w;
    m_center.y = m_origin.y + m_offsetVector.y * w;
    m_center.z = m_origin.z + m_offsetVector.z * w;
}


void SplinePoint::select( bool flag )
{
    m_selected = true;

    if( m_treeId && flag )
    {
        m_dh->m_mainFrame->m_treeWidget->SelectItem(m_treeId);
        m_dh->m_mainFrame->m_treeWidget->EnsureVisible(m_treeId);
        m_dh->m_mainFrame->m_treeWidget->SetFocus();
    }
}

void SplinePoint::setX( float x )
{
    m_center.x = x;
    m_origin.x = x;
}


void SplinePoint::moveLeft()
{
    m_center.x -= 1;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveRight()
{
    m_center.x += 1;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveForward()
{
    m_center.y += 1;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveBack()
{
    m_center.y -= 1;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveUp()
{
    m_center.z += 1;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveDown()
{
    m_center.z -= 1;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveLeft5()
{
    m_center.x -= 5;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveRight5()
{
    m_center.x += 5;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveForward5()
{
    m_center.y += 5;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveBack5()
{
    m_center.y -= 5;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveUp5()
{
    m_center.z += 5;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::moveDown5()
{
    m_center.z -= 5;
    m_dh->m_surfaceIsDirty = true;
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void SplinePoint::createPropertiesSizer(PropertiesWindow *parent)
{
    SceneObject::createPropertiesSizer(parent);  
    wxSizer *l_sizer;

    m_ptxtName = new wxTextCtrl(parent, wxID_ANY, getName(),wxDefaultPosition, wxSize(180,-1), wxTE_CENTRE | wxTE_READONLY);    
    m_ptxtName->SetBackgroundColour(*wxLIGHT_GREY);
    wxFont l_font = m_ptxtName->GetFont();
    l_font.SetPointSize(10);
    l_font.SetWeight(wxFONTWEIGHT_BOLD);
    m_ptxtName->SetFont(l_font);
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(m_ptxtName,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);

    wxImage bmpDelete(MyApp::iconsPath+ wxT("delete.png" ), wxBITMAP_TYPE_PNG);
    m_pbtnDelete = new wxBitmapButton(parent, wxID_ANY, bmpDelete, wxDefaultPosition, wxSize(40,-1));
    m_propertiesSizer->Add(m_pbtnDelete,0,wxALIGN_CENTER);
    parent->Connect(m_pbtnDelete->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxTreeEventHandler(PropertiesWindow::OnDeleteTreeItem));

    m_propertiesSizer->Add(new wxStaticText(parent, wxID_ANY, wxT(""),wxDefaultPosition, wxSize(200,15)));

}

void SplinePoint::updatePropertiesSizer()
{
    SceneObject::updatePropertiesSizer();
}