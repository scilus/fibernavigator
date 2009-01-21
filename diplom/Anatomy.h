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
#include "lic/TensorField.h"
#include "selectionBox.h"
#include "nifti/nifti1_io.h"

class Anatomy : public DatasetInfo , public wxTreeItemData
{

public:
	Anatomy(DatasetHelper*);
	virtual ~Anatomy();

	bool load(wxString filename);
	void draw() {};
	GLuint getGLuint();

	float* getFloatDataset();
	TensorField* getTensorField() { return m_tensorField; };
	bool loadNifti(wxString filename);

	SelectionBox* m_roi;

private:
	void generateTexture();
	void generateGeometry() {};
	void initializeBuffer() {};
	void clean() {};
	void smooth() {};
	void activateLIC() {};

	float *m_floatDataset;
	TensorField* m_tensorField;
};

#endif /* ANATOMY_H_ */
