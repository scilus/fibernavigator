
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

#include "PropertiesWindow.h"

class SceneObject
{
public:
    SceneObject();
    virtual ~SceneObject();
    
    virtual void createPropertiesPanel(PropertiesWindow *parent);
    virtual void updatePropertiesPanel() { }

    wxPanel* getPropertiesPanel() const { return m_pPropertiesPanel; }

public:
    wxPanel    *m_pPropertiesPanel;
    wxBoxSizer *m_pPropertiesSizer;

protected:
    void swap( SceneObject &s );

private:
    SceneObject( const SceneObject & );
    SceneObject & operator=( const SceneObject & );
};

#endif
