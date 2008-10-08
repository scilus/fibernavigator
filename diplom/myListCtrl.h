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

class MyTreeItemData: public wxTreeItemData {
public:
	MyTreeItemData(void* data, int type) :
		m_data(data), m_type(type) {
	}
	void* getData() {
		return m_data;
	}
	;
	int getType() {
		return m_type;
	}
	;
private:
	void* m_data;
	int m_type;
};

class MyTreeCtrl: public wxTreeCtrl {
public:
	MyTreeCtrl(wxWindow *parent, const wxWindowID id, const wxPoint& pos,
			const wxSize& size, long style) :
		wxTreeCtrl(parent, id, pos, size, style) {
	};

private:
	void OnChar(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
}	;

class MyListCtrl: public wxListCtrl {
public:
	MyListCtrl(wxWindow *parent, const wxWindowID id, const wxPoint& pos,
			const wxSize& size, long style) :
		wxListCtrl(parent, id, pos, size, style) {
	};
	void OnLeftClick(wxMouseEvent& event);
	int getColClicked();
	void moveItemUp(long);
	void moveItemDown(long);

private:
	int m_col_clicked;
	void swap(long, long);

	DECLARE_EVENT_TABLE()
}	;

#endif /*MYLISTCTRL_H_*/
