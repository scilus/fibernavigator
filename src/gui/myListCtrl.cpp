#include "myListCtrl.h"
#include "../dataset/datasetInfo.h"
#include "mainFrame.h"
#include "SelectionObject.h"

BEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
	EVT_LEFT_DOWN(MyListCtrl::OnLeftClick)
	//EVT_RIGHT_DOWN(MyListCtrl::OnRightClick)
END_EVENT_TABLE()

MyListCtrl::MyListCtrl(wxWindow *parent, const wxWindowID id, const wxPoint& pos,
            const wxSize& size, long style) :
        wxListCtrl(parent, id, pos, size, style)
{
    m_col_clicked = 0;
    m_col_activated = 0;
}

void MyListCtrl::OnLeftClick(wxMouseEvent& event)
{
	int col;
	int x = event.GetPosition().x;
	int sizeX = 0;
	for ( col = 0 ; col < GetColumnCount() ; ++col)
	{
		sizeX += GetColumnWidth(col);
		if (x <= sizeX) break;
	}
	m_col_clicked = col + 10;
	m_col_activated = col + 10;

	event.Skip();
}

void MyListCtrl::OnRightClick(wxMouseEvent& WXUNUSED(event) )
{

}


int MyListCtrl::getColClicked()
{
	int r = m_col_clicked;
	m_col_clicked = 0;
	return r;
}

int MyListCtrl::getColActivated()
{
	int r = m_col_activated;
	m_col_activated = 0;
	return r;
}


void MyListCtrl::swap(long a, long b)
{
	DatasetInfo *infoA = (DatasetInfo*) GetItemData(a);
	DatasetInfo *infoB = (DatasetInfo*) GetItemData(b);

	SetItem(a, 0, wxT(""), infoB->getShow() ? 0 : 1);
	SetItem(a, 1, infoB->getName());
	SetItem(a, 2, wxString::Format(wxT("%.2f"), infoB->getThreshold()));
	SetItemData(a, (long)infoB);

	SetItem(b, 0, wxT(""), infoA->getShow() ? 0 : 1);
	SetItem(b, 1, infoA->getName());
	SetItem(b, 2, wxString::Format(wxT("%.2f"), infoA->getThreshold()));
	SetItemData(b, (long)infoA);
}

void MyListCtrl::moveItemUp(long item)
{
	if (item == 0) return;
	swap (item - 1, item);
	SetItemState(item - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void MyListCtrl::moveItemDown(long item)
{
	if (item == GetItemCount() - 1) return;
	swap (item, item +1);
	SetItemState(item + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

DECLARE_EVENT_TYPE(wxEVT_TREE_EVENT, -1)
DEFINE_EVENT_TYPE(wxEVT_TREE_EVENT)

BEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
	EVT_CHAR(MyTreeCtrl::OnChar)
	//EVT_RIGHT_DOWN(MyTreeCtrl::OnRightClick)
	EVT_MENU(TREE_CTRL_TOGGLE_ANDNOT, MyTreeCtrl::OnToggleAndNot)
	EVT_MENU(TREE_CTRL_DELETE_BOX, MyTreeCtrl::OnDeleteBox)
END_EVENT_TABLE()


MyTreeCtrl::MyTreeCtrl(wxWindow *parent, const wxWindowID id, const wxPoint& pos,
            const wxSize& size, long style) : wxTreeCtrl(parent, id, pos, size, style)
{

};

int MyTreeCtrl::getSelectedType()
{
	wxTreeItemId treeid = GetSelection();
	if( ! treeid.IsOk() ) 
        return 0;
	if( GetItemText( treeid ) == _T( "points" ) || GetItemText(treeid) == _T( "selection objects" ) ) 
        return 0;

	wxTreeItemId pId = GetItemParent( treeid );
	wxTreeItemId ppId = GetItemParent( pId );

	if( GetItemText(pId) == _T( "selection objects" ) )
		return MASTER_OBJECT;
	else if( GetItemText(pId) == _T( "points" ) )
	        return POINT_DATASET;
	else if( GetItemText(ppId) == _T( "selection objects" ) )
		return CHILD_OBJECT;

	else return 0;
}

void MyTreeCtrl::OnChar( wxKeyEvent& event )
{
	int selected = getSelectedType();

	wxTreeItemId treeid = GetSelection();

	wxTreeItemId pId  = GetItemParent( treeid );
	wxTreeItemId ppId = GetItemParent( pId );

	if( event.GetKeyCode() == WXK_DELETE)
    {
    	if (selected == CHILD_OBJECT)
		{
			((SelectionObject*) ((GetItemData(pId))))->setIsDirty(true);
		}
    	if (selected == CHILD_OBJECT || selected == MASTER_OBJECT || selected == POINT_DATASET)
    	{
			Delete(treeid);
			wxCommandEvent event1( wxEVT_TREE_EVENT, GetId() );
			GetEventHandler()->ProcessEvent( event1 );
    	}
    }

	else if (selected == MASTER_OBJECT || selected == CHILD_OBJECT)
	{
		switch( event.GetKeyCode() )
		{
		case WXK_LEFT:
			 if (wxGetKeyState(WXK_CONTROL))
				 ((SelectionObject*) (GetItemData(treeid)))->resizeLeft();
			 else
				 ((SelectionObject*) (GetItemData(treeid)))->moveLeft();
			 break;
		case WXK_RIGHT:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionObject*) (GetItemData(treeid)))->resizeRight();
			else
				((SelectionObject*) (GetItemData(treeid)))->moveRight();
			break;
		case WXK_UP:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionObject*) (GetItemData(treeid)))->resizeForward();
			else
				((SelectionObject*) (GetItemData(treeid)))->moveBack();
			break;
		case WXK_DOWN:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionObject*) (GetItemData(treeid)))->resizeBack();
			else
				((SelectionObject*) (GetItemData(treeid)))->moveForward();
			break;
		case WXK_PAGEDOWN:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionObject*) (GetItemData(treeid)))->resizeDown();
			else
				((SelectionObject*) (GetItemData(treeid)))->moveUp();
			break;
		case WXK_PAGEUP:
			if (wxGetKeyState(WXK_CONTROL))
				((SelectionObject*) (GetItemData(treeid)))->resizeUp();
			else
				((SelectionObject*) (GetItemData(treeid)))->moveDown();
			break;
		case WXK_HOME:
			((SelectionObject*) (GetItemData(treeid)))->lockToCrosshair();
			break;
		default:
			event.Skip();
			return;
		}
	}
	else if( selected == POINT_DATASET )
    {
        switch( event.GetKeyCode() )
        {
        case WXK_LEFT:
             if (wxGetKeyState(WXK_CONTROL))
                 ((SplinePoint*) (GetItemData(treeid)))->moveLeft5();
             else
                 ((SplinePoint*) (GetItemData(treeid)))->moveLeft();
             break;
        case WXK_RIGHT:
            if (wxGetKeyState(WXK_CONTROL))
                ((SplinePoint*) (GetItemData(treeid)))->moveRight5();
            else
                ((SplinePoint*) (GetItemData(treeid)))->moveRight();
            break;
        case WXK_UP:
            if (wxGetKeyState(WXK_CONTROL))
                ((SplinePoint*) (GetItemData(treeid)))->moveForward5();
            else
                ((SplinePoint*) (GetItemData(treeid)))->moveForward();
            break;
        case WXK_DOWN:
            if (wxGetKeyState(WXK_CONTROL))
                ((SplinePoint*) (GetItemData(treeid)))->moveBack5();
            else
                ((SplinePoint*) (GetItemData(treeid)))->moveBack();
            break;
        case WXK_PAGEDOWN:
            if (wxGetKeyState(WXK_CONTROL))
                ((SplinePoint*) (GetItemData(treeid)))->moveDown5();
            else
                ((SplinePoint*) (GetItemData(treeid)))->moveDown();
            break;
        case WXK_PAGEUP:
            if (wxGetKeyState(WXK_CONTROL))
                ((SplinePoint*) (GetItemData(treeid)))->moveUp5();
            else
                ((SplinePoint*) (GetItemData(treeid)))->moveUp();
            break;
        case WXK_HOME:
            //((SplinePoint*) (GetItemData(treeid)))->lockToCrosshair();
            break;
        default:
            event.Skip();
            return;
        }
    }
    Refresh(false);
}

void MyTreeCtrl::OnRightClick(wxMouseEvent& WXUNUSED(event) )
{

}

void MyTreeCtrl::OnToggleAndNot(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId treeid = GetSelection();
	((SelectionObject*) (GetItemData(treeid)))->toggleIsNOT();
}

void MyTreeCtrl::OnDeleteBox(wxCommandEvent& WXUNUSED(event))
{
	if (getSelectedType() == CHILD_OBJECT)
	{
		((SelectionObject*) ((GetItemData(GetItemParent(GetSelection())))))->setIsDirty(true);
	}
	Delete(GetSelection());
	wxCommandEvent event1( wxEVT_TREE_EVENT, GetId() );
	GetEventHandler()->ProcessEvent( event1 );
}

BEGIN_EVENT_TABLE(MySlider, wxSlider)
	EVT_MOUSE_EVENTS(MySlider::OnMouseEvent)
END_EVENT_TABLE()

void MySlider::OnMouseEvent(wxMouseEvent& event)
{
	if ( event.LeftDown())
	{
		m_leftIsDown = true;
	}
	if ( event.LeftUp())
	{
		m_leftIsDown = false;
	}
	if ( event.LeftIsDown())
	{
		m_leftIsDown = true;
	}
	event.Skip();
}
