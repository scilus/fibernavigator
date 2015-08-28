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
#include "Maximas.h"
#include "Anatomy.h"

#include <GL/glew.h>
#include <vector>

class RTTFibers 
{
public:
    RTTFibers(); //Constructor
	~RTTFibers(); //Destructor

    //RTT functions
    void seed();
    void renderRTTFibers(bool bindBuffers, bool isPlaying, bool changeAlpha);
    void performDTIRTT( Vector seed, int bwdfwd, std::vector<float>& points, std::vector<float>& color );
    void performHARDIRTT( Vector seed, int bwdfwd, std::vector<float>& points, std::vector<float>& color );
    void setDiffusionAxis( const FMatrix &tensor, Vector& e1, Vector& e2, Vector& e3 );
	std::vector<float> pickDirection(std::vector<float> initialPeaks, bool initWithDir, Vector currPos);
    bool withinMapThreshold(unsigned int sticksNumber, Vector pos);

    Vector generateRandomSeed( const Vector &min, const Vector &max );
    FMatrix trilinearInterp( float fx, float fy, float fz );
    Vector advecIntegrate( Vector vin, const FMatrix &tensor, Vector e1, Vector e2, Vector e3, float tensorNumber );
    Vector advecIntegrateHARDI( Vector vin, const std::vector<float> &sticks, float peaksNumber, Vector pos );
    Vector magneticField( Vector vin, const std::vector<float> &sticks, float peaksNumber, Vector pos, Vector& vOut, float& F);
    
    void clearFibersRTT();

    void setFAThreshold( float FAThreshold )						  { m_FAThreshold = FAThreshold; }
    void setTensorsMatrix( const std::vector<FMatrix> tensorsMatrix ) { m_tensorsMatrix = tensorsMatrix; }
    void setTensorsEV( const std::vector< F::FVector > tensorsEV )    { m_tensorsEV = tensorsEV; }
    void setTensorsFA( const std::vector<float> tensorsFA )           { m_tensorsFA = tensorsFA; }
    void setAngleThreshold( float angleThreshold )					  { m_angleThreshold = angleThreshold; }
    void setPuncture( float puncture )								  { m_puncture = puncture; }
    void setVinVout( float vinvout )								  { m_vinvout = vinvout; }
    void setStep( float step )										  { m_step = step; }
    void setGMStep( float step )                                      { m_GMstep = step; }
    void setIsHardi( bool method )								      { m_isHARDI = method; }
    void setNbSeed ( float nbSeed )									  { m_nbSeed = nbSeed; }
    void setMinFiberLength( float minLength )						  { m_minFiberLength = minLength; }
    void setMaxFiberLength( float maxLength )						  { m_maxFiberLength = maxLength; }
    void setTensorsInfo( Tensors* info )							  { m_pTensorsInfo = info; }
    void setHARDIInfo( Maximas* info );							      
	void setShellInfo( DatasetInfo* info )							  { m_pShellInfo = info; }
    void setMaskInfo( Anatomy* info )                                 { m_pMaskInfo = info; }
    void setGMInfo( Anatomy* info )                                   { m_pGMInfo = info; }
    void setInitSeed( Vector init )                                   { m_initVec = init; } 

	void setOpacity( float alpha )                                    { m_alpha = alpha; }
    float getOpacity() {return m_alpha;}
	void setSeedMapInfo( Anatomy* info );	
	void setSeedFromfMRI( const std::vector<std::pair<Vector,float> > &seedFromfMRI )	  { m_pSeedFromfMRI = seedFromfMRI; }
    void insertPointsForTractoDriven( std::vector<float> pointsF, std::vector<float> pointsB);

	void setExcludeInfo( Anatomy* info )                              { m_pExcludeInfo = info; }
	bool checkExclude(unsigned int sticksNumber);						  

    float getFAThreshold()                       { return m_FAThreshold; }
    float getAngleThreshold()                    { return m_angleThreshold; }
    float getStep()                              { return m_step; }
    float getNbMeshPoint()                       { return m_nbMeshPt; }
	float getShellSeedNb();		
	float getSeedMapNb();

    float getPuncture()                          { return m_puncture; }
    float getVinVout()                           { return m_vinvout; }
    float getMinFiberLength()                    { return m_minFiberLength; } 
    float getMaxFiberLength()                    { return m_maxFiberLength; }
	void insert(std::vector<Vector> pointsF, std::vector<Vector> pointsB, std::vector<Vector> colorF, std::vector<Vector> colorB);

    bool isHardiSelected()                       { return m_isHARDI;}
    
    wxString getRTTFileName()                    { if(m_isHARDI) 
                                                        return m_pMaximasInfo->getPath(); 
                                                   else
                                                        return m_pTensorsInfo->getPath(); }

    size_t getSize()                                  { return m_streamlinesPoints.size(); }

	std::vector<float>* getRTTFibers() { return &m_streamlinesPoints; }
    std::vector<int>* getRTTLinePointer() { return &m_linePointer; }
    std::vector<int>* getRTTNbPointsPerLine() { return &m_nbPtsPerLine; }
    std::vector<bool>* getRTTLeftRightVector() { return & m_LeftRightVector;}


	unsigned int  m_trackActionStep;
	float m_timerStep;
	std::vector<Vector> m_pSeedMap;
	
private:
    float       m_FAThreshold;
    float       m_angleThreshold;
    float       m_step;
    int         m_GMstep;
    float       m_nbSeed;
    float       m_nbMeshPt;
    float       m_puncture;
    float       m_vinvout;
    float       m_minFiberLength;
    float       m_maxFiberLength;
    bool        m_isHARDI;
    float       m_countGMstep;
    Tensors     *m_pTensorsInfo;
    Maximas     *m_pMaximasInfo;
	DatasetInfo *m_pShellInfo;
    Anatomy     *m_pMaskInfo;
	Anatomy     *m_pExcludeInfo;
	Anatomy     *m_pSeedMapInfo;
    Anatomy     *m_pGMInfo;
    Vector       m_initVec;

    std::vector<float> m_streamlinesPoints; // Points to be rendered Forward
	std::vector<float> m_streamlinesColors; //Color (local directions)Forward
    

	float m_alpha;
    int m_currentSeedBoxID;
	bool m_stop;
    bool m_render;
    bool m_steppedOnceInsideChildBox;
    bool m_prune;
    GLuint*     m_bufferObjects;

    std::vector< FMatrix > m_tensorsMatrix;
    std::vector< F::FVector >  m_tensorsEV;
    std::vector<float> m_tensorsFA;
	std::vector<std::pair<Vector,float> > m_pSeedFromfMRI;
    std::vector<std::vector<Vector> > m_fibersRTT;
    std::vector<std::vector<Vector> > m_colorsRTT;
    std::vector< SelectionObject* > selObjs;

    std::vector< int > m_nbPtsPerLine;
    std::vector< int > m_linePointer;
    std::vector< bool > m_LeftRightVector;
    int m_lines;

};

#endif /* RTT_FIBERS_H_ */
