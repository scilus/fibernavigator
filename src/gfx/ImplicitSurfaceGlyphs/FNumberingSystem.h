// This file belongs to an implementation originally
// done by Mario Hlawitschka and Younis Hijazi 
// The implementation was adapted for the FiberNavigator
// Alexander Wiebel in the beginninng of 2009.

#ifndef FNumberingSystems_hh
#define FNumberingSystems_hh

#include <iostream>
#include <vector>
#include <cassert>
namespace{
  inline unsigned int factorial(unsigned int l)
  {
    if(l==0) return 1;
    return factorial(l-1)*l;
  }

  inline unsigned int binomial(unsigned int a, unsigned int b)
  {
	  //std::cout << a << " " << b << std::endl;
    return factorial(a)/(factorial(b)*factorial(a-b));
  }

  inline unsigned int multiplicity(unsigned int l, unsigned int x, unsigned int y, unsigned int z)
  {
    //  return binomial(l,x)*binomial(l-x,y);
    return factorial(l)/factorial(x)/factorial(y)/factorial(z);
  }
}


  //---------------------------------------------------------------------------
  /*
     struct mutliple
     {
     positive x, y, z;
     bool operator ==( const multiple& rhs )
     {
     return (x==rhs.x && y == rhs.y && z==rhs.z)
     }
     }
     */

/*   int pow(int base, int exp) */
/*   { */
/*     unsigned int ret = 1; */
/*     while(exp > 0) */
/*     { */
/*       ret *= base; */
/*       exp--; */
/*     } */
/*     return ret; */
/*   } */

/*   unsigned int distinct(unsigned int order) */
/*   { */
/*     return (order+1)*(order+2)/2; */
/*   } */


  /**
   * struct counting numbers of length size in base n, e.g.,
   * index_count(3,2) counts numbers in base 3 of length 2:
   * 00, 01, 02, 10, 11, 12, 20, 21, 22
  */
  struct index_count{

    index_count(int base, int size) : base(base), size(size)
    {
      values = new unsigned int[size];
      for(int i=0;i<size; ++i)
        values[i] = 0;
    }

    ~index_count()
    {
      delete[] values;
    }

    index_count &operator++()
    {
      unsigned int digit = 0;
      while(digit < size)
      {
        values[digit]++;
        if(values[digit] < (unsigned int)base)
        {
          for(int j=digit; j>0; --j)
          {
            values[j-1] = 0;
          }
          return *this;
        }
        digit++;
      }
      //                        assert(false);
      return *this;
    }

    //! convert to a "real" integer number
    unsigned int operator()()
    {
      unsigned int ret=0;
      for(unsigned int i=size; i>0; --i)
      {
        ret *= base;
        ret += values[i-1];
      }
      return ret;
    }

    //! how often do we have which number here?
    std::vector<unsigned int> histogram() const
    {
      //map<unsigned, unsigned> mymap;
      std::vector<unsigned int> mymap(base);
      for(unsigned int i=0; i<size; ++i)
        mymap[values[i]]++;

      return mymap;
    }


    //! give the index of the smallest number
    //! we can create using the given digits
    unsigned int unique() const
    {
      //map<unsigned, unsigned> mymap;
      //mymap = histogram();
      std::vector<unsigned int> mymap = histogram();
      std::vector<unsigned> values;
      for(int b =0; b<base; ++b)
      {
        for( unsigned int c=0; c<mymap[b]; ++c)
          values.push_back(b);
      }
      assert(values.size() == size);
      // now values contains zeros then ones etc.
      unsigned int ret=0;
      for( unsigned int i=0;i<size; ++i)
      {
        ret *= base;
        ret += values[i];
      }
      return ret;
    }

    //! return the i-th digit (least significant digits first)
    unsigned int operator[](unsigned int i) const
    {
      return values[i];
    }

    unsigned int *values;
    int base;
    unsigned int size;

    friend std::ostream& operator<<(std::ostream& o, const index_count&n);
  };




/**
 * struct that enumerates different permutations of a given value
 */
  struct unique_count{
    unique_count(int base, int size) : base(base), size(size)
    {
      values = new unsigned int[size];
      for(int i=0;i<size; ++i)
        values[i] = 0;

      initialize();
    }
    ~unique_count()
    {
      delete[] values;
    }


    void initialize()
    {
	    /*
      // create the index map
      umap.clear();
      unique_count n(base, size);
      for(unsigned int i=0; i<this->uniqueComp; ++i, ++n)
      {
#ifdef VERBOSE
        cout << i << " " << n << "  " << n() <<endl;
#endif
        umap.push_back(n());
      }
      */
    }


    int nbUnique() const
    {
      // Ziehen mit Zuruecklegen ohne Beachtung der Reihenfolge (n+k-1)!/(k! (n-1)!)
      // bei n Zuegen und k Elementen => laenge n zur Basis k
	    int ret = binomial(size+base-1, size); //binomial(n+k-1, k);
	    //std::cout << "nbUnique " << size <<" "<< base << " -> " << ret <<  std::endl;
	    return ret;
      //
      //if(base ==3 && (size &0x01)==0 )
	//      return (size+1)*(size+2)/2;
      //assert(false);
    }

    static int nbUnique(int base, int size)
    {
	    //std::cout << "static nbUnique " << size <<" "<< base << std::endl;
      return binomial(size+base-1, size); //binomial(n+k-1, k);
    }

    unique_count &operator++()
    {
      int digit = 0;
      while(digit < size)
      {
        values[digit]++;
        if(values[digit] < (unsigned int)base)
        {
          for(int j=digit; j>0; --j)
          {
            values[j-1] = values[digit];
          }
          return *this;
        }
        digit++;
      }
      //                        assert(false);
      return *this;
    }



    unsigned int operator()()
    {
      unsigned int ret=0;
      for(unsigned int i=size; i>0; --i)
      {
        ret *= base;
        ret += values[i-1];
      }
      return ret;
    }

    std::vector<int> histogram()
    {
      std::vector<int> mymap(base);
      for(int i=0; i<size; ++i)
        mymap[values[i]]++;

      return mymap;
    }

    unsigned int operator[](unsigned int i)
    {
      return values[i];
    }

    unsigned int *values;
    int base;
    int size;

    friend std::ostream& operator<<(std::ostream& o, const unique_count&n);
  };


/* std::ostream& operator<<(std::ostream& o, const index_count& n) */
/*   { */
/*     o << "index: " << n.size << " "; */
/*     for(int i=0; i<n.size; ++i) */
/*     { */
/*       o << n.values[i]; */
/*     } */
/*     return o; */
/*   } */

/* std::ostream& operator<<(std::ostream& o, const unique_count& n) */
/*   { */
/*     o << "unique: " << n.size << " "; */
/*     for(int i=0; i<n.size; ++i) */
/*     { */
/*       o << n.values[i]; */
/*     } */
/*     return o; */
/*   } */


#ifdef INVALIDATE_NEGATIVE
  void invalidate( FVector & v )
  {
    //      invalidated++;
    for(unsigned int i=0; i< v.size() ; ++i)
    {
      v(i) = 0;
    }
  }
#endif
#endif

