#ifndef MYCANVAS_H_
#define MYCANVAS_H_

class MyCanvas: public wxScrolledWindow
{
  public:
    MyCanvas(wxWindow *parent, const wxPoint& pos, const wxSize& size);
    virtual void OnDraw(wxDC& dc);
    void OnEvent(wxMouseEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif /*MYCANVAS_H_*/
