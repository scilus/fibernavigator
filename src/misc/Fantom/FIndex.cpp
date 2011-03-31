



//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FIndex.cc,v $
// Language:  C++
// Date:      $Date: 2003/06/25 07:54:46 $
// Author:    $Author: garth $
// Version:   $Revision: 1.6 $
//
//---------------------------------------------------------------------------

#include "FIndex.h"
#include <stdio.h>

//===========================================================================
#ifdef OUTLINE
#include "FIndex.icc"
#endif

//---------------------------------------------------------------------------

const positive FIndex::invalid = (positive) -1;

//---------------------------------------------------------------------------

FIndex::~FIndex()
{
}

//---------------------------------------------------------------------------
#if 0
const FString& FIndex::getClassName() const
{
  static const FString name( "FIndex" );
  return name;
}
#endif
//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os,const FIndex& inIndex)
{
  if( inIndex.isValid() )
      os << inIndex.getIndex();
  else
      os << "INVALID";

  return os;
}

//---------------------------------------------------------------------------

std::istream& operator>>(std::istream& is, FIndex& inIndex)
{
  std::string tmp;
  is >> tmp;

  if( tmp == "INVALID" )
    inIndex.setToInvalid();
  else
  {
      positive index;
      sscanf( tmp.c_str(), "%ud", &index );
      inIndex = index;
  }

  return is;
}
