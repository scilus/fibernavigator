#ifndef POINT_H_
#define POINT_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GL/glew.h"
#include "../gui/boundingBox.h"
#include "../gui/fnObject.h"
#include "DatasetHelper.h"

class DatasetHelper;

class SplinePoint : public wxTreeItemData, public FNObject
{
public:
    SplinePoint( Vector, DatasetHelper* );
    SplinePoint( double, double, double, DatasetHelper* );
    ~SplinePoint();

    void setCenter( Vector c ) { m_center = c; };
    Vector getCenter()         { return m_center; };
    void draw();

    hitResult hitTest( Ray* ray );
    void drag( wxPoint click, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] );
    void move( float );


    void    select( bool flag );
    void    unselect() { m_selected = false; };

    float   X()             { return m_center.x; };
    float   Y()             { return m_center.y; };
    float   Z()             { return m_center.z; };
    void    setX( float x );
    void    setY( float y ) { m_center.y = y; };
    void    setZ( float z ) { m_center.z = z; };

    void    setOffsetVector ( Vector vec )  { m_offsetVector = vec; };
    Vector  getOffsetVector()               { return m_offsetVector; };

    void    setTreeId( wxTreeItemId treeId ) { m_treeId = treeId; };
    void    setIsBoundary( bool v )          { m_isBoundary = v; };    
    bool    isBoundary()                     { return m_isBoundary; };

    void     setName(wxString i_name)         { m_name = i_name;};
    wxString getName()                       {return m_name;};

    void moveLeft();
    void moveRight();
    void moveForward();
    void moveBack();
    void moveUp();
    void moveDown();
    void moveLeft5();
    void moveRight5();
    void moveForward5();
    void moveBack5();
    void moveUp5();
    void moveDown5();
    virtual void createPropertiesSizer(MainFrame *parent);
    virtual void updatePropertiesSizer();

private:
    void drawSphere( float, float, float, float );

    DatasetHelper*  m_dh;
    Vector          m_center;
    Vector          m_origin;
    Vector          m_offsetVector;
    bool            m_selected;
    bool            m_isBoundary;
    hitResult       m_hr;
    wxTreeItemId    m_treeId;
    wxString        m_name;
    wxTextCtrl      *m_ptxtName;
    wxBitmapButton  *m_pbtnDelete;
};

#endif /*POINT_H_*/
