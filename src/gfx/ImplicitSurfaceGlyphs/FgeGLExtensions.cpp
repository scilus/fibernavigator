///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2009-02-13 16:31:22 +0100 (Fri, 13 Feb 2009) $
         //             Author   :   $Author: cheine $
//////////              Revision :   $Revision: 9697 $


#include "FgeGLExtensions.h"

#if defined(__APPLE__)
  #define USE_DEPRECATED_NS_API
  #ifdef USE_DEPRECATED_NS_API
    #include <mach-o/dyld.h>
  #else
    #include <dlfcn.h>
  #endif
#elif defined(WIN32)
  #include <windows.h>
#else
  #include <GL/glx.h>
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include "FgeGL.h"

namespace FgeOpenGL
{

  void printGLExtensions(std::ostream &o )
  {
    o << (char*)glGetString(GL_EXTENSIONS);
  }

  /// return address of specified OpenGL function
  void* getGLFuncPtr(const char* funcName)
  {
#if defined(__APPLE__)
  #ifdef USE_DEPRECATED_NS_API
    std::string temp( "_" );
    temp += funcName;
    if( NSIsSymbolNameDefined( temp.c_str()))
    {
      NSSymbol symbol = NSLookupAndBindSymbol( temp.c_str());
      return NSAddressOfSymbol( symbol );
    }
    else
      return NULL;
  #else
    return dlsym(0, funcName ); //< this is same as linux now
  #endif
#elif defined(WIN32)
    return (void*) wglGetProcAddress(funcName);
#else
    return (void*) glXGetProcAddress( (const GLubyte *)funcName );
#endif
  }

  void* getGLFuncPtr( const char *name, const char *altname )
  {
    void *fn = getGLFuncPtr( name );
    if( !fn && altname )
      fn = getGLFuncPtr( altname );
    return fn;
  }

  void *getGLFuncPtrWarn( const char *name, const char *altname )
  {
    void *fn = getGLFuncPtr( name, altname );
    if( !fn )
      std::cerr << "warning: could not find OpenGL function pointer " << name << std::endl;
    return fn;
  }

  // FAnToM currently only has fixed 4 viewports
#define MAX_DISPLAYS 4

  bool isGLExtensionSupported( unsigned int contextID, const char*extension)
  {
    typedef std::set<std::string>  ExtensionSet;
    //    static osg::buffered_object<ExtensionSet> s_extensionSetList;
    //    static osg::buffered_object<std::string> s_rendererList;
    //    static osg::buffered_value<int> s_initializedList;


    //ExtensionSet& extensionSet = s_extensionSetList[contextID];
    //std::string& rendererString = s_rendererList[contextID];


    // DUMMY until something like buffered object is implemented
    static std::vector<int> s_initializedList(MAX_DISPLAYS);
    static std::vector<ExtensionSet> s_extensionSetList(MAX_DISPLAYS);

    ExtensionSet &extensionSet = s_extensionSetList[contextID];
    std::string rendererString;


    // if not already set up, initialize all the per graphic context values.
    if (!s_initializedList[contextID])
    {
      s_initializedList[contextID] = 1;

      // set up the renderer
      const GLubyte* renderer = glGetString(GL_RENDERER);
      rendererString = renderer ? (const char*)renderer : "";

      // get the extension list from OpenGL.
      const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
      if (extensions==NULL) return false;

      // insert the ' ' delimiated extensions words into the extensionSet.
      const char *startOfWord = extensions;
      const char *endOfWord;
      while ((endOfWord = strchr(startOfWord,' '))!=NULL)
      {
        extensionSet.insert(std::string(startOfWord,endOfWord));
        startOfWord = endOfWord+1;
      }
      if (*startOfWord!=0) extensionSet.insert(std::string(startOfWord));

      std::cout<<"OpenGL extensions supported by installed OpenGL drivers are:"<<std::endl;
      for(ExtensionSet::iterator itr=extensionSet.begin();
          itr!=extensionSet.end();
          ++itr)
      {
        std::cout<<"    "<<*itr<<std::endl;
      }

    }
    // true if extension found in extensionSet.
    bool result = extensionSet.find(extension)!=extensionSet.end();

    // now see if extension is in the extension disabled list
    bool extensionDisabled = false;
    if (result)
    {

      const std::string& disableString = getGLExtensionDisableString();
      if (!disableString.empty())
      {

        std::string::size_type pos=0;
        while ( pos!=std::string::npos && (pos=disableString.find(extension,pos))!=std::string::npos )
        {
          std::string::size_type previousColon = disableString.find_last_of(':',pos);
          std::string::size_type previousSemiColon = disableString.find_last_of(';',pos);

          std::string renderer = "";
          if (previousColon!=std::string::npos)
          {
            if (previousSemiColon==std::string::npos) renderer = disableString.substr(0,previousColon);
            else if (previousSemiColon<previousColon) renderer = disableString.substr(previousSemiColon+1,previousColon-previousSemiColon-1);
          }

          if (!renderer.empty())
          {

            // remove leading spaces if they exist.
            std::string::size_type leadingSpaces = renderer.find_first_not_of(' ');
            if (leadingSpaces==std::string::npos) renderer = ""; // nothing but spaces
            else if (leadingSpaces!=0) renderer.erase(0,leadingSpaces);

            // remove trailing spaces if they exist.
            std::string::size_type trailingSpaces = renderer.find_last_not_of(' ');
            if (trailingSpaces!=std::string::npos) renderer.erase(trailingSpaces+1,std::string::npos);

          }

          if (renderer.empty())
          {
            extensionDisabled = true;
            break;
          }

          if (rendererString.find(renderer)!=std::string::npos)
          {
            extensionDisabled = true;
            break;

          }

          // move the position in the disable string along so that the same extension is found multiple times
          ++pos;
        }

      }
    }

    if (result)
    {
      if (!extensionDisabled) 
      {
#ifdef VERBOSE
        std::cout<<"OpenGL extension '"<<extension<<"' is supported."<<std::endl;
#endif
      }
      else std::cout<<"OpenGL extension '"<<extension<<"' is supported by OpenGL\ndriver but has been disabled by osg::getGLExtensionDisableString()."<<std::endl;
    }
    else std::cout<<"OpenGL extension '"<<extension<<"' is not supported."<<std::endl;


    return result && !extensionDisabled;
  }

  std::string& getGLExtensionDisableString()
  {
    static const char* envVar = getenv("FANTOM_GL_EXTENSION_DISABLE");
    static std::string s_GLExtensionDisableString(envVar?envVar:"Nothing defined");
    return s_GLExtensionDisableString;
  }

#ifndef USE_GLU

  bool isGLUExtensionSupported(unsigned int /*constextID*/, const char * /*extension*/)
  {
    return false;
  }
#else

  bool isGLUExtensionSupported(unsigned int contextID, const char *extension)
  {
    typedef std::set<std::string>  ExtensionSet;
    //static osg::buffered_object<ExtensionSet> s_extensionSetList;
    //static osg::buffered_object<std::string> s_rendererList;
    //static osg::buffered_value<int> s_initializedList;

    //ExtensionSet& extensionSet = s_extensionSetList[contextID];
    //std::string& rendererString = s_rendererList[contextID];

    ExtensionSet extensionSet;
    std::string rendererString;
    std::vector<int> s_initializedList(contextID);


    // if not already set up, initialize all the per graphic context values.
    if (!s_initializedList[contextID])
    {
      s_initializedList[contextID] = 1;

      // set up the renderer
      const GLubyte* renderer = glGetString(GL_RENDERER);
      rendererString = renderer ? (const char*)renderer : "";

      // get the extension list from OpenGL.
      const char* extensions = (const char*)gluGetString(GLU_EXTENSIONS);
      if (extensions==NULL) return false;

      // insert the ' ' delimiated extensions words into the extensionSet.
      const char *startOfWord = extensions;
      const char *endOfWord;
      while ((endOfWord = strchr(startOfWord,' '))!=NULL)
      {
        extensionSet.insert(std::string(startOfWord,endOfWord));
        startOfWord = endOfWord+1;
      }
      if (*startOfWord!=0) extensionSet.insert(std::string(startOfWord));

      std::cout<<"OpenGL extensions supported by installed OpenGL drivers are:"<<std::endl;
      for(ExtensionSet::iterator itr=extensionSet.begin();
          itr!=extensionSet.end();
          ++itr)
      {
        std::cout<<"    "<<*itr<<std::endl;
      }

    }

    // true if extension found in extensionSet.
    bool result = extensionSet.find(extension)!=extensionSet.end();

    if (result)
    {
#ifdef VERBOSE
      std::cout<<"OpenGL utility library extension '"<<extension<<"' is supported."<<std::endl;
#endif
    }
    else std::cout<<"OpenGL utility library extension '"<<extension<<"' is not supported."<<std::endl;

    return result;
  }
#endif
} // namespace FgeOpenGL
