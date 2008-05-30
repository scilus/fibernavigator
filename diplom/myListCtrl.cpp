#include "myListCtrl.h"
#include "datasetInfo.h"
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
