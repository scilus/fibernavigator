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
:   m_propertiesSizer( NULL )
{
}

//////////////////////////////////////////////////////////////////////////

void SceneObject::createPropertiesSizer( PropertiesWindow *parent )
{
    m_propertiesSizer = new wxBoxSizer( wxVERTICAL );
    m_propertiesSizer->SetMinSize( wxSize(210,15));
}
