///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: $
//             Author   :   $Author: wiebel $
//////////              Revision :   $Revision: 8815 $

#include "FgeImageSpaceLIC.h"

#include "FgeFramebufferObject.h"
#include "FgeRenderbuffer.h"
#include "FgeGLTexture.h"

#include "wx/wxprec.h"

#include "../../Logger.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

using namespace std;

FgeImageSpaceLIC::FgeImageSpaceLIC(DatasetHelper* pDh) 
:  m_transformShader( wxT("transform") ),
   m_edgeShader( wxT( "edgedetection" ) ),
   m_advectionShader( wxT( "advection" ) ),
   m_clippingShader( wxT( "clipping" ) )
{

    m_dh = pDh;

    minX = 0.0f;
    minY = 0.0f;
    minZ = 0.0f;
    maxX = 1.0f;
    maxY = 1.0f;
    maxZ = 1.0f;

    // init the shaders
    m_dh->m_scheduledReloadShaders = true;

    // 2 iterations by default
    iterations = 2;

    // blend factor default
    noiseBlend = 0.1f;
    // blend factor of colormap
    colormapBlend = 0.3f;

    // default scaler
    tensorAdvectionScale = 2.0f;

    // there is no previously advected texture
    previousAdvectedTexture = NULL;

    // currently there is no noise texture
    noiseTexture = NULL;

    frameSize = 20;
}

FgeImageSpaceLIC::~FgeImageSpaceLIC() {
    if (noiseTexture)
        delete noiseTexture;

    // delete stored advection image
    if (previousAdvectedTexture)
        delete previousAdvectedTexture;
}

void FgeImageSpaceLIC::initiate() {
    //FgeGeometry::initiate();
}

/**
 * \par Description
 * Sets the number of iterations to use for LIC.
 * \param it the iteration count
 */
void FgeImageSpaceLIC::setIterations(unsigned int it) {
    iterations = it;
}

/**
 * \par Description
 * Returns the currently set number of iterations
 * \return the iteration count
 */
unsigned int FgeImageSpaceLIC::getIterations() {
    return iterations;
}

/**
 * \par Description
 * Sets the blending factor for blending original noise image with advected noise image
 * \param nb the blend factor (between 0 and 1)
 */
void FgeImageSpaceLIC::setNoiseBlend(float nb) {
    if (nb > 1.0)
        nb = 1.0;
    else if (nb < 0.0)
        nb = 0.0;

    noiseBlend = nb;
}

/**
 * \par Description
 * Gets the currently set blending factor for blending original noise image with advected noise image
 * \return the blend factor (between 0 and 1)
 */
float FgeImageSpaceLIC::getNoiseBlend() {
    return noiseBlend;
}

/**
 * \par Description
 * Sets the blending factor for blending in the color coded tensors.
 * \param nb the blend factor (between 0 and 1)
 */
void FgeImageSpaceLIC::setColormapBlend(float cb) {
    if (cb > 1.0)
        cb = 1.0;
    else if (cb < 0.0)
        cb = 0.0;

    colormapBlend = cb;
}

/**
 * \par Description
 * Gets the currently set blending factor for blending in the color coded tensors.
 * \return the blend factor (between 0 and 1)
 */
float FgeImageSpaceLIC::getColormapBlend() {
    return colormapBlend;
}

/**
 * \par Description
 * Sets the animation flag.
 * \param aa the animation flag. If true there will be some particle flow visible on the surface.
 */
void FgeImageSpaceLIC::setAdvectionAnimation(bool aa) {
    advectionAnimation = aa;
}

/**
 * \par Description
 * Gets the currently set animation flag.
 * \return the animation flag
 */
bool FgeImageSpaceLIC::getAdvectionAnimation() {
    return advectionAnimation;
}

/**
 * \par Description
 * Sets the scaling value used for scaling the advection vectors
 * \param tas the scale value
 */
void FgeImageSpaceLIC::setTensorAdvectionScale(float tas) {
    tensorAdvectionScale = tas;
}

/**
 * \par Description
 * Gets the currently set scaling value used for scaling the advection vectors
 * \return the scale value
 */
float FgeImageSpaceLIC::getTensorAdvectionScale() {
    return tensorAdvectionScale;
}

/**
 * \par Description
 * Reloads all needed shaders, compiles them, links them, dumps errors.
 */
void FgeImageSpaceLIC::reloadShaders() {

    Logger::getInstance()->print( _T( "Reloading shaders..." ), LOGLEVEL_MESSAGE );

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Transformation Shader
    /////////////////////////////////////////////////////////////////////////////////////////////

    m_transformShader.release();

    Logger::getInstance()->print( _T( "Initializing LIC texture shader..." ), LOGLEVEL_MESSAGE );
    if( m_transformShader.load() && m_transformShader.compileAndLink() )
    {
        Logger::getInstance()->print( _T( "LIC texture shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize LIC texture shader." ), LOGLEVEL_ERROR );
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Edge Detection Shader
    /////////////////////////////////////////////////////////////////////////////////////////////

    m_edgeShader.release();

    Logger::getInstance()->print( _T( "Initializing LIC edge shader..." ), LOGLEVEL_MESSAGE );
    if( m_edgeShader.load() && m_edgeShader.compileAndLink() )
    {
        Logger::getInstance()->print( _T( "LIC edge shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize LIC edge shader." ), LOGLEVEL_ERROR );
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Advection Shader
    /////////////////////////////////////////////////////////////////////////////////////////////

    m_advectionShader.release();

    Logger::getInstance()->print( _T( "Initializing LIC advection shader..." ), LOGLEVEL_MESSAGE );
    if( m_advectionShader.load() && m_advectionShader.compileAndLink() )
    {
        Logger::getInstance()->print( _T( "LIC advection shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize LIC advection shader." ), LOGLEVEL_ERROR );
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Clipping Shader
    /////////////////////////////////////////////////////////////////////////////////////////////

    m_clippingShader.release();

    Logger::getInstance()->print( _T( "Initializing LIC clipping shader..." ), LOGLEVEL_MESSAGE );
    if( m_clippingShader.load() && m_clippingShader.compileAndLink() )
    {
        Logger::getInstance()->print( _T( "LIC clipping shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize LIC clipping shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->print( _T( "Finished reloading shaders." ), LOGLEVEL_MESSAGE );
}

void FgeImageSpaceLIC::render(DatasetInfo *info) {
    // to later on restore status quo
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport); // get the current viewport


    /////////////////////////////////////////////////////////////////////////////////////////////
    // PRERENDER
    //
    // Set uniforms, attribs and so on; they are needed in more than one pass
    /////////////////////////////////////////////////////////////////////////////////////////////

    // if reload is scheduled -> reload
    if (m_dh->m_scheduledReloadShaders)
        reloadShaders();
    m_dh->m_scheduledReloadShaders = false;

    // initialize Framebuffer for offscreen rendering ( calculates proper texture sizes for us )
    FgeOffscreen fbo(viewport[2] + frameSize, viewport[3] + frameSize, true);

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 1
    //
    // Render geometry as is, for later edge detection, also transform tensors to image space
    /////////////////////////////////////////////////////////////////////////////////////////////


    /* The Problem:
     *
     * The black background color in color coded image would result in invalid tensors after
     * unscaling in advection shader. So we use the apropriate background color that would result
     * in a 0,0,0 vector after unscaling (ok we could have used the depth buffer for detecting
     * this but we just want to use 4 texture units during advection -> so we use this way.
     */

    fbo.setClearColor(0.0, 0.0, 0.0);
    //unsigned int list[] = { 1,2 };
    //fbo.activate(2, list );
    fbo.activate();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 1a -- prepare projectionmatrix
    /////////////////////////////////////////////////////////////////////////////////////////////

    // just scaling the texture is not enough, we also need to scale the projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    // get the current projection matrix
    float projection[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    // scale first two rows
    projection[0] *= 1.0 - (float) frameSize / (float) viewport[2];
    projection[5] *= 1.0 - (float) frameSize / (float) viewport[3];

    // upload new matrix
    //  glLoadIdentity();
    //  glMultMatrixf(projection);

    glScalef(1.0 - (float) frameSize / (float) viewport[2], 1.0
            - (float) frameSize / (float) viewport[3], 1.0);

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 1b -- render geometry
    /////////////////////////////////////////////////////////////////////////////////////////////

    m_transformShader.bind();

    // give the min max infos
    m_transformShader.setUniFloat("minX", minX);
    m_transformShader.setUniFloat("maxX", maxX);
    m_transformShader.setUniFloat("minY", minY);
    m_transformShader.setUniFloat("maxY", maxY);
    m_transformShader.setUniFloat("minZ", minZ);
    m_transformShader.setUniFloat("maxZ", maxZ);


    m_transformShader.setUniInt("dimX", m_dh->m_columns);
    m_transformShader.setUniInt("dimY", m_dh->m_rows);
    m_transformShader.setUniInt("dimZ", m_dh->m_frames);

    GLint* tex = new GLint[10];
    float* threshold = new float[10];
    GLint* type = new GLint[10];
    int c = 0;
    for (int i = 0 ; i < m_dh->m_mainFrame->m_pListCtrl->GetItemCount() ; ++i)
    {
        DatasetInfo* info = (DatasetInfo*)m_dh->m_mainFrame->m_pListCtrl->GetItemData(i);
        if(info->getType() < MESH) {
            tex[c] = c;
            threshold[c] = info->getThreshold();
            type[c] = info->getType();
            ++c;
        }
    }

    m_transformShader.setUniArrayInt("texes", tex, c);
    m_transformShader.setUniArrayInt("type", type, c);
    m_transformShader.setUniArray1Float("threshold", threshold, c);
    m_transformShader.setUniInt("countTextures", c);

    // render -> the shader does all the work for us
    info->draw();

    m_transformShader.release();

    fbo.deactivate();

    const unsigned int geometryDepth = 1;
    const unsigned int colorCodedTensors = 1; // the id's of the color coded texture and the depth texture


    // restore matrix stuff
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 2
    //
    // use generated Texture for edgedetection
    /////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 2a -- apply edge detection filter
    /////////////////////////////////////////////////////////////////////////////////////////////

    fbo.setClearColor(0.0, 0.0, 0.0);

    // aktivate fbo
    fbo.addDepthTexture(); // this is needed because rendering involves the old depth texture
    const unsigned int edgeImage = fbo.addTexture();
    fbo.activate();

    // now just render a quad with the depth texture to apply a edge detection filter, use the first created depth tex
    m_edgeShader.bind();
    m_edgeShader.setUniFloat("textureSizeW", fbo.getTextureWidth());
    m_edgeShader.setUniFloat("textureSizeH", fbo.getTextureHeight());
    fbo.renderDepthTexture(geometryDepth);

    m_edgeShader.release();

    fbo.deactivate();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 3
    //
    // generate noise image
    /////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 3a -- generate white noise
    /////////////////////////////////////////////////////////////////////////////////////////////

    // if fbo's size has changed -> rebuild noise texture
    if (noiseTexture && ((noiseTexture->getWidth() != fbo.getTextureWidth())
            || (noiseTexture->getHeight() != fbo.getTextureHeight()))) {
        delete noiseTexture;
        noiseTexture = NULL;
    }
    /*
    // rebuild the noise texture every render pass
    if (noiseTexture) {
        delete noiseTexture;
        noiseTexture = NULL;
    }
*/
    // build noise texture with proper size
    if (!noiseTexture) {
#ifndef NODEBUG
        cout << __FILE__ << ":" << __LINE__ << ": rebuilding noise texture: "
                << fbo.getTextureWidth() << "x" << fbo.getTextureHeight()
                << endl;
#endif

        // generate white noise texture
#ifdef __WXMSW__
        srand(time(0));
#else
        srand48(time(0));
#endif
        unsigned char *randomLuminance =
                new unsigned char[fbo.getTextureWidth()
                        * fbo.getTextureHeight()];
#ifdef __WXMSW__
        for (unsigned int x = 0; x < fbo.getTextureWidth(); x++)
            for (unsigned int y = 0; y < fbo.getTextureHeight(); y++)
                randomLuminance[(y * fbo.getTextureWidth()) + x]
                        = (unsigned char) (255.0 * rand());
#else
        for (unsigned int x = 0; x < fbo.getTextureWidth(); x++)
            for (unsigned int y = 0; y < fbo.getTextureHeight(); y++)
                randomLuminance[(y * fbo.getTextureWidth()) + x]
                        = (unsigned char) (255.0 * drand48());
#endif
        // create texture
        noiseTexture
                = new FgeGLTexture(GL_TEXTURE_2D, fbo.getTextureWidth(), fbo.getTextureHeight());
        noiseTexture->initialize();

        // upload image
        noiseTexture->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, fbo.getTextureWidth(),
                fbo.getTextureHeight(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                randomLuminance);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 4 (iteration times passes)
    //
    // iterate advection shader over the noise image and the advected image
    /////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 4a -- set shaders uniforms
    /////////////////////////////////////////////////////////////////////////////////////////////

    // use advection shader now
    m_advectionShader.bind();

    wxDateTime dt = wxDateTime::Now();
    int time = dt.GetMillisecond();
    m_advectionShader.setUniInt("fantom_time", time);

    // set texture unit numbers
    m_advectionShader.setUniFloat("textureSizeW", fbo.getTextureWidth());
    m_advectionShader.setUniFloat("textureSizeH", fbo.getTextureHeight());
    m_advectionShader.setUniFloat("textureAreaSizeW", fbo.getTextureAreaWidth());
    m_advectionShader.setUniFloat("textureAreaSizeH", fbo.getTextureAreaHeight());

    m_advectionShader.setUniInt("tex0", 0);
    m_advectionShader.setUniInt("tex1", 1);
    m_advectionShader.setUniInt("tex2", 2);
    m_advectionShader.setUniInt("tex3", 3);

    // give the blending and scaling parameters
    m_advectionShader.setUniFloat("noiseBlend", noiseBlend);
    m_advectionShader.setUniFloat("tensorAdvectionScale", tensorAdvectionScale * info->getAlpha()*5);

    // animation flag
    //m_advectionShader.setUniFloat("advectionAnimation", advectionAnimation ? 1.0 : 0.0);
    m_advectionShader.setUniFloat("advectionAnimation", 1.0 );

    m_advectionShader.release();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 4b -- setup texture list
    /////////////////////////////////////////////////////////////////////////////////////////////

    // set up the list with the noise texture and the current image, the depth tex and so on
    vector<GLuint> texIdList;
    texIdList.push_back(fbo.getTexID(colorCodedTensors)); // color coded tensor image
    texIdList.push_back(noiseTexture->getId()); // noise image
    texIdList.push_back(fbo.getTexID(edgeImage)); // edges
    if (previousAdvectedTexture)
        texIdList.push_back(previousAdvectedTexture->getId());// start with old advection texture
    else
        texIdList.push_back(noiseTexture->getId()); // start with the original noise image

    // we need one texture as target and one as (advected) input texture
    // we have to alter between them each step
    unsigned int advectedImage1 = fbo.addTexture();
    unsigned int advectedImage2 = fbo.addTexture();

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 4c -- iterate over the advected image
    /////////////////////////////////////////////////////////////////////////////////////////////
    m_advectionShader.bind();

    // iterate
    for (unsigned int i = 0; i < iterations; i++) {
        fbo.selectTexture(i % 2 ? advectedImage1 : advectedImage2);
        fbo.activate();

        fbo.renderArbitraryTextures(texIdList);

        fbo.deactivate();

        // now alter the advected image to be the last render target
        texIdList.pop_back();
        texIdList.push_back(i % 2 ? fbo.getTexID(advectedImage1)
                : fbo.getTexID(advectedImage2));

    }

    m_advectionShader.release();

    // now make the advected texture permanent
    fbo.setTexturePermanent(iterations % 2 ? advectedImage2 : advectedImage1);

    // delete old permanent texture
    if (previousAdvectedTexture)
        delete previousAdvectedTexture;

    // and get the new texture object
    previousAdvectedTexture = fbo.getTexObject(iterations % 2 ? advectedImage2
            : advectedImage1);

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 5 -- FINAL OUTPUT
    //
    // now use depth image to cut away noise on not geometry fragments
    /////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 5a -- set shaders uniforms
    /////////////////////////////////////////////////////////////////////////////////////////////

    // now just render a quad with the former result advection image and use depth map to clip
    m_clippingShader.bind();
    m_clippingShader.setUniFloat("textureSizeW", fbo.getTextureWidth());
    m_clippingShader.setUniFloat("textureSizeH", fbo.getTextureHeight());
    m_clippingShader.setUniInt("tex0", 0);
    m_clippingShader.setUniInt("tex1", 1);
    m_clippingShader.setUniInt("tex2", 2);
    m_clippingShader.setUniInt("tex3", 3);

    // blending parameters
    m_clippingShader.setUniFloat("colormapBlend", colormapBlend);

    /////////////////////////////////////////////////////////////////////////////////////////////
    // PASS 5b -- setup texture list and render to quad
    /////////////////////////////////////////////////////////////////////////////////////////////

    texIdList.clear();
    texIdList.push_back( fbo.getTexID() );
    texIdList.push_back( fbo.getDepthTexID( geometryDepth ) );
    texIdList.push_back( fbo.getTexID( colorCodedTensors ) ); // color coded tensor image
    texIdList.push_back( fbo.getTexID( edgeImage ) ); // edge detection image

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    fbo.renderArbitraryTextures( texIdList, 
                                 fbo.getTextureWidth(),
                                 fbo.getTextureHeight(),
                                 fbo.getTextureAreaWidth() - frameSize,
                                 fbo.getTextureAreaHeight() - frameSize,
                                 -frameSize / 2, 
                                 -frameSize / 2 );

    m_clippingShader.release();

    //m_dh->m_mainFrame->m_gl0->testRender(fbo.getDepthTexID(geometryDepth));
    //m_dh->m_mainFrame->m_gl1->testRender(fbo.getTexID(advectedImage1));
    //m_dh->m_mainFrame->m_gl2->testRender(fbo.getTexID(advectedImage2));
    //m_dh->m_mainFrame->m_gl1->testRender(fbo.getTexID(edgeImage));
    //m_dh->m_mainFrame->m_gl2->testRender(fbo.getTexID(colorCodedTensors));

    /////////////////////////////////////////////////////////////////////////////////////////////
    // RESTORE STATUS QUO
    //
    //
    /////////////////////////////////////////////////////////////////////////////////////////////

    // and restore all attributes to previous status
    glPopAttrib();

    // make it repaint and repaint all the time!!!
    //GLint time = advectionShader.getUniformLocation( "fantom_time" );
    //  if ( time != -1 )
}

/**
 * \par Description:
 * Sets the minima and maxima used to scale the color values.
 * \param minX minimum X value
 * \param minY minimum Y value
 * \param minZ minimum Z value
 * \param maxX minimum X value
 * \param maxY minimum Y value
 * \param maxZ minimum Z value
 *
 */
void FgeImageSpaceLIC::setMinMax(float minX, float minY, float minZ,
        float maxX, float maxY, float maxZ) {

    this->minX = minX;
    this->minY = minY;
    this->minZ = minZ;

    this->maxX = maxX;
    this->maxY = maxY;
    this->maxZ = maxZ;

}

