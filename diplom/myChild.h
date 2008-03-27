#ifndef MYCHILD_H_
#define MYCHILD_H_

#include "myCanvas.h"

class MyChild: public wxMDIChildFrame
{
  public:
    MyCanvas *canvas;
    MyChild(wxMDIParentFrame *parent, const wxString& title, const wxPoint& pos, const wxSize& size, const long style);
    ~MyChild(void);
    void OnActivate(wxActivateEvent& event);
    void OnQuit(wxCommandEvent& event);

DECLARE_EVENT_TABLE()
};

#define SASHTEST_CHILD_QUIT  4

#endif /*MYCHILD_H_*/
