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

class NavigationCanvas: public wxGLCanvas
{
public:
	NavigationCanvas( wxWindow *parent, wxWindowID id = wxID_ANY,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize,
         long style = 0, const wxString& name = _T("GLCanvas") );
    bool	m_init;
    bool	m_texture_loaded;

   ~NavigationCanvas(){};

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void updateView(wxPoint, float);
    void init();
    void render();
    void setDataset(TheDataset*, int);
    wxPoint getMousePos();
    
    DECLARE_EVENT_TABLE()

private:
	 wxImage *m_image;
	 float m_xOffset0;
 	 float m_yOffset0;
 	 float m_xOffset1;
 	 float m_yOffset1;
 	 float m_xOffset2;
 	 float m_yOffset2;
	 float m_Slize;
	 TheDataset *m_dataset;
	 int m_view;
	 wxPoint m_clicked;
};

#endif /*NAVIGATIONCANVAS_H_*/
