
/////////////////////////////////////////////////////////////////////////////
// Name:            fnObject.h
// Author:          GGirard
// Creation Date:   09/12/2010
//
// Description: Interface to the proprieties panel of the mainFrame
//
// Last modifications:
//      
/////////////////////////////////////////////////////////////////////////////
#ifndef FNOBJECT_H_
#define FNOBJECT_H_

#include <wx/wx.h>

#include "propertiesWindow.h"

class SceneObject
{

public:
    SceneObject();
    wxBoxSizer  *m_propertiesSizer;
    virtual void createPropertiesSizer(PropertiesWindow *parent);
    virtual void updatePropertiesSizer();
    wxBoxSizer* getProprietiesSizer();   
    
};

#endif
