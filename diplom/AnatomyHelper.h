/*
 * AnatomyHelper.h
 *
 *  Created on: 23.07.2008
 *      Author: ralph
 */

#ifndef ANATOMYHELPER_H_
#define ANATOMYHELPER_H_

#include "GLSL/GLSLShaderProgram.h"
#include "DatasetHelper.h"

class DatasetHelper;

class AnatomyHelper {
public:
	AnatomyHelper(DatasetHelper*);
	virtual ~AnatomyHelper();

	void renderMain();
	void renderNav(int, FGLSLShaderProgram*);

	void render() {};


private:
	void renderA1();
	void renderA2();
	void renderA3();
	void renderA4();
	void renderC1();
	void renderC2();
	void renderC3();
	void renderC4();
	void renderS1();
	void renderS2();
	void renderS3();
	void renderS4();

	int x;
	int y;
	int z;
	int xb;
	int yb;
	int zb;
	float xc;
	float yc;
	float zc;

	DatasetHelper* m_dh;
};

#endif /* ANATOMYHELPER_H_ */
