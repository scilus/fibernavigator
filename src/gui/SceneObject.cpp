/////////////////////////////////////////////////////////////////////////////
// Name:            fnObject.cpp
// Author:          GGirard
// Creation Date:   09/12/2010
//
// Description: Interface to the proprieties panel of the mainFrame
//
// Last modifications:
//      
/////////////////////////////////////////////////////////////////////////////

#include "SceneObject.h"

SceneObject::SceneObject()
:   m_pPropertiesPanel( NULL ),
    m_pPropertiesSizer( NULL )
{
}

//////////////////////////////////////////////////////////////////////////

void SceneObject::createPropertiesPanel( PropertiesWindow *pParent )
{
    m_pPropertiesPanel = new wxPanel( pParent );
    wxBoxSizer *pPadding = new wxBoxSizer( wxHORIZONTAL );
    m_pPropertiesSizer = new wxBoxSizer( wxVERTICAL );
    pPadding->Add( m_pPropertiesSizer, 1, wxEXPAND | wxRIGHT | wxLEFT | wxALIGN_CENTER_HORIZONTAL, 16 );
    m_pPropertiesPanel->SetSizer( pPadding );
}

//////////////////////////////////////////////////////////////////////////

void SceneObject::swap( SceneObject &s )
{
    // Not swapping GUI elements
}

//////////////////////////////////////////////////////////////////////////

SceneObject::~SceneObject()
{
}
