/////////////////////////////////////////////////////////////////////////////
// Name:            Maximas.h
// Author:          Maxime Chamberland
/////////////////////////////////////////////////////////////////////////////
#ifndef MAXIMAS_H_
#define MAXIMAS_H_

#include "DatasetInfo.h"
#include "Glyph.h"
#include "../misc/nifti/nifti1_io.h"

class Maximas : public Glyph
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
    void    draw();

private:
    bool createStructure  ( std::vector< float > &i_fileFloatData );
    void drawGlyph        ( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis );
    std::vector<std::vector<float> >   m_mainDirections;

};

#endif /* MAXIMAS_H_ */
