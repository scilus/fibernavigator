//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FException.hh,v $
// Language:  C++
// Date:      $Date: 2004/07/30 06:59:53 $
// Author:    $Author: wiebel $
// Version:   $Revision: 1.15 $
//
//--------------------------------------------------------------------------- 

#ifndef __FException_hh
#define __FException_hh

#include <list>
#include <iosfwd>
#include <sstream>
#include <stdexcept>
#include <new>
// --------------------------------------------------------------------------

class FException: public std::runtime_error
          
{
public:
    std::list<std::string> trace;

    FException( const std::string &s = std::string() ) throw() : std::runtime_error(s)
    {
    }

    virtual ~FException() throw() {}

    virtual void print( std::ostream& out ) const;
    std::string getMessage( ) const;

    void addTraceMessage( const std::string& s );
};


std::ostream& operator<<( std::ostream& out, const FException& e );

// --------------------------------------------------------------------------

/* NOTE: the following section is copy/n/pasted from assert.h

   Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */

#if defined ( WIN32 )
#  ifdef _MSC_VER
#    define __F_EXCEPTION_FUNC        "MSVC_NO_FUNCTION_NAMES"
#  else
#   define __F_EXCEPTION_FUNC    __PRETTY_FUNCTION__
#  endif

#elif defined (__APPLE__ )

#   define __F_EXCEPTION_FUNC    __PRETTY_FUNCTION__

#else // linux
# if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   define __F_EXCEPTION_FUNC    __PRETTY_FUNCTION__
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __F_EXCEPTION_FUNC    __func__
#  else
#   define __F_EXCEPTION_FUNC    ((__const char *) 0)
#  endif
# endif
#endif



#define RETHROW_EXCEPTION(e)                                \
{                                       \
  std::ostringstream f__oss;                                                \
  f__oss << __LINE__;                                                       \
  std::string f__except_str;                                                \
  f__except_str = "function: ";                                             \
  f__except_str += __F_EXCEPTION_FUNC;                                      \
  f__except_str += " at line  ";                                            \
  f__except_str += f__oss.str();                                             \
  f__except_str += " in file ";                                             \
  f__except_str += __FILE__;                                                \
  e.addTraceMessage( f__except_str );                                       \
  throw e;                                                                 \
}                                                                       \

#define THROW_DEFAULT_EXCEPTION(type)                                      \
{                                                                          \
  type   f__except;                                                         \
  RETHROW_EXCEPTION( f__except );                                           \
}                                                                          \


// same as above but alters message string
#define THROW_EXCEPTION(type,msg)                                          \
{                                                                          \
  type   f__except( msg );                                                  \
  RETHROW_EXCEPTION( f__except );                                           \
}                                                                          \

#define THROW( e )\
{                                                                           \
  RETHROW_EXCEPTION( e ); \
}\

#define CATCH_N_RETHROW(type)                                              \
catch ( type &e ){                                                         \
 RETHROW_EXCEPTION(e);                                                     \
}                                                                          \

                                                                                                    
// --------------------------------------------------------------------------

#define DEFINE_DEFAULTMSG_EXCEPTION_TYPE(type,msg)                         \
class type: public FException                                              \
{                                                                          \
public:                                                                    \
    type( const std::string& s = msg ) throw() : FException(s) {}          \
};                                                                         \

/**
 * Thrown by FNewHandler::new_handler() if memory allocation fails.
 *
 * This exception has to be derived from std::bad_alloc, otherwise
 * the application would be killed, because new_handler seems to
 * be only allowed to throw exceptions derived from std::bad_alloc
 */
class FOutOfMemoryException : public FException, public std::bad_alloc
{
  public:
    FOutOfMemoryException( const std::string & s = "out of memory" ) throw()
      : FException(s), std::bad_alloc()
      {
      }
};
// ---- a list of FAnToM exceptions follows ---------------------------------

DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FClosedOrbitException,      "closed orbit reached" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FDistTooSmallException,     "iterated distance too small" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FEmptyObjectException,      "empty object" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FErrorTooLargeException,    "approximation error too large" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FIOException,               "input/output error"  )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FIndexOutOfBoundsException, "index out of bounds" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FIntersectionPointFoundException, "itersection point found" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FInvalidCellException,      "invalid cell" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FInvalidDimensionException, "invalid dimension" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FInvalidOrderException,     "invalid order" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FInvalidFileException,      "invalid file" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FInvalidFilenameException,  "invalid filename" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FInvalidPositionException,  "invalid position" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FMatrixSingularException,   "matrix singular" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FNotImplementedException,   "something not implemented (yet?)" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FNullPointerAssignmentException, "null pointer assignment" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FObjectNotEmptyException,   "object not empty" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FSingularityException,      "singularity reached" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FStepSizeTooSmallException, "step size too small" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FSyntaxErrorException,      "syntax error" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FTooManyIterationsException, "too many iterations" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FPositionOutsideGridException, "position outside grid" )

DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FInvalidUserException,   "invalid user" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FUserTooStupidException, "user too stupid" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FProblemBetweenKeyboardAndChairException, "unresolved problem between keyboard and chair" )

DEFINE_DEFAULTMSG_EXCEPTION_TYPE( FPythonException,        "Error while executing python script" )
DEFINE_DEFAULTMSG_EXCEPTION_TYPE( parse_failure,           "Parse failure." )
#endif // __FException_hh
 
