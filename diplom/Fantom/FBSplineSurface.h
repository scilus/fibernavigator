//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FBSplineSurface.hh,v $
// Language:  C++
// Date:      $Date:  $
// Author:    $Author: oesterling $
// Version:   $Revision: $
//
//---------------------------------------------------------------------------

#ifndef FBSPLINESURFACE_HH
#define FBSPLINESURFACE_HH

#include <vector>
#include "FArray.h"


/**
@author Patrick Oesterling
*/
/*

         numDeBoorPoints1 -> t-parameter (knots1)
         0  1  2  3  4  5  .....
    n     ___________________________
    u    |  |  |  |  |  |  |  |  |  |
    m    |__________________________|
    D    |  |  |  |  |  |  |  |  |  |
    e    |__________________________|
    B    |  |  |  |  |  |  |  |  |  |
    o    |__________________________|
    o    |  |  |  |  |  |  |  |  |  |
    r    |__________________________|
    P    |  |  |  |  |  |  |  |  |  |
    2    |__________________________|
    ->   |  |  |  |  |  |  |  |  |  |
    u    |__________________________|
    -    |  |  |  |  |  |  |  |  |  |
    param|__________________________|
 (knots2)|  |  |  |  |  |  |  |  |  |
         |__________________________|


*/
class FBSplineSurface
{
  public:

    FBSplineSurface(int order1, int order2, std::vector< std::vector< double > > deBoorPoints, int numDeBoorPoints1, int numDeBoorPoints2);
    FBSplineSurface(int order1, int order2, std::vector< std::vector< double > > deBoorPoints, int numDeBoorPoints1, int numDeBoorPoints2, std::vector<double> knots1, std::vector<double> knots2);
    ~FBSplineSurface();

    FArray f(double t, double u);

    std::vector< std::vector< double > > getDeBoorPoints();
    int getNumDeBoorPoints1();
    int getNumDeBoorPoints2();
    std::vector<double> getKnots1();
    std::vector<double> getKnots2();
    int getOrder1();
    int getOrder2();
    int getNumSamplePointsT();
    int getNumSamplePointsU();

    void setDeBoorPoints(std::vector< std::vector< double > > deBoorPoints, int numDeBoorPoints1, int numDeBoorPoints2);
    void setKnots1(std::vector<double> knots);
    void setKnots2(std::vector<double> knots);
    void setOrder1(int order);
    void setOrder2(int order);

    void samplePoints(std::vector< std::vector< double > > &points, double tResolution, double uResolution);

  private:
    int order1, order2;
    int numDeBoorPoints1, numDeBoorPoints2;
    int numSamplePointsT, numSamplePointsU;
    std::vector<std::vector< double > > deBoorPoints;
    std::vector<double> knots1, knots2;

};

#endif
