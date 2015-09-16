/////////////////////////////////////////////////////////////////////////////
// Name:            SelectionEllipsoid.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/26/2009
//
// Description: SelectionEllipsoid class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef SELECTIONELLIPSOID_H_
#define SELECTIONELLIPSOID_H_

#include "SelectionObject.h"

class SelectionEllipsoid : public SelectionObject
{
public:
    // Constructor / Destructor
    SelectionEllipsoid( Vector i_center, Vector i_size );
    SelectionEllipsoid( Vector i_center, Vector i_size, Vector magnet );
    SelectionEllipsoid( const wxXmlNode selObjNode );
    virtual ~SelectionEllipsoid();

    // Function from SelectionObject (virtual pure)
    hitResult hitTest( Ray* i_ray );

    // Function from SelectionObject (virtual)
    void objectUpdate();
    
    wxString getTypeTag() const;

private:
    // Function from SelectionObject (virtual pure)
    void drawObject( GLfloat* i_color );

    float m_xRadius;
    float m_yRadius;
    float m_zRadius;
};

#endif /*SELECTIONELLIPSOID_H_*/
