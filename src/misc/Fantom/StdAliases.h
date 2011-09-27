//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: stdAliases.hh,v $
// Language:  C++
// Date:      $Date: 2004/07/13 18:37:12 $
// Author:    $Author: wiebel $
// Version:   $Revision: 1.8 $
//
//--------------------------------------------------------------------------- 

#ifndef stdAliases_hh
#define stdAliases_hh

/**
 * A bunch of defines. 
 */


#include <limits.h>

//! Define <unsigned int> as <positive>.
typedef unsigned int positive;
//#define positive unsigned int
/// Define POS_MAX as UINT_MAX.
//const unsigned int POS_MAX = UINT_MAX;
//commented out line above because it was not used
//rethink whether the values are correct when uncommenting

/// Define POS_MIN as 0
//const unsigned int POS_MIN = 0;
//commented out line above because it was not used
//rethink whether the values are correct when uncommenting

//! Define <unsigned long int> as <lpositive>
typedef unsigned long int lpositive;

/// Define LPOS_MAX as ULONG_MAX.
//const unsigned long int LPOS_MAX =  ULONG_MAX;
//commented out line above because it was not used
//rethink whether the values are correct when uncommenting

/// Define LPOS_MIN as 0.
//const unsigned long int LPOS_MIN = 0;
//commented out line above because it was not used
//rethink whether the values are correct when uncommenting

//! Maximum double value to be used within FAnToM
const double MAX_FANTOM_DOUBLE = 1.0E200;

//! Minimum double value to be used within FAnToM
const double MIN_FANTOM_DOUBLE = -1.0E200;

#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2)
# define FDEPRECATED \
        __attribute__ ((deprecated)) 
#else
# define FDEPRECATED
#endif

#endif // __STD_ALIASES_
