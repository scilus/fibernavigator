
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

class MainFrame;
class FNObject
{

public:
    FNObject();
    wxBoxSizer  *m_propertiesSizer;
    virtual void createPropertiesSizer(MainFrame *parent);
    virtual void updatePropertiesSizer();
    wxBoxSizer* getProprietiesSizer();   
    
};

#endif
