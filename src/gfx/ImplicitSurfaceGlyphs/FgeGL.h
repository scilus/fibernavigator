///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2009-02-23 19:50:59 +0100 (Mon, 23 Feb 2009) $
         //             Author   :   $Author: cheine $
//////////              Revision :   $Revision: 9721 $

#ifndef FgeGL_hh
#define FgeGL_hh

// include this file if you want to use OpenGL instead of GL/gl.h because this could be somewhere else on different platforms

#if defined(__APPLE__)
#define GL_GLEXT_LEGACY 1
#include <stddef.h> //< definition of NULL
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/glu.h>
#endif

#include <iostream>

#ifdef USE_GLU
inline const GLubyte* getErrorString( GLenum val )
{
  return gluErrorString( val );
}
#else

inline const char* getErrorString( GLenum eval )
{
  const char *error=0;
  /*
  static const char* error =" ** error ** (no glu, thus no further strings)";
  static const char* no_err = " ** no error";
  if( eval == 0) return no_err;
  */
  switch ( eval )
  {
    case GL_NO_ERROR:
      error = "GL_NO_ERROR: No error has been recorded.";
      break;
    case GL_INVALID_ENUM:
      error = "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored, and has no other side effect thatn to set the error flag.";
      break;
    case GL_INVALID_VALUE:
      error = "GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored, and has no other side effect than to set the error flag.";
      break;
    case GL_INVALID_OPERATION:
      error = "GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored, and has no other side effect than to set the error.";
      break;
    case GL_STACK_OVERFLOW:
      error = "GL_STACK_OVERFLOW: This command would cause a stack overflow. The offending command is ignored and has noother side effects than to set the error.";
      break;
    case GL_STACK_UNDERFLOW:
      error = "GL_STACK_UNDERFLOW: This command would cause a stack underflow. The offending command is ignored and has noother side effects than to set the error.";
      break;
    case GL_OUT_OF_MEMORY:
      error = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
      break;
    default:
      error = "INTERNAL_GL_ERROR!!! Invalid/Unknown GL error";
  }
  return error;
}
#endif

/**
 * Helper macro to dump an openGL error to the console if in debug mode.
 */
#define DUMP_GL_ERROR( str ) \
{\
    GLenum err = glGetError(); \
        if(err != GL_NO_ERROR ) \
              std::cerr << "OpenGL error: [" << str << "]: " << getErrorString( err ) << std::endl; \
}

#ifndef NODEBUG
#define DUMP_GL_ERROR_DEBUG( str ) DUMP_GL_ERROR( str )
#else
#define DUMP_GL_ERROR_DEBUG( str )
#endif

struct FgeGLErrorHelper
{
  FgeGLErrorHelper( const char* err )
    :err( err )
  {
    DUMP_GL_ERROR( err );
  }
  ~FgeGLErrorHelper()
  {
    DUMP_GL_ERROR( "after " + err );
  }
  const char* err;
};

#define GL_ERROR \
  FgeGLErrorHelper XXXXXXXX22071980( __F_EXCEPTION_FUNC );

inline bool isGLError( const char* where )
{
  GLenum err = glGetError();
  if(err != GL_NO_ERROR )
  {
    std::cerr << "OpenGL error: [" << where << "]: " << getErrorString( err ) << std::endl;
    return true;
  }
  return false;
}


#endif // FgeGL_hh
