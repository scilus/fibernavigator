/////////////////////////////////////////////////////////////////////////////
// Name:            ToolBar.h
// Author:          GGirard
// Creation Date:   30/11/2010
//
// Description: MainFrame ToolBar
//
// Last modifications:
//      
/////////////////////////////////////////////////////////////////////////////

#ifndef TOOLBAR_H_
#define TOOLBAR_H_

#include <wx/wx.h>
#include <wx/toolbar.h>

class MainFrame;
class ToolBar : public wxToolBar
{
public:
    ToolBar(wxWindow *parent);
    void initToolBar(MainFrame *mf);
    void updateToolBar(MainFrame *mf);

public:
    wxToolBarToolBase *m_btnOpen;
    wxToolBarToolBase *m_toggleShowAxial;
    wxToolBarToolBase *m_toggleShowCoronal;
    wxToolBarToolBase *m_toggleShowSagittal;
    wxToolBarToolBase *m_toggleAlphaBlending;
    wxToolBarToolBase *m_btnNewSelectionBox;
    wxToolBarToolBase *m_toggleInverseSelection;
    wxToolBarToolBase *m_toggleShowAllSelectionObjects;
    wxToolBarToolBase *m_toggleActivateAllSelectionObjects;
    //wxToolBarToolBase *m_btnNewSplineSurface;
    //wxToolBarToolBase *m_toggleDrawPoints;
    //wxToolBarToolBase *m_btnMoveBoundaryPointLeft;
    //wxToolBarToolBase *m_btnMoveBoundaryPointRight;
    wxToolBarToolBase *m_toggleLighting;
    wxToolBarToolBase *m_toggleFakeTubes;
    wxToolBarToolBase *m_toggleClearToBlack;
	wxToolBarToolBase *m_selectRuler;
	wxToolBarToolBase *m_selectDrawer;
	wxTextCtrl        *m_txtRuler;
	wxToolBarToolBase *m_selectPen;
	wxToolBarToolBase *m_selectEraser;
};


#endif