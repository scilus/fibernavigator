/*
 *  The RTTFibers class declaration.
 *
 */

#ifndef RTT_FIBERS_H_
#define RTT_FIBERS_H_

#include "../misc/Fantom/FArray.h"
#include "../misc/Fantom/FMatrix.h"
#include "../misc/IsoSurface/Vector.h"
#include "Tensors.h"

#include <GL/glew.h>
#include <vector>

class RTTFibers 
{
public:
    RTTFibers(); //Constructor
    ~RTTFibers(); //Destructor

    //RTT functions
    void seed();
    void renderRTTFibers();
    void performRTT( Vector seed, int bwdfwd, std::vector<Vector>& points, std::vector<Vector>& color );
    void setDiffusionAxis( const FMatrix &tensor, Vector& e1, Vector& e2, Vector& e3 );

    Vector generateRandomSeed( const Vector &min, const Vector &max );
    FMatrix trilinearInterp( float fx, float fy, float fz );
    Vector advecIntegrate( Vector vin, const FMatrix &tensor, Vector e1, Vector e2, Vector e3, float tensorNumber );
    
    void clearFibersRTT()                           { m_fibersRTT.clear(); }
    void clearColorsRTT()                           { m_colorsRTT.clear(); }

    void setFAThreshold( float FAThreshold )						  { m_FAThreshold = FAThreshold; }
    void setTensorsMatrix( const std::vector<FMatrix> tensorsMatrix ) { m_tensorsMatrix = tensorsMatrix; }
    void setTensorsEV( const std::vector< F::FVector > tensorsEV )    { m_tensorsEV = tensorsEV; }
    void setTensorsFA( const std::vector<float> tensorsFA )           { m_tensorsFA = tensorsFA; }
    void setAngleThreshold( float angleThreshold )					  { m_angleThreshold = angleThreshold; }
    void setPuncture( float puncture )								  { m_puncture = puncture; }
    void setStep( float step )										  { m_step = step; }
    void setNbSeed ( float nbSeed )									  { m_nbSeed = nbSeed; }
    void setMinFiberLength( float minLength )						  { m_minFiberLength = minLength; }
    void setMaxFiberLength( float maxLength )						  { m_maxFiberLength = maxLength; }
    void setTensorsInfo( Tensors* info )							  { m_pTensorsInfo = info; }
	void setShellInfo( DatasetInfo* info )							  { m_pShellInfo = info; }

    float getFAThreshold()                       { return m_FAThreshold; }
    float getAngleThreshold()                    { return m_angleThreshold; }
    float getStep()                              { return m_step; }
    float getNbMeshPoint()                       { return m_nbMeshPt; }
	float getShellSeedNb();						 

    float getPuncture()                          { return m_puncture; }
    float getMinFiberLength()                    { return m_minFiberLength; } 
    float getMaxFiberLength()                    { return m_maxFiberLength; }
    
    wxString getTensorsFileName()                     { return m_pTensorsInfo->getPath();}
    size_t getSize()                                  { return m_fibersRTT.size(); }
	std::vector<std::vector<Vector> >* getRTTFibers() { return &m_fibersRTT; }
    

    //GPGPU functions
    bool checkFramebufferStatus(void);
    void checkGLErrors(const char *label);
    void compareResults(void);
    void createTextures(void);
    void createAllTextureParameters(void);
    void initGLSL(void);
    void initFBO(void);
    void initGLEW(void);
    void initGLUT(int argc, char** argv);
    void performComputation(void);
    void printProgramInfoLog(GLuint obj);
    void printShaderInfoLog(GLuint obj);
    void printVector(const float *p, const int N);
    void setupTexture(const GLuint texID);
    void swap(void);
    void transferFromTexture(float* data);
    void transferToTexture(float* data, GLuint texID);
    void setupALL();

    //GPGPU vars
    // problem size, texture size, number of iterations (set from command line)
    int N;
    int texSize;
    int numIterations;

    // texture identifiers
    GLuint yTexID[2];
    GLuint xTexID;
    // ping pong management vars
    int writeTex;
    int readTex;
    GLenum attachmentpoints[2];


    // FBO identifier
    GLuint fb;
    GLint yParam;
    GLuint fbo;

    float* seeds;
    float* result;
    float* xValues;

private:
    float       m_FAThreshold;
    float       m_angleThreshold;
    float       m_step;
    float       m_nbSeed;
    float       m_nbMeshPt;
    float       m_puncture;
    float       m_minFiberLength;
    float       m_maxFiberLength;
    Tensors     *m_pTensorsInfo;
	DatasetInfo *m_pShellInfo;


    std::vector< FMatrix > m_tensorsMatrix;
    std::vector< F::FVector >  m_tensorsEV;
    std::vector<float> m_tensorsFA;
    std::vector<std::vector<Vector> > m_fibersRTT;
    std::vector<std::vector<Vector> > m_colorsRTT;

};

#endif /* RTT_FIBERS_H_ */
