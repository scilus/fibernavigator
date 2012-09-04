/////////////////////////////////////////////////////////////////////////////
// Name:            Maximas.cpp
// Author:          Maxime Chamberland
/////////////////////////////////////////////////////////////////////////////

#include "Maximas.h"

#include "DatasetManager.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gui/MyListCtrl.h"
#include "../gui/SceneManager.h"
#include "../misc/nifti/nifti1_io.h"

#include <GL/glew.h>
#include <wx/math.h>
#include <wx/xml/xml.h>

#include <algorithm>


#include <fstream>


#include <vector>
using std::vector;





Maximas::Maximas( const wxString &filename )
{

}

Maximas::~Maximas()
{

}

bool Maximas::load( nifti_image *pHeader, nifti_image *pBody )
{
    return true;
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the tensors.
///////////////////////////////////////////////////////////////////////////
void Maximas::draw()
{
    
}



void Maximas::createPropertiesSizer( PropertiesWindow *pParent )
{
    
}

void Maximas::updatePropertiesSizer()
{

}


