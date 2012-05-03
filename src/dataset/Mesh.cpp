#include "Mesh.h"

#include "DatasetManager.h"
#include "../main.h"
#include "../Logger.h"
#include "../gui/MainFrame.h"

#include <wx/datstrm.h>
#include <wx/textfile.h>
#include <wx/tglbtn.h>
#include <wx/wfstream.h>

Mesh::Mesh()
:   DatasetInfo()
{
    m_tMesh = new TriangleMesh();
}

Mesh::Mesh( const wxString &filename )
:   DatasetInfo()
{
    m_tMesh = new TriangleMesh();

    m_fullPath = filename;

#ifdef __WXMSW__
    m_name = filename.AfterLast( '\\' );
#else
    m_name = filename.AfterLast( '/' );
#endif
}

Mesh::~Mesh()
{
    delete m_tMesh;
}

bool Mesh::loadDip( wxString filename )
{
    Logger::getInstance()->print( wxT( "Loading DIP mesh file" ), LOGLEVEL_MESSAGE );
    wxTextFile file;
    wxString line;
    wxString numberString, xString, yString, zString;
    double x,y,z;
    long tmpVal, v1, v2, v3;
    float minMagnitude = 100000;
    float maxMagnitude = 0;

    float rows   = DatasetManager::getInstance()->getRows();
    float frames = DatasetManager::getInstance()->getFrames();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();
    
    if( file.Open( filename ) )
    {
        line = file.GetFirstLine();
        while( ! file.Eof() )
        {
            line = file.GetNextLine();
            if( line.BeforeFirst( '=' ) == _T("NumberPositions" ) )
            {
                 numberString = line.AfterLast('=');
                 numberString.ToLong(&tmpVal, 10);
                 setCountVerts((int)tmpVal);
            }
            if( line == _T( "PositionsFixed" ) )
            {
                for( size_t i = 0; i < m_countVerts; ++i )
                {
                    line = file.GetNextLine();
                    xString = line.BeforeFirst( ' ' );
                    yString = line.AfterFirst( ' ' ).BeforeLast( ' ' );
                    zString = line.AfterLast( ' ' );
                    xString.ToDouble( &x );
                    yString.ToDouble( &y );
                    zString.ToDouble( &z );
                    m_tMesh->addVert(   x, rows * voxelY - y, frames * voxelZ - z );
                }
            }
            else if( line == _T( "Magnitudes" ) )
            {
                std::vector<float>tmpMagnitudes( m_countVerts, 0 );
                for( size_t i = 0 ; i < m_countVerts ; ++i )
                {
                    line = file.GetNextLine();
                    line.ToDouble(&x);
                    if( x < minMagnitude ) minMagnitude = x;
                    if( x > maxMagnitude ) maxMagnitude = x;

                    tmpMagnitudes[i] = x;
                }

                float diff = maxMagnitude - minMagnitude;
                for( size_t i = 0 ; i < m_countVerts ; ++i )
                {
                    float c = ( ( tmpMagnitudes[i] - minMagnitude ) / diff ) * 255.;
                    wxColour color( c, c, c );
                    m_tMesh->setVertColor( i, color );
                }

            }
            if( line.BeforeFirst( '=' ) == _T( "NumberPolygons" ) )
            {
                 numberString = line.AfterLast('=');
                 numberString.ToLong(&tmpVal, 10);
                 setCountPolygons((int)tmpVal);
            }
            if( line == _T( "Polygons" ) )
            {
                for( size_t i = 0 ; i < m_countPolygons ; ++i )
                {
                    line = file.GetNextLine();
                    xString = line.BeforeFirst( ' ' );
                    yString = line.AfterFirst( ' ' ).BeforeLast( ' ' );
                    zString = line.AfterLast( ' ' );
                    xString.ToLong( &v1, 10 );
                    yString.ToLong( &v2, 10 );
                    zString.ToLong( &v3, 10 );
                    m_tMesh->addTriangle( v1, v2, v3 );
                }
            }
        }
    }

    m_type = MESH;
    m_isGlyph = true;

    return true;
}

bool Mesh::loadSurf(wxString filename)
{
    Logger::getInstance()->print( wxT( "Loading freesurfer mesh file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize = 0;
    int pc = 3;
    converterByteINT32 cbi;
    converterByteFloat cbf;

    if (dataFile.Open(filename))
    {
        nSize = dataFile.Length();
        if (nSize == wxInvalidOffset) return false;
    }

    wxUint8* buffer = new wxUint8[nSize];
    dataFile.Read(buffer, nSize);
    dataFile.Close();

    // find double \n (0x0a)
    while (pc < nSize)
    {
        if (buffer[pc++] == 0x0a)
        {
            if (buffer[pc++] == 0x0a)
                break;
        }
    }

    cbi.b[3] = buffer[pc++];
    cbi.b[2] = buffer[pc++];
    cbi.b[1] = buffer[pc++];
    cbi.b[0] = buffer[pc++];
    setCountVerts(cbi.i);
    cbi.b[3] = buffer[pc++];
    cbi.b[2] = buffer[pc++];
    cbi.b[1] = buffer[pc++];
    cbi.b[0] = buffer[pc++];
    setCountPolygons(cbi.i);

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for (unsigned int i = 0 ; i < m_countVerts ; ++i)
    {
        cbf.b[3] = buffer[pc++];
        cbf.b[2] = buffer[pc++];
        cbf.b[1] = buffer[pc++];
        cbf.b[0] = buffer[pc++];
        float x = cbf.f + 0.5f * voxelX + columns * 0.5f * voxelX;
        cbf.b[3] = buffer[pc++];
        cbf.b[2] = buffer[pc++];
        cbf.b[1] = buffer[pc++];
        cbf.b[0] = buffer[pc++];
        float y = cbf.f + 0.5f * voxelY + rows * 0.5f * voxelY;
        cbf.b[3] = buffer[pc++];
        cbf.b[2] = buffer[pc++];
        cbf.b[1] = buffer[pc++];
        cbf.b[0] = buffer[pc++];
        float z = cbf.f + 0.5f * voxelZ + frames * 0.5f * voxelZ;
        m_tMesh->addVert(x, y, z);
    }

    for (unsigned int i = 0 ; i < getCountPolygons() ; ++i)
    {
        cbi.b[3] = buffer[pc++];
        cbi.b[2] = buffer[pc++];
        cbi.b[1] = buffer[pc++];
        cbi.b[0] = buffer[pc++];
        int v1 = cbi.i;
        cbi.b[3] = buffer[pc++];
        cbi.b[2] = buffer[pc++];
        cbi.b[1] = buffer[pc++];
        cbi.b[0] = buffer[pc++];
        int v2 = cbi.i;
        cbi.b[3] = buffer[pc++];
        cbi.b[2] = buffer[pc++];
        cbi.b[1] = buffer[pc++];
        cbi.b[0] = buffer[pc++];
        int v3 = cbi.i;

        m_tMesh->addTriangle(v1, v2, v3);
    }

    m_fullPath = filename;
    #ifdef __WXMSW__
        m_name = filename.AfterLast('\\');
    #else
    m_name = filename.AfterLast('/');
    #endif
    m_type = MESH;


    return true;
}

bool Mesh::loadMesh( wxString filename )
{
    wxFile dataFile;
    wxFileOffset nSize = 0;

    if( dataFile.Open( filename ) )
    {
         nSize = dataFile.Length();
        if( nSize == wxInvalidOffset )
            return false;
    }

    wxUint8* buffer = new wxUint8[nSize];
    if (dataFile.Read(buffer, (size_t) nSize) != nSize)
    {
        dataFile.Close();
        delete[] buffer;
        return false;
    }
    if (buffer[0] == 'a')
    {
        setFiletype(ascii);
        // ascii file, maybe later
        return false;
    }
    if (buffer [0] == 'b')
    {
        // maybe binary file
        char* filetype = new char[10];
        for (int i = 0; i < 9; ++i)
            filetype[i] = buffer[i];
        filetype[9] = 0;

        wxString type(filetype, wxConvUTF8);
        if (type == wxT("binarABCD")) {
            setFiletype(binaryBE);
            //big endian, maybe later
            return false;
        }
        else if (type == wxT("binarDCBA")) {
            setFiletype(binaryLE);
        }
        else return false;

        setPolygonDim(buffer[17]);

        int fp = 29;
        converterByteINT32 c;
        converterByteFloat f;

        c.b[0] = buffer[fp];
        c.b[1] = buffer[fp+1];
        c.b[2] = buffer[fp+2];
        c.b[3] = buffer[fp+3];
        // number of vertices
        setCountVerts(c.i);

        float rows   = DatasetManager::getInstance()->getRows();
        float frames = DatasetManager::getInstance()->getFrames();
        float voxelY = DatasetManager::getInstance()->getVoxelY();
        float voxelZ = DatasetManager::getInstance()->getVoxelZ();

        fp += 4;
        for (unsigned int i = 0 ; i < c.i ; ++i)
        {
            f.b[0] = buffer[fp];
            f.b[1] = buffer[fp+1];
            f.b[2] = buffer[fp+2];
            f.b[3] = buffer[fp+3];
            float x = f.f;
            fp += 4;
            f.b[0] = buffer[fp];
            f.b[1] = buffer[fp+1];
            f.b[2] = buffer[fp+2];
            f.b[3] = buffer[fp+3];
            float y = rows * voxelY - f.f;
            fp += 4;
            f.b[0] = buffer[fp];
            f.b[1] = buffer[fp+1];
            f.b[2] = buffer[fp+2];
            f.b[3] = buffer[fp+3];
            float z = frames * voxelZ - f.f;
            fp += 4;
            m_tMesh->addVert(x, y, z);
        }

        c.b[0] = buffer[fp];
        c.b[1] = buffer[fp+1];
        c.b[2] = buffer[fp+2];
        c.b[3] = buffer[fp+3];

        setCountNormals(c.i);

        fp += 8 + 12 * c.i;

        c.b[0] = buffer[fp];
        c.b[1] = buffer[fp+1];
        c.b[2] = buffer[fp+2];
        c.b[3] = buffer[fp+3];
        setCountPolygons(c.i);

        fp += 4;
        for (unsigned int i = 0 ; i < getCountPolygons() ; ++i)
        {
            c.b[0] = buffer[fp];
            c.b[1] = buffer[fp+1];
            c.b[2] = buffer[fp+2];
            c.b[3] = buffer[fp+3];
            int v1 = c.i;
            fp += 4;
            c.b[0] = buffer[fp];
            c.b[1] = buffer[fp+1];
            c.b[2] = buffer[fp+2];
            c.b[3] = buffer[fp+3];
            int v2 = c.i;
            fp += 4;
            c.b[0] = buffer[fp];
            c.b[1] = buffer[fp+1];
            c.b[2] = buffer[fp+2];
            c.b[3] = buffer[fp+3];
            int v3 = c.i;
            fp += 4;
            m_tMesh->addTriangle(v1, v2, v3);
        }
    }

    m_fullPath = filename;
#ifdef __WXMSW__
    m_name = filename.AfterLast('\\');
#else
    m_name = filename.AfterLast('/');
#endif
    m_type = MESH;

    return true;
}

void Mesh::generateGeometry()
{
    if( m_GLuint )
        glDeleteLists( m_GLuint, 1 );

    GLuint dl = glGenLists( 1 );
    glNewList( dl, GL_COMPILE );

    Triangle triangleEdges;
    Vector   point;
    Vector   pointNormal;
    wxColour color;

    if( ! m_isGlyph)
    {
        glBegin( GL_TRIANGLES );
        for( int i = 0; i < m_tMesh->getNumTriangles(); ++i )
        {
            triangleEdges = m_tMesh->getTriangle( i );
            for( int j = 0; j < 3; ++j )
            {
                pointNormal = m_tMesh->getVertNormal( triangleEdges.pointID[j] );
                glNormal3d( pointNormal.x, pointNormal.y, pointNormal.z );

                point = m_tMesh->getVertex( triangleEdges.pointID[j] );
                glVertex3d( point.x, point.y, point.z );
            }
        }
        glEnd();
    }
    else
    {
        glBegin( GL_TRIANGLES );
        for( int i = 0; i < m_tMesh->getNumTriangles(); ++i )
        {
            triangleEdges = m_tMesh->getTriangle( i );
            for( int j = 0; j < 3; ++j )
            {
                pointNormal = m_tMesh->getVertNormal( triangleEdges.pointID[j] );
                glNormal3d( pointNormal.x, pointNormal.y, pointNormal.z );

                color = m_tMesh->getVertColor( triangleEdges.pointID[j] );
                glColor3f( color.Red() / 255., color.Green() / 255., color.Blue() / 255. );

                point = m_tMesh->getVertex( triangleEdges.pointID[j] );
                glVertex3d( point.x, point.y, point.z );
            }
        }
        glEnd();
    }

    glEndList();
    m_GLuint = dl;

}

GLuint Mesh::getGLuint()
{
    if (!m_GLuint)
        generateGeometry();
    return m_GLuint;
}

void Mesh::draw()
{
    if (!m_GLuint)
        generateGeometry();
    glCallList(m_GLuint);
}

void Mesh::createPropertiesSizer( PropertiesWindow *pParent )
{
    DatasetInfo::createPropertiesSizer( pParent );

    wxBoxSizer *pBoxMain = new wxBoxSizer( wxVERTICAL );

    wxImage bmpColor( MyApp::iconsPath + wxT( "colorSelect.png" ), wxBITMAP_TYPE_PNG );

    //////////////////////////////////////////////////////////////////////////

    m_pToggleCutFrontSector = new wxToggleButton( pParent, wxID_ANY, wxT( "Cut Front Sector" ) );
    m_pToggleUseColoring    = new wxToggleButton( pParent, wxID_ANY, wxT( "Use Coloring" ) );
    m_pBtnSelectColor       = new wxBitmapButton( pParent, wxID_ANY, bmpColor );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( m_pToggleCutFrontSector, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    wxBoxSizer *pBoxColoring = new wxBoxSizer( wxHORIZONTAL );
    pBoxColoring->Add( m_pToggleUseColoring, 3, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxColoring->Add( m_pBtnSelectColor,    1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxMain->Add( pBoxColoring, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    m_pPropertiesSizer->Add( pBoxMain, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    pParent->Connect( m_pToggleCutFrontSector->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(        PropertiesWindow::OnToggleShowFS ) );
    pParent->Connect( m_pToggleUseColoring->GetId(),    wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnToggleUseTex ) );
    pParent->Connect( m_pBtnSelectColor->GetId(),       wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnAssignColor ) );
}

void Mesh::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();

    m_pToggleFiltering->Enable(false);
    m_pToggleFiltering->SetValue(false);
    m_pToggleUseColoring->SetValue(!getUseTex());
    m_pToggleCutFrontSector->SetValue(!getShowFS());
    m_pSliderThresholdIntensity->SetValue(m_pSliderThresholdIntensity->GetMin());
    m_pSliderThresholdIntensity->Enable(false);
    
    // Disabled for the moment, not implemented.
    m_pBtnFlipX->Enable( false );
    m_pBtnFlipY->Enable( false );
    m_pBtnFlipZ->Enable( false );
}
