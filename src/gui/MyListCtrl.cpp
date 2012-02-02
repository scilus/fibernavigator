#include "MyListCtrl.h"
#include "../dataset/DatasetInfo.h"
#include "MainFrame.h"
#include "SelectionObject.h"

namespace
{
    const wxString strSelectionObj = wxT( "Selection Objects" );
    const wxString strPoints = wxT( "Points" );
}

BEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
    EVT_CHAR(MyTreeCtrl::OnChar)
    EVT_MENU(TREE_CTRL_TOGGLE_ANDNOT, MyTreeCtrl::OnToggleAndNot)
    EVT_MENU(TREE_CTRL_DELETE_BOX, MyTreeCtrl::OnDeleteBox)
END_EVENT_TABLE()


MyTreeCtrl::MyTreeCtrl(MainFrame *parent, const wxWindowID id, const wxPoint& pos,
            const wxSize& size, long style) : wxTreeCtrl(parent, id, pos, size, style)
{
    m_mainFrame = parent;
};

int MyTreeCtrl::getSelectedType()
{
    wxTreeItemId treeid = GetSelection();
    if( ! treeid.IsOk() ) 
        return 0;
    if( GetItemText( treeid ) == strPoints || GetItemText(treeid) == strSelectionObj ) 
        return 0;

    wxTreeItemId pId = GetItemParent( treeid );
    wxTreeItemId ppId = GetItemParent( pId );

    if( GetItemText(pId) == strSelectionObj )
        return MASTER_OBJECT;
    else if( GetItemText(pId) == strPoints )
            return POINT_DATASET;
    else if( GetItemText(ppId) == strSelectionObj )
        return CHILD_OBJECT;

    else return 0;
}

void MyTreeCtrl::OnChar( wxKeyEvent& evt )
{
    int selected = getSelectedType();

    wxTreeItemId treeid = GetSelection();

    wxTreeItemId pId  = GetItemParent( treeid );
    wxTreeItemId ppId = GetItemParent( pId );

    if( evt.GetKeyCode() == WXK_DELETE)
    {
        m_mainFrame->deleteTreeItem();
    }

    else if (selected == MASTER_OBJECT || selected == CHILD_OBJECT)
    {
        switch( evt.GetKeyCode() )
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
            evt.Skip();
            return;
        }
    }
    else if( selected == POINT_DATASET )
    {
        switch( evt.GetKeyCode() )
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
            evt.Skip();
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
    m_mainFrame->onTreeChange();
}

BEGIN_EVENT_TABLE(MySlider, wxSlider)
    EVT_MOUSE_EVENTS(MySlider::OnMouseEvent)
END_EVENT_TABLE()

void MySlider::OnMouseEvent(wxMouseEvent& evt)
{
    if ( evt.LeftDown())
    {
        m_leftIsDown = true;
    }
    if ( evt.LeftUp())
    {
        m_leftIsDown = false;
    }
    if ( evt.LeftIsDown())
    {
        m_leftIsDown = true;
    }
    evt.Skip();    
}
