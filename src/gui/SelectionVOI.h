
#ifndef SELECTIONVOI_H_
#define SELECTIONVOI_H_

#include "SelectionObject.h"

#include "../dataset/DatasetIndex.h"
#include "../misc/Algorithms/Helper.h"

#include <vector>
using std::vector;

class Anatomy;
class CBoolIsoSurface;

class SelectionVOI : public SelectionObject
{
public:
    // Constructor / Destructor
    // Probably not needed.
    SelectionVOI( Anatomy *pSourceAnatomy, const float threshold, const ThresholdingOperationType opType );
    //SelectionVoi();
    virtual ~SelectionVOI();
    
    // Fonctions from SelectionObject (virtual pure)
    hitResult hitTest( Ray* i_ray );
    
    // Fonctions from SelectionObject (virtual)
    void objectUpdate() {};
    
    // Checks if a point is inside the VOI.
    bool isPointInside( const float xPos, const float yPos, const float zPos ) const;
    
    virtual void flipNormals();
    
    // Methods related to loading and saving.
    virtual wxString getTypeTag() const;
    bool populateXMLNode( wxXmlNode *pCurNode );
    
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
    
    // TODO compute on construction
    unsigned int m_voiSize;
    
    float        m_generationThreshold;
    
    CBoolIsoSurface *m_pIsoSurface;
    
    DatasetIndex m_sourceAnatIndex;
private:
    // GUI data
    wxTextCtrl *m_pVOISize;
};

#endif // SELECTIONVOI_H_