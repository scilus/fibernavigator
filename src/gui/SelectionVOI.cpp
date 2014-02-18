
#include "SelectionVOI.h"

#include "../main.h"
#include "../dataset/Anatomy.h"
#include "../dataset/DatasetManager.h"

#include "../misc/XmlHelper.h"
#include "../misc/Algorithms/Helper.h"
#include "../misc/IsoSurface/CBoolIsoSurface.h"
#include "../misc/IsoSurface/TriangleMesh.h"

#include <algorithm>
#include <functional>
#include <limits>

SelectionVOI::SelectionVOI( Anatomy *pSourceAnatomy, const float threshold, const ThresholdingOperationType opType )
    : SelectionObject( Vector( 0.0f, 0.0f, 0.0f ), Vector( 0.0f, 0.0f, 0.0f ) ),
      m_voiSize( 0 ),
      m_generationThreshold( threshold ),
      m_thresType( opType ),
      m_pIsoSurface( NULL ),
      m_sourceAnatIndex( pSourceAnatomy->getDatasetIndex() )
{   
    wxString mystring(wxT("[VOI] - ") + pSourceAnatomy->getName());
    m_name          = mystring;
    m_objectType    = VOI_TYPE;
    
    buildSurface( pSourceAnatomy );
    
    setColor( wxColour( 240, 30, 30 ) );
}

SelectionVOI::SelectionVOI( const wxXmlNode selObjNode )
: SelectionObject( selObjNode ),
  m_voiSize( 0 ),
  m_generationThreshold( 0.0f ),
  m_thresType( THRESHOLD_INVALID ),
  m_pIsoSurface( NULL ),
  m_sourceAnatIndex( 0 )
{
    m_objectType = VOI_TYPE;
    
    wxXmlNode *pChildNode = selObjNode.GetChildren();
    
    while( pChildNode != NULL )
    {
        wxString nodeName = pChildNode->GetName();
        wxString propVal;
        
        if( nodeName == wxT("voi_properties") )
        {
            double temp;
            pChildNode->GetPropVal( wxT("gen_threshold"), &propVal );
            propVal.ToDouble(&temp);
            m_generationThreshold = temp;
            
            pChildNode->GetPropVal( wxT("thres_op_type"), &propVal );
            m_thresType = Helper::getThresholdingTypeFromString( propVal );
            
            wxXmlNode *pAnatNode = pChildNode->GetChildren();
            if( pAnatNode->GetName() == wxT("generation_anatomy") )
            {
                wxString anatPath = pAnatNode->GetNodeContent();
                
                // Find the anatomy related to this file.
                vector< Anatomy* > anats = DatasetManager::getInstance()->getAnatomies();
                for( vector< Anatomy* >::iterator anatIt( anats.begin() ); anatIt != anats.end(); ++anatIt )
                {
                    if( (*anatIt)->getPath() == anatPath )
                    {
                        m_sourceAnatIndex = (*anatIt)->getDatasetIndex();
                        break;
                    }
                }
                
                if( !m_sourceAnatIndex.isOk() )
                {
                    wxString err( wxT("Anatomy: ") );
                    err << anatPath << wxT(" does not exist when creating VOI called: ") << getName() << wxT(".");
                    throw  err;
                }
                
                Anatomy *pCurAnat = dynamic_cast< Anatomy* >( DatasetManager::getInstance()->getDataset( m_sourceAnatIndex ) );
                buildSurface( pCurAnat );
            }
        }
        
        pChildNode = pChildNode->GetNext();
    }
}

void SelectionVOI::buildSurface( Anatomy *pSourceAnatomy )
{
    m_nbRows   = pSourceAnatomy->getRows();
    m_nbCols   = pSourceAnatomy->getColumns();
    m_nbFrames = pSourceAnatomy->getFrames();
    
    vector< float > * const pAnatDataset = pSourceAnatomy->getFloatDataset();
    
    m_includedVoxels.assign( pAnatDataset->size(), false );
    
    if( m_thresType == THRESHOLD_EQUAL )
    {
        std::transform( pAnatDataset->begin(), pAnatDataset->end(),
                       m_includedVoxels.begin(), bind2nd( std::equal_to< float >(), m_generationThreshold ) );
    }
    else if( m_thresType == THRESHOLD_GREATER )
    {
        std::transform( pAnatDataset->begin(), pAnatDataset->end(),
                       m_includedVoxels.begin(), bind2nd( std::greater< float >(), m_generationThreshold ) );
    }
    else if( m_thresType == THRESHOLD_GREATER_EQUAL )
    {
        std::transform( pAnatDataset->begin(), pAnatDataset->end(),
                       m_includedVoxels.begin(), bind2nd( std::greater_equal< float >(), m_generationThreshold ) );
    }
    else if( m_thresType == THRESHOLD_SMALLER )
    {
        std::transform( pAnatDataset->begin(), pAnatDataset->end(),
                       m_includedVoxels.begin(), bind2nd( std::less< float >(), m_generationThreshold ) );
    }
    else if( m_thresType == THRESHOLD_SMALLER_EQUAL )
    {
        std::transform( pAnatDataset->begin(), pAnatDataset->end(),
                       m_includedVoxels.begin(), bind2nd( std::less_equal< float >(), m_generationThreshold ) );
    }
    
    m_pIsoSurface = new CBoolIsoSurface( m_includedVoxels );
    m_pIsoSurface->GenerateSurface();
    
    // Compute the size and position of the bounding box of the VOI.
    unsigned int xIdxMin( m_nbCols );
    unsigned int yIdxMin( m_nbRows );
    unsigned int zIdxMin( m_nbFrames );
    unsigned int xIdxMax( 0 );
    unsigned int yIdxMax( 0 );
    unsigned int zIdxMax( 0 );
    
    unsigned int dataIdx( 0 );
    for( unsigned int zPos( 0 ); zPos < m_nbFrames; ++zPos )
    {
        for( unsigned int yPos( 0 ); yPos < m_nbRows; ++yPos )
        {
            for( unsigned int xPos( 0 ); xPos < m_nbCols; ++xPos, ++dataIdx )
            {
                if( m_includedVoxels[dataIdx] )
                {
                    xIdxMin = std::min( xIdxMin, xPos );
                    yIdxMin = std::min( yIdxMin, yPos );
                    zIdxMin = std::min( zIdxMin, zPos );
                    xIdxMax = std::max( xIdxMax, xPos );
                    yIdxMax = std::max( yIdxMax, yPos );
                    zIdxMax = std::max( zIdxMax, zPos );
                }
            }
        }
    }
    
    // Convert to space coordinates
    DatasetManager *pDM( DatasetManager::getInstance() );
    float spaceXMin( xIdxMin * pDM->getVoxelX() );
    float spaceYMin( yIdxMin * pDM->getVoxelY() );
    float spaceZMin( zIdxMin * pDM->getVoxelZ() );
    
    // For the max value, since we want to grab all of the voxel,
    // and the idx * the size of the voxel gives the begininng of the voxel
    // in space, we adjust with the +1, then the - 0.1 * voxelSize to
    // make sure not to fall in an inexisting coordinate.
    float spaceXMax( ( xIdxMax + 1 ) * pDM->getVoxelX() - 0.1 * pDM->getVoxelX() );
    float spaceYMax( ( yIdxMax + 1 ) * pDM->getVoxelY() - 0.1 * pDM->getVoxelY() );
    float spaceZMax( ( zIdxMax + 1 ) * pDM->getVoxelZ() - 0.1 * pDM->getVoxelZ() );
    
    setCenter( ( spaceXMax + spaceXMin ) / 2.0f, 
              ( spaceYMax + spaceYMin ) / 2.0f, 
              ( spaceZMax + spaceZMin ) / 2.0f );
    
    setSize( spaceXMax - spaceXMin, spaceYMax - spaceYMin, spaceZMax - spaceZMin );
    
    m_voiSize = std::count( m_includedVoxels.begin(), m_includedVoxels.end(), true );
}

///////////////////////////////////////////////////////////////////////////
// This is the specific implementation to draw a selectionEllipsoid object.
//
// i_color      : The color of the ellipsoid to draw.
///////////////////////////////////////////////////////////////////////////
void SelectionVOI::drawObject( GLfloat * pColor )
{
    wxColour objCol = getColor();
    
    if( objCol.IsOk() )
    {
        glColor4f( objCol.Red() / 255.0f, objCol.Green() / 255.0f, 
                   objCol.Blue() / 255.0f, objCol.Alpha() / 255.0f );
    }
    else
    {
        // When no color has been defined, we use a default red.
        glColor4f( 1.0f, 0.0f, 0.0f, pColor[3] );
    }
    
    m_pIsoSurface->draw();
    /*glColor4f( i_color[0], i_color[1], i_color[2], i_color[3] );
    
    glDepthMask(GL_FALSE);
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    
    
    glPushMatrix();
    glTranslatef( m_center.x, m_center.y, m_center.z );
    GLUquadricObj* l_quadric = gluNewQuadric();
    gluQuadricNormals( l_quadric, GLU_SMOOTH );
    glScalef( m_xRadius, m_yRadius, m_zRadius );
    gluSphere( l_quadric, 1.0f, 32, 32 );
    glPopMatrix();
    
    glDisable( GL_BLEND );
    glDepthMask(GL_TRUE);*/
}

hitResult SelectionVOI::hitTest( Ray* i_ray )
{    
    hitResult hr = { false, 0.0f, 0, NULL };
    
    m_hitResult = hr;
    
    return m_hitResult;
}

bool SelectionVOI::isPointInside( const float xPos, const float yPos, const float zPos ) const
{
    DatasetManager *pDM( DatasetManager::getInstance() );
    unsigned int xVoxelCoord( static_cast< unsigned int >( ( xPos / pDM->getVoxelX() ) ) );
    unsigned int yVoxelCoord( static_cast< unsigned int >( ( yPos / pDM->getVoxelY() ) ) );
    unsigned int zVoxelCoord( static_cast< unsigned int >( ( zPos / pDM->getVoxelZ() )  ) );
    
    unsigned int dataCoord( zVoxelCoord * m_nbCols * m_nbRows + yVoxelCoord * m_nbCols + xVoxelCoord );
    
    return m_includedVoxels.at( dataCoord );
}

void SelectionVOI::flipNormals()
{
    if( m_pIsoSurface != NULL && m_pIsoSurface->m_tMesh != NULL)
    {
        m_pIsoSurface->m_tMesh->flipNormals();
        m_pIsoSurface->clean();
    }
}

wxString SelectionVOI::getTypeTag() const
{
    return wxT( "voi" );
}

bool SelectionVOI::populateXMLNode( wxXmlNode *pCurNode )
{
    bool result( SelectionObject::populateXMLNode( pCurNode ) );
    
    if( result )
    {
        wxString floatPrecision = wxT( ".8" );
        
        wxXmlNode *pVoiNode = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "voi_properties" ) );
        pCurNode->AddChild( pVoiNode );
        
        pVoiNode->AddProperty( new wxXmlProperty( wxT( "gen_threshold" ), wxStrFormat( m_generationThreshold, floatPrecision ) ) );
        pVoiNode->AddProperty( new wxXmlProperty( wxT( "thres_op_type" ), Helper::getThresholdingTypeString( m_thresType ) ) );
        
        wxXmlNode *pVoiGenAnatPath = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "generation_anatomy" ) );
        pVoiNode->AddChild( pVoiGenAnatPath );
        
        // Get the dataset to be able to get the index.
        DatasetInfo *pDS = DatasetManager::getInstance()->getDataset( m_sourceAnatIndex );
        if( pDS )
        {
            pVoiGenAnatPath->AddChild( new wxXmlNode( NULL, wxXML_TEXT_NODE, wxT( "path"), pDS->getPath() ) );
        }
        else
        {
            // Should never happen.
            result = false;
        }
    }
    
    return result;
}

void SelectionVOI::createPropertiesSizer( PropertiesWindow *pParent )
{
    SelectionObject::createPropertiesSizer( pParent );
    
    m_pPropertiesSizer->AddSpacer( 8 );
    m_pPropertiesSizer->Add( new wxStaticText( pParent, wxID_ANY, wxT( "VOI specific: " ),wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER ), 0, wxALIGN_CENTER );

    m_pVOISize = new wxTextCtrl( pParent, wxID_ANY, wxString::Format( wxT("%d"), m_voiSize ), wxDefaultPosition, wxDefaultSize , wxTE_CENTRE | wxTE_READONLY);    
    m_pVOISize->SetBackgroundColour( *wxLIGHT_GREY );
    
    wxBoxSizer *pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Nb. of voxels: " ),wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER ), 0, wxALIGN_CENTER );
    pBoxSizer->Add( m_pVOISize, 0, wxALIGN_CENTER );
    
    m_pPropertiesSizer->Add( pBoxSizer, 0, wxALIGN_CENTER );

    //// 
    
    wxImage bmpColor( MyApp::iconsPath + wxT( "colorSelect.png" ), wxBITMAP_TYPE_PNG );
    
    wxBitmapButton *pBtnSelectColor = new wxBitmapButton( pParent, wxID_ANY, bmpColor );
    
#if !_USE_LIGHT_GUI
    wxButton       *pBtnFlipNormal  = new wxButton( pParent, wxID_ANY, wxT( "Flip Normal" ) );
#endif
    
    pBoxSizer = new wxBoxSizer( wxHORIZONTAL );

#if !_USE_LIGHT_GUI
    pBoxSizer->Add( pBtnFlipNormal,  3, wxEXPAND | wxALL, 1 );
#endif
    
    pBoxSizer->Add( pBtnSelectColor, 1, wxEXPAND | wxALL, 1 );
    m_pPropertiesSizer->Add( pBoxSizer, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );    
    
    // Establish connections
    pParent->Connect( pBtnSelectColor->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnColorRoi ) );
    
#if !_USE_LIGHT_GUI
    pParent->Connect( pBtnFlipNormal->GetId(),  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnVoiFlipNormals ) );
#endif
}

// TODO update the fields to try to hide the remaining things.
void SelectionVOI::updatePropertiesSizer()
{
    SelectionObject::updatePropertiesSizer();
}

SelectionVOI::~SelectionVOI()
{
    delete m_pIsoSurface;
    m_pIsoSurface = NULL;
}