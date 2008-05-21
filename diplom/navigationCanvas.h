#ifndef NAVIGATIONCANVAS_H_
#define NAVIGATIONCANVAS_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/math.h"
#include "wx/glcanvas.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "theDataset.h"
#include "theScene.h"

class NavigationCanvas: public wxGLCanvas
{
public:
	NavigationCanvas(TheScene *scene, int view, wxWindow *parent, wxWindowID id = wxID_ANY,
	const wxPoint& pos = wxDefaultPosition,
	const wxSize& size = wxDefaultSize,
	long style = 0, const wxString& name = _T("GLCanvas"),
	int* gl_attrib = NULL);
    

   ~NavigationCanvas(){};

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void init();
    void render();
    void setScene(TheScene*, int);
    wxPoint getMousePos();
    void invalidate();
    
    DECLARE_EVENT_TABLE()

private:
	bool		m_init;
	bool 		m_texturesAssigned;
	
	float 		m_xOffset0;
 	float 		m_yOffset0;
 	float 		m_xOffset1;
 	float 		m_yOffset1;
 	float 		m_xOffset2;
 	float 		m_yOffset2;
	float 		m_Slize;
	TheScene 	*m_scene;
	int 		m_view;
	wxPoint m_clicked;
	wxSize m_oldSize;
};

#endif /*NAVIGATIONCANVAS_H_*/
