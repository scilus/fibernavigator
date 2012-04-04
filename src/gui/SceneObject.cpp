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
:   m_pPropertiesSizer( NULL ),
    m_pBoxPadding( NULL )
{
}

//////////////////////////////////////////////////////////////////////////

void SceneObject::createPropertiesSizer( PropertiesWindow *pParent )
{
    m_pBoxPadding = new wxBoxSizer( wxHORIZONTAL );
    m_pPropertiesSizer = new wxBoxSizer( wxVERTICAL );
    pParent->GetSizer()->Add( m_pBoxPadding, 0, wxEXPAND | wxALL, 1 );
    m_pBoxPadding->Add( m_pPropertiesSizer, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 16 );
}

//////////////////////////////////////////////////////////////////////////

void SceneObject::swap( SceneObject &s )
{
    // Not swapping GUI elements
}

//////////////////////////////////////////////////////////////////////////

SceneObject::~SceneObject()
{
    // Don't delete sizers, they are deleted automatically by the parent
}
