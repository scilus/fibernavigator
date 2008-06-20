#include "KdTree.h"

KdTree::KdTree(int size, float *pointArray)
{
	printf ("build kd tree\n");
	m_pointArray = pointArray;
	m_tree = new wxUint32[size];
	for (int i = 0 ; i < size ;  ++i)
		m_tree[i] = i;
	buildTree(0,size-1, 0);
}

KdTree::~KdTree()
{
	delete[] m_tree;
}

void KdTree::quicksort(int left, int right, int pos)
{
	if (left < right)
	{
		int div = quicksortSplit(left, right, pos);
		quicksort(left, div-1, pos);
		quicksort(div+1, right, pos);
	}
}

int KdTree::quicksortSplit(int left, int right, int pos)
{
	float pivotValue = m_pointArray[3*m_tree[right] + pos];
	int i = left;
	int j = right -1;
	while (i < j)
	{
		while ( (m_pointArray[3*m_tree[i] + pos] <= pivotValue) && (i < right) )
		{
			++i;
		}
		while ( (m_pointArray[3*m_tree[j] + pos] > pivotValue) && (j > left) )
		{
			--j;
		}
		if (i < j) 
		{
			int temp = m_tree[i];
			m_tree[i] = m_tree[j];
			m_tree[j] = temp;
		}
		if ( m_pointArray[3*m_tree[i] + pos] > pivotValue)
		{
			int temp = m_tree[i];
			m_tree[i] = m_tree[right];
			m_tree[right] = temp;
		}
	}
	return i;
}

void *KdTree::buildTree(int left, int right, int depth)
{
	if (left > right) return NULL;
	int axis = depth % 3;
	quicksort(left, right, axis);
	int median = left + ((right - left)/2);
	
	buildTree(left, median-1, depth+1);
	buildTree(median+1, right, depth+1);
}
