#ifndef ANATOMY_H_
#define ANATOMY_H_

#include "DatasetInfo.h"
#include "../misc/IsoSurface/Vector.h"
#include "../misc/nifti/nifti1_io.h"

#include <wx/tglbtn.h>
#include <stack>
#include <vector>

class SelectionObject;
class MainFrame;
class MySlider;
class PropertiesWindow;
class TensorField;
class wxButton;
class wxRadioButton;
class wxSlider;
class wxStaticText;
class wxTextCtrl;

struct SubTextureBox {
    int x;
    int y;
    int z;
    int width;
    int height;
    int depth;

    int datasize;
    std::vector<float> data;
};

/**
* This class represents a dataset related to an anatomy file.
* This can either be a T1 acquisition, or any other 
* acquisition modality that can be used as an anatomy reference.
*/
class Anatomy : public DatasetInfo
{
friend class PropertiesWindow;
    
    enum AxialOrientation
    {
        ORIENTATION_LEFT_TO_RIGHT,
        ORIENTATION_RIGHT_TO_LEFT,
        ORIENTATION_UNDEFINED
    };
    enum SagOrientation
    {
        ORIENTATION_ANT_TO_POST,
        ORIENTATION_POST_TO_ANT,
    };

public:
    //constructor/destructor
    Anatomy();
    Anatomy( const wxString &filename );
    Anatomy( const Anatomy * const pAnatomy, bool createOffset );
    Anatomy( std::vector<float> *pDataset, const int sample );
    Anatomy( const int type );
    Anatomy( const wxString &filename, const int type );
    virtual ~Anatomy();

    void add( Anatomy* anatomy);

    float at( const int i ) const;
    unsigned int getSize() { return m_floatDataset.size(); }
    std::vector<float>* getFloatDataset();
    std::vector<float>* getEqualizedDataset();
    void setFloatDataset(std::vector<float>& dataset) { m_floatDataset = dataset; }

    MySlider            *m_pSliderFlood;
    MySlider            *m_pSliderGraphSigma;
    wxStaticText        *m_pLblThres;

    GLuint getGLuint();

    void setZero(    const int sizeX, const int sizeY, const int sizeZ );
    void setRGBZero( const int sizeX, const int sizeY, const int sizeZ );

    TensorField* getTensorField();

    void dilate();
    void erode();
    void minimize();

    void writeVoxel( Vector v, int layer, int ds, bool dr, bool d3d, wxColor colorRGB ) { writeVoxel(v.x, v.y, v.z, layer, ds, dr, d3d, colorRGB); };
    //void eraseVoxel( Vector v, float ds, bool dr, bool d3d ) { eraseVoxel(v.x, v.y, v.z, ds, dr, d3d); };
    void writeVoxel( const int x, const int y, const int z, const int layer, const int size, const bool isRound, const bool draw3d, wxColor colorRGB );
    //void eraseVoxel( const int x, const int y, const int z, const int size, const bool isRound, const bool draw3d );
    SubTextureBox getStrokeBox( const int x, const int y, const int z, const int layer, const int size, const bool draw3d );

    void flipAxis( AxisType axe );

    void draw(){};

    bool load( nifti_image *pHeader, nifti_image *pBody );
    virtual bool save( wxXmlNode *pNode, const wxString &rootPath ) const;
    void saveNifti( wxString fileName );
    void saveToNewFilename( const wxString &fullPath );

    void setDataType( const int type) { m_dataType = type; }
    int  getDataType()                { return m_dataType; }
    bool usingEqualizedDataset()      { return m_useEqualizedDataset; }

    virtual void createPropertiesSizer( PropertiesWindow *pParentWindow );
    virtual void updatePropertiesSizer();

    float getFloodThreshold()           { return m_floodThreshold; };
    void  setFloodThreshold(float v)    { m_floodThreshold = v; };
    float getGraphSigma()               { return m_graphSigma; };
    void  setGraphSigma(float v)        { m_graphSigma = v; };

    void  toggleSegment()
    {
        m_isSegmentOn = !m_isSegmentOn;
        m_pToggleSegment->SetValue( m_isSegmentOn ); 
    }

    void pushHistory();
    void popHistory(bool isRGB);

    bool toggleEqualization();
    void equalizationSliderChange();
    void generateTexture();
    

public:
    bool  m_isSegmentOn;

private:
    wxButton        *m_pBtnCut;
    wxButton        *m_pBtnEdgeDetect;
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
    wxTextCtrl      *m_pTxtThres;
    wxSlider        *m_pLowerEqSlider;
    wxSlider        *m_pUpperEqSlider;

    void createOffset( const Anatomy * const pAnatomy );
    void edgeDetect( const Anatomy * const pAnatomy );
    double xxgauss( const double x, const double sigma );   
    
    void dilateInternal( std::vector<bool> &workData, int curIndex );
    void erodeInternal(  std::vector<bool> &workData, int curIndex );

    void equalizeHistogram();

    void updateTexture( SubTextureBox drawZone, const bool isRound, float color );
    void updateTexture( SubTextureBox drawZone, const bool isRound, wxColor colorRGB );
    void fillHistory(const SubTextureBox drawZone, bool isRGB);

    void generateGeometry() {};
    void initializeBuffer() {};
    void smooth()           {};
    
    // Created to work around the virtual qualifier of flipAxis(...)
    void flipAxisInternal( AxisType axe, const bool regenerateDisplayObjects );

    std::stack< std::stack< SubTextureBox > >    m_drawHistory;

    float                   m_floodThreshold;
    float                   m_graphSigma;
    std::vector<float>      m_floatDataset;
    std::vector<float>      m_equalizedDataset; // Dataset having its histogram equalized
    int                     m_dataType;
    TensorField             *m_pTensorField;

    bool                    m_useEqualizedDataset;
    unsigned int            m_lowerEqThreshold;
    unsigned int            m_upperEqThreshold;
    unsigned int            m_currentLowerEqThreshold;
    unsigned int            m_currentUpperEqThreshold;
    unsigned int            m_cdf[256];
    
    AxialOrientation        m_originalAxialOrientation;
    SagOrientation          m_originalSagOrientation;
};

#endif /* ANATOMY_H_ */
