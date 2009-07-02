///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2009-02-23 19:50:59 +0100 (Mon, 23 Feb 2009) $
         //             Author   :   $Author: cheine $
//////////              Revision :   $Revision: 9721 $

#ifndef FgeGLExtensions_hh
#define FgeGLExtensions_hh
#include <iostream>

#include "FgeGL.h"

#define GL_GLEXT_PROTOTYPES
//#include "ext/GL/glext.h"

/**
 * Namespace encapsulating OpenGL related functions
 *
 *
 * When checking for extensions, checking for a valid function pointer is
 * not enough, as the extension might be compiled but may not ba available
 * on the current graphics card or on the current network connection e.g.
 * to the X server.
 */
namespace FgeOpenGL
{

  void* getGLFuncPtr( const char *name, const char *altname );
  void* getGLFuncPtrWarn( const char *name, const char *altname );

    /**
     * Lookup OpenGL function pointer.
     *
     * @param FuncPtr_T type of function pointer to avoid cast.
     * @param fn the pointer to the function
     * @param name name of function
     * @param altname alternative name of function
     * @return true, if function pointer could be set
     */
    template< typename FuncPtr_T >
    bool lookupGLFuncPtr( FuncPtr_T &fn, const char *name, const char *altname = 0 )
    {
        return fn = reinterpret_cast< FuncPtr_T >( getGLFuncPtr( name, altname ) );
    }

    /**
     * Lookup OpenGL function pointer and issue a warning if pointer could not be retrieved.
     *
     * @param FuncPtr_T type of function pointer to avoid cast.
     * @param fn the pointer to the function
     * @param name name of function
     * @param altname alternative name of function
     * @return true, if function pointer could be set
     */
    template< typename FuncPtr_T >
    bool lookupGLFuncPtrWarn( FuncPtr_T &fn, const char *name, const char *altname = 0 )
    {
        return ( fn = reinterpret_cast< FuncPtr_T >( getGLFuncPtrWarn( name, altname ) ) );
    }

  /// Print the glGetString( GL_EXTENSIONS ) string for debugging purpose
  void printGLExtensions(std::ostream &o );


  /// must only be called in a valid context
  /// contextID is the current context number
  /// variables are buffered, so reusing this function relatively fast
  bool isGLExtensionSupported( unsigned int contextID, const char*extension);
  bool isGLUExtensionSupported(unsigned int contextID, const char* extension);
  std::string& getGLExtensionDisableString();
}
#endif

