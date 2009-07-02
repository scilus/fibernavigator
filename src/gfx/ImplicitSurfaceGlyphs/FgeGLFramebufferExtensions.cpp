#include "FgeGLFramebufferExtensions.h"

void FgeGLFramebufferExtensions::initialize( void )
{
    FgeOpenGL::lookupGLFuncPtrWarn( pglIsRenderbufferEXT, "glIsRenderbufferEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglDeleteRenderbuffersEXT, "glDeleteRenderbuffersEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglGenRenderbuffersEXT, "glGenRenderbuffersEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglGetRenderbufferParameterivEXT, "glGetRenderbufferParameterivEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglIsFramebufferEXT, "glIsFramebufferEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglBindFramebufferEXT, "glBindFramebufferEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglBindRenderbufferEXT, "glBindRenderbufferEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglDeleteFramebuffersEXT, "glDeleteFramebuffersEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglGenFramebuffersEXT, "glGenFramebuffersEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglCheckFramebufferStatusEXT, "glCheckFramebufferStatusEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglFramebufferTexture1DEXT, "glFramebufferTexture1DEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglFramebufferTexture2DEXT, "glFramebufferTexture2DEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglFramebufferTexture3DEXT, "glFramebufferTexture3DEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglFramebufferRenderbufferEXT, "glFramebufferRenderbufferEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglGetFramebufferAttachmentParameterivEXT, "glGetFramebufferAttachmentParameterivEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglGenerateMipmapEXT, "glGenerateMipmapEXT" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglRenderbufferStorageEXT, "glRenderbufferStorageEXT" );
}
