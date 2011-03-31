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

class Anatomy : public DatasetInfo , public wxTreeItemData
{

public:
    //constructor/destructor
	Anatomy( DatasetHelper* datasetHelper );
	Anatomy( DatasetHelper* datasetHelper, std::vector<float>* dataset );
	virtual ~Anatomy();

    //set/get functions
    float                   at( int i );
    std::vector<float>*     getFloatDataset();
    GLuint                  getGLuint();
    void                    setRGBZero( int x, int y, int z );
	TensorField*            getTensorField();
    void                    setZero( int x, int y, int z );
	
    //functions
    void dilate();
    void draw()             {};
    void erode();
    void minimize();
	bool load     ( wxString fileName );
    bool loadNifti( wxString fileName );
	void saveNifti( wxString fileName );	
	
	//variables
	SelectionObject*           m_roi;

private:
    //functions
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

	//variables
    std::vector<float>      m_floatDataset;
	TensorField*            m_tensorField;
};

#endif /* ANATOMY_H_ */
