#ifndef TOOLBAR_H_
#define TOOLBAR_H_

#include <wx/wx.h>
#include <wx/toolbar.h>

class MainFrame;

// Please note that this class is now a container for a wxToolbar, instead
// of deriving from it, to overcome a problem with showing the Toolbar on OSX.
class ToolBar
{
public:
    ToolBar(wxToolBar *pToolBar);
    void connectToolsEvents(MainFrame *mf);
    void updateToolBar(MainFrame *mf);
    
    void updateDrawerToolBar( const bool drawingActive );
    void setColorPickerColor( const wxColour &color );

public:
    wxTextCtrl        *m_txtRuler;
    
private:    // Private UI controls
    wxToolBarToolBase *m_btnOpen;
    wxToolBarToolBase *m_toggleShowAxial;
    wxToolBarToolBase *m_toggleShowCoronal;
    wxToolBarToolBase *m_toggleShowSagittal;
    wxToolBarToolBase *m_toggleAlphaBlending;
    wxToolBarToolBase *m_btnNewSelectionBox;
    wxToolBarToolBase *m_btnNewSelectionEllipsoid;
    wxToolBarToolBase *m_toggleInverseSelection;
    wxToolBarToolBase *m_toggleShowAllSelectionObjects;
    wxToolBarToolBase *m_toggleActivateAllSelectionObjects;
    wxToolBarToolBase *m_toggleLighting;
    wxToolBarToolBase *m_toggleFakeTubes;
    wxToolBarToolBase *m_toggleClearToBlack;
    wxToolBarToolBase *m_selectNormalPointer;
    wxToolBarToolBase *m_selectRuler;
    wxToolBarToolBase *m_selectDrawer;
    wxToolBarToolBase *m_selectColorPicker;
    wxToolBarToolBase *m_toggleDrawRound;
    wxToolBarToolBase *m_toggleDraw3d;
    wxToolBarToolBase *m_selectPen;
    wxToolBarToolBase *m_selectEraser;
    
    wxImage  m_drawColorIcon;

private:
    wxToolBar *m_pToolBar;
};


#endif
