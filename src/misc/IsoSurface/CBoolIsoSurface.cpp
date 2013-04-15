// File Name: CIsoSurface.cpp
// Last Modified: 5/8/2000
// Author: Raghavendra Chandrashekara (based on source code provided
// by Paul Bourke and Cory Gene Bloyd)
// Email: rc99@doc.ic.ac.uk, rchandrashekara@hotmail.com
//
// Description: This is the implementation file for the CIsoSurface class.

#include "CBoolIsoSurface.h"

#include "../../dataset/Anatomy.h"
#include "../../main.h"

#include <math.h>

#include <algorithm>
#include <ctime>
#include <fstream>
#include <vector>
using std::vector;

CBoolIsoSurface::CBoolIsoSurface( vector< bool > &voxelsState ) :
    CIsoSurfaceBase()
{
    m_ptBoolField = voxelsState;

    // TODO check
    //m_type = ISO_SURFACE;
}

CBoolIsoSurface::~CBoolIsoSurface()
{
}

void CBoolIsoSurface::GenerateSurface()
{
    if ( m_bValidSurface )
    {
        DeleteSurface();
    }

    unsigned int nPointsInXDirection = ( m_nCellsX + 1 );
    unsigned int nPointsInSlice = nPointsInXDirection * ( m_nCellsY + 1 );

#if 1
    // Generate isosurface.
    for ( unsigned int z = 0; z < m_nCellsZ; z++ )
    {
        for ( unsigned int y = 0; y < m_nCellsY; y++ )
        {
            for ( unsigned int x = 0; x < m_nCellsX; x++ )
            {
                // Calculate table lookup index from those
                // vertices which are below the isolevel.
                unsigned int tableIndex = 0;
                if ( m_ptBoolField[z * nPointsInSlice + y * nPointsInXDirection + x] == false )
                    tableIndex |= 1;
                if ( m_ptBoolField[z * nPointsInSlice + ( y + 1 ) * nPointsInXDirection + x] == false )
                    tableIndex |= 2;
                if ( m_ptBoolField[z * nPointsInSlice + ( y + 1 ) * nPointsInXDirection + ( x + 1 )]
                        == false )
                    tableIndex |= 4;
                if ( m_ptBoolField[z * nPointsInSlice + y * nPointsInXDirection + ( x + 1 )] == false )
                    tableIndex |= 8;
                if ( m_ptBoolField[( z + 1 ) * nPointsInSlice + y * nPointsInXDirection + x] == false )
                    tableIndex |= 16;
                if ( m_ptBoolField[( z + 1 ) * nPointsInSlice + ( y + 1 ) * nPointsInXDirection + x]
                        == false )
                    tableIndex |= 32;
                if ( m_ptBoolField[( z + 1 ) * nPointsInSlice + ( y + 1 ) * nPointsInXDirection + ( x + 1 )]
                        == false )
                    tableIndex |= 64;
                if ( m_ptBoolField[( z + 1 ) * nPointsInSlice + y * nPointsInXDirection + ( x + 1 )]
                        == false )
                    tableIndex |= 128;

                // Now create a triangulation of the isosurface in this
                // cell.
                if ( m_edgeTable[tableIndex] != 0 )
                {
                    if ( m_edgeTable[tableIndex] & 8 )
                    {
                        POINT3DID pt = CalculateIntersection( x, y, z, 3 );
                        unsigned int id = GetEdgeID( x, y, z, 3 );
                        m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                    }
                    if ( m_edgeTable[tableIndex] & 1 )
                    {
                        POINT3DID pt = CalculateIntersection( x, y, z, 0 );
                        unsigned int id = GetEdgeID( x, y, z, 0 );
                        m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                    }
                    if ( m_edgeTable[tableIndex] & 256 )
                    {
                        POINT3DID pt = CalculateIntersection( x, y, z, 8 );
                        unsigned int id = GetEdgeID( x, y, z, 8 );
                        m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                    }

                    if ( x == m_nCellsX - 1 )
                    {
                        if ( m_edgeTable[tableIndex] & 4 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 2 );
                            unsigned int id = GetEdgeID( x, y, z, 2 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }
                        if ( m_edgeTable[tableIndex] & 2048 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 11 );
                            unsigned int id = GetEdgeID( x, y, z, 11 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }
                    }
                    if ( y == m_nCellsY - 1 )
                    {
                        if ( m_edgeTable[tableIndex] & 2 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 1 );
                            unsigned int id = GetEdgeID( x, y, z, 1 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }
                        if ( m_edgeTable[tableIndex] & 512 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 9 );
                            unsigned int id = GetEdgeID( x, y, z, 9 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }
                    }
                    if ( z == m_nCellsZ - 1 )
                    {
                        if ( m_edgeTable[tableIndex] & 16 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 4 );
                            unsigned int id = GetEdgeID( x, y, z, 4 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }
                        if ( m_edgeTable[tableIndex] & 128 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 7 );
                            unsigned int id = GetEdgeID( x, y, z, 7 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }
                    }
                    if ( ( x == m_nCellsX - 1 ) && ( y == m_nCellsY - 1 ) )
                        if ( m_edgeTable[tableIndex] & 1024 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 10 );
                            unsigned int id = GetEdgeID( x, y, z, 10 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }
                    if ( ( x == m_nCellsX - 1 ) && ( z == m_nCellsZ - 1 ) )
                        if ( m_edgeTable[tableIndex] & 64 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 6 );
                            unsigned int id = GetEdgeID( x, y, z, 6 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }
                    if ( ( y == m_nCellsY - 1 ) && ( z == m_nCellsZ - 1 ) )
                        if ( m_edgeTable[tableIndex] & 32 )
                        {
                            POINT3DID pt = CalculateIntersection( x, y, z, 5 );
                            unsigned int id = GetEdgeID( x, y, z, 5 );
                            m_i2pt3idVertices.insert( ID2POINT3DID::value_type( id, pt ) );
                        }

                    for ( int i = 0; m_triTable[tableIndex][i] != -1; i += 3 )
                    {
                        TRIANGLE triangle;
                        unsigned int pointID0, pointID1, pointID2;
                        pointID0 = GetEdgeID( x, y, z, m_triTable[tableIndex][i] );
                        pointID1 = GetEdgeID( x, y, z, m_triTable[tableIndex][i + 1] );
                        pointID2 = GetEdgeID( x, y, z, m_triTable[tableIndex][i + 2] );
                        triangle.pointID[0] = pointID0;
                        triangle.pointID[1] = pointID1;
                        triangle.pointID[2] = pointID2;
                        m_trivecTriangles.push_back( triangle );
                    }
                }
            }
        }
    }
#else
    std::vector<int> aIndex(m_dh->m_columns * m_dh->m_rows * m_dh->m_frames ,0);

    // Generate isosurface.
    for (unsigned int z = 1; z < m_nCellsZ + 1; z++)
    {
        for (unsigned int y = 1; y < m_nCellsY + 1; y++)
        {
            for (unsigned int x = 1; x < m_nCellsX + 1; x++)
            {
                // Calculate table lookup index from those
                // vertices which are below the isolevel.
                if ( m_ptBoolField[z * nPointsInSlice + y * nPointsInXDirection + x] == false )
                {
                    aIndex[z * nPointsInSlice + y * nPointsInXDirection + x] |= 1;
                    aIndex[z * nPointsInSlice + (y - 1) * nPointsInXDirection + x] |= 2;
                    aIndex[z * nPointsInSlice + (y - 1) * nPointsInXDirection + (x - 1)] |= 4;
                    aIndex[z * nPointsInSlice + y * nPointsInXDirection + (x - 1)] |= 8;
                    aIndex[(z - 1) * nPointsInSlice + y * nPointsInXDirection + x] |= 16;
                    aIndex[(z - 1) * nPointsInSlice + (y - 1) * nPointsInXDirection + x] |= 32;
                    aIndex[(z - 1) * nPointsInSlice + (y - 1) * nPointsInXDirection + (x - 1)] |= 64;
                    aIndex[(z - 1) * nPointsInSlice + y * nPointsInXDirection + (x - 1)] |= 128;
                }
            }
        }
    }
    unsigned int tableIndex = 0;
    for (unsigned int z = 0; z < m_nCellsZ; z++)
    {
        for (unsigned int y = 0; y < m_nCellsY; y++)
        {
            for (unsigned int x = 0; x < m_nCellsX; x++)
            {
                tableIndex = aIndex[z * nPointsInSlice + y * nPointsInXDirection + x];
                // Now create a triangulation of the isosurface in this
                // cell.
                if (m_edgeTable[tableIndex] != 0)
                {
                    if (m_edgeTable[tableIndex] & 8)
                    {
                        POINT3DID pt = CalculateIntersection(x, y, z, 3);
                        unsigned int id = GetEdgeID(x, y, z, 3);
                        m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                    }
                    if (m_edgeTable[tableIndex] & 1)
                    {
                        POINT3DID pt = CalculateIntersection(x, y, z, 0);
                        unsigned int id = GetEdgeID(x, y, z, 0);
                        m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                    }
                    if (m_edgeTable[tableIndex] & 256)
                    {
                        POINT3DID pt = CalculateIntersection(x, y, z, 8);
                        unsigned int id = GetEdgeID(x, y, z, 8);
                        m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                    }

                    if (x == m_nCellsX - 1)
                    {
                        if (m_edgeTable[tableIndex] & 4)
                        {
                            POINT3DID pt = CalculateIntersection(x, y, z, 2);
                            unsigned int id = GetEdgeID(x, y, z, 2);
                            m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                        }
                        if (m_edgeTable[tableIndex] & 2048)
                        {
                            POINT3DID pt = CalculateIntersection(x, y, z, 11);
                            unsigned int id = GetEdgeID(x, y, z, 11);
                            m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                        }
                    }
                    if (y == m_nCellsY - 1)
                    {
                        if (m_edgeTable[tableIndex] & 2)
                        {
                            POINT3DID pt = CalculateIntersection(x, y, z, 1);
                            unsigned int id = GetEdgeID(x, y, z, 1);
                            m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                        }
                        if (m_edgeTable[tableIndex] & 512)
                        {
                            POINT3DID pt = CalculateIntersection(x, y, z, 9);
                            unsigned int id = GetEdgeID(x, y, z, 9);
                            m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                        }
                    }
                    if (z == m_nCellsZ - 1)
                    {
                        if (m_edgeTable[tableIndex] & 16)
                        {
                            POINT3DID pt = CalculateIntersection(x, y, z, 4);
                            unsigned int id = GetEdgeID(x, y, z, 4);
                            m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                        }
                        if (m_edgeTable[tableIndex] & 128)
                        {
                            POINT3DID pt = CalculateIntersection(x, y, z, 7);
                            unsigned int id = GetEdgeID(x, y, z, 7);
                            m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                        }
                    }
                    if ((x == m_nCellsX - 1) && (y == m_nCellsY - 1))
                    if (m_edgeTable[tableIndex] & 1024)
                    {
                        POINT3DID pt = CalculateIntersection(x, y, z, 10);
                        unsigned int id = GetEdgeID(x, y, z, 10);
                        m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                    }
                    if ((x == m_nCellsX - 1) && (z == m_nCellsZ - 1))
                    if (m_edgeTable[tableIndex] & 64)
                    {
                        POINT3DID pt = CalculateIntersection(x, y, z, 6);
                        unsigned int id = GetEdgeID(x, y, z, 6);
                        m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                    }
                    if ((y == m_nCellsY - 1) && (z == m_nCellsZ - 1))
                    if (m_edgeTable[tableIndex] & 32)
                    {
                        POINT3DID pt = CalculateIntersection(x, y, z, 5);
                        unsigned int id = GetEdgeID(x, y, z, 5);
                        m_i2pt3idVertices.insert(ID2POINT3DID::value_type(id, pt));
                    }

                    for (int i = 0; m_triTable[tableIndex][i] != -1; i += 3)
                    {
                        TRIANGLE triangle;
                        unsigned int pointID0, pointID1, pointID2;
                        pointID0 = GetEdgeID(x, y, z, m_triTable[tableIndex][i]);
                        pointID1 = GetEdgeID(x, y, z, m_triTable[tableIndex][i + 1]);
                        pointID2 = GetEdgeID(x, y, z, m_triTable[tableIndex][i + 2]);
                        triangle.pointID[0] = pointID0;
                        triangle.pointID[1] = pointID1;
                        triangle.pointID[2] = pointID2;
                        m_trivecTriangles.push_back(triangle);
                    }
                }
            }
        }
    }
#endif

    RenameVerticesAndTriangles();
    m_bValidSurface = true;
}

POINT3DID CBoolIsoSurface::CalculateIntersection( unsigned int nX, unsigned int nY, unsigned int nZ,
        unsigned int nEdgeNo )
{
    float x1, y1, z1, x2, y2, z2;
    unsigned int v1x = nX, v1y = nY, v1z = nZ;
    unsigned int v2x = nX, v2y = nY, v2z = nZ;

    switch ( nEdgeNo )
    {
        case 0:
            v2y += 1;
            break;
        case 1:
            v1y += 1;
            v2x += 1;
            v2y += 1;
            break;
        case 2:
            v1x += 1;
            v1y += 1;
            v2x += 1;
            break;
        case 3:
            v1x += 1;
            break;
        case 4:
            v1z += 1;
            v2y += 1;
            v2z += 1;
            break;
        case 5:
            v1y += 1;
            v1z += 1;
            v2x += 1;
            v2y += 1;
            v2z += 1;
            break;
        case 6:
            v1x += 1;
            v1y += 1;
            v1z += 1;
            v2x += 1;
            v2z += 1;
            break;
        case 7:
            v1x += 1;
            v1z += 1;
            v2z += 1;
            break;
        case 8:
            v2z += 1;
            break;
        case 9:
            v1y += 1;
            v2y += 1;
            v2z += 1;
            break;
        case 10:
            v1x += 1;
            v1y += 1;
            v2x += 1;
            v2y += 1;
            v2z += 1;
            break;
        case 11:
            v1x += 1;
            v2x += 1;
            v2z += 1;
            break;
    }

    x1 = v1x * m_fCellLengthX;
    y1 = v1y * m_fCellLengthY;
    z1 = v1z * m_fCellLengthZ;
    x2 = v2x * m_fCellLengthX;
    y2 = v2y * m_fCellLengthY;
    z2 = v2z * m_fCellLengthZ;

    unsigned int nPointsInXDirection = ( m_nCellsX + 1 );
    unsigned int nPointsInSlice = nPointsInXDirection * ( m_nCellsY + 1 );
    
    float val1 = m_ptBoolField[v1z * nPointsInSlice + v1y * nPointsInXDirection + v1x];
    float val2 = m_ptBoolField[v2z * nPointsInSlice + v2y * nPointsInXDirection + v2x];
    
    POINT3DID intersection = Interpolate( x1, y1, z1, x2, y2, z2, val1, val2 );
    intersection.newID = 0;
    return intersection;
}

POINT3DID CBoolIsoSurface::Interpolate( float fX1, float fY1, float fZ1, float fX2, float fY2, float fZ2,
        float tVal1, float tVal2 )
{
    POINT3DID interpolation;
    float mu;

    mu = float( ( 0.5 - tVal1 ) ) / ( tVal2 - tVal1 );
    interpolation.x = fX1 + mu * ( fX2 - fX1 );
    interpolation.y = fY1 + mu * ( fY2 - fY1 );
    interpolation.z = fZ1 + mu * ( fZ2 - fZ1 );
    interpolation.newID = 0;
    return interpolation;
}

void CBoolIsoSurface::createPropertiesSizer(PropertiesWindow *parent)
{
    DatasetInfo::createPropertiesSizer(parent);

    /*m_ptoggleCutFrontSector = new wxToggleButton(parent, wxID_ANY,wxT("Cut Front Sector"),wxDefaultPosition, wxSize(140,-1));
    m_propertiesSizer->Add(m_ptoggleCutFrontSector,0,wxALIGN_CENTER);
    parent->Connect(m_ptoggleCutFrontSector->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(PropertiesWindow::OnToggleShowFS));  
    
    wxSizer *l_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_ptoggleUseColoring = new wxToggleButton(parent, wxID_ANY,wxT("Use Coloring"),wxDefaultPosition, wxSize(100,-1));
    wxImage bmpColor(MyApp::iconsPath+ wxT("colorSelect.png" ), wxBITMAP_TYPE_PNG);
    m_pbtnSelectColor = new wxBitmapButton(parent, wxID_ANY, bmpColor, wxDefaultPosition, wxSize(40,-1));
    l_sizer->Add(m_ptoggleUseColoring,0,wxALIGN_CENTER);
    l_sizer->Add(m_pbtnSelectColor,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_ptoggleUseColoring->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnListMenuThreshold));
    parent->Connect(m_pbtnSelectColor->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnAssignColor));*/
}

void CBoolIsoSurface::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();
    /*m_ptoggleFiltering->Enable(false);
    m_ptoggleFiltering->SetValue(false);
    m_ptoggleUseColoring->SetValue(!getUseTex());
    m_ptoggleCutFrontSector->SetValue(!getShowFS());

    // Disabled for the moment, not implemented.
    m_pBtnFlipX->Enable( false );
    m_pBtnFlipY->Enable( false );
    m_pBtnFlipZ->Enable( false );*/
}
