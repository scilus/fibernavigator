// TODO header syntax

#ifndef SELECTIONVOI_H_
#define SELECTIONVOI_H_

#include "SelectionObject.h"

#include "../dataset/Anatomy.h"
#include "../misc/Algorithms/Helper.h"

#include <vector>
using std::vector;

class CBoolIsoSurface;

class SelectionVOI : public SelectionObject
{
public:
    // Constructor / Destructor
    // Probably not needed.
    //SelectionVoi( Vector i_center, Vector i_size, DatasetHelper* i_datasetHelper );
    SelectionVOI( /*DatasetHelper *pDH, */Anatomy *pSourceAnatomy, const float threshold, const ThresholdingOperationType opType );
    //SelectionVoi();
    virtual ~SelectionVOI();
    
    // Fonctions from SelectionObject (virtual pure)
    hitResult hitTest( Ray* i_ray );
    
    // Fonctions from SelectionObject (virtual)
    void objectUpdate();
    
    // Checks if a point is inside the VOI.
    bool isPointInside( const float xPos, const float yPos, const float zPos ) const;
    
    // Methods related to loading and saving.
    virtual wxString getTypeTag() const;
    
    virtual void flipNormals();
    
    virtual void createPropertiesSizer( PropertiesWindow *pParent );
    virtual void updatePropertiesSizer();
    
private:
    // Fonction from SelectionObject (virtual pure)
    void drawObject( GLfloat* i_color );
    
    // Same size as the source anatomy, positions set to true are included in the VOI.
    vector< bool > m_includedVoxels;
    
    unsigned int m_nbRows;
    unsigned int m_nbCols;
    unsigned int m_nbFrames;
    
    unsigned int m_voiSize;
    
    CBoolIsoSurface *m_pIsoSurface;
    
private:
    // GUI data
    wxTextCtrl *m_pVOISize;
};

#endif // SELECTIONVOI_H_