/////////////////////////////////////////////////////////////////////////////
// Name:            ODFs.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/07/2009
//
// Description: ODFs class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////
#ifndef ODFS_H_
#define ODFS_H_

#include "datasetInfo.h"
#include "Glyph.h"

#include <complex>
#include <map>


using namespace std;

class ODFs : public Glyph
{
public:
    // Constructor/Destructor
	ODFs( DatasetHelper* datasetHelper );
	~ODFs();

    // From DatasetInfo
    void draw       ();
    bool load       ( wxString i_fileName );

    // Functions
    bool loadNifti  ( wxString i_fileName );
    
private:
    // From Glyph
    bool createStructure  ( vector< float >& i_fileFloatData );
    void drawGlyph        ( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis );
    void loadBuffer       ();
    void sliderPosChanged ( AxisType i_axis );    

    // Functions
    void             computeXRadiusSlice();
    void             computeYRadiusSlice();
    void             computeZRadiusSlice();

    void             computeRadiiArray          ( const FMatrix &i_B, vector< float > &i_C, 
                                                  vector< float >& o_radius, 
                                                  pair< float, float > &o_minMax );

    void             getODFpoints               ( FMatrix &i_phiThetaDirection, 
                                                  vector < float > &o_deformedMeshPts );    
    complex< float > getSphericalHarmonic       ( int i_l, int i_m, float i_theta, float i_phi );
    void             getSphericalHarmonicMatrix ( const vector< float > &i_meshPts, 
                                                  FMatrix &o_phiThetaDirection, 
                                                  FMatrix &o_shMatrix );
    void             getSphericalHarmonicMatrixRR5768 ( const vector< float > &i_meshPts, 
                                                           FMatrix &o_phiThetaDirection, 
                                                           FMatrix &o_shMatrix );
    void getSphericalHarmonicMatrixDescoteauxThesis ( const vector< float > &i_meshPts, 
                                                      FMatrix &o_phiThetaDirection, 
                                                      FMatrix &o_shMatrix );
    void             getSphericalHarmonicMatrixTournier ( const vector< float > &i_meshPts, 
                                                           FMatrix &o_phiThetaDirection, 
                                                           FMatrix &o_shMatrix );
    void             loadRadiusBuffer           ( AxisType i_axis );
    void             reloadRadiusBuffer         ( AxisType i_axis );

    // Variables
    int     m_order;
    GLuint  m_radiusAttribLoc;
    GLuint* m_radiusBuffer; 

    vector< vector < float > >        m_coefficients;
    vector< vector < float > >        m_radius;
    vector< FMatrix >                 m_shMatrix;
    vector< FMatrix >                 m_phiThetaDirection;    
    vector< float >                   m_meshPts;    
    map< int, pair< float, float > >  m_radiiMinMaxMap;

    int                               m_sh_basis;
};

#endif /* ODFS_H_ */
