#ifndef MYLISTCTRL_H_
#define MYLISTCTRL_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/listctrl.h"
#include "wx/imaglist.h"

class MyListCtrl : public wxListCtrl
{
public:
    MyListCtrl(wxWindow *parent,
               const wxWindowID id,
               const wxPoint& pos,
               const wxSize& size,
               long style)
        : wxListCtrl(parent, id, pos, size, style) {};
        
   
        void OnLeftClick(wxMouseEvent& event);
        int getColClicked();
        
private:
	int  m_col_clicked;
   
   DECLARE_EVENT_TABLE()
};

#endif /*MYLISTCTRL_H_*/
