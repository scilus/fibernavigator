#ifndef KDTREE_H_
#define KDTREE_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class TreeNode {
	TreeNode(int index) {m_index = index ;};
	
	wxUint32 m_index;
	TreeNode *m_leftChild;
	TreeNode *m_rightChild;
};


class KdTree {
public:
	KdTree(int, float*);
	void sort(int, int);
	void quickSort(int, int, int);
	int quicksortSplit(int ,int ,int);
	TreeNode buildTree(int, int, int);
	
private:
	wxUint32 *m_sortedXArray;
	wxUint32 *m_sortedYArray;
	wxUint32 *m_sortedZArray;
	wxUint32 *m_sortedArray;
	
	float *m_pointArray;
};

#endif /*KDTREE_H_*/
