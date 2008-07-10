#ifndef KDTREE_H_
#define KDTREE_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class KdTree : public wxThread{
public:
	KdTree(int, float*);
	~KdTree();

	virtual void *Entry();

	void sort(int, int);
	void buildTree(int, int, int);
	void buildTreeP(int, int, int);

	wxUint32 *m_tree;

private:
	int m_size;
	wxUint32 m_root;
	float *m_pointArray;
};

#endif /*KDTREE_H_*/
