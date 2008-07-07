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

#include "FBSplineSurface.hh"
#include "FBSpline.hh"

#include <iostream>

FBSplineSurface::FBSplineSurface(int order1, int order2, std::vector< std::vector< double > > deBoorPoints, int numDeBoorPoints1, int numDeBoorPoints2)
{
  this->order1 = order1;
  this->order2 = order2;
  this->deBoorPoints = deBoorPoints;
  this->numDeBoorPoints1 = numDeBoorPoints1;
  this->numDeBoorPoints2 = numDeBoorPoints2;

  //define a normalized knotVector1
  int n = this->numDeBoorPoints1;
  int k = this->order1;
  for( int i = 0; i < (n + k); i++)
  {
    int tempKnot = 0;
    if( i < k)
      tempKnot = k - 1;
    if( (i >= k) && (i < n))
      tempKnot = i;
    if( i >= n)
      tempKnot = n;

    knots1.push_back(tempKnot);
  }

  //define a normalized knotVector2
  n = this->numDeBoorPoints2;
  k = this->order2;
  for( int i = 0; i < (n + k); i++)
  {
    int tempKnot = 0;
    if( i < k)
      tempKnot = k - 1;
    if( (i >= k) && (i < n))
      tempKnot = i;
    if( i >= n)
      tempKnot = n;

    knots2.push_back(tempKnot);
  }
}

FBSplineSurface::FBSplineSurface(int order1, int order2, std::vector< std::vector< double > > deBoorPoints, int numDeBoorPoints1, int numDeBoorPoints2, std::vector<double> knots1, std::vector<double> knots2)
{
  this->order1 = order1;
  this->order2 = order2;
  this->deBoorPoints = deBoorPoints;
  this->numDeBoorPoints1 = numDeBoorPoints1;
  this->numDeBoorPoints2 = numDeBoorPoints2;
  this->knots1 = knots1;
  this->knots2 = knots2;
}

FBSplineSurface::~FBSplineSurface()
{
}

FArray FBSplineSurface::f(double _t, double _u)
{
    /*
           numDeBoorPoints1 -> t-parameter (knots1)
    n 0,0 _____________x_____________
    u    |  |  |  |  |  |  |  |  |  |
    m    |_____________x____________|
    D    |  |  |  |  |  |  |  |  |  |
    e    |_____________x____________| first all splines with t-param
    B    |  |  |  |  |  |  |  |  |  | then final spline with u-param
    o    |_____________x____________|
    o    |  |  |  |  |  |  |  |  |  |
    r    |____________[x]___________| f(t, u)
    P    |  |  |  |  |  |  |  |  |  |
    2    |_____________x____________|
    ->   |  |  |  |  |  |  |  |  |  |
    u    |_____________x____________|
    -    |  |  |  |  |  |  |  |  |  |
    param|_____________x____________|
 (knots2)|  |  |  |  |  |  |  |  |  |
         |_____________x____________|
  */

  std::vector< std::vector< double > > uSplineDeBoorPoints;

  for(int row = 0; row < numDeBoorPoints2; row++)
  {
    std::vector< std::vector< double > > tSplineDeBoorPoints;

    for(int col = 0; col < numDeBoorPoints1; col++)
      tSplineDeBoorPoints.push_back(this->deBoorPoints[row * numDeBoorPoints1 + col]);

    FBSpline tSpline(order1, tSplineDeBoorPoints);
    FArray dmyArray = tSpline.f(_t);

    std::vector< double > dmyVector;
    dmyArray.getCoordinates(dmyVector);

    uSplineDeBoorPoints.push_back(dmyVector);
  }

  FBSpline uSpline(order2, uSplineDeBoorPoints);
  return uSpline.f(_u);
}

std::vector< std::vector< double > > FBSplineSurface::getDeBoorPoints()
{
  return deBoorPoints;
}

int FBSplineSurface::getNumDeBoorPoints1()
{
  return numDeBoorPoints1;
}

int FBSplineSurface::getNumDeBoorPoints2()
{
  return numDeBoorPoints2;
}

std::vector<double> FBSplineSurface::getKnots1()
{
  return knots1;
}

std::vector<double> FBSplineSurface::getKnots2()
{
  return knots2;
}

int FBSplineSurface::getOrder1()
{
  return order1;
}

int FBSplineSurface::getOrder2()
{
  return order2;
}

int FBSplineSurface::getNumSamplePointsT()
{
  return numSamplePointsT;
}

int FBSplineSurface::getNumSamplePointsU()
{
  return numSamplePointsU;
}

void FBSplineSurface::setDeBoorPoints(std::vector< std::vector< double > > deBoorPoints, int numDeBoorPoints1, int numDeBoorPoints2)
{
  this->deBoorPoints = deBoorPoints;
  this->numDeBoorPoints1 = numDeBoorPoints1;
  this->numDeBoorPoints2 = numDeBoorPoints2;
}

void FBSplineSurface::setKnots1(std::vector<double> knots)
{
  this->knots1 = knots;
}

void FBSplineSurface::setKnots2(std::vector<double> knots)
{
  this->knots2 = knots;
}

void FBSplineSurface::setOrder1(int order)
{
  this->order1 = order;
}

void FBSplineSurface::setOrder2(int order)
{
  this->order2 = order;
}

void FBSplineSurface::samplePoints(std::vector< std::vector< double > > &points, double tResolution, double uResolution)
{
  double currentT = knots1[0];
  double currentU = knots2[0];
  double deltaT = tResolution;
  double deltaU = uResolution;


  int stepsT = numSamplePointsT = (int)((knots1[knots1.size() - 1] - knots1[0]) / deltaT + 1);
  int stepT = 0;
  int stepsU = numSamplePointsU = (int)((knots2[knots2.size() - 1] - knots2[0]) / deltaU + 1);
  int stepU = 0;

  for( stepU = 0; stepU < stepsU; stepU++)
  {
    currentU = knots2[0] + stepU * deltaU;
    for( stepT = 0; stepT < stepsT; stepT++)
    {
      std::vector< double > dmy;

      currentT = knots1[0] + stepT * deltaT;
      FArray samplePoint = this->f(currentT, currentU);
      samplePoint.getCoordinates(dmy);
      points.push_back(dmy);
    }
  }

  return;
}
