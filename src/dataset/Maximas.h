/////////////////////////////////////////////////////////////////////////////
// Name:            Maximas.h
// Author:          Maxime Chamberland
/////////////////////////////////////////////////////////////////////////////
#ifndef MAXIMAS_H_
#define MAXIMAS_H_

#include "DatasetInfo.h"
#include "../misc/nifti/nifti1_io.h"

class Maximas : public DatasetInfo
{
public:
    // Constructor/Destructor
    //Maximas();
    Maximas( const wxString &filename );
    virtual ~Maximas();

    // From DatasetInfo
    bool load( nifti_image *pHeader, nifti_image *pBody );

    // PropertiesSizer
    virtual void createPropertiesSizer( PropertiesWindow *parent );
    virtual void updatePropertiesSizer();
    
    //Empty
    void    draw()                      {};
    void    smooth()                    {};
    void    flipAxis( AxisType i_axe )  {};
    void    drawVectors()               {};
    void    generateTexture()           {};
    void    generateGeometry()          {};
    void    initializeBuffer()          {};
    GLuint  getGLuint() { return 0; }

private:
    std::vector<std::vector<Vector> >   m_mainDirections;

};

#endif /* MAXIMAS_H_ */
