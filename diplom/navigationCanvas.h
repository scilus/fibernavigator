#ifndef NAVIGATIONCANVAS_H_
#define NAVIGATIONCANVAS_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/timer.h"
#include "wx/math.h"
#include "wx/glcanvas.h"
#include <GL/gl.h>
#include <GL/glu.h>


class NavigationCanvas: public wxGLCanvas
{
public:
	NavigationCanvas( wxWindow *parent, wxWindowID id = wxID_ANY,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize,
         long style = 0, const wxString& name = _T("TestGLCanvas") );
    bool	m_init;
    bool	m_texture_loaded;

   ~NavigationCanvas(){};

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void init();
    void render();
    void setTextureImage(wxImage*);
    
   
    DECLARE_EVENT_TABLE()
private:
	 wxImage *m_image;
	 float m_xOffset;
	 float m_yOffset;
};

#endif /*NAVIGATIONCANVAS_H_*/
