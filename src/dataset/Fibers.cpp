/*

 *  The Fibers class implementation.
 *
 */

#include "Fibers.h"

#include "Anatomy.h"
#include "DatasetManager.h"
#include "RTTrackingHelper.h"

#include "../main.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gui/MainFrame.h"
#include "../gui/SceneManager.h"
#include "../gui/SelectionTree.h"
#include "../misc/Fantom/FMatrix.h"

#include <wx/file.h>
#include <wx/tglbtn.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <fstream>
using std::ofstream;

#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <vector>
using std::vector;

#define DEF_POS   wxDefaultPosition
#define DEF_SIZE  wxDefaultSize

// TODO replace by const
#define LINEAR_GRADIENT_THRESHOLD 0.085f
#define MIN_ALPHA_VALUE 0.017f

Fibers::Fibers()
:   DatasetInfo(),
    m_isSpecialFiberDisplay( false ),
    m_barycenter(),
    m_boxMax(),
    m_boxMin(),
    m_colorArray(),
    m_count( 0 ),
    m_countLines( 0 ),
    m_countPoints( 0 ),
    m_isInitialized( false ),
    m_lineArray(),
    m_linePointers(),
    m_pointArray(),
    m_normalArray(),
    m_normalsPositive( false ),
    m_reverse(),
    m_selected(),
    m_filtered(),
    m_length(),
    m_subsampledLines( 0 ),
    m_maxLength( 0.0f ),
    m_minLength( 0.0f ),
    m_localizedAlpha(),
    m_cachedThreshold( 0.0f ),
    m_fibersInverted( false ),
    m_useFakeTubes( false ),
    m_useTransparency( false ),
    m_isColorationUpdated( false ),
    m_fiberColorationMode( NORMAL_COLOR ),
    m_pOctree( NULL ),
    m_cfDrawDirty( true ),
    m_axialShown(    SceneManager::getInstance()->isAxialDisplayed() ),
    m_coronalShown(  SceneManager::getInstance()->isCoronalDisplayed() ),
    m_sagittalShown( SceneManager::getInstance()->isSagittalDisplayed() ),
    m_useIntersectedFibers( false ),
    m_thickness( 2.5f ),
    m_tubeRadius( 3.175f ),
    m_xDrawn( 0.0f ),
    m_yDrawn( 0.0f ),
    m_zDrawn( 0.0f ),
    m_cfStartOfLine(),
    m_cfPointsPerLine(),
    m_constantColor( 0, 0, 0 ),
    m_pSliderFibersFilterMin( NULL ),
    m_pSliderFibersFilterMax( NULL ),
    m_pSliderFibersSampling( NULL ),
    m_pSliderInterFibersThickness( NULL ),
    m_pTubeRadius( NULL ),
    m_pToggleLocalColoring( NULL ),
    m_pToggleNormalColoring( NULL ),
    m_pSelectConstantFibersColor( NULL ),
    m_pToggleCrossingFibers( NULL ),
    m_pRadNormalColoring( NULL ),
    m_pRadDistanceAnchoring( NULL ),
    m_pRadMinDistanceAnchoring( NULL ),
    m_pRadCurvature( NULL ),
    m_pRadTorsion( NULL ),
    m_pRadConstant( NULL ),
    m_exponent( 3.0f ),
 	m_xAngle( 0.0f ),
 	m_yAngle( 0.0f ),
    m_zAngle( 1.0f ),
    m_lina(1.5f),
    m_linb(-0.9f),
    m_cl(0.0f),
	m_axisView( true ),
	m_ModeOpac( true ),
    m_isAlphaFunc( true ),
    m_isLocalRendering( true ),
    m_usingEndpts( false ),
    m_pToggleAxisView( NULL ),
	m_pToggleModeOpac( NULL ),
    m_pToggleRenderFunc( NULL ),
    m_pToggleLocalGlobal( NULL ),
    m_pToggleEndpts( NULL )
{
    m_bufferObjects = new GLuint[3];
}

Fibers::~Fibers()
{
    Logger::getInstance()->print( wxT( "Executing fibers destructor" ), LOGLEVEL_DEBUG );
    
    SceneManager::getInstance()->getSelectionTree().removeFiberDataset( getDatasetIndex() );

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glDeleteBuffers( 3, m_bufferObjects );
    }

    if( m_pOctree )
    {
        delete m_pOctree;
        m_pOctree = NULL;
    }

    m_lineArray.clear();
    m_linePointers.clear();
    m_reverse.clear();
    m_pointArray.clear();
    m_normalArray.clear();
    m_colorArray.clear();
}

bool Fibers::load( const wxString &filename )
{
    bool res( false );

    wxString extension = filename.AfterLast( '.' );
    if( wxT( "fib" ) == extension )
    {
        if( loadVTK( filename ) )
        {
            res = true;
        }
        else
        {
            res = loadDmri( filename );
        }
    }
    else if( wxT( "bundlesdata" ) == extension )
    {
        res = loadPTK( filename );
    }
    else if( wxT( "Bfloat" ) == extension )
    {
        res = loadCamino( filename );
    }
    else if( wxT( "trk" ) == extension )
    {
        res = loadTRK( filename );
    }
    else if( wxT( "tck" ) == extension )
    {
        res = loadMRtrix( filename );
    }

    /* OcTree points classification */
    m_pOctree = new Octree( 2, m_pointArray, m_countPoints );

    //Global properties for opacity rendering
    computeGLobalProperties();

    return res;
}

bool Fibers::loadTRK( const wxString &filename )
{
    stringstream ss;
    Logger::getInstance()->print( wxT( "Loading TRK file..." ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize( 0 );
    converterByteINT16 cbi;
    converterByteINT32 cbi32;
    converterByteFloat cbf;

    if( !dataFile.Open( filename ) )
    {
        return false;
    }

    nSize = dataFile.Length();

    if( nSize == wxInvalidOffset )
    {
        return false;
    }

    ////
    // READ HEADER
    ////
    //Read file header. [1000 bytes]
    wxUint8 *pBuffer = new wxUint8[1000];
    dataFile.Read( pBuffer, ( size_t )1000 );

    //ID String for track file. The first 5 characters must match "TRACK". [6 bytes]
    char idString[6];
    memcpy( idString, &pBuffer[0], 6 );
    ss.str( "" );
    ss << "Type: " << idString;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    if( strncmp( idString, "TRACK", 5 ) != 0 )
    {
        return false;
    }

    //Dimension of the image volume. [6 bytes]
    wxUint16 dim[3];

    for( int i = 0; i != 3; ++i )
    {
        memcpy( cbi.b, &pBuffer[6 + ( i * 2 )], 2 );
        dim[i] = cbi.i;
    }

    ss.str( "" );
    ss << "Dim: " << dim[0] << "x" << dim[1] << "x" << dim[2];
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Voxel size of the image volume. [12 bytes]
    float voxelSize[3];

    for( int i = 0; i != 3; ++i )
    {
        memcpy( cbf.b, &pBuffer[12 + ( i * 4 )], 4 );
        voxelSize[i] = cbf.f;
    }

    ss.str( "" );
    ss << "Voxel size: " << voxelSize[0] << "x" << voxelSize[1] << "x" << voxelSize[2];
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Origin of the image volume. [12 bytes]
    float origin[3];

    for( int i = 0; i != 3; ++i )
    {
        memcpy( cbf.b, &pBuffer[24 + ( i * 4 )], 4 );
        origin[i] = cbf.f;
    }

    ss.str( "" );
    ss << "Origin: (" << origin[0] << "," << origin[1] << "," << origin[2] << ")";
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Number of scalars saved at each track point. [2 bytes]
    wxUint16 nbScalars;
    memcpy( cbi.b, &pBuffer[36], 2 );
    nbScalars = cbi.i;
    ss.str( "" );
    ss << "Nb. scalars: " << nbScalars;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Name of each scalar. (20 characters max each, max 10 names) [200 bytes]
    char scalarNames[10][20];
    memcpy( scalarNames, &pBuffer[38], 200 );

    for( int i = 0; i != 10; ++i )
    {
        ss.str( "" );
        ss << "Scalar name #" << i << ": " << scalarNames[i];
        Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    }

    //Number of properties saved at each track. [2 bytes]
    wxUint16 nbProperties;
    memcpy( cbi.b, &pBuffer[238], 2 );
    nbProperties = cbi.i;
    ss.str( "" );
    ss << "Nb. properties: " << nbProperties;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Name of each property. (20 characters max each, max 10 names) [200 bytes]
    char propertyNames[10][20];
    memcpy( propertyNames, &pBuffer[240], 200 );

    for( int i = 0; i != 10; ++i )
    {
        ss.str( "" );
        ss << "Property name #" << i << ": " << propertyNames[i];
    }

    //4x4 matrix for voxel to RAS (crs to xyz) transformation.
    // If vox_to_ras[3][3] is 0, it means the matrix is not recorded.
    // This field is added from version 2. [64 bytes]
    float voxToRas[4][4];

    for( int i = 0; i != 4; ++i )
    {
        ss.str( "" );

        for( int j = 0; j != 4; ++j )
        {
            memcpy( cbf.b, &pBuffer[440 + ( i * 4 + j )], 4 );
            voxToRas[i][j] = cbf.f;
            ss << voxToRas[i][j] << " ";
        }

        Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    }

    //Reserved space for future version. [444 bytes]
    //char reserved[444];
    //pBuffer[504]...
    //Storing order of the original image data. [4 bytes]
    char voxelOrder[4];
    memcpy( voxelOrder, &pBuffer[948], 4 );
    ss.str( "" );
    ss << "Voxel order: " << voxelOrder;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Paddings [4 bytes]
    char pad2[4];
    memcpy( pad2, &pBuffer[952], 4 );
    ss.str( "" );
    ss << "Pad #2: " << pad2;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Image orientation of the original image. As defined in the DICOM header. [24 bytes]
    float imageOrientationPatient[6];
    ss.str( "" );
    ss << "Image orientation patient: ";

    for( int i = 0; i != 6; ++i )
    {
        memcpy( cbf.b, &pBuffer[956 + ( i * 4 )], 4 );
        imageOrientationPatient[i] = cbf.f;
        ss << imageOrientationPatient[i] << " ";
    }

    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Paddings. [2 bytes]
    char pad1[2];
    memcpy( pad1, &pBuffer[980], 2 );
    ss.str( "" );
    ss << "Pad #1: " << pad1;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invertX = pBuffer[982] > 0;
    ss.str( "" );
    ss << "Invert X: " << invertX;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invertY = pBuffer[983] > 0;
    ss.str( "" );
    ss << "Invert Y: " << invertY;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invertZ = pBuffer[984] > 0;
    ss.str( "" );
    ss << "Invert Z: " << invertZ;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swapXY = pBuffer[985] > 0;
    ss.str( "" );
    ss << "Swap XY: " << swapXY;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swapYZ = pBuffer[986] > 0;
    ss.str( "" );
    ss << "Swap YZ: " << swapYZ;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swapZX = pBuffer[987] > 0;
    ss.str( "" );
    ss << "Swap ZX: " << swapZX;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Number of tracks stored in this track file. 0 means the number was NOT stored. [4 bytes]
    wxUint32 nbCount;
    memcpy( cbi32.b, &pBuffer[988], 4 );
    nbCount = cbi32.i;
    ss.str( "" );
    ss << "Nb. tracks: " << nbCount;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Version number. Current version is 2. [4 bytes]
    wxUint32 version;
    memcpy( cbi32.b, &pBuffer[992], 4 );
    version = cbi32.i;
    ss.str( "" );
    ss << "Version: " << version;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Size of the header. Used to determine byte swap. Should be 1000. [4 bytes]
    wxUint32 hdrSize;
    memcpy( cbi32.b, &pBuffer[996], 4 );
    hdrSize = cbi32.i;
    ss.str( "" );
    ss << "HDR size: " << hdrSize;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    ////
    // READ DATA
    ////
    delete[] pBuffer;
    pBuffer = NULL;
    vector<float> tmpPoints;

    //File size - header size (used when the number of fiber was not stored)
    int remainingBytes = nSize - hdrSize;

    vector< vector< float > > lines;
    m_countPoints = 0;
    vector< float > colors;

    //if( nbCount == 0 )
    //{
    //   return false; //TODO: handle it. (0 means the number was NOT stored.)
    //}

    for( unsigned int i = 0; i < nbCount || remainingBytes > 0; ++i )
    //for( unsigned int i = 0; i != nbCount ; ++i )
    {
        //Number of points in this track. [4 bytes]
        wxUint32 nbPoints;
        dataFile.Read( cbi32.b, ( size_t )4 );
        nbPoints = cbi32.i;

        //Read data of one track.
        size_t ptsSize = 3 + nbScalars;
        size_t tractSize = 4 * ( nbPoints * ( ptsSize ) + nbProperties );
        pBuffer = new wxUint8[tractSize];
        dataFile.Read( pBuffer, tractSize );
        vector< float > curLine;

        for( unsigned int j = 0; j != nbPoints; ++j )
        {
            //Read coordinates (x,y,z) and scalars associated to each point.
            for( unsigned int k = 0; k != ptsSize; ++k )
            {
                memcpy( cbf.b, &pBuffer[4 * ( j * ptsSize + k )], 4 );

                if( k >= 6 ) //TODO: incorporate other scalars in the navigator.
                {
                    break;
                }
                else if( k >= 3 ) //RGB color of each point.
                {
                    colors.push_back( cbf.f );
                }
                else
                {
                    curLine.push_back( cbf.f );
                }
            }
        }

        //TODO: incorporate properties in the navigator.
//         for( unsigned int j = 0; j != nbProperties; ++j )
//         {}

        m_countPoints += curLine.size() / 3;
        lines.push_back( curLine );
        delete[] pBuffer;
        pBuffer = NULL;

       //Adjust remainingBytes
       remainingBytes -= 1*4; //Number of points (4 bytes)
       remainingBytes -= 3*nbPoints * 4; //Coordinates (4 bytes)
       remainingBytes -= nbScalars*nbPoints * 4; //Scalars (4 bytes)
       remainingBytes -= nbProperties * 4; //Properties (4 bytes)
       //cout <<  "i : " << i << ", remainingBytes : " << remainingBytes << "\n";
    }

    dataFile.Close();

    ////
    //POST PROCESS: set all the data in the right format for the navigator
    ////
    Logger::getInstance()->print( wxT( "Setting data in right format for the navigator..." ), LOGLEVEL_MESSAGE );
    m_countLines = lines.size();
    m_pointArray.max_size();
    m_colorArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_colorArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    ss.str( "" );
    ss << "m_countLines: " << m_countLines;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    ss.str( "" );
    ss << "m_countPoints: " << m_countPoints;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    m_linePointers[0] = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i + 1] = m_linePointers[i] + lines[i].size() / 3;
    }

    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    unsigned int pos( 0 );
    vector< vector< float > >::iterator it;

    for( it = lines.begin(); it < lines.end(); it++ )
    {
        vector< float >::iterator it2;

        for( it2 = ( *it ).begin(); it2 < ( *it ).end(); it2++ )
        {
            if (colors.size() > 0)
            {
                m_colorArray[pos] = colors[pos] / 255.;
            }
            m_pointArray[pos++] = *it2;
        }
    }

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    if( voxelSize[0] == 0 && voxelSize[1] == 0 && voxelSize[2] == 0 )
    {
        ss.str( "" );
        ss << "Using anatomy's voxel size: [" << voxelX << ", " << voxelY << ", " << voxelZ << "]";
        Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
        voxelSize[0] = voxelX;
        voxelSize[1] = voxelY;
        voxelSize[2] = voxelZ;
        ss.str( "" );
        ss << "Centering with respect to the anatomy: [" << columns / 2 << "," << rows / 2 << "," << frames / 2 << "]";
        Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
        origin[0] = columns / 2;
        origin[1] = rows / 2;
        origin[2] = frames / 2;
    }

    float flipX = ( invertX ) ? -1. : 1.;
    float flipY = ( invertY ) ? -1. : 1.;
    float flipZ = ( invertZ ) ? -1. : 1.;
    float anatomy[3];
    anatomy[0] = ( flipX - 1. ) * columns * voxelX / -2.;
    anatomy[1] = ( flipY - 1. ) * rows    * voxelY / -2.;
    anatomy[2] = ( flipZ - 1. ) * frames  * voxelZ / -2.;

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        m_pointArray[i] = flipX * ( m_pointArray[i] - origin[0] ) * voxelX / voxelSize[0] + anatomy[0];
        ++i;
        m_pointArray[i] = flipY * ( m_pointArray[i] - origin[1] ) * voxelY / voxelSize[1] + anatomy[1];
        ++i;
        m_pointArray[i] = flipZ * ( m_pointArray[i] - origin[2] ) * voxelZ / voxelSize[2] + anatomy[2];
    }

    Logger::getInstance()->print( wxT( "TRK file loaded" ), LOGLEVEL_MESSAGE );
    createColorArray( colors.size() > 0 );
    m_type = FIBERS;
    m_fullPath = filename;

#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif
    return true;
}

bool Fibers::loadCamino( const wxString &filename )
{
    Logger::getInstance()->print( wxT( "Loading Camino file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize = 0;

    if( dataFile.Open( filename ) )
    {
        nSize = dataFile.Length();

        if( nSize == wxInvalidOffset )
        {
            return false;
        }
    }

    wxUint8 *pBuffer = new wxUint8[nSize];
    dataFile.Read( pBuffer, nSize );
    dataFile.Close();
    m_countLines  = 0; // Number of lines.
    m_countPoints = 0; // Number of points.
    int cl = 0;
    int pc = 0;
    converterByteFloat cbf;
    vector< float > tmpPoints;

    while( pc < nSize )
    {
        ++m_countLines;
        cbf.b[3] = pBuffer[pc++];
        cbf.b[2] = pBuffer[pc++];
        cbf.b[1] = pBuffer[pc++];
        cbf.b[0] = pBuffer[pc++];
        cl = ( int )cbf.f;
        m_lineArray.push_back( cl );
        pc += 4;

        for( int i = 0; i < cl; ++i )
        {
            m_lineArray.push_back( m_countPoints );
            ++m_countPoints;
            cbf.b[3] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[0] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
            cbf.b[3] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[0] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
            cbf.b[3] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[0] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );

            if( pc > nSize )
            {
                break;
            }
        }
    }

    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_pointArray.resize( tmpPoints.size() );

    for( size_t i = 0; i < tmpPoints.size(); ++i )
    {
        m_pointArray[i] = tmpPoints[i];
    }

    printf( "%d lines and %d points \n", m_countLines, m_countPoints );
    Logger::getInstance()->print( wxT( "Move vertices" ), LOGLEVEL_MESSAGE );

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        m_pointArray[i] = columns * voxelX - m_pointArray[i];
        ++i;
        m_pointArray[i] = rows    * voxelY - m_pointArray[i];
        ++i;
        m_pointArray[i] = frames  * voxelZ - m_pointArray[i];
    }

    calculateLinePointers();
    createColorArray( false );
    Logger::getInstance()->print( wxT( "Read all" ), LOGLEVEL_MESSAGE );
    delete[] pBuffer;
    pBuffer = NULL;

    m_type = FIBERS;
    m_fullPath = filename;
#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif

    return true;
}

bool Fibers::loadMRtrix( const wxString &filename )
{
    Logger::getInstance()->print( wxT( "Loading MRtrix file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    long int nSize = 0;
    long int pc = 0, nodes = 0;
    converterByteFloat cbf;
    float x, y, z, x2, y2, z2;
    std::vector< float > tmpPoints;
    vector< vector< float > > lines;

    //Open file
    FILE *pFs = fopen( filename.ToAscii(), "r" ) ;
    ////
    // read header
    ////
    char lineBuffer[200];
    std::string readLine("");
    bool countFieldFound( false );

    while(readLine.find( "END" ) == std::string::npos)
    {
        fgets( lineBuffer, 200, pFs );

        readLine = std::string( lineBuffer );

        if( readLine.find( "file" ) != std::string::npos )
        {
            sscanf( lineBuffer, "file: . %ld", &pc );
        }

        if( readLine.find( "count" ) != std::string::npos && !countFieldFound )
        {
            sscanf( lineBuffer, "count: %d", &m_countLines );
            countFieldFound = true;
        }
    }

    fclose( pFs );

    if( dataFile.Open( filename ) )
    {
        nSize = dataFile.Length();

        if( nSize < 1 )
        {
            return false;
        }
    }

    nSize -= pc;
    dataFile.Seek( pc );
    wxUint8 *pBuffer = new wxUint8[nSize];
    dataFile.Read( pBuffer, nSize );
    dataFile.Close();

    Logger::getInstance()->print( wxT( "Reading fibers" ), LOGLEVEL_DEBUG );
    pc = 0;
    m_countPoints = 0; // number of points

    for( int i = 0; i < m_countLines; i++ )
    {
        tmpPoints.clear();
        nodes = 0;
        // read one tract
        cbf.b[0] = pBuffer[pc++];
        cbf.b[1] = pBuffer[pc++];
        cbf.b[2] = pBuffer[pc++];
        cbf.b[3] = pBuffer[pc++];
        x = cbf.f;
        cbf.b[0] = pBuffer[pc++];
        cbf.b[1] = pBuffer[pc++];
        cbf.b[2] = pBuffer[pc++];
        cbf.b[3] = pBuffer[pc++];
        y = cbf.f;
        cbf.b[0] = pBuffer[pc++];
        cbf.b[1] = pBuffer[pc++];
        cbf.b[2] = pBuffer[pc++];
        cbf.b[3] = pBuffer[pc++];
        z = cbf.f;
        //add first point
        tmpPoints.push_back( x );
        tmpPoints.push_back( y );
        tmpPoints.push_back( z );
        ++nodes;
        x2 = x;
        cbf.f = x2;

        //Read points (x,y,z) until x2 equals NaN (0x0000C07F), meaning end of the tract.
        while( !( cbf.b[0] == 0x00 && cbf.b[1] == 0x00 && cbf.b[2] == 0xC0 && cbf.b[3] == 0x7F ) )
        {
            cbf.b[0] = pBuffer[pc++];   // get next float
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            x2 = cbf.f;
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            y2 = cbf.f;
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            z2 = cbf.f;

            // downsample fibers: take only points in distance of min 0.75 mm
            if( ( ( x - x2 ) * ( x - x2 ) + ( y - y2 ) * ( y - y2 ) + ( z - z2 ) * ( z - z2 ) ) >= 0.2 )
            {
                x = x2;
                y = y2;
                z = z2;
                tmpPoints.push_back( x );
                tmpPoints.push_back( y );
                tmpPoints.push_back( z );
                ++nodes;
            }

            cbf.f = x2;
        }

        // put the tract in the line array
        lines.push_back( tmpPoints );

        for( int i = 0; i < nodes ; i++ )
        {
            m_countPoints++;
        }
    }

    delete[] pBuffer;
    pBuffer = NULL;

    ////
    //POST PROCESS: set all the data in the right format for the navigator
    ////
    m_pointArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_linePointers[0] = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i + 1] = m_linePointers[i] + lines[i].size() / 3;
    }

    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    unsigned int pos = 0;
    vector< vector< float > >::iterator it;

    for( it = lines.begin(); it < lines.end(); it++ )
    {
        vector< float >::iterator it2;

        for( it2 = ( *it ).begin(); it2 < ( *it ).end(); it2++ )
        {
            m_pointArray[pos++] = *it2;
        }
    }

    // The MrTrix fibers are defined in the same geometric reference
    // as the anatomical file. That is, the fibers coordinates are related to
    // the anatomy in world space. The transformation from local to world space
    // for the anatomy is encoded in the m_dh->m_niftiTransform member.
    // Since we do not consider this tranform when loading the anatomy, we must
    // bring back the fibers in the same reference, using the inverse of the
    // local to world transformation. A further problem arises when loading an
    // anatomy that has voxels with dimensions differing from 1x1x1. The
    // scaling factor is encoded in the transformation matrix, but we do not,
    // for the moment, use this scaling. Therefore, we must remove it from the
    // the transformation matrix before computing its inverse.
    FMatrix localToWorld = FMatrix( DatasetManager::getInstance()->getNiftiTransform() );

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    if( voxelX != 1.0 || voxelY != 1.0 || voxelZ != 1.0 )
    {
        FMatrix rotMat( 3, 3 );
        localToWorld.getSubMatrix( rotMat, 0, 0 );

        FMatrix scaleInversion( 3, 3 );
        scaleInversion( 0, 0 ) = 1.0 / voxelX;
        scaleInversion( 1, 1 ) = 1.0 / voxelY;
        scaleInversion( 2, 2 ) = 1.0 / voxelZ;

        rotMat = scaleInversion * rotMat;

        localToWorld.setSubMatrix( 0, 0, rotMat );
    }

    FMatrix invertedTransform( 4, 4 );
    invertedTransform = invert( localToWorld );

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        FMatrix curPoint( 4, 1 );
        curPoint( 0, 0 ) = m_pointArray[i];
        curPoint( 1, 0 ) = m_pointArray[i + 1];
        curPoint( 2, 0 ) = m_pointArray[i + 2];
        curPoint( 3, 0 ) = 1;

        FMatrix invertedPoint = invertedTransform * curPoint;

        m_pointArray[i] = invertedPoint( 0, 0 );
        m_pointArray[i + 1] = invertedPoint( 1, 0 );
        m_pointArray[i + 2] = invertedPoint( 2, 0 );

        i += 2;
    }

    Logger::getInstance()->print( wxT( "TCK file loaded" ), LOGLEVEL_MESSAGE );
    createColorArray( false );
    m_type = FIBERS;
    m_fullPath = filename;

#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif
    return true;
}

bool Fibers::loadPTK( const wxString &filename )
{
    Logger::getInstance()->print( wxT( "Loading PTK file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize = 0;
    int pc = 0;
    converterByteINT32 cbi;
    converterByteFloat cbf;
    vector< float > tmpPoints;

    if( dataFile.Open( filename ) )
    {
        nSize = dataFile.Length();

        if( nSize == wxInvalidOffset )
            return false;
    }

    wxUint8 *pBuffer = new wxUint8[nSize];
    dataFile.Read( pBuffer, nSize );
    m_countLines  = 0; // Number of lines.
    m_countPoints = 0; // Number of points.

    while( pc < nSize )
    {
        ++m_countLines;
        cbi.b[0] = pBuffer[pc++];
        cbi.b[1] = pBuffer[pc++];
        cbi.b[2] = pBuffer[pc++];
        cbi.b[3] = pBuffer[pc++];
        m_lineArray.push_back( cbi.i );

        for( size_t i = 0; i < cbi.i; ++i )
        {
            m_lineArray.push_back( m_countPoints );
            ++m_countPoints;
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
        }
    }

    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_pointArray.resize( tmpPoints.size() );

    for( size_t i = 0; i < tmpPoints.size(); ++i )
    {
        m_pointArray[i] = tmpPoints[i];
    }

    printf( "%d lines and %d points \n", m_countLines, m_countPoints );
    Logger::getInstance()->print( wxT( "Move vertices" ), LOGLEVEL_MESSAGE );

    /*for( int i = 0; i < m_countPoints * 3; ++i )
    {
    m_pointArray[i] = m_dh->m_columns - m_pointArray[i];
    ++i;
    m_pointArray[i] = m_dh->m_rows - m_pointArray[i];
    ++i;
    m_pointArray[i] = m_dh->m_frames - m_pointArray[i];
    }*/
    /********************************************************************
    * This is a fix for the visContest
    * Only tested on -visContest fibers
    *                -PGuevara datas
    *
    * Hypothesis: If bundles computed in ptk, coordinates (x,y,z) are
    * already in the space of the dataset. Good voxel size and origin
    *
    ********************************************************************/
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        m_pointArray[i] = columns * voxelX - m_pointArray[i];
        ++i;
        m_pointArray[i] = rows    * voxelY - m_pointArray[i];
        ++i;
        m_pointArray[i] = frames  * voxelZ - m_pointArray[i];
    }

    calculateLinePointers();
    createColorArray( false );
    Logger::getInstance()->print( wxT( "Read all" ), LOGLEVEL_MESSAGE );

    delete[] pBuffer;
    pBuffer = NULL;

    m_type = FIBERS;
    m_fullPath = filename;
#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif

    return true;
}

bool Fibers::loadVTK( const wxString &filename )
{
    Logger::getInstance()->print( wxT( "Loading VTK file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize = 0;

    if( dataFile.Open( filename ) )
    {
        nSize = dataFile.Length();

        if( nSize == wxInvalidOffset )
            return false;
    }

    wxUint8 *pBuffer = new wxUint8[255];
    dataFile.Read( pBuffer, ( size_t )255 );

    int pointOffset         = 0;
    int lineOffset          = 0;
    int pointColorOffset    = 0;
    int lineColorOffset     = 0;
    int fileOffset          = 0;
    int j                   = 0;

    bool colorsLoadedFromFile( false );

    char *pTemp = new char[256];

    // Ignore the first 2 lines.
    while( pBuffer[fileOffset] != '\n' )
    {
        ++fileOffset;
    }

    ++fileOffset;

    while( pBuffer[fileOffset] != '\n' )
    {
        ++fileOffset;
    }

    ++fileOffset;

    // Check the file type.
    while( pBuffer[fileOffset] != '\n' )
    {
        pTemp[j] = pBuffer[fileOffset];
        ++fileOffset;
        ++j;
    }

    ++fileOffset;
    pTemp[j] = 0;
    wxString type( pTemp, wxConvUTF8 );

    if( type == wxT( "ASCII" ) )
    {
        // ASCII file, maybe later.
        return false;
    }

    if( type != wxT( "BINARY" ) )
    {
        // Something else, don't know what to do.
        return false;
    }

    // Ignore line DATASET POLYDATA.
    while( pBuffer[fileOffset] != '\n' )
    {
        ++fileOffset;
    }

    ++fileOffset;
    j = 0;

    // Read POINTS.
    while( pBuffer[fileOffset] != '\n' )
    {
        pTemp[j] = pBuffer[fileOffset];
        ++fileOffset;
        ++j;
    }

    ++fileOffset;
    pTemp[j] = 0;
    wxString points( pTemp, wxConvUTF8 );
    points = points.AfterFirst( ' ' );
    points = points.BeforeFirst( ' ' );
    long tempValue = 0;

    if( ! points.ToLong( &tempValue, 10 ) )
    {
        return false; // Can't read point count.
    }

    int countPoints = ( int )tempValue;
    // Start position of the point array in the file.
    pointOffset = fileOffset;
    // Jump to postion after point array.
    fileOffset += ( 12 * countPoints ) + 1;
    j = 0;
    dataFile.Seek( fileOffset );
    dataFile.Read( pBuffer, ( size_t ) 255 );

    while( pBuffer[j] != '\n' )
    {
        pTemp[j] = pBuffer[j];
        ++fileOffset;
        ++j;
    }

    ++fileOffset;
    pTemp[j] = 0;
    wxString sLines( pTemp, wxConvUTF8 );
    wxString sLengthLines = sLines.AfterLast( ' ' );

    if( ! sLengthLines.ToLong( &tempValue, 10 ) )
    {
        return false; // Can't read size of lines array.
    }

    int lengthLines = ( int( tempValue ) );
    sLines = sLines.AfterFirst( ' ' );
    sLines = sLines.BeforeFirst( ' ' );

    if( ! sLines.ToLong( &tempValue, 10 ) )
    {
        return false; // Can't read lines.
    }

    int countLines = ( int ) tempValue;
    // Start postion of the line array in the file.
    lineOffset = fileOffset;
    // Jump to postion after line array.
    fileOffset += ( lengthLines * 4 ) + 1;
    dataFile.Seek( fileOffset );
    dataFile.Read( pBuffer, ( size_t ) 255 );
    j = 0;
    int k = 0;

    // TODO test if there's really a color array.
    while( pBuffer[k] != '\n' )
    {
        pTemp[j] = pBuffer[k];
        ++fileOffset;
        ++j;
        ++k;
    }

    ++k;
    ++fileOffset;
    pTemp[j] = 0;
    wxString tmpString( pTemp, wxConvUTF8 );
    j = 0;

    while( pBuffer[k] != '\n' )
    {
        pTemp[j] = pBuffer[k];
        ++fileOffset;
        ++j;
        ++k;
    }

    ++fileOffset;
    pTemp[j] = 0;
    wxString tmpString2( pTemp, wxConvUTF8 );

    if( tmpString.BeforeFirst( ' ' ) == _T( "CELL_DATA" ) )
    {
        lineColorOffset = fileOffset;
        fileOffset += ( countLines * 3 ) + 1;
        dataFile.Seek( fileOffset );
        dataFile.Read( pBuffer, ( size_t ) 255 );
        // aa 2009/06/26 workaround if the pBuffer doesn't contain a string.
        pBuffer[254] = '\n';
        int k = j = 0;

        // TODO test if there's really a color array.
        while( pBuffer[k] != '\n' )
        {
            pTemp[j] = pBuffer[k];
            ++fileOffset;
            ++j;
            ++k;
        }

        ++k;
        ++fileOffset;
        pTemp[j] = 0;
        wxString tmpString3( pTemp, wxConvUTF8 );
        tmpString = tmpString3;
        j = 0;

        while( pBuffer[k] != '\n' )
        {
            pTemp[j] = pBuffer[k];
            ++fileOffset;
            ++j;
            ++k;
        }

        ++fileOffset;
        pTemp[j] = 0;
        wxString tmpString4( pTemp, wxConvUTF8 );
        tmpString2 = tmpString4;
    }

    if( tmpString.BeforeFirst( ' ' ) == _T( "POINT_DATA" ) && tmpString2.BeforeFirst( ' ' ) == _T( "COLOR_SCALARS" ) )
    {
        pointColorOffset = fileOffset;
    }

    Logger::getInstance()->print( wxString::Format( wxT( "Loading %d points and %d lines" ), countPoints, countLines ), LOGLEVEL_MESSAGE );
    m_countLines        = countLines;
    m_countPoints       = countPoints;

    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[countLines] = countPoints;
    m_reverse.resize( countPoints );
    m_filtered.resize( countLines, false );
    m_selected.resize( countLines, false );
    m_pointArray.resize( countPoints * 3 );
    m_lineArray.resize( lengthLines * 4 );
    m_colorArray.resize( countPoints * 3 );

    dataFile.Seek( pointOffset );
    dataFile.Read( &m_pointArray[0], ( size_t )countPoints * 12 );
    dataFile.Seek( lineOffset );
    dataFile.Read( &m_lineArray[0], ( size_t )lengthLines * 4 );

    if( pointColorOffset != 0 )
    {
        vector< wxUint8 > tmpColorArray( countPoints * 3, 0 );
        dataFile.Seek( pointColorOffset );
        dataFile.Read( &tmpColorArray[0], ( size_t ) countPoints * 3 );

        for( size_t i = 0; i < tmpColorArray.size(); ++i )
        {
            m_colorArray[i] = tmpColorArray[i] / 255.;
        }

        colorsLoadedFromFile = true;
    }

    toggleEndianess();
    Logger::getInstance()->print( wxT( "Move vertices" ), LOGLEVEL_MESSAGE );

    calculateLinePointers();
    createColorArray( colorsLoadedFromFile );
    Logger::getInstance()->print( wxT( "Read all" ), LOGLEVEL_MESSAGE );
    m_type      = FIBERS;
    m_fullPath  = filename;
#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif

    delete[] pBuffer;
    delete[] pTemp;
    return true;
}

bool Fibers::loadDmri( const wxString &filename )
{
    FILE *pFile;
    pFile = fopen( filename.mb_str(), "r" );

    if( pFile == NULL )
    {
        return false;
    }

    char *pS1 = new char[10];
    char *pS2 = new char[10];
    char *pS3 = new char[10];
    char *pS4 = new char[10];
    float f1, f2, f3, f4, f5;
    int res;

    // the header
    res = fscanf( pFile, "%f %s", &f1, pS1 );
    res = fscanf( pFile, "%f %s %s %s %s", &f1, pS1, pS2, pS3, pS4 );
    res = fscanf( pFile, "%f", &f1 );
    res = fscanf( pFile, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5 );
    res = fscanf( pFile, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5 );
    res = fscanf( pFile, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5 );
    res = fscanf( pFile, "%s %s", pS1, pS2 );
    m_countLines = std::atof(pS1);

    vector< vector< float > > lines;
    m_countPoints = 0;
    float back, front;
    stringstream ss;

    for( int i = 0; i < m_countLines; i++ )
    {
        res = fscanf( pFile, "%s %s %s", pS1, pS2, pS3 );

        ss << pS1;
        ss >> back;
        ss.clear();
        ss << pS2;
        ss >> front;
        ss.clear();

        int nbpoints = back + front;

        if( back != 0 && front != 0 )
        {
            nbpoints--;
        }

        if( nbpoints > 0 )
        {
            vector< float > curLine;
            curLine.resize( nbpoints * 3 );

            //back
            for( int j = back - 1; j >= 0; j-- )
            {
                res = fscanf( pFile, "%s %s %s %s", pS1, pS2, pS3, pS4 );


                ss << pS1;
                ss >> f1;
                ss.clear();

                ss << pS2;
                ss >> f2;
                ss.clear();

                ss << pS3;
                ss >> f3;
                ss.clear();

                curLine[j * 3]  = f1;
                curLine[j * 3 + 1] = f2;
                curLine[j * 3 + 2] = f3;
            }

            if( back != 0 && front != 0 )
            {
                //repeated pts
                res = fscanf( pFile, "%s %s %s %s", pS1, pS2, pS3, pS4 );
            }

            //front
            for( int j = back; j < nbpoints; j++ )
            {
                res = fscanf( pFile, "%s %s %s %s", pS1, pS2, pS3, pS4 );

                ss << pS1;
                ss >> f1;
                ss.clear();

                ss << pS2;
                ss >> f2;
                ss.clear();

                ss << pS3;
                ss >> f3;
                ss.clear();

                curLine[j * 3]  = f1;
                curLine[j * 3 + 1] = f2;
                curLine[j * 3 + 2] = f3;
            }

            m_countPoints += curLine.size() / 3;
            lines.push_back( curLine );
        }
    }

    fclose( pFile );

    delete[] pS1;
    delete[] pS2;
    delete[] pS3;
    delete[] pS4;
    pS1 = NULL;
    pS2 = NULL;
    pS3 = NULL;
    pS4 = NULL;

    //set all the data in the right format for the navigator
    m_countLines = lines.size();
    m_pointArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_linePointers[0] = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i + 1] = m_linePointers[i] + lines[i].size() / 3;
    }

    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    unsigned int pos = 0;
    vector< vector< float > >::iterator it;

    for( it = lines.begin(); it < lines.end(); it++ )
    {
        vector< float >::iterator it2;

        for( it2 = ( *it ).begin(); it2 < ( *it ).end(); it2++ )
        {
            m_pointArray[pos++] = *it2;
        }
    }

    createColorArray( false );
    m_type = FIBERS;
    m_fullPath = filename;

#ifdef __WXMSW__
    m_name = /*"-"+*/ filename.AfterLast( '\\' );
#else
    m_name = /*"-" +*/ filename.AfterLast( '/' );
#endif
    return true;
}

bool Fibers::createFrom( const vector<Fibers*>& bundles, wxString name )
{
    m_pointArray.clear();
    m_colorArray.clear();
    m_linePointers.clear();
    m_reverse.clear();

    m_linePointers.push_back(0);
    // Copy points, copy colors, set line pointers and set reverse lookup
    for( vector<Fibers *>::const_iterator it = bundles.begin(); it != bundles.end(); ++it )
    {
        for( int i=0; i < (*it)->m_countPoints * 3; ++i )
        {
            m_pointArray.push_back( (*it)->m_pointArray[i] );
            m_colorArray.push_back( (*it)->m_colorArray[i] );
        }

        for( int i=1; i <= (*it)->m_countLines; ++i )
        {
            int length = (*it)->m_linePointers[i] - (*it)->m_linePointers[i-1];

            for( int j=0; j < length; ++j )
            {
                m_reverse.push_back( m_linePointers.size()-1 );
            }

            m_linePointers.push_back( m_linePointers.back() + length );
        }
    }

    m_countPoints = m_pointArray.size() / 3;
    m_countLines  = m_linePointers.size() - 1;
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );

    createColorArray( false );
    m_type = FIBERS;
    m_fullPath = wxString(name);
    m_name = wxString(name);

    m_pOctree = new Octree( 2, m_pointArray, m_countPoints );

    return true;
}

///////////////////////////////////////////////////////////////////////////
// This function was made for debug purposes, it will create a fake set of
// fibers with hardcoded value to be able to test different things.
///////////////////////////////////////////////////////////////////////////
void Fibers::loadTestFibers()
{
    m_countLines        = 2;  // The number of fibers you want to display.
    int lengthLines   = 10; // The number of points each fiber will have.
    int pos = 0;
    m_countPoints       = m_countLines * lengthLines;

    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );

    // Because you need to load an anatomy file first in order to load this fake set of fibers,
    // the points composing your fibers have to be between [0,159] in x, [0,199] in y and [0,159] in z.
    // This is for a straight line.
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 110.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 120.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 130.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 140.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 150.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 160.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 170.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 180.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 190.0f;
    m_pointArray[pos++] = 60.0f;

    // This is for a circle in 2D (Z never changes).
    float circleRadius = 10.0f;
    float offset       = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI *  0.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI *  0.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 10.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 10.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 20.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 20.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 30.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 30.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 40.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 40.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 50.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 50.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 60.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 60.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 70.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 70.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 80.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 80.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 90.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 90.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;

    // No need to modify the rest of this function if you only want to add a test fiber.
    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i] = i * lengthLines;
    }


    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    m_pointArray.resize( m_countPoints * 3 );
    createColorArray( false );
    m_type = FIBERS;
}

///////////////////////////////////////////////////////////////////////////
// This function will call the proper coloring function for the fibers.
///////////////////////////////////////////////////////////////////////////
void Fibers::updateFibersColors()
{
    if( m_fiberColorationMode == NORMAL_COLOR )
    {
        resetColorArray();
    }
    else
    {
        float *pColorData( NULL );

        if( SceneManager::getInstance()->isUsingVBO() )
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
            pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
        }
        else
        {
            pColorData  = &m_normalArray[0];
        }

        if( m_fiberColorationMode == CURVATURE_COLOR )
        {
            colorWithCurvature( pColorData );
        }
        else if( m_fiberColorationMode == TORSION_COLOR )
        {
            colorWithTorsion( pColorData );
        }
        else if( m_fiberColorationMode == DISTANCE_COLOR )
        {
            colorWithDistance( pColorData );
        }
        else if( m_fiberColorationMode == MINDISTANCE_COLOR )
        {
            colorWithMinDistance( pColorData );
        }
        else if( m_fiberColorationMode == CONSTANT_COLOR )
        {
            colorWithConstantColor( pColorData );
        }

        if( SceneManager::getInstance()->isUsingVBO() )
        {
            glUnmapBuffer( GL_ARRAY_BUFFER );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their torsion value.
//
// pColorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithTorsion( float *pColorData )
{
    if( pColorData == NULL )
    {
        return;
    }

    int    pc = 0;
    // TODO remove
    //Vector firstDerivative, secondDerivative, thirdDerivative;

    // For each fibers.
    for( int i = 0; i < getLineCount(); ++i )
    {
        double color        = 0.0f;
        int    index        = 0;
        float  progression  = 0.0f;
        int    pointPerLine = getPointsPerLine( i );

        // We cannot calculate the torsion for a fiber that as less that 5 points.
        // So we simply do not cange the color for this fiber
        if( pointPerLine < 5 )
        {
            continue;
        }

        // For each points of this fiber.
        for( int j = 0; j < pointPerLine; ++j )
        {
            if( j == 0 )
            {
                index = 6;                             // For the first point of each fiber.
                progression = 0.0f;
            }
            else if( j == 1 )
            {
                index = 6;                             // For the second point of each fiber.
                progression = 0.25f;
            }
            else if( j == pointPerLine - 2 )
            {
                index = ( pointPerLine - 2 ) * 3;    // For the before last point of each fiber.
                progression = 0.75f;
            }
            else if( j == pointPerLine - 1 )
            {
                index = ( pointPerLine - 2 ) * 3;    // For the last point of each fiber.
                progression = 1.0f;
            }
            else
            {
                progression = 0.5f;     // For every other points.
            }

            Helper::getProgressionTorsion(
                    Vector( m_pointArray[index - 6], m_pointArray[index - 5], m_pointArray[index - 4] ),
                    Vector( m_pointArray[index - 3], m_pointArray[index - 2], m_pointArray[index - 1] ),
                    Vector( m_pointArray[index],     m_pointArray[index + 1], m_pointArray[index + 2] ),
                    Vector( m_pointArray[index + 3], m_pointArray[index + 4], m_pointArray[index + 5] ),
                    Vector( m_pointArray[index + 6], m_pointArray[index + 7], m_pointArray[index + 8] ),
                    progression, color );

            // Lets apply a specific hard coded coloration for the torsion.
            float realColor;

            if( color <= 0.01f ) // Those points have no torsion so we simply but them pure blue.
            {
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = 0.0f;
                pColorData[pc + 2] = 1.0f;
            }
            else if( color < 0.1f )  // The majority of the values are here.
            {
                double normalizedValue = ( color - 0.01f ) / ( 0.1f - 0.01f );
                realColor = ( pow( ( double )2.71828182845904523536, normalizedValue ) ) - 1.0f;
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = realColor;
                pColorData[pc + 2] = 1.0f - realColor;
            }
            else // All the rest is simply pure green.
            {
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = 1.0f;
                pColorData[pc + 2] = 0.0f;
            }

            pc    += 3;
            index += 3;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their curvature value.
//
// pColorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithCurvature( float *pColorData )
{
    if( pColorData == NULL )
    {
        return;
    }

    int    pc = 0;
    // TODO remove
    //Vector firstDerivative, secondDerivative, thirdDerivative;

    // For each fibers.
    for( int i = 0; i < getLineCount(); ++i )
    {
        double color        = 0.0f;
        int    index        = 0;
        float  progression  = 0.0f;
        int    pointPerLine = getPointsPerLine( i );

        // We cannot calculate the curvature for a fiber that as less that 5 points.
        // So we simply do not cange the color for this fiber
        if( pointPerLine < 5 )
        {
            continue;
        }

        // For each point of this fiber.
        for( int j = 0; j < pointPerLine; ++j )
        {
            if( j == 0 )
            {
                index = 6;                             // For the first point of each fiber.
                progression = 0.0f;
            }
            else if( j == 1 )
            {
                index = 6;                             // For the second point of each fiber.
                progression = 0.25f;
            }
            else if( j == pointPerLine - 2 )
            {
                index = ( pointPerLine - 2 ) * 3;    // For the before last point of each fiber.
                progression = 0.75f;
            }
            else if( j == pointPerLine - 1 )
            {
                index = ( pointPerLine - 2 ) * 3;    // For the last point of each fiber.
                progression = 1.0f;
            }
            else
            {
                progression = 0.5f;     // For every other points.
            }

            Helper::getProgressionCurvature(
                    Vector( m_pointArray[index - 6], m_pointArray[index - 5], m_pointArray[index - 4] ),
                    Vector( m_pointArray[index - 3], m_pointArray[index - 2], m_pointArray[index - 1] ),
                    Vector( m_pointArray[index],     m_pointArray[index + 1], m_pointArray[index + 2] ),
                    Vector( m_pointArray[index + 3], m_pointArray[index + 4], m_pointArray[index + 5] ),
                    Vector( m_pointArray[index + 6], m_pointArray[index + 7], m_pointArray[index + 8] ),
                    progression, color );

            // Lets apply a specific hard coded coloration for the curvature.
            float realColor;

            if( color <= 0.01f ) // Those points have no curvature so we simply but them pure blue.
            {
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = 0.0f;
                pColorData[pc + 2] = 1.0f;
            }
            else if( color < 0.1f )  // The majority of the values are here.
            {
                double normalizedValue = ( color - 0.01f ) / ( 0.1f - 0.01f );
                realColor = ( pow( ( double )2.71828182845904523536, normalizedValue ) ) - 1.0f;
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = realColor;
                pColorData[pc + 2] = 1.0f - realColor;
            }
            else // All the rest is simply pure green.
            {
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = 1.0f;
                pColorData[pc + 2] = 0.0f;
            }

            pc    += 3;
            index += 3;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their distance to the
// flagged distance anchors voi.
//
// pColorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithDistance( float *pColorData )
{
    if( pColorData == NULL )
    {
        return;
    }

    SelectionTree::SelectionObjectVector selectionObjects = SceneManager::getInstance()->getSelectionTree().getAllObjects();

    vector< SelectionObject* > simplifiedList;

    for( unsigned int objIdx( 0 ); objIdx < selectionObjects.size(); ++objIdx )
    {
        if( selectionObjects[objIdx]->IsUsedForDistanceColoring() )
        {
            simplifiedList.push_back( selectionObjects[objIdx] );
        }
    }

    int   columns = DatasetManager::getInstance()->getColumns();
    int   rows    = DatasetManager::getInstance()->getRows();
    int   frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < getPointCount(); ++i )
    {
        float minDistance = FLT_MAX;
        int x     = std::min( columns - 1, std::max( 0, (int)( m_pointArray[i * 3 ]    / voxelX ) ) );
        int y     = std::min( rows    - 1, std::max( 0, (int)( m_pointArray[i * 3 + 1] / voxelY ) ) );
        int z     = std::min( frames  - 1, std::max( 0, (int)( m_pointArray[i * 3 + 2] / voxelZ ) ) );
        int index = x + y * columns + z * rows * columns;

        for( unsigned int j = 0; j < simplifiedList.size(); ++j )
        {
            // TODO selection VOI adjust
            /*if( simplifiedList[j]->m_sourceAnatomy != NULL )
            {
                float curValue = simplifiedList[j]->m_sourceAnatomy->at( index );

                if( curValue < minDistance )
                {
                    minDistance = curValue;
                }
            }*/
        }

        float thresh = m_threshold / 2.0f;

        if( minDistance > ( thresh ) && minDistance < ( thresh + LINEAR_GRADIENT_THRESHOLD ) )
        {
            float greenVal = ( minDistance - thresh ) / LINEAR_GRADIENT_THRESHOLD;
            float redVal = 1 - greenVal;
            pColorData[3 * i]      = redVal;
            pColorData[3 * i + 1]  = greenVal;
            pColorData[3 * i + 2]  = 0.0f;
        }
        else if( minDistance > ( thresh + LINEAR_GRADIENT_THRESHOLD ) )
        {
            pColorData[3 * i ]     = 0.0f;
            pColorData[3 * i + 1]  = 1.0f;
            pColorData[3 * i + 2]  = 0.0f;
        }
        else
        {
            pColorData[3 * i ]     = 1.0f;
            pColorData[3 * i + 1]  = 0.0f;
            pColorData[3 * i + 2]  = 0.0f;
        }
    }
}

void Fibers::colorWithMinDistance( float *pColorData )
{
    if( pColorData == NULL )
    {
        return;
    }

    SelectionTree::SelectionObjectVector selectionObjects = SceneManager::getInstance()->getSelectionTree().getAllObjects();

    vector< SelectionObject* > simplifiedList;

    for( unsigned int objIdx( 0 ); objIdx < selectionObjects.size(); ++objIdx )
    {
        if( selectionObjects[objIdx]->IsUsedForDistanceColoring() )
        {
            simplifiedList.push_back( selectionObjects[objIdx] );
        }
    }

    int   columns = DatasetManager::getInstance()->getColumns();
    int   rows    = DatasetManager::getInstance()->getRows();
    int   frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < getLineCount(); ++i )
    {
        int nbPointsInLine = getPointsPerLine( i );
        int index = getStartIndexForLine( i );
        float minDistance = FLT_MAX;

        for( int j = 0; j < nbPointsInLine; ++j )
        {
            int x     = std::min( columns - 1, std::max( 0, (int)( m_pointArray[( index + j ) * 3 ]    / voxelX ) ) ) ;
            int y     = std::min( rows    - 1, std::max( 0, (int)( m_pointArray[( index + j ) * 3 + 1] / voxelY ) ) ) ;
            int z     = std::min( frames  - 1, std::max( 0, (int)( m_pointArray[( index + j ) * 3 + 2] / voxelZ ) ) ) ;
            int index = x + y * columns + z * rows * columns;

            for( unsigned int k = 0; k < simplifiedList.size(); ++k )
            {
                // TODO selection VOI m_sourceanat
                /*float curValue = simplifiedList[k]->m_sourceAnatomy->at( index );

                if( curValue < minDistance )
                {
                    minDistance = curValue;
                }*/
            }
        }

        float thresh = m_threshold / 2.0f;
        Vector theColor;
        float theAlpha;

        if( m_localizedAlpha.size() != ( unsigned int ) getPointCount() )
        {
            m_localizedAlpha = vector< float >( getPointCount() );
        }

        if( minDistance > ( thresh ) && minDistance < ( thresh + LINEAR_GRADIENT_THRESHOLD ) )
        {
            float greenVal = ( minDistance - thresh ) / LINEAR_GRADIENT_THRESHOLD;
            float redVal = 1 - greenVal;
            theColor.x  = redVal;
            theColor.y  = 0.9f;
            theColor.z  = 0.0f;

            if( redVal < MIN_ALPHA_VALUE )
            {
                theAlpha = MIN_ALPHA_VALUE;
            }
            else
            {
                theAlpha = pow( redVal, 6.0f );
            }
        }
        else if( minDistance > ( thresh + LINEAR_GRADIENT_THRESHOLD ) )
        {
            theColor.x  = 0.0f;
            theColor.y  = 1.0f;
            theColor.z  = 0.0f;
            theAlpha = MIN_ALPHA_VALUE;
        }
        else
        {
            theColor.x  = 1.0f;
            theColor.y  = 0.0f;
            theColor.z  = 0.0f;
            theAlpha = 1.0;
        }

        for( int j = 0; j < nbPointsInLine; ++j )
        {
            pColorData[( index + j ) * 3]     = theColor.x;
            pColorData[( index + j ) * 3 + 1] = theColor.y;
            pColorData[( index + j ) * 3 + 2] = theColor.z;
            m_localizedAlpha[index + j] = theAlpha;
        }
    }
}

void Fibers::colorWithConstantColor( float *pColorData )
{
    if( pColorData == NULL )
    {
        return;
    }

    float r = m_constantColor.Red() / 255.f;
    float g = m_constantColor.Green() / 255.f;
    float b = m_constantColor.Blue() / 255.f;

    for( int ptColorIdx = 0; ptColorIdx < m_countPoints * 3; ptColorIdx += 3 )
    {
        pColorData[ptColorIdx] = r;
        pColorData[ptColorIdx + 1] = g;
        pColorData[ptColorIdx + 2] = b;
    }
}

Anatomy* Fibers::generateFiberVolume()
{
    float* pColorData( NULL );
    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );

        #ifdef __WXMAC__
            // TODO check this
            //glBufferData(GL_ARRAY_BUFFER, getPointCount()*3 + 2, NULL, GL_STREAM_DRAW);
        #endif
        pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        pColorData  = &m_normalArray[0];
    }

    if( m_localizedAlpha.size() != ( unsigned int )getPointCount() )
    {
        m_localizedAlpha = vector< float >( getPointCount(), 1 );
    }

    DatasetIndex index = DatasetManager::getInstance()->createAnatomy( RGB );
    Anatomy *pTmpAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( index );
    pTmpAnatomy->setName( m_name.BeforeFirst( '.' ) + wxT(" Fiber-Orientation Volume" ) );

    MyApp::frame->m_pListCtrl->InsertItem( index );

    MyApp::frame->refreshAllGLWidgets();

    int   columns = DatasetManager::getInstance()->getColumns();
    int   rows    = DatasetManager::getInstance()->getRows();
    int   frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < getPointCount(); ++i )
    {
        int x     = std::min( columns - 1, std::max( 0, (int)( m_pointArray[i * 3 ]    / voxelX ) ) ) ;
        int y     = std::min( rows    - 1, std::max( 0, (int)( m_pointArray[i * 3 + 1] / voxelY ) ) ) ;
        int z     = std::min( frames  - 1, std::max( 0, (int)( m_pointArray[i * 3 + 2] / voxelZ ) ) ) ;
        int index = x + y * columns + z * rows * columns;

        ( *pTmpAnatomy->getFloatDataset() )[index * 3]     = pColorData[i * 3]     ;
        ( *pTmpAnatomy->getFloatDataset() )[index * 3 + 1] = pColorData[i * 3 + 1] ;
        ( *pTmpAnatomy->getFloatDataset() )[index * 3 + 2] = pColorData[i * 3 + 2] ;
    }

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }

    return pTmpAnatomy;
}

void Fibers::getFibersInfoToSave( vector<float>& pointsToSave,  vector<int>& linesToSave, vector<int>& colorsToSave, int& countLines )
{
    int pointIndex( 0 );
    countLines = 0;

    float *pColorData( NULL );

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        pColorData = &m_colorArray[0];
    }

    for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l] )
        {
            unsigned int pc = getStartIndexForLine( l ) * 3;
            linesToSave.push_back( getPointsPerLine( l ) );

            for( int j = 0; j < getPointsPerLine( l ); ++j )
            {
                pointsToSave.push_back( m_pointArray[pc] );
                colorsToSave.push_back( ( wxUint8 )( pColorData[pc] * 255 ) );
                ++pc;
                pointsToSave.push_back( m_pointArray[pc] );
                colorsToSave.push_back( ( wxUint8 )( pColorData[pc] * 255 ) );
                ++pc;
                pointsToSave.push_back( m_pointArray[pc] );
                colorsToSave.push_back( ( wxUint8 )( pColorData[pc] * 255 ) );
                ++pc;
                linesToSave.push_back( pointIndex );
                ++pointIndex;
            }
            ++countLines;
        }
    }

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
}

void Fibers::getNbLines( int& nbLines )
{
    nbLines = 0;

    for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l] )
        {
            nbLines++;
        }
    }
}

void Fibers::loadDMRIFibersInFile( ofstream& myfile )
{
    for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l] )
        {
            unsigned int pc = getStartIndexForLine( l ) * 3;
            myfile << getPointsPerLine( l ) << " 1\n1\n";

            for( int j = 0; j < getPointsPerLine( l ); ++j )
            {
                myfile <<  m_pointArray[pc] << " " <<  m_pointArray[pc + 1] << " " <<  m_pointArray[pc + 2] << " 0\n";
                pc += 3;
            }

            pc = getStartIndexForLine( l ) * 3;
            myfile <<  m_pointArray[pc] << " " <<  m_pointArray[pc + 1] << " " <<  m_pointArray[pc + 2] << " 0\n";
        }
    }
}

/**
 * Save using the VTK binary format.
 */
void Fibers::save( wxString filename )
{
    ofstream myfile;
    char *pFn;
    vector<char> vBuffer;
    converterByteINT32 c;
    converterByteFloat f;
    vector<float> pointsToSave;
    vector<int> linesToSave;
    vector<int> colorsToSave;
    int countLines = 0;

    if( filename.AfterLast( '.' ) != _T( "fib" ) )
    {
        filename += _T( ".fib" );
    }

    pFn = ( char * ) malloc( filename.length() );
    strcpy( pFn, ( const char * ) filename.mb_str( wxConvUTF8 ) );
    myfile.open( pFn, std::ios::binary );

    getFibersInfoToSave( pointsToSave, linesToSave, colorsToSave, countLines );

    string header1 = "# vtk DataFile Version 3.0\nvtk output\nBINARY\nDATASET POLYDATA\nPOINTS ";
    header1 += intToString( pointsToSave.size() / 3 );
    header1 += " float\n";
    for( unsigned int i = 0; i < header1.size(); ++i )
    {
        vBuffer.push_back( header1[i] );
    }
    for( unsigned int i = 0; i < pointsToSave.size(); ++i )
    {
        f.f = pointsToSave[i];
        vBuffer.push_back( f.b[3] );
        vBuffer.push_back( f.b[2] );
        vBuffer.push_back( f.b[1] );
        vBuffer.push_back( f.b[0] );
    }

    vBuffer.push_back( '\n' );
    string header2 = "LINES " + intToString( countLines ) + " " + intToString( linesToSave.size() ) + "\n";
    for( unsigned int i = 0; i < header2.size(); ++i )
    {
        vBuffer.push_back( header2[i] );
    }
    for( unsigned int i = 0; i < linesToSave.size(); ++i )
    {
        c.i = linesToSave[i];
        vBuffer.push_back( c.b[3] );
        vBuffer.push_back( c.b[2] );
        vBuffer.push_back( c.b[1] );
        vBuffer.push_back( c.b[0] );
    }

    vBuffer.push_back( '\n' );
    string header3 = "POINT_DATA ";
    header3 += intToString( pointsToSave.size() / 3 );
    header3 += " float\n";
    header3 += "COLOR_SCALARS scalars 3\n";
    for( unsigned int i = 0; i < header3.size(); ++i )
    {
        vBuffer.push_back( header3[i] );
    }
    for( unsigned int i = 0; i < colorsToSave.size(); ++i )
    {
        vBuffer.push_back( colorsToSave[i] );
    }
    vBuffer.push_back( '\n' );

    // Put the buffer vector into a char* array.
    char* pBuffer = new char[vBuffer.size()];

    for( unsigned int i = 0; i < vBuffer.size(); ++i )
    {
        pBuffer[i] = vBuffer[i];
    }

    myfile.write( pBuffer, vBuffer.size() );
    myfile.close();

    delete[] pBuffer;
    pBuffer = NULL;
}

//////////////////////////////////////////////////////////////////////////

bool Fibers::save( wxXmlNode *pNode, const wxString &rootPath ) const
{
    assert( pNode != NULL );

    pNode->SetName( wxT( "dataset" ) );
    DatasetInfo::save( pNode, rootPath );

    return true;
}

//////////////////////////////////////////////////////////////////////////

void Fibers::saveDMRI( wxString filename )
{
    ofstream myfile;
    int nbrlines;
    char *pFn;
    float dist = 0.5;

    if( filename.AfterLast( '.' ) != _T( "fib" ) )
    {
        filename += _T( ".fib" );
    }

    pFn = ( char * ) malloc( filename.length() );
    strcpy( pFn, ( const char * ) filename.mb_str( wxConvUTF8 ) );
    myfile.open( pFn, std::ios::out );

    getNbLines( nbrlines );

    myfile << "1 FA\n4 min max mean var\n1\n4 0 0 0 0\n4 0 0 0 0\n4 0 0 0 0\n";
    myfile << nbrlines << " " << dist << "\n";
    loadDMRIFibersInFile( myfile);

    myfile.close();
}

string Fibers::intToString( const int number )
{
    stringstream out;
    out << number;
    return out.str();
}

void Fibers::toggleEndianess()
{
    Logger::getInstance()->print( wxT( "Toggle Endianess" ), LOGLEVEL_MESSAGE );
    wxUint8 temp = 0;
    wxUint8 *pPointBytes = ( wxUint8 * )&m_pointArray[0];

    for( int i = 0; i < m_countPoints * 12; i += 4 )
    {
        temp = pPointBytes[i];
        pPointBytes[i] = pPointBytes[i + 3];
        pPointBytes[i + 3] = temp;
        temp = pPointBytes[i + 1];
        pPointBytes[i + 1] = pPointBytes[i + 2];
        pPointBytes[i + 2] = temp;
    }

    // Toggle endianess for the line array.
    wxUint8 *pLineBytes = ( wxUint8 * )&m_lineArray[0];

    for( size_t i = 0; i < m_lineArray.size() * 4; i += 4 )
    {
        temp = pLineBytes[i];
        pLineBytes[i] = pLineBytes[i + 3];
        pLineBytes[i + 3] = temp;
        temp = pLineBytes[i + 1];
        pLineBytes[i + 1] = pLineBytes[i + 2];
        pLineBytes[i + 2] = temp;
    }
}

int Fibers::getPointsPerLine( const int lineId )
{
    return ( m_linePointers[lineId + 1] - m_linePointers[lineId] );
}

int Fibers::getStartIndexForLine( const int lineId )
{
    return m_linePointers[lineId];
}

int Fibers::getLineForPoint( const int pointIdx )
{
    return m_reverse[pointIdx];
}

void Fibers::calculateLinePointers()
{
    Logger::getInstance()->print( wxT( "Calculate line pointers" ), LOGLEVEL_MESSAGE );
    int pc = 0;
    int lc = 0;
    int tc = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i] = tc;
        lc = m_lineArray[pc];
        tc += lc;
        pc += ( lc + 1 );
    }

    lc = 0;
    pc = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lc + 1] )
        {
            ++lc;
        }

        m_reverse[i] = lc;
    }
}

void Fibers::createColorArray( const bool colorsLoadedFromFile )
{
    Logger::getInstance()->print( wxT( "Create color arrays" ), LOGLEVEL_MESSAGE );

    if( !colorsLoadedFromFile )
    {
        m_colorArray.clear();
        m_colorArray.resize( m_countPoints * 3 );
    }

    m_normalArray.clear();
    m_normalArray.resize( m_countPoints * 3 );
    int   pc = 0;

    float x1, x2, y1, y2, z1, z2 = 0.0f;
    float r, g, b, rr, gg, bb    = 0.0f;
    float lastX, lastY, lastZ          = 0.0f;

    for( int i = 0; i < getLineCount(); ++i )
    {
        x1 = m_pointArray[pc];
        y1 = m_pointArray[pc + 1];
        z1 = m_pointArray[pc + 2];
        x2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 3];
        y2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 2];
        z2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 1];
        r = ( x1 ) - ( x2 );
        g = ( y1 ) - ( y2 );
        b = ( z1 ) - ( z2 );

        if( r < 0.0 )
        {
            r *= -1.0;
        }

        if( g < 0.0 )
        {
            g *= -1.0;
        }

        if( b < 0.0 )
        {
            b *= -1.0;
        }

        float norm = sqrt( r * r + g * g + b * b );
        r *= 1.0 / norm;
        g *= 1.0 / norm;
        b *= 1.0 / norm;

        lastX = m_pointArray[pc]     + ( m_pointArray[pc]     - m_pointArray[pc + 3] );
        lastY = m_pointArray[pc + 1] + ( m_pointArray[pc + 1] - m_pointArray[pc + 4] );
        lastZ = m_pointArray[pc + 2] + ( m_pointArray[pc + 2] - m_pointArray[pc + 5] );

        for( int j = 0; j < getPointsPerLine( i ); ++j )
        {
            rr = lastX - m_pointArray[pc];
            gg = lastY - m_pointArray[pc + 1];
            bb = lastZ - m_pointArray[pc + 2];
            lastX = m_pointArray[pc];
            lastY = m_pointArray[pc + 1];
            lastZ = m_pointArray[pc + 2];

            if( rr < 0.0 )
            {
                rr *= -1.0;
            }

            if( gg < 0.0 )
            {
                gg *= -1.0;
            }

            if( bb < 0.0 )
            {
                bb *= -1.0;
            }

            float norm = sqrt( rr * rr + gg * gg + bb * bb );
            rr *= 1.0 / norm;
            gg *= 1.0 / norm;
            bb *= 1.0 / norm;
            m_normalArray[pc]     = rr;
            m_normalArray[pc + 1] = gg;
            m_normalArray[pc + 2] = bb;

            if( ! colorsLoadedFromFile )
            {
                m_colorArray[pc]     = r;
                m_colorArray[pc + 1] = g;
                m_colorArray[pc + 2] = b;
            }

            pc += 3;
        }
    }
}

void Fibers::resetColorArray()
{
    Logger::getInstance()->print( wxT( "Reset color arrays" ), LOGLEVEL_MESSAGE );
    float *pColorData( NULL );
    float *pColorData2( &m_normalArray[0] );

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        pColorData  = &m_normalArray[0];
    }

    int   pc = 0;

    float x1, x2, y1, y2, z1, z2 = 0.0f;
    float r, g, b, rr, gg, bb    = 0.0f;
    float lastX, lastY, lastZ          = 0.0f;

    for( int i = 0; i < getLineCount(); ++i )
    {
        x1 = m_pointArray[pc];
        y1 = m_pointArray[pc + 1];
        z1 = m_pointArray[pc + 2];
        x2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 3];
        y2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 2];
        z2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 1];
        r = ( x1 ) - ( x2 );
        g = ( y1 ) - ( y2 );
        b = ( z1 ) - ( z2 );

        if( r < 0.0 )
        {
            r *= -1.0;
        }

        if( g < 0.0 )
        {
            g *= -1.0;
        }

        if( b < 0.0 )
        {
            b *= -1.0;
        }

        float norm = sqrt( r * r + g * g + b * b );
        r *= 1.0 / norm;
        g *= 1.0 / norm;
        b *= 1.0 / norm;

        lastX = m_pointArray[pc]     + ( m_pointArray[pc]     - m_pointArray[pc + 3] );
        lastY = m_pointArray[pc + 1] + ( m_pointArray[pc + 1] - m_pointArray[pc + 4] );
        lastZ = m_pointArray[pc + 2] + ( m_pointArray[pc + 2] - m_pointArray[pc + 5] );

        for( int j = 0; j < getPointsPerLine( i ); ++j )
        {
            rr = lastX - m_pointArray[pc];
            gg = lastY - m_pointArray[pc + 1];
            bb = lastZ - m_pointArray[pc + 2];
            lastX = m_pointArray[pc];
            lastY = m_pointArray[pc + 1];
            lastZ = m_pointArray[pc + 2];

            if( rr < 0.0 )
            {
                rr *= -1.0;
            }

            if( gg < 0.0 )
            {
                gg *= -1.0;
            }

            if( bb < 0.0 )
            {
                bb *= -1.0;
            }

            float norm = sqrt( rr * rr + gg * gg + bb * bb );
            rr *= 1.0 / norm;
            gg *= 1.0 / norm;
            bb *= 1.0 / norm;
            pColorData[pc]     = rr;
            pColorData[pc + 1] = gg;
            pColorData[pc + 2] = bb;

            pc += 3;
        }
    }

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }

    m_fiberColorationMode = NORMAL_COLOR;
}

void Fibers::setFiberColor( const int fiberIdx, const wxColour &col )
{
    assert(fiberIdx < m_countLines );

    float *pColorData( NULL );

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        pColorData  = &m_normalArray[0];
    }

    int curPointStart( getStartIndexForLine( fiberIdx ) * 3);

    for( int i = 0; i < getPointsPerLine( fiberIdx ); ++i, curPointStart += 3 )
    {
        pColorData[curPointStart] = col.Red() / 255.0f;
        pColorData[curPointStart + 1] = col.Green() / 255.0f;
        pColorData[curPointStart + 2] = col.Blue() / 255.0f;
    }

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
}

wxColour Fibers::getFiberPointColor( const int fiberIdx, const int ptIdx )
{
    assert(fiberIdx < m_countLines );

    float *pColorData( NULL );

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        pColorData  = &m_colorArray[0];
    }

    int curPointStart( getStartIndexForLine( fiberIdx ) * 3 + ptIdx * 3);

    wxColour ptCol;
    ptCol.Set( static_cast<unsigned char>(pColorData[curPointStart] * 255.0f),
               static_cast<unsigned char>(pColorData[curPointStart + 1] * 255.0f),
               static_cast<unsigned char>(pColorData[curPointStart + 2] * 255.0f));

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }

    return ptCol;
}

void Fibers::resetLinesShown()
{
    m_selected.assign( m_countLines, false );
}

void Fibers::updateLinesShown()
{
    SelectionTree::SelectionObjectVector selectionObjects = SceneManager::getInstance()->getSelectionTree().getAllObjects();

    m_selected.assign( m_countLines, true );

    int activeCount( 0 );

    for( unsigned int objIdx( 0 ); objIdx < selectionObjects.size(); ++objIdx)
    {
        if( selectionObjects[objIdx]->getIsActive() )
        {
            ++activeCount;
        }
    }

    if( activeCount == 0 )
    {
        return;
    }

    m_selected = SceneManager::getInstance()->getSelectionTree().getSelectedFibers( this );

    if( m_fibersInverted )
    {
        for( int k = 0; k < m_countLines; ++k )
        {
            m_selected[k] = !m_selected[k];
        }
    }

    // TODO selection convex hull
    // This is to update the information display in the fiber grid info and the mean fiber
    /*if( boxWasUpdated && m_dh->m_lastSelectedObject != NULL )
     {
     m_dh->m_lastSelectedObject->computeConvexHull();
     }*/
}

void Fibers::initializeBuffer()
{
    if( m_isInitialized || !SceneManager::getInstance()->isUsingVBO()  )
    {
        return;
    }

    m_isInitialized = true;
    bool isOK = true;

    glGenBuffers( 3, m_bufferObjects );
    glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * m_countPoints * 3, &m_pointArray[0], GL_STATIC_DRAW );

    isOK = !Logger::getInstance()->printIfGLError( wxT( "initialize vbo points" ) );

    if( isOK )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * m_countPoints * 3, &m_colorArray[0], GL_STATIC_DRAW );

        isOK = !Logger::getInstance()->printIfGLError( wxT( "initialize vbo colors" ) );
    }

    if( isOK )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * m_countPoints * 3, &m_normalArray[0], GL_STATIC_DRAW );

        isOK = !Logger::getInstance()->printIfGLError( wxT( "initialize vbo normals" ) );
    }

    SceneManager::getInstance()->setUsingVBO( isOK );

    if( isOK )
    {
        freeArrays();
    }
    else
    {
        Logger::getInstance()->print( wxT( "Not enough memory on your gfx card. Using vertex arrays." ),            LOGLEVEL_ERROR );
        Logger::getInstance()->print( wxT( "This shouldn't concern you. Perfomance just will be slightly worse." ), LOGLEVEL_ERROR );
        Logger::getInstance()->print( wxT( "Get a better graphics card if you want more juice." ),                  LOGLEVEL_ERROR );
        glDeleteBuffers( 3, m_bufferObjects );
    }
}

//Compute T matrix for each fiber and extract the first eigen vec (t0), and the K term using eigen values.
void Fibers::computeGLobalProperties()
{
    m_tractDirection.max_size();
    m_tractDirection.resize( m_countLines * 3 );

    m_dispFactors.max_size();
    m_dispFactors.resize( m_countLines );

    m_endPointsVector.max_size();
    m_endPointsVector.resize( m_countLines * 3);

    int t = 0;
    //For each fiber
    for( int i = 0; i < m_countLines; ++i )
    {        
        
        int idx1 = getStartIndexForLine( i ) * 3;
        int idx2 = idx1+3;
        
        FMatrix T(3,3); //For watson distribution

        //Also keep end points for comparison
        int ptsperline = getPointsPerLine( i );
        Vector startPt = Vector(m_pointArray[idx1], m_pointArray[idx1+1], m_pointArray[idx1+2]);
        Vector endPt = Vector(m_pointArray[idx1+ptsperline*3-3], m_pointArray[idx1+ptsperline*3-2], m_pointArray[idx1+ptsperline*3-1]);
        Vector endDir = Vector(endPt - startPt);
        endDir.normalize();

        //for each segment
        for( int k = 0; k < getPointsPerLine( i ) - 1; ++k )
        {
            //Extract segment
            Vector localDir = Vector( m_pointArray[idx2] - m_pointArray[idx1], m_pointArray[idx2 + 1] - m_pointArray[idx1 + 1], m_pointArray[idx2 + 2] - m_pointArray[idx1 + 2]);
            localDir.normalize();

            FMatrix n(3,1);
            n(0,0) = localDir.x;
            n(1,0) = localDir.y;
            n(2,0) = localDir.z;
            
            //Perform Outter product
            FMatrix n_t = n.transposed();
            FMatrix tmp = n*n_t;
            
            //Accumulate Outter products
            T += tmp;

            idx1 += 3;
            idx2 += 3;
        } 

        //Divide Outter product by number of line
        T = T * (1.0f/(getPointsPerLine( i )));

        //Extract first eigen Vector and 3 eigen values
        std::vector< FArray > evecs;
        FArray evals( 0.0f, 0.0f, 0.0f );
        T.getEigenSystem( evals, evecs);

        //Sort
        Vector e1;
        float B1, B2, B3;
        if( evals[0] >= evals[1] && evals[0] > evals[2] )
        {
            e1.x = evecs[0][0];
            e1.y = evecs[0][1];
            e1.z = evecs[0][2];
            B1 = evals[0];
            B2 = evals[1];
            B3 = evals[2];

            if( evals[2] > evals[1])
            {
                B2 = evals[2];
                B3 = evals[1];
            }     
        }
        else if( evals[1] > evals[0] && evals[1] >= evals[2] )
        {
            e1.x = evecs[1][0];
            e1.y = evecs[1][1];
            e1.z = evecs[1][2];
            B1 = evals[1];
            B2 = evals[0];
            B3 = evals[2];

            if( evals[2] > evals[0])
            {
                B2 = evals[2];
                B3 = evals[0];
            }
        }
        else if( evals[2] >= evals[0] && evals[2] > evals[1] )
        {
            e1.x = evecs[2][0];
            e1.y = evecs[2][1];
            e1.z = evecs[2][2];
            B1 = evals[2];
            B2 = evals[0];
            B3 = evals[1];

            if( evals[1] > evals[0])
            {
                B2 = evals[0];
                B3 = evals[1];
            }
        }
        else
        {
            e1.x = evecs[0][0];
            e1.y = evecs[0][1];
            e1.z = evecs[0][2];
            B1 = evals[0];
            B2 = evals[1];
            B3 = evals[2];

            if( evals[2] > evals[1])
            {
                B2 = evals[2];
                B3 = evals[1];
            } 
        }
         
        //Compute dipersion term from 3 eigen values
        //float K = 1.0f - (sqrt(B2+B3)/2.0f*B1);
        float cl = (B1-B2)/(B1+B2+B3);

        //Save terms 
        m_tractDirection[t] = e1.x;
        m_tractDirection[t+1] = e1.y;
        m_tractDirection[t+2] = e1.z;

        m_endPointsVector[t] = endDir.x;
        m_endPointsVector[t+1] = endDir.y;
        m_endPointsVector[t+2] = endDir.z;

        //std::cout << "Evals: " << evals[0] << " " << evals[1] << " " << evals[2] << "\n";
        //std::cout << "Evecs1: " << evecs[0][0] << " " << evecs[0][1] << " " << evecs[0][2] << "\n";
        //std::cout << "Evecs2: " << evecs[1][0] << " " << evecs[1][1] << " " << evecs[1][2] << "\n";
        //std::cout << "Evecs3: " << evecs[2][0] << " " << evecs[2][1] << " " << evecs[2][2] << "\n";

        m_dispFactors[i] = cl;
        t+=3;
    }   
}

void Fibers::draw()
{
    setShader();

    if( m_cachedThreshold != m_threshold )
    {
        updateFibersColors();
        m_cachedThreshold = m_threshold;
    }

    initializeBuffer();

    if( m_useFakeTubes )
    {
        drawFakeTubes();
        return;
    }

    if( m_useTransparency && !m_useIntersectedFibers )
    {
        glPushAttrib( GL_ALL_ATTRIB_BITS );
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE );
        glDepthMask( GL_FALSE );
        drawSortedLines();
        glPopAttrib();
        return;
    }

    // If geometry shaders are supported, the shader will take care of the filtering
    // Otherwise, use the drawCrossingFibers
    if ( !SceneManager::getInstance()->isFibersGeomShaderActive() && m_useIntersectedFibers )
    {
        if( m_useTransparency )
        {
            glPushAttrib( GL_ALL_ATTRIB_BITS );
            glEnable( GL_BLEND );
            glBlendFunc( GL_ONE, GL_ONE );
            glDepthMask( GL_FALSE );
            drawCrossingFibers();
            glPopAttrib();
        }
        else
        {
            drawCrossingFibers();
        }
        return;
    }

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );

    if( !SceneManager::getInstance()->isUsingVBO() )
    {
        glVertexPointer( 3, GL_FLOAT, 0, &m_pointArray[0] );

        if( m_showFS )
        {
            glColorPointer( 3, GL_FLOAT, 0, &m_normalArray[0] );  // Global colors.
        }
        else
        {
            glColorPointer( 3, GL_FLOAT, 0, &m_normalArray[0] ); // Local colors.
        }

        glNormalPointer( GL_FLOAT, 0, &m_normalArray[0] );
    }
    else
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
        glVertexPointer( 3, GL_FLOAT, 0, 0 );

        if( m_showFS )
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
            glColorPointer( 3, GL_FLOAT, 0, 0 );
        }
        else
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
            glColorPointer( 3, GL_FLOAT, 0, 0 );
        }

        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        glNormalPointer( GL_FLOAT, 0, 0 );
    }

    for( int i = 0; i < m_countLines; ++i )
    {
        if( ( m_selected[i] || !SceneManager::getInstance()->getActivateAllSelObj() ) && !m_filtered[i] )
        {
            glDrawArrays( GL_LINE_STRIP, getStartIndexForLine( i ), getPointsPerLine( i ) );
        }
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    releaseShader();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
namespace
{
template< class T > struct IndirectComp
{
    IndirectComp( const T &zvals ) :
        zvals( zvals )
    {
    }

    // Watch out: operator less, but we are sorting in descending z-order, i.e.,
    // highest z value will be first in array and painted first as well
    template< class I > bool operator()( const I &i1, const I &i2 ) const
    {
        return zvals[i1] > zvals[i2];
    }

private:
    const T &zvals;
};
}


void Fibers::drawFakeTubes()
{
    if( ! m_normalsPositive )
    {
        switchNormals( false );
    }

    GLfloat *pColors  = NULL;
    GLfloat *pNormals = NULL;
    pColors  = &m_colorArray[0];
    pNormals = &m_normalArray[0];

    if( SceneManager::getInstance()->isPointMode() )
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    if ( m_useIntersectedFibers )
    {
        findCrossingFibers();

        for( unsigned int i = 0; i < m_cfStartOfLine.size(); ++i )
        {
            if ( 3 < m_cfPointsPerLine[i] )
            {
                int index = m_cfStartOfLine[i] * 3;
                glBegin( GL_QUAD_STRIP );

                for( unsigned int k = 0; k < m_cfPointsPerLine[i]; ++k, index += 3 )
                {
                    glNormal3f( m_normalArray[index], m_normalArray[index + 1], m_normalArray[index + 2] );
                    glColor3f( m_colorArray[index],  m_colorArray[index + 1],  m_colorArray[index + 2] );
                    glTexCoord2f( -1.0f, 0.0f );
                    glVertex3f( m_pointArray[index], m_pointArray[index + 1], m_pointArray[index + 2] );
                    glTexCoord2f( 1.0f, 0.0f );
                    glVertex3f( m_pointArray[index], m_pointArray[index + 1], m_pointArray[index + 2] );
                }

                glEnd();
            }
        }
    }
    else
    {
        for( int i = 0; i < m_countLines; ++i )
        {
            if( m_selected[i] && !m_filtered[i] )
            {
                int idx = getStartIndexForLine( i ) * 3;
                glBegin( GL_QUAD_STRIP );

                for( int k = 0; k < getPointsPerLine( i ); ++k )
                {
                    glNormal3f( pNormals[idx], pNormals[idx + 1], pNormals[idx + 2] );
                    glColor3f( pColors[idx],  pColors[idx + 1],  pColors[idx + 2] );
                    glTexCoord2f( -1.0f, 0.0f );
                    glVertex3f( m_pointArray[idx], m_pointArray[idx + 1], m_pointArray[idx + 2] );
                    glTexCoord2f( 1.0f, 0.0f );
                    glVertex3f( m_pointArray[idx], m_pointArray[idx + 1], m_pointArray[idx + 2] );
                    idx += 3;
                }

                glEnd();
            }
        }
    }
}

void Fibers::drawSortedLines()
{
    //SORT LINES
    // Only sort those lines we see.
    unsigned int *pSnippletSort = NULL;
    unsigned int *pLineIds      = NULL;

    int nbSnipplets = 0;

    // Estimate memory required for arrays.
    for( int i = 0; i < m_countLines; ++i )
    {
        if( m_selected[i] && !m_filtered[i] )
        {
            nbSnipplets += getPointsPerLine( i ) - 1;
        }
    }

    pSnippletSort = new unsigned int[nbSnipplets + 1]; // +1 just to be sure because of fancy problems with some sort functions.
    pLineIds      = new unsigned int[nbSnipplets * 2];
    // Build data structure for sorting.
    int snp = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        if( !( m_selected[i] && !m_filtered[i] ) )
        {
            continue;
        }

        const unsigned int p = getPointsPerLine( i );

        // TODO: update pLineIds and pSnippletSort size only when fiber selection changes.
        for( unsigned int k = 0; k < p - 1; ++k )
        {
            pLineIds[snp << 1] = getStartIndexForLine( i ) + k;
            pLineIds[( snp << 1 ) + 1] = getStartIndexForLine( i ) + k + 1;
            pSnippletSort[snp] = snp;
            snp++;
        }
    }

    GLfloat projMatrix[16];
    glGetFloatv( GL_PROJECTION_MATRIX, projMatrix );

    // Compute z values of lines (in our case: starting points only).
    vector< float > zVals( nbSnipplets );

    for( int i = 0; i < nbSnipplets; ++i )
    {
        const int id = pLineIds[i << 1] * 3;
        zVals[i] = ( m_pointArray[id + 0] * projMatrix[2] + m_pointArray[id + 1] * projMatrix[6]
                      + m_pointArray[id + 2] * projMatrix[10] + projMatrix[14] ) / ( m_pointArray[id + 0] * projMatrix[3]
                              + m_pointArray[id + 1] * projMatrix[7] + m_pointArray[id + 2] * projMatrix[11] + projMatrix[15] );
    }

    sort( &pSnippletSort[0], &pSnippletSort[nbSnipplets], IndirectComp< vector< float > > ( zVals ) );

    float *pColors  = NULL;
    float *pNormals = NULL;

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        pColors = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_ONLY );
        glUnmapBuffer( GL_ARRAY_BUFFER );
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        pNormals = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_ONLY );
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
    else
    {
        pColors  = &m_colorArray[0];
        pNormals = &m_normalArray[0];
    }

    if( SceneManager::getInstance()->isPointMode() )
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBegin( GL_LINES );

    if( m_fiberColorationMode == MINDISTANCE_COLOR )
    {
        int i = 0;

        for( int c = 0; c < nbSnipplets; ++c )
        {
            i = c;
            int idx  = pLineIds[pSnippletSort[i] << 1];
            int idx3 = idx * 3;
            int id2  = pLineIds[( pSnippletSort[i] << 1 ) + 1];
            int id23 = id2 * 3;
            glColor4f(  pColors[idx3 + 0],       pColors[idx3 + 1],       pColors[idx3 + 2],   m_localizedAlpha[idx] * m_alpha );
            glNormal3f( pNormals[idx3 + 0],      pNormals[idx3 + 1],      pNormals[idx3 + 2] );
            glVertex3f( m_pointArray[idx3 + 0],  m_pointArray[idx3 + 1],  m_pointArray[idx3 + 2] );
            glColor4f(  pColors[id23 + 0],       pColors[id23 + 1],       pColors[id23 + 2],   m_localizedAlpha[id2] * m_alpha );
            glNormal3f( pNormals[id23 + 0],      pNormals[id23 + 1],      pNormals[id23 + 2] );
            glVertex3f( m_pointArray[id23 + 0],  m_pointArray[id23 + 1],  m_pointArray[id23 + 2] );
        }
    }
    else
    {
        int i = 0;

        for( int c = 1; c < nbSnipplets; ++c )
        {
            i = c;
            int idx  = pLineIds[pSnippletSort[i] << 1];
            int idx3 = idx * 3;
            int id2  = pLineIds[( pSnippletSort[i] << 1 ) + 1];
            int id23 = id2 * 3;

            /* --------------OPACITY TEST -------------*/
            //View vector
            Matrix4fT transform = SceneManager::getInstance()->getTransform();
            float dots[8];
            Vector3fT v1 = { { 0, 0, 1 } };
            Vector3fT v2 = { { 1, 1, 1 } };
            Vector3fT view;

            Vector3fMultMat4( &view, &v1, &transform );
            dots[0] = Vector3fDot( &v2, &view );

            //Local vector
            Vector normalVector = Vector(m_pointArray[id23 + 0]-m_pointArray[idx3 + 0],m_pointArray[id23 + 1]-m_pointArray[idx3 + 1],m_pointArray[id23 + 2]-m_pointArray[idx3 + 2]);
            
            if(!m_isLocalRendering)
            {
                int id = getLineForPoint(idx);
                normalVector = Vector(m_tractDirection[id*3], m_tractDirection[id*3+1], m_tractDirection[id*3+2]); 

                if(m_usingEndpts)
                {
                    normalVector = Vector(m_endPointsVector[id*3], m_endPointsVector[id*3+1], m_endPointsVector[id*3+2]);
                }
            }
                
            normalVector.normalize();
            
			Vector zVector;
            if(m_axisView)
			{
                //View axis
				zVector = Vector(view.s.X, view.s.Y, view.s.Z); 
				zVector.normalize();
			}
			else
			{
                //Fixed axis
				zVector = Vector(m_xAngle,m_yAngle,m_zAngle); 
			}

			float alphaValue;
			if(m_ModeOpac)
			{
                //Transparent
                if(m_isAlphaFunc)
                {
                    //Alpha func
				    alphaValue = 1-std::abs(normalVector.Dot(zVector)); 
                    alphaValue = std::pow(alphaValue,m_exponent);
                }
                else
                {
                    //Linear func
                    float theta = std::acos(std::abs(normalVector.Dot(zVector)));

                    if(theta > (1-m_linb)/m_lina)
                    {
                        alphaValue = 1.0f;
                    }
                    else if(theta < (-m_linb)/m_lina)
                    {
                        alphaValue = 0.0f;
                    }
                    else
                    {
                        alphaValue = (m_lina*theta+m_linb);
                    }
                }
			}
			else
			{
                //Opaque
                if(m_isAlphaFunc)
                {
                    //Alpha func
				    alphaValue = std::abs(normalVector.Dot(zVector)); 
                    alphaValue = std::pow(alphaValue,m_exponent);
                }
                else
                {
                    //Linear func
                    float theta = std::acos(std::abs(normalVector.Dot(zVector)));

                    if(theta > (1-m_linb)/m_lina)
                    {
                        alphaValue = 1-1.0f;
                    }
                    else if(theta < (-m_linb)/m_lina)
                    {
                        alphaValue = 1-0.0f;
                    }
                    else
                    {
                        alphaValue = 1-m_lina*theta+m_linb;
                    }
                }
			}

            int id = getLineForPoint(idx);
            if(m_dispFactors[id] < m_cl)
                alphaValue = 1.0f;

			//glColor4f(  normalVector.x, normalVector.y, normalVector.z,   alphaValue );
            glColor4f( m_normalArray[idx3+0], m_normalArray[idx3 + 1],  m_normalArray[idx3 + 2], alphaValue );
            glNormal3f( pNormals[idx3 + 0],      pNormals[idx3 + 1],      pNormals[idx3 + 2] );
            glVertex3f( m_pointArray[idx3 + 0],  m_pointArray[idx3 + 1],  m_pointArray[idx3 + 2] );

			//glColor4f(  normalVector.x, normalVector.y, normalVector.z,   alphaValue );
            glColor4f( m_normalArray[idx3+0], m_normalArray[idx3 + 1],  m_normalArray[idx3 + 2], alphaValue );
            glNormal3f( pNormals[id23 + 0],      pNormals[id23 + 1],      pNormals[id23 + 2] );
            glVertex3f( m_pointArray[id23 + 0],  m_pointArray[id23 + 1],  m_pointArray[id23 + 2] );
        }
    }

    glEnd();
    glDisable( GL_BLEND );

    // FIXME: store these later on!
    delete[] pSnippletSort;
    delete[] pLineIds;
}

void Fibers::useFakeTubes()
{
    m_useFakeTubes = !m_useFakeTubes;
    switchNormals( m_useFakeTubes );
}

void Fibers::useTransparency()
{
    m_useTransparency = ! m_useTransparency;
}

void Fibers::drawCrossingFibers()
{
    findCrossingFibers();

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );

    if( !SceneManager::getInstance()->isUsingVBO() )
    {
        glVertexPointer( 3, GL_FLOAT, 0, &m_pointArray[0] );

        if( m_showFS )
        {
            glColorPointer( 3, GL_FLOAT, 0, &m_colorArray[0] );  // Global colors.
        }
        else
        {
            glColorPointer( 3, GL_FLOAT, 0, &m_normalArray[0] ); // Local colors.
        }

        glNormalPointer( GL_FLOAT, 0, &m_normalArray[0] );
    }
    else
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
        glVertexPointer( 3, GL_FLOAT, 0, 0 );

        if( m_showFS )
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
            glColorPointer( 3, GL_FLOAT, 0, 0 );
        }
        else
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
            glColorPointer( 3, GL_FLOAT, 0, 0 );
        }

        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        glNormalPointer( GL_FLOAT, 0, 0 );
    }

    for( unsigned int i = 0; i < m_cfStartOfLine.size(); ++i )
    {
        if ( 1 < m_cfPointsPerLine[i] )
        {
            glDrawArrays( GL_LINE_STRIP, m_cfStartOfLine[i], m_cfPointsPerLine[i] );
        }
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
}

void Fibers::switchNormals( bool positive )
{
    float *pNormals = NULL;
    pNormals = &m_normalArray[0];

    if( positive )
    {
        int pc = 0;
        float rr, gg, bb, lastX, lastY, lastZ = 0.0f;

        for( int i = 0; i < getLineCount(); ++i )
        {
            lastX = m_pointArray[pc] + ( m_pointArray[pc] - m_pointArray[pc + 3] );
            lastY = m_pointArray[pc + 1] + ( m_pointArray[pc + 1] - m_pointArray[pc + 4] );
            lastZ = m_pointArray[pc + 2] + ( m_pointArray[pc + 2] - m_pointArray[pc + 5] );

            for( int j = 0; j < getPointsPerLine( i ); ++j )
            {
                rr = lastX - m_pointArray[pc];
                gg = lastY - m_pointArray[pc + 1];
                bb = lastZ - m_pointArray[pc + 2];
                lastX = m_pointArray[pc];
                lastY = m_pointArray[pc + 1];
                lastZ = m_pointArray[pc + 2];

                if( rr < 0.0 )
                {
                    rr *= -1.0;
                }

                if( gg < 0.0 )
                {
                    gg *= -1.0;
                }

                if( bb < 0.0 )
                {
                    bb *= -1.0;
                }

                float norm = sqrt( rr * rr + gg * gg + bb * bb );
                rr *= 1.0 / norm;
                gg *= 1.0 / norm;
                bb *= 1.0 / norm;
                pNormals[pc] = rr;
                pNormals[pc + 1] = gg;
                pNormals[pc + 2] = bb;
                pc += 3;
            }
        }

        m_normalsPositive = true;
    }
    else
    {
        int pc = 0;
        float rr, gg, bb, lastX, lastY, lastZ = 0.0f;

        for( int i = 0; i < getLineCount(); ++i )
        {
            lastX = m_pointArray[pc] + ( m_pointArray[pc] - m_pointArray[pc + 3] );
            lastY = m_pointArray[pc + 1] + ( m_pointArray[pc + 1] - m_pointArray[pc + 4] );
            lastZ = m_pointArray[pc + 2] + ( m_pointArray[pc + 2] - m_pointArray[pc + 5] );

            for( int j = 0; j < getPointsPerLine( i ); ++j )
            {
                rr = lastX - m_pointArray[pc];
                gg = lastY - m_pointArray[pc + 1];
                bb = lastZ - m_pointArray[pc + 2];
                lastX = m_pointArray[pc];
                lastY = m_pointArray[pc + 1];
                lastZ = m_pointArray[pc + 2];
                pNormals[pc] = rr;
                pNormals[pc + 1] = gg;
                pNormals[pc + 2] = bb;
                pc += 3;
            }
        }

        m_normalsPositive = false;
    }
}

void Fibers::freeArrays()
{
    // Disabled for now, due to problems with glMapBuffer.
    //m_colorArray.clear();
    //m_normalArray.clear();
}

float Fibers::getPointValue( int ptIndex )
{
    return m_pointArray[ptIndex];
}

int Fibers::getLineCount()
{
    return m_countLines;
}

int Fibers::getPointCount()
{
    return m_countPoints;
}

bool Fibers::isSelected( int fiberId )
{
    return m_selected[fiberId];
}

float Fibers::getLocalizedAlpha( int index )
{
    return m_localizedAlpha[index];
}

void Fibers::setFibersLength()
{
    m_length.resize( m_countLines, false );

    vector< Vector >           currentFiberPoints;
    vector< vector< Vector > > fibersPoints;

    for( int i = 0; i < m_countLines; i++ )
    {
        if( getFiberCoordValues( i, currentFiberPoints ) )
        {
            fibersPoints.push_back( currentFiberPoints );
            currentFiberPoints.clear();
        }
    }

    float dx, dy, dz;
    m_maxLength = 0;
    m_minLength = 1000000;

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    for( unsigned int j = 0 ; j < fibersPoints.size(); j++ )
    {
        currentFiberPoints = fibersPoints[j];
        m_length[j] = 0;

        for( unsigned int i = 1; i < currentFiberPoints.size(); ++i )
        {
            // The values are in pixel, we need to set them in millimeters using the spacing
            // specified in the anatomy file ( m_datasetHelper->xVoxel... ).
            dx = ( currentFiberPoints[i].x - currentFiberPoints[i - 1].x ) * voxelX;
            dy = ( currentFiberPoints[i].y - currentFiberPoints[i - 1].y ) * voxelY;
            dz = ( currentFiberPoints[i].z - currentFiberPoints[i - 1].z ) * voxelZ;
            FArray currentVector( dx, dy, dz );
            m_length[j] += ( float )currentVector.norm();
        }

        if( m_length[j] > m_maxLength ) m_maxLength = m_length[j];

        if( m_length[j] < m_minLength ) m_minLength = m_length[j];
    }
}

bool Fibers::getFiberCoordValues( int fiberIndex, vector< Vector > &fiberPoints )
{
    int index = getStartIndexForLine( fiberIndex ) * 3;
    Vector point3D;

    for( int i = 0; i < getPointsPerLine( fiberIndex ); ++i )
    {
        point3D.x = getPointValue( index );
        point3D.y = getPointValue( index + 1 );
        point3D.z = getPointValue( index + 2 );
        fiberPoints.push_back( point3D );
        index += 3;
    }

    return true;
}

void Fibers::updateFibersFilters()
{
    m_cfDrawDirty = true;
    int min = m_pSliderFibersFilterMin->GetValue();
    int max = m_pSliderFibersFilterMax->GetValue();
    int subSampling = m_pSliderFibersSampling->GetValue();
    int maxSubSampling = m_pSliderFibersSampling->GetMax() + 1;

    updateFibersFilters(min, max, subSampling, maxSubSampling);
    m_pTxtSamplingBox->SetValue(wxString::Format( wxT( "%i"), m_subsampledLines));
}

void Fibers::updateFibersFilters(int minLength, int maxLength, int minSubsampling, int maxSubsampling)
{
    m_subsampledLines = 0;
    for( int i = 0; i < m_countLines; ++i )
    {
        if(( i % maxSubsampling ) >= minSubsampling)
        {
            m_subsampledLines += 1;
        }
        m_filtered[i] = !( ( i % maxSubsampling ) >= minSubsampling && m_length[i] >= minLength && m_length[i] <= maxLength );
    }

    SceneManager::getInstance()->getSelectionTree().notifyAllObjectsNeedUpdating();

    //Update stats, mean fiber and convexhull only if an object is selected.
    //if( pLastSelObj != NULL )
    // TODO selection convex hull
    {
        //pLastSelObj->computeConvexHull();
    }

}

vector< bool > Fibers::getFilteredFibers()
{
    return m_filtered;
}

void Fibers::flipAxis( AxisType i_axe )
{
    unsigned int i = 0;

    switch ( i_axe )
    {
        case X_AXIS:
            i = 0;
            m_pOctree->flipX();
            break;
        case Y_AXIS:
            i = 1;
            m_pOctree->flipY();
            break;
        case Z_AXIS:
            i = 2;
            m_pOctree->flipZ();
            break;
        default:
            Logger::getInstance()->print( wxT( "Cannot flip fibers. The specified axis is undefined" ), LOGLEVEL_ERROR );
            return; //No axis specified - Cannot flip
    }

    // Compute the dimension of the bounding box.
    DatasetManager *pDatMan = DatasetManager::getInstance();

    float axisShift(0.0f);

    if( i_axe == X_AXIS )
        axisShift = (pDatMan->getColumns() * pDatMan->getVoxelX()) / 2.0f;
    else if( i_axe == Y_AXIS )
        axisShift = (pDatMan->getRows() * pDatMan->getVoxelY()) / 2.0f;
    else if( i_axe == Z_AXIS )
        axisShift = (pDatMan->getFrames() * pDatMan->getVoxelZ()) / 2.0f;


    // Translate fibers at origin, flip them and move them back.
    for ( ; i < m_pointArray.size(); i += 3 )
    {
        m_pointArray[i] = -( m_pointArray[i] - axisShift ) + axisShift;
    }

    glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * m_countPoints * 3, &m_pointArray[0], GL_STATIC_DRAW );

    SceneManager::getInstance()->getSelectionTree().notifyAllObjectsNeedUpdating();
}

void Fibers::createPropertiesSizer( PropertiesWindow *pParent )
{
    DatasetInfo::createPropertiesSizer( pParent );

    setFibersLength();

    wxBoxSizer *pBoxMain = new wxBoxSizer( wxVERTICAL );

    //////////////////////////////////////////////////////////////////////////

    // Round to make sure the min and max length sliders reach the real maximal values.
    int minLength = static_cast<int>( std::floor( getMinFibersLength() ) );
    int maxLength = static_cast<int>( std::ceil( getMaxFibersLength() ) );

    m_pSliderFibersFilterMin = new wxSlider( pParent, wxID_ANY, minLength, minLength, maxLength, DEF_POS, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderFibersFilterMax = new wxSlider( pParent, wxID_ANY, maxLength, minLength, maxLength, DEF_POS, DEF_SIZE,         wxSL_HORIZONTAL | wxSL_AUTOTICKS );

    m_pSliderFibersSampling  = new wxSlider( pParent, wxID_ANY,
                                            FIBERS_SUBSAMPLING_RANGE_START,
                                            FIBERS_SUBSAMPLING_RANGE_MIN,
                                            FIBERS_SUBSAMPLING_RANGE_MAX ,
                                            wxDefaultPosition, wxSize(80, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTxtSamplingBox = new wxTextCtrl( pParent, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize(60, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pSliderInterFibersThickness = new wxSlider(  pParent, wxID_ANY, m_thickness * 4, 1, 20, DEF_POS, DEF_SIZE,         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTubeRadius = new wxSlider(  pParent, wxID_ANY, m_tubeRadius, 1, 10, DEF_POS, DEF_SIZE,         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    
    // OPACITY
    //ALPHA
    m_pSliderFibersAlpha     = new wxSlider( pParent, wxID_ANY,         30,         0,       100, wxDefaultPosition, wxSize(80, -1),         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTxtAlphaBox = new wxTextCtrl( pParent, wxID_ANY, wxT("3.0"), wxDefaultPosition, wxSize(60, -1), wxTE_CENTRE | wxTE_READONLY );

    //Linear func a
    m_pSliderFibersLina     = new wxSlider( pParent, wxID_ANY,         15,         20.0f/M_PI,       500, wxDefaultPosition, wxSize(80, -1),         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTxtlina = new wxTextCtrl( pParent, wxID_ANY, wxT("1.5"), wxDefaultPosition, wxSize(60, -1), wxTE_CENTRE | wxTE_READONLY );

    //Linear func b
    m_pSliderFibersLinb     = new wxSlider( pParent, wxID_ANY,         -9,         10.0f-M_PI*(m_pSliderFibersLina->GetValue())/2.0f,       0, wxDefaultPosition, wxSize(80, -1),         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTxtlinb = new wxTextCtrl( pParent, wxID_ANY, wxT("-0.9"), wxDefaultPosition, wxSize(60, -1), wxTE_CENTRE | wxTE_READONLY );

    //THETA
    m_pSliderFibersTheta  = new wxSlider( pParent, wxID_ANY,         90,         0,       180, wxDefaultPosition, wxSize(80, -1),         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTxtThetaBox = new wxTextCtrl( pParent, wxID_ANY, wxT("90"), wxDefaultPosition, wxSize(60, -1), wxTE_CENTRE | wxTE_READONLY );

    //PHI
    m_pSliderFibersPhi  = new wxSlider( pParent, wxID_ANY,         0,         -180,       180, wxDefaultPosition, wxSize(80, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTxtPhiBox = new wxTextCtrl( pParent, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize(60, -1), wxTE_CENTRE | wxTE_READONLY );

    //Cl
    m_pSliderFiberscl  = new wxSlider( pParent, wxID_ANY,         0,         0,       100, wxDefaultPosition, wxSize(80, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTxtclBox = new wxTextCtrl( pParent, wxID_ANY, wxT("0.00"), wxDefaultPosition, wxSize(60, -1), wxTE_CENTRE | wxTE_READONLY );

#if !_USE_LIGHT_GUI
    wxButton *pBtnGeneratesDensityVolume = new wxButton( pParent, wxID_ANY, wxT( "New Orientation Volume" ) );
#endif

    m_pToggleLocalColoring  = new wxToggleButton(   pParent, wxID_ANY, wxT( "Local Coloring" ) );
    m_pToggleNormalColoring = new wxToggleButton(   pParent, wxID_ANY, wxT( "Color With Overlay" ) );
    m_pSelectConstantFibersColor = new wxButton(    pParent, wxID_ANY, wxT( "Select Constant Color..." ) );
    m_pToggleCrossingFibers = new wxToggleButton(   pParent, wxID_ANY, wxT( "Intersected Fibers" ) );
    m_pRadNormalColoring       = new wxRadioButton( pParent, wxID_ANY, wxT( "Normal" ), DEF_POS, DEF_SIZE, wxRB_GROUP );

#if !_USE_LIGHT_GUI
    m_pRadDistanceAnchoring    = new wxRadioButton( pParent, wxID_ANY, wxT( "Dist. Anchoring" ) );
    m_pRadMinDistanceAnchoring = new wxRadioButton( pParent, wxID_ANY, wxT( "Min Dist. Anchoring" ) );
    m_pRadCurvature            = new wxRadioButton( pParent, wxID_ANY, wxT( "Curvature" ) );
    m_pRadTorsion              = new wxRadioButton( pParent, wxID_ANY, wxT( "Torsion" ) );
#endif

    m_pRadConstant             = new wxRadioButton( pParent, wxID_ANY, wxT( "Constant" ) );
    m_pToggleAxisView             = new wxToggleButton( pParent, wxID_ANY, wxT( "View Axis" ) );
	m_pToggleModeOpac			   = new wxToggleButton( pParent, wxID_ANY, wxT( "Opacity Mode" ) );
    m_pToggleRenderFunc		   = new wxToggleButton( pParent, wxID_ANY, wxT( "Power function" ) );
    m_pToggleLocalGlobal		   = new wxToggleButton( pParent, wxID_ANY, wxT( "Local rendering" ) );
    m_pToggleEndpts                 = new wxToggleButton( pParent, wxID_ANY, wxT( "End points OFF" ) );
    m_pToggleEndpts->Enable(false);

    //////////////////////////////////////////////////////////////////////////

    wxFlexGridSizer *pGridSliders1 = new wxFlexGridSizer( 2 );

    pGridSliders1->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Min Length" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders1->Add( m_pSliderFibersFilterMin, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders1->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Max Length" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders1->Add( m_pSliderFibersFilterMax, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders1->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Thickness" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders1->Add( m_pSliderInterFibersThickness, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders1->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Tube radius" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders1->Add( m_pTubeRadius, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pBoxMain->Add( pGridSliders1, 0, wxEXPAND | wxALL, 2 );

    wxBoxSizer *pBoxSampling = new wxBoxSizer( wxHORIZONTAL );
    pBoxSampling->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Subsampling" ), wxDefaultPosition, wxSize(80, -1), wxALIGN_LEFT ), 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxSampling->Add( m_pSliderFibersSampling, 0, wxALIGN_CENTER | wxALL, 1);
	pBoxSampling->Add( m_pTxtSamplingBox,   0, wxALIGN_CENTER | wxALL, 1);
    
    pBoxMain->Add(pBoxSampling, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    wxBoxSizer *pBoxcalpha = new wxBoxSizer( wxHORIZONTAL );
    pBoxcalpha->Add( new wxStaticText( pParent, wxID_ANY, wxT( "c" ), wxDefaultPosition, wxSize(80, -1), wxALIGN_LEFT ), 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxcalpha->Add( m_pSliderFibersAlpha, 0, wxALIGN_CENTER | wxALL, 1);
    pBoxcalpha->Add( m_pTxtAlphaBox,   0, wxALIGN_CENTER | wxALL, 1);
    
    pBoxMain->Add(pBoxcalpha, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    wxBoxSizer *pBoxRowlina = new wxBoxSizer( wxHORIZONTAL );
    pBoxRowlina->Add( new wxStaticText( pParent, wxID_ANY, wxT( "a" ), wxDefaultPosition, wxSize(80, -1), wxALIGN_LEFT ), 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxRowlina->Add( m_pSliderFibersLina, 0, wxALIGN_CENTER | wxALL, 1);
    pBoxRowlina->Add( m_pTxtlina,   0, wxALIGN_CENTER | wxALL, 1);
    
    pBoxMain->Add(pBoxRowlina, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    wxBoxSizer *pBoxRowlinb = new wxBoxSizer( wxHORIZONTAL );
    pBoxRowlinb->Add( new wxStaticText( pParent, wxID_ANY, wxT( "b" ), wxDefaultPosition, wxSize(80, -1), wxALIGN_LEFT ), 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxRowlinb->Add( m_pSliderFibersLinb, 0, wxALIGN_CENTER | wxALL, 1);
    pBoxRowlinb->Add( m_pTxtlinb,   0, wxALIGN_CENTER | wxALL, 1);
    
    pBoxMain->Add(pBoxRowlinb, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    wxBoxSizer *pBoxRow1 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow1->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Theta" ), wxDefaultPosition, wxSize(80, -1), wxALIGN_LEFT ), 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxRow1->Add( m_pSliderFibersTheta, 0, wxALIGN_CENTER | wxALL, 1);
    pBoxRow1->Add( m_pTxtThetaBox,   0, wxALIGN_CENTER | wxALL, 1);
    
    pBoxMain->Add(pBoxRow1, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    wxBoxSizer *pBoxRow2 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow2->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Phi" ), wxDefaultPosition, wxSize(80, -1), wxALIGN_LEFT ), 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxRow2->Add( m_pSliderFibersPhi, 0, wxALIGN_CENTER | wxALL, 1);
    pBoxRow2->Add( m_pTxtPhiBox,   0, wxALIGN_CENTER | wxALL, 1);
    
    pBoxMain->Add(pBoxRow2, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    wxBoxSizer *pBoxcl = new wxBoxSizer( wxHORIZONTAL );
    pBoxcl->Add( new wxStaticText( pParent, wxID_ANY, wxT( "T_cl" ), wxDefaultPosition, wxSize(80, -1), wxALIGN_LEFT ), 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxcl->Add( m_pSliderFiberscl, 0, wxALIGN_CENTER | wxALL, 1);
    pBoxcl->Add( m_pTxtclBox,   0, wxALIGN_CENTER | wxALL, 1);
    
    pBoxMain->Add(pBoxcl, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );


    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( m_pToggleCrossingFibers,    0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

#if !_USE_LIGHT_GUI
    pBoxMain->Add( pBtnGeneratesDensityVolume, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
#endif

    pBoxMain->Add( m_pToggleLocalColoring,     0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( m_pToggleNormalColoring,    0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( m_pSelectConstantFibersColor, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxColoring = new wxBoxSizer( wxVERTICAL );
    pBoxColoring->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Coloring:" ) ), 0, wxALIGN_LEFT | wxALL, 1 );

    wxBoxSizer *pBoxColoringRadios = new wxBoxSizer( wxVERTICAL );
    pBoxColoringRadios->Add( m_pRadNormalColoring,       0, wxALIGN_LEFT | wxALL, 1 );

#if !_USE_LIGHT_GUI
    pBoxColoringRadios->Add( m_pRadDistanceAnchoring,    0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadMinDistanceAnchoring, 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadCurvature,            0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadTorsion,              0, wxALIGN_LEFT | wxALL, 1 );
#endif

    pBoxColoringRadios->Add( m_pRadConstant,             0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoring->Add( pBoxColoringRadios, 0, wxALIGN_LEFT | wxLEFT, 32 );

    pBoxMain->Add( pBoxColoring, 0, wxFIXED_MINSIZE | wxEXPAND | wxTOP | wxBOTTOM, 8 );

    	// HERE
	wxBoxSizer *pBoxAlpha = new wxBoxSizer( wxVERTICAL );
    pBoxAlpha->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Opacity axis:" ) ), 0, wxALIGN_LEFT | wxALL, 1 );

    wxBoxSizer *pBoxViewRadios = new wxBoxSizer( wxVERTICAL );
	pBoxViewRadios->Add( m_pToggleAxisView,       0, wxALIGN_LEFT | wxALL, 1 );
    pBoxAlpha->Add( pBoxViewRadios, 0, wxALIGN_LEFT | wxLEFT, 50 );

	wxBoxSizer *pBoxOpac = new wxBoxSizer( wxVERTICAL );
    pBoxOpac->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Rendering functions:" ) ), 0, wxALIGN_LEFT | wxALL, 1 );

    wxBoxSizer *pBoxOpacBtn = new wxBoxSizer( wxVERTICAL );
	pBoxOpacBtn->Add( m_pToggleModeOpac,       0, wxALIGN_LEFT | wxALL, 1 );
    pBoxOpacBtn->Add( m_pToggleRenderFunc, 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxOpacBtn->Add( m_pToggleLocalGlobal, 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxOpacBtn->Add( m_pToggleEndpts, 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxOpac->Add( pBoxOpacBtn, 0, wxALIGN_LEFT | wxLEFT, 50 );

    pBoxMain->Add( pBoxAlpha, 0, wxFIXED_MINSIZE | wxEXPAND | wxTOP | wxBOTTOM, 8 );
	pBoxMain->Add( pBoxOpac, 0, wxFIXED_MINSIZE | wxEXPAND | wxTOP | wxBOTTOM, 8 );


    //////////////////////////////////////////////////////////////////////////

    m_pPropertiesSizer->Add( pBoxMain, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////
    // Connect widgets with callback function
    pParent->Connect( m_pSliderFibersFilterMin->GetId(),         wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    pParent->Connect( m_pSliderFibersFilterMax->GetId(),         wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    pParent->Connect( m_pSliderFibersSampling->GetId(),          wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    pParent->Connect( m_pSliderInterFibersThickness->GetId(),    wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnCrossingFibersThicknessChange ) );
    pParent->Connect( m_pTubeRadius->GetId(),                    wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnTubeRadius ) );
    pParent->Connect( m_pSliderFibersAlpha->GetId(),             wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersAlpha ) );
    pParent->Connect( m_pSliderFibersTheta->GetId(),           wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersAlpha ) );
    pParent->Connect( m_pSliderFibersPhi->GetId(),           wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersAlpha ) );
    pParent->Connect( m_pSliderFibersLina->GetId(),           wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersAlpha ) );
    pParent->Connect( m_pSliderFibersLinb->GetId(),           wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersAlpha ) );
    pParent->Connect( m_pSliderFiberscl->GetId(),           wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersAlpha ) );
    pParent->Connect( m_pToggleLocalColoring->GetId(),           wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnToggleUseTex ) );
    pParent->Connect( m_pToggleNormalColoring->GetId(),          wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(        PropertiesWindow::OnToggleShowFS ) );
    pParent->Connect( m_pSelectConstantFibersColor->GetId(),     wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnSelectConstantColor ) );
    pParent->Connect( m_pToggleCrossingFibers->GetId(),          wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(        PropertiesWindow::OnToggleCrossingFibers ) );
    pParent->Connect( m_pRadNormalColoring->GetId(),             wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnNormalColoring ) );

#if !_USE_LIGHT_GUI
    pParent->Connect( m_pRadDistanceAnchoring->GetId(),          wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnListMenuDistance ) );
    pParent->Connect( m_pRadMinDistanceAnchoring->GetId(),       wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnListMenuMinDistance ) );
    pParent->Connect( m_pRadTorsion->GetId(),                    wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnColorWithTorsion ) );
    pParent->Connect( m_pRadCurvature->GetId(),                  wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnColorWithCurvature ) );
#endif

    pParent->Connect( m_pRadConstant->GetId(),                   wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnColorWithConstantColor ) );
    pParent->Connect( m_pToggleAxisView->GetId(),                   wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnAxisChange ) );
	pParent->Connect( m_pToggleModeOpac->GetId(),                   wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnModeOpacChange ) );
    pParent->Connect( m_pToggleRenderFunc->GetId(),                 wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnRenderFuncChange ) );
    pParent->Connect( m_pToggleLocalGlobal->GetId(),                 wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnLocalGlobalChange ) );
    pParent->Connect( m_pToggleEndpts->GetId(),                 wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnEndPtsChange ) );


#if !_USE_LIGHT_GUI
    pParent->Connect( pBtnGeneratesDensityVolume->GetId(),
                      wxEVT_COMMAND_BUTTON_CLICKED,
                      wxCommandEventHandler( PropertiesWindow::OnGenerateFiberVolume ) );
#endif

    m_pRadNormalColoring->SetValue( true );

    updateFibersFilters();
}

void Fibers::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();

    m_pSliderOpacity->Enable( false );
    m_pSliderInterFibersThickness->Enable( m_useIntersectedFibers );
    m_pToggleFiltering->Enable( false );
    m_pToggleCrossingFibers->Enable( true );
    m_pRadNormalColoring->Enable(       getShowFS() );

#if !_USE_LIGHT_GUI
    m_pRadDistanceAnchoring->Enable(    getShowFS() );
    m_pRadMinDistanceAnchoring->Enable( getShowFS() );
    m_pRadCurvature->Enable(            getShowFS() );
    m_pRadTorsion->Enable(              getShowFS() );
#endif

    m_pRadConstant->Enable(             getShowFS() );

    m_pToggleFiltering->SetValue( false );
    m_pToggleCrossingFibers->SetValue( m_useIntersectedFibers );
    m_pSliderOpacity->SetValue( m_pSliderOpacity->GetMin() );
    m_pToggleNormalColoring->SetValue( !getShowFS() );
    m_pSliderThresholdIntensity->SetValue( getThreshold() * 100 );
    m_pSliderOpacity->SetValue( getAlpha() * 100 );

    // Hide temporarily opacity and intensity functionalities.
    m_pSliderOpacity->Hide();
    m_pOpacityText->Hide();
    m_pIntensityText->Hide();
    m_pSliderThresholdIntensity->Hide();

    if( m_isColorationUpdated )
    {
        m_pRadNormalColoring->SetValue( m_fiberColorationMode == NORMAL_COLOR );

#if !_USE_LIGHT_GUI
        m_pRadDistanceAnchoring->SetValue( m_fiberColorationMode == DISTANCE_COLOR );
        m_pRadMinDistanceAnchoring->SetValue( m_fiberColorationMode == MINDISTANCE_COLOR );
        m_pRadTorsion->SetValue( m_fiberColorationMode == TORSION_COLOR );
        m_pRadCurvature->SetValue( m_fiberColorationMode == CURVATURE_COLOR );
#endif

        m_pRadConstant->SetValue( m_fiberColorationMode == CONSTANT_COLOR );
        m_isColorationUpdated = false;
    }

    DatasetInfo* pDatasetInfo = NULL;

    long nextItemId = MyApp::frame->getCurrentListIndex();

    if( nextItemId >= 0)
    {
        pDatasetInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( nextItemId ) );
        if( pDatasetInfo != NULL)
        {
            if(pDatasetInfo->getType() != FIBERS)
            {
                DatasetInfo::m_pBtnDown->Disable();
            }
            else
            {
                DatasetInfo::m_pBtnDown->Enable();
            }
        }
        else
        {
            DatasetInfo::m_pBtnDown->Disable();
        }
    }
    else
    {
        DatasetInfo::m_pBtnDown->Disable();
    }

    long prevItemId = MyApp::frame->getCurrentListIndex() - 1;

    if( prevItemId != -1)
    {
        pDatasetInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( prevItemId ) );
        if( pDatasetInfo != NULL)
        {
            if(pDatasetInfo->getType() != FIBERS)
            {
                DatasetInfo::m_pBtnUp->Disable();
            }
            else
            {
                DatasetInfo::m_pBtnUp->Enable();
            }
        }
        else
        {
            DatasetInfo::m_pBtnUp->Disable();
        }
    }
}

bool Fibers::toggleShow()
{
    SceneManager::getInstance()->getSelectionTree().notifyAllObjectsNeedUpdating();
	DatasetInfo::toggleShow();
	if(getShow())
	{
		SceneManager::getInstance()->setSelBoxChanged(true);
	}
	return getShow();
}

//////////////////////////////////////////////////////////////////////////

void Fibers::updateCrossingFibersThickness()
{
    if ( NULL != m_pSliderInterFibersThickness )
    {
        m_thickness = m_pSliderInterFibersThickness->GetValue() * 0.25f;
        m_cfDrawDirty = true;
    }
}

//////////////////////////////////////////////////////////////////////////

void Fibers::updateTubeRadius()
{
    if ( NULL != m_pTubeRadius )
    {
        m_tubeRadius = m_pTubeRadius->GetValue();
    }
}

//////////////////////////////////////////////////////////////////////////

void Fibers::findCrossingFibers()
{
    if (   m_cfDrawDirty
        || m_xDrawn != SceneManager::getInstance()->getSliceX()
        || m_yDrawn != SceneManager::getInstance()->getSliceY()
        || m_zDrawn != SceneManager::getInstance()->getSliceZ()
        || m_axialShown    != SceneManager::getInstance()->isAxialDisplayed()
        || m_coronalShown  != SceneManager::getInstance()->isCoronalDisplayed()
        || m_sagittalShown != SceneManager::getInstance()->isSagittalDisplayed() )
    {
        m_xDrawn = SceneManager::getInstance()->getSliceX();
        m_yDrawn = SceneManager::getInstance()->getSliceY();
        m_zDrawn = SceneManager::getInstance()->getSliceZ();
        m_axialShown    = SceneManager::getInstance()->isAxialDisplayed();
        m_coronalShown  = SceneManager::getInstance()->isCoronalDisplayed();
        m_sagittalShown = SceneManager::getInstance()->isSagittalDisplayed();

        float xVoxSize = DatasetManager::getInstance()->getVoxelX();
        float yVoxSize = DatasetManager::getInstance()->getVoxelY();
        float zVoxSize = DatasetManager::getInstance()->getVoxelZ();

        m_cfDrawDirty = true;

        // Determine X, Y and Z range
        const float xMin( (m_xDrawn + 0.5f) * xVoxSize - m_thickness );
        const float xMax( (m_xDrawn + 0.5f) * xVoxSize + m_thickness );
        const float yMin( (m_yDrawn + 0.5f) * yVoxSize - m_thickness );
        const float yMax( (m_yDrawn + 0.5f) * yVoxSize + m_thickness );
        const float zMin( (m_zDrawn + 0.5f) * zVoxSize - m_thickness );
        const float zMax( (m_zDrawn + 0.5f) * zVoxSize + m_thickness );

        bool lineStarted(false);

        m_cfStartOfLine.clear();
        m_cfPointsPerLine.clear();

        unsigned int index( 0 );
        unsigned int point( 0 );
        for ( unsigned int line( 0 ); line < static_cast<unsigned int>(m_countLines); ++line )
        {
            if ( m_selected[line] && !m_filtered[line] )
            {
                for ( unsigned int i( 0 ); i < static_cast<unsigned int>(getPointsPerLine(line)); ++i, ++point, index += 3 )
                {
                    if ( m_sagittalShown && xMin <= m_pointArray[index] && xMax >= m_pointArray[index] )
                    {
                        if ( !lineStarted )
                        {
                            m_cfStartOfLine.push_back(point);
                            m_cfPointsPerLine.push_back(0);
                            lineStarted = true;
                        }
                        ++m_cfPointsPerLine.back();
                    }
                    else if ( m_coronalShown && yMin <= m_pointArray[index + 1] && yMax >= m_pointArray[index + 1] )
                    {
                        if ( !lineStarted )
                        {
                            m_cfStartOfLine.push_back(point);
                            m_cfPointsPerLine.push_back(0);
                            lineStarted = true;
                        }
                        ++m_cfPointsPerLine.back();
                    }
                    else if ( m_axialShown && zMin <= m_pointArray[index + 2] && zMax >= m_pointArray[index + 2] )
                    {
                        if ( !lineStarted )
                        {
                            m_cfStartOfLine.push_back(point);
                            m_cfPointsPerLine.push_back(0);
                            lineStarted = true;
                        }
                        ++m_cfPointsPerLine.back();
                    }
                    else
                    {
                        lineStarted = false;
                    }
                }
                lineStarted = false;
            }
            else
            {
                point += getPointsPerLine(line);
                index += getPointsPerLine(line) * 3;
            }
        }
    }
}

void Fibers::setShader()
{
    DatasetInfo *pDsInfo = (DatasetInfo*) this;

    if( m_useFakeTubes )
    {
        ShaderHelper::getInstance()->getFakeTubesShader()->bind();
        ShaderHelper::getInstance()->getFakeTubesShader()->setUniInt  ( "globalColor", getShowFS() );
        ShaderHelper::getInstance()->getFakeTubesShader()->setUniFloat( "dimX", (float)MyApp::frame->m_pMainGL->GetSize().x );
        ShaderHelper::getInstance()->getFakeTubesShader()->setUniFloat( "dimY", (float)MyApp::frame->m_pMainGL->GetSize().y );
        ShaderHelper::getInstance()->getFakeTubesShader()->setUniFloat( "thickness", GLfloat( m_tubeRadius ) );
    }
    else if( SceneManager::getInstance()->isFibersGeomShaderActive() && m_useIntersectedFibers )
    {
        // Determine X, Y and Z range
        int curSliceX = SceneManager::getInstance()->getSliceX();
        int curSliceY = SceneManager::getInstance()->getSliceY();
        int curSliceZ = SceneManager::getInstance()->getSliceZ();

        float xVoxSize = DatasetManager::getInstance()->getVoxelX();
        float yVoxSize = DatasetManager::getInstance()->getVoxelY();
        float zVoxSize = DatasetManager::getInstance()->getVoxelZ();

        const float xMin( ( curSliceX + 0.5f ) * xVoxSize - m_thickness );
        const float xMax( ( curSliceX + 0.5f ) * xVoxSize + m_thickness );
        const float yMin( ( curSliceY + 0.5f ) * yVoxSize - m_thickness );
        const float yMax( ( curSliceY + 0.5f ) * yVoxSize + m_thickness );
        const float zMin( ( curSliceZ + 0.5f ) * zVoxSize - m_thickness );
        const float zMax( ( curSliceZ + 0.5f ) * zVoxSize + m_thickness );

        ShaderHelper::getInstance()->getCrossingFibersShader()->bind();

        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("xMin", SceneManager::getInstance()->isSagittalDisplayed() ? xMin : 0 );
        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("xMax", SceneManager::getInstance()->isSagittalDisplayed() ? xMax : 0 );

        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("yMin", SceneManager::getInstance()->isCoronalDisplayed() ? yMin : 0 );
        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("yMax", SceneManager::getInstance()->isCoronalDisplayed() ? yMax : 0 );

        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("zMin", SceneManager::getInstance()->isAxialDisplayed() ? zMin : 0 );
        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("zMax", SceneManager::getInstance()->isAxialDisplayed() ? zMax : 0 );
    }
    else if ( !m_useTex )
    {
        ShaderHelper::getInstance()->getFibersShader()->bind();
        ShaderHelper::getInstance()->setFiberShaderVars();
        ShaderHelper::getInstance()->getFibersShader()->setUniInt( "useTex", !pDsInfo->getUseTex() );
         ShaderHelper::getInstance()->getFibersShader()->setUniInt( "useColorMap", SceneManager::getInstance()->getColorMap() );
        ShaderHelper::getInstance()->getFibersShader()->setUniInt( "useOverlay", pDsInfo->getShowFS() );
    }
}

void Fibers::releaseShader()
{
    if( m_useFakeTubes )
    {
        ShaderHelper::getInstance()->getFakeTubesShader()->release();
    }
    else if( SceneManager::getInstance()->isFibersGeomShaderActive() && m_useIntersectedFibers )
    {
        ShaderHelper::getInstance()->getCrossingFibersShader()->release();
    }
    else if( !m_useTex )
    {
        ShaderHelper::getInstance()->getFibersShader()->release();
    }
}

void Fibers::convertFromRTT( std::vector<std::vector<Vector> >* RTT )
{
    // the list of points
    vector< vector< float > > lines;
    m_countPoints = 0;
    float back, front;

    for( unsigned int i = 0; i < RTT->size() - 1; i+=2 )
    {
		if( RTT->size() > 0 )
		{
			back = RTT->at(i).size();
			front = RTT->at(i+1).size();
            unsigned int nbpoints;

            if( front == 0 )
            {
                nbpoints = back;
            }
            else if( back == 0 )
            {
                nbpoints = front;
            }
            else
            {
			    nbpoints = back + front - 1;
            }

            vector< float > curLine;
			curLine.resize( nbpoints * 3 );
            int skipFirst = 0;

			if( back > 0 )
			{
                skipFirst = 1;
				//back
				for( int j = back - 1; j >= 0; j-- )
				{
					curLine[j * 3]  = RTT->at(i)[back - 1 - j].x;
					curLine[j * 3 + 1] = RTT->at(i)[back - 1 - j].y;
					curLine[j * 3 + 2] = RTT->at(i)[back - 1 - j].z;
				}
            }

            if( front > 0 )
            {
				//front
				for( unsigned int j = back, k = 0+skipFirst; j < nbpoints, k < front; j++, k++ )
				{
					curLine[j * 3]  = RTT->at(i+1)[k].x;
					curLine[j * 3 + 1] = RTT->at(i+1)[k].y;
					curLine[j * 3 + 2] = RTT->at(i+1)[k].z;
				}
				
            }

            m_countPoints += curLine.size() / 3;
			lines.push_back( curLine );
		}
    }

    //set all the data in the right format for the navigator
    m_countLines = lines.size();
    m_pointArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_linePointers[0] = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i + 1] = m_linePointers[i] + lines[i].size() / 3;
    }

    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    unsigned int pos = 0;
    vector< vector< float > >::iterator it;

    for( it = lines.begin(); it < lines.end(); it++ )
    {
        vector< float >::iterator it2;

        for( it2 = ( *it ).begin(); it2 < ( *it ).end(); it2++ )
        {
            m_pointArray[pos++] = *it2;
        }
    }

    createColorArray( false );
    m_type = FIBERS;
    m_fullPath = MyApp::frame->m_pMainGL->m_pRealTimeFibers->getRTTFileName();

	wxString id = wxString::Format(_T("%d"), RTTrackingHelper::getInstance()->generateId());
    m_name = wxT( "RTTFibers" + id );

	m_pOctree = new Octree( 2, m_pointArray, m_countPoints );
}
void Fibers::updateAlpha()
{
    m_exponent = m_pSliderFibersAlpha->GetValue() / 10.0f;

    float phi = m_pSliderFibersPhi->GetValue() * M_PI / 180.0f;
    float theta = m_pSliderFibersTheta->GetValue() * M_PI / 180.0f;

    m_xAngle = std::cos(phi)*std::sin(theta);
    m_yAngle = std::sin(phi)*std::sin(theta);
    m_zAngle = std::cos(theta);

    m_lina = m_pSliderFibersLina->GetValue()/10.0f;
    m_linb = m_pSliderFibersLinb->GetValue()/10.0f;

    m_cl = m_pSliderFiberscl->GetValue()/100.0f;

    //Linear b change
    m_pSliderFibersLinb->SetMin(10.0f-M_PI*(m_pSliderFibersLina->GetValue())/2.0f);

    //Boxes
    m_pTxtAlphaBox->SetValue(wxString::Format( wxT( "%.1f"), m_pSliderFibersAlpha->GetValue()/10.0f));
    m_pTxtThetaBox->SetValue(wxString::Format( wxT( "%i"), m_pSliderFibersTheta->GetValue()));
    m_pTxtPhiBox->SetValue(wxString::Format( wxT( "%i"), m_pSliderFibersPhi->GetValue()));
    m_pTxtlina->SetValue(wxString::Format( wxT( "%.1f"), m_pSliderFibersLina->GetValue()/10.0f));
    m_pTxtlinb->SetValue(wxString::Format( wxT( "%.1f"), m_pSliderFibersLinb->GetValue()/10.0f));
    m_pTxtclBox->SetValue(wxString::Format( wxT( "%.2f"), m_pSliderFiberscl->GetValue()/100.0f));
}

void Fibers::setAxisView(bool value)
{
	m_axisView = !value;
	if(value)
		m_pToggleAxisView->SetLabel( wxT("Fixed axis") );
	else
		m_pToggleAxisView->SetLabel( wxT("View axis") );
}

void Fibers::setModeOpac(bool value)
{
	m_ModeOpac = !value;
	if(value)
        m_pToggleModeOpac->SetLabel( wxT("Transparent mode") );
	else
		m_pToggleModeOpac->SetLabel( wxT("Opacity mode") );
}

void Fibers::setRenderFunc(bool value)
{
	m_isAlphaFunc = !value;
	if(value)
        m_pToggleRenderFunc->SetLabel( wxT("Linear function") );
	else
		m_pToggleRenderFunc->SetLabel( wxT("Power function") );
}

void Fibers::setLocalGlobal(bool value)
{
	m_isLocalRendering = !value;
	if(value)
    {
        m_pToggleLocalGlobal->SetLabel( wxT("Global rendering") );
        m_pToggleEndpts->Enable(true);
    }
	else
    {
	    m_pToggleLocalGlobal->SetLabel( wxT("Local rendering") );
        m_pToggleEndpts->Enable(false);
    }
}

void Fibers::setUsingEndpts(bool value)
{
	m_usingEndpts = value;
	if(value)
		m_pToggleEndpts->SetLabel( wxT("End points ON") );
	else
		m_pToggleEndpts->SetLabel( wxT("End points OFF") );
}

void PropertiesWindow::OnFibersAlpha( wxCommandEvent& WXUNUSED( event ) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnFibersFilter" ), LOGLEVEL_DEBUG );

    DatasetIndex index = MyApp::frame->m_pListCtrl->GetItem( MyApp::frame->getCurrentListIndex() );

    Fibers* pTmpFib = DatasetManager::getInstance()->getSelectedFibers( index );
    if( pTmpFib != NULL )
    {
        pTmpFib->updateAlpha();  
    }
}