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

#include "DatasetInfo.h"
#include "Glyph.h"
#include "../misc/nifti/nifti1_io.h"
#include "../misc/Fantom/FMatrix.h"

#include <complex>
#include <map>

class MySlider;

class ODFs : public Glyph
{
public:
    // Constructor/Destructor
    ODFs();
    ODFs( const wxString &filename );
    virtual ~ODFs();

    // From DatasetInfo
    void draw       ();
    bool load       ( nifti_image *pHeader, nifti_image *pBody );
    bool save( wxXmlNode *pNode ) const;

    // Functions
    virtual void createPropertiesSizer(PropertiesWindow *parent);
    virtual void updatePropertiesSizer();
    
    bool isShBasis( int i_sh_basis )                    { return m_sh_basis == i_sh_basis; }
    std::vector< std::vector < float > > getCoeffs()    { return m_coefficients; }
    std::vector< FMatrix > getShMatrix()                { return m_shMatrix; }
    std::vector< FMatrix > getPhiTheta()                { return m_phiThetaDirection; }

    void setShBasis( int value )                        { m_sh_basis = value; }
    void changeShBasis( ODFs*, int );
    
	void extractMaximas();

public:
    //Vars
    wxString    m_lastODF_path;

    struct direction_value { double x, y, z, v; };
    struct direction { double x, y, z; };

	bool   m_isMaximasSet;
	float  m_axisThreshold;

    MySlider        *m_pSliderFlood;
    wxStaticText    *m_pTextThres;
    wxTextCtrl      *m_pTxtThresBox;
    wxButton        *m_pbtnMainDir;

private:
    // From Glyph
    bool createStructure  ( std::vector< float > &i_fileFloatData );
    void drawGlyph        ( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis );
    void loadBuffer       ();
    void sliderPosChanged ( AxisType i_axis );  
    

    // Functions
    void             computeXRadiusSlice();
    void             computeYRadiusSlice();
    void             computeZRadiusSlice();

    void             computeRadiiArray          ( const FMatrix &i_B, std::vector< float > &i_C, 
                                                  std::vector< float >& o_radius, 
                                                  std::pair< float, float > &o_minMax );

    void             getODFpoints               ( FMatrix &i_phiThetaDirection, 
                                                  std::vector < float > &o_deformedMeshPts );    
    std::complex< float > getSphericalHarmonic  ( int i_l, int i_m, float i_theta, float i_phi );
    void             getSphericalHarmonicMatrix ( const std::vector< float > &i_meshPts, 
                                                  FMatrix &o_phiThetaDirection, 
                                                  FMatrix &o_shMatrix );
    void             getSphericalHarmonicMatrixRR5768 ( const std::vector< float > &i_meshPts, 
                                                           FMatrix &o_phiThetaDirection, 
                                                           FMatrix &o_shMatrix );
    void getSphericalHarmonicMatrixDescoteauxThesis ( const std::vector< float > &i_meshPts, 
                                                      FMatrix &o_phiThetaDirection, 
                                                      FMatrix &o_shMatrix );
    void       getSphericalHarmonicMatrixPTK ( const std::vector< float > &i_meshPts, 
                                               FMatrix &o_phiThetaDirection, 
                                               FMatrix &o_shMatrix );
    void             getSphericalHarmonicMatrixTournier ( const std::vector< float > &i_meshPts, 
                                                          FMatrix &o_phiThetaDirection, 
                                                          FMatrix &o_shMatrix );
    void             loadRadiusBuffer           ( AxisType i_axis );
    void             reloadRadiusBuffer         ( AxisType i_axis );

    
	
    std::vector<Vector> getODFmax(std::vector < float >  coefs,const FMatrix & SHmatrix, 
                                  const FMatrix & grad,
								  const float & max_thresh);
    void			    set_nbors(FMatrix i_phiThetaDirection);
    float               get_min_angle();
    void setScalingFactor( float i_scalingFactor );
    
private:
    // Variables
    int     m_order;
    GLuint  m_radiusAttribLoc;
    GLuint* m_radiusBuffer; 
    wxRadioButton *m_pRadiobtnOriginalBasis;
    wxRadioButton *m_pRadiobtnDescoteauxBasis;
    wxRadioButton *m_pRadiobtnTournierBasis;
    wxRadioButton *m_pRadiobtnPTKBasis;

    std::vector< std::vector < float > >    m_coefficients;
    std::vector< std::vector < float > >    m_radius;
    std::vector< FMatrix >                  m_shMatrix;
    std::vector< FMatrix >                  m_phiThetaDirection;    
    std::vector< float >                    m_meshPts;    
    std::map< int, std::pair< float, float > > m_radiiMinMaxMap;
  
    float								m_angle_min;
    std::vector<std::pair<float,int> >* m_nbors;
    std::vector<std::vector<Vector> >   m_mainDirections;

	int                                 m_sh_basis;
};

#endif /* ODFS_H_ */
