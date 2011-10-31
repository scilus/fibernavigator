/*
 *  The Anatomy class declaration.
 *
 */

#ifndef ANATOMY_H_
#define ANATOMY_H_

#include "DatasetInfo.h"
#include "Surface.h"
#include "../misc/lic/TensorField.h"

#include <vector>

class SelectionObject;
class MainFrame;
class PropertiesWindow;

/**
* This class represents a dataset related to an anatomy file.
* This can either be a T1 acquisition, or any other 
* acquisition modality that can be used as an anatomy reference.
*/
class Anatomy : public DatasetInfo , public wxTreeItemData
{
friend class PropertiesWindow;

public:
    //constructor/destructor
    Anatomy( DatasetHelper *pDatasetHelper );
    Anatomy( DatasetHelper *pDatasetHelper, std::vector<float> *pDataset );
    Anatomy( DatasetHelper *pDatasetHelper, std::vector<float> *pDataset, const int sample );
    Anatomy( DatasetHelper *pDatasetHelper, const int type );
    virtual ~Anatomy();
   
   
    float at( const int i );
    std::vector<float>* getFloatDataset();
    std::vector<float>* getEqualizedDataset();

    MySlider            *m_pSliderFlood;
    MySlider            *m_pSliderGraphSigma;
    wxStaticText        *m_pTextThres;
  
    GLuint getGLuint();
    
    void setZero(    const int sizeX, const int sizeY, const int sizeZ );
    void setRGBZero( const int sizeX, const int sizeY, const int sizeZ );
    
    TensorField* getTensorField();
 
    void dilate();
    void erode();
    void minimize();
    void flipAxis( AxisType axe );
    
    void draw(){};
    
    bool load     ( wxString fileName );
    bool loadNifti( wxString fileName );
    void saveNifti( wxString fileName );

    void setDataType( const int type) { m_dataType = type; }
    int  getDataType()                { return m_dataType; }
    
    virtual void createPropertiesSizer( PropertiesWindow *pParentWindow );
    virtual void updatePropertiesSizer();
    
    float getFloodThreshold()           { return m_floodThreshold; };
    void  setFloodThreshold(float v)    { m_floodThreshold = v; };
    float getGraphSigma()               { return m_graphSigma; };
    void  setGraphSigma(float v)        { m_graphSigma = v; };
    
    bool  m_isSegmentOn;
    void  toggleSegment()
    { 
        m_isSegmentOn = !m_isSegmentOn; 
        m_pToggleSegment->SetValue(m_isSegmentOn); 
    }

    bool toggleEqualization();
   
public:
    SelectionObject *m_pRoi;

private:
    wxButton        *m_pBtnCut;
    wxButton        *m_pBtnMinimize;
    wxButton        *m_pBtnDilate;
    wxButton        *m_pBtnErode;
    wxButton        *m_pBtnNewIsoSurface;
    wxButton        *m_pBtnNewDistanceMap;
    wxButton        *m_pBtnNewOffsetSurface;
    wxButton        *m_pBtnNewVOI;
    wxToggleButton  *m_pToggleSegment;
    wxToggleButton  *m_pEqualize;
    wxRadioButton   *m_pRadioBtnFlood;
    wxRadioButton   *m_pRadioBtnBck;
    wxRadioButton   *m_pRadioBtnObj;
    wxButton        *m_pBtnGraphCut;
    wxButton        *m_pBtnKmeans;
    wxTextCtrl      *m_pTxtThresBox;

    void activateLIC() {};
    void clean()       {};

    void createOffset( const std::vector<float> &sourceDataset );
    double xxgauss( const double x, const double sigma );   
    
    void dilateInternal( std::vector<bool> &workData, int curIndex );
    void erodeInternal(  std::vector<bool> &workData, int curIndex );

    void equalizeHistogram();
    
    void generateTexture();
    void generateGeometry() {};
    void initializeBuffer() {};
    void smooth()           {};
    
    float                   m_floodThreshold;
    float                   m_graphSigma;
    std::vector<float>      m_floatDataset;
    std::vector<float>      m_equalizedDataset; // Dataset having its histogram equalized
    int                     m_dataType;
    TensorField             *m_pTensorField;
    bool                    m_useEqualizedDataset;
};

#endif /* ANATOMY_H_ */
