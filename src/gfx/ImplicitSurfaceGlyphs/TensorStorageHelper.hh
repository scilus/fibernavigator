// This file belongs to an implementation originally
// done by Mario Hlawitschka and Younis Hijazi 
// The implementation was adapted for the FiberNavigator
// Alexander Wiebel in the beginninng of 2009.


#include "FNumberingSystem.h"
#include "../../misc/Fantom/FArray.h"
#include "../../misc/Fantom/FTensor.h"
typedef FArray FVector;
struct FTensorStorageHelper
{
  FTensorStorageHelper(int dim, int order)
	  : dim(dim), order(order)
  {
	  initialize();
  }

  const int dim;
  const int order;

  void initialize()
  {
      unsigned int s = pow((float)dim,order);
      std::cout << "FTensorStorageHelper::initialize()" << std::endl;
      {
	      // create the index map
	      umap.clear();
	      iumap.resize(s);
	      unique_count n(dim, order);
	      unsigned int nbLoops = n.nbUnique();
	      for(unsigned int i=0; i<nbLoops; ++i, ++n)
	      {
//#ifdef VERBOSE
	//	std::cout << i << " " << n << "  " << n() <<std::endl;
//#endif
		umap.push_back(n());
		iumap[n()] = i;
	      }
      }
/*
      revmap.resize(umap.size());
      for(int i=0; i< umap.size(); ++i)
      {
	      revmap[umap[i]] = i;
      }
     */

      {
	      index_count n(dim, order);
	      map.resize(s);
	      for(unsigned int i=0; i< s; ++n, ++i)
	      {
		      map[i] = iumap[n.unique()];
		      std::cout << " mapping " << i << " to " << map[i] << std::endl;
	      }
      }
      std::cout << "end FTensorStorageHelper::initialize()" << std::endl;
  }

  std::vector<int> iumap;
  std::vector<int> map;
  std::vector<unsigned int> umap;
  //std::vector<int> revmap;

  unsigned int uniquify(unsigned int unique) const
  {
    for(unsigned int j=0; j< umap.size(); ++j)
    {
      if(umap[j] == unique) return j;
    }
    std::cerr << "data: " << unique << std::endl;
    assert(false);
    //return revmap[unique];
  }

  FTensor assignSymmetric(const FVector &v) const
  {
      //std::cout << "assignSymmetric" << std::endl;
    //vector<double> data(pow(dim, order));
    FTensor data(dim,order);

    //index_count n(dim,order);
    for(unsigned int i=0; i< data.size(); ++i /*,++n*/)
    {
      //unsigned unique = n.unique();
      //std::cout << i << " maps to " << unique << std::endl;
      //unique = uniquify(unique);
      //data.getComp(i) = v[unique];
      data.getComp(i) = v[map[i]];
    }

    //FTensor dummy(dim, order, data);
    //  cout << "assign" << dim << " " << order << " " << dummy.size() << endl;
      //std::cout << "end assignSymmetric" << std::endl;
    //return dummy;
    return data;
  }

  int getNbSymmetricComponents() const
  {
    return unique_count::nbUnique(dim, order);
  }

  FVector getSymmetric(const FTensor&t) const
  {
    unique_count n(dim, order);
    FVector v( n.nbUnique() );
    //std::vector<double> data(t.size());
    FVector d(t);
    for(unsigned int i=0; i< v.size(); ++i, ++n)
    {
       v(i) = d(n());
    }
    return v;
  }
};
