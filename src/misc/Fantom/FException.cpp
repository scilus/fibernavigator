//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FException.cc,v $
// Language:  C++
// Date:      $Date: 2004/07/30 06:59:53 $
// Author:    $Author: wiebel $
// Version:   $Revision: 1.9 $
//
//---------------------------------------------------------------------------

#include "FException.h"
#include <iostream>

using namespace std;

//---------------------------------------------------------------------------

std::ostream& operator<<( std::ostream& out, const FException& e )
{
    e.print( out );

    return out;
}

//---------------------------------------------------------------------------

void FException::print( std::ostream& out ) const
{
    out << what() << std::endl;

    for( std::list<std::string>::const_iterator i = trace.begin();
     i != trace.end(); ++i )
    out << "trace: " << *i << std::endl;
}

//---------------------------------------------------------------------------

std::string FException::getMessage() const
{
  std::string out;
  out+=what();
  out+="\n\n";
  for( std::list<std::string>::const_iterator i = trace.begin();
       i != trace.end(); ++i )
    out+= "trace: "+*i+"\n";
  return out;
}

//---------------------------------------------------------------------------

void FException::addTraceMessage( const std::string& s )
{
    trace.push_back( s );
}

