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
    long style = 0, const wxString& name = _T("GLCanvas"),
    int* gl_attrib = NULL);
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
	 float m_xOffset0;
	 float m_yOffset0;
	 float m_xOffset1;
	 float m_yOffset1;
	 float m_xOffset2;
	 float m_yOffset2;
	 float m_xSlize;
	 float m_ySlize;
	 float m_zSlize;
	 float m_xTexture;
	 float m_yTexture;
	 float m_zTexture;
	 TheDataset *m_dataset;
	 float *m_texture;
	 wxPoint m_clicked;
};

#endif /*MAINCANVAS_H_*/
