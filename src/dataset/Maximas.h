/////////////////////////////////////////////////////////////////////////////
// Name:            Maximas.h
// Author:          Maxime Chamberland
/////////////////////////////////////////////////////////////////////////////
#ifndef MAXIMAS_H_
#define MAXIMAS_H_

#include "DatasetInfo.h"
#include "../misc/nifti/nifti1_io.h"

class Maximas
{
public:
    // Constructor/Destructor
    //Maximas();
    Maximas( const wxString &filename );
    virtual ~Maximas();

    // From DatasetInfo
    void draw();
    bool load( nifti_image *pHeader, nifti_image *pBody );

    // Functions
    virtual void createPropertiesSizer( PropertiesWindow *parent );
    virtual void updatePropertiesSizer();
    
    std::vector<std::vector<Vector> >   m_mainDirections;

};

#endif /* MAXIMAS_H_ */
