#ifndef KDTREE_H_
#define KDTREE_H_

#include "wx/wxprec.h"
#include "DatasetHelper.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

struct iter
{
  typedef std::random_access_iterator_tag iterator_category;
  typedef wxUint32 value_type;
  typedef int difference_type;
  typedef wxUint32* pointer;
  typedef wxUint32& reference;

  wxUint32 *data;
  size_t pos;

  iter( const iter &rhs ) : data( rhs.data ), pos( rhs.pos ){}

  iter( wxUint32*data, size_t pos ):
    data( data ), pos( pos ) {}


  iter operator+( size_t i ) const { return iter( data, pos+i );}
  iter operator-( size_t i ) const { return iter( data, pos-i );}
  int operator-( iter &rhs ) const { return pos - rhs.pos;}

  wxUint32& operator*(){ return data[ pos ];}
  const wxUint32& operator*() const { return data[ pos ];}

  iter& operator +=( size_t i ){ pos += i; return *this;}
  iter& operator++(){ ++pos; return *this;}
  iter& operator--(){ --pos; return *this;}

  bool operator==( const iter&rhs ) const { return pos == rhs.pos; }
  bool operator!=( const iter&rhs ) const { return pos != rhs.pos; }
  bool operator<=( const iter&rhs ) const { return pos <= rhs.pos; }
  bool operator< ( const iter&rhs ) const { return pos <  rhs.pos; }
};

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
	KdTreeThread(float*, wxUint32*, int, int, int, DatasetHelper* );

	void buildTree(int, int, int);
	virtual void *Entry();

	wxUint32 *m_tree;
	float *m_pointArray;
	DatasetHelper* m_dh;
	int m_left;
	int m_right;
	int m_axis;
};

class KdTree {
public:
	KdTree(int, float*, DatasetHelper*);
	~KdTree();

	wxUint32 *m_tree;
	DatasetHelper* m_dh;

private:
	int m_size;
	wxUint32 m_root;
	float *m_pointArray;
};

#define KDTREE_EVENT 50

#endif /*KDTREE_H_*/
