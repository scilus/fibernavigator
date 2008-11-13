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
	EVT_RIGHT_UP(MyTreeCtrl::OnRightClick)
	EVT_MENU(TREE_CTRL_TOGGLE_ANDNOT, MyTreeCtrl::OnToggleAndNot)
	EVT_MENU(TREE_CTRL_DELETE_BOX, MyTreeCtrl::OnDeleteBox)
	EVT_MENU(TREE_CTRL_TOGGLE_BOX_ACTIVE, MyTreeCtrl::OnToggleBoxActive)
	EVT_MENU(TREE_CTRL_TOGGLE_BOX_SHOW, MyTreeCtrl::OnToggleBoxShow)
END_EVENT_TABLE()

int MyTreeCtrl::getSelectedType()
{
	wxTreeItemId treeid = this->GetSelection();

	wxTreeItemId pId = this->GetItemParent(treeid);
	wxTreeItemId ppId = this->GetItemParent(pId);

	if (this->GetItemText(pId) == _T("selection boxes"))
		return MasterBox;
	else if (this->GetItemText(ppId) == _T("selection boxes"))
		return ChildBox;
	else if (this->GetItemText(pId) == _T("points"))
		return Point_;
	else return 0;
}
void MyTreeCtrl::OnChar(wxKeyEvent& event)
{
	int selected = getSelectedType();

	wxTreeItemId treeid = this->GetSelection();

	wxTreeItemId pId = this->GetItemParent(treeid);
	wxTreeItemId ppId = this->GetItemParent(pId);

	if ( event.GetKeyCode() == WXK_DELETE)
    {
    	if (selected == ChildBox)
		{
			((SelectionBox*) ((this->GetItemData(pId))))->setDirty();
		}
		this->Delete(treeid);
		wxCommandEvent event1( wxEVT_TREE_EVENT, GetId() );
		GetEventHandler()->ProcessEvent( event1 );
    }

	else if (selected == MasterBox || selected == ChildBox)
	{
		switch( event.GetKeyCode() )
		{
		case WXK_LEFT:
			 if (wxGetKeyState(WXK_CONTROL))
				 ((SelectionBox*) (this->GetItemData(treeid)))->resizeLeft();
			 else
				 ((SelectionBox*) (this->GetItemData(treeid)))->moveLeft();
			 break;
		case WXK_RIGHT:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionBox*) (this->GetItemData(treeid)))->resizeRight();
			else
				((SelectionBox*) (this->GetItemData(treeid)))->moveRight();
			break;
		case WXK_UP:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionBox*) (this->GetItemData(treeid)))->resizeForward();
			else
				((SelectionBox*) (this->GetItemData(treeid)))->moveForward();
			break;
		case WXK_DOWN:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionBox*) (this->GetItemData(treeid)))->resizeBack();
			else
				((SelectionBox*) (this->GetItemData(treeid)))->moveBack();
			break;
		case WXK_PAGEUP:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionBox*) (this->GetItemData(treeid)))->resizeUp();
			else
				((SelectionBox*) (this->GetItemData(treeid)))->moveUp();
			break;
		case WXK_PAGEDOWN:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionBox*) (this->GetItemData(treeid)))->resizeDown();
			else
				((SelectionBox*) (this->GetItemData(treeid)))->moveDown();
			break;
		case WXK_HOME:
			((SelectionBox*) (this->GetItemData(treeid)))->lockToCrosshair();
			break;
		default:
			event.Skip();
			return;
		}
	}
    Refresh(false);
}

void MyTreeCtrl::OnRightClick(wxMouseEvent& event)
{
	int selected = getSelectedType();

	wxMenu* menu = new wxMenu;

	if (selected == MasterBox)
	{
		SelectionBox* box =((SelectionBox*) (this->GetItemData( this->GetSelection())));
		if (box->getActive())
			menu->Append(TREE_CTRL_TOGGLE_BOX_ACTIVE, _T("deactivate"));
		else
			menu->Append(TREE_CTRL_TOGGLE_BOX_ACTIVE, _T("activate"));
		if (box->getShow())
			menu->Append(TREE_CTRL_TOGGLE_BOX_SHOW, _T("hide"));
		else
			menu->Append(TREE_CTRL_TOGGLE_BOX_SHOW, _T("show"));

		menu->AppendSeparator();
		menu->Append(TREE_CTRL_DELETE_BOX, _T("delete"));
	}

	if (selected == ChildBox)
	{
		SelectionBox* box =((SelectionBox*) (this->GetItemData( this->GetSelection())));
		menu->Append(TREE_CTRL_TOGGLE_ANDNOT, _T("toggle AND/NOT"));
		menu->AppendSeparator();
		if (box->getActive())
			menu->Append(TREE_CTRL_TOGGLE_BOX_ACTIVE, _T("deactivate"));
		else
			menu->Append(TREE_CTRL_TOGGLE_BOX_ACTIVE, _T("activate"));
		if (box->getShow())
			menu->Append(TREE_CTRL_TOGGLE_BOX_SHOW, _T("hide"));
		else
			menu->Append(TREE_CTRL_TOGGLE_BOX_SHOW, _T("show"));
		menu->AppendSeparator();
		menu->Append(TREE_CTRL_DELETE_BOX, _T("delete"));

	}
	PopupMenu(menu, event.GetPosition());
}

void MyTreeCtrl::OnToggleAndNot(wxCommandEvent& event)
{
	wxTreeItemId treeid = this->GetSelection();
	((SelectionBox*) (this->GetItemData(treeid)))->toggleNOT();
}

void MyTreeCtrl::OnDeleteBox(wxCommandEvent& event)
{
	if (getSelectedType() == ChildBox)
	{
		((SelectionBox*) ((this->GetItemData(this->GetItemParent(this->GetSelection())))))->setDirty();
	}
	this->Delete(this->GetSelection());
	wxCommandEvent event1( wxEVT_TREE_EVENT, GetId() );
	GetEventHandler()->ProcessEvent( event1 );
}

void MyTreeCtrl::OnToggleBoxActive(wxCommandEvent& event)
{
	wxTreeItemId treeid = this->GetSelection();
	((SelectionBox*) (this->GetItemData(treeid)))->toggleActive();
	((SelectionBox*) (this->GetItemData(treeid)))->setDirty();

	if (getSelectedType() == MasterBox)
	{
		int childboxes = GetChildrenCount(treeid);
		wxTreeItemIdValue childcookie = 0;
		for (int i = 0 ; i < childboxes ; ++i)
		{
			wxTreeItemId childId = GetNextChild(treeid, childcookie);
			if (childId.IsOk()) {
				SelectionBox *childBox = ((SelectionBox*)(GetItemData(childId)));
				childBox->toggleActive();
				childBox->setDirty();
			}
		}
	}
}

void MyTreeCtrl::OnToggleBoxShow(wxCommandEvent& event)
{
	wxTreeItemId treeid = this->GetSelection();
	((SelectionBox*) (this->GetItemData(treeid)))->toggleShow();
}
