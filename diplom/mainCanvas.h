#ifndef MAINCANVAS_H_
#define MAINCANVAS_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/math.h"
#include "wx/glcanvas.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "theDataset.h"

class MainCanvas: public wxGLCanvas
{
public:
	MainCanvas( wxWindow *parent, wxWindowID id = wxID_ANY,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize,
         long style = 0, const wxString& name = _T("GLCanvas") );
    bool	m_init;
    bool	m_texture_loaded;

   ~MainCanvas(){};

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void updateView(int, float);
    void init();
    void render();
    void setDataset(TheDataset*);
    wxPoint getMousePos();
    
    DECLARE_EVENT_TABLE()

private:
	 wxImage *m_image;
	 float m_xOffset;
	 float m_yOffset;
	 float m_zOffset;
	 float m_xSlize;
	 float m_ySlize;
	 float m_zSlize;
	 TheDataset *m_dataset;
	  wxPoint m_clicked;
};

#endif /*MAINCANVAS_H_*/
