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

#include "fnObject.h"
#include "mainFrame.h"
FNObject::FNObject()
{
    m_propertiesSizer = NULL;
}

wxBoxSizer* FNObject::getProprietiesSizer()
{
    return m_propertiesSizer;
}

void FNObject::createPropertiesSizer( MainFrame *parent )
{
    m_propertiesSizer = new wxBoxSizer( wxVERTICAL );
}

void FNObject::updatePropertiesSizer()
{

}




