#include "KdTree.h"

#include <algorithm>

KdTree::KdTree(int size, float *pointArray, DatasetHelper* dh)
{
	m_dh = dh;
	m_size = size;
	m_pointArray = pointArray;
	m_tree = new wxUint32[size];
	for (int i = 0 ; i < size ;  ++i)
		m_tree[i] = i;
	dh->printTime();
	printf ("build kd tree...\n");

	int root = ( size - 1 )/2;
	{
		iter begin( m_tree, 0 );
		iter end( m_tree, size -1 );
		iter nth( m_tree, root );
		std::nth_element( begin, nth, end, lessy( m_pointArray, 0 ) );
	}
	int rootLeft = (root - 1) / 2;
	{
		iter begin( m_tree, 0 );
		iter end( m_tree, root - 1 );
		iter nth( m_tree, rootLeft );
		std::nth_element( begin, nth, end, lessy( m_pointArray, 1 ) );
	}
	int rootRight = ((size - 1) + (root + 1)) / 2;
	{
		iter begin( m_tree, root + 1 );
		iter end( m_tree, size - 1 );
		iter nth( m_tree, rootRight );
		std::nth_element( begin, nth, end, lessy( m_pointArray, 1 ) );
	}
	dh->threadsActive = 4;

	KdTreeThread *thread1 = new KdTreeThread(m_pointArray, m_tree, 0, rootLeft-1, 2, m_dh);
	thread1->Run();

	KdTreeThread *thread2 = new KdTreeThread(m_pointArray, m_tree, rootLeft+1, root-1, 2, m_dh);
	thread2->Run();

	KdTreeThread *thread3 = new KdTreeThread(m_pointArray, m_tree, root+1, rootRight-1, 2, m_dh);
	thread3->Run();

	KdTreeThread *thread4 = new KdTreeThread(m_pointArray, m_tree, rootRight+1, size-1, 2, m_dh);
	thread4->Run();
}

KdTree::KdTree(int size, float *pointArray)
{
	m_size = size;
	m_pointArray = pointArray;
	m_tree = new wxUint32[size];
	for (int i = 0 ; i < size ;  ++i)
		m_tree[i] = i;
	buildTree(0, size-1, 0);
}

KdTree::~KdTree()
{
	delete[] m_tree;
}

void KdTree::buildTree(int left, int right, int axis)
{
	if (left >= right) return;

	int div = ( left+right )/2;
	iter begin( m_tree, left );
	iter end( m_tree, right );
	iter nth( m_tree, div );
	std::nth_element( begin, nth, end, lessy( m_pointArray, axis ) );

	buildTree(left, div-1, (axis+1)%3);
	buildTree(div+1, right, (axis+1)%3);
}



KdTreeThread::KdTreeThread(float *pointArray, wxUint32 *tree, int left, int right, int axis, DatasetHelper* dh)
{
	m_pointArray = pointArray;
	m_tree = tree;
	m_left = left;
	m_right = right;
	m_axis = axis;
	m_dh = dh;

	Create();
}

void* KdTreeThread::Entry()
{
	buildTree(m_left, m_right, m_axis);

	wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, KDTREE_EVENT );
    wxPostEvent( m_dh->mainFrame, event );

	return NULL;
}

void KdTreeThread::buildTree(int left, int right, int axis)
{
    if (left >= right) return;

    int div = ( left+right )/2;
    iter begin( m_tree, left );
    iter end( m_tree, right );
    iter nth( m_tree, div );
    std::nth_element( begin, nth, end, lessy( m_pointArray, axis ) );

    buildTree(left, div-1, (axis+1)%3);
    buildTree(div+1, right, (axis+1)%3);
}
