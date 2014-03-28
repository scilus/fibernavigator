/*
 *  The SelectionTree and SelectionTreeNode classes implementations.
 *
 */

#include "SelectionTree.h"
#include "SelectionObject.h"

#include "../Logger.h"
#include "../dataset/Fibers.h"
#include "../dataset/Octree.h"
#include "../gui/SelectionBox.h"
#include "../gui/SelectionEllipsoid.h"
#include "../gui/SelectionVOI.h"

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

SelectionTree::SelectionObjectVector SelectionTree::SelectionTreeNode::getDirectChildrenSelectionObjects() const
{
    SelectionObjectVector objs;
    
    if( !m_children.empty() )
    {
        for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
        {            
            objs.push_back( m_children[childIdx]->m_pSelObject );
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
    // OPTIM: If we ever encounter a case / dataset where selection is really, really 
    // slow, this should be the first method to be optimized, using a 
    // m_inBranchNeedsUpdating mecanism on the SelectionStates.

    bool atLeastOneActiveIncludeChild( false );
    bool atLeastOneActiveExcludeChild( false );
    vector< bool > childIncludedFibers( fibersCount, false );
    vector< bool > childExcludedFibers( fibersCount, false );
    
    for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
    {
        m_children[ childIdx ]->updateInBranchRecur( fibersCount, fiberId );
        
        if( m_children[ childIdx ]->m_pSelObject->getIsActive() )
        {
            SelectionObject::SelectionState &curChildState = m_children[ childIdx ]->m_pSelObject->getState( fiberId );
            
            if( !m_children[ childIdx ]->m_pSelObject->getIsNOT() )
            {
                atLeastOneActiveIncludeChild = true;
                
                // Merge in inclusion child vector.
                for( unsigned int elemIdx( 0 ); elemIdx < curChildState.m_inBox.size(); ++elemIdx )
                {
                    childIncludedFibers[ elemIdx ] = childIncludedFibers[ elemIdx ] | curChildState.m_inBranch[ elemIdx ];
                }
            }
            else
            {
                atLeastOneActiveExcludeChild = true;
                
                // Merge in exclusion child vector
                for( unsigned int elemIdx( 0 ); elemIdx < curChildState.m_inBox.size(); ++elemIdx )
                {
                    childExcludedFibers[ elemIdx ] = childExcludedFibers[ elemIdx ] | curChildState.m_inBranch[ elemIdx ];
                }
            }
        }
        
    }
    
    // Update self state, if we have a selection object.
    if( m_pSelObject != NULL )
    {
        SelectionObject::SelectionState &curState = m_pSelObject->getState( fiberId );
        
        if( curState.m_inBranch.empty() )
        {
            curState.m_inBranch.assign( fibersCount, false );
        }
        
        // Basic update of the inBranch of this object.
        // If no (active) child object, it will simply be the inBox.
        std::copy( curState.m_inBox.begin(), curState.m_inBox.end(), curState.m_inBranch.begin() );

        if( atLeastOneActiveIncludeChild )
        {
            // TODO what do we do if not active.
            // Combine the child state with the current.
            for( unsigned int elemIdx( 0 ); elemIdx < curState.m_inBox.size(); ++elemIdx )
            {
                curState.m_inBranch[ elemIdx ] = curState.m_inBranch[ elemIdx ] & childIncludedFibers[ elemIdx ];
            }
        }
        
        if( atLeastOneActiveExcludeChild )
        {
            for( unsigned int elemIdx( 0 ); elemIdx < curState.m_inBox.size(); ++elemIdx )
            {
                curState.m_inBranch[ elemIdx ] = curState.m_inBranch[ elemIdx ] & !childExcludedFibers[ elemIdx ];
            }
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

bool SelectionTree::SelectionTreeNode::populateXMLNode( wxXmlNode *pParentNode, const wxString &rootPath )
{
    wxXmlNode *pSelObjNode( NULL );
    
    if( m_pSelObject != NULL )
    {
        // Create node
        pSelObjNode = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "selection_object" ) );
        
        if( m_pSelObject->populateXMLNode( pSelObjNode, rootPath ) )
        {
            pParentNode->AddChild( pSelObjNode );
            pParentNode = pSelObjNode;
        }
        else
        {
            return false;
        }
    }
    
    if( hasChildren() )
    {
        // Create "children" node
        wxXmlNode *pChildNode = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "children_objects" ) );
        pParentNode->AddChild( pChildNode );
        
        // Call this method for each child
        for( unsigned int childIdx( 0 ); childIdx < m_children.size(); ++childIdx )
        {
            if( !m_children[ childIdx ]->populateXMLNode( pChildNode, rootPath ) )
            {
                return false;
            }
        }
    }
    
    return true;
}

bool SelectionTree::SelectionTreeNode::loadChildrenFromXMLNode( wxXmlNode *pChildContainingNode,
                                                                SelectionTree *pSelTree,
                                                                const wxString &rootPath )
{
    if( pChildContainingNode->GetName() != wxT( "children_objects" ) )
    {
        return false;
    }
    
    // Iterate over children selection objects
    wxXmlNode *pSelObjNode = pChildContainingNode->GetChildren();
    while( pSelObjNode != NULL )
    {
        SelectionObject *pLoadedObj;
        
        // Check for node validity
        wxString objType;
        if( !pSelObjNode->GetPropVal( wxT("type"), &objType ) )
        {
            return false;
        }
        
        // Build node and load content
        if( objType == wxT("box") )
        {
            pLoadedObj = new SelectionBox( wxXmlNode( *pSelObjNode ) );
        }
        else if( objType == wxT("ellipsoid") )
        {
            pLoadedObj = new SelectionEllipsoid( wxXmlNode( *pSelObjNode ) );
        }
        else if( objType == wxT("voi") )
        {
            try
            {
                pLoadedObj = new SelectionVOI( wxXmlNode( *pSelObjNode ), rootPath );
            }
            catch( wxString &err )
            {
                Logger::getInstance()->print( err, LOGLEVEL_ERROR );
                pLoadedObj = NULL;
            }
        }
        else
        {
            Logger::getInstance()->print( wxT( "A selection object was skipped, it had an invalid type." ), LOGLEVEL_WARNING );
            pLoadedObj = NULL;
        }
        
        if( pLoadedObj != NULL )
        {
            // Add to children of current node
            int newNodeId = pSelTree->addChildrenObject( m_nodeId, pLoadedObj );

            // If current object has children in the hierarchy, populate them.
            wxXmlNode *pSelObjNodeChildNode = pSelObjNode->GetChildren();
            while(pSelObjNodeChildNode != NULL )
            {
                wxString childName = pSelObjNodeChildNode->GetName();
                if( childName == wxT("children_objects") )
                {
                    SelectionTreeNode *childTreeNode = findNode( newNodeId );
                    childTreeNode->loadChildrenFromXMLNode( pSelObjNodeChildNode, pSelTree, rootPath );
                }
                pSelObjNodeChildNode = pSelObjNodeChildNode->GetNext();
            }
        }
        
        // Get next children
        pSelObjNode = pSelObjNode->GetNext();
    }
    
    return true;
}

SelectionTree::SelectionTreeNode::~SelectionTreeNode()
{
    for( vector< SelectionTreeNode* >::iterator nodeIt( m_children.begin() );
        nodeIt != m_children.end(); 
        ++nodeIt )
    {
        delete *nodeIt;
        *nodeIt = NULL;
    }
    
    if( m_pSelObject != NULL )
    {
        delete m_pSelObject;
        m_pSelObject = NULL;
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
        
        // Increment the nextId.
        ++m_nextNodeId;
        
        // Notify the Selection Object of all existing fiber sets.
        for( map< SelectionObject::FiberIdType, int >::iterator fibIt( m_fibersIdAndCount.begin() ); 
            fibIt != m_fibersIdAndCount.end(); ++fibIt )
        {
            pSelObject->addFiberDataset( (*fibIt).first, (*fibIt).second );
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
        
        return true;
    }
    
    return false;
}

bool SelectionTree::removeObject( SelectionObject *pSelObj )
{
    SelectionTreeNode * const pNode = m_pRootNode->findNode( pSelObj );
    
    return removeObject( pNode->getId() );
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

int SelectionTree::getId( SelectionObject *pSelObj ) const
{
    SelectionTreeNode *pNode = m_pRootNode->findNode( pSelObj );
    
    if( pNode != NULL )
    {
        return pNode->getId();
    }
    
    return -1;
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

SelectionTree::SelectionObjectVector SelectionTree::getChildrenObjects( SelectionObject *pSelObj ) const
{
    SelectionObjectVector selObjs;
    
    SelectionTreeNode *pNode = m_pRootNode->findNode( pSelObj );
    
    if( pNode != NULL )
    {
        selObjs = pNode->getAllChildrenSelectionObjects();
    }
    
    return selObjs;
}

SelectionTree::SelectionObjectVector SelectionTree::getDirectChildrenObjects( const int itemId ) const
{
    SelectionObjectVector selObjs;
    
    SelectionTreeNode *pNode = m_pRootNode->findNode( itemId );
    
    if( pNode != NULL )
    {
        selObjs = pNode->getDirectChildrenSelectionObjects();
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

void SelectionTree::unselectAll()
{
    SelectionObjectVector allObjs = getAllObjects();
    
    for( unsigned int objIdx( 0 ); objIdx < allObjs.size(); ++objIdx )
    {
        allObjs[ objIdx ]->unselect();
    }
}

void SelectionTree::clear()
{
    m_pRootNode->removeAllChildren();
}

void SelectionTree::notifyAllObjectsNeedUpdating()
{
    SelectionObjectVector objs = getAllObjects();
    
    for( SelectionObjectVector::iterator objIt( objs.begin() ); objIt != objs.end(); ++objIt )
    {
        (*objIt)->notifyStatsNeedUpdating();
    }
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
    
    SelectionObject::FiberIdType fiberId = pFibers->getDatasetIndex();
    
    // Update all selection objects to make sure that each of them knows which 
    // fibers is in it.
    m_pRootNode->updateInObjectRecur( fibersCount, pCurOctree, reverseIndex, fiberId );
    
    // Update all selection objects to make sure they take into account their state
    // and the selected fibers of its children.
    m_pRootNode->updateInBranchRecur( fibersCount, fiberId );
    
    // Since the root does not have a selection object, we need to combine each of its
    // children to get the selected fibers.
    m_rootSelectionStatus[ fiberId ] = m_pRootNode->combineChildrenFiberStates( fiberId );
    
    return m_rootSelectionStatus[ fiberId ];
}

vector< bool > SelectionTree::getSelectedFibersInBranch( const Fibers *const pFibers, SelectionObject *pSelObj )
{
    if( pFibers == NULL )
    {
        // TODO determine what we do
    }
    
    if( pSelObj == NULL || !pSelObj->getIsActive() )
    {
        // TODO determine what to do.
    }
    
    // Find the intersection of the root selection and the selection object's
    // inBranch.
    SelectionObject::FiberIdType fiberId = pFibers->getDatasetIndex();
    SelectionObject::SelectionState &childState = pSelObj->getState( fiberId );
    vector< bool > &rootSel = m_rootSelectionStatus[ fiberId ];
    
    vector< bool > selInter( pFibers->getFibersCount(), false );
    
    if( !pSelObj->getIsNOT() )
    {
        for( int fibIdx( 0 ); fibIdx < pFibers->getFibersCount(); ++fibIdx )
        {
            selInter[ fibIdx ] = rootSel[ fibIdx ] & childState.m_inBranch[ fibIdx ];
        }
    }
    else
    {
        for( int fibIdx( 0 ); fibIdx < pFibers->getFibersCount(); ++fibIdx )
        {
            selInter[ fibIdx ] = rootSel[ fibIdx ] & !childState.m_inBranch[ fibIdx ];
        }
    }
    
    return selInter;
}

bool SelectionTree::addFiberDataset( const SelectionObject::FiberIdType &fiberId, const int fibersCount )
{
    SelectionObjectVector selObjs = getAllObjects();
 
    for( SelectionObjectVector::iterator objIt (selObjs.begin()); objIt != selObjs.end(); ++objIt )
    {
        (*objIt)->addFiberDataset( fiberId, fibersCount );
    }
    
    m_rootSelectionStatus.insert( pair< SelectionObject::FiberIdType, vector< bool > >( fiberId, vector< bool >( fibersCount, false ) ) );
    
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
    m_rootSelectionStatus.erase( fiberId );
}

void SelectionTree::notifyStatsNeedUpdating( SelectionObject *pSelObject )
{
    vector< SelectionObject* > genealogy = findGenealogy( pSelObject );
    
    for( vector< SelectionObject* >::iterator nodeIt( genealogy.begin() );
        nodeIt != genealogy.end(); ++nodeIt )
    {
        (*nodeIt)->notifyStatsNeedUpdating();
    }
}

bool SelectionTree::populateXMLNode( wxXmlNode *pRootSelObjNode, const wxString &rootPath )
{
    if( !m_pRootNode->hasChildren() )
    {
        return true;
    }
    
    return m_pRootNode->populateXMLNode( pRootSelObjNode, rootPath );
}

bool SelectionTree::loadFromXMLNode( wxXmlNode *pRootSelObjNode, const wxString &rootPath )
{
    if( m_pRootNode->hasChildren() )
    {
        return false;
    }
    
    wxXmlNode *pRootChildNode = pRootSelObjNode->GetChildren();
    if( pRootChildNode == NULL )
    {
        return true;
    }
    
    m_pRootNode->loadChildrenFromXMLNode( pRootChildNode, this, rootPath );
    
    return true;
}

vector< SelectionObject* > SelectionTree::findGenealogy( SelectionObject *pSelObject )
{
    vector< SelectionObject* > genealogy;
    
    SelectionTreeNode * const curNode = m_pRootNode->findNode( pSelObject );
    bool found( false );
    int childId( -1 );
    
    if( curNode != NULL )
    {
        found = true;
        genealogy.push_back( curNode->getSelectionObject() );
        childId = curNode->getId();
    }
    
    while( found )
    {
        SelectionTreeNode * const parNode = m_pRootNode->findParentNode( childId );
        
        if( parNode != NULL && parNode->getSelectionObject() != NULL )
        {
            found = true;
            genealogy.push_back( parNode->getSelectionObject() );
            childId = parNode->getId();
        }
        else
        {
            found = false;
        }
    }
    
    return genealogy;
}

SelectionTree::~SelectionTree()
{
    delete m_pRootNode;
    m_pRootNode = NULL;
}
