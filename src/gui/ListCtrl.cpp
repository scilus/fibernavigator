#include "ListCtrl.h"

#include "../Logger.h"
#include "../dataset/DatasetInfo.h"
#include "../dataset/FibersGroup.h"

#include <set>
using std::set;

#include "../dataset/DatasetManager.h"

#include <wx/imaglist.h>
#include <wx/string.h>
#include <vector>
using std::vector;

BEGIN_EVENT_TABLE( ListCtrl, wxListCtrl )
    EVT_LEFT_DOWN( ListCtrl::onLeftClick )
    EVT_LIST_ITEM_ACTIVATED( ID_LIST_CTRL, ListCtrl::onActivate )
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

ListCtrl::ListCtrl( wxWindow *pParent, const wxPoint &point, const wxSize &size, const long style )
: wxListCtrl( pParent, ID_LIST_CTRL, point, size, style ),
  m_column( 0 )
{
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

void ListCtrl::AssignImageList( wxImageList *imageList, int which )
{
    wxListCtrl::AssignImageList( imageList, which );
}

//////////////////////////////////////////////////////////////////////////

bool ListCtrl::DeleteItem( long index )
{
    DatasetIndex dsIndex  = GetItem( index );
    DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( dsIndex );

    if( FIBERSGROUP == pDataset->getType() )
    {
        DatasetIndex dsFiberIndex;
        while( index + 1 < GetItemCount() && FIBERS == DatasetManager::getInstance()->getDataset( dsFiberIndex = GetItem( index + 1 ) )->getType() )
        {
            DatasetManager::getInstance()->remove( dsFiberIndex );
            wxListCtrl::DeleteItem( index + 1 );
        }
    }
    DatasetManager::getInstance()->remove( dsIndex );
    return wxListCtrl::DeleteItem( index );
}


//////////////////////////////////////////////////////////////////////////

void ListCtrl::DeleteSelectedItem()
{
    long index = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( -1 != index )
    {
        DeleteItem( index );
    }
}

//////////////////////////////////////////////////////////////////////////

long ListCtrl::InsertColumn( long col, wxListItem& info )
{
    return wxListCtrl::InsertColumn( col, info );
}

//////////////////////////////////////////////////////////////////////////
void ListCtrl::InsertItem( DatasetIndex datasetIndex )
{
    // insert at zero is a well-known bug on OSX, so we append there...
    // http://trac.wxwidgets.org/ticket/4492
    // To have the same behavior on all platforms, we add to the end of the list
    long index( GetItemCount() );
    long pos = index;

    DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( datasetIndex );

    if( FIBERS == pDataset->getType() && DatasetManager::getInstance()->isFibersGroupLoaded() )
    {
        long fiberGroupPos = FindFiberGroupPosition();
        if( -1 != fiberGroupPos )
        {
            pos = fiberGroupPos + 1;
            while( pos < GetItemCount() )
            {
                DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( GetItem( pos ) );
                if ( FIBERS != pDataset->getType() )
                    break;

                ++pos;
            }
        }
    }
	
    wxListCtrl::InsertItem( index, pDataset->getShow() ? 0 : 1 );
    SetItemData( index, datasetIndex );

    for( long i = index; i != pos; --i )
    {
        Swap( i, i - 1);
        Update( i );
    }

    SetItemState( pos, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    Update( pos );
}

//////////////////////////////////////////////////////////////////////////

void ListCtrl::InsertItemRange( const vector<DatasetIndex> &items )
{
    for( vector<DatasetIndex>::const_iterator it = items.begin(); it != items.end(); ++it )
    {
        InsertItem( *it );
    }
}

//////////////////////////////////////////////////////////////////////////

void ListCtrl::MoveItemDown()
{
    // NOTE: Important to use the swap to move items because the MainFrame catches DeleteItems events
    // to delete the index from the DatasetManager.

    long index = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( index < GetItemCount() - 1 )
    {
        set<long> refreshNeeded;

        if( FIBERSGROUP == DatasetManager::getInstance()->getDataset( GetItem( index ) )->getType() )
        {
            refreshNeeded.insert( index );
            long from = index + 1;
            while( from < GetItemCount() && FIBERS == DatasetManager::getInstance()->getDataset( GetItem( from ) )->getType() )
            {
                refreshNeeded.insert( from );
                ++from;
            }
            if( from < GetItemCount() )
            {
                refreshNeeded.insert( from );
                for( int i = from; i > index; --i )
                {
                    Swap( i, i - 1 );
                }

                SetItemState( index + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
        }
        else if( FIBERS == DatasetManager::getInstance()->getDataset( GetItem( index ) )->getType() )
        {
            if( FIBERS == DatasetManager::getInstance()->getDataset( GetItem( index + 1 ) )->getType() )
            {
                Swap( index, index + 1 );
                refreshNeeded.insert( index );
                refreshNeeded.insert( index + 1 );
                SetItemState( index + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
        }
        else
        {
            if( FIBERSGROUP == DatasetManager::getInstance()->getDataset( GetItem( index + 1 ) )->getType() )
            {
                Swap( index, index + 1 );
                refreshNeeded.insert( index );
                refreshNeeded.insert( index + 1 );
                ++index;

                while( index + 1 < GetItemCount() && FIBERS == DatasetManager::getInstance()->getDataset( GetItem( index + 1 ) )->getType() )
                {
                    Swap( index, index + 1 );
                    refreshNeeded.insert( index + 1 );
                    ++index;
                }
                SetItemState( index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
            else
            {
                Swap( index, index + 1 );
                refreshNeeded.insert( index );
                refreshNeeded.insert( index + 1 );
                SetItemState( index + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
        }

        // Refresh items
        for( set<long>::iterator it = refreshNeeded.begin(); it != refreshNeeded.end(); ++it )
        {
            Update( *it );
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void ListCtrl::MoveItemUp()
{
    // NOTE: Important to use the swap to move items because the MainFrame catches DeleteItems events
    // to delete the index from the DatasetManager.

    long index = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( 0 < index )
    {
        set<long> refreshNeeded;

        DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( GetItem( index ) );

        if( FIBERSGROUP == pDataset->getType() )
        {
            // Move fiber group
            Swap( index, index - 1 );
            SetItemState( index - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

            refreshNeeded.insert( index );
            refreshNeeded.insert( index - 1);

            ++index;

            while( index < GetItemCount() && FIBERS == DatasetManager::getInstance()->getDataset( GetItem( index ) )->getType() )
            {
                // Move fibers
                Swap( index, index - 1 );
                refreshNeeded.insert( index );

                ++index;
            }
        }
        else if( FIBERS == pDataset->getType() )
        {
            if( FIBERS == DatasetManager::getInstance()->getDataset( GetItem( index - 1 ) )->getType() )
            {
                Swap( index, index - 1 );
                refreshNeeded.insert( index );
                refreshNeeded.insert( index - 1);
                SetItemState( index - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
        }
        else
        {
            if( FIBERS == DatasetManager::getInstance()->getDataset( GetItem( index - 1 ) )->getType() )
            {
                Swap( index, index - 1 );
                refreshNeeded.insert( index );
                refreshNeeded.insert( index - 1);

                --index;

                while( FIBERSGROUP != DatasetManager::getInstance()->getDataset( GetItem( index - 1 ) )->getType() )
                {
                    Swap( index, index - 1 );
                    refreshNeeded.insert( index - 1 );
                    --index;
                }

                Swap( index, index - 1 );
                refreshNeeded.insert( index - 1 );

                SetItemState( index - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
            else
            {                
                Swap( index, index - 1 );
                refreshNeeded.insert( index );
                refreshNeeded.insert( index - 1);
                SetItemState( index - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
        }

        // Refresh items
        for( set<long>::iterator it = refreshNeeded.begin(); it != refreshNeeded.end(); ++it )
        {
            Update( *it );
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void ListCtrl::UnselectAll()
{
    long index = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    wxListCtrl::SetItemState( index, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED );
}

//////////////////////////////////////////////////////////////////////////

void ListCtrl::UpdateFibers()
{
    for( long index( 0 ); index < GetItemCount(); ++index )
    {
        if( FIBERS == DatasetManager::getInstance()->getDataset( GetItem( index ) )->getType() )
        {
            Update( index );
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void ListCtrl::UpdateSelected()
{
    long index = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( -1 != index )
    {
        DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( GetItem( index ) );
        if( FIBERSGROUP == pDataset->getType() )
        {
            for( long i( index + 1); i < GetItemCount() && FIBERS == DatasetManager::getInstance()->getDataset( GetItem( i ) )->getType(); ++i )
            {
                Update( i );
            }
        }

        Update( index );
    }
}

void ListCtrl::SelectItem( long item )
{
    wxListCtrl::SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

//////////////////////////////////////////////////////////////////////////
// GETTERS/SETTERS
//////////////////////////////////////////////////////////////////////////

DatasetIndex ListCtrl::GetItem( long index ) const
{
    if( 0 > index || index >= wxListCtrl::GetItemCount() )
    {
        return DatasetIndex();
    }

    return DatasetIndex( GetItemData( index ) );
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void ListCtrl::onActivate( wxListEvent& evt )
{
    Logger::getInstance()->print( _T( "Event triggered - ListCtrl::onActivate" ), LOGLEVEL_DEBUG );

    int index = evt.GetIndex();
    DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( GetItem( index ) );

    switch( m_column )
    {        
    case 0:
        pDataset->toggleShow();
        if( FIBERSGROUP == pDataset->getType() )
        {
            FibersGroup *pFibersGroup = (FibersGroup *)pDataset;
            pFibersGroup->OnToggleVisibleBtn();
            unsigned int count = DatasetManager::getInstance()->getFibersCount();
            for( unsigned int i = 1; i <= count; ++i )
            {
                Update( i + index );
            }
        }
        Update( index );
        break;
    case 1:
        pDataset->toggleShowFS();
        Update( index );
        break;
    case 3:
    default:
        break;
    }

    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////

void ListCtrl::onLeftClick( wxMouseEvent& evt )
{
    int posX = evt.GetPosition().x;
    int sizeX( 0 );
    for( int col( 0 ); col < GetColumnCount(); ++col)
    {
        sizeX += GetColumnWidth(col);
        if( posX <= sizeX )
        {
            m_column = col;
            break;
        }
    }
    evt.Skip();
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

long ListCtrl::FindFiberGroupPosition()
{
    for( long pos( 0 ); pos < GetItemCount(); ++pos )
    {
        DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( GetItem( pos ) );
        if ( FIBERSGROUP == pDataset->getType() )
        {
            return pos;
        }
    }
    return -1;
}

void ListCtrl::Swap( long i, long j )
{
    long tmp = GetItemData( i );
    SetItemData( i, GetItemData( j ) );
    SetItemData( j, tmp );
}

void ListCtrl::Update( long index )
{
    DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( GetItem( index ) );
    SetItem( index, 0, wxT( "" ), pDataset->getShow() ? 0 : 2 );
    SetItem( index, 1, pDataset->getName().BeforeFirst('.') + ( pDataset->getShowFS() ? wxT( "" ) : wxT( "*" ) ) );
    SetItem( index, 2, wxString::Format( wxT( "%.2f" ), pDataset->getThreshold() * pDataset->getOldMax() ) );
    SetItem( index, 3, wxT( "" ), 2 );
}

//////////////////////////////////////////////////////////////////////////
// DESTRUCTOR
//////////////////////////////////////////////////////////////////////////

ListCtrl::~ListCtrl()
{
}
