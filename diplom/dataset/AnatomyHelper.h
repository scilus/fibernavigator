/*
 * AnatomyHelper.h
 *
 *  Created on: 23.07.2008
 *      Author: ralph
 */

#ifndef ANATOMYHELPER_H_
#define ANATOMYHELPER_H_

#include "../gfx/Shader.h"
#include "DatasetHelper.h"

class DatasetHelper;

class AnatomyHelper {
public:
	AnatomyHelper(DatasetHelper*);
	virtual ~AnatomyHelper();

	void renderMain();
	void renderFreeSlize();
	void renderCrosshair();
	void renderNav(int, Shader*);

	void render() {};


private:
	void renderAxial();
	void renderCoronal();
	void renderSagittal();

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

	float m_x;
	float m_y;
	float m_z;
	float m_xb;
	float m_yb;
	float m_zb;
	float m_xc;
	float m_yc;
	float m_zc;

	DatasetHelper* m_dh;
};

#endif /* ANATOMYHELPER_H_ */
