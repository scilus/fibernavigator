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
    virtual void createPropertiesSizer(MainFrame *parent);
    virtual void updatePropertiesSizer();

private:
    wxRadioButton      *m_pradiobtnAxes;
    // From Glyph
    bool createStructure ( vector< float >& i_fileFloatData );
    void drawGlyph       ( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis = AXIS_UNDEFINED );
    void freeArrays      ( bool i_VBOActivated );
    void setScalingFactor( float i_scalingFactor );
     
    void setTensorInfo( FTensor i_FTensor, vector< FMatrix > &o_matrixVector, vector< float > &o_FAVector );
    
    vector< float   >  m_tensorsFA;        // All the tensors's FA values.
    vector< FMatrix >  m_tensorsMatrix;    // All the tensors's matrix.    
};

#endif /* TENSORS_H_ */
