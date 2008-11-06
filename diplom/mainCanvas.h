#ifndef MAINCANVAS_H_
#define MAINCANVAS_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "theScene.h"
#include "mainFrame.h"
#include "wx/glcanvas.h"

#include "boundingBox.h"
#include "DatasetHelper.h"
#include "wx/math.h"
#include "Fantom/FVector.h"

class DatasetHelper;

class MainCanvas: public wxGLCanvas
{
public:

    MainCanvas(DatasetHelper*, int,  wxWindow*, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0, const wxString& name = _T("GLCanvas"),
        int* gl_attrib = NULL,
        wxGLCanvas* shared=( wxGLCanvas* )NULL);
   ~MainCanvas();

   	bool	m_init;

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    wxPoint getMousePos() {return m_clicked;};
    float getDelta() {return m_delta;};
    int getPicked() {return m_hr.picked;};
    void updateView();
    void init();
    void render();
    void invalidate();
    void renderTestRay();
    hitResult pick(wxPoint);
    float getAxisParallelMovement(int, int, int, int, Vector3fT);
    Vector3fT getEventCenter();
    void setRotation();
    void orthonormalize();
    double scalar(const F::FVector&, const F::FVector&);

    void testRender(GLuint tex);

    DECLARE_EVENT_TABLE()

private:
	 int 		m_view;			// identifier of gl widget
	 wxPoint 	m_clicked;		// last clicked point
	 wxPoint	m_lastPos;		// last position to calculate right mouse dragging
	 float 		m_delta;
	 hitResult  m_hr;


	 GLdouble m_pos1X, m_pos1Y, m_pos1Z;	// point of origin for picking
	 GLdouble m_pos2X, m_pos2Y, m_pos2Z;	// distant point for picking

	 //Matrix4fT m_transform;		// transformation matrix of current view
	 Matrix3fT m_lastRot;
	 Matrix3fT m_thisRot ;

	 ArcBallT   *m_arcBall;
	 Point2fT    m_mousePt;

	 DatasetHelper* m_dh;;
};

#endif /*MAINCANVAS_H_*/
