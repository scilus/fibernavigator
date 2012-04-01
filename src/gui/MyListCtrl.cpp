#include "MyListCtrl.h"

#include "MainFrame.h"
#include "SelectionObject.h"
#include "../dataset/DatasetInfo.h"

namespace
{
    const wxString strSelectionObj = wxT( "Selection Objects" );
}

BEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
    EVT_CHAR(MyTreeCtrl::OnChar)
    EVT_MENU(TREE_CTRL_TOGGLE_ANDNOT, MyTreeCtrl::OnToggleAndNot)
    EVT_MENU(TREE_CTRL_DELETE_BOX, MyTreeCtrl::OnDeleteBox)
END_EVENT_TABLE()


MyTreeCtrl::MyTreeCtrl( MainFrame *pMainFrame, wxWindow *pParent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) 
:   wxTreeCtrl( pParent, id, pos, size, style )
{
    m_mainFrame = pMainFrame;
}

int MyTreeCtrl::getSelectedType()
{
    wxTreeItemId treeId = GetSelection();
    if( !treeId.IsOk() ) 
        return 0;
    if( GetItemText( treeId ) == strSelectionObj ) 
        return 0;

    wxTreeItemId pId =  GetItemParent( treeId );
    wxTreeItemId ppId = GetItemParent( pId );

    if( GetItemText( pId ) == strSelectionObj )
        return MASTER_OBJECT;
    else if( GetItemText( ppId ) == strSelectionObj )
        return CHILD_OBJECT;

    else return 0;
}

void MyTreeCtrl::OnChar( wxKeyEvent& evt )
{
    int selected = getSelectedType();

    wxTreeItemId treeId = GetSelection();

    if( evt.GetKeyCode() == WXK_DELETE)
    {
        m_mainFrame->deleteTreeItem();
    }
    else if( selected == MASTER_OBJECT || selected == CHILD_OBJECT )
    {
        switch( evt.GetKeyCode() )
        {
        case WXK_LEFT:
             if( wxGetKeyState(WXK_CONTROL) )
                 ((SelectionObject*) (GetItemData(treeId)))->resizeLeft();
             else
                 ((SelectionObject*) (GetItemData(treeId)))->moveLeft();
             break;
        case WXK_RIGHT:
            if (wxGetKeyState(WXK_CONTROL))
                ((SelectionObject*) (GetItemData(treeId)))->resizeRight();
            else
                ((SelectionObject*) (GetItemData(treeId)))->moveRight();
            break;
        case WXK_UP:
            if (wxGetKeyState(WXK_CONTROL))
                ((SelectionObject*) (GetItemData(treeId)))->resizeForward();
            else
                ((SelectionObject*) (GetItemData(treeId)))->moveBack();
            break;
        case WXK_DOWN:
            if (wxGetKeyState(WXK_CONTROL))
                ((SelectionObject*) (GetItemData(treeId)))->resizeBack();
            else
                ((SelectionObject*) (GetItemData(treeId)))->moveForward();
            break;
        case WXK_PAGEDOWN:
            if (wxGetKeyState(WXK_CONTROL))
                ((SelectionObject*) (GetItemData(treeId)))->resizeDown();
            else
                ((SelectionObject*) (GetItemData(treeId)))->moveUp();
            break;
        case WXK_PAGEUP:
            if (wxGetKeyState(WXK_CONTROL))
                ((SelectionObject*) (GetItemData(treeId)))->resizeUp();
            else
                ((SelectionObject*) (GetItemData(treeId)))->moveDown();
            break;
        case WXK_HOME:
            ((SelectionObject*) (GetItemData(treeId)))->lockToCrosshair();
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
    if( getSelectedType() == CHILD_OBJECT )
    {
        ((SelectionObject*) ((GetItemData(GetItemParent(GetSelection())))))->setIsDirty(true);
    }
    Delete( GetSelection() );
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
