#include "KdTree.h"
#include "theDataset.h"

#include <algorithm>

KdTree::KdTree(int size, float *pointArray)
{
	TheDataset::printTime();
	printf ("build kd tree...\n");
	m_pointArray = pointArray;
	m_tree = new wxUint32[size];
	for (int i = 0 ; i < size ;  ++i)
		m_tree[i] = i;
	buildTree(0,size-1, 0);
	TheDataset::printTime();
	printf ("tree finished\n");
}

KdTree::~KdTree()
{
	delete[] m_tree;
}


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

void KdTree::buildTree(int left, int right, int axis)
{
    if (left >= right) return;

    int div = ( left+right )/2;
    iter begin( m_tree, left );
    iter end( m_tree, right );
    iter nth( m_tree, div );
    std::nth_element( begin, nth, end, lessy( m_pointArray, axis ) );

    int median = div;
    buildTree(left, median-1, (axis+1)%3);
    buildTree(median+1, right, (axis+1)%3);
}
