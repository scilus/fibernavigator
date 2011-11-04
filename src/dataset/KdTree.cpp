#include "KdTree.h"

#include <algorithm>

KdTree::KdTree(int size, float *pointArray, DatasetHelper* dh)
:   m_dh(dh),
    m_size(size),
    m_pointArray(pointArray)
{
    m_tree.clear();
    m_tree.resize(size);
    for (int i = 0 ; i < size ;  ++i)
        m_tree[i] = i;

    m_dh->printDebug(_T("build kd tree..."), MESSAGELEVEL );

    int root = ( size - 1 )/2;
    std::nth_element( m_tree.begin(), m_tree.begin()+root, m_tree.end(), lessy( m_pointArray, 0 ) );

    int rootLeft = (root - 1) / 2;
    std::nth_element( m_tree.begin(), m_tree.begin()+rootLeft, m_tree.begin()+root-1, lessy( m_pointArray, 1 ) );

    int rootRight = (size + root) / 2;
    std::nth_element( m_tree.begin()+root+1, m_tree.begin()+rootRight, m_tree.end(), lessy( m_pointArray, 1 ) );

    dh->m_threadsActive = 4;

    KdTreeThread *thread1 = new KdTreeThread(m_pointArray, &m_tree, 0, rootLeft-1, 2, m_dh);
    thread1->Run();

    KdTreeThread *thread2 = new KdTreeThread(m_pointArray, &m_tree, rootLeft+1, root-1, 2, m_dh);
    thread2->Run();

    KdTreeThread *thread3 = new KdTreeThread(m_pointArray, &m_tree, root+1, rootRight-1, 2, m_dh);
    thread3->Run();

    KdTreeThread *thread4 = new KdTreeThread(m_pointArray, &m_tree, rootRight+1, size-1, 2, m_dh);
    thread4->Run();
}

KdTree::KdTree(int size, float *pointArray)
:   m_size(size),
    m_pointArray(pointArray)
{
    m_tree.clear();
    m_tree.resize(size);
    for (int i = 0 ; i < size ;  ++i)
        m_tree[i] = i;
    buildTree(0, size-1, 0);
}

KdTree::~KdTree()
{

}

void KdTree::buildTree(int left, int right, int axis)
{
    if (left >= right) return;

    int div = ( left+right )/2;
    std::nth_element( m_tree.begin()+left, m_tree.begin()+div, m_tree.begin()+right, lessy( m_pointArray, axis ) );

    buildTree(left, div-1, (axis+1)%3);
    buildTree(div+1, right, (axis+1)%3);
}



KdTreeThread::KdTreeThread(float *pointArray, std::vector<wxUint32>* tree, int left, int right, int axis, DatasetHelper* dh)
:   m_tree(tree),
    m_pointArray(pointArray),
    m_dh(dh),
    m_left(left),
    m_right(right),
    m_axis(axis)
{
    Create();
}

void* KdTreeThread::Entry()
{
    buildTree(m_left, m_right, m_axis);

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, KDTREE_EVENT );
    wxPostEvent( m_dh->m_mainFrame, event );

    return NULL;
}

void KdTreeThread::buildTree(int left, int right, int axis)
{
    if (left >= right) return;

    int div = ( left+right )/2;
    std::nth_element( m_tree->begin()+left, m_tree->begin()+div, m_tree->begin()+right, lessy( m_pointArray, axis ) );

    buildTree(left, div-1, (axis+1)%3);
    buildTree(div+1, right, (axis+1)%3);
}
