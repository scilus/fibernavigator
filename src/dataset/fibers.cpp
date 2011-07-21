/////////////////////////////////////////////////////////////////////////////
// Name:            fibers.cpp
// Author:          ---
// Creation Date:   ---
//
// Description: This is the implementation file for fibers class.
//
// Last modifications:
//      by : ggirard - 29/12/2010
/////////////////////////////////////////////////////////////////////////////

#include "fibers.h"

#include <iostream>
#include <fstream>
#include <cfloat>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <wx/tokenzr.h>

#include "Anatomy.h"
#include "../main.h"

#define LINEAR_GRADIENT_THRESHOLD 0.085f
#define MIN_ALPHA_VALUE 0.017f

Fibers::Fibers( DatasetHelper* i_datasetHelper ) :
    DatasetInfo( i_datasetHelper )
{
    m_isInitialized         = false;
    m_normalsPositive       = false;
    m_bufferObjects         = new GLuint[3];
    m_cachedThreshold       = 0.0f;
    m_isSpecialFiberDisplay = false;
}

Fibers::~Fibers()
{
    m_dh->printDebug( _T( "executing fibers destructor" ), 1 );
    m_dh->m_fibersLoaded = false;

    if( m_dh->m_useVBO )
        glDeleteBuffers( 3, m_bufferObjects );

    m_pointArray.clear();
    m_normalArray.clear();
    m_colorArray.clear();

    if( m_kdTree )
        delete m_kdTree;

    m_lineArray.clear();
    m_linePointers.clear();
    m_reverse.clear();
}

bool Fibers::load( wxString i_filename )
{
   bool res = false;
   if( i_filename.AfterLast( '.' ) == _T( "fib" ) ){
        if (loadVTK( i_filename )){
            res = true;
        } else {
            res = loadDmri(i_filename);  
        }
   }
   if( i_filename.AfterLast( '.' ) == _T( "bundlesdata" ) )
      res = loadPTK( i_filename );
   
   if( i_filename.AfterLast( '.' ) == _T( "Bfloat" ) )
      res = loadCamino( i_filename );
   
   if( i_filename.AfterLast( '.' ) == _T( "trk" ) )
      res = loadTRK( i_filename );

    return res;
}

bool Fibers::loadTRK(wxString i_fileName)
{
	stringstream ss;
	m_dh->printDebug(wxT("Start loading TRK file..."), 1 );

    wxFile l_dataFile;
    wxFileOffset l_nSize = 0;
    converterByteINT16 l_cbi;
    converterByteINT32 l_cbi32;
    converterByteFloat l_cbf;

    if(!l_dataFile.Open(i_fileName)) return false;
    
    l_nSize = l_dataFile.Length();
    if( l_nSize == wxInvalidOffset ) return false;

    ////
    // READ HEADER
    ////

    //Read file header. [1000 bytes]
    wxUint8* l_buffer = new wxUint8[1000];
    l_dataFile.Read( l_buffer, (size_t)1000 );

    //ID String for track file. The first 5 characters must match "TRACK". [6 bytes]
    char id_string[6];
    memcpy(id_string, &l_buffer[0], 6);
	ss.str("");
	ss << "Type: " << id_string;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    if (strncmp(id_string, "TRACK", 5) != 0) return false;    
    
    //Dimension of the image volume. [6 bytes]
    wxUint16 dim[3];
    for (int i=0; i!=3;++i)
    {
        memcpy(l_cbi.b, &l_buffer[6 + (i*2)], 2);
        dim[i] = l_cbi.i;
    }
	ss.str("");
	ss << "Dim: " << dim[0] << "x" << dim[1] << "x" << dim[2];
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    //Voxel size of the image volume. [12 bytes]
    float voxel_size[3];
    for (int i=0; i!=3;++i)
    {
        memcpy(l_cbf.b, &l_buffer[12 + (i*4)], 4);
        voxel_size[i] = l_cbf.f;
    }
	ss.str("");
	ss << "Voxel size: " << voxel_size[0] << "x" << voxel_size[1] << "x" << voxel_size[2];
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    //Origin of the image volume. [12 bytes]
    float origin[3];
    for (int i=0; i!=3;++i)
    {
        memcpy(l_cbf.b, &l_buffer[24 + (i*4)], 4);
        origin[i] = l_cbf.f;
    }
	ss.str("");
	ss << "Origin: (" << origin[0] << "," << origin[1] << "," << origin[2] << ")";
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    //Number of scalars saved at each track point. [2 bytes]
    wxUint16 n_scalars;
    memcpy(l_cbi.b, &l_buffer[36], 2);
    n_scalars = l_cbi.i;
	ss.str("");
	ss << "Nb. scalars: " << n_scalars;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    //Name of each scalar. (20 characters max each, max 10 names) [200 bytes]
    char scalar_name[10][20];
    memcpy(scalar_name, &l_buffer[38], 200);
    for (int i=0; i!=10;++i)
	{
		ss.str("");
		ss << "Scalar name #" << i << ": " << scalar_name[i];
		m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
	}

    //Number of properties saved at each track. [2 bytes]
    wxUint16 n_properties;
    memcpy(l_cbi.b, &l_buffer[238], 2);
    n_properties = l_cbi.i;
	ss.str("");
	ss << "Nb. properties: " << n_properties;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Name of each property. (20 characters max each, max 10 names) [200 bytes]
    char property_name[10][20];
    memcpy(property_name, &l_buffer[240], 200);
    for (int i=0; i!=10;++i)
	{
		ss.str("");
		ss << "Property name #" << i << ": " << property_name[i];
	}

    //4x4 matrix for voxel to RAS (crs to xyz) transformation. 
    // If vox_to_ras[3][3] is 0, it means the matrix is not recorded.
    // This field is added from version 2. [64 bytes]
    float vox_to_ras[4][4];
    for (int i=0; i != 4; ++i)
    {
		ss.str("");
        for (int j=0; j != 4; ++j)
        {
            memcpy(l_cbf.b, &l_buffer[440 + (i*4+j)], 4);
            vox_to_ras[i][j] = l_cbf.f;
            ss << vox_to_ras[i][j] << " ";
        }
        ss;
		m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    }

    //Reserved space for future version. [444 bytes]
    char reserved[444]; //Not used.
    //l_buffer[504]...

    //Storing order of the original image data. [4 bytes]
    char voxel_order[4];
    memcpy(voxel_order, &l_buffer[948], 4);
	ss.str("");
	ss << "Voxel order: " << voxel_order;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    //Paddings [4 bytes]
    char pad2[4];
    memcpy(pad2, &l_buffer[952], 4);
	ss.str("");
	ss << "Pad #2: " << pad2;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    //Image orientation of the original image. As defined in the DICOM header. [24 bytes]
    float image_orientation_patient[6];
	ss.str("");
    ss << "Image orientation patient: ";
    for (int i=0; i != 6; ++i)
    {
        memcpy(l_cbf.b, &l_buffer[956 + (i*4)], 4);
        image_orientation_patient[i] = l_cbf.f;
        ss << image_orientation_patient[i] << " ";
    }
	ss;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    //Paddings. [2 bytes]
    char pad1[2];
    memcpy(pad1, &l_buffer[980], 2);
	ss.str("");
	ss << "Pad #1: " << pad1;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invert_x = l_buffer[982] > 0;
	ss.str("");
	ss << "Invert X: " << invert_x;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invert_y = l_buffer[983] > 0;
	ss.str("");
	ss << "Invert Y: " << invert_y;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invert_z = l_buffer[984] > 0;
	ss.str("");
	ss << "Invert Z: " << invert_z;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swap_xy = l_buffer[985] > 0;
	ss.str("");
	ss << "Swap XY: " << swap_xy;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swap_yz = l_buffer[986] > 0;
	ss.str("");
	ss << "Swap YZ: " << swap_yz;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swap_zx = l_buffer[987] > 0;
	ss.str("");
	ss << "Swap ZX: " << swap_zx;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Number of tracks stored in this track file. 0 means the number was NOT stored. [4 bytes]
    wxUint32 n_count;
    memcpy(l_cbi32.b, &l_buffer[988], 4);
    n_count = l_cbi32.i;
	ss.str("");
	ss << "Nb. tracks: " << n_count;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
    
    //Version number. Current version is 2. [4 bytes]
    wxUint32 version;
    memcpy(l_cbi32.b, &l_buffer[992], 4);
    version = l_cbi32.i;
	ss.str("");
	ss << "Version: " << version;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    //Size of the header. Used to determine byte swap. Should be 1000. [4 bytes]
    wxUint32 hdr_size;
    memcpy(l_cbi32.b, &l_buffer[996], 4);
    hdr_size = l_cbi32.i;
	ss.str("");
	ss << "HDR size: " << hdr_size;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

	////
    // READ DATA
    ////
    delete[] l_buffer;
    l_buffer = NULL;
	
    vector<float> l_tmpPoints;

    if (n_count == 0) return false; //TODO: handle it. (0 means the number was NOT stored.)
    
    vector<vector<float> > lines;
    m_countPoints = 0;

    for (int i=0; i!=n_count; ++i)
    {
        //Number of points in this track. [4 bytes]
        wxUint32 n_points;
        l_dataFile.Read(l_cbi32.b, (size_t)4);
        n_points = l_cbi32.i;

        //Read data of one track.
        size_t ptsSize = 3+n_scalars;
        size_t tractSize = 4*(n_points*(ptsSize) + n_properties);
        l_buffer = new wxUint8[tractSize];
        l_dataFile.Read(l_buffer, tractSize);

        vector<float> cur_line;
        for (int j=0; j!=n_points; ++j)
        {
            //Read coordinates (x,y,z) and scalars associated to each point.
            for (int k=0; k!=ptsSize; ++k)
            {
                if (k > 3) break; //TODO: incorporate scalars in the navigator.
                memcpy(l_cbf.b, &l_buffer[4*(j*ptsSize+k)], 4);
                cur_line.push_back(l_cbf.f);
            }
        }

        for (int j=0; j!=n_properties; ++j) {} //TODO: incorporate properties in the navigator.

        m_countPoints += cur_line.size()/3;
        lines.push_back(cur_line);

        delete[] l_buffer;
        l_buffer = NULL;
    }

    l_dataFile.Close();

	////
    //POST PROCESS: set all the data in the right format for the navigator
	////
    m_dh->printDebug( wxT( "Setting data in right format for the navigator..." ), 1 );

    m_countLines = lines.size();
    m_dh->m_countFibers = m_countLines;   
    m_pointArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    
	ss.str("");
	ss << "m_countLines: " << m_countLines;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
	ss.str("");
	ss << "m_countPoints: " << m_countPoints;
	m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );

    m_linePointers[0] = 0;
    for( int i = 0; i < m_countLines; ++i )
        m_linePointers[i+1] = m_linePointers[i]+ lines[i].size()/3;

    int l_lineCounter = 0;
    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[l_lineCounter + 1] )
            ++l_lineCounter;
        m_reverse[i] = l_lineCounter;
    }

    unsigned int pos=0;
    vector< vector<float> >::iterator it;
    for (it=lines.begin(); it<lines.end(); it++){
        vector<float>::iterator it2;
        for (it2=(*it).begin(); it2<(*it).end(); it2++){
            m_pointArray[pos++] = *it2;
        }
    }

	if (voxel_size[0] == 0 && voxel_size[1] == 0 && voxel_size[2] == 0)
	{
		ss.str("");
		ss << "Using anatomy's voxel size: [" << m_dh->m_xVoxel << "," << m_dh->m_yVoxel << "," << m_dh->m_zVoxel << "]";
		m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
		voxel_size[0] = m_dh->m_xVoxel;
		voxel_size[1] = m_dh->m_yVoxel;
		voxel_size[2] = m_dh->m_zVoxel;

		ss.str("");
		ss << "Centering with respect to the anatomy: [" << m_dh->m_columns/2 << "," << m_dh->m_rows/2 << "," << m_dh->m_frames/2 << "]";
		m_dh->printDebug(wxString(ss.str().c_str(), wxConvUTF8), 1 );
		origin[0] = m_dh->m_columns/2;
		origin[1] = m_dh->m_rows/2;
		origin[2] = m_dh->m_frames/2;
	}

    int flipX = (!invert_x)? 1: -1;
    int flipY = (!invert_y)? 1: -1;
    int flipZ = (!invert_z)? 1: -1;

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        m_pointArray[i] = ((flipX * m_pointArray[i] + origin[0]) / voxel_size[0]) * m_dh->m_xVoxel;
        ++i;
        m_pointArray[i] = ((flipY * m_pointArray[i] + origin[1]) / voxel_size[1]) * m_dh->m_yVoxel;
        ++i;
        m_pointArray[i] = ((flipZ * m_pointArray[i] + origin[2]) / voxel_size[2]) * m_dh->m_zVoxel;
    }

	m_dh->printDebug(wxT("End loading TRK file"), 1 );

    createColorArray( false );
    m_type = FIBERS;
    m_fullPath = i_fileName;
    m_kdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
#ifdef __WXMSW__
    m_name = i_fileName.AfterLast( '\\' );
#else
    m_name = i_fileName.AfterLast( '/' );
#endif
    return true;
}


bool Fibers::loadCamino( wxString i_filename )
{
    m_dh->printDebug( _T( "start loading Camino file" ), 1 );

    wxFile l_dataFile;
    wxFileOffset l_nSize = 0;

    if( l_dataFile.Open( i_filename ) )
    {
        l_nSize = l_dataFile.Length();
    
        if( l_nSize == wxInvalidOffset )
            return false;
    }

    wxUint8* l_buffer = new wxUint8[l_nSize];
    l_dataFile.Read( l_buffer, l_nSize );

    l_dataFile.Close();

    m_countLines  = 0; // Number of lines.
    m_countPoints = 0; // Number of points.

    int l_cl = 0;
    int l_pc = 0;
    converterByteFloat l_cbf;
    vector< float > l_tmpPoints;

    while( l_pc < l_nSize )
    {
        ++m_countLines;

        l_cbf.b[3] = l_buffer[l_pc++];
        l_cbf.b[2] = l_buffer[l_pc++];
        l_cbf.b[1] = l_buffer[l_pc++];
        l_cbf.b[0] = l_buffer[l_pc++];

        l_cl = (int)l_cbf.f;
        m_lineArray.push_back( l_cl );

        l_pc += 4;
        for( int i = 0; i < l_cl; ++i )
        {
            m_lineArray.push_back( m_countPoints );
            ++m_countPoints;

            l_cbf.b[3] = l_buffer[l_pc++];
            l_cbf.b[2] = l_buffer[l_pc++];
            l_cbf.b[1] = l_buffer[l_pc++];
            l_cbf.b[0] = l_buffer[l_pc++];
            l_tmpPoints.push_back( l_cbf.f );

            l_cbf.b[3] = l_buffer[l_pc++];
            l_cbf.b[2] = l_buffer[l_pc++];
            l_cbf.b[1] = l_buffer[l_pc++];
            l_cbf.b[0] = l_buffer[l_pc++];
            l_tmpPoints.push_back( l_cbf.f );

            l_cbf.b[3] = l_buffer[l_pc++];
            l_cbf.b[2] = l_buffer[l_pc++];
            l_cbf.b[1] = l_buffer[l_pc++];
            l_cbf.b[0] = l_buffer[l_pc++];
            l_tmpPoints.push_back( l_cbf.f );

            if( l_pc > l_nSize )
                break;
        }
    }

    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[m_countLines] = m_countPoints;

    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize(m_countLines, false);
    m_pointArray.resize( l_tmpPoints.size() );

    for( size_t i = 0; i < l_tmpPoints.size(); ++i )
        m_pointArray[i] = l_tmpPoints[i];

    printf( "%d lines and %d points \n", m_countLines, m_countPoints );

    m_dh->printDebug( _T( "move vertices" ), 1 );

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        m_pointArray[i] = m_dh->m_columns * m_dh->m_xVoxel - m_pointArray[i];
        ++i;
        m_pointArray[i] = m_dh->m_rows * m_dh->m_yVoxel - m_pointArray[i];
        ++i;
        m_pointArray[i] = m_dh->m_frames * m_dh->m_zVoxel - m_pointArray[i];
    }

    calculateLinePointers();
    createColorArray( false );
    m_dh->printDebug( _T( "read all" ), 1 );

    delete[] l_buffer;

    m_dh->m_countFibers = m_countLines;

    m_type = FIBERS;
    m_fullPath = i_filename;

#ifdef __WXMSW__
    m_name = i_filename.AfterLast('\\');
#else
    m_name = i_filename.AfterLast( '/' );
#endif

    m_kdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );

    return true;
}

bool Fibers::loadPTK( wxString l_fileName )
{
    m_dh->printDebug( _T( "start loading PTK file" ), 1 );
    wxFile l_dataFile;
    wxFileOffset l_nSize = 0;
    int l_pc = 0;
    converterByteINT32 l_cbi;
    converterByteFloat l_cbf;
    vector< float > l_tmpPoints;

    if( l_dataFile.Open( l_fileName ) )
    {
        l_nSize = l_dataFile.Length();
        if( l_nSize == wxInvalidOffset )
            return false;
    }

    wxUint8* l_buffer = new wxUint8[l_nSize];
    l_dataFile.Read( l_buffer, l_nSize );

    m_countLines  = 0; // Number of lines.
    m_countPoints = 0; // Number of points.

    while( l_pc < l_nSize )
    {
        ++m_countLines;

        l_cbi.b[0] = l_buffer[l_pc++];
        l_cbi.b[1] = l_buffer[l_pc++];
        l_cbi.b[2] = l_buffer[l_pc++];
        l_cbi.b[3] = l_buffer[l_pc++];

        m_lineArray.push_back( l_cbi.i );

        for( size_t i = 0; i < l_cbi.i; ++i )
        {
            m_lineArray.push_back( m_countPoints );
            ++m_countPoints;

            l_cbf.b[0] = l_buffer[l_pc++];
            l_cbf.b[1] = l_buffer[l_pc++];
            l_cbf.b[2] = l_buffer[l_pc++];
            l_cbf.b[3] = l_buffer[l_pc++];
            l_tmpPoints.push_back( l_cbf.f );

            l_cbf.b[0] = l_buffer[l_pc++];
            l_cbf.b[1] = l_buffer[l_pc++];
            l_cbf.b[2] = l_buffer[l_pc++];
            l_cbf.b[3] = l_buffer[l_pc++];
            l_tmpPoints.push_back( l_cbf.f );

            l_cbf.b[0] = l_buffer[l_pc++];
            l_cbf.b[1] = l_buffer[l_pc++];
            l_cbf.b[2] = l_buffer[l_pc++];
            l_cbf.b[3] = l_buffer[l_pc++];
            l_tmpPoints.push_back( l_cbf.f );
        }

    }

    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize(m_countLines, false);

    m_pointArray.resize( l_tmpPoints.size() );

    for( size_t i = 0; i < l_tmpPoints.size(); ++i )
        m_pointArray[i] = l_tmpPoints[i];

    printf( "%d lines and %d points \n", m_countLines, m_countPoints );
    m_dh->printDebug( _T( "move vertices" ), 1 );


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
    for( int i = 0; i < m_countPoints * 3; ++i )
    {
       m_pointArray[i] = m_dh->m_columns* m_dh->m_xVoxel - m_pointArray[i];
       ++i;
       m_pointArray[i] = m_dh->m_rows   * m_dh->m_yVoxel - m_pointArray[i];
       ++i;
       m_pointArray[i] = m_dh->m_frames * m_dh->m_zVoxel - m_pointArray[i];
    }
    calculateLinePointers();
    createColorArray( false );
    m_dh->printDebug( _T( "read all" ), 1 );

    delete[] l_buffer;

    m_dh->m_countFibers = m_countLines;
    m_type = FIBERS;
    m_fullPath = l_fileName;

#ifdef __WXMSW__
    m_name = l_fileName.AfterLast('\\');
#else
    m_name = l_fileName.AfterLast( '/' );
#endif

    m_kdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
    return true;
}

bool Fibers::loadVTK( wxString i_fileName )
{
    m_dh->printDebug( _T( "start loading VTK file" ), 1 );
    wxFile l_dataFile;
    wxFileOffset l_nSize = 0;

    if( l_dataFile.Open( i_fileName ) )
    {
        l_nSize = l_dataFile.Length();

        if ( l_nSize == wxInvalidOffset )
            return false;
    }

    wxUint8* l_buffer = new wxUint8[255];
    l_dataFile.Read( l_buffer, (size_t)255 );

    int l_pointOffset         = 0;
    int l_lineOffset          = 0;
    int l_pointColorOffset    = 0;
    int l_lineColorOffset     = 0;
    int l_fileOffset          = 0;
    int l_j                   = 0;
    bool colorsLoadedFromFile = false;
    char* l_temp              = new char[256];

    // Ignore the first 3 lines.
    while( l_buffer[l_fileOffset] != '\n' )
        ++l_fileOffset;

    ++l_fileOffset;

    while( l_buffer[l_fileOffset] != '\n' )
        ++l_fileOffset;

    ++l_fileOffset;

    // Check the file type.
    while( l_buffer[l_fileOffset] != '\n' )
    {
        l_temp[l_j] = l_buffer[l_fileOffset];
        ++l_fileOffset;
        ++l_j;
    }
    ++l_fileOffset;

    l_temp[l_j] = 0;
    wxString type( l_temp, wxConvUTF8 );

    if( type == wxT( "ASCII" ) )
        // ASCII file, maybe later.
        return false;

    if( type != wxT( "BINARY" ) )
        // Something else, don't know what to do.
        return false;

    // Ignore line DATASET POLYDATA.
    while( l_buffer[l_fileOffset] != '\n' )
        ++l_fileOffset;
    ++l_fileOffset;
    
    l_j = 0;
    // Read POINTS.
    while( l_buffer[l_fileOffset] != '\n' )
    {
        l_temp[l_j] = l_buffer[l_fileOffset];
        ++l_fileOffset;
        ++l_j;
    }
    ++l_fileOffset;

    l_temp[l_j] = 0;
    wxString l_points( l_temp, wxConvUTF8 );

    l_points = l_points.AfterFirst( ' ' );
    l_points = l_points.BeforeFirst( ' ' );

    long l_tempValue = 0;

    if( ! l_points.ToLong( &l_tempValue, 10 ) )
        return false; // Can't read point count.

    int l_countPoints = (int)l_tempValue;

    // Start position of the point array in the file.
    l_pointOffset = l_fileOffset;

    // Jump to postion after point array.
    l_fileOffset += ( 12 * l_countPoints ) + 1;
    l_j = 0;
    l_dataFile.Seek( l_fileOffset );
    l_dataFile.Read( l_buffer, (size_t) 255 );

    while( l_buffer[l_j] != '\n' )
    {
        l_temp[l_j] = l_buffer[l_j];
        ++l_fileOffset;
        ++l_j;
    }
    ++l_fileOffset;

    l_temp[l_j] = 0;

    wxString l_sLines( l_temp, wxConvUTF8 );
    wxString l_sLengthLines = l_sLines.AfterLast( ' ' );

    if( ! l_sLengthLines.ToLong( &l_tempValue, 10 ) )
        return false; // Can't read size of lines array.

    int l_lengthLines = ( int( l_tempValue ) );
    l_sLines = l_sLines.AfterFirst( ' ' );
    l_sLines = l_sLines.BeforeFirst( ' ' );

    if( ! l_sLines.ToLong( &l_tempValue, 10 ) )
        return false; // Can't read lines.

    int countLines = (int) l_tempValue;

    // Start postion of the line array in the file.
    l_lineOffset = l_fileOffset;

    // Jump to postion after line array.
    l_fileOffset += ( l_lengthLines * 4 ) + 1;
    l_dataFile.Seek( l_fileOffset );
    l_dataFile.Read( l_buffer, (size_t) 255 );

    l_j = 0;
    int l_k = 0;

    // TODO test if there's really a color array.
    while( l_buffer[l_k] != '\n' )
    {
        l_temp[l_j] = l_buffer[l_k];
        ++l_fileOffset;
        ++l_j;
        ++l_k;
    }
    ++l_k;
    ++l_fileOffset;
    l_temp[l_j] = 0;
    wxString tmpString( l_temp, wxConvUTF8 );

    l_j = 0;
    while( l_buffer[l_k] != '\n' )
    {
        l_temp[l_j] = l_buffer[l_k];
        ++l_fileOffset;
        ++l_j;
        ++l_k;
    }
    ++l_fileOffset;
    l_temp[l_j] = 0;
    wxString tmpString2( l_temp, wxConvUTF8 );

    if( tmpString.BeforeFirst( ' ' ) == _T( "CELL_DATA" ) )
    {
        l_lineColorOffset = l_fileOffset;
        l_fileOffset += ( countLines * 3 ) + 1;
        l_dataFile.Seek( l_fileOffset );
        l_dataFile.Read( l_buffer, (size_t) 255 );

        // aa 2009/06/26 workaround if the l_buffer doesn't contain a string.
        l_buffer[254] = '\n';

        int l_k = l_j = 0;
        // TODO test if there's really a color array.
        while( l_buffer[l_k] != '\n' )
        {
            l_temp[l_j] = l_buffer[l_k];
            ++l_fileOffset;
            ++l_j;
            ++l_k;
        }
        ++l_k;
        ++l_fileOffset;
        l_temp[l_j] = 0;
        wxString tmpString3( l_temp, wxConvUTF8 );
        tmpString = tmpString3;

        l_j = 0;
        while( l_buffer[l_k] != '\n' )
        {
            l_temp[l_j] = l_buffer[l_k];
            ++l_fileOffset;
            ++l_j;
            ++l_k;
        }
        ++l_fileOffset;
        l_temp[l_j] = 0;
        wxString tmpString4( l_temp, wxConvUTF8 );
        tmpString2 = tmpString4;
    }

    if( tmpString.BeforeFirst( ' ' ) == _T( "POINT_DATA" ) && tmpString2.BeforeFirst( ' ' ) == _T( "COLOR_SCALARS" ) )
        l_pointColorOffset = l_fileOffset;

    m_dh->printDebug( wxString::Format( _T( "loading %d l_points and %d lines." ), l_countPoints, countLines ), 1 );

    m_countLines        = countLines;
    m_dh->m_countFibers = m_countLines;
    m_countPoints       = l_countPoints;

    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[countLines] = l_countPoints;
    m_reverse.resize( l_countPoints );
    m_filtered.resize(countLines, false);
    m_selected.resize( countLines, false );

    m_pointArray.resize( l_countPoints * 3 );
    m_lineArray.resize ( l_lengthLines * 4 );
    m_colorArray.resize( l_countPoints * 3 );

    l_dataFile.Seek( l_pointOffset );
    l_dataFile.Read( &m_pointArray[0], (size_t)l_countPoints * 12 );

    l_dataFile.Seek( l_lineOffset );
    l_dataFile.Read( &m_lineArray[0], (size_t)l_lengthLines * 4 );

    if( l_pointColorOffset != 0 )
    {
        vector< wxUint8 > tmpColorArray( l_countPoints * 3, 0 );
        l_dataFile.Seek( l_pointColorOffset );
        l_dataFile.Read( &tmpColorArray[0], (size_t) l_countPoints * 3 );
        
        for( size_t i = 0; i < tmpColorArray.size(); ++i )
        {
            m_colorArray[i] = tmpColorArray[i] / 255.;
        }
        colorsLoadedFromFile = true;
    }
    toggleEndianess();

    m_dh->printDebug( _T( "move vertices" ), 1 );

    for( int i = 0; i < l_countPoints * 3; ++i )
    {
        m_pointArray[i] = m_dh->m_columns * m_dh->m_xVoxel - m_pointArray[i];
        ++i;
        m_pointArray[i] = m_dh->m_rows    * m_dh->m_yVoxel - m_pointArray[i];
        ++i;
        //m_pointArray[i] = m_dh->m_frames - m_pointArray[i];
    }

    calculateLinePointers();
    createColorArray( colorsLoadedFromFile );
    m_dh->printDebug( _T( "read all" ), 1 );

    m_type      = FIBERS;
    m_fullPath  = i_fileName;

#ifdef __WXMSW__
    m_name = i_fileName.AfterLast( '\\' );
#else
    m_name = i_fileName.AfterLast( '/' );
#endif

    m_kdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );

    delete[] l_buffer;
    delete[] l_temp;
    return true;
}

bool Fibers::loadDmri(wxString i_fileName)
{
    FILE *l_file;    
    
    l_file = fopen(i_fileName.mb_str(),"r");
    if (l_file == NULL) return false;
    char *s1 = new char[10];
    char *s2 = new char[10];
    char *s3 = new char[10];
    char *s4 = new char[10];
    float f1,f2,f3,f4,f5;
    int res;
    // the header
    res = fscanf(l_file, "%f %s", &f1, s1);
    res = fscanf(l_file, "%f %s %s %s %s", &f1, s1, s2, s3, s4);
    res = fscanf(l_file, "%f", &f1);
    res = fscanf(l_file, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5);    
    res = fscanf(l_file, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5);
    res = fscanf(l_file, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5);
    res = fscanf(l_file, "%d %f", &m_countLines, &f2);
    
    // the list of points
    vector< vector<float> > lines;
    m_countPoints = 0;
    float back,front;
    for (int i=0; i<m_countLines; i++){        
        res = fscanf(l_file, "%f %f %f", &back, &front, &f1);
        
        int nbpoints = back+front;
        if (back!=0 && front!=0)
        {
            nbpoints--;
        }
        if (nbpoints>0){        
            vector<float> cur_line;
            
            cur_line.resize(nbpoints*3);  
            
            //back
            for(int j=back-1;j>=0;j--){
                res = fscanf(l_file, "%f %f %f %f", &f1, &f2, &f3, &f4);                
                cur_line[j*3]  = f1;
                cur_line[j*3+1]= f2;
                cur_line[j*3+2]= f3;                
            }
            if (back !=0 && front!=0)
            {
                //repeated pts
                res = fscanf(l_file, "%f %f %f %f", &f1, &f2, &f3, &f4);  
            }
            //front    
            for (int j=back;j<nbpoints;j++){
                res = fscanf(l_file, "%f %f %f %f", &f1, &f2, &f3, &f4);                    
                cur_line[j*3]  = f1;
                cur_line[j*3+1]= f2;
                cur_line[j*3+2]= f3;
            }            
            m_countPoints += cur_line.size()/3;
            lines.push_back(cur_line);            
        } 
    }
    fclose(l_file);
    //set all the data in the right format for the navigator
    m_countLines = lines.size();
    m_dh->m_countFibers = m_countLines + 1;   
    m_pointArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );

    m_linePointers[0] = 0;
    for( int i = 0; i < m_countLines; ++i )
        m_linePointers[i+1] = m_linePointers[i]+ lines[i].size()/3;
    
    int l_lineCounter = 0;
    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[l_lineCounter + 1] )
            ++l_lineCounter;
        m_reverse[i] = l_lineCounter;
    }

    unsigned int pos=0;
    vector< vector<float> >::iterator it;
    for (it=lines.begin(); it<lines.end(); it++){
        vector<float>::iterator it2;
        for (it2=(*it).begin(); it2<(*it).end(); it2++){
            m_pointArray[pos++] = *it2;
        }
    }
    
    createColorArray( false );
    m_type = FIBERS;
    m_fullPath = i_fileName;
    m_kdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
    #ifdef __WXMSW__
    m_name = i_fileName.AfterLast( '\\' );
#else
    m_name = i_fileName.AfterLast( '/' );
#endif
    return true;
}


///////////////////////////////////////////////////////////////////////////
// This function was made for debug purposes, it will create a fake set of 
// fibers with hardcoded value to be able to test different things.
///////////////////////////////////////////////////////////////////////////
void Fibers::loadTestFibers()
{
    m_countLines        = 2;  // The number of fibers you want to display.
    int l_lengthLines   = 10; // The number of points each fiber will have.
    int pos = 0;

    m_dh->m_countFibers = m_countLines;
    m_countPoints       = m_countLines * l_lengthLines;

    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize(m_countLines, false);

    // Because you need to load an anatomy file first in order to load this fake set of fibers, 
    // the points composing your fibers have to be between [0,159] in x, [0,199] in y and [0,159] in z.
    // This is for a straight line.
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 100.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 110.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 120.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 130.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 140.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 150.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 160.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 170.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 180.0f; m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f; m_pointArray[pos++] = 190.0f; m_pointArray[pos++] = 60.0f;

    // This is for a circle in 2D (Z never changes).
    float l_circleRadius = 10.0f;
    float l_offset       = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI *  0.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI *  0.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 10.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 10.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 20.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 20.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 30.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 30.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 40.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 40.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 50.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 50.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 60.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 60.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 70.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 70.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 80.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 80.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = l_circleRadius * sin( M_PI * 90.0f / 180.0f ) + l_offset; m_pointArray[pos++] = l_circleRadius * cos( M_PI * 90.0f / 180.0f ) + l_offset; m_pointArray[pos++] = 100.0f;

    
    // No need to modify the rest of this function if you only want to add a test fiber.
    for( int i = 0; i < m_countLines; ++i )
        m_linePointers[i] = i * l_lengthLines;
    
    int l_lineCounter = 0;
    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[l_lineCounter + 1] )
            ++l_lineCounter;

        m_reverse[i] = l_lineCounter;
    }

    m_pointArray.resize( m_countPoints * 3 );

    createColorArray( false );

    m_type = FIBERS;

    m_kdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
}

///////////////////////////////////////////////////////////////////////////
// This function will call the proper coloring function for the fibers.
///////////////////////////////////////////////////////////////////////////
void Fibers::updateFibersColors()
{    
    if( m_dh->m_fiberColorationMode == NORMAL_COLOR )
    {
        resetColorArray();
    }
    else
    {
        float* l_colorData    = NULL;
        if( m_dh->m_useVBO )
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
            l_colorData  = (float *) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
        }
        else
        {
            l_colorData  = &m_colorArray[0];
        }

        if( m_dh->m_fiberColorationMode == CURVATURE_COLOR )
        {
            colorWithCurvature( l_colorData );
        }
        else if( m_dh->m_fiberColorationMode == TORSION_COLOR )
        {
            colorWithTorsion( l_colorData );
        }
        else if( m_dh->m_fiberColorationMode == DISTANCE_COLOR)
        {
            colorWithDistance( l_colorData );
        }
        else if( m_dh->m_fiberColorationMode == MINDISTANCE_COLOR)
        {
            colorWithMinDistance( l_colorData);
        }

        if( m_dh->m_useVBO )
        {
            glUnmapBuffer( GL_ARRAY_BUFFER );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their torsion value.
//
// i_colorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithTorsion( float* i_colorData )
{
    if( i_colorData == NULL )
        return;

    int    l_pc = 0;
    Vector l_firstDerivative, l_secondDerivative, l_thirdDerivative;

    // For each fibers.
    for( int i = 0; i < getLineCount(); ++i )
    {
        double l_color        = 0.0f;
        int    l_index        = 0;
        float  l_progression  = 0.0f;
        int    l_pointPerLine = getPointsPerLine( i );
        
        // We cannot calculate the torsion for a fiber that as less that 5 points. 
        // So we simply do not cange the color for this fiber
        if( l_pointPerLine < 5 )
            continue;

        // For each points of this fiber.
        for( int j = 0; j < l_pointPerLine; ++j )
        {
            if     ( j == 0 )                  { l_index = 6;                          l_progression = 0.0f;  } // For the first point of each fiber.
            else if( j == 1 )                  { l_index = 6;                          l_progression = 0.25f; } // For the second point of each fiber.
            else if( j == l_pointPerLine - 2 ) { l_index = ( l_pointPerLine - 2 ) * 3; l_progression = 0.75f; } // For the before last point of each fiber.
            else if( j == l_pointPerLine - 1 ) { l_index = ( l_pointPerLine - 2 ) * 3; l_progression = 1.0f;  } // For the last point of each fiber.
            else                               {                                       l_progression = 0.5f;  } // For every other points.
            
            m_dh->m_lastSelectedObject->getProgressionTorsion( Vector( m_pointArray[l_index-6], m_pointArray[l_index-5], m_pointArray[l_index-4] ), 
                                                               Vector( m_pointArray[l_index-3], m_pointArray[l_index-2], m_pointArray[l_index-1] ),
                                                               Vector( m_pointArray[l_index],   m_pointArray[l_index+1], m_pointArray[l_index+2] ),
                                                               Vector( m_pointArray[l_index+3], m_pointArray[l_index+4], m_pointArray[l_index+5] ),
                                                               Vector( m_pointArray[l_index+6], m_pointArray[l_index+7], m_pointArray[l_index+8] ),
                                                               l_progression, l_color );

            // Lets apply a specific harcoded coloration for the torsion.
            float l_realColor;
            if( l_color <= 0.01f ) // Those points have no torsion so we simply but them pure blue.
            {
                i_colorData[l_pc]     = 0.0f;
                i_colorData[l_pc + 1] = 0.0f;
                i_colorData[l_pc + 2] = 1.0f;
            }
            else if( l_color < 0.1f  ) // The majority of the values are here.
            {
                double l_normalizedValue = ( l_color - 0.01f ) / ( 0.1f - 0.01f );
                l_realColor = ( pow( (double)2.71828182845904523536, l_normalizedValue ) ) - 1.0f;

                i_colorData[l_pc]     = 0.0f;
                i_colorData[l_pc + 1] = l_realColor;
                i_colorData[l_pc + 2] = 1.0f - l_realColor;
            }
            else // All the rest is simply pure green.
            {
                i_colorData[l_pc]     = 0.0f;
                i_colorData[l_pc + 1] = 1.0f;
                i_colorData[l_pc + 2] = 0.0f;
            }

            l_pc    += 3;
            l_index += 3;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their curvature value.
//
// i_colorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithCurvature( float* i_colorData )
{
    if( i_colorData == NULL )
        return;
    
    int    l_pc = 0;
    Vector l_firstDerivative, l_secondDerivative, l_thirdDerivative;

    // For each fibers.
    for( int i = 0; i < getLineCount(); ++i )
    {
        double l_color        = 0.0f;
        int    l_index        = 0;
        float  l_progression  = 0.0f;
        int    l_pointPerLine = getPointsPerLine( i );
        
        // We cannot calculate the curvature for a fiber that as less that 5 points. 
        // So we simply do not cange the color for this fiber
        if( l_pointPerLine < 5 )
            continue;

        // For each points of this fiber.
        for( int j = 0; j < l_pointPerLine; ++j )
        {
            if     ( j == 0 )                  { l_index = 6;                          l_progression = 0.0f;  } // For the first point of each fiber.
            else if( j == 1 )                  { l_index = 6;                          l_progression = 0.25f; } // For the second point of each fiber.
            else if( j == l_pointPerLine - 2 ) { l_index = ( l_pointPerLine - 2 ) * 3; l_progression = 0.75f; } // For the before last point of each fiber.
            else if( j == l_pointPerLine - 1 ) { l_index = ( l_pointPerLine - 2 ) * 3; l_progression = 1.0f;  } // For the last point of each fiber.
            else                               {                                       l_progression = 0.5f;  } // For every other points.
            
            m_dh->m_lastSelectedObject->getProgressionCurvature( Vector( m_pointArray[l_index-6], m_pointArray[l_index-5], m_pointArray[l_index-4] ), 
                                                                 Vector( m_pointArray[l_index-3], m_pointArray[l_index-2], m_pointArray[l_index-1] ),
                                                                 Vector( m_pointArray[l_index],   m_pointArray[l_index+1], m_pointArray[l_index+2] ),
                                                                 Vector( m_pointArray[l_index+3], m_pointArray[l_index+4], m_pointArray[l_index+5] ),
                                                                 Vector( m_pointArray[l_index+6], m_pointArray[l_index+7], m_pointArray[l_index+8] ),
                                                                 l_progression, l_color );

            // Lets apply a specific harcoded coloration for the curvature.
            float l_realColor;
            if( l_color <= 0.01f ) // Those points have no curvature so we simply but them pure blue.
            {
                i_colorData[l_pc]     = 0.0f;
                i_colorData[l_pc + 1] = 0.0f;
                i_colorData[l_pc + 2] = 1.0f;
            }
            else if( l_color < 0.1f  ) // The majority of the values are here.
            {
                double l_normalizedValue = ( l_color - 0.01f ) / ( 0.1f - 0.01f );
                l_realColor = ( pow( (double)2.71828182845904523536, l_normalizedValue ) ) - 1.0f;

                i_colorData[l_pc]     = 0.0f;
                i_colorData[l_pc + 1] = l_realColor;
                i_colorData[l_pc + 2] = 1.0f - l_realColor;
            }
            else // All the rest is simply pure green.
            {
                i_colorData[l_pc]     = 0.0f;
                i_colorData[l_pc + 1] = 1.0f;
                i_colorData[l_pc + 2] = 0.0f;
            }

            l_pc    += 3;
            l_index += 3;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their distance to the
// flagged distance anchors voi.
//
// i_colorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithDistance( float* i_colorData )
{
    SelectionObjectList selObjs =  m_dh->getSelectionObjects();

    vector<SelectionObject*> simplifiedList;

    for(unsigned int i = 0; i < selObjs.size(); ++i)
    {
        for(unsigned int j = 0; j < selObjs[i].size(); ++j)
        {
            if( selObjs[i][j]->IsUsedForDistanceColoring())
            {
                simplifiedList.push_back(selObjs[i][j]); 
            }
        }
    }

    for(int i = 0; i < getPointCount(); ++i)
    {
        float minDistance = FLT_MAX;

        int l_x     = (int)wxMin( m_dh->m_columns -1, wxMax( 0, m_pointArray[i * 3 ]/ m_dh->m_xVoxel    ) ) ;
        int l_y     = (int)wxMin( m_dh->m_rows    -1, wxMax( 0, m_pointArray[i * 3 + 1]/ m_dh->m_yVoxel ) ) ;
        int l_z     = (int)wxMin( m_dh->m_frames  -1, wxMax( 0, m_pointArray[i * 3 + 2]/ m_dh->m_zVoxel ) ) ;

        int l_index = l_x + l_y * m_dh->m_columns + l_z * m_dh->m_rows * m_dh->m_columns;

        for(unsigned int j = 0; j < simplifiedList.size(); ++j)
        {
            if(simplifiedList[j]->m_sourceAnatomy != NULL)
            {
                float Value = simplifiedList[j]->m_sourceAnatomy->at( l_index );

                if( Value < minDistance)
                {
                    minDistance = Value;
                }
            }
        }

        float thresh = m_threshold/2.0f;

        if(minDistance > (thresh) && minDistance < (thresh + LINEAR_GRADIENT_THRESHOLD))
        {
            float Green = (minDistance-thresh)/ LINEAR_GRADIENT_THRESHOLD;
            float Red = 1 - Green;

            i_colorData[3 * i]      = Red;
            i_colorData[3 * i + 1]  = Green;
            i_colorData[3 * i + 2]  = 0.0f;
        }
        else if(minDistance > (thresh + LINEAR_GRADIENT_THRESHOLD))
        {
            i_colorData[3 * i ]     = 0.0f;
            i_colorData[3 * i + 1]  = 1.0f;
            i_colorData[3 * i + 2]  = 0.0f;
        }
        else
        {
            i_colorData[3 * i ]     = 1.0f;
            i_colorData[3 * i + 1]  = 0.0f;
            i_colorData[3 * i + 2]  = 0.0f;
        }
    }
}

void Fibers::colorWithMinDistance( float* i_colorData )
{
    SelectionObjectList selObjs =  m_dh->getSelectionObjects();

    vector<SelectionObject*> simplifiedList;

    for(unsigned int i = 0; i < selObjs.size(); ++i)
    {
        for(unsigned int j = 0; j < selObjs[i].size(); ++j)
        {
            if( selObjs[i][j]->IsUsedForDistanceColoring())
            {
                simplifiedList.push_back(selObjs[i][j]); 
            }
        }
    }

    for(int i = 0; i < getLineCount(); ++i)
    {
        int NbPointsInLine = getPointsPerLine(i);
        int Index = getStartIndexForLine(i);

        float minDistance = FLT_MAX;

        for(int j = 0; j < NbPointsInLine; ++j)
        {
            int l_x     = (int)wxMin( m_dh->m_columns -1, wxMax( 0, m_pointArray[(Index + j) * 3 ]/ m_dh->m_xVoxel    ) ) ;
            int l_y     = (int)wxMin( m_dh->m_rows    -1, wxMax( 0, m_pointArray[(Index + j) * 3 + 1]/ m_dh->m_yVoxel ) ) ;
            int l_z     = (int)wxMin( m_dh->m_frames  -1, wxMax( 0, m_pointArray[(Index + j) * 3 + 2]/ m_dh->m_zVoxel ) ) ;

            int l_index = l_x + l_y * m_dh->m_columns + l_z * m_dh->m_rows * m_dh->m_columns;

            for(unsigned int k = 0; k < simplifiedList.size(); ++k)
            {
                float Value = simplifiedList[k]->m_sourceAnatomy->at( l_index );

                if( Value < minDistance)
                {
                    minDistance = Value;
                } 
            }
        }

        float thresh = m_threshold/2.0f;
        Vector theColor;
        float theAlpha;

        if(m_localizedAlpha.size() != (unsigned int) getPointCount())
        {
            m_localizedAlpha = vector<float>(getPointCount());
        }

        if(minDistance > (thresh) && minDistance < (thresh + LINEAR_GRADIENT_THRESHOLD))
        {
            float Green = (minDistance-thresh)/ LINEAR_GRADIENT_THRESHOLD;
            float Red = 1 - Green;

            theColor.x  = Red;
            theColor.y  = 0.9f;
            theColor.z  = 0.0f;

            if(Red < MIN_ALPHA_VALUE)
            {
                theAlpha = MIN_ALPHA_VALUE;
            }
            else
            {
                theAlpha = pow(Red,6.0f);
            }
        }
        else if(minDistance > (thresh + LINEAR_GRADIENT_THRESHOLD))
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

        for(int j = 0; j < NbPointsInLine; ++j)
        {
            i_colorData[(Index + j)*3]     = theColor.x;
            i_colorData[(Index + j)*3 + 1] = theColor.y;
            i_colorData[(Index + j)*3 + 2] = theColor.z;

            m_localizedAlpha[Index + j] = theAlpha;
        }
    }
}

void Fibers::generateFiberVolume()
{
    float* l_colorData    = NULL;
    if( m_dh->m_useVBO )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        l_colorData  = (float *) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        l_colorData  = &m_colorArray[0];
    }

    if(m_localizedAlpha.size() != (unsigned int)getPointCount())
    {
        m_localizedAlpha = vector<float>(getPointCount(),1);
    }

    Anatomy* tmpAnatomy = new Anatomy(m_dh,RGB);

    tmpAnatomy->setName(wxT("Fiber-Density Volume"));

    m_dh->m_mainFrame->m_listCtrl->InsertItem(0, wxT(""),0);
    m_dh->m_mainFrame->m_listCtrl->SetItem(0,1, tmpAnatomy->getName());
    m_dh->m_mainFrame->m_listCtrl->SetItem(0,2, wxT("1.0"));
    m_dh->m_mainFrame->m_listCtrl->SetItem(0,3, wxT(""),1);

    m_dh->m_mainFrame->m_listCtrl->SetItemData(0,(long) tmpAnatomy);

    m_dh->m_mainFrame->m_listCtrl->SetItemState(0,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);

    m_dh->updateLoadStatus();
    m_dh->m_mainFrame->refreshAllGLWidgets();

    for(int i = 0; i < getPointCount(); ++i)
    {
        int l_x     = (int)wxMin( m_dh->m_columns -1, wxMax( 0, m_pointArray[i * 3 ]/ m_dh->m_xVoxel    ) ) ;
        int l_y     = (int)wxMin( m_dh->m_rows    -1, wxMax( 0, m_pointArray[i * 3 + 1]/ m_dh->m_yVoxel ) ) ;
        int l_z     = (int)wxMin( m_dh->m_frames  -1, wxMax( 0, m_pointArray[i * 3 + 2]/ m_dh->m_zVoxel ) ) ;

        int l_index = l_x + l_y * m_dh->m_columns + l_z * m_dh->m_rows * m_dh->m_columns;

        (*tmpAnatomy->getFloatDataset())[l_index*3] += l_colorData[i * 3] * m_localizedAlpha[i];
        (*tmpAnatomy->getFloatDataset())[l_index*3+1] += l_colorData[i * 3+1] * m_localizedAlpha[i];
        (*tmpAnatomy->getFloatDataset())[l_index*3+2] += l_colorData[i * 3+2] * m_localizedAlpha[i];
    }
    if( m_dh->m_useVBO )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
}

void Fibers::save( wxString i_fileName )
{
    vector<float>   l_pointsToSave;
    vector<int>     l_linesToSave;
    vector<wxUint8> l_colorsToSave;
    int l_pointIndex = 0;
    int l_countLines = 0;

    if( i_fileName.AfterLast( '.' ) != _T( "fib" ) )
        i_fileName += _T( ".fib" );

    float* l_colorData = NULL;
    if( m_dh->m_useVBO )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        l_colorData = (float *) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        l_colorData = &m_colorArray[0];
    }
    
    for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l] )
        {
            unsigned int pc = getStartIndexForLine( l ) * 3;
            l_linesToSave.push_back( getPointsPerLine( l ) );

            for ( int j = 0; j < getPointsPerLine( l ); ++j )
            {
                l_pointsToSave.push_back( m_dh->m_columns * m_dh->m_xVoxel - m_pointArray[pc] );
                l_colorsToSave.push_back( (wxUint8) ( l_colorData[pc] * 255 ) );
                ++pc;

                l_pointsToSave.push_back( m_dh->m_rows * m_dh->m_yVoxel - m_pointArray[pc] );
                l_colorsToSave.push_back( (wxUint8) ( l_colorData[pc] * 255 ) );
                ++pc;

                l_pointsToSave.push_back( m_pointArray[pc] );
                l_colorsToSave.push_back( (wxUint8) ( l_colorData[pc] * 255 ) );
                ++pc;

                l_linesToSave.push_back( l_pointIndex );
                ++l_pointIndex;
            }
            ++l_countLines;
        }
    }

    if( m_dh->m_useVBO )
        glUnmapBuffer( GL_ARRAY_BUFFER );

    converterByteINT32 c;
    converterByteFloat f;

    ofstream myfile;
    vector< char > vBuffer;

    string header1 = "# vtk DataFile Version 3.0\nvtk output\nBINARY\nDATASET POLYDATA\nPOINTS ";
    header1 += intToString( l_pointsToSave.size() / 3 );
    header1 += " float\n";

    for( unsigned int i = 0; i < header1.size(); ++i )
        vBuffer.push_back( header1[i] );

    for( unsigned int i = 0; i < l_pointsToSave.size(); ++i )
    {
        f.f = l_pointsToSave[i];
        vBuffer.push_back( f.b[3] );
        vBuffer.push_back( f.b[2] );
        vBuffer.push_back( f.b[1] );
        vBuffer.push_back( f.b[0] );
    }

    vBuffer.push_back( '\n' );
    string header2 = "LINES " + intToString( l_countLines ) + " " + intToString( l_linesToSave.size() )+ "\n";

    for( unsigned int i = 0; i < header2.size(); ++i )
        vBuffer.push_back( header2[i] );

    for( unsigned int i = 0; i < l_linesToSave.size(); ++i )
    {
        c.i = l_linesToSave[i];
        vBuffer.push_back( c.b[3] );
        vBuffer.push_back( c.b[2] );
        vBuffer.push_back( c.b[1] );
        vBuffer.push_back( c.b[0] );
    }

    vBuffer.push_back( '\n' );

    string header3 = "POINT_DATA ";
    header3 += intToString( l_pointsToSave.size() / 3 );
    header3 += " float\n";
    header3 += "COLOR_SCALARS scalars 3\n";

    for( unsigned int i = 0; i < header3.size(); ++i )
        vBuffer.push_back( header3[i] );

    for( unsigned int i = 0; i < l_colorsToSave.size(); ++i )
        vBuffer.push_back( l_colorsToSave[i] );

    vBuffer.push_back( '\n' );

    // Finally put the buffer vector into a char* array.
    char* l_buffer;
    l_buffer = new char[vBuffer.size()];

    for( unsigned int i = 0; i < vBuffer.size(); ++i )
        l_buffer[i] = vBuffer[i];

    char* l_fn;
    l_fn = (char*) malloc( i_fileName.length() );
    strcpy( l_fn, (const char*) i_fileName.mb_str( wxConvUTF8 ) );

    myfile.open( l_fn, ios::binary );
    myfile.write( l_buffer, vBuffer.size() );

    myfile.close();
}

void Fibers::saveDMRI( wxString i_fileName )
{
    int l_countLines = 0;

    if( i_fileName.AfterLast( '.' ) != _T( "fib" ) )
        i_fileName += _T( ".fib" );

    int nbrlines=0;
    for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l])
        {
            nbrlines++;
        }
    }

    ofstream myfile;
    char* l_fn;
    l_fn = (char*) malloc( i_fileName.length() );
    strcpy( l_fn, (const char*) i_fileName.mb_str( wxConvUTF8 ) );
    myfile.open( l_fn, ios::out );

    float dist = 0.5;

    myfile << "1 FA\n4 min max mean var\n1\n4 0 0 0 0\n4 0 0 0 0\n4 0 0 0 0\n";
    myfile << nbrlines << " " << dist <<"\n";

    for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l])
        {
            unsigned int pc = getStartIndexForLine( l ) * 3;            
            myfile << getPointsPerLine( l ) << " 1\n1\n";
            for ( int j = 0; j < getPointsPerLine( l ); ++j )
            {
                myfile <<  m_pointArray[pc] << " " <<  m_pointArray[pc+1] << " " <<  m_pointArray[pc+2] << " 0\n";  
                pc+=3;
            }
            pc = getStartIndexForLine( l ) * 3;  
            myfile <<  m_pointArray[pc] << " " <<  m_pointArray[pc+1] << " " <<  m_pointArray[pc+2] << " 0\n";  
            ++l_countLines;
        }
    }
    myfile.close();
}

string Fibers::intToString( int i_number )
{
    stringstream out; 
    out << i_number;
    return out.str();
}

void Fibers::toggleEndianess()
{
    m_dh->printDebug( _T( "toggle Endianess" ), 1 );

    wxUint8 l_temp = 0;
    wxUint8* l_pointBytes = (wxUint8*)&m_pointArray[0];

    for( int i = 0; i < m_countPoints * 12; i += 4 )
    {
        l_temp = l_pointBytes[i];
        l_pointBytes[i] = l_pointBytes[i + 3];
        l_pointBytes[i + 3] = l_temp;

        l_temp = l_pointBytes[i + 1];
        l_pointBytes[i + 1] = l_pointBytes[i + 2];
        l_pointBytes[i + 2] = l_temp;
    }

    // Toggle endianess for the line array.
    wxUint8* l_lineBytes = (wxUint8*)&m_lineArray[0];

    for( size_t i = 0; i < m_lineArray.size() * 4; i += 4 )
    {
        l_temp = l_lineBytes[i];
        l_lineBytes[i] = l_lineBytes[i + 3];
        l_lineBytes[i + 3] = l_temp;

        l_temp = l_lineBytes[i + 1];
        l_lineBytes[i + 1] = l_lineBytes[i + 2];
        l_lineBytes[i + 2] = l_temp;
    }
}

int Fibers::getPointsPerLine( int i_line )
{
    return ( m_linePointers[i_line + 1] - m_linePointers[i_line] );
}

int Fibers::getStartIndexForLine( int i_line )
{
    return m_linePointers[i_line];
}

int Fibers::getLineForPoint( int i_point )
{
    return m_reverse[i_point];
}

void Fibers::calculateLinePointers()
{
    m_dh->printDebug( _T( "calculate line pointers" ), 1 );

    int l_pc = 0;
    int l_lc = 0;
    int l_tc = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i] = l_tc;
        l_lc = m_lineArray[l_pc];
        l_tc += l_lc;
        l_pc += ( l_lc + 1 );
    }

    l_lc = 0;
    l_pc = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[l_lc + 1] )
            ++l_lc;

        m_reverse[i] = l_lc;
    }
}

void Fibers::createColorArray( bool i_colorsLoadedFromFile )
{
    m_dh->printDebug( _T( "create color arrays" ), 1 );

    if( ! i_colorsLoadedFromFile )
    {
        m_colorArray.clear();
        m_colorArray.resize( m_countPoints * 3 );
    }

    m_normalArray.clear();
    m_normalArray.resize( m_countPoints * 3 );

    int   l_pc = 0;    
    float l_x1, l_x2, l_y1, l_y2, l_z1, l_z2 = 0.0f;
    float l_r, l_g, l_b, l_rr, l_gg, l_bb    = 0.0f;
    float l_lastX, l_lastY, l_lastZ          = 0.0f;

    for( int i = 0; i < getLineCount(); ++i )
    {
        l_x1 = m_pointArray[l_pc];
        l_y1 = m_pointArray[l_pc + 1];
        l_z1 = m_pointArray[l_pc + 2];
        l_x2 = m_pointArray[l_pc + getPointsPerLine( i ) * 3 - 3];
        l_y2 = m_pointArray[l_pc + getPointsPerLine( i ) * 3 - 2];
        l_z2 = m_pointArray[l_pc + getPointsPerLine( i ) * 3 - 1];

        l_r = ( l_x1 ) - ( l_x2 );
        l_g = ( l_y1 ) - ( l_y2 );
        l_b = ( l_z1 ) - ( l_z2 );

        if( l_r < 0.0 )
            l_r *= -1.0;

        if( l_g < 0.0 )
            l_g *= -1.0;

        if( l_b < 0.0 )
            l_b *= -1.0;

        float norm = sqrt( l_r * l_r + l_g * l_g + l_b * l_b );
        l_r *= 1.0 / norm;
        l_g *= 1.0 / norm;
        l_b *= 1.0 / norm;

        l_lastX = m_pointArray[l_pc]     + ( m_pointArray[l_pc]     - m_pointArray[l_pc + 3] );
        l_lastY = m_pointArray[l_pc + 1] + ( m_pointArray[l_pc + 1] - m_pointArray[l_pc + 4] );
        l_lastZ = m_pointArray[l_pc + 2] + ( m_pointArray[l_pc + 2] - m_pointArray[l_pc + 5] );

        for( int j = 0; j < getPointsPerLine( i ); ++j )
        {
            l_rr = l_lastX - m_pointArray[l_pc];
            l_gg = l_lastY - m_pointArray[l_pc + 1];
            l_bb = l_lastZ - m_pointArray[l_pc + 2];

            l_lastX = m_pointArray[l_pc];
            l_lastY = m_pointArray[l_pc + 1];
            l_lastZ = m_pointArray[l_pc + 2];

            if( l_rr < 0.0 )
                l_rr *= -1.0;

            if( l_gg < 0.0 )
                l_gg *= -1.0;

            if( l_bb < 0.0 )
                l_bb *= -1.0;

            float norm = sqrt( l_rr * l_rr + l_gg * l_gg + l_bb * l_bb );
            l_rr *= 1.0 / norm;
            l_gg *= 1.0 / norm;
            l_bb *= 1.0 / norm;

            m_normalArray[l_pc]     = l_rr;
            m_normalArray[l_pc + 1] = l_gg;
            m_normalArray[l_pc + 2] = l_bb;

            if( ! i_colorsLoadedFromFile )
            {
                m_colorArray[l_pc]     = l_r;
                m_colorArray[l_pc + 1] = l_g;
                m_colorArray[l_pc + 2] = l_b;
            }
            l_pc += 3;
        }
    }
}

void Fibers::resetColorArray()
{
    m_dh->printDebug( _T( "reset color arrays" ), 1 );

    float* l_colorData =  NULL;
    float* l_colorData2 = NULL;

    if( m_dh->m_useVBO )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        l_colorData  = (float *) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
        l_colorData2 = &m_colorArray[0];
    }
    else
    {
        l_colorData  = &m_colorArray[0];
        l_colorData2 = &m_colorArray[0];
    }

    int l_pc = 0;
    float l_r, l_g, l_b, l_x1, l_x2, l_y1, l_y2, l_z1, l_z2, l_lastX, l_lastY, l_lastZ = 0.0f;

    for( int i = 0; i < getLineCount(); ++i )
    {
        l_x1 = m_pointArray[l_pc];
        l_y1 = m_pointArray[l_pc + 1];
        l_z1 = m_pointArray[l_pc + 2];
        l_x2 = m_pointArray[l_pc + getPointsPerLine( i ) * 3 - 3];
        l_y2 = m_pointArray[l_pc + getPointsPerLine( i ) * 3 - 2];
        l_z2 = m_pointArray[l_pc + getPointsPerLine( i ) * 3 - 1];

        l_r = ( l_x1 ) - ( l_x2 );
        l_g = ( l_y1 ) - ( l_y2 );
        l_b = ( l_z1 ) - ( l_z2 );

        if( l_r < 0.0 )
            l_r *= -1.0;

        if( l_g < 0.0 )
            l_g *= -1.0;

        if( l_b < 0.0 )
            l_b *= -1.0;

        float l_norm = sqrt( l_r * l_r + l_g * l_g + l_b * l_b );
        l_r *= 1.0 / l_norm;
        l_g *= 1.0 / l_norm;
        l_b *= 1.0 / l_norm;

        l_lastX = m_pointArray[l_pc] + ( m_pointArray[l_pc] - m_pointArray[l_pc + 3] );
        l_lastY = m_pointArray[l_pc + 1] + ( m_pointArray[l_pc + 1] - m_pointArray[l_pc + 4] );
        l_lastZ = m_pointArray[l_pc + 2] + ( m_pointArray[l_pc + 2] - m_pointArray[l_pc + 5] );

        for( int j = 0; j < getPointsPerLine( i ); ++j )
        {
            l_colorData[l_pc] = l_r;
            l_colorData[l_pc + 1] = l_g;
            l_colorData[l_pc + 2] = l_b;

            l_colorData2[l_pc] = l_r;
            l_colorData2[l_pc + 1] = l_g;
            l_colorData2[l_pc + 2] = l_b;
            l_pc += 3;
        }
    }

    if( m_dh->m_useVBO )
        glUnmapBuffer( GL_ARRAY_BUFFER );

    m_dh->m_fiberColorationMode = NORMAL_COLOR;
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void Fibers::resetLinesShown()
{
    for( int i = 0; i < m_countLines; ++i )
        m_selected[i] = 0;
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void Fibers::updateLinesShown()
{
    vector< vector< SelectionObject* > > l_selectionObjects = m_dh->getSelectionObjects();

    for( int i = 0; i < m_countLines; ++i )
    {
        m_selected[i] = 1;
    }

    int activeCount = 0;

    //First pass to make sure there is at least one intersection volume active;
    for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
    {
        if( l_selectionObjects[i][0]->getIsActive() )
        {
            activeCount++;

            for( unsigned int j = 1; j < l_selectionObjects[i].size(); ++j )
            {
                if( l_selectionObjects[i][j]->getIsActive() )
                {
                    activeCount++;
                }
            }
        }
    }

    if(activeCount == 0)
    {
        return;
    }



    // For all the master selection objects.
    for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
    {
        if( l_selectionObjects[i][0]->getIsActive() )
        {

            if( l_selectionObjects[i][0]->getIsDirty() )
            {
                l_selectionObjects[i][0]->m_inBox.clear();
                l_selectionObjects[i][0]->m_inBox.resize( m_countLines );
                l_selectionObjects[i][0]->m_inBranch.clear();
                l_selectionObjects[i][0]->m_inBranch.resize( m_countLines );
                // Sets the fibers that are inside this object to true in the m_inBox vector.
                l_selectionObjects[i][0]->m_inBox = getLinesShown( l_selectionObjects[i][0] );
                l_selectionObjects[i][0]->setIsDirty( false );
            }

            for( int k = 0; k < m_countLines; ++k )
                l_selectionObjects[i][0]->m_inBranch[k] = l_selectionObjects[i][0]->m_inBox[k];

            // For all its child box.
            for( unsigned int j = 1; j < l_selectionObjects[i].size(); ++j )
            {
                if( l_selectionObjects[i][j]->getIsActive() )
                {
                    if( l_selectionObjects[i][j]->getIsDirty() )
                    {
                        l_selectionObjects[i][j]->m_inBox.clear();
                        l_selectionObjects[i][j]->m_inBox.resize( m_countLines );
                        // Sets the fibers that are inside this object to true in the m_inBox vector.
                        l_selectionObjects[i][j]->m_inBox = getLinesShown( l_selectionObjects[i][j] );
                        l_selectionObjects[i][j]->setIsDirty( false );
                    }
                    // Sets the fibers that are INSIDE this child object and INSIDE its master to be in branch.
                    if( ! l_selectionObjects[i][j]->getIsNOT() )
                    {
                        for( int k = 0; k < m_countLines; ++k )
                        {
                            l_selectionObjects[i][0]->m_inBranch[k] = l_selectionObjects[i][0]->m_inBranch[k] & l_selectionObjects[i][j]->m_inBox[k];
                        }
                    }
                    else // Sets the fibers that are NOT INSIDE this child object and INSIDE its master to be in branch.
                    {
                        for( int k = 0; k < m_countLines; ++k )
                        {
                            l_selectionObjects[i][0]->m_inBranch[k] = l_selectionObjects[i][0]->m_inBranch[k] & ! l_selectionObjects[i][j]->m_inBox[k];
                        }
                    }
                }
            }
        }

        if( l_selectionObjects[i].size() > 0 && l_selectionObjects[i][0]->isColorChanged() )
        {
            float* l_colorData  = NULL;
            float* l_colorData2 = NULL;

            if( m_dh->m_useVBO )
            {
                glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
                l_colorData  = (float*) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
                l_colorData2 = &m_colorArray[0];
            }
            else
            {
                l_colorData  = &m_colorArray[0];
                l_colorData2 = &m_colorArray[0];
            }
            wxColour l_col = l_selectionObjects[i][0]->getFiberColor();

            for( int l = 0; l < m_countLines; ++l )
            {
                if( l_selectionObjects[i][0]->m_inBranch[l] )
                {
                    unsigned int pc = getStartIndexForLine( l ) * 3;

                    for( int j = 0; j < getPointsPerLine( l ); ++j )
                    {
                        l_colorData[pc]      = ( (float) l_col.Red() )   / 255.0f;
                        l_colorData[pc + 1]  = ( (float) l_col.Green() ) / 255.0f;
                        l_colorData[pc + 2]  = ( (float) l_col.Blue() )  / 255.0f;

                        l_colorData2[pc]     = ( (float) l_col.Red() )   / 255.0f;
                        l_colorData2[pc + 1] = ( (float) l_col.Green() ) / 255.0f;
                        l_colorData2[pc + 2] = ( (float) l_col.Blue() )  / 255.0f;
                        pc += 3;
                    }
                }
            }

            if( m_dh->m_useVBO )
                glUnmapBuffer( GL_ARRAY_BUFFER );
            
            l_selectionObjects[i][0]->setColorChanged( false );
        }
    }

    resetLinesShown();

    bool l_boxWasUpdated = false;
    for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
    {
        if( l_selectionObjects[i].size() > 0 && l_selectionObjects[i][0]->getIsActive() )
        {
            for ( int k = 0; k < m_countLines; ++k )
                m_selected[k] = m_selected[k] | l_selectionObjects[i][0]->m_inBranch[k];
        }

        l_boxWasUpdated = true;
    }

    if( m_dh->m_fibersInverted )
    {
        for( int k = 0; k < m_countLines; ++k )
            m_selected[k] = ! m_selected[k];
    }

    // This is to update the information display in the fiber grid info.
    if( l_boxWasUpdated && m_dh->m_lastSelectedObject!=NULL )
    {
        m_dh->m_lastSelectedObject->SetFiberInfoGridValues();
    }
}

///////////////////////////////////////////////////////////////////////////
// Will return the fibers that are inside the selection object passed in argument.
//
// i_selectionObject        : The selection object to test with.
//
// Return a vector of bool, a value of TRUE indicate that this fiber is inside the selection object passed in argument.
// A value of false, indicate that this fiber is not inside the selection object.
///////////////////////////////////////////////////////////////////////////
vector< bool > Fibers::getLinesShown( SelectionObject* i_selectionObject )
{
    if( ! i_selectionObject->isSelectionObject() && ! i_selectionObject->m_sourceAnatomy )
    {
        return i_selectionObject->m_inBox;
    }

    resetLinesShown();

    if( i_selectionObject->getSelectionType() == BOX_TYPE || i_selectionObject->getSelectionType() == ELLIPSOID_TYPE )
    {
        Vector l_center = i_selectionObject->getCenter();
        Vector l_size   = i_selectionObject->getSize();

        m_boxMin.resize( 3 );
        m_boxMax.resize( 3 );

        m_boxMin[0] = l_center.x - l_size.x / 2 * m_dh->m_xVoxel;
        m_boxMax[0] = l_center.x + l_size.x / 2 * m_dh->m_xVoxel;
        m_boxMin[1] = l_center.y - l_size.y / 2 * m_dh->m_yVoxel;
        m_boxMax[1] = l_center.y + l_size.y / 2 * m_dh->m_yVoxel;
        m_boxMin[2] = l_center.z - l_size.z / 2 * m_dh->m_zVoxel;
        m_boxMax[2] = l_center.z + l_size.z / 2 * m_dh->m_zVoxel;

        if( i_selectionObject->getSelectionType() == ELLIPSOID_TYPE )
        {
            ellipsoidTest( 0, m_countPoints - 1, 0 );
        }
        else
        {
            boxTest( 0, m_countPoints - 1, 0 );
        }
    }
    else
    {
        for( int i = 0; i < m_countPoints; ++i )
        {
            if(m_selected[getLineForPoint(i)] != 1)
            {
                int l_x     = (int)wxMin( m_dh->m_columns -1, wxMax( 0, m_pointArray[i * 3 ]/ m_dh->m_xVoxel    ) ) ;
                int l_y     = (int)wxMin( m_dh->m_rows    -1, wxMax( 0, m_pointArray[i * 3 + 1]/ m_dh->m_yVoxel ) ) ;
                int l_z     = (int)wxMin( m_dh->m_frames  -1, wxMax( 0, m_pointArray[i * 3 + 2]/ m_dh->m_zVoxel ) ) ;

                int l_index = l_x + l_y * m_dh->m_columns + l_z * m_dh->m_rows * m_dh->m_columns;

                if( ( i_selectionObject->m_sourceAnatomy->at( l_index ) > i_selectionObject->getThreshold() ))
                {
                    m_selected[getLineForPoint( i )] = 1;            
                }
            }

        }
    }

    return m_selected;
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_left       :
// i_right      :
// i_axis       :
///////////////////////////////////////////////////////////////////////////
void Fibers::boxTest( int i_left, int i_right, int i_axis )
{
    // Abort condition.
    if( i_left > i_right )
        return;

    int l_root       = i_left + ( ( i_right - i_left ) / 2 );
    int l_axis1      = ( i_axis + 1 ) % 3;
    int l_pointIndex = m_kdTree->m_tree[l_root] * 3;

    if( m_pointArray[l_pointIndex + i_axis] < m_boxMin[i_axis] )
        boxTest( l_root + 1, i_right, l_axis1 );
    else if( m_pointArray[l_pointIndex + i_axis] > m_boxMax[i_axis] )
        boxTest( i_left, l_root - 1, l_axis1 );
    else
    {
        int l_axis2 = ( i_axis + 2 ) % 3;
        if ( m_pointArray[l_pointIndex + l_axis1] <= m_boxMax[l_axis1] && 
             m_pointArray[l_pointIndex + l_axis1] >= m_boxMin[l_axis1] && 
             m_pointArray[l_pointIndex + l_axis2] <= m_boxMax[l_axis2] && 
             m_pointArray[l_pointIndex + l_axis2] >= m_boxMin[l_axis2] )
        {
            m_selected[getLineForPoint( m_kdTree->m_tree[l_root] )] = 1;
        }

        boxTest( i_left, l_root - 1, l_axis1 );
        boxTest( l_root + 1, i_right, l_axis1 );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function is exactly the same as boxTest with the simple modification that to set 
// a fiber to selected, we test with the ellipsoid detectioninstead of the quare detection.
///////////////////////////////////////////////////////////////////////////
void Fibers::ellipsoidTest( int i_left, int i_right, int i_axis )
{
    // Abort condition.
    if( i_left > i_right )
        return;

    int l_root  = i_left + ( ( i_right - i_left ) / 2 );
    int l_axis1 = ( i_axis + 1 ) % 3;
    int l_pointIndex = m_kdTree->m_tree[l_root] * 3;

    if( m_pointArray[l_pointIndex + i_axis] < m_boxMin[i_axis] )
        ellipsoidTest( l_root + 1, i_right, l_axis1 );
    else if( m_pointArray[l_pointIndex + i_axis] > m_boxMax[i_axis] )
        ellipsoidTest( i_left, l_root - 1, l_axis1 );
    else
    {
        int l_axis2 = ( i_axis + 2 ) % 3;
        float l_axisRadius  = ( m_boxMax[i_axis]  - m_boxMin[i_axis]  ) / 2.0f;
        float l_axis1Radius = ( m_boxMax[l_axis1] - m_boxMin[l_axis1] ) / 2.0f;
        float l_axis2Radius = ( m_boxMax[l_axis2] - m_boxMin[l_axis2] ) / 2.0f;
        float l_axisCenter  = m_boxMax[i_axis]  - l_axisRadius;
        float l_axis1Center = m_boxMax[l_axis1] - l_axis1Radius;
        float l_axis2Center = m_boxMax[l_axis2] - l_axis2Radius;
        // This if will set a fibers to be selected if its inside the ellipsoid.
        if( ( pow( m_pointArray[l_pointIndex + i_axis]  - l_axisCenter,  2.0f ) ) / ( l_axisRadius  * l_axisRadius  ) + 
            ( pow( m_pointArray[l_pointIndex + l_axis1] - l_axis1Center, 2.0f ) ) / ( l_axis1Radius * l_axis1Radius ) + 
            ( pow( m_pointArray[l_pointIndex + l_axis2] - l_axis2Center, 2.0f ) ) / ( l_axis2Radius * l_axis2Radius ) <= 1.0f )
        {
            m_selected[getLineForPoint( m_kdTree->m_tree[l_root] )] = 1;
        }

        ellipsoidTest( i_left, l_root - 1, l_axis1 );
        ellipsoidTest( l_root + 1, i_right, l_axis1 );
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_point      :
///////////////////////////////////////////////////////////////////////////
bool Fibers::getBarycenter( SplinePoint* i_point )
{
    // Number of fibers needed to keep a i_point.
    int l_threshold = 20;

    // Multiplier for moving the i_point towards the barycenter.
    m_boxMin.resize( 3 );
    m_boxMax.resize( 3 );
    m_boxMin[0] = i_point->X() - 25.0 / 2;
    m_boxMax[0] = i_point->X() + 25.0 / 2;
    m_boxMin[1] = i_point->Y() - 5.0 / 2;
    m_boxMax[1] = i_point->Y() + 5.0 / 2;
    m_boxMin[2] = i_point->Z() - 5.0 / 2;
    m_boxMax[2] = i_point->Z() + 5.0 / 2;

    m_barycenter.x = m_barycenter.y = m_barycenter.z = m_count = 0;

    barycenterTest( 0, m_countPoints - 1, 0 );
    if( m_count > l_threshold )
    {
        m_barycenter.x /= m_count;
        m_barycenter.y /= m_count;
        m_barycenter.z /= m_count;

        float l_x1 = ( m_barycenter.x - i_point->X() );
        float l_y1 = ( m_barycenter.y - i_point->Y() );
        float l_z1 = ( m_barycenter.z - i_point->Z() );

        Vector l_vector( l_x1, l_y1, l_z1 );
        i_point->setOffsetVector( l_vector );

        i_point->setX( i_point->X() + l_x1 );
        i_point->setY( i_point->Y() + l_y1 );
        i_point->setZ( i_point->Z() + l_z1 );

        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void Fibers::barycenterTest( int i_left, int i_right, int i_axis )
{
    // Abort condition.
    if( i_left > i_right )
        return;

    int l_root  = i_left + ( ( i_right - i_left ) / 2 );
    int l_axis1 = ( i_axis + 1 ) % 3;
    int l_pointIndex = m_kdTree->m_tree[l_root] * 3;

    if( m_pointArray[l_pointIndex + i_axis] < m_boxMin[i_axis] )
        barycenterTest( l_root + 1, i_right, l_axis1 );
    else if( m_pointArray[l_pointIndex + i_axis] > m_boxMax[i_axis] )
        barycenterTest( i_left, l_root - 1, l_axis1 );
    else
    {
        int l_axis2 = ( i_axis + 2 ) % 3;

        if ( m_selected[getLineForPoint( m_kdTree->m_tree[l_root] )] == 1 && 
             m_pointArray[l_pointIndex + l_axis1] <= m_boxMax[l_axis1]    && 
             m_pointArray[l_pointIndex + l_axis1] >= m_boxMin[l_axis1]    && 
             m_pointArray[l_pointIndex + l_axis2] <= m_boxMax[l_axis2]    && 
             m_pointArray[l_pointIndex + l_axis2] >= m_boxMin[l_axis2] )
        {
            m_barycenter[0] += m_pointArray[m_kdTree->m_tree[l_root] * 3];
            m_barycenter[1] += m_pointArray[m_kdTree->m_tree[l_root] * 3 + 1];
            m_barycenter[2] += m_pointArray[m_kdTree->m_tree[l_root] * 3 + 2];
            m_count++;
        }

        barycenterTest( i_left, l_root - 1, l_axis1 );
        barycenterTest( l_root + 1, i_right, l_axis1 );
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void Fibers::initializeBuffer()
{
    if( m_isInitialized || ! m_dh->m_useVBO )
        return;

    m_isInitialized = true;
    
    bool isOK = true;

    glGenBuffers( 3, m_bufferObjects );
    glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * m_countPoints * 3, &m_pointArray[0], GL_STATIC_DRAW );

    if( m_dh->GLError() )
    {
        m_dh->printGLError( wxT( "initialize vbo points" ) );
        isOK = false;
    }

    if( isOK )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * m_countPoints * 3, &m_colorArray[0], GL_STATIC_DRAW );

        if ( m_dh->GLError() )
        {
            m_dh->printGLError( wxT( "initialize vbo colors" ) );
            isOK = false;
        }
    }

    if( isOK )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * m_countPoints * 3, &m_normalArray[0], GL_STATIC_DRAW );

        if( m_dh->GLError() )
        {
            m_dh->printGLError( wxT( "initialize vbo normals" ) );
            isOK = false;
        }
    }

    m_dh->m_useVBO = isOK;

    if( isOK )
    {
        freeArrays();
    }
    else
    {
        m_dh->printDebug( _T( "Not enough memory on your gfx card. Using vertex arrays." ),            2 );
        m_dh->printDebug( _T( "This shouldn't concern you. Perfomance just will be slightly worse." ), 2 );
        m_dh->printDebug( _T( "Get a better graphics card if you want more juice." ),                  2 );
        glDeleteBuffers( 3, m_bufferObjects );
    }
}

void Fibers::draw()
{

    if( m_cachedThreshold != m_threshold)
    {
        updateFibersColors();
        m_cachedThreshold = m_threshold;
    }
  
    initializeBuffer();

    if( m_dh->m_useFakeTubes )
    {
        drawFakeTubes();
        return;
    }

    if( m_dh->m_useTransparency )
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDepthMask(GL_FALSE);

        drawSortedLines();
        glPopAttrib();

        return;
    }

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY  );
    glEnableClientState( GL_NORMAL_ARRAY );

    if( ! m_dh->m_useVBO )
    {
        glVertexPointer( 3, GL_FLOAT, 0, &m_pointArray[0] );

        if( m_showFS )
            glColorPointer( 3, GL_FLOAT, 0, &m_colorArray[0] );  // Global colors.
        else
            glColorPointer( 3, GL_FLOAT, 0, &m_normalArray[0] ); // Local colors.

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

    for( int i = 0; i < m_countLines; ++i )
    {
        if ( (m_selected[i] || !m_dh->m_activateObjects) && !m_filtered[i])
            glDrawArrays( GL_LINE_STRIP, getStartIndexForLine( i ), getPointsPerLine( i ) );
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
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
        switchNormals( false );

    GLfloat* l_colors  = NULL;
    GLfloat* l_normals = NULL;

    l_colors  = &m_colorArray[0];
    l_normals = &m_normalArray[0];

    if( m_dh->getPointMode() )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    
    
    for( int i = 0; i < m_countLines; ++i )
    {
        if( m_selected[i] && !m_filtered[i] )
        {
            int idx = getStartIndexForLine( i ) * 3;

            glBegin( GL_QUAD_STRIP );
            
            for( int k = 0; k < getPointsPerLine( i ); ++k )
            {
                glNormal3f( l_normals[idx], l_normals[idx + 1], l_normals[idx + 2] );
                glColor3f( l_colors[idx],  l_colors[idx + 1],  l_colors[idx + 2] );
                glTexCoord2f(-1.0f, 0.0f);

                glVertex3f( m_pointArray[idx], m_pointArray[idx + 1], m_pointArray[idx + 2]);
                glTexCoord2f(1.0f, 0.0f);

                glVertex3f( m_pointArray[idx], m_pointArray[idx + 1], m_pointArray[idx + 2]);
                idx += 3;                
            }
            glEnd();
        }
    }
}

void Fibers::drawSortedLines()
{
    // Only sort those lines we see.
    unsigned int *l_snippletSort = NULL;
    unsigned int *l_lineIds      = NULL;
    int l_nbSnipplets = 0;
    
    // Estimate memory required for arrays.
    for( int i = 0; i < m_countLines; ++i )
    {
        if ( m_selected[i] && !m_filtered[i])
            l_nbSnipplets += getPointsPerLine( i ) - 1;
    }

    l_snippletSort = new unsigned int[l_nbSnipplets + 1]; // +1 just to be sure because of fancy problems with some sort functions.
    l_lineIds      = new unsigned int[l_nbSnipplets * 2];

    // Build data structure for sorting.
    int l_snp = 0;
    for( int i = 0; i < m_countLines; ++i )
    {
        if (!(m_selected[i] && !m_filtered[i]))
            continue;

        const unsigned int l_p = getPointsPerLine( i );

        // TODO: update l_lineIds and l_snippletSort size only when fiber selection changes.
        for( unsigned int k = 0; k < l_p - 1; ++k )
        {
            l_lineIds[l_snp << 1] = getStartIndexForLine( i ) + k;
            l_lineIds[( l_snp << 1 ) + 1] = getStartIndexForLine( i ) + k + 1;
            
            l_snippletSort[l_snp] = l_snp;
            l_snp++;
        }
    }    

    GLfloat l_matrix[16];
    glGetFloatv( GL_PROJECTION_MATRIX, l_matrix );

    // Compute z values of lines (in our case: starting points only).
    vector< float > l_zVal( l_nbSnipplets );
    for( int i = 0; i < l_nbSnipplets; ++i )
    {
        const int id = l_lineIds[i << 1] * 3;
        l_zVal[i] = ( m_pointArray[id + 0] * l_matrix[2] + m_pointArray[id + 1] * l_matrix[6]
                  + m_pointArray[id + 2] * l_matrix[10] + l_matrix[14] ) / ( m_pointArray[id + 0] * l_matrix[3]
                  + m_pointArray[id + 1] * l_matrix[7] + m_pointArray[id + 2] * l_matrix[11] + l_matrix[15] );
    }

    sort( &l_snippletSort[0], &l_snippletSort[l_nbSnipplets], IndirectComp< vector< float > > ( l_zVal ) );

    float* l_colors  = NULL;
    float* l_normals = NULL;

    if( m_dh->m_useVBO )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        l_colors = (float *) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_ONLY );
        glUnmapBuffer( GL_ARRAY_BUFFER );

        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        l_normals = (float *) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_ONLY );
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
    else
    {
        l_colors  = &m_colorArray[0];
        l_normals = &m_normalArray[0];
    }

    if( m_dh->getPointMode() )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBegin( GL_LINES );

    if(m_dh->m_fiberColorationMode == MINDISTANCE_COLOR)
    {

        int i = 0;
        for( int c = 0; c < l_nbSnipplets; ++c )
        {
            i = c;
            int l_idx  = l_lineIds[l_snippletSort[i] << 1];
            int l_idx3 = l_idx * 3;
            int l_id2  = l_lineIds[( l_snippletSort[i] << 1 ) + 1];
            int l_id23 = l_id2 * 3;

            glColor4f ( l_colors[l_idx3 + 0],       l_colors[l_idx3 + 1],       l_colors[l_idx3 + 2],   m_localizedAlpha[l_idx] * m_alpha);
            glNormal3f( l_normals[l_idx3 + 0],      l_normals[l_idx3 + 1],      l_normals[l_idx3 + 2] );
            glVertex3f( m_pointArray[l_idx3 + 0],   m_pointArray[l_idx3 + 1],   m_pointArray[l_idx3 + 2] );

            glColor4f ( l_colors[l_id23 + 0], l_colors[l_id23 + 1], l_colors[l_id23 + 2], m_localizedAlpha[l_id2] * m_alpha);
            glNormal3f( l_normals[l_id23 + 0], l_normals[l_id23 + 1], l_normals[l_id23 + 2] );
            glVertex3f( m_pointArray[l_id23 + 0], m_pointArray[l_id23 + 1], m_pointArray[l_id23 + 2] );
        }
    }
    else
    {
        int i = 0;
        for( int c = 0; c < l_nbSnipplets; ++c )
        {
            i = c;
            int l_idx  = l_lineIds[l_snippletSort[i] << 1];
            int l_idx3 = l_idx * 3;
            int l_id2  = l_lineIds[( l_snippletSort[i] << 1 ) + 1];
            int l_id23 = l_id2 * 3;

            glColor4f ( l_colors[l_idx3 + 0],       l_colors[l_idx3 + 1],       l_colors[l_idx3 + 2],   m_alpha );
            glNormal3f( l_normals[l_idx3 + 0],      l_normals[l_idx3 + 1],      l_normals[l_idx3 + 2] );
            glVertex3f( m_pointArray[l_idx3 + 0],   m_pointArray[l_idx3 + 1],   m_pointArray[l_idx3 + 2] );

            glColor4f ( l_colors[l_id23 + 0], l_colors[l_id23 + 1], l_colors[l_id23 + 2], m_alpha );
            glNormal3f( l_normals[l_id23 + 0], l_normals[l_id23 + 1], l_normals[l_id23 + 2] );
            glVertex3f( m_pointArray[l_id23 + 0], m_pointArray[l_id23 + 1], m_pointArray[l_id23 + 2] );
        }
    }

    glEnd();
    glDisable( GL_BLEND );

    // FIXME: store these later on!
    delete[] l_snippletSort;
    delete[] l_lineIds;
}

void Fibers::switchNormals( bool i_positive )
{
    float* l_normals = NULL;

    l_normals = &m_normalArray[0];

    if( i_positive )
    {
        int l_pc = 0;
        float l_rr, l_gg, l_bb, l_lastx, l_lasty, l_lastz = 0.0f;

        for( int i = 0; i < getLineCount(); ++i )
        {
            l_lastx = m_pointArray[l_pc] + ( m_pointArray[l_pc] - m_pointArray[l_pc + 3] );
            l_lasty = m_pointArray[l_pc + 1] + ( m_pointArray[l_pc + 1] - m_pointArray[l_pc + 4] );
            l_lastz = m_pointArray[l_pc + 2] + ( m_pointArray[l_pc + 2] - m_pointArray[l_pc + 5] );

            for( int j = 0; j < getPointsPerLine( i ); ++j )
            {
                l_rr = l_lastx - m_pointArray[l_pc];
                l_gg = l_lasty - m_pointArray[l_pc + 1];
                l_bb = l_lastz - m_pointArray[l_pc + 2];

                l_lastx = m_pointArray[l_pc];
                l_lasty = m_pointArray[l_pc + 1];
                l_lastz = m_pointArray[l_pc + 2];

                if( l_rr < 0.0 )
                    l_rr *= -1.0;

                if( l_gg < 0.0 )
                    l_gg *= -1.0;

                if( l_bb < 0.0 )
                    l_bb *= -1.0;

                float norm = sqrt( l_rr * l_rr + l_gg * l_gg + l_bb * l_bb );
                l_rr *= 1.0 / norm;
                l_gg *= 1.0 / norm;
                l_bb *= 1.0 / norm;

                l_normals[l_pc] = l_rr;
                l_normals[l_pc + 1] = l_gg;
                l_normals[l_pc + 2] = l_bb;

                l_pc += 3;
            }
        }
        m_normalsPositive = true;
    }
    else
    {
        int l_pc = 0;
        float l_rr, l_gg, l_bb, l_lastx, l_lasty, l_lastz = 0.0f;

        for( int i = 0; i < getLineCount(); ++i )
        {
            l_lastx = m_pointArray[l_pc] + ( m_pointArray[l_pc] - m_pointArray[l_pc + 3] );
            l_lasty = m_pointArray[l_pc + 1] + ( m_pointArray[l_pc + 1] - m_pointArray[l_pc + 4] );
            l_lastz = m_pointArray[l_pc + 2] + ( m_pointArray[l_pc + 2] - m_pointArray[l_pc + 5] );

            for( int j = 0; j < getPointsPerLine( i ); ++j )
            {
                l_rr = l_lastx - m_pointArray[l_pc];
                l_gg = l_lasty - m_pointArray[l_pc + 1];
                l_bb = l_lastz - m_pointArray[l_pc + 2];

                l_lastx = m_pointArray[l_pc];
                l_lasty = m_pointArray[l_pc + 1];
                l_lastz = m_pointArray[l_pc + 2];

                l_normals[l_pc] = l_rr;
                l_normals[l_pc + 1] = l_gg;
                l_normals[l_pc + 2] = l_bb;

                l_pc += 3;
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

float Fibers::getPointValue( int i_index )
{
    return m_pointArray[i_index];
}

int Fibers::getLineCount()
{
    return m_countLines;
}

int Fibers::getPointCount()
{
    return m_countPoints;
}

bool Fibers::isSelected( int i_fiber )
{
    return m_selected[i_fiber];
}

void Fibers::setFibersLength()
{
    m_length.resize(m_countLines,false);
    vector< Vector >           l_currentFiberPoints;
    vector< vector< Vector > > l_FibersPoints;
    for (int i=0;i<m_countLines;i++){
        if (getFiberCoordValues(i, l_currentFiberPoints)){
            l_FibersPoints.push_back(l_currentFiberPoints);
            l_currentFiberPoints.clear();
        } 
    }    

    float l_dx,l_dy, l_dz;
    m_maxLength=0;
    m_minLength=1000000;
      for( unsigned int j = 0 ; j< l_FibersPoints.size(); j++){
        l_currentFiberPoints = l_FibersPoints[j];
        m_length[j] = 0;
        for( unsigned int i = 1; i < l_currentFiberPoints.size(); ++i )
        {
            // The values are in pixel, we need to set them in millimeters using the spacing 
            // specified in the anatomy file ( m_datasetHelper->xVoxel... ).
            l_dx = ( l_currentFiberPoints[i].x - l_currentFiberPoints[i-1].x ) * m_dh->m_xVoxel;
            l_dy = ( l_currentFiberPoints[i].y - l_currentFiberPoints[i-1].y ) * m_dh->m_yVoxel;
            l_dz = ( l_currentFiberPoints[i].z - l_currentFiberPoints[i-1].z ) * m_dh->m_zVoxel;

            FArray currentVector( l_dx, l_dy, l_dz );
            m_length[j] += (float)currentVector.norm();
        }
        if (m_length[j]>m_maxLength) m_maxLength = m_length[j];
        if (m_length[j]<m_minLength) m_minLength = m_length[j];
    }
}

bool Fibers::getFiberCoordValues( int i_fiberIndex, vector< Vector > &o_fiberPoints )
{
    Fibers* l_fibers = NULL;
    m_dh->getFiberDataset( l_fibers );

    if( l_fibers == NULL || i_fiberIndex < 0 )
        return false;

    int l_index = l_fibers->getStartIndexForLine( i_fiberIndex ) * 3;
    Vector l_point3D;
    for( int i = 0; i < l_fibers->getPointsPerLine( i_fiberIndex ); ++i )
    {
        l_point3D.x = l_fibers->getPointValue( l_index );
        l_point3D.y = l_fibers->getPointValue( l_index + 1);
        l_point3D.z = l_fibers->getPointValue( l_index + 2 );
        o_fiberPoints.push_back( l_point3D );

        l_index += 3;                
    }
    return true;
}

void Fibers::updateFibersFilters()
{
    int min = m_psliderFibersFilterMin->GetValue();
    int max = m_psliderFibersFilterMax->GetValue();
    int subSampling = m_psliderFibersSampling->GetValue();
    int maxSubSampling = m_psliderFibersSampling->GetMax()+1;

    for ( int i = 0; i < m_countLines; ++i )
        m_filtered[i]=!((i%maxSubSampling)>=subSampling && m_length[i]>=min && m_length[i]<=max);     
}


void Fibers::createPropertiesSizer(PropertiesWindow *parent)
{
    setFibersLength();
    DatasetInfo::createPropertiesSizer(parent);    
    wxSizer *l_sizer;
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText(parent, wxID_ANY ,wxT("Min Length"),wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE),0,wxALIGN_CENTER);
    m_psliderFibersFilterMin = new wxSlider(parent, wxID_ANY, getMinFibersLength(), getMinFibersLength(), getMaxFibersLength(), wxDefaultPosition, wxSize( 140,-1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    l_sizer->Add(m_psliderFibersFilterMin,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_psliderFibersFilterMin->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(PropertiesWindow::OnFibersFilter));

    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText(parent, wxID_ANY ,wxT("Max Length"),wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE),0,wxALIGN_CENTER);
    m_psliderFibersFilterMax = new wxSlider(parent, wxID_ANY, getMaxFibersLength(), getMinFibersLength(), getMaxFibersLength(), wxDefaultPosition, wxSize( 140,-1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    l_sizer->Add(m_psliderFibersFilterMax,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_psliderFibersFilterMax->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(PropertiesWindow::OnFibersFilter));
            
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText(parent, wxID_ANY ,wxT("Subsampling"),wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE),0,wxALIGN_CENTER);
    m_psliderFibersSampling = new wxSlider(parent, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize( 140,-1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    l_sizer->Add(m_psliderFibersSampling,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);    
    parent->Connect(m_psliderFibersSampling->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(PropertiesWindow::OnFibersFilter));

    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_pGeneratesFibersDensityVolume = new wxButton(parent, wxID_ANY,wxT("New Density Volume"),wxDefaultPosition, wxSize(140,-1));
    l_sizer->Add(m_pGeneratesFibersDensityVolume,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_pGeneratesFibersDensityVolume->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnGenerateFiberVolume));

    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_ptoggleLocalColoring = new wxToggleButton(parent, wxID_ANY,wxT("Local Coloring"),wxDefaultPosition, wxSize(140,-1));
    l_sizer->Add(m_ptoggleLocalColoring,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_ptoggleLocalColoring->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnListMenuThreshold));

    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_ptoggleNormalColoring = new wxToggleButton(parent, wxID_ANY,wxT("Color With Overley"),wxDefaultPosition, wxSize(140,-1));
    l_sizer->Add(m_ptoggleNormalColoring,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_ptoggleNormalColoring->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(PropertiesWindow::OnToggleShowFS));

    m_propertiesSizer->AddSpacer(8);
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText(parent, wxID_ANY, _T( "Coloring" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(8,1,0);
    m_pradioNormalColoring = new wxRadioButton(parent, wxID_ANY, _T( "Normal" ), wxDefaultPosition, wxSize(132,-1));
    l_sizer->Add(m_pradioNormalColoring);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    m_pradioDistanceAnchoring  = new wxRadioButton(parent, wxID_ANY, _T( "Dist. Anchoring" ), wxDefaultPosition, wxSize(132,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(68,1,0);
    l_sizer->Add(m_pradioDistanceAnchoring);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    m_pradioMinDistanceAnchoring  = new wxRadioButton(parent, wxID_ANY, _T( "Min Dist. Anchoring" ), wxDefaultPosition, wxSize(132,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(68,1,0);
    l_sizer->Add(m_pradioMinDistanceAnchoring);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    m_pradioCurvature  = new wxRadioButton(parent, wxID_ANY, _T( "Curvature" ), wxDefaultPosition, wxSize(132,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(68,1,0);
    l_sizer->Add(m_pradioCurvature);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    m_pradioTorsion  = new wxRadioButton(parent, wxID_ANY, _T( "Torsion" ), wxDefaultPosition, wxSize(132,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(68,1,0);
    l_sizer->Add(m_pradioTorsion);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_pradioNormalColoring->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnNormalColoring));
    parent->Connect(m_pradioDistanceAnchoring->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnListMenuDistance));
    parent->Connect(m_pradioMinDistanceAnchoring->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnListMenuMinDistance));
    parent->Connect(m_pradioTorsion->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnColorWithTorsion));
    parent->Connect(m_pradioCurvature->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnColorWithCurvature));

    m_pradioNormalColoring->SetValue(m_dh->m_fiberColorationMode == NORMAL_COLOR);
}

void Fibers::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();

    m_ptoggleFiltering->Enable(false);
    m_ptoggleFiltering->SetValue(false);
    m_psliderOpacity->SetValue(m_psliderOpacity->GetMin());
    m_psliderOpacity->Enable(false);
    m_ptoggleNormalColoring->SetValue(!getShowFS());
    m_pradioNormalColoring->Enable(getShowFS());
    m_pradioCurvature->Enable(getShowFS());
    m_pradioDistanceAnchoring->Enable(getShowFS());
    m_pradioMinDistanceAnchoring->Enable(getShowFS());
    m_pradioTorsion->Enable(getShowFS());
}
