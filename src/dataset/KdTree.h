#ifndef KDTREE_H_
#define KDTREE_H_

#include "wx/wxprec.h"
#include "DatasetHelper.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <algorithm>

struct lessy
{
  float const * const  data;
  const int pos;
  lessy( float const * const  data, const int pos ): data( data ),pos( pos )
  { }

  bool operator()( const wxUint32& a, const wxUint32&b ) const
  {
    return data[ 3*a+pos ] < data[ 3*b+pos ];
  }
};

class KdTreeThread : public wxThread {

public:
	KdTreeThread(float*, std::vector<wxUint32>*, int, int, int, DatasetHelper* );

	void buildTree(int, int, int);
	virtual void *Entry();

	std::vector<wxUint32>* m_tree;
	float *m_pointArray;
	DatasetHelper* m_dh;
	int m_left;
	int m_right;
	int m_axis;
};

class KdTree {
public:
	KdTree(int, float*, DatasetHelper*);
	KdTree(int, float*);
	~KdTree();

	std::vector<wxUint32>m_tree;
	DatasetHelper* m_dh;

private:
	void buildTree(int, int, int);
	int m_size;
	wxUint32 m_root;
	float *m_pointArray;
};

#endif /*KDTREE_H_*/
