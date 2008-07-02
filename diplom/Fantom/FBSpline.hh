//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FBSpline.hh,v $
// Language:  C++
// Date:      $Date:  $
// Author:    $Author: oesterling $
// Version:   $Revision: $
//
//--------------------------------------------------------------------------- 

#ifndef FBSPLINE_HH
#define FBSPLINE_HH

#include <vector>
#include "math/src/FArray.hh"


/**
@author Patrick Oesterling
*/
class FBSpline
{
  public:
    FBSpline(int order, std::vector< std::vector< double > > deBoorPoints);
    FBSpline(int order, std::vector< std::vector< double > > deBoorPoints, std::vector<double> knots);
    ~FBSpline();
    
    FArray f(double t);
    
    std::vector< std::vector< double > > getDeBoorPoints();
    std::vector<double> getKnots();
    int getOrder();
     
    void setDeBoorPoints(std::vector< std::vector< double > > deBoorPoints);
    void setKnots(std::vector<double> knots);
    void setOrder(int order);
    
    void samplePoints(std::vector< std::vector< double > > &p, double resolution);
        
  private:
    int order;
    std::vector<std::vector< double > > deBoorPoints;
    std::vector<double> knots;
    double t;
    
    double getAlpha_i_j(int _i, int _j);
    FArray controlPoint_i_j(int _i, int _j);
};

#endif
