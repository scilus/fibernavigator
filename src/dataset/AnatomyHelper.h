/*
 * AnatomyHelper.h
 *
 *  Created on: 23.07.2008
 *      Author: ralph
 */

#ifndef ANATOMYHELPER_H_
#define ANATOMYHELPER_H_

#include "../gfx/ShaderProgram.h"

class AnatomyHelper
{
public:
    //constructor/destructor
    AnatomyHelper();
    virtual ~AnatomyHelper();

    //functions
    void renderCrosshair();
    void renderMain();
    void renderNav( int );
    
private:
    //functions
    void renderAxial();
    void renderCoronal();
    void renderSagittal();

    //variables
    float m_x;
    float m_y;
    float m_z;
    float m_xb;
    float m_yb;
    float m_zb;
    float m_xc;
    float m_yc;
    float m_zc;
};

#endif /* ANATOMYHELPER_H_ */
