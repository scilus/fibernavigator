/*
 *  The SelectionTree and SelectionTreeNode classes declaration.
 *
 */

#ifndef SELECTIONTREE_H_
#define SELECTIONTREE_H_

#include "SelectionObject.h"

#include <wx/xml/xml.h>

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
    bool removeObject( SelectionObject *pSelObj );
    
    SelectionObject* getObject( const int itemId ) const;

    int getId( SelectionObject *pSelObj ) const;
    
    SelectionObject* getParentObject( SelectionObject *pSelObj ) const;
    
    SelectionObjectVector getAllObjects() const;
    
    SelectionObjectVector getChildrenObjects( const int itemId ) const;
    SelectionObjectVector getChildrenObjects( SelectionObject *pSelObj ) const;
    
    SelectionObjectVector getDirectChildrenObjects( const int itemId ) const;
    
    int getActiveChildrenObjectsCount( SelectionObject *pSelObject ) const;
    
    bool isEmpty() const
    {
        return !m_pRootNode->hasChildren();
    }
    
    bool containsId( const int itemId ) const;
    
    void unselectAll();
    void clear();
    
    void notifyAllObjectsNeedUpdating();
    
    // Methods related to fiber selection.
    vector< bool > getSelectedFibers( const Fibers* const pFibers );
    vector< bool > getSelectedFibersInBranch( const Fibers* const pFibers, SelectionObject* pSelObj );
    
    // Methods related to multiple fibers dataset management.
    bool addFiberDataset(    const SelectionObject::FiberIdType &fiberId, const int fibersCount );
    void removeFiberDataset( const SelectionObject::FiberIdType &fiberId );
    
    // Methods related to stats computation
    void notifyStatsNeedUpdating( SelectionObject *pSelObject );
    
    // Methods related to saving and loading.
    bool populateXMLNode( wxXmlNode *pRootSelObjNode, const wxString &rootPath );
    bool loadFromXMLNode( wxXmlNode *pRootSelObjNode, const wxString &rootPath );
    
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
        SelectionObjectVector getDirectChildrenSelectionObjects() const;
        
        int getActiveDirectChildrenCount() const;

        void addChildren( SelectionTreeNode *pNode );
        bool removeChildren( const int nodeId );
        void removeAllChildren();
        
        bool hasChildren() const;
        
        SelectionTreeNode * const findNode( const int nodeId );
        SelectionTreeNode * const findParentNode( const int searchedChildNodeId );
        
        SelectionTreeNode * const findNode( SelectionObject *pSelObj );
        
        //vector< SelectionTreeNode * const > findGenealogy( SelectionObject *pSelObj );
        
        void updateInObjectRecur( const int fibersCount, Octree *pCurOctree, 
                                 const vector< int > &reverseIdx, const SelectionObject::FiberIdType &fiberId );
        void updateInBranchRecur( const int fibersCount, const SelectionObject::FiberIdType &fiberId );
        
        vector< bool > combineChildrenFiberStates( const SelectionObject::FiberIdType &fiberId ) const;
        
        int getId() const;
        
        bool populateXMLNode( wxXmlNode *pParentNode, const wxString &rootPath );
        bool loadChildrenFromXMLNode( wxXmlNode *pChildContainingNode, 
                                      SelectionTree *pSelTree, 
                                      const wxString &rootPath );
        
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
    vector< SelectionObject* > findGenealogy( SelectionObject *pSelObject );

private:
    SelectionTreeNode *m_pRootNode;
    
    int m_nextNodeId;
    
    map< SelectionObject::FiberIdType, int > m_fibersIdAndCount;
    
    map< SelectionObject::FiberIdType, vector< bool > > m_rootSelectionStatus;
};



#endif // SELECTIONTREE_H_