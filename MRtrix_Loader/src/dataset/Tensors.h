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

#include "datasetInfo.h"
#include "Glyph.h"
#include "../misc/lic/TensorField.h"
#include "../misc/Fantom/FVector.h"

using namespace std;

class Tensors : public Glyph
{

public:
    // Constructor/Destructor
    Tensors( DatasetHelper* i_datasetHelper );
    virtual ~Tensors();
        
    void draw(); // From DatasetInfo

    bool loadNifti( wxString i_fileName );
    bool load     ( wxString i_fileName );
    void normalize();
    virtual void createPropertiesSizer(PropertiesWindow *parent);
    virtual void updatePropertiesSizer();
    
private:
    wxRadioButton      *m_pradiobtnAxes;
    wxToggleButton       *m_pbtnNormalize;
    // From Glyph
    bool createStructure ( vector< float >& i_fileFloatData );
    void drawGlyph       ( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis = AXIS_UNDEFINED );
    void freeArrays      ( bool i_VBOActivated );
    void setScalingFactor( float i_scalingFactor );
     
    void setTensorInfo( FTensor i_FTensor);
    
    vector< float   >  m_tensorsFA;        // All the tensors's FA values.
    vector< FMatrix >  m_tensorsMatrix;    // All the tensors's matrix.  
    vector< F::FVector >  m_tensorsEigenValues;// All the tensors's eigen values
    bool m_isNormalized;
    static const int VISUALIZATION_FACTOR = 600;
};

#endif /* TENSORS_H_ */
