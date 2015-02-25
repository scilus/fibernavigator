#ifndef CISOSURFACE_H
#define CISOSURFACE_H

// Based on code from: Raghavendra Chandrashekara, Paul Bourke and Cory Gene Bloyd

#include "CIsoSurfaceBase.h"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <map>
#include <vector>

typedef std::map<unsigned int, POINT3DID> ID2POINT3DID;

class Anatomy;

class CIsoSurface : public CIsoSurfaceBase
{
public:
    // Constructor and destructor.
    CIsoSurface( Anatomy* pAnatomy );
    virtual ~CIsoSurface() {};

    virtual bool load(wxString filename) {return false;};
    virtual void createPropertiesSizer(PropertiesWindow *parent);
    virtual void updatePropertiesSizer();
    void draw();
    void smooth();
    void flipAxis( AxisType i_axe ){};
    std::vector<Vector> getSurfaceVoxelPositions();

    void GenerateWithThreshold();

    // Generates the isosurface from the scalar field contained in the
    // buffer ptScalarField[].
    void GenerateSurface(float tIsoLevel);

    // Returns true if a valid surface has been generated.
    bool IsSurfaceValid();

    // Deletes the isosurface.
    void DeleteSurface();

    // Returns the length, width, and height of the volume in which the
    // isosurface in enclosed in.  Returns -1 if the surface is not
    // valid.
    int GetVolumeLengths(float& fVolLengthX, float& fVolLengthY, float& fVolLengthZ);

    bool save( wxString filename ) const;
    bool save( wxXmlNode *pNode, const wxString &rootPath ) const;

protected:
    // The number of vertices which make up the isosurface.
    unsigned int m_nVertices;

    // The number of triangles which make up the isosurface.
    unsigned int m_nTriangles;

    // The number of normals.
    unsigned int m_nNormals;

    // List of POINT3Ds which form the isosurface.
    ID2POINT3DID m_i2pt3idVertices;

    // List of TRIANGLES which form the triangulation of the isosurface.
    TRIANGLEVECTOR m_trivecTriangles;

    // Returns the edge ID.
    int GetEdgeID(unsigned int nX, unsigned int nY, unsigned int nZ, unsigned int nEdgeNo);

    // Returns the vertex ID.
    unsigned int GetVertexID(unsigned int nX, unsigned int nY, unsigned int nZ);

    // Calculates the intersection point of the isosurface with an
    // edge.
    POINT3DID CalculateIntersection(unsigned int nX, unsigned int nY, unsigned int nZ, unsigned int nEdgeNo);

    // Interpolates between two grid points to produce the point at which
    // the isosurface intersects an edge.
    POINT3DID Interpolate(float fX1, float fY1, float fZ1, float fX2, float fY2, float fZ2, float tVal1, float tVal2);

    // Renames vertices and triangles so that they can be accessed more
    // efficiently.
    void RenameVerticesAndTriangles();

    // No. of cells in x, y, and z directions.
    unsigned int m_nCellsX, m_nCellsY, m_nCellsZ;

    // Cell length in x, y, and z directions.
    float m_fCellLengthX, m_fCellLengthY, m_fCellLengthZ;

    // The buffer holding the scalar field.
    std::vector<float> m_ptScalarField;

    // The isosurface value.
    float m_tIsoLevel;

    // Indicates whether a valid surface is present.
    bool m_bValidSurface;

    // Lookup tables used in the construction of the isosurface.
    static const unsigned int m_edgeTable[256];
    static const int m_triTable[256][16];
    
    bool m_positionsCalculated;
    std::vector<Vector>m_svPositions;

private:
    GLuint getGLuint() {return 0;};
    void generateTexture() {};
    void generateGeometry();
    void initializeBuffer() {};

    wxToggleButton *m_pToggleCutFrontSector;
    wxToggleButton *m_pToggleUseColoring;
};
#endif // CISOSURFACE_H

