#ifndef MAINCANVAS_H_
#define MAINCANVAS_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "theScene.h"

#include "wx/math.h"
#include "wx/glcanvas.h"
#include "boundingBox.h"


class MainCanvas: public wxGLCanvas
{
public:
	
    MainCanvas(TheScene*, int,  wxWindow*, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0, const wxString& name = _T("GLCanvas"),
        int* gl_attrib = NULL);
   ~MainCanvas(){};
   
   	bool	m_init;
   	
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    wxPoint getMousePos() {return m_clicked;};
    float getDelta() {return m_delta;};
    int getPicked() {return m_picked;};
    void updateView(int, float);
    void init();
    void render();
    void setScene(TheScene*);
    void invalidate();
    void renderTestRay();
    int pick(wxPoint);
    Vector3fT mapMouse2World(int, int);
    Vector3fT mapMouse2WorldBack(int, int);
    float getAxisParallelMovement(int, int, int, int, Vector3fT);
    
    DECLARE_EVENT_TABLE()

private:
	 int 		m_view;			// identifier of gl widget
	 TheScene 	*m_scene;
	 wxPoint 	m_clicked;		// last clicked point 
	 wxPoint	m_lastPos;		// last position to calculate right mouse dragging
	 float 		m_delta;
	 
	 int 		m_picked;
	 
	 GLdouble m_pos1X, m_pos1Y, m_pos1Z;	// point of origin for picking
	 GLdouble m_pos2X, m_pos2Y, m_pos2Z;	// distant point for picking
	 float m_tpicked;		// distance variables for bb test
	
	 bool m_isDragging;
	 bool m_isrDragging;
	 
	 Matrix4fT m_transform;		// transformation matrix of current view
	 Matrix3fT m_lastRot;  		
	 Matrix3fT m_thisRot ; 

	 ArcBallT   *m_arcBall;
	 Point2fT    m_mousePt;
};

#endif /*MAINCANVAS_H_*/
