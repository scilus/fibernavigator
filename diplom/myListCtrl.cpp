#include "myListCtrl.h"
#include "datasetInfo.h"
#include "mainFrame.h"
#include "selectionBox.h"
BEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
	EVT_LEFT_DOWN(MyListCtrl::OnLeftClick)
END_EVENT_TABLE()



void MyListCtrl::OnLeftClick(wxMouseEvent& event)
{
	int col;
	int x = event.GetPosition().x;
	int sizeX = 0;
	for ( col = 0 ; col < this->GetColumnCount() ; ++col)
	{
		sizeX += this->GetColumnWidth(col);
		if (x <= sizeX) break;
	}
	m_col_clicked = col;
	
	event.Skip();
}

int MyListCtrl::getColClicked()
{
	return m_col_clicked;
}

void MyListCtrl::swap(long a, long b)
{
	DatasetInfo *infoA = (DatasetInfo*) this->GetItemData(a);
	DatasetInfo *infoB = (DatasetInfo*) this->GetItemData(b);
	
	this->SetItem(a, 0, wxT(""), infoB->getShow() ? 0 : 1);
	this->SetItem(a, 1, infoB->getName());
	this->SetItem(a, 2, wxString::Format(wxT("%.2f"), infoB->getThreshold()));
	this->SetItemData(a, (long)infoB);
	
	this->SetItem(b, 0, wxT(""), infoA->getShow() ? 0 : 1);
	this->SetItem(b, 1, infoA->getName());
	this->SetItem(b, 2, wxString::Format(wxT("%.2f"), infoA->getThreshold()));
	this->SetItemData(b, (long)infoA);
}

void MyListCtrl::moveItemUp(long item)
{
	if (item == 0) return;
	swap (item - 1, item);
	this->SetItemState(item - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void MyListCtrl::moveItemDown(long item)
{
	if (item == this->GetItemCount() - 1) return;
	swap (item, item +1);
	this->SetItemState(item + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

DECLARE_EVENT_TYPE(wxEVT_TREE_EVENT, -1)
DEFINE_EVENT_TYPE(wxEVT_TREE_EVENT)

BEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
	EVT_CHAR(MyTreeCtrl::OnChar)
END_EVENT_TABLE()

void MyTreeCtrl::OnChar(wxKeyEvent& event)
{
	switch( event.GetKeyCode() )
    {
    case WXK_DELETE: {
    	wxTreeItemId treeid = this->GetSelection();
    	wxTreeItemId parentid = this->GetItemParent(treeid);
		MyTreeItemData *data = (MyTreeItemData*)this->GetItemData(treeid);
		if (!data) return;
		if (this->GetItemText(treeid) == wxT("box")) {
			if (this->GetItemText(parentid) == wxT("box"))
			{
				((SelectionBox*) (((MyTreeItemData*)this->GetItemData(parentid))->getData()))->setDirty();
			}
			this->Delete(treeid);
			wxCommandEvent event1( wxEVT_TREE_EVENT, GetId() );
			GetEventHandler()->ProcessEvent( event1 );
		}
    } break;
    default:
	    event.Skip();
	    return;
    }
    Refresh(false);
}

