/////////////////////////////////////////////////////////////////////////////
// Name:            Tensors.cpp
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/07/2009
//
// Description: This is the implementation file for Tensors class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#include "Tensors.h"

#include <GL/glew.h>
#include "../misc/nifti/nifti1_io.h"
#include "../gui/mainFrame.h"

///////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////
Tensors::Tensors( DatasetHelper* i_datasetHelper ) :
    Glyph              ( i_datasetHelper )
{
    m_isNormalized = false;
    m_scalingFactor = 5.0f;
    m_currentLOD = LOD_3;
    // Generating hemispheres
    generateSpherePoints( m_scalingFactor/5 );
    }

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
Tensors::~Tensors()
{
}

///////////////////////////////////////////////////////////////////////////
// This function will load a ODFs type Nifty file.
//
// i_fileName       : The name of the file to load.
///////////////////////////////////////////////////////////////////////////
bool Tensors::load( wxString i_fileName )
{
    m_fullPath = i_fileName;

#ifdef __WXMSW__
    m_name = i_fileName.AfterLast( '\\' );
#else
    m_name = i_fileName.AfterLast( '/' );
#endif

    return loadNifti( i_fileName );
}

///////////////////////////////////////////////////////////////////////////
// This function will load a tensor type Nifty file.
//
// i_fileName       : The name of the file to load.
// Returns true if succesfull, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool Tensors::loadNifti( wxString i_fileName )
{
    char* l_hdrFile;
    l_hdrFile = (char*)malloc( i_fileName.length() + 1 );
    strcpy( l_hdrFile, (const char*)i_fileName.mb_str( wxConvUTF8 ) );

    nifti_image* l_image = nifti_image_read( l_hdrFile, 0 );

    if( ! l_image )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "nifti file corrupt, cannot create nifti image from header" );
        return false;
    }

    m_datasetHelper.m_columns = l_image->dim[1]; //93
    m_datasetHelper.m_rows    = l_image->dim[2]; //116
    m_datasetHelper.m_frames  = l_image->dim[3]; //93
    m_bands                   = l_image->dim[4];

    m_datasetHelper.m_xVoxel = l_image->dx;
    m_datasetHelper.m_yVoxel = l_image->dy;
    m_datasetHelper.m_zVoxel = l_image->dz;

    if( l_image->datatype != 16  || m_bands != 6 )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "not a valid tensor file format" );
        return false;
    }

    m_type = TENSORS;

    nifti_image* l_fileData = nifti_image_read( l_hdrFile, 1 );
    if( ! l_fileData )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "nifti file corrupt" );
        return false;
    }

    int l_nSize = l_image->dim[1] * l_image->dim[2] * l_image->dim[3];

    float* l_data = (float*)l_fileData->data;
    
    vector< float > l_fileFloatData;    
    l_fileFloatData.resize( l_nSize * m_bands );

    // We need to do a bit of moving around with the data in order to have it like we want.
    for( int i = 0; i < l_nSize; ++i )
        for( int j = 0; j < m_bands; ++j )
            l_fileFloatData[i * m_bands + j] = l_data[(j * l_nSize) + i];

    // Once the file has been read succesfully, we need to create the structure 
    // that will contain all the sphere points representing the tensors.
    if( ! createStructure( l_fileFloatData ) )
       return false;
    
    m_isLoaded = true;
    normalize();
    return true;
}

///////////////////////////////////////////////////////////////////////////
// This function will fill up m_tensorsMatrix and m_tensorsFA with the data 
// calculated from i_fileFloatData. 
//
// i_fileFloatData      : The data from wich the tensorField can be constructed.
//
// Returns true if succesfull, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool Tensors::createStructure( vector< float >& i_fileFloatData )
{   
    TensorField l_tensorField( &m_datasetHelper, &i_fileFloatData, 2, 3 );   

    m_nbGlyphs = m_datasetHelper.m_columns * m_datasetHelper.m_rows * m_datasetHelper.m_frames;

    if( l_tensorField.getCells() != m_nbGlyphs )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "problem initializing the tensor field" );
        return false;
    }

    // Set the number of points per glyph.
    m_nbPointsPerGlyph = getLODNbOfPoints( m_currentLOD );

    // Fetching initial sliders positions and loading buffers
    getSlidersPositions( m_currentSliderPos );
    loadBuffer();

    // This vector will contain all the matrix of the tensors to be able to draw them properly.
    m_tensorsMatrix.reserve( m_nbGlyphs );
    // This vector will contain all the FA of the tensors to be able to color them properly.
    m_tensorsFA.reserve( m_nbGlyphs );

    // This is simply to avoid having to calculate this number for every cell of the tensor field.
    int l_rowTimesColumns = m_datasetHelper.m_rows * m_datasetHelper.m_columns;

    // For each tensors in the tensorField.
    for( int z = 0; z < m_datasetHelper.m_frames; ++z )
        for( int y = 0; y < m_datasetHelper.m_rows; ++y )
            for( int x = 0; x < m_datasetHelper.m_columns; ++x )
            {                  
                // Set the info of this tensor so we will be able to draw it.
                setTensorInfo( l_tensorField.getTensorAtIndex( z * l_rowTimesColumns         + 
                                                               y * m_datasetHelper.m_columns +
                                                               x                               ));
            }

    return true;
}

///////////////////////////////////////////////////////////////////////////
// This function will push back the calculated FA in o_FAVector and 
// the calculated matrix in o_matrixVector for the tensor i_FTensor.
//
// i_FTensor        : The tensor we want to extract info from.
// o_matrixVector   : The vector where the calculated matrix will be pushed. 
// o_FAVector       : The vector where the calculated FA will be pushed. 
///////////////////////////////////////////////////////////////////////////
void Tensors::setTensorInfo( FTensor i_FTensor)
{
    FMatrix l_transMatrix = FMatrix( 3, 3 );
    F::FVector l_eigenValues( 0.0, 0.0, 0.0 );
    F::FVector l_eigenVectors[3];
    l_eigenVectors[0].resize( 3 );
    l_eigenVectors[1].resize( 3 );
    l_eigenVectors[2].resize( 3 );

    i_FTensor.getEigenSystem( l_eigenValues, l_eigenVectors );

    int l_count = 0;
    // Make sure we have no negative eigen values.
    if( l_eigenValues[0] < EPSILON )
    {
        l_eigenValues[0] = EPSILON;
        l_count++;
    }
    if( l_eigenValues[1] < EPSILON )
    {
        l_eigenValues[1] = EPSILON;
        l_count++;
    }
    if( l_eigenValues[2] < EPSILON )
    {
        l_eigenValues[2] = EPSILON;
        l_count++;
    }

    // This means that all the eigen values where < EPSILON, we set this tensor 
    // matrix and FA to some default values because we will not display this tensor anyway.
    if( l_count < 3 )
    {
        //slow on linux
        //l_eigenVectors[0].normalize();
        //l_eigenVectors[1].normalize();
        //l_eigenVectors[2].normalize();
        //l_eigenValues.normalize();
        
        l_eigenValues[0]*=VISUALIZATION_FACTOR;
        l_eigenValues[1]*=VISUALIZATION_FACTOR;
        l_eigenValues[2]*=VISUALIZATION_FACTOR;
        
        l_transMatrix( 0, 0 ) = l_eigenValues[0] * l_eigenVectors[0][0];
        l_transMatrix( 1, 0 ) = l_eigenValues[0] * l_eigenVectors[0][1];
        l_transMatrix( 2, 0 ) = l_eigenValues[0] * l_eigenVectors[0][2];

        l_transMatrix( 0, 1 ) = l_eigenValues[1] * l_eigenVectors[1][0];
        l_transMatrix( 1, 1 ) = l_eigenValues[1] * l_eigenVectors[1][1];
        l_transMatrix( 2, 1 ) = l_eigenValues[1] * l_eigenVectors[1][2];

        l_transMatrix( 0, 2 ) = l_eigenValues[2] * l_eigenVectors[2][0];
        l_transMatrix( 1, 2 ) = l_eigenValues[2] * l_eigenVectors[2][1];
        l_transMatrix( 2, 2 ) = l_eigenValues[2] * l_eigenVectors[2][2];
    }
    // Saves the FA of this tensor to be able to set it's color when drawing it.
    m_tensorsFA.push_back( Helper::getFAFromEigenValues( l_eigenValues[0], l_eigenValues[1], l_eigenValues[2] ) );
    // Saves the matrix of this tensor to be able to deform it's points when drawing it.
    m_tensorsMatrix.push_back( l_transMatrix );
    //Saves eigen values
    m_tensorsEigenValues.push_back( l_eigenValues );
}

void Tensors::normalize()
{
    int len = m_datasetHelper.m_columns*m_datasetHelper.m_rows*m_datasetHelper.m_frames;
    for (int i=0; i<len; i++)
    {    
        double correction = sqrt(m_tensorsEigenValues[i][0]*m_tensorsEigenValues[i][0] + m_tensorsEigenValues[i][1]*m_tensorsEigenValues[i][1] + m_tensorsEigenValues[i][2]*m_tensorsEigenValues[i][2]);
        if (m_isNormalized)
        {
            correction = 1/correction;
        }
        m_tensorsMatrix[i]( 0, 0 ) /= correction;
        m_tensorsMatrix[i]( 1, 0 ) /= correction;
        m_tensorsMatrix[i]( 2, 0 ) /= correction;

        m_tensorsMatrix[i]( 0, 1 ) /= correction;
        m_tensorsMatrix[i]( 1, 1 ) /= correction;
        m_tensorsMatrix[i]( 2, 1 ) /= correction;

        m_tensorsMatrix[i]( 0, 2 ) /= correction;
        m_tensorsMatrix[i]( 1, 2 ) /= correction;
        m_tensorsMatrix[i]( 2, 2 ) /= correction;
    }
    m_isNormalized = !m_isNormalized;
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the tensors.
///////////////////////////////////////////////////////////////////////////
void Tensors::draw()
{      
    // Enable the tensor shader.    
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->bind();
    
    glBindTexture( GL_TEXTURE_1D, m_textureId );

    // This is the color look up table texture.
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniSampler( "clut",          0                         );
    // This is the alpha level of the tensors.
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniFloat  ( "alpha",         DatasetInfo::m_alpha      );
    // This is the value for the lighting attenuation.
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniFloat  ( "attenuation",   m_lighAttenuation         );
    // This is the value for the light direction.
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUni3Float ( "lightPosition", m_lightPosition           );
    // This is the brightness level of the tensors.
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniFloat  ( "brightness",    DatasetInfo::m_brightness );
    // If m_colorWithPosition is true then the glyph will be colored with the position of the vertex.
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniInt    ( "colorWithPos", (GLint)m_colorWithPosition );
 
    Glyph::draw();

    // Disable the tensor color shader.
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->release();
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the tensor located at the specified voxel position.
//
// i_zIndex         : The z voxel of the tensor.
// i_yIndex         : The y voxel of the tensor.
// i_xIndex         : The x voxel of the tensor.
///////////////////////////////////////////////////////////////////////////
void Tensors::drawGlyph( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis )
{
    // To quiet down the compiler warning, this param is not used for the tensor.
    // Before we start calculating everything, 
    // lets make sure that the glyph is visible on the screen (inside the frustum).
    // To make things faster and easier, we use the glyph voxel as its bounding box.
    // The first vector represent the voxel center and the second one represend the tensor size.
    if( ! boxInFrustum( Vector( ( i_xVoxel + 0.5f ) * m_datasetHelper.m_xVoxel,
                                ( i_yVoxel + 0.5f ) * m_datasetHelper.m_yVoxel,
                                ( i_zVoxel + 0.5f ) * m_datasetHelper.m_zVoxel ),
                        Vector( m_datasetHelper.m_xVoxel / 2.0f,
                                m_datasetHelper.m_yVoxel / 2.0f,
                                m_datasetHelper.m_zVoxel / 2.0f ) ) )
        return;
   
    int l_tensorNumber = i_zVoxel * m_datasetHelper.m_columns * m_datasetHelper.m_rows + 
                         i_yVoxel * m_datasetHelper.m_columns                          +
                         i_xVoxel;
    
    float l_tensorFA = m_tensorsFA[l_tensorNumber];
    // When we saved the informations for the tensors, the FA was set to -1.0f 
    // for tensors that were garbage, we dont display those.
    if( l_tensorFA < 0.0f )
        return;

    // Lets set the offset of this tensor.
    GLfloat l_offset[3];
    getVoxelOffset( i_zVoxel, i_yVoxel, i_xVoxel, l_offset );
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUni3Float( "offset", l_offset );

    // Lets set the color to draw this tensor.
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setAttribFloat( "color", l_tensorFA );

    // Set axis flip.
    GLfloat l_flippedAxes[3];
    m_flippedAxes[0] ? l_flippedAxes[0] = -1.0f : l_flippedAxes[0] = 1.0f;
    m_flippedAxes[1] ? l_flippedAxes[1] = -1.0f : l_flippedAxes[1] = 1.0f;
    m_flippedAxes[2] ? l_flippedAxes[2] = -1.0f : l_flippedAxes[2] = 1.0f;
    
    DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUni3Float(   "axisFlip",    
                                                                        l_flippedAxes);


    if (getDisplayShape()== NORMAL || getDisplayShape()== SPHERE ){
        if (isDisplayShape(SPHERE)){
            // If we want to draw this tensor on a perfect sphere instead of its deformed way,
            // then we set the tensor matrix to be a identity matrix devided by the sacling 
            // factor because the sphere in the video memory is already multiplied by that factor. 
            float factor = 5 / getScalingFactor();
            FMatrix l_noDeformMatrix( 3, 3);
            l_noDeformMatrix( 0, 0 ) = factor;
            l_noDeformMatrix( 0, 1 ) = 0.0f;
            l_noDeformMatrix( 0, 2 ) = 0.0f;
            l_noDeformMatrix( 1, 0 ) = 0.0f;
            l_noDeformMatrix( 1, 1 ) = factor;
            l_noDeformMatrix( 1, 2 ) = 0.0f;
            l_noDeformMatrix( 2, 0 ) = 0.0f;
            l_noDeformMatrix( 2, 1 ) = 0.0f;
            l_noDeformMatrix( 2, 2 ) = factor;
            // Lets set the matrix of this tensor so the shader will be able to deform it properly.
            DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniMatrix3f( "tensorMatrix", l_noDeformMatrix );
        } else {
            // Lets set the matrix of this tensor so the shader will be able to deform it properly.
            DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniMatrix3f( "tensorMatrix", m_tensorsMatrix[l_tensorNumber] );
        }
        if( ! m_datasetHelper.m_useVBO  )
        {
            glVertexPointer( 3, GL_FLOAT, 0, &m_LODspheres[m_currentLOD][0] ); 
        }
        else
        {
            glBindBuffer( GL_ARRAY_BUFFER, *m_hemisphereBuffer );
            glVertexPointer( 3, GL_FLOAT, 0, 0 );
        } 
        // Lets set the radius modifier.
        DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniInt( "displayControl", 0 );
        // Depending on the orientation of the glyph we do a front or back face culling.
        m_axisFlippedToggled ? glCullFace( GL_FRONT ) : glCullFace( GL_BACK );
        // Draw the first half of the tensor.
        glDrawArrays( GL_TRIANGLE_STRIP, 0, m_nbPointsPerGlyph );

        // Lets set the radius modifier.
        DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniInt( "displayControl", 1 );
        // Depending on the orientation of the glyph we do a front or back face culling.
        m_axisFlippedToggled ? glCullFace( GL_BACK ) : glCullFace( GL_FRONT );
        // Draw the other half of the tensor.
        glDrawArrays( GL_TRIANGLE_STRIP, 0, m_nbPointsPerGlyph );    
    } 
    else if (isDisplayShape(AXES) || isDisplayShape(AXIS)){
        glDisable( GL_CULL_FACE );
        // Lets set the matrix of this tensor so the shader will be able to deform it properly.
        DatasetInfo::m_dh->m_shaderHelper->m_tensorsShader->setUniMatrix3f( "tensorMatrix", m_tensorsMatrix[l_tensorNumber] );
        
        float l_factor = m_scalingFactor/3;
        if (isDisplayShape(AXIS)){
            //display the main axis   

            float lvx = m_tensorsMatrix[l_tensorNumber](0,0)*m_tensorsMatrix[l_tensorNumber](0,0)
                + m_tensorsMatrix[l_tensorNumber](1,0)*m_tensorsMatrix[l_tensorNumber](1,0) 
                + m_tensorsMatrix[l_tensorNumber](2,0)*m_tensorsMatrix[l_tensorNumber](2,0);
            float lvy = m_tensorsMatrix[l_tensorNumber](0,1)*m_tensorsMatrix[l_tensorNumber](0,1)
                + m_tensorsMatrix[l_tensorNumber](1,1)*m_tensorsMatrix[l_tensorNumber](1,1) 
                + m_tensorsMatrix[l_tensorNumber](2,1)*m_tensorsMatrix[l_tensorNumber](2,1);
            float lvz = m_tensorsMatrix[l_tensorNumber](0,2)*m_tensorsMatrix[l_tensorNumber](0,2)
                + m_tensorsMatrix[l_tensorNumber](1,2)*m_tensorsMatrix[l_tensorNumber](1,2) 
                + m_tensorsMatrix[l_tensorNumber](2,2)*m_tensorsMatrix[l_tensorNumber](2,2);
            
            glBegin(GL_LINES); 
            if (lvx>lvy && lvx>lvz) {
                glVertex3f(-l_factor,0,0);
                glVertex3f(l_factor,0,0); 
            } else if (lvy>lvx && lvy>lvz) { 
                glVertex3f(0,-l_factor,0);
                glVertex3f(0,l_factor,0);  
            } else {  
                glVertex3f(0,0,-l_factor);
                glVertex3f(0,0,l_factor); 
            }
            glEnd();           
          
        } else {           
            //display the 3 axes
            glBegin(GL_LINES);  
                glVertex3f(-l_factor,0,0);
                glVertex3f(l_factor,0,0);
                glVertex3f(0,-l_factor,0);
                glVertex3f(0,l_factor,0);
                glVertex3f(0,0,-l_factor);
                glVertex3f(0,0,l_factor);        
            glEnd();
         }   
    }
}



///////////////////////////////////////////////////////////////////////////
// This function will set a specific scaling factor for the glyph.
//
// i_scalingFactor      : The glyph scaling factor.
///////////////////////////////////////////////////////////////////////////
void Tensors::setScalingFactor( float i_scalingFactor )
{
    m_scalingFactor = i_scalingFactor;
    generateSpherePoints( m_scalingFactor/5 );   
    loadBuffer();
}

void Tensors::createPropertiesSizer(PropertiesWindow *parent)
{
    Glyph::createPropertiesSizer(parent);
    m_pbtnNormalize = new wxToggleButton(parent, wxID_ANY, wxT("Normalize"), wxDefaultPosition, wxSize(140,-1));    
    parent->Connect(m_pbtnNormalize->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,wxCommandEventHandler(PropertiesWindow::OnNormalizeTensors));
    m_propertiesSizer->Add(m_pbtnNormalize,0,wxALIGN_CENTER);

    m_pradiobtnAxes  = new wxRadioButton(parent, wxID_ANY, _T( "Tensors Axes" ), wxDefaultPosition, wxSize(132,-1));    
    m_psizerDisplay->Add(m_pradiobtnAxes);
    parent->Connect(m_pradiobtnAxes->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnGlyphAxesSelected));
    m_pradiobtnAxes->SetValue          (isDisplayShape(AXES));
}

void Tensors::updatePropertiesSizer()
{
    Glyph::updatePropertiesSizer();

    m_pbtnNormalize->SetValue(m_isNormalized);

}

