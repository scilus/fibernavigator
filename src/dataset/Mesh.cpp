#include "Mesh.h"

#include "../main.h"
#include "../Logger.h"
#include "../gui/MainFrame.h"

#include <wx/datstrm.h>
#include <wx/wfstream.h>


Mesh::Mesh(DatasetHelper* dh) : DatasetInfo(dh)
{
    m_tMesh = new TriangleMesh(m_dh);
}

Mesh::~Mesh()
{
    delete m_tMesh;
}

bool Mesh::load( wxString i_filename )
{
    if( i_filename.AfterLast( '.' )      == _T( "mesh" ) )
        return loadMesh( i_filename );
    else if( i_filename.AfterLast( '.' ) == _T( "surf" ) )
        return loadSurf( i_filename );
    else if( i_filename.AfterLast( '.' ) == _T( "dip" ) )
        return loadDip( i_filename) ;

    return true;
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


    if( file.Open(filename) )
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
                    m_tMesh->addVert( x, m_dh->m_rows * m_dh->m_yVoxel - y, m_dh->m_frames * m_dh->m_zVoxel - z );
                }
            }
            if( line == _T( "Magnitudes" ) )
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

    m_fullPath = filename;
    #ifdef __WXMSW__
    m_name = filename.AfterLast( '\\' );
    #else
    m_name = filename.AfterLast( '/' );
    #endif
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

    for (unsigned int i = 0 ; i < m_countVerts ; ++i)
    {
        cbf.b[3] = buffer[pc++];
        cbf.b[2] = buffer[pc++];
        cbf.b[1] = buffer[pc++];
        cbf.b[0] = buffer[pc++];
        float x = cbf.f + 0.5  * m_dh->m_xVoxel + m_dh->m_columns/2 * m_dh->m_xVoxel;
        cbf.b[3] = buffer[pc++];
        cbf.b[2] = buffer[pc++];
        cbf.b[1] = buffer[pc++];
        cbf.b[0] = buffer[pc++];
        float y = cbf.f + 0.5 * m_dh->m_yVoxel + m_dh->m_rows/2  * m_dh->m_yVoxel;
        cbf.b[3] = buffer[pc++];
        cbf.b[2] = buffer[pc++];
        cbf.b[1] = buffer[pc++];
        cbf.b[0] = buffer[pc++];
        float z = cbf.f + 0.5 * m_dh->m_zVoxel + m_dh->m_frames/2  * m_dh->m_zVoxel;
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
            float y = m_dh->m_rows * m_dh->m_yVoxel - f.f;
            fp += 4;
            f.b[0] = buffer[fp];
            f.b[1] = buffer[fp+1];
            f.b[2] = buffer[fp+2];
            f.b[3] = buffer[fp+3];
            float z = m_dh->m_frames * m_dh->m_zVoxel - f.f;
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

void Mesh::activateLIC()
{

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

void Mesh::createPropertiesSizer(PropertiesWindow *parent)
{
    DatasetInfo::createPropertiesSizer(parent);

    m_ptoggleCutFrontSector = new wxToggleButton(parent, wxID_ANY,wxT("Cut Front Sector"),wxDefaultPosition, wxSize(140,-1));
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
    parent->Connect(m_pbtnSelectColor->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnAssignColor));
}

void Mesh::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();

    m_ptoggleFiltering->Enable(false);
    m_ptoggleFiltering->SetValue(false);
    m_ptoggleUseColoring->SetValue(!getUseTex());
    m_ptoggleCutFrontSector->SetValue(!getShowFS());
    m_psliderThresholdIntensity->SetValue(m_psliderThresholdIntensity->GetMin());
    m_psliderThresholdIntensity->Enable(false);
    
    // Disabled for the moment, not implemented.
    m_pBtnFlipX->Enable( false );
    m_pBtnFlipY->Enable( false );
    m_pBtnFlipZ->Enable( false );
}
