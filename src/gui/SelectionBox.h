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
    SelectionBox( Vector i_center, Vector i_size );
    SelectionBox( Anatomy* i_anatomy );
    ~SelectionBox();

    // Function from SelectionObject (pure virtual)
    hitResult hitTest( Ray* i_ray );

    virtual void createPropertiesPanel( PropertiesWindow *parent );
    virtual void updatePropertiesPanel();

private:
    // Function from SelectionObject (pure virtual)
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
