//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FIndex.hh,v $
// Language:  C++
// Date:      $Date: 2004/07/20 07:27:53 $
// Author:    $Author: wiebel $
// Version:   $Revision: 1.15 $
//
//---------------------------------------------------------------------------

#ifndef __FMultiIndex_hh
#define __FMultiIndex_hh

#include <iosfwd>
#include <iostream>
#include "FException.hh"

#include <vector>

//===========================================================================

/**
 * A class defining the mathematic "datatype" of a multiindex.
 *
 * Defines an index of arbitrary size esp. important when calculating with
 * tensors of arbitrary order. A multiindex in mainly an array of indices,
 * in terms of tensor calculation, as many indices as the rank of the tensor.
 * Thus, a tensor
 * \f$ \delta_{i_0,...,i_k}\f$  can be written as \f$ \delta_{m} \f$ where
 * m is the multiindex of size k.
 */
class FMultiIndex
{
public:
  FMultiIndex( const std::vector<unsigned int> &indices )
  {
    this->indices = indices;
  }

  FMultiIndex(unsigned int i0)
  {
    indices.resize(1);
    indices[0] = i0;
  }

  FMultiIndex(unsigned int i1, unsigned int i0)
  {
    indices.resize(2);
    indices[0] = i0; indices[1] = i1;
  }

  FMultiIndex(unsigned int i2, unsigned int i1, unsigned int i0)
  {
    indices.resize(3);
    indices[0] = i0; indices[1] = i1; indices[2] = i2;
  }

  FMultiIndex(unsigned int i3, unsigned int i2, unsigned int i1, unsigned int i0)
  {
    indices.resize(3);
    indices[0] = i0; indices[1] = i1; indices[2] = i2;
    indices[3] = i3;
  }

  FMultiIndex(unsigned int i4, unsigned int i3, unsigned int i2, unsigned int i1, unsigned int i0)
  {
    indices.resize(3);
    indices[0] = i0; indices[1] = i1; indices[2] = i2;
    indices[3] = i3; indices[4] = i4;
  }

  FMultiIndex(unsigned int i5, unsigned int i4, unsigned int i3, unsigned int i2, unsigned int i1, unsigned int i0)
  {
    indices.resize(3);
    indices[0] = i0; indices[1] = i1; indices[2] = i2;
    indices[3] = i3; indices[4] = i4; indices[5] = i5;
  }

  /**
   * concatenation of two MultiIndices
   */
  FMultiIndex( const FMultiIndex &first, const FMultiIndex &second )
  {
    indices.resize( first.order() + second.order() );
    unsigned int i;
    for(i=0; i< first.order(); ++i)
    {
      indices[i] = first[i];
    }
    for(unsigned int j=0; j<second.order(); ++j)
    {
      indices[j+i] = second[j];
    }
  }

  unsigned int order() const
  {
    return indices.size();
  }

  unsigned int maxDim() const
  {
    return *std::max(indices.begin(), indices.end());
  }

  void resize(unsigned int size)
  {
    indices.resize(size);
  }

  void setValues( const std::vector<unsigned int>& indices )
  {
    this->indices = indices;
  }

  void getValues( std::vector<unsigned int>& indices ) const
  {
    indices = this->indices;
  }

  unsigned int operator[](unsigned int i) const
  {
#ifndef NODEBUG
    if( i >= indices.size() )
    {
      std::cout << "size " << order() << " i=" << i << std::endl;
      THROW_EXCEPTION( FInvalidDimensionException, " MultiIndex size smaller than index specified" );
    }
#endif
    return indices[i];
  }

  unsigned int& operator[](unsigned int i)
  {
#ifndef NODEBUG
    if( i >= indices.size() )
      THROW_EXCEPTION( FInvalidDimensionException, "MultiIndex size smaller than index specified" );
#endif
    return indices[i];
  }

  /*
   * to loop all indices, do:
   * FMultiIndex max(...);
   * FMultiIndex i;
   * do{
   *    do_something_with(i);
   * } while (i.increment( max ) );
   */
  bool increment( const FMultiIndex& max )
  {
#ifndef NODEBUG
    if(indices.size() != max.order())
      THROW_EXCEPTION( FInvalidDimensionException, "MultiIndex size must be equal" );
#endif
    int order = indices.size();
    for(int i=0; i< order; ++i)
    {
      ++indices[i];
      if( indices[i] == max[i] )
      {
        indices[i] = 0;
      }
      else
      {
        // finished, got our next valid index.
        return true;
      }
    }
    // the index now is zero in all componentes and not what we relly wanted
    return false;
  }

  bool increment( unsigned int max )
  {
    int size = indices.size();
    for(int i=0; i< size; ++i)
    {
      ++indices[i];
      if( indices[i] == max )
      {
        indices[i] = 0;
      }
      else
      {
        // finished, got our next valid index.
        return true;
      }
    }
    // the index now is zero in all componentes and not what we relly wanted
    return false;
  }

  //! same as increment(max) but skips indices listed in ignore
  bool increment( unsigned int max, const FMultiIndex& ignore )
  {
    try{
    unsigned int size = indices.size();
    for(unsigned i=0; i< size; ++i)
    {
      unsigned ign;
      for(ign=0; ign < ignore.order(); ++ign)
        if(ignore[ign] == i) break;
      if(ign < ignore.order()) continue; // this index shall be ignored

      ++indices[i];
      if( indices[i] == max )
      {
        indices[i] = 0;
      }
      else
      {
        // finished, got our next valid index.
        return true;
      }
    }

    }CATCH_N_RETHROW(FException);
    // the index now is zero in all componentes and not what we relly wanted
    return false;
  }

  FMultiIndex& contract( const FMultiIndex& rhs )
  {
#ifndef NODEBUG
    if(rhs.order() != 2)
      THROW_EXCEPTION( FInvalidDimensionException, "Need exactly two indices" );
    if(this->order() != 2)
      THROW_EXCEPTION( FInvalidDimensionException, "MultiIndex has to have at least a size of 2" );
    if(rhs[0] == rhs[1])
      THROW_EXCEPTION( FEmptyObjectException, "contraction indices match, but should not" );
    if(rhs[0] < this->order())
      THROW_EXCEPTION( FEmptyObjectException, "contraction indices larger than order" );
    if(rhs[0] < this->order())
      THROW_EXCEPTION( FEmptyObjectException, "contraction indices larger than order" );
#endif
#ifndef NODEBUG
    try{
#endif
    FMultiIndex me(*this);
    resize(this->order() -2);
    unsigned int j=0;
    for(unsigned int i=0; i< this->order(); ++i)
    {
      if( i == rhs[0] ) j++;
      if( i == rhs[1] ) j++;
      indices[i] = me.indices[j];
    }
#ifndef NODEBUG
    }CATCH_N_RETHROW(FException);
#endif
    return (*this);
  }

  bool operator==(const FMultiIndex& rhs)
  {
#ifndef NODEBUG
    if(indices.size() != rhs.order())
      THROW_EXCEPTION( FInvalidDimensionException, "MultiIndex size must be equal" );
#endif
    unsigned int size = indices.size();
    for(unsigned int i=0; i< size; ++i)
      if( indices[i] != rhs.indices[i] ) return false;
    return true;
  }

  bool operator!=(const FMultiIndex& rhs)
  {
#ifndef NODEBUG
    if(indices.size() != rhs.order())
      THROW_EXCEPTION( FInvalidDimensionException, "MultiIndex size must be equal" );
#endif
    unsigned int size = indices.size();
    for(unsigned int i=0; i< size; ++i)
      if( indices[i] != rhs.indices[i] ) return true;
    return false;
  }

  ///  Undocumented.
  ~FMultiIndex()
  {}

  friend std::ostream& operator<<(std::ostream& os, const FMultiIndex& mi);

private:
  std::vector<unsigned int> indices;
};

//===========================================================================
#endif // __FMultiIndex_hh

