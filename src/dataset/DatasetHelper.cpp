/*
 * DatasetHelper.cpp
 *
 *  Created on: 27.07.2008
 *      Author: ralph
 */
#include "DatasetHelper.h"
#include "../gui/MainFrame.h"

#include <memory>
#include <exception>
#include <stdexcept>
#include <new>

#include "Anatomy.h"
#include "ODFs.h"
#include "Fibers.h"
#include "FibersGroup.h"

#include "KdTree.h"
#include "../main.h"
#include "Mesh.h"
#include "SplinePoint.h"
#include "Surface.h"
#include "Tensors.h"
#include "../gui/MyListCtrl.h"
#include "../gui/SelectionBox.h"
#include "../gui/SelectionEllipsoid.h"

#include "../misc/IsoSurface/CIsoSurface.h"
#include "Surface.h"
#include "../misc/nifti/nifti1_io.h"

void out_of_memory() 
{
    cerr << "Error : Out of memory! \n";
    throw bad_alloc();
}

///////////////////////////////////////////////////////////////////////////
// Constructor
DatasetHelper::DatasetHelper( MainFrame *mf ) :
    m_rows          ( 1    ),
    m_columns       ( 1    ),
    m_frames        ( 1    ),
    m_floatDataset  ( NULL ),

    m_xVoxel     ( 1.0f ),
    m_yVoxel     ( 1.0f ),
    m_zVoxel     ( 1.0f ),

	m_niftiTransform( 4, 4 ),

    m_countFibers( 0    ),

    m_scnFileLoaded      ( false ),
    m_anatomyLoaded      ( false ),
	m_fibersGroupLoaded	 ( false ),
    m_fibersLoaded       ( false ),
    m_vectorsLoaded      ( false ),
    m_tensorsFieldLoaded ( false ),
    m_tensorsLoaded      ( false ),
    m_ODFsLoaded         ( false ),
    m_surfaceLoaded      ( false ),
    m_surfaceIsDirty     ( true  ),

    m_useVBO( true ),
    m_lastGLError( GL_NO_ERROR ),
    m_quadrant( 6 ),
    m_textures( 0 ),

    m_scheduledReloadShaders( true  ),
    m_scheduledScreenshot   ( false ),

    m_showObjects  ( true  ),
    m_activateObjects ( true),
    m_blendAlpha   ( false ),
    m_pointMode    ( false ),
    m_isShowAxes   ( false ),
	m_pColorData   ( NULL ),
    m_animationStep( 0     ),

#ifdef DEBUG
            m_debugLevel( 0 ),
#else
            m_debugLevel( 1 ),
#endif

    m_isRulerToolActive( false ),
    m_rulerFullLength(0),
    m_rulerPartialLength(0),
    m_fibersSamplingFactor(1),
    m_isSegmentActive( false ),
	m_SegmentMethod(0),
	m_isFloodfillActive ( true ),
	m_isSelectBckActive ( false ),
	m_isSelectObjActive ( false ),
	m_isObjfilled ( false ),
	m_isBckfilled ( false ),
	m_isObjCreated ( false ),
	m_isBckCreated ( false ),
	m_isBoxCreated ( false ),
	m_thresSliderMoved ( false ),
	m_showSagittal( true ),
	m_showCoronal ( true ),
	m_showAxial   ( true ),	
	m_showCrosshair( false ),

    m_xSlize( 0.5 ),
    m_ySlize( 0.5 ),
    m_zSlize( 0.5 ),

    m_lighting      ( true ),
    m_blendTexOnMesh( true ),
    m_useLic        ( false ),
    m_drawVectors   ( false ),

    m_normalDirection( 1.0 ),

    m_clearToBlack   ( false ),
    m_filterIsoSurf  ( false ),

    m_colorMap( 0 ),
    m_showColorMapLegend       ( false        ),
    m_displayMinMaxCrossSection( false        ),
    m_displayGlyphOptions      ( false        ),

    m_morphing     ( false ),
    m_boxLockIsOn  ( false ),
    m_semaphore    ( false ),
    m_threadsActive( 0 ),

    m_isDragging ( false ),
    m_isrDragging( false ),
    m_ismDragging( false ),
    m_zoom ( 1 ),
    m_xMove( 0 ),
    m_yMove( 0 ),

    m_texAssigned  ( false ),
    m_selBoxChanged( true ),
    m_guiBlocked   ( false ),

    m_geforceLevel( 6 ),

    m_lastError     ( _T( "" ) ),
    m_lastPath      ( MyApp::respath + _T( "data" ) ),
    m_scenePath     ( _T( "" ) ),
    m_scnFileName   ( _T( "" ) ),
    m_screenshotPath( _T( "" ) ),
    m_screenshotName( _T( "" ) ),

    m_anatomyHelper     ( 0 ),
	m_boxAtCrosshair    ( 0 ),
	m_lastSelectedPoint ( 0 ),
	m_lastSelectedObject( 0 ),
    m_mainFrame			( mf ),
    m_theScene          ( 0 ),
	m_shaderHelper      ( 0 )
{
    Matrix4fSetIdentity( &m_transform );
}

DatasetHelper::~DatasetHelper()
{
    printDebug( _T( "execute dataset helper destructor" ), 0 );

    if ( m_theScene )
        delete m_theScene;

    if ( m_anatomyHelper )
        delete m_anatomyHelper;

	if ( m_shaderHelper )
        delete m_shaderHelper;

	//Not causing Memory leaks for now!!! But should be deleted, if allocated.
	//if ( m_boxAtCrosshair )
	//	delete m_boxAtCrosshair;
	//if ( m_lastSelectedPoint )
	//	delete m_lastSelectedPoint;
	//if ( m_lastSelectedObject )
	//	delete m_lastSelectedObject;
	//if ( m_mainFrame )
	//	delete m_mainFrame;
    
    printDebug( _T( "dataset helper destructor done" ), 0 );
}

bool DatasetHelper::load( const int i_index )
{
    wxArrayString l_fileNames;
    wxString l_caption          = wxT( "Choose a file" );
	wxString l_wildcard         = wxT( "*.*|*.*|Nifti (*.nii)|*.nii*|Mesh files (*.mesh)|*.mesh|Mesh files (*.surf)|*.surf|Mesh files (*.dip)|*.dip|Fibers VTK/DMRI (*.fib)|*.fib|Fibers PTK (*.bundlesdata)|*.bundlesdata|Fibers TrackVis (*.trk)|*.trk|Fibers MRtrix (*.tck)|*.tck|Scene Files (*.scn)|*.scn|Tensor files (*.nii*)|*.nii|ODF files (*.nii)|*.nii*" );
    wxString l_defaultDir       = wxEmptyString;
    wxString l_defaultFileName  = wxEmptyString;
    wxFileDialog dialog( m_mainFrame, l_caption, l_defaultDir, l_defaultFileName, l_wildcard, wxOPEN | wxFD_MULTIPLE );
    dialog.SetFilterIndex( i_index );
    dialog.SetDirectory( m_lastPath );
    if( dialog.ShowModal() == wxID_OK )
    {
        m_lastPath = dialog.GetDirectory();
        dialog.GetPaths( l_fileNames );
    }
    else
        return true;

    bool l_flag = true;
    for( size_t i = 0; i < l_fileNames.size(); ++i )
    {
        if( ! load( l_fileNames[i], i_index ) && l_flag )
            l_flag = false;
    }

    return l_flag;
}

bool DatasetHelper::load( wxString i_fileName, int i_index, const float i_threshold, const bool i_active, const bool i_showFS, const bool i_useTex, const float i_alpha )
{
	std::set_new_handler(&out_of_memory);
	
    try 
    {
		// check if i_fileName is valid
		if( ! wxFile::Exists( i_fileName ) )
		{
			printf( "File " );
			printwxT( i_fileName );
			printf( " doesn't exist!\n" );
			m_lastError = wxT( "File doesn't exist!" );
			return false;
		}

		// If the file is in compressed formed, we check what kinda file it is.
		wxString l_ext = i_fileName.AfterLast( '.' );
		if( l_ext == _T( "gz" ) )
		{
			wxString l_tmpName = i_fileName.BeforeLast( '.' );
			l_ext = l_tmpName.AfterLast( '.' );
		}

		if( l_ext == wxT( "scn" ) )
		{
			if( ! loadScene( i_fileName ) )
			{
				return false;
			}

			m_selBoxChanged = true;
			m_mainFrame->refreshAllGLWidgets();

		#ifdef __WXMSW__
			m_scnFileName = i_fileName.AfterLast ( '\\' );
			m_scenePath   = i_fileName.BeforeLast( '\\' );
		#else
			m_scnFileName = i_fileName.AfterLast ( '/' );
			m_scenePath   = i_fileName.BeforeLast( '/' );
		#endif
			m_scnFileLoaded = true;
			return true;
		}   
		else if( l_ext == _T( "nii" ) )
		{
			char* l_hdrFile;
			l_hdrFile = (char*)malloc( i_fileName.length() + 1 );
			strcpy( l_hdrFile, (const char*)i_fileName.mb_str( wxConvUTF8 ) );

			nifti_image* l_image = nifti_image_read( l_hdrFile, 0 );

			free(l_hdrFile);

			if( ! l_image )
			{
				m_lastError = wxT( "nifti file corrupt, cannot create nifti image from header" );
				return false;
			}

			if (l_image->datatype == 16 && l_image->ndim == 4 && l_image->dim[4] == 6)
			{
				i_index=8;
			}
			else if (l_image->datatype == 16 && l_image->ndim == 4 && (l_image->dim[4] == 0 || l_image->dim[4] == 15 || l_image->dim[4] == 28 || l_image->dim[4] == 45 || l_image->dim[4] == 66 || l_image->dim[4] == 91 || l_image->dim[4] == 120 || l_image->dim[4] == 153 ))
			{
				i_index=9;
			}

			DatasetInfo *l_dataset = NULL;
			if (i_index==8)
			{
				if( ! m_anatomyLoaded )
				{
					m_lastError = wxT( "no anatomy file loaded" );
					return false;
				}

				if( m_tensorsLoaded )
				{
					m_lastError = wxT( "tensors already loaded" );
					return false;
				}
				l_dataset = new Tensors( this );            
			}
			else if (i_index==9)
			{
				if( ! m_anatomyLoaded )
				{
					m_lastError = wxT( "no anatomy file loaded" );
					return false;
				}
				l_dataset = new ODFs( this );            
			}
			else
			{
				l_dataset = new Anatomy( this );
			}
			if( l_dataset->load(i_fileName ))
			{
				l_dataset->setThreshold( i_threshold );
				l_dataset->setAlpha    ( i_alpha );
				l_dataset->setShow     ( i_active );
				l_dataset->setShowFS   ( i_showFS );
				l_dataset->setuseTex   ( i_useTex );
				finishLoading( l_dataset );        
				if (i_index==8)
				{
					m_tensorsLoaded = true;
				}
				else if (i_index == 9)
				{
					m_ODFsLoaded = true;
				}
				else
				{
					m_floatDataset = ((Anatomy*)l_dataset)->getFloatDataset();
				}
				return true;
			}
			return false;
		}
		else if( l_ext == _T( "mesh" ) || l_ext == _T( "surf" ) || l_ext == _T( "dip" ) )
		{
			if( ! m_anatomyLoaded )
			{
				m_lastError = wxT( "no anatomy file loaded" );
				return false;
			}

			Mesh *l_mesh = new Mesh( this );

			if( l_mesh->load( i_fileName ) )
			{
				l_mesh->setThreshold( i_threshold );
				l_mesh->setShow     ( i_active );
				l_mesh->setShowFS   ( i_showFS );
				l_mesh->setuseTex   ( i_useTex );
				finishLoading       ( l_mesh );
				return true;
			}
			return false;
		}
		else if( l_ext == _T( "fib" ) || l_ext == _T( "trk" ) || l_ext == _T( "bundlesdata" ) || l_ext == _T( "Bfloat" ) || l_ext == _T("tck") )
		{
			if( ! m_anatomyLoaded )
			{
				m_lastError = wxT( "no anatomy file loaded" );
				return false;
			}

			Fibers* l_fibers = new Fibers( this );

			if( l_fibers->load( i_fileName ) )
			{
				if( m_fibersGroupLoaded == false )
				{
					FibersGroup* l_fibersGroup = new FibersGroup( this );
					l_fibersGroup->setName( wxT( "Fibers Group" ) );
					l_fibersGroup->setShow(true);
					l_fibersGroup->setType(FIBERSGROUP);
					finishLoading( l_fibersGroup );

					m_fibersGroupLoaded = true;
				}

				std::vector< std::vector< SelectionObject* > > l_selectionObjects = getSelectionObjects();
				for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
				{
					for( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
					{
						l_selectionObjects[i][j]->m_inBox.resize( m_countFibers, sizeof(bool) );
						for( unsigned int k = 0; k < m_countFibers; ++k )
						{
							l_selectionObjects[i][j]->m_inBox[k] = 0;
						}

						l_selectionObjects[i][j]->setIsDirty( true );
					}
				}

				l_fibers->setThreshold( i_threshold );
				l_fibers->setShow     ( i_active );
				l_fibers->setShowFS   ( i_showFS );
				l_fibers->setuseTex   ( i_useTex );
				
				if( m_fibersGroupLoaded )
				{
					FibersGroup* pFibersGroup;
					getFibersGroupDataset(pFibersGroup);

					if( pFibersGroup != NULL )
					{
						if(pFibersGroup->getFibersCount() > 0)
						{
							l_fibers->setShow( false );
						}
						pFibersGroup->addFibersSet( l_fibers );
					}
				}			

				l_fibers->updateLinesShown();
				m_mainFrame->refreshAllGLWidgets();
				
				finishLoading( l_fibers, true );
				m_fibersLoaded = true;
				m_selBoxChanged = true;

				return true;
			}
			return false;
		}
		m_lastError = wxT( "unsupported file format" );
		
		return false;
	}
	catch (const exception &e)
	{
		cerr << "Exception: " << e.what() << endl;
		exit(1);
	}
}

void DatasetHelper::updateItemsId()
{
	for(int i = 0; i < m_mainFrame->m_pListCtrl->GetItemCount(); i++)
	{
		DatasetInfo* pDatasetInfo = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData(i);
		if(pDatasetInfo != NULL)
		{
			pDatasetInfo->setListCtrlItemId(pDatasetInfo->getListCtrlItemId() + 1);
		}
	}
}

void DatasetHelper::finishLoading( DatasetInfo* i_info, bool isChild )
{
    m_guiBlocked = true;
#ifdef __WXMAC__
    // insert at zero is a well-known bug on OSX, so we append there...
    // http://trac.wxwidgets.org/ticket/4492
    long l_id = m_mainFrame->m_pListCtrl->GetItemCount();
#else
    long l_id = 0;
	updateItemsId();
#endif
	i_info->setListCtrlItemId(l_id);
	
	m_mainFrame->m_pListCtrl->InsertItem( l_id, wxT( "" ), 0 );

    if( i_info->getShow() )
        m_mainFrame->m_pListCtrl->SetItem( l_id, 0, wxT( "" ), 0 );
    else
        m_mainFrame->m_pListCtrl->SetItem( l_id, 0, wxT( "" ), 1 );

    if( ! i_info->getShowFS() )
        m_mainFrame->m_pListCtrl->SetItem( l_id, 1, i_info->getName().BeforeFirst( '.' ) + wxT( "*" ) );
    else
        m_mainFrame->m_pListCtrl->SetItem( l_id, 1, i_info->getName().BeforeFirst( '.' ));

    if( ! i_info->getUseTex() )
        m_mainFrame->m_pListCtrl->SetItem( 0, 2, wxT( "(" ) + wxString::Format( wxT( "%.2f" ), ( i_info->getThreshold() ) * i_info->getOldMax() ) + wxT( ")" ) );
    else
        m_mainFrame->m_pListCtrl->SetItem( l_id, 2, wxString::Format( wxT( "%.2f" ), i_info->getThreshold() * i_info->getOldMax() ) );

    m_mainFrame->m_pListCtrl->SetItem( l_id, 3, wxT( "" ), 0 );
    m_mainFrame->m_pListCtrl->SetItemData( l_id, (long)i_info );
	
	m_mainFrame->m_pListCtrl->unselectAll();
	m_mainFrame->m_pListCtrl->SetItemState( l_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
	
	if( isChild )
	{
		m_mainFrame->m_pListCtrl->moveItemDown( l_id );
	}

    m_mainFrame->GetStatusBar()->SetStatusText( wxT( "Ready" ), 1 );
    m_mainFrame->GetStatusBar()->SetStatusText( i_info->getName() + wxT( " loaded" ), 2 );

    if( m_mainFrame->m_pListCtrl->GetItemCount() == 1 )
    {
        m_mainFrame->m_pXSlider->SetMax( wxMax( 2, m_columns - 1 ) );
        m_mainFrame->m_pXSlider->SetValue( m_columns / 2 );
        m_mainFrame->m_pYSlider->SetMax( wxMax( 2, m_rows - 1 ) );
        m_mainFrame->m_pYSlider->SetValue( m_rows / 2 );
        m_mainFrame->m_pZSlider->SetMax( wxMax( 2, m_frames - 1 ) );
        m_mainFrame->m_pZSlider->SetValue( m_frames / 2 );
        
        updateView( m_mainFrame->m_pXSlider->GetValue(), m_mainFrame->m_pYSlider->GetValue(), m_mainFrame->m_pZSlider->GetValue() );

        m_mainFrame->m_pMainGL->changeOrthoSize();
        m_mainFrame->m_pGL0->changeOrthoSize();
        m_mainFrame->m_pGL1->changeOrthoSize();
        m_mainFrame->m_pGL2->changeOrthoSize();
    }

    updateLoadStatus();
    m_guiBlocked = false;
    m_mainFrame->refreshAllGLWidgets();
}

bool DatasetHelper::fileNameExists( const wxString i_fileName )
{
    int l_countDataSets = m_mainFrame->m_pListCtrl->GetItemCount();

    if( l_countDataSets == 0 )
        return false;

    for( int i = 0; i < l_countDataSets; ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_mainFrame->m_pListCtrl->GetItemData( i );
        if ( l_info->getPath() == i_fileName )
        {
            return true;
        }
    }

    return false;
}

bool DatasetHelper::loadScene( const wxString i_fileName )
{
    /*
     * Variables to store the slice postions in, have to be set after loading
     * the anatomy files
     */
    long xp, yp, zp;
    xp = yp = zp = 0;
    double r00, r10, r20, r01, r11, r21, r02, r12, r22;
    r10 = r20 = r01 = r21 = r02 = r12 = 0;
    r00 = r11 = r22 = 1;

    wxXmlDocument l_xmlDoc;
    if( ! l_xmlDoc.Load( i_fileName ) )
        return false;

    wxXmlNode* l_child = l_xmlDoc.GetRoot()->GetChildren();
    while( l_child )
    {
        if( l_child->GetName() == wxT( "anatomy" ) )
        {
            wxString srows      = l_child->GetPropVal( wxT( "rows" ),    wxT( "1" ) );
            wxString scolumns   = l_child->GetPropVal( wxT( "columns" ), wxT( "1" ) );
            wxString sframes    = l_child->GetPropVal( wxT( "frames" ),  wxT( "1" ) );

            long l_rows, l_columns, l_frames;

            srows.ToLong   ( &l_rows, 10 );
            scolumns.ToLong( &l_columns, 10 );
            sframes.ToLong ( &l_frames, 10 );
            if( m_anatomyLoaded )
            {
                if( ( l_rows != m_rows ) || ( l_columns != m_columns ) || ( l_frames != m_frames ) )
                {
                    m_lastError = wxT( "dimensions of loaded files must be the same" );
                    return false;
                }
            }
            else
            {
                m_rows            = l_rows;
                m_columns         = l_columns;
                m_frames          = l_frames;
                m_anatomyLoaded   = true;
            }
        }
        else if( l_child->GetName() == wxT( "position" ) )
        {
            l_child->GetPropVal( wxT( "x" ), wxT( "1" ) ).ToLong( &xp, 10 );
            l_child->GetPropVal( wxT( "y" ), wxT( "1" ) ).ToLong( &yp, 10 );
            l_child->GetPropVal( wxT( "z" ), wxT( "1" ) ).ToLong( &zp, 10 );
        }
        else if( l_child->GetName() == wxT( "rotation" ) )
        {
            //l_child->GetPropVal( wxT( "rot00" ), wxT( "1" ) ).ToDouble( &r00 );
            //l_child->GetPropVal( wxT( "rot10" ), wxT( "1" ) ).ToDouble( &r10 );
            //l_child->GetPropVal( wxT( "rot20" ), wxT( "1" ) ).ToDouble( &r20 );
            //l_child->GetPropVal( wxT( "rot01" ), wxT( "1" ) ).ToDouble( &r01 );
            //l_child->GetPropVal( wxT( "rot11" ), wxT( "1" ) ).ToDouble( &r11 );
            //l_child->GetPropVal( wxT( "rot21" ), wxT( "1" ) ).ToDouble( &r21 );
            //l_child->GetPropVal( wxT( "rot02" ), wxT( "1" ) ).ToDouble( &r02 );
            //l_child->GetPropVal( wxT( "rot12" ), wxT( "1" ) ).ToDouble( &r12 );
            //l_child->GetPropVal( wxT( "rot22" ), wxT( "1" ) ).ToDouble( &r22 );
        }

        else if( l_child->GetName() == wxT( "data" ) )
        {
            wxXmlNode *l_dataSetNode = l_child->GetChildren();
            while( l_dataSetNode )
            {
                wxXmlNode *l_nodes  = l_dataSetNode->GetChildren();
                // initialize to mute compiler
                bool l_active       = true;
                bool l_useTex       = true;
                bool l_showFS       = true;
                double l_threshold  = 0.0;
                double l_alpha      = 1.0;
                wxString l_path;

                while( l_nodes )
                {
                    if( l_nodes->GetName() == _T( "status" ) )
                    {
                        l_active = ( l_nodes->GetPropVal( _T( "active" ), _T( "yes" ) ) == _T( "yes" ) );
                        l_useTex = ( l_nodes->GetPropVal( _T( "useTex" ), _T( "yes" ) ) == _T( "yes" ) );
                        l_showFS = ( l_nodes->GetPropVal( _T( "showFS" ), _T( "yes" ) ) == _T( "yes" ) );
                        ( l_nodes->GetPropVal( wxT( "threshold"), wxT( "0.0" ) ) ).ToDouble( &l_threshold );
                        ( l_nodes->GetPropVal( wxT( "alpha"),     wxT( "0.0" ) ) ).ToDouble( &l_alpha );
                    }
                    else if( l_nodes->GetName() == _T( "path" ) )
                    {
                        l_path = l_nodes->GetNodeContent();
                    }

                    l_nodes = l_nodes->GetNext();
                }
                load( l_path, -1, l_threshold, l_active, l_showFS, l_useTex, l_alpha );
                l_dataSetNode = l_dataSetNode->GetNext();
            }
        }
        else if( l_child->GetName() == wxT( "points" ) )
        {
            wxXmlNode* l_pNode = l_child->GetChildren();
            while( l_pNode )
            {
                wxString l_sx = l_pNode->GetPropVal( wxT( "x" ), wxT( "0.0" ) );
                wxString l_sy = l_pNode->GetPropVal( wxT( "y" ), wxT( "0.0" ) );
                wxString l_sz = l_pNode->GetPropVal( wxT( "z" ), wxT( "0.0" ) );

                double l_x, l_y, l_z;

                l_sx.ToDouble( &l_x );
                l_sy.ToDouble( &l_y );
                l_sz.ToDouble( &l_z );

                SplinePoint* l_point = new SplinePoint( l_x, l_y, l_z, this );
                m_mainFrame->m_pTreeWidget->AppendItem( m_mainFrame->m_tPointId, wxT( "point" ), -1, -1, l_point );
                l_pNode = l_pNode->GetNext();
            }

            if( m_mainFrame->m_pTreeWidget->GetChildrenCount( m_mainFrame->m_tPointId ) > 0 )
            {
                Surface* l_surface = new Surface( this );
#ifdef __WXMAC__
                // insert at zero is a well-known bug on OSX, so we append there...
                // http://trac.wxwidgets.org/ticket/4492
                long l_id = m_mainFrame->m_pListCtrl->GetItemCount();
#else
                long l_id = 0;
#endif
                m_mainFrame->m_pListCtrl->InsertItem( l_id, wxT( "" ), 0 );
                m_mainFrame->m_pListCtrl->SetItem( l_id, 1, _T( "spline surface" ) );
                m_mainFrame->m_pListCtrl->SetItem( l_id, 2, wxT( "0.50" ) );
                m_mainFrame->m_pListCtrl->SetItem( l_id, 3, wxT( "" ), 1 );
                m_mainFrame->m_pListCtrl->SetItemData( l_id, (long)l_surface );
                m_mainFrame->m_pListCtrl->SetItemState( l_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
            }
        }
        else if( l_child->GetName() == wxT( "selection_objects" ) )
        {
            wxXmlNode* l_boxNode = l_child->GetChildren();
            wxTreeItemId l_currentMasterId;

            wxString l_name, l_type, l_active, l_visible, l_isBox;
            double cx, cy, cz, ix, iy, iz;
            double _cx, _cy, _cz, _ix, _iy, _iz;
            cx = cy = cz = ix = iy = iz = 0;
            _cx = _cy = _cz = _ix = _iy = _iz = 0;

            while( l_boxNode )
            {
                wxXmlNode* l_infoNode = l_boxNode->GetChildren();
                while( l_infoNode )
                {
                    if( l_infoNode->GetName() == wxT( "status" ) )
                    {
                        l_type    = l_infoNode->GetPropVal( wxT( "type" ),    wxT( "MASTER" ) );
                        l_active  = l_infoNode->GetPropVal( wxT( "active" ),  wxT( "yes" ) );
                        l_visible = l_infoNode->GetPropVal( wxT( "visible" ), wxT( "yes" ) );
                        l_isBox   = l_infoNode->GetPropVal( wxT( "isBox" ), wxT( "yes" ) );

                    }
                    if( l_infoNode->GetName() == wxT( "name" ) )
                    {
                        l_name = l_infoNode->GetPropVal( wxT( "string" ), wxT( "object" ) );

                    }
                    if( l_infoNode->GetName() == wxT( "size" ) )
                    {
                        wxString sx = l_infoNode->GetPropVal( wxT( "x" ), wxT( "0.0" ) );
                        wxString sy = l_infoNode->GetPropVal( wxT( "y" ), wxT( "0.0" ) );
                        wxString sz = l_infoNode->GetPropVal( wxT( "z" ), wxT( "0.0" ) );

                        sx.ToDouble( &_ix );
                        sy.ToDouble( &_iy );
                        sz.ToDouble( &_iz );
                    }
                    if( l_infoNode->GetName() == wxT( "center" ) )
                    {
                        wxString sx = l_infoNode->GetPropVal( wxT( "x" ), wxT( "0.0" ) );
                        wxString sy = l_infoNode->GetPropVal( wxT( "y" ), wxT( "0.0" ) );
                        wxString sz = l_infoNode->GetPropVal( wxT( "z" ), wxT( "0.0" ) );

                        sx.ToDouble( &_cx );
                        sy.ToDouble( &_cy );
                        sz.ToDouble( &_cz );
                    }

                    l_infoNode = l_infoNode->GetNext();
                }

                Vector l_vc( _cx, _cy, _cz );
                Vector l_vs( _ix, _iy, _iz );

                // get selected l_anatomy dataset
                long l_item = m_mainFrame->m_pListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
                if( l_item == -1 )
                    return false;

                //DatasetInfo* l_info = (DatasetInfo*) m_mainFrame->m_listCtrl->GetItemData( l_item );
                //if( l_info->getType() > OVERLAY )
                //    return false;
                SelectionObject* l_selectionObject;
                if( l_isBox == _T( "yes" ) )
                    l_selectionObject = new SelectionBox( l_vc, l_vs, this );
                else
                    l_selectionObject = new SelectionEllipsoid( l_vc, l_vs, this );

                l_selectionObject->setName( l_name );
                l_selectionObject->setIsActive ( l_active  == _T( "yes" ) );
                l_selectionObject->setIsVisible( l_visible == _T( "yes" ) );

                if( l_type == wxT( "MASTER" ) )
                {
                    l_selectionObject->setIsMaster( true );
                    l_currentMasterId = m_mainFrame->m_pTreeWidget->AppendItem( m_mainFrame->m_tSelectionObjectsId, l_selectionObject->getName(), 0, -1, l_selectionObject );
                    m_mainFrame->m_pTreeWidget->EnsureVisible( l_currentMasterId );
                    m_mainFrame->m_pTreeWidget->SetItemImage( l_currentMasterId, l_selectionObject->getIcon() );
                    m_mainFrame->m_pTreeWidget->SetItemBackgroundColour( l_currentMasterId, *wxCYAN );
                    l_selectionObject->setTreeId( l_currentMasterId );
                }
                else
                {
                    l_selectionObject->setIsNOT( l_type == _T( "NOT" ) );
                    wxTreeItemId boxId = m_mainFrame->m_pTreeWidget->AppendItem( l_currentMasterId, l_selectionObject->getName(), 0, -1, l_selectionObject );
                    m_mainFrame->m_pTreeWidget->EnsureVisible( boxId );
                    m_mainFrame->m_pTreeWidget->SetItemImage( boxId, l_selectionObject->getIcon() );

                    if( l_selectionObject->getIsNOT() )
                        m_mainFrame->m_pTreeWidget->SetItemBackgroundColour( boxId, *wxRED );
                    else
                        m_mainFrame->m_pTreeWidget->SetItemBackgroundColour( boxId, *wxGREEN );

                    l_selectionObject->setTreeId( boxId );
                }
                l_boxNode = l_boxNode->GetNext();
            }
        }
        l_child = l_child->GetNext();
    }

    m_mainFrame->m_pXSlider->SetValue( xp );
    m_mainFrame->m_pYSlider->SetValue( yp );
    m_mainFrame->m_pZSlider->SetValue( zp );
    updateView( xp, yp, zp );

    /*m_transform.s.M00 = r00;
    m_transform.s.M10 = r10;
    m_transform.s.M20 = r20;
    m_transform.s.M01 = r01;
    m_transform.s.M11 = r11;
    m_transform.s.M21 = r21;
    m_transform.s.M02 = r02;
    m_transform.s.M12 = r12;
    m_transform.s.M22 = r22;
    m_mainFrame->m_mainGL->setRotation();*/

    updateLoadStatus();
    return true;
}

void DatasetHelper::save( const wxString i_fileName )
{
    wxXmlNode* l_root                 = new wxXmlNode( NULL,   wxXML_ELEMENT_NODE, wxT( "theScene" ) );
    wxXmlNode* l_nodeSelectionObjects = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "selection_objects" ) );
    wxXmlNode* l_nodePoints           = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "points" ) );
    wxXmlNode* l_data                 = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "data" ) );
    wxXmlNode* l_anatomy              = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "anatomy" ) );
    wxXmlNode* l_rotation             = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "rotation" ) );
    wxXmlNode* l_anatomyPos           = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "position" ) );

    wxXmlProperty* l_prop1 = new wxXmlProperty( wxT( "rows" ),    wxString::Format( wxT( "%d" ), m_rows ) );
    wxXmlProperty* l_prop2 = new wxXmlProperty( wxT( "columns" ), wxString::Format( wxT( "%d" ), m_columns ), l_prop1 );
    wxXmlProperty* l_prop3 = new wxXmlProperty( wxT( "frames" ),  wxString::Format( wxT( "%d" ), m_frames ), l_prop2 );
    l_anatomy->AddProperty( l_prop3 );

    wxXmlProperty* l_rot00 = new wxXmlProperty( wxT( "rot00" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M00 ) );
    wxXmlProperty* l_rot10 = new wxXmlProperty( wxT( "rot10" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M10 ), l_rot00 );
    wxXmlProperty* l_rot20 = new wxXmlProperty( wxT( "rot20" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M20 ), l_rot10 );
    wxXmlProperty* l_rot01 = new wxXmlProperty( wxT( "rot01" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M01 ), l_rot20 );
    wxXmlProperty* l_rot11 = new wxXmlProperty( wxT( "rot11" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M11 ), l_rot01 );
    wxXmlProperty* l_rot21 = new wxXmlProperty( wxT( "rot21" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M21 ), l_rot11 );
    wxXmlProperty* l_rot02 = new wxXmlProperty( wxT( "rot02" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M02 ), l_rot21 );
    wxXmlProperty* l_rot12 = new wxXmlProperty( wxT( "rot12" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M12 ), l_rot02 );
    wxXmlProperty* l_rot22 = new wxXmlProperty( wxT( "rot22" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M22 ), l_rot12 );
    l_rotation->AddProperty( l_rot22 );

    int l_countPoints = m_mainFrame->m_pTreeWidget->GetChildrenCount( m_mainFrame->m_tPointId, true );
    wxTreeItemId l_id, l_childId;
    wxTreeItemIdValue l_cookie = 0;

    for( int i = 0; i < l_countPoints; ++i )
    {
        l_id = m_mainFrame->m_pTreeWidget->GetNextChild( m_mainFrame->m_tPointId, l_cookie );
        SplinePoint* l_point = (SplinePoint*)( m_mainFrame->m_pTreeWidget->GetItemData( l_id ) );
        wxXmlNode* l_pointNode = new wxXmlNode( l_nodePoints, wxXML_ELEMENT_NODE, wxT( "point" ) );

        wxXmlProperty* l_propZ = new wxXmlProperty( wxT( "z" ), wxString::Format( wxT( "%f" ), l_point->getCenter().z ) );
        wxXmlProperty* l_propY = new wxXmlProperty( wxT( "y" ), wxString::Format( wxT( "%f" ), l_point->getCenter().y ), l_propZ );
        wxXmlProperty* l_propX = new wxXmlProperty( wxT( "x" ), wxString::Format( wxT( "%f" ), l_point->getCenter().x ), l_propY );
        l_pointNode->AddProperty( l_propX );
    }

    SelectionObject* l_currentSelectionObject;
    std::vector< std::vector< SelectionObject* > > l_selectionObjects = getSelectionObjects();

    for( unsigned int i = l_selectionObjects.size(); i > 0; --i )
    {
        for( unsigned int j = l_selectionObjects[i - 1].size(); j > 0; --j )
        {
            wxXmlNode* l_selectionObject = new wxXmlNode( l_nodeSelectionObjects, wxXML_ELEMENT_NODE, wxT( "object" ) );
            l_currentSelectionObject = l_selectionObjects[i - 1][j - 1];

            if( ! l_currentSelectionObject->isSelectionObject() )
                continue;

            wxXmlNode* l_center    = new wxXmlNode( l_selectionObject, wxXML_ELEMENT_NODE, wxT( "center" ) );
            wxXmlProperty *l_propZ = new wxXmlProperty( wxT( "z" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getCenter().z ) );
            wxXmlProperty *l_propY = new wxXmlProperty( wxT( "y" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getCenter().y ), l_propZ );
            wxXmlProperty *l_propX = new wxXmlProperty( wxT( "x" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getCenter().x ), l_propY );
            l_center->AddProperty( l_propX );

            wxXmlNode* l_size = new wxXmlNode( l_selectionObject, wxXML_ELEMENT_NODE, wxT( "size" ) );
            l_propZ = new wxXmlProperty( wxT( "z" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getSize().z ) );
            l_propY = new wxXmlProperty( wxT( "y" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getSize().y ), l_propZ );
            l_propX = new wxXmlProperty( wxT( "x" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getSize().x ), l_propY );
            l_size->AddProperty( l_propX );

            wxXmlNode* l_name = new wxXmlNode( l_selectionObject, wxXML_ELEMENT_NODE, wxT( "name" ) );
            wxXmlProperty* l_propName = new wxXmlProperty( wxT( "string" ), l_currentSelectionObject->getName() );
            l_name->AddProperty( l_propName );

            wxXmlNode *status = new wxXmlNode( l_selectionObject, wxXML_ELEMENT_NODE, wxT( "status" ) );
            wxXmlProperty *l_propType;

            if( j - 1 == 0 )
                l_propType = new wxXmlProperty( wxT( "type" ), wxT( "MASTER" ) );
            else
                l_propType = new wxXmlProperty( wxT( "type" ), l_currentSelectionObject->getIsNOT() ? wxT( "NOT" ) : wxT( "AND" ) );

            wxXmlProperty* l_propActive  = new wxXmlProperty( wxT( "active" ),  l_currentSelectionObject->getIsActive()       ? wxT( "yes" ) : wxT( "no" ), l_propType );
            wxXmlProperty* l_propVisible = new wxXmlProperty( wxT( "visible" ), l_currentSelectionObject->getIsVisible()      ? wxT( "yes" ) : wxT( "no" ), l_propActive );
            wxXmlProperty* l_propIsBox   = new wxXmlProperty( wxT( "isBox" ),   (l_currentSelectionObject->getSelectionType() == BOX_TYPE) ? wxT( "yes" ) : wxT( "no" ), l_propVisible );
            
            status->AddProperty( l_propIsBox );
        }
    }

    int l_countTextures = m_mainFrame->m_pListCtrl->GetItemCount();

    if( l_countTextures == 0 )
        return;

    for( int i = 0; i < l_countTextures; ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_mainFrame->m_pListCtrl->GetItemData( i );

        if( l_info->getType() < SURFACE )
        {
            wxXmlNode* l_dataSetNode = new wxXmlNode( l_data, wxXML_ELEMENT_NODE, wxT( "dataset" ) );

            wxXmlNode* l_pathNode = new wxXmlNode( l_dataSetNode, wxXML_ELEMENT_NODE, wxT( "path" ) );
            new wxXmlNode( l_pathNode, wxXML_TEXT_NODE, wxT( "path" ), l_info->getPath() );

            wxXmlNode* l_statusNode = new wxXmlNode( l_dataSetNode, wxXML_ELEMENT_NODE, wxT( "status" ) );
            wxXmlProperty* l_propT  = new wxXmlProperty( wxT( "threshold" ), wxString::Format( wxT( "%.2f" ), l_info->getThreshold() ) );
            wxXmlProperty* l_propTA = new wxXmlProperty( wxT( "alpha" ), wxString::Format( wxT( "%.2f" ), l_info->getAlpha() ), l_propT );
            wxXmlProperty* l_propA  = new wxXmlProperty( wxT( "active" ), l_info->getShow()   ? _T( "yes" ) : _T( "no" ), l_propTA );
            wxXmlProperty* l_propF  = new wxXmlProperty( wxT( "showFS" ), l_info->getShowFS() ? _T( "yes" ) : _T( "no" ), l_propA );
            wxXmlProperty* l_propU  = new wxXmlProperty( wxT( "useTex" ), l_info->getUseTex() ? _T( "yes" ) : _T( "no" ), l_propF );
            l_statusNode->AddProperty( l_propU );
        }
    }

    wxXmlProperty* l_propPosX = new wxXmlProperty( wxT( "x" ), wxString::Format( wxT( "%d" ), m_mainFrame->m_pXSlider->GetValue() ) );
    wxXmlProperty* l_propPosY = new wxXmlProperty( wxT( "y" ), wxString::Format( wxT( "%d" ), m_mainFrame->m_pYSlider->GetValue() ), l_propPosX );
    wxXmlProperty* l_propPosZ = new wxXmlProperty( wxT( "z" ), wxString::Format( wxT( "%d" ), m_mainFrame->m_pZSlider->GetValue() ), l_propPosY );
    l_anatomyPos->AddProperty( l_propPosZ );

    wxXmlDocument l_xmlDoc;
    l_xmlDoc.SetRoot( l_root );

    if ( i_fileName.AfterLast( '.' ) != _T( "scn" ) )
        l_xmlDoc.Save( i_fileName + _T( ".scn" ), 2 );
    else
        l_xmlDoc.Save( i_fileName, 2 );
}

std::vector< std::vector< SelectionObject* > > DatasetHelper::getSelectionObjects()
{
    std::vector< std::vector< SelectionObject* > > l_selectionObjects;

    wxTreeItemId l_id, l_childId;
    wxTreeItemIdValue l_cookie = 0;

    l_id = m_mainFrame->m_pTreeWidget->GetFirstChild( m_mainFrame->m_tSelectionObjectsId, l_cookie );

    while( l_id.IsOk() )
    {
        std::vector< SelectionObject* > l_b;
        l_b.push_back( (SelectionObject*)( m_mainFrame->m_pTreeWidget->GetItemData( l_id ) ) );
        wxTreeItemIdValue childcookie = 0;
        l_childId = m_mainFrame->m_pTreeWidget->GetFirstChild( l_id, childcookie );

        while( l_childId.IsOk() )
        {
            l_b.push_back( (SelectionObject*)( m_mainFrame->m_pTreeWidget->GetItemData( l_childId ) ) );
            l_childId = m_mainFrame->m_pTreeWidget->GetNextChild( l_id, childcookie );
        }

        l_id = m_mainFrame->m_pTreeWidget->GetNextChild( m_mainFrame->m_tSelectionObjectsId, l_cookie );
        l_selectionObjects.push_back( l_b );
    }

    return l_selectionObjects;
}

void DatasetHelper::treeFinished()
{
    m_threadsActive--;

    if ( m_threadsActive > 0 )
        return;

    printDebug( _T( "tree finished" ), 1 );
    m_fibersLoaded = true;
    updateAllSelectionObjects();
    m_selBoxChanged = true;
    m_mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::deleteAllSelectionObjects()
{
    std::vector< std::vector< SelectionObject* > > l_selectionObjects = getSelectionObjects();
    for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
        for( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
        {
            m_mainFrame->m_pTreeWidget->Delete(l_selectionObjects[i][j]->GetId());      
        }
}

void DatasetHelper::deleteAllPoints()
{
    std::vector< std::vector< SplinePoint* > > l_points;

    wxTreeItemId l_id, l_childId;
    wxTreeItemIdValue l_cookie = 0;

    l_id = m_mainFrame->m_pTreeWidget->GetFirstChild( m_mainFrame->m_tPointId, l_cookie );

    while( l_id.IsOk() )
    {
        std::vector< SplinePoint* > l_b;
        l_b.push_back( (SplinePoint*)( m_mainFrame->m_pTreeWidget->GetItemData( l_id ) ) );
        wxTreeItemIdValue childcookie = 0;
        l_childId = m_mainFrame->m_pTreeWidget->GetFirstChild( l_id, childcookie );

        while( l_childId.IsOk() )
        {
            l_b.push_back( (SplinePoint*)( m_mainFrame->m_pTreeWidget->GetItemData( l_childId ) ) );
            l_childId = m_mainFrame->m_pTreeWidget->GetNextChild( l_id, childcookie );
        }

        l_id = m_mainFrame->m_pTreeWidget->GetNextChild( m_mainFrame->m_tPointId, l_cookie );
        l_points.push_back( l_b );
    }

    for( unsigned int i = 0; i < l_points.size(); ++i )
        for( unsigned int j = 0; j < l_points[i].size(); ++j )
        {
            m_mainFrame->m_pTreeWidget->Delete(l_points[i][j]->GetId());      
        }
}

void DatasetHelper::updateAllSelectionObjects()
{
    std::vector< std::vector< SelectionObject* > > l_selectionObjects = getSelectionObjects();
    for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
        for( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
            l_selectionObjects[i][j]->setIsDirty( true );
}


Vector DatasetHelper::mapMouse2World( const int i_x, const int i_y,GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16])
{
    glPushMatrix();
    doMatrixManipulation();
 
    GLfloat l_winX, l_winY;

    l_winX = (float) i_x;
    l_winY = (float) i_viewport[3] - (float) i_y;

    GLdouble l_posX, l_posY, l_posZ;
    gluUnProject( l_winX, l_winY, 0, i_modelview, i_projection, i_viewport, &l_posX, &l_posY, &l_posZ );
    glPopMatrix();

    Vector l_vector( l_posX, l_posY, l_posZ );
    return l_vector;
}

Vector DatasetHelper::mapMouse2WorldBack( const int i_x, const int i_y,GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] )
{   
    GLfloat l_winX, l_winY;

    l_winX = (float) i_x;
    l_winY = (float) i_viewport[3] - (float) i_y;

    GLdouble l_posX, l_posY, l_posZ;
    gluUnProject( l_winX, l_winY, 1, i_modelview, i_projection, i_viewport, &l_posX, &l_posY, &l_posZ );

    Vector l_vector( l_posX, l_posY, l_posZ );
    return l_vector;
}


bool DatasetHelper::GLError()
{
    m_lastGLError = glGetError();

    if( m_lastGLError == GL_NO_ERROR )
        return false;

    return true;
}

bool DatasetHelper::loadTextFile( wxString* i_string, const wxString i_fileName )
{
    wxTextFile l_file;
    *i_string = wxT( "" );

    if ( l_file.Open( i_fileName ) )
    {
        for ( size_t i = 0; i < l_file.GetLineCount(); ++i )
        {
            *i_string += l_file.GetLine( i );
        }
        return true;
    }
    return false;
}

void DatasetHelper::createIsoSurface()
{
    // check l_anatomy - quit if not present
    if( ! m_anatomyLoaded )
        return;

    // get selected l_anatomy dataset
    long l_item = m_mainFrame->m_pListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = (DatasetInfo*) m_mainFrame->m_pListCtrl->GetItemData( l_item );
    if( l_info->getType() > OVERLAY )
        return;

    Anatomy* l_anatomy = (Anatomy*) l_info;

    printDebug( _T( "start generating iso surface..." ), 1 );
    CIsoSurface* isosurf = new CIsoSurface( this, l_anatomy ); 
    isosurf->GenerateSurface( 0.4f );

    printDebug( _T( "iso surface done" ), 1 );

    wxString l_anatomyName = l_anatomy->getName().BeforeFirst( '.' );

    if ( isosurf->IsSurfaceValid() )
    {
        isosurf->setName( l_anatomyName + wxT( " (iso surface)" ) );

#ifdef __WXMAC__
        // insert at zero is a well-known bug on OSX, so we append there...
        // http://trac.wxwidgets.org/ticket/4492
        long l_id = m_mainFrame->m_pListCtrl->GetItemCount();
#else
        long l_id = 0;
#endif
        m_mainFrame->m_pListCtrl->InsertItem( l_id, wxT( "" ), 0 );
        m_mainFrame->m_pListCtrl->SetItem( l_id, 1, isosurf->getName() );
        m_mainFrame->m_pListCtrl->SetItem( l_id, 2, wxT( "0.40" ) );
        m_mainFrame->m_pListCtrl->SetItem( l_id, 3, wxT( "" ), 1 );
        m_mainFrame->m_pListCtrl->SetItemData( l_id, (long) isosurf );
        m_mainFrame->m_pListCtrl->SetItemState( l_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }
    else
    {
        printDebug( _T( "***ERROR*** surface is not valid" ), 2 );
    }

    updateLoadStatus();
    m_mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::createDistanceMapAndIso()
{
    // check l_anatomy - quit if not present
    if( ! m_anatomyLoaded )
        return;

    // get selected l_anatomy dataset
    long l_item = m_mainFrame->m_pListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData( l_item );
    if( l_info->getType() > OVERLAY )
        return;

    Anatomy* l_anatomy = (Anatomy*)l_info;

    printDebug( _T( "start generating distance map..." ), 1 );

    Anatomy* l_newAnatomy = new Anatomy( this, l_anatomy->getFloatDataset() );

    printDebug( _T( "distance map done" ), 1 );

    printDebug( _T( "start generating iso surface..." ), 1 );

    CIsoSurface* isosurf = new CIsoSurface( this, l_newAnatomy );
    isosurf->GenerateSurface( 0.2f );

    printDebug( _T( "iso surface done" ), 1 );

    wxString anatomyName = l_anatomy->getName().BeforeFirst( '.' );

#ifdef __WXMAC__
    // insert at zero is a well-known bug on OSX, so we append there...
    // http://trac.wxwidgets.org/ticket/4492
    long l_id = m_mainFrame->m_pListCtrl->GetItemCount();
#else
    long l_id = 0;
#endif

    if( isosurf->IsSurfaceValid() )
    {
        isosurf->setName( anatomyName + wxT( " (offset)" ) );

        m_mainFrame->m_pListCtrl->InsertItem( l_id, wxT( "" ), 0 );
        m_mainFrame->m_pListCtrl->SetItem( l_id, 1, isosurf->getName() );
        m_mainFrame->m_pListCtrl->SetItem( l_id, 2, wxT( "0.10" ) );
        m_mainFrame->m_pListCtrl->SetItem( l_id, 3, wxT( "" ), 1 );
        m_mainFrame->m_pListCtrl->SetItemData( l_id, (long) isosurf );
        m_mainFrame->m_pListCtrl->SetItemState( l_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

    }
    else
    {
        printDebug( _T( "***ERROR*** surface is not valid" ), 2 );
    }

    delete l_newAnatomy;

    updateLoadStatus();
    m_mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::createDistanceMap()
{
    if( ! m_anatomyLoaded )
        return;

    // get selected l_anatomy dataset
    long l_item = m_mainFrame->m_pListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData( l_item );
    if( l_info->getType() > OVERLAY )
        return;

    Anatomy* l_anatomy = (Anatomy*)l_info;

    printDebug( _T( "start generating distance map..." ), 1 );

    Anatomy* l_newAnatomy = new Anatomy( this, l_anatomy->getFloatDataset() );

    printDebug( _T( "distance map done" ), 1 );

    
    l_newAnatomy->setName( l_anatomy->getName().BeforeFirst('.') + wxT("_DistMap"));

    //Feed the distance to the objects list
    m_mainFrame->m_pListCtrl->InsertItem(0, wxT(""),0);
    m_mainFrame->m_pListCtrl->SetItem(0,1, l_newAnatomy->getName());
    m_mainFrame->m_pListCtrl->SetItem(0,2, wxT("1.0"));
    m_mainFrame->m_pListCtrl->SetItem(0,3, wxT(""),1);

    m_mainFrame->m_pListCtrl->SetItemData(0,(long) l_newAnatomy);

    m_mainFrame->m_pListCtrl->SetItemState(
        0,
        wxLIST_STATE_SELECTED,
        wxLIST_STATE_SELECTED);

    updateLoadStatus();
    m_mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::createCutDataset()
{
    // check l_anatomy - quit if not present
    if( !m_anatomyLoaded )
        return;

    // get selected l_anatomy dataset
    long l_item = m_mainFrame->m_pListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DatasetInfo* info = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData( l_item );
    if ( info->getType() > OVERLAY )
        return;

    Anatomy* l_anatomy = (Anatomy*) info;
    Anatomy* l_newAnatomy = new Anatomy( this );
    l_newAnatomy->setZero( m_columns, m_rows, m_frames );

    std::vector< std::vector< SelectionObject* > > l_selectionObjects = getSelectionObjects();
    int x1, x2, y1, y2, z1, z2;

    for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
    {
        for( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
        {
            if( l_selectionObjects[i][j]->getIsVisible() )
            {
                x1 = (int)( l_selectionObjects[i][j]->getCenter().x / m_xVoxel - l_selectionObjects[i][j]->getSize().x / 2 );
                x2 = (int)( l_selectionObjects[i][j]->getCenter().x / m_xVoxel + l_selectionObjects[i][j]->getSize().x / 2 );
                y1 = (int)( l_selectionObjects[i][j]->getCenter().y / m_yVoxel - l_selectionObjects[i][j]->getSize().y / 2 );
                y2 = (int)( l_selectionObjects[i][j]->getCenter().y / m_yVoxel + l_selectionObjects[i][j]->getSize().y / 2 );
                z1 = (int)( l_selectionObjects[i][j]->getCenter().z / m_zVoxel - l_selectionObjects[i][j]->getSize().z / 2 );
                z2 = (int)( l_selectionObjects[i][j]->getCenter().z / m_zVoxel + l_selectionObjects[i][j]->getSize().z / 2 );

                x1 = wxMax(0, wxMin(x1, m_columns));
                x2 = wxMax(0, wxMin(x2, m_columns));
                y1 = wxMax(0, wxMin(y1, m_rows));
                y2 = wxMax(0, wxMin(y2, m_rows));
                z1 = wxMax(0, wxMin(z1, m_frames));
                z2 = wxMax(0, wxMin(z2, m_frames));

                std::vector< float >* l_src = l_anatomy->getFloatDataset();
                std::vector< float >* l_dst = l_newAnatomy->getFloatDataset();

                for( int b = z1; b < z2; ++b )
                {
                    for( int r = y1; r < y2; ++r )
                    {
                        for( int c = x1; c < x2; ++c )
                        {
                            l_dst->at( b * m_rows * m_columns + r * m_columns + c ) = l_src->at( b * m_rows * m_columns + r * m_columns + c );
                        }
                    }
                }
            }
        }
    }

    l_newAnatomy->setName( l_anatomy->getName().BeforeFirst( '.' ) + wxT( " (cut)" ) );
    l_newAnatomy->setType( l_anatomy->getType() );
    l_newAnatomy->setDataType(l_anatomy->getDataType());
    l_newAnatomy->setNewMax(l_anatomy->getNewMax());

    m_mainFrame->m_pListCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_mainFrame->m_pListCtrl->SetItem( 0, 1, l_newAnatomy->getName() );
    m_mainFrame->m_pListCtrl->SetItem( 0, 2, wxT( "0.00" ) );
    m_mainFrame->m_pListCtrl->SetItem( 0, 3, wxT( "" ), 1 );
    m_mainFrame->m_pListCtrl->SetItemData( 0, (long)l_newAnatomy );
    m_mainFrame->m_pListCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

    updateLoadStatus();
    m_mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::changeZoom( const int i_z )
{
    float delta = ( (int)m_zoom ) * 0.1f;
    i_z >= 0 ? m_zoom = wxMin( 50, m_zoom+delta ) : m_zoom = wxMax( 1, m_zoom-delta );
}

void DatasetHelper::moveScene( int i_x, int i_y )
{
    float l_max = (float)wxMax( m_columns * m_xVoxel, wxMax( m_rows * m_yVoxel, m_frames * m_zVoxel ) );
    float l_div = 500.0 / l_max;

    m_xMove -= (float)i_x / l_div;
    m_yMove += (float)i_y / l_div;
}

void DatasetHelper::doMatrixManipulation()
{
    float l_max = (float)wxMax( m_columns * m_xVoxel, wxMax( m_rows * m_yVoxel, m_frames * m_zVoxel) ) / 2.0;
    glTranslatef( l_max + m_xMove, l_max + m_yMove, l_max );
    glScalef( m_zoom, m_zoom, m_zoom );
    glMultMatrixf( m_transform.M );
    glTranslatef( -m_columns * m_xVoxel / 2.0, -m_rows * m_yVoxel / 2.0, -m_frames * m_zVoxel / 2.0 );
}

void DatasetHelper::updateView( const float i_x, const float i_y, const float i_z )
{
    m_xSlize = i_x;
    m_ySlize = i_y;
    m_zSlize = i_z;

    if( m_boxLockIsOn && ! m_semaphore )
        m_boxAtCrosshair->setCenter( i_x, i_y, i_z );

    for( int i = 0; i < m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_datasetInfo = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData( i );
        if( l_datasetInfo->getType() == TENSORS || l_datasetInfo->getType() == ODFS  )
            ( (Glyph*)m_mainFrame->m_pListCtrl->GetItemData( i ) )->refreshSlidersValues();
    }
}

bool DatasetHelper::getFiberDataset( Fibers* &io_f )
{
    io_f = NULL;

    for( int i = 0; i < m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_datasetInfo = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData( i );
        if( l_datasetInfo->getType() == FIBERS )
        {
            io_f = (Fibers*)m_mainFrame->m_pListCtrl->GetItemData( i );
            return true;
        }
    }
    return false;
}

bool DatasetHelper::getSelectedFiberDataset( Fibers* &io_f )
{
	io_f = NULL;

	long selItem = m_mainFrame->m_pListCtrl->GetSelectedItem();

    DatasetInfo* l_datasetInfo = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData( selItem );
    if( l_datasetInfo->getType() == FIBERS)
    {
        io_f = (Fibers*)l_datasetInfo;
        return true;
    }
    return false;
}

bool DatasetHelper::getFibersGroupDataset( FibersGroup* &io_fg )
{
    io_fg = NULL;

    for( int i = 0; i < m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_datasetInfo = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData( i );
        if( l_datasetInfo->getType() == FIBERSGROUP )
        {
            io_fg = (FibersGroup*)l_datasetInfo;
            return true;
        }
    }
    return false;
}

bool DatasetHelper::getSurfaceDataset( Surface *&io_s )
{
    io_s = NULL;

    for( int i = 0; i < m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_datasetInfo = (DatasetInfo*) m_mainFrame->m_pListCtrl->GetItemData( i );
        if( l_datasetInfo->getType() == SURFACE )
        {
            io_s = (Surface*)m_mainFrame->m_pListCtrl->GetItemData( i );
            return true;
        }
    }
    return false;
}

std::vector< float >* DatasetHelper::getVectorDataset()
{
    if( ! m_vectorsLoaded )
        return NULL;

    for( int i = 0; i < m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_datasetInfo = (DatasetInfo*) m_mainFrame->m_pListCtrl->GetItemData( i );
        if( l_datasetInfo->getType() == VECTORS )
        {
            Anatomy* l_anatomy = (Anatomy*)m_mainFrame->m_pListCtrl->GetItemData( i );
            return l_anatomy->getFloatDataset();
        }
    }

    m_vectorsLoaded = false;
    return NULL;
}

TensorField* DatasetHelper::getTensorField()
{
    if( ! m_tensorsFieldLoaded )
        return NULL;

    for( int i = 0; i < m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_datasetInfo = (DatasetInfo*) m_mainFrame->m_pListCtrl->GetItemData( i );
        if( l_datasetInfo->getType() == TENSOR_FIELD || l_datasetInfo->getType() == VECTORS )
        {
            Anatomy* l_anatomy = (Anatomy*)m_mainFrame->m_pListCtrl->GetItemData( i );
            return l_anatomy->getTensorField();
        }
    }

    m_tensorsFieldLoaded = false;
    return NULL;
}

void DatasetHelper::printGLError( const wxString i_function )
{
    printDebug( _T( "***ERROR***: " ) + i_function, 2 );
    printf( " : ERROR: %s\n", gluErrorString( m_lastGLError ) );
}

void DatasetHelper::printTime()
{
    wxDateTime l_dataTime = wxDateTime::Now();
    printf( "[%02d:%02d:%02d] ", l_dataTime.GetHour(), l_dataTime.GetMinute(), l_dataTime.GetSecond() );
}

void DatasetHelper::printwxT( const wxString i_string )
{
    char* l_cstring = (char*)malloc( i_string.length() + 1 );
    strcpy( l_cstring, (const char*)i_string.mb_str( wxConvUTF8 ) );
    printf( "%s", l_cstring );
    free( l_cstring );
}

void DatasetHelper::printDebug( const wxString i_string, const int i_level )
{
    if ( m_debugLevel > i_level )
        return;

    printTime();
    printwxT( i_string + _T( "\n" ) );
}


void DatasetHelper::updateLoadStatus()
{
    m_anatomyLoaded      = false;
    m_meshLoaded         = false;
    m_fibersLoaded       = false;
    m_vectorsLoaded      = false;
    m_tensorsFieldLoaded = false;
    m_tensorsLoaded      = false;
    m_ODFsLoaded         = false;
    m_surfaceLoaded      = false;

    for( int i = 0; i < m_mainFrame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData( i );
		if(info != NULL)
		{
			switch( info->getType() )
			{
				case HEAD_BYTE:
				case HEAD_SHORT:
				case OVERLAY:
				case RGB:
					m_anatomyLoaded      = true;
					break;
				case VECTORS:
					m_anatomyLoaded      = true;
					m_vectorsLoaded      = true;
					m_tensorsFieldLoaded = true;
					break;
				case MESH:
					m_meshLoaded         = true;
					break;
				case TENSOR_FIELD:
					m_tensorsFieldLoaded = true;
					break;
				case FIBERS:
					m_fibersLoaded       = true;
					break;
				case FIBERSGROUP:
					m_fibersGroupLoaded	 = true;
					break;
				case SURFACE:
					m_surfaceLoaded      = true;
					break;
				case ISO_SURFACE:
					m_meshLoaded         = true;
					break;
				case TENSORS:
					m_tensorsLoaded      = true;
					break;
				case ODFS:
					m_ODFsLoaded         = true;
					break;
				default:
					break;
			}
		}
    }
}

void DatasetHelper::doLicMovie( int i_mode )
{
    wxString l_caption          = wxT( "Choose a file" );
    wxString l_wildcard         = wxT( "PPM files (*.ppm)|*.ppm|*.*|*.*" );
    wxString defaultDir         = wxEmptyString;
    wxString l_defaultFileName  = wxEmptyString;
    wxFileDialog l_dialog( m_mainFrame, l_caption, defaultDir, l_defaultFileName, l_wildcard, wxSAVE );
    wxString l_tmpFileName      = _T( "" );

    l_dialog.SetFilterIndex( 0 );
    l_dialog.SetDirectory( m_screenshotPath );
    l_dialog.SetFilename( _T( "image" ) );

    if( l_dialog.ShowModal() == wxID_OK )
    {
        m_screenshotPath = l_dialog.GetDirectory();
        l_tmpFileName    = l_dialog.GetPath();
    }
    else
        return;

    int l_gf = m_geforceLevel;
    m_geforceLevel = 99;

    switch( i_mode )
    {
        case 0:
            for( int i = 0; i < m_columns; ++i )
            {
                if( i < 100 )
                    m_screenshotName = l_tmpFileName + _T( "0" ) + wxString::Format( wxT( "%d" ), i ) + _T( ".ppm" );
                else
                    m_screenshotName = l_tmpFileName + wxString::Format( wxT( "%d" ), i ) + _T( ".ppm" );

                if( i < 10 )
                    m_screenshotName = l_tmpFileName + _T( "00" ) + wxString::Format( wxT( "%d" ), i ) + _T( ".ppm" );

                createLicSliceSag( i );
            }break;

        case 1:
            for( int i = 0; i < m_rows; ++i )
            {
                if ( i < 100 )
                    m_screenshotName = l_tmpFileName + _T( "0" ) + wxString::Format( wxT( "%d" ), i ) + _T( ".ppm" );
                else
                    m_screenshotName = l_tmpFileName + wxString::Format( wxT( "%d" ), i ) + _T( ".ppm" );

                if ( i < 10 )
                    m_screenshotName = l_tmpFileName + _T( "00" ) + wxString::Format( wxT("%d"), i ) + _T( ".ppm" );

                createLicSliceCor( i );
            } break;

        case 2:
            for ( int i = 0; i < m_frames; ++i )
            {
                if ( i < 100 )
                    m_screenshotName = l_tmpFileName + _T( "0" ) + wxString::Format( wxT( "%d" ), i ) + _T( ".ppm" );
                else
                    m_screenshotName = l_tmpFileName + wxString::Format( wxT( "%d" ), i ) + _T( ".ppm" );

                if ( i < 10 )
                    m_screenshotName = l_tmpFileName + _T( "00" ) + wxString::Format( wxT( "%d" ), i ) + _T( ".ppm" );

                createLicSliceAxi( i );
            }break;

        default:
            break;
    }

    m_geforceLevel = l_gf;
}

void DatasetHelper::licMovieHelper()
{
    Surface* l_surface = new Surface( this );
    l_surface->execute();

#ifdef __WXMAC__
    // insert at zero is a well-known bug on OSX, so we append there...
    // http://trac.wxwidgets.org/ticket/4492
    long l_id = m_mainFrame->m_pListCtrl->GetItemCount();
#else
    long l_id = 0;
#endif

    m_mainFrame->m_pListCtrl->InsertItem( l_id, wxT( "" ), 0 );
    m_mainFrame->m_pListCtrl->SetItem( l_id, 1, l_surface->getName() );
    m_mainFrame->m_pListCtrl->SetItem( l_id, 2, wxT( "0.50" ) );
    m_mainFrame->m_pListCtrl->SetItem( l_id, 3, wxT( "" ), 1 );
    m_mainFrame->m_pListCtrl->SetItemData( l_id, (long)l_surface );
    m_mainFrame->m_pListCtrl->SetItemState( l_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

    m_surfaceLoaded = true;
    l_surface->activateLIC();

    m_scheduledScreenshot = true;
    m_mainFrame->m_pMainGL->render();
    m_mainFrame->m_pMainGL->render();

    delete l_surface;
    m_mainFrame->m_pListCtrl->DeleteItem( l_id );

}

void DatasetHelper::createLicSliceSag( int i_slize )
{
    int l_xs = (int)( i_slize * m_xVoxel );

    //delete all existing points
    m_mainFrame->m_pTreeWidget->DeleteChildren( m_mainFrame->m_tPointId );

    for( int i = 0; i < 11; ++i )
    {
        for( int j = 0; j < 11; ++j )
        {
            int yy = (int)( ( m_rows   / 10 * m_yVoxel ) * i );
            int zz = (int)( ( m_frames / 10 * m_zVoxel ) * j );

            // create the l_point
            SplinePoint* l_point = new SplinePoint( l_xs, yy, zz, this );

            if ( i == 0 || i == 10 || j == 0 || j == 10 )
            {
                wxTreeItemId l_treeId = m_mainFrame->m_pTreeWidget->AppendItem( m_mainFrame->m_tPointId, wxT( "boundary point" ), -1, -1, l_point );
                l_point->setTreeId( l_treeId );
                l_point->setIsBoundary( true );
            }
        }
    }
    licMovieHelper();
}

void DatasetHelper::createLicSliceCor( int i_slize )
{
    int l_ys = (int)( i_slize * m_yVoxel );

    //delete all existing points
    m_mainFrame->m_pTreeWidget->DeleteChildren( m_mainFrame->m_tPointId );

    for( int i = 0; i < 11; ++i )
    {
        for( int j = 0; j < 11; ++j )
        {
            int xx = (int) ( ( m_columns / 10 * m_xVoxel ) * i );
            int zz = (int) ( ( m_frames  / 10 * m_zVoxel ) * j );

            // create the point
            SplinePoint* l_point = new SplinePoint( xx, l_ys, zz, this );

            if( i == 0 || i == 10 || j == 0 || j == 10 )
            {
                wxTreeItemId l_treeId = m_mainFrame->m_pTreeWidget->AppendItem( m_mainFrame->m_tPointId, wxT( "boundary point" ), -1, -1, l_point );
                l_point->setTreeId( l_treeId );
                l_point->setIsBoundary( true );
            }
        }
    }

    licMovieHelper();
}

void DatasetHelper::createLicSliceAxi( int i_slize )
{
    int l_zs = (int)( i_slize * m_zVoxel );

    //delete all existing points
    m_mainFrame->m_pTreeWidget->DeleteChildren( m_mainFrame->m_tPointId );

    for( int i = 0; i < 11; ++i )
    {
        for( int j = 0; j < 11; ++j )
        {
            int xx = (int)( ( m_columns / 10 * m_xVoxel ) * i );
            int yy = (int)( ( m_rows    / 10 * m_yVoxel ) * j );

            // create the l_point
            SplinePoint *l_point = new SplinePoint( xx, yy, l_zs, this );

            if( i == 0 || i == 10 || j == 0 || j == 10 )
            {
                wxTreeItemId l_treeId = m_mainFrame->m_pTreeWidget->AppendItem( m_mainFrame->m_tPointId, wxT( "boundary point" ), -1, -1, l_point );
                l_point->setTreeId( l_treeId );
                l_point->setIsBoundary( true );
            }
        }
    }

    licMovieHelper();
}

void DatasetHelper::increaseAnimationStep()
{
    m_animationStep = ( m_animationStep + 1 ) % 1000000;
}
