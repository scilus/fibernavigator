#ifndef MYLISTCTRL_H_
#define MYLISTCTRL_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/listctrl.h"
#include "wx/treectrl.h"
#include "wx/imaglist.h"
#include "datasetInfo.h"

class MyTreeItemData : public wxTreeItemData
{
public:
	MyTreeItemData (void* data) : m_data(data) {}
		void* getData() {return m_data;};
private:
	void* m_data;
};

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
        void moveItemUp(long);
        void moveItemDown(long);
        
private:
	int  m_col_clicked;
	void swap(long, long);
   
   DECLARE_EVENT_TABLE()
};

#endif /*MYLISTCTRL_H_*/
