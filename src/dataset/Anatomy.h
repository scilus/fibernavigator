/*
 * Anatomy.h
 *
 *  Created on: 07.07.2008
 *      Author: ralph
 */

#ifndef ANATOMY_H_
#define ANATOMY_H_

#include "datasetInfo.h"
#include "surface.h"
#include "../misc/lic/TensorField.h"

class SelectionObject;
class MainFrame;
class Anatomy : public DatasetInfo , public wxTreeItemData
{

public:
    //constructor/destructor
    Anatomy( DatasetHelper *datasetHelper );
    Anatomy( DatasetHelper *datasetHelper, std::vector<float> *dataset);
	Anatomy( DatasetHelper *datasetHelper, std::vector<float> *dataset, int sample);
    Anatomy( DatasetHelper *datasetHelper, int type);
    virtual ~Anatomy();
   
   
   float at( int i );
   std::vector<float>* getFloatDataset();
   MySlider        *m_psliderFlood;
   MySlider		   *m_psliderGraphSigma;
  
   GLuint getGLuint();
   void setRGBZero( int x, int y, int z );
   TensorField* getTensorField();
   void setZero( int x, int y, int z );
    
 
   void dilate();
   void draw()             {};
   void erode();
   void minimize();
   bool load     (wxString fileName);
   bool loadNifti(wxString fileName);
   void saveNifti(wxString fileName);    
   void setDataType(int i_type){m_dataType = i_type;}
   int  getDataType(){return m_dataType;}
   virtual void createPropertiesSizer(MainFrame *parent);
   virtual void updatePropertiesSizer();
   float    getFloodThreshold()                          { return m_floodThreshold;              };
   void     setFloodThreshold(float v)                   { m_floodThreshold = v;                 };
   float    getGraphSigma()								 { return m_graphSigma;                  };
   void     setGraphSigma(float v)                       { m_graphSigma = v;                 };
   bool     isSegmentOn;
   void		toggleSegment()								 { isSegmentOn = !isSegmentOn; m_ptoggleSegment->SetValue(isSegmentOn); };
   


   SelectionObject         *m_roi;
 private:
    wxButton		*m_pbtnCut;
    wxButton		*m_pbtnMinimize;
    wxButton		*m_pbtnDilate;
    wxButton		*m_pbtnErode;
    wxButton		*m_pbtnNewIsoSurface;
    wxButton		*m_pbtnNewDistanceMap;
    wxButton		*m_pbtnNewOffsetSurface;
    wxButton		*m_pbtnNewVOI;
	wxToggleButton  *m_ptoggleSegment;
	wxRadioButton	*m_pradiobtnFlood;
	wxRadioButton	*m_pradiobtnBck;
	wxRadioButton	*m_pradiobtnObj;
	wxButton		*m_pbtnGraphCut;
	wxButton		*m_pbtnKmeans;
	wxStaticText    *m_thresBox;

	
	
 
    void activateLIC()      {};
    void clean()            {};
    void createOffset( std::vector<float>* dataset );
    void dilate1( std::vector<bool>*, int index );
    void erode1( std::vector<bool>*, int index );
    void generateTexture();
    void generateGeometry() {};
    void initializeBuffer() {};
    void smooth()           {};
    double xxgauss( double x, double sigma );   
	
    
	float					m_floodThreshold;
	float				    m_graphSigma;
    std::vector<float>      m_floatDataset;
    int                     m_dataType;
    TensorField             *m_tensorField;
};

#endif /* ANATOMY_H_ */
