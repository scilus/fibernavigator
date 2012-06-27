/*
 *  The SelectionTree and SelectionTreeNode classes declaration.
 *
 */

#ifndef SELECTIONTREE_H_
#define SELECTIONTREE_H_

#include "SelectionObject.h"

#include <map>
using std::map;
#include <vector>
using std::vector;

class Fibers;
class Octree;

class SelectionTree
{
public:
    typedef vector< SelectionObject* > SelectionObjectVector;
    
public:
    SelectionTree();
    ~SelectionTree();
    
    int addChildrenObject( const int parentId, SelectionObject *pSelObject );
    
    bool removeObject( const int nodeId );
    
    SelectionObject* getObject( const int itemId ) const;
    
    SelectionObject* getParentObject( SelectionObject *pSelObj ) const;
    
    SelectionObjectVector getAllObjects() const;
    
    SelectionObjectVector getChildrenObjects( const int itemId ) const;
    
    int getActiveChildrenObjectsCount( SelectionObject *pSelObject ) const;
    
    bool isEmpty() const
    {
        return !m_pRootNode->hasChildren();
    }
    
    bool containsId( const int itemId ) const;
    
    // Methods related to fiber selection.
    vector< bool > getSelectedFibers( const Fibers* const pFibers );
    
    // Methods related to multiple fibers dataset management.
    bool addFiberDataset(    const SelectionObject::FiberIdType &fiberId, const int fibersCount );
    void removeFiberDataset( const SelectionObject::FiberIdType &fiberId );
    
    // Methods related to stats computation
    void notifyStatsNeedUpdating();
    
    // Methods related to saving and loading.
    // TODO selection tree
    //bool populateXMLNode( wxXmlNode *pRootSelObjNode );
    //bool loadFromXMLNode( wxXmlNode *pRootSelObjNode, DatasetHelper *pDH );
    
private:
    class SelectionTreeNode
    {
    public:
        SelectionTreeNode( const int id, SelectionObject *pSelObject );
        ~SelectionTreeNode();
        
        void setSelectionObject( SelectionObject *pSelObject );
        SelectionObject* getSelectionObject() const;
        
        SelectionObjectVector getAllSelectionObjects() const;
        SelectionObjectVector getAllChildrenSelectionObjects() const;
        
        int getActiveDirectChildrenCount() const;

        void addChildren( SelectionTreeNode *pNode );
        bool removeChildren( const int nodeId );
        void removeAllChildren();
        
        bool hasChildren() const;
        
        SelectionTreeNode * const findNode( const int nodeId );
        SelectionTreeNode * const findParentNode( const int searchedChildNodeId );
        
        SelectionTreeNode * const findNode( SelectionObject *pSelObj );
        
        void updateInObjectRecur( const int fibersCount, Octree *pCurOctree, 
                                 const vector< int > &reverseIdx, const SelectionObject::FiberIdType &fiberId );
        void updateInBranchRecur( const int fibersCount, const SelectionObject::FiberIdType &fiberId );
        
        vector< bool > combineChildrenFiberStates( const SelectionObject::FiberIdType &fiberId ) const;
        
        int getId() const;
        
        // TODO selection tree
        //bool populateXMLNode( wxXmlNode *pParentNode );
        
    private:
        SelectionTreeNode();    // Disable default constructor.
        
    private:
        int m_nodeId;
        SelectionObject *m_pSelObject;
        vector< SelectionTreeNode* > m_children;
    };
    
    // Structure used with the find_if algorithm.
    struct SelectionTreeNodeFinder
    {
    public:
        SelectionTreeNodeFinder( const int searchedNodeId )
        : m_searchedId( searchedNodeId )
        {}
        
        bool operator() ( const SelectionTreeNode *treeNode )
        {
            return treeNode->getId() == m_searchedId;
        }
        
    private:
        int m_searchedId;
    };
    
private:
    SelectionTreeNode *m_pRootNode;
    
    int m_nextNodeId;
    
    map< SelectionObject::FiberIdType, int > m_fibersIdAndCount;
};



#endif // SELECTIONTREE_H_