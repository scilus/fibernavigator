//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FArray.cc,v $
// Language:  C++
// Date:      $Date: 2003/07/02 08:07:12 $
// Author:    $Author: garth $
// Version:   $Revision: 1.23 $
//
//---------------------------------------------------------------------------
#include <ctype.h>
#include <iomanip>

using namespace std;

#include "FArray.h"
#include "FString.h"
#include "FException.h"
#include "FMatrix.h"

#include "binio.h"

#ifdef OUTLINE
#include "FArray.icc"
#endif

//--------------------------------------------------------------------

FArray::FArray()
{
  // for test purpose...
  // consistent with the default constructor of the former FVector class

//   comp=new double[3]; sizeOfArray=3;
//   memset(comp, 0, 3*sizeof(double));

  comp = 0;
  sizeOfArray = 0;
}

//---------------------------------------------------------------------

FArray::FArray(unsigned int s)
{
  comp=new double[s];
  sizeOfArray=s;
  memset(comp, 0, s*sizeof(double));
}

//---------------------------------------------------------------------

FArray::FArray(const FArray& a)
{
  sizeOfArray=a.sizeOfArray;
  comp=new double[sizeOfArray];
  memcpy(comp, a.comp, sizeOfArray*sizeof(double));
}

//---------------------------------------------------------------------

FArray::FArray(const std::vector<double>& a)
{
  sizeOfArray=a.size();
  comp=new double[sizeOfArray];

  // copy the elements
  if( sizeOfArray > 0 ) 
      memcpy( comp, &a[0], sizeof(double) * a.size() );
}

//---------------------------------------------------------------------

FArray::FArray(const std::vector<float>& a)
{
  std::vector<double> temp(a.begin(), a.end());

  sizeOfArray=temp.size();
  comp=new double[sizeOfArray];

  // copy the elements
  if( sizeOfArray > 0 )    
    memcpy( comp, &temp[0], sizeof(double) * temp.size() );
}

//---------------------------------------------------------------------

FArray::FArray(unsigned int s, const double ref[])
{
  comp= new double[s];
  sizeOfArray = s;
  memcpy(comp, ref, s*sizeof(double));
}

//---------------------------------------------------------------------

FArray::FArray(double x, double y, double z, double w, double w2, double w3)
{
  sizeOfArray=6;
  comp=new double[6];
  comp[0] = x;
  comp[1] = y;
  comp[2] = z;
  comp[3] = w;
  comp[4] = w2;
  comp[5] = w3;
}

//---------------------------------------------------------------------

FArray::FArray(double x, double y, double z, double w, double w2)
{
  sizeOfArray=5;
  comp=new double[5];
  comp[0] = x;
  comp[1] = y;
  comp[2] = z;
  comp[3] = w;
  comp[4] = w2;
}

//---------------------------------------------------------------------

FArray::FArray(double x, double y, double z, double w)
{
  sizeOfArray=4;
  comp=new double[4];
  comp[0] = x;
  comp[1] = y;
  comp[2] = z;
  comp[3] = w;
}

//---------------------------------------------------------------------

FArray::FArray(double x, double y, double z)
{
  sizeOfArray=3;
  comp=new double[3];
  comp[0] = x;
  comp[1] = y;
  comp[2] = z;
}

//---------------------------------------------------------------------------

FArray::FArray(double x, double y)
{
  sizeOfArray=2;
  comp=new double[2];
  comp[0] = x;
  comp[1] = y;
}

//---------------------------------------------------------------------------

FArray::FArray(const FMatrix& m)
{
  sizeOfArray=m.getDimensionX()*m.getDimensionY();
  comp=new double[sizeOfArray];
  for(unsigned int s=0; s<sizeOfArray; s++)
    comp[s] = m[s];
}

//---------------------------------------------------------------------

FArray::FArray(unsigned int /*c*/, double /*v*/)
{
  THROW_EXCEPTION(FException, "Ambigous! Please take a look at FArray.hh!");
}

//---------------------------------------------------------------------

FArray::~FArray()
{
  if (comp) delete[] comp;
}

//---------------------------------------------------------------------------

void FArray::resize(unsigned int s, bool keepContent)
{
  // nothing to be done;
  if (sizeOfArray == s)
    return;

  if (s==0)
    {
      delete[] comp;
      sizeOfArray=0;
      comp=0;
      return;
    }

  if (comp) {

    if (keepContent) { //(keepContent)
      double* dmy = new double[s];
      if (sizeOfArray > s) sizeOfArray = s;
      memcpy(dmy, comp, sizeOfArray * sizeof(double));
      if ( s > sizeOfArray )
        memset( dmy+sizeOfArray, ( s-sizeOfArray ) * sizeof( double ), 0 );
      sizeOfArray = s;
      delete [] comp;
      comp = dmy;
    }
    else {
      delete[] comp;
      comp = new double[s];
    }
    sizeOfArray = s;
  }
  else {
    comp = new double [s];
    memset(comp, s * sizeof(double), 0);
    sizeOfArray = s;
  }
}

//---------------------------------------------------------------------------
ostream& operator<< ( ostream& os, const FArray& a )
{
  unsigned int i;
  os << "[ ";
  for(i=0; i<a.size(); i++)
    {
        os << setw(16) << setprecision(16) << a.comp[i];
        if (i<a.size()-1)
        os << ",";
    }
  os << " ]";
  return os;
}

//---------------------------------------------------------------------------

istream& operator>> ( istream& is , FArray& a)
{
  char c=' ';
  //  unsigned int length, i;
  // jump the first character
  while (isspace(c) && (is >> c));

  if (c != '[')
    { is.putback(c);
    THROW_EXCEPTION( FSyntaxErrorException, "no definition of an array found"); }

  if (!is)
    THROW_EXCEPTION( FSyntaxErrorException, "reached end of file while while looking for array definition");

  vector<double> tmp;
  double tmpdouble;

  while ( is >> tmpdouble >> c )
    {
      tmp.push_back ( tmpdouble );

      // finished !
      if (c==']') break;

      // if operation did not succeed
      if (c!=',')
        {
          is.clear(ios::badbit);
          break;
        }
    }

  if (!is) THROW_EXCEPTION( FSyntaxErrorException, "end of file encountered before end of array !");

  a=tmp;

  return is;
}

//---------------------------------------------------------------------------

ostream& binwrite( ostream & os, const FArray & a )
{
    unsigned int size = a.size();

    binwrite_raw( os, &size );

    if( size )
    binwrite_raw( os, a.comp, size );

    return os;
}

//---------------------------------------------------------------------------

istream& binread( istream & is, FArray& a )
{
    unsigned int size;

    binread_raw( is, &size );

    a.resize( size );
    binread_raw( is, a.comp, size );

    return is;
}

//=====================================================================
