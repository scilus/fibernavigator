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

class Anatomy : public DatasetInfo , public wxTreeItemData
{

public:
	Anatomy(DatasetHelper*);
	virtual ~Anatomy();

	bool load(wxString filename);
	void draw() {};
	void generateTexture();
	void generateGeometry() {};
	void initializeBuffer() {};
	void drawVectors() {};
	void clean() {};
	void smooth() {};

	wxUint8* getByteDataset();
	float* getFloatDataset();
	TensorField* getTensorField() { return m_tensorField; };

	//bool isInsideTriangle(double, double, double, double, double, double, double, double);
	//void cutRestOfLine(int, int, int);

private:
	wxUint8 *m_byteDataset;
	wxUint16 *m_shortDataset;
	wxUint8 *m_rgbDataset;
	float *m_floatDataset;
	TensorField* m_tensorField;
};

#endif /* ANATOMY_H_ */
