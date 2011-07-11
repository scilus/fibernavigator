/////////////////////////////////////////////////////////////////////////////
// Name:            SelectionBox.h
// Author:          ---
// Creation Date:   ---
//
// Description: SelectionBox class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef SELECTIONBOX_H_
#define SELECTIONBOX_H_

#include "SelectionObject.h"

class Anatomy;

class SelectionBox : public SelectionObject
{
public:
    // Constructor / Destructor
    SelectionBox( Vector i_center, Vector i_size, DatasetHelper* i_datasetHelper );
    SelectionBox( DatasetHelper* i_datasetHelper, Anatomy* i_anatomy );
    ~SelectionBox();

    // Fonction from SelectionObject (virtual pure)
    hitResult hitTest( Ray* i_ray );

    virtual void createPropertiesSizer(PropertiesWindow *parent);
    virtual void updatePropertiesSizer();

private:
    // Fonction from SelectionObject (virtual pure)
    void      drawObject( GLfloat* i_color );

    // Functions
    void     draw1();
    void     draw2();
    void     draw3();
    void     draw4();
    void     draw5();
    void     draw6();
};

#endif /*SELECTIONBOX_H_*/
