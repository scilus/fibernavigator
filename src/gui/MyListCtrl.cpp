#include "MyListCtrl.h"

#include "MainFrame.h"
#include "SceneManager.h"
#include "SelectionObject.h"
#include "../dataset/DatasetInfo.h"

namespace
{
    const wxString strSelectionObj = wxT( "Selection Objects" );
}

BEGIN_EVENT_TABLE( MyTreeCtrl, wxTreeCtrl )
    EVT_CHAR( MyTreeCtrl::OnChar )
END_EVENT_TABLE()


MyTreeCtrl::MyTreeCtrl( MainFrame *pMainFrame, wxWindow *pParent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) 
:   wxTreeCtrl( pParent, id, pos, size, style )
{
    m_mainFrame = pMainFrame;
}

void MyTreeCtrl::OnChar( wxKeyEvent& evt )
{
    wxTreeItemId treeId = GetSelection();
    
    SelectionObject *pSelObj( NULL );
    
    if( treeId.IsOk() )
    {
        CustomTreeItem *pTreeItem = (CustomTreeItem*) GetItemData( treeId );
        
        if( pTreeItem != NULL )
        {
            pSelObj = SceneManager::getInstance()->getSelectionTree().getObject( pTreeItem->getId() );
        }
    }

    if( pSelObj == NULL )
    {
        evt.Skip();
        return;
    }
    
    if( evt.GetKeyCode() == WXK_DELETE)
    {
        m_mainFrame->deleteTreeItem();
    }
    else
    {
        switch( evt.GetKeyCode() )
        {
        case WXK_LEFT:
             if( wxGetKeyState(WXK_CONTROL) )
                 pSelObj->resizeLeft();
             else
                 pSelObj->moveLeft();
             break;
        case WXK_RIGHT:
            if (wxGetKeyState(WXK_CONTROL))
                pSelObj->resizeRight();
            else
                pSelObj->moveRight();
            break;
        case WXK_UP:
            if (wxGetKeyState(WXK_CONTROL))
                pSelObj->resizeForward();
            else
                pSelObj->moveBack();
            break;
        case WXK_DOWN:
            if (wxGetKeyState(WXK_CONTROL))
                pSelObj->resizeBack();
            else
                pSelObj->moveForward();
            break;
        case WXK_PAGEDOWN:
            if (wxGetKeyState(WXK_CONTROL))
                pSelObj->resizeDown();
            else
                pSelObj->moveUp();
            break;
        case WXK_PAGEUP:
            if (wxGetKeyState(WXK_CONTROL))
                pSelObj->resizeUp();
            else
                pSelObj->moveDown();
            break;
        default:
            evt.Skip();
            return;
        }
    }
    Refresh(false);
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
