/////////////////////////////////////////////////////////////////////////////
// Name:            Tensors.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/07/2009
//
// Description: Tensors class.
//
// Last modifications:
//      by : ggirard - 29/12/2010
/////////////////////////////////////////////////////////////////////////////
#ifndef TENSORS_H_
#define TENSORS_H_

#include "DatasetInfo.h"
#include "Glyph.h"
#include "../misc/Fantom/FVector.h"
#include "../misc/lic/TensorField.h"
#include "../misc/nifti/nifti1_io.h"

class PropertiesWindow;

class Tensors : public Glyph
{
public:
    // Constructor/Destructor
    Tensors( );
    Tensors( const wxString &filename );
    virtual ~Tensors();

    std::vector< FMatrix > getTensorsMatrix()                       { return m_tensorsMatrix;           };
    std::vector< float   > getTensorsFA()                           { return m_tensorsFA;               };
    std::vector< F::FVector > getTensorsEV()                        { return m_tensorsEigenValues;      };
        
    void draw(); // From DatasetInfo

    bool load     ( nifti_image *pHeader, nifti_image *pBody );
    void normalize();
    virtual void createPropertiesSizer(PropertiesWindow *parent);
    virtual void updatePropertiesSizer();
    
private:
    wxToggleButton *m_pBtnNormalize;
    
    // From Glyph
    bool createStructure ( std::vector< float >& i_fileFloatData );
    void drawGlyph       ( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis = AXIS_UNDEFINED );
    void freeArrays      ( bool i_VBOActivated );
    void setScalingFactor( float i_scalingFactor );
     
    void setTensorInfo( FTensor i_FTensor);
    std::vector< FMatrix >  m_tensorsMatrix;    // All the tensors's matrix.
    std::vector< float   >  m_tensorsFA;        // All the tensors's FA values.
    std::vector< F::FVector >  m_tensorsEigenValues;// All the tensors's eigen values
    bool m_isNormalized;
    static const int VISUALIZATION_FACTOR = 600;
};

#endif /* TENSORS_H_ */
