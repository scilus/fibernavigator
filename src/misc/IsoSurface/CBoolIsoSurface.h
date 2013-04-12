#ifndef CBOOLISOSURFACE_H
#define CBOOLISOSURFACE_H

// Based on code from: Raghavendra Chandrashekara, Paul Bourke and Cory Gene Bloyd

#include "CIsoSurfaceBase.h"

#include "../../dataset/DatasetInfo.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include <map>
#include <vector>

class CBoolIsoSurface  : public CIsoSurfaceBase {
public:
    // Constructor and destructor.
    CBoolIsoSurface(std::vector< bool > &voxelsState);
    virtual ~CBoolIsoSurface();

    virtual void createPropertiesSizer(PropertiesWindow *parent);
    virtual void updatePropertiesSizer();

    // Generates the isosurface from the boolean field.
    void GenerateSurface();

private:
    // Calculates the intersection point of the isosurface with an
    // edge.
    POINT3DID CalculateIntersection(unsigned int nX, unsigned int nY, unsigned int nZ, unsigned int nEdgeNo);
    
    // Interpolates between two grid points to produce the point at which
    // the isosurface intersects an edge.
    POINT3DID Interpolate(float fX1, float fY1, float fZ1, float fX2, float fY2, float fZ2, float tVal1, float tVal2);
    
    // TODO selection iso clean
    //GLuint getGLuint() {return 0;};
    //void generateTexture() {};
    //void generateGeometry();
    //void initializeBuffer() {};

    //wxToggleButton *m_ptoggleCutFrontSector;
    //wxToggleButton *m_ptoggleUseColoring;
    //wxBitmapButton *m_pbtnSelectColor;
    
    // The buffer holding the boolean field.
    std::vector< bool > m_ptBoolField;

};
#endif // CISOSURFACE_H

