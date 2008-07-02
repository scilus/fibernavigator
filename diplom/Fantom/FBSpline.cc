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

#include "FBSpline.hh"

#include <iostream>

using namespace std;

FBSpline::FBSpline(int order, std::vector< std::vector< double > > deBoorPoints)
{
  this->deBoorPoints = deBoorPoints;
  int n = this->deBoorPoints.size();
  int k = this->order = order;
  
  //define a normalized knotVector
  for( int i = 0; i < (n + k); i++)
  {
    double tempKnot;
    if( i < k)
      tempKnot = k - 1;
    if( (i >= k) && (i < n))
      tempKnot = i;
    if( i >= n)
      tempKnot = n;
      
    knots.push_back(tempKnot);
  }
  
}

FBSpline::FBSpline(int order, std::vector< std::vector< double > > deBoorPoints, std::vector<double> knots)
{
  this->order = order;
  this->deBoorPoints = deBoorPoints;
  this->knots = knots;
}

FBSpline::~FBSpline()
{
}

FArray FBSpline::f(double _t)
{
  FArray result;
  unsigned int r = 0, i;
  
  if( _t < knots[0])
    _t = knots[0];
  
  if( _t > knots[ knots.size() - 1])
    _t = knots[ knots.size() - 1];
  
  t = _t; // set current paramter _t as class variable

  for(i = 0; i < knots.size(); i++) // -1 ?
    if(knots[i] > _t)
      break;
      
  r = i - 1;

  if( _t == knots[ knots.size() - 1])
  {
    for(i = (knots.size() - 1); i > 0; i--)
      if(knots[i] < _t)
          break;
    r = i;
  }

  result = controlPoint_i_j(r, order - 1); 

  return result;
}

std::vector< std::vector< double > > FBSpline::getDeBoorPoints()
{
  return deBoorPoints;
}

std::vector<double> FBSpline::getKnots()
{
  return this->knots;
}

int FBSpline::getOrder()
{
  return this->order;
}

void FBSpline::setDeBoorPoints(std::vector< std::vector< double > > deBoorPoints)
{
  this->deBoorPoints = deBoorPoints;
}

void FBSpline::setKnots(std::vector<double> knots)
{
  this->knots = knots;
}

void FBSpline::setOrder(int order)
{
  this->order = order;
}
    
void FBSpline::samplePoints(std::vector< std::vector< double > > &points, double resolution)
{
  double deltaT = resolution;
  double currentT = knots[0];
  
  int steps = (int)((knots[knots.size() - 1] - knots[0]) / deltaT + 1);
  
  for( int step = 0; step < steps; step++)
  {
    std::vector< double > dmy;
    currentT = knots[0] + step * deltaT;
    cout << currentT << endl;
    FArray samplePoint = f(currentT);
    samplePoint.getCoordinates(dmy);
    points.push_back(dmy);
  }
}

double FBSpline::getAlpha_i_j(int _i, int _j)
{
   double result = (t - knots[_i]) / (knots[_i + order - _j] - knots[_i]);
   return result;
}

FArray FBSpline::controlPoint_i_j(int _i, int _j)
{
   FArray result(3);

   if( _j == 0)
   {
      result[0] = deBoorPoints[_i][0];
      result[1] = deBoorPoints[_i][1];
      result[2] = deBoorPoints[_i][2];
      return result;
   }
   double bufferedAlpha = getAlpha_i_j(_i, _j);
  
   result = (1 - bufferedAlpha) * controlPoint_i_j(_i - 1, _j - 1) +
             bufferedAlpha * controlPoint_i_j(_i, _j - 1);


   return result;
}
