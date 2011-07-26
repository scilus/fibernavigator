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

#include "sceneObject.h"
SceneObject::SceneObject()
{
    m_propertiesSizer = NULL;
}

wxBoxSizer* SceneObject::getProprietiesSizer()
{
    return m_propertiesSizer;
}

void SceneObject::createPropertiesSizer( PropertiesWindow *parent )
{
    m_propertiesSizer = new wxBoxSizer( wxVERTICAL );
    m_propertiesSizer->SetMinSize( wxSize(210,15));
    
}

void SceneObject::updatePropertiesSizer()
{

}




