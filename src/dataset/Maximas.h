/////////////////////////////////////////////////////////////////////////////
// Name:            Maximas.h
// Author:          Maxime Chamberland
/////////////////////////////////////////////////////////////////////////////
#ifndef MAXIMAS_H_
#define MAXIMAS_H_

#include "DatasetInfo.h"
#include "Glyph.h"
#include "../misc/nifti/nifti1_io.h"

enum DISPLAY { SLICES, WHOLE };

class Maximas : public Glyph
{
public:
    // Constructor/Destructor
    //Maximas();
    Maximas( const wxString &filename );
    virtual ~Maximas();
    bool save( wxXmlNode *pNode, const wxString &rootPath ) const;

    std::vector< std::vector<float> > *getMainDirData()                       { return &m_mainDirections;           };

    // From DatasetInfo
    bool load( nifti_image *pHeader, nifti_image *pBody );
    bool createMaximas( std::vector<std::vector<Vector> > &mainDirections);
    void saveNifti( wxString fileName );
    
    void changeDisplay( DISPLAY value )                   { m_displayType = value; }
    bool isDisplay( DISPLAY disp )                        { return m_displayType == disp; }

    // PropertiesSizer
    virtual void createPropertiesSizer( PropertiesWindow *parent );
    virtual void updatePropertiesSizer();
    
    //Empty
    void    draw();
    void         flipAnat( AxisType axe );

private:
    
    bool createStructure  ( std::vector< float > &i_fileFloatData );
    void drawGlyph        ( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis );
    void setScalingFactor( float i_scalingFactor );
    
    std::vector<std::vector<float> >   m_mainDirections;
    std::vector< float > l_fileFloatData;
    DISPLAY m_displayType;
    int m_dataType;

};

#endif /* MAXIMAS_H_ */
