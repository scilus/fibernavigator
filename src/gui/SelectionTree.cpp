/*
 *  The SelectionTree and SelectionTreeNode classes implementations.
 *
 */

#include "SelectionTree.h"
#include "SelectionObject.h"

#include "../dataset/Fibers.h"
#include "../dataset/Octree.h"
#include "../gui/SelectionBox.h"
#include "../gui/SelectionEllipsoid.h"

#include <algorithm>
#include <utility>
using std::pair;

// Anonymous namespace
namespace {
    void combineBoolVectors( vector< bool > &dest, const vector< bool > &source, const bool negateSrc, const bool useAnd )
    {
        if( dest.size() != source.size() )
        {
            return;
        }

        for( unsigned int elemIdx( 0 ); elemIdx < dest.size(); ++elemIdx )
        {
            if( !negateSrc && !useAnd )
            {
                dest[elemIdx] = dest[elemIdx] | source[elemIdx];
            }
            else if( !negateSrc && useAnd )
            {
                dest[elemIdx] = dest[elemIdx] & source[elemIdx];
            }
            else if( negateSrc && useAnd )
            {
                dest[elemIdx] = dest[elemIdx] & !source[elemIdx];
            }
            else // Negate and use Or
            {
                dest[elemIdx] = dest[elemIdx] | !source[elemIdx];
            }
        }
    }
};

/////
// SelectionTreeNode methods
/////

SelectionTree::SelectionTreeNode::SelectionTreeNode( const int id, SelectionObject *pSelObject )
    : m_nodeId( id ),
      m_pSelObject( pSelObject )
{}

void SelectionTree::SelectionTreeNode::setSelectionObject( SelectionObject *pSelObject )
{
    m_pSelObject = pSelObject;
}

SelectionObject* SelectionTree::SelectionTreeNode::getSelectionObject() const
{
    return m_pSelObject;
}

SelectionTree::SelectionObjectVector SelectionTree::SelectionTreeNode::getAllSelectionObjects() const
{
    SelectionObjectVector objs;
    
    if( m_pSelObject != NULL )
    {
        objs.push_back( m_pSelObject );
    }
    
    SelectionObjectVector childObjs = getAllChildrenSelectionObjects();
    objs.insert( objs.end(), childObjs.begin(), childObjs.end() );
    
    return objs;
}

SelectionTree::SelectionObjectVector SelectionTree::SelectionTreeNode::getAllChildrenSelectionObjects() const
{
    SelectionObjectVector objs;
    
    if( !m_children.empty() )
    {
        for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
        {
            SelectionObjectVector childObjs = m_children[childIdx]->getAllSelectionObjects();
            
            objs.insert( objs.end(), childObjs.begin(), childObjs.end() );
        }
    }
    
    return objs;
}

int SelectionTree::SelectionTreeNode::getActiveDirectChildrenCount() const
{
    int count( 0 );
    
    for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
    {
        if( m_children[ childIdx ]->m_pSelObject->getIsActive() )
        {
            ++count;
        }
    }
    
    return count;
}

void SelectionTree::SelectionTreeNode::addChildren( SelectionTreeNode *pNode )
{
    m_children.push_back( pNode );
}

bool SelectionTree::SelectionTreeNode::removeChildren( const int nodeId )
{
    vector< SelectionTreeNode* >::iterator foundPos = std::find_if( m_children.begin(), 
                                                                    m_children.end(),
                                                                   SelectionTreeNodeFinder( nodeId ) );
    
    if( foundPos != m_children.end() )
    {
        delete *foundPos;
        *foundPos = NULL;
        
        m_children.erase( foundPos );
        
        // TODO update selection
        
        return true;
    }
    
    return false;
}

void SelectionTree::SelectionTreeNode::removeAllChildren()
{
    for( vector< SelectionTreeNode* >::iterator nodeIt( m_children.begin() );
         nodeIt != m_children.end(); 
         ++nodeIt )
    {
        delete *nodeIt;
        *nodeIt = NULL;
    }
    
    m_children.clear();
}

bool SelectionTree::SelectionTreeNode::hasChildren() const
{
    return !m_children.empty();
}

SelectionTree::SelectionTreeNode * const
    SelectionTree::SelectionTreeNode::findNode( const int nodeId )
{
    if( getId() == nodeId )
    {
        return this;
    }
    
    for( unsigned int nodeIdx(0); nodeIdx < m_children.size(); ++nodeIdx )
    {
        SelectionTreeNode * const pReturnedNode = m_children[nodeIdx]->findNode( nodeId );
        
        if( pReturnedNode != NULL )
        {
            return pReturnedNode;
        }
    }
    
    return NULL;
}

SelectionTree::SelectionTreeNode * const
    SelectionTree::SelectionTreeNode::findParentNode( const int searchedChildNodeId )
{
    for( unsigned int childNodeIdx( 0 ); childNodeIdx < m_children.size(); ++childNodeIdx )
    {
        if( m_children[childNodeIdx]->getId() == searchedChildNodeId )
        {
            return this;
        }
        else
        {
            SelectionTreeNode *pFoundNode = m_children[childNodeIdx]->findParentNode( searchedChildNodeId );
            
            if( pFoundNode != NULL )
            {
                return pFoundNode;
            }
        }
    }
    
    return NULL;
}

SelectionTree::SelectionTreeNode * const
    SelectionTree::SelectionTreeNode::findNode( SelectionObject *pSelObj )
{
    if( m_pSelObject == pSelObj )
    {
        return this;
    }
    
    for( unsigned int nodeIdx(0); nodeIdx < m_children.size(); ++nodeIdx )
    {
        SelectionTreeNode * const pReturnedNode = m_children[nodeIdx]->findNode( pSelObj );
        
        if( pReturnedNode != NULL )
        {
            return pReturnedNode;
        }
    }
    
    return NULL;
}

void SelectionTree::SelectionTreeNode::updateInObjectRecur( const int fibersCount, Octree *pCurOctree, const vector< int > &reverseIdx, const SelectionObject::FiberIdType &fiberId )
{
    if( m_pSelObject != NULL )
    {
        SelectionObject::SelectionState &curState = m_pSelObject->getState( fiberId );
        
        if( curState.m_inBoxNeedsUpdating )
        {
            vector< int > pointsInsideObject = pCurOctree->getPointsInside( m_pSelObject );
            
            curState.m_inBox.assign( fibersCount, false );
            
            for( unsigned int ptIdx( 0 ); ptIdx < pointsInsideObject.size(); ++ptIdx )
            {
                curState.m_inBox[ reverseIdx[ pointsInsideObject[ ptIdx ] ] ] = true;
            }
        }
        
        curState.m_inBoxNeedsUpdating = false;
    }
    
    // Call this recursively for all children.
    for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
    {
        m_children[ childIdx ]->updateInObjectRecur( fibersCount, pCurOctree, reverseIdx, fiberId );
    }
}

void SelectionTree::SelectionTreeNode::updateInBranchRecur( const int fibersCount,
                                                            const SelectionObject::FiberIdType &fiberId )
{
    // TODO find a way to check if dirty and propagate
    
    vector< bool > childInBranch( fibersCount, false );
    bool atLeastOneActiveChildren( false );

    // Call update for all children.
    for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
    {
        m_children[ childIdx ]->updateInBranchRecur( fibersCount, fiberId );
        
        if( m_children[ childIdx ]->m_pSelObject->getIsActive() )
        {
            // Get the inBranch and the state, and combine.
            // First false probably not needed.
            // Get current state for specific fiber dataset.
            SelectionObject::SelectionState &curChildState = m_children[ childIdx ]->m_pSelObject->getState( fiberId ); 
            combineBoolVectors( childInBranch, curChildState.m_inBranch, false, false );
            atLeastOneActiveChildren = true;
        }
    }
    
    if( m_pSelObject != NULL )
    {
        SelectionObject::SelectionState &curState = m_pSelObject->getState( fiberId );
        curState.m_inBranch.assign( fibersCount, false );
        
        if( m_children.empty() || !atLeastOneActiveChildren )
        {
            // This node does not have any active child.
            // Use the elements in the current box to update the m_inBranch.
            bool negate( m_pSelObject->getIsNOT() );
            
            for( unsigned int elemIdx( 0 ); elemIdx < curState.m_inBox.size(); ++elemIdx )
            {
                curState.m_inBranch[ elemIdx ] = ( !negate ? curState.m_inBox[ elemIdx ] :
                                                   !curState.m_inBox[ elemIdx ] );
            }
            
            return;
        }
    
        // Combine the child state with the current.
        bool negate( m_pSelObject->getIsNOT() );

        for( unsigned int elemIdx( 0 ); elemIdx < curState.m_inBox.size(); ++elemIdx )
        {
            curState.m_inBranch[ elemIdx ] = ( !negate ? curState.m_inBox[ elemIdx ] :
                                               !curState.m_inBox[ elemIdx ] ) & childInBranch[ elemIdx ];
        }
    }
}

vector< bool > SelectionTree::SelectionTreeNode::combineChildrenFiberStates(const SelectionObject::FiberIdType &fiberId ) const
{
    vector< bool > combinedStates;
    
    if( m_children.empty() )
    {
        return combinedStates;
    }
    
    SelectionObject::SelectionState tempChildState = m_children[0]->m_pSelObject->getState( fiberId );

    combinedStates.assign( tempChildState.m_inBox.size(), false );
    
    for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
    {
        if( m_children[ childIdx ]->m_pSelObject->getIsActive() )
        {
            // Get the inBranch and the state, and combine.
            // First false probably not needed.
            SelectionObject::SelectionState &curChildState = m_children[childIdx]->m_pSelObject->getState( fiberId );
            combineBoolVectors( combinedStates, curChildState.m_inBranch, false, false );
        }
    }
    
    return combinedStates;
}

int SelectionTree::SelectionTreeNode::getId() const
{
    return m_nodeId;
}

/*bool SelectionTree::SelectionTreeNode::populateXMLNode( wxXmlNode *pParentNode )
{
    wxXmlNode *pSelObjNode( NULL );
    
    if( m_pSelObject != NULL )
    {
        // Check if the parent node already has children
        wxXmlNode *pCurChild = pParentNode->GetChildren();
        wxXmlNode *pNextChild( NULL );

        // Iterate over all children, to find the last.

        if( pCurChild != NULL )
        {
            pNextChild = pCurChild->GetNext();
            
            while( pNextChild != NULL )
            {
                pCurChild = pNextChild;
                pNextChild = pCurChild->GetNext();
            }
        }
                
        // Create node
        pSelObjNode = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "selection_object" ) );
        
        m_pSelObject->populateXMLNode( pSelObjNode );
        
        pParentNode->InsertChildAfter( pSelObjNode, pCurChild );
    }
    
    if( hasChildren() )
    {
        // Create "children" node
        wxXmlNode *pChildNode( pParentNode );
        
        // The root object is the only SelectionTreeNode which is still valid
        // without a Selection Object
        if( pSelObjNode != NULL )
        {
            pChildNode = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "children_objects" ) );
            
            // Check if the parent node already has children
            wxXmlNode *pCurChild = pSelObjNode->GetChildren();
            wxXmlNode *pNextChild( NULL );
            
            // Iterate over all children, to find the last.
            
            if( pCurChild != NULL )
            {
                pNextChild = pCurChild->GetNext();
                
                while( pNextChild != NULL )
                {
                    pCurChild = pNextChild;
                    pNextChild = pCurChild->GetNext();
                }
            }

            pCurChild->SetNext( pChildNode );
        }
        
        // Call this method for each child
        for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
        {
            m_children[ childIdx ]->populateXMLNode( pChildNode );
        }
    }
    
    return true;
}

// This assumes that the selection tree has been emptied.
bool SelectionTree::loadFromXMLNode( wxXmlNode *pRootSelObjNode, DatasetHelper *pDH )
{
    wxXmlNode *pRootChildNode = pRootSelObjNode->GetChildren();
    
    while( pRootChildNode != NULL )
    {
        // Check if valid selection object node.
        if( !pRootChildNode->HasProp( wxT( "name" ) ) || !pRootChildNode->HasProp( wxT( "type" ) ) )
        {
            // TODO, how do we react if that is not the case?
        }
        
        // Get the type.
        wxString selObjType = pRootChildNode->GetPropVal( wxT( "type" ), wxT( "invalid" ) );
        
        SelectionObject *pNewSelObj( NULL );
        if( selObjType == "selectionBox" )
        {
            pNewSelObj = new SelectionBox( pDH );
        }
        else if( selObjType == "selectionEllipsoid" )
        {
            pNewSelObj = new SelectionEllipsoid( pDH );
        }
        else if( selObjType == "selectionVOI" )
        {
            // TODO implement
        }
        
        if( pNewSelObj != NULL )
        {
            // TODO check return value
            if( pNewSelObj->loadFromXMLNode( pRootChildNode ) )
            {
                addChildrenObject( -1, pNewSelObj );
                // TODO add to the tree widget
            }
            else
            {
                delete pNewSelObj;
                pNewSelObj = NULL;
            }
        }
        
        pRootChildNode = pRootChildNode->GetNext();
    }
    
    return true;
}*/

SelectionTree::SelectionTreeNode::~SelectionTreeNode()
{
    // TODO delete selection object. Ownership should be transferred to the selection tree.
    for( vector< SelectionTreeNode* >::iterator nodeIt( m_children.begin() );
        nodeIt != m_children.end(); 
        ++nodeIt )
    {
        delete *nodeIt;
        *nodeIt = NULL;
    }
}

/////
// SelectionTree methods
/////

SelectionTree::SelectionTree()
    : m_nextNodeId( 0 )
{
    m_pRootNode = new SelectionTreeNode( m_nextNodeId, NULL );
    ++m_nextNodeId;
}

// To add a node to the first layer of the SelectionTree, use -1 as the id.
int SelectionTree::addChildrenObject( const int parentId, SelectionObject *pSelObject )
{
    if( pSelObject == NULL )
    {
        return -1;
    }
    
    SelectionTreeNode *pParentNode( NULL );
    
    if( parentId == -1 )
    {
        pParentNode = m_pRootNode;
    }
    else
    {
        // Find the node with id, if possible.
        pParentNode = m_pRootNode->findNode( parentId );
    }
    
    // If found, create the children node with the next id and set its selection object.
    if( pParentNode != NULL )
    {        
        // Add the node to the children of the current.
        SelectionTreeNode *pChildrenNode = new SelectionTreeNode( m_nextNodeId, pSelObject );
        pParentNode->addChildren( pChildrenNode );
        
        // TODO update selection
        
        // Increment the nextId.
        ++m_nextNodeId;
        
        // Notufy the Selection Object of all existing fiber sets.
        for( map< SelectionObject::FiberIdType, int >::iterator fibIt( m_fibersIdAndCount.begin() ); 
            fibIt != m_fibersIdAndCount.end(); ++fibIt )
        {
            pSelObject->addFiberDataset( (*fibIt).first );
        }
            
        return pChildrenNode->getId();
    }

    return -1;
}

bool SelectionTree::removeObject( const int nodeId )
{
    SelectionTreeNode * const pParentNode = m_pRootNode->findParentNode( nodeId );

    // If found, remove it
    if( pParentNode != NULL )
    {
        pParentNode->removeChildren( nodeId );
        
        // TODO Update selection
        
        return true;
    }
    
    return false;
}

SelectionObject* SelectionTree::getObject( const int itemId ) const
{
    SelectionTreeNode *pNode = m_pRootNode->findNode( itemId );
    
    if( pNode != NULL )
    {
        return pNode->getSelectionObject();
    }
    
    return NULL;
}

SelectionObject* SelectionTree::getParentObject( SelectionObject *pSelObj ) const
{
    SelectionTreeNode * const pTreeNode = m_pRootNode->findNode( pSelObj );
    
    if( pTreeNode != NULL )
    {
        SelectionTreeNode * const pParentNode = m_pRootNode->findParentNode( pTreeNode->getId() );
        
        if( pParentNode != NULL )
        {
            return pParentNode->getSelectionObject();
        }
    }
    
    return NULL;
}

SelectionTree::SelectionObjectVector SelectionTree::getAllObjects() const
{
    SelectionObjectVector selObj;
    
    selObj = m_pRootNode->getAllSelectionObjects();

    return selObj;
}

SelectionTree::SelectionObjectVector SelectionTree::getChildrenObjects( const int itemId ) const
{
    SelectionObjectVector selObjs;
    
    SelectionTreeNode *pNode = m_pRootNode->findNode( itemId );
        
    if( pNode != NULL )
    {
        selObjs = pNode->getAllChildrenSelectionObjects();
    }
    
    return selObjs;
}

int SelectionTree::getActiveChildrenObjectsCount( SelectionObject *pSelObj ) const
{
    int activeChildrenCount( 0 );
    
    SelectionTreeNode * const pTreeNode = m_pRootNode->findNode( pSelObj );
 
    if( pTreeNode != NULL )
    {
        activeChildrenCount = pTreeNode->getActiveDirectChildrenCount();
    }
    
    return activeChildrenCount;
}

bool SelectionTree::containsId( const int itemId ) const
{
    SelectionTreeNode *pFoundNode = m_pRootNode->findNode( itemId );
    
    return pFoundNode != NULL;
}

vector< bool > SelectionTree::getSelectedFibers( const Fibers* const pFibers )
{
    if( pFibers == NULL )
    {
        // TODO determine what we do
    }
        
    const int fibersCount( pFibers->getFibersCount() );
    
    // This should never happen.
    if( isEmpty() || m_pRootNode->getActiveDirectChildrenCount() == 0 )
    {
        // TODO print warning message.
        return vector< bool >( fibersCount, true );
    }
    
    const vector< int > reverseIndex( pFibers->getReverseIdx() );
    
    Octree *pCurOctree( pFibers->getOctree() );
    
    SelectionObject::FiberIdType fiberId = const_cast< Fibers* >(pFibers)->getName();
    
    // Update all selection objects to make sure that each of them knows which 
    // fibers is in it.
    m_pRootNode->updateInObjectRecur( fibersCount, pCurOctree, reverseIndex, fiberId );
    
    // Update all selection objects to make sure they take into account their state
    // and the selected fibers of its children.
    m_pRootNode->updateInBranchRecur( fibersCount, fiberId );
    
    // Since the root does not have a selection object, we need to combine each of its
    // children to get the selected fibers.
    vector< bool > combinedChildrenStates = m_pRootNode->combineChildrenFiberStates( fiberId );
    
    return combinedChildrenStates;
}

bool SelectionTree::addFiberDataset( const SelectionObject::FiberIdType &fiberId, const int fibersCount )
{
    SelectionObjectVector selObjs = getAllObjects();
 
    for( SelectionObjectVector::iterator objIt (selObjs.begin()); objIt != selObjs.end(); ++objIt )
    {
        (*objIt)->addFiberDataset( fiberId );
    }
    
    return ( m_fibersIdAndCount.insert( pair< SelectionObject::FiberIdType, int >( fiberId, fibersCount ) ) ).second;
}

void SelectionTree::removeFiberDataset( const SelectionObject::FiberIdType &fiberId )
{
    if( m_fibersIdAndCount.count( fiberId ) > 0 )
    {
        SelectionObjectVector selObjs = getAllObjects();
        
        for( SelectionObjectVector::iterator objIt (selObjs.begin()); objIt != selObjs.end(); ++objIt )
        {
            (*objIt)->removeFiberDataset( fiberId );
        }
    }
    
    m_fibersIdAndCount.erase( fiberId );
}

void SelectionTree::notifyStatsNeedUpdating()
{
    SelectionObjectVector objs = getAllObjects();
    
    for( SelectionObjectVector::iterator objIt( objs.begin() ); objIt != objs.end(); ++objIt )
    {
        (*objIt)->notifyStatsNeedUpdating();
    }
}

// TODO selection tree
/*bool SelectionTree::populateXMLNode( wxXmlNode *pRootSelObjNode )
{
    if( !m_pRootNode->hasChildren() )
    {
        return true;
    }
    
    return m_pRootNode->populateXMLNode( pRootSelObjNode );
}*/

SelectionTree::~SelectionTree()
{
    delete m_pRootNode;
    m_pRootNode = NULL;
}
