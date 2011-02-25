///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: $
//             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 9455 $

#include "FgeOffscreen.h"

#include <math.h>

using namespace std;

/**
 * \par Description
 * The constructor. Initializes FBO and adds a render target and a depth buffer texture (both have number 1)
 * \param offscreenDimension the size of the offscreen textures in pixels.
 */
FgeOffscreen::FgeOffscreen(unsigned int offscreenDimension) {
    // init
    fbo = NULL;

    texID = 1;
    depthID = 1;

    sizeW = offscreenDimension;
    sizeH = offscreenDimension;

    usedW = sizeW;
    usedH = sizeH;

    textures.clear();
    depthTextures.clear();

    // now create the fbo
    fbo = new FgeFramebufferObject(true);
    //DUMP_GL_ERROR( "FgeOffscreen_FBO_CREATE" );

    // disable it
    fbo->disable();

    // add default texture
    addTexture();

    // add default depth texture
    addDepthTexture();

    // clear color is per default black
    clearColor = new float[4];
    clearColor[0] = 0.0;
    clearColor[1] = 0.0;
    clearColor[2] = 0.0;
    clearColor[3] = 1.0;
}

/**
 * \par Description
 * The constructor. Initializes FBO and adds a render target and a depth buffer texture (both have number 1)
 * \param width the size of the offscreen textures in pixels.
 * \param width the size of the offscreen textures in pixels.
 * \param autoScale automaticaly select texture valid size that is >= width and hight
 */
FgeOffscreen::FgeOffscreen(unsigned int width, unsigned int height,
        bool autoScale) {
    // init
    fbo = NULL;

    texID = 1;
    depthID = 1;

    usedW = width;
    usedH = height;

    if (!autoScale) {
        sizeH = height;
        sizeW = width;
    } else {
        // find next fitting texture

        double log2 = 0.693147;

        sizeW = (unsigned int) pow(2, ceil((log((double) width - 1) / log2)));
        sizeH = (unsigned int) pow(2, ceil((log((double) height - 1) / log2)));
    }

    textures.clear();
    depthTextures.clear();

    // now create the fbo
    fbo = new FgeFramebufferObject(true);
    //DUMP_GL_ERROR( "FgeOffscreen_FBO_CREATE" );

    // disable it
    fbo->disable();

    // add default texture
    addTexture();

    // add default depth texture
    addDepthTexture();

    // clear color is per default black
    clearColor = new float[4];
    clearColor[0] = 0.0;
    clearColor[1] = 0.0;
    clearColor[2] = 0.0;
    clearColor[3] = 1.0;

}

/**
 * \par Description
 * The destructor. Cleans all added textures.
 */
FgeOffscreen::~FgeOffscreen() {
    // deactivate the fbo
    deactivate();

    // delete the fbo
    delete fbo;

    // free the textures
    for (unsigned int i = 0; i < textures.size(); i++) {
        if (!textures[i].permanent)
            delete textures[i].texture;
    }
    textures.clear();

    // free the textures
    for (unsigned int i = 0; i < depthTextures.size(); i++) {
        if (!depthTextures[i].permanent)
            delete depthTextures[i].texture;
    }
    depthTextures.clear();
}

/**
 * \par Description
 * Selects the given texture as render target.
 * \param no The number of the texture to use. The first added texture is 1, the second 2 and so on.
 */
void FgeOffscreen::selectTexture(unsigned int no) {
    if (no > textures.size())
        return;

    // get the opengl id
    //  texID=textures[no-1].texture->getId();
    // store id
    texID = no;
}

/**
 * \par Description
 * Selects the given texture as depth target.
 * \param The number of the texture to use. The first added texture is 1, the second 2 and so on.
 */
void FgeOffscreen::selectDepthTexture(unsigned int no) {
    if (no > depthTextures.size())
        return;

    // get the opengl id
    //depthID=depthTextures[no-1].texture->getId();
    depthID = no;
}

/**
 * \par Description
 * Adds a new texture as render target, initializes it and selects it as new render target.
 * \param permanent if true the texture won't get deleted during destruction
 * \return the number of the texture in the list (used for selectTexture).
 */
unsigned int FgeOffscreen::addTexture(bool permanent) {
    // just create and push back
    FgeGLTexture* tmpTex = new FgeGLTexture(GL_TEXTURE_2D, sizeW, sizeH);
    tmpTex->initialize();
    TextureDescriptor tmpTexDesc = { permanent, tmpTex };
    textures.push_back(tmpTexDesc);

    // select this texture
    selectTexture(textures.size());

    // return number
    return textures.size();
}

/**
 * \par Description
 * Adds a new texture as depth target, initializes it and selects it as new depthr target.
 * \param permanent if true the texture won't get deleted during destruction
 * \return the number of the texture in the list (used for selectDepthTexture).
 */
unsigned int FgeOffscreen::addDepthTexture(bool permanent) {
    // just create and push back
    FgeGLTexture* tmpDepthTex =
            new FgeGLTexture(GL_TEXTURE_2D, sizeW, sizeH, true, GL_FLOAT);
    tmpDepthTex->initialize();
    TextureDescriptor tmpDepthTexDesc = { permanent, tmpDepthTex };
    depthTextures.push_back(tmpDepthTexDesc);

    // select this texture
    selectDepthTexture(depthTextures.size());

    // return number
    return depthTextures.size();
}

/**
 * \par Description
 * Sets the color to use as background color in fbo textures.
 * \param cc the color
 */
void FgeOffscreen::setClearColor(const float r, const float g, const float b)
{
  clearColor = new float[3];
  clearColor[0] = r;
  clearColor[1] = g;
  clearColor[2] = b;
}


/**
 * \par Description
 * Returns the color to use as background color in fbo textures.
 * \return the color
 */
float* FgeOffscreen::getClearColor() {
    return clearColor;
}

/**
 * \par Description
 * Gives the current render target's OpenGL ID.
 * \return the OpenGL ID
 */
GLuint FgeOffscreen::getTexID() {
    return textures[texID - 1].texture->getId();
}

/**
 * \par Description
 * Gives the OpenGL ID of the texture with number "no".
 * \return the OpenGL ID
 * \param no the number of the texture in list (returned by addTexture).
 */
GLuint FgeOffscreen::getTexID(unsigned int no) {
    if (no > textures.size())
        return 0;

    // get the opengl id
    return textures[no - 1].texture->getId();
}

/**
 * \par Description
 * Gives the current depthbuffer attachement texture's OpenGL ID.
 * \return the OpenGL ID
 */
GLuint FgeOffscreen::getDepthTexID() {
    return depthTextures[depthID - 1].texture->getId();
}

/**
 * \par Description
 * Gives the OpenGL ID of the depth texture with number "no".
 * \return the OpenGL ID
 * \param no the number of the depth texture in list (returned by addDepthTexture).
 */
GLuint FgeOffscreen::getDepthTexID(unsigned int no) {
    if (no > depthTextures.size())
        return 0;

    // get the opengl id
    return depthTextures[no - 1].texture->getId();
}

/**
 * \par Description
 * Gives the FgeGLTexture object of the depth texture with number "no".
 * \return the FgeGLTexture object
 * \param no the number of the depth texture in list (returned by addDepthTexture).
 */
FgeGLTexture* FgeOffscreen::getTexObject(unsigned int no) {
    if (no > textures.size())
        return 0;

    // get the object adress
    return textures[no - 1].texture;
}

/**
 * \par Description
 * Gives the FgeGLTexture object of the depth texture with number "no".
 * \return the FgeGLTexture object
 * \param no the number of the depth texture in list (returned by addDepthTexture).
 */
FgeGLTexture* FgeOffscreen::getDepthTexObject(unsigned int no) {
    if (no > depthTextures.size())
        return 0;

    // get the object adress
    return depthTextures[no - 1].texture;

}

/**
 * \par Description
 * Renders the given texture on a quad. Is usefull if you want a shader to modify your texture (filters).
 * \param id the OpenGL ID of the texture to render
 */
void FgeOffscreen::renderArbitraryTexture(GLuint id) {
    vector<GLuint> ids;
    ids.push_back(id);
    renderArbitraryTextures(ids);
}

/**
 * \par Description
 * Renders the given textures on a quad with multitexturing. Is usefull if you want a shader to modify your texture (filters).
 * Renders nothing if too much textures are specified.
 * \param ids the list of OpenGL ID's of the textures to render
 */
void FgeOffscreen::renderArbitraryTextures(vector<GLuint> &ids) {
    renderArbitraryTextures(ids, sizeW, sizeH, usedW, usedH);
}

/**
 * \par Description
 * Renders the given textures on a quad with multitexturing. Is usefull if you want a shader to modify your texture (filters).
 * Renders nothing if too much textures are specified.
 * \param ids the list of OpenGL ID's of the textures to render
 * \param width with of the quad
 * \param height height of the quad
 */
void FgeOffscreen::renderArbitraryTextures(vector<GLuint> &ids,
        unsigned int width, unsigned int height) {
    renderArbitraryTextures(ids, width, height, width, height);
}

/**
 * \par Description
 * Renders the given textures on a quad with multitexturing. Is usefull if you want a shader to modify your texture (filters).
 * Renders nothing if too much textures are specified.
 * \param ids the list of OpenGL ID's of the textures to render
 * \param width with of the quad
 * \param height height of the quad
 * \param areaWidth width of area to render
 * \param areaHeight height of area to render
 */
void FgeOffscreen::renderArbitraryTextures(vector<GLuint> &ids,
        unsigned int width, unsigned int height, unsigned int areaWidth,
        unsigned int areaHeight) {
    renderArbitraryTextures(ids, width, height, areaWidth, areaHeight, 0, 0);
}

/**
 * \par Description
 * Renders the given textures on a quad with multitexturing. Is usefull if you want a shader to modify your texture (filters).
 * Renders nothing if too much textures are specified. If the offset causes the specified area to exceed over quad bounds the area will be croped.
 * \param ids the list of OpenGL ID's of the textures to render
 * \param width with of the quad
 * \param height height of the quad
 * \param areaWidth width of area to render
 * \param areaHeight height of area to render
 * \param areaXOffset an X offset to move the area around
 * \param areaXOffset an Y offset to move the area around
 */
void FgeOffscreen::renderArbitraryTextures(vector<GLuint> &ids,
        unsigned int width, unsigned int height, unsigned int areaWidth,
        unsigned int areaHeight, int areaXOffset, int areaYOffset) {
    // area exceedes texture bounds?
    if (areaWidth + areaXOffset > width)
        areaWidth = width - areaXOffset;
    if (areaHeight + areaYOffset > height)
        areaHeight = height - areaYOffset;

    // enough units availiable?
    GLint numberUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &numberUnits);
    if ((unsigned int) numberUnits < ids.size()) {
#ifndef NODEBUG
        cout << __FILE__ << ":" << __LINE__
                << ": not enough texture units availiable (" << ids.size()
                << " of " << numberUnits << " requested)" << endl;
#endif
        return;
    }

    // push attribs on stack to get status quo on  end of rendering
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // select matrix and back it up
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // setup 2d projection matrix, viewport
    glViewport(0, 0, areaWidth, areaHeight);
    glOrtho(0.0, areaWidth, 0.0, areaHeight, -1, 1);

    // everything else is based upon the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // no lighting
    glDisable(GL_LIGHTING);

    for (unsigned int i = 0; i < ids.size(); i++) {

        // enable multitexture unit
        glActiveTextureARB(GL_TEXTURE0_ARB + i);

        // texturing
        glEnable(GL_TEXTURE_2D);

        // bind texture
        glBindTexture(GL_TEXTURE_2D, ids[i]);

        // setup wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // we do not want linear/mip mapped texture filters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        // texture environment
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    }

    // render the texture box
    glColor3f(1.0, 1.0, 1.0);
    glTranslatef((float) areaXOffset, (float) areaYOffset, 0.0);
    glBegin(GL_QUADS);

    for (unsigned int i = 0; i < ids.size(); i++)
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB + i, 0.0, 0.0);

    glVertex2i(0, 0);

    for (unsigned int i = 0; i < ids.size(); i++)
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB + i, 1.0, 0.0);

    glVertex2i(width, 0);

    for (unsigned int i = 0; i < ids.size(); i++)
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB + i, 1.0, 1.0);

    glVertex2i(width, height);

    for (unsigned int i = 0; i < ids.size(); i++)
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB + i, 0.0, 1.0);

    glVertex2i(0, height);

    glEnd();

    // restore the status quo
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // restore status quo
    glPopAttrib();

}

/**
 * \par Description
 * Renders the current texture on a quad. Is usefull if you want a shader to modify your texture (filters).
 */
void FgeOffscreen::renderTexture() {
    renderArbitraryTexture(getTexID());
}

/**
 * \par Description
 * Renders the given texture on a quad. Is usefull if you want a shader to modify your texture (filters).
 * \param no the number of texture in list (returned by addTexture).
 */
void FgeOffscreen::renderTexture(unsigned int no) {
    renderArbitraryTexture(getTexID(no));
}

/**
 * \par Description
 * Renders the current depth texture on a quad. Is usefull if you want a shader to modify your texture (filters).
 */
void FgeOffscreen::renderDepthTexture() {
    renderArbitraryTexture(getDepthTexID());
}

/**
 * \par Description
 * Renders the given texture on a quad. Is usefull if you want a shader to modify your texture (filters).
 * \param no the number of the depth texture in list (returned by addDepthTexture).
 */
void FgeOffscreen::renderDepthTexture(unsigned int no) {
    renderArbitraryTexture(getDepthTexID(no));
}

/**
 * \par Description
 * Returns the real texture width
 * \return texture width
 */
unsigned int FgeOffscreen::getTextureWidth() {
    return sizeW;
}

/**
 * \par Description
 * Returns the real texture height
 * \return texture height
 */
unsigned int FgeOffscreen::getTextureHeight() {
    return sizeH;
}

/**
 * \par Description
 * Returns the used texture area width
 * \return texture area width
 */
unsigned int FgeOffscreen::getTextureAreaWidth() {
    return usedW;
}

/**
 * \par Description
 * Returns the used texture area height
 * \return texture area height
 */
unsigned int FgeOffscreen::getTextureAreaHeight() {
    return usedH;
}

/**
 * \par Description
 * Sets the permanent flag for a given texture.
 * \param no the textures id
 * \param permanent the flag. true if the texture should not be deleted during destruction
 */
void FgeOffscreen::setTexturePermanent(unsigned int no, bool permanent) {
    if (no > textures.size())
        return;

    // get the descriptor
    textures[no - 1].permanent = permanent;
}

/**
 * \par Description
 * Returns the permanent flag for a given texture.
 * \param no the textures id
 * \return the flag. true if the texture should not be deleted during destruction
 */
bool FgeOffscreen::getTexturePermanent(unsigned int no) {
    if (no > textures.size())
        return false;

    // get the descriptor
    return textures[no - 1].permanent;
}

/**
 * \par Description
 * Sets the permanent flag for a given depth texture.
 * \param no the textures id
 * \param permanent the flag. true if the texture should not be deleted during destruction
 */
void FgeOffscreen::setDepthTexturePermanent(unsigned int no, bool permanent) {
    if (no > depthTextures.size())
        return;

    // get the descriptor�
    depthTextures[no - 1].permanent = permanent;
}

/**
 * \par Description
 * Returns the permanent flag for a given depth texture.
 * \param no the textures id
 * \return the flag. true if the texture should not be deleted during destruction
 */
bool FgeOffscreen::getDepthTexturePermanent(unsigned int no) {
    if (no > depthTextures.size())
        return false;

    // get the descriptor�
    return depthTextures[no - 1].permanent;
}

/**
 * \par Description
 * Just activates the Framebuffer Object. After calling this all rendering will go to the selected texture.
 */
void FgeOffscreen::activate() {
    /*
     // if something has went wrong BEFORE activate has been called
     //DUMP_GL_ERROR( "FgeOffscreen_PRE_ACTIVATE");

     if (fbo->getAttachedId(GL_COLOR_ATTACHMENT0_EXT)!=texID)
     {

     // unattach previously attached one
     fbo->unattach(GL_COLOR_ATTACHMENT0_EXT);
     //DUMP_GL_ERROR( "FgeOffscreen_FBO_UNATTACH_TEX" );

     // attach the selected texture
     fbo->attachTexture( GL_TEXTURE_2D, texID, GL_COLOR_ATTACHMENT0_EXT );
     //DUMP_GL_ERROR( "FgeOffscreen_FBO_ATTACH_TEX" );

     // bind it and check validity
     fbo->bind();
     //DUMP_GL_ERROR( "FgeOffscreen_FBO_BIND" );
     fbo->isValid();
     //DUMP_GL_ERROR( "FgeOffscreen_FBO_VALIDITY" );

     }

     if (fbo->getAttachedId(GL_DEPTH_ATTACHMENT_EXT)!=depthID)
     {
     // unattach previously attached one
     fbo->unattach(GL_DEPTH_ATTACHMENT_EXT);
     //DUMP_GL_ERROR( "FgeOffscreen_FBO_UNATTACH_DEPTHTEX" );

     // attach the selected depth texture
     fbo->attachTexture( GL_TEXTURE_2D, depthID, GL_DEPTH_ATTACHMENT_EXT );
     //DUMP_GL_ERROR( "FgeOffscreen_FBO_ATTACH_DEPTHTEX" );

     // check validity
     if( !fbo->isValid() )
     {
     std::cerr << "ERROR: invalid framebuffer" << std::endl;
     }
     //DUMP_GL_ERROR( "FgeOffscreen_DEPTHTEX_VALIDITY" );

     }

     // bind the fbo
     fbo->bind();
     //DUMP_GL_ERROR( "FgeOffscreen_FBO_BIND" );

     // select COLOR_ATTACHMENT0 as render target
     glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT ); // draw into the first texture
     //DUMP_GL_ERROR( "FgeOffscreen_DRAWBUFFER_SETFBO" );

     // clear color and depth texture
     glClearColor( clearColor.getRed(), clearColor.getGreen(), clearColor.getBlue(), clearColor.getAlpha());
     //DUMP_GL_ERROR( "FgeOffscreen_COLOR_CLEAR" );
     glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
     //DUMP_GL_ERROR( "FgeOffscreen_CLEAR_COLOR_DEPTH" );

     // copy viewport for later restoring
     glGetIntegerv( GL_VIEWPORT, &viewportParams[0] );
     glViewport( 0, 0,  usedW, usedH);
     //DUMP_GL_ERROR( "FgeOffscreen_VIEWPORT_SET" );
     */

    unsigned int list[] = { texID };
    activate(1, list);
}

/**
 * \par Description
 * Just activates the Framebuffer Object. After calling this all rendering will go to the selected textures.
 * \param num number of textures to use
 * \param texList list of texture id's
 */
void FgeOffscreen::activate(unsigned int num, unsigned int texList[]) {
    // valid parameters?
    if (num > textures.size())
        return;

    // if something has went wrong BEFORE activate has been called
    //DUMP_GL_ERROR( "FgeOffscreen_PRE_ACTIVATE");

    // activate all textures
    for (unsigned int i = 0; i < num; i++) {
        if (fbo->getAttachedId(GL_COLOR_ATTACHMENT0_EXT + i)
                != textures[texList[i] - 1].texture->getId()) {
            // unattach previously attached one
            fbo->unattach(GL_COLOR_ATTACHMENT0_EXT + i);
            //DUMP_GL_ERROR( "FgeOffscreen_FBO_UNATTACH_TEX" );

            // attach the selected texture
            fbo->attachTexture(GL_TEXTURE_2D,
                    textures[texList[i] - 1].texture->getId(),
                    GL_COLOR_ATTACHMENT0_EXT + i);
            //DUMP_GL_ERROR( "FgeOffscreen_FBO_ATTACH_TEX" );

            // bind it and check validity
            fbo->bind();
            //DUMP_GL_ERROR( "FgeOffscreen_FBO_BIND" );
            fbo->isValid();
            //DUMP_GL_ERROR( "FgeOffscreen_FBO_VALIDITY" );
        }
    }

    if (fbo->getAttachedId(GL_DEPTH_ATTACHMENT_EXT) != depthTextures[depthID
            - 1].texture->getId()) {
        // unattach previously attached one
        fbo->unattach(GL_DEPTH_ATTACHMENT_EXT);
        //DUMP_GL_ERROR( "FgeOffscreen_FBO_UNATTACH_DEPTHTEX" );

        // attach the selected depth texture
        fbo->attachTexture(GL_TEXTURE_2D,
                depthTextures[depthID - 1].texture->getId(),
                GL_DEPTH_ATTACHMENT_EXT );
        //DUMP_GL_ERROR( "FgeOffscreen_FBO_ATTACH_DEPTHTEX" );

        // check validity
        if (!fbo->isValid()) {
            std::cerr << "ERROR: invalid framebuffer" << std::endl;
        }
        //DUMP_GL_ERROR( "FgeOffscreen_DEPTHTEX_VALIDITY" );

    }

    // bind the fbo
    fbo->bind();
    //DUMP_GL_ERROR( "FgeOffscreen_FBO_BIND" );

    // select COLOR_ATTACHMENT0 as render target
    // DIRTY but it works for now
    GLenum mrt[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT,
            GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT,
            GL_COLOR_ATTACHMENT4_EXT, GL_COLOR_ATTACHMENT5_EXT,
            GL_COLOR_ATTACHMENT6_EXT, GL_COLOR_ATTACHMENT7_EXT,
            GL_COLOR_ATTACHMENT8_EXT, GL_COLOR_ATTACHMENT9_EXT,
            GL_COLOR_ATTACHMENT10_EXT, GL_COLOR_ATTACHMENT11_EXT,
            GL_COLOR_ATTACHMENT12_EXT, GL_COLOR_ATTACHMENT13_EXT,
            GL_COLOR_ATTACHMENT14_EXT, GL_COLOR_ATTACHMENT15_EXT };
    glDrawBuffers(num, mrt);
    //DUMP_GL_ERROR( "FgeOffscreen_DRAWBUFFER_SETFBO" );

    // clear color and depth texture
    glClearColor( clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    //DUMP_GL_ERROR( "FgeOffscreen_COLOR_CLEAR" );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //DUMP_GL_ERROR( "FgeOffscreen_CLEAR_COLOR_DEPTH" );

    // copy viewport for later restoring
    glGetIntegerv(GL_VIEWPORT, &viewportParams[0]);
    glViewport(0, 0, usedW, usedH);
    //DUMP_GL_ERROR( "FgeOffscreen_VIEWPORT_SET" );
}

/**
 * \par Description
 * Disables the Framebuffer Object.
 */
void FgeOffscreen::deactivate() {
    // if something has went wrong BEFORE deactivate has been called
    //DUMP_GL_ERROR( "FgeOffscreen_PRE_DEACTIVATE");

    // disable the FBO
    fbo->disable();
    //DUMP_GL_ERROR( "FgeOffscreen_FBO_DISABLE" );

    // set default renderbuffer
    glDrawBuffer(GL_BACK);
    //DUMP_GL_ERROR( "FgeOffscreen_DRAWBUFFER_SET" );

    fbo->disable();
    //DUMP_GL_ERROR( "FgeOffscreen_FBO_DISABLE" );

    // restore viewport
    glViewport(viewportParams[0], viewportParams[1], viewportParams[2],
            viewportParams[3]);
    //DUMP_GL_ERROR( "FgeOffscreen_VIEWPORT_RESTORE" );
}

