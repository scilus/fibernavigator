#include "KdTree.h"

KdTree::KdTree(int size, float *pointArray)
{
	printf ("build kd tree\n");
	m_pointArray = pointArray;
	printf("sort vertices in X direction...");
	sort(size, 0);
	m_sortedXArray = m_sortedArray;
	printf("done\nsort vertices in Y direction...");
	sort(size, 1);
	m_sortedYArray = m_sortedArray;
	printf("done\nsort vertices in Z direction...");
	sort(size, 2);
	m_sortedZArray = m_sortedArray;
	printf("done\n");
}

void KdTree::sort(int size, int pos)
{
	/*
	 * initial run to fill array
	 * start with 0 as pivot value, since the dataset is centered 
	 * around the coordinate origin 
	 */
	m_sortedArray = new wxUint32[size];
	int left = 0;
	int right = size - 1;
	for (int i = 0 ; i < size ; ++i)
	{
		if ( m_pointArray[3*i + pos] <= 0.0)
		{
			m_sortedArray[left] = i;
			++left;
		}
		else
		{
			m_sortedArray[right] = i;
			--right;
		}
	}
	if (left > right) {
		--left;
		++right;
	}
	
	quickSort(0, left, pos);
	quickSort(right, size - 1, pos);
}

void KdTree::quickSort(int left, int right, int pos)
{
	if (left < right)
	{
		int div = quicksortSplit(left, right, pos);
		quickSort(left, div-1, pos);
		quickSort(div+1, right, pos);
	}
}

int KdTree::quicksortSplit(int left, int right, int pos)
{
	float pivotValue = m_pointArray[3*m_sortedArray[right] + pos];
	int i = left;
	int j = right -1;
	while (i < j)
	{
		while ( (m_pointArray[3*m_sortedArray[i] + pos] <= pivotValue) && (i < right) )
		{
			++i;
		}
		while ( (m_pointArray[3*m_sortedArray[j] + pos] > pivotValue) && (j > left) )
		{
			--j;
		}
		if (i < j) 
		{
			int temp = m_sortedArray[i];
			m_sortedArray[i] = m_sortedArray[j];
			m_sortedArray[j] = temp;
		}
		if ( m_pointArray[3*m_sortedArray[i] + pos] > pivotValue)
		{
			int temp = m_sortedArray[i];
			m_sortedArray[i] = m_sortedArray[right];
			m_sortedArray[right] = temp;
		}
	}
	return i;
}

TreeNode KdTree::buildTree(int left, int right, int depth)
{
	
}
