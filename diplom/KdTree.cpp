#include "KdTree.h"
#include "theDataset.h"

#include <omp.h>
#include <algorithm>

KdTree::KdTree(int size, float *pointArray)
{
	m_size = size;
	m_pointArray = pointArray;
	m_tree = new wxUint32[size];
	for (int i = 0 ; i < size ;  ++i)
		m_tree[i] = i;
	TheDataset::printTime();
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
	wxMutex mutex;
	wxCondition condition(mutex);
	mutex.Lock();


	KdTreeThread *thread1 = new KdTreeThread(&mutex, &condition, m_pointArray, m_tree, 0, rootLeft-1, 2);
	thread1->Run();

	KdTreeThread *thread2 = new KdTreeThread(&mutex, &condition, m_pointArray, m_tree, rootLeft+1, root-1, 2);
	thread2->Run();

	KdTreeThread *thread3 = new KdTreeThread(&mutex, &condition, m_pointArray, m_tree, root+1, rootRight-1, 2);
	thread3->Run();

	KdTreeThread *thread4 = new KdTreeThread(&mutex, &condition, m_pointArray, m_tree, rootRight+1, size-1, 2);
	thread4->Run();

	condition.Wait();

	TheDataset::treeFinished();
}

KdTree::~KdTree()
{
	delete[] m_tree;
}

KdTreeThread::KdTreeThread(wxMutex *mutex, wxCondition *condition, float *pointArray, wxUint32 *tree, int left, int right, int axis)
	: wxThread(wxTHREAD_JOINABLE)
{
	m_mutex = mutex;
	m_condition = condition;
	m_pointArray = pointArray;
	m_tree = tree;
	m_left = left;
	m_right = right;
	m_axis = axis;

	Create();
}

void* KdTreeThread::Entry()
{
	buildTree(m_left, m_right, m_axis);
	wxMutexLocker lock(*m_mutex);
	m_condition->Broadcast();
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
